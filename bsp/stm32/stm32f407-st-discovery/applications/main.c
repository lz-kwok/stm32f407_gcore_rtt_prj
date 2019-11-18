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

#define MajorVer            1
#define MiddleVer           0
#define MinnorVer           0

uint8_t sendBuf[10] = {0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D};

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
    dev = rt_device_find("vcom");
   
    if (dev)
        rt_device_open(dev, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
    else
        return 0;
   
    while (RT_TRUE)
    {   
        recvLen = rt_device_read(dev,0,dataRecv,32);
        if(recvLen > 0){
            if((dataRecv[0] == 0x0D)&&(dataRecv[recvLen - 1] == 0x0D)){
                switch(dataRecv[1])
                {
                case 0xFE:       //查询版本
                    sendBuf[1] = dataRecv[1];
                    sendBuf[2] = MajorVer;
                    sendBuf[3] = MiddleVer;
                    sendBuf[4] = MinnorVer;
                    rt_device_write(dev, 0, sendBuf, 10);
                break;
                }
            }
        }
        
        rt_thread_mdelay(10);
    }

    return RT_EOK;
}



