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

#define LocalPort_NUM       8080
#define IP1                 192
#define IP2                 168
#define IP3                 0
#define IP4                 10

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

rt_uint8_t sendbuf[1024];


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
    // char dbuf[10];
	// int len;
	// int sock, sd, res;
	// SOCKADDR_IN addr;
	// SOCKADDR_IN ReAddr;

    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    finit("N0:");
    init_TcpNet ();
    // rt_thread_mdelay(10000);        //等协议栈初始化完

    // unsigned long sck_mode;
    
    while (RT_TRUE)
    {   
        // /* 创建一个socket 
        // 第1个参数AF_INET：当前仅支持这个类型的地址族。
        // 第2个参数SOCK_STREAM：表示数据流通信类型，即使用的TCP。
        // 第3个参数0 ：配置为0的话，自动跟第2个参数进行协议匹配，这里就是TCP协议。
        // */
        // sock = socket (AF_INET, SOCK_STREAM, 0);

        // /* 端口号设置为1001 */
        // addr.sin_port        = htons(LocalPort_NUM);

        // /* 与函数socket中的AF_INET作用一样 */
        // addr.sin_family      = PF_INET;
        // /* 
        //     INADDR_ANY就是指定地址为0.0.0.0的地址，这个地址事实上表示不确定地址，或所有地址，
        //     任意地址。用在这里的话就表示监控端口号为ddr.sin_port的所有IP地址消息。一般主要用
        //     于有多个网卡或者IP地址的情况。
        // */
        // addr.sin_addr.s_addr = INADDR_ANY;

        // /* 给socket绑定IP和端口号 */
        // bind (sock, (SOCKADDR *)&addr, sizeof(addr));

        // /* 设置监听，最大监听1个连接 */
        // listen (sock, 2);

        // /* 
        //     等待soket连接请求，有的话，自动创建1个新的socket进行连接通信，没有的话，等待连接。
        //     注意，能够accept的个数受到listen函数的限制，而listen函数又受到Net_Config.c中宏定义
        //     BSD_NUMSOCKS 的限制。
        // */
        // len = sizeof(ReAddr);
        // sd = accept (sock, (SOCKADDR *)&ReAddr, &len);
        // rt_kprintf ("远程客户端请求连接IP: %d.%d.%d.%d\n", ReAddr.sin_addr.s_b1,
        //                                                         ReAddr.sin_addr.s_b2,
        //                                                         ReAddr.sin_addr.s_b3,
        //                                                         ReAddr.sin_addr.s_b4);
        // rt_kprintf ("远程客户端端口号: %d\n", ntohs(ReAddr.sin_port));

        // /* 关闭监听socket，这个监听socket是调用函数socket后自动创建的 */
        // closesocket (sock);
        // sock = sd;           

        /* 创建一个socket 
		   第1个参数AF_INET：当前仅支持这个类型的地址族。
		   第2个参数SOCK_STREAM：表示数据流通信类型，即使用的TCP。
		   第3个参数0 ：配置为0的话，自动跟第2个参数进行协议匹配，这里就是TCP协议。
		*/
		// sock = socket (AF_INET, SOCK_STREAM, 0);
		
		// /* 设置使能KEEP ALIVE，让客户端和服务器保存连接 */
		// sck_mode = 1;
		// res = ioctlsocket (sock, FIO_KEEP_ALIVE, &sck_mode);
		// if (res == SCK_SUCCESS) 
		// {
		// 	rt_kprintf("KEEP ALIVE set done\r\n");
		// }
		// else 
		// {
		// 	rt_kprintf("KEEP ALIVE set fail\r\n");
		// }

		// /* 端口号设置为1001 */
		// addr.sin_port = htons(LocalPort_NUM);
		
		// /* 与函数socket中的AF_INET作用一样 */
		// addr.sin_family = PF_INET;
		
		// addr.sin_addr.s_b1 = IP1;
		// addr.sin_addr.s_b2 = IP2;
		// addr.sin_addr.s_b3 = IP3;
		// addr.sin_addr.s_b4 = IP4;

		// /* 客户端连接远程服务器，如果远程服务器还未创建，此函数会立即返回 */
		// res = connect (sock, (SOCKADDR *)&addr, sizeof (addr));
		// rt_kprintf("客户端连接远程服务器状态%s\r\n", ReVal_Table[abs(res)]);



        rt_thread_mdelay(5000);

        // for(;;){
            /*
            socket数据接收函数，如果recv工作在阻塞模式，使用这个函数注意以下事项：
            1. 此函数的溢出时间受到Net_Config.c中宏定义 BSD_RCVTOUT 的限制。溢出时间到会自动退出。
            2. 这个函数接收到一次数据包就会返回，大于或者小于设置的缓冲区大小都没有关系，如果数据量
                大于接收缓冲区大小，用户只需多次调用函数recv进行接收即可。
            3. 实际接收到数据大小通过判断此函数的返回值即可。
            */
            rt_thread_mdelay(50);
        //     res = recv (sock, dbuf, sizeof(dbuf), 0);
        //     if (res <= 0) 
        //     {
        //         // // closesocket (sock);
        //         // rt_kprintf("退出接收函数，重新开始监听%s\r\n", ReVal_Table[abs(res)]);
        //         // break;
        //     }
        //     else
        //     {
        //         rt_kprintf("Receive Data Length = %d\r\n", res);
        //         switch(dbuf[0])
        //         {
        //             /* 字符命令 1 */
        //             case '1':
        //                 sendbuf[0] = '1';
        //                 sendbuf[1] = '2';
        //                 sendbuf[2] = '3';
        //                 sendbuf[3] = '4';
        //                 sendbuf[4] = '5';
        //                 sendbuf[5] = '6';
        //                 sendbuf[6] = '7';
        //                 sendbuf[7] = '8';
        //                 sendbuf[8] = '\r';
        //                 sendbuf[9] = '\n';						
        //                 res = send (sock, (char *)sendbuf, 10, 0);
        //                 if (res < 0) 
        //                 {
        //                     rt_kprintf("函数send发送数据失败\r\n");
        //                 }
        //                 else
        //                 {
        //                     rt_kprintf("函数send发送数据成功\r\n");							
        //                 }
        //                 break;
                    
        //             /* 字符命令 2 */
        //             case '2':
        //                 /* 将数据缓冲区清成字符0，方便网络调试助手查看数据 */
        //                 len = sizeof(sendbuf);
        //                 memset(sendbuf, 48, len);
                    
        //                 /* 这里仅初始化了数据包的前4个字节和最后4个字节 */
        //                 sendbuf[0] = 'a';
        //                 sendbuf[1] = 'b';
        //                 sendbuf[2] = 'c';
        //                 sendbuf[3] = 'd';
        //                 sendbuf[len - 4] = 'e';
        //                 sendbuf[len - 3] = 'f';
        //                 sendbuf[len - 2] = 'g';
        //                 sendbuf[len - 1] = 'h';					
        //                 res = send (sock, (char *)sendbuf, len, 0);
        //                 if (res < 0) 
        //                 {
        //                     rt_kprintf("函数send发送数据失败%s\r\n", ReVal_Table[abs(res)]);
        //                 }
        //                 else
        //                 {
        //                     rt_kprintf("函数send成功发送数据 = %d字节\r\n", res);							
        //                 }
        //                 break;
                
        //             /* 其它数值不做处理 */
        //             default:                     
        //                 break;
        //         }
        //     }
        // }
		
    
    }

    /* 
        溢出时间到，远程设备断开连接等，程序都会执行到这里，我们在这里关闭socket，
        程序返回到第一个大while循环的开头重新创建socket并监听。
    */
    // closesocket (sock);

    return RT_EOK;
}



