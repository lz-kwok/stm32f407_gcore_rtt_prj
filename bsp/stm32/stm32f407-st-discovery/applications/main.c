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




rt_uint8_t data[128];
rt_uint8_t wdata[64] = {1,2,3,4,5,6,7,8,9,10,12,13,14};


int main(void)
{
    int i ;

    while (RT_TRUE)
    {   
        rt_thread_mdelay(2000);
        g_Gui_show_pic("1");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("2");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("3");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("4");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("5");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("A");
        rt_thread_mdelay(2000);
        g_Gui_show_pic("F");
    }

    return RT_EOK;
}



 void phy_reset(void)
 {
     int i,j=0;
     rt_pin_write(phy_rst_pin, PIN_HIGH);
     for(i=0;i<=1000;i++){
         for(j=0;j<=1000;j++);
     }
     rt_pin_write(phy_rst_pin, PIN_LOW);
     for(i=0;i<=1000;i++){
         for(j=0;j<=1000;j++);
     }
     rt_pin_write(phy_rst_pin, PIN_HIGH);
 }