/*
*********************************************************************************************************
*
*	模块名称 : Host STM32F40x Driver header file 
*	文件名称 : usbh_stm32f40x.h
*	版    本 : V1.0
*	说    明 : KEIL官方添加存在于stm32f4xx.h文件里面的寄存器定义到这个文件里面
*
*	修改记录 :
*		版本号    日期         作者            说明
*       V1.0    2015-12-15   Eric2013          首发
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
/*------------------------------------------------------------------------------
 *      RL-ARM - USB
 *------------------------------------------------------------------------------
 *      Name:    usbh_stm32f40x.h
 *      Purpose: Host STM32F40x Driver header file
 *      Rev.:    V4.70
 *------------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef __USBH_STM32_H__
#define __USBH_STM32_H__

#include "stm32f4xx.h"
#pragma anon_unions

/**
  * @brief USB OTG FS
  */
typedef struct
{
  __IO uint32_t GOTGCTL;
  __IO uint32_t GOTGINT;
  __IO uint32_t GAHBCFG;
  __IO uint32_t GUSBCFG;
  __IO uint32_t GRSTCTL;
  __IO uint32_t GINTSTS;
  __IO uint32_t GINTMSK;
  __IO uint32_t GRXSTSR;
  __IO uint32_t GRXSTSP;
  __IO uint32_t GRXFSIZ;
  union {
    __IO uint32_t HNPTXFSIZ;
    __IO uint32_t DIEPTXF0;
  };
  __IO uint32_t HNPTXSTS;
  __IO uint32_t GI2CCTL;
       uint32_t RESERVED0;
  __IO uint32_t GCCFG;
  __IO uint32_t CID;
       uint32_t RESERVED1[48];
  __IO uint32_t HPTXFSIZ;
  __IO uint32_t DIEPTXF1;
  __IO uint32_t DIEPTXF2;
  __IO uint32_t DIEPTXF3;
  __IO uint32_t DIEPTXF4;
  __IO uint32_t DIEPTXF5;
  __IO uint32_t DIEPTXF6;
  __IO uint32_t DIEPTXF7;
  __IO uint32_t DIEPTXF8;
  __IO uint32_t DIEPTXF9;
  __IO uint32_t DIEPTXF10;
  __IO uint32_t DIEPTXF11;
  __IO uint32_t DIEPTXF12;
  __IO uint32_t DIEPTXF13;
  __IO uint32_t DIEPTXF14;
  __IO uint32_t DIEPTXF15;
       uint32_t RESERVED2[176];
  __IO uint32_t HCFG;
  __IO uint32_t HFIR;
  __IO uint32_t HFNUM;
       uint32_t RESERVED3;
  __IO uint32_t HPTXSTS;
  __IO uint32_t HAINT;
  __IO uint32_t HAINTMSK;
       uint32_t RESERVED4[9];
  __IO uint32_t HPRT;
       uint32_t RESERVED5[47];
  __IO uint32_t HCCHAR0;
       uint32_t RESERVED6;
  __IO uint32_t HCINT0;
  __IO uint32_t HCINTMSK0;
  __IO uint32_t HCTSIZ0;
       uint32_t RESERVED7[3];
  __IO uint32_t HCCHAR1;
       uint32_t RESERVED8;
  __IO uint32_t HCINT1;
  __IO uint32_t HCINTMSK1;
  __IO uint32_t HCTSIZ1;
       uint32_t RESERVED9[3];
  __IO uint32_t HCCHAR2;
       uint32_t RESERVED10;
  __IO uint32_t HCINT2;
  __IO uint32_t HCINTMSK2;
  __IO uint32_t HCTSIZ2;
       uint32_t RESERVED11[3];
  __IO uint32_t HCCHR3;
       uint32_t RESERVED12;
  __IO uint32_t HCINT3;
  __IO uint32_t HCINTMSK3;
  __IO uint32_t HCTSIZ3;
       uint32_t RESERVED13[3];
  __IO uint32_t HCCHR4;
       uint32_t RESERVED14;
  __IO uint32_t HCINT4;
  __IO uint32_t HCINTMSK4;
  __IO uint32_t HCTSIZ4;
       uint32_t RESERVED15[3];
  __IO uint32_t HCCHR5;
       uint32_t RESERVED16;
  __IO uint32_t HCINT5;
  __IO uint32_t HCINTMSK5;
  __IO uint32_t HCTSIZ5;
       uint32_t RESERVED17[3];
  __IO uint32_t HCCHR6;
       uint32_t RESERVED18;
  __IO uint32_t HCINT6;
  __IO uint32_t HCINTMSK6;
  __IO uint32_t HCTSIZ6;
       uint32_t RESERVED19[3];
  __IO uint32_t HCCHR7;
       uint32_t RESERVED20;
  __IO uint32_t HCINT7;
  __IO uint32_t HCINTMSK7;
  __IO uint32_t HCTSIZ7;
       uint32_t RESERVED21[3];
  __IO uint32_t HCCHR8;
       uint32_t RESERVED22;
  __IO uint32_t HCINT8;
  __IO uint32_t HCINTMSK8;
  __IO uint32_t HCTSIZ8;
       uint32_t RESERVED23[3];
  __IO uint32_t HCCHR9;
       uint32_t RESERVED24;
  __IO uint32_t HCINT9;
  __IO uint32_t HCINTMSK9;
  __IO uint32_t HCTSIZ9;
       uint32_t RESERVED25[3];
  __IO uint32_t HCCHR10;
       uint32_t RESERVED26;
  __IO uint32_t HCINT10;
  __IO uint32_t HCINTMSK10;
  __IO uint32_t HCTSIZ10;
       uint32_t RESERVED27[3];
  __IO uint32_t HCCHR11;
       uint32_t RESERVED28;
  __IO uint32_t HCINT11;
  __IO uint32_t HCINTMSK11;
  __IO uint32_t HCTSIZ11;
       uint32_t RESERVED29[3];
  __IO uint32_t HCCHR12;
       uint32_t RESERVED30;
  __IO uint32_t HCINT12;
  __IO uint32_t HCINTMSK12;
  __IO uint32_t HCTSIZ12;
       uint32_t RESERVED31[3];
  __IO uint32_t HCCHR13;
       uint32_t RESERVED32;
  __IO uint32_t HCINT13;
  __IO uint32_t HCINTMSK13;
  __IO uint32_t HCTSIZ13;
       uint32_t RESERVED33[3];
  __IO uint32_t HCCHR14;
       uint32_t RESERVED34;
  __IO uint32_t HCINT14;
  __IO uint32_t HCINTMSK14;
  __IO uint32_t HCTSIZ14;
       uint32_t RESERVED35[3];
  __IO uint32_t HCCHR15;
       uint32_t RESERVED36;
  __IO uint32_t HCINT15;
  __IO uint32_t HCINTMSK15;
  __IO uint32_t HCTSIZ15;
       uint32_t RESERVED37[3];
       uint32_t RESERVED38[64];
  __IO uint32_t DCFG;
  __IO uint32_t DCTL;
  __IO uint32_t DSTS;
       uint32_t RESERVED39;
  __IO uint32_t DIEPMSK;
  __IO uint32_t DOEPMSK;
  __IO uint32_t DAINT;
  __IO uint32_t DAINTMSK;
       uint32_t RESERVED40[2];
  __IO uint32_t DVBUSDIS;
  __IO uint32_t DVBUSPULSE;
       uint32_t RESERVED41;
  __IO uint32_t DIEPEMPMSK;
       uint32_t RESERVED42[50];
  __IO uint32_t DIEPCTL0;
       uint32_t RESERVED43;
  __IO uint32_t DIEPINT0;
       uint32_t RESERVED44;
  __IO uint32_t DIEPTSIZ0;
       uint32_t RESERVED45;
  __IO uint32_t DTXFSTS0;
       uint32_t RESERVED46;
  __IO uint32_t DIEPCTL1;
       uint32_t RESERVED47;
  __IO uint32_t DIEPINT1;
       uint32_t RESERVED48;
  __IO uint32_t DIEPTSIZ1;
       uint32_t RESERVED49;
  __IO uint32_t DTXFSTS1;
       uint32_t RESERVED50;
  __IO uint32_t DIEPCTL2;
       uint32_t RESERVED51;
  __IO uint32_t DIEPINT2;
       uint32_t RESERVED52;
  __IO uint32_t DIEPTSIZ2;
       uint32_t RESERVED53;
  __IO uint32_t DTXFSTS2;
       uint32_t RESERVED54;
  __IO uint32_t DIEPCTL3;
       uint32_t RESERVED55;
  __IO uint32_t DIEPINT3;
       uint32_t RESERVED56;
  __IO uint32_t DIEPTSIZ3;
       uint32_t RESERVED57;
  __IO uint32_t DTXFSTS3;
       uint32_t RESERVED58;
  __IO uint32_t DIEPCTL4;
       uint32_t RESERVED59;
  __IO uint32_t DIEPINT4;
       uint32_t RESERVED60;
  __IO uint32_t DIEPTSIZ4;
       uint32_t RESERVED61;
  __IO uint32_t DTXFSTS4;
       uint32_t RESERVED62;
  __IO uint32_t DIEPCTL5;
       uint32_t RESERVED63;
  __IO uint32_t DIEPINT5;
       uint32_t RESERVED64;
  __IO uint32_t DIEPTSIZ5;
       uint32_t RESERVED65[3];
  __IO uint32_t DIEPCTL6;
       uint32_t RESERVED66;
  __IO uint32_t DIEPINT6;
       uint32_t RESERVED67;
  __IO uint32_t DIEPTSIZ6;
       uint32_t RESERVED68[3];
  __IO uint32_t DIEPCTL7;
       uint32_t RESERVED69;
  __IO uint32_t DIEPINT7;
       uint32_t RESERVED70;
  __IO uint32_t DIEPTSIZ7;
       uint32_t RESERVED71[3];
  __IO uint32_t DIEPCTL8;
       uint32_t RESERVED72;
  __IO uint32_t DIEPINT8;
       uint32_t RESERVED73;
  __IO uint32_t DIEPTSIZ8;
       uint32_t RESERVED74[3];
  __IO uint32_t DIEPCTL9;
       uint32_t RESERVED75;
  __IO uint32_t DIEPINT9;
       uint32_t RESERVED76;
  __IO uint32_t DIEPTSIZ9;
       uint32_t RESERVED77[3];
  __IO uint32_t DIEPCTL10;
       uint32_t RESERVED78;
  __IO uint32_t DIEPINT10;
       uint32_t RESERVED79;
  __IO uint32_t DIEPTSIZ10;
       uint32_t RESERVED80[3];
  __IO uint32_t DIEPCTL11;
       uint32_t RESERVED81;
  __IO uint32_t DIEPINT11;
       uint32_t RESERVED82;
  __IO uint32_t DIEPTSIZ11;
       uint32_t RESERVED83[3];
  __IO uint32_t DIEPCTL12;
       uint32_t RESERVED84;
  __IO uint32_t DIEPINT12;
       uint32_t RESERVED85;
  __IO uint32_t DIEPTSIZ86;
       uint32_t RESERVED86[3];
  __IO uint32_t DIEPCTL13;
       uint32_t RESERVED87;
  __IO uint32_t DIEPINT13;
       uint32_t RESERVED88;
  __IO uint32_t DIEPTSIZ13;
       uint32_t RESERVED89[3];
  __IO uint32_t DIEPCTL14;
       uint32_t RESERVED90;
  __IO uint32_t DIEPINT14;
       uint32_t RESERVED91;
  __IO uint32_t DIEPTSIZ14;
       uint32_t RESERVED92[3];
  __IO uint32_t DIEPCTL15;
       uint32_t RESERVED93;
  __IO uint32_t DIEPINT15;
       uint32_t RESERVED94;
  __IO uint32_t DIEPTSIZ15;
       uint32_t RESERVED95[3];
  __IO uint32_t DOEPCTL0;
       uint32_t RESERVED96;
  __IO uint32_t DOEPINT0;
       uint32_t RESERVED97;
  __IO uint32_t DOEPTSIZ0;
       uint32_t RESERVED98[3];
  __IO uint32_t DOEPCTL1;
       uint32_t RESERVED99;
  __IO uint32_t DOEPINT1;
       uint32_t RESERVED100;
  __IO uint32_t DOEPTSIZ1;
       uint32_t RESERVED101[3];
  __IO uint32_t DOEPCTL2;
       uint32_t RESERVED102;
  __IO uint32_t DOEPINT2;
       uint32_t RESERVED103;
  __IO uint32_t DOEPTSIZ2;
       uint32_t RESERVED104[3];
  __IO uint32_t DOEPCTL3;
       uint32_t RESERVED105;
  __IO uint32_t DOEPINT3;
       uint32_t RESERVED106;
  __IO uint32_t DOEPTSIZ3;
       uint32_t RESERVED107[3];
  __IO uint32_t DOEPCTL4;
       uint32_t RESERVED108;
  __IO uint32_t DOEPINT4;
       uint32_t RESERVED109;
  __IO uint32_t DOEPTSIZ4;
       uint32_t RESERVED110[3];
  __IO uint32_t DOEPCTL5;
       uint32_t RESERVED111;
  __IO uint32_t DOEPINT5;
       uint32_t RESERVED112;
  __IO uint32_t DOEPTSIZ5;
       uint32_t RESERVED113[3];
  __IO uint32_t DOEPCTL6;
       uint32_t RESERVED114;
  __IO uint32_t DOEPINT6;
       uint32_t RESERVED115;
  __IO uint32_t DOEPTSIZ6;
       uint32_t RESERVED116[3];
  __IO uint32_t DOEPCTL7;
       uint32_t RESERVED117;
  __IO uint32_t DOEPINT7;
       uint32_t RESERVED118;
  __IO uint32_t DOEPTSIZ7;
       uint32_t RESERVED119[3];
  __IO uint32_t DOEPCTL8;
       uint32_t RESERVED120;
  __IO uint32_t DOEPINT8;
       uint32_t RESERVED121;
  __IO uint32_t DOEPTSIZ8;
       uint32_t RESERVED122[3];
  __IO uint32_t DOEPCTL9;
       uint32_t RESERVED123;
  __IO uint32_t DOEPINT9;
       uint32_t RESERVED124;
  __IO uint32_t DOEPTSIZ9;
       uint32_t RESERVED125[3];
  __IO uint32_t DOEPCTL10;
       uint32_t RESERVED126;
  __IO uint32_t DOEPINT10;
       uint32_t RESERVED127;
  __IO uint32_t DOEPTSIZ10;
       uint32_t RESERVED128[3];
  __IO uint32_t DOEPCTL11;
       uint32_t RESERVED129;
  __IO uint32_t DOEPINT11;
       uint32_t RESERVED130;
  __IO uint32_t DOEPTSIZ11;
       uint32_t RESERVED131[3];
  __IO uint32_t DOEPCTL12;
       uint32_t RESERVED132;
  __IO uint32_t DOEPINT12;
       uint32_t RESERVED133;
  __IO uint32_t DOEPTSIZ12;
       uint32_t RESERVED134[3];
  __IO uint32_t DOEPCTL13;
       uint32_t RESERVED135;
  __IO uint32_t DOEPINT13;
       uint32_t RESERVED136;
  __IO uint32_t DOEPTSIZ13;
       uint32_t RESERVED137[3];
  __IO uint32_t DOEPCTL14;
       uint32_t RESERVED138;
  __IO uint32_t DOEPINT14;
       uint32_t RESERVED139;
  __IO uint32_t DOEPTSIZ14;
       uint32_t RESERVED140[3];
  __IO uint32_t DOEPCTL15;
       uint32_t RESERVED141;
  __IO uint32_t DOEPINT15;
       uint32_t RESERVED142;
  __IO uint32_t DOEPTSIZ15;
       uint32_t RESERVED143[3];
       uint32_t RESERVED144[64];
  __IO uint32_t PCGCCTL;
} OTG_FS_TypeDef;

