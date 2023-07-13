/*
* Description: thread for linux
* Author by: 
* Date: 
* 
*/
#define LOG_TAG "ADM_OS"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#ifdef PR_SET_NAME
#include <sys/prctl.h> 
#endif


/* uncomment the following line to allow mutex debugging */
//#define ADM_THREAD_DEBUG_MODE 1

#ifdef ADM_THREAD_DEBUG_MODE
#include <sys/syscall.h> /* for getting thread ids */
#endif

#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_thread.h"
// #include "adm_debug.h"

#include "adm_queue.h"

#ifdef ADM_THREAD_DEBUG_MODE
//g_mutexLockError and g_mutexUnlockError are the results of the calls to
//pthread_mutex_lock and pthread_mutex_unlock. If there was no error,
//their value will be zero.
//Note that this is only relevant when creating the mutex as PTHREAD_MUTEX_ERRORCHECK.
D32S g_mutexLockError = 0;
D32S g_mutexUnlockError = 0;

#ifdef __ANDROID__
#define DBGPRINTF           LOGV
#define GETTID              (syscall(__NR_gettid))
#else
//#define DBGPRINTF           printf
#define GETTID              ((int)syscall(SYS_gettid))
#endif

#define adm_thread_mutex_lock(ARG) do { \
        log_i("pthread_mutex_lock (%p) called from [threadId=%d]", ARG, GETTID); \
        g_mutexLockError = pthread_mutex_lock(ARG); \
        log_i("After pthread_mutex_lock (%p): g_mutexLockError=%d [threadId=%d]", ARG, g_mutexLockError, GETTID); \
        } while (0)

#define adm_thread_mutex_unlock(ARG) do { \
        g_mutexUnlockError = pthread_mutex_unlock(ARG); \
        log_i("pthread_mutex_unlock (%p) called from . g_mutexUnlockError=%d [threadId=%d]", ARG, g_mutexUnlockError, GETTID); \
        } while (0)

#else
#define DBGPRINTF(ARGS...)
#define GETTID   getpid()
#define adm_thread_mutex_lock(ARG)       (pthread_mutex_lock(ARG))
#define adm_thread_mutex_unlock(ARG)     (pthread_mutex_unlock(ARG))
#endif



void adm_thread_queue_init(adm_thread_event_t *outEvent)
{
    pthread_mutexattr_t mattr;

    pthread_mutexattr_init(&mattr);
#ifdef ADM_THREAD_DEBUG_MODE
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
#endif

    pthread_mutex_init(&outEvent->m_lock, &mattr);
    outEvent->m_isSignaled = FALSE;

    pthread_mutexattr_destroy(&mattr);

    pthread_cond_init(&outEvent->m_ready, NULL);

    log_i("adm_thread_queue_init: mutex ('m_lock') = %p", &outEvent->m_lock);
}


void adm_thread_queue_term(adm_thread_event_t *inEvent)
{
    pthread_mutex_destroy(&inEvent->m_lock);
    inEvent->m_isSignaled = FALSE;

    pthread_cond_destroy(&inEvent->m_ready);
}

void adm_thread_queue_set(adm_thread_event_t *ioEvent, pthread_t inSignalingThread)
{
    log_i("DM_PL_ThreadEvent LOCK->%p [threadId=%d]", &ioEvent->m_lock, GETTID);
    adm_thread_mutex_lock(&ioEvent->m_lock);
    ioEvent->m_isSignaled = TRUE;
    log_i("Calling pthread_cond_signal (%p) [threadId=%d]", &ioEvent->m_ready, GETTID);
    pthread_cond_signal(&ioEvent->m_ready);
    log_i("After pthread_cond_signal (%p) [threadId=%d]", &ioEvent->m_ready, GETTID);

    log_i("DM_PL_ThreadEvent UNLOCK->%p [threadId=%d]", &ioEvent->m_lock, GETTID);
    adm_thread_mutex_unlock(&ioEvent->m_lock);
}

