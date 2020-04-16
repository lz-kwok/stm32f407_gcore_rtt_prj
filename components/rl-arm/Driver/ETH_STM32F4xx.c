/*
*********************************************************************************************************
*
*	模块名称 : STM32F4XX的MAC驱动
*	文件名称 : ETH_STM32F4XX.C
*	版    本 : V1.0
*	说    明 : 由官方的驱动修改而来。
*              1. 原始驱动支持DP83848C 和ST802RT1，新增DM9161C。
*              2. 增加PHY的中断触发功能，主要用于检测网线的连接状态。
*              3. 变量g_ucEthLinkStatus用于表示连接状态。
*              4. 待今年2017制作TCPnet教程时会对此文件做专门的规范化。
*                         
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2015-12-22    Eric2013         首发
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
/*-----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *-----------------------------------------------------------------------------
 *      Name:    ETH_STM32F4XX.C
 *      Purpose: Driver for ST STM32F4xx Ethernet Controller
 *      Rev.:    V4.73
 *-----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
 
#include <Net_Config.h>
#include <stm32f4xx.h>             
#include "ETH_STM32F4xx.h"
#include "stdio.h"


/*
	STM32-V6开发板使用的是RMII接口，PHY芯片是DM9161:

	PA1/ETH_RMII_RX_CLK
	PA2/ETH_MDIO
	PA7/RMII_CRS_DV
	PC1/ETH_MDC
	PC4/ETH_RMII_RX_D0
	PC5/ETH_RMII_RX_D1
	PG11/ETH_RMII_TX_EN
	PG13/FSMC_A24/ETH_RMII_TXD0
	PB13/ETH_RMII_TXD1
	PH6/MII_INT ----- 中断引脚,这里将其用于网线断开或者连接的状态触发
*/
#if 0
	#define printf_eth printf
#else
	#define printf_eth(...)
#endif


/* 以太网连接状态，0 表示未连接，1 表示连接 */
__IO uint8_t  g_ucEthLinkStatus = 0;  


/* The following macro definitions may be used to select the speed
   of the physical link:

  _10MBIT_   - connect at 10 MBit only
  _100MBIT_  - connect at 100 MBit only

  By default an autonegotiation of the link speed is used. This may take 
  longer to connect, but it works for 10MBit and 100MBit physical links.
  
    The following macro definitions may be used to select PYH interface:
  -MII_      - use MII interface instead of RMII
                                                                              */
/* #define _MII_ */

/* Net_Config.c */
extern U8 own_hw_adr[];

/* Local variables */
static U8 TxBufIndex;
static U8 RxBufIndex;

/* ENET local DMA Descriptors. */
static RX_Desc Rx_Desc[NUM_RX_BUF];
static TX_Desc Tx_Desc[NUM_TX_BUF];

/* ENET local DMA buffers. */
static U32 rx_buf[NUM_RX_BUF][ETH_BUF_SIZE>>2];
static U32 tx_buf[NUM_TX_BUF][ETH_BUF_SIZE>>2];

/*-----------------------------------------------------------------------------
 *      ENET Ethernet Driver Functions
 *-----------------------------------------------------------------------------
 *  Required functions for Ethernet driver module:
 *  a. Polling mode: - void init_ethernet ()
 *                   - void send_frame (OS_FRAME *frame)
 *                   - void poll_ethernet (void)
 *  b. Interrupt mode: - void init_ethernet ()
 *                     - void send_frame (OS_FRAME *frame)
 *                     - void int_enable_eth ()
 *                     - void int_disable_eth ()
 *                     - interrupt function 
 *----------------------------------------------------------------------------*/

/* Local Function Prototypes */
static void rx_descr_init (void);
static void tx_descr_init (void);
static void write_PHY (U32 PhyReg, U16 Value);
static U16  read_PHY (U32 PhyReg);
static void Eth_Link_EXTIConfig(void);

/*--------------------------- init_ethernet ----------------------------------*/

