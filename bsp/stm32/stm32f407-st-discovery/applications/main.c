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
#if 0
        if(g_MeasureAuto_Check_Get() == 1){
            if(mycheckbits == 0){
                mycheckbits = g_MeasureAuto_Checkbyte_Get();
                rt_thread_mdelay(100);
                dpsp_vol_set = 110.0;
            }else{
                //负载效应测试
                for(;;){
                    if(hal_GetBit(mycheckbits,Loadeffect) == 1){
                        static rt_uint8_t loadeffectStep = 0;
                        float vol_f = 0.0;
                        adc_val[0] = Filter(channel5);
                        adc_val[1] = Filter(channel6);
                        vol_in = adc_val[0]*330/4096;     //扩大100倍
                        cur_in = adc_val[1]*330/4096;     //扩大10倍
                        vol_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                        if(loadeffectStep == 0){
                            dpsp_vol_set = 110.0;
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                            rt_thread_mdelay(1000);
                            loadeffectStep = 1;
                        }else if(loadeffectStep == 1){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                loadeffectStep = 2;
                                //send
                                g_usb_pin_control(Load_1_5kW_ON);
                            }
                            
                        }else if(loadeffectStep == 2){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                loadeffectStep = 3;
                                //send
                                g_usb_pin_control(Load_1_5kW_OFF);
                                rt_thread_mdelay(1000);
                                g_usb_pin_control(Load_3kW_ON);
                            }
                        }else if(loadeffectStep == 3){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                loadeffectStep = 0;

                                //send
                                g_usb_pin_control(Load_3kW_OFF);
                                hal_ResetBit(mycheckbits,Loadeffect);
                                break;
                            }
                        }

                        if((vol_f - 110) > 1){
                            if(dpsp_vol_set > 65){
                                dpsp_vol_set -= 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }
                    }else{
                        break;
                    }
                }

                //效率测试
                for(;;){
                    if(hal_GetBit(mycheckbits,Efficiency) == 1){
                        static rt_uint8_t EfficiencyStep = 0;
                        float vol_f = 0.0;
                        adc_val[0] = Filter(channel5);
                        adc_val[1] = Filter(channel6);
                        vol_in = adc_val[0]*330/4096;     //扩大100倍
                        cur_in = adc_val[1]*330/4096;     //扩大10倍
                        vol_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                        if(EfficiencyStep == 0){
                            dpsp_vol_set = 110.0;
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                            rt_thread_mdelay(1000);
                            EfficiencyStep = 1;
                        }else if(EfficiencyStep == 1){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                EfficiencyStep = 0;

                                //send
                                g_usb_pin_control(Load_3kW_OFF);
                                hal_ResetBit(mycheckbits,Efficiency);
                                break;
                            }
                        }

                        if((vol_f - 110) > 1){
                            if(dpsp_vol_set > 65){
                                dpsp_vol_set -= 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }
                    }else{
                        break;
                    }
                }
                
                //源效应测试
                for(;;){
                    if(hal_GetBit(mycheckbits,Sourceeffect) == 1){
                        static rt_uint8_t SourceeffectStep = 0;
                        float wishVolt = 0.0;
                        float vol_f = 0.0;
                        adc_val[0] = Filter(channel5);
                        adc_val[1] = Filter(channel6);
                        vol_in = adc_val[0]*330/4096;     //扩大100倍
                        cur_in = adc_val[1]*330/4096;     //扩大10倍
                        vol_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                        if(SourceeffectStep == 0){
                            dpsp_vol_set = 77.0;
                            wishVolt = 77.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            SourceeffectStep = 1;
                        }else if(SourceeffectStep == 1){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                SourceeffectStep = 2;

                                //send

                                dpsp_vol_set = 110.0;
                                wishVolt = 110.0;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(1000);
                            }
                        }else if(SourceeffectStep == 2){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                SourceeffectStep = 3;

                                //send

                                dpsp_vol_set = 137.5;
                                wishVolt = 137.5;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(1000);
                            }
                        }else if(SourceeffectStep == 3){
                            index++;
                            rt_thread_mdelay(200);
                            if(index == 10){
                                index = 0;
                                SourceeffectStep = 0;

                                //send

                                g_usb_pin_control(Load_3kW_OFF);
                                hal_ResetBit(mycheckbits,Sourceeffect);
                                break;
                            }
                        }

                        if((vol_f - wishVolt) > 1){
                            if(dpsp_vol_set > 65){
                                dpsp_vol_set -= 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - wishVolt)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }
                    }else{
                        break;
                    }
                }

                //输入欠压测试
                for(;;){
                    float vol_f = 0.0;
                    adc_val[0] = Filter(channel5);
                    adc_val[1] = Filter(channel6);
                    vol_in = adc_val[0]*330/4096;     //扩大100倍
                    cur_in = adc_val[1]*330/4096;     //扩大10倍
                    vol_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                    if(hal_GetBit(mycheckbits,Undervoltage) == 1){
                        static rt_uint8_t UndervoltageStep = 0;
                        if(UndervoltageStep == 0){
                            dpsp_vol_set = 110.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            UndervoltageStep = 1;
                        }if(UndervoltageStep == 1){
                            dpsp_vol_set -= 1.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            gScan_Error_Code();

                            if(g_MeasureError_Code_Get() == 0x02){         //输入欠压
                                UndervoltageStep = 0;
                                //send
                                g_usb_pin_control(Load_3kW_OFF);
                                break;
                            }
                        }
                    }else{
                        break;
                    }
                }

                //输入过压测试
                for(;;){
                    float vol_f = 0.0;
                    adc_val[0] = Filter(channel5);
                    adc_val[1] = Filter(channel6);
                    vol_in = adc_val[0]*330/4096;     //扩大100倍
                    cur_in = adc_val[1]*330/4096;     //扩大10倍
                    vol_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                    if(hal_GetBit(mycheckbits,Overvoltage) == 1){
                        static rt_uint8_t OvervoltageStep = 0;
                        if(OvervoltageStep == 0){
                            dpsp_vol_set = 110.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            OvervoltageStep = 1;
                        }if(OvervoltageStep == 1){
                            dpsp_vol_set -= 1.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            gScan_Error_Code();

                            if(g_MeasureError_Code_Get() == 0x02){         //输入欠压
                                OvervoltageStep = 0;
                                //send
                                g_usb_pin_control(Load_3kW_OFF);
                                break;
                            }
                        }
                    }else{
                        break;
                    }
                }

                //过载过流测试
                for(;;){
                    float cur_f = 0.0;
                    float vol_f = 0.0;
                    adc_val[0] = Filter(channel5);
                    adc_val[1] = Filter(channel6);
                    vol_in = adc_val[0]*330/4096;     //扩大100倍
                    cur_in = adc_val[1]*330/4096;     //扩大10倍
                    cur_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                    if(hal_GetBit(mycheckbits,OverLoad) == 1){
                        static rt_uint8_t OverLoadStep = 0;
                        if(OverLoadStep == 0){
                            dpsp_vol_set = 110.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_Over_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            OverLoadStep = 1;
                        }if(OverLoadStep == 1){
                            rt_thread_mdelay(1000);
                            gScan_Error_Code();     

                            if(g_ade_base_parameter.PhaseAIRMS >= 19){
                                OverLoadStep = 2;
                                rt_uint8_t err = g_MeasureError_Code_Get();
                                //send
                            }
                        }if(OverLoadStep == 2){
                            rt_thread_mdelay(1000);
                            gScan_Error_Code();     

                            if(g_ade_base_parameter.PhaseAIRMS >= 32){
                                OverLoadStep = 2;
                                rt_uint8_t err = g_MeasureError_Code_Get();
                                //send
                                g_usb_pin_control(Load_Over_OFF);
                                break;
                            }
                        }

                        if((vol_f - 110) > 1){
                            if(dpsp_vol_set > 65){
                                dpsp_vol_set -= 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }
                    }else{
                        break;
                    }
                }

                //短路测试
                for(;;){
                    float cur_f = 0.0;
                    float vol_f = 0.0;
                    adc_val[0] = Filter(channel5);
                    adc_val[1] = Filter(channel6);
                    vol_in = adc_val[0]*330/4096;     //扩大100倍
                    cur_in = adc_val[1]*330/4096;     //扩大10倍
                    cur_f = (float)(((float)vol_in)/100*70.82 - 1.98);
                    if(hal_GetBit(mycheckbits,ShortCircuit) == 1){
                        static rt_uint8_t ShortCircuitStep = 0;
                        if(ShortCircuitStep == 0){
                            dpsp_vol_set = 110.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);   
                            g_uart_sendto_Dpsp(dpsp_cmd);
                            rt_thread_mdelay(1000);
                            ShortCircuitStep = 1;
                        }if(ShortCircuitStep == 1){
                            rt_thread_mdelay(1000);
                            rt_thread_mdelay(1000);    
                            g_usb_pin_control(Load_Short_Circuit_ON);

                            ShortCircuitStep = 2;
                        }if(ShortCircuitStep == 2){
                            ShortCircuitStep = 0;
                            rt_thread_mdelay(1000);
                            gScan_Error_Code();     

                            rt_uint8_t err = g_MeasureError_Code_Get();
                            //send
                            break;
                        }

                        if((vol_f - 110) > 1){
                            if(dpsp_vol_set > 65){
                                dpsp_vol_set -= 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf((char *)dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }
                    }else{
                        break;
                    }
                }


            }
        }
