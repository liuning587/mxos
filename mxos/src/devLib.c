/**
 ******************************************************************************
 * @file      devlib.c
 * @brief     C Source file of devlib.c.
 * @details   设备驱动操作
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
Section: Includes
-----------------------------------------------------------------------------*/
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <intLib.h>
#include <devLib.h>


#ifdef Dprintf
#undef Dprintf
#endif
#define Dprintf(x...)

static SEM_ID the_devlib_lock = NULL;
static struct ListNode the_dev_list;    /* 指向设备链表 */
static device_t* the_opend_devs[MAX_OPEN_NUM];  /* 已打开设备池 */

/**
 ******************************************************************************
 * @brief 判断句柄是否合法
 * @param[in]  realfd
 * @retval     FALSE    : 非法
 * @retval     TRUE     : 合法
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static bool_e
is_fd_valid(int32_t realfd)
{
    if (realfd >= MAX_OPEN_NUM)
    {
        Dprintf("fd is out of range!\n");
        return FALSE;
    }

    if (the_opend_devs[realfd] == NULL)
    {
        Dprintf("fd is not opend!\n");
        return FALSE;
    }

    return TRUE;
}

/**
 ******************************************************************************
 * @brief 寻找空闲句柄空间,并插入
 * @param[in]  *pnode    : 设备描述符
 * @param[out] None
 * @retval     -1   : 无空闲空间
 * @retval     >0   : 插入的句柄
 ******************************************************************************
 */
static int32_t
find_free_fd(device_t* pnode)
{
    for (int32_t i = 0; i < MAX_OPEN_NUM; i++)
    {
        if (the_opend_devs[i] == NULL)
        {
            the_opend_devs[i] = pnode;
            Dprintf("find free fd node\n");
            return i + 1;   /* 注意返回句柄加1防止句柄 */
        }
    }
    Dprintf("can not find free fd node\n");

    return -1;
}

/**
 ******************************************************************************
 * @brief      在已打开文件列表中查询文件
 * @param[in]  * pname  : 查找的设备名
 *
 * @retval     -1   ：找不到
 * @retval     >=0  : 已打开文件数组下标(实际的文件句柄)
 ******************************************************************************
 */
static int32_t
find_opend_by_name(const char_t* pname)
{
    for (int32_t i = 0; i < MAX_OPEN_NUM; i++)
    {
        if (strncmp(the_opend_devs[i]->name, pname, MAX_DEVICE_NAME) == 0)
        {
            Dprintf("dev:%s is using\n", pname);
            return i;
        }
    }
    return -1;
}

/**
 ******************************************************************************
 * @brief 寻找已注册的设备 (按名称查找)
 * @param[in]  * pname  : 查找的设备名
 *
 * @retval     NULL ：找不到
 * @retval     非空    : 设备描述符地址
 ******************************************************************************
 */
static device_t*
find_dev_by_name(const char_t* pname)
{
    device_t* pnode = NULL;
    struct ListNode *iter;

    LIST_FOR_EACH(iter, &the_dev_list)
    {
        /* 取得遍历到的对象 */
        pnode = MemToObj(iter, struct device, list);
        if (strncmp(pnode->name, pname, sizeof(pnode->name)) == 0)
        {
            return pnode;
        }
    }
    return NULL;
}

/**
 ******************************************************************************
 * @brief   寻找已注册的设备 (按大类号小类号查找)
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
static device_t*
find_dev_by_serial(int32_t serial)
{
    device_t* pnode = NULL;
    struct ListNode *iter;

    LIST_FOR_EACH(iter, &the_dev_list)
    {
        /* 取得遍历到的对象 */
        pnode = MemToObj(iter, struct device, list);
        if (pnode->serial == serial)
        {
            return pnode;
        }
    }
    return NULL;
}
/**
 ******************************************************************************
 * @brief      设备核心初始化
 *
 * @retval     OK   : 初始化成功
 * @retval     ERROR: 初始化失败
 ******************************************************************************
 */
status_t
devlib_init(void)
{
    if (the_devlib_lock != NULL)
        return OK;
    if ((the_devlib_lock = semBCreate(1)) == NULL)
    {
        Dprintf("semBCreate err\n");
        return ERROR;
    }
    InitListHead(&the_dev_list);
    Dprintf("init OK\n");

    return OK;
}

/**
 ******************************************************************************
 * @brief 创建设备
 * @param[in]  *pname       : 设备名
 * @param[in]  *pfileopt    ：设备操作方法
 * @param[in]  serial       ：设备序列号（大小类号）

 * @retval  OK      : 成功
 * @retval  ERROR   : 失败
 ******************************************************************************
 */
