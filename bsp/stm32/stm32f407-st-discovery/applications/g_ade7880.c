 /*
 * Change Logs:
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <g_measure.h>
#include <g_uart.h>
#include <g_usb_cdc.h>
#include <g_ade7880.h>

#define SPI_BUS_NAME                "spi2"
#define SPI_ADE7880_DEVICE_NAME     "ade7880"

#define CS_PIN               75

#define ADE7880_DEBUG

#ifdef  ADE7880_DEBUG
#define ADE7880_TRACE         rt_kprintf
#else
#define ADE7880_TRACE(...)
#endif 

struct stm32_hw_spi_cs
{
    rt_uint32_t pin;
};

rt_uint32_t IRQStautsRead0 = 0;
rt_uint32_t IRQStautsRead1 = 0;
rt_uint8_t IRQFlag = 0;

rt_uint32_t PhaseAEnergy = 0;		 
rt_uint32_t PhaseAIRMS   = 0;
rt_uint32_t PhaseAVRMS   = 0;
rt_uint16_t PhaseAPeroid = 0;
rt_uint32_t PhaseAAngle  = 0;

static struct rt_spi_device spi_dev_ade7880;    // SPI设备ssd1351对象 
static struct stm32_hw_spi_cs  spi_cs;	        // SPI设备CS片选引脚 


static void rt_hw_ade7880_reset(void)
{
	rt_pin_mode(ade7880_rst_pin,PIN_MODE_OUTPUT);
	rt_thread_mdelay(10);
	rt_pin_write(phy_rst_pin, PIN_LOW);
	rt_thread_mdelay(30);		
    ADE7880_TRACE("%s done\r\n",__func__);
	rt_pin_write(phy_rst_pin, PIN_HIGH);
	rt_thread_mdelay(100);	       	
}


static void rt_hw_ade7880_pm_select(PSM_MODE psm_mode) 
{
    rt_pin_mode(ade7880_pm0_pin,PIN_MODE_OUTPUT);
    rt_pin_mode(ade7880_pm1_pin,PIN_MODE_OUTPUT);
    switch (psm_mode) {
        case PSM0:
            //order PSM0: PM1=0, PM0=1
            rt_pin_write(ade7880_pm0_pin, PIN_HIGH);
            rt_pin_write(ade7880_pm1_pin, PIN_LOW);
            break;
        case PSM1:
            //order PSM1: PM1=0, PM0=0
            rt_pin_write(ade7880_pm0_pin, PIN_LOW);
            rt_pin_write(ade7880_pm1_pin, PIN_LOW);
            break;
        case PSM2:
            //order PSM2: PM1=1, PM0=0
            rt_pin_write(ade7880_pm0_pin, PIN_LOW);
            rt_pin_write(ade7880_pm1_pin, PIN_HIGH);
            break;
        case PSM3:
            //order PSM3: PM1=1, PM0=1
            rt_pin_write(ade7880_pm0_pin, PIN_HIGH);
            rt_pin_write(ade7880_pm1_pin, PIN_HIGH);
            break
    }
}

static void rt_hw_ade7880_spi_setup(void)
{
    rt_pin_write(spi_cs.pin, PIN_HIGH);
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_LOW);	  //toggle once
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_HIGH);
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_LOW);	  //toggle twice
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_HIGH);
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_LOW);	  //triple triple
	rt_thread_mdelay(5);
	rt_pin_write(spi_cs.pin, PIN_HIGH);
	rt_thread_mdelay(5);

	// SPIWrite1Byte(0xEC01, 0x00);	 //SPI is the active serial port, any write to CONFIG2 register locks the port

	//The 2nd way to choose SPI mode is to execute three SPI write operations 
	//to a location in the address space that is not allocated to a specific ADE78xx register
	/*
	SPIWrite1Byte(0xEBFF, 0x00);								 
	SPI_read_32(0xEBFF);
	SPIWrite1Byte(0xEBFF, 0x00);
	SPI_read_32(0xEBFF);
	SPIWrite1Byte(0xEBFF, 0x00);
	SPI_read_32(0xEBFF);
	*/
}



