/**
 ******************************************************************************
 * @file       taskLib.h
 * @brief      API include file of taskLib.h.
 * @details    This file including all API functions's declare of taskLib.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __TASKLIB_H__
#define __TASKLIB_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define WAIT_FOREVER  ((uint32_t)0)

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
typedef void * TASK_ID;
typedef void * SEM_ID;
typedef void * MSG_Q_ID;

/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/

extern TASK_ID
taskSpawn(const signed char * const name, uint32_t priority,
        uint32_t stackSize, OSFUNCPTR entryPt, uint32_t arg);

extern void
taskDelete(TASK_ID tid);

extern void
taskSuspend(TASK_ID tid);

extern void
taskResume(TASK_ID tid);

extern void
taskLock(void);

extern void
taskUnlock(void);

extern void
taskDelay(uint32_t ticks);

extern char *
taskName(TASK_ID tid);

extern TASK_ID
taskIdSelf();

extern uint32_t
tickGet(void);

extern uint32_t
osClkRateGet(void);

extern SEM_ID
semBCreate(uint32_t cnt);

extern void
semDelete(SEM_ID semId);

extern status_t
semTake(SEM_ID semId, uint32_t timeout);

extern status_t
semGive(SEM_ID semId);

extern MSG_Q_ID
msgQCreate(uint32_t msgQLen);

extern status_t
msgQSend(MSG_Q_ID msgQId, void *pmsg);

extern status_t
msgQReceive(MSG_Q_ID msgQId, uint32_t timeout, void **pmsg);

extern int
msgQNumMsgs(MSG_Q_ID msgQId);


#endif /* __TASKLIB_H__ */
/*----------------------------End of taskLib.h-------------------------------*/