/** 
  * @brief USB OTG HS
  */

typedef struct
{
  __IO uint32_t GOTGCTL;
  __IO uint32_t GOTGINT;
  __IO uint32_t GAHBCFG;
  __IO uint32_t GUSBCFG;
  __IO uint32_t GRSTCTL;
  __IO uint32_t GINTSTS;
  __IO uint32_t GINTMSK;
  __IO uint32_t GRXSTSR;
  __IO uint32_t GRXSTSP;
  __IO uint32_t GRXFSIZ;
  union {
    __IO uint32_t GNPTXFSIZ;
    __IO uint32_t TX0FSIZ;
  };
  __IO uint32_t GNPTXSTS;
  __IO uint32_t GI2CCTL;
       uint32_t RESERVED0;
  __IO uint32_t GCCFG;
  __IO uint32_t CID;
       uint32_t RESERVED1[48];
  __IO uint32_t HPTXFSIZ;
  __IO uint32_t DIEPTXF1;
  __IO uint32_t DIEPTXF2;
  __IO uint32_t DIEPTXF3;
  __IO uint32_t DIEPTXF4;
  __IO uint32_t DIEPTXF5;
  __IO uint32_t DIEPTXF6;
  __IO uint32_t DIEPTXF7;
  __IO uint32_t DIEPTXF8;
  __IO uint32_t DIEPTXF9;
  __IO uint32_t DIEPTXF10;
  __IO uint32_t DIEPTXF11;
  __IO uint32_t DIEPTXF12;
  __IO uint32_t DIEPTXF13;
  __IO uint32_t DIEPTXF14;
  __IO uint32_t DIEPTXF15;
       uint32_t RESERVED2[176];
  __IO uint32_t HCFG;
  __IO uint32_t HFIR;
  __IO uint32_t HFNUM;
       uint32_t RESERVED3;
  __IO uint32_t HPTXSTS;
  __IO uint32_t HAINT;
  __IO uint32_t HAINTMSK;
       uint32_t RESERVED4[9];
  __IO uint32_t HPRT;
       uint32_t RESERVED5[47];
  __IO uint32_t HCCHAR0;
  __IO uint32_t HCSPLT0;
  __IO uint32_t HCINT0;
  __IO uint32_t HCINTMSK0;
  __IO uint32_t HCTSIZ0;
  __IO uint32_t HCDMA0;
       uint32_t RESERVED6[2];
  __IO uint32_t HCCHAR1;
  __IO uint32_t HCSPLT1;
  __IO uint32_t HCINT1;
  __IO uint32_t HCINTMSK1;
  __IO uint32_t HCTSIZ1;
  __IO uint32_t HCDMA1;
       uint32_t RESERVED7[2];
  __IO uint32_t HCCHAR2;
  __IO uint32_t HCSPLT2;
  __IO uint32_t HCINT2;
  __IO uint32_t HCINTMSK2;
  __IO uint32_t HCTSIZ2;
  __IO uint32_t HCDMA2;
       uint32_t RESERVED8[2];
  __IO uint32_t HCCHAR3;
  __IO uint32_t HCSPLT3;
  __IO uint32_t HCINT3;
  __IO uint32_t HCINTMSK3;
  __IO uint32_t HCTSIZ3;
  __IO uint32_t HCDMA3;
       uint32_t RESERVED9[2];
  __IO uint32_t HCCHAR4;
  __IO uint32_t HCSPLT4;
  __IO uint32_t HCINT4;
  __IO uint32_t HCINTMSK4;
  __IO uint32_t HCTSIZ4;
  __IO uint32_t HCDMA4;
       uint32_t RESERVED10[2];
  __IO uint32_t HCCHAR5;
  __IO uint32_t HCSPLT5;
  __IO uint32_t HCINT5;
  __IO uint32_t HCINTMSK5;
  __IO uint32_t HCTSIZ5;
  __IO uint32_t HCDMA5;
       uint32_t RESERVED11[2];
  __IO uint32_t HCCHAR6;
  __IO uint32_t HCSPLT6;
  __IO uint32_t HCINT6;
  __IO uint32_t HCINTMSK6;
  __IO uint32_t HCTSIZ6;
  __IO uint32_t HCDMA6;
       uint32_t RESERVED12[2];
  __IO uint32_t HCCHAR7;
  __IO uint32_t HCSPLT7;
  __IO uint32_t HCINT7;
  __IO uint32_t HCINTMSK7;
  __IO uint32_t HCTSIZ7;
  __IO uint32_t HCDMA7;
       uint32_t RESERVED13[2];
  __IO uint32_t HCCHAR8;
  __IO uint32_t HCSPLT8;
  __IO uint32_t HCINT8;
  __IO uint32_t HCINTMSK8;
  __IO uint32_t HCTSIZ8;
  __IO uint32_t HCDMA8;
       uint32_t RESERVED14[2];
  __IO uint32_t HCCHAR9;
  __IO uint32_t HCSPLT9;
  __IO uint32_t HCINT9;
  __IO uint32_t HCINTMSK9;
  __IO uint32_t HCTSIZ9;
  __IO uint32_t HCDMA9;
       uint32_t RESERVED15[2];
  __IO uint32_t HCCHAR10;
  __IO uint32_t HCSPLT10;
  __IO uint32_t HCINT10;
  __IO uint32_t HCINTMSK10;
  __IO uint32_t HCTSIZ10;
  __IO uint32_t HCDMA10;
       uint32_t RESERVED16[2];
  __IO uint32_t HCCHAR11;
  __IO uint32_t HCSPLT11;
  __IO uint32_t HCINT11;
  __IO uint32_t HCINTMSK11;
  __IO uint32_t HCTSIZ11;
  __IO uint32_t HCDMA11;
       uint32_t RESERVED17[2];
  __IO uint32_t HCCHAR12;
  __IO uint32_t HCSPLT12;
  __IO uint32_t HCINT12;
  __IO uint32_t HCINTMSK12;
  __IO uint32_t HCTSIZ12;
  __IO uint32_t HCDMA12;
       uint32_t RESERVED18[2];
  __IO uint32_t HCCHAR13;
  __IO uint32_t HCSPLT13;
  __IO uint32_t HCINT13;
  __IO uint32_t HCINTMSK13;
  __IO uint32_t HCTSIZ13;
  __IO uint32_t HCDMA13;
       uint32_t RESERVED19[2];
  __IO uint32_t HCCHAR14;
  __IO uint32_t HCSPLT14;
  __IO uint32_t HCINT14;
  __IO uint32_t HCINTMSK14;
  __IO uint32_t HCTSIZ14;
  __IO uint32_t HCDMA14;
       uint32_t RESERVED20[2];
  __IO uint32_t HCCHAR15;
  __IO uint32_t HCSPLT15;
  __IO uint32_t HCINT15;
  __IO uint32_t HCINTMSK15;
  __IO uint32_t HCTSIZ15;
  __IO uint32_t HCDMA15;
       uint32_t RESERVED21[2];
       uint32_t RESERVED22[64];
  __IO uint32_t DCFG;
  __IO uint32_t DCTL;
  __IO uint32_t DSTS;
       uint32_t RESERVED23;
  __IO uint32_t DIEPMSK;
  __IO uint32_t DOEPMSK;
  __IO uint32_t DAINT;
  __IO uint32_t DAINTMSK;
       uint32_t RESERVED24[2];
  __IO uint32_t DVBUSDIS;
  __IO uint32_t DVBUSPULSE;
       uint32_t RESERVED25;
  __IO uint32_t DIEPEMPMSK;
  __IO uint32_t EACHHINT;
  __IO uint32_t EACHHINTMSK;
  __IO uint32_t DIEPEACHMSK1;
       uint32_t RESERVED26[15];
  __IO uint32_t DOEPEACHMSK1;
       uint32_t RESERVED27[31];
  __IO uint32_t DIEPCTL0;
       uint32_t RESERVED28;
  __IO uint32_t DIEPINT0;
       uint32_t RESERVED29;
  __IO uint32_t DIEPTSIZ0;
  __IO uint32_t DIEPDMA0;
  __IO uint32_t DTXFSTS0;
       uint32_t RESERVED30;
  __IO uint32_t DIEPCTL1;
       uint32_t RESERVED31;
  __IO uint32_t DIEPINT1;
       uint32_t RESERVED32;
  __IO uint32_t DIEPTSIZ1;
  __IO uint32_t DIEPDMA1;
  __IO uint32_t DTXFSTS1;
       uint32_t RESERVED33;
  __IO uint32_t DIEPCTL2;
       uint32_t RESERVED34;
  __IO uint32_t DIEPINT2;
       uint32_t RESERVED35;
  __IO uint32_t DIEPTSIZ2;
  __IO uint32_t DIEPDMA2;
  __IO uint32_t DTXFSTS2;
       uint32_t RESERVED36;
  __IO uint32_t DIEPCTL3;
       uint32_t RESERVED37;
  __IO uint32_t DIEPINT3;
       uint32_t RESERVED38;
  __IO uint32_t DIEPTSIZ3;
  __IO uint32_t DIEPDMA3;
  __IO uint32_t DTXFSTS3;
       uint32_t RESERVED39;
  __IO uint32_t DIEPCTL4;
       uint32_t RESERVED40;
  __IO uint32_t DIEPINT4;
       uint32_t RESERVED41;
  __IO uint32_t DIEPTSIZ4;
  __IO uint32_t DIEPDMA4;
  __IO uint32_t DTXFSTS4;
       uint32_t RESERVED42;
  __IO uint32_t DIEPCTL5;
       uint32_t RESERVED43;
  __IO uint32_t DIEPINT5;
       uint32_t RESERVED44;
  __IO uint32_t DIEPTSIZ5;
  __IO uint32_t DIEPDMA5;
  __IO uint32_t DTXFSTS5;
       uint32_t RESERVED45;
  __IO uint32_t DIEPCTL6;
       uint32_t RESERVED46;
  __IO uint32_t DIEPINT6;
       uint32_t RESERVED47;
  __IO uint32_t DIEPTSIZ6;
  __IO uint32_t DIEPDMA6;
  __IO uint32_t DTXFSTS6;
       uint32_t RESERVED48;
  __IO uint32_t DIEPCTL7;
       uint32_t RESERVED49;
  __IO uint32_t DIEPINT7;
       uint32_t RESERVED50;
  __IO uint32_t DIEPTSIZ7;
  __IO uint32_t DIEPDMA7;
  __IO uint32_t DTXFSTS7;
       uint32_t RESERVED51;
  __IO uint32_t DIEPCTL8;
       uint32_t RESERVED52;
  __IO uint32_t DIEPINT8;
       uint32_t RESERVED53;
  __IO uint32_t DIEPTSIZ8;
  __IO uint32_t DIEPDMA8;
  __IO uint32_t DTXFSTS8;
       uint32_t RESERVED54;
  __IO uint32_t DIEPCTL9;
       uint32_t RESERVED55;
  __IO uint32_t DIEPINT9;
       uint32_t RESERVED56;
  __IO uint32_t DIEPTSIZ9;
  __IO uint32_t DIEPDMA9;
  __IO uint32_t DTXFSTS9;
       uint32_t RESERVED57;
  __IO uint32_t DIEPCTL10;
       uint32_t RESERVED58;
  __IO uint32_t DIEPINT10;
       uint32_t RESERVED59;
  __IO uint32_t DIEPTSIZ10;
  __IO uint32_t DIEPDMA10;
  __IO uint32_t DTXFSTS10;
       uint32_t RESERVED60;
  __IO uint32_t DIEPCTL11;
       uint32_t RESERVED61;
  __IO uint32_t DIEPINT11;
       uint32_t RESERVED62;
  __IO uint32_t DIEPTSIZ11;
  __IO uint32_t DIEPDMA11;
  __IO uint32_t DTXFSTS11;
       uint32_t RESERVED63;
  __IO uint32_t DIEPCTL12;
       uint32_t RESERVED64;
  __IO uint32_t DIEPINT12;
       uint32_t RESERVED65;
  __IO uint32_t DIEPTSIZ12;
  __IO uint32_t DIEPDMA12;
  __IO uint32_t DTXFSTS12;
       uint32_t RESERVED66;
  __IO uint32_t DIEPCTL13;
       uint32_t RESERVED67;
  __IO uint32_t DIEPINT13;
       uint32_t RESERVED68;
  __IO uint32_t DIEPTSIZ13;
  __IO uint32_t DIEPDMA13;
  __IO uint32_t DTXFSTS13;
       uint32_t RESERVED69;
  __IO uint32_t DIEPCTL14;
       uint32_t RESERVED70;
  __IO uint32_t DIEPINT14;
       uint32_t RESERVED71;
  __IO uint32_t DIEPTSIZ14;
  __IO uint32_t DIEPDMA14;
  __IO uint32_t DTXFSTS14;
       uint32_t RESERVED72;
  __IO uint32_t DIEPCTL15;
       uint32_t RESERVED73;
  __IO uint32_t DIEPINT15;
       uint32_t RESERVED74;
  __IO uint32_t DIEPTSIZ15;
  __IO uint32_t DIEPDMA15;
  __IO uint32_t DTXFSTS15;
       uint32_t RESERVED75;
  __IO uint32_t DOEPCTL0;
       uint32_t RESERVED76;
  __IO uint32_t DOEPINT0;
       uint32_t RESERVED77;
  __IO uint32_t DOEPTSIZ0;
  __IO uint32_t DOEPDMAB0;
       uint32_t RESERVED78[2];
  __IO uint32_t DOEPCTL1;
       uint32_t RESERVED79;
  __IO uint32_t DOEPINT1;
       uint32_t RESERVED80;
  __IO uint32_t DOEPTSIZ1;
  __IO uint32_t DOEPDMAB1;
       uint32_t RESERVED81[2];
  __IO uint32_t DOEPCTL2;
       uint32_t RESERVED82;
  __IO uint32_t DOEPINT2;
       uint32_t RESERVED83;
  __IO uint32_t DOEPTSIZ2;
  __IO uint32_t DOEPDMAB2;
       uint32_t RESERVED84[2];
  __IO uint32_t DOEPCTL3;
       uint32_t RESERVED85;
  __IO uint32_t DOEPINT3;
       uint32_t RESERVED86;
  __IO uint32_t DOEPTSIZ3;
  __IO uint32_t DOEPDMAB3;
       uint32_t RESERVED87[2];
  __IO uint32_t DOEPCTL4;
       uint32_t RESERVED88;
  __IO uint32_t DOEPINT4;
       uint32_t RESERVED89;
  __IO uint32_t DOEPTSIZ4;
  __IO uint32_t DOEPDMAB4;
       uint32_t RESERVED90[2];
  __IO uint32_t DOEPCTL5;
       uint32_t RESERVED91;
  __IO uint32_t DOEPINT5;
       uint32_t RESERVED92;
  __IO uint32_t DOEPTSIZ5;
  __IO uint32_t DOEPDMAB5;
       uint32_t RESERVED93[2];
  __IO uint32_t DOEPCTL6;
       uint32_t RESERVED94;
  __IO uint32_t DOEPINT6;
       uint32_t RESERVED95;
  __IO uint32_t DOEPTSIZ6;
  __IO uint32_t DOEPDMAB6;
       uint32_t RESERVED96[2];
  __IO uint32_t DOEPCTL7;
       uint32_t RESERVED97;
  __IO uint32_t DOEPINT7;
       uint32_t RESERVED98;
  __IO uint32_t DOEPTSIZ7;
  __IO uint32_t DOEPDMAB7;
       uint32_t RESERVED99[2];
  __IO uint32_t DOEPCTL8;
       uint32_t RESERVED100;
  __IO uint32_t DOEPINT8;
       uint32_t RESERVED101;
  __IO uint32_t DOEPTSIZ8;
  __IO uint32_t DOEPDMAB8;
       uint32_t RESERVED102[2];
  __IO uint32_t DOEPCTL9;
       uint32_t RESERVED103;
  __IO uint32_t DOEPINT9;
       uint32_t RESERVED104;
  __IO uint32_t DOEPTSIZ9;
  __IO uint32_t DOEPDMAB9;
       uint32_t RESERVED105[2];
  __IO uint32_t DOEPCTL10;
       uint32_t RESERVED106;
  __IO uint32_t DOEPINT10;
       uint32_t RESERVED107;
  __IO uint32_t DOEPTSIZ10;
  __IO uint32_t DOEPDMAB10;
       uint32_t RESERVED108[2];
  __IO uint32_t DOEPCTL11;
       uint32_t RESERVED109;
  __IO uint32_t DOEPINT11;
       uint32_t RESERVED110;
  __IO uint32_t DOEPTSIZ11;
  __IO uint32_t DOEPDMAB11;
       uint32_t RESERVED111[2];
  __IO uint32_t DOEPCTL12;
       uint32_t RESERVED112;
  __IO uint32_t DOEPINT12;
       uint32_t RESERVED113;
  __IO uint32_t DOEPTSIZ12;
  __IO uint32_t DOEPDMAB12;
       uint32_t RESERVED114[2];
  __IO uint32_t DOEPCTL13;
       uint32_t RESERVED115;
  __IO uint32_t DOEPINT13;
       uint32_t RESERVED116;
  __IO uint32_t DOEPTSIZ13;
  __IO uint32_t DOEPDMAB13;
       uint32_t RESERVED117[2];
  __IO uint32_t DOEPCTL14;
       uint32_t RESERVED118;
  __IO uint32_t DOEPINT14;
       uint32_t RESERVED119;
  __IO uint32_t DOEPTSIZ14;
  __IO uint32_t DOEPDMAB14;
       uint32_t RESERVED120[2];
  __IO uint32_t DOEPCTL15;
       uint32_t RESERVED121;
  __IO uint32_t DOEPINT15;
       uint32_t RESERVED122;
  __IO uint32_t DOEPTSIZ15;
  __IO uint32_t DOEPDMAB15;
       uint32_t RESERVED123[2];
       uint32_t RESERVED143[64];
  __IO uint32_t PCGCCTL;
} OTG_HS_TypeDef;

