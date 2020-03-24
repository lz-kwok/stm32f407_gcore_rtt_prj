/*
*********************************************************************************************************
*
*	模块名称 : NAND Flash驱动模块
*	文件名称 : NAND_STM32F1xx.c
*	版    本 : V1.0
*	说    明 : 提供NAND Flash (HY27UF081G2A， 8bit 128K字节 大页)的底层接口函数。【安富莱原创】
*              基于bsp_nand_flash.c文件修改而来。
*
*	修改记录 :
*		版本号  日期        作者       说明
*		V1.0    2020-03-24  leonGuo  测试版本
*
*	Copyright (C), 2020-present, lz_kwok@163.com
*
*********************************************************************************************************
*/

/* 引脚PG6的超时判断和使能 */
#define TIMEOUT_COUNT			168000000  
#define ENABLE_TIMEOUT		0

/*
	【硬件说明】
	安富莱开发板配置的NAND Flahs为海力士的HY27UF081G2A,  439板子配的为 H27U4G8F2DTR
	（1）NAND Flash的片选信号连接到CPU的FSMC_NCE2，这决定了NAND Flash的地址空间为 0x70000000（见CPU的数据
		手册的FSMC章节)
	（2）有FSMC总线上有多个总线设备（如TFT、SRAM、CH374T、NOR），因此必须确保其他总线设备的片选处于禁止
		状态，否则将出现总线冲突问题 （参见本文件初始化FSMC GPIO的函数）

	【NAND Flash 结构定义】
     备用区有16x4字节，每page 2048字节，每512字节一个扇区，每个扇区对应16自己的备用区：

	 每个PAGE的逻辑结构，前面512Bx4是主数据区，后面16Bx4是备用区
	┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐┌──────┐
	│ Main area  ││ Main area  ││ Main area  ││Main area   ││ Spare area ││ Spare area ││ Spare area ││Spare area  │
	│            ││            ││            ││            ││            ││            ││            ││            │
	│   512B     ││    512B    ││    512B    ││    512B    ││    16B     ││     16B    ││     16B    ││    16B     │
	└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘└──────┘

	 每16B的备用区的逻辑结构如下:(FlashFS中的配置）
	┌───┐┌───┐┌───┐┌───┐┌──┐┌───┐┌───┐┌───┐┌──┐┌──┐┌──┐┌──┐┌───┐┌───┐┌───┐┌───┐
	│LSN0  ││LSN1  ││LSN2  ││LSN3  ││COR ││BBM   ││ECC0  ││ECC1  ││ECC2││ECC3││ECC4││ECC5││  ECC6││  ECC7││ ECC8 ││ ECC9 │
	│      ││      ││      ││      ││    ││      ││      ││      ││    ││    ││    ││    ││      ││      ││      ││      │
	└───┘└───┘└───┘└───┘└──┘└───┘└───┘└───┘└──┘└──┘└──┘└──┘└───┘└───┘└───┘└───┘

    - LSN0 ~ LSN3 : 逻辑扇区号(logical sector number) 。
		- COR         : 数据损坏标记（data corrupted marker）。
		- BBM         : 坏块标志(Bad Block Marker)。
    - ECC0 ~ ECC9 : 512B主数据区的ECC校验。

	K9F1G08U0A 和 HY27UF081G2A 是兼容的。芯片出厂时，厂商保证芯片的第1个块是好块。如果是坏块，则在该块的第1个PAGE的第1个字节
	或者第2个PAGE（当第1个PAGE坏了无法标记为0xFF时）的第1个字节写入非0xFF值。坏块标记值是随机的，软件直接判断是否等于0xFF即可。

	注意：网上有些资料说NAND Flash厂商的默认做法是将坏块标记定在第1个PAGE的第6个字节处。这个说法是错误。坏块标记在第6个字节仅针对部分小扇区（512字节）的NAND Flash
	并不是所有的NAND Flash都是这个标准。大家在更换NAND Flash时，请仔细阅读芯片的数据手册。

*/

/* 定义NAND Flash的物理地址。这个是由硬件决定的 */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR

/* 定义操作NAND Flash用到3个宏 */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