status_t
dev_create(const char_t* pname, const fileopt_t* pfileopt, int32_t serial, void* pexparam)
{
    D_ASSERT(pname != NULL);
    D_ASSERT(pfileopt != NULL);

    (void)devlib_init();

    /* 防止重复注册 */
    if ((find_dev_by_name(pname) != NULL)
            || (find_dev_by_serial(serial) != NULL))
    {
        return ERROR;
    }
    device_t* new = malloc(sizeof(struct device));
    if (new == NULL)
    {
        printf("dev_create out of mem! when creat name: %s.\n", pname);
        return ERROR;
    }

    memset(new, 0x00, sizeof(struct device));
    new->lock = semBCreate(1);
    if (new->lock == NULL)
    {
        printf("dev_create create sem err.\n");
        free(new);
        return ERROR;
    }
    strncpy(new->name, pname, sizeof(new->name));
    new->pfileopt = pfileopt;
    new->serial = serial;
    new->param = pexparam;
    if (new->pfileopt->init != NULL)
    {
        if (new->pfileopt->init(new) != OK)
        {
            semDelete(new->lock);
            free(new);
            return ERROR;
        }
    }
    /* 将新设备节点加入链表 */
    semTake(the_devlib_lock, WAIT_FOREVER);
    ListAddTail(&new->list, &the_dev_list);
    semGive(the_devlib_lock);

    return OK;
}

/**
 ******************************************************************************
 * @brief 设备释放
 * @param[in]  *pname   : 设备名
 *
 * @retval  OK      : 成功
 * @retval  ERROR   : 失败
 ******************************************************************************
 */
status_t
dev_release(const char_t* pname)
{
    D_ASSERT(pname != NULL);

    /* 判断设备是否在使用 */
    if (0 < find_opend_by_name(pname))
    {
        Dprintf("dev:%s is using\n", pname);
        return ERROR;
    }

    device_t* pnode = find_dev_by_name(pname);
    if (pnode == NULL)
    {
        Dprintf("dev:%s is not found\n", pname);
        return ERROR;
    }

    if (pnode->pfileopt->release != NULL)
    {
        if (OK != pnode->pfileopt->release(pnode))
        {
            Dprintf("dev:%s release err!\n\n", pname);
            return ERROR;
        }
    }

    semDelete(pnode->lock);
    semTake(the_devlib_lock, WAIT_FOREVER);
    ListDelNode(&pnode->list);
    semGive(the_devlib_lock);
    free(pnode);

    return OK;
}

/**
 ******************************************************************************
 * @brief 设备打开
 * @param[in]  *pname   : 设备名
 * @param[in]   flags   : 打开模式O_RDONLY | OWRONLY | O_RDWR
 *
 * @retval  >=0   : 成功
 * @retval  - 1   : 失败
 ******************************************************************************
 */
int32_t
dev_open(const char_t* pname, int32_t flags)
{
    int32_t fd;
    device_t* pnode;

    D_ASSERT(pname != NULL);

    /* 如果设备已打开则仅将文件打开次数累加 */
    fd = find_opend_by_name(pname);
    if ((fd = find_opend_by_name(pname)) >= 0)
    {
        pnode = the_opend_devs[fd];
        (void)semTake(pnode->lock, WAIT_FOREVER);
        pnode->usrs++;
        (void)semGive(pnode->lock);
        return OK;
    }
    if ((pnode = find_dev_by_name(pname)) == NULL)
    {
        Dprintf("dev:%s is not found\n", pname);
        return -1;
    }
    if ((fd = find_free_fd(pnode)) < 0)
    {
        Dprintf("dev: opend max file.\n");
        return -1;
    }

    pnode->flags = flags;
    if (pnode->pfileopt->open != NULL)
    {
        (void)semTake(pnode->lock, WAIT_FOREVER);
        if (OK != pnode->pfileopt->open(pnode))
        {
            (void)semGive(pnode->lock);
            Dprintf("dev: opend err.\n");
            return -1;
        }
        (void)semGive(pnode->lock);
    }
    pnode->usrs++;

    return fd;
}

/**
 ******************************************************************************
 * @brief 设备读
 * @param[in]  fd       : 设备句柄
 * @param[in] *buf      : 写缓存地址
 * @param[in]  count    : 写入字节数

 * @retval  >=0   : 成功
 * @retval  - 1   : 失败
 ******************************************************************************
 */
