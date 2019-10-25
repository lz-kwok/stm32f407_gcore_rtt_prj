#ifndef __DRV_GPRS_H__
#define __DRV_GPRS_H__
#include <board.h>
#include <rtdevice.h>
#include <rthw.h>
#include <rtthread.h>


#ifdef __cplusplus
 extern "C" {
#endif

int g_gprs_module_http_post(const char *host,const char* path,const char *apikey,const char *data,
                      char *response,int timeout);

#ifdef __cplusplus
 }
#endif
#endif /*__DRV_CAN_H__ */
