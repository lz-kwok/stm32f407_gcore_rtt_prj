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

static rt_uint8_t measure_thread_stack[RT_MANAGER_THREAD_STACK_SZ];

static struct rt_messagequeue g_measure_mq;
static rt_uint8_t measure_mq_pool[(MANAGER_MQ_MSG_SZ+sizeof(void*))*MANAGER_MQ_MAX_MSG];
MesureManager mMesureManager;

static void g_measure_manager_entry(void *param)
{
    int g_m = 0;
    static rt_uint8_t g_index = 0;
    rt_err_t result = RT_EOK;
    memset(&mMesureManager,0x0,sizeof(MesureManager));
    while (RT_TRUE)
    {
        struct hal_message msg;
        /* receive message */
        result = rt_mq_recv(&g_measure_mq, &msg, sizeof(struct hal_message),RT_WAITING_FOREVER);
        if(result == RT_EOK){
            if(msg.what == m_OverCurrent){          //过流测试
                for(g_m=0;g_m<15;g_m++){            //等待30s
                    rt_thread_mdelay(1000);
                }
                mMesureManager.dpsp1000_Onoff = 1;                   //打开程控电源
                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON"); 
                rt_thread_mdelay(1000);
                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON"); 
                rt_thread_mdelay(1000);
                g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                rt_thread_mdelay(1000);
                g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");

                for(;;){
                    g_index++;
                    rt_thread_mdelay(1000);
                    g_uart_sendto_Dpsp("VOLT 110.0");
                    if(mMesureManager.dc_voltage > 10500.0){
                        g_index = 0;
                        break;
                    }
                    
                    if(g_index == 10){
                        g_index = 0;
                        break;
                    }
                }
                for(;;){
                    g_index++;
                    rt_thread_mdelay(2000);
                    if(mMesureManager.ac_voltage > 21000.0){
                        g_index = 0;
                        break;
                    }
                    if(g_index == 10){
                        g_index = 0;
                        break;
                    }
                }

                mMesureManager.step = 11;
            }else if(msg.what == m_StartTime){    
                for(;;){
                    g_index++;
                    rt_thread_mdelay(1000);
                    g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF"); 
                    mMesureManager.dpsp1000_Onoff = 0;
                    if(mMesureManager.dc_voltage < 60.0){
                        g_index = 0;
                        break;
                    }
                    
                    if(g_index == 10){
                        g_index = 0;
                        break;
                    }
                }
                rt_pin_write(MCU_KOUT11, PIN_HIGH);
                rt_pin_write(MCU_KOUT9, PIN_HIGH);

                mMesureManager.dpsp1000_Onoff = 1;                   //打开程控电源
                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON"); 
                rt_thread_mdelay(1000);
                g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                for(;;){
                    g_index++;
                    rt_thread_mdelay(1000);
                    g_uart_sendto_Dpsp("VOLT 110.0");
                    if(mMesureManager.dc_voltage > 10500.0){
                        g_index = 0;
                        break;
                    }
                    
                    if(g_index == 10){
                        g_index = 0;
                        break;
                    }
                }

                mMesureManager.step = 21;
            }

            if(msg.freecb != NULL){
                msg.freecb(msg.content);
            }
        }

        rt_thread_mdelay(100);
    }
    
}

void g_MeasureQueue_send(rt_uint8_t type, const char *content)
{   
    struct hal_message g_msg;
    g_msg.what = type;
    g_msg.content = (void *)content;
    g_msg.freecb = NULL;

    rt_mq_send(&g_measure_mq, (void*)&g_msg, sizeof(struct hal_message));
}

rt_uint8_t g_MeasureAuto_Check_Get(void)
{
    return mMesureManager.autoChecktype;
}

rt_uint16_t g_MeasureAuto_Checkbyte_Get(void)
{
    return mMesureManager.autoCheckbyte;
}

rt_uint8_t g_MeasureError_Code_Get(void)
{
    return mMesureManager.ErrorCode;
}

rt_err_t g_measure_manager_init(void)
{
    static struct rt_thread measure_thread;
    rt_err_t ret;
        
    rt_mq_init(&g_measure_mq,
            "g_measure_mq",
            measure_mq_pool, MANAGER_MQ_MSG_SZ,
            sizeof(measure_mq_pool),
            RT_IPC_FLAG_FIFO);

    rt_thread_init(&measure_thread,
                   "measure",
                   g_measure_manager_entry, RT_NULL,
                   measure_thread_stack, RT_MANAGER_THREAD_STACK_SZ,
                   RT_MANAGER_THREAD_PRIO, 30);

    ret = rt_thread_startup(&measure_thread);

    return ret;
}
// INIT_COMPONENT_EXPORT(g_Measuer_manager_init);	

