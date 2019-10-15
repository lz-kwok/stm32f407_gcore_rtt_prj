#ifndef _PRGMACRO_C_
#define _PRGMACRO_C_

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

#include "mlayer.h"
#include "llayer.h"
#include "prgmacro.h"
#include "mdebug.h"

extern void prg_winshow(void);

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/****************************************************************************
 *
 * static variables
 *
 ***************************************************************************/

static long *gl_prg;
static long gl_state;
static long gl_pc;
static unsigned long gl_counters[CNT_MAX];

/****************************************************************************
 *
 * cnt_resets
 *
 * resetting all the counters
 *
 ***************************************************************************/

void cnt_resets() {
unsigned char a;
	for (a=0; a<CNT_MAX; a++) gl_counters[a]=0;
}

/****************************************************************************
 *
 * cnt_increase
 *
 * increasing a counter value
 *
 * INPUTS
 *
 * counter - which counter need to be increased (CNT_xxx)
 *
 ***************************************************************************/


void cnt_increase(unsigned char counter) {
	if (counter<CNT_MAX) gl_counters[counter]++;
	if (counter==CNT_ERROR) {
		counter=CNT_ERROR;
	}
}

/****************************************************************************
 *
 * cnt_getvalue
 *
 * getting a counter current value
 *
 * INPUTS
 *
 * counter - which counter value is needed (CNT_xxx)
 *
 * RETURNS
 *
 * returns with the counter value
 *
 ***************************************************************************/

unsigned long cnt_getvalue(unsigned char counter) {
	if (counter<CNT_MAX) return gl_counters[counter];
	return 0;
}

enum {
    CHECK_LEN = 2
};


unsigned long sectorrand[MAX_SECTOR*MLAYER_SETS];

/****************************************************************************
 debug code to check page data content ...
 ****************************************************************************/

//static long filtest_page_data_value = 0;
unsigned char prg_buffer [MAX_DATA_SIZE];

#define CHECK_LEN 2

void prg_set_content(long sector) {
    long *lptr = (long *) prg_buffer;
	unsigned long r=rand();

	r<<=16;
	r+=rand();

	lptr[0] = sector;
	lptr[1] = r;

	sectorrand[sector]=r;
}

int prg_check_content(unsigned long sector) {
unsigned long *lptr = (unsigned long *) prg_buffer;

	if (sector != lptr[0] || sectorrand[sector]!=lptr[1]) {
		cnt_increase(CNT_ERROR);
//        printf("error at page %l: exp %lx got %lx offset %lx\n", sector, sector, lptr[0], lptr[1]);
        return 1;
    }
    return 0; //ok
}

/****************************************************************************
 *
 * prg_srand
 *
 * reinitialize the random number generator
 *
 ***************************************************************************/

static void prg_srand(void) {
  srand(0);        //040611_nicola
}

/****************************************************************************
 *
 * prg_readsector
 *
 * read a simple sector n times
 *
 * INPUTS
 *
 * sector - which sector is requested
 * times - number of writing
 *
 ***************************************************************************/

static void prg_readsector(long sector,long times) {
	while (times--) {
	    if (ml_open(sector, 1,ML_READ)) cnt_increase(CNT_ERROR);
			
	    if (ml_read(prg_buffer)) cnt_increase(CNT_ERROR);
			
	    if (ml_close())	cnt_increase(CNT_ERROR);

        prg_check_content(sector);
	}
}

