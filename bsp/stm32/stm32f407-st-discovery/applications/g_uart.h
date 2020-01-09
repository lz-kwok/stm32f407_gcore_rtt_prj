/*
 * File      : g_uart.c
 * Change Logs:
 * Date           Author       Notes
 *2019-11-22      GLZ          the first version 
 */


#ifndef __G_UART_H__
#define __G_UART_H__

#include <rthw.h>
#include <rtthread.h>

void g_uart_sendto_Dpsp(const rt_uint8_t *cmd);

rt_err_t g_uart_init(void);
rt_device_t uart_open(const char *name);
rt_uint8_t g_Client_data_receive(rt_uint8_t *buf,rt_uint8_t len);
rt_uint8_t g_ErrorCode_data_receive(rt_uint8_t *buf,rt_uint8_t len);
void g_Client_data_send(rt_uint8_t *buf,rt_uint8_t len);
void gScan_Error_Code(void);

#endif
