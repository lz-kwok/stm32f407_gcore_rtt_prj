/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 * 2019-09-12     Guolz     v1.0.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <at.h>
#include <string.h>
#include <stdarg.h>
#include  <math.h>
#include "RTL.h"
#include "File_Config.h"
#include "drv_nand.h"

static const char * ReVal_Table[]= 
{
	"0：成功",				                        
	"1：IO错误，I/O驱动初始化失败，或者没有存储设备，或者设备初始化失败",
	"2：卷错误，挂载失败，对于FAT文件系统意味着无效的MBR，启动记录或者非FAT格式",
	"3：FAT日志初始化失败，FAT初始化成功了，但是日志初始化失败",
};

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)


void phy_reset(void)
{
    rt_pin_mode(phy_rst_pin,PIN_MODE_OUTPUT);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_pin_write(phy_rst_pin, PIN_LOW);
    rt_thread_mdelay(500);
    rt_kprintf("%s done\r\n",__func__);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
}

static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* 数值大于等于10^9 */
	if (_ullVal >= (U64)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 数值大于等于10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* 其它数值 */
	sprintf (_sp,"%d",(U32)(_ullVal));
}

static void ViewNandCapacity(void)
{
	uint8_t result;
	Media_INFO info;
	uint64_t ullNANDCapacity;
	FAT_VI *mc0;  
	uint8_t buf[15];
	
	/* 
	   低级格式化，注意一定要执行了函数NAND_Init后再执行NAND_Format()，因为：
	   1. finit("N0:") 会调用NAND_Init，见FlahFS对nandflash的接口文件FS_NAND_FlashPrg.c
	   2. funinit("N0:") 会调用UnInit，见FlahFS对nandflash的接口文件FS_NAND_FlashPrg.c
	*/
	// rt_hw_mtd_nand_init();
	// rt_kprintf("正在进行低级格式化中....\r\n");
	// NAND_EraseChip();
	// rt_kprintf("低级格式化完成....\r\n");

	/* 加载SD卡 */
	result = finit("N0:");
	if(result != NULL)
	{
		rt_kprintf("挂载文件系统失败 (%s)\r\n", ReVal_Table[result]);
		rt_kprintf("挂载失败，NAND Flash需要进行FAT32格式化\r\n");
	}
	else
	{
		rt_kprintf("挂载文件系统成功 (%s)\r\n", ReVal_Table[result]);
	}
	
	rt_kprintf("正在进行FAT32格式化中....\r\n");
	if (fformat ("N0:") != 0)  
	{            
		rt_kprintf ("格式化失败\r\n");
	}
	else  
	{
		rt_kprintf ("格式化成功\r\n");
	}
	
	rt_kprintf("------------------------------------------------------------------\r\n");
	/* 获取volume label */
	if (fvol ("N0:", (char *)buf) == 0) 
	{
		if (buf[0]) 
		{
			rt_kprintf ("NAND Flash的volume label是 %s\r\n", buf);
		}
		else 
		{
			rt_kprintf ("NAND Flash没有volume label\r\n");
		}
	}
	else 
	{
		rt_kprintf ("Volume访问错误\r\n");
	}

	/* 获取NAND Flash剩余容量 */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	rt_kprintf("NAND Flash剩余容量 = %10s字节\r\n", buf);
	
	/* 卸载NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		rt_kprintf("卸载文件系统失败\r\n");
	}
	else
	{
		rt_kprintf("卸载文件系统成功\r\n");
	}
	
	/* 获取相应存储设备的句柄 */
	mc0 = ioc_getcb("N0");          
   
	/* 初始化FAT文件系统格式的存储设备 */
	if (ioc_init (mc0) == 0) 
	{
		/* 获取存储设备的扇区信息 */
		ioc_read_info (&info, mc0);

		/* 总的扇区数 * 扇区大小，NAND Flash的扇区大小是512字节 */
		ullNANDCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullNANDCapacity, (char *)buf);
		rt_kprintf("NAND Flash总容量 = %10s字节\r\nNAND Flash的总扇区数 = %d \r\n", buf, info.block_cnt);
	
		rt_kprintf("NAND Flash读扇区大小 = %d字节\r\n", info.read_blen);
		rt_kprintf("NAND Flash写扇区大小 = %d字节\r\n", info.write_blen);
	}
	else 
	{
		rt_kprintf("初始化失败\r\n");
	}
	
	/* 卸载NAND Flash */
	if(ioc_uninit (mc0) != NULL)
	{
		rt_kprintf("卸载失败\r\n");		
	}
	else
	{
		rt_kprintf("卸载成功\r\n");	
	}
	rt_kprintf("------------------------------------------------------------------\r\n");
}


int main(void)
{
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    rt_thread_mdelay(2000);
    ViewNandCapacity();
    while (RT_TRUE)
    {   
       

        // g_Gui_show_pic("5");
        rt_thread_mdelay(2000);
        // g_Gui_show_pic("F");
    }

    return RT_EOK;
}



