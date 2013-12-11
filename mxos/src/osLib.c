/**
 ******************************************************************************
 * @file      osLib.c
 * @brief     本文实现了OS的公共函数库.
 * @details   This file including all API functions's implement of osinit.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <maths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <devLib.h>
#include <osLib.h>
#include <oscfg.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/**
 * 闰年判断
 * */
#define IS_LEAP_YEAR(y) (((((y) % 4) == 0) && (((y) % 100) != 0)) \
          || (((y) % 400) == 0))

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
/**
 ******************************************************************************
 * @brief   输出os logo
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
void
os_print_logo(void)
{
    printf("\r\n"
           " \\|/\n"
           "--O-- The CM3 RTOS is starting...\n"
           " /|\\\n");
}

/**
 ******************************************************************************
 * @brief   输出os版本
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
void
os_print_banner(void)
{
    printf("\n  OS Version: "OS_RELEASE
           "\n  FreeRTOS  : V7.6.0"
           "\n  Build Time: "__DATE__" "__TIME__
           "\n  CopyRight : Xxxx Xxxx..\n");
}

extern status_t
devnull_create(void);
/**
 ******************************************************************************
 * @brief   os资源初始化
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
status_t
os_resource_init(void)
{
    (void)devlib_init();
//    (void)devnull_create();

    return OK;
}

/**
 ******************************************************************************
 * @brief      将字节数组转换成hex格式的ascii字符串(以'\0'结束)
 * @param[in]  pbyte: 字节数组
 * @param[out] pstr: ascii字符串
 * @param[in]  len: hex长度
 * @retval     None
 *
 * @details  例如: {0, 1, 2, 3} -> "01020304"
 *
 * @note    本函数并未考虑到输出字符串缓冲区的溢出问题，
 *          调用时请确保ascii指向的字符串缓冲区大小至少为(2 * len + 1)个字节
 ******************************************************************************
 */
void
hex2a(const char_t * pbyte,
        char_t * pstr,
        uint32_t len)
{
    int32_t i;

    for (i = 0; i < len; i++)
    {
        pstr[i * 2u] = (pbyte[i] >> 4u) + '0';
        pstr[(i * 2u) + 1u] = (pbyte[i] & 0xFu) + '0';
    }
    for (i = 0; i < (len * 2); i++)
    {
        if ((uint8_t)pstr[i] > (uint8_t)'9')
        {
            pstr[i] = (uint8_t)pstr[i] + 7;
        }
    }
    pstr[i] = (int8_t)0;
}

/**
 ******************************************************************************
 * @brief      将某浮点数限制在指定的范围内(若超出则修改其为边界值)
 * @param[in]  pvalue: 要检查的值
 * @param[in]  max_value: 上限
 * @param[in]  min_value: 下限
 * @param[out] pvalue 若超出上(下)限则修改为上(下)边界值
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
float_limit(float32_t * pvalue,
        float32_t max_value,
        float32_t min_value)
{
    *pvalue = (*pvalue > max_value) ? max_value
            : ((*pvalue < min_value) ? min_value : *pvalue);
}

/**
 ******************************************************************************
 * @brief      字符串转HEX数组
 * @param[in]  pstr: 字符串
 * @param[out] phex: hex缓冲区
 * @retval     uint32_t 写入phex的字节数
 * @details     例如 "0123456989ab" -> {1, 23, 45, 67, 89, 171}, 返回6
 ******************************************************************************
 */
