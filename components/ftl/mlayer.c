#ifndef _MLAYER_C_
#define _MLAYER_C_

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

#include "drv_nand.h"
#include "mlayer.h"
#include "fil_wear.h"
#include "fil_bad.h"
#include "prgmacro.h"
#include "string.h"
#include "mdebug.h"


/****************************************************************************
 *
 *	global Variables
 *
 ***************************************************************************/


/****************************************************************************
 *
 *	static Variables
 *
 ***************************************************************************/

ST_MAPBLOCK *gl_stepback;

unsigned char gl_buffer[MAX_PAGE_SIZE];
unsigned char gl_buffer2[MAX_PAGE_SIZE];
unsigned char gl_state;
unsigned char gl_save_map_need;
unsigned long gl_seccou;

t_ba gl_lba;
t_po gl_lpo;

ST_MLAYER mlayers[MLAYER_SETS];
ST_MLAYER *gl_mlayer;

/****************************************************************************
 *
 * ml_chk_frag
 *
 * function checks if requested fragment is in the cache area
 * and set ppba pointer its start address if found
 *
 * RETURNS
 *
 * 1 - if cached
 * 0 - if not
 *
 ***************************************************************************/

static t_bit ml_chk_frag(unsigned char frag) {
unsigned char idx; 

	for (idx=0; idx<MAX_CACHEFRAG; idx++) {
		if (gl_mlayer->frag.indexes[idx]==frag) {
			gl_mlayer->frag.ppba=gl_mlayer->frag.ppbas[idx];
			return 1;
		}
	}
	return 0;
}

/****************************************************************************
 *
 * ml_get_log2phy
 *
 * convert logical address to physical
 *
 * INPUTS
 *
 * lba - logical block address
 *
 * RETURNS
 *
 * physical address or BLK_NA if address is invalid
 *
 ***************************************************************************/

static t_ba ml_get_log2phy(t_ba lba) {
unsigned short index; 
unsigned char *buff;
	if (lba>=(MAX_BLOCK-MAX_FREE_BLOCK)) return BLK_NA;

	gl_mlayer->frag.current=(unsigned char)(lba/MAX_FRAGSIZE);
	index=(unsigned short)(lba%MAX_FRAGSIZE);

	if (ml_chk_frag(gl_mlayer->frag.current)) return gl_mlayer->frag.ppba[index];

	gl_mlayer->frag.pos++;
	if (gl_mlayer->frag.pos>=MAX_CACHEFRAG) gl_mlayer->frag.pos=0;

	buff=gl_mlayer->fragbuff+MAX_PAGE_SIZE*((unsigned short)(gl_mlayer->frag.pos));

	if (ll_read((t_ba)(gl_mlayer->start_pba+gl_mlayer->mapdir[gl_mlayer->frag.current].pba),gl_mlayer->mapdir[gl_mlayer->frag.current].ppo,buff)) {
		gl_mlayer->frag.indexes[gl_mlayer->frag.pos]=INDEX_NA;
		return BLK_NA;
	}

	if (gl_mlayer->mapdir[gl_mlayer->frag.current].index) gl_mlayer->frag.ppbas[gl_mlayer->frag.pos]=(t_ba *)buff;
	else gl_mlayer->frag.ppbas[gl_mlayer->frag.pos]=(t_ba *)(buff+MAX_DATA_SIZE/2);

	gl_mlayer->frag.ppba=gl_mlayer->frag.ppbas[gl_mlayer->frag.pos];
	gl_mlayer->frag.indexes[gl_mlayer->frag.pos]=gl_mlayer->frag.current;

	return gl_mlayer->frag.ppba[index];
}

/****************************************************************************
 *
 * ml_set_log2phy
 *
 * set logical block's physical address, 1st gl_mlayer->get_log2phy function has to
 * be called to get the fragments into the cached area and ppba must be set
 *
 * INPUTS
 *
 * lba - logical block address
 * pba - physical block address
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_set_log2phy(t_ba lba,t_ba pba) {
unsigned short index; 
	if (lba>=(MAX_BLOCK-MAX_FREE_BLOCK)) return 1;

	index=(unsigned short)(lba%MAX_FRAGSIZE);
	gl_mlayer->frag.ppba[index]=pba;

	return 0;
}


/****************************************************************************
 *
 * ml_createmap
 *
 * Creates complete mapdir information for a given block address
 *
 * INPUTS
 *
 * map - which map is used to 
 * ppo - physical page offset, where to start this information
 *
 ***************************************************************************/

static void ml_createmap(ST_MAPBLOCK  *map,unsigned char ppo) {
unsigned char frag;
	for (frag=0; frag<MAX_FRAG_PER_BLK;) {
		map->mapdir[frag].pba=map->pba;
		map->mapdir[frag].ppo=ppo;
		map->mapdir[frag].index=1;
		frag++;

		map->mapdir[frag].pba=map->pba;
		map->mapdir[frag].ppo=ppo;
		map->mapdir[frag].index=0;
		frag++;
		ppo++;
	}
}

/****************************************************************************
 *
 * ml_lowinit
 *
 * low level initialization, this must be called from gl_mlayer->init and gl_mlayer->format
 *
 * RETURN
 *
 * 0 - if ok
 * other if error
 *
 ***************************************************************************/