/****************************************************************************
 *
 * prg_readlinear
 *
 * reading a range of sectors n times linearly
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * endsector - last sector (this sector won't be written!)
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_readlinear(long startsector,long endsector,long times) {
long sector;
	while (times--) {
		for (sector=startsector; sector<endsector; sector++) {
            if (ml_open(sector, 1 ,ML_READ)) cnt_increase(CNT_ERROR);
		    if (ml_read(prg_buffer)) cnt_increase(CNT_ERROR);
		    if (ml_close()) cnt_increase(CNT_ERROR);

            prg_check_content(sector);
		}
	}
}


/****************************************************************************
 *
 * prg_readbatch
 *
 * reading a range of sectors n times linearly, with 1 open
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * numsector - number of sectors
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_readbatch(long startsector,long numsector,long times) {
long sector;
	while (times--) {
        if (ml_open(startsector,numsector,ML_READ)) cnt_increase(CNT_ERROR);

		for (sector=0; sector<numsector; sector++) {

		    if(ml_read(prg_buffer)) cnt_increase(CNT_ERROR);

            prg_check_content(sector+startsector);
		}

        if (ml_close()) cnt_increase(CNT_ERROR);
	}
}

/****************************************************************************
 *
 * prg_readrandom
 *
 * writing a range of sectors n times randomly
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * endsector - last sector (this sector won't be written!)
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_readrandom(long startsector,long endsector,long times) {
long range=endsector-startsector;
	while (times--) {
		unsigned long r=rand();

		r<<=16;
		r+=rand();
		r%=range;

		if (ml_open(r+startsector, 1,ML_READ)) cnt_increase(CNT_ERROR);
		if (ml_read(prg_buffer)) cnt_increase(CNT_ERROR);
		if (ml_close()) cnt_increase(CNT_ERROR);

        prg_check_content(r+startsector);
	}
}
  


/****************************************************************************
 *
 * prg_writesector
 *
 * writing a simple sector n times
 *
 * INPUTS
 *
 * sector - which sector is requested
 * times - number of writing
 *
 ***************************************************************************/
		 
static void prg_writesector(long sector,long times) {
	while (times--) {

    	if (ml_open(sector, 1,ML_WRITE)) cnt_increase(CNT_ERROR);

        prg_set_content(sector);

		if (ml_write(prg_buffer)) cnt_increase(CNT_ERROR);
	    if (ml_close()) cnt_increase(CNT_ERROR);
	}
}

/****************************************************************************
 *
 * prg_writelinear
 *
 * writing a range of sectors n times linearly
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * endsector - last sector (this sector won't be written!)
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_writelinear(long startsector,long endsector,long times) {
long sector;
	while (times--) {
   		for (sector=startsector; sector<endsector; sector++) {

   			if (ml_open(sector,1,ML_WRITE)) cnt_increase(CNT_ERROR);

            prg_set_content(sector);

		    if (ml_write(prg_buffer)) cnt_increase(CNT_ERROR);
    		if (ml_close()) cnt_increase(CNT_ERROR);
        }
	}
}


/****************************************************************************
 *
 * prg_writebatch
 *
 * writing a range of sectors n times linearly with 1 open
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * numsector - number of sectors
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_writebatch(long startsector,long numsector,long times) {
long sector;
	while (times--) {
   		if (ml_open(startsector, numsector,ML_WRITE)) cnt_increase(CNT_ERROR);

   		for (sector=0; sector<numsector; sector++) {
		    prg_set_content(sector+startsector);

		    if (ml_write(prg_buffer)) cnt_increase(CNT_ERROR);
        }
     	if (ml_close()) cnt_increase(CNT_ERROR);
	}
}

/****************************************************************************
 *
 * prg_writerandom
 *
 * writing a range of sectors n times randomly
 *
 * INPUTS
 *
 * startsector - 1st sector in range
 * endsector - last sector (this sector won't be written!)
 * times - number of cycles
 *
 ***************************************************************************/

static void prg_writerandom(long startsector,long endsector,long times) {
long range=endsector-startsector;
	while (times--) {
		unsigned long r=rand();
		r<<=16;
		r+=rand();
		r%=range;

    	if (ml_open(r+startsector, 1,ML_WRITE)) cnt_increase(CNT_ERROR);

        prg_set_content(r+startsector);

		if (ml_write(prg_buffer)) cnt_increase(CNT_ERROR);
	    if (ml_close()) cnt_increase(CNT_ERROR);
	}
}
  
/****************************************************************************
 *
 * prg_reinit
 *
 * reinit FIL layer
 *
 *******
********************************************************************/

//extern ST_MAPDIR ml_mapdir[MAX_FRAGNUM];
//ST_MAPDIR ml_copymapdir[MAX_FRAGNUM];
void copymap() {
//	memcpy(ml_copymapdir,ml_mapdir,sizeof(ml_mapdir));
}

int checkmap() {
//char *p1=(char*)ml_mapdir;
//char *p2=(char*)ml_copymapdir;
//long a;
//	for (a=0; a<sizeof(ml_mapdir);a++) {
//		if (*p1++!=*p2++) return 1;
//	}
	return 0;
}

