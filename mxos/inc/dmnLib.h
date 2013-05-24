/**
 ******************************************************************************
 * @file       dmnLib.h
 * @brief      API include file of dmnLib.h.
 * @details    This file including all API functions's declare of dmnLib.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __DMNLIB_H__
#define __DMNLIB_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define FOREVER while(!theRebootSignal)

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
typedef void * DMN_ID;

/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/
extern bool_e theRebootSignal;

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern status_t
dmn_init(uint32_t stacksize);

extern DMN_ID
dmn_register(void);

extern status_t
dmn_sign(DMN_ID id);

extern status_t
dmn_unregister(DMN_ID id);

extern void
dmn_info(bool_e is_sem);

extern void
dmn_reboot(void);

#endif /* __DMNLIB_H__ */
/*----------------------------End of dmnLib.h--------------------------------*/
