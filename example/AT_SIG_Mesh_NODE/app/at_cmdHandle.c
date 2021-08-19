#include "tl_common.h"
#include "at_cmd.h"
#include "drivers.h"

#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh/remote_prov.h"
#include "mesh/fast_provision_model.h"
#include "mesh/app_heartbeat.h"
#include "mesh/app_health.h"
#include "tinyFlash_Index.h"

#include "stack/ble/ble.h"

#define AT_VERSION "0.3"



#define IS_COMMA(__ch)		((__ch)==',')
#define IS_CR(__ch)			((__ch)==0x0D)
#define IS_LF(__ch)			((__ch)==0x0A)
#define IS_CRLF(__ch)		(IS_CR(__ch)||IS_LF(__ch))
#define CMD_PA_MAX          (40)

extern u8 baud_buf[];
extern  const u8 tbl_scanRsp[];
extern u8 my_scanRsp[32];
extern u8 ATE;
extern u8  mac_public[6];

int my_atoi(const char *src)
{
      int s = 0;
      bool isMinus = false;
  
      while(*src == ' ')  //跳过空白�?
      {
          src++; 
      }
  
      if(*src == '+' || *src == '-')
      {
          if(*src == '-')
          {
              isMinus = true;
          }
          src++;
      }
      else if(*src < '0' || *src > '9')  //如果第一位既不是符号也不是数字，直接返回异常�?
      {
          s = 2147483647;
          return s;
      }
  
      while(*src != '\0' && *src >= '0' && *src <= '9')
      {
          s = s * 10 + *src - '0';
          src++;
      }
      return s * (isMinus ? -1 : 1);
}

int get_atcmd_param(char*str,char*argv[],int size)
{
	int cnt=0;
	char *p;
	char  *pfirst;

	p = str;
	pfirst = p;
	while((*p)&&(cnt<size))
	{
	
		if(IS_COMMA(*p))
		{
	
			if(p-pfirst>=1)
			{
				argv[cnt] = pfirst;
				cnt++;
			}
			*p=0;
			pfirst=p+1;
		}
		else if(IS_CRLF(*p))
		{
	
			if(p-pfirst>=1)
			{
				argv[cnt] = pfirst;
				cnt++;
			}
			*p=0;
			pfirst=p+1;
			break;
		}
		p++;
	}
	
	return cnt;
}

int str2hex(char * pbuf, int len)
{
	int i = 0;
	for(i = 0; i < len; i ++)
	{
		if(((pbuf[i] >= '0') && (pbuf[i] <= '9')) || ((pbuf[i] >= 'A') && (pbuf[i] <= 'F')) || ((pbuf[i] >= 'a') && (pbuf[i] <= 'f')) )
		{
			if((pbuf[i] >= '0') && (pbuf[i] <= '9'))
			{
				pbuf[i] -= '0';
			}
			else if(((pbuf[i] >= 'A') && (pbuf[i] <= 'F')))
			{
				pbuf[i] -= 'A';
				pbuf[i] += 0x0A;
			}else 
			{
				pbuf[i] -= 'a';
				pbuf[i] += 0x0A;
			}

			if(i%2)
			{
				pbuf[i/2] = (pbuf[i-1] << 4) | pbuf[i];
			}
		}
		else
		{
			return -1;
		}
	}

	return 0;
}

u8 HEX2BYTE(u8 hex_ch)
{
	if (hex_ch >= '0' && hex_ch <= '9')
	{
		return hex_ch - '0';
	}

	if (hex_ch >= 'a' && hex_ch <= 'f')
	{
		return hex_ch - 'a' + 10;
	}

	if (hex_ch >= 'A' && hex_ch <= 'F')
	{
		return hex_ch - 'A' + 10;
	}

	return 0x00;
}

u16 HEX2BIN(u8 * p_hexstr, u8 * p_binstr)
{
	u8 bin_len = 0;
	u8 hex_len = strlen((char *)p_hexstr);
	u8 index = 0;

	if (hex_len % 2 == 1)
	{
		hex_len -= 1;
	}

	bin_len = hex_len / 2;
	for(index = 0; index < hex_len; index+=2)
	{
		p_binstr[index/2] = ((HEX2BYTE(p_hexstr[index]) << 4) & 0xF0) + HEX2BYTE(p_hexstr[index + 1]); 
	}

	return bin_len;
}

