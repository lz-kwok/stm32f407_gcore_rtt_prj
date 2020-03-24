/*
 * Change Logs:
 * Date           Author           Notes
 * 2019-09-15     guolz      first implementation
 */
#include <rtdevice.h>
#include "board.h"
#include "drv_nand.h"


#define NAND_DEBUG    rt_kprintf

#define DMA_CHANNEL     DMA_Channel_0
#define DMA_STREAM      DMA2_Stream0
#define DMA_TCIF        DMA_FLAG_TCIF0
#define DMA_IRQN        DMA2_Stream0_IRQn
#define DMA_IRQ_HANDLER DMA2_Stream0_IRQHandler
#define DMA_CLK         RCC_AHB1Periph_DMA2

#define NAND_BANK     ((rt_uint32_t)0x70000000)
#define ECC_SIZE     4

static struct stm32f4_nand _device;
static struct rt_mtd_nand_device _partition[2];

NAND_HandleTypeDef NAND_Handler;    //NAND FLASH句柄
nand_attriute nand_dev;             //nand重要参数结构体
static rt_err_t nandflash_readid(struct rt_mtd_nand_device *mtd)
{
    NAND_IDTypeDef nand_id;

    HAL_NAND_Read_ID(&NAND_Handler,&nand_id);

    NAND_DEBUG("ID[%X,%X,%X,%X]\n",nand_id.Maker_Id,nand_id.Device_Id,nand_id.Third_Id,nand_id.Fourth_Id);
    if (nand_id.Maker_Id == 0xEF && nand_id.Device_Id == 0xDA)
    {
        return (RT_EOK);
    }

    return (RT_ERROR);
}

static rt_err_t nandflash_readpage(struct rt_mtd_nand_device* device, rt_off_t page,
                                   rt_uint8_t *data, rt_uint32_t data_len,
                                   rt_uint8_t *spare, rt_uint32_t spare_len)
{
    rt_err_t result;
    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);
    
    result = NAND_ReadPage(page,0,data,data_len);
    if(result == 0){
        if (spare && spare_len){
            NAND_ReadSpare(page,0,spare,spare_len);
        }
    }
 
    rt_mutex_release(&_device.lock);

    return (result);
}

static rt_err_t nandflash_writepage(struct rt_mtd_nand_device* device, rt_off_t page,
                                    const rt_uint8_t *data, rt_uint32_t data_len,
                                    const rt_uint8_t *spare, rt_uint32_t spare_len)
{
    rt_err_t result;
    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);
    
    result = NAND_WritePage(page,0,(rt_uint8_t *)data,data_len);
    if(result == 0){
        if (spare && spare_len){
            NAND_WriteSpare(page,0,(rt_uint8_t *)spare,spare_len);
        }
    }
 
    rt_mutex_release(&_device.lock);

    return (result);
}

rt_err_t nandflash_eraseblock(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    rt_err_t result;

    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);
    result = NAND_EraseBlock(block);
    rt_mutex_release(&_device.lock);

    return (result);
}

static rt_err_t nandflash_pagecopy(struct rt_mtd_nand_device *device, rt_off_t src_page, rt_off_t dst_page)
{
    rt_err_t result;

    rt_mutex_take(&_device.lock, RT_WAITING_FOREVER);
    result = NAND_CopyPageWithoutWrite(src_page,dst_page);
    rt_mutex_release(&_device.lock);

    return (result);
}

static rt_err_t nandflash_checkblock(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    return (RT_MTD_EOK);
}

static rt_err_t nandflash_markbad(struct rt_mtd_nand_device* device, rt_uint32_t block)
{
    return (RT_MTD_EOK);
}

static struct rt_mtd_nand_driver_ops ops =
{
    nandflash_readid,
    nandflash_readpage,
    nandflash_writepage,
    nandflash_pagecopy,
    nandflash_eraseblock,
#if defined(RT_USING_DFS_UFFS) && !defined(RT_UFFS_USE_CHECK_MARK_FUNCITON)
    RT_NULL,
    RT_NULL,
#else
    nandflash_checkblock,
    nandflash_markbad
#endif
};

void rt_hw_mtd_nand_deinit(void)
{
    HAL_NAND_DeInit(&NAND_Handler);
}


