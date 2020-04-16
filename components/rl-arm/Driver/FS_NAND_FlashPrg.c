/*
*********************************************************************************************************
*
*	模块名称 : FlashFS的接口文件
*	文件名称 : FS_NAND_FlashPrg.c
*	版    本 : V1.0
*	说    明 : FlashFS对NAND Flash的接口文件
*
*	修改记录 :
*		版本号  日期        作者       说明
*		V1.0    2015-09-19  Eric2013  正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include <File_Config.h>
#include "bsp.h"
#include "NAND_STM32F4xx.h"


/*-----------------------------------------------------------------------------
 *      NAND driver prototypes
 *----------------------------------------------------------------------------*/
static U32 Init         (NAND_DRV_CFG *cfg);
static U32 UnInit       (NAND_DRV_CFG *cfg);
static U32 PageRead     (U32 row, U8 *buf, NAND_DRV_CFG *cfg);
static U32 PageWrite    (U32 row, U8 *buf, NAND_DRV_CFG *cfg);
static U32 BlockErase   (U32 row, NAND_DRV_CFG *cfg);

/*-----------------------------------------------------------------------------
  NAND Device Driver Control Block
 *----------------------------------------------------------------------------*/
const NAND_DRV nand0_drv = {
  Init,
  UnInit,
  PageRead,
  PageWrite,
  BlockErase,
};


/*-----------------------------------------------------------------------------
 *      Initialise NAND flash driver
 *  *cfg = Pointer to configuration structure
 *
 *  Return: RTV_NOERR         - NAND Flash Initialisation successful
 *          ERR_NAND_HW_TOUT  - NAND Flash Reset Command failed
 *          ERR_INVALID_PARAM - Invalid parameter
 *----------------------------------------------------------------------------*/
static U32 Init (NAND_DRV_CFG *cfg) {
	
  /* Define spare area layout */
  cfg->PgLay->Pos_LSN = 0;
  cfg->PgLay->Pos_COR = 4;
  cfg->PgLay->Pos_BBM = 5;
  cfg->PgLay->Pos_ECC = 6;

  /* Define page organization */
  cfg->PgLay->SectInc  =  512;
  cfg->PgLay->SpareOfs =  2048;
  cfg->PgLay->SpareInc =  16;
	
  NAND_Init();
  return RTV_NOERR;

}

/*-----------------------------------------------------------------------------
 *      Uninitialise NAND flash driver
 *  *cfg = Pointer to configuration structure
 *
 *  Return: RTV_NOERR         - UnInit successful
 *----------------------------------------------------------------------------*/
static U32 UnInit (NAND_DRV_CFG *cfg) {
   NAND_UnInit();
   return RTV_NOERR;
}

/*-----------------------------------------------------------------------------
 *      Read page
 *  row  = Page address
 *  *buf = Pointer to data buffer
 *  *cfg = Pointer to configuration structure
 *
 *  Return: RTV_NOERR         - Page read successful
 *          ERR_NAND_HW_TOUT  - Hardware transfer timeout
 *          ERR_ECC_COR       - ECC corrected the data within page
 *          ERR_ECC_UNCOR     - ECC was not able to correct the data
 *----------------------------------------------------------------------------*/
static U32 PageRead (U32 row, U8 *buf, NAND_DRV_CFG *cfg) {
   U8 Status;
	
   Status = FSMC_NAND_ReadPage(buf, row, 0, NAND_PAGE_TOTAL_SIZE);
   return Status;
}

/*-----------------------------------------------------------------------------
 *      Write page
 *  row  = Page address
 *  *buf = Pointer to data buffer
 *  *cfg = Pointer to configuration structure
 *
 *  Return: RTV_NOERR         - Page write successful
 *          ERR_NAND_PROG     - Page write failed
 *          ERR_NAND_HW_TOUT  - Hardware transfer timeout
 *----------------------------------------------------------------------------*/
static U32 PageWrite (U32 row, U8 *buf, NAND_DRV_CFG *cfg) {
   U8 Status;

   Status = FSMC_NAND_WritePage(buf, row, 0, NAND_PAGE_TOTAL_SIZE);
   return Status;
}

/*-----------------------------------------------------------------------------
 *      Erase block
 *  row  = Block address
 *  *cfg = Pointer to configuration structure
 *
 *  Return: RTV_NOERR         - Block erase successful
 *          ERR_NAND_ERASE    - Block erase failed
 *          ERR_NAND_HW_TOUT  - Hardware transfer timeout
 *----------------------------------------------------------------------------*/
static U32 BlockErase (U32 row, NAND_DRV_CFG *cfg) {
   U32 ulPBN;
   U8 Status;
	
   ulPBN = row / cfg->NumPages;
   Status = FSMC_NAND_EraseBlock(ulPBN);
   return Status;
}

/*-----------------------------------------------------------------------------
 * end of file
 *----------------------------------------------------------------------------*/
