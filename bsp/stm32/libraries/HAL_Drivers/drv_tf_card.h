/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-27     Guolz   first version
 */

#ifndef __DRV_TF_CARD_H_
#define __DRV_TF_CARD_H_

#include <rtthread.h>
#include "rtdevice.h"
#include <rthw.h>
#include <drv_common.h>
#include "drv_dma.h"

#define TF_CS_PIN               GET_PIN(A, 0)
#define TF_DET_PIN              GET_PIN(E, 11)


#define SD_CS_High()	        rt_pin_write(TF_CS_PIN, PIN_HIGH)
#define SD_CS_Low() 	        rt_pin_write(TF_CS_PIN, PIN_LOW)

void tf_Card_Init(void);
rt_uint8_t tf_Card_Send(rt_uint8_t cmd);
void tf_Card_SendBuf(rt_uint8_t *data,rt_uint32_t len);
void tf_Card_RecBuf(rt_uint8_t *data,rt_uint32_t len);
void tf_Card_BusSpeed(rt_uint32_t khz);


#endif /*__DRV_TF_CARD_H_ */