static void prg_reinit() {
	copymap();
	if (ml_init()) {
		cnt_increase(CNT_FATALERROR);
	}
	if (checkmap()) {
		cnt_increase(CNT_ERROR);
	}
}


/****************************************************************************
 *
 * prg_powerfail
 *
 ***************************************************************************/

#define MAXEVENT 255

#define EVENT_BEFORE 0x01
#define EVENT_DURING 0x02
#define EVENT_AFTER  0x04

typedef struct {
	long eventwhen;
	long eventnum;
} ST_EVENT;

ST_EVENT prg_pfevents[MAXEVENT];

static void prg_powerfail(long onevent, long eventwhen,long eventnum) {
	if (onevent>=0 && onevent<MAXEVENT) {
		prg_pfevents[onevent].eventnum=eventnum;
		prg_pfevents[onevent].eventwhen=eventwhen;
	}
}

ST_EVENT prg_ffevents[MAXEVENT];

static void prg_flashfail(long onevent, long eventwhen,long eventnum) {
	if (onevent>=0 && onevent<MAXEVENT) {
		prg_ffevents[onevent].eventnum=eventnum;
		prg_ffevents[onevent].eventwhen=eventwhen;
	}
}

#include <setjmp.h>

jmp_buf ret_jump;


int prg_calls(long f, long p0,long p1,long p2,long p3) {
	if (f==(long)ll_erase) return ll_erase((t_ba)p0);
	if (f==(long)ll_read) return ll_read((t_ba)p0,(t_po)p1,(unsigned char *)p2);
	if (f==(long)ll_write) return ll_write((t_ba)p0,(t_po)p1,(unsigned char *)p2);
	if (f==(long)ll_writedouble) return ll_writedouble((t_ba)p0,(t_po)p1,(unsigned char *)p2,(unsigned char *)p3);

	return LL_ERROR;	//missing wrapper function
}

//void prg_destroy(long f,long p0,long p1,long p2,long p3) {
//}

int gl_powerfailstress=0;

int prg_wrapper(long f,long p0,long p1,long p2,long p3, long event) {
long what=0;
long when=0;
int ret;


    if (gl_powerfailstress) {
		int a=rand();
		if (f==(long)ll_erase) a&=16383; //less percentage of erase
		else a&=1023;
		if (!a) {               //set a percentage of power failure low
			switch (rand()%6) {
			case 0:
				prg_pfevents[event].eventwhen=1;          //before
				prg_pfevents[event].eventnum=1; //create power fail
				break;
			case 1:
				prg_pfevents[event].eventwhen=2;          //during
				prg_pfevents[event].eventnum=1; //create power fail
				break;
			case 2:
				prg_pfevents[event].eventwhen=3;          //after
				prg_pfevents[event].eventnum=1; //create power fail
				break;

			case 3:
				prg_ffevents[event].eventwhen=1;          //before
				prg_ffevents[event].eventnum=1; //create flash fail
				break;
			case 4:
				prg_ffevents[event].eventwhen=2;          //during
				prg_ffevents[event].eventnum=1; //create flash fail
				break;
			case 5:
				prg_ffevents[event].eventwhen=3;          //after
				prg_ffevents[event].eventnum=1; //create flash fail
				break;
			}
		}
    }

	if (prg_ffevents[event].eventnum) {
		prg_ffevents[event].eventnum--;
		when=prg_ffevents[event].eventwhen;
		what=1;
	}
	else if (prg_pfevents[event].eventnum) {
		prg_pfevents[event].eventnum--;
		when=prg_pfevents[event].eventwhen;
		what=2;
	}
	else return prg_calls(f,p0,p1,p2,p3);


	switch (when) {
	case 1: //before
			if (what==2) longjmp(ret_jump,1); 
			return LL_ERROR; 

	case 2: //during
			prg_calls(f,p0,p1,p2,p3);
//			prg_destroy(f,p0,p1,p2,p3);
			if (what==2) longjmp(ret_jump,2);
			return LL_ERROR; //signal error

	case 3: //after finished
			ret=prg_calls(f,p0,p1,p2,p3);
			if (what==2) longjmp(ret_jump,2);

//			prg_destroy(f,p0,p1,p2,p3);
			return ret; //may be error is not signalled (in flashfail)
		}

	return LL_ERROR;	//missing wrapper function
}

