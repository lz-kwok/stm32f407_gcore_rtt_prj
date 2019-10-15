#ifndef _PRGMACRO_H_
#define _PRGMACRO_H_

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

#ifdef __cplusplus
extern "C" {  
#endif

#define CHECK_DATA

/****************************************************************************
 *
 * Command definitions
 *
 ***************************************************************************/

enum {
	PRG_BROPEN,	
	PRG_BRCLOSE,

	PRG_WRSECTOR,	
	PRG_WRLINEAR,	
	PRG_WRBATCH,	
	PRG_WRRANDOM,	

    PRG_RDSECTOR,	
	PRG_RDLINEAR,	
	PRG_RDBATCH,	
	PRG_RDRANDOM,	

	PRG_SHOW,
	PRG_SRAND,	
	PRG_REINIT,
	PRG_POWERFAIL,
	PRG_FLASHFAIL,
	PRG_POWERFAILSTRESS,

	PRG_END
};

/****************************************************************************
 *
 * status of program
 *
 ***************************************************************************/

#define PRG_LOADED   0x01  //program is loaded
#define PRG_RUNNING  0x02  //program is running
#define PRG_FINISHED 0x04  //program finished
#define PRG_ERROR	 0x08    //program has invalid instruction

/****************************************************************************
 *
 * functions
 *
 ***************************************************************************/

extern void prg_load(long *prg);
extern long prg_getstate(void);
extern void prg_step(void);
extern void prg_stop(void);

/****************************************************************************
 *
 * global variables
 *
 ***************************************************************************/

extern char useAbsAccess;   // set to 1 if we want to use Absolute IO Funcs (raw device access)

/****************************************************************************
 *
 * Test Counters
 *
 ***************************************************************************/

extern void cnt_resets(void);
extern void cnt_increase(unsigned char counter);
extern unsigned long cnt_getvalue(unsigned char counter);

enum {
	CNT_TWRITES,	//total number of writes
	CNT_TERASES,	//total number of erases
	CNT_TSTATIC,	//total number of static wears
	CNT_ERROR,		//total number of error
	CNT_FATALERROR,	//total number of fatal error
	CNT_MAX			//number of counters
};

#ifdef __cplusplus
}
#endif


/****************************************************************************
 *
 * end of prgmacro.h
 *
 ***************************************************************************/

#endif	//_PRGMACRO_H_




