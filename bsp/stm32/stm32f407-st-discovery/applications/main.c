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
#include <stdlib.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>

#include <g_usb_cdc.h>
#include <g_uart.h>
#include <g_measure.h>
#include <g_ade7880.h>

typedef enum  {
  channel5         = 1,
  channel6,
  channel7,
  channel8
} adc_channel;

typedef enum {
    Loadeffect = 0,
    Efficiency = 1,
    Sourceeffect,
    Undervoltage,
    Overvoltage,
    OverLoad,
    ShortCircuit,

} mesureIndex;

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)

#define ADC_DEV_NAME        "adc3"  
#define FILTER_N            20

rt_adc_device_t adc_dev; 
rt_uint16_t adc_val[10];
rt_uint32_t filter1_buf[FILTER_N + 1];
rt_uint32_t filter2_buf[FILTER_N + 1];
rt_uint32_t filter3_buf[FILTER_N + 1];
rt_uint32_t filter4_buf[FILTER_N + 1];


rt_uint32_t Filter(adc_channel channel) 
{
    int i;
    static int bufprevol_in = 0;
    static int bufprecur_in = 0;
    static int bufprevol_out = 0;
    static int bufprecur_out = 0;
    rt_uint32_t filter1_sum = 0;
    rt_uint32_t filter2_sum = 0;
    rt_uint32_t filter3_sum = 0;
    rt_uint32_t filter4_sum = 0;

    if(channel == channel5){
        if(bufprevol_in == 0){
            bufprevol_in = 1;
            for(i=0;i<FILTER_N;i++){
                filter1_buf[i] = rt_adc_read(adc_dev, 5);
                rt_thread_mdelay(1);
            }
        }
        
        filter1_buf[FILTER_N] = rt_adc_read(adc_dev, 5);
        rt_thread_mdelay(1);
        for(i = 0; i < FILTER_N; i++){
            filter1_buf[i] = filter1_buf[i + 1];  //所有数据左移，低位仍掉
            filter1_sum += filter1_buf[i];
        }
        return (filter1_sum / FILTER_N);
    }else if(channel == channel6){
        if(bufprecur_in == 0){
            bufprecur_in = 1;
            for(i=0;i<FILTER_N;i++){
                filter2_buf[i] = rt_adc_read(adc_dev, 6);
                rt_thread_mdelay(1);
            }
        }
        
        filter2_buf[FILTER_N] = rt_adc_read(adc_dev, 6);
        rt_thread_mdelay(1);
        for(i = 0; i < FILTER_N; i++){
            filter2_buf[i] = filter2_buf[i + 1];  //所有数据左移，低位仍掉
            filter2_sum += filter2_buf[i];
        }
        return (filter2_sum / FILTER_N);
    }else if(channel == channel7){
        if(bufprevol_out == 0){
            bufprevol_out = 1;
            for(i=0;i<FILTER_N;i++){
                filter3_buf[i] = rt_adc_read(adc_dev, 7);
                rt_thread_mdelay(1);
            }
        }
        
        filter3_buf[FILTER_N] = rt_adc_read(adc_dev, 7);
        rt_thread_mdelay(1);
        for(i = 0; i < FILTER_N; i++){
            filter3_buf[i] = filter3_buf[i + 1];  //所有数据左移，低位仍掉
            filter3_sum += filter3_buf[i];
        }
        return (filter3_sum / FILTER_N);
    }else if(channel == channel8){
        if(bufprecur_out == 0){
            bufprecur_out = 1;
            for(i=0;i<FILTER_N;i++){
                filter4_buf[i] = rt_adc_read(adc_dev, 8);
                rt_thread_mdelay(1);
            }
        }
        filter4_buf[FILTER_N] = rt_adc_read(adc_dev, 8);
        rt_thread_mdelay(1);
        for(i = 0; i < FILTER_N; i++){
            filter4_buf[i] = filter4_buf[i + 1];  //所有数据左移，低位仍掉
            filter4_sum += filter4_buf[i];
        }

       return (filter4_sum / FILTER_N);
    }
    
    return 0;
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
    int i;
    static int index = 0;
    static int send_num_total = 0;
    static rt_uint16_t tmp_cur = 0;
    static rt_uint8_t ctr_done = 0;
    static rt_uint16_t mycheckbits = 0;
    rt_uint16_t vol_in,cur_in;
    rt_uint16_t vol_in_dc,cur_in_dc;
    rt_uint16_t vol_in_ac,cur_in_ac;
    // static float dpsp_vol_set = 0.0;
    // rt_uint8_t dpsp_cmd[32];
    // memset(dpsp_cmd,0x0,32);
    rt_uint8_t data_measure[32] = {0x0d,0xf9,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0d};
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    g_measure_manager_init();
#if (RT_CONFIG_USB_CDC)
    g_usb_cdc_init();
#endif
    g_uart_init();
    // rt_hw_ade7880_int();
    rt_thread_mdelay(1000);

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    rt_adc_enable(adc_dev, 5);
    rt_adc_enable(adc_dev, 6);
    rt_adc_enable(adc_dev, 7);
    rt_adc_enable(adc_dev, 8);
    
    while (RT_TRUE)
    {   
        adc_val[0] = Filter(channel5);
        adc_val[1] = Filter(channel6);
        adc_val[2] = Filter(channel7);
        adc_val[3] = Filter(channel8);
        g_usb_IO_status_scan();
        rt_thread_mdelay(50);
        vol_in_dc = adc_val[0]*330/4096;     //扩大100倍
        cur_in_dc = adc_val[1]*330/4096;     //扩大10倍
        vol_in_ac = adc_val[2]*330/4096;     //扩大100倍
        cur_in_ac = adc_val[3]*330/4096;     //扩大10倍
        
        mMesureManager.dc_voltage = (float)(((float)vol_in_dc)/100*79.42 - 2.407)*100;
        mMesureManager.dc_current = (float)(((float)cur_in_dc)/100*18.086 - 0.102)*100;
        mMesureManager.dc_energy = mMesureManager.dc_voltage*mMesureManager.dc_current;
        if(mMesureManager.dc_energy != 0.0){
            mMesureManager.out_efficiency = (mMesureManager.ac_energy/mMesureManager.dc_energy)*10000;
        }


        
        data_measure[0] = 0x0d;
        data_measure[1] = 0xe0;
        data_measure[2] = mMesureManager.step;
        data_measure[3] = ((rt_uint16_t)mMesureManager.dc_voltage)/100;
        data_measure[4] = ((rt_uint16_t)mMesureManager.dc_voltage)%100;
        data_measure[5] = ((rt_uint16_t)mMesureManager.dc_current)/100;
        data_measure[6] = ((rt_uint16_t)mMesureManager.dc_current)%100;
        data_measure[7] = ((rt_uint16_t)mMesureManager.ac_voltage)/100;
        data_measure[8] = ((rt_uint16_t)mMesureManager.ac_voltage)%100;

        if(mMesureManager.step == 10){
            if(tmp_cur < mMesureManager.ac_current){
                tmp_cur = mMesureManager.ac_current;
                mMesureManager.ac_current = tmp_cur;
                data_measure[9] = ((rt_uint16_t)tmp_cur)/100;
                data_measure[10] = ((rt_uint16_t)tmp_cur)%100;
            }
        }else{
            tmp_cur = 0;
            data_measure[9] = ((rt_uint16_t)mMesureManager.ac_current)/100;
            data_measure[10] = ((rt_uint16_t)mMesureManager.ac_current)%100;
        }

        data_measure[11] = ((rt_uint16_t)mMesureManager.ac_freq)/100;
        data_measure[12] = ((rt_uint16_t)mMesureManager.ac_freq)%100;
        data_measure[13] = ((rt_uint16_t)mMesureManager.out_efficiency)/100;
        data_measure[14] = ((rt_uint16_t)mMesureManager.out_efficiency)%100;
        data_measure[15] = mMesureManager.ErrorCode;
        data_measure[16] = send_num_total;
        data_measure[17] = mMesureManager.IOStatus;
        data_measure[18] = mMesureManager.AutoCheck;
        data_measure[19] = 0x0d;
        index ++;
        if(index == 20){
            index = 0;
            gInquiry_AC_data();
#if (RT_CONFIG_USB_CDC)
            if(mMesureManager.step != 0){
                g_usb_cdc_sendData(data_measure, 20);        
                if(mMesureManager.AutoCheck == 1){
                    if(mMesureManager.step <= 7){
                        send_num_total ++;
                        if(send_num_total == 11){
                            send_num_total = 0;
                            ctr_done = 0;
                            if((mMesureManager.step < 3)&&(mMesureManager.step > 0)){
                                mMesureManager.step++;
                            }else if(mMesureManager.step == 4){
                                mMesureManager.step = 0;;
                            }else if((mMesureManager.step > 4)&&(mMesureManager.step < 7)){
                                mMesureManager.step ++;
                            }else if(mMesureManager.step == 7){
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                mMesureManager.step = 0;
                            }
                        }
                    }else if(mMesureManager.step == 8){       //欠压
                        if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x02)){
                            send_num_total++;
                            if(send_num_total == 4){
                                send_num_total = 0;
                                mMesureManager.step = 0;
                                g_uart_sendto_Dpsp("VOLT 110.0");
                            }
                        }else{
                            send_num_total = 0;
                        }
                    }else if(mMesureManager.step == 9){       //过压
                        if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x01)){
                            send_num_total++;
                            if(send_num_total == 4){
                                send_num_total = 0;
                                mMesureManager.step = 0;
                                g_uart_sendto_Dpsp("VOLT 110.0");
                            }
                        }else{
                            send_num_total = 0;
                        }
                    }else if(mMesureManager.step == 10){
                        if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x06)){
                            send_num_total++;
                            if(send_num_total == 4){
                                send_num_total = 0;
                                mMesureManager.step = 0;
                            }
                        }else{
                            send_num_total = 0;
                        }
                    }else if(mMesureManager.step == 11){
                        // if(((mMesureManager.ac_voltage/100) < 200)&&((mMesureManager.ac_current/100) > 18)&&(mMesureManager.ErrorCode == 0x05)){
                        if(mMesureManager.ErrorCode == 0x05){
                            send_num_total++;
                            if(send_num_total == 4){
                                send_num_total = 0;
                                mMesureManager.step = 0;
                                rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                                rt_pin_write(MCU_KOUT8, PIN_LOW);
                                rt_pin_write(MCU_KOUT9, PIN_LOW);
                                rt_pin_write(MCU_KOUT10, PIN_LOW);
                                rt_pin_write(MCU_KOUT11, PIN_LOW);
                                rt_pin_write(MCU_KOUT13, PIN_LOW);
                            }
                        }else{
                            send_num_total = 0;
                            // rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            // rt_pin_write(MCU_KOUT8, PIN_LOW);
                            // rt_pin_write(MCU_KOUT9, PIN_LOW);
                            // rt_pin_write(MCU_KOUT10, PIN_LOW);
                            // rt_pin_write(MCU_KOUT11, PIN_LOW);
                            // rt_pin_write(MCU_KOUT13, PIN_LOW);
                        }
                    }else if(mMesureManager.step == 12){          //短路
                        if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x0B)){
                            send_num_total++;
                            if(send_num_total == 4){
                                send_num_total = 0;
                                mMesureManager.step = 0;
                                rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                                rt_pin_write(MCU_KOUT8, PIN_LOW);
                                rt_pin_write(MCU_KOUT9, PIN_LOW);
                                rt_pin_write(MCU_KOUT10, PIN_LOW);
                                rt_pin_write(MCU_KOUT11, PIN_LOW);
                                rt_pin_write(MCU_KOUT12, PIN_LOW);
                                rt_pin_write(MCU_KOUT13, PIN_LOW);
                            }

                        }
                    }else if(mMesureManager.step == 20){          //反接
                        send_num_total++;
                        if(send_num_total >= 15){
                            send_num_total = 0;
                            mMesureManager.step = 0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            rt_pin_write(MCU_KOUT13, PIN_LOW);
                        }else{
                            if((mMesureManager.ac_voltage/100) > 200){
                                if((send_num_total > 4)&&(send_num_total <= 10)){
                                    send_num_total = 0;
                                    mMesureManager.step = 0;
                                    rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                                    rt_pin_write(MCU_KOUT8, PIN_LOW);
                                    rt_pin_write(MCU_KOUT9, PIN_LOW);
                                    rt_pin_write(MCU_KOUT10, PIN_LOW);
                                    rt_pin_write(MCU_KOUT11, PIN_LOW);
                                    rt_pin_write(MCU_KOUT13, PIN_LOW);
                                }
                            }
                        }
                    }else if(mMesureManager.step == 22){  //&启动时间
                        send_num_total++;
                        if(send_num_total >= 15){
                            send_num_total = 0;
                            mMesureManager.step = 0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            rt_pin_write(MCU_KOUT13, PIN_LOW);
                        }else{
                            if((mMesureManager.ac_voltage/100) > 200){
                                if((send_num_total > 4)&&(send_num_total <= 10)){
                                    send_num_total = 0;
                                    mMesureManager.step = 0;
                                    rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                                    rt_pin_write(MCU_KOUT8, PIN_LOW);
                                    rt_pin_write(MCU_KOUT9, PIN_LOW);
                                    rt_pin_write(MCU_KOUT10, PIN_LOW);
                                    rt_pin_write(MCU_KOUT11, PIN_LOW);
                                    rt_pin_write(MCU_KOUT13, PIN_LOW);
                                }
                            }
                        }
                    }else if(mMesureManager.step == 21){
                        send_num_total++;
                        if(send_num_total == 4){
                            send_num_total = 0;
                            mMesureManager.step = 0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            rt_pin_write(MCU_KOUT12, PIN_LOW);
                            rt_pin_write(MCU_KOUT13, PIN_LOW);
                        }
                    }
                    
                }   
            }
            