//初始化NAND FLASH
rt_uint8_t rt_hw_mtd_nand_init(void)
{
    rt_kprintf("%s\r\n",__func__);
    FMC_NAND_PCC_TimingTypeDef ComSpaceTiming,AttSpaceTiming;
                                              
    NAND_Handler.Instance=FMC_NAND_DEVICE;
    NAND_Handler.Init.NandBank=FSMC_NAND_BANK2;                             //NAND挂在BANK2上
    NAND_Handler.Init.Waitfeature=FSMC_NAND_PCC_WAIT_FEATURE_DISABLE;        //关闭等待特性
    NAND_Handler.Init.MemoryDataWidth=FSMC_NAND_PCC_MEM_BUS_WIDTH_8;        //8位数据宽度
    NAND_Handler.Init.EccComputation=FSMC_NAND_ECC_DISABLE;                 //不使用ECC
    NAND_Handler.Init.ECCPageSize=FSMC_NAND_ECC_PAGE_SIZE_2048BYTE;         //ECC页大小为2k
    NAND_Handler.Init.TCLRSetupTime=0;                                      //设置TCLR(tCLR=CLE到RE的延时)=(TCLR+TSET+2)*THCLK,THCLK=1/180M=5.5ns
    NAND_Handler.Init.TARSetupTime=1;                                       //设置TAR(tAR=ALE到RE的延时)=(TAR+TSET+2)*THCLK,THCLK=1/180M=5.5n。   
   
    ComSpaceTiming.SetupTime=2;         //建立时间
    ComSpaceTiming.WaitSetupTime=5;     //等待时间
    ComSpaceTiming.HoldSetupTime=3;     //保持时间
    ComSpaceTiming.HiZSetupTime=1;      //高阻态时间
    
    AttSpaceTiming.SetupTime=2;         //建立时间
    AttSpaceTiming.WaitSetupTime=5;     //等待时间
    AttSpaceTiming.HoldSetupTime=3;     //保持时间
    AttSpaceTiming.HiZSetupTime=1;      //高阻态时间
    
    HAL_NAND_Init(&NAND_Handler,&ComSpaceTiming,&AttSpaceTiming); 
    NAND_Reset();       		        //复位NAND
    rt_thread_mdelay(100);
    nand_dev.id = NAND_ReadID();	    //读取ID
	NAND_ModeSet(4);			        //设置为MODE4,高速模式 

    if(nand_dev.id==W29N02GVSIAA){
        nand_dev.page_totalsize = 2112;  	//nand一个page的总大小（包括spare区）     
        nand_dev.page_mainsize  = 2048;   	//nand一个page的有效数据区大小    
        nand_dev.page_sparesize = 64;	    //nand一个page的spare区大小
        nand_dev.block_pagenum  = 64;		//nand一个block所包含的page数目
        nand_dev.plane_blocknum = 1024;	    //nand一个plane所包含的block数目
        nand_dev.block_totalnum = 2048;  	//nand的总block数目  

        rt_mutex_init(&_device.lock, "nand", RT_IPC_FLAG_FIFO);

        _partition[0].page_size   = 2048;
        _partition[0].pages_per_block = 64;
        _partition[0].block_total = 2048;
        _partition[0].oob_size    = 64;
        _partition[0].oob_free    = 60;
        _partition[0].block_start = 0;
        _partition[0].block_end   = 2047;
        _partition[0].ops         = &ops;

        rt_mtd_nand_register_device("nandflash", &_partition[0]);
    }else{
        return 1;	//错误，返回
    }
    return 0;
}
// INIT_DEVICE_EXPORT(rt_hw_mtd_nand_init);

//读取NAND FLASH的ID
//返回值:0,成功;
//    其他,失败
rt_uint8_t NAND_ModeSet(rt_uint8_t mode)
{   
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_FEATURE;//发送设置特性命令
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=0X01;		//地址为0X01,设置mode
 	*(rt_uint8_t*)NAND_ADDRESS=mode;					//P1参数,设置mode
	*(rt_uint8_t*)NAND_ADDRESS=0;
	*(rt_uint8_t*)NAND_ADDRESS=0;
	*(rt_uint8_t*)NAND_ADDRESS=0; 

    if(NAND_WaitForReady()==NSTA_READY)
        return 0;               //成功
    else 
        return 1;				//失败
}