static t_bit ml_lowinit() {
int a;
	DEBOPEN;

	wear_init();

	gl_mlayer->curlog=0;
	gl_mlayer->logblocknum=0;
	gl_mlayer->logmerge=0;

	memset (gl_mlayer->log,0xff,sizeof(gl_mlayer->log)); //reset all entries

	gl_mlayer->freetable=(t_ba *)(gl_mlayer->basebuff+MAX_DATA_SIZE/2);
	gl_mlayer->mapinfo = (ST_MAPINFO *)(&gl_mlayer->freetable[MAX_FREE_BLOCK]);

	gl_mlayer->frag.pos=0;
	memset(gl_mlayer->frag.indexes,INDEX_NA,sizeof(gl_mlayer->frag.indexes));
	gl_mlayer->frag.current=INDEX_NA;

	for (a=0; a<MAX_NUM_OF_DIF_MAP; a++) {
		ST_MAPBLOCK *map=&gl_mlayer->mapblocks[a];

		map->last_pba=BLK_NA;
		map->last_ppo=INDEX_NA;

		map->block_type=(unsigned char)(BLK_TYPE_MAP_00+a);
		map->mapdir=&gl_mlayer->mapdir[a*MAX_FRAG_PER_BLK];
		map->pba=BLK_NA;
		map->ppo=INDEX_NA;
		map->ref_count=0;
		map->start_frag=(unsigned char)(a*MAX_FRAG_PER_BLK);
		map->end_frag=(unsigned char)(map->start_frag+MAX_FRAG_PER_BLK);

		map->indexcou=0;
		memset(map->index,INDEX_NA,MAX_MAP_BLK);
	}

	gl_mlayer->max_mappagecou_lo=0;
	gl_mlayer->max_mappagecou_hi=0;
	
	gl_mlayer->badblock=&gl_mlayer->mapinfo->badblock;

	gl_mlayer->badblock->index=INDEX_NA;
	gl_mlayer->badblock->pba=BLK_NA;
	gl_mlayer->badblock->ppo=INDEX_NA;

	bb_init(MAX_RESERVED_BLOCK); //here the BB page is not loaded!!! 
	//We should call this function again later if we have the MAP-PAGE

	gl_save_map_need=0;

	memset(gl_mlayer->mapdir,0xff,sizeof(gl_mlayer->mapdir));
	gl_mlayer->data_blk_count=0;

	return 0;
}									   

/****************************************************************************
 *
 * ml_buildmap
 *
 * function for rebuilding mapdir from a given index from free block table
 *
 * INPUTS
 *
 * idx  - mapinfo index, to be built from
 *
 * RETURNS
 *
 * 0 - if ok
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_buildmap(ST_MAPBLOCK *map,unsigned char idx) {
t_ba pba;
t_po ppo;
unsigned char index;
ST_SPARE *sptr=GET_SPARE_AREA(gl_buffer);
unsigned char frag;
unsigned char afrm=0;
again:
	map->index[idx]=gl_mlayer->mapinfo->index[idx];
	index=gl_mlayer->mapinfo->index[idx];

	if (index!=INDEX_NA) {
		map->indexcou=(unsigned char)(idx+1); //update indexcou
	}

	DEBPR0("ml_buildmap\n");

	if (idx==0) {
		if (index==INDEX_NA) return 1; //fatal

		map->mappagecou_hi=gl_mlayer->mapinfo->mappagecou_hi;
		map->mappagecou_lo=gl_mlayer->mapinfo->mappagecou_lo; /* get the counter, if step back then it is also ok */


		if (gl_mlayer->mapinfo->mappagecou_hi>gl_mlayer->max_mappagecou_hi) {
			gl_mlayer->max_mappagecou_lo=gl_mlayer->mapinfo->mappagecou_lo;
			gl_mlayer->max_mappagecou_hi=gl_mlayer->mapinfo->mappagecou_hi; 
		}
		else if (gl_mlayer->mapinfo->mappagecou_hi==gl_mlayer->max_mappagecou_hi && gl_mlayer->mapinfo->mappagecou_lo>gl_mlayer->max_mappagecou_lo) {
			gl_mlayer->max_mappagecou_lo=gl_mlayer->mapinfo->mappagecou_lo;
			gl_mlayer->max_mappagecou_hi=gl_mlayer->mapinfo->mappagecou_hi; 	
		}

		pba=(t_ba)(gl_mlayer->freetable[index]&FT_ADDRMASK);
		ppo=0;

		if (!ll_read((t_ba)(gl_mlayer->start_pba+pba),ppo,gl_buffer)) {
			if (sptr->page_state==STA_MAPPAGE) {
				frag=sptr->frag;
				gl_mlayer->mapdir[frag].pba=pba;
				gl_mlayer->mapdir[frag].ppo=ppo;
				gl_mlayer->mapdir[frag].index=1;
				ppo=1;
			}
			else if (sptr->page_state!=STA_ORIGFRAG) {
				DEBPR1("sptr->page_state %d\n",sptr->page_state);
				return 1; //only mappage can be here, or origfrag after format
			}
			else afrm=1; //we are after format
		}
		else return 1; //must be ok!

		for (frag=0;frag<MAX_FRAG_PER_BLK; ppo++) {
			if (!ll_read((t_ba)(gl_mlayer->start_pba+pba),ppo,gl_buffer)) {
				if (sptr->page_state==STA_ORIGFRAG) {
					map->mapdir[frag].pba=pba;
					map->mapdir[frag].ppo=ppo;
					map->mapdir[frag].index=1;
					frag++;

					map->mapdir[frag].pba=pba;
					map->mapdir[frag].ppo=ppo;
					map->mapdir[frag].index=0;
					frag++;
				}
				else {
					return 1; //only mappage can be here
				}
			}
			else {
				//broken mapdir, so step back
				if (gl_stepback) return 1; //only once is allowed
				gl_stepback=map;

				if (gl_mlayer->mapinfo->last_pba==BLK_NA || gl_mlayer->mapinfo->last_ppo==INDEX_NA) return 1; //cant

				map->pba=gl_mlayer->mapinfo->last_pba;
				map->ppo=gl_mlayer->mapinfo->last_ppo;

				if (ll_read((t_ba)(gl_mlayer->start_pba+map->pba),map->ppo,gl_mlayer->basebuff)) return 1; //fatal

				map->last_pba=map->pba; //update last information
				map->last_ppo=map->ppo;

				map->ppo++; //set next position

				goto again;
			}
		}
	}
	else {
		if (index==INDEX_NA) return 0; //nothing to do
		pba=(t_ba)(gl_mlayer->freetable[index]&FT_ADDRMASK);
		ppo=0;
	}

	for (;ppo<MAX_PAGE_PER_BLOCK; ppo++) {
		int ret=ll_read((t_ba)(gl_mlayer->start_pba+pba),ppo,gl_buffer);
		if (ret==LL_OK) {
			if (sptr->page_state==STA_MAPPAGE) {
				frag=sptr->frag;
				gl_mlayer->mapdir[frag].pba=pba;
				gl_mlayer->mapdir[frag].ppo=ppo;
				gl_mlayer->mapdir[frag].index=1;
			}
			else {
				return 1; //only mappage can be here
			}
		}
		else if (ret==LL_ERASED) continue; //maybe flashfail happened
	}

	if (afrm) {
		//we are after format
		if (ppo<MAX_FRAG_PER_BLK/2) return 1; //flt missing after format
	}

	return 0;
}