uint32_t
str2hex(const char_t * pstr,
        uint8_t * phex)
{
    int32_t i;
    uint32_t len;

    len = (uint16_t) strlen(pstr) / 2;
    for (i = 0; i < len; i++)
    {
        if (((uint8_t)pstr[2 * i] >= (uint8_t)'0')
                && ((uint8_t)pstr[2 * i] <= (uint8_t)'9'))
        {
            phex[i] = ((uint8_t)(((uint8_t)pstr[2 * i] & 0x0Fu) << 4u));
        }
        else if (((uint8_t)pstr[2 * i] >= (uint8_t)'A')
                && ((uint8_t)pstr[2 * i] <= (uint8_t)'F'))
        {
            phex[i] = ((uint8_t)(((uint8_t)pstr[2 * i] - (uint8_t)'7') << 4u));
        }
        else if (((uint8_t)pstr[2 * i] >= (uint8_t)'a')
                && ((uint8_t)pstr[2 * i] <= (uint8_t)'f'))
        {
            phex[i] = (((uint8_t)pstr[2 * i] - (uint8_t)'W') << 4u);
        }
        else
        {
            phex[i] = 0x00;
        }

        if (((uint8_t)pstr[(2 * i) + 1] >= (uint8_t)'0')
                && ((uint8_t)pstr[(2 * i) + 1] <= (uint8_t)'9'))
        {
            phex[i] += ((uint8_t)pstr[(2 * i) + 1] & 0x0Fu);
        }
        else if (((uint8_t)pstr[(2 * i) + 1] >= (uint8_t)'A')
                && ((uint8_t)pstr[(2 * i) + 1] <= (uint8_t)'F'))
        {
            phex[i] += ((uint8_t)pstr[(2 * i) + 1] - (uint8_t)'7');
        }
        else if (((uint8_t)pstr[(2 * i) + 1] >= (uint8_t)'a')
                && ((uint8_t)pstr[(2 * i) + 1] <= (uint8_t)'f'))
        {
            phex[i] += ((uint8_t)pstr[(2 * i) + 1] - (uint8_t)'W');
        }
        else
        {
            phex[i] += 0x00;
        }
    }
    return len;
}

/**
 ******************************************************************************
 * @brief      整数转换成二进制方式显示的字符串（以'\0'结尾）
 * @param[in]  num   整数值
 * @param[out] pstr   二进制数据字符串
 * @param[in]  len 二进制位数（1~32）
 * @details     例如：int2bin(0xdeadbeaf, pstr, 7)后, pstr为"0101111"
 * @note        确保\a pstr指向的缓冲区至少有len+1个字节
 ******************************************************************************
 */
void
int2bin(const uint32_t num,
        int8_t * pstr,
        uint8_t len)
{
    if (len > 32)
    {
        return;
    }

    pstr[len] = '\0';
    for (uint8_t i = 0u; i < len; i++)
    {
        if (0x01 == BITS(num, i))
        {
            pstr[len - i - 1] = '1';
        }
        else
        {
            pstr[len - i - 1] = '0';
        }
    }
}

/**
 ******************************************************************************
 * @brief      hex转换成bcd
 * @param[in]  hex: hex值
 * @retval     uint32_t: 转换的值
 * @sa          bcd_to_hex
 ******************************************************************************
 */
uint32_t
hex_to_bcd(uint32_t hex)
{
    uint32_t  midval = 0u;
    uint32_t  hex2 = 1u;

    do
    {
        midval = midval + ((hex % 10) * hex2);
        hex = hex / 10;
        hex2 = hex2 * 16;
    }
    while (hex > 0);

    return  midval;
}

/**
 ******************************************************************************
 * @brief      BCD码转HEX
 * @param[in]  bcd bcd值
 * @retval     uint32_t: 转换的值
 * @sa          hex_to_bcd
 ******************************************************************************
 */
uint32_t
bcd_to_hex(uint32_t bcd)
{
    uint32_t  midval = 0u;
    uint32_t  hex = 1u;

    do
    {
        midval = midval + ((bcd % 16) * hex);
        bcd = bcd / 16;
        hex = hex * 10;
    }
    while (bcd > 0);

    return  midval;
}

/**
 ******************************************************************************
 * @brief      64位hex转换成bcd
 * @param[in]  hex hex值
 * @retval     uint64: 转换的值
 * @sa          bcd_to_hex64
 ******************************************************************************
 */