static uint8_t FSMC_NAND_GetStatus(void);

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_Init
*	功能说明: 配置FSMC和GPIO用于NAND Flash接口。这个函数必须在读写nand flash前被调用一次。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void FSMC_NAND_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NANDInitTypeDef  FSMC_NANDInitStructure;
	FSMC_NAND_PCCARDTimingInitTypeDef  p;


	/*--NAND Flash GPIOs 配置  ------
		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE
		PD5/FSMC_NWE
		PD7/FSMC_NCE2
		PD11/FSMC_A16
		PD12/FSMC_A17
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7

		PG6/FSMC_INT2	(本例程用查询方式判忙，此口线作为普通GPIO输入功能使用)
	*/

	/* 使能 GPIO 时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

	/* 使能 FSMC 时钟 */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/*  配置GPIOD */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/*  配置GPIOE */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*  配置GPIOG, PG6作为忙信息，配置为输入 */

	/* INT2 引脚配置为内部上来输入，用于忙信号 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* 配置 FSMC 时序 2014-06-29 调整时序 */
	/*
		Defines the number of HCLK cycles to setup address before the command assertion for NAND-Flash
		read or write access  to common/Attribute or I/O memory space (depending on the memory space
		timing to be configured).This parameter can be a value between 0 and 0xFF.
	*/
  	//p.FSMC_SetupTime = 0x01;  ---- 01 err, 02 ok
  	p.FSMC_SetupTime = 0x2;

	/*
		Defines the minimum number of HCLK cycles to assert the command for NAND-Flash read or write
		access to common/Attribute or I/O memory space (depending on the memory space timing to be
		configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_WaitSetupTime = 0x03; ---- 03 err;  05 ok
	p.FSMC_WaitSetupTime = 0x5;

	/*
		Defines the number of HCLK clock cycles to hold address (and data for write access) after the
		command deassertion for NAND-Flash read or write access to common/Attribute or I/O memory space
		 (depending on the memory space timing to be configured).
		This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HoldSetupTime = 0x02;  --- 02 err  03 ok
	p.FSMC_HoldSetupTime = 0x3;

	/*
		Defines the number of HCLK clock cycles during which the databus is kept in HiZ after the start
		of a NAND-Flash  write access to common/Attribute or I/O memory space (depending on the memory
		space timing to be configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HiZSetupTime = 0x01;
	p.FSMC_HiZSetupTime = 0x1;


	FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;  					/* 定义FSMC NAND BANK 号 */
	FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Disable;		/* 插入等待时序使能 */
	FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;	/* 数据宽度 8bit */
	FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Disable; 					/* ECC错误检查和纠正功能使能 */
	FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;	/* ECC 页面大小 */
	FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x01;						/* CLE低和RE低之间的延迟，HCLK周期数 */	
	FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x01;						/* ALE低和RE低之间的延迟，HCLK周期数 */

	FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;				/* FSMC Common Space Timing */
	FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;			/* FSMC Attribute Space Timing */

	FSMC_NANDInit(&FSMC_NANDInitStructure);

	/* FSMC NAND Bank 使能 */
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_Reset
*	功能说明: 复位NAND Flash
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;

		/* 检查操作状态 */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}

	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadStatus
*	功能说明: 使用Read statuc 命令读NAND Flash内部状态
*	形    参:  - Address: 被擦除的快内任意地址
*	返 回 值: NAND操作状态，有如下几种值：
*             - NAND_BUSY: 内部正忙
*             - NAND_READY: 内部空闲，可以进行下步操作
*             - NAND_ERROR: 先前的命令执行失败
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;

	/* 读状态操作 */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);

	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	}
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY;
	}

	return (ucStatus);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_GetStatus
*	功能说明: 获取NAND Flash操作状态
*	形    参:  - Address: 被擦除的快内任意地址
*	返 回 值: NAND操作状态，有如下几种值：
*             - NAND_TIMEOUT_ERROR  : 超时错误
*             - NAND_READY          : 操作成功
*             - NAND_ERROR: 先前的命令执行失败
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x10000;
	uint8_t ucStatus = NAND_READY;

	ucStatus = FSMC_NAND_ReadStatus();

	/* 等待NAND操作结束，超时后会退出 */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		if(ucStatus == NAND_ERROR)
		{
			/* 返回操作状态 */
			return (ucStatus);
		}
		ulTimeout--;
	}

	if(ulTimeout == 0x00)
	{
		ucStatus =  NAND_TIMEOUT_ERROR;
	}

	/* 返回操作状态 */
	return (ucStatus);
}