/****************************************************************************
 *
 * check_reference_count
 *
 * function check and set the latest map reference counter
 *
 * INPUTS
 *
 * mapnum - which map need to be checked
 * refcount - current reference counter
 * pba - current block physical address
 *
 ***************************************************************************/

static void check_reference_count(unsigned char mapnum,unsigned long ref_count,t_ba pba) {
ST_MAPBLOCK *map=&gl_mlayer->mapblocks[mapnum];

	if (map->pba==BLK_NA) { /* we have found the 1st map block */
		map->pba=pba;
		map->ref_count=ref_count;
	} 
	else {
		if (map->ref_count<ref_count) {
			map->pba=pba;
            map->ref_count=ref_count;
		}
	}
}

/****************************************************************************
 *
 * ml_alloc
 *
 * allocate a block from free table and erases it
 *
 * RETURNS
 *
 * index in free table or INDEX_NA if any error
 *
 ***************************************************************************/

static unsigned char ml_alloc() {
unsigned char index;

	for (;;) {	   //loop for wear alloc
		t_ba pba;

		wear_alloc();
		index=gl_wear_allocstruct.free_index;

		DEBPR1("gl_mlayer->alloc index %d\n",index);

	    if (index==INDEX_NA) {
		    return INDEX_NA; //FATAL error no free blocks are available
	    }


		pba=gl_mlayer->freetable[index];

		for (;;) { //internal loop for pba changing

			DEBPR2("gl_mlayer->alloc pba %d, %04x\n", pba,pba);

			if (wll_erase(100,(t_ba)(gl_mlayer->start_pba+pba)) == LL_OK) {
				return index; //erase is ok, so return with the index
			}

	        DEBPR1("gl_mlayer->alloc: Erase of block %d failed\n", pba);

			pba=bb_setasbb(pba); //change pba, set current as BAD
			if (pba==BLK_NA) {	 //check if success
				gl_mlayer->freetable[index] |= FT_BAD;     //signal it as locked BAD
				break; //get a new index
			}

			gl_mlayer->freetable[index] = pba; //write back new pba
			gl_wear_allocstruct.free_wear=1; //set new wear info
			wear_updatedynamicinfo(index,1); //update wear info
		}
	}

}

/****************************************************************************
 *
 * get_mapblock
 *
 * Function for retreiving the map according fragment counter
 *
 * INPUTS
 *
 * frag_num - fragment number
 *
 * RETURNS
 *
 * map pointer where the fragment is or NULL if any error
 *
 ***************************************************************************/

ST_MAPBLOCK *get_mapblock(unsigned char frag_num) {
ST_MAPBLOCK *map=gl_mlayer->mapblocks;

	if (frag_num<map->end_frag) return map;

	map++; 
	if (frag_num<map->end_frag) return map;

	map++; 
	if (frag_num<map->end_frag) return map;

	map++; 
	if (frag_num<map->end_frag) return map;

	return 0;
}

/****************************************************************************
 *
 * ml_save_map
 *
 * Save map informations (fragments, free table modification)
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_save_map() {
ST_MAPBLOCK  *map=get_mapblock(gl_mlayer->frag.current);
ST_SPARE *sptr;
char domap=0;
unsigned char cou;

	DEBPR0("ml_save_map\n")

	if (!map) return 1; //fatal

	for (;;) {
again:
		if (map->ppo==MAX_PAGE_PER_BLOCK || domap) {
			unsigned char index = ml_alloc();
			if (index==INDEX_NA) return 1; //fatal

			map->pba=gl_mlayer->freetable[index];
			map->ppo=0;
			wear_updatedynamicinfo(index,gl_wear_allocstruct.free_wear);

			map->ref_count++;

			if (map->indexcou==MAX_MAP_BLK) {
				domap=1;
			}

			if (domap) {
				unsigned char idx;
				for (idx=0; idx<MAX_MAP_BLK;idx++) {
					unsigned char midx=map->index[idx];
					if (midx!=INDEX_NA) {
						gl_mlayer->freetable[midx]&=~FT_MAP;
						wear_updatedynamicinfo(midx,WEAR_NA); //still locked
						map->index[idx]=INDEX_NA;				
					}
				}
				map->indexcou=0;
			}

			map->index[map->indexcou++]=index;
			gl_mlayer->freetable[index]|=FT_MAP;

			DEBPR3("map_pba %d flt %04x domap %d\n",map->pba,gl_mlayer->freetable[index],domap)
		}

		if (bb_flush()) return 1; //fatal

		sptr=GET_SPARE_AREA(gl_mlayer->basebuff);

		if (!map->ppo) {
			sptr->wear=gl_wear_allocstruct.free_wear;
			sptr->u.map.ref_count=map->ref_count;
			sptr->block_type=map->block_type;
			sptr->bad_tag=0xff;
		}

		sptr->page_state=STA_MAPPAGE;
		sptr->frag=gl_mlayer->frag.current;
	
		for (cou=0; cou<MAX_MAP_BLK; cou++) {
			gl_mlayer->mapinfo->index[cou]=map->index[cou]; //copy history
		}

		gl_mlayer->mapinfo->last_pba=map->last_pba;
		gl_mlayer->mapinfo->last_ppo=map->last_ppo;

		gl_mlayer->mapinfo->mappagecou_lo++;
		if (!gl_mlayer->mapinfo->mappagecou_lo)	gl_mlayer->mapinfo->mappagecou_hi++; /* 64bit counter */

		if (!ml_chk_frag(gl_mlayer->frag.current)) {
			return 1; //fatal
		}

		if (!wll_writedouble(30,(t_ba)(gl_mlayer->start_pba+map->pba),map->ppo,(unsigned char*)gl_mlayer->frag.ppba,(unsigned char*)gl_mlayer->freetable)) {

			if (!domap) {
				map->last_pba=map->pba;
				map->last_ppo=map->ppo;

				gl_mlayer->mapdir[gl_mlayer->frag.current].pba=map->pba;
				gl_mlayer->mapdir[gl_mlayer->frag.current].ppo=map->ppo;
				gl_mlayer->mapdir[gl_mlayer->frag.current].index=1;

				map->ppo++;
			}
			else {
				unsigned char frag;
				unsigned char *sou,*dest;

				map->ppo++;

				sptr=GET_SPARE_AREA(gl_buffer);

				for (frag=0;frag<MAX_FRAG_PER_BLK;) {
	
					dest=gl_buffer;
					if (ml_chk_frag((unsigned char)(frag+map->start_frag))) {
						memcpy(dest,gl_mlayer->frag.ppba,MAX_DATA_SIZE/2);
					}
					else {
						if (!ll_read((t_ba)(gl_mlayer->start_pba+map->mapdir[frag].pba),map->mapdir[frag].ppo,gl_buffer2)) {
							sou=gl_buffer2;
							if (!map->mapdir[frag].index) sou+=MAX_DATA_SIZE/2;
							memcpy(dest,sou,MAX_DATA_SIZE/2);
						}
						else memset(dest,0xff,MAX_DATA_SIZE);
					}
					frag++;
	
					dest+=MAX_DATA_SIZE/2;
					if (ml_chk_frag((unsigned char)(frag+map->start_frag))) {
						memcpy(dest,gl_mlayer->frag.ppba,MAX_DATA_SIZE/2);
					}
					else {
						if (!ll_read((t_ba)(gl_mlayer->start_pba+map->mapdir[frag].pba),map->mapdir[frag].ppo,gl_buffer2)) {
							sou=gl_buffer2;
							if (!map->mapdir[frag].index) sou+=MAX_DATA_SIZE/2;
							memcpy(dest,sou,MAX_DATA_SIZE/2);
						}
						else memset(dest,0xff,MAX_DATA_SIZE);
					}
					frag++;

					sptr->page_state=STA_ORIGFRAG;

					if (wll_write(31,(t_ba)(gl_mlayer->start_pba+map->pba),map->ppo,gl_buffer)) goto again;

					map->ppo++;
				}
	
				ml_createmap(map,1);

				map->last_pba=map->pba;
				map->last_ppo=0;  //because of domap!

			}

			wear_releasedynamiclock();
			gl_save_map_need=0;
			return 0;
		}
		else {
			if (!map->ppo) {
				domap=1;
			}
			else {
				map->ppo++;
			}
		}
	}
}