void init_ethernet (void) {
  /* Initialize the ETH ethernet controller. */
  U32 regv,tout,conn; 
	
  /* 关闭PHY中断触发引脚 added by eric2013 */
  NVIC_DisableIRQ(EXTI9_5_IRQn);
	
  /* Enable System configuration controller clock */
  RCC->APB2ENR |= (1 << 14);
  
  /* Reset Ethernet MAC */
  RCC->AHB1RSTR |=  0x02000000;
#ifdef _MII_
  SYSCFG->PMC &= ~(1 << 23);
#else
  SYSCFG->PMC |=  (1 << 23);
#endif
  RCC->AHB1RSTR &= ~0x02000000;

#ifdef _MII_
  /* Enable Ethernet and GPIOA, GPIOB, GPIOC, GPIOG, GPIOH clocks */
  RCC->AHB1ENR |= 0x1E0000C7;
#else
  /* Enable Ethernet and GPIOA, GPIOB, GPIOC, GPIOG clocks */
  RCC->AHB1ENR |= 0x1E000047;
#endif
  
#ifdef __XYNERGY
  /* Configure Port A ethernet pins (PA.1, PA.2, PA.7) */
  GPIOA->MODER   &= ~0x0000C03C;
  GPIOA->MODER   |=  0x00008028;              /* Pins to alternate function */
  GPIOA->OTYPER  &= ~0x00000086;              /* Pins in push-pull mode     */
  GPIOA->OSPEEDR |=  0x0000C03C;              /* Slew rate as 100MHz pin    */
  GPIOA->PUPDR   &= ~0x0000C03C;              /* No pull up, no pull down   */

  GPIOA->AFR[0]  &= ~0xF0000FF0;
  GPIOA->AFR[0]  |=  0xB0000BB0;              /* Pins to AF11 (Ethernet)    */

  /* Configure Port B ethernet pins (PB.10, PB.11, PB.12, PB13) */
  GPIOB->MODER   &= ~0x0FF00000;
  GPIOB->MODER   |=  0x0AA00000;              /* Pins to alternate function */
  GPIOB->OTYPER  &= ~0x00003C00;              /* Pins in push-pull mode     */
  GPIOB->OSPEEDR |=  0x0FF00000;              /* Slew rate as 100MHz pin    */
  GPIOB->PUPDR   &= ~0x0FF00000;              /* No pull up, no pull down   */

  GPIOB->AFR[1]  &= ~0x00FFFF00;
  GPIOB->AFR[1]  |=  0x00BBBB00;              /* Pins to AF11 (Ethernet)    */

  /* Configure Port C ethernet pins (PC.1, PC.4, PC.5) */
  GPIOC->MODER   &= ~0x00000F0C;
  GPIOC->MODER   |=  0x00000A08;              /* Pins to alternate function */
  GPIOC->OTYPER  &= ~0x00000032;              /* Pins in push-pull mode     */
  GPIOC->OSPEEDR |=  0x00000F0C;              /* Slew rate as 100MHz pin    */
  GPIOC->PUPDR   &= ~0x00000F0C;              /* No pull up, no pull down   */

  GPIOC->AFR[0]  &= ~0x00FF00F0;
  GPIOC->AFR[0]  |=  0x00BB00B0;              /* Pins to AF11 (Ethernet)    */
#else
  /* 
     原始驱动还配置了PA8，用于给PHY芯片提供时钟，V5开发板外置有源晶振，无需配置
	 PA8，这里仅需配置PA1，PA2和PA7即可
  */
  GPIOA->MODER   &= ~0x0000C03C;
  GPIOA->MODER   |=  0x00008028;              /* Pins to alternate function */
  GPIOA->OTYPER  &= ~0x00000086;              /* Pins in push-pull mode     */
  GPIOA->OSPEEDR |=  0x0003C03C;              /* Slew rate as 100MHz pin    */
  GPIOA->PUPDR   &= ~0x0003C03C;              /* No pull up, no pull down   */

  GPIOA->AFR[0]  &= ~0xF0000FF0;
  GPIOA->AFR[0]  |=  0xB0000BB0;              /* Pins to AF11 (Ethernet)    */

#ifdef _MII_
  /* Configure Port B ethernet pin (PB8) */
  GPIOB->MODER   &= ~0x00030000;
  GPIOB->MODER   |=  0x00020000;              /* Pins to alternate function */
  GPIOB->OTYPER  &= ~0x00000100;              /* Pin in push-pull mode      */
  GPIOB->OSPEEDR |=  0x00030000;              /* Slew rate as 100MHz pin    */
  GPIOB->PUPDR   &= ~0x00030000;              /* No pull up, no pull down   */

  GPIOB->AFR[1]  &= ~0x0000000F;
  GPIOB->AFR[1]  |=  0x0000000B;              /* Pin to AF11 (Ethernet)     */

  /* Configure Port C ethernet pins (PC.1, PC.2, PC.3, PC.4, PC.5) */
  GPIOC->MODER   &= ~0x00000FFC;
  GPIOC->MODER   |=  0x00000AA8;              /* Pins to alternate function */
  GPIOC->OTYPER  &= ~0x0000003E;              /* Pins in push-pull mode     */
  GPIOC->OSPEEDR |=  0x00000FFC;              /* Slew rate as 100MHz pin    */
  GPIOC->PUPDR   &= ~0x00000FFC;              /* No pull up, no pull down   */

  GPIOC->AFR[0]  &= ~0x00FFFFF0;
  GPIOC->AFR[0]  |=  0x00BBBBB0;              /* Pins to AF11 (Ethernet)    */
#else
  /* Configure Port C ethernet pins (PC.1, PC.4, PC.5) */
  GPIOC->MODER   &= ~0x00000F0C;
  GPIOC->MODER   |=  0x00000A08;              /* Pins to alternate function */
  GPIOC->OTYPER  &= ~0x00000032;              /* Pins in push-pull mode     */
  GPIOC->OSPEEDR |=  0x00000F0C;              /* Slew rate as 100MHz pin    */
  GPIOC->PUPDR   &= ~0x00000F0C;              /* No pull up, no pull down   */

  GPIOC->AFR[0]  &= ~0x00FF00F0;
  GPIOC->AFR[0]  |=  0x00BB00B0;              /* Pins to AF11 (Ethernet)    */
#endif
  
  /* Configure Port G ethernet pins (PG.11, PG.13, PB.13) */
  GPIOG->MODER   &= ~0x0CC00000;
  GPIOG->MODER   |=  0x08800000;              /* Pin to alternate function  */
  GPIOG->OTYPER  &= ~0x00002800;              /* Pin in push-pull mode      */
  GPIOG->OSPEEDR |=  0x0CC00000;              /* Slew rate as 100MHz pin    */
  GPIOG->PUPDR   &= ~0x0CC00000;              /* No pull up, no pull down   */

  GPIOG->AFR[1]  &= ~0x00F0F000;
  GPIOG->AFR[1]  |=  0x00B0B000;              /* Pin to AF11 (Ethernet)     */
  
  //////////////////////////////
  GPIOB->MODER   &= ~0x0C000000;
  GPIOB->MODER   |=  0x08000000;              /* Pin to alternate function  */
  GPIOB->OTYPER  &= ~0x00002000;              /* Pin in push-pull mode      */
  GPIOB->OSPEEDR |=  0x0C000000;              /* Slew rate as 100MHz pin    */
  GPIOB->PUPDR   &= ~0x0C000000;              /* No pull up, no pull down   */

  GPIOB->AFR[1]  &= ~0x00F00000;
  GPIOB->AFR[1]  |=  0x00B00000;              /* Pin to AF11 (Ethernet)     */

#ifdef _MII_
  /* Configure Port H ethernet pins (PH.0, PH.2, PH.3, PH.6, PH.7) */
  GPIOH->MODER   &= ~0x0000F0F3;
  GPIOH->MODER   |=  0x0000A0A2;              /* Pins to alternate function */
  GPIOH->OTYPER  &= ~0x000000CD;              /* Pins in push-pull mode     */
  GPIOH->OSPEEDR |=  0x0000F0F3;              /* Slew rate as 100MHz pin    */
  GPIOH->PUPDR   &= ~0x0000F0F3;              /* No pull up, no pull down   */

  GPIOH->AFR[0]  &= ~0xFF00FF0F;
  GPIOH->AFR[0]  |=  0xBB00BB0B;              /* Pins to AF11 (Ethernet)    */  
#endif

#endif

  /* 
     当该位置 1 时，MAC DMA控制器会复位所有MAC子系统的内部寄存器和逻辑。
	 在所有内核时钟域完成复位操作后，该位自动清零。
  */
  ETH->DMABMR  |= DBMR_SR;
  while (ETH->DMABMR & DBMR_SR);
  conn = 0;

  /* HCLK Clock range 100-120MHz. */
  ETH->MACMIIAR = 0x00000004;

  /*
     测试发现DM9161可以上电后就读取其ID寄存器，但是DM9162不行，需要延迟一段时间
     这里为了方便起见，直接将其复位，发送复位指令可以立即执行。
  */
   /* Check if this is a DM9161C PHY. --------------------------*/
	printf_eth("PHY_ID_DM9161C Init\r\n");
    /* Put the DM9161C in reset mode */
    write_PHY (PHY_REG_BMCR, 0x8000);

    /* Wait for hardware reset to end. */
    for (tout = 0; tout < 0x10000; tout++) {
      regv = read_PHY (PHY_REG_BMCR);
      if (!(regv & 0x8800)) {
        /* Reset complete, device not Power Down. */
		printf_eth("reset\r\n");
        break;
      }
    }
	
    /* Configure the PHY device */
#if defined (_10MBIT_)
    /* Connect at 10MBit */
    write_PHY (PHY_REG_BMCR, PHY_FULLD_10M);
#elif defined (_100MBIT_)
    /* Connect at 100MBit */
    write_PHY (PHY_REG_BMCR, PHY_FULLD_100M);
#else
    /* Use autonegotiation about the link speed. */
    write_PHY (PHY_REG_BMCR, PHY_AUTO_NEG);
    /* Wait to complete Auto_Negotiation. */
    for (tout = 0; tout < 0x100000; tout++) {
      regv = read_PHY (PHY_REG_BMSR);
      if (regv & 0x0020) {
		  printf_eth("Autonegotiation Complete\r\n");
        /* Autonegotiation Complete. */
        break;
      }
    }
#endif
    /* Check the link status. */
    for (tout = 0; tout < 0x10000; tout++) {
      regv = read_PHY (PHY_REG_BMSR);
      if (regv & (1 << 2)) {
		/* Link */ 
		g_ucEthLinkStatus = 1;
        /* Link is on, get connection info */
        regv = read_PHY (PHY_REG_DSCSR);
        if ((regv & (1 << 15))|(regv & (1 << 13))) {
		  printf_eth("Full-duplex connection\r\n");
          /* Full-duplex connection */
          conn |= PHY_CON_SET_FULLD;
        }
        if ((regv & (1 << 15))|(regv & (1 << 14))) {
		  printf_eth("100Mb/s mode\r\n");
          /* 100Mb/s mode */
          conn |= PHY_CON_SET_100M;
        }
        break;
      }
	  else{
		/* UnLink */ 
	  	g_ucEthLinkStatus = 0;
	  }
    }
	
	/* 使能DM9161C的连接中断 Link Change */
	write_PHY (PHY_REG_INTERRUPT, 1<<12);
	
	/* 配置引脚PH6来接收中断信号 */
	Eth_Link_EXTIConfig();
    
  /* 
	 Initialize MAC configuration register 
	 当该位置 1 时，MAC 禁止在半双工模式下接收帧。
	 当该位复位时，MAC 接收发送时 PHY 提供的所有包。
	 如果 MAC 在全双工模式下工作，该位不适用。
  */
  ETH->MACCR  = MCR_ROD;

  /* 
	 Configure Full/Half Duplex mode. 
     当该位置 1 时，MAC 在全双工模式下工作，此时它可以同时发送和接收。
  */
  if (conn & PHY_CON_SET_FULLD) {
    /* Full duplex is enabled. */
    ETH->MACCR |= MCR_DM;
  }

  /* 
     Configure 100MBit/10MBit mode. 
     指示快速以太网 (MII) 模式下的速度：
	 0：10 Mbit/s
     1：100 Mbit/s
  */
  if (conn & PHY_CON_SET_100M) {
    /* 100MBit mode. */
    ETH->MACCR |= MCR_FES;
  }

  /* 
     MAC address filtering, accept multicast packets. 
     MACFFR 以太网帧过滤寄存器。
     MACFCR 以太网流控制寄存器，ZQPD零时间片暂停禁止
  */
  ETH->MACFFR = MFFR_HPF | MFFR_PAM;
  ETH->MACFCR = MFCR_ZQPD;

  /* Set the Ethernet MAC Address registers */
  ETH->MACA0HR = ((U32)own_hw_adr[5] <<  8) | (U32)own_hw_adr[4];
  ETH->MACA0LR = ((U32)own_hw_adr[3] << 24) | (U32)own_hw_adr[2] << 16 |
                 ((U32)own_hw_adr[1] <<  8) | (U32)own_hw_adr[0];

  /* Initialize Tx and Rx DMA Descriptors */
  rx_descr_init ();
  tx_descr_init ();

  /* Flush FIFO, start DMA Tx and Rx 
     DMAOMR 工作模式寄存器
     DOMR_FTF：
       该位置1时，发送FIFO控制器逻辑会复位为默认值，因此，Tx FIFO中的所有数据均会丢
	   失 / 刷新。刷新操作结束时该位在内部清零。此位清零之前不得对工作模式寄存器执行写操作。
	 DOMR_ST：
       该位置1时，发送过程会进入运行状态，DMA会检查当前位置的发送列表查找待发送的帧。
	 DOMR_SR：
	   该位置1时，接收过程会进入运行状态，DMA尝试从接收列表中获取描述符并处理传入帧。
  */
  ETH->DMAOMR = DOMR_FTF | DOMR_ST | DOMR_SR;

  /* Enable receiver and transmiter */
  ETH->MACCR |= MCR_TE | MCR_RE;

  /* Reset all interrupts */
  ETH->DMASR  = 0xFFFFFFFF;

  /* 
     Enable Rx and Tx interrupts. 
     NISE：正常中断汇总使能 
	 AISE：异常中断汇总使能
	 RBUIE：接收缓冲区不可用中断使能
	 RIE：接收中断使能
  */
  ETH->DMAIER = ETH_DMAIER_NISE | ETH_DMAIER_AISE | ETH_DMAIER_RBUIE | ETH_DMAIER_RIE;
  
  /* 仅调用NVIC->ISER设置的默认优先级也是最高优先级0 */
  NVIC_SetPriority(ETH_IRQn, 0);
}

