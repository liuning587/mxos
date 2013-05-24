/**
 ******************************************************************************
 * @file      clkLib.c
 * @brief     系统时钟接口模块.
 * @details   This file including all API functions's implement of clkLib.c.
 * @copyright
 *
 ******************************************************************************
 */
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <clkLib.h>
#include <string.h>
#include <time.h>
#include <taskLib.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */
/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define TV_NORMALIZE(a) \
    if ((a).tv_nsec >= BILLION) \
        { (a).tv_sec++; (a).tv_nsec -= BILLION; }   \
    else if ((a).tv_nsec < 0)   \
        { (a).tv_sec--; (a).tv_nsec += BILLION; }

#define TV_CONVERT_TO_SEC(a,b)  \
        do { \
        register uint32_t hz = osClkRateGet(); \
    (a).tv_sec  = (time_t)((b) / hz);   \
    (a).tv_nsec = (long)(((b) % hz) * (BILLION / hz)); \
        } while (0)

#define TV_ADD(a,b) \
        (a).tv_sec += (b).tv_sec; (a).tv_nsec += (b).tv_nsec; TV_NORMALIZE(a)

#define BILLION         1000000000  /* 1000 million nanoseconds / second */
/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* NONE */
/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static struct clock _clockRealtime;

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/

/**
 ******************************************************************************
 * @brief      initialize clock facility.
 * @retval
 * 0 (OK), or -1 (ERROR) on failure or already initialized or clock rate
 * less than 1 Hz.
 * @details
 *  This routine initializes the POSIX timer/clock facilities.
 * It should be called by usrRoot() in usrConfig.c before any other
 * routines in this module.
 *
 * @note
 ******************************************************************************
 */
static int
clockLibInit(void)
{
    static bool_e libInstalled = FALSE;

    if (libInstalled)
        return (ERROR);

    if (osClkRateGet() < 1)
        return (ERROR);

    libInstalled = TRUE;

    memset((char*) &_clockRealtime, 0, sizeof(_clockRealtime));

    _clockRealtime.tickBase = tickGet();

    return (OK);
}


/** @defgroup group_clkLib clkLib
 *   @brief  系统时钟管理模块
*
*
* @{
*/

/**
 ******************************************************************************
 * @brief   get the current time of the clock.
 * @param[in]   clock_id : clock ID (always CLOCK_REALTIME)
 *
 * @param[out]  tp :  where to store current time
 * @retval     0 (OK), or -1 (ERROR) if clock_id is invalid or  tp  is NULL.
 *
 * @details
 *  This routine gets the current value tp for the clock.
 * @note
 ******************************************************************************
 */
extern int
clock_gettime(int clock_id,
struct timespec * tp
)
{
    uint32_t diffTicks; /* system clock tick count */

    clockLibInit();

    if (clock_id != CLOCK_REALTIME)
    {
        return (ERROR);
    }

    if (tp == NULL)
    {
        return (ERROR);
    }

    diffTicks = tickGet() - _clockRealtime.tickBase;

    TV_CONVERT_TO_SEC(*tp, diffTicks);
    TV_ADD(*tp, _clockRealtime.timeBase);

    return (OK);
}

/**
 ******************************************************************************
 * @brief      set the clock to a specified time.
 * @param[in]  clock_id : clock ID (always CLOCK_REALTIME)
 * @param[in]  tp: time to set
 * @retval
 * 0 (OK), or -1 (ERROR) if clock_id is invalid, tp is outside the supported
 * range, or the tp nanosecond value is less than 0 or equal to or greater than
 * 1,000,000,000.
 * @details
 * This routine sets the clock to the value tp, which should be a multiple
 * of the clock resolution.  If tp is not a multiple of the resolution, it
 * is truncated to the next smallest multiple of the resolution.
 * @note
 ******************************************************************************
 */
extern int
clock_settime(int clock_id,
const struct timespec * tp
)
{
    clockLibInit();

    if (clock_id != CLOCK_REALTIME)
    {
        return (ERROR);
    }

    if (tp == NULL || tp->tv_nsec < 0 || tp->tv_nsec >= BILLION)
    {
        return (ERROR);
    }

    /* convert timespec to vxTicks XXX use new kernel time */

    _clockRealtime.tickBase = tickGet();
    _clockRealtime.timeBase = *tp;

    return (OK);
}


/**
 ******************************************************************************
 * @brief      determine the current calendar time (ANSI).
 * @param[in]  timer : calendar time in seconds
 *      The current calendar time in seconds, or ERROR (-1) if the calendar
 * time is not available.
 * @details
 *      This routine returns the implementation's best approximation of current
 * calendar time in seconds.  If timer is non-NULL, the return value is
 * also copied to the location timer points to.
 * @note
 ******************************************************************************
 */
time_t
time(time_t *timer
)
{
    struct timespec tp;

    if (clock_gettime(CLOCK_REALTIME, &tp) == OK)
    {
        if (timer != NULL)
            *timer = (time_t) tp.tv_sec;
        return (time_t) (tp.tv_sec);
    }
    else
        return (time_t) (ERROR);
}

/**
 ******************************************************************************
 * @brief      convert the calendar time to a formatted string.
 * @param[in]   timer : time to convert
 * @retval     Return a pointer to a formatted string.
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
char *
time_ntoa(const time_t timer)
{
    static char str[22];
    struct tm daytime;
    (void)localtime_r(&timer, &daytime);
    sprintf(str, "%04d-%02d-%02d  %02d:%02d:%02d", daytime.tm_year + 1900,
            daytime.tm_mon + 1, daytime.tm_mday, daytime.tm_hour,
            daytime.tm_min, daytime.tm_sec);
    return str;
}

/**
 * @}
 */
/*--------------------------------clkLib.c-----------------------------------*/