#endif

        adc_val[0] = Filter(channel5);
        adc_val[1] = Filter(channel6);
        adc_val[2] = Filter(channel7);
        adc_val[3] = Filter(channel8);
        rt_thread_mdelay(50);
        vol_in_dc = adc_val[0]*330/4096;     //扩大100倍
        cur_in_dc = adc_val[1]*330/4096;     //扩大10倍
        vol_in_ac = adc_val[2]*330/4096;     //扩大100倍
        cur_in_ac = adc_val[3]*330/4096;     //扩大10倍
        
        mMesureManager.dc_voltage = (float)(((float)vol_in_dc)/100*79.42 - 2.407)*100;
        mMesureManager.dc_current = (float)(((float)cur_in_dc)/100*18.086 - 0.102)*100;
        mMesureManager.dc_energy = mMesureManager.dc_voltage*mMesureManager.dc_current;
        if(mMesureManager.dc_energy != 0.0){
            mMesureManager.out_efficiency = (mMesureManager.ac_energy/mMesureManager.dc_energy)*100;
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
        data_measure[9] = ((rt_uint16_t)mMesureManager.ac_current)/100;
        data_measure[10] = ((rt_uint16_t)mMesureManager.ac_current)%100;
        data_measure[11] = ((rt_uint16_t)mMesureManager.ac_freq)/100;
        data_measure[12] = ((rt_uint16_t)mMesureManager.ac_freq)%100;
        data_measure[13] = ((rt_uint16_t)mMesureManager.out_efficiency)/100;
        data_measure[14] = ((rt_uint16_t)mMesureManager.out_efficiency)%100;
        data_measure[15] = mMesureManager.ErrorCode;
        data_measure[16] = mMesureManager.delta_voltage_percent;
        data_measure[17] = 0x0d;
        index ++;
        if(index == 20){
            index = 0;

#if (RT_CONFIG_USB_CDC)
            g_usb_cdc_sendData(data_measure, 18);
#endif
#if (RT_CONFIG_UART3)
            g_Client_data_send(data_measure, 16);
#endif
        }

        
    }

    return RT_EOK;
}