//读取NAND FLASH的ID
//不同的NAND略有不同，请根据自己所使用的NAND FALSH数据手册来编写函数
//返回值:NAND FLASH的ID值
rt_uint32_t NAND_ReadID(void)
{
    rt_uint8_t deviceid[5]; 
    rt_uint32_t id;  
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_READID; //发送读取ID命令
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=0X00;
	//ID一共有5个字节
    deviceid[0]=*(rt_uint8_t*)NAND_ADDRESS;      
    deviceid[1]=*(rt_uint8_t*)NAND_ADDRESS;  
    deviceid[2]=*(rt_uint8_t*)NAND_ADDRESS; 
    deviceid[3]=*(rt_uint8_t*)NAND_ADDRESS; 
    deviceid[4]=*(rt_uint8_t*)NAND_ADDRESS;  

    id=((rt_uint32_t)deviceid[0])<<24|((rt_uint32_t)deviceid[1])<<16|((rt_uint32_t)deviceid[2])<<8|deviceid[3];
    NAND_DEBUG("NAND_ReadID = %08x\r\n",id);

    return id;
}  
//读NAND状态
//返回值:NAND状态值
//bit0:0,成功;1,错误(编程/擦除/READ)
//bit6:0,Busy;1,Ready
rt_uint8_t NAND_ReadStatus(void)
{
    rt_uint8_t data=0; 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_READSTA;//发送读状态命令
    NAND_Delay(NAND_TWHR_DELAY);	            //等待tWHR,再读取状态寄存器
 	data=*(rt_uint8_t*)NAND_ADDRESS;			//读取状态值

    return data;
}
//等待NAND准备好
//返回值:NSTA_TIMEOUT 等待超时了
//      NSTA_READY    已经准备好
rt_uint8_t NAND_WaitForReady(void)
{
    rt_uint8_t status=0;
    rt_uint32_t time=0; 
	while(1)						//等待ready
	{
		status = NAND_ReadStatus();	//获取状态值
		if(status&NSTA_READY)break;
		time++;
		if(time>=0X1FFFFFFF)return NSTA_TIMEOUT;//超时
	}  

    return NSTA_READY;//准备好
}  
//复位NAND
//返回值:0,成功;
//    其他,失败
rt_uint8_t NAND_Reset(void)
{ 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD) = NAND_RESET;	//复位NAND
    if(NAND_WaitForReady()==NSTA_READY)
        return 0;           //复位成功
    else 
        return 1;			//复位失败
} 
//等待RB信号为某个电平
//rb:0,等待RB==0
//   1,等待RB==1
//返回值:0,成功
//       1,超时
rt_uint8_t NAND_WaitRB(rt_uint8_t rb)
{
    rt_uint32_t time=0;  
	while(time<0X1FFFFFF)
	{
		time++;
		if(rt_pin_read(NAND_RB)==rb)return 0;
	}
	return 1;
}
//NAND延时
//一个i++至少需要4ns
void NAND_Delay(rt_uint32_t i)
{
	while(i>0)i--;
}
//读取NAND Flash的指定页指定列的数据(main区和spare区都可以使用此函数)
//PageNum:要读取的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要读取的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//*pBuffer:指向数据存储区
//NumByteToRead:读取字节数(不能跨页读)
//返回值:0,成功 
//    其他,错误代码
rt_uint8_t NAND_ReadPage(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToRead)
{
    rt_uint16_t i=0;
	rt_uint8_t res=0;
	rt_uint8_t eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	rt_uint8_t eccstart=0;		//第一个ECC值所属的地址范围
	rt_uint8_t errsta=0;
	rt_uint8_t *p;
     *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)ColNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(ColNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;

	res=NAND_WaitRB(0);			//等待RB=0 
    if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出
	if(NumByteToRead%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{ 
		//读取NAND FLASH中的值
		for(i=0;i<NumByteToRead;i++)
		{
			*(rt_uint8_t*)pBuffer++ = *(rt_uint8_t*)NAND_ADDRESS;
		}
	}else
	{
		eccnum=NumByteToRead/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE;
		p=pBuffer;
		for(res=0;res<eccnum;res++)
		{
            NAND_Handler.Instance->PCR2|=1<<6;              //使能ECC校验      
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//读取NAND_ECC_SECTOR_SIZE个数据
			{
				*(rt_uint8_t*)pBuffer++ = *(rt_uint8_t*)NAND_ADDRESS;
			}		
			while(!(NAND_Handler.Instance->SR2&(1<<6)));				    //等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=NAND_Handler.Instance->ECCR2;//读取硬件计算后的ECC值
			NAND_Handler.Instance->PCR2&=~(1<<6);						//禁止ECC校验
		} 
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//从spare区的0X10位置开始读取之前存储的ecc值
		NAND_Delay(NAND_TRHW_DELAY);//等待tRHW 
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=0x05;				//随机读指令
		//发送地址
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)i;
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(i>>8);
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=0xE0;				//开始读数据
		NAND_Delay(NAND_TWHR_DELAY);//等待tWHR 
		pBuffer=(rt_uint8_t*)&nand_dev.ecc_rdbuf[eccstart];
		for(i=0;i<4*eccnum;i++)								//读取保存的ECC值
		{
			*(rt_uint8_t*)pBuffer++= *(rt_uint8_t*)NAND_ADDRESS;
		}			
		for(i=0;i<eccnum;i++)								//检验ECC
		{
			if(nand_dev.ecc_rdbuf[i+eccstart]!=nand_dev.ecc_hdbuf[i+eccstart])//不相等,需要校正
			{
				NAND_DEBUG("err hd,rd:0x%x,0x%x\r\n",nand_dev.ecc_hdbuf[i+eccstart],nand_dev.ecc_rdbuf[i+eccstart]); 
 				NAND_DEBUG("eccnum,eccstart:%d,%d\r\n",eccnum,eccstart);	
				NAND_DEBUG("PageNum,ColNum:%d,%d\r\n",PageNum,ColNum);	
				res=NAND_ECC_Correction(p+NAND_ECC_SECTOR_SIZE*i,nand_dev.ecc_rdbuf[i+eccstart],nand_dev.ecc_hdbuf[i+eccstart]);//ECC校验
				if(res)errsta=NSTA_ECC2BITERR;				//标记2BIT及以上ECC错误
				else errsta=NSTA_ECC1BITERR;				//标记1BIT ECC错误
			} 
		} 		
	}
    if(NAND_WaitForReady()!=NSTA_READY)errsta=NSTA_ERROR;	//失败
    return errsta;	//成功   
} 


