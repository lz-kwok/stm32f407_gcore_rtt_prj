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
static struct rt_event event;
/* 串口设备句柄 */
static rt_device_t uart_device = RT_NULL;

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
    rt_event_send(&event, UART_RX_EVENT);
    
    return RT_EOK;
}

rt_uint8_t uart_getchar(void)
{
    rt_uint32_t e;
    rt_uint8_t ch;
    
    /* 读取1字节数据 */
    while (rt_device_read(uart_device, 0, &ch, 1) != 1)
    {
         /* 接收事件 */
        rt_event_recv(&event, UART_RX_EVENT,RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,RT_WAITING_FOREVER, &e);
    }

    return ch;
}

void uart_putchar(const rt_uint8_t c)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(uart_device, 0, &c, 1);
        timeout++;
    }
    while (len != 1 && timeout < 500);
}



rt_err_t uart_open(const char *name)
{
    rt_err_t res;
                                 
    /* 查找系统中的串口设备 */
    uart_device = rt_device_find(name);   
    /* 查找到设备后将其打开 */
    if (uart_device != RT_NULL)
    {   
       
        res = rt_device_set_rx_indicate(uart_device, uart_intput);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return -RT_ERROR;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(uart_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",name,res);
            return -RT_ERROR;
        }

        res = rt_device_control(uart_device,RT_DEVICE_CTRL_CONFIG,(void *)&dpsp_useconfig);
        if (res != RT_EOK)
        {
            rt_kprintf("control %s device error.%d\n",name,res);
            return -RT_ERROR;
        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n",name);
        return -RT_ERROR;
    }

    /* 初始化事件对象 */
    rt_event_init(&event, "event", RT_IPC_FLAG_FIFO); 
            
    return RT_EOK;
}


void g_uart_sendto_Dpsp(const rt_uint8_t *cmd)
{
    while(*cmd)
    {
        uart_putchar(*cmd++);
    }
}


void uart_thread_entry(void* parameter)
{    
    rt_uint8_t uart_recv[64] = {0};
    rt_uint8_t uart_recv_num = 0;
    
    /* 打开串口 */
    if (uart_open("uart1") != RT_EOK)
    {
        rt_kprintf("uart open error.\n");
         while (1)
         {
            rt_thread_delay(10);
         }
    }
       
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
        

