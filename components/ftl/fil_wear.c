#ifndef _FIL_WEAR_C_
#define _FIL_WEAR_C_

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

/****************************************************************************
 *
 * Wear leveling for allocations
 *
 ***************************************************************************/

#include "mlayer.h"
#include "fil_wear.h"

/****************************************************************************
 *
 * variable for wear_alloc function
 *
 ***************************************************************************/

WEAR_ALLOCSTRUCT gl_wear_allocstruct; //this structure is filled when wear_alloc returns

/****************************************************************************
 *
 * dynamic wear leveling variable
 *
 ***************************************************************************/

static const unsigned char ml_lookup_mask[8]= //optimalized table for fast masking
	{0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/****************************************************************************
 *
 * wear_init
 *
 * initiate wear leveling, this function must be called at power on
 *
 ***************************************************************************/

void wear_init() {
unsigned short a;
   for (a=0; a<MAXSTATICWEAR; a++) {  
      gl_mlayer->static_wear_info[a].logblock=BLK_NA;
      gl_mlayer->static_wear_info[a].wear=WEAR_NA;
   }

   for (a=0; a<MAX_FREE_BLOCK; a++) {   
      gl_mlayer->dynamic_wear_info[a]=WEAR_NA;
   }

   gl_mlayer->static_cnt=2; //wait cycle after power on

   for (a=0; a<sizeof (gl_mlayer->dynamic_lock); a++) {
      gl_mlayer->dynamic_lock[a]=0xff; //set all bits to 1 means no free block available
   }
}


/****************************************************************************
 *
 * wear_check_static
 *
 * checking if static wear is needed
 *
 * RETURN
 *
 * 0 - if not needed
 * 1 - if static wear necessary
 *
 ***************************************************************************/

t_bit wear_check_static() {
unsigned short static_logblock=BLK_NA,a;
unsigned long static_wear=WEAR_NA;
   if (!gl_mlayer->static_cnt) {
      for (a=0; a<MAXSTATICWEAR; a++) {  
         unsigned long wear=gl_mlayer->static_wear_info[a].wear;
         unsigned short logblock=gl_mlayer->static_wear_info[a].logblock;

         if ((wear!=WEAR_NA) && (logblock!=BLK_NA)) {
            if (static_logblock==BLK_NA) {
               static_logblock=logblock;
               static_wear=wear;
            }
            else if (wear<static_wear) {
               static_logblock=logblock;
               static_wear=wear;
            }
         }
      }

      if (static_logblock!=BLK_NA) {
		 unsigned char *lookup_mask=(unsigned char *)ml_lookup_mask;
		 unsigned char *dynamic_lock=gl_mlayer->dynamic_lock;
		 t_ba *free_table=gl_mlayer->freetable;
		 unsigned char b;

         for (b=0; b<MAX_FREE_BLOCK; b++) {   //search for limit
			if (!( (*dynamic_lock) & (*lookup_mask) ) ) { //check if locked
				if (!((*free_table) & FT_BAD)) {	//if it is realy free
            unsigned long wear=gl_mlayer->dynamic_wear_info[b];
   
            if (wear!=WEAR_NA) {
               if (wear>static_wear+WEAR_STATIC_LIMIT) {
                  gl_wear_allocstruct.free_index=b;
                  gl_wear_allocstruct.free_wear=wear+1; //incremented by 1!
   
                  gl_wear_allocstruct.static_logblock=static_logblock;
                  gl_wear_allocstruct.static_wear=static_wear;

                  gl_mlayer->static_cnt=WEAR_STATIC_COUNT;
                  return 1;	 //static wear successfully
               }
					}
				}
			}

			free_table++;
			lookup_mask++;

			if ((b&7)==7) {
				lookup_mask=(unsigned char*)ml_lookup_mask; //reset look up
				dynamic_lock++;				//goto next bitfield;
			}
         }
      }
   }
   else {
	   gl_mlayer->static_cnt--;
   }

   gl_wear_allocstruct.free_index=INDEX_NA; 

   gl_wear_allocstruct.static_logblock=BLK_NA;   
   gl_wear_allocstruct.static_wear=WEAR_NA;

   return 0;  //no static wear needed
}

/****************************************************************************
 *
 * wear_alloc
 *
 * allocates a block from free table list according to theirs wear level info
 * datas will be unchanged in all table (calling twice it returns with same
 * values) if static_logblock is not equal with BLK_NA then static wear heandler
 * has to be called immediately. gl_wear_allocstruct holds all the 
 * information
 *
 ***************************************************************************/

void wear_alloc() {
unsigned char free_index=INDEX_NA,a;
unsigned long free_wear=WEAR_NA;
unsigned char *lookup_mask=(unsigned char *)ml_lookup_mask;
unsigned char *dynamic_lock=gl_mlayer->dynamic_lock;
t_ba *free_table=gl_mlayer->freetable;

	for (a=0; a<MAX_FREE_BLOCK; a++) {   //search for the less used free blk
		if (!( (*dynamic_lock) & (*lookup_mask) ) ) { //check if locked
			if (!((*free_table) & FT_BAD)) {	//if it is realy free
				unsigned long wear=gl_mlayer->dynamic_wear_info[a];

					if (wear!=WEAR_NA) {
						if (free_index==INDEX_NA) {
							free_index=a;
							free_wear=wear;
						}
						else if (wear<free_wear) {
							free_index=a;
							free_wear=wear;
					}
				}
			}
		}

		lookup_mask++;
		free_table++;

		if ((a&7)==7) {
			lookup_mask=(unsigned char*)ml_lookup_mask; //reset look up
			dynamic_lock++;				//goto next bitfield;
		}
	}

	if ((free_index==INDEX_NA) || (free_wear==WEAR_NA)) {
 		gl_wear_allocstruct.free_index=INDEX_NA;
 		gl_wear_allocstruct.free_wear=WEAR_NA;
		return; //fatal error! no free block in the table
	}

	gl_wear_allocstruct.free_index=free_index;
	gl_wear_allocstruct.free_wear=free_wear+1; //store increased value of wear
}

/****************************************************************************
 *
 * wear_updatedynamicinfo
 *
 * updating dynamic wear level info of an indexed block (free block) 
 *
 * INPUTS
 *
 * index - index of the block in free table entry
 * wear - wear info of given block
 *
 ***************************************************************************/

void wear_updatedynamicinfo(unsigned char index, unsigned long wear) {
   if (index<MAX_FREE_BLOCK) {

		if (wear<WEAR_NA) { //keep original data inside <e.g. for log,map blocks>
//			if (wear==WEAR_AUTO) gl_mlayer->dynamic_wear_info[index]=gl_mlayer->wear_average;
//			else gl_mlayer->dynamic_wear_info[index]=wear;
			gl_mlayer->dynamic_wear_info[index]=wear;
		}

		gl_mlayer->dynamic_lock[index>>3]|=ml_lookup_mask[index&7]; //optimalized!
	}
}


/****************************************************************************
 *
 * wear_releasedynamiclock
 *
 * Releasing newly added free blocks (enabled for allocation)
 * Must be called when a MAP dir or entry is stored
 *
 ***************************************************************************/

void wear_releasedynamiclock() {
unsigned char a;
	for (a=0; a<sizeof (gl_mlayer->dynamic_lock); a++) {
		gl_mlayer->dynamic_lock[a]=0; //set all bits to 0
	}
}

/****************************************************************************
 *
 * wear_updatestaticinfo
 *
 * updating static wear info of a static block (used block)
 *
 * INPUTS
 *
 * logblock - logical block number 
 * wear - wear info of the given block
 *
 ***************************************************************************/

void wear_updatestaticinfo(unsigned short logblock, unsigned long wear) {
unsigned long max_wear=WEAR_NA;
unsigned char a,max_index=INDEX_NA;

   if (logblock==BLK_NA) return;
//   if (wear==WEAR_AUTO) wear=gl_mlayer->wear_average;
//   if (gl_mlayer->wear_ave_block!=BLK_NA) { //check if average calculation is needed
//	   if (logblock>=gl_mlayer->wear_ave_block) { //check if startup cycle finished
//		   gl_mlayer->wear_count++;					//increase counts
//		   gl_mlayer->wear_sum+=wear;				//add to summary
//		   gl_mlayer->wear_average=gl_mlayer->wear_sum/gl_mlayer->wear_count; //calculate current average
//		   gl_mlayer->wear_ave_block=logblock;
//	   }
//	   else gl_mlayer->wear_ave_block=BLK_NA; //end of average calculation
//   }


   for (a=0; a<MAXSTATICWEAR; a++) {   //check if its already existed
      if (gl_mlayer->static_wear_info[a].logblock==logblock) {
         gl_mlayer->static_wear_info[a].wear=wear;
         return;
      }
   }

   for (a=0; a<MAXSTATICWEAR; a++) {   //search maximum or empty entry
      unsigned long twear=gl_mlayer->static_wear_info[a].wear;
      if (twear!=WEAR_NA) {
         if (max_wear==WEAR_NA) {
            max_wear=twear;    
            max_index=a;        
         }
         else if (twear>max_wear) {
            max_wear=twear;    
            max_index=a;        
         }
      }
      else {
         gl_mlayer->static_wear_info[a].wear=wear;
         gl_mlayer->static_wear_info[a].logblock=logblock;
         return;
      }
   }

   if (max_index!=INDEX_NA) {
      if (wear<max_wear) {
         gl_mlayer->static_wear_info[max_index].wear=wear;
         gl_mlayer->static_wear_info[max_index].logblock=logblock;         
      }
   }
}

/****************************************************************************
 *
 * end of fil_wear.c
 *
 ***************************************************************************/

#endif	//_FIL_WEAR_C_

