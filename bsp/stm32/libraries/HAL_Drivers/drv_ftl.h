/*
 * Copyright (c) 2009-present, by Guoliangzhi
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author            Notes
 * 2019-10-06     Guoliangzhi        first version
 */
#ifndef __DRV_FTL_H__
#define __DRV_FTL_H__

#include <rtthread.h>


struct ftl_flash_device
{
    struct rt_device                flash_device;
    struct rt_device_blk_geometry   geometry;
    struct rt_mutex                 lock;
};

//坏块搜索控制
//如果设置为1,将在FTL_Format的时候,搜寻坏块,耗时久(512M,3分钟以上),且会导致RGB屏乱闪
#define FTL_USE_BAD_BLOCK_SEARCH		0		//定义是否使用坏块搜索



rt_uint8_t FTL_Init(void); 
void FTL_BadBlockMark(rt_uint32_t blocknum);
rt_uint8_t FTL_CheckBadBlock(rt_uint32_t blocknum); 
rt_uint8_t FTL_UsedBlockMark(rt_uint32_t blocknum);
rt_uint32_t FTL_FindUnusedBlock(rt_uint32_t sblock,rt_uint8_t flag);
rt_uint32_t FTL_FindSamePlaneUnusedBlock(rt_uint32_t sblock);
rt_uint8_t FTL_CopyAndWriteToBlock(rt_uint32_t Source_PageNum,rt_uint16_t ColNum,rt_uint8_t *pBuffer,rt_uint32_t NumByteToWrite);
rt_uint16_t FTL_LBNToPBN(rt_uint32_t LBNNum); 
rt_uint8_t FTL_WriteSectors(rt_uint8_t *pBuffer,rt_uint32_t SectorNo,rt_uint16_t SectorSize,rt_uint32_t SectorCount);
rt_uint8_t FTL_ReadSectors(rt_uint8_t *pBuffer,rt_uint32_t SectorNo,rt_uint16_t SectorSize,rt_uint32_t SectorCount);
rt_uint8_t FTL_CreateLUT(rt_uint8_t mode);
rt_uint8_t FTL_BlockCompare(rt_uint32_t blockx,rt_uint32_t cmpval);
rt_uint32_t FTL_SearchBadBlock(void);
rt_uint8_t FTL_Format(void); 



#endif



