/*
 * Copyright (c) 2019-present, Guolz
 *
 * Date           Author       Notes
 * 2019-11-28     guolz     first version
 */

#ifndef __G_MEASUER_H__
#define __G_MEASUER_H__

#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RT_CONFIG_USB_CDC               1
#define RT_CONFIG_UART3                 0

#define RT_MANAGER_THREAD_STACK_SZ      2048
#define RT_MANAGER_THREAD_PRIO          5
#define MANAGER_MQ_MSG_SZ               16
#define MANAGER_MQ_MAX_MSG              8

typedef struct {
	rt_uint8_t dpsp1000_Onoff;
	rt_uint8_t autoChecktype;
	rt_uint16_t autoCheckbyte;
	rt_uint8_t MainStep;
	rt_uint8_t step;
	float dc_voltage;
	float dc_current;
	float dc_energy;
	rt_uint16_t ac_voltage;
	rt_uint16_t ac_current;
	rt_uint16_t ac_freq;
	float ac_energy;
	float out_efficiency;
	float delta_voltage_percent;
	rt_uint8_t ErrorCode;
	rt_uint8_t IOStatus_h;
	rt_uint8_t IOStatus_l;
	rt_uint8_t AutoCheck;
	rt_uint8_t calibration_index;
}MesureManager;
extern MesureManager mMesureManager;

typedef enum{
	m_OverCurrent = 0,
	m_StartTime = 1,
	uart6_rx_signal
}msg_type;

#define hal_SetBit(data, offset)      data |= 1 << offset      //置位某位为1
#define hal_ResetBit(data, offset)    data &= ~(1 << offset)   //复位某位为0
#define hal_GetBit(data, offset)      ((data >> offset) &0x01) //获取某位
#define UshortToByte1(data)     	  ((uint8_t *)(&data))[0]  //获取ushort类型数据低位(low 8 bit)高位(high 8 bit)
#define UshortToByte0(data)     	  ((uint8_t *)(&data))[1]  //获取ushort类型数据

void g_MeasureQueue_send(rt_uint8_t type, const char *content);
rt_uint8_t g_MeasureAuto_Check_Get(void);
rt_uint16_t g_MeasureAuto_Checkbyte_Get(void);
rt_uint8_t g_MeasureError_Code_Get(void);
rt_err_t g_measure_manager_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