/*--------------------------- int_enable_eth ---------------------------------*/

void int_enable_eth (void) {
  /* Ethernet Interrupt Enable function. */
  NVIC->ISER[1] = 1 << 29;
}


/*--------------------------- int_disable_eth --------------------------------*/

void int_disable_eth (void) {
  /* Ethernet Interrupt Disable function. */
  NVIC->ICER[1] = 1 << 29;
}


/*--------------------------- send_frame -------------------------------------*/
extern OS_TID HandleTaskTCPMain;
void send_frame (OS_FRAME *frame) {
  /* Send frame to ETH ethernet controller */
  U32 *sp,*dp;
  U32 i,j;

  j = TxBufIndex;
  /* Wait until previous packet transmitted. */
  while (Tx_Desc[j].CtrlStat & DMA_TX_OWN);

  sp = (U32 *)&frame->data[0];
  dp = (U32 *)(Tx_Desc[j].Addr & ~3);

  /* Copy frame data to ETH IO buffer. */
  for (i = (frame->length + 3) >> 2; i; i--) {
    *dp++ = *sp++;
  }
  Tx_Desc[j].Size      = frame->length;
  Tx_Desc[j].CtrlStat |= DMA_TX_OWN;
  if (++j == NUM_TX_BUF) j = 0;
  TxBufIndex = j;
  /* Start frame transmission. */
  ETH->DMASR   = DSR_TPSS;
  ETH->DMATPDR = 0;
  
  os_evt_set(0x0001, HandleTaskTCPMain);
}


