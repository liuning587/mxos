/**
 ******************************************************************************
 * @file       osLib.h
 * @brief      API include file of osLib.h.
 * @details    This file including all API functions's declare of osLib.h.
 * @copyright  Copyrigth(C), 2008-2012,Sanxing Electric Co.,Ltd.
 *
 ******************************************************************************
 */
#ifndef __OSLIB_H__
#define __OSLIB_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <time.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define SEC_OF_NOW  (time_of_now(0))  /* seconds after the minute - [0, 59]  */
#define MIN_OF_NOW  (time_of_now(1))  /* minutes after the hour   - [0, 59]  */
#define HOUR_OF_NOW (time_of_now(2))  /* hours after midnight     - [0, 23]  */
#define DAY_OF_NOW  (time_of_now(3))  /* day of the month         - [1, 31]  */
#define MON_OF_NOW  (time_of_now(4))  /* months since January     - [1, 12]  */
#define YEAR_OF_NOW (time_of_now(5))  /* years since 2000                    */
#define WEEK_OF_NOW (time_of_now(6))  /* days since Sunday        - [0, 6]   */

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Globals
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern void
os_print_logo(void);

extern void
os_print_banner(void);

extern status_t
os_resource_init(void);

extern void
hex2a(const char_t * pbyte,
        char_t * pstr,
        uint32_t len);

extern uint8_t
hex2str(uint8_t * phex,
        uint32_t n);

extern void
float_limit(float32_t * pvalue,
        float32_t max_value,
        float32_t min_value);

extern uint32_t
str2hex(const char_t * pstr,
        uint8_t * phex);

extern void
int2bin(const uint32_t num,
        int8_t * pstr,
        uint8_t len);

extern uint32_t
hex_to_bcd(uint32_t hex);

extern uint32_t
bcd_to_hex(uint32_t bcd);

extern uint64_t
hex_to_bcd64(uint64_t hex);

extern uint64_t
bcd_to_hex64(uint64_t bcd);

extern int32_t
uintbcd_to_hex(uint32_t bcd);

extern int32_t
time_of_now(int32_t type);

extern void
time_to_str(const time_t dt,
        char_t * pstr);

extern time_t
byte_to_time(const uint8_t * pdata);

extern time_t
bytes_to_time(const uint8_t * pdata);

extern void
printf_time(const char_t * pinfo,
        time_t time);

extern void
print_logtime(void);

extern void
printf_frame(const char_t * pfmt,
        const uint8_t * pfrm,
        uint16_t len);

extern uint8_t
get_cs(const uint8_t * pfbuf,
        uint16_t len);

extern int32_t
mem_equal(void * pfbuf,
        uint8_t c,
        size_t len);

extern bool_e
is_bcd(void * pfbuf,
        uint32_t len);

extern void
ositoa(uint32_t i,
        char_t * pstr,
        int32_t radix);

extern int32_t
split(char_t * psrc,
        const char_t * psep,
        uint8_t * pdata);

extern int32_t
match_data(void * pfbuf,
        int32_t len,
        void * pdata,
        uint32_t cmplen);

extern uint16_t
get_crc16(const int8_t * pdata,
        uint16_t count);
extern uint32_t
get_crc32_stm32(const uint32_t *ptr,
        int32_t len);
extern uint32_t
get_crc32_stm32_ex(uint32_t precrc,
        const uint32_t *ptr,
        int32_t len);
extern void
inc3bcd(const uint8_t * psrc,
        int32_t n,
        uint8_t * pdes);

extern void
inc3bcdm(const uint8_t * psrc,
        int32_t n,
        int32_t m,
        uint8_t * pdes);

extern uint8_t
get_lastday_of_month(uint32_t year,
        uint8_t month);

extern void
binvert(uint8_t * pfbuf,
        int32_t bytes);

extern time_t
day_inc(time_t date,
        int32_t i);

extern time_t
month_inc(time_t date,
        int32_t i);

#endif /* __OSLIB_H__ */
/*-----------------------------End of osLib.h--------------------------------*/