int32_t
dev_read(int32_t fd, void* buf, int32_t count)
{
    int32_t size = -1;
    int32_t realfd = fd - 1;    /* 这里取得真实的fd */

    if (FALSE == is_fd_valid(realfd))
    {
        return -1;
    }
    device_t* pnode = the_opend_devs[realfd];

    if ((pnode->pfileopt->read == NULL) || (pnode->flags & O_WRONLY))
    {
        return -1;
    }

    (void)semTake(pnode->lock, WAIT_FOREVER);
    size = pnode->pfileopt->read(pnode, pnode->offset, buf, count);
    (void)semGive(pnode->lock);

    return size;
}

/**
 ******************************************************************************
 * @brief 设备写
 * @param[in]  fd       : 设备句柄
 * @param[in] *buf      : 写缓存地址
 * @param[in]  count    : 写入字节数

 * @retval  >=0   : 成功
 * @retval  - 1   : 失败
 ******************************************************************************
 */
int32_t
dev_write(int32_t fd, const void* buf, int32_t count)
{
    int32_t size = -1;
    int32_t realfd = fd - 1;    /* 这里取得真实的fd */

    if (FALSE == is_fd_valid(realfd))
    {
        return -1;
    }
    device_t* pnode = the_opend_devs[realfd];

    if ((pnode->pfileopt->write == NULL) || (pnode->flags & O_RDONLY))
    {
        return -1;
    }

    (void)semTake(pnode->lock, WAIT_FOREVER);
    size = pnode->pfileopt->write(pnode, pnode->offset, buf, count);
    (void)semGive(pnode->lock);

    return size;
}

/**
 ******************************************************************************
 * @brief 设备ioctl
 * @param[in]  fd       : 设备句柄
 * @param[in]  cmd      : 命令
 * @param[in]  *args    : 参数

 * @retval  >=0   : 成功
 * @retval  -1    : 失败
 ******************************************************************************
 */
int32_t
dev_ioctl(int32_t fd, uint32_t cmd, void *args)
{
    int32_t realfd = fd - 1;    /* 这里取得真实的fd */

    if (FALSE == is_fd_valid(realfd))
    {
        return -1;
    }
    device_t* pnode = the_opend_devs[realfd];

    if (pnode->pfileopt->ioctl == NULL)
    {
        return -1;
    }

    (void)semTake(pnode->lock, WAIT_FOREVER);
    int32_t ret = pnode->pfileopt->ioctl(pnode, cmd, args);
    (void)semGive(pnode->lock);

    return ret;
}

/**
 ******************************************************************************
 * @brief 设备关闭
 * @param[in]  fd       : 设备句柄
 *
 * @retval  1   : 关闭成功
 * @retval -1   : 关闭失败
 ******************************************************************************
 */
int32_t
dev_close(int32_t fd)
{
    int32_t realfd = fd - 1;    /* 这里取得真实的fd */

    if (FALSE == is_fd_valid(realfd))
    {
        return -1;
    }
    device_t* pnode = the_opend_devs[realfd];
    pnode->usrs--;
    if (pnode->usrs <= 0)
    {
        if (pnode->pfileopt->close != NULL)
        {
            if (OK != pnode->pfileopt->close(pnode))
            {
                return -1;
            }
        }
        the_opend_devs[realfd] = NULL;  /* release fd */
    }

    return 1;
}

/**
 ******************************************************************************
 * @brief      通过设备名获取设备节点信息
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
device_t*
devlib_get_info_by_name(const char_t *pname)
{
    device_t *pdev = NULL;

    semTake(the_devlib_lock, WAIT_FOREVER);
    pdev = find_dev_by_name(pname);
    semGive(the_devlib_lock);
    return pdev;
}

/**
 ******************************************************************************
 * @brief      通过大类号小类号获取设备节点信息
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
device_t*
devlib_get_info_by_serial(int32_t serial)
{
    device_t *pdev = NULL;

    semTake(the_devlib_lock, WAIT_FOREVER);
    pdev = find_dev_by_serial(serial);
    semGive(the_devlib_lock);
    return pdev;
}

/**
 ******************************************************************************
 * @brief      显示设备信息
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
devlib_show_info(void)
{
    if (OK != devlib_init())
    {
        printf("devlib init err!\n");
        return ;
    }
    printf("   list devices:\n");
    printf("  -- dev name -- \r\t\t-- opend num --\n");
    device_t* pnode = NULL;
    struct ListNode *iter;

    semTake(the_devlib_lock, WAIT_FOREVER);
    LIST_FOR_EACH(iter, &the_dev_list)
    {
        /* 取得遍历到的对象 */
        pnode = MemToObj(iter, struct device, list);
        printf("       %s\r\t\t      %d\n", pnode->name, pnode->usrs);
    }
    semGive(the_devlib_lock);
    //printf("  --- --- --- ---\r\t\t--- --- --- ---\n");
}
/*------------------------------- devlib.c ----------------------------------*/
