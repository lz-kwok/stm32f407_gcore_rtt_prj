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
#include <File_Config.h>
#include "drv_nand.h"


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
static void CheckMedia   (void);
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
	

	
  return __TRUE;
}


static BOOL UnInit (void) {


   return __TRUE;
}


static U8 Send(U8 outb){
   U8 Status;
	

   return Status;
}


static BOOL SendBuf(U8 *buf, U32 sz)
{
   U8 Status;

   return Status;
}

static BOOL RecBuf(U8 *buf, U32 sz)
{
   U8 Status;

   return Status;
}

static BOOL BusSpeed(U32 kbaud)
{
   return __TRUE;
}

static BOOL SetSS(U32 ss)
{
   return __TRUE;
}

static void CheckMedia(void)
{
   
}