/****************************************************************************
 *
 * ml_init
 *
 * Initialize this layer, this function must be called at the begining
 *
 * RETURNS
 *
 * 0 - if ok
 * other if any error
 *
 ***************************************************************************/

t_bit ml_init() {
	unsigned long wear_sum;	
	unsigned long wear_count;
	unsigned long wear_average;
	unsigned char idx;
	unsigned char cou;
	unsigned char mlcou;
	t_ba pba,lba;
	t_po po,lastpo;
	ST_SPARE *sptr;
	ST_LOG *log;

	DEBPR0("ml_init\n");

	gl_state=ML_INIT;

	if (ll_init()) return 1;

	gl_stepback=0;

  	for (mlcou=0; mlcou<MLAYER_SETS; mlcou++) {
		gl_mlayer=&mlayers[mlcou];
		gl_mlayer->start_pba=(t_ba)(MAX_BLOCK*(unsigned short)mlcou);
		gl_mlayer->laynum=mlcou;

		wear_sum=0;		//summary of wears
		wear_count=0;	//counting number of wears 
		wear_average=0;

		if (ml_lowinit()) return 1;

		sptr=GET_SPARE_AREA(gl_buffer);

		//search last map blk
		for (pba=0; pba<MAX_BLOCK; pba++) {

			wear_count++; //increase counts

			if (!ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer)) {

				if (sptr->wear<WEAR_NA) wear_sum+=sptr->wear;				//add to summary


				switch (sptr->block_type) {
				case BLK_TYPE_MAP_00: 
				case BLK_TYPE_MAP_01: 
				case BLK_TYPE_MAP_02: 
				case BLK_TYPE_MAP_03: check_reference_count(
									(unsigned char)(sptr->block_type-BLK_TYPE_MAP_00),
									sptr->u.map.ref_count,pba
									); 
				}
			}
		}

  		for (cou=0; cou<MAX_NUM_OF_DIF_MAP; cou++) {
			ST_MAPBLOCK *map=&gl_mlayer->mapblocks[cou];
			if (map->pba==BLK_NA) return 1; //fatal, no map blk found

			//search last erased only page
			lastpo=MAX_PAGE_PER_BLOCK;
			for (po=0;po<MAX_PAGE_PER_BLOCK;po++) {
				int ret=ll_read((t_ba)(gl_mlayer->start_pba+map->pba),(unsigned char)(MAX_PAGE_PER_BLOCK-po-1),gl_buffer);
				if (ret==LL_ERASED) lastpo=(unsigned char)(MAX_PAGE_PER_BLOCK-po-1);
				else break;
			}

			//search last po in block
			map->ppo=INDEX_NA;
			for (po=0;po<lastpo;po++) {
				int ret=ll_read((t_ba)(gl_mlayer->start_pba+map->pba),po,gl_buffer);
				if (ret==LL_OK) {
					if (sptr->page_state==STA_MAPPAGE) {
						map->ppo=po;
					}
				}
		}

		if (map->ppo==INDEX_NA) return 1; //fatal, no map blk found

		//read it
		if (ll_read((t_ba)(gl_mlayer->start_pba+map->pba),map->ppo,gl_mlayer->basebuff)) return 1; //fatal

		map->last_pba=map->pba; //update last information
		map->last_ppo=map->ppo;

		map->ppo=lastpo;

		for (idx=0; idx<MAX_MAP_BLK; idx++) {
			if (ml_buildmap(map,idx)) {
				DEBPR1("ERROR: gl_mlayer->buildmap at idx %d\n",idx);
				return 1; //fatal
			}
		}
	}


	{
		//finding last fltable
		ST_MAPBLOCK *mapcur=gl_mlayer->mapblocks;
		unsigned long mappagecou_lo=mapcur->mappagecou_lo;
		unsigned long mappagecou_hi=mapcur->mappagecou_hi;
		ST_MAPBLOCK *map=mapcur;

		for (cou=1,mapcur++; cou<MAX_NUM_OF_DIF_MAP;cou++,mapcur++) {
			if (mapcur->mappagecou_hi>mappagecou_hi) {
				mappagecou_lo=mapcur->mappagecou_lo;
				mappagecou_hi=mapcur->mappagecou_hi; 
				map=mapcur;				
			}
			else if (mapcur->mappagecou_hi==mappagecou_hi && mapcur->mappagecou_lo>mappagecou_lo) {
				mappagecou_lo=mapcur->mappagecou_lo;
				mappagecou_hi=mapcur->mappagecou_hi; 	
				map=mapcur;				
			}
		}

		if (ll_read((t_ba)(gl_mlayer->start_pba+map->last_pba),map->last_ppo,gl_mlayer->basebuff)) return 1; //fatal

		gl_mlayer->mapinfo->mappagecou_lo=gl_mlayer->max_mappagecou_lo;
		gl_mlayer->mapinfo->mappagecou_hi=gl_mlayer->max_mappagecou_hi; //write back the maximum found
	}

	if (bb_init(MAX_RESERVED_BLOCK)) return 1; //here the BB page is loaded!!! 

//building log blocks
	gl_mlayer->logblocknum=0;
	log=gl_mlayer->log;
	for (idx=0; idx<MAX_FREE_BLOCK; idx++) {
		if ((gl_mlayer->freetable[idx]&(FT_MAP|FT_LOG))==FT_LOG) {
			pba=(t_ba)(gl_mlayer->freetable[idx]&FT_ADDRMASK);

			log->lastppo=MAX_PAGE_PER_BLOCK;
			log->pba=pba;
			log->switchable=1;
			log->index=idx;

			lastpo=MAX_PAGE_PER_BLOCK;
			for (po=0;po<MAX_PAGE_PER_BLOCK;po++) { //searching the last page from top
				int ret=ll_read((t_ba)(gl_mlayer->start_pba+pba),(unsigned char)(MAX_PAGE_PER_BLOCK-po-1),gl_buffer);
				if (ret==LL_ERASED) lastpo=(unsigned char)(MAX_PAGE_PER_BLOCK-po-1);
				else break;
			}

			log->lastppo=lastpo;
			for (po=0; po<lastpo; po++) {	//building log blk
				int ret=ll_read((t_ba)(gl_mlayer->start_pba+pba),po,gl_buffer);
				if (ret==LL_OK) {
					log->ppo[sptr->u.log.lpo]=po;
					log->lba=sptr->u.log.lba;
					if (po!=sptr->u.log.lpo) log->switchable=0;
					if (!po) log->wear=sptr->wear;
				}
				else log->switchable=0; //if error or hole in the log block
			}

			if (log->lastppo!=0) { 
				log++;
				gl_mlayer->logblocknum++;
			}
			else {
				gl_mlayer->freetable[idx]&=~FT_LOG; //nothing was written, so remove log block flag
				log->lastppo=INDEX_NA;		//reset data in not used log blk
				log->pba=BLK_NA;
			}
		}
	}

   if (wear_count) wear_average=wear_sum/wear_count; //calculate current average

//fill dynamic wear info table
	for (idx=0; idx<MAX_FREE_BLOCK; idx++) {
		if ((gl_mlayer->freetable[idx]&FT_BAD)!=FT_BAD) {
		 	pba=(t_ba)(gl_mlayer->freetable[idx]&FT_ADDRMASK);
			if (ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer)==LL_OK) {
				wear_updatedynamicinfo(idx,sptr->wear);
			}
			else {
                wear_updatedynamicinfo(idx,wear_average);
			}
		}
	}

	wear_releasedynamiclock();

