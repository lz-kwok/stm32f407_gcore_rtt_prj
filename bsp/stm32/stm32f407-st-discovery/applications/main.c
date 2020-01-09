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



rt_uint32_t Filter(adc_channel channel) 
{
    int i;
    static int bufpreIN = 0;
    rt_uint32_t filter1_sum = 0;
    rt_uint32_t filter2_sum = 0;

    if(channel == channel5){
        if(bufpreIN == 0){
            bufpreIN = 1;
            for(i=0;i<FILTER_N;i++){
                filter1_buf[i] = rt_adc_read(adc_dev, 5);
            }
        }
        
        filter1_buf[FILTER_N] = rt_adc_read(adc_dev, 5);
        for(i = 0; i < FILTER_N; i++){
            filter1_buf[i] = filter1_buf[i + 1];  //所有数据左移，低位仍掉
            filter1_sum += filter1_buf[i];
        }

        return (filter1_sum / FILTER_N);
    }else if(channel == channel6){
        if(bufpreIN == 0){
            bufpreIN = 1;
            for(i=0;i<FILTER_N;i++){
                filter2_buf[i] = rt_adc_read(adc_dev, 6);
            }
        }
        filter2_buf[FILTER_N] = rt_adc_read(adc_dev, 6);
        for(i = 0; i < FILTER_N; i++){
            filter2_buf[i] = filter2_buf[i + 1];  //所有数据左移，低位仍掉
            filter2_sum += filter2_buf[i];
        }

       return (filter2_sum / FILTER_N);
    }
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
    static float dpsp_vol_set = 0.0;
    rt_uint8_t dpsp_cmd[32];
    memset(dpsp_cmd,0x0,32);
    rt_uint8_t data_adc[8] = {0x0d,0xf9,0x00,0x00,0x00,0x00,0x00,0x0d};
    rt_uint8_t spreadshowBuf[12] = {0x0d,0xe0,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0d};
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    g_measure_manager_init();
    g_usb_cdc_init();
    g_uart_init();
    rt_hw_ade7880_int();
    rt_thread_mdelay(1000);

    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    rt_adc_enable(adc_dev, 5);
    rt_adc_enable(adc_dev, 6);
    
    while (RT_TRUE)
    {   
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
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
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_vol_set);
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_vol_set);
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - wishVolt)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
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
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
                            rt_thread_mdelay(1000);
                            UndervoltageStep = 1;
                        }if(UndervoltageStep == 1){
                            dpsp_vol_set -= 1.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
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
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_3kW_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
                            rt_thread_mdelay(1000);
                            OvervoltageStep = 1;
                        }if(OvervoltageStep == 1){
                            dpsp_vol_set -= 1.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
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
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_usb_pin_control(Load_Over_ON);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp(dpsp_vol_set);
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
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
                        if(ShortCircuit == 0){
                            dpsp_vol_set = 110.0;
                            memset(dpsp_cmd,0x0,32);
                            rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);   
                            g_uart_sendto_Dpsp(dpsp_vol_set);
                            rt_thread_mdelay(1000);
                            ShortCircuit = 1;
                        }if(ShortCircuit == 1){
                            rt_thread_mdelay(1000);
                            rt_thread_mdelay(1000);    
                            g_usb_pin_control(Load_Short_Circuit_ON);

                            ShortCircuit = 2;
                        }if(ShortCircuit == 2){
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
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(500);
                            }
                        }else if((vol_f - 110)< -1){
                            if(dpsp_vol_set < 150){
                                dpsp_vol_set += 0.1;
                                memset(dpsp_cmd,0x0,32);
                                rt_sprintf(dpsp_cmd,"VOLT .1%f",dpsp_vol_set);
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


        // adc_val[0] = Filter(channel5);
        // adc_val[1] = Filter(channel6);
        // rt_thread_mdelay(100);
        // rt_hw_ade7880_IVE_get();
        // index ++;
        // if(index == 10){
        //     index = 0;
        //     vol_in = adc_val[0]*330/4096;     //扩大100倍
        //     cur_in = adc_val[1]*330/4096;     //扩大10倍
            
        //     float vol_f = (float)(((float)vol_in)/100*70.82 - 1.98)*100;
        //     data_adc[2] = ((rt_uint16_t)vol_f)/100;
        //     data_adc[3] = ((rt_uint16_t)vol_f)%100;
        //     data_adc[4] = cur_in/100;
        //     data_adc[5] = cur_in%100;
        //     // g_usb_cdc_sendData(data_adc, 8);
        // }

        
    }

    return RT_EOK;
}



