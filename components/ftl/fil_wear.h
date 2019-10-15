#ifndef _FIL_WEAR_H_
#define _FIL_WEAR_H_

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

typedef struct {
	unsigned char free_index;        /**<. zero based index in freeblock table. */
	unsigned long free_wear;         /**<. freeblock wear info. */
	unsigned short static_logblock;  /**<. logical block address. */
	unsigned long static_wear;       /**<. static block wear info. */
} WEAR_ALLOCSTRUCT;

extern WEAR_ALLOCSTRUCT gl_wear_allocstruct;

#define WEAR_NA        0xfffffffeUL  //if entry is not available
#define BLK_NA         0xffff        //if block is not available
#define INDEX_NA       0xff          //if index is not available

#define WEAR_STATIC_LIMIT  1024        //minimum limit in static wear checking
#define WEAR_STATIC_COUNT  1023       //number of allocation when to check static

extern void wear_init(void);
extern t_bit wear_check_static(void);
extern void wear_alloc(void);
extern void wear_updatedynamicinfo(unsigned char index, unsigned long wear);
extern void wear_releasedynamiclock(void);
extern void wear_updatestaticinfo(unsigned short logblock, unsigned long wear);

#ifdef __cplusplus
}
#endif

/****************************************************************************
 *
 * end of fil_wear.c
 *
 ***************************************************************************/

#endif	//_FIL_WEAR_H_
