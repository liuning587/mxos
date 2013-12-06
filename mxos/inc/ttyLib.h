/**
 ******************************************************************************
 * @file       ttyLib.h
 * @brief      API include file of ttyLib.h.
 * @details   ͨ��tty�豸����.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __TTYLIB_H__
#define __TTYLIB_H__
/*-----------------------------------------------------------------------------
Section: Includes
-----------------------------------------------------------------------------*/
#include <types.h>
#include <ring.h>
/*-----------------------------------------------------------------------------
Section: Macro Definitions
-----------------------------------------------------------------------------*/
#define TTY_MAJOR   (6u)    /**< tty�����豸��Ϊ6 */
#define WordLength_8b                  ((uint16_t)0x0000)
#define WordLength_9b                  ((uint16_t)0x1000)

#define StopBits_1                     ((uint16_t)0x0000)
#define StopBits_0_5                   ((uint16_t)0x1000)
#define StopBits_2                     ((uint16_t)0x2000)
#define StopBits_1_5                   ((uint16_t)0x3000)

#define Parity_No                      ((uint16_t)0x0000)
#define Parity_Even                    ((uint16_t)0x0400)
#define Parity_Odd                     ((uint16_t)0x0600)

/* serial device I/O controls */
#define TTY_BAUD_SET        0x1003
#define TTY_BAUD_GET        0x1004
#define TTY_FIOFLUSH        0x1010
#define TTY_FIONREAD        0x1011
/*-----------------------------------------------------------------------------
Section: Type Definitions
-----------------------------------------------------------------------------*/
#pragma pack(push, 1)

typedef struct
{
    uint32_t baudrate;      /**< ������ */
    uint16_t wordlength;    /**< ����λ */
    uint16_t stopbits;      /**< ֹͣλ */
    uint16_t parity;        /**< У��λ */
} tty_param_t;

typedef struct
{
    struct ring_buf rd;
    struct ring_buf wt;
} tty_ring_t;

typedef struct tty_exparam tty_exparam_t;
typedef struct
{
    void (*tx_enable)(tty_exparam_t* , bool_e);        /**< �����ж�ʹ�� */
    void (*rx_enable)(tty_exparam_t* , bool_e);        /**< �����ж�ʹ�� */
    void (*tr_enable)(tty_exparam_t* , bool_e);        /**< ����ʹ�� */
    int32_t (*set_param)(tty_exparam_t* , tty_param_t*);/**< ���ò��� */
} tty_opt;

struct tty_exparam
{
    const tty_opt *popt;
    uint32_t baseregs;
    tty_ring_t ring;
};
#pragma pack(pop)

/*-----------------------------------------------------------------------------
Section: Globals
-----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
Section: Function Prototypes
-----------------------------------------------------------------------------*/
extern uint16_t
ttylib_getchar(tty_exparam_t *pexparam, uint8_t *pch);

extern void
ttylib_putchar(tty_exparam_t *pexparam, uint8_t ch);

extern status_t
tty_create(uint8_t ttyno, tty_exparam_t *pexparam, uint16_t rdsz, uint16_t wtsz);

extern void
tty_show_info(void);

#endif /* __TTYLIB_H__ */
/*----------------------------End of ttyLib.h--------------------------------*/