u16 HEX2U16(u8 * p_hexstr) //�?6进制字符串转换成U16类型的整�?
{
	u8 hexStr_len = strlen((char *)p_hexstr);
	u16 numBer = 0;
	u8 index = 0;

	for(index = 0; index < hexStr_len; index++)
	{
		numBer <<=4;
		numBer += HEX2BYTE(p_hexstr[index]);
	}

	return numBer;
}

u8 STR2U16(u8 * p_hexstr) //�?0进制字符串转换成U16类型的整�?
{
	u8 hexStr_len = strlen((char *)p_hexstr);
	u16 numBer = 0;
	u8 index = 0;

	for(index = 0; index < hexStr_len; index++)
	{
		numBer *= 10;
		numBer += HEX2BYTE(p_hexstr[index]);
	}

	return numBer;
}

static unsigned char atCmd_ATE0(char *pbuf,  int mode, int lenth)
{
	ATE = 0;
	//tinyFlash_Write(STORAGE_ATE, &ATE, 1);
	return 0;
}

static unsigned char atCmd_ATE1(char *pbuf,  int mode, int lenth)
{
	ATE = 1;
	//tinyFlash_Write(STORAGE_ATE, &ATE, 1);
	return 0;
}

static unsigned char atCmd_GMR(char *pbuf,  int mode, int lenth)
{
	at_print("\r\n+VER:"AT_VERSION);
	return 0;
}

static unsigned char atCmd_Reset(char *pbuf,  int mode, int lenth)
{
	at_print("\r\nOK\r\n");
	start_reboot();
	return 0;
}

static unsigned char atCmd_Restore(char *pbuf,  int mode, int lenth)
{
	irq_disable();
	factory_reset();
	at_print("\r\nOK\r\n");
	start_reboot();
	return 0;
}

static unsigned char atCmd_State(char *pbuf,  int mode, int lenth)
{
	if(is_provision_success())
	{
		at_print("\r\nSTATE:1\r\n");
	}
	else
	{
		at_print("\r\nSTATE:0\r\n");
	}
	
	return 0;
}

//AT+SEND2ALI=8421,12345678
static unsigned char atCmd_Send2Ali(char *pbuf,  int mode, int lenth)
{
	char * tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }

	tmp[0] = 0; tmp++;

	u16 op = HEX2U16((u8*)pbuf);  //获取Op Code
	u32 len = lenth -(tmp - pbuf);

	for(int i = 0; i < len; i ++) //�?6进制字符串转换成二进制数�?
	{
		if(((tmp[i] >= '0') && (tmp[i] <= '9')) || ((tmp[i] >= 'A') && (tmp[i] <= 'F')))
		{
			if((tmp[i] >= '0') && (tmp[i] <= '9'))
			{
				tmp[i] -= '0';
			}
			else
			{
				tmp[i] -= 'A';
				tmp[i] += 0x0A;
			}

			if(i%2)
			{
				tmp[i/2] = (tmp[i-1] << 4) | tmp[i];
			}
		}
		else
		{
			return 2;
		}
	}

	mesh_tx_cmd_rsp(op, (u8 *)tmp, len/2, ele_adr_primary, 0xffff, 0, 0);
	return 0;
}

extern const u8 USER_DEFINE_ATT_HANDLE ;
extern u32 device_in_connection_state;//连接状�?
//AT+SEND2APP=5,12345 发送数据到手机
static unsigned char atCmd_Send2App(char *pbuf,  int mode, int lenth)
{
	if(device_in_connection_state == 0) return 2;

	char * tmp = strstr(pbuf,",");
    if(tmp == NULL)
    {
        return AT_RESULT_CODE_ERROR;
    }

	tmp[0] = 0; tmp++;

	u16 data_len = STR2U16((u8*)pbuf); //获取数据长度

	bls_att_pushNotifyData(USER_DEFINE_ATT_HANDLE, (u8*)tmp, data_len); //release
	return 0;
}


