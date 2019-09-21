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

#define DigitLedData    GET_PIN(B, 11)
#define DigitLedclk     GET_PIN(E, 15)


#define RT_GUI_THREAD_STACK_SZ      256
#define RT_GUI_THREAD_PRIO          12
#define GUI_MQ_MSG_SZ               16
#define GUI_MQ_MAX_MSG              8

void g_Gui_show_pic(const char *pic);
rt_err_t g_Gui_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
