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

typedef enum  {
  channel5         = 1,
  channel6,
  channel7,
  channel8
} adc_channel;

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)

#define ADC_DEV_NAME        "adc3"  
#define FILTER_N            20

rt_adc_device_t adc_dev; 
rt_uint32_t adc_val[10];
rt_uint32_t filter1_buf[FILTER_N + 1];
rt_uint32_t filter2_buf[FILTER_N + 1];


rt_uint32_t Filter(adc_channel channel) 
{
    int i;
    rt_uint32_t filter_sum = 0;
    rt_uint32_t filter1_sum = 0;
    rt_uint32_t filter2_sum = 0;
    rt_uint8_t adc1_index = 0;
    rt_uint8_t adc2_index = 0;
    switch(channel)
    {
        case channel5:
            if(adc1_index < FILTER_N){
                filter1_buf[adc1_index++] = rt_adc_read(adc_dev, 5);
            }else{
                filter1_buf[FILTER_N] = rt_adc_read(adc_dev, 5);
                for(i = 0; i < FILTER_N; i++){
                    filter1_buf[i] = filter1_buf[i + 1];  //所有数据左移，低位仍掉
                    filter1_sum += filter1_buf[i];
                }

                filter_sum = (rt_uint32_t)(filter1_sum / FILTER_N);
            }
        break;
        case channel6:
            if(adc2_index < FILTER_N){
                filter2_buf[adc1_index++] = rt_adc_read(adc_dev, 5);
            }else{
                filter2_buf[FILTER_N] = rt_adc_read(adc_dev, 5);
                for(i = 0; i < FILTER_N; i++){
                    filter2_buf[i] = filter2_buf[i + 1];  //所有数据左移，低位仍掉
                    filter2_sum += filter2_buf[i];
                }

                filter_sum = (rt_uint32_t)(filter2_sum / FILTER_N);
            }
        break;
    }

    return filter_sum;
}


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
        adc_val[0] = Filter(channel5);
        adc_val[1] = Filter(channel6);

        rt_thread_mdelay(100);
    }

    return RT_EOK;
}