uint64_t
hex_to_bcd64(uint64_t hex)
{
    uint64_t  midval = 0u;
    uint64_t  hex2 = 1u;

    do
    {
        midval = midval + ((hex % 10) * hex2);
        hex = hex / 10;
        hex2 = hex2 * 16;
    }
    while (hex > 0);

    return  midval;
}

/**
 ******************************************************************************
 * @brief      64位BCD码转HEX
 * @param[in]  bcd: bcd值
 * @retval     uint64: 转换的值
 * @sa          hex_to_bcd64
 ******************************************************************************
 */
uint64_t
bcd_to_hex64(uint64_t bcd)
{
    uint64_t  midval = 0u;
    uint64_t  hex = 1u;

    do
    {
        midval = midval + ((bcd % 16) * hex);
        bcd = bcd / 16;
        hex = hex * 10;
    }
    while (bcd > 0);

    return  midval;
}

/**
 ******************************************************************************
 * @brief      BCD码转HEX码 20H --> 20
 * @param[in]  bcd: bcd值
 * @retval     int32: 转换的值
 ******************************************************************************
 */
int32_t
uintbcd_to_hex(uint32_t bcd)
{
    int32_t  midval = 0;

    midval = 1000000 * BCD2HEX(bcd >> 24u);
    midval += 10000 * BCD2HEX((bcd >> 16u) & 0xffu);
    midval += 100 * BCD2HEX((bcd >> 8u) & 0xffu);
    midval += BCD2HEX(bcd & 0xffu);

    return  midval;
}

/**
 ******************************************************************************
 * @brief      取得当前秒分时日月年周等数据
 * @param[in]  type: 要获取的数据类型
 *      - 0: 秒, 范围为[0, 59]
 *      - 1: 分, 范围为[0, 59]
 *      - 2: 时, 范围为[0, 23]
 *      - 3: 日, 范围为[0, 30]
 *      - 4: 月, 范围为[1, 12]
 *      - 5: 年, 范围为[0, n], 起始为2000年
 *      - 6: 星期几, 范围为[0, 6]
 * @retval     int32: 获取的数据，若\a type不合法则返回负数
 ******************************************************************************
 */
int32_t
time_of_now(int32_t type)
{
    time_t stime = time(NULL);
    struct tm daytime = *localtime(&stime);
    switch (type)
    {
        case 0:
            return daytime.tm_sec;
        case 1:
            return daytime.tm_min;
        case 2:
            return daytime.tm_hour;
        case 3:
            return daytime.tm_mday;
        case 4:
            return daytime.tm_mon + 1;
        case 5:
            return daytime.tm_year - 100;
        case 6:
            return daytime.tm_wday;
        default:
            return -1;
    }
}

/**
 ******************************************************************************
 * @brief      将时间转换成字符串格式
 * @param[in]  dt: 时间
 * @param[out] pstr: 字符串
 * @retval     None
 * @note  格式化而成的字符串为"YYYY-MM-DD HH:MM:SS",
 *          因此\a pstr的缓冲区大小至少应有20个字节
 ******************************************************************************
 */
void
time_to_str(const time_t dt,
        char_t * pstr)
{
    time_t t;
    struct tm daytime;

    if (0 == dt)
    {
        t = time(NULL);
    }
    else
    {
        t = dt;
    }
    daytime = *localtime(&t);
    (void)sprintf(pstr, "%04d-%02d-%02d %02d:%02d:%02d",
        daytime.tm_year+ 1900, daytime.tm_mon + 1, daytime.tm_mday,
        daytime.tm_hour, daytime.tm_min, daytime.tm_sec);
}

/**
 ******************************************************************************
 * @brief      转换时间数据格式
 * @param[in]  pdata: 时间数据,BCD格式
 * @retval     time_t：时间
 * @note      输入的参数\a pdata至少应当有6字节，且其意义分别为
 *      - 偏移0: 年(自2000年开始)
 *      - 偏移1: 月,[1-12]
 *      - 偏移2: 日
 *      - 偏移3: 时
 *      - 偏移4: 分
 *      - 偏移5: 秒
 ******************************************************************************
 */
