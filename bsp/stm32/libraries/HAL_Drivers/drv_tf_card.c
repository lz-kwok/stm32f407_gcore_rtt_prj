/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      SummerGift   first version
 * 2018-12-11     greedyhao    Porting for stm32f7xx
 * 2019-01-03     zylx         modify DMA initialization and spixfer function
 */

#include "board.h"

#ifdef RT_USING_SPI

#if defined(BSP_USING_TFCARD) 
/* this driver can be disabled at menuconfig */

#include "drv_spi.h"
#include "drv_config.h"
#include "drv_tf_card.h"
#include <string.h>

//#define DRV_DEBUG
#define LOG_TAG              "drv.tfCard"
#include <drv_log.h>

#define TF_SPI_DEVICE_NAME     "tfCard"        /* SPI 设备名称 */


rt_uint8_t  SD_Ctrbus_Init = 0;       
struct rt_spi_device *spi_dev_microsd = NULL;

void tf_Card_Init(void)
{
    // if(SD_Ctrbus_Init == 0){
        SD_Ctrbus_Init = 1;
        __HAL_RCC_GPIOA_CLK_ENABLE();
        rt_pin_mode(TF_CS_PIN,PIN_MODE_OUTPUT);
        rt_pin_mode(TF_DET_PIN,PIN_MODE_INPUT);

        rt_hw_spi_device_attach("spi2", "tfCard", GPIOA, GPIO_PIN_0);

        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;

        cfg.max_hz = 20*400*1000;  

        spi_dev_microsd = (struct rt_spi_device *)rt_device_find(TF_SPI_DEVICE_NAME);
        rt_spi_configure(spi_dev_microsd, &cfg);                 
    // }
}

rt_uint8_t tf_Card_Send(rt_uint8_t cmd)
{
    rt_uint8_t recb;
    if(spi_dev_microsd != NULL){
        rt_spi_transfer(spi_dev_microsd,&cmd,&recb,1);
    }

    return recb;
}

void tf_Card_SendBuf(rt_uint8_t *data,rt_uint32_t len)
{
    if(spi_dev_microsd != NULL){
        rt_spi_transfer(spi_dev_microsd,data,RT_NULL,len);
    }
}

void tf_Card_RecBuf(rt_uint8_t *data,rt_uint32_t len)
{
    if(spi_dev_microsd != NULL){
        rt_spi_transfer(spi_dev_microsd,RT_NULL,data,len);
    }
}

void tf_Card_BusSpeed(rt_uint32_t khz)
{
    if(spi_dev_microsd != NULL){
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;

        cfg.max_hz = 20*khz*1000;                       

        rt_spi_configure(spi_dev_microsd, &cfg);
    }
}


#endif /* BSP_USING_TFCARD */
#endif /* RT_USING_SPI */
