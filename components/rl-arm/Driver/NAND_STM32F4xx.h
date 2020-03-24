/*
*********************************************************************************************************
*
*	模块名称 : NAND Flash驱动模块
*	文件名称 : NAND_STM32F1xx.c
*	版    本 : V1.0
*	说    明 : 提供NAND Flash (HY27UF081G2A， 8bit 128K字节 大页)的底层接口函数。【安富莱原创】
*              基于bsp_nand_flash.h文件修改而来。
*
*	修改记录 :
*		版本号  日期        作者       说明
*		V1.0    2015-09-19  Eric2013  正式发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __NAND_STM32F1xx_H
#define __NAND_STM32F1xx_H

#include "File_Config.h"

#define NAND_TYPE	HY27UF081G2A	/* for 128MB */
//#define NAND_TYPE	H27U4G8F2DTR 	/* for 512MB */

/*
	定义有效的 NAND ID
	HY27UF081G2A  	= 0xAD 0xF1 0x80 0x1D
	K9F1G08U0A		= 0xEC 0xF1 0x80 0x15
	K9F1G08U0B		= 0xEC 0xF1 0x00 0x95

	H27U4G8F2DTR    = 0xAD DC 90 95
*/

/* NAND Flash 型号 */
#define HY27UF081G2A	0xADF1801D
#define K9F1G08U0A		0xECF18015
#define K9F1G08U0B		0xECF10095
#define H27U1G8F2BTR	0xADF1001D		/* STM32-V4 缺省 */

#define H27U4G8F2DTR	0xADDC9095

#define NAND_UNKNOW		0xFFFFFFFF

/* Exported constants --------------------------------------------------------*/
#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */
#define DATA_AREA                  ((uint32_t)0x00000000)

/* FSMC NAND memory command */
#define	NAND_CMD_AREA_A            ((uint8_t)0x00)
#define	NAND_CMD_AREA_B            ((uint8_t)0x01)
#define NAND_CMD_AREA_C            ((uint8_t)0x50)
#define NAND_CMD_AREA_TRUE1        ((uint8_t)0x30)

#define NAND_CMD_WRITE0            ((uint8_t)0x80)
#define NAND_CMD_WRITE_TRUE1       ((uint8_t)0x10)

#define NAND_CMD_ERASE0            ((uint8_t)0x60)
#define NAND_CMD_ERASE1            ((uint8_t)0xD0)

//#define NAND_CMD_READID            ((uint8_t)0x90)

#define NAND_CMD_LOCK_STATUS       ((uint8_t)0x7A)
//#define NAND_CMD_RESET             ((uint8_t)0xFF)

/* NAND memory status */
#define NAND_BUSY                  ((uint8_t)0x00)
#define NAND_ERROR                 ((uint8_t)0x01)
#define NAND_READY                 ((uint8_t)0x40)
#define NAND_TIMEOUT_ERROR         ((uint8_t)0x80)

/* FSMC NAND memory parameters */
/* 用于HY27UF081G2A    K9F1G08 */
#if NAND_TYPE == HY27UF081G2A
	#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 1024 block */
	#define NAND_ADDR_5					0			/* 0表示只用发送4个字节的地址，1表示5个 */

	/* 命令代码定义 */
	#define NAND_CMD_COPYBACK_A			((uint8_t)0x00)		/* PAGE COPY-BACK 命令序列 */
	#define NAND_CMD_COPYBACK_B			((uint8_t)0x35)
	#define NAND_CMD_COPYBACK_C			((uint8_t)0x85)
	#define NAND_CMD_COPYBACK_D			((uint8_t)0x10)

//	#define NAND_CMD_STATUS				((uint8_t)0x70)		/* 读NAND Flash的状态字 */

	#define MAX_PHY_BLOCKS_PER_ZONE  1024	/* 每个区最大物理块号 */
	#define MAX_LOG_BLOCKS_PER_ZONE  1000	/* 每个区最大逻辑块号 */

	#define NAND_BLOCK_COUNT			1024 /* 块个数 */
	#define NAND_PAGE_TOTAL_SIZE		(NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)	/* 页面总大小 */

#elif NAND_TYPE == H27U4G8F2DTR
	#define NAND_PAGE_SIZE             ((uint16_t)0x0800) /* 2 * 1024 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0040) /* 64 pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x1000) /* 4096 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0040) /* last 64 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 4096 block */
	#define NAND_ADDR_5					1			/* 1表示只发送4个字节的地址，1表示5个 */

	/* 命令代码定义 */
	#define NAND_CMD_COPYBACK_A			((uint8_t)0x00)		/* PAGE COPY-BACK 命令序列 */
	#define NAND_CMD_COPYBACK_B			((uint8_t)0x35)
	#define NAND_CMD_COPYBACK_C			((uint8_t)0x85)
	#define NAND_CMD_COPYBACK_D			((uint8_t)0x10)

//	#define NAND_CMD_STATUS				((uint8_t)0x70)		/* 读NAND Flash的状态字 */

	#define MAX_PHY_BLOCKS_PER_ZONE     4096	/* 每个区最大物理块号 */
	#define MAX_LOG_BLOCKS_PER_ZONE     4000	/* 每个区最大逻辑块号 */

	#define NAND_BLOCK_COUNT			4096 /* 块个数 */
	#define NAND_PAGE_TOTAL_SIZE		(NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)	/* 页面总大小 */

#else
	#define NAND_PAGE_SIZE             ((uint16_t)0x0200) /* 512 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0020) /* 32x512 bytes pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x0400) /* 1024 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x0010) /* last 16 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0004) /* 4 zones of 1024 block */
#endif

#define LSN0_OFFSET				0		/* 块内第1个page备用区的第0个字节 */
#define LSN1_OFFSET				1		/* 块内第1个page备用区的第1个字节 */
#define LSN2_OFFSET				2		/* 块内第1个page备用区的第2个字节 */
#define LSN3_OFFSET				3		/* 块内第1个page备用区的第3个字节 */
#define COR_OFFSET				4		/* 块内第1个page备用区的第3个字节 */
#define BBM_OFFSET				5		/* 块内第1个page备用区的第3个字节 */ 
#define ECC_OFFSET				6		/* 块内第1个page备用区的第3个字节 */  

#define NAND_OK  				0
#define NAND_FAIL 				1
#define NAND_TIMEOUT 			2

void NAND_Init(void);
void NAND_UnInit(void);
uint8_t NAND_Format(void);
uint32_t NAND_ReadID(void);
uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount);
uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount);
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo);

#endif /* __FSMC_NAND_H */

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
