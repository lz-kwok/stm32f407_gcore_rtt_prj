/*
*********************************************************************************************************
*
*	模块名称 : SDIO的底层驱动
*	文件名称 : SDIO_STM32F103.c
*	版    本 : V1.0
*	说    明 : 本文件来自KEIL官方，为支持V6开发板，修改地方有二
*              1. Init函数中SD检测引脚改成PE2。
*              2. CheckMedia函数改成使用PE2做检测。
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2015-09-10   Eric2013    1. RL-FlashFS版本V4.74
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
/*-----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *-----------------------------------------------------------------------------
 *      Name:    SDIO_STM32F4xx.c
 *      Purpose: SD/SDIO MMC Interface Driver for ST STM32F4xx
 *      Rev.:    V4.60
 *-----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2012 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "includes.h"
#include <File_Config.h>
#include <stm32f4xx.h>                  /* STM32F4xx Definitions              */
#include "SDIO_STM32F4xx.h"

/*-----------------------------------------------------------------------------
  Memory Card FAT Driver instance definition
   mci0_drv: First SD/MMC drive [M0:]
   mci1_drv: Second SD/MMC drive [M1:]
 *----------------------------------------------------------------------------*/

#define __DRV_ID   mci0_drv
#define __SDIOCLK  48000000
#define __CPUCLK  168000000

/* MCI Driver Interface functions */
static BOOL Init (void);
static BOOL UnInit (void);
static void Delay (U32 us);
static BOOL BusMode (U32 mode);
static BOOL BusWidth (U32 width);
static BOOL BusSpeed (U32 kbaud);
static BOOL Command (U8 cmd, U32 arg, U32 resp_type, U32 *rp);
static BOOL ReadBlock (U32 bl, U8 *buf, U32 cnt);
static BOOL WriteBlock (U32 bl, U8 *buf, U32 cnt);
static BOOL SetDma (U32 mode, U8 *buf, U32 cnt);
static U32  CheckMedia (void);        /* Optional function for SD card check  */
void SD_Link_EXTIConfig(void);

/* MCI Device Driver Control Block */
MCI_DRV __DRV_ID = {
  Init,
  UnInit,
  Delay,
  BusMode,
  BusWidth,
  BusSpeed,
  Command,
  ReadBlock,
  WriteBlock,
  SetDma,
  CheckMedia                            /* Can be NULL if not existing        */
};


/* Wait time in for loop cycles */
#define DMA_TOUT  10000000

/*--------------------------- Init -------------------------------------------*/

static BOOL Init (void) {
  /* Initialize SDIO interface. */
  /* Enable GPIOE (card detect), GPIOD (command), GPIOC (data) clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN |
                  RCC_AHB1ENR_GPIODEN |
                  RCC_AHB1ENR_GPIOCEN;

  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; 
  /* Enable IO compensation cell */
  SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD;
  while (!(SYSCFG->CMPCR & SYSCFG_CMPCR_READY)); 

  /* PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */ 
  GPIOC->MODER   &= ~0x03FF0000;
  GPIOC->MODER   |=  0x02AA0000;        /* Pins to alternate function         */
  GPIOC->OTYPER  &= ~0x00001F00;        /* Configure as push-pull pins        */
  GPIOC->PUPDR   &= ~0x03FF0000;
  GPIOC->PUPDR   |=  0x01550000;        /* Pull-ups on pins                   */
  GPIOC->OSPEEDR |=  0x03FF0000;        /* Pins output speed to 100MHz        */

  GPIOC->AFR[1]  &= ~0x000FFFFF;
  GPIOC->AFR[1]  |=  0x000CCCCC;        /* Pins assigned to AF12 (SDIO)       */

  /* Configure PD.02 CMD line */
  GPIOD->MODER   &= ~0x0000030;
  GPIOD->MODER   |=  0x0000020;         /* Pins to alternate function         */
  GPIOD->OTYPER  &= ~0x0000004;         /* Configure as push-pull pin         */
  GPIOD->PUPDR   &= ~0x0000030;
  GPIOD->PUPDR   |=  0x0000010;         /* Pull-up on pin                     */       
  GPIOD->OSPEEDR |=  0x0000030;         /* Pins output speed to 100MHz        */

  GPIOD->AFR[0]  &= ~0x00000F00;
  GPIOD->AFR[0]  |=  0x00000C00;        /* Pin assigned to AF12 (SDIO)        */

  /* Configure PE.2 Card Detect input */
  SD_Link_EXTIConfig();

  /* Enable SDIO clock */
  RCC->APB2ENR  |=  RCC_APB2ENR_SDIOEN;

  /* Reset/Dereset SDIO */
  RCC->APB2RSTR |=  RCC_APB2RSTR_SDIORST;
  RCC->APB2RSTR &= ~RCC_APB2RSTR_SDIORST;

  /* Power-on, the card is clocked */
  SDIO->POWER  = SDIO_POWER_PWRCTRL_1 | SDIO_POWER_PWRCTRL_0;

    /* Enable DMA2 clock */
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  /* Success, SDIO initialized. */
  return (__TRUE);
}