void adm_thread_queue_reset(adm_thread_event_t *ioEvent)
{
    log_i("DM_PL_ThreadEvent UNLOCK->%p [threadId=%d]", &ioEvent->m_lock, GETTID);
    adm_thread_mutex_unlock(&ioEvent->m_lock);
}

DBOOL adm_thread_queue_wait(adm_thread_event_t *inEvent)
{
    if (inEvent) {
        log_i("DM_PL_ThreadEvent LOCK->%p %s [threadId=%d]", &inEvent->m_lock, NULL, GETTID);
        adm_thread_mutex_lock(&inEvent->m_lock);//We must lock the mutex before performing the conditional wait

        if (!inEvent->m_isSignaled) {
            log_i("Calling pthread_cond_wait (%p) [threadId=%d]", &inEvent->m_lock, GETTID);
            pthread_cond_wait(&inEvent->m_ready, &inEvent->m_lock); //This function automatically and atomically unlocks
            //the associated mutex variable, so that it can be
            //used by the signalling thread. Then, the thread keeps
            //waiting until it is released by the signalling thread.
            log_i("After pthread_cond_wait (%p) [threadId=%d]", &inEvent->m_lock, GETTID);
        }
        inEvent->m_isSignaled = FALSE;
        log_i("After conditional block 'if (!inEvent->m_isSignaled)'");

        adm_thread_mutex_unlock(&inEvent->m_lock); //If we reached this section, it means that we
        //need to explicitly unlock the mutex, since this
        //thread took ownership of the mutex (and locked it) by being
        //released from pthread_cond_wait.
    }

    return TRUE;
}


