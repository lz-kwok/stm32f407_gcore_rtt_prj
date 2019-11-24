 /*
 * Change Logs:
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <g_usb_cdc.h>

static rt_uint8_t usb_cdc_thread_stack[RT_USB_CDC_THREAD_STACK_SZ];
static struct rt_mutex usb_lock;
static rt_device_t vcom_dev = RT_NULL;
static rt_uint8_t recvLen = 0;
rt_uint8_t sendBuf[10] = {0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0D};
static rt_uint8_t pin_ctr_h,pin_ctr_l;

static void usb_cdc_entry(void *param)
{
    static rt_uint8_t u_index = 0;
    rt_uint8_t dataRecv[32];
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
                        g_usb_pin_control(dataRecv[2]);
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
                }
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


rt_err_t g_usb_cdc_init(void)
{
    static struct rt_thread usb_cdc_thread;
    rt_mutex_init(&usb_lock, "usb_lock", RT_IPC_FLAG_FIFO);

    rt_thread_init(&usb_cdc_thread,
                   "usb_cdc",
                   usb_cdc_entry, RT_NULL,
                   usb_cdc_thread_stack, RT_USB_CDC_THREAD_STACK_SZ,
                   RT_USB_CDC_THREAD_PRIO, 20);

    return rt_thread_startup(&usb_cdc_thread);
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

    rt_pin_mode(FAULT1,PIN_MODE_INPUT);
    rt_pin_mode(FAULT2,PIN_MODE_INPUT);
    rt_pin_mode(FAULT3,PIN_MODE_INPUT);
}
INIT_DEVICE_EXPORT(g_usb_pin_status_init);


void g_usb_pin_control(relaycmd cmd)
{
    if(cmd == Load_1_5kW_ON){
        rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
    }else if(cmd == Load_1_5kW_OFF){
        rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
    }else if(cmd == Load_3kW_ON){
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
    }else if(cmd == Load_3kW_OFF){
        rt_pin_write(MCU_KOUT11, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
    }else if(cmd == Load_Reverse_ON){
        rt_pin_write(MCU_KOUT4, PIN_HIGH);
    }else if(cmd == Load_Reverse_OFF){
        rt_pin_write(MCU_KOUT4, PIN_LOW);
    }else if(cmd == Load_Over_ON){
        rt_pin_write(MCU_KOUT13, PIN_HIGH);
        rt_pin_write(MCU_KOUT7, PIN_HIGH);
        // rt_pin_write(MCU_KOUT8, PIN_HIGH);
        rt_pin_write(MCU_KOUT9, PIN_HIGH);
        rt_pin_write(MCU_KOUT10, PIN_HIGH);
        rt_pin_write(MCU_KOUT11, PIN_HIGH);
    }else if(cmd == Load_Over_OFF){
        rt_pin_write(MCU_KOUT13, PIN_LOW);
        rt_pin_write(MCU_KOUT7, PIN_LOW);
        // rt_pin_write(MCU_KOUT8, PIN_LOW);
        rt_pin_write(MCU_KOUT9, PIN_LOW);
        rt_pin_write(MCU_KOUT10, PIN_LOW);
        rt_pin_write(MCU_KOUT11, PIN_LOW);
    }else if(cmd == Load_Short_Circuit_ON){
        rt_pin_write(MCU_KOUT12, PIN_HIGH);
    }else if(cmd == Load_Short_Circuit_OFF){
        rt_pin_write(MCU_KOUT12, PIN_LOW);
    }else if(cmd == Load_Precharge_ON){
        rt_pin_write(MCU_KOUT5, PIN_HIGH);
    }else if(cmd == Load_Precharge_OFF){
        rt_pin_write(MCU_KOUT5, PIN_LOW);
    }else if(cmd == Load_Main_ON){
        rt_pin_write(MCU_KOUT3, PIN_HIGH);
    }else if(cmd == Load_Main_OFF){
        rt_pin_write(MCU_KOUT3, PIN_LOW);
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