/*--------------------------- UnInit -----------------------------------------*/

static BOOL UnInit (void) {

  /* Reset PC.8, PC.9, PC.10, PC.11, PC.12 pins */
  GPIOC->MODER &= ~0x03FF0000;
  /* Reset PD.2 */
  GPIOD->MODER &= ~(3 <<  4);

  /* Disable SDIO module clock */
  RCC->APB2ENR &= ~RCC_APB2ENR_SDIOEN;

  return (__TRUE);
}


/*--------------------------- Delay ------------------------------------------*/

static void Delay (U32 us) {
  /* Approximate delay in micro seconds. */
  U32 i;

  for (i = WAIT_CNT(__CPUCLK, us); i; i--);
}


/*--------------------------- BusMode ----------------------------------------*/

static BOOL BusMode (U32 mode) {
  /* Set SDIO Bus mode to Open Drain or Push Pull. */

  switch (mode) {
    case BUS_OPEN_DRAIN:
      GPIOD->OTYPER  |=  0x00000010;    /* Configure as open-drain            */
      return (__TRUE);

    case BUS_PUSH_PULL:
      GPIOD->OTYPER  &= ~0x00000010;    /* Configure as push-pull pin         */
      return (__TRUE);
    
    default:
      return (__FALSE);
  }
}


/*--------------------------- BusWidth ---------------------------------------*/

static BOOL BusWidth (U32 width) {
  /* Set SDIO Bus width. */

  switch (width) {
    case 1:
      SDIO->CLKCR &= ~SDIO_CLKCR_WIDBUS;
      return (__TRUE);

    case 4:
#ifdef __XYNERGY
      return (__FALSE);
#else
      SDIO->CLKCR |=  SDIO_CLKCR_WIDBUS_0;
      return (__TRUE);
#endif
    
    default:
      return (__FALSE);
  }
}


/*--------------------------- BusSpeed ---------------------------------------*/

static BOOL BusSpeed (U32 kbaud) {
  /* Set SDIO clock speed to desired value. */
  U32 div;

  /* baud = SDIOCLK / (div + 2) */
  div = (__SDIOCLK/1000 + kbaud - 1) / kbaud;
  if (div < 2)    div  = 0;
  else            div -= 2;
  if (div > 0xFF) div  = 0xFF;
  SDIO->CLKCR = (SDIO->CLKCR & ~0xFF) | SDIO_CLKCR_PWRSAV | SDIO_CLKCR_CLKEN | div;
  return (__TRUE);
}


/*--------------------------- Command ----------------------------------------*/