static int rt_hw_ade7880_spi_config(void)
{
    rt_err_t res;

    // ade7880 use PE11 as CS 
    spi_cs.pin = CS_PIN;
    rt_pin_mode(spi_cs.pin, PIN_MODE_OUTPUT);    // 设置片选管脚模式为输出 

    res = rt_spi_bus_attach_device(&spi_dev_ade7880, SPI_ADE7880_DEVICE_NAME, SPI_BUS_NAME, (void*)&spi_cs);
    if (res != RT_EOK)
    {
        ADE7880_TRACE("rt_spi_bus_attach_device!\r\n");
        return res;
    }

    /* config spi */
    {
        struct rt_spi_configuration cfg;
        cfg.data_width = 8;
        cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        cfg.max_hz = 5 * 1000 *1000;       // 5M,SPI max 42MHz 

        rt_spi_configure(&spi_dev_ade7880, &cfg);
    }

    return RT_EOK;
}

static void SPIDelay(void)
{
	int i;
	for(i=0; i<50; i++);
}

static void SPIWrite2Bytes(rt_uint16_t address , rt_uint16_t sendtemp)
{
	rt_uint8_t szTxData[7];
	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x00; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address);
	szTxData[3] = (rt_uint8_t)(sendtemp>>8);
	szTxData[4] = (rt_uint8_t)(sendtemp);

    rt_spi_send(&spi_dev_ade7880,szTxData,5);
    SPIDelay();
	rt_pin_write(spi_cs.pin, PIN_HIGH);
}

static void SPIWrite4Bytes(rt_uint16_t address , rt_uint32_t sendtemp)
{
	rt_uint8_t szTxData[7];
	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x00; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address);
	szTxData[3] = (rt_uint8_t)(sendtemp>>24);
	szTxData[4] = (rt_uint8_t)(sendtemp>>16);
	szTxData[5] = (rt_uint8_t)(sendtemp>>8);
	szTxData[6] = (rt_uint8_t)(sendtemp);
	
    rt_spi_send(&spi_dev_ade7880,szTxData,7);
    SPIDelay();
	rt_pin_write(spi_cs.pin, PIN_HIGH);
}

static void SPIWrite1Byte(rt_uint16_t address , rt_uint8_t sendtemp)
{
	char i;
	rt_uint8_t szTxData[7];
	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x00; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address);
	szTxData[3] = sendtemp;

	rt_spi_send(&spi_dev_ade7880,szTxData,4);
    SPIDelay();
	rt_pin_write(spi_cs.pin, PIN_HIGH);
}

static rt_uint32_t SPIRead4Bytes(rt_uint16_t address)
{
	rt_uint32_t readout;
	rt_uint8_t szTxData[7];
	rt_uint8_t spireadbuf[4];

	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x01; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address); 
	
    rt_spi_send_then_recv(&spi_dev_ade7880,szTxData,3,spireadbuf,4);

	readout = (spireadbuf[0]<<24)+(spireadbuf[1]<<16)+(spireadbuf[2]<<8)+spireadbuf[3];

	rt_pin_write(spi_cs.pin, PIN_HIGH);

	return(readout); 
}

static rt_uint16_t SPIRead2Bytes(rt_uint16_t address)
{
	rt_uint16_t readout;
	rt_uint8_t szTxData[7];
	rt_uint8_t spireadbuf[2];

	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x01; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address); 
	
	rt_spi_send_then_recv(&spi_dev_ade7880,szTxData,3,spireadbuf,2);
		
	readout = (spireadbuf[0]<<8)+spireadbuf[1];

	rt_pin_write(spi_cs.pin, PIN_HIGH);

	return(readout);
}

static rt_uint8_t SPIRead1Bytes(rt_uint16_t address)
{
	rt_uint8_t szTxData[7];
	rt_uint8_t spireadbuf;

	rt_pin_write(spi_cs.pin, PIN_LOW);
	szTxData[0] = 0x01; 	
	szTxData[1] = (rt_uint8_t)(address>>8);
	szTxData[2] = (rt_uint8_t)(address); 
	
	rt_spi_send_then_recv(&spi_dev_ade7880,szTxData,3,&spireadbuf,1);

	rt_pin_write(spi_cs.pin, PIN_HIGH);

	return(spireadbuf);
}