/*--------------------------- interrupt_ethernet -----------------------------*/

void ETH_IRQHandler (void) {
  OS_FRAME *frame;
  U32 i, RxLen;
  U32 *sp,*dp;

  i = RxBufIndex;
  do {
    if (Rx_Desc[i].Stat & DMA_RX_ERROR_MASK) {
      goto rel;
    }
    if ((Rx_Desc[i].Stat & DMA_RX_SEG_MASK) != DMA_RX_SEG_MASK) {
      goto rel;
    }
    RxLen = ((Rx_Desc[i].Stat >> 16) & 0x3FFF) - 4;
    if (RxLen > ETH_MTU) {
      /* Packet too big, ignore it and free buffer. */
      goto rel;
    }
    /* Flag 0x80000000 to skip sys_error() call when out of memory. */
    frame = alloc_mem (RxLen | 0x80000000);
    /* if 'alloc_mem()' has failed, ignore this packet. */
    if (frame != NULL) {
      sp = (U32 *)(Rx_Desc[i].Addr & ~3);
      dp = (U32 *)&frame->data[0];
      for (RxLen = (RxLen + 3) >> 2; RxLen; RxLen--) {
        *dp++ = *sp++;
      }
      put_in_queue (frame);
    }
    /* Release this frame from ETH IO buffer. */
rel:Rx_Desc[i].Stat = DMA_RX_OWN;

    if (++i == NUM_RX_BUF) i = 0;
  }
  while (!(Rx_Desc[i].Stat & DMA_RX_OWN));
  RxBufIndex = i;

  if (ETH->DMASR & INT_RBUIE) {
    /* Receive buffer unavailable, resume DMA */
    ETH->DMASR = ETH_DMASR_RBUS;
    ETH->DMARPDR = 0;
  }
  /* Clear pending interrupt bits */
  ETH->DMASR = ETH_DMASR_NIS | ETH_DMASR_AIS | ETH_DMASR_RS;
  
  isr_evt_set(0x0001, HandleTaskTCPMain);
}


