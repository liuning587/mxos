/**
 ******************************************************************************
 * @file      devnull.c
 * @brief     本文实现NULL驱动.
 * @details   This file including all API functions's implement of devnull.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <stdio.h>
#include <types.h>
#include <devLib.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

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
 * @brief   NULL设备初始化方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     OK   : 初始化成功
 ******************************************************************************
 */
static status_t
devnull_init(struct device* dev)
{
    return OK;
}

/**
 ******************************************************************************
 * @brief   NULL设备打开方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     OK   : 打开成功
 ******************************************************************************
 */
static status_t
devnull_open(struct device* dev)
{
    return OK;
}

/**
 ******************************************************************************
 * @brief   NULL设备释放方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     OK   : 释放成功
 ******************************************************************************
 */
static status_t
devnull_relesae(struct device* dev)
{
    return OK;
}

/**
 ******************************************************************************
 * @brief   NULL设备关闭方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     OK   : 关闭成功
 ******************************************************************************
 */
static status_t
devnull_close(struct device* dev)
{
    return OK;
}

/**
 ******************************************************************************
 * @brief   NULL设备读取方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     size
 ******************************************************************************
 */
static size_t
devnull_read(struct device* dev, int32_t pos, void *buffer, size_t size)
{
    return size;
}

/**
 ******************************************************************************
 * @brief   NULL设备写入方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval     size
 ******************************************************************************
 */
static size_t
devnull_write(struct device* dev, int32_t pos, const void *buffer, size_t size)
{
    return size;
}


/**
 ******************************************************************************
 * @brief   NULL设备控制方法
 * @param[in]  None
 * @param[out] None
 *
 * @retval  0
 ******************************************************************************
 */
static int32_t
devnull_ioctl(struct device* dev, uint32_t cmd, void *args)
{
    return 0;
}

const static fileopt_t the_testopt =
{
    .init = devnull_init,
    .release = devnull_relesae,
    .open = devnull_open,
    .close = devnull_close,
    .read = devnull_read,
    .write = devnull_write,
    .ioctl = devnull_ioctl,
};


/**
 ******************************************************************************
 * @brief   创建NULL设备
 * @param[in]  None
 * @param[out] None
 * @retval     OK   : 创建成功
 * @retval     ERROR: 创建失败
 ******************************************************************************
 */
status_t
devnull_create(void)
{
    if (OK != dev_create("NULL", &the_testopt, MKDEV(0, 0), NULL))
    {
        printf("devnull create err\n");
        return ERROR;
    }
    printf("devnull create OK!\n");
    return OK;
}

/*-------------------------------devnull.c-----------------------------------*/