DBOOL adm_thread_handle_queue(adm_thread_t *pThread)
{
    DBOOL result = TRUE, isOk;

    //log_i("====>+adm_thread_handle_queue(pThread = %x)(%.*s) [threadId=%d]", (D32U)pThread, MAX_THREAD_NAME, pThread->name, GETTID);

    if (!pThread) {
        result = FALSE;
        goto end;
    }

    log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_lock(&pThread->lock);
    log_i("====>adm_thread_handle_queue: Setting state to: ThreadStateBusy id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
    pThread->state = ThreadStateBusy;
    if (!pThread->isRunning) {
        log_i("====>adm_thread_handle_queue: Setting state to: ThreadStateShuttingDown id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
        pThread->state = ThreadStateShuttingDown;
        result = FALSE;
    }
    log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_unlock(&pThread->lock);
    if (result == FALSE)
        goto end;

    if (pThread && pThread->handleEventFunc)
    {
        isOk = pThread->handleEventFunc(pThread->handleEventParam);
    }

    if (!isOk) {
        log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_lock(&pThread->lock);
        log_i("====>adm_thread_handle_queue: Setting state to: ThreadStateShuttingDown id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
        pThread->state = ThreadStateShuttingDown;
        log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_unlock(&pThread->lock);
        result = FALSE;
        goto end;
    }

    log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_lock(&pThread->lock);
    log_i("====>adm_thread_handle_queue: Setting state to: ThreadStateWaiting id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
    pThread->state = ThreadStateWaiting;
    log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_unlock(&pThread->lock);

end:
    log_i("====>-adm_thread_handle_queue(result = %d) [threadId=%d]", result, GETTID);
    return result;
}

static void *ThreadKernel(void *arg)
{
    adm_thread_t *pThread = (adm_thread_t *)arg;

#ifdef DM_RUN_ON_ANDROID
    JNI_Utils_AttachCurrentThread();
#endif

    adm_thread_mutex_lock(&pThread->lock);
    log_i("====>ThreadKernel: Setting state to: ThreadStateWaiting id = %u (%.*s)", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name);
    pThread->state = ThreadStateWaiting;
    pThread->isRunning = TRUE;
    pThread->id = pthread_self();

#if !defined (PROD_MIN) && defined (PR_SET_NAME)
    //set thread name. use ps -L to see thread name.
    //must be call when thread is active
    prctl(PR_SET_NAME, pThread->name, 0, 0, 0);
#endif

    adm_thread_mutex_unlock(&pThread->lock);

    while (TRUE) {
 //brucezuo       adm_thread_queue_wait(&pThread->event);    //lock mutex, wait

        if (!adm_thread_handle_queue(pThread))
            break;

        usleep(2); //brucezuo add
    }

    log_i("ADM_Thread__ (%.*s) LOCK->%p %s [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, NULL, GETTID);
    adm_thread_mutex_lock(&pThread->lock);
    log_i("====>ThreadKernel: Setting state to: ThreadStateDown id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
    pThread->state = ThreadStateDown;
    pThread->isRunning = FALSE;
    log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_unlock(&pThread->lock);

#ifdef DM_RUN_ON_ANDROID
    JNI_Utils_DetachCurrentThread();
#endif

    return NULL;
}

adm_handle_t adm_thread_create_noname(void)
{
    return adm_thread_create_byname(NULL);
}

adm_handle_t adm_thread_create_byname(const char *inName)
{
    DBOOL result;

    adm_thread_t *pThread = adm_malloc(sizeof(adm_thread_t));
    if (!pThread)
        goto end;

    adm_memset(pThread, 0, sizeof(adm_thread_t), sizeof(adm_thread_t));

#if !defined (PROD_MIN) && defined (PR_SET_NAME)
    if (inName)
        adm_strncpy(pThread->name, inName, MAX_THREAD_NAME);
#else
    UNUSED(inName);
#endif
    pthread_mutex_init(&pThread->lock, NULL);
    log_i("adm_thread_create_noname mutex ('lock') = %p [threadId=%d]", &pThread->lock, GETTID);

    adm_thread_queue_init(&pThread->event);

    result = adm_thread_start(pThread);
    if (!result) {
        goto err;
    }

    goto end;

err:
    adm_thread_destroy(pThread);
    pThread = NULL;

end:
    return pThread;
}


adm_handle_t adm_thread_create_ext2(void *threadHandle, void *param, void *pQueue)
{
    DBOOL result;

    adm_thread_t *pThread = adm_malloc(sizeof(adm_thread_t));
    if (!pThread)
        goto end;

    adm_memset(pThread, 0, sizeof(adm_thread_t), sizeof(adm_thread_t));

    pthread_mutex_init(&pThread->lock, NULL);
    log_i("adm_thread_create_ext2 mutex ('lock') = %p [threadId=%d]", &pThread->lock, GETTID);

    adm_thread_queue_init(&pThread->event);

    pThread->handleEventFunc = threadHandle;
    pThread->handleEventParam = param;
    pThread->pQueue = pQueue;

    result = adm_thread_start(pThread);
    if (!result) {
        goto err;
    }

    goto end;

err:
    adm_thread_destroy(pThread);
    pThread = NULL;

end:
    return pThread;
}

adm_handle_t adm_thread_create(void *threadHandle, void *param, D16U queueNum, D16U queueSize)
{
    DBOOL result;
    adm_queue_handle_t queue = adm_queue_create(queueNum, queueSize);

   if( queue == NULL ) 
   {
       log_i("create queue failed!");
       return NULL;
   }
   else
   {
       log_i("create queue succeed!");
   } 

    adm_thread_t *pThread = adm_malloc(sizeof(adm_thread_t));
    if (!pThread)
        goto end;

    adm_memset(pThread, 0, sizeof(adm_thread_t), sizeof(adm_thread_t));

    pthread_mutex_init(&pThread->lock, NULL);
    log_i("adm_thread_create mutex ('lock') = %p [threadId=%d]", &pThread->lock, GETTID);

    adm_thread_queue_init(&pThread->event);

    pThread->handleEventFunc = threadHandle;
    pThread->handleEventParam = param;
    pThread->pQueue = queue;

    result = adm_thread_start(pThread);
    if (!result) {
        goto err;
    }

    goto end;

err:
    adm_thread_destroy(pThread);
    pThread = NULL;

end:
    return pThread;
}

DBOOL adm_thread_start(adm_handle_t inThreadHandle)
{
    DBOOL result = TRUE;
    int error;
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;

    log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_lock(&pThread->lock);
    if (pThread->isRunning) {
        log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_unlock(&pThread->lock);
        goto end;
    }

    log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_unlock(&pThread->lock);

    if (pThread->hThread) {
        result = FALSE;
        goto end;
    }

    error = pthread_create(&pThread->hThread, NULL, ThreadKernel, pThread);
    if (error) {
        pThread->state = ThreadStateFault;

        switch (error) { /* show the thread error */
        case EINVAL:
            log_i("err: attr in an invalid thread attributes object");
            break;
        case EAGAIN:
            log_i("err: the necessary resources to create a thread are not available");
            break;
        case EPERM:
            log_i("err: the caller does not have the privileges to create the thread with the specified attr object.");
            break;
        default:
            log_i("err: an unknown error was encountered attempting to create the requested thread.");
            break;
        }
        result = FALSE;
    }

end:
    return result;
}

void adm_thread_set_queue(adm_handle_t inThreadHandle, void *pQueue)
{
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;

    if (pThread) {
        log_i("ADM_Thread_Queue_ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_lock(&pThread->lock);
        pThread->pQueue = pQueue;
        log_i("ADM_Thread_Queue_ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_unlock(&pThread->lock);
    }
}

void adm_thread_set_func(adm_handle_t inThreadHandle,
                               THREAD_EVENT_FUNC inHandleEventFunc, void *inHandleEventParam)
{
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;

    if (pThread) {
        log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_lock(&pThread->lock);
        pThread->handleEventFunc = inHandleEventFunc;
        pThread->handleEventParam = inHandleEventParam;
        log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_unlock(&pThread->lock);
    }
}

DBOOL adm_thread_stop(adm_handle_t inThreadHandle)
{
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;
    DBOOL ret = FALSE;

    log_i("====>+adm_thread_stop [threadId=%d]", GETTID);
    if (!pThread)
        goto end;

    log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_lock(&pThread->lock);
    if (!pThread->isRunning) {
        pthread_mutex_unlock(&pThread->lock);
        goto end;
    }
    pThread->isRunning = FALSE;
    log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
    adm_thread_mutex_unlock(&pThread->lock);
    ret = TRUE;

    log_i("====> adm_thread_stop: signaling... id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
    adm_thread_queue_set(&pThread->event, pThread->id);

    usleep(IDLE_TIME);
    while (TRUE) {
        log_i("ADM_Thread__ (%.*s) LOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_lock(&pThread->lock);
        if (pThread->state == ThreadStateDown) {
            log_i("====> adm_thread_stop: ThreadStateDown -> exiting loop id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
            log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
            adm_thread_mutex_unlock(&pThread->lock);
            break;
        }
        log_i("ADM_Thread__ (%.*s) UNLOCK->%p [threadId=%d]", MAX_THREAD_NAME, pThread->name, &pThread->lock, GETTID);
        adm_thread_mutex_unlock(&pThread->lock);

        usleep(IDLE_TIME);
    }

end:
    return ret;
}

void adm_thread_destroy(adm_handle_t inThreadHandle)
{
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;

    if (!inThreadHandle) {
        goto end;
    }

    log_i("====>+adm_thread_destroy id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);

    if (adm_thread_stop(inThreadHandle)) {
        pthread_join(pThread->hThread, NULL);
    } else {
        int err = pthread_detach(pThread->id);

        assert(!err);
        log_i("====> adm_thread_destroy: pthread_detach returns %d [threadId=%d]", err, GETTID);
    }

    pthread_mutex_destroy(&pThread->lock);
    adm_thread_queue_term(&pThread->event);

    adm_free(inThreadHandle);

end:
    return;
}

DBOOL adm_thread_signal(adm_handle_t inThreadHandle)
{
    DBOOL result = TRUE;
    adm_thread_t *pThread = (adm_thread_t *)inThreadHandle;

    if (!pThread) {
        result = FALSE;
        goto end;
    }

    log_i("====>adm_thread_signal id = %u (%.*s) [threadId=%d]", (unsigned long)(pThread->id), MAX_THREAD_NAME, pThread->name, GETTID);
    adm_thread_queue_set(&pThread->event, pThread->id);

end:
    return result;
}

void  adm_thread_set_name(char *inName)
{
#if !defined (PROD_MIN) && defined (PR_SET_NAME)
    //set thread name. use ps -L to see thread name.
    //must be call when thread is active
    prctl(PR_SET_NAME, inName, 0, 0, 0);
#endif
}

D32U adm_thread_get_tid(void)
{
    return (D32U)pthread_self();
}

/*********************************************************************************************/

adm_handle_t adm_mutex_create(void)
{
    pthread_mutex_t *pMutex = adm_malloc(sizeof(pthread_mutex_t));

    if (pMutex) {
        pthread_mutex_init(pMutex, NULL);
    }

    return pMutex;
}

//Lock mutex
void adm_mutex_lock(adm_handle_t inMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)inMutex;

    if (pMutex) {
        pthread_mutex_lock(pMutex);
    }
}

//Unlock mutex
void adm_mutex_unlock(adm_handle_t inMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)inMutex;

    if (pMutex) {
        pthread_mutex_unlock(pMutex);
    }
}

//Close mutex
void adm_mutex_close(adm_handle_t inMutex)
{
    pthread_mutex_t *pMutex = (pthread_mutex_t *)inMutex;

    if (pMutex) {
        pthread_mutex_destroy(pMutex);
        adm_free(pMutex);
    }
}


/*********************************************************************************************/
//thread test sample
#define PROMPT "DIL: "

/* Always add at least this many bytes when extending the buffer.  */
#define MIN_CHUNK 64

#define NIA_CMD_TEXT "nia"


typedef struct {
    char *str;
    int num;
    char *description;
} String2Enum_t;

typedef enum {
    NIA_CMD,
    USER_CMD,
    CONFIRM_CMD,
    CANCEL_CMD,
    UPDATE_ACCEPT,
    UPDATE_REJECT,
    HELP_CMD,
    EXIT_CMD,
    POWER_ON,
    DOWNLOAD_ACCEPT,
    DOWNLOAD_DEFER,
    DOWNLOAD_CANCEL,
    UI_FUMO,
    UI_DM,
    UI_ALERT_ACCEPT,
    UI_ALERT_REJECT,
    UI_ALERT_TIMEOUT,
    UPDATE_COMPLETED,
    UPDATE_NOT_COMPLETED,
    ADM_CMD_TIMER_STARTUG_MODE,
    ADM_CMD_TIMER_STOP
} CliCommands_t;

typedef enum {
    E_CONFIRM_DOWNLOAD = 4,
    E_CONFIRM_UPDATE,
    E_CONFIRM_UPDATE_COMPLETE,
    E_CONFIRM_UPDATE_RESULT
} E_ConfirmType_t;

typedef struct {
    pthread_t       thread_engine;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int confirmType;
} ThreadData_t;

ThreadData_t g_engineThreadData;

static String2Enum_t commands[] = {
    {NIA_CMD_TEXT, NIA_CMD, "send NIA msg, provided it in hex encoded format"},
    {"accept download", DOWNLOAD_ACCEPT, "confirm download package"},
    {"defer download", DOWNLOAD_DEFER, "defer download"},
    {"cancel download", DOWNLOAD_CANCEL, "cancel download"},
    {"accept update", UPDATE_ACCEPT, "accept update"},
    {"reject update", UPDATE_REJECT, "reject update"},
    {"exit", EXIT_CMD, "exit the application"},
    {"quit", EXIT_CMD, "quit the application"},
    {"help", HELP_CMD, "This command"},
    {"poweron", POWER_ON, "power on"},
    {"user fumo", UI_FUMO, "User init FUMO"},
    {"user dm", UI_DM, "User init DM"},
    {"accept uialert", UI_ALERT_ACCEPT, "accept uialert"},
    {"cancel uialert", UI_ALERT_REJECT, "cancel uialert"},
    {"timeout uialert", UI_ALERT_TIMEOUT, "uialert screen timeout"},
    {"yes complete", UPDATE_COMPLETED, "yes complete"},
    {"no complete", UPDATE_NOT_COMPLETED, "yes complete"},
    {"start timer", ADM_CMD_TIMER_STARTUG_MODE, "start timer"},
    {"stop timer", ADM_CMD_TIMER_STOP, "stop timer"},

    
    {NULL}
};

static int str2Enum(String2Enum_t *list, char *str)
{
    int i;

    for (i = 0; list[i].str; i++) {
        char *cmd = list[i].str;
        size_t cmdLen = strlen(cmd);

        if (!strncmp(cmd, str, cmdLen) &&
            (str[cmdLen] == ' ' || str[cmdLen] == '\0')) {
            return list[i].num;
        }
    }

    return -1;
}

static void printCommands(void)
{
    int i;

    log_i("Possible commands are:\n");
    for (i = 0; commands[i].str; i++)
        ;log_i("%s : %s\n", commands[i].str, commands[i].description);
}

static int processCmd(int cmd, char *line)
{
    int ret = 0;

    switch (cmd) {
    case NIA_CMD: {
            int niaOffset = sizeof(NIA_CMD_TEXT) - 1;
            char *nia = line + niaOffset;
            log_i("line = %s\n", line);

            /* if the nia contains msg, then skip the space */
            if (*nia)
                nia++;


            break;
        }
    case HELP_CMD:
        printCommands();
        break;
    case EXIT_CMD:
        ret = 1;
        break;
    case POWER_ON:

        break;
    case DOWNLOAD_ACCEPT: {
            log_i("DOWNLOAD_ACCEPT\n");
            //DM_notifyDLSessionProceed();
        }
        break;
    case DOWNLOAD_DEFER:

        break;
    case DOWNLOAD_CANCEL:

        break;
    case UPDATE_ACCEPT:
        //DM_FUMO_executeFwUpdate(fumoMO);
        break;
    case UPDATE_REJECT:

        break;
    case UI_FUMO:
        //dmcTriggerFUMOTask();
        break;
    case UI_DM:
        //dmcTriggerDMTask();
        break;
    case UI_ALERT_ACCEPT:

        break;
    case UI_ALERT_REJECT:

        break;
    case UI_ALERT_TIMEOUT:

        break;
    case UPDATE_COMPLETED:
        //dmcSendUpdateResult();
        break;
    case UPDATE_NOT_COMPLETED:

        break;
    case ADM_CMD_TIMER_STARTUG_MODE:
        //adm_timer_entry_test_start();
        break;
    case ADM_CMD_TIMER_STOP:
        //adm_timer_entry_test_stop();
        break;

    default:
        log_i("non-implemented command: %s\n", line);
        break;
    }
    return ret;
}

static int getLineFromStream(char **lineptr,
                         size_t *n,
                         FILE *stream,
                         char terminator,
                         int offset)
{
    int availChars;     /* Allocated but unused chars in *LINEPTR.  */
    char *readPos;       /* Where we're reading into *LINEPTR. */
    int ret;

    if (!lineptr || !n || !stream) {
        errno = EINVAL;
        return -1;
    }

    if (!*lineptr) {
        *n = MIN_CHUNK;
        *lineptr = malloc(*n);
        if (!*lineptr) {
            errno = ENOMEM;
            return -1;
        }
    }

    availChars = *n - offset;
    readPos = *lineptr + offset;

    for (;;) {
        int saveErrno;
        register int c = getc(stream);

        saveErrno = errno;

        /* We always want at least one char left in the buffer, since we
        always (unless we get an error while reading the first char)
         NUL-terminate the line buffer.  */

        //DM_PL_Client_assert((*lineptr + *n) == (readPos + availChars));
        if (availChars < 2) {
            if (*n > MIN_CHUNK)
                *n *= 2;
            else
                *n += MIN_CHUNK;

            availChars = *n + *lineptr - readPos;
            *lineptr = realloc(*lineptr, *n);
            if (!*lineptr) {
                errno = ENOMEM;
                return -1;
            }
            readPos = *n - availChars + *lineptr;
            //DM_PL_Client_assert((*lineptr + *n) == (readPos + availChars));
        }

        if (ferror(stream)) {
            /* Might like to return partial line, but there is no
               place for us to store errno.  And we don't want to just
               lose errno.  */
            errno = saveErrno;
            return -1;
        }

        if (c == EOF) {
            /* Return partial line, if any.  */
            if (readPos == *lineptr)
                return -1;
            else
                break;
        }

        if (c == terminator)
            /* Return the line.  */
            break;

        *readPos++ = c;
        availChars--;
    }

    /* Done - NUL terminate and return the number of chars read.  */
    *readPos = '\0';

    ret = readPos - (*lineptr + offset);
    return ret;
}

char *adm_thread_readline(const char *prompt)
{
    size_t n;
    char *lineptr = NULL;
    getLineFromStream(&lineptr, &n, stdin, '\n', 0);
    return lineptr;
}

static D32S adm_thread_readline_loop(void *data)
{
    char *line;

    log_i("starting CLI\n");

    while ((line = adm_thread_readline(PROMPT))) {
        int cmd;

        /* skip an empty line */
        if (!*line) {
           adm_free(line);
            continue;
        }

        cmd = str2Enum(commands, line);
        if (cmd == -1) {
            log_i("DIL unknown command (use 'help' for details): %s\n", line);
            adm_free(line);
            continue;
        }

        if (processCmd(cmd, line)) {
            adm_free(line);
            break;
        }
        adm_free(line);
    }

    log_i("finished CLI\n");

    return 0;
}

static DBOOL adm_thread_test_mainloop(void *param)
{
    D32S ret=0;

    log_i("starting DM_run()");
    ret = adm_thread_readline_loop(param);
    (void)ret;
    return TRUE;//ret == ADM_ERR_OK;
}

int adm_thread_test_main(int argc, char *argv[])
{
    // int ret;
    // D32S result = 0;
    // pthread_t thread_cli;
    adm_handle_t dmThread = NULL;

    log_i("test start");

    //g_DMLogFilePath = "dmc_log.txt";

    /* opening the CLI thread */
    ///pthread_create(&thread_cli, NULL, &cli, NULL);

    //dmcLoggerInit();

    /*********************************************/

    /* opening the CLI thread */
    //pthread_create(&thread_cli, NULL, &adm_thread_test_mainloop, NULL);


    dmThread = adm_thread_create_noname();
    if (dmThread)
        adm_thread_set_func(dmThread, adm_thread_test_mainloop, NULL);
    /*************************************************/

    //result = DM_create(dmThread);
    //printf("after call DM_create, result=0x%x\n", (unsigned int)result);
    //DM_Config_setEnum("dmserver", 2);
    //DM_registerSessionStateObserver(sessionStateCB);


    //result = DM_start();
    //printf("after call DM_start, result=0x%x\n", (unsigned int)result);
    //dmcFumoPostInit(FUMO_ROOT_URI);
    //threadInit();

    //readline thread exit
    //pthread_join(thread_cli, NULL);

    adm_thread_stop(dmThread);

    //DM_stop();
    //DM_destroy();
    //dmcLoggerTerm();
 //   adm_thread_destroy(dmThread);
    //threadDestroy();
    log_i("exit main thread test function\n");
    return 0;
}



