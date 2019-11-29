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
#include <string.h>

#include <g_usb_cdc.h>
#include <g_uart.h>
#include <g_measure.h>

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)

#define ADC_DEV_NAME        "adc3"  


rt_adc_device_t adc_dev; 
rt_uint32_t adc_val[10];


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
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    g_measure_manager_init();
    g_usb_cdc_init();
    g_uart_init();
    rt_thread_mdelay(1000);

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    rt_adc_enable(adc_dev, 4);
    
    while (RT_TRUE)
    {   
        adc_val[0] = rt_adc_read(adc_dev, 5);
        adc_val[1] = rt_adc_read(adc_dev, 6);
        adc_val[2] = rt_adc_read(adc_dev, 7);
        adc_val[3] = rt_adc_read(adc_dev, 8);

        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}



