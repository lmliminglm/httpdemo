#define LOG_TAG "ADM_OS"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "adm_typedefs.h"

#include "adm_thread.h"
#include "adm_queue.h"
#include "adm_memory.h"
// #include "adm_debug.h"
#ifdef __cplusplus
extern "C" {
#endif


#define ADM_QUEUE_LIST_IS_EMPTY( pxList )	( ( ( pxList )->uxNumberOfItems == ( D32U ) 0 ) ? TRUE : FALSE )

/*
 * Macro to mark a queue as locked.  Locking a queue prevents an ISR from
 * accessing the queue event lists.
 */
#if 0
	taskENTER_CRITICAL();									\

	taskEXIT_CRITICAL()
#endif

#define ADM_QUEUE_LOCK_QUEUE( pxQueue )								\
	{														\
		if( ( pxQueue )->cRxLock == queueUNLOCKED )			\
		{													\
			( pxQueue )->cRxLock = queueLOCKED_UNMODIFIED;	\
		}													\
		if( ( pxQueue )->cTxLock == queueUNLOCKED )			\
		{													\
			( pxQueue )->cTxLock = queueLOCKED_UNMODIFIED;	\
		}													\
	}

/********************************thread mutex*****************************************/

adm_handle_t s_adm_thread_mutex=NULL;
D32S s_adm_thread_mutex_init=0;

/******************************************thread mutex end**********************************/


static D32S adm_queue_remove_from_event_list( const List_t * const pxEventList )
{
    return TRUE;
}

static D32S adm_queue_generic_reset( adm_queue_handle_t xQueue, D32S xNewQueue )
{
    adm_queue_t * const pxQueue = xQueue;

	{
		pxQueue->u.xQueue.pcTail = pxQueue->pcHead + ( pxQueue->uxLength * pxQueue->uxItemSize ); /*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists conveying intent. */
		pxQueue->uxMessagesWaiting = ( D32S ) 0U;
		pxQueue->pcWriteTo = pxQueue->pcHead;
		pxQueue->u.xQueue.pcReadFrom = pxQueue->pcHead + ( ( pxQueue->uxLength - 1U ) * pxQueue->uxItemSize ); 
	}

	return pdPASS;
}

static D32U adm_queue_copy_data2list( adm_queue_t * const pxQueue, const void *pvItemToQueue, const D32U xPosition )
{
    D32U xReturn = FALSE;
    D32U uxMessagesWaiting;

	uxMessagesWaiting = pxQueue->uxMessagesWaiting;

	if( pxQueue->uxItemSize == ( D32U ) 0 )
	{
	}
	else if( xPosition == ADM_QUEUE_INSERT_TO_BACK )
	{
        if (pxQueue && pxQueue->pcWriteTo && pvItemToQueue)
        {
            //log_i("pxQueue->pcWriteTo: 0x%x,len:%d", pxQueue->pcWriteTo, pxQueue->uxItemSize);

            //log_bin(1, pvItemToQueue, pxQueue->uxItemSize);
            
            (void)memcpy((void *)pxQueue->pcWriteTo, pvItemToQueue, (size_t)pxQueue->uxItemSize); /*lint !e961 !e418 !e9087 MISRA exception as the casts are only redundant for some ports, plus previous logic ensures a null pointer can only be passed to memcpy() if the copy size is 0.  Cast to void required by function signature and safe as no alignment requirement and copy length specified in bytes. */

            //log_bin(1, pxQueue->pcWriteTo, pxQueue->uxItemSize);

            pxQueue->pcWriteTo += pxQueue->uxItemSize; /*lint !e9016 Pointer arithmetic on char types ok, especially in this use case where it is the clearest way of conveying intent. */
           
        }
		if( pxQueue->pcWriteTo >= pxQueue->u.xQueue.pcTail ) /*lint !e946 MISRA exception justified as comparison of pointers is the cleanest solution. */
		{
            
			pxQueue->pcWriteTo = pxQueue->pcHead;
            //log_i("pxQueue->pcWriteTo point to pcHead: 0x%x", pxQueue->pcWriteTo);
		}
		else
		{
		}
	}
	else
	{
        if (pvItemToQueue && pxQueue->u.xQueue.pcReadFrom)
        {
            (void)memcpy((void *)pxQueue->u.xQueue.pcReadFrom, pvItemToQueue, (size_t)pxQueue->uxItemSize); /*lint !e961 !e9087 !e418 MISRA exception as the casts are only redundant for some ports.  Cast to void required by function signature and safe as no alignment requirement and copy length specified in bytes.  Assert checks null pointer only used when length is 0. */
            pxQueue->u.xQueue.pcReadFrom -= pxQueue->uxItemSize;
        }
		if( pxQueue->u.xQueue.pcReadFrom < pxQueue->pcHead ) /*lint !e946 MISRA exception justified as comparison of pointers is the cleanest solution. */
		{
			pxQueue->u.xQueue.pcReadFrom = ( pxQueue->u.xQueue.pcTail - pxQueue->uxItemSize );
		}
		else
		{
		}

		if( xPosition == ADM_QUEUE_INSERT_OVERWRITE )
		{
			if( uxMessagesWaiting > ( D32U ) 0 )
			{
				--uxMessagesWaiting;
			}
			else
			{
			}
		}
		else
		{
		}
	}

	pxQueue->uxMessagesWaiting = uxMessagesWaiting + ( D32U ) 1;

    log_i("pxQueue->uxMessagesWaiting(send): %d, maxlimit: %d", pxQueue->uxMessagesWaiting, pxQueue->uxLength);

	return xReturn;
}
/*-----------------------------------------------------------*/

static void adm_queue_copy_data_from_list( adm_queue_t * const pxQueue, void * const pvBuffer )
{
	if( pxQueue->uxItemSize != ( D32U ) 0 )
	{
		pxQueue->u.xQueue.pcReadFrom += pxQueue->uxItemSize; 
		if( pxQueue->u.xQueue.pcReadFrom >= pxQueue->u.xQueue.pcTail ) 
		{
			pxQueue->u.xQueue.pcReadFrom = pxQueue->pcHead;
		}
		else
		{
		}
        if (pvBuffer && pxQueue->u.xQueue.pcReadFrom)
        {
            //log_i("pxQueue->u.xQueue.pcReadFrom: 0x%x, len:%d", pxQueue->u.xQueue.pcReadFrom, pxQueue->uxItemSize);

            //log_bin(1, pxQueue->u.xQueue.pcReadFrom, pxQueue->uxItemSize);
            
            (void)memcpy((void *)pvBuffer, (void *)pxQueue->u.xQueue.pcReadFrom, (size_t)pxQueue->uxItemSize); /*lint !e961 !e418 !e9087 MISRA exception as the casts are only redundant for some ports.  Also previous logic ensures a null pointer can only be passed to memcpy() when the count is 0.  Cast to void required by function signature and safe as no alignment requirement and copy length specified in bytes. */
            memset((void *)pxQueue->u.xQueue.pcReadFrom, 0, (size_t)pxQueue->uxItemSize);
            
            //log_bin(1, pvBuffer, pxQueue->uxItemSize);
        }
	}
}

#if 0
static D32S adm_queue_is_empty( const adm_queue_t *pxQueue )
{
    D32S xReturn;

    adm_mutex_lock(s_adm_thread_mutex);
	{
		if( pxQueue->uxMessagesWaiting == ( D32U )  0 )
		{
			xReturn = TRUE;
		}
		else
		{
			xReturn = FALSE;
		}
	}
    adm_mutex_unlock(s_adm_thread_mutex);

	return xReturn;
}


static D32S adm_queue_is_full( const adm_queue_t *pxQueue )
{
    D32S xReturn;

    adm_mutex_lock(s_adm_thread_mutex);
	{
		if( pxQueue->uxMessagesWaiting == pxQueue->uxLength )
		{
			xReturn = TRUE;
		}
		else
		{
			xReturn = FALSE;
		}
	}
    adm_mutex_unlock(s_adm_thread_mutex);

	return xReturn;
}
#endif

static D32S adm_queue_notify_container( const adm_queue_t * const pxQueue, const D32S xCopyPosition )
{
    adm_queue_t *pxQueueSetContainer = pxQueue->pxQueueSetContainer;
    D32S xReturn = pdFALSE;

    if( pxQueueSetContainer->uxMessagesWaiting < pxQueueSetContainer->uxLength )
    {
        const D8S cTxLock = pxQueueSetContainer->cTxLock;

        xReturn = adm_queue_copy_data2list( pxQueueSetContainer, &pxQueue, xCopyPosition );

        if( cTxLock == queueUNLOCKED )
        {
            if( ADM_QUEUE_LIST_IS_EMPTY( &( pxQueueSetContainer->xTasksWaitingToReceive ) ) == pdFALSE )
            {
                if( adm_queue_remove_from_event_list( &( pxQueueSetContainer->xTasksWaitingToReceive ) ) != pdFALSE )
                {
                    xReturn = pdTRUE;
                }
                else
                {
                }
            }
            else
            {
            }
        }
        else
        {
            pxQueueSetContainer->cTxLock = ( D8S ) ( cTxLock + 1 );
        }
    }
    else
    {
    }

    return xReturn;
}

#if 0
static void adm_queue_lock( adm_queue_t * const pxQueue )
{
    adm_mutex_lock(s_adm_thread_mutex);
	{
		D8S cTxLock = pxQueue->cTxLock;

		while( cTxLock > queueLOCKED_UNMODIFIED )
		{
			{
				if( pxQueue->pxQueueSetContainer != NULL )
				{
				}
				else
				{
					if( ADM_QUEUE_LIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
					{
						if( adm_queue_remove_from_event_list( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
						{
						}
						else
						{
						}
					}
					else
					{
						break;
					}
				}
			}

			--cTxLock;
		}

		pxQueue->cTxLock = queueUNLOCKED;
	}

    adm_mutex_unlock(s_adm_thread_mutex);

    adm_mutex_lock(s_adm_thread_mutex);
	{
		D8S cRxLock = pxQueue->cRxLock;

		while( cRxLock > queueLOCKED_UNMODIFIED )
		{
			if( ADM_QUEUE_LIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == FALSE )
			{
				if( adm_queue_remove_from_event_list( &( pxQueue->xTasksWaitingToSend ) ) != FALSE )
				{
				}
				else
				{
				}

				--cRxLock;
			}
			else
			{
				break;
			}
		}

		pxQueue->cRxLock = queueUNLOCKED;
	}

    adm_mutex_unlock(s_adm_thread_mutex);
}
#endif

static void adm_queue_init_new( const D32U uxQueueLength, const D32U uxItemSize, D8U *pucQueueStorage, const D8U ucQueueType, adm_queue_t *pxNewQueue )
{
	( void ) ucQueueType;

	if( uxItemSize == ( D32U ) 0 )
	{
		pxNewQueue->pcHead = ( D8S * ) pxNewQueue;
	}
	else
	{
		pxNewQueue->pcHead = ( D8S * ) pucQueueStorage;
	}

	pxNewQueue->uxLength = uxQueueLength;
	pxNewQueue->uxItemSize = uxItemSize;
	( void ) adm_queue_generic_reset( pxNewQueue, TRUE );

	#if 1
	{
		pxNewQueue->ucQueueType = ucQueueType;
	}
	#endif

	#if 1//
	{
		pxNewQueue->pxQueueSetContainer = NULL;
	}
	#endif

}

adm_queue_handle_t adm_queue_generic_create( const D32U uxQueueLength, const D32S uxItemSize, const D8U ucQueueType )
{
    adm_queue_t *pxNewQueue;
    size_t xQueueSizeInBytes;
    D8U *pucQueueStorage;

    if( uxItemSize == ( D32U ) 0 )
    {
        xQueueSizeInBytes = ( size_t ) 0;
    }
    else
    {
        xQueueSizeInBytes = ( size_t ) ( uxQueueLength * uxItemSize ); /*lint !e961 MISRA exception as the casts are only redundant for some ports. */
    }

    pxNewQueue = ( adm_queue_t * ) adm_malloc( sizeof( adm_queue_t ) + xQueueSizeInBytes ); /*lint !e9087 !e9079 see comment above. */

    if( pxNewQueue != NULL )
    {
        pucQueueStorage = ( D8U * ) pxNewQueue;
        pucQueueStorage += sizeof( adm_queue_t ); /*lint !e9016 Pointer arithmetic allowed on char types, especially when it assists conveying intent. */

        adm_queue_init_new( uxQueueLength, uxItemSize, pucQueueStorage, ucQueueType, pxNewQueue );
    }
    else
    {
        return FALSE;
    }

	adm_thread_mutex_init();

    return pxNewQueue;
}


D32S adm_queue_generic_send( adm_queue_handle_t xQueue, const void * const pvItemToQueue, /*TickType_t xTicksToWait,*/ const D32S xCopyPosition )
{
    D32S xYieldRequired;
    adm_queue_t * const pxQueue = xQueue;

	adm_mutex_lock(s_adm_thread_mutex);
	
	//for( ;; )
	{
        //adm_mutex_lock(s_adm_thread_mutex);
		{
			if( ( pxQueue->uxMessagesWaiting < pxQueue->uxLength ) || ( xCopyPosition == ADM_QUEUE_INSERT_OVERWRITE ) )
			{
				{
                    D32U uxPreviousMessagesWaiting = pxQueue->uxMessagesWaiting;

					xYieldRequired = adm_queue_copy_data2list( pxQueue, pvItemToQueue, xCopyPosition );

					if( pxQueue->pxQueueSetContainer != NULL )
					{
						if( ( xCopyPosition == ADM_QUEUE_INSERT_OVERWRITE ) && ( uxPreviousMessagesWaiting != ( D32U ) 0 ) )
						{
						}
						else if( adm_queue_notify_container( pxQueue, xCopyPosition ) != FALSE )
						{
						}
						else
						{
						}
					}
					else
					{
						if( ADM_QUEUE_LIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == FALSE )
						{
							if( adm_queue_remove_from_event_list( &( pxQueue->xTasksWaitingToReceive ) ) != FALSE )
							{
							}
							else
							{
							}
						}
						else if( xYieldRequired != FALSE )
						{
						}
						else
						{
						}
					}
				}

                adm_mutex_unlock(s_adm_thread_mutex);
				return pdPASS;
			}
			else
			{
                log_e("adm_queue_generic_send: QUEUE is full!!!!!!!!, waiting cnt:%d, max:%d", pxQueue->uxMessagesWaiting, pxQueue->uxLength);
                //adm_mutex_unlock(s_adm_thread_mutex);
                //break;
			}
		}
	}
	
	adm_mutex_unlock(s_adm_thread_mutex);	
	return pdFAIL;
}


D32S adm_queue_generic_recv( adm_queue_handle_t xQueue, void * const pvBuffer/*, TickType_t xTicksToWait */, const D32U xJustPeeking )
{
    // D32S xEntryTimeSet = pdFALSE;
    adm_queue_t * const pxQueue = xQueue;

	adm_mutex_lock(s_adm_thread_mutex);
	
	//for( ;; )
	{
        //adm_mutex_lock(s_adm_thread_mutex);
		{
			const D32U uxMessagesWaiting = pxQueue->uxMessagesWaiting;
           
			if( uxMessagesWaiting > ( D32U ) 0 )
			{
				log_e("pop-before: waiting cnt:%d, max:%d", pxQueue->uxMessagesWaiting, pxQueue->uxLength);
				adm_queue_copy_data_from_list( pxQueue, pvBuffer );
				pxQueue->uxMessagesWaiting = uxMessagesWaiting - ( D32U ) 1;
				log_e("pop-after: waiting cnt:%d, max:%d", pxQueue->uxMessagesWaiting, pxQueue->uxLength);

				if( ADM_QUEUE_LIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
				{
					if( adm_queue_remove_from_event_list( &( pxQueue->xTasksWaitingToSend ) ) != pdFALSE )
					{
					}
					else
					{
					}
				}
				else
				{
				}

                adm_mutex_unlock(s_adm_thread_mutex);
				return pdPASS;
			}
			else
			{
                //adm_mutex_unlock(s_adm_thread_mutex);
                //break;
			}
		}
        //adm_mutex_unlock(s_adm_thread_mutex);
	} 

    ///log_e("adm_queue_generic_recv: NO QUEUE ISSUE to copy!!!!!!!!");

	adm_mutex_unlock(s_adm_thread_mutex);
	
    return pdFAIL;
}

void adm_thread_send_msg(adm_handle_t dstThreadHdl, D32U msgId, void* param, D32U len)
{
	adm_thread_msg_t msg={0};
    adm_thread_t *pThread = (adm_thread_t *)dstThreadHdl;

    //log_i("send start......");

    //adm_mutex_lock(s_adm_thread_mutex);

    //log_i("malloc ptr: %x", msg);
	if(pThread && param)
	{
        void *data = adm_memdup(param, len);
        //log_i("====>adm_thread_send_msg: malloc data: 0x%x ****************", data);
        if (data)
        {
            msg.msg_id = msgId;
            msg.msg_len = len;
            msg.param = data;
            //log_i("====>msg_id=%d",msg.msg_id);
            //log_i("param string: 0x%x", (D32U)(msg.param));
            adm_queue_send((adm_queue_handle_t)(pThread->pQueue), &msg, 0);
        }
	}

    //adm_mutex_unlock(s_adm_thread_mutex);

    //log_i("send end......");
}

D32S adm_thread_mutex_init(void)
{
    if (s_adm_thread_mutex_init > 0)
    {
        //log_i("adm_thread_mutex_init: %d", s_adm_thread_mutex_init);
        return 1;
    }

    s_adm_thread_mutex = adm_mutex_create();
    if (s_adm_thread_mutex)
    {
        //log_i("adm_thread_mutex_init init ok, s_adm_thread_mutex=0x%x", s_adm_thread_mutex);
        s_adm_thread_mutex_init = 1;
        return 1;
    }
    else
        return 0;
}



#ifdef __cplusplus
}
#endif