#endif
#if (RT_CONFIG_UART3)
            g_Client_data_send(data_measure, 16);
#endif
        }

        if(mMesureManager.AutoCheck == 1){
            if(ctr_done == 0){
                ctr_done = 1;
                if(mMesureManager.step == 2){
                    rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                    rt_pin_write(MCU_KOUT8, PIN_LOW);
                    rt_pin_write(MCU_KOUT9, PIN_LOW);
                    rt_pin_write(MCU_KOUT10, PIN_LOW);
                    rt_pin_write(MCU_KOUT11, PIN_LOW);
                    rt_pin_write(MCU_KOUT13, PIN_LOW);
                    rt_thread_mdelay(500);
                    g_usb_pin_control(Load_1_5kW_ON);
                }else if(mMesureManager.step == 3){
                    rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                    rt_pin_write(MCU_KOUT8, PIN_LOW);
                    rt_pin_write(MCU_KOUT9, PIN_LOW);
                    rt_pin_write(MCU_KOUT10, PIN_LOW);
                    rt_pin_write(MCU_KOUT11, PIN_LOW);
                    rt_pin_write(MCU_KOUT13, PIN_LOW);
                    rt_thread_mdelay(500);
                    g_usb_pin_control(Load_3kW_ON);
                }else if(mMesureManager.step == 6){
                    g_uart_sendto_Dpsp("VOLT 90.0");
                    rt_thread_mdelay(2000);
                    g_uart_sendto_Dpsp("VOLT 100.0");
                    rt_thread_mdelay(2000);
                    g_uart_sendto_Dpsp("VOLT 110.0");
                    rt_thread_mdelay(2000);
                    // g_set_dpsp_vol(110.0);
                }else if(mMesureManager.step == 7){
                    g_uart_sendto_Dpsp("VOLT 120.0");
                    rt_thread_mdelay(1000);
                    g_uart_sendto_Dpsp("VOLT 125.0");
                    rt_thread_mdelay(2000);
                    g_uart_sendto_Dpsp("VOLT 132.0");
                    rt_thread_mdelay(2000);
                    g_uart_sendto_Dpsp("VOLT 137.5");
                    rt_thread_mdelay(2000);
                    // g_set_dpsp_vol(137.5);
                }
            }
        }else{
            send_num_total = 0;
            ctr_done = 0;
            mMesureManager.step = 0;
        }
        
    }

    return RT_EOK;
}



