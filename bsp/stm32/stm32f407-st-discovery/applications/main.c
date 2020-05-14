/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 * 2019-09-12     Guolz     v1.0.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <at.h>
#include <string.h>
#include <stdarg.h>
#include  <math.h>
#include "RTL.h"
#include "drv_nand.h"
#include "drv_tf_card.h"
#include "File_Config.h"

#define client

static rt_uint8_t defaultDisk = Nand_Flash;
static void Mount_Disk(rt_uint8_t disk_name);
static rt_uint16_t tcp_callback (rt_uint8_t soc, rt_uint8_t evt, rt_uint8_t *ptr, rt_uint16_t par);

static rt_uint8_t socket_tcp;
static rt_uint8_t send_flag = 0;

#define LocalPort_NUM       1001
#define IP1                 192
#define IP2                 168
#define IP3                 0
#define IP4                 10
#define RemotePort_NUM      5689

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)

const char * ReVal_Table[]= 
{
	" 0: SCK_SUCCESS       Success                             ",
	"-1: SCK_ERROR         General Error                       ",
	"-2: SCK_EINVALID      Invalid socket descriptor           ",
	"-3: SCK_EINVALIDPARA  Invalid parameter                   ",
	"-4: SCK_EWOULDBLOCK   It would have blocked.              ",
	"-5: SCK_EMEMNOTAVAIL  Not enough memory in memory pool    ",
	"-6: SCK_ECLOSED       Connection is closed or aborted     ",
	"-7: SCK_ELOCKED       Socket is locked in RTX environment ",
	"-8: SCK_ETIMEOUT      Socket, Host Resolver timeout       ",
	"-9: SCK_EINPROGRESS   Host Name resolving in progress     ",
	"-10: SCK_ENONAME      Host Name not existing              ",
};


void phy_reset(void)
{
    rt_pin_mode(phy_rst_pin,PIN_MODE_OUTPUT);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_pin_write(phy_rst_pin, PIN_LOW);
    rt_thread_mdelay(500);
    rt_kprintf("%s done\r\n",__func__);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
}




int main(void)
{
    char dbuf[10];
    rt_uint8_t *sendbuf;
    int res;
    rt_uint8_t Rem_IP[4] = {IP1,IP2,IP3,IP4};

    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    Mount_Disk(tf_Card);

    init_TcpNet ();


    rt_thread_mdelay(10000);        //等协议栈初始化完

#ifdef server
    socket_tcp = tcp_get_socket (TCP_TYPE_SERVER|TCP_TYPE_KEEP_ALIVE, 0, 10, tcp_callback);
	if(socket_tcp != 0)
	{
		res = tcp_listen (socket_tcp, LocalPort_NUM);
		rt_kprintf("tcp listen res = %d\r\n", res);
	}
#endif

#ifdef client
        socket_tcp = tcp_get_socket (TCP_TYPE_CLIENT | TCP_TYPE_KEEP_ALIVE, 0, 10, tcp_callback);
        if(socket_tcp != 0)
        {
            res = tcp_connect (socket_tcp, Rem_IP, RemotePort_NUM, LocalPort_NUM);
            rt_kprintf("TCP Socket creat done res = %d\r\n", res);
        }
#endif
    while (RT_TRUE)
    {   
        rt_thread_mdelay(1000);
        if(send_flag){
            sendbuf = tcp_get_buf(11);
            sendbuf[0] = 'h';
            sendbuf[1] = 'e';
            sendbuf[2] = 'l';
            sendbuf[3] = 'l';
            sendbuf[4] = 'o';
            sendbuf[5] = ' ';
            sendbuf[6] = 'w';
            sendbuf[7] = 'o';
            sendbuf[8] = 'r';
            sendbuf[9] = 'l';
            sendbuf[10] = 'd';
            tcp_send (socket_tcp, sendbuf, 11);
        }else{
			tcp_abort (socket_tcp);
			tcp_close (socket_tcp);
			tcp_release_socket (socket_tcp);
			rt_thread_mdelay(1000);
			socket_tcp = tcp_get_socket (TCP_TYPE_CLIENT | TCP_TYPE_KEEP_ALIVE, 0, 10, tcp_callback);
			if(socket_tcp != 0)
			{
				res = tcp_connect (socket_tcp, Rem_IP, RemotePort_NUM, LocalPort_NUM);
				rt_kprintf("TCP Socket creat done res = %d\r\n", res);
			}
			rt_thread_mdelay(1000);
		}


    }	

    return RT_EOK;
}


static void Mount_Disk(rt_uint8_t disk_name)
{
    defaultDisk = disk_name;
    if(disk_name == tf_Card){
        funinit("N0:");
        finit("M0:");
    }else{
        funinit("M0:");
        finit("N0:");
    }
}

rt_uint8_t Get_MountDisk(void)
{
    return defaultDisk;
}


static rt_uint16_t tcp_callback (rt_uint8_t soc, rt_uint8_t evt, rt_uint8_t *ptr, rt_uint16_t par)
{
	char buf[50];
	rt_uint16_t i;
	
	/* 确保是socket_tcp的回调 */
	if (soc != socket_tcp) 
	{
		return (0);
	}

	switch (evt) 
	{
		/*
			远程客户端连接消息
		    1、数组ptr存储远程设备的IP地址，par中存储端口号。
		    2、返回数值1允许连接，返回数值0禁止连接。
		*/
		case TCP_EVT_CONREQ:
			sprintf(buf, "[remote client][IP: %d.%d.%d.%d] connectting ... ", ptr[0], ptr[1], ptr[2], ptr[3]);
			rt_kprintf("IP:%s  port:%d\r\n", buf, par);
			return (1);
		
		/* 连接终止 */
		case TCP_EVT_ABORT:
			break;
		
		/* Socket远程连接已经建立 */
		case TCP_EVT_CONNECT:
			rt_kprintf("Socket is connected to remote peer\r\n");
            send_flag = 1;
			break;
		
		/* 连接断开 */
		case TCP_EVT_CLOSE:
		   	rt_kprintf("Connection has been closed\r\n");
            send_flag = 0;
			break;
		
		/* 发送的数据收到远程设备应答 */
		case TCP_EVT_ACK:
			break;
		
		/* 接收到TCP数据帧，ptr指向数据地址，par记录数据长度，单位字节 */
		case TCP_EVT_DATA:
			rt_kprintf("Data length = %d\r\n", par);
            rt_kprintf("[hex]:");
			for(i = 0; i < par; i++)
			{
				rt_kprintf("0x%02x ", ptr[i]);
			}
            rt_kprintf("\r\n");


			break;
	}
	
	return (0);
}