static BOOL Command (U8 cmd, U32 arg, U32 resp_type, U32 *rp) {
  /* Send a Command to Flash card and get a Response. */
  U32 cmdval,stat;

  cmd   &= SDIO_CMD_CMDINDEX;
  cmdval = SDIO_CMD_CPSMEN | cmd;
  switch (resp_type) {
    case RESP_SHORT:
      cmdval |= SDIO_CMD_WAITRESP_0;
      break;
    case RESP_LONG:
      cmdval |= SDIO_CMD_WAITRESP_1 | SDIO_CMD_WAITRESP_0;
      break;
  }
  /* Send the command. */
  SDIO->ARG = arg;
  SDIO->CMD = cmdval;

  if (resp_type == RESP_NONE) {
    /* Wait until command finished. */
    while (SDIO->STA & SDIO_STA_CMDACT);
    stat = SDIO->STA;
    SDIO->ICR = 0x00C007FF; 
    if (stat & SDIO_STA_CMDSENT) {
      return (__TRUE);
    }
    return (__FALSE);    
  }

  for (;;) {
    stat = SDIO->STA;
    if (stat & SDIO_STA_CTIMEOUT) {
      SDIO->ICR = stat & SDIO_STA_CLEAR_MASK;
      return (__FALSE);
    }
    if (stat & SDIO_STA_CCRCFAIL) {
      SDIO->ICR = stat & SDIO_STA_CLEAR_MASK;
      if ((cmd == SEND_OP_COND)      ||
          (cmd == SEND_APP_OP_COND)  ||
          (cmd == STOP_TRANS)) {
        SDIO->CMD = 0;
        break;
      }
      return (__FALSE);
    }
    if (stat & SDIO_STA_CMDREND) {
      SDIO->ICR = stat & SDIO_STA_CLEAR_MASK;
      break;
    }
  }
  if ((SDIO->RESPCMD & 0x3F) != cmd) {
    if ((SDIO->RESPCMD & 0x3F) != 0x3F) {
      return (__FALSE);
    }
  }
  /* Read MCI response registers */
  rp[0] = SDIO->RESP1;
  if (resp_type == RESP_LONG) {
    rp[1] = SDIO->RESP2;
    rp[2] = SDIO->RESP3;
    rp[3] = SDIO->RESP4;
  }
  return (__TRUE);
}


/*--------------------------- ReadBlock --------------------------------------*/

static BOOL ReadBlock (U32 bl, U8 *buf, U32 cnt) {
  /* Read one or more 512 byte blocks from Flash Card. */
  U32 i;

  SDIO->DLEN    = cnt * 512;
  SDIO->DTIMER  = cnt * DATA_RD_TOUT_VALUE;

  SDIO->DCTRL   = SDIO_DCTRL_DBLOCKSIZE_3 | SDIO_DCTRL_DBLOCKSIZE_0 |
                  SDIO_DCTRL_DMAEN        | SDIO_DCTRL_DTDIR        |
                  SDIO_DCTRL_DTEN         ;

  for (i = DMA_TOUT; i; i--) {    
    if (DMA2->LISR & DMA_LISR_TEIF3) {
      break;
    }
    
    if (DMA2->LISR & DMA_LISR_TCIF3) {
      if ((SDIO->STA & (SDIO_STA_DBCKEND|SDIO_STA_DATAEND)) == (SDIO_STA_DBCKEND|SDIO_STA_DATAEND)) {
        /* Data transfer finished. */
        return (__TRUE);
      }
    }
  }
  /* DMA Transfer timeout. */
  return (__FALSE);
}


/*--------------------------- WriteBlock -------------------------------------*/

static BOOL WriteBlock (U32 bl, U8 *buf, U32 cnt) {
  /* Write a cnt number of 512 byte blocks to Flash Card. */
  U32 i;

  SDIO->DLEN    = cnt * 512;
  SDIO->DTIMER  = cnt * DATA_WR_TOUT_VALUE;
  SDIO->DCTRL   = SDIO_DCTRL_DBLOCKSIZE_3 | SDIO_DCTRL_DBLOCKSIZE_0 |
                  SDIO_DCTRL_DMAEN        | SDIO_DCTRL_DTEN         ;

  for (i = DMA_TOUT; i; i--) {
    if (DMA2->LISR & DMA_LISR_TEIF3) {
      break;
    }
    
    if (DMA2->LISR & DMA_LISR_TCIF3) {
      if ((SDIO->STA & (SDIO_STA_DBCKEND|SDIO_STA_DATAEND)) == (SDIO_STA_DBCKEND|SDIO_STA_DATAEND)) { 
        /* Data transfer finished. */
        return (__TRUE);
      }
    }
  }
  /* DMA Transfer timeout. */
  return (__FALSE);
}


