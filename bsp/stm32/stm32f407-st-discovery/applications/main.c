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
#include <gui.h>
#include <at.h>
#include <string.h>

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)


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
    int recvLen = 0;
    uint8_t dataRecv[32];
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);

    rt_device_t dev = RT_NULL;
    char buf[] = "hello zhizhuo\r\n";

    dev = rt_device_find("vcom");
   
    if (dev)
        rt_device_open(dev, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
    else
        return 0;
   
    while (RT_TRUE)
    {   
        recvLen = rt_device_read(dev,0,dataRecv,32);
        if(recvLen > 0){
            rt_device_write(dev, 0, &dataRecv, recvLen);
        }

        rt_device_write(dev, 0, buf, strlen(buf));
        
        rt_thread_mdelay(10);
    }

    return RT_EOK;
}