rt_uint8_t FSMC_NAND_ReadPage(rt_uint8_t *_pBuffer, rt_uint32_t _ulPageNo, rt_uint16_t _usAddrInPage, rt_uint16_t NumByteToRead)
{
	rt_uint32_t i;
    rt_uint8_t res=0;

     *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)_usAddrInPage;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_usAddrInPage>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)_ulPageNo;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_ulPageNo>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_ulPageNo>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;

	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	for (i = 0; i < 20; i++);
//    res=NAND_WaitRB(0);			//等待RB=0 
//    if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出


	/* 读数据到缓冲区pBuffer */
	for(i = 0; i < NumByteToRead; i++)
	{
		_pBuffer[i] = NAND_ADDRESS;
	}

	return RT_EOK;
}


//读取NAND Flash的指定页指定列的数据(main区和spare区都可以使用此函数),并对比(FTL管理时需要)
//PageNum:要读取的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要读取的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//CmpVal:要对比的值,以rt_uint32_t为单位
//NumByteToRead:读取字数(以4字节为单位,不能跨页读)
//NumByteEqual:从初始位置持续与CmpVal值相同的数据个数
//返回值:0,成功
//    其他,错误代码
rt_uint8_t NAND_ReadPageComp(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint32_t CmpVal,rt_uint16_t NumByteToRead,rt_uint16_t *NumByteEqual)
{
    rt_uint16_t i=0;
	rt_uint8_t res=0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)ColNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(ColNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;

	res=NAND_WaitRB(0);			//等待RB=0 
	if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出  
    for(i=0;i<NumByteToRead;i++)//读取数据,每次读4字节
    {
		if(*(rt_uint32_t*)NAND_ADDRESS!=CmpVal)break;	//如果有任何一个值,与CmpVal不相等,则退出.
    }
	*NumByteEqual=i;					//与CmpVal值相同的个数
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
    return 0;	//成功   
} 
//在NAND一页中写入指定个字节的数据(main区和spare区都可以使用此函数)
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//pBbuffer:指向数据存储区
//NumByteToWrite:要写入的字节数，该值不能超过该页剩余字节数！！！
//返回值:0,成功 
//    其他,错误代码
rt_uint8_t NAND_WritePage(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite)
{
    rt_uint16_t i=0;  
	rt_uint8_t res=0;
	rt_uint8_t eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	rt_uint8_t eccstart=0;		//第一个ECC值所属的地址范围
	
	*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)ColNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(ColNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>16);
	NAND_Delay(NAND_TADL_DELAY);//等待tADL
    // NAND_DEBUG("Write PageNum %d ColNum %d   %d data:\r\n",PageNum,ColNum,NumByteToWrite); 

	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{  
		for(i=0;i<NumByteToWrite;i++)		//写入数据
		{
            // NAND_DEBUG("%02x ",*(rt_uint8_t*)pBuffer); 
			*(rt_uint8_t*)NAND_ADDRESS = *(rt_uint8_t*)pBuffer++;
            // if((i%8) == 0)
            //     NAND_DEBUG("\r\n");
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
            NAND_Handler.Instance->PCR2|=1<<6;              //使能ECC校验      
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//读取NAND_ECC_SECTOR_SIZE个数据
			{
				*(rt_uint8_t*)NAND_ADDRESS = *(rt_uint8_t*)pBuffer++;
			}		
			while(!(NAND_Handler.Instance->SR2&(1<<6)));				    //等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=NAND_Handler.Instance->ECCR2;  //读取硬件计算后的ECC值
			NAND_Handler.Instance->PCR2&=~(1<<6);	
		}  
		i=nand_dev.page_mainsize+0x10+eccstart*4;			//计算写入ECC的spare区地址
		NAND_Delay(NAND_TADL_DELAY);//等待tADL 
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=0x85;				//随机写指令
		//发送地址
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)i;
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(i>>8);
		NAND_Delay(NAND_TADL_DELAY);//等待tADL 
		pBuffer=(rt_uint8_t*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//写入ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(rt_uint8_t*)NAND_ADDRESS=*(rt_uint8_t*)pBuffer++;
			}
		} 		
	}
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
 	rt_thread_mdelay(NAND_TPROG_DELAY);	//等待tPROG
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
    return 0;//成功   
}

