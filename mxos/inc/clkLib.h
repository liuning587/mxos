/**
 ******************************************************************************
 * @file       clkLib.h
 * @brief      API include file of clkLib.h.
 * @details    This file including all API functions's declare of clkLib.h.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __CLKLIB_H__
#define __CLKLIB_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 -----------------------------------------------------------------------------*/
#include <types.h>
#include <stdio.h>
#include <time.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 -----------------------------------------------------------------------------*/
//#define CLOCK_REALTIME  0x0 /* system wide realtime clock */

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 -----------------------------------------------------------------------------*/
typedef struct clock
{
    uint32_t tickBase; /* vxTicks when time set */
    struct timespec timeBase; /* time set */
} CLOCK;

/*-----------------------------------------------------------------------------
 Section: Globals
 -----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 -----------------------------------------------------------------------------*/
int clock_gettime
    (
    int clock_id,           /* clock ID (always CLOCK_REALTIME) */
    struct timespec * tp    /* where to store current time */
    );
int clock_settime
    (
    int clock_id,       /* clock ID (always CLOCK_REALTIME) */
    const struct timespec * tp  /* time to set */
    );

extern char *
time_ntoa(const time_t stime);


#endif /* __CLKLIB_H__ */
/*-----------------------------End of clkLib.h-------------------------------*/