//fill static wear info
    for(lba=0; lba<MAX_BLOCK; lba++) {
		pba=ml_get_log2phy(lba);
        if (pba==BLK_NA) break;

        if (ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer)) {
            wear_updatestaticinfo(lba, 0);  
        } else {
            wear_updatestaticinfo(lba, sptr->wear);  /* only data block could be updated */
        }
        gl_mlayer->data_blk_count++;
	}

	if (gl_stepback) {
		t_ba lba=gl_stepback->start_frag;

		lba*=MAX_FRAGSIZE;

		gl_stepback->ppo=MAX_PAGE_PER_BLOCK;
		gl_stepback->indexcou=MAX_MAP_BLK; //force map merge

		if (ml_get_log2phy(lba)==BLK_NA) return 1;
		if (ml_save_map()) return 1;
	}
  }

  gl_state=ML_CLOSE;

  return 0;
}


/****************************************************************************
 *
 * write_frags
 *
 * subfunction for formatting to write current fragments in fragbuff
 *
 * INPUTS
 *
 * frag - current frag number
 *
 * RETURNS
 *
 * 0 - if successfuly
 * other if any error
 *
 ***************************************************************************/

static t_bit write_frags(unsigned char frag) {
ST_SPARE *sptr=GET_SPARE_AREA(gl_mlayer->fragbuff);
ST_MAPBLOCK  *map=get_mapblock(frag);

	if (!map->ppo) {
		sptr->wear=1;//gl_wear_allocstruct.free_wear;
		sptr->u.map.ref_count=map->ref_count;
		sptr->block_type=map->block_type;
		sptr->bad_tag=0xff;
	}

	sptr->page_state=STA_ORIGFRAG;

	if (wll_write(1,(t_ba)(gl_mlayer->start_pba+map->pba),map->ppo,gl_mlayer->fragbuff)) return 1; //fatal
	map->ppo++;
	return 0;
}

/****************************************************************************
 *
 * ml_getmaxsector
 *
 * retreives the maximum number of sectors can be used in the system
 *
 * RETURNS
 *
 * maximum number of sectors
 *
 ***************************************************************************/

unsigned long ml_getmaxsector() {
	return gl_mlayer->data_blk_count;
}