time_t
byte_to_time(const uint8_t * pdata)
{
    struct tm daytime;

    daytime.tm_sec = BCD2HEX(*(pdata + 5));
    daytime.tm_min = BCD2HEX(*(pdata + 4));
    daytime.tm_hour = BCD2HEX(*(pdata + 3));
    daytime.tm_mday = BCD2HEX(*(pdata + 2));
    daytime.tm_mon = BCD2HEX(*(pdata + 1)) - 1;        /* 0...11 */
    daytime.tm_year = BCD2HEX(*(pdata + 0)) + 100;
    daytime.tm_isdst = 0;

    return mktime(&daytime);
}

/**
 ******************************************************************************
 * @brief      转换时间数据格式
 * @param[in]  pdata  时间数据
 * @retval     time_t 时间
 * @note      输入的参数\a pdata至少应当有6字节，且其意义分别为
 *      - 偏移0: 年(自2000年开始)
 *      - 偏移1: 月,[1-12]
 *      - 偏移2: 日
 *      - 偏移3: 时
 *      - 偏移4: 分
 *      - 偏移5: 秒
 ******************************************************************************
 */
time_t
bytes_to_time(const uint8_t * pdata)
{
    struct tm daytime;

    daytime.tm_sec = pdata[5];
    daytime.tm_min = pdata[4];
    daytime.tm_hour = pdata[3];
    daytime.tm_mday = pdata[2];
    daytime.tm_mon = pdata[1] - 1;  /* 0...11 */
    daytime.tm_year = pdata[0] + 100;
    daytime.tm_isdst = 0;

    return mktime(&daytime);
}

/**
 ******************************************************************************
 * @brief      打印指定时间
 * @param[in]  pinfo: 打印内容(位于时间内容前面的文本前缀)
 * @param[in]  time 指定时间
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
printf_time(const char_t *pinfo,
        time_t time)
{
    struct tm daytime;

    (void)localtime_r(&time, &daytime);
    (void)printf("%s: %04d-%02d-%02d  %02d:%02d:%02d\r\n", pinfo,
        daytime.tm_year + 1900, daytime.tm_mon + 1, daytime.tm_mday,
        daytime.tm_hour, daytime.tm_min, daytime.tm_sec);
}

/**
 ******************************************************************************
 * @brief      打印当前时间
 * @retval     None
 * @sa  printf_time
 *  ******************************************************************************
 */
void
print_logtime(void)
{
    struct tm daytime;
    time_t stime = time(NULL);
    (void)localtime_r(&stime, &daytime);
    (void)printf("[%04d-%02d-%02d %02d:%02d:%02d]  ",
        daytime.tm_year + 1900, daytime.tm_mon + 1, daytime.tm_mday,
        daytime.tm_hour, daytime.tm_min, daytime.tm_sec);
}

/**
 ******************************************************************************
 * @brief       打印出前导格式字符串和指定长度的二进制缓冲区
 * @param[in]   pfmt  前导(描述性的)字符串
 * @param[in]   pfrm   缓冲区地址
 * @param[in]   len       缓冲区字节数
 * @retval      void
 *
 ******************************************************************************
 */
void
printf_frame(const char_t *pfmt,
        const uint8_t * pfrm,
        uint16_t len)
{
    int32_t i;

    (void)printf(pfmt);

    for (i = 0; i < len; i++)
    {
        (void)printf("%02X ", pfrm[i]);
    }
    (void)printf("\r\n");
}

