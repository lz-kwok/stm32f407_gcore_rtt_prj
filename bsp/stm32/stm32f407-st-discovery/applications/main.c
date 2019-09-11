/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <gui.h>






int main(void)
{
    g_Gui_init();

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
