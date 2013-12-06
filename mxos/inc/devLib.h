/**
 ******************************************************************************
 * @file       devlib.c
 * @brief      �豸��������
 * @details    This file including all API functions's implement of dps.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __DEVLIB_H__
#define __DEVLIB_H__

/*-----------------------------------------------------------------------------
Section: Includes
-----------------------------------------------------------------------------*/
#include <stddef.h>
#include <types.h>
#include <listLib.h>
#include <ring.h>
#include <taskLib.h>

/*-----------------------------------------------------------------------------
Section: Macro Definitions
-----------------------------------------------------------------------------*/
#define MAX_DEVICE_NAME     (5u)    /**< ����豸������ */
#define MAX_DEVICE_NUM      (5u)    /**< ����豸�� */
#define MAX_OPEN_NUM        (5u)    /**< ���ͬʱ���豸�� */

/** �豸��ģʽ */
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_NONBLOCK  00004000

/** �豸��С��� */
#define MINORBITS   20
#define MINORMASK   ((1U << MINORBITS) - 1)

#define MAJOR(dev)  ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)  ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)    (((ma) << MINORBITS) | (mi))

#pragma pack(push, 1)
struct device;
typedef struct fileopt
{
    status_t  (*init)   (struct device* dev);
    status_t  (*release)(struct device* dev);
    status_t  (*open)   (struct device* dev);
    status_t  (*close)  (struct device* dev);
    size_t    (*read)   (struct device* dev, int32_t pos, void *buffer, size_t size);
    size_t    (*write)  (struct device* dev, int32_t pos, const void *buffer, size_t size);
    int32_t   (*ioctl)  (struct device* dev, uint32_t cmd, void *args);
} fileopt_t;

typedef struct device
{
    struct ListNode list;           /**< ͨ������ڵ� */
    const struct fileopt *pfileopt; /**< �豸�������� */
    char_t name[MAX_DEVICE_NAME];   /**< �豸�� */
    SEM_ID lock;                    /**< �豸������ */
    int32_t serial;                 /**< �豸�����к� */
    int32_t flags;                  /**< �豸��ģʽ */
    int32_t offset;                 /**< ��дƫ�Ƶ�ַ */
    int32_t usrs;                   /**< �豸�򿪴��� */
    void* param;                    /**< �豸��չ����,����ring buf */
} device_t;

#pragma pack(pop)

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern status_t
devlib_init(void);

extern status_t
dev_create(const char_t* pname,
        const fileopt_t* pfileopt,
        int32_t serial,
        void* pexparam);

extern status_t
dev_release(const char_t* pname);

extern int32_t
dev_open(const char_t* pname,
        int32_t flags);

extern int32_t
dev_read(int32_t fd,
        void* buf,
        int32_t count);

extern int32_t
dev_write(int32_t fd,
        const void* buf,
        int32_t count);

extern int32_t
dev_ioctl(int32_t fd,
        uint32_t cmd,
        void *args);

extern int32_t
dev_close(int32_t fd);

extern device_t*
devlib_get_info_by_name(const char_t *pname);

extern device_t*
devlib_get_info_by_serial(int32_t serial);

extern void
devlib_show_info(void);

#endif /* __DEVLIB_H__ */
/*-----------------------------End of devLib.h-------------------------------*/