/**
 ******************************************************************************
 * @brief      计算校验和
 * @param[in]  pfbuf 缓冲区
 * @param[in]  len  缓冲区长度
 * @retval     uint8 校验和
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
uint8_t
get_cs(const uint8_t * pfbuf,
        uint16_t len)
{
    uint8_t cs = 0u;
    int32_t i;

    for (i = 0; i < len; i++)
    {
        cs = cs + pfbuf[i];
    }

    return cs;
}

/**
 ******************************************************************************
 * @brief      判断给定的缓冲区数据是否全等于指定的字符
 * @param[in]  pfbuf:  需要判断的缓冲区数据
 * @param[in]  c： 用于比较的字符
 * @param[in]  len： 缓冲区长度
 * @retval     int32: 0-相等 其它-不相等
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
int32_t
mem_equal(void * pfbuf,
        uint8_t c,
        size_t len)
{
    int32_t i;
    uint8_t *p;

    p = (uint8_t *)pfbuf;
    for (i = 0; i < len; ++i)
    {
        if (*(p + i) != c)
        {
            return (i + 1);
        }
    }
    return 0;
}

/**
 ******************************************************************************
 * @brief      判断给定的缓冲区数据是否全为BCD码
 * @param[in]  pfbuf:  需要判断的缓冲区数据
 * @param[in]  len：  缓冲区长度
 * @retval     bool_e: TRUE-是BCD码 FALSE-非BCD码
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
bool_e
is_bcd(void * pfbuf,
        uint32_t len)
{
    int32_t i;
    uint8_t *p;
    uint8_t val;

    p = (uint8_t*)pfbuf;
    for (i = 0; i < len; i++)
    {
        val = *(p + i);
        if ((LLLSB(val) > 9) || (LLNLSB(val) > 9))
        {
            return FALSE;
        }
    }
    return TRUE;
}

/**
 ******************************************************************************
 * @brief      整数转换为字符串
 * @param[in]  i： 需要转换的数据.
 * @param[in]  pstr: 转换后字符串存放位置.
 * @param[in]  radix： 数据进制
 * @retval     None
 *
 * @details    例如: 345(10进制)-->"345"; 0x345(16进制)-->"345"
 *
 * @note
 ******************************************************************************
 */
void
ositoa(uint32_t i,
        char_t *pstr,
        int32_t radix)
{
    uint32_t num = 1u;
    uint32_t tmp;

    /* first, test the char needs; */
    tmp = i;

    i /= radix;
    while (i > 0)
    {
        num++;
        i /= radix;
    }

    /* set the last ch as '\0' */
    *(pstr + num) = '\0';
    num--;
    while (num > 0)
    {
        *(pstr + num) = (tmp % radix) + '0';
        tmp /= radix;
        num--;
    }
}

/**
 ******************************************************************************
 * @brief      将字符串以特定分隔符分割到数组里面去
 * @param[in]  psrc  需要分割的字符串
 * @param[in]  psep  分隔字符
 * @param[out] pdata 分割到的字节存放的位置
 * @retval     int32_t 返回分割到的字节的个数
 * @details     例如指定psep为", "则 "12, 34, 56, 78" -> {12, 34, 56, 78}， 返回4
 *
 * @note
 ******************************************************************************
 */
int32_t
split(char_t * psrc,
        const char_t * psep,
        uint8_t * pdata)
{
    char_t* p;
    int32_t i = 0;

    p = strtok(psrc, psep);
    while (p != 0)
    {
        pdata[i] = ((uint8_t)atol(p) & 0xFFu);
        i++;
        p = strtok(NULL, psep);
    }
    return i;
}

