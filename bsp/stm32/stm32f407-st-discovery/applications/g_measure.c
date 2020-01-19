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
    static rt_uint8_t g_index = 0;
    rt_err_t result = RT_EOK;
    memset(&mMesureManager,0x0,sizeof(MesureManager));
    while (RT_TRUE)
    {
        struct hal_message msg;
        /* receive message */
        result = rt_mq_recv(&g_measure_mq, &msg, sizeof(struct hal_message),RT_WAITING_FOREVER);
        if(result == RT_EOK){
            if(msg.what == uart3_rx_signal){
                rt_uint8_t rec_buf[32];
                rt_memset(rec_buf,0x0,sizeof(rec_buf));
                int *rec_len = (int *)msg.content;

                rt_uint8_t c_len = g_Client_data_receive(rec_buf,*rec_len);
                if(c_len == 8){
                    switch(rec_buf[1])
                    {
                        case 0xFE:       //查询版本
                            sendBuf[1] = rec_buf[1];
                            sendBuf[2] = MajorVer;
                            sendBuf[3] = MiddleVer;
                            sendBuf[4] = MinnorVer;
                            g_Client_data_send(sendBuf, 10);
                        break;
                        case 0xFD:       //接触器控制
                            g_usb_pin_control((relaycmd)rec_buf[2]);
                            if(rec_buf[2] == Load_Main_ON){
                                rec_buf[2] = Load_Precharge_ON;
                                sendBuf[1] = rec_buf[1];
                                sendBuf[2] = rec_buf[2];
                                g_usb_set_timer(RT_TRUE);
                            }
                            g_Client_data_send(rec_buf,8);
                        break;
                        case 0xFC:       //逆变器软启动
                            sendBuf[1] = rec_buf[1];
                            sendBuf[2] = rec_buf[2];
                            g_Client_data_send(sendBuf, 10);
                            if(rec_buf[2]){
                                g_usb_control_softstart(RT_TRUE);
                            }else{
                                g_usb_control_softstart(RT_FALSE);
                            }
                        break;
                        case 0xFB:       //开启直流电源
                            if(rec_buf[2] == DC_Power_ON){
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP ON");
                                rt_thread_mdelay(1000);
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"VOLT 110.0");
                            }else if(rec_buf[2] == DC_Power_OFF){
                                g_uart_sendto_Dpsp((const rt_uint8_t *)"OUTP OFF");
                            }                  
                            g_usb_cdc_sendData(rec_buf,c_len);      
                        break;
                        case 0xFA:       //源效应电压设置
                            if(rec_buf[2] == 0x01){
                                g_uart_sendto_Dpsp("VOLT 70.0");
                            }else if(rec_buf[2] == 0x02){
                                g_uart_sendto_Dpsp("VOLT 77.0");
                            }else if(rec_buf[2] == 0x03){
                                g_uart_sendto_Dpsp("VOLT 110.0");
                            }else if(rec_buf[2] == 0x04){
                                g_uart_sendto_Dpsp("VOLT 137.5");
                            }else if(rec_buf[2] == 0x05){
                                g_uart_sendto_Dpsp("VOLT 142.0");
                            }                         
                            g_Client_data_send(rec_buf,c_len);     
                            rt_thread_mdelay(1000); 
                        break;
                        case 0xE0:       //自动化测试
                            if(rec_buf[2] == 0x01){
                                mMesureManager.autoChecktype = 1;
                            }else{
                                mMesureManager.autoChecktype = 0;
                            }

                            mMesureManager.autoCheckbyte = (rt_uint16_t)(((rt_uint16_t)rec_buf[3])<<8)+rec_buf[4];
                        break;
                        case 0x5F:
                            mMesureManager.ErrorCode = rec_buf[2];
                        break;
                    }
                }

                if(msg.freecb != NULL){
                    msg.freecb(rec_len);
                }
            }else if(msg.what == uart6_rx_signal){
                rt_uint8_t err_buf[32];
                rt_memset(err_buf,0x0,sizeof(err_buf));
                int *err_len = (int *)msg.content;
                rt_uint8_t r_len = g_ErrorCode_data_receive(err_buf,*err_len);
                mMesureManager.ErrorCode = err_buf[2];

                if(msg.freecb != NULL){
                    msg.freecb(err_len);
                }
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

