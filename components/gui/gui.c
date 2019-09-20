 /*
 * Change Logs:
 * Date           Author       Notes
 * 2019-9-11      guolz     first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <gui.h>

static rt_uint8_t gui_thread_stack[RT_GUI_THREAD_STACK_SZ];

static struct rt_messagequeue gui_mq;
static rt_uint8_t gui_mq_pool[(GUI_MQ_MSG_SZ+sizeof(void*))*GUI_MQ_MAX_MSG];;
                    //0123456789.ACEF
const rt_uint8_t tab[] = {0xee,0x28,0xcd,0x6d,0x2b,0x67,0xe7,0x2c,0xef,0x6f,0x10,0xaf,0xc6,0xc7,0x87};
const char pic_index[] = {'0','1','2','3','4','5','6','7','8','9','.','A','C','E','F'};

static void led_gui_entry(void *param)
{
    int8_t num,c,byte;
    rt_device_t dev = RT_NULL;
    char buf[] = "hello zhizhuo\r\n";

    dev = rt_device_find("vcom");
   
    if (dev)
        rt_device_open(dev, RT_DEVICE_FLAG_RDWR);
    else
        return ;
    while (RT_TRUE)
    {
        struct hal_message msg;
        /* receive message */
        if(rt_mq_recv(&gui_mq, &msg, sizeof(struct hal_message),
                    RT_WAITING_FOREVER) != RT_EOK )
            continue;

        char *pic_show = (char *)msg.content;
        byte = -1;
        for(c=0;c<15;c++){
            if(pic_index[c] == *pic_show){
                byte = c;
                break;
            }
        }
        if(byte >= 0){
            num = tab[byte];
            for(c=0;c<8;c++){
                rt_pin_write(DigitLedclk, PIN_LOW);
                if((num&0x01) == 1){
                    rt_pin_write(DigitLedData, PIN_HIGH);
                }else{
                    rt_pin_write(DigitLedData, PIN_LOW);
                }

                rt_pin_write(DigitLedclk, PIN_HIGH);
                num>>=1;
            }
        }

       rt_device_write(dev, 0, buf, rt_strlen(buf));

        rt_thread_mdelay(100);
    }
    
}

void g_Gui_show_pic(const char *pic)
{   
    struct hal_message gui_msg;
    gui_msg.what = 0x01;
    gui_msg.content = (void *)pic;
    gui_msg.freecb = NULL;

    rt_mq_send(&gui_mq, (void*)&gui_msg, sizeof(struct hal_message));
}


rt_err_t g_Gui_init(void)
{
    static struct rt_thread gui_thread;
    rt_pin_mode(DigitLedData, PIN_MODE_OUTPUT);
    rt_pin_mode(DigitLedclk, PIN_MODE_OUTPUT);
    
    rt_mq_init(&gui_mq,
            "gui_mq",
            gui_mq_pool, GUI_MQ_MSG_SZ,
            sizeof(gui_mq_pool),
            RT_IPC_FLAG_FIFO);

    rt_thread_init(&gui_thread,
                   "usbd",
                   led_gui_entry, RT_NULL,
                   gui_thread_stack, RT_GUI_THREAD_STACK_SZ,
                   RT_GUI_THREAD_PRIO, 20);

    return rt_thread_startup(&gui_thread);
}
INIT_COMPONENT_EXPORT(g_Gui_init);	