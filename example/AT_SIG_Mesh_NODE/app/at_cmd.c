
#include "tl_common.h"
#include "drivers.h"

#include "proj/common/tstring.h"

#include "at_cmd.h"

static char *at_result_string[AT_RESULT_CODE_MAX] = 
{
    "\r\nOK\r\n",         //AT_RESULT_CODE_OK         = 0x00,
    "\r\nERROR(-1)\r\n",  //AT_RESULT_CODE_ERROR      = 0x01,
    "\r\nERROR(-2)\r\n",
};

void at_response_result(unsigned char result_code)
{
    if (result_code < AT_RESULT_CODE_MAX) 
    {
        at_print(at_result_string[result_code]);
    }
}

// 数据处理命令模式
static int data_process_cmd_mode(char *pbuf)
{
    char *s = NULL;

    s = strstr(pbuf, PREFIX_AT_CMD_HELP);
    if(s != NULL)
    {
        return AT_CMD_MODE_HELP;
    }

    s = strstr(pbuf, PREFIX_ATCMD_SET);
    if(s != NULL)
    {
        return AT_CMD_MODE_SET;
    }

    s = strstr(pbuf, PREFIX_AT_CMD_READ);
    if(s != NULL)
    {
        return AT_CMD_MODE_READ;
    }

    return AT_CMD_MODE_EXECUTION;
}

static unsigned char data_process_parse(char *pbuf,  int mode, int len)
{
	char *ps = NULL;
	unsigned char result = AT_RESULT_CODE_ERROR;
    const _at_command_t *cmd_ptr = NULL;
	unsigned char dataLen = 0;
	
	
	if(mode == AT_CMD_MODE_EXECUTION)
		cmd_ptr= gAtCmdTb_exe;
	else
		cmd_ptr= gAtCmdTb_writeRead;

    for(; cmd_ptr->cmd; cmd_ptr++)
    {
        if(strxcmp(cmd_ptr->cmd, pbuf)) continue;
			
		if(mode == AT_CMD_MODE_SET )
		{	
			ps = strstr(pbuf, "=") + 1;
			dataLen = len - (int)(ps - pbuf);
		}
				
        result = cmd_ptr->cmd_handle(ps, mode, dataLen);
				
		break;
    }

	return result;
}


//AT命令入口
void at_data_process(char *pbuf, int len)
{
    if (pbuf == NULL || len == 0) return;

    if((pbuf[len - 1] == 0x0A) || (pbuf[len - 1] == 0x0D))
    {
        if((pbuf[len - 2] == 0x0A) || (pbuf[len - 2] == 0x0D))
        {
            pbuf[len - 2] = 0;
            len -=2;
        }
        else
        {
            pbuf[len - 1] = 0;
            len -= 1;
        }
    }
    else
    {
        return;
    }
    
    int mode = AT_CMD_MODE_INVALID;

    mode = data_process_cmd_mode(pbuf);

    // 我加的
    // char temp[20] = {0};
    // u_sprintf(temp, "mode=%d\r\n", mode);
    // at_print(temp);

    //at_print(pbuf);

    // 不存在AT或ATE
    if((strxcmp("AT",pbuf) != 0) && (strxcmp("ATE",pbuf) != 0))
    {
        at_response_result(1);
        at_print("don't exist AT or ATE\r\n");
        return;
    }

    if(len == 2)
    {
		at_response_result(AT_RESULT_CODE_OK);
    	return;
    }

    at_response_result(data_process_parse(pbuf + 3, mode, len-3));
    return;
}