///////////////////////////////////////////////////////////
#define OTG_HS_BASE           (AHB1PERIPH_BASE + 0x20000)
#define OTG_HS_DFIFO0_BASE    (OTG_HS_BASE + 0x01000)
#define OTG_HS_DFIFO1_BASE    (OTG_HS_BASE + 0x02000)
#define OTG_HS_DFIFO2_BASE    (OTG_HS_BASE + 0x03000)
#define OTG_HS_DFIFO3_BASE    (OTG_HS_BASE + 0x04000)
#define OTG_HS_DFIFO4_BASE    (OTG_HS_BASE + 0x05000)
#define OTG_HS_DFIFO5_BASE    (OTG_HS_BASE + 0x06000)
#define OTG_HS_DFIFO6_BASE    (OTG_HS_BASE + 0x07000)
#define OTG_HS_DFIFO7_BASE    (OTG_HS_BASE + 0x08000)
#define OTG_HS_DFIFO8_BASE    (OTG_HS_BASE + 0x09000)
#define OTG_HS_DFIFO9_BASE    (OTG_HS_BASE + 0x0A000)
#define OTG_HS_DFIFO10_BASE   (OTG_HS_BASE + 0x0B000)
#define OTG_HS_DFIFO11_BASE   (OTG_HS_BASE + 0x0C000)
#define OTG_HS_DFIFO12_BASE   (OTG_HS_BASE + 0x0D000)
#define OTG_HS_DFIFO13_BASE   (OTG_HS_BASE + 0x0E000)
#define OTG_HS_DFIFO14_BASE   (OTG_HS_BASE + 0x0F000)
#define OTG_HS_DFIFO15_BASE   (OTG_HS_BASE + 0x10000)
///////////////////////////////////////////////////////////
#define OTG_FS_BASE           (AHB2PERIPH_BASE + 0x00000)
#define OTG_FS_DFIFO0_BASE    (OTG_FS_BASE + 0x01000)
#define OTG_FS_DFIFO1_BASE    (OTG_FS_BASE + 0x02000)
#define OTG_FS_DFIFO2_BASE    (OTG_FS_BASE + 0x03000)
#define OTG_FS_DFIFO3_BASE    (OTG_FS_BASE + 0x04000)
#define OTG_FS_DFIFO4_BASE    (OTG_FS_BASE + 0x05000)
#define OTG_FS_DFIFO5_BASE    (OTG_FS_BASE + 0x06000)
#define OTG_FS_DFIFO6_BASE    (OTG_FS_BASE + 0x07000)
#define OTG_FS_DFIFO7_BASE    (OTG_FS_BASE + 0x08000)
///////////////////////////////////////////////////////////
#define OTG_HS              ((OTG_HS_TypeDef *) OTG_HS_BASE)  
#define OTG_HS_DFIFO0       (((uint32_t *) OTG_HS_DFIFO0_BASE)  )
#define OTG_HS_DFIFO1       (((uint32_t *) OTG_HS_DFIFO1_BASE)  )
#define OTG_HS_DFIFO2       (((uint32_t *) OTG_HS_DFIFO2_BASE)  )
#define OTG_HS_DFIFO3       (((uint32_t *) OTG_HS_DFIFO3_BASE)  )
#define OTG_HS_DFIFO4       (((uint32_t *) OTG_HS_DFIFO4_BASE)  )
#define OTG_HS_DFIFO5       (((uint32_t *) OTG_HS_DFIFO5_BASE)  )
#define OTG_HS_DFIFO6       (((uint32_t *) OTG_HS_DFIFO6_BASE)  )
#define OTG_HS_DFIFO7       (((uint32_t *) OTG_HS_DFIFO7_BASE)  )
#define OTG_HS_DFIFO8       (((uint32_t *) OTG_HS_DFIFO8_BASE)  )
#define OTG_HS_DFIFO9       (((uint32_t *) OTG_HS_DFIFO9_BASE)  )
#define OTG_HS_DFIFO10      (((uint32_t *) OTG_HS_DFIFO10_BASE) )
#define OTG_HS_DFIFO11      (((uint32_t *) OTG_HS_DFIFO11_BASE) )
#define OTG_HS_DFIFO12      (((uint32_t *) OTG_HS_DFIFO12_BASE) )
#define OTG_HS_DFIFO13      (((uint32_t *) OTG_HS_DFIFO13_BASE) )
#define OTG_HS_DFIFO14      (((uint32_t *) OTG_HS_DFIFO14_BASE) )
#define OTG_HS_DFIFO15      (((uint32_t *) OTG_HS_DFIFO15_BASE) )
#define OTG_FS              ((OTG_FS_TypeDef *) OTG_FS_BASE)  
#define OTG_FS_DFIFO0       (((uint32_t *) OTG_FS_DFIFO0_BASE)  )
#define OTG_FS_DFIFO1       (((uint32_t *) OTG_FS_DFIFO1_BASE)  )
#define OTG_FS_DFIFO2       (((uint32_t *) OTG_FS_DFIFO2_BASE)  )
#define OTG_FS_DFIFO3       (((uint32_t *) OTG_FS_DFIFO3_BASE)  )
#define OTG_FS_DFIFO4       (((uint32_t *) OTG_FS_DFIFO4_BASE)  )
#define OTG_FS_DFIFO5       (((uint32_t *) OTG_FS_DFIFO5_BASE)  )
#define OTG_FS_DFIFO6       (((uint32_t *) OTG_FS_DFIFO6_BASE)  )
#define OTG_FS_DFIFO7       (((uint32_t *) OTG_FS_DFIFO7_BASE)  )
/************************* Structures *****************************************/