static unsigned char atCmd_Baud(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_READ)
	{
		at_print("\r\n+BAUD:");
	switch(baud_buf[0])
	{
		case AT_BAUD_9600:printf("+BAUD:9600\r\n");break;

		case AT_BAUD_19200:printf("+BAUD:19200\r\n");break;
		case AT_BAUD_38400:printf("+BAUD:38400\r\n");break;
		case AT_BAUD_115200:printf("+BAUD:115200\r\n");break;
		default : break;
	}
		printf("+BAUD:%d\r\n",baud_buf[0]);
		return 0;
	}

	if(mode == AT_CMD_MODE_SET)
	{

		int baud=my_atoi(pbuf);
		switch(baud)
		{
			case 9600:baud_buf[0]=AT_BAUD_9600;break;

			case 19200:baud_buf[0]=AT_BAUD_19200;break;
			case 38400:baud_buf[0]=AT_BAUD_38400;break;
			case 115200:baud_buf[0]=AT_BAUD_115200;break;
			default : return 2;break;
		}
		
		
		tinyFlash_Write(STORAGE_BAUD, (unsigned char*)baud_buf, 1);
		return 0;
		#if 0
		if((pbuf[0] >= '0') && (pbuf[0] <= '9'))
		{
			pbuf[0] -= '0';
			baud_buf[0]=pbuf[0];
			tinyFlash_Write(STORAGE_BAUD, (unsigned char*)pbuf, 1);
			return 0;
		}
		else
		{
			return 2;
		}
		#endif
	}
	return 1;
}

static unsigned char atCmd_AliGenie(char *pbuf,  int mode, int lenth)
{
/*	 u32 con_product_id_a;
	 u8  tbl_mac_a [6];
	 char con_sec_data_a[32];
	 u8 idx =0;

	if(mode == AT_CMD_MODE_READ)
	{
		u32 product_id_flash;
		flash_read_page(FLASH_ADR_THREE_PARA_ADR,sizeof(product_id_flash),(u8 *)(&product_id_flash));
		if(product_id_flash == 0xffffffff){
			printf("no info\r\n");
			return 0;
		}
		flash_read_page(FLASH_ADR_THREE_PARA_ADR,sizeof(con_product_id_a),(u8 *)&con_product_id_a);
		idx += sizeof(con_product_id_a);
		flash_read_page(FLASH_ADR_THREE_PARA_ADR+idx,sizeof(tbl_mac_a),tbl_mac_a);// read big endian
		idx += sizeof(tbl_mac);
		flash_read_page(FLASH_ADR_THREE_PARA_ADR+idx,sizeof(con_sec_data_a),(u8 *)con_sec_data_a);
		//at_print("\r\n+BAUD:");
		printf("+product_id:%d\r\n",con_product_id_a);

		printf("+MAC:%02x%02x%02x%02x%02x%02x\r\n",tbl_mac_a[0],tbl_mac_a[1],tbl_mac_a[2],tbl_mac_a[3],tbl_mac_a[4],tbl_mac_a[5]);
		at_print("+product_secret:");
		at_print_array(con_sec_data_a,16);
		return 0;
	}

	if(mode == AT_CMD_MODE_SET)
	{
		char *p[CMD_PA_MAX]={0};

		u8 cnt=get_atcmd_param(pbuf,p,CMD_PA_MAX);
	
		if(cnt!=3)
		{
			return 2;
		}
		con_product_id_a=my_atoi(p[0]);
		if(12!=strlen(p[1])) 
			return 2;
		if(32!=strlen(p[2])) 
			return 2;
	
		if(str2hex(p[1], 12) == -1 ) 
			return 2;
		if(str2hex(p[2], 32) == -1 ) 
			return 2;
		//memcpy(con_sec_data,p[2],16);
		idx =0;
		flash_erase_sector(FLASH_ADR_THREE_PARA_ADR);
		flash_write_page(FLASH_ADR_THREE_PARA_ADR, sizeof(con_product_id_a), &con_product_id_a);
		idx += sizeof(con_product_id_a);
		flash_write_page(FLASH_ADR_THREE_PARA_ADR+idx, sizeof(tbl_mac_a), p[1]);
		idx += sizeof(tbl_mac_a);
		flash_write_page(FLASH_ADR_THREE_PARA_ADR+idx, sizeof(con_sec_data_a), p[2]);

		sleep_ms(400);
		return 0;
		
	}
	return 1;*/
}

