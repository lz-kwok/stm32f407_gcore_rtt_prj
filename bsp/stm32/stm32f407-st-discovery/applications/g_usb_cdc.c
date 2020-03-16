 /*
 * Change Logs:
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <g_usb_cdc.h>
#include <g_uart.h>
#include <g_measure.h>

static rt_uint8_t usb_cdc_thread_stack[RT_USB_CDC_THREAD_STACK_SZ];
static struct rt_mutex usb_lock;
static rt_timer_t u_timer;
static rt_device_t vcom_dev = RT_NULL;
static rt_uint8_t recvLen = 0;
rt_uint8_t sendBuf[10] = {0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D};
static rt_uint8_t reSend;
rt_uint8_t MesureType = 0;
static rt_uint8_t vol_set = 0;
static float dpsp_vol_set_L = 77.0;
static float dpsp_vol_set_H = 130.0;


static void usb_cdc_entry(void *param)
{
    int u = 0;
    static rt_uint8_t u_index = 0;
    rt_uint8_t dataRecv[32];
    rt_uint8_t dpsp_cmd[32];
    memset(dpsp_cmd,0x0,32);
    vcom_dev = rt_device_find("vcom");
   
    if (vcom_dev)
        rt_device_open(vcom_dev, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_INT_TX);
    else
        return;

    while (RT_TRUE)
    {
        recvLen = rt_device_read(vcom_dev,0,dataRecv,32);
        if(recvLen > 0){
            if((dataRecv[0] == 0x0D)&&(dataRecv[recvLen - 1] == 0x0D)){
                switch(dataRecv[1])
                {
                    case 0xFE:       //查询版本
                        sendBuf[1] = dataRecv[1];
                        sendBuf[2] = MajorVer;
                        sendBuf[3] = MiddleVer;
                        sendBuf[4] = MinnorVer;
                        g_usb_cdc_sendData(sendBuf, 10);
                    break;
                    case 0xFD:       //接触器控制
                        g_usb_pin_control((relaycmd)dataRecv[2]);
                        if(dataRecv[2] == Load_Main_ON){
                            dataRecv[2] = Load_Precharge_ON;
                            sendBuf[1] = dataRecv[1];
                            sendBuf[2] = dataRecv[2];
                            g_usb_set_timer(RT_TRUE);
                        }
                        g_usb_cdc_sendData(dataRecv,recvLen);
                    break;
                    case 0xFC:       //逆变器软启动
                        sendBuf[1] = dataRecv[1];
                        sendBuf[2] = dataRecv[2];
                        g_usb_cdc_sendData(sendBuf, 10);
                        if(dataRecv[2]){
                            g_usb_control_softstart(RT_TRUE);
                        }else{
                            g_usb_control_softstart(RT_FALSE);
                        }
                    break;
                    case 0xFB:       //开启直流电源
                        if(dataRecv[2] == DC_Power_ON){
                            mMesureManager.dpsp1000_Onoff = 1;
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                        }else if(dataRecv[2] == DC_Power_OFF){
                            mMesureManager.dpsp1000_Onoff = 0;
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF");
                        }                  
                        g_usb_cdc_sendData(dataRecv,recvLen);      
                    break;
                    case 0xF2:       //标定            
                        if((dataRecv[2] == 0x00)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 1;
                            g_uart_sendto_Dpsp("VOLT 50.0");
                        }else if((dataRecv[2] == 0x01)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 2;
                            g_uart_sendto_Dpsp("VOLT 60.0");
                        }else if((dataRecv[2] == 0x02)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 3;
                            g_uart_sendto_Dpsp("VOLT 70.0");
                        }else if((dataRecv[2] == 0x03)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 4;
                            g_uart_sendto_Dpsp("VOLT 80.0");
                        }else if((dataRecv[2] == 0x04)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 5;
                            g_uart_sendto_Dpsp("VOLT 90.0");
                        }else if((dataRecv[2] == 0x05)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 6;
                            g_uart_sendto_Dpsp("VOLT 100.0");
                        }else if((dataRecv[2] == 0x06)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 7;
                            g_uart_sendto_Dpsp("VOLT 110.0");
                        }else if((dataRecv[2] == 0x07)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 8;
                            g_uart_sendto_Dpsp("VOLT 120.0");
                        }else if((dataRecv[2] == 0x08)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 9;
                            g_uart_sendto_Dpsp("VOLT 130.0");
                        }else if((dataRecv[2] == 0x09)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 10;
                            g_uart_sendto_Dpsp("VOLT 140.0");
                        }else if((dataRecv[2] == 0x0A)&&(dataRecv[3] == 0x02)){
                            mMesureManager.calibration_index = 11;
                            g_uart_sendto_Dpsp("VOLT 150.0");
                        }

                        else if((dataRecv[2] == 0x00)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 12; //30 41 52 63 74 85 96
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                        }
                        else if((dataRecv[2] == 0x01)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 13; //30 41 52 63 74 85 96
                            rt_pin_write(MCU_KOUT8, PIN_HIGH);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_SetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                        }else if((dataRecv[2] == 0x02)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 14;
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                        }else if((dataRecv[2] == 0x03)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 15;
                            rt_pin_write(MCU_KOUT8, PIN_HIGH);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_SetBit(mMesureManager.IOStatus_h,5);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            
                        }else if((dataRecv[2] == 0x04)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 16;
                            
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_HIGH);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_SetBit(mMesureManager.IOStatus_l,0);
                        }else if((dataRecv[2] == 0x05)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 17;
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            rt_pin_write(MCU_KOUT11, PIN_HIGH);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_SetBit(mMesureManager.IOStatus_l,0);
                        }else if((dataRecv[2] == 0x06)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 18;
                            rt_pin_write(MCU_KOUT8, PIN_HIGH);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            rt_pin_write(MCU_KOUT11, PIN_HIGH);
                            hal_SetBit(mMesureManager.IOStatus_h,5);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_SetBit(mMesureManager.IOStatus_l,0);
                        }else if((dataRecv[2] == 0x07)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 19;
                        }else if((dataRecv[2] == 0x08)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 20;
                        }else if((dataRecv[2] == 0x09)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 21;
                        }else if((dataRecv[2] == 0x0A)&&(dataRecv[3] == 0x05)){
                            mMesureManager.calibration_index = 22;
                        }

                        else if(dataRecv[2] == 0x0B){
                            mMesureManager.calibration_index = 0;
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                        }
                        g_usb_cdc_sendData(dataRecv,recvLen);      
                    break;
                    case 0xFA:       //源效应电压设置
                        if(dataRecv[2] == 0x01){
                            g_uart_sendto_Dpsp("VOLT 70.0");
                        }else if(dataRecv[2] == 0x02){
                            g_uart_sendto_Dpsp("VOLT 100.0");
                            rt_thread_mdelay(1000);
                            dpsp_vol_set_H = 94.0;
                            for(;;){
                                memset(dpsp_cmd,0x0,32);
                                dpsp_vol_set_H -= 1.0;
                                if(dpsp_vol_set_H < 81.0){
                                   break;
                                }
                                sprintf((char *)dpsp_cmd,"VOLT %.1f",dpsp_vol_set_H);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(1000);
                            }
                            g_uart_sendto_Dpsp("VOLT 80.0");
                            rt_thread_mdelay(1000);
                            mMesureManager.step = 5;
                        }else if(dataRecv[2] == 0x03){
                            mMesureManager.step = 6;
                            g_uart_sendto_Dpsp("VOLT 110.0");
                        }else if(dataRecv[2] == 0x04){
                            mMesureManager.step = 7;
                            g_uart_sendto_Dpsp("VOLT 137.5");
                        }else if(dataRecv[2] == 0x05){
                            g_uart_sendto_Dpsp("VOLT 142.0");
                        }                         
                        g_usb_cdc_sendData(dataRecv,recvLen);     
                        rt_thread_mdelay(1000); 
                    break;
                case 0xF9:
                        mMesureManager.AutoCheck = 1;
                        if(dataRecv[2] == 0x01){
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            g_usb_pin_control(Load_None_ON);
                        }else if(dataRecv[2] == 0x02){
                            mMesureManager.ErrorCode = 0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            g_usb_pin_control(Efficiency_ON);
                        }else if(dataRecv[2] == 0x03){
                            mMesureManager.ErrorCode = 0;
                            mMesureManager.step = 0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(1000);
                            rt_pin_write(MCU_KOUT11, PIN_HIGH);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            hal_SetBit(mMesureManager.IOStatus_l,0);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            rt_thread_mdelay(2000);
                            g_uart_sendto_Dpsp("VOLT 108.0");
                            rt_thread_mdelay(2000);
                            dpsp_vol_set_H = 106.0;
                            for(;;){
                                memset(dpsp_cmd,0x0,32);
                                dpsp_vol_set_H -= 2.0;
                                if(dpsp_vol_set_H < 82.0){
                                   break;
                                }
                                sprintf((char *)dpsp_cmd,"VOLT %.1f",dpsp_vol_set_H);
                                g_uart_sendto_Dpsp(dpsp_cmd);
                                rt_thread_mdelay(2000);
                            }

                            g_uart_sendto_Dpsp("VOLT 80.0");
                            // g_uart_sendto_Dpsp("VOLT 77.0");
                            rt_thread_mdelay(2000);
                            g_uart_sendto_Dpsp("VOLT 80.0");
                            // g_set_dpsp_vol(77.0);
                            mMesureManager.step = 5;
                        }else if(dataRecv[2] == 0x04){           //欠压
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(2000);
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            // g_set_dpsp_vol(77.0);
                            
                            g_usb_pin_control(Undervoltage_ON);
                        }else if(dataRecv[2] == 0x05){           //过压
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp("VOLT 100.0");
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(1000);
                            // g_set_dpsp_vol(120.0);
                            
                            g_usb_pin_control(Overvoltage_ON);
                        }else if(dataRecv[2] == 0x06){           //过载
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);

                            rt_thread_mdelay(1000);
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(1000);
                            // g_set_dpsp_vol(110.0);
                            g_usb_pin_control(Load_Over_ON);
                        }else if(dataRecv[2] == 0x07){           //过流
                            if(dataRecv[3] == 0x00){
                                mMesureManager.ErrorCode = 0x0;
                                mMesureManager.step = 11;
                                rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                                rt_pin_write(MCU_KOUT8, PIN_LOW);
                                rt_pin_write(MCU_KOUT9, PIN_LOW);
                                rt_pin_write(MCU_KOUT10, PIN_LOW);
                                rt_pin_write(MCU_KOUT11, PIN_LOW);
                                hal_ResetBit(mMesureManager.IOStatus_h,4);
                                hal_ResetBit(mMesureManager.IOStatus_h,5);
                                hal_ResetBit(mMesureManager.IOStatus_h,6);
                                hal_ResetBit(mMesureManager.IOStatus_h,7);
                                hal_ResetBit(mMesureManager.IOStatus_l,0);
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");    //恢复直流电源
                                mMesureManager.step = 11;
                                for(;;){
                                    u_index++;
                                    rt_thread_mdelay(1000);
                                    g_uart_sendto_Dpsp("VOLT 110.0");
                                    if(mMesureManager.dc_voltage > 10500.0){
                                        u_index = 0;
                                        break;
                                    }
                                    
                                    if(u_index == 10){
                                        u_index = 0;
                                        break;
                                    }
                                }
                                for(;;){
                                    u_index++;
                                    rt_thread_mdelay(2000);
                                    if(mMesureManager.ac_voltage > 21000.0){
                                        u_index = 0;
                                        break;
                                    }
                                    if(u_index == 10){
                                        u_index = 0;
                                        break;
                                    }
                                }   
                                mMesureManager.dpsp1000_Onoff = 0;
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF");     //关闭程控直流电源
                                rt_thread_mdelay(2000);
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF");
                            }else if(dataRecv[3] == 0x01){
                                g_usb_pin_control(OverCurrent_ON);
                            }
                            
                            // g_set_dpsp_vol(110.0);
                            // g_usb_pin_control(OverCurrent_ON);
                        }else if(dataRecv[2] == 0x08){           //短路
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            rt_thread_mdelay(1000);
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(1000);
                            // g_set_dpsp_vol(110.0);
                            g_usb_pin_control(Load_Short_Circuit_ON);
                        }else if(dataRecv[2] == 0x09){           //反接&
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            rt_pin_write(MCU_KOUT12, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            hal_ResetBit(mMesureManager.IOStatus_l,1);
                            rt_thread_mdelay(1000);
                            
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            g_usb_pin_control(Load_Main_OFF);        //关闭主接触器
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage < 1000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(10000);
                            rt_pin_write(MCU_KOUT4, PIN_HIGH);          //反接
                            hal_SetBit(mMesureManager.IOStatus_h,1);
                            rt_thread_mdelay(1000);                     //延迟1s
                            rt_pin_write(MCU_KOUT4, PIN_LOW);           //正接
                            hal_ResetBit(mMesureManager.IOStatus_h,1);
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF"); 
                            rt_thread_mdelay(1000);
                            rt_thread_mdelay(1000);
                            rt_thread_mdelay(1000);
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");    //恢复直流电源
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_pin_write(MCU_KOUT14, PIN_HIGH);         //预充电
                            hal_SetBit(mMesureManager.IOStatus_l,3);
                            rt_thread_mdelay(1000);
                            rt_pin_write(MCU_KOUT14, PIN_HIGH);         //预充电
                            rt_thread_mdelay(10000);   
                            rt_pin_write(MCU_KOUT3, PIN_HIGH);          //打开主接触器
                            hal_SetBit(mMesureManager.IOStatus_h,0);
                            rt_pin_write(MCU_KOUT2, PIN_HIGH);          //启动信号
                            hal_SetBit(mMesureManager.IOStatus_l,7);
                            rt_pin_write(MCU_KOUT14, PIN_LOW);          //关闭预充电
                            hal_ResetBit(mMesureManager.IOStatus_l,3);
                            mMesureManager.step = 20;

                            // g_set_dpsp_vol(110.0);
                            // g_usb_pin_control(Load_Reverse_ON);
                        }else if(dataRecv[2] == 0x0A){                  //启动时间
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            rt_pin_write(MCU_KOUT12, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            hal_ResetBit(mMesureManager.IOStatus_l,1);
                            rt_thread_mdelay(1000);
                            g_usb_pin_control(Load_Main_OFF);        //关闭主接触器
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage < 1000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            //3-0 4-1 5-2 6-3 7-4 8-5 9-6 10-7 11-0 12-1 13-2 14-3
                            rt_thread_mdelay(1000);
                            rt_pin_write(MCU_KOUT11, PIN_HIGH);
                            rt_pin_write(MCU_KOUT9, PIN_HIGH);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_SetBit(mMesureManager.IOStatus_l,0);
                            rt_thread_mdelay(10000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                            rt_thread_mdelay(1000);
                            g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");    //恢复直流电源
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10700.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            rt_thread_mdelay(1000); 
                            rt_pin_write(MCU_KOUT14, PIN_HIGH);         //预充电
                            hal_SetBit(mMesureManager.IOStatus_l,3);
                            rt_thread_mdelay(10000);   
                            rt_pin_write(MCU_KOUT3, PIN_HIGH);          //打开主接触器
                            rt_pin_write(MCU_KOUT2, PIN_HIGH);          //启动信号
                            rt_pin_write(MCU_KOUT14, PIN_LOW);          //关闭预充电
                            hal_SetBit(mMesureManager.IOStatus_h,0);
                            hal_SetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_l,3);
                            mMesureManager.step = 22;
                        }else if(dataRecv[2] == 0x0B){           //带吸尘器启动
                            mMesureManager.ErrorCode = 0x0;
                            rt_pin_write(MCU_KOUT7, PIN_LOW);    //每次执行前先置所有负载控制器单元为断开状态
                            rt_pin_write(MCU_KOUT8, PIN_LOW);
                            rt_pin_write(MCU_KOUT9, PIN_LOW);
                            rt_pin_write(MCU_KOUT10, PIN_LOW);
                            rt_pin_write(MCU_KOUT11, PIN_LOW);
                            hal_ResetBit(mMesureManager.IOStatus_h,4);
                            hal_ResetBit(mMesureManager.IOStatus_h,5);
                            hal_ResetBit(mMesureManager.IOStatus_h,6);
                            hal_ResetBit(mMesureManager.IOStatus_h,7);
                            hal_ResetBit(mMesureManager.IOStatus_l,0);
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp("VOLT 110.0");
                                if(mMesureManager.dc_voltage > 10500.0){
                                    u_index = 0;
                                    break;
                                }
                                
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            for(;;){
                                u_index++;
                                rt_thread_mdelay(2000);
                                if(mMesureManager.ac_voltage > 21000.0){
                                    u_index = 0;
                                    break;
                                }
                                if(u_index == 10){
                                    u_index = 0;
                                    break;
                                }
                            }
                            mMesureManager.step = 21;
                        }
                        g_usb_cdc_sendData(dataRecv,recvLen);   
                    break;
                case 0xF8:
                        mMesureManager.AutoCheck = 0;
                        mMesureManager.step = 0;
                        g_usb_cdc_sendData(dataRecv,recvLen);   
                    break;
                }
            }
        }
        

        if(mMesureManager.step == 8){   //欠压
            if(vol_set == 1){
                memset(dpsp_cmd,0x0,32);
                dpsp_vol_set_L -= 0.5;
                if(dpsp_vol_set_L < 60.0){
                    dpsp_vol_set_L = 60.0;
                }
                sprintf((char *)dpsp_cmd,"VOLT %.1f",dpsp_vol_set_L);
                g_uart_sendto_Dpsp(dpsp_cmd);
            }
            
            rt_thread_mdelay(2000);
            //赋值故障码
            if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x02)){
                vol_set = 0;
                dpsp_vol_set_L = 85.0;
                mMesureManager.ErrorCode = 0x02;
            }
            // g_set_dpsp_vol(60.0);
        }else if(mMesureManager.step == 9){     //过压
            if(vol_set == 1){
                memset(dpsp_cmd,0x0,32);
                dpsp_vol_set_H += 0.5;
                if(dpsp_vol_set_H > 155.0){
                    dpsp_vol_set_H = 155.0;
                }
                sprintf((char *)dpsp_cmd,"VOLT %.1f",dpsp_vol_set_H);
                g_uart_sendto_Dpsp(dpsp_cmd);
            }
            rt_thread_mdelay(2000);
            //赋值故障码
            if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x01)){
                vol_set = 0;
                dpsp_vol_set_H = 137.0;
                mMesureManager.ErrorCode = 0x01;
            }
        }else if(mMesureManager.step == 10){    //过载
             if(((mMesureManager.ac_voltage/100) < 200)&&(mMesureManager.ErrorCode == 0x06)){
                mMesureManager.ErrorCode = 0x06;
            }
        }else if(mMesureManager.step == 13){    //启动时间

        }
        // else if((mMesureManager.step == 0)&&(mMesureManager.dpsp1000_Onoff == 1)&&(mMesureManager.step != 8)&&(mMesureManager.step != 9)){ 
        //     if((mMesureManager.dc_voltage > 11200.0)||(mMesureManager.dc_voltage < 10800.0)){
        //         g_uart_sendto_Dpsp("VOLT 110.0");
        //         rt_thread_mdelay(500);
        //     }
        // }

        rt_thread_mdelay(20);
    }
    
}


void g_set_dpsp_vol(float volt)
{
    rt_uint8_t vol_set_string[32];
    float now_voltage = 0.0;
    
    for(;;){
        rt_thread_mdelay(800);
        now_voltage = (float)mMesureManager.dc_voltage/100;
        if((now_voltage - volt) > 1.0){
            memset(vol_set_string,0x0,32);
            now_voltage -= 0.5;
            sprintf((char *)vol_set_string,"VOLT %.1f",vol_set_string);
            g_uart_sendto_Dpsp(vol_set_string);
        }else if((now_voltage - volt) < -1.0){
            memset(vol_set_string,0x0,32);
            now_voltage += 0.5;
            sprintf((char *)vol_set_string,"VOLT %.1f",vol_set_string);
            g_uart_sendto_Dpsp(vol_set_string);
        }else{
            break;
        }
        
    }
}


rt_uint8_t g_usb_cdc_sendData(rt_uint8_t* data,rt_uint8_t len)
{
    rt_mutex_take(&usb_lock, RT_WAITING_FOREVER);
    int ret = rt_device_write(vcom_dev, 0, data, len);
    rt_mutex_release(&usb_lock);

    return ret;
}

static void g_usb_timerout_callback(void *parameter)
{
     if(reSend == 1){
        rt_pin_write(MCU_KOUT3, PIN_HIGH);        //打开主接触器
        hal_SetBit(mMesureManager.IOStatus_h,0);
        reSend = 0;
        sendBuf[2] = Load_Main_ON;
#if RT_CONFIG_USB_CDC
        g_usb_cdc_sendData(sendBuf, 10);
#endif
#if RT_CONFIG_UART3
        g_Client_data_send(sendBuf, 10);
#endif
        rt_pin_write(MCU_KOUT2, PIN_HIGH);
        rt_pin_write(MCU_KOUT14, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_l,3);
        hal_SetBit(mMesureManager.IOStatus_l,7);
     }
}

void g_usb_set_timer(rt_bool_t sta)
{
    if(sta == RT_TRUE){
        reSend = 1;
    }else{
        reSend = 0;
    }
}


rt_err_t g_usb_cdc_init(void)
{
    static struct rt_thread usb_cdc_thread;
    rt_err_t ret;

    rt_mutex_init(&usb_lock, "usb_lock", RT_IPC_FLAG_FIFO);
    u_timer = rt_timer_create("u_timer", 
                             g_usb_timerout_callback, 
                             RT_NULL, 
                             5000, 
                             RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER); 

    rt_thread_init(&usb_cdc_thread,
                   "usb_cdc",
                   usb_cdc_entry, RT_NULL,
                   usb_cdc_thread_stack, RT_USB_CDC_THREAD_STACK_SZ,
                   RT_USB_CDC_THREAD_PRIO, 20);

    ret = rt_thread_startup(&usb_cdc_thread);

    return ret;
}



void g_usb_pin_status_init(void)
{
    rt_pin_mode(MCU_KOUT1,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT2,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT3,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT4,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT5,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT6,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT7,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT8,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT9,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT10,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT11,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT12,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT13,PIN_MODE_OUTPUT);
    rt_pin_mode(MCU_KOUT14,PIN_MODE_OUTPUT);

    rt_pin_mode(FAULT1,PIN_MODE_INPUT);
    rt_pin_mode(FAULT2,PIN_MODE_INPUT);
    rt_pin_mode(FAULT3,PIN_MODE_INPUT);
}
INIT_DEVICE_EXPORT(g_usb_pin_status_init);

void g_usb_IO_status_scan(void)
{
    if(rt_pin_read(FAULT1) == PIN_LOW){
        hal_SetBit(mMesureManager.IOStatus_l,4);
    }else if(rt_pin_read(FAULT1) == PIN_HIGH){
        hal_ResetBit(mMesureManager.IOStatus_l,4);
    }

    if(rt_pin_read(FAULT2) == PIN_LOW){
        hal_SetBit(mMesureManager.IOStatus_l,5);
    }else if(rt_pin_read(FAULT2) == PIN_HIGH){
        hal_ResetBit(mMesureManager.IOStatus_l,5);
    }

    if(rt_pin_read(FAULT3) == PIN_LOW){
        hal_ResetBit(mMesureManager.IOStatus_l,6);
    }else if(rt_pin_read(FAULT3) == PIN_HIGH){
        hal_SetBit(mMesureManager.IOStatus_l,6);
    }
}

void g_usb_pin_control(relaycmd cmd)
{
    if(cmd == Load_None_ON){
        mMesureManager.step = 1;
        rt_pin_write(MCU_KOUT7, PIN_LOW);         //3-0 4-1 5-2 6-3 7-4 8-5 9-6 10-7 11-0 12-1 
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,4);
        hal_ResetBit(mMesureManager.IOStatus_h,5);
        hal_ResetBit(mMesureManager.IOStatus_h,6);
        hal_ResetBit(mMesureManager.IOStatus_h,7);
        hal_ResetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Load_None_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Load_1_5kW_ON){
        mMesureManager.step = 2;
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_h,5);
        hal_SetBit(mMesureManager.IOStatus_h,6);
    }else if(cmd == Load_1_5kW_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,5);
        hal_ResetBit(mMesureManager.IOStatus_h,6);
    }else if(cmd == Load_3kW_ON){
        mMesureManager.step = 3;
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_h,6);
        hal_SetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Load_3kW_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,6);
        hal_ResetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Efficiency_ON){
        mMesureManager.step = 4;
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_h,6);
        hal_SetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Efficiency_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,6);
        hal_ResetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Undervoltage_ON){
        mMesureManager.step = 8;
        vol_set = 1;
        dpsp_vol_set_L = 77.0;
    }else if(cmd == Undervoltage_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Overvoltage_ON){
        mMesureManager.step = 9;
        vol_set = 1;
        dpsp_vol_set_H = 120.0;
    }else if(cmd == Overvoltage_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Load_Reverse_ON){
        rt_pin_write(MCU_KOUT4, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_h,1);
        mMesureManager.step = 20;
    }else if(cmd == Load_Reverse_OFF){
        rt_pin_write(MCU_KOUT4, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,1);
    }else if(cmd == Load_Over_ON){      //3-0 4-1 5-2 6-3 7-4 8-5 9-6 10-7 11-0 12-1 
        mMesureManager.step = 10;
        rt_pin_write(MCU_KOUT6, PIN_HIGH);
        rt_pin_write(MCU_KOUT7, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        rt_pin_write(MCU_KOUT10, PIN_HIGH);
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_h,3);
        hal_SetBit(mMesureManager.IOStatus_h,4);
        hal_SetBit(mMesureManager.IOStatus_h,6);
        hal_SetBit(mMesureManager.IOStatus_h,7);
        hal_SetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Load_Over_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT6, PIN_LOW);
        rt_pin_write(MCU_KOUT7, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_h,3);
        hal_ResetBit(mMesureManager.IOStatus_h,4);
        hal_ResetBit(mMesureManager.IOStatus_h,6);
        hal_ResetBit(mMesureManager.IOStatus_h,7);
        hal_ResetBit(mMesureManager.IOStatus_l,0);
    }else if(cmd == Load_Short_Circuit_ON){
         mMesureManager.step = 12;
         rt_pin_write(MCU_KOUT12, PIN_HIGH);
         hal_SetBit(mMesureManager.IOStatus_l,1);
    }else if(cmd == Load_Short_Circuit_OFF){
         mMesureManager.step = 0;
         rt_pin_write(MCU_KOUT12, PIN_LOW);
         hal_ResetBit(mMesureManager.IOStatus_l,1);
    }else if(cmd == Load_Precharge_ON){
        rt_pin_write(MCU_KOUT14, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_l,3);
    }else if(cmd == Load_Precharge_OFF){
        rt_pin_write(MCU_KOUT14, PIN_LOW);
        hal_ResetBit(mMesureManager.IOStatus_l,3);
    }else if(cmd == Load_Main_ON){
        rt_pin_write(MCU_KOUT14, PIN_HIGH);
        hal_SetBit(mMesureManager.IOStatus_l,3);
        if(u_timer != RT_NULL){
            rt_timer_start(u_timer);
        }
    }else if(cmd == Load_Main_OFF){         //3-0 4-1 5-2 6-3 7-4 8-5 9-6 10-7 11-0 12-1 
        rt_pin_write(MCU_KOUT2, PIN_LOW);   //启动信号
        rt_pin_write(MCU_KOUT3, PIN_LOW);   //主接触器
        rt_pin_write(MCU_KOUT14, PIN_LOW);  //预充电
        hal_ResetBit(mMesureManager.IOStatus_h,0);
        hal_ResetBit(mMesureManager.IOStatus_l,3);
        hal_ResetBit(mMesureManager.IOStatus_l,7);
    }else if(cmd == OverCurrent_ON){
        rt_thread_mdelay(1000);
        rt_pin_write(MCU_KOUT6, PIN_HIGH);
        rt_pin_write(MCU_KOUT7, PIN_HIGH);
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        rt_pin_write(MCU_KOUT10, PIN_HIGH);
        rt_pin_write(MCU_KOUT11, PIN_HIGH);

        hal_SetBit(mMesureManager.IOStatus_h,3);
        hal_SetBit(mMesureManager.IOStatus_h,4);
        hal_SetBit(mMesureManager.IOStatus_h,5);
        hal_SetBit(mMesureManager.IOStatus_h,6);
        hal_SetBit(mMesureManager.IOStatus_h,7);
        hal_SetBit(mMesureManager.IOStatus_l,0);
        g_MeasureQueue_send(m_OverCurrent,RT_NULL);
    }else if(cmd == OverCurrent_OFF){
        mMesureManager.step = 0;
    }else if(cmd == AllContactor_OFF){
        rt_pin_write(MCU_KOUT1, PIN_LOW);
        rt_pin_write(MCU_KOUT2, PIN_LOW);   //启动信号
        rt_pin_write(MCU_KOUT3, PIN_LOW);
        rt_pin_write(MCU_KOUT4, PIN_LOW);
        rt_pin_write(MCU_KOUT5, PIN_LOW);
        rt_pin_write(MCU_KOUT6, PIN_LOW);
        rt_pin_write(MCU_KOUT7, PIN_LOW);
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT12, PIN_LOW);
        rt_pin_write(MCU_KOUT13, PIN_LOW);
        rt_pin_write(MCU_KOUT14, PIN_LOW);  //预充电
    }
}


void g_usb_control_softstart(rt_bool_t start)
{
    if(start){
        rt_pin_write(MCU_KOUT2, PIN_HIGH);
    }else{
        rt_pin_write(MCU_KOUT2, PIN_LOW);
    }
}