rt_uint8_t FSMC_NAND_WritePage(rt_uint8_t *_pBuffer, rt_uint32_t _ulPageNo, rt_uint16_t _usAddrInPage, rt_uint16_t NumByteToRead)
{
	rt_uint32_t i;

	*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)_usAddrInPage;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_usAddrInPage>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)_ulPageNo;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_ulPageNo>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(_ulPageNo>>16);

	rt_thread_mdelay(NAND_TPROG_DELAY);	//等待tPROG
	for(i = 0; i < NumByteToRead; i++)
	{
        *(rt_uint8_t*)NAND_ADDRESS = *(rt_uint8_t*)_pBuffer++;
	}

	*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 

	rt_thread_mdelay(NAND_TPROG_DELAY);	//等待tPROG
	
    if(NAND_WaitForReady() != NSTA_READY)return RT_ERROR;   //失败
	
	return RT_EOK;
}











//在NAND一页中的指定地址开始,写入指定长度的恒定数字
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的列开始地址(也就是页内地址),范围:0~(page_totalsize-1)
//cval:要写入的指定常数
//NumByteToWrite:要写入的字数(以4字节为单位)
//返回值:0,成功 
//    其他,错误代码
rt_uint8_t NAND_WritePageConst(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint32_t cval,rt_uint16_t NumByteToWrite)
{
    rt_uint16_t i=0;  
	*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //发送地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)ColNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(ColNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(PageNum>>16);
	NAND_Delay(NAND_TADL_DELAY);//等待tADL 
	for(i=0;i<NumByteToWrite;i++)		//写入数据,每次写4字节
	{
		*(rt_uint32_t*)NAND_ADDRESS=cval;
	} 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
 	rt_thread_mdelay(NAND_TPROG_DELAY);	//等待tPROG
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
    return 0;//成功   
}
//将一页数据拷贝到另一页,不写入新数据
//注意:源页和目的页要在同一个Plane内！
//Source_PageNo:源页地址,范围:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:目的页地址,范围:0~(block_pagenum*block_totalnum-1)  
//返回值:0,成功
//    其他,错误代码
rt_uint8_t NAND_CopyPageWithoutWrite(rt_uint32_t Source_PageNum,rt_uint32_t Dest_PageNum)
{
	rt_uint8_t res=0;
    rt_uint16_t source_block=0,dest_block=0;  
    //判断源页和目的页是否在同一个plane中
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;	//不在同一个plane内 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;	//发送命令0X00
    //发送源页地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)Source_PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Source_PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Source_PageNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;//发送命令0X35 
    //下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
    //将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
    //就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
    //闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
    //代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
	res=NAND_WaitRB(0);			//等待RB=0 
	if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //发送命令0X85
    //发送目的页地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)Dest_PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Dest_PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Dest_PageNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//发送命令0X10 
	rt_thread_mdelay(NAND_TPROG_DELAY);	//等待tPROG
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//NAND未准备好 
    return 0;//成功   
}

