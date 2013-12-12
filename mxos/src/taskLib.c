/**
 ******************************************************************************
 * @file      taskLib.c
 * @brief     内核接口模块.
 * @details   This file including all API functions's
 *            implement of taskLib.c.
 * @copyright
 *
 ******************************************************************************
 */

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <intLib.h>
#include <taskLib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#include <oscfg.h>
/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/** @defgroup group_taskLib taskLib
 *   @brief  多任务内核模块
 *
*   本模块封装了所有任务、信号量、消息队列的相关操作接口
*
* @{
*/
/**
 ******************************************************************************
 * @brief      spawn a task.
 * @param[in]  name             ：name of new task
 * @param[in]   priority        : priority of new task
 * @param[in]  stackSize        : size (bytes) of stack, need a multiple of 4
 * @param[in]  entryPt          : entry point of new task
 * @param[in]   arg             : task args to pass to func
 * @retval
 *          TASK_ID             : task handler(task tcb pointer)
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern TASK_ID
taskSpawn(const signed char * const name, uint32_t priority, uint32_t stackSize,
        OSFUNCPTR entryPt, uint32_t arg)
{
    xTaskHandle createdTask = NULL;
    uint16_t usStackDepth = stackSize / sizeof(long);
    uint32_t param = arg;
    /* configMAX_PRIORITIES是优先级数组下标，当优先级等于它时，OS内部也会减1 */
    uint32_t prior = (priority >= MAX_TASK_PRIORITIES)?0:(MAX_TASK_PRIORITIES - priority-1);
    int32_t result = xTaskCreate( (pdTASK_CODE)entryPt, name, usStackDepth,(void *)&param, prior, &createdTask);

     if (result == pdPASS)
    {
        return (TASK_ID) createdTask;
    }
    else
    {
        return NULL;
    }
}

/**
 ******************************************************************************
 * @brief      delete a task.
 * @param[in]
 *              tid : task ID of task to delete
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
taskDelete(TASK_ID tid)
{
    vTaskDelete((xTaskHandle) tid);
}

/**
 ******************************************************************************
 * @brief      suspend a task.
 * @param[in]   tid : task ID of task to suspend
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
taskSuspend(TASK_ID tid)
{
    vTaskSuspend((xTaskHandle) tid);
}

/**
 ******************************************************************************
 * @brief      resume a previously suspended task.
 * @param[in]   tid : task ID of task to resume
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
taskResume(TASK_ID tid)
{
    vTaskResume((xTaskHandle) tid);
}

/**
 ******************************************************************************
 * @brief      This function is used to prevent rescheduling to take place.
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
taskLock(void)
{
    vTaskSuspendAll();
}

/**
 ******************************************************************************
 * @brief      This function is used to re-allow rescheduling.
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
taskUnlock(void)
{
    xTaskResumeAll();
}

/**
 ******************************************************************************
 * @brief      delay the current task.
 * @param[in]   ticks : number of ticks to delay task
 *
 * @details  This function is called to delay execution of the currently running
 *  task until the specified number of system ticks expires
 *
 * @note
 ******************************************************************************
 */
extern void
taskDelay(uint32_t ticks)
{
    vTaskDelay(ticks);
}

/**
 ******************************************************************************
 * @brief      get number of clock ticks.

 * @retval     Return the number of clock ticks
 *
 * @details   This function is used by your application to obtain the current
 * value of the 32-bit counter which keeps track of the number of clock ticks
 *
 * @note
 ******************************************************************************
 */
extern uint32_t
tickGet(void)
{
    return xTaskGetTickCount();
}

/**
 ******************************************************************************
 * @brief      Get the task ID of a runing task..
 * @retval     The task ID of the calling task;
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern TASK_ID
taskIdSelf()
{
    extern void* pxCurrentTCB; /* 当前执行的任务句柄 */
    return ((TASK_ID)pxCurrentTCB);
}

/**
 ******************************************************************************
 * @brief      Get the name associated with a task ID.
 * @param[in]   tid  task ID of task whose name is to be found
 * @retval    a pointer to task name.
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern char *taskName(TASK_ID tid)
{
  extern  char  *vTaskGetTaskName(xTaskHandle *pxTask);
  return vTaskGetTaskName((xTaskHandle) tid);
}

/**
 ******************************************************************************
 * @brief      get  tick numbers per second.
 * @details
 *
 * @note
 ******************************************************************************
 */
extern uint32_t
osClkRateGet(void)
{
    return TICKS_PER_SECOND;
}

/*
 *******************************************************************************
 *                                SEMAPHORE INTERFACE
 *******************************************************************************
 */

