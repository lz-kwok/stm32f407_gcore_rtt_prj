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
static rt_uint8_t startTimeindex = 0;
rt_uint8_t MesureType = 0;
static rt_uint8_t vol_set = 0;
static float dpsp_vol_set_L = 77.0;
static float dpsp_vol_set_H = 130.0;


static void usb_cdc_entry(void *param)
{
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
                    case 0xFA:       //源效应电压设置
                        if(dataRecv[2] == 0x01){
                            g_uart_sendto_Dpsp("VOLT 70.0");
                        }else if(dataRecv[2] == 0x02){
                            mMesureManager.step = 5;
                            g_uart_sendto_Dpsp("VOLT 77.0");
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
            
            rt_thread_mdelay(3000);
            //赋值故障码
            if((mMesureManager.ac_voltage/100) < 200){
                vol_set = 0;
                dpsp_vol_set_L = 77.0;
                mMesureManager.ErrorCode = 0x02;
            }
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
            rt_thread_mdelay(3000);
            //赋值故障码
            if((mMesureManager.ac_voltage/100) < 200){
                vol_set = 0;
                dpsp_vol_set_H = 137.0;
                mMesureManager.ErrorCode = 0x01;
            }
        }else if(mMesureManager.step == 10){    //过载
            if((mMesureManager.ac_voltage/10000) < 200){
                mMesureManager.ErrorCode = 0x06;
            }
        }else if(mMesureManager.step == 11){    //过流
            if((mMesureManager.ac_voltage/10000) < 200){
                mMesureManager.ErrorCode = 0x05;
            }
        }else if(mMesureManager.step == 12){    //短路
            if((mMesureManager.ac_voltage/10000) < 200){
                mMesureManager.ErrorCode = 0x05;
            }
        }else if(mMesureManager.step == 13){    //启动时间
            if(startTimeindex == 2){
                startTimeindex = 0;
                mMesureManager.step = 0;
                if((mMesureManager.ac_voltage/10000) > 200){
                    hal_ResetBit(mMesureManager.IOStatus,4);
                }
            }
        }else if((mMesureManager.step == 0)&&(mMesureManager.dpsp1000_Onoff == 1)&&(mMesureManager.step != 8)&&(mMesureManager.step != 9)){ 
            if((mMesureManager.dc_voltage > 11200.0)||(mMesureManager.dc_voltage < 10800.0)){
                g_uart_sendto_Dpsp("VOLT 110.0");
                rt_thread_mdelay(500);
            }
        }

        rt_thread_mdelay(20);
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

        if(mMesureManager.step == 13){
            if(startTimeindex < 2){
                rt_timer_start(u_timer);
                startTimeindex ++;
            }
        }
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
        hal_ResetBit(mMesureManager.IOStatus,1);
    }else if(rt_pin_read(FAULT1) == PIN_HIGH){
        hal_SetBit(mMesureManager.IOStatus,1);
    }

    if(rt_pin_read(FAULT2) == PIN_LOW){
        hal_ResetBit(mMesureManager.IOStatus,2);
    }else if(rt_pin_read(FAULT2) == PIN_HIGH){
        hal_SetBit(mMesureManager.IOStatus,2);
    }

    if(rt_pin_read(FAULT3) == PIN_LOW){
        hal_ResetBit(mMesureManager.IOStatus,3);
    }else if(rt_pin_read(FAULT3) == PIN_HIGH){
        hal_SetBit(mMesureManager.IOStatus,3);
    }
}

