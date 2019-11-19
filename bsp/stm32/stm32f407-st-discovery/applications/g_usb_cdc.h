/*
 * Copyright (c) 2019-present, Guolz
 *
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#ifndef __GUI_H__
#define __GUI_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MajorVer            1
#define MiddleVer           0
#define MinnorVer           0


#define RT_USB_CDC_THREAD_STACK_SZ      2048
#define RT_USB_CDC_THREAD_PRIO          7

#define SetBit(data, offset)            data |= 1 << offset      
#define ResetBit(data, offset)          data &= ~(1 << offset)   
#define GetBit(data, offset)            ((data >> offset) &0x01) 

rt_uint8_t g_usb_cdc_sendData(rt_uint8_t* data,rt_uint8_t len);

rt_err_t g_usb_cdc_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
