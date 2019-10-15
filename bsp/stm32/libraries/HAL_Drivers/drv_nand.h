/*
 * Change Logs:
 * Date           Author           Notes
 * 2019-09-15     guolz      first implementation
 */

#ifndef __DRV_NAND_H__
#define __DRV_NAND_H__

#include <rtdevice.h>

typedef unsigned short t_ba;    //typedef for block address
typedef unsigned char  t_po;    //typedef for page offset

enum {
	LL_OK,
	LL_ERASED,
	LL_ERROR
};
/* nandflash confg */
#define PAGE_DATA_SIZE          2048
#define NAND_MARK_SPARE_OFFSET  4
#define NAND_PAGE_SIZE          (2048)



struct stm32f4_nand
{
    rt_uint8_t id[5];
    struct rt_mutex lock;
    struct rt_completion comp;
};

rt_uint8_t rt_hw_mtd_nand_init(void);



#define NAND_RB     GET_PIN(D,6)


#define NAND_MAX_PAGE_SIZE			2048		//定义NAND FLASH的最大的PAGE大小（不包括SPARE区），默认2048字节
#define NAND_ECC_SECTOR_SIZE		512			//执行ECC计算的单元大小，默认512字节
#define NAND_PAGES_PER_BLOCK        64          //
#define NAND_MAS_SPARE_SIZE         64

//NAND FLASH操作相关延时函数
#define NAND_TADL_DELAY				70			//tADL等待延迟,最少70ns
#define NAND_TWHR_DELAY				25			//tWHR等待延迟,最少60ns
#define NAND_TRHW_DELAY				35			//tRHW等待延迟,最少100ns
#define NAND_TPROG_DELAY			1			//tPROG等待延迟,典型值200us,最大需要700us
#define NAND_TBERS_DELAY			4			//tBERS等待延迟,典型值3.5ms,最大需要10ms


//NAND属性结构体
typedef struct
{
    rt_uint16_t page_totalsize;     	//每页总大小，main区和spare区总和
    rt_uint16_t page_mainsize;      	//每页的main区大小
    rt_uint16_t page_sparesize;     	//每页的spare区大小
    rt_uint8_t  block_pagenum;      	//每个块包含的页数量
    rt_uint16_t plane_blocknum;     	//每个plane包含的块数量
    rt_uint16_t block_totalnum;     	//总的块数量
    rt_uint16_t good_blocknum;      	//好块数量    
    rt_uint16_t valid_blocknum;     	//有效块数量(供文件系统使用的好块数量)
    rt_uint32_t id;             		//NAND FLASH ID
    rt_uint16_t *lut;      			   	//LUT表，用作逻辑块-物理块转换
	rt_uint32_t ecc_hard;				//硬件计算出来的ECC值
	rt_uint32_t ecc_hdbuf[NAND_MAX_PAGE_SIZE/NAND_ECC_SECTOR_SIZE];//ECC硬件计算值缓冲区  	
	rt_uint32_t ecc_rdbuf[NAND_MAX_PAGE_SIZE/NAND_ECC_SECTOR_SIZE];//ECC读取的值缓冲区
}nand_attriute;      

extern nand_attriute nand_dev;				//nand重要参数结构体 


#define NAND_ADDRESS			0x70000000	//nand flash的访问地址,接NCE2,地址为:0X7000 0000
#define NAND_CMD				1<<16		//发送命令
#define NAND_ADDR				1<<17		//发送地址

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


//NAND FLASH型号和对应的ID号
#define W29N02GVSIAA			0xefda9095	//W29N02GVSIAA

 
rt_uint8_t NAND_ModeSet(rt_uint8_t mode);
rt_uint32_t NAND_ReadID(void);
rt_uint8_t NAND_ReadStatus(void);
rt_uint8_t NAND_WaitForReady(void);
rt_uint8_t NAND_Reset(void);
rt_uint8_t NAND_WaitRB(rt_uint8_t rb);
void NAND_Delay(rt_uint32_t i);
rt_uint8_t NAND_ReadPage(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToRead);
rt_uint8_t NAND_ReadPageComp(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint32_t CmpVal,rt_uint16_t NumByteToRead,rt_uint16_t *NumByteEqual);
rt_uint8_t NAND_WritePage(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite);
rt_uint8_t NAND_WritePageConst(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint32_t cval,rt_uint16_t NumByteToWrite);
rt_uint8_t NAND_CopyPageWithoutWrite(rt_uint32_t Source_PageNum,rt_uint32_t Dest_PageNum);
rt_uint8_t NAND_CopyPageWithWrite(rt_uint32_t Source_PageNum,rt_uint32_t Dest_PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite);
rt_uint8_t NAND_ReadSpare(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToRead);
rt_uint8_t NAND_WriteSpare(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToRead);
rt_uint8_t NAND_EraseBlock(rt_uint32_t BlockNum);
void NAND_EraseChip(void);

rt_uint16_t NAND_ECC_Get_OE(rt_uint8_t oe,rt_uint32_t eccval);
rt_uint8_t NAND_ECC_Correction(rt_uint8_t* data_buf,rt_uint32_t eccrd,rt_uint32_t ecccl);

rt_uint8_t ll_read(t_ba pba,t_ba ppo, rt_uint8_t *buffer);
rt_uint8_t ll_write(t_ba pba,t_ba ppo,rt_uint8_t *buffer);
rt_uint8_t ll_writedouble(t_ba pba,t_ba ppo, rt_uint8_t *buffer0,rt_uint8_t *buffer1);
rt_uint8_t ll_erase(t_ba pba);
rt_uint8_t ll_init();

#endif /* __K9F2G08U0B_H__ */
