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


int main(void)
{
    rt_pin_write(usbfs, PIN_HIGH);
    g_Gui_init();

    rt_device_t dev = RT_NULL;
    char buf[] = "hello Nanjingzhizhuo\r\n";

    dev = rt_device_find("vcom");
    
    if (dev)
        rt_device_open(dev, RT_DEVICE_FLAG_RDWR);
    else
        return -RT_ERROR;

    while (RT_TRUE)
    {
        rt_thread_mdelay(2000);
        rt_device_write(dev, 0, buf, rt_strlen(buf));
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