/*--------------------------- rx_descr_init ----------------------------------*/

static void rx_descr_init (void) {
  /* Initialize Receive DMA Descriptor array. */
  U32 i,next;

  RxBufIndex = 0;
  for (i = 0, next = 0; i < NUM_RX_BUF; i++) {
    if (++next == NUM_RX_BUF) next = 0;
    Rx_Desc[i].Stat = DMA_RX_OWN;
    Rx_Desc[i].Ctrl = DMA_RX_RCH | ETH_BUF_SIZE;
    Rx_Desc[i].Addr = (U32)&rx_buf[i];
    Rx_Desc[i].Next = (U32)&Rx_Desc[next];
  }
  /* 接收描述符列表地址寄存器会指向接收描述符列表的起始处 */
  ETH->DMARDLAR = (U32)&Rx_Desc[0];
}



/*--------------------------- tx_descr_init ----------------------------------*/

static void tx_descr_init (void) {
  /* Initialize Transmit DMA Descriptor array. */
  U32 i,next;

  TxBufIndex = 0;
  for (i = 0, next = 0; i < NUM_TX_BUF; i++) {
    if (++next == NUM_TX_BUF) next = 0;
    Tx_Desc[i].CtrlStat = DMA_TX_TCH | DMA_TX_LS | DMA_TX_FS;
    Tx_Desc[i].Addr     = (U32)&tx_buf[i];
    Tx_Desc[i].Next     = (U32)&Tx_Desc[next];
  }
  /* 发送描述符列表地址寄存器会指向发送描述符列表的起始处 */
  ETH->DMATDLAR = (U32)&Tx_Desc[0];
}


