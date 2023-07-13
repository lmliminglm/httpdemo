#define LOG_TAG "ADM_OS"

#include <unistd.h>
//#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
//#include <bits/siginfo.h> //
#include <sys/syscall.h> 
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sem.h>      /* for semget(), semctl(), semop(), semtimedop() */
#include <sys/shm.h>      /* for shmget(), shmat(), shmdt() */
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sched.h>        /* for sched_yield() */
#include <sys/msg.h>      /* for msgget(), msgsnd(), msgrcv(), msgctl() */


#include "adm_typedefs.h"
#include "adm_string.h"
#include "adm_memory.h"
#include "adm_timer.h"
// #include "adm_debug.h"

#define CLOCKID CLOCK_MONOTONIC 

D32S adm_timer_init( ADM_TIMER_T* timer, D32U time_ms, ADM_TIMER_HANDLER_T callback_func, void* arg )
{
    if((timer == NULL )/*||(arg == NULL)*/||(callback_func == NULL))
    {
        log_e("create timer error, timer pointer or cb null");

        return -2;
    }
    
     if(time_ms <= 0)
    {
        return -3;
    }   
    timer->pid = getpid();
    timer->callback_func = callback_func;
    timer->arg = arg;
    timer->interval_ms = time_ms;
    //timer->timer_event_spec.sival_int = 123;
    timer->timer_event.sigev_notify = SIGEV_THREAD;            
    timer->timer_event.sigev_notify_function = callback_func;
    timer->timer_event.sigev_value.sival_ptr = timer;
    timer->timer_event.sigev_notify_attributes = NULL;

/*
 se.sigev_notify = SIGEV_THREAD;
 se.sigev_value.sival_ptr = &timer_id;
 se.sigev_notify_function = timer_thread;
 se.sigev_notify_attributes = NULL;
 status = timer_create(CLOCK_REALTIME, &se, &timer_id);
*/      
    if (timer_create(CLOCKID, &timer->timer_event, & timer->timer_id) == -1)  
    {  
        log_e("create timer error");
        return -1;
    }
    return 0;
}

D32S adm_timer_start( ADM_TIMER_T* timer )
{
    int ret;
    struct itimerspec       interval;

    if ((timer == NULL) || (timer->timer_id ==NULL))
    {
        return -1;
    }

    interval.it_interval.tv_nsec = timer->interval_ms%1000*1000000;
    interval.it_interval.tv_sec = timer->interval_ms/1000;

    interval.it_value = interval.it_interval;

    ret = timer_settime(timer->timer_id, 0, &interval, NULL);
    return ret;
}


D32S adm_timer_stop( ADM_TIMER_T* timer )
{
    int ret;
    struct itimerspec       interval;
    
    if ((timer == NULL) || (timer->timer_id ==NULL))
    {
        return -1;
    }
    interval.it_interval.tv_sec = interval.it_interval.tv_nsec = interval.it_value.tv_sec = interval.it_value.tv_nsec = 0;
    ret = timer_settime(timer->timer_id, 0, &interval, NULL);
    return ret;
}


D32S adm_timer_destroy( ADM_TIMER_T* timer )
{
    if ((timer == NULL) || (timer->timer_id ==NULL))
    {
        return -1;
    }

    if (timer_delete(timer->timer_id)) 
    {
        return -1;
    }
    adm_memset(timer,0,sizeof(ADM_TIMER_T),sizeof(ADM_TIMER_T));
    return 0;
}


/* timer test sample*/

#if 1 
ADM_TIMER_T timer1;
ADM_TIMER_T timer2;
ADM_TIMER_T timer3;

D32U adm_timer_count_test1 = 0;
D32U adm_timer_count_test2 = 0;
D32U adm_timer_count_test3 = 0;
void adm_timer_entry_test_cb1( union sigval v )
{
    ADM_TIMER_T *sival_ptr = (ADM_TIMER_T *)v.sival_ptr;
    ADM_TIMER_T *arg = sival_ptr->arg;


    adm_timer_count_test1++;
    log_i("cnt:%d", adm_timer_count_test1);

    log_i("T1: timeout:%d,pid:%d ", arg->interval_ms, arg->pid);

    if(adm_timer_count_test1 > 1000)
    {
        log_i("..........stop.............");
        adm_timer_stop(arg);
        adm_timer_destroy(arg);
    }
        

    log_i("idm_po_timer_entry_test_cb1 e, cnt:%d", adm_timer_count_test1);
}

void adm_timer_entry_test_cb2( union sigval v )
{
    ADM_TIMER_T *sival_ptr = (ADM_TIMER_T *)v.sival_ptr;
    ADM_TIMER_T *arg = sival_ptr->arg;

    adm_timer_count_test2++;
    log_i("b, cnt:%d", adm_timer_count_test2);

    log_i("T2: timeout:%d,pid:%d ", arg->interval_ms, arg->pid);

    if(adm_timer_count_test2 > 100)
    {
        log_i("..........stop.............");
        adm_timer_stop(arg);
        adm_timer_destroy(arg);
    }

    log_i("idm_timer_entry_test_cb2 e, cnt:%d", adm_timer_count_test2);
}

void adm_timer_entry_test_cb3( union sigval v )
{
    ADM_TIMER_T *sival_ptr = (ADM_TIMER_T *)v.sival_ptr;
    ADM_TIMER_T *arg = sival_ptr->arg;

    adm_timer_count_test3++;
    log_i("b, cnt:%d", adm_timer_count_test3);

    log_i("T3: timeout:%d,pid:%d ", arg->interval_ms, arg->pid);

    if(adm_timer_count_test3 > 100)
    {
        log_i("..........idm_timer_entry_test_cb3 stop.............");
        adm_timer_stop(arg);
        adm_timer_destroy(arg);
    }

    log_i("e, cnt:%d", adm_timer_count_test3);
}

void adm_timer_entry_test_start(void)
{

    D32S ret = -1;

    adm_timer_count_test1 = 0;
    adm_timer_count_test2 = 0;
    adm_timer_count_test3 = 0;

#if 1
    //timer 1
    ret = adm_timer_init(&timer1, 100, adm_timer_entry_test_cb1, &timer1);
    if (ret < 0) 
    {
        log_i("timer1 init fail!!!");
    }
    
    //timer 2
    ret = adm_timer_init(&timer2, 200, adm_timer_entry_test_cb2, &timer2);
    if (ret < 0) 
    {
        log_i("timer2 init fail!!!");
    }
#endif

    //timer 3
#if 1
    ret = adm_timer_init(&timer3, 50, adm_timer_entry_test_cb3, &timer3);
    if (ret < 0) 
    {
        log_i("timer3 init fail!!!");
    }
#endif

    //
    adm_timer_start(&timer1);
    adm_timer_start(&timer2);
    adm_timer_start(&timer3);
}

void adm_timer_entry_test_stop(void)
{
    log_i("...stop timer1 begin...");

    adm_timer_stop(&timer1);
    adm_timer_destroy(&timer1);

    log_i("...stop timer2 begin...");
    adm_timer_stop(&timer2);
    adm_timer_destroy(&timer2);

    log_i("...stop timer3 begin...");
    adm_timer_stop(&timer3);
    adm_timer_destroy(&timer3);
    log_i("...stop all timer end...");
}
#endif
/* timer test sample*/


