 /*
 * Change Logs:
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <g_measure.h>

static rt_uint8_t measure_thread_stack[RT_MANAGER_THREAD_STACK_SZ];

static struct rt_messagequeue g_measure_mq;
static rt_uint8_t measure_mq_pool[(MANAGER_MQ_MSG_SZ+sizeof(void*))*MANAGER_MQ_MAX_MSG];;

static void g_measure_manager_entry(void *param)
{
    while (RT_TRUE)
    {
        struct hal_message msg;
        /* receive message */
        if(rt_mq_recv(&g_measure_mq, &msg, sizeof(struct hal_message),
                    RT_WAITING_FOREVER) != RT_EOK )
            continue;

        // char *pic_show = (char *)msg.content;
        

        

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

