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
        return 0;

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
                        for(u_index=0;u_index<8;u_index++){
                            if(GetBit(dataRecv[2],u_index) == 1){
                                SetBit(pin_ctr_h,u_index);
                                //pinset            
                            }else{
                                ResetBit(pin_ctr_h,u_index);
                                //pinreset
                            }

                            if(GetBit(dataRecv[3],u_index) == 1){
                                SetBit(pin_ctr_l,u_index);
                                //pinset            
                            }else{
                                ResetBit(pin_ctr_l,u_index);
                                //pinreset
                            }
                        }

                        sendBuf[2] = pin_ctr_h;
                        sendBuf[3] = pin_ctr_l;
                        g_usb_cdc_sendData(sendBuf, 10);
                    break;
                    case 0xFC:       //逆变器软启动
                    break
                }
            }
        }
        

        rt_thread_mdelay(20);
    }
    
}


rt_uint8_t g_usb_cdc_sendData(rt_uint8_t* data,rt_uint8_t len)
{
    rt_mutex_take(&usb_lock, RT_WAITING_FOREVER);
    rt_device_write(vcom_dev, 0, data, len);
    rt_mutex_release(&usb_lock);
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