void rt_hw_ade7880_reg_cfg(void)
{
	SPIWrite2Bytes(Gain,0x0000);			  //config the  Gain of the PGA , which is before the ADC of ADE7880
	SPIWrite2Bytes(CONFIG,0x0000);			  //
	
	SPIWrite4Bytes(AIGAIN,0x00000000);		  //calibration
	SPIWrite4Bytes(AVGAIN,0x00000000);
	SPIWrite4Bytes(BIGAIN,0x00000000);
	SPIWrite4Bytes(BVGAIN,0x00000000);
	SPIWrite4Bytes(CIGAIN,0x00000000);
	SPIWrite4Bytes(CVGAIN,0x00000000);
	SPIWrite4Bytes(NIGAIN,0x00000000);
 	SPIWrite4Bytes(AIRMSOS,0x00000000);
	SPIWrite4Bytes(AVRMSOS,0x00000000);
	SPIWrite4Bytes(BIRMSOS,0x00000000);
	SPIWrite4Bytes(BVRMSOS,0x00000000);
	SPIWrite4Bytes(CIRMSOS,0x00000000);
	SPIWrite4Bytes(CVRMSOS,0x00000000);
	SPIWrite4Bytes(NIRMSOS,0x00000000);
	SPIWrite4Bytes(AWATTOS,0x00000000);
	SPIWrite4Bytes(BWATTOS,0x00000000);
	SPIWrite4Bytes(CWATTOS,0x00000000);
	SPIWrite2Bytes(APHCAL,0x0000);
	SPIWrite2Bytes(BPHCAL,0x0000);
	SPIWrite2Bytes(CPHCAL,0x0000);

	SPIWrite2Bytes(CF1DEN,0x00A7);	  		  //configure the ENERGY-TO-FREQUENCY
	SPIWrite2Bytes(CF2DEN,0x00A7);
	SPIWrite2Bytes(CF3DEN,0x00A7);
	SPIWrite2Bytes(CFMODE,0x0E88);			  //CF1-total active power, disable; CF2-total reactvie power, disable; CF3- fundamental avtive power, disable

	SPIWrite2Bytes(ACCMODE,0x0000);
	SPIWrite2Bytes(COMPMODE,0x01FF);
	SPIWrite1Byte(MMODE,0x00);

	SPIWrite1Byte(LCYCMODE,0x0F);			  //phase A is selected for zero cross
	SPIWrite2Bytes(LINECYC,0x0064);

	SPIWrite4Bytes(MASK0,0x00000020);	      //line cycle interrupt enable
	SPIWrite4Bytes(MASK1,0x00000000);   
}

static void rt_hw_ade7880_irq0_handler(void *arg)
{

}

static void rt_hw_ade7880_irq1_handler(void *arg)
{
    rt_uint32_t IRQSTATUS = 0;

	IRQSTATUS = SPIRead4Bytes(MASK0);	  // Read off IRQSTA register
    if ((IRQSTATUS & REG_BIT13) == REG_BIT13)	//External Interrupt0 source
	{
		 IRQFlag = 1;
	}
}

void rt_hw_ade7880_irq_init()
{
    rt_pin_mode(ade7880_irq0_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(ade7880_irq0_pin, PIN_IRQ_MODE_FALLING, rt_hw_ade7880_irq0_handler, RT_NULL);
    rt_pin_irq_enable(ade7880_irq0_pin, PIN_IRQ_ENABLE);

    rt_pin_mode(ade7880_irq1_pin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(ade7880_irq1_pin, PIN_IRQ_MODE_FALLING, rt_hw_ade7880_irq1_handler, RT_NULL);
    rt_pin_irq_enable(ade7880_irq1_pin, PIN_IRQ_ENABLE);
}


int rt_hw_ade7880_int(void)
{
    rt_hw_ade7880_spi_config();
    rt_hw_ade7880_reset();
    rt_hw_ade7880_pm_select(PSM0);
    rt_hw_ade7880_spi_setup();

    IRQStautsRead0 = SPIRead4Bytes(STATUS1);
	SPIWrite4Bytes(STATUS1,IRQStautsRead0);
    if((IRQStautsRead0&REG_BIT15)==REG_BIT15)
	{
		ADE7880_TRACE("%s OK\n",__func__);

        rt_hw_ade7880_irq_init();
        rt_hw_ade7880_reg_cfg();
        return 0;
	}

    return -1;	
}

void rt_hw_ade7880_IVE_get(void)
{
    if(IRQFlag == 1){
        IRQFlag = 0;

        IRQStautsRead0 = SPIRead4Bytes(STATUS0);		    //read the interrupt status
        SPIDelay();			
        SPIWrite4Bytes(STATUS0,IRQStautsRead0);             //write the same STATUSx content back to clear the flag and reset the IRQ line
        SPIDelay();  

        PhaseAEnergy = SPIRead4Bytes(AWATTHR);		 
        PhaseAIRMS   = SPIRead4Bytes(AIRMS);
        PhaseAVRMS   = SPIRead4Bytes(AVRMS);
        PhaseAPeroid = SPIRead2Bytes(APERIOD);

        ADE7880_TRACE("%s PhaseAEnergy = %d,PhaseAIRMS = %d,PhaseAVRMS = %d,PhaseAPeroid = %d\n");
    }
}