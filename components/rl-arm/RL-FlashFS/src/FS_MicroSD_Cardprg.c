/*
*********************************************************************************************************
*
*	模块名称 : FlashFS的接口文件
*	文件名称 : FS_MicroSD_Cardprg.c
*	版    本 : V1.0
*	说    明 : FlashFS对Micro SD卡的接口文件
*
*	修改记录 :
*		版本号  日期        作者       说明
*		V1.0    2020-03-24  leonGuo  测试版本
*
*	Copyright (C), 2020-present, lz_kwok@163.com
*
*********************************************************************************************************
*/
#include <rtthread.h>
#include <rtdevice.h>
#include <File_Config.h>
#include "drv_spi.h"
#include "drv_tf_card.h"

// #define TF_CS_PIN              GET_PIN(A, 0)
// #define TF_DET_PIN             GET_PIN(E, 11)
// #define TF_SPI_DEVICE_NAME     "tfCard"        /* SPI 设备名称 */

// struct rt_spi_device *spi_dev_microsd = NULL;

/*-----------------------------------------------------------------------------
 *      NAND driver prototypes
 *----------------------------------------------------------------------------*/
static BOOL Init         (void);
static BOOL UnInit       (void);
static U8 Send           (U8 outb);
static BOOL SendBuf      (U8 *buf, U32 sz);
static BOOL RecBuf       (U8 *buf, U32 sz);
static BOOL BusSpeed     (U32 kbaud);
static BOOL SetSS        (U32 ss);
static U32  CheckMedia   (void);
/*-----------------------------------------------------------------------------
  NAND Device Driver Control Block
 *----------------------------------------------------------------------------*/
const SPI_DRV spi0_drv = {
  Init,
  UnInit,
  Send,
  SendBuf,
  RecBuf,
  BusSpeed,
  SetSS,
  CheckMedia
};



static BOOL Init (void) {
   // __HAL_RCC_GPIOA_CLK_ENABLE();
   // rt_pin_mode(TF_CS_PIN,PIN_MODE_OUTPUT);
   // rt_pin_mode(TF_DET_PIN,PIN_MODE_INPUT);

   // rt_hw_spi_device_attach("spi2", "tfCard", GPIOA, GPIO_PIN_0);

   // struct rt_spi_configuration cfg;
   // cfg.data_width = 8;
   // cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;

   // cfg.max_hz = 20*400*1000;  

	// spi_dev_microsd = (struct rt_spi_device *)rt_device_find(TF_SPI_DEVICE_NAME);
   // rt_spi_configure(spi_dev_microsd, &cfg);

   tf_Card_Init();

   return __TRUE;
}


static BOOL UnInit (void) {

   return __TRUE;
}


static U8 Send(U8 outb){
   U8 recb;
   
   // if(spi_dev_microsd != NULL){
   //    rt_spi_transfer(spi_dev_microsd,&outb,&recb,1);
   // }
   recb = tf_Card_Send(outb);

   return recb;
}


static BOOL SendBuf(U8 *buf, U32 sz)
{
   // U8 Status;
   // U32 i;
   
   // if(spi_dev_microsd != NULL){
   //    rt_spi_transfer(spi_dev_microsd,buf,RT_NULL,sz);
   // }
   tf_Card_SendBuf(buf,sz);

   return __TRUE;
}

static BOOL RecBuf(U8 *buf, U32 sz)
{
   // U8 Status;
   // U8 outb = 0xff;
   // U32 i;

   // if(spi_dev_microsd != NULL){
   //    rt_spi_transfer(spi_dev_microsd,RT_NULL,buf,sz);
   // }
   tf_Card_RecBuf(buf,sz);

   return __TRUE;
}

static BOOL BusSpeed(U32 kbaud)
{
   // struct rt_spi_configuration cfg;
   // cfg.data_width = 8;
   // cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;

   // cfg.max_hz = 20*kbaud*1000;                       

   // rt_spi_configure(spi_dev_microsd, &cfg);
   tf_Card_BusSpeed(kbaud);

   return __TRUE;
}

static BOOL SetSS(U32 ss)
{
   if(ss){
      rt_pin_write(TF_CS_PIN, PIN_HIGH);
   }
   else
   {
      rt_pin_write(TF_CS_PIN, PIN_LOW);
   }
   
   return __TRUE;
}

static U32 CheckMedia(void)
{
	U32 stat = 0;
	if(rt_pin_read(TF_DET_PIN) == PIN_LOW){
		stat |= M_INSERTED; 
	}else{
		stat |= M_PROTECTED;
	}

   return stat;
}
