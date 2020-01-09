/*
 * Copyright (c) 2019-present, Guolz
 *
 * Date           Author       Notes
 * 2019-12-04     guolz     first version
 * this file contains the addresses of all ADE78xx registers
 */

#ifndef __G_ADE7880_H__
#define __G_ADE7880_H__

#include <rtthread.h>
#include "drv_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define   ade7880_rst_pin			GET_PIN(F, 13)
#define   ade7880_pm0_pin			GET_PIN(F, 15)
#define   ade7880_pm1_pin			GET_PIN(F, 14)
#define   ade7880_irq0_pin			GET_PIN(A, 3)
#define   ade7880_irq1_pin			GET_PIN(A, 0)
#define   ade7880_cs_pin			GET_PIN(E, 11)

#define   ade7880_chip_version		0x01

typedef enum {
	PSM0 = 0,		//PSM0正常功耗模式
	PSM1,			//PSM1降耗模式
	PSM2,			//PSM2低功耗模式
	PSM3			//PSM3休眠模式
}PSM_MODE;

typedef struct {
	rt_uint32_t PhaseAEnergy;
	rt_uint32_t PhaseAIRMS;
	rt_uint32_t PhaseAVRMS;
	rt_uint32_t PhaseAPeroid;
}ade_base_parameter;
ade_base_parameter g_ade_base_parameter;

#define   AIGAIN	           		0x4380
#define   AVGAIN	          		0x4381
#define   BIGAIN	           		0x4382
#define   BVGAIN	           		0x4383
#define   CIGAIN	           		0x4384
#define   CVGAIN	           		0x4385
#define   NIGAIN	           		0x4386
#define   DICOEFF	          		0x4388
#define   APGAIN	           		0x4389
#define   AWATTOS	          		0x438A
#define   BPGAIN	           		0x438B
#define   BWATTOS	          		0x438C
#define   CPGAIN	           		0x438D
#define   CWATTOS	          		0x438E
#define   AIRMSOS	          		0x438F
#define   AVRMSOS	          		0x4390
#define   BIRMSOS	          		0x4391
#define   BVRMSOS	          		0x4392
#define   CIRMSOS	          		0x4393
#define   CVRMSOS	          		0x4394
#define   NIRMSOS	          		0x4395
#define   HPGAIN	           		0x4398
#define   ISUMLVL	          		0x4399
#define   VLEVEL	           		0x439F
#define   AFWATTOS	         		0x43A2
#define   BFWATTOS	         		0x43A3
#define   CFWATTOS	         		0x43A4
#define   AFVAROS	          		0x43A5
#define   BFVAROS	          		0x43A6
#define   CFVAROS	          		0x43A7
#define   AFIRMSOS	         		0x43A8
#define   BFIRMSOS	         		0x43A9
#define   CFIRMSOS	         		0x43AA
#define   AFVRMSOS	         		0x43AB
#define   BFVRMSOS	         		0x43AC
#define   CFVRMSOS	         		0x43AD
#define   HXWATTOS	         		0x43AE
#define   HYWATTOS	         		0x43AF
#define   HZWATTOS	         		0x43B0
#define   HXVAROS	          		0x43B1
#define   HYVAROS	          		0x43B2
#define   HZVAROS	          		0x43B3
#define   HXIRMSOS	         		0x43B4
#define   HYIRMSOS	         		0x43B5
#define   HZIRMSOS	         		0x43B6
#define   HXVRMSOS	         		0x43B7
#define   HYVRMSOS	         		0x43B8
#define   HZVRMSOS	         		0x43B9
#define   AIRMS	            		0x43C0
#define   AVRMS	            		0x43C1
#define   BIRMS	            		0x43C2
#define   BVRMS	            		0x43C3
#define   CIRMS	            		0x43C4
#define   CVRMS	            		0x43C5
#define   NIRMS	            		0x43C6
#define   ISUM	             		0x43C7
#define   RUN	              		0xE228
#define   AWATTHR	          		0xE400
#define   BWATTHR	          		0xE401
#define   CWATTHR	          		0xE402
#define   AFWATTHR	         		0xE403
#define   BFWATTHR	         		0xE404
#define   CFWATTHR	         		0xE405
#define   AFVARHR	          		0xE409
#define   BFVARHR	          		0xE40A
#define   CFVARHR	          		0xE40B
#define   AVAHR	            		0xE40C
#define   BVAHR	            		0xE40D
#define   CVAHR	            		0xE40E
#define   IPEAK	            		0xE500
#define   VPEAK	            		0xE501
#define   STATUS0	          		0xE502
#define   STATUS1	          		0xE503
#define   AIMAV	            		0xE504
#define   BIMAV	            		0xE505
#define   CIMAV	            		0xE506
#define   OILVL	            		0xE507
#define   OVLVL	            		0xE508
#define   SAGLVL	           		0xE509
#define   MASK0	            		0xE50A
#define   MASK1	            		0xE50B
#define   IAWV	             		0xE50C
#define   IBWV	             		0xE50D
#define   ICWV	             		0xE50E
#define   INWV	             		0xE50F
#define   VAWV	             		0xE510
#define   VBWV	             		0xE511
#define   VCWV	             		0xE512
#define   AWATT	            		0xE513
#define   BWATT	            		0xE514
#define   CWATT	            		0xE515
#define   AFVAR	            		0xE516
#define   BFVAR	            		0xE517
#define   CFVAR	            		0xE518
#define   AVA	              		0xE519
#define   BVA	              		0xE51A
#define   CVA	              		0xE51B
#define   CHECKSUM	         		0xE51F
#define   VNOM	             		0xE520
#define   LAST_RWDATA_24bit			0xE5FF
#define   PHSTATUS	         		0xE600
#define   ANGLE0	           		0xE601
#define   ANGLE1	           		0xE602
#define   ANGLE2	           		0xE603
#define   PHNOLOAD	         		0xE608
#define   LINECYC	          		0xE60C
#define   ZXTOUT	           		0xE60D
#define   COMPMODE	         		0xE60E
#define   Gain	             		0xE60F
#define   CFMODE	           		0xE610
#define   CF1DEN	           		0xE611
#define   CF2DEN	           		0xE612
#define   CF3DEN	           		0xE613
#define   APHCAL	           		0xE614
#define   BPHCAL	           		0xE615
#define   CPHCAL	           		0xE616
#define   PHSIGN	           		0xE617
#define   CONFIG	           		0xE618
#define   MMODE	            		0xE700
#define   ACCMODE	          		0xE701
#define   LCYCMODE	         		0xE702
#define   PEAKCYC	          		0xE703
#define   SAGCYC	           		0xE704
#define   CFCYC	            		0xE705
#define   HSDC_CFG	         		0xE706
#define   Version	          		0xE707
#define   LAST_RWDATA_8bit	 		0xE7FD
#define   FVRMS	            		0xE880
#define   FIRMS	            		0xE881
#define   FWATT	            		0xE882
#define   FVAR	             		0xE883
#define   FVA	             	 	0xE884
#define   FPF	              		0xE885
#define   VTHDN	            		0xE886
#define   ITHDN	           	 		0xE887
#define   HXVRMS	           		0xE888
#define   HXIRMS	           		0xE889
#define   HXWATT	           		0xE88A
#define   HXVAR	            		0xE88B
#define   HXVA	             		0xE88C
#define   HXPF	             		0xE88D
#define   HXVHD	            		0xE88E
#define   HXIHD	            		0xE88F
#define   HYVRMS	           		0xE890
#define   HYIRMS	           		0xE891
#define   HYWATT	           		0xE892
#define   HYVAR	            		0xE893
#define   HYVA	             		0xE894
#define   HYPF	             		0xE895
#define   HYVHD	            		0xE896
#define   HYIHD	            		0xE897
#define   HZVRMS	           		0xE898
#define   HZIRMS	           		0xE899
#define   HZWATT	           		0xE89A
#define   HZVAR	            		0xE89B
#define   HZVA	             		0xE89C
#define   HZPF	             		0xE89D
#define   HZVHD	            		0xE89E
#define   HZIHD	            		0xE89F
#define   HCONFIG	          		0xE900
#define   APF	              		0xE902
#define   BPF	              		0xE903
#define   CPF	              		0xE904
#define   APERIOD	          		0xE905
#define   BPERIOD	          		0xE906
#define   CPERIOD	          		0xE907
#define   APNOLOAD	         		0xE908
#define   VARNOLOAD	        		0xE909
#define   VANOLOAD	         		0xE90A
#define   LAST_ADD	         		0xE9FE
#define   LAST_RWDATA_16bit			0xE9FF
#define   CONFIG3	          		0xEA00
#define   LAST_OP	          		0xEA01
#define   WTHR	             		0xEA02
#define   VARTHR	           		0xEA03
#define   VATHR	            		0xEA04
#define   HX_reg	               	0xEA08
#define   HY_reg	               	0xEA09
#define   HZ_reg	               	0xEA0A
#define   LPOILVL	          		0xEC00
#define   CONFIG2	          		0xEC01