typedef __packed struct {               /* Channel typedef (CH)               */
  __packed union {
    U32 HCCHAR;                         /* Channel Characteristics            */
    __packed struct {
      U32 MPSIZ     : 11;               /* Endpoint Maximum Packet Size       */
      U32 EPNUM     :  4;               /* Endpoint Number                    */
      U32 EPDIR     :  1;               /* Endpoint Direction                 */
      U32 Reserved0 :  1;               /* Reserved                           */
      U32 LSDEV     :  1;               /* Endpoint Low-speed                 */
      U32 EPTYP     :  2;               /* Endpoint Type                      */
      U32 MCNT      :  2;               /* Periodic Endpoint Multicount       */
      U32 DAD       :  7;               /* Device Address                     */
      U32 ODDFRM    :  1;               /* Periodic Transaction Odd Frame     */
      U32 CHDIS     :  1;               /* Channel Disable                    */
      U32 CHENA     :  1;               /* Channel Enable                     */
    };
  };
  U32 ReservedU32_0;                    /* Reserved                           */
  __packed union {
    U32 HCINT;                          /* Channel Interrupt                  */
    __packed struct {
      U32 XFCR      :  1;               /* Transfer Completed                 */
      U32 CHH       :  1;               /* Channel Halted                     */
      U32 Reserved1 :  1;               /* Reserved                           */
      U32 STALL     :  1;               /* STALL Response Received Interrupt  */
      U32 NAK       :  1;               /* NAK Response Received Interrupt    */
      U32 ACK       :  1;               /* ACK Response Rece/Transmit Int     */
      U32 Reserved2 :  1;               /* Reserved                           */
      U32 TXERR     :  1;               /* Transaction Error                  */
      U32 BBERR     :  1;               /* Babble Error                       */
      U32 FRMOR     :  1;               /* Frame Overrun                      */
      U32 DTERR     :  1;               /* Data Toggle Error                  */
    };
  };
  __packed union {
    U32 HCINTMSK;                       /* Channel Interrupt Mask             */
    __packed struct {
      U32 XFCRM     :  1;               /* Transfer Completed Mask            */
      U32 CHHM      :  1;               /* Channel Halted Mask                */
      U32 Reserved3 :  1;               /* Reserved                           */
      U32 STALLM    :  1;               /* STALL Response Rece Interrupt Mask */
      U32 NAKM      :  1;               /* NAK Response Rece Interrupt Mask   */
      U32 ACKM      :  1;               /* ACK Response Rece/Transmit Int Mask*/
      U32 Reserved4 :  1;               /* Reserved                           */
      U32 TXERRM    :  1;               /* Transaction Error Mask             */
      U32 BBERRM    :  1;               /* Babble Error Mask                  */
      U32 FRMORM    :  1;               /* Frame Overrun Mask                 */
      U32 DTERRM    :  1;               /* Data Toggle Error Mask             */
    };
  };
  __packed union {
    U32 HCTSIZ;                         /* Channel Transfer Size              */
    __packed struct {
      U32 XFRSIZ    : 19;               /* Transfer Size                      */
      U32 PKTCNT    : 10;               /* Packet Count                       */
      U32 DPID      :  2;               /* Data PID                           */
      U32 Reserved5 :  1;               /* Reserved                           */
    };
  };
  U32 ReservedU32_1;                    /* Reserved                           */
  U32 ReservedU32_2;                    /* Reserved                           */
  U32 ReservedU32_3;                    /* Reserved                           */
} USBH_STM32_CH;


