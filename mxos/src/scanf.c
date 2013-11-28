/**
 ******************************************************************************
 * @file       scanf.c
 * @author     liuning
 * @brief      ���Ĵ���libc��sscanf��Ƕ��ʽƽ̨�м����С��Դռ��
 * @details    ROM�ռ�����ʡ��20k  RAM���ٽ�Լ���1.5k
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
 * @brief   ���ҵ���һ������
 * @param[in]  *str : �����ַ���
 * @param[in]  base : ����
 *
 * @retval  NULL : �Ҳ���
 * @retval !NULL : ��ʵλ��
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
 * @brief   ���ƶ����ȵ��ַ���ת��Ϊ��Ӧ��ʽ���ַ�
 * @param[in]  *nptr : �����ַ��׵�ַ
 * @param[out] **ptr : �����ַ���������ַ ,��ΪNULL��ʾת��ʧ��
 * @param[in]  len   : �����ַ�������,��Ϊ-1��ʾ���޶�����
 * @param[in]  base  : 10����16����
 *
 * @retval  ת�������ֵ
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

    case 16: /* 16����û�и��������! */
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
 * @brief   ����libc��sscanf��Ƕ��ʽƽ̨�м����С��Դռ��
 * @param[in]  *str    :
 * @param[in]  *format :
 *
 * @retval  ��ȷ�����ʽ�ĸ���
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
                width += *format - '0'; /* ��ȡ���� */
            }
            width = (width == 0) ? -1 : width;

            switch (*format)
            {
            case 'd':
            case 'i':   /* 10���� */
            case 'u':
                p_int = va_arg( ap, int *);
                str = scan_string(str, 10);
                if (*str == 0x0) goto end_parse;
                *p_int = (int)strntol(str, &str, width, 10);
                ret ++;
                break;

            case 'x':
            case 'X': /* 16���� */
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