static unsigned char atCmd_test(char *pbuf,  int mode, int lenth)
{
	
	if(is_provision_success()!=1)
	{
		at_print("unpar\r\n");
		return 2;
	}
	char *p[CMD_PA_MAX]={0};

	pbuf[lenth]=0x0D;

	u8 cnt=get_atcmd_param(pbuf,p,CMD_PA_MAX);

	if(cnt!=3)
	{
		at_print("para cnt error\r\n");
		return 2;
	}
	u8 op_len=strlen(p[1]);
	u16 op=0;
	/*if(str2hex(p[1], strlen(p[1])) == -1 )
	{
		at_print("p[1],error");
	}*/

	if(op_len/2==2)//op is 2 byte
	{
		op=HEX2U16(p[1]);
	}else if(op_len/2==3)// op is 3 byte
	{
		if(str2hex(p[1], strlen(p[1])) == -1 )
		{
			at_print("p[1],error");
		}
		op=p[1][0];
	}
	
	u8 str_len=strlen(p[2]);
	if(str2hex(p[2], strlen(p[2])) == -1 )
	{
		at_print("p[2],error");
	}
	
	
	u16 addr=HEX2U16(p[0]);
	//at_print_array(p[0],2);
	//at_print_array(p[1],3);
	//at_print_array(p[2],4);
	//bat_printf("\r\np0:%02x,p1:%02x%02x%02x,p2:%02x",p[0][0],p[1][0],p[1][1],p[1][2],p[2][0]);
	mesh_tx_cmd2normal_primary(op,p[2],str_len/2,addr,1);
	return 0;
}


static unsigned char atCmd_addr(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_SET)
	{
		at_print("not suppor set\r\n");
		return -2;
	}
	extern pro_para_mag provision_mag;
	printf("addr:%02x \r\n",provision_mag.pro_net_info.unicast_address);
	return 0;
}
u8 enable_gatt_mode=0;
static unsigned char atCmd_setup(char *pbuf,  int mode, int lenth)
{
	if(is_provision_success())
	{
		printf("already provision\r\n");
		return 1;
	}else
		provision_mag.gatt_mode = GATT_PROVISION_MODE;
	return 0;
}

static unsigned char atCmd_mac(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_READ)
	{
		extern u8 tbl_mac[6];
		printf("mac:%02x%02x%02x%02x%02x%02x\r\n",tbl_mac[5],tbl_mac[4],tbl_mac[3],tbl_mac[2],tbl_mac[1],tbl_mac[0]);
		return 0;
	}

	if(mode == AT_CMD_MODE_SET)
	{
		at_print("not suppor set\r\n");
		return -2;
	}
}

static unsigned char atCmd_appkey(char *pbuf,  int mode, int lenth)
{
	extern mesh_key_t mesh_key;
	
	if(mode == AT_CMD_MODE_READ)
	{
		printf("appkey:");
		at_print_array(mesh_key.net_key[0][0].app_key[0].key, 16);
		//printf("app:%02x%02x%02x%02x%02x%02x\r\n",mesh_key.net_key.app_key.key);
		return 0;
	}

	if(mode == AT_CMD_MODE_SET)
	{
		at_print("not suppor set\r\n");
		return -2;
	}
}

static unsigned char atCmd_nwkey(char *pbuf,  int mode, int lenth)
{
	if(mode == AT_CMD_MODE_READ)
	{
		printf("nwkey:");
		at_print_array(mesh_key.net_key[0][0].key, 16);
		return 0;
	}

	if(mode == AT_CMD_MODE_SET)
	{
		at_print("not suppor set\r\n");
		return -2;
	}
}

_at_command_t gAtCmdTb_writeRead[] =
{ 
//	{ "SEND2ALI", 	atCmd_Send2Ali,		"Send data to Tmall \r\n"},	
	{ "SEND2APP", 	atCmd_Send2App,	"Send data to phone\r\n"},
	{ "BAUD", 	atCmd_Baud,	"Set/Read BT Baud\r\n"},
	{ "AliGenie", atCmd_AliGenie, "Set/Read three info\r\n"},
	{ "TEST", atCmd_test, "send data \r\n"},
	{ "MAC", atCmd_mac, "send data \r\n"},
	{ "APPKEY", atCmd_appkey, "read data \r\n"},
	{ "NWKEY", atCmd_nwkey, "read data \r\n"},
	{ "ADDR", atCmd_addr, "get addr\r\n"},
	{0, 	0,	0}
};

_at_command_t gAtCmdTb_exe[] =
{
//	{ "1", 		atCmd_ATE1, "ATE1\r\n"},  //ATE1
//	{ "0", 		atCmd_ATE0, "ATE0\r\n"},  //ATE0
	{ "GMR", 	atCmd_GMR,  "Get Version\r\n"}, 
	{ "RST", 	atCmd_Reset, "RESET\r\n"}, 
	{ "RESTORE",atCmd_Restore, "Restore Factory\r\n"}, 
	{ "STATE", 	atCmd_State, "Prove State\r\n"}, 
	{ "SETUP", atCmd_setup, "get addr\r\n"},
	{0, 	0,	0}
};