//将一页数据拷贝到另一页,并且可以写入数据
//注意:源页和目的页要在同一个Plane内！
//Source_PageNo:源页地址,范围:0~(block_pagenum*block_totalnum-1)
//Dest_PageNo:目的页地址,范围:0~(block_pagenum*block_totalnum-1)  
//ColNo:页内列地址,范围:0~(page_totalsize-1)
//pBuffer:要写入的数据
//NumByteToWrite:要写入的数据个数
//返回值:0,成功 
//    其他,错误代码
rt_uint8_t NAND_CopyPageWithWrite(rt_uint32_t Source_PageNum,rt_uint32_t Dest_PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite)
{
	rt_uint8_t res=0;
    rt_uint16_t i=0;
	rt_uint16_t source_block=0,dest_block=0;  
	rt_uint8_t eccnum=0;		//需要计算的ECC个数，每NAND_ECC_SECTOR_SIZE字节计算一个ecc
	rt_uint8_t eccstart=0;		//第一个ECC值所属的地址范围
    //判断源页和目的页是否在同一个plane中
    source_block=Source_PageNum/nand_dev.block_pagenum;
    dest_block=Dest_PageNum/nand_dev.block_pagenum;
    if((source_block%2)!=(dest_block%2))return NSTA_ERROR;//不在同一个plane内
	*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD0;  //发送命令0X00
    //发送源页地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)0;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)Source_PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Source_PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Source_PageNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD1;  //发送命令0X35
    
    //下面两行代码是等待R/B引脚变为低电平，其实主要起延时作用的，等待NAND操作R/B引脚。因为我们是通过
    //将STM32的NWAIT引脚(NAND的R/B引脚)配置为普通IO，代码中通过读取NWAIT引脚的电平来判断NAND是否准备
    //就绪的。这个也就是模拟的方法，所以在速度很快的时候有可能NAND还没来得及操作R/B引脚来表示NAND的忙
    //闲状态，结果我们就读取了R/B引脚,这个时候肯定会出错的，事实上确实是会出错!大家也可以将下面两行
    //代码换成延时函数,只不过这里我们为了效率所以没有用延时函数。
	res=NAND_WaitRB(0);			//等待RB=0 
	if(res)return NSTA_TIMEOUT;	//超时退出
    //下面2行代码是真正判断NAND是否准备好的
	res=NAND_WaitRB(1);			//等待RB=1 
    if(res)return NSTA_TIMEOUT;	//超时退出 
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD2;  //发送命令0X85
    //发送目的页地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)ColNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(ColNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)Dest_PageNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Dest_PageNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(Dest_PageNum>>16); 
    //发送页内列地址
	NAND_Delay(NAND_TADL_DELAY);//等待tADL 
	if(NumByteToWrite%NAND_ECC_SECTOR_SIZE)//不是NAND_ECC_SECTOR_SIZE的整数倍，不进行ECC校验
	{  
		for(i=0;i<NumByteToWrite;i++)		//写入数据
		{
			*(rt_uint8_t*)NAND_ADDRESS=*(rt_uint8_t*)pBuffer++;
		}
	}else
	{
		eccnum=NumByteToWrite/NAND_ECC_SECTOR_SIZE;			//得到ecc计算次数
		eccstart=ColNum/NAND_ECC_SECTOR_SIZE; 
 		for(res=0;res<eccnum;res++)
		{
            NAND_Handler.Instance->PCR2|=1<<6;              //使能ECC校验      
			for(i=0;i<NAND_ECC_SECTOR_SIZE;i++)				//读取NAND_ECC_SECTOR_SIZE个数据
			{
				*(rt_uint8_t*)pBuffer++ = *(rt_uint8_t*)NAND_ADDRESS;
			}		
			while(!(NAND_Handler.Instance->SR2&(1<<6)));				    //等待FIFO空	
			nand_dev.ecc_hdbuf[res+eccstart]=NAND_Handler.Instance->ECCR2;//读取硬件计算后的ECC值
			NAND_Handler.Instance->PCR2&=~(1<<6);	
		}  
		i=nand_dev.page_mainsize+0X10+eccstart*4;			//计算写入ECC的spare区地址
		NAND_Delay(NAND_TADL_DELAY);//等待tADL 
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=0X85;				//随机写指令
		//发送地址
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)i;
		*(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(i>>8);
		NAND_Delay(NAND_TADL_DELAY);//等待tADL 
		pBuffer=(rt_uint8_t*)&nand_dev.ecc_hdbuf[eccstart];
		for(i=0;i<eccnum;i++)					//写入ECC
		{ 
			for(res=0;res<4;res++)				 
			{
				*(rt_uint8_t*)NAND_ADDRESS=*(rt_uint8_t*)pBuffer++;
			}
		} 		
	}
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_MOVEDATA_CMD3;	//发送命令0X10 
 	rt_thread_mdelay(NAND_TPROG_DELAY);							//等待tPROG
    if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;	//失败
    return 0;	//成功   
} 
//读取spare区中的数据
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的spare区地址(spare区中哪个地址),范围:0~(page_sparesize-1) 
//pBuffer:接收数据缓冲区 
//NumByteToRead:要读取的字节数(不大于page_sparesize)
//返回值:0,成功
//    其他,错误代码
rt_uint8_t NAND_ReadSpare(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToRead)
{
    rt_uint8_t temp=0;
    rt_uint8_t remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToRead>remainbyte) NumByteToRead=remainbyte;  //确保要写入的字节数不大于spare剩余的大小
    temp=NAND_ReadPage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToRead);//读取数据
    return temp;
} 
//向spare区中写数据
//PageNum:要写入的页地址,范围:0~(block_pagenum*block_totalnum-1)
//ColNum:要写入的spare区地址(spare区中哪个地址),范围:0~(page_sparesize-1)  
//pBuffer:要写入的数据首地址 
//NumByteToWrite:要写入的字节数(不大于page_sparesize)
//返回值:0,成功
//    其他,失败
rt_uint8_t NAND_WriteSpare(rt_uint32_t PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint16_t NumByteToWrite)
{
    rt_uint8_t temp=0;
    rt_uint8_t remainbyte=0;
    remainbyte=nand_dev.page_sparesize-ColNum;
    if(NumByteToWrite>remainbyte) NumByteToWrite=remainbyte;  //确保要读取的字节数不大于spare剩余的大小
    temp=NAND_WritePage(PageNum,ColNum+nand_dev.page_mainsize,pBuffer,NumByteToWrite);//读取
    return temp;
} 
//擦除一个块
//BlockNum:要擦除的BLOCK编号,范围:0-(block_totalnum-1)
//返回值:0,擦除成功
//    其他,擦除失败
rt_uint8_t NAND_EraseBlock(rt_uint32_t BlockNum)
{
    if(nand_dev.id==W29N02GVSIAA)BlockNum<<=6;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE0;
    //发送块地址
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)BlockNum;
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(BlockNum>>8);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_ADDR)=(rt_uint8_t)(BlockNum>>16);
    *(rt_uint8_t*)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE1;
	rt_thread_mdelay(NAND_TBERS_DELAY);		//等待擦除成功
	if(NAND_WaitForReady()!=NSTA_READY)return NSTA_ERROR;//失败
    return 0;	//成功   
} 
//全片擦除NAND FLASH
void NAND_EraseChip(void)
{
    rt_uint8_t status;
    rt_uint16_t i=0;
    for(i=0;i<nand_dev.block_totalnum;i++) //循环擦除所有的块
    {
        status=NAND_EraseBlock(i);
        if(status)
            NAND_DEBUG("Erase %d block fail!!，ERRORCODE %d\r\n",i,status);//擦除失败
    }
}

