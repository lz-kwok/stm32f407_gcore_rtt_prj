/*-----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *-----------------------------------------------------------------------------
 *      Name:    SDIO_STM32F4xx.h
 *      Purpose: SD/SDIO MMC Interface Driver for ST STM32F4xx Definitions
 *      Rev.:    V4.24
 *-----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2011 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef __SDIO_STM32F4XX_H
#define __SDIO_STM32F4XX_H

/* SD Card communication speed */
#define SD_CLK              24000000

/* Wait timeouts, in multiples of 6 byte send over MCI (for 1 bit mode)       */
#define WR_TOUT             100000              /* ~ 200 ms at MCI clk 24MHz  */
#define RD_STOP_TOUT        100                 /* ~ 200 us at MCI clk 24MHz  */
#define DATA_RD_TOUT_VALUE  (100*(SD_CLK/1000)) /* ~100ms at 24MHz SD clock   */
#define DATA_WR_TOUT_VALUE  (200*(SD_CLK/1000)) /* ~200ms at 24MHz SD clock   */
#define WAIT_2SD_CLK(ck)    (ck/(SD_CLK*2)+1)   /* ~2 SD clocks wait time     */
#define WAIT_CNT(ck,us)     ((ck/5000000)*us)   /* delay in for loop cycles   */

#define SDIO_STA_CLEAR_MASK     0x00C007FF

#endif /* __SDIO_STM32F4XX_H */

/*-----------------------------------------------------------------------------
 * end of file
 *----------------------------------------------------------------------------*/
