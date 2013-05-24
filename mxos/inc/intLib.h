/**
 ******************************************************************************
 * @file       intLib.h
 * @brief      API include file of intLib.h.
 * @details    This file including all API functions's declare of intLib.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __INTLIB_H__
#define __INTLIB_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
//#define intLock()       taskENTER_CRITICAL()
//#define intUnlock()     taskEXIT_CRITICAL()

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern status_t
intConnect(uint32_t irq_num, VOIDFUNCPTR routine, uint32_t parameter);

extern status_t
intDisconnect(uint32_t irq_num);

extern status_t
intPrioSet(uint32_t irq_num, uint8_t prio);

extern status_t
intEnable(uint32_t irq_num);

extern status_t
intDisable(uint32_t irq_num);

bool_e
intContext (void);

extern status_t
intLibInit(void);

extern void
intLock(void);

extern void
intUnlock(void);
/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/

#endif /* __INTLIB_H__ */
/*-------------------------------End of intLib.h-----------------------------*/
