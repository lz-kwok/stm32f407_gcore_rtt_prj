/*
 * Change Logs:
 * Date           Author           Notes
 * 2019-09-15     guolz      first implementation
 */

#ifndef __DRV_NAND_H__
#define __DRV_NAND_H__

#include <rtdevice.h>

/* nandflash confg */
#define PAGES_PER_BLOCK         64
#define PAGE_DATA_SIZE          2048
#define PAGE_OOB_SIZE           64
#define NAND_MARK_SPARE_OFFSET  4

#define NAND_PAGE_SIZE          (2048)
#define NAND_PAGE_TOTAL_SIZE    PAGE_DATA_SIZE+PAGE_OOB_SIZE

#define NAND_RB     GET_PIN(D,6)


#define NAND_MAX_PAGE_SIZE			2048		//定义NAND FLASH的最大的PAGE大小（不包括SPARE区），默认2048字节
#define NAND_ECC_SECTOR_SIZE		512			//执行ECC计算的单元大小，默认512字节


//NAND FLASH操作相关延时函数
#define NAND_TADL_DELAY				70			//tADL等待延迟,最少70ns
#define NAND_TWHR_DELAY				25			//tWHR等待延迟,最少60ns
#define NAND_TRHW_DELAY				35			//tRHW等待延迟,最少100ns
#define NAND_TPROG_DELAY			1			//tPROG等待延迟,典型值200us,最大需要700us
#define NAND_TBERS_DELAY			4			//tBERS等待延迟,典型值3.5ms,最大需要10ms



//#define CMD_AREA                   (rt_uint32_t)(1<<16)  /* A16 = CLE  high */
//#define ADDR_AREA                  (rt_uint32_t)(1<<17)  /* A17 = ALE high */
#define DATA_AREA                  ((rt_uint32_t)0x00000000)


//NAND FLASH命令
#define NAND_READID         	0X90    	//读ID指令
#define NAND_FEATURE			0XEF    	//设置特性指令
#define NAND_RESET          	0XFF    	//复位NAND
#define NAND_READSTA        	0X70   	 	//读状态
#define NAND_AREA_A         	0X00   
#define NAND_AREA_TRUE1     	0X30  
#define NAND_WRITE0        	 	0X80
#define NAND_WRITE_TURE1    	0X10
#define NAND_ERASE0        	 	0X60
#define NAND_ERASE1         	0XD0
#define NAND_MOVEDATA_CMD0  	0X00
#define NAND_MOVEDATA_CMD1  	0X35
#define NAND_MOVEDATA_CMD2  	0X85
#define NAND_MOVEDATA_CMD3  	0X10

//NAND FLASH状态
#define NSTA_READY       	   	0X40		//nand已经准备好
#define NSTA_ERROR				0X01		//nand错误
#define NSTA_TIMEOUT        	0X02		//超时
#define NSTA_ECC1BITERR       	0X03		//ECC 1bit错误
#define NSTA_ECC2BITERR       	0X04		//ECC 2bit以上错误

//#define NAND_BUSY                  ((rt_uint8_t)0x00)
//#define NAND_ERROR                 ((rt_uint8_t)0x01)
//#define NAND_READY                 ((rt_uint8_t)0x40)
//#define NAND_TIMEOUT_ERROR         ((rt_uint8_t)0x80)

//NAND FLASH型号和对应的ID号
#define W29N02GVSIAA			0xefda9095	//W29N02GVSIAA
void ViewNandCapacity(void);
rt_uint8_t rt_hw_mtd_nand_init(void);
void rt_hw_mtd_nand_deinit(void);
rt_uint8_t NAND_EraseBlock(rt_uint32_t _ulBlockNo);
void NAND_EraseChip(void);
rt_uint8_t FSMC_NAND_ReadPage(rt_uint8_t *_pBuffer, rt_uint32_t _ulPageNo, rt_uint16_t _usAddrInPage, rt_uint16_t NumByteToRead);
rt_uint8_t FSMC_NAND_WritePage(rt_uint8_t *_pBuffer, rt_uint32_t _ulPageNo, rt_uint16_t _usAddrInPage, rt_uint16_t NumByteToRead);
#endif /* __K9F2G08U0B_H__ */
