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
    // rt_thread_mdelay(5000);
    // rt_pin_mode(phy_rst_pin,PIN_MODE_INPUT_PULLDOWN);
    // rt_pin_write(phy_rst_pin, PIN_LOW);
    // rt_thread_mdelay(500);
    // rt_pin_mode(phy_rst_pin,PIN_MODE_OUTPUT_OD);
    // rt_pin_write(phy_rst_pin, PIN_HIGH);
    // phy_reset();
    while (RT_TRUE)
    {   
        rt_thread_mdelay(2000);
        // g_Gui_show_pic("5");
        // rt_thread_mdelay(2000);
        // g_Gui_show_pic("F");
    }

    return RT_EOK;
}