/*--------------------------- DmaStart ---------------------------------------*/

static BOOL SetDma (U32 mode, U8 *buf, U32 cnt) {
  /* Configure DMA Channel 4, Stream 3 for read or write */

  DMA2->LIFCR = 0x0F4 << 20;                   /* Clear all IRQs              */

  DMA2_Stream3->CR  &= ~DMA_SxCR_EN;           /* Disable stream 3            */
  while (DMA2_Stream3->CR & DMA_SxCR_EN);      /* Wait until stream disabled  */

  DMA2_Stream3->FCR |=  DMA_SxFCR_DMDIS |      /* Disable direct mode         */
                        DMA_SxFCR_FTH   ;      /* FIFO threshold full         */

  DMA2_Stream3->PAR  = (U32)&(SDIO->FIFO);
  DMA2_Stream3->M0AR = (U32)buf;

  if (mode == DMA_READ) {
    /* Transfer from SDIO-FIFO to memory. */
    DMA2_Stream3->CR = (4 << 25) |             /* Use channel 4               */
                       (1 << 23) |             /* Memory burst 4 beats        */
                       (1 << 21) |             /* Peripheral burst 4 beats    */
                       (3 << 16) |             /* Priority level high         */
                       (2 << 13) |             /* Memory size 32-bits         */
                       (2 << 11) |             /* Peripheral size 32-bits     */
                       (1 << 10) |             /* Memory addr increment       */
                       (1 <<  5) ;             /* Peripheral flow control     */
  }
  else {
    /* Transfer from memory to SDIO-FIFO. */
    DMA2_Stream3->CR = (4 << 25) |             /* Use channel 4               */
                       (1 << 23) |             /* Memory burst 4 beats        */
                       (1 << 21) |             /* Peripheral burst 4 beats    */
                       (3 << 16) |             /* Priority level very high    */
                       (2 << 13) |             /* Memory size 32-bits         */
                       (2 << 11) |             /* Peripheral size 32-bits     */
                       (1 << 10) |             /* Memory addr increment       */
                       (1 <<  6) |             /* Memory-to-peripheral        */
                       (1 <<  5) ;             /* Peripheral flow control     */
  }

  /* Enable DMA channels, little endian */
  DMA2_Stream3->CR  |= DMA_SxCR_EN;            /* Enable stream 3             */
  return __TRUE;
}


/*--------------------------- CheckMedia -------------------------------------*/

static U32 CheckMedia (void) {
  /* Read CardDetect and WriteProtect SD card socket pins. */
  U32 stat = 0;

  if (!(GPIOE->IDR & (1 << 2))) {
    /* Card is inserted (CD=0). */
    stat |= M_INSERTED;
  }

 #if 0 
  if (/* Write protect switch is active */) {
    stat |= M_PROTECTED;
  }
 #endif

  return (stat);
}
  

/*--------------------------- SD_Link_EXTIConfig ---------------------------------------*/

void SD_Link_EXTIConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 安富莱STM32-V6开发板使用PE2作为中断输入口, 下降沿表示中断信号 
	   未插入SD卡的时，PE2引脚是高电平，插入后，PE2引脚是低电平。
	*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* 配置中断引脚是输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 配置外部中断线连接到相应引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource2);

	/* 配置外部中断线 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* 使能中断通道 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*--------------------------- EXTI2_IRQHandler ---------------------------------------*/
void EXTI2_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI->IMR&=~(1<<2);	 /* 关闭中断 */
		if(HandlePinDetectStart)
		{
			isr_evt_set(SDDetectBIT_0, HandlePinDetectStart);
		}
		
		/* 清中断挂起位 */
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}

/*-----------------------------------------------------------------------------
 * end of file
 *----------------------------------------------------------------------------*/