/************************* Driver Functions ***********************************/

void USBH_STM32_Get_Capabilities (USBH_HCI_CAP *cap);
void USBH_STM32_Delay_ms         (U32 ms);
BOOL USBH_STM32_Pins_Config      (U8 ctrl, BOOL on);
BOOL USBH_STM32_Init             (U8 ctrl, BOOL on);
BOOL USBH_STM32_Port_Power       (U8 ctrl, BOOL on);
BOOL USBH_STM32_Port_Reset       (U8 ctrl, U8 port);
U32  USBH_STM32_Get_Connect      (U8 ctrl);
U32  USBH_STM32_Get_Speed        (U8 ctrl);
U32  USBH_STM32_EP_Add           (U8 ctrl,           U8 dev_adr, U8 ep_spd, USB_ENDPOINT_DESCRIPTOR *ptrEPD);
BOOL USBH_STM32_EP_Config        (U8 ctrl, U32 hndl, U8 dev_adr, U8 ep_spd, USB_ENDPOINT_DESCRIPTOR *ptrEPD);
BOOL USBH_STM32_EP_Remove        (U8 ctrl, U32 hndl);
BOOL USBH_STM32_URB_Submit       (U8 ctrl, U32 hndl, USBH_URB *ptrURB);
BOOL USBH_STM32_URB_Cancel       (U8 ctrl, U32 hndl, USBH_URB *ptrURB);
void OTG_xS_IRQHandler           (U8 ctrl);