/*
*********************************************************************************************************
*	函 数 名: NAND_ReadID
*	功能说明: 读NAND Flash的ID。ID存储到形参指定的结构体变量中。
*	形    参: 无
*	返 回 值: 32bit的NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;

	/* 发送命令 Command to the command area */
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;

	/* 顺序读取NAND Flash的ID */
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) |
			((data << 8 ) & 0x00FF0000) |
			((data >> 8 ) & 0x0000FF00) |
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_WritePage
*	功能说明: 写一组数据至NandFlash指定页面的指定位置，写入的数据长度不大于一页的大小。
*	形    参:  - _pBuffer: 指向包含待写数据的缓冲区
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内地址，范围为：0-2111
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*             RTV_NOERR         - 页写入成功
*             ERR_NAND_PROG     - 页写入失败
*             ERR_NAND_HW_TOUT  - 超时
*********************************************************************************************************
*/
uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint32_t i;
	uint8_t ucStatus;

	/* 发送页写命令 */
	NAND_CMD_AREA = NAND_CMD_WRITE0;

	/* 发送页内地址 ， 对于 HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20
		第5字节： A28  A29  A30  A31  0    0    0    0
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	#if NAND_ADDR_5 == 1
		NAND_ADDR_AREA = (_ulPageNo & 0xFF0000) >> 16;
	#endif

	/* tADL = 100ns,  Address to Data Loading */
	for (i = 0; i < 20; i++);	/* 需要大于 100ns */

	/* 写数据 */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;

	/* WE High to Busy , 100ns */
	for (i = 0; i < 20; i++);	/* 需要大于 100ns */
	
#if ENABLE_TIMEOUT
	for (i = TIMEOUT_COUNT; i; i--) 
	{
		 if (GPIOG->IDR & GPIO_Pin_6) break;
	}
	
	/* 返回超时 */
	if(i == 0)
	{
		return (ERR_NAND_HW_TOUT);
	}	
#else
	while((GPIOG->IDR & GPIO_Pin_6) == 0);
