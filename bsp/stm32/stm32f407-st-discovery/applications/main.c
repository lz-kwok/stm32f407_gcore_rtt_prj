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



#define usbfs     GET_PIN(C, 9)

rt_uint8_t data[128];
rt_uint8_t wdata[64] = {1,2,3,4,5,6,7,8,9,10,12,13,14};


int main(void)
{
    int i ;
    g_Gui_init();
    rt_hw_mtd_nand_init();

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