/*--------------------------- write_PHY --------------------------------------*/

static void write_PHY (U32 PhyReg, U16 Value) {
  /* Write a data 'Value' to PHY register 'PhyReg'. */
  U32 tout;

  /* 在某次管理写操作之前要写入PHY的16位数据 */
  ETH->MACMIIDR = Value;
  ETH->MACMIIAR = PHY_DEF_ADDR << 11 | PhyReg << 6 | MMAR_MW | MMAR_MB;

  /* Wait utill operation completed */
  tout = 0;
  for (tout = 0; tout < MII_WR_TOUT; tout++) {
    if ((ETH->MACMIIAR & MMAR_MB) == 0) {
      break;
    }
  }
}


/*--------------------------- read_PHY ---------------------------------------*/

static U16 read_PHY (U32 PhyReg) {
  /* Read a PHY register 'PhyReg'. */
  U32 tout;

  /*
	PHY_DEF_ADDR：PHY地址
	PhyReg ：这些位在所选PHY器件中选择需要的MII寄存器。
	MMAR_MB：MII忙碌 。
	向ETH_MACMIIAR和ETH_MACMIIDR写入前，此位应读取逻辑0。向ETH_MACMIIAR写入过程中，此
	位也必须复位为0。在PHY寄存器访问过程中，此位由应用程序设为0b1，指示读或写访问正在
	进行中。在对PHY进行写操作过程中，ETH_MACMIIDR（MII数据）应始终保持有效，直到MAC将
	此位清零。在对PHY进行读操作过程中，ETH_MACMIIDR 始终无效，直到MAC将此位清零。在此
	位清零后，才可以向ETH_MACMIIAR（MII 地址）写入。
  */
  ETH->MACMIIAR = PHY_DEF_ADDR << 11 | PhyReg << 6 | MMAR_MB;

  /* Wait until operation completed */
  tout = 0;
  for (tout = 0; tout < MII_RD_TOUT; tout++) {
    if ((ETH->MACMIIAR & MMAR_MB) == 0) {
      break;
    }
  }
  /* 在某次管理读操作之后从 PHY 中读取的 16 位数据值 */
  return (ETH->MACMIIDR & MMDR_MD);
}