/****************************************************************************
 *
 * ml_format
 *
 * Formatting the layer, this function must be called once at manufacturing
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_format() {
unsigned long lbacou;
unsigned short fidx;
unsigned char index;
unsigned char cou;
unsigned char frag;
t_ba pba;
t_ba *ppba;
ST_SPARE *sptr=GET_SPARE_AREA(gl_buffer);
unsigned char mlcou;

  gl_state=ML_INIT;

  if (ll_init()) return 1;

  for (mlcou=0; mlcou<MLAYER_SETS; mlcou++) {
  	gl_mlayer=&mlayers[mlcou];
	gl_mlayer->start_pba=(t_ba)(MAX_BLOCK*(unsigned short)mlcou);
	gl_mlayer->laynum=mlcou;

	if (ml_lowinit()) return 1;

	for (index=0,pba=0;index<MAX_FREE_BLOCK+MAX_RESERVED_BLOCK; pba++,index++) {
		int ret=ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer);
		if (ret==LL_OK || ret==LL_ERROR) {
			if (sptr->bad_tag!=0xff) {
				DEBPR2("Bad Block found (badtag) pba %d [%4x]",pba,pba);
				continue; //bad block
			}
			if (ll_erase((t_ba)(gl_mlayer->start_pba+pba))) {
				sptr->bad_tag=0; //signalling bad block
				ll_write((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer);
				DEBPR2("Bad Block found (erased) pba %d [%4x]",pba,pba);
				continue; //cannot be erased so bad block
			}
		}

		if (index<MAX_FREE_BLOCK) {
			gl_mlayer->freetable[index]=pba;
			wear_updatedynamicinfo((unsigned char)index,0);
		}
		else {
			if (bb_addreserved(pba)) {
				return 1;
			}
		}
	}

	wear_releasedynamiclock();

//allocate MAP block
	for (cou=0; cou<MAX_NUM_OF_DIF_MAP; cou++) {
		ST_MAPBLOCK *map=&gl_mlayer->mapblocks[cou];

		index = ml_alloc();
		if (index==INDEX_NA) return 1; //fatal
		map->pba=gl_mlayer->freetable[index];
		map->ppo=0;
		gl_mlayer->freetable[index]|=FT_MAP;
		wear_updatedynamicinfo(index,gl_wear_allocstruct.free_wear);

		map->indexcou=0;
		map->index[map->indexcou++]=index;
		map->ref_count=1;
	}

	memset (gl_mlayer->mapinfo,0xff,sizeof(ST_MAPINFO));

//create MAP dir from remaining blocks
	ppba=(t_ba *)gl_mlayer->fragbuff;
	fidx=0;
	frag=0;
	lbacou=0;

	for (;pba<MAX_BLOCK;pba++) {
		int ret=ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer);

		lbacou++;

		if (ret==LL_OK || ret==LL_ERROR) {
			if (sptr->bad_tag!=0xff) {
				DEBPR2("Bad Block found (badtag) pba %d [%4x]",pba,pba);
				continue; //bad block
			}
			if (ll_erase((t_ba)(gl_mlayer->start_pba+pba))) {
				sptr->bad_tag=0; //signalling bad block
				ll_write((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer);
				DEBPR2("Bad Block found (erased) pba %d [%4x]",pba,pba);
				continue; //cannot be erased so bad block
			}
		}

		gl_mlayer->data_blk_count++;

		ppba[fidx++]=pba;
		if (fidx==MAX_FRAGSIZE*2) {
			if (write_frags(frag)) return 1; //fatal
			fidx=0;
			frag+=2;
		}
	}

	for (;lbacou<MAX_BLOCK;lbacou++) { //fill remaining with BLK_NA
		ppba[fidx++]=BLK_NA;		
		if (fidx==MAX_FRAGSIZE*2) {
			if (write_frags(frag)) return 1; //fatal
			fidx=0;
			frag+=2;
		}
	}

	gl_mlayer->mapinfo->mappagecou_hi=0;
	gl_mlayer->mapinfo->mappagecou_lo=0;

	for (cou=0; cou<MAX_NUM_OF_DIF_MAP; cou++) {
		ST_MAPBLOCK *map=&gl_mlayer->mapblocks[cou];
		t_ba lba=map->start_frag;

		lba*=MAX_FRAGSIZE;

		ml_createmap(map,0);

		if (ml_get_log2phy(lba)==BLK_NA) return 1;
		if (ml_save_map()) return 1;
	}

  }	

  gl_state=ML_CLOSE;

  return 0;
}

/****************************************************************************
 *
 * ml_findlog
 *
 * Searching for a log block (check if logical block is logged)
 * it sets gl_mlayer->curlog to a log block or set it to NULL
 *
 * INPUTS
 *
 * lba - logical block address
 *
 * RETURNS 
 *
 * 1 - if found
 * 0 - not found
 *
 ***************************************************************************/

static t_bit ml_findlog(t_ba lba) {
ST_LOG *log=gl_mlayer->log;
long a;
	for (a=0; a<MAX_LOG_BLOCK; a++,log++) {
		if (log->lba==lba) {
			gl_mlayer->curlog=log;
			return 1;
		}
	}
	gl_mlayer->curlog=0;
	return 0;
}

/****************************************************************************
 *
 * ml_dostatic
 *
 * does static wear leveling, its read logical block, updates static wear
 * info, and if static request is coming, then copy the block 
 *
 ***************************************************************************/

static void ml_dostatic() {
unsigned long wear;
t_ba pba; 

	gl_mlayer->lbastatic++;
	if (gl_mlayer->lbastatic>=gl_mlayer->data_blk_count) gl_mlayer->lbastatic=0;

	pba=ml_get_log2phy(gl_mlayer->lbastatic);
	if (pba==BLK_NA) return; //nothing to do

	if (!ll_read((t_ba)(gl_mlayer->start_pba+pba),0,gl_buffer)) {; //get original
	 	wear=GET_SPARE_AREA(gl_buffer)->wear;
		if (wear<WEAR_NA) wear_updatestaticinfo(gl_mlayer->lbastatic,wear);
		else wear_updatestaticinfo(gl_mlayer->lbastatic,0);
	}
	else wear_updatestaticinfo(gl_mlayer->lbastatic,0);

	if (wear_check_static()) {
		t_po po;
		t_ba d_pba,lba;
		t_ba s_pba;
		unsigned char index=gl_wear_allocstruct.free_index;

		if (index==INDEX_NA) return;
		d_pba=gl_mlayer->freetable[index];

		if (wll_erase(101,(t_ba)(gl_mlayer->start_pba+d_pba))) {

			d_pba=bb_setasbb(d_pba);			  //get a reserved block
			if (d_pba==BLK_NA) {		 		  //if reserved not available
				gl_mlayer->freetable[index] |= FT_BAD;    //signal it as BAD
			}
			else {
				wear_updatedynamicinfo(index,0); //set new reserved block wear
				gl_mlayer->freetable[index] = d_pba;	 //put f_pba back into FLTable as FREE
			}

			return;
		}

#ifdef _WINDOWS
		cnt_increase(CNT_TSTATIC);
#endif

		lba=gl_wear_allocstruct.static_logblock;
		s_pba=ml_get_log2phy(lba);
		if (s_pba==BLK_NA) return; //nothing to do

		for (po=0; po<MAX_PAGE_PER_BLOCK; po++) {
			int ret=ll_read((t_ba)(gl_mlayer->start_pba+s_pba),po,gl_buffer);
			if (!po) {
				ST_SPARE *sptr=GET_SPARE_AREA(gl_buffer);
				sptr->wear=gl_wear_allocstruct.free_wear;
				sptr->block_type=BLK_TYPE_DAT;
				sptr->bad_tag=0xff;
			}
			else if (ret) continue;

			if (wll_write(10,(t_ba)(gl_mlayer->start_pba+d_pba),po,gl_buffer)) return; //we can stop static swap
		
		}

		ml_set_log2phy(lba,d_pba);
		wear_updatestaticinfo(lba,gl_wear_allocstruct.free_wear);

		gl_mlayer->freetable[index]=s_pba;
		wear_updatedynamicinfo(index,gl_wear_allocstruct.static_wear);

		ml_save_map();
	}
}


