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

#define MCU_KOUT1                       GET_PIN(G, 7)
#define MCU_KOUT2                       GET_PIN(G, 6)
#define MCU_KOUT3                       GET_PIN(G, 5)
#define MCU_KOUT4                       GET_PIN(G, 4)
#define MCU_KOUT5                       GET_PIN(G, 3)
#define MCU_KOUT6                       GET_PIN(D, 13)
#define MCU_KOUT7                       GET_PIN(G, 2)
#define MCU_KOUT8                       GET_PIN(D, 10)
#define MCU_KOUT9                       GET_PIN(E, 12)
#define MCU_KOUT10                      GET_PIN(E, 14)
#define MCU_KOUT11                      GET_PIN(E, 13)
#define MCU_KOUT12                      GET_PIN(F, 13)
#define MCU_KOUT13                      GET_PIN(F, 15)

#define FAULT1                          GET_PIN(B, 0)
#define FAULT2                          GET_PIN(B, 1)
#define FAULT3                          GET_PIN(F, 12)



rt_uint8_t g_usb_cdc_sendData(rt_uint8_t* data,rt_uint8_t len);

rt_err_t g_usb_cdc_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */