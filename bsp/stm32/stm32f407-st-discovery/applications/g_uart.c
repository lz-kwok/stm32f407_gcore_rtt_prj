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
#include "g_measure.h"


/* 串口接收事件标志 */
#define UART_RX_EVENT (1 << 0)

/* 串口设备句柄 */
static rt_device_t g_uart1 = RT_NULL;
const char *uart1_name = "uart1";
static rt_device_t g_uart3 = RT_NULL;
const char *uart3_name = "uart3";
static rt_device_t g_uart6 = RT_NULL;
const char *uart6_name = "uart6";

const rt_uint8_t scan_code[4] = {0xA1,0x5f,0x00,0xfe};
const rt_uint8_t uart3_int_num = 8;

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
static rt_err_t uart_rx_callback(rt_device_t dev, rt_size_t size)
{
    if(dev == g_uart3){
        if(size == uart3_int_num){
            g_MeasureQueue_send(uart3_rx_signal,(void *)&uart3_int_num);
        }
        
    }
    
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

rt_uint8_t g_Client_data_receive(rt_uint8_t *buf,rt_uint8_t len)
{
    rt_uint8_t ch;

    // rt_uint8_t recv_size = (sizeof(buf) - 1)>len ? len : (sizeof(buf) - 1);

    ch = rt_device_read(g_uart3, 0, buf, len);

    return ch;
}

void g_Client_data_send(rt_uint8_t *buf,rt_uint8_t len)
{
    // rt_size_t m_len = 0;
    // rt_uint32_t timeout = 0;
    // do
    // {
    //     m_len = rt_device_write(g_uart3, 0, buf, len);
    //     timeout++;
    // }
    // while (m_len != len && timeout < 500);
    rt_device_write(g_uart3, 0, (const void *)buf, len);
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

void gScan_Error_Code(rt_device_t dev,const rt_uint8_t *c,rt_uint8_t cmd_len)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(dev, 0, c, cmd_len);
        timeout++;
    }
    while (len != cmd_len && timeout < 500);
}

rt_uint8_t gGet_Error_Code(rt_device_t dev,rt_uint8_t *buf,rt_uint8_t len)
{
    rt_uint32_t e;
    rt_uint8_t rec_len = 0;
    
    
    rec_len = rt_device_read(dev, 0, buf, len);

    return 0;
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
       
        res = rt_device_set_rx_indicate(dev, uart_rx_callback);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return RT_NULL;
        }

        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",name,res);
            return RT_NULL;
        }

//        res = rt_device_control(dev,RT_DEVICE_CTRL_CONFIG,(void *)&dpsp_useconfig);
//        if (res != RT_EOK)
//        {
//            rt_kprintf("control %s device error.%d\n",name,res);
//            return RT_NULL;
//        }
    }
    else
    {
        rt_kprintf("can't find %s device.\n",name);
        return RT_NULL;
    }
            
    return dev;
}


void g_uart_sendto_Dpsp(const rt_uint8_t *cmd)
{
    while(*cmd)
    {
        uart_putchar(g_uart1,*cmd++);
    }
}

void uart_thread_entry(void* parameter)
{    
    rt_uint8_t uart_recv[64] = {0};
    rt_uint8_t uart_recv_num = 0;
    
    /* 打开串口 */
    g_uart1 = uart_open(uart1_name);                //dpsp
    if(g_uart1 == RT_NULL){
        rt_kprintf("%s open error.\n",uart1_name);
        while (1)
        {
            rt_thread_delay(10);
        }
    }

    g_uart3 = uart_open(uart3_name);                //client
    if(g_uart3 == RT_NULL){
        rt_kprintf("%s open error.\n",uart3_name);
        while (1)
        {
            rt_thread_delay(10);
        }
    }
    
    /* 打开串口 */
    g_uart6 = uart_open(uart6_name);                //单逆
    if(g_uart6 == RT_NULL){
        rt_kprintf("%s open error.\n",uart1_name);
        while (1)
        {
            rt_thread_delay(10);
        }
    }
       
    rt_kprintf("%s\n",__func__);
    
    while (1)
    {   
        // gScan_Error_Code(g_uart6,scan_code,4);

        rt_thread_mdelay(1000);
        

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
        

