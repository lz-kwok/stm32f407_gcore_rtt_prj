/*
 * File      : g_uart.c
 * Change Logs:
 * Date           Author       Notes
 *2019-11-22      GLZ          the first version 
 */

#include <rthw.h>
#include <rtthread.h>
#include "board.h"

#include "g_uart.h"
#include "g_usb_cdc.h"


/* 串口接收事件标志 */
#define UART_RX_EVENT (1 << 0)

/* 事件控制块 */
static struct rt_event g_event1;
/* 串口设备句柄 */
static rt_device_t g_uart1 = RT_NULL;
const char *uart1_name = "uart1";



static struct serial_configure dpsp_useconfig = {
    BAUD_RATE_115200,
    DATA_BITS_8,
    STOP_BITS_1,
    PARITY_ODD,
    BIT_ORDER_MSB,
    NRZ_NORMAL,
    128,
    0
};
    
/* 回调函数 */
static rt_err_t uart_intput(rt_device_t dev, rt_size_t size)
{
    /* 发送事件 */
    rt_event_send(&g_event1, UART_RX_EVENT);
    
    return RT_EOK;
}

rt_uint8_t uart_getchar(rt_device_t dev)
{
    rt_uint32_t e;
    rt_uint8_t ch;
    
    /* 读取1字节数据 */
    while (rt_device_read(dev, 0, &ch, 1) != 1)
    {
         /* 接收事件 */
        rt_event_recv(&g_event1, UART_RX_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
    }

    return ch;
}

void uart_putchar(rt_device_t dev,const rt_uint8_t c)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(dev, 0, &c, 1);
        timeout++;
    }
    while (len != 1 && timeout < 500);
}



rt_device_t uart_open(const char *name)
{
    rt_err_t res;
    rt_device_t dev;                             
    /* 查找系统中的串口设备 */
    dev = rt_device_find(name);   
    /* 查找到设备后将其打开 */
    if (dev != RT_NULL)
    {   
       
        res = rt_device_set_rx_indicate(dev, uart_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return RT_NULL;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",name,res);
            return RT_NULL;
        }

        res = rt_device_control(dev,RT_DEVICE_CTRL_CONFIG,(void *)&dpsp_useconfig);
        if (res != RT_EOK)
        {
            rt_kprintf("control %s device error.%d\n",name,res);
            return RT_NULL;
        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n",name);
        return RT_NULL;
    }
            
    return dev;
}


void g_uart_sendto_Dpsp(rt_device_t dev,const rt_uint8_t *cmd)
{
    while(*cmd)
    {
        uart_putchar(dev,*cmd++);
    }
}


void uart_thread_entry(void* parameter)
{    
    rt_uint8_t uart_recv[64] = {0};
    rt_uint8_t uart_recv_num = 0;
    
    /* 打开串口 */
    g_uart1 = uart_open(uart1_name);
    if(g_uart1 == RT_NULL){
        rt_kprintf("%s open error.\n",uart1_name);
        while (1)
        {
            rt_thread_delay(10);
        }
    }
    /* 初始化事件对象 */
    rt_event_init(&g_event1, "g_event1", RT_IPC_FLAG_FIFO); 
       
    rt_kprintf("%s\n",__func__);
    
    while (1)
    {   
        uart_recv[0] = uart_getchar();
        
        g_usb_cdc_sendData(&uart_recv[0],1);
        rt_thread_delay(100);

    }            
}

rt_err_t g_uart_init(void)
{
    rt_thread_t g_tid;
    g_tid = rt_thread_create("uart_thread_entry",
                    uart_thread_entry, 
                    RT_NULL,
                    1024, 
                    2, 
                    10);
    /* 创建成功则启动线程 */
    if (g_tid != RT_NULL){
        rt_thread_startup(g_tid);
        return RT_EOK;
    }

    return RT_ERROR;
}
        

