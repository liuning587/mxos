/**
 ******************************************************************************
 * @file      osLib.c
 * @brief     ����ʵ����OS�Ĺ���������.
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
 * �����ж�
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
 * @brief   ���os logo
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
 * @brief   ���os�汾
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
 * @brief   os��Դ��ʼ��
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
 * @brief      ���ֽ�����ת����hex��ʽ��ascii�ַ���(��'\0'����)
 * @param[in]  pbyte: �ֽ�����
 * @param[out] pstr: ascii�ַ���
 * @param[in]  len: hex����
 * @retval     None
 *
 * @details  ����: {0, 1, 2, 3} -> "01020304"
 *
 * @note    ��������δ���ǵ�����ַ�����������������⣬
 *          ����ʱ��ȷ��asciiָ����ַ�����������С����Ϊ(2 * len + 1)���ֽ�
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
 * @brief      ��ĳ������������ָ���ķ�Χ��(���������޸���Ϊ�߽�ֵ)
 * @param[in]  pvalue: Ҫ����ֵ
 * @param[in]  max_value: ����
 * @param[in]  min_value: ����
 * @param[out] pvalue ��������(��)�����޸�Ϊ��(��)�߽�ֵ
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
 * @brief      �ַ���תHEX����
 * @param[in]  pstr: �ַ���
 * @param[out] phex: hex������
 * @retval     uint32_t д��phex���ֽ���
 * @details     ���� "0123456989ab" -> {1, 23, 45, 67, 89, 171}, ����6
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
 * @brief      ����ת���ɶ����Ʒ�ʽ��ʾ���ַ�������'\0'��β��
 * @param[in]  num   ����ֵ
 * @param[out] pstr   �����������ַ���
 * @param[in]  len ������λ����1~32��
 * @details     ���磺int2bin(0xdeadbeaf, pstr, 7)��, pstrΪ"0101111"
 * @note        ȷ��\a pstrָ��Ļ�����������len+1���ֽ�
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
 * @brief      hexת����bcd
 * @param[in]  hex: hexֵ
 * @retval     uint32_t: ת����ֵ
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
 * @brief      BCD��תHEX
 * @param[in]  bcd bcdֵ
 * @retval     uint32_t: ת����ֵ
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
 * @brief      64λhexת����bcd
 * @param[in]  hex hexֵ
 * @retval     uint64: ת����ֵ
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
 * @brief      64λBCD��תHEX
 * @param[in]  bcd: bcdֵ
 * @retval     uint64: ת����ֵ
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
 * @brief      BCD��תHEX�� 20H --> 20
 * @param[in]  bcd: bcdֵ
 * @retval     int32: ת����ֵ
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
 * @brief      ȡ�õ�ǰ���ʱ�������ܵ�����
 * @param[in]  type: Ҫ��ȡ����������
 *      - 0: ��, ��ΧΪ[0, 59]
 *      - 1: ��, ��ΧΪ[0, 59]
 *      - 2: ʱ, ��ΧΪ[0, 23]
 *      - 3: ��, ��ΧΪ[0, 30]
 *      - 4: ��, ��ΧΪ[1, 12]
 *      - 5: ��, ��ΧΪ[0, n], ��ʼΪ2000��
 *      - 6: ���ڼ�, ��ΧΪ[0, 6]
 * @retval     int32: ��ȡ�����ݣ���\a type���Ϸ��򷵻ظ���
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
 * @brief      ��ʱ��ת�����ַ�����ʽ
 * @param[in]  dt: ʱ��
 * @param[out] pstr: �ַ���
 * @retval     None
 * @note  ��ʽ�����ɵ��ַ���Ϊ"YYYY-MM-DD HH:MM:SS",
 *          ���\a pstr�Ļ�������С����Ӧ��20���ֽ�
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
 * @brief      ת��ʱ�����ݸ�ʽ
 * @param[in]  pdata: ʱ������,BCD��ʽ
 * @retval     time_t��ʱ��
 * @note      ����Ĳ���\a pdata����Ӧ����6�ֽڣ���������ֱ�Ϊ
 *      - ƫ��0: ��(��2000�꿪ʼ)
 *      - ƫ��1: ��,[1-12]
 *      - ƫ��2: ��
 *      - ƫ��3: ʱ
 *      - ƫ��4: ��
 *      - ƫ��5: ��
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
 * @brief      ת��ʱ�����ݸ�ʽ
 * @param[in]  pdata  ʱ������
 * @retval     time_t ʱ��
 * @note      ����Ĳ���\a pdata����Ӧ����6�ֽڣ���������ֱ�Ϊ
 *      - ƫ��0: ��(��2000�꿪ʼ)
 *      - ƫ��1: ��,[1-12]
 *      - ƫ��2: ��
 *      - ƫ��3: ʱ
 *      - ƫ��4: ��
 *      - ƫ��5: ��
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
 * @brief      ��ӡָ��ʱ��
 * @param[in]  pinfo: ��ӡ����(λ��ʱ������ǰ����ı�ǰ׺)
 * @param[in]  time ָ��ʱ��
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
 * @brief      ��ӡ��ǰʱ��
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
 * @brief       ��ӡ��ǰ����ʽ�ַ�����ָ�����ȵĶ����ƻ�����
 * @param[in]   pfmt  ǰ��(�����Ե�)�ַ���
 * @param[in]   pfrm   ��������ַ
 * @param[in]   len       �������ֽ���
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
 * @brief      ����У���
 * @param[in]  pfbuf ������
 * @param[in]  len  ����������
 * @retval     uint8 У���
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
 * @brief      �жϸ����Ļ����������Ƿ�ȫ����ָ�����ַ�
 * @param[in]  pfbuf:  ��Ҫ�жϵĻ���������
 * @param[in]  c�� ���ڱȽϵ��ַ�
 * @param[in]  len�� ����������
 * @retval     int32: 0-��� ����-�����
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
 * @brief      �жϸ����Ļ����������Ƿ�ȫΪBCD��
 * @param[in]  pfbuf:  ��Ҫ�жϵĻ���������
 * @param[in]  len��  ����������
 * @retval     bool_e: TRUE-��BCD�� FALSE-��BCD��
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
 * @brief      ����ת��Ϊ�ַ���
 * @param[in]  i�� ��Ҫת��������.
 * @param[in]  pstr: ת�����ַ������λ��.
 * @param[in]  radix�� ���ݽ���
 * @retval     None
 *
 * @details    ����: 345(10����)-->"345"; 0x345(16����)-->"345"
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
 * @brief      ���ַ������ض��ָ����ָ��������ȥ
 * @param[in]  psrc  ��Ҫ�ָ���ַ���
 * @param[in]  psep  �ָ��ַ�
 * @param[out] pdata �ָ���ֽڴ�ŵ�λ��
 * @retval     int32_t ���طָ���ֽڵĸ���
 * @details     ����ָ��psepΪ", "�� "12, 34, 56, 78" -> {12, 34, 56, 78}�� ����4
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
 * @brief      �ڻ������ڲ���ָ��������
 * @param[in]  pfbuf: �����ҵĻ�����ָ��
 * @param[in]  len: ����������
 * @param[in]  pdata: ��Ҫ���ҵ�����ָ��
 * @param[in]  cmplen: ���ݳ���
 * @retval     uint32_t: -1-û���ҵ� ����-������������ƥ�����ʼλ��
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
 * @brief      CRCУ�����
 * @param[in]  pdata У��������ʼ��ַ
 * @param[in]  count У�����ݳ���
 * @retval     uint16 CRCУ����
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
 * @brief      CRCУ�����
 * @param[in]  ptr У��������ʼ��ַ
 * @param[in]  len У�����ݳ���
 * @retval     uint32_t: CRCУ����
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
 * @brief      CRCУ�����
 * @param[in]  precrc ��ʼcrcֵ
 * @param[in]  ptr    У��������ʼ��ַ
 * @param[in]  len    У�����ݳ���
 * @retval     uint32_t: CRCУ����
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
 * @brief      ���ֽ�BCD��ӷ�����
 * @param[in]  psrc: ���ӵ�BCD������
 * @param[in]  n ������
 * @param[out] pdes �����к�����λ��
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
 * @brief      ���ֽ�BCD��ӷ�����(��bcd��������ƹ���)
 * @param[in]  psrc ���ӵ�BCD������
 * @param[in]  n ������
 * @param[in]  m  bcd���������ֵ
 * @param[out] pdes �����к�����λ��
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
 * @brief      ȡ��ĳ�µ����һ��
 * @param[in]  year ���
 * @param[in]  month �·�, ȡֵΪ[1,12]
 * @retval     uint8 ĳ�µ����һ��
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
 * @brief      ��ת�������е�����
 * @param[in]  pfbuf ������
 * @param[in]  bytes �������ֽ���

 * @retval     None
 *
 * @details    ����: {1, 2, 3, 4, 5} -> {5, 4, 3, 2, 1}.
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
 * @brief      ���������ǰ����ʱ��ֵ������
 * @param[in]  date: ָ����ʱ��
 * @param[in]  i: Ҫ�ƶ���������Ϊ����ʾ��δ����Ϊ����ʾ����ǰ
 * @retval     time_t: �������ʱ��ֵ
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
 * @brief      ���������ǰ����ʱ��ֵ���·�
 * @param[in]  date: ָ����ʱ��
 * @param[in]  i: Ҫ�ƶ����·�����Ϊ����ʾ��δ����Ϊ����ʾ����ǰ
 * @retval     time_t: �������ʱ��ֵ
 * @note        �����������·�������ĿС�ڵ���ǰ���·�(�������10-31 ����һ����)
 *              ��ȡ�������·ݵ��������(11-30)
 ******************************************************************************
 */
time_t
month_inc(time_t date,
        int32_t i)
{
    struct tm dt;

    date = (date == 0) ? time(NULL) : date;
    dt = *localtime(&date);

    /* �޸��·� */
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
    /* ��������Ƿ�Ϸ� */
    uint32_t lastday = get_lastday_of_month(dt.tm_year + 1900, dt.tm_mon + 1);
    if (dt.tm_mday > lastday)
    {
        dt.tm_mday = lastday;
    }
    return (mktime(&dt));
}

/*--------------------------------osinit.c-----------------------------------*/
