/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-09-12     Guolz     v1.0.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <gui.h>
#include <at.h>
#include <string.h>

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)

const char *g_IoT_HOST = "47.111.88.91:6096"; 
const char *g_IoT_PATH = "/iot/data/receive";
const char *zhizhuo = "{ \"appkey\":	\"tian jin B2 line\", \"devicename\":	\"hello zhizhuo\", \"devicekey\":	2, \"header\":	{ \"namespace\":	\"jd.data.origin\", \"version\":	\"1.0\" }, \"payload\":	{ }, \"uptime\":	\"2019-10-05 18:38:14\" }";


static int g_gprs_module_CheckString(char *dst ,char *src)
{
    if(strstr((const char *)dst,(const char *)src) != NULL)
        return RT_EOK;
	else
	    return RT_ERROR;
}

int g_gprs_module_init()
{
    int csq1 = 0,csq2;
    int cgatt = 0;
    int http_cid = 0,connect_status;
    char ip_get[32];

    rt_pin_write(gprs_rst, PIN_HIGH);
    rt_thread_mdelay(1000);
    rt_pin_mode(gprs_power,PIN_MODE_OUTPUT);
    rt_pin_write(gprs_power, PIN_HIGH);
    rt_thread_mdelay(2000);
    rt_pin_write(gprs_power, PIN_LOW);
    rt_kprintf("AIR202 Power on !\r\n");
    at_client_init("uart3",256);
    rt_thread_mdelay(5000);
    at_response_t resp = at_create_resp(64, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        rt_kprintf("No memory for response structure!");
        return -RT_ENOMEM;
    }
    if (at_exec_cmd(resp, "ATE0") != RT_EOK){
        rt_kprintf("AT client send commands failed, response error or timeout !");
    }
    rt_thread_mdelay(1000);
    while(csq1 == 0){
        if (at_exec_cmd(resp, "AT+CSQ") != RT_EOK){
            rt_kprintf("AT client send commands failed, response error or timeout !");
        }
        at_resp_parse_line_args(resp, 2,"%*[^:]:%d", &csq1, &csq2);
        rt_thread_mdelay(200);
    }

    while(cgatt == 0){
        if (at_exec_cmd(resp, "AT+CGATT?") != RT_EOK){
            rt_kprintf("AT client send commands failed, response error or timeout !");
        }
        at_resp_parse_line_args(resp, 2,"%*[^:]:%d", &cgatt);
        rt_thread_mdelay(200);
    }

    if((csq1 > 0)&&(cgatt == 1)){
        while(http_cid == 0){
            if (at_exec_cmd(resp, "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            rt_thread_mdelay(200);
            if (at_exec_cmd(resp, "AT+SAPBR=3,1,\"APN\",\"CMNET\"") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            rt_thread_mdelay(200);
            if (at_exec_cmd(resp, "AT+SAPBR=5,1") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            rt_thread_mdelay(200);
            if (at_exec_cmd(resp, "AT+SAPBR =1,1") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            rt_thread_mdelay(200);
            if (at_exec_cmd(resp, "AT+SAPBR=2,1") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            at_resp_parse_line_args(resp, 2,"%*[^:]:%d,%d,%s", &http_cid,&connect_status,ip_get);
            rt_kprintf("http_cid = %d\r\n",http_cid);
            rt_kprintf("ip_get = %s\r\n",ip_get);
            rt_thread_mdelay(200);
        }
        
        if(http_cid == 1){
            if (at_exec_cmd(resp, "AT+HTTPINIT") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }
            rt_thread_mdelay(200);
            if (at_exec_cmd(resp, "AT+HTTPPARA=\"CID\",%d",http_cid) != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }  
            rt_thread_mdelay(200);

            at_delete_resp(resp);
           
            return RT_EOK;
        } 
    };

    return RT_ERROR;
}
INIT_COMPONENT_EXPORT(g_gprs_module_init);	

// g_gprs_module_http_post(g_IoT_HOST,g_IoT_PATH,RT_NULL,zhizhuo,RT_NULL,5000);
int g_gprs_module_http_post(const char *host,const char* path,const char *apikey,const char *data,
                      char *response,int timeout)
{
	uint32_t datalen = 0;
	int16_t g_err = 0;
	static int gprs_tick;
    char gprs_ack[16];
    int method,status_code,data_response;
    memset(gprs_ack,0x0,16);

	while(g_err == 0){
		if(gprs_tick == 0){      //设置Http会话参数
            at_response_t resp = at_create_resp(32, 0, rt_tick_from_millisecond(timeout));
            if (!resp){
                rt_kprintf("No memory for response structure!");
                return -RT_ENOMEM;
            }
            if (at_exec_cmd(resp, "AT+HTTPPARA=\"URL\",\"%s%s\"",host,path) != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            } 
            at_delete_resp(resp);
			rt_thread_mdelay(200);
            gprs_tick = 1;
		}else if(gprs_tick == 1){      //
            at_response_t resp = at_create_resp(256, 0, rt_tick_from_millisecond(timeout));
            if (!resp){
                rt_kprintf("No memory for response structure!");
                return -RT_ENOMEM;
            }
            datalen = strlen(data);
            if (at_exec_cmd(resp, "AT+HTTPDATA=%d,100000",datalen) != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            } 
			at_resp_parse_line_args(resp, 2,"%s", gprs_ack);
            rt_kprintf("download_ack [%s]\r\n",gprs_ack);
            if(g_gprs_module_CheckString(gprs_ack,"DOWNLOAD") == RT_EOK){
                if (at_exec_cmd(resp, "%s",data) != RT_EOK){
                    rt_kprintf("AT client send commands failed, response error or timeout !");
                } 
                rt_thread_mdelay(200);
                gprs_tick = 2;
            }else{
                rt_thread_mdelay(1000);  
            }
            at_delete_resp(resp);
		}else if(gprs_tick == 2){      //
            at_response_t resp = at_create_resp(256, 4, rt_tick_from_millisecond(timeout));
            if (!resp){
                rt_kprintf("No memory for response structure!");
                return -RT_ENOMEM;
            }
            if (at_exec_cmd(resp, "AT+HTTPACTION=1") != RT_EOK){
                rt_kprintf("AT client send commands failed, response error or timeout !");
            }

            // rt_thread_mdelay(2000); 
            at_resp_parse_line_args(resp, 4,"%*[^:]:%d,%d,%d", &method,&status_code,&data_response);
            rt_kprintf("status_code [%d]\r\n",status_code);
            rt_kprintf("response_len [%d]\r\n",data_response);
            g_err = status_code;
            if((status_code == 200)&&(data_response > 0)){  //
                char da[256];
                memset(da,0x0,256);
                at_delete_resp(resp);
                at_response_t resp = at_create_resp(256, 0, rt_tick_from_millisecond(timeout));
                if (at_exec_cmd(resp, "AT+HTTPREAD") != RT_EOK){
                    rt_kprintf("AT client send commands failed, response error or timeout !");
                } 
                at_resp_parse_line_args(resp, 3,"%s",da);
                rt_kprintf("response:%s\r\n",da);
                rt_thread_mdelay(500); 
                if (at_exec_cmd(resp, "AT+HTTPTERM") != RT_EOK){
                    rt_kprintf("AT client send commands failed, response error or timeout !");
                } 
                at_delete_resp(resp);
            }else{
                if (at_exec_cmd(resp, "AT+HTTPTERM") != RT_EOK){
                    rt_kprintf("AT client send commands failed, response error or timeout !");
                } 
                at_delete_resp(resp);
            }

            gprs_tick = 3;
            return g_err;
		}
    }

	return g_err;
}




