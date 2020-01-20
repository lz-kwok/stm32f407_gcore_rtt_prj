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
#include <g_ade7880.h>
#include <math.h>

/* 串口接收事件标志 */
#define UART_RX_EVENT (1 << 0)

/*******************************************************************************
*	        Variables Definitions										                                  											  *
*******************************************************************************/
const uint8_t CrcHighBlock[256] = {
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

const uint8_t CrcLowBlock[256] = {
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

/* 串口设备句柄 */
static rt_device_t g_uart1 = RT_NULL;
const char *uart1_name = "uart1";
static rt_device_t g_uart3 = RT_NULL;
const char *uart3_name = "uart3";
static rt_device_t g_uart6 = RT_NULL;
const char *uart6_name = "uart6";

const rt_uint8_t scan_code[4] = {0xA1,0x5f,0x00,0xfe};
const rt_uint8_t inquiry_code[20] = {0x01,0x03,0x00,0x48,0x00,0x08,0xc4,0x1a};
const rt_uint8_t uart3_int_num = 37;
const rt_uint8_t uart6_int_num = 8;
rt_uint8_t recv_flag = 0;
static struct rt_semaphore rx_sem;

#define IM1281B           \
{                                          \
    BAUD_RATE_4800,   /* 4800 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}

#define TN19002           \
{                                          \
    BAUD_RATE_9600,   /* 9600 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}
    
/* 回调函数 */
static rt_err_t uart_rx_callback(rt_device_t dev, rt_size_t size)
{
    if(dev == g_uart3){
        rt_sem_release(&rx_sem);        
    }
    else if(dev == g_uart6){
        if(size == uart6_int_num){
            g_MeasureQueue_send(uart6_rx_signal,(void *)&uart6_int_num);
        }
    }
    
    return RT_EOK;
}


rt_uint8_t g_Client_data_receive(rt_uint8_t *buf,rt_uint8_t len)
{
    rt_uint8_t ch;

    // rt_uint8_t recv_size = (sizeof(buf) - 1)>len ? len : (sizeof(buf) - 1);

    ch = rt_device_read(g_uart3, 0, buf, len);

    return ch;
}

rt_uint8_t g_ErrorCode_data_receive(rt_uint8_t *buf,rt_uint8_t len)
{
    rt_uint8_t ch;

    // rt_uint8_t recv_size = (sizeof(buf) - 1)>len ? len : (sizeof(buf) - 1);

    ch = rt_device_read(g_uart6, 0, buf, len);

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

void gScan_Error_Code(void)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(g_uart6, 0, scan_code, 4);
        timeout++;
    }
    while (len != 4 && timeout < 500);
}

rt_uint8_t gGet_Error_Code(rt_device_t dev,rt_uint8_t *buf,rt_uint8_t len)
{
    rt_uint8_t rec_len = 0;
    
    
    rec_len = rt_device_read(dev, 0, buf, len);

    return 0;
}

/*******************************************************************************
* Function Name  : Crc16(uint8_t *bufferpoint,int16_t sum)
* Description    : X^16 + X^15 + X^2 +1
* Input para     : *bufferpoint,sum
* Output para    : None
*******************************************************************************/
rt_uint16_t Crc16(rt_uint8_t *bufferpoint,rt_uint16_t sum)
{
	rt_uint8_t High = 0xFF;
	rt_uint8_t Low = 0xFF;
	rt_uint8_t index;
	rt_uint16_t Result;

	while(sum--)
	{
		index = High ^ *bufferpoint++;
		High = Low ^ CrcHighBlock[index];
		Low = CrcLowBlock[index];
	}
    UshortToByte1(Result) = Low;
    UshortToByte0(Result) = High;

    return(Result);
}


void gInquiry_AC_data(void)
{
    rt_size_t len = 0;
    rt_uint32_t timeout = 0;
    do
    {
        len = rt_device_write(g_uart3, 0, inquiry_code, 8);
        timeout++;
    }
    while (len != 8 && timeout < 500);
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
        /* 打开设备，以可读写、中断方式 */
        res = rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX );       
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("open %s device error.%d\n",name,res);
            return RT_NULL;
        }
        
        res = rt_device_set_rx_indicate(dev, uart_rx_callback);
        /* 检查返回值 */
        if (res != RT_EOK)
        {
            rt_kprintf("set %s rx indicate error.%d\n",name,res);
            return RT_NULL;
        }

        if(strcmp(name,"uart3") == 0){
            struct serial_configure uart_config = IM1281B;
            res = rt_device_control(dev,RT_DEVICE_CTRL_CONFIG,(void *)&uart_config);
            if (res != RT_EOK)
            {
                rt_kprintf("control %s device error.%d\n",name,res);
                return RT_NULL;
            }
        }else if(strcmp(name,"uart6") == 0){
            struct serial_configure uart_config = TN19002;
            res = rt_device_control(dev,RT_DEVICE_CTRL_CONFIG,(void *)&uart_config);
            if (res != RT_EOK)
            {
                rt_kprintf("control %s device error.%d\n",name,res);
                return RT_NULL;
            }
        }
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
    rt_uint8_t CRC_Result[2];
    rt_uint8_t uart_recv_num = 0;
    rt_uint16_t CalcuResult;
    rt_uint32_t im_vol,im_cur,im_pow,im_energy,im_pf,im_frq,im_co2;
    rt_uint32_t im_vol_max = 0;
    rt_uint32_t im_vol_min = 380*10000;
    rt_uint8_t data_recv;

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
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
    memset(uart_recv,0x0,64);
    while (1)
    {   
        while (rt_device_read(g_uart3, -1, &data_recv, 1) != 1){
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        uart_recv[uart_recv_num++] = data_recv;
        if(uart_recv_num == 37){
            CalcuResult = Crc16(uart_recv,uart_recv_num-2);
            CRC_Result[0] = (uint8_t)((CalcuResult & 0xFF00) >> 8);
            CRC_Result[1] = (uint8_t)(CalcuResult & 0xFF);
            if((uart_recv[uart_recv_num-2] == CRC_Result[0]) && (uart_recv[uart_recv_num-1] == CRC_Result[1]))   //判断数据接收是否存在异常
            {
                im_vol = (((rt_uint32_t)(uart_recv[3]))<<24)|(((rt_uint32_t)(uart_recv[4]))<<16)|(((rt_uint32_t)(uart_recv[5]))<<8)|uart_recv[6];
                mMesureManager.ac_voltage = (rt_uint16_t)(im_vol/100);
                im_cur = (((rt_uint32_t)(uart_recv[7]))<<24)|(((rt_uint32_t)(uart_recv[8]))<<16)|(((rt_uint32_t)(uart_recv[9]))<<8)|uart_recv[10]; 
                mMesureManager.ac_current = (rt_uint16_t)(im_cur/100);
                im_pow = (((rt_uint32_t)(uart_recv[11]))<<24)|(((rt_uint32_t)(uart_recv[12]))<<16)|(((rt_uint32_t)(uart_recv[13]))<<8)|uart_recv[14]; 
                mMesureManager.ac_energy = (float)(((float)im_pow)/100);
                im_energy = (((rt_uint32_t)(uart_recv[15]))<<24)|(((rt_uint32_t)(uart_recv[16]))<<16)|(((rt_uint32_t)(uart_recv[17]))<<8)|uart_recv[18]; 
                im_pf = (((rt_uint32_t)(uart_recv[19]))<<24)|(((rt_uint32_t)(uart_recv[20]))<<16)|(((rt_uint32_t)(uart_recv[21]))<<8)|uart_recv[22]; 
                im_co2 = (((rt_uint32_t)(uart_recv[23]))<<24)|(((rt_uint32_t)(uart_recv[24]))<<16)|(((rt_uint32_t)(uart_recv[25]))<<8)|uart_recv[26]; 
                im_frq = (((rt_uint32_t)(uart_recv[23]))<<31)|(((rt_uint32_t)(uart_recv[32]))<<16)|(((rt_uint32_t)(uart_recv[33]))<<8)|uart_recv[34]; 
                mMesureManager.ac_freq = (rt_uint16_t)(im_frq/100);

                if(im_vol > im_vol_max){
                    im_vol_max = im_vol;
                }
                if(im_vol_min > im_vol){
                    im_vol_min = im_vol;
                }
                mMesureManager.delta_voltage_percent = (abs(im_vol-2200000))*100/im_vol;
            }
            uart_recv_num = 0;
            memset(uart_recv,0x0,64);
            gScan_Error_Code();
        }
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
        

