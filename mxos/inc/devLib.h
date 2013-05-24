/**
 ******************************************************************************
 * @file       devlib.c
 * @brief      设备驱动操作
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
#define MAX_DEVICE_NAME     (5u)    /**< 最大设备名长度 */
#define MAX_DEVICE_NUM      (5u)    /**< 最大设备数 */
#define MAX_OPEN_NUM        (5u)    /**< 最大同时打开设备数 */

/** 设备打开模式 */
#define O_RDONLY    00000000
#define O_WRONLY    00000001
#define O_RDWR      00000002
#define O_NONBLOCK  00004000

/** 设备大小类号 */
#define MINORBITS   20
#define MINORMASK   ((1U << MINORBITS) - 1)

#define MAJOR(dev)  ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)  ((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)    (((ma) << MINORBITS) | (mi))

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
    struct ListNode list;           /**< 通用链表节点 */
    const struct fileopt *pfileopt; /**< 设备操作方法 */
    char_t name[MAX_DEVICE_NAME];   /**< 设备名 */
    SEM_ID lock;                    /**< 设备操作锁 */
    int32_t serial;                 /**< 设备的序列号 */
    int32_t flags;                  /**< 设备打开模式 */
    int32_t offset;                 /**< 读写偏移地址 */
    int32_t usrs;                   /**< 设备打开次数 */
    void* param;                    /**< 设备扩展参数,例如ring buf */
} device_t;

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern status_t devlib_init(void);
extern status_t dev_create(const char_t* pname, const fileopt_t* pfileopt, int32_t serial, void* pexparam);
extern status_t dev_release(const char_t* pname);
extern int32_t dev_open(const char_t* pname, int32_t flags);
extern int32_t dev_read(int32_t fd, void* buf, int32_t count);
extern int32_t dev_write(int32_t fd, const void* buf, int32_t count);
extern int32_t dev_ioctl(int32_t fd, uint32_t cmd, void *args);
extern int32_t dev_close(int32_t fd);
extern device_t* devlib_get_info_by_name(const char_t *pname);
extern device_t* devlib_get_info_by_serial(int32_t serial);
extern void devlib_show_info(void);

#endif /* __DEVLIB_H__ */
/*-----------------------------End of devLib.h-------------------------------*/
