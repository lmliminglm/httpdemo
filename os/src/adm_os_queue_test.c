
#define LOG_TAG "ADM_OS"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>

#include "adm_typedefs.h"
#include "adm_thread.h"
#include "adm_queue.h"
#include "adm_memory.h"

// #include "adm_debug.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef void (*adm_timer_cb_t)(void);

#define ADM_TASK_A_SLEEP_UMS   10000
#define ADM_TASK_B_SLEEP_UMS   10000
#define ADM_TASK_C_SLEEP_UMS   10000
#define ADM_TASK_D_SLEEP_UMS   10000

D32U dm_task_A_count=0, dm_task_A_recv_count=0;
D32U dm_task_B_count=0, dm_task_B_recv_count=0;
D32U dm_task_C_count=0, dm_task_C_recv_count=0;
D32U dm_task_ownd_count=0, dm_task_ownd_recv_count=0;

adm_handle_t dm_thread_a = NULL;
adm_handle_t dm_thread_b = NULL;
adm_handle_t dm_thread_c = NULL;
adm_handle_t dm_thread_d = NULL;


void dm_task_dispatch(adm_thread_msg_t *msg)
{
    if (!msg)
    {
        log_e("dm_task_dispatch: msg null!!!");
        return;
    }

    adm_timer_cb_t TimerCB = (adm_timer_cb_t)msg->param;
    switch (msg->msg_id)
    {
        case ADM_TASK_MSGID_A2B:
            log_i("received ADM_TASK_MSGID_A2B");
            if (TimerCB)
            {
                TimerCB();
            }
            break;
        case ADM_TASK_MSGID_B2C:
            log_i("received ADM_TASK_MSGID_B2C");
            if (TimerCB)
            {
                TimerCB();
            }
            break;
        case ADM_TASK_MSGID_C2A:
            log_i("received ADM_TASK_MSGID_C2A");
            if (TimerCB)
            {
                TimerCB();
            }
            break;
        case ADM_TASK_MSGID_OWND:
            log_i("received ADM_TASK_MSGID_OWND");
            break;
        case ADM_TASK_MSGID_OWND1:
            log_i("received ADM_TASK_MSGID_OWND1");
            break;
        case ADM_TASK_MSGID_OWND2:
            log_i("received ADM_TASK_MSGID_OWND2");
            break;
        default:
            log_e("received unknown msgid!!!");
            break;
    }
}

/****************************************TASK A***************************************************/
static void dm_task_a_loop(void *param)
{
    adm_thread_msg_t message={0};
    adm_thread_t *pThread=NULL;
   
	while (1) 
	{
        pThread = (adm_thread_t *)dm_thread_a;
        if (pThread && pThread->pQueue)
        {
           if (adm_queue_receive(pThread->pQueue, (void *)&message, 0))
           {
               {
                    log_i("<====message: id: %d, msg: 0x%x", message.msg_id, message.param);
                    
                    dm_task_dispatch(&message);

                    dm_task_A_recv_count++;

                    if (dm_task_C_count != dm_task_A_recv_count)
                    {
                        log_e("count not match!!!, C sent:%d, A recv:%d", dm_task_C_count, dm_task_A_recv_count);
                    }
                    else
                    {
                        log_i("count match, C sent:%d, A recv:%d", dm_task_C_count, dm_task_A_recv_count);
                    }

                    log_i("release message!!!");

                    if(message.param)
                        adm_free(message.param);
               }
           }
        }

       usleep(ADM_TASK_A_SLEEP_UMS);
    }

    //vTaskDelete(dm_task_hdlr);
    //dm_task_hdlr = NULL;
}

void dm_creat_task_a(void)
{
    dm_thread_a = adm_thread_create(dm_task_a_loop, NULL, 10, sizeof(adm_thread_msg_t));
}

void dm_send_msg_A2B_cb(void)
{
    log_i("exec dm_send_msg_A2B_cb()");
}

void dm_send_msg_A2B(void)
{
    dm_task_A_count++;
    adm_thread_send_msg(dm_thread_b, ADM_TASK_MSGID_A2B, (void *)dm_send_msg_A2B_cb, sizeof(void *));
}

/****************************************TASK B***************************************************/
static void dm_task_b_loop(void *param)
{
    adm_thread_msg_t message={0};
    adm_thread_t *pThread=NULL;
		
   
	while (1) 
	{
        pThread = (adm_thread_t *)dm_thread_b;
        if (pThread && pThread->pQueue)
        {
           if (adm_queue_receive(pThread->pQueue, (void *)&message, 0))
           {
               {
                    log_i("<====message: id: %d, msg: 0x%x", message.msg_id, message.param);

                    dm_task_dispatch(&message);

                    dm_task_B_recv_count++;

                    if (dm_task_A_count != dm_task_B_recv_count)
                    {
                        log_e("count not match!!!, A sent:%d, B recv:%d", dm_task_A_count, dm_task_B_recv_count);
                    }
                    else
                    {
                        log_i("count match, A sent:%d, B recv:%d", dm_task_A_count, dm_task_B_recv_count);
                    }

                    log_i("release message!!!");

                    if(message.param)
                        adm_free(message.param);
               }
           }
        }
        usleep(ADM_TASK_B_SLEEP_UMS);
    }

    //vTaskDelete(dm_task_hdlr);
    //dm_task_hdlr = NULL;
}

void dm_creat_task_b(void)
{
    dm_thread_b = adm_thread_create(dm_task_b_loop, NULL, 10, sizeof(adm_thread_msg_t));
}

void dm_send_msg_B2C_cb(void)
{
    log_i("exec dm_send_msg_B2C_cb()");
}