/************************* Exported Driver Structure **************************/

extern USBH_HCD usbh0_hcd;
extern USBH_HCD usbh1_hcd;


/************************* Constant Definitions *******************************/

/* OTG_FS_GAHBCFG Register bits                                               */
#define USBH_STM32_GAHBCFG_PTXFELVL     ((1         ) <<  8)
#define USBH_STM32_GAHBCFG_TXFELVL      ((1         ) <<  7)
#define USBH_STM32_GAHBCFG_GINTMSK      ((1         ) <<  0)

/* OTG_FS_GUSBCFG Register bits                                               */
#define USBH_STM32_GUSBCFG_CTXPKT       ((1UL       ) << 31)
#define USBH_STM32_GUSBCFG_FDMOD        ((1         ) << 30)
#define USBH_STM32_GUSBCFG_FHMOD        ((1         ) << 29)
#define USBH_STM32_GUSBCFG_TRDT(x)      ((x &   0x0F) << 10)
#define USBH_STM32_GUSBCFG_HNPCAP       ((1         ) <<  9)
#define USBH_STM32_GUSBCFG_SRPCAP       ((1         ) <<  8)
#define USBH_STM32_GUSBCFG_PHYSEL       ((1         ) <<  7)
#define USBH_STM32_GUSBCFG_TOCAL(x)     ((x &   0x07) <<  0)

