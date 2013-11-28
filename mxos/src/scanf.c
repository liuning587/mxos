/**
 ******************************************************************************
 * @file       scanf.c
 * @author     liuning
 * @brief      本文代替libc的sscanf在嵌入式平台中极大减小资源占用
 * @details    ROM空间至少省出20k  RAM至少节约大概1.5k
 * @copyright
 ******************************************************************************
 */

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   查找到第一个数字
 * @param[in]  *str : 输入字符串
 * @param[in]  base : 进制
 *
 * @retval  NULL : 找不到
 * @retval !NULL : 其实位置
 ******************************************************************************
 */
static char *
scan_string (const char *str, int base)
{
    switch (base)
    {
    case 10:
        while (!(isdigit(*str) || *str == '-' || *str == 0x0))
        {
            str++;
        }
        break;
    case 16:
        while (!(isxdigit(*str) || *str == 0x0))
        {
            str++;
        }
        break;
    }

    return (char*)str;
}

/**
 ******************************************************************************
 * @brief   将制定长度的字符串转换为相应格式的字符
 * @param[in]  *nptr : 输入字符首地址
 * @param[out] **ptr : 返回字符串结束地址 ,若为NULL表示转换失败
 * @param[in]  len   : 输入字符串长度,若为-1表示不限定长度
 * @param[in]  base  : 10进制16进制
 *
 * @retval  转换后的数值
 ******************************************************************************
 */
static long
strntol(const char *nptr,
        const char **ptr,
        int len,
        int base)
{
    int sig = 1;
    long ret = 0;

    switch (base)
    {
    case 10:
        if (*nptr == '-')
        {
            sig = -1;
            if (len > 0) len--;
            nptr++;
        }
        for (; *nptr && ((len > 0) || (len == -1)); nptr++)
        {
            if (isdigit(*nptr))
            {
                ret = ret * base + (*nptr - '0');
            }
            else
            {
                break;
            }
            if (len > 0) len--;
        }
        break;

    case 16: /* 16进制没有负数概念吧! */
        for (; *nptr && ((len > 0) || (len == -1)); nptr++)
        {
            if (isxdigit(*nptr))
            {
                ret *= base;
                if (isdigit(*nptr))
                {
                    ret += (*nptr - '0');
                }
                else if (islower(*nptr))
                {
                    ret += (*nptr - 'a' + 10);
                }
                else
                {
                    ret += (*nptr - 'A' + 10);
                }
            }
            else
            {
                break;
            }
            if (len > 0) len--;
        }
        break;
    }
    *ptr = nptr;

    return ret * sig;
}

/**
 ******************************************************************************
 * @brief   代替libc的sscanf在嵌入式平台中极大减小资源占用
 * @param[in]  *str    :
 * @param[in]  *format :
 *
 * @retval  正确处理格式的个数
 ******************************************************************************
 */
int
sscanf(const char *str, const char *format, ...)
{
    int width;
    int ret;
    int *p_int;
    va_list ap;

    ret = 0;

    va_start (ap, format);

    for (; *format != 0; ++format)
    {
        if (*format == '%')
        {
            if (*format == '%')
            {
                ++format;
                width = 0;
            }
            if (*format == '\0') break;
            if (*format == '%') continue;
            if (*format == '-')
            {
                ++format;
            }
            while (*format == '0')
            {
                ++format;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format)
            {
                width *= 10;
                width += *format - '0'; /* 获取长度 */
            }
            width = (width == 0) ? -1 : width;

            switch (*format)
            {
            case 'd':
            case 'i':   /* 10进制 */
            case 'u':
                p_int = va_arg( ap, int *);
                str = scan_string(str, 10);
                if (*str == 0x0) goto end_parse;
                *p_int = (int)strntol(str, &str, width, 10);
                ret ++;
                break;

            case 'x':
            case 'X': /* 16进制 */
                p_int = va_arg( ap, int *);
                str = scan_string(str, 16);
                if (*str == 0x0) goto end_parse;
                if ((*str == '0') && ((*(str + 1) == 'x') || (*(str + 1) == 'X')))
                {
                    str += 2;
                }
                *p_int = (int)strntol(str, &str, width, 16);
                ret ++;
                break;
            }
        }
    }

end_parse:
    va_end (ap);

    if (*format == 0x0) ret = EOF;
    return ret;
}

/*---------------------------------scanf.c-----------------------------------*/