void dm_send_msg_B2C(void)
{
    dm_task_B_count++;
    adm_thread_send_msg(dm_thread_c, ADM_TASK_MSGID_B2C, (void *)dm_send_msg_B2C_cb, sizeof(void *));
}

/****************************************TASK C***************************************************/
static void dm_task_c_loop(void *param)
{
    adm_thread_msg_t message={0};
    adm_thread_t *pThread=NULL;
   
	while (1) 
	{
        pThread = (adm_thread_t *)dm_thread_c;
        if (pThread && pThread->pQueue)
        {
           if (adm_queue_receive(pThread->pQueue, (void *)&message, 0))
           {
               {
                   log_i("<====message: id: %d, msg: 0x%x", message.msg_id, message.param);
                    
                    dm_task_dispatch(&message);

                    dm_task_C_recv_count++;

                    if (dm_task_B_count != dm_task_C_recv_count)
                    {
                        log_e("count not match!!!, B sent:%d, C recv:%d", dm_task_B_count, dm_task_C_recv_count);
                    }
                    else
                    {
                        log_i("count match, B sent:%d, C recv:%d", dm_task_B_count, dm_task_C_recv_count);
                    }

                    log_i("release message!!!");

                    if(message.param)
                        adm_free(message.param);

               }
           }
        }

       usleep(ADM_TASK_C_SLEEP_UMS);
    }

    //vTaskDelete(dm_task_hdlr);
    //dm_task_hdlr = NULL;
}

void dm_creat_task_c(void)
{
    dm_thread_c = adm_thread_create(dm_task_c_loop, NULL, 10, sizeof(adm_thread_msg_t));
}

void dm_send_msg_C2A_cb(void)
{
    log_i("exec dm_send_msg_C2A_cb()");
}

void dm_send_msg_C2A(void)
{
    dm_task_C_count++;
    adm_thread_send_msg(dm_thread_a, ADM_TASK_MSGID_C2A, (void *)dm_send_msg_C2A_cb, sizeof(void *));
}

/****************************************TASK D***************************************************/
static void dm_task_d_loop(void *param)
{
    adm_thread_msg_t message={0};
    adm_thread_t *pThread=NULL;
   
	while (1) 
	{
#if 1
        pThread = (adm_thread_t *)dm_thread_d;
        if (pThread && pThread->pQueue)
        {
           if (adm_queue_receive(pThread->pQueue, (void *)&message, 0))
           {
               {
                   
                    log_i("<====message: id: %d, msg: 0x%x", message.msg_id, message.param);


                    dm_task_dispatch(&message);

                    dm_task_ownd_recv_count++;
                    if (dm_task_ownd_count != dm_task_ownd_recv_count)
                    {
                        log_e("count not match!!!, D sent:%d, D recv:%d", dm_task_ownd_count, dm_task_ownd_recv_count);
                    }
                    else
                    {
                        log_i("count match, D sent:%d, D recv:%d", dm_task_ownd_count, dm_task_ownd_recv_count);
                    }

                    log_i("release message!!!");

                    if(message.param)
                        adm_free(message.param);
               }
           }
        }
#endif
       usleep(ADM_TASK_D_SLEEP_UMS);
    }
}

void dm_creat_task_d(void)
{
    log_i("dm_creat_task_d ...");
    dm_thread_d = adm_thread_create(dm_task_d_loop, NULL, 10, sizeof(adm_thread_msg_t));
    log_i("dm_creat_task_d ...: 0x%x", dm_thread_d);
}

void dm_send_msg_ownd_cb(void)
{
    log_i("exec dm_send_msg_ownd_cb()");
}

void dm_send_msg_ownd(void)
{
    dm_task_ownd_count++;
    adm_thread_send_msg(dm_thread_d, ADM_TASK_MSGID_OWND, (void *)dm_send_msg_ownd_cb, sizeof(void *));
}

void dm_send_msg_ownd1(void)
{
    dm_task_ownd_count++;
    adm_thread_send_msg(dm_thread_d, ADM_TASK_MSGID_OWND1, (void *)dm_send_msg_ownd_cb, sizeof(void *));
}

void dm_send_msg_ownd2(void)
{
    dm_task_ownd_count++;
    adm_thread_send_msg(dm_thread_d, ADM_TASK_MSGID_OWND2, (void *)dm_send_msg_ownd_cb, sizeof(void *));
}

void dm_send_msg_ownd3(void *param)
{
    dm_task_ownd_count++;
    adm_thread_send_msg(dm_thread_d, ADM_TASK_MSGID_OWND2, (void *)param, sizeof(void *));
}

void dm_send_msg_ownd3_a(void *param)
{
    //dm_task_ownd_count++;
    adm_thread_send_msg(dm_thread_a, ADM_TASK_MSGID_OWND, (void *)param, sizeof(void *));
}

void adm_queue_test_main_all(int argc, char *argv[])
{
    if(adm_thread_mutex_init() == 0)
    {
        return;
    }

#if 1
    dm_creat_task_a();

    dm_creat_task_b();

    dm_creat_task_c();
//#elif 1
    dm_creat_task_d();
#endif

    //sleep(3);

    while (1)
    {
        usleep(100000);
#if 1
        dm_send_msg_A2B();
        dm_send_msg_B2C();
        dm_send_msg_C2A();
//#elif 1
        dm_send_msg_ownd();
        dm_send_msg_ownd1();
        //dm_send_msg_ownd2();

        //dm_send_msg_ownd();
        //dm_send_msg_ownd1();
        //dm_send_msg_ownd2();
        //dm_send_msg_A2B();
#endif
        
    }

    
}


void adm_queue_test_main_one(int argc, char *argv[])
{
    if(adm_thread_mutex_init() == 0)
    {
        return;
    }

    dm_creat_task_a();

    dm_creat_task_d();

}

#ifdef __cplusplus
}
#endif