/*--------------------------- added by eric2013 ---------------------------------------*/
/*--------------------------- Eth_Link_EXTIConfig ---------------------------------------*/
static void Eth_Link_EXTIConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* 安富莱STM32-V6开发板使用PH6作为中断输入口, 下降沿表示中断信号 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* 配置中断引脚是输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	/* 配置外部中断线连接到相应引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOH, EXTI_PinSource6);

	/* 配置外部中断线 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* 使能中断通道 */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*--------------------------- EXTI9_5_IRQHandler ---------------------------------------*/
//#define ETH_CONSTATUS
#define ETH_CONNECT    "ETH_LINK Connect\r\n"
#define ETH_DISCONNECT "ETH_LINK Disconnect\r\n"
void EXTI9_5_IRQHandler(void)
{
	U32 regv, tout;
	
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		/* 可以考虑在此处加入延迟，有时连接状态变了，但是寄存器没有及时更新*/
		regv = read_PHY(PHY_REG_INTERRUPT);
		if(regv & (1 << 2))
		{
			/* 重新插入后要多读几次，保证寄存器BMSR被更新 */
			for(tout = 0; tout < 10; tout++) 
			{
				regv = read_PHY (PHY_REG_BMSR);
				if (regv & (1 << 2)) 
				{
					break;
				}
			}

			if(regv & (1 << 2)) 
			{
				#ifdef ETH_CONSTATUS
					const char *pError = ETH_CONNECT;
					uint8_t i;
				#endif
				
				g_ucEthLinkStatus = 1;
				
				#ifdef ETH_CONSTATUS
					for (i = 0; i < sizeof(ETH_CONNECT); i++)
					{
						USART1->DR = pError[i];
						/* 等待发送结束 */
						while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET);
					}
				#endif 
			}
			else
			{
				#ifdef ETH_CONSTATUS
					const char *pError = ETH_DISCONNECT;
					uint8_t i;
				#endif
				
				g_ucEthLinkStatus = 0;
				
				#ifdef ETH_CONSTATUS
					for (i = 0; i < sizeof(ETH_DISCONNECT); i++)
					{
						USART1->DR = pError[i];
						/* 等待发送结束 */
						while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET);
					}
				#endif
				
			}
			
		}
		/* 清中断挂起位 */
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
}

/*-----------------------------------------------------------------------------
 * end of file
 *----------------------------------------------------------------------------*/