//获取ECC的奇数位/偶数位
//oe:0,偶数位
//   1,奇数位
//eccval:输入的ecc值
//返回值:计算后的ecc值(最多16位)
rt_uint16_t NAND_ECC_Get_OE(rt_uint8_t oe,rt_uint32_t eccval)
{
	rt_uint8_t i;
	rt_uint16_t ecctemp=0;
	for(i=0;i<24;i++)
	{
		if((i%2)==oe)
		{
			if((eccval>>i)&0x01)ecctemp+=1<<(i>>1); 
		}
	}
	return ecctemp;
} 
//ECC校正函数
//eccrd:读取出来,原来保存的ECC值
//ecccl:读取数据时,硬件计算的ECC只
//返回值:0,错误已修正
//    其他,ECC错误(有大于2个bit的错误,无法恢复)
rt_uint8_t NAND_ECC_Correction(rt_uint8_t* data_buf,rt_uint32_t eccrd,rt_uint32_t ecccl)
{
	rt_uint16_t eccrdo,eccrde,eccclo,ecccle;
	rt_uint16_t eccchk=0;
	rt_uint16_t errorpos=0; 
	rt_uint32_t bytepos=0;  
	eccrdo=NAND_ECC_Get_OE(1,eccrd);	//获取eccrd的奇数位
	eccrde=NAND_ECC_Get_OE(0,eccrd);	//获取eccrd的偶数位
	eccclo=NAND_ECC_Get_OE(1,ecccl);	//获取ecccl的奇数位
	ecccle=NAND_ECC_Get_OE(0,ecccl); 	//获取ecccl的偶数位
	eccchk=eccrdo^eccrde^eccclo^ecccle;
	if(eccchk==0xFFF)	//全1,说明只有1bit ECC错误
	{
		errorpos=eccrdo^eccclo; 
		NAND_DEBUG("errorpos:%d\r\n",errorpos); 
		bytepos=errorpos/8; 
		data_buf[bytepos]^=1<<(errorpos%8);
	}else				//不是全1,说明至少有2bit ECC错误,无法修复
	{
		NAND_DEBUG("2bit ecc error or more\r\n");
		return 1;
	} 
	return 0;
}
 