/**
 ******************************************************************************
 * @brief      this function creates a semaphore.
 * @param[in]   cnt    the initial value for the semaphore
 * @retval     Return the new semaphore ID
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern SEM_ID
semBCreate(uint32_t cnt)
{
    xSemaphoreHandle xSemaphore;
    vSemaphoreCreateBinary( xSemaphore );

    if (xSemaphore == NULL)
    {
        return NULL;
    }

    if (cnt == 0) /* Means it can't be taken */
    {
        xSemaphoreTake(xSemaphore, 1);
    }

    return xSemaphore;
}

/**
 ******************************************************************************
 * @brief      This function delete a mutual exclusion semaphore.
 * @param[in]   semId    semaphore ID to take

 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern void
semDelete(SEM_ID semId)
{
    vQueueDelete(semId);
}

/**
 ******************************************************************************
 * @brief      This function waits for a mutual exclusion semaphore.
 * @param[in]   semId    semaphore ID to delete
 * @param[in]  timeout   timeout in ticks
 * @retval         OK on success, ERROR otherwise
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern status_t
semTake(SEM_ID semId, uint32_t timeout)
{
    signed portBASE_TYPE  pdRtn = pdFALSE;

    timeout = (timeout == 0u) ? portMAX_DELAY : timeout;

    if (intContext() == TRUE)
    {
        pdRtn = xSemaphoreTakeFromISR(semId, NULL);   //fixme: 未测试
    }
    else
    {
        pdRtn = xSemaphoreTake(semId, timeout);
    }

	return (pdRtn == pdTRUE) ? OK : ERROR;
}

/**
 ******************************************************************************
 * @brief      This function signals a semaphore.
 * @param[in]   semId   Semaphore ID to give
 * @retval         OK on success, ERROR otherwise
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern status_t
semGive(SEM_ID semId)
{
    signed portBASE_TYPE  pdRtn = pdFALSE;

    if (intContext() == TRUE)
    {
        pdRtn = xSemaphoreGiveFromISR(semId, NULL);   //fixme: 未测试
    }
    else
    {
        pdRtn = xSemaphoreGive(semId);
    }

    return (pdRtn == pdTRUE) ? OK : ERROR;
}

/*
 *******************************************************************************
 *                                MESSAGE INTERFACE
 *******************************************************************************
 */

/**
 ******************************************************************************
 * @brief      creates a message queue.
 * @param[in]   msgQLen : the number of elements in the storage area
 * @retval     Return the queue id to create.
 *
 * @details  This function creates a message queue if free event control blocks
 * are available.
 *
 * @note
 ******************************************************************************
 */
extern MSG_Q_ID
msgQCreate(uint32_t msgQLen)
{
    return (MSG_Q_ID) xQueueCreate(msgQLen, sizeof(void *));
}

/**
 ******************************************************************************
 * @brief      sends a message to a queue.
 * @param[in]     msgQId    msgQId associated with the desired queue
 * @param[in]     pmsg        A pointer to the message to send
 * @retval         OK on success, ERROR otherwise
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern status_t
msgQSend(MSG_Q_ID msgQId, void *pmsg)
{
    signed portBASE_TYPE  pdRtn = pdFALSE;

    if (intContext() == TRUE)
    {
        pdRtn = xQueueSendFromISR(msgQId, &pmsg, NULL);   //fixme: 未测试

    }
    else
    {
        pdRtn = xQueueSend(msgQId, &pmsg, 0);
    }

    return (pdRtn == pdTRUE) ? OK : ERROR;
}

/**
 ******************************************************************************
 * @brief      waits for a message to be sent to a queue.
 * @param[in]   msgQId      msgQId associated with the desired queue
 * @param[in]    timeout      ticks to wait
 * @param[in]    pmsg        a pointer to the message to recv
 * @retval         OK on success, ERROR otherwise
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern status_t
msgQReceive(MSG_Q_ID msgQId, uint32_t timeout, void **pmsg)
{
    signed portBASE_TYPE  pdRtn = pdFALSE;
    void * dummyptr;

    if (pmsg == NULL)
    {
        pmsg = &dummyptr;
    }

    timeout = (timeout == 0u) ? portMAX_DELAY : timeout;
    if (intContext() == TRUE)
    {
//        pdRtn = xQueueCRReceiveFromISR()( msgQId, &(*pmsg), timeout );//fixme: 未测试
    }
    else
    {
        pdRtn = xQueueReceive( msgQId, &(*pmsg), timeout );
    }

    return (pdRtn == pdTRUE) ? OK : ERROR;
}

/**
 ******************************************************************************
 * @brief      gets the number of messages queued to a message queue.
 * @param[in]   msgQId : msgQId associated with the desired queue
 * @retval     Return the number of message in msgQ.
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern int
msgQNumMsgs(MSG_Q_ID msgQId)
{
    return uxQueueMessagesWaiting(msgQId);
}
/**
 * @}
 */
/*-------------------------------taskLib.c-----------------------------------*/