/****************************************************************************
 *
 * ml_alloclog
 *
 * alloc log block and initiate it
 *
 * INPUTS
 *
 * log - pointer where to alloc to
 * lba - which logical block is connected to log block
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_alloclog(ST_LOG *log,t_ba lba) {
unsigned char index;
t_ba pba;

	index=ml_alloc();
	if (index==INDEX_NA) return 1;

	pba=gl_mlayer->freetable[index];;

	log->index=index;
	log->lastppo=0;
	log->lba=lba;
	log->pba=pba;
	log->wear=gl_wear_allocstruct.free_wear;
	log->switchable=1;

	gl_mlayer->freetable[index]=(t_ba)(pba | FT_LOG);
	wear_updatedynamicinfo(index,log->wear); //locked now
	gl_mlayer->logblocknum++;

	memset(log->ppo,INDEX_NA,sizeof(log->ppo));

	gl_save_map_need=1;

	return 0;
}

/****************************************************************************
 *
 * ml_open
 *
 * Opening the chanel for read or write from a specified sector
 *
 * INPUTS
 *
 * sector - start of sector for read or write
 * secnum - number of sector will be read or written
 * mode - ML_READ open for read, ML_WRITE open for write
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_open(unsigned long sector, unsigned long secnum, unsigned char mode) {
unsigned char mlcou;
t_ba lba;
	if (gl_state==ML_INIT) return 1;

	lba=(t_ba)(sector/MAX_PAGE_PER_BLOCK);

	for (mlcou=0; mlcou<MLAYER_SETS; mlcou++) {
		gl_mlayer=&mlayers[mlcou];
		if (lba<gl_mlayer->data_blk_count) goto ok;
		lba=(t_ba)(lba-gl_mlayer->data_blk_count);
	}

	return 1; //not found
ok:

	gl_state=ML_CLOSE;
    gl_seccou=secnum;

	gl_lba=lba;
	gl_lpo=(t_po)(sector%MAX_PAGE_PER_BLOCK);

	if (gl_lba>=gl_mlayer->data_blk_count) return 1;
	if (gl_lpo>=MAX_PAGE_PER_BLOCK) return 1;

	if (mode==ML_READ) gl_state=ML_PENDING_READ;
	else if (mode==ML_WRITE) gl_state=ML_PENDING_WRITE;
	else return 1;

	return 0;
}


/****************************************************************************
 *
 * ml_domerge
 *
 * does log block and logical block merge
 *
 * INPUTS
 *
 * log - log block pointer (it contains logical block number also)
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_domerge(ST_LOG *log) {
unsigned long wear=WEAR_NA;
unsigned char po,lpo,index;
t_ba pba;
t_ba orig_pba;


	gl_save_map_need=1;

	if (log->lastppo==MAX_PAGE_PER_BLOCK && log->switchable) { // check if switchable

		orig_pba=ml_get_log2phy(log->lba);
		if (orig_pba==BLK_NA) return 1; // fatal

		ml_set_log2phy(log->lba,log->pba);		//put log block to original
		wear_updatestaticinfo(log->lba,log->wear);

		if (!ll_read((t_ba)(gl_mlayer->start_pba+orig_pba),0,gl_buffer)) {//get original
		 	wear=GET_SPARE_AREA(gl_buffer)->wear;
		}
		else wear=0;

		gl_mlayer->freetable[log->index]=orig_pba; //release original
		if (wear<WEAR_NA) wear_updatedynamicinfo(log->index,wear);
		else wear_updatedynamicinfo(log->index,0);

		gl_mlayer->curlog->lba=BLK_NA;

		gl_mlayer->logblocknum--;

		return 0;
	}
again:
	index=ml_alloc();
	if (index==INDEX_NA) return 1;
	pba=gl_mlayer->freetable[index];
	orig_pba=ml_get_log2phy(log->lba);
	if (orig_pba==BLK_NA) return 1; //fatal

	for (po=0; po<MAX_PAGE_PER_BLOCK; po++) {
		lpo=log->ppo[po];						
		if (lpo==INDEX_NA) {
//			if (po && 
				ll_read((t_ba)(gl_mlayer->start_pba+orig_pba),po,gl_buffer);
			//) continue; //get original
			if (!po) wear=GET_SPARE_AREA(gl_buffer)->wear;
		}
		else {
//			if (po && 
				ll_read((t_ba)(gl_mlayer->start_pba+log->pba),lpo,gl_buffer);
				//) continue;	 //get logged
		}

		if (!po) {
			ST_SPARE *sptr=GET_SPARE_AREA(gl_buffer);
			sptr->wear=gl_wear_allocstruct.free_wear;
			sptr->block_type=BLK_TYPE_DAT;
			sptr->bad_tag=0xff;
		}

		if (wll_write(20,(t_ba)(gl_mlayer->start_pba+pba),po,gl_buffer)) goto again;
	}

	if (wear==WEAR_NA) {
		if (!ll_read((t_ba)(gl_mlayer->start_pba+orig_pba),0,gl_buffer)) {
			wear=GET_SPARE_AREA(gl_buffer)->wear;//get original
		}
		else {
			wear=0;
		}
	}

	gl_mlayer->freetable[index]=orig_pba; //release original
	if (wear<WEAR_NA) wear_updatedynamicinfo(index,wear);
	else wear_updatedynamicinfo(index,0);

	gl_mlayer->freetable[log->index]&=~FT_LOG;	 //release old log block
	wear_updatedynamicinfo(log->index,WEAR_NA);

	ml_set_log2phy(log->lba,pba);		//put new block to original
	wear_updatestaticinfo(log->lba,gl_wear_allocstruct.free_wear);

	gl_mlayer->curlog->lba=BLK_NA;

	gl_mlayer->logblocknum--;

	return 0;
}


/****************************************************************************
 *
 * ml_releaselog
 *
 * releasing a log block for a new logical block
 *
 * INPUTS
 *
 * lba - logical block address for new log block
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

static t_bit ml_releaselog(t_ba lba) {
	if (gl_mlayer->logblocknum < MAX_LOG_BLOCK) {
		if (ml_findlog(BLK_NA)) { //find a free
			return ml_alloclog(gl_mlayer->curlog,lba);
		}
	}

	{
		ST_LOG *log=&gl_mlayer->log[gl_mlayer->logmerge++];
		if (gl_mlayer->logmerge>=MAX_LOG_BLOCK) gl_mlayer->logmerge=0;

		gl_mlayer->curlog=log;
		if (ml_domerge(log)) return 1;
		return ml_alloclog(log,lba);
	}
}

/****************************************************************************
 *
 * ml_write
 *
 * Writing sector data
 *
 * INPUTS
 *
 * data - data pointer for an array which length is sector size
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_write(unsigned char *datap) {

#ifdef _WINDOWS
	cnt_increase(CNT_TWRITES);
#endif

	if (gl_state==ML_PENDING_WRITE) {
		if (!ml_findlog(gl_lba))	{
			if (ml_releaselog(gl_lba)) return 1;
		}
		gl_state=ML_WRITE;
	}

	if (gl_state==ML_WRITE) {

      if (!gl_seccou--) {
         gl_state=ML_ABORT;
         return 1;
      }

		if (gl_lpo>=MAX_PAGE_PER_BLOCK) {
			gl_lba++;

			if (gl_lba>=gl_mlayer->data_blk_count) {
				if (gl_mlayer->laynum+1<MLAYER_SETS) {
					gl_mlayer++;
					gl_lba=0;
				}
				else {
					gl_state=ML_CLOSE;
					return 1;
				}
			}
			
			if (!ml_findlog(gl_lba)) {
				if (ml_releaselog(gl_lba)) return 1;
			}
			gl_lpo=0;
		}

		if (gl_mlayer->curlog->lastppo>=MAX_PAGE_PER_BLOCK) {
			if (ml_domerge(gl_mlayer->curlog)) return 1;
			if (ml_alloclog(gl_mlayer->curlog,gl_lba)) return 1;		//allocate a new log block
		}


		memcpy (gl_buffer,datap,MAX_DATA_SIZE);

		{
			ST_SPARE *sptr=GET_SPARE_AREA(gl_buffer);
			if (!gl_mlayer->curlog->lastppo) {
				sptr->wear=gl_mlayer->curlog->wear;
				sptr->block_type=BLK_TYPE_DAT;
				sptr->bad_tag=0xff;
			}
			sptr->u.log.lba=gl_lba;
			sptr->u.log.lpo=gl_lpo;
		}

		if (gl_lpo!=gl_mlayer->curlog->lastppo) gl_mlayer->curlog->switchable=0; //not switchable from this point

		gl_mlayer->curlog->ppo[gl_lpo++]=gl_mlayer->curlog->lastppo;

		wll_write(40,(t_ba)(gl_mlayer->start_pba+gl_mlayer->curlog->pba),gl_mlayer->curlog->lastppo++,gl_buffer);

		if (gl_save_map_need) {
			ml_save_map();
			ml_dostatic();
		}

		return 0;
	}

	return 1;
}

/****************************************************************************
 *
 * ml_read
 *
 * read sector data
 *
 * INPUTS
 *
 * data - where to read a given sector
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_read(unsigned char *datap) {
	if (gl_state==ML_PENDING_READ) {
		gl_mlayer->pba=ml_get_log2phy(gl_lba);
		if (gl_mlayer->pba==BLK_NA) return 1;

		ml_findlog(gl_lba);
		gl_state=ML_READ;
	}

	if (gl_state==ML_READ) {
		int ret;

		if (!gl_seccou--) {
			gl_state=ML_ABORT;
			return 1;
		}

		if (gl_lpo>=MAX_PAGE_PER_BLOCK) {
			gl_lba++;
			if (gl_lba>=gl_mlayer->data_blk_count) {
				if (gl_mlayer->laynum+1<MLAYER_SETS) {
					gl_mlayer++; //goto next set
					gl_lba=0;
				}
				else {
					gl_state=ML_CLOSE;
					return 1;
				}
			}
			gl_mlayer->pba=ml_get_log2phy(gl_lba);
			if (gl_mlayer->pba==BLK_NA) return 1;

			ml_findlog(gl_lba);
			gl_lpo=0;
		}

		if (gl_mlayer->curlog) {
			t_po po=gl_mlayer->curlog->ppo[gl_lpo];
			if (po==INDEX_NA) ret=ll_read((t_ba)(gl_mlayer->start_pba+gl_mlayer->pba),gl_lpo,gl_buffer);
			else ret=ll_read((t_ba)(gl_mlayer->start_pba+gl_mlayer->curlog->pba),po,gl_buffer);
		}
		else {
			ret=ll_read((t_ba)(gl_mlayer->start_pba+gl_mlayer->pba),gl_lpo,gl_buffer);
		}

		memcpy (datap,gl_buffer,MAX_DATA_SIZE);

		gl_lpo++;

		if (ret==LL_OK) return 0;
		else if (ret==LL_ERASED) return 0; //erased
		return 1;
	}

	return 1;
}

/****************************************************************************
 *
 * ml_close
 *
 * closing sector reading or writing
 *
 * RETURNS
 *
 * 0 - if success
 * other if any error
 *
 ***************************************************************************/

t_bit ml_close() {
t_bit ret=0;

	if (gl_state==ML_INIT) return 1;

	if (gl_seccou) ret=1;
	if (gl_state==ML_ABORT) ret=1;

	gl_state=ML_CLOSE;
	return ret;
}


/****************************************************************************
 *
 * End of emlayer.c
 *
 ***************************************************************************/

#endif	//_MLAYER_C_