/**
 ******************************************************************************
 * @brief      在缓冲区内查找指定的内容
 * @param[in]  pfbuf: 被查找的缓冲区指针
 * @param[in]  len: 缓冲区长度
 * @param[in]  pdata: 需要查找的内容指针
 * @param[in]  cmplen: 内容长度
 * @retval     uint32_t: -1-没有找到 其它-缓冲区中内容匹配的起始位置
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
int32_t
match_data(void * pfbuf,
        int32_t len,
        void * pdata,
        uint32_t cmplen)
{
    int32_t i;
    int32_t j;
    if (len >= cmplen)
    {
        j = len - cmplen;
        for (i = 0; i <= j; i++)
        {
            if (memcmp((int8_t *)pdata, (int8_t *)pfbuf + i, cmplen) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

/**
 ******************************************************************************
 * @brief      CRC校验计算
 * @param[in]  pdata 校验内容起始地址
 * @param[in]  count 校验内容长度
 * @retval     uint16 CRC校验结果
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
uint16_t
get_crc16(const int8_t * pdata,
        uint16_t count)
{
    uint16_t crc = 0u;
    uint16_t i;
    while (count > 0)
    {
        crc = crc ^ (uint16_t)((uint16_t)*pdata << 8u);
        pdata++;

        for (i = 0; i < 8; i++)
        {
            if ((crc & 0x8000u) != 0)
            {
                crc = ((uint16_t)(crc << 1)) ^ 0x1021u;
            }
            else
            {
                crc = (uint16_t)(crc << 1u);
            }
        }
        count--;
    }

    return (crc & 0xFFFFu);
}

static const uint32_t the_dw_polynomial = 0x04c11db7u;

/**
 ******************************************************************************
 * @brief      CRC校验计算
 * @param[in]  ptr 校验内容起始地址
 * @param[in]  len 校验内容长度
 * @retval     uint32_t: CRC校验结果
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
uint32_t
get_crc32_stm32(const uint32_t *ptr,
        int32_t len)
{
    uint32_t xbit;
    uint32_t data;
    uint32_t crc = 0xFFFFFFFF;
    while (len > 0)
    {
        len--;
        xbit = 1u << 31u;

        data = *ptr;
        ptr++;
        for (int32_t nbits = 0; nbits < 32; nbits++)
        {
            if ((crc & 0x80000000) != 0x00)
            {
                crc <<= 1;
                crc ^= the_dw_polynomial;
            }
            else
            {
                crc <<= 1;
            }
            if ((data & xbit) != 0x00)
            {
                crc ^= the_dw_polynomial;
            }

            xbit >>= 1;
        }
    }
    return crc;
}

/**
 ******************************************************************************
 * @brief      CRC校验计算
 * @param[in]  precrc 初始crc值
 * @param[in]  ptr    校验内容起始地址
 * @param[in]  len    校验内容长度
 * @retval     uint32_t: CRC校验结果
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
uint32_t
get_crc32_stm32_ex(uint32_t precrc,
        const uint32_t *ptr,
        int32_t len)
{
    uint32_t xbit;
    uint32_t data;
    uint32_t crc = precrc;
    while (len > 0)
    {
        len--;
        xbit = 1u << 31u;

        data = *ptr;
        ptr++;
        for (int32_t nbits = 0; nbits < 32; nbits++)
        {
            if ((crc & 0x80000000) != 0x00)
            {
                crc <<= 1;
                crc ^= the_dw_polynomial;
            }
            else
            {
                crc <<= 1;
            }
            if ((data & xbit) != 0x00)
            {
                crc ^= the_dw_polynomial;
            }

            xbit >>= 1;
        }
    }
    return crc;
}
/**
 ******************************************************************************
 * @brief      三字节BCD码加法运算
 * @param[in]  psrc: 待加的BCD码数组
 * @param[in]  n 待加数
 * @param[out] pdes 加运行后存入的位置
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
inc3bcd(const uint8_t * psrc,
        int32_t n,
        uint8_t * pdes)
{
#if 0
    uint32_t aa;
    uint32_t bb;

    aa.array[0] = 0;
    memcpy(&aa.array[1], psrc, 3);
    bb.longValue = (uint32_t)bcd_to_hex((uint32_t)aa.longValue) + (uint32_t)n;
    bb.longValue = hex_to_bcd((uint32_t)bb.longValue);
    memcpy(pdes, &bb.array[1], 3);
#endif
}

/**
 ******************************************************************************
 * @brief      三字节BCD码加法运算(带bcd码最大限制功能)
 * @param[in]  psrc 待加的BCD码数组
 * @param[in]  n 待加数
 * @param[in]  m  bcd码最大限制值
 * @param[out] pdes 加运行后存入的位置
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
inc3bcdm(const uint8_t * psrc,
        int32_t n,
        int32_t m,
        uint8_t * pdes)
{
#if 0
    U_uint32_t aa;
    U_uint32_t bb;

    aa.array[0] = 0;
    memcpy(&aa.array[1], psrc, 3);
    bb.longValue = (uint32_t)bcd_to_hex((uint32_t)aa.longValue) + (uint32_t)n;
    bb.longValue = ((bb.longValue > m) ? m : bb.longValue);
    bb.longValue = hex_to_bcd((uint32_t)bb.longValue);
    memcpy(pdes, &bb.array[1], 3);
#endif
}

/**
 ******************************************************************************
 * @brief      取得某月的最后一天
 * @param[in]  year 年份
 * @param[in]  month 月份, 取值为[1,12]
 * @retval     uint8 某月的最后一天
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
uint8_t
get_lastday_of_month(uint32_t year,
        uint8_t month)
{
    const static uint8_t monthdays[2][13] =
    {
        {0u, 31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u},
        {0u, 31u, 29u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u}
    };

    uint8_t i = IS_LEAP_YEAR(year);
    return (monthdays[i][month]);
}

/**
 ******************************************************************************
 * @brief      反转缓冲区中的数据
 * @param[in]  pfbuf 缓冲区
 * @param[in]  bytes 缓冲区字节数

 * @retval     None
 *
 * @details    例如: {1, 2, 3, 4, 5} -> {5, 4, 3, 2, 1}.
 *
 * @note
 ******************************************************************************
 */