#define   REG_BIT0  				0x01
#define   REG_BIT1  				0x02
#define   REG_BIT2  				0x04
#define   REG_BIT3  				0x08
#define   REG_BIT4  				0x10
#define   REG_BIT5  				0x20
#define   REG_BIT6  				0x40
#define   REG_BIT7  				0x80
#define   REG_BIT8  				0x100
#define   REG_BIT9  				0x200
#define   REG_BIT10 				0x400
#define   REG_BIT11 				0x800
#define   REG_BIT12 				0x1000
#define   REG_BIT13 				0x2000
#define   REG_BIT14 				0x4000
#define   REG_BIT15 				0x8000
#define   REG_BIT16 				0x10000
#define   REG_BIT19 				0x80000
#define   REG_BIT20 				0x100000
#define   REG_BIT21 				0x200000
#define   REG_BIT22 				0x400000
#define   REG_BIT24 				0x1000000
#define   REG_BIT26 				0x4000000
#define   REG_BIT27 				0x8000000
#define   REG_BIT28 				0x10000000
#define   REG_BIT29 				0x20000000
#define   REG_BIT30 				0x40000000




int rt_hw_ade7880_int(void);
void rt_hw_ade7880_IVE_get(void);
void rt_hw_ade7880_HAR_get(void);

#ifdef __cplusplus
}
#endif

#endif /* __AT_H__ */