/****************************************************************************
 *
 * prg_load
 *
 * loading an external program, and reset program states
 *
 * INPUTS
 *
 * prg - pointer to the compiled program array
 *
 ***************************************************************************/

void prg_load(long *prg) {
//	memset (prg_pfevents,0,sizeof(prg_pfevents));
//	memset (prg_ffevents,0,sizeof(prg_ffevents));

	gl_prg=prg;
	gl_state=PRG_LOADED|PRG_RUNNING;
	gl_pc=0;
	gl_powerfailstress=0;
}

/****************************************************************************
 *
 * prg_step
 *
 * step one instruction forward in the program
 *
 ***************************************************************************/

void prg_step() {
long rpc=gl_pc >> 2;

	if (setjmp(ret_jump)) {
		copymap();
		if (ml_init()) {
			cnt_increase(CNT_FATALERROR);
		}
		if (checkmap()) {
			cnt_increase(CNT_ERROR);
		}
		return;
	}

	if (! (gl_state&PRG_RUNNING)) return;

	switch (gl_prg[rpc]) {
	case PRG_BROPEN: 
					 gl_pc+=2*4;
					 gl_prg[gl_prg[rpc+1] >> 2]=0;
					 break;

	case PRG_BRCLOSE:
					 gl_prg[rpc+1]++;
					 if (gl_prg[rpc+1]<gl_prg[rpc+2]) gl_pc=gl_prg[rpc+3];
					 else gl_pc+=4*4;
					 break;

	case PRG_WRSECTOR:
					 gl_pc+=3*4;
					 prg_writesector(gl_prg[rpc+1],gl_prg[rpc+2]);
					 break;

	case PRG_WRLINEAR:
					 gl_pc+=4*4;
					 prg_writelinear(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;

	case PRG_WRBATCH:
					 gl_pc+=4*4;
					 prg_writebatch(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;


	case PRG_WRRANDOM:
					 gl_pc+=4*4;
					 prg_writerandom(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;


	case PRG_RDSECTOR:
					 gl_pc+=3*4;
					 prg_readsector(gl_prg[rpc+1],gl_prg[rpc+2]);
					 break;

	case PRG_RDLINEAR:
					 gl_pc+=4*4;
					 prg_readlinear(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;

	case PRG_RDBATCH:
					 gl_pc+=4*4;
					 prg_readbatch(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;

	case PRG_RDRANDOM:
					 gl_pc+=4*4;
					 prg_readrandom(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;

	case PRG_SHOW:  
					gl_pc+=4;
					prg_winshow();
					 break;

	case PRG_SRAND: 
					gl_pc+=4;
					prg_srand();
					 break;
			   
	case PRG_REINIT: 
					 gl_pc+=4;
					 prg_reinit();
					 break;

	case PRG_POWERFAIL:
					 gl_pc+=4*4;
					 prg_powerfail(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;

	case PRG_FLASHFAIL:
					 gl_pc+=4*4;
					 prg_flashfail(gl_prg[rpc+1],gl_prg[rpc+2],gl_prg[rpc+3]);
					 break;
					 
	case PRG_POWERFAILSTRESS:  
					gl_pc+=4;
                    gl_powerfailstress=1;
					break;

	case PRG_END:
			 gl_state|=PRG_FINISHED;
			 gl_state&=~PRG_RUNNING;
			 prg_winshow();
			 break;

	default: gl_state|=PRG_ERROR;
			 gl_state&=~PRG_RUNNING;
	}
}

/****************************************************************************
 *
 * prg_getstate
 *
 * get current program status (see PRG_xxx definitions in header for status)
 *
 * RETURNS
 *
 * current program state
 *
 ***************************************************************************/

long prg_getstate() {
	return gl_state;
}

/****************************************************************************
 *
 * prg_stop
 *
 * stop the program execution
 *
 ***************************************************************************/


void prg_stop() {
	gl_state&=~PRG_RUNNING;
	prg_winshow();
}

/****************************************************************************
 *
 * End of prgmacro.c
 *
 ***************************************************************************/

#endif	//_PRGMACRO_C_