#include <finsh.h>
void nread(rt_uint32_t partion, rt_uint32_t page)
{
    int i;
    rt_uint8_t spare[64];
    rt_uint8_t *data_ptr;
    struct rt_mtd_nand_device *device;

    if (partion >= 3)
        return;
    device = &_partition[partion];
    data_ptr = (rt_uint8_t*) rt_malloc(PAGE_DATA_SIZE);
    if (data_ptr == RT_NULL)
    {
        rt_kprintf("no memory\n");
        return;
    }

    rt_memset(spare, 0, sizeof(spare));
    rt_memset(data_ptr, 0, PAGE_DATA_SIZE);
    nandflash_readpage(device, page, data_ptr, PAGE_DATA_SIZE, spare, sizeof(spare));
    for (i = 0; i < 512; i ++)
    {
        rt_kprintf("0x%X,",data_ptr[i]);
    }

    rt_kprintf("\n spare\n");
    for (i = 0; i < sizeof(spare); i ++)
    {
        rt_kprintf("0x%X,",spare[i]);
    }
    rt_kprintf("\n\n");

    /* release memory */
    rt_free(data_ptr);
}

void nwrite(rt_uint32_t partion, int page)
{
    int i;
    rt_uint8_t spare[64];
    rt_uint8_t *data_ptr;
    struct rt_mtd_nand_device *device;

    if (partion >= 3)
        return;
    device = &_partition[partion];

    data_ptr = (rt_uint8_t*) rt_malloc (PAGE_DATA_SIZE);
    if (data_ptr == RT_NULL)
    {
        rt_kprintf("no memory.\n");
        return;
    }

    /* Need random data to test ECC */
    for (i = 0; i < PAGE_DATA_SIZE; i ++)
        data_ptr[i] = i/5 -i;
    rt_memset(spare, 0xdd, sizeof(spare));
    nandflash_writepage(device, page, data_ptr, PAGE_DATA_SIZE, spare, sizeof(spare));

    rt_free(data_ptr);
}

void ncopy(rt_uint32_t partion, int src, int dst)
{
    struct rt_mtd_nand_device *device;

    if (partion >= 3)
        return;
    device = &_partition[partion];
    nandflash_pagecopy(device, src, dst);
}

void nerase(int partion, int block)
{
    struct rt_mtd_nand_device *device;

    if (partion >= 3)
        return;
    device = &_partition[partion];
    nandflash_eraseblock(device, block);
}

void nerase_all(rt_uint32_t partion)
{
    rt_uint32_t index;
    struct rt_mtd_nand_device *device;

    if (partion >= 3)
        return;
    device = &_partition[partion];
    for (index = 0; index < device->block_total; index ++)
    {
        nandflash_eraseblock(device, index);
    }
}

void nid(void)
{
    nandflash_readid(0);
}

FINSH_FUNCTION_EXPORT(nid, nand id);
FINSH_FUNCTION_EXPORT(ncopy, nand copy page);
FINSH_FUNCTION_EXPORT(nerase, nand erase a block of one partiton);
FINSH_FUNCTION_EXPORT(nerase_all, erase all blocks of a partition);
FINSH_FUNCTION_EXPORT(nwrite, nand write page);
FINSH_FUNCTION_EXPORT(nread, nand read page);