void
binvert(uint8_t * pfbuf,
        int32_t bytes)
{
    uint8_t *pfbuf_end = pfbuf + (bytes - 1);
    uint8_t temp;

    while (pfbuf < pfbuf_end)
    {
        temp = *pfbuf;
        *pfbuf = *pfbuf_end;
        *pfbuf_end = temp;

        pfbuf_end--;
        pfbuf++;
    }
}

/**
 ******************************************************************************
 * @brief      往后或者往前调整时间值的天数
 * @param[in]  date: 指定的时间
 * @param[in]  i: 要移动的天数，为正表示朝未来，为负表示朝以前
 * @retval     time_t: 调整后的时间值
 ******************************************************************************
 */
time_t
day_inc(time_t date,
        int32_t i)
{
    date = (date == 0) ? time(NULL) : date;

    return (date + (time_t)(i * 24L * 60 * 60));
}

/**
 ******************************************************************************
 * @brief      往后或者往前调整时间值的月份
 * @param[in]  date: 指定的时间
 * @param[in]  i: 要移动的月份数，为正表示朝未来，为负表示朝以前
 * @retval     time_t: 调整后的时间值
 * @note        如果调整后的月份日期数目小于调整前的月份(比如调整10-31 往后一个月)
 *              则取调整后月份的最大日期(11-30)
 ******************************************************************************
 */
time_t
month_inc(time_t date,
        int32_t i)
{
    struct tm dt;

    date = (date == 0) ? time(NULL) : date;
    dt = *localtime(&date);

    /* 修改月份 */
    while (i != 0)
    {
        if (i > 0)
        {
            if (dt.tm_mon == 11)
            {
                dt.tm_year++;
                dt.tm_mon = 0;
            }
            else
            {
                dt.tm_mon++;
            }
            i--;
        }
        else
        {
            if (dt.tm_mon == 0)
            {
                dt.tm_year--;
                dt.tm_mon = 11;
            }
            else
            {
                dt.tm_mon--;
            }
            i++;
        }
    }
    /* 检查日期是否合法 */
    uint32_t lastday = get_lastday_of_month(dt.tm_year + 1900, dt.tm_mon + 1);
    if (dt.tm_mday > lastday)
    {
        dt.tm_mday = lastday;
    }
    return (mktime(&dt));
}

/*--------------------------------osinit.c-----------------------------------*/
