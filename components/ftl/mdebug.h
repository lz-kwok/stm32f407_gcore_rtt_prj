#ifndef _MDEBUG_H_
#define _MDEBUG_H_

/****************************************************************************
 *
 *            Copyright (c) 2005 by HCC Embedded 
 *
 * This software is copyrighted by and is the sole property of 
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,  
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1132
 * Victor Hugo Utca 11-15
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#if 0
//set to 1 if debug info is needed

#include "stdio.h"

#ifdef _MLAYER_C_
FILE *debfile=0;
#else
extern FILE *debfile;
#endif

#define DEBOPEN if (!debfile) debfile=fopen("C:/wrtest.txt","wt+");
#define DEBPR0(s) fprintf (debfile,s);
#define DEBPR1(s,p1) fprintf (debfile,s,p1);
#define DEBPR2(s,p1,p2) fprintf (debfile,s,p1,p2);
#define DEBPR3(s,p1,p2,p3) fprintf (debfile,s,p1,p2,p3);
#else 

#define DEBOPEN 
#define DEBPR0(s)
#define DEBPR1(s,p1) 
#define DEBPR2(s,p1,p2) 
#define DEBPR3(s,p1,p2,p3)

#endif


#ifdef _WINDOWS_ 

extern int prg_wrapper(long f,long p0,long p1,long p2,long p3, long event);

#define wll_writedouble(event,pba,ppo,buffer0,buffer1) prg_wrapper((long)ll_writedouble,(long)(pba),(long)(ppo),(long)(buffer0),(long)(buffer1),event)
#define wll_write(event,pba,ppo,buffer) prg_wrapper((long)ll_write,(long)(pba),(long)(ppo),(long)(buffer),0,event)
#define wll_read(event,pba,ppo,buffer) prg_wrapper((long)ll_read,(long)(pba),(long)(ppo),(long)(buffer),0,event)
#define wll_erase(event,pba) prg_wrapper((long)ll_erase,(long)(pba),0,0,0,event)

#else

#define wll_writedouble(event,pba,ppo,buffer0,buffer1) ll_writedouble(pba,ppo,buffer0,buffer1)
#define wll_write(event,pba,ppo,buffer) ll_write(pba,ppo,buffer)
#define wll_read(event,pba,ppo,buffer) ll_read(pba,ppo,buffer)
#define wll_erase(event,pba) ll_erase(pba)

#endif

/****************************************************************************
 *
 * end of mdebug.h
 *
 ***************************************************************************/

#endif //_MDEBUG_H_
