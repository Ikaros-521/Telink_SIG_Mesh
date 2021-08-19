#include "proj/tl_common.h"
#ifndef WIN32
#include "proj/mcu/watchdog_i.h"
#endif 
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/blt_config.h"
#include "mesh/user_config.h"
#include "mesh/app_health.h"
#include "mesh/app_heartbeat.h"
#include "proj_lib/sig_mesh/app_mesh.h"
#include "mesh/remote_prov.h"
#include "mesh/lighting_model.h"
#include "mesh/lighting_model_HSL.h"
#include "mesh/lighting_model_xyL.h"
#include "mesh/lighting_model_LC.h"
#include "mesh/scene.h"
#include "mesh/time_model.h"
#include "mesh/sensors_model.h"
#include "mesh/scheduler.h"
#include "mesh/mesh_ota.h"
#include "mesh/mesh_property.h"
#include "mesh/generic_model.h"

// 我加的
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "mesh/app_provison.h"
#include "mesh/app_proxy.h"
#include "mesh/app_beacon.h"

#define DATA_JSON "\r\n{\"mesh_data\":\r\n{\"daddr\":%02X%02X,\"saddr\":%02X%02X,\"opcode\":%02X%02X,\"data_len\":%d,\"data\":"
//接收到Mesh数据
int mesh_cmd_at_data(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
    char buf[200] = {0};

    if((cb_par->adr_src == ele_adr_primary) && (cb_par->op != 0x0282)) //如果原地址是自�?且不是开关操作，直接返回
    {
    	at_print("b_par->adr_src == ele_adr_primary");
        return 0;
    }
//	u_sprintf(buf, "\r\n+DATA:%02X%02X,%d,",(cb_par->op)&0xff,(cb_par->op)>>8,par_len);
 //   at_print(buf);
 //   at_print_hexstr((char*)par,par_len);
 //   at_print("\r\n");
	
	
	memset(buf,0,200);
	u_sprintf(buf,DATA_JSON,(cb_par->adr_dst)>>8,(cb_par->adr_dst)&0x00ff,(cb_par->adr_src)>>8,(cb_par->adr_src)&0x00ff,(cb_par->op)&0xff,(cb_par->op)>>8,par_len);
    at_print(buf);
    at_print_hexstr((char*)par,par_len);
    at_print("}}\r\n");

    if(par[0] == 0)
    {
        gpio_write(GPIO_LED, 0);
		//gpio_write(GPIO_PB6, 0);
		at_print("gpio_write(GPIO_LED, 0)\r\n");
    }
    else if(par[0] == 1)
    {
        gpio_write(GPIO_LED, 1);
		//gpio_write(GPIO_PB6, 1);
		at_print("gpio_write(GPIO_LED, 1)\r\n");
    }
    else
    {

    }

    return 0;
}


int node_del_key_info(u8 *par, int par_len, mesh_cb_fun_par_t *cb_par)
{
	 char buf[200] = {0};
	memset(buf,0,200);
	u_sprintf(buf,DATA_JSON,(cb_par->adr_dst)>>8,(cb_par->adr_dst)&0x00ff,(cb_par->adr_src)>>8,(cb_par->adr_src)&0x00ff,(cb_par->op)&0xff,(cb_par->op)>>8,par_len);
    at_print(buf);
	irq_disable();
	factory_reset();
	//at_print("\r\nOK\r\n");
	start_reboot();
}