/* OTG_FS_GRSTCTL Register bits                                               */
#define USBH_STM32_GRSTCTL_AHBIDL       ((1UL       ) << 31)
#define USBH_STM32_GRSTCTL_TXFNUM(x)    ((x &   0x1F) <<  6)
#define USBH_STM32_GRSTCTL_TXFFLSH      ((1         ) <<  5)
#define USBH_STM32_GRSTCTL_RXFFLSH      ((1         ) <<  4)
#define USBH_STM32_GRSTCTL_FCRST        ((1         ) <<  2)
#define USBH_STM32_GRSTCTL_HSRST        ((1         ) <<  1)
#define USBH_STM32_GRSTCTL_CSRST        ((1         ) <<  0)

/* OTG_FS_GINTSTS Register bits                                               */
#define USBH_STM32_GINTSTS_WKUINT       ((1UL       ) << 31)
#define USBH_STM32_GINTSTS_SRQINT       ((1         ) << 30)
#define USBH_STM32_GINTSTS_DISCINT      ((1         ) << 29)
#define USBH_STM32_GUSBCFG_PTCI         ((1         ) << 24)
#define USBH_STM32_GUSBCFG_PCCI         ((1         ) << 23)
#define USBH_STM32_GUSBCFG_ULPIEVBUSI   ((1         ) << 21)
#define USBH_STM32_GUSBCFG_ULPIEVBUSD   ((1         ) << 20)
#define USBH_STM32_GINTSTS_CIDSCHG      ((1         ) << 28)
#define USBH_STM32_GINTSTS_PTXFE        ((1         ) << 26)
#define USBH_STM32_GINTSTS_HCINT        ((1         ) << 25)
#define USBH_STM32_GINTSTS_HPRTINT      ((1         ) << 24)
#define USBH_STM32_GINTSTS_IPXFR        ((1         ) << 21)
#define USBH_STM32_GINTSTS_INCOMPISOOUT ((1         ) << 21)
#define USBH_STM32_GINTSTS_IISOIXFR     ((1         ) << 20)
#define USBH_STM32_GINTSTS_OEPINT       ((1         ) << 19)
#define USBH_STM32_GINTSTS_IEPINT       ((1         ) << 18)
#define USBH_STM32_GINTSTS_EOPF         ((1         ) << 15)
#define USBH_STM32_GINTSTS_ISOODRP      ((1         ) << 14)
#define USBH_STM32_GINTSTS_ENUMDNE      ((1         ) << 13)
#define USBH_STM32_GINTSTS_USBRST       ((1         ) << 12)
#define USBH_STM32_GINTSTS_USBSUSP      ((1         ) << 11)
#define USBH_STM32_GINTSTS_ESUSP        ((1         ) << 10)
#define USBH_STM32_GINTSTS_GOUTNAKEFF   ((1         ) <<  7)
#define USBH_STM32_GINTSTS_GINAKEFF     ((1         ) <<  6)
#define USBH_STM32_GINTSTS_NPTXFE       ((1         ) <<  5)
#define USBH_STM32_GINTSTS_RXFLVL       ((1         ) <<  4)
#define USBH_STM32_GINTSTS_SOF          ((1         ) <<  3)
#define USBH_STM32_GINTSTS_OTGINT       ((1         ) <<  2)
#define USBH_STM32_GINTSTS_MMIS         ((1         ) <<  1)
#define USBH_STM32_GINTSTS_CMOD         ((1         ) <<  0)

/* OTG_FS_GINTMSK Register bits                                               */
#define USBH_STM32_GINTMSK_WUIM         ((1UL       ) << 31)
#define USBH_STM32_GINTMSK_SRQIM        ((1         ) << 30)
#define USBH_STM32_GINTMSK_DISCINT      ((1         ) << 29)
#define USBH_STM32_GINTMSK_CIDSCHGM     ((1         ) << 28)
#define USBH_STM32_GINTMSK_PTXFEM       ((1         ) << 26)
#define USBH_STM32_GINTMSK_HCIM         ((1         ) << 25)
#define USBH_STM32_GINTMSK_PRTIM        ((1         ) << 24)
#define USBH_STM32_GINTMSK_IPXFRM       ((1         ) << 21)
#define USBH_STM32_GINTMSK_IISOOXFRM    ((1         ) << 21)
#define USBH_STM32_GINTMSK_IISOIXFRM    ((1         ) << 20)
#define USBH_STM32_GINTMSK_OEPINT       ((1         ) << 19)
#define USBH_STM32_GINTMSK_IEPINT       ((1         ) << 18)
#define USBH_STM32_GINTMSK_EPMISM       ((1         ) << 15)
#define USBH_STM32_GINTMSK_ISOODRPM     ((1         ) << 14)
#define USBH_STM32_GINTMSK_ENUMDNEM     ((1         ) << 13)
#define USBH_STM32_GINTMSK_USBRST       ((1         ) << 12)
#define USBH_STM32_GINTMSK_USBSUSPM     ((1         ) << 11)
#define USBH_STM32_GINTMSK_ESUSPM       ((1         ) << 10)
#define USBH_STM32_GINTMSK_GONAKEFFM    ((1         ) <<  7)
#define USBH_STM32_GINTMSK_GINAKEFFM    ((1         ) <<  6)
#define USBH_STM32_GINTMSK_NPTXFEM      ((1         ) <<  5)
#define USBH_STM32_GINTMSK_RXFLVLM      ((1         ) <<  4)
#define USBH_STM32_GINTMSK_SOFM         ((1         ) <<  3)
#define USBH_STM32_GINTMSK_OTGINT       ((1         ) <<  2)
#define USBH_STM32_GINTMSK_MMISM        ((1         ) <<  1)

/* OTG_FS_GCCFG Register bits                                                 */
#define USBH_STM32_GCCFG_SOFOUTEN       ((1         ) << 20)
#define USBH_STM32_GCCFG_VBUSBSEN       ((1         ) << 19)
#define USBH_STM32_GCCFG_VBUSASEN       ((1         ) << 18)
#define USBH_STM32_GCCFG_PWRDWN         ((1         ) << 16)

/* OTG_FS_HCFG Register bits                                                  */
#define USBH_STM32_HCFG_FSLS(x)         ((x &      1) <<  2)
#define USBH_STM32_HCFG_FSLSPCS(x)      ((x &      3) <<  0)

/* OTG_FS_HFIR Register bits                                                  */
#define USBH_STM32_HFIR_FRIVL(x)        ((x & 0xFFFF) <<  0)

/* OTG_FS_HPTXSTS Register bits                                               */
#define USBH_STM32_HPTXSTS_PTXFSAVL(x)  ((x & 0xFFFF) <<  0)

/* OTG_FS_HAINTMSK Register bits                                              */
#define USBH_STM32_HAINTMSK_HAINTM(x)   ((x & 0xFFFF) <<  0)

/* OTG_FS_HPRT Register bits                                                  */
#define USBH_STM32_HPRT_PTCTL(x)        ((x &   0x0F) << 13)
#define USBH_STM32_HPRT_PPWR            ((1         ) << 12)
#define USBH_STM32_HPRT_PRST            ((1         ) <<  8)
#define USBH_STM32_HPRT_PSUSP           ((1         ) <<  7)
#define USBH_STM32_HPRT_PRES            ((1         ) <<  6)
#define USBH_STM32_HPRT_POCCHNG         ((1         ) <<  5)
#define USBH_STM32_HPRT_POCA            ((1         ) <<  4)
#define USBH_STM32_HPRT_PENCHNG         ((1         ) <<  3)
#define USBH_STM32_HPRT_PENA            ((1         ) <<  2)
#define USBH_STM32_HPRT_PCDET           ((1         ) <<  1)
#define USBH_STM32_HPRT_PCSTS           ((1         ) <<  0)

/* OTG_FS_HCCHARx Register bits                                               */
#define USBH_STM32_HCCHAR_CHENA         ((1UL       ) << 31)
#define USBH_STM32_HCCHAR_CHDIS         ((1         ) << 30)
#define USBH_STM32_HCCHAR_ODDFRM        ((1         ) << 29)
#define USBH_STM32_HCCHAR_DAD           ((0x7F      ) << 22)
#define USBH_STM32_HCCHAR_MCNT          ((3         ) << 20)
#define USBH_STM32_HCCHAR_MCNT0         ((0         ) << 20)
#define USBH_STM32_HCCHAR_MCNT1         ((1         ) << 20)
#define USBH_STM32_HCCHAR_MCNT2         ((2         ) << 20)
#define USBH_STM32_HCCHAR_MCNT3         ((3         ) << 20)
#define USBH_STM32_HCCHAR_EPTYP         ((3         ) << 18)
#define USBH_STM32_HCCHAR_LSDEV         ((1         ) << 17)
#define USBH_STM32_HCCHAR_EPDIR         ((1         ) << 15)
#define USBH_STM32_HCCHAR_EPNUM         ((0x0F      ) << 11)
#define USBH_STM32_HCCHAR_MPSIZ         ((0x7FF     ) <<  0)

/* OTG_FS_HCINTx Register bits                                                */
#define USBH_STM32_HCINT_DTERR          ((1         ) << 10)
#define USBH_STM32_HCINT_FRMOR          ((1         ) <<  9)
#define USBH_STM32_HCINT_BBERR          ((1         ) <<  8)
#define USBH_STM32_HCINT_TXERR          ((1         ) <<  7)
#define USBH_STM32_HCINT_NYET           ((1         ) <<  6)
#define USBH_STM32_HCINT_ACK            ((1         ) <<  5)
#define USBH_STM32_HCINT_NAK            ((1         ) <<  4)
#define USBH_STM32_HCINT_STALL          ((1         ) <<  3)
#define USBH_STM32_HCINT_CHH            ((1         ) <<  1)
#define USBH_STM32_HCINT_XFRC           ((1         ) <<  0)
#define USBH_STM32_HCINT_ERR            (USBH_STM32_HCINT_DTERR | \
                                         USBH_STM32_HCINT_FRMOR | \
                                         USBH_STM32_HCINT_BBERR | \
                                         USBH_STM32_HCINT_TXERR   )

/* OTG_FS_HCINTMSKx Register bits                                             */
#define USBH_STM32_HCINTMSK_DTERRM      ((1         ) << 10)
#define USBH_STM32_HCINTMSK_FRMORM      ((1         ) <<  9)
#define USBH_STM32_HCINTMSK_BBERRM      ((1         ) <<  8)
#define USBH_STM32_HCINTMSK_TXERRM      ((1         ) <<  7)
#define USBH_STM32_HCINTMSK_NYET        ((1         ) <<  6)
#define USBH_STM32_HCINTMSK_ACKM        ((1         ) <<  5)
#define USBH_STM32_HCINTMSK_NAKM        ((1         ) <<  4)
#define USBH_STM32_HCINTMSK_STALLM      ((1         ) <<  3)
#define USBH_STM32_HCINTMSK_CHHM        ((1         ) <<  1)
#define USBH_STM32_HCINTMSK_XFRCM       ((1         ) <<  0)

/* OTG_FS_HCTSIZx Register bits                                               */
#define USBH_STM32_HCTSIZ_RESERVED      ((1UL       ) << 31)
#define USBH_STM32_HCTSIZ_DOPING        ((1UL       ) << 31)
#define USBH_STM32_HCTSIZ_DPID          ((3         ) << 29)
#define USBH_STM32_HCTSIZ_DPID_DATA0    ((0         ) << 29)
#define USBH_STM32_HCTSIZ_DPID_DATA2    ((1         ) << 29)
#define USBH_STM32_HCTSIZ_DPID_DATA1    ((2         ) << 29)
#define USBH_STM32_HCTSIZ_DPID_MDATA    ((3         ) << 29)


#pragma no_anon_unions

#endif  /* __USBH_STM32_H__ */