#endif 
	
	/* 返回状态 */
	ucStatus = FSMC_NAND_GetStatus();
	if(ucStatus == NAND_READY)   
	{
		ucStatus = RTV_NOERR;
	}
	else if(ucStatus == NAND_ERROR)
	{
		ucStatus = ERR_NAND_PROG;		
	}
	else if(ucStatus == NAND_TIMEOUT_ERROR)
	{
		ucStatus = ERR_NAND_HW_TOUT;		
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadPage
*	功能说明: 从NandFlash指定页面的指定位置读一组数据，读出的数据长度不大于一页的大小。
*	形    参:  - _pBuffer: 指向包含待写数据的缓冲区
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内地址，范围为：0-2111
*             - _usByteCount: 字节个数，  （最大 2048 + 64）
*	返 回 值: 执行结果：
*             RTV_NOERR         - 页读取成功
 *            ERR_NAND_HW_TOUT  - 超时
*********************************************************************************************************
*/
uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint32_t i;

    /* 发送页面读命令 */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/* 发送页内地址 ， 对于 HY27UF081G2A  (128MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20
		第5字节： A28  A29  A30  A31  0    0    0    0
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	#if NAND_ADDR_5 == 1
		NAND_ADDR_AREA = (_ulPageNo & 0xFF0000) >> 16;
	#endif

	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;

	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	for (i = 0; i < 20; i++);

#if ENABLE_TIMEOUT
	for (i = TIMEOUT_COUNT; i; i--) 
	{
		if (GPIOG->IDR & GPIO_Pin_6) break;
	}
	
	/* 返回超时 */
	if(i == 0)
	{
		return (ERR_NAND_HW_TOUT);
	}	
#else
	while((GPIOG->IDR & GPIO_Pin_6) == 0);
#endif 

	/* 读数据到缓冲区pBuffer */
	for(i = 0; i < _usByteCount; i++)
	{
		_pBuffer[i] = NAND_DATA_AREA;
	}

	return RTV_NOERR;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_EraseBlock
*	功能说明: 擦除NAND Flash一个块（block）
*	形    参:  - _ulBlockNo: 块号，范围为：0 - 1023,   0-4095
*	返 回 值: NAND操作状态，有如下几种值：
*             RTV_NOERR         - 擦除成功
*             ERR_NAND_ERASE    - 擦除失败
*             ERR_NAND_HW_TOUT  - 超时
*********************************************************************************************************
*/
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	uint8_t ucStatus;
	
	/* HY27UF081G2A  (128MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12    A18以上是块号
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    0    A11  A10  A9   A8		(_usPageAddr 的bit11 - bit8, 高4bit必须是0)
		第3字节： A19  A18  A17  A16  A15  A14  A13  A12    A18以上是块号
		第4字节： A27  A26  A25  A24  A23  A22  A21  A20
		第5字节： A28  A29  A30  A31  0    0    0    0
	*/

	/* 发送擦除命令 */
	NAND_CMD_AREA = NAND_CMD_ERASE0;

	_ulBlockNo <<= 6;	/* 块号转换为页编号 */

	#if NAND_ADDR_5 == 0	/* 128MB的 */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
	#else		/* 512MB的 */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
		NAND_ADDR_AREA = _ulBlockNo >> 16;
	#endif

	NAND_CMD_AREA = NAND_CMD_ERASE1;

	/* 返回状态 */
	ucStatus = FSMC_NAND_GetStatus();
	if(ucStatus == NAND_READY)   
	{
		ucStatus = RTV_NOERR;
	}
	else if(ucStatus == NAND_ERROR)
	{
		ucStatus = ERR_NAND_PROG;		
	}
	else if(ucStatus == NAND_TIMEOUT_ERROR)
	{
		ucStatus = ERR_NAND_HW_TOUT;		
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	函 数 名: NAND_IsBadBlock
*	功能说明: 根据坏块标记检测NAND Flash指定的块是否坏块
*	形    参: _ulBlockNo ：块号 0 - 1023 （对于128M字节，2K Page的NAND Flash，有1024个块）
*	返 回 值: 0 ：该块可用； 1 ：该块是坏块
*********************************************************************************************************
*/
uint8_t NAND_IsBadBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;

	/* 如果NAND Flash出厂前已经标注为坏块了，则就认为是坏块 */
	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, NAND_PAGE_SIZE + BBM_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}

	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE + 1, NAND_PAGE_SIZE + BBM_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}
	return 0;	/* 是好块 */
}

/*
*********************************************************************************************************
*	函 数 名: NAND_Format
*	功能说明: NAND Flash格式化，擦除所有的数据，重建LUT
*	形    参:  无
*	返 回 值: NAND_OK : 成功； NAND_Fail ：失败（一般是坏块数量过多导致）
*********************************************************************************************************
*/
uint8_t NAND_Format(void)
{
#if 0
	uint16_t i;
	uint16_t usGoodBlockCount;

	/* 擦除每个块 */
	usGoodBlockCount = 0;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/* 如果是好块，则擦除 */
		if (!NAND_IsBadBlock(i))
		{
			FSMC_NAND_EraseBlock(i);
			usGoodBlockCount++;
		}
	}

	/* 如果好块的数量少于100，则NAND Flash报废 */
	if (usGoodBlockCount < 100)
	{
		return NAND_FAIL;
	}

	return NAND_OK;
#else
	uint16_t i;

	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		FSMC_NAND_EraseBlock(i);
	}

	return NAND_OK;
#endif
}

/*
*********************************************************************************************************
*	函 数 名: NAND_Init
*	功能说明: 初始化NAND Flash接口
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void NAND_Init(void)
{
	FSMC_NAND_Init();			/* 配置FSMC和GPIO用于NAND Flash接口 */

	FSMC_NAND_Reset();			/* 通过复位命令复位NAND Flash到读状态 */
}

/*
*********************************************************************************************************
*	函 数 名: NAND_UnInit
*	功能说明: 卸载NAND Flash
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void NAND_UnInit(void)
{
	FSMC_NANDDeInit(FSMC_Bank2_NAND);
	
	/* FSMC NAND Bank 禁能 */
	FSMC_NANDCmd(FSMC_Bank2_NAND, DISABLE);
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