void g_usb_pin_control(relaycmd cmd)
{
    if(cmd == Load_None_ON){
        mMesureManager.step = 1;
        rt_pin_write(MCU_KOUT7, PIN_LOW);
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT13, PIN_LOW);
    }else if(cmd == Load_None_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Load_1_5kW_ON){
        mMesureManager.step = 2;
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
    }else if(cmd == Load_1_5kW_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
    }else if(cmd == Load_3kW_ON){
        mMesureManager.step = 3;
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
    }else if(cmd == Load_3kW_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
    }else if(cmd == Efficiency_ON){
        mMesureManager.step = 4;
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
    }else if(cmd == Efficiency_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
    }else if(cmd == Undervoltage_ON){
        mMesureManager.step = 8;
        vol_set = 1;
        dpsp_vol_set_L = 77.0;
    }else if(cmd == Undervoltage_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Overvoltage_ON){
        mMesureManager.step = 9;
        vol_set = 1;
        dpsp_vol_set_H = 130.0;
    }else if(cmd == Overvoltage_OFF){
        mMesureManager.step = 0;
    }else if(cmd == Load_Reverse_ON){
        rt_pin_write(MCU_KOUT4, PIN_HIGH);
    }else if(cmd == Load_Reverse_OFF){
        rt_pin_write(MCU_KOUT4, PIN_LOW);
    }else if(cmd == Load_Over_ON){
        mMesureManager.step = 10;
        // rt_pin_write(MCU_KOUT13, PIN_HIGH);
        // rt_pin_write(MCU_KOUT7, PIN_HIGH);
        // // rt_pin_write(MCU_KOUT8, PIN_HIGH);
        // rt_pin_write(MCU_KOUT9, PIN_HIGH);
        // rt_pin_write(MCU_KOUT10, PIN_HIGH);
        // rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT6, PIN_HIGH);
        rt_pin_write(MCU_KOUT7, PIN_HIGH);
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        rt_pin_write(MCU_KOUT10, PIN_HIGH);
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
    }else if(cmd == Load_Over_OFF){
        mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT6, PIN_LOW);
        rt_pin_write(MCU_KOUT7, PIN_LOW);
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        // rt_pin_write(MCU_KOUT13, PIN_LOW);
        // rt_pin_write(MCU_KOUT7, PIN_LOW);
        // // rt_pin_write(MCU_KOUT8, PIN_LOW);
        // rt_pin_write(MCU_KOUT9, PIN_LOW);
        // rt_pin_write(MCU_KOUT10, PIN_LOW);
        // rt_pin_write(MCU_KOUT11, PIN_LOW);
    }else if(cmd == Load_Short_Circuit_ON){
         mMesureManager.step = 12;
        rt_pin_write(MCU_KOUT12, PIN_HIGH);
    }else if(cmd == Load_Short_Circuit_OFF){
         mMesureManager.step = 0;
        rt_pin_write(MCU_KOUT12, PIN_LOW);
    }else if(cmd == Load_Precharge_ON){
        rt_pin_write(MCU_KOUT14, PIN_HIGH);
    }else if(cmd == Load_Precharge_OFF){
        rt_pin_write(MCU_KOUT14, PIN_LOW);
    }else if(cmd == Load_Main_ON){
        rt_pin_write(MCU_KOUT14, PIN_HIGH);

        // mMesureManager.dpsp1000_Onoff = 1;
        // g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
        // rt_thread_mdelay(1000);
        // g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");

        if(u_timer != RT_NULL){
            rt_timer_start(u_timer);
        }
    }else if(cmd == Load_Main_OFF){
        rt_pin_write(MCU_KOUT2, PIN_LOW);   //启动信号
        rt_pin_write(MCU_KOUT3, PIN_LOW);   //主接触器
        rt_pin_write(MCU_KOUT14, PIN_LOW);  //预充电
    }else if(cmd == StartSig_ON){

    }else if(cmd == StartSig_ON){

    }else if(cmd == OverCurrent_ON){
        mMesureManager.step = 11;
    }else if(cmd == OverCurrent_OFF){
        mMesureManager.step = 0;
    }else if(cmd == StartTime_ON){
        mMesureManager.step = 13;
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);      //1.5kw on
        rt_thread_mdelay(1000);
        rt_pin_write(MCU_KOUT14, PIN_HIGH);     //预充电
        g_usb_set_timer(RT_TRUE);
        if(u_timer != RT_NULL){
            rt_timer_start(u_timer);
        }
        hal_SetBit(mMesureManager.IOStatus,4);
    }else if(cmd == StartTime_OFF){
        rt_pin_write(MCU_KOUT8, PIN_LOW);       
        rt_pin_write(MCU_KOUT9, PIN_LOW);      //1.5kw off
        mMesureManager.step = 0;
        hal_ResetBit(mMesureManager.IOStatus,4);
        startTimeindex = 0;
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
