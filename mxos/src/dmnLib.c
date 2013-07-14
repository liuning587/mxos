/**
 ******************************************************************************
 * @file      dmnLib.c
 * @brief     本文实现了任务间喂软件狗的机制.
 * @details   This file including all API functions's implement of dmnLib.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <taskLib.h>
#include <listLib.h>
#include <dmnLib.h>
#include <debug.h>
#include <oshook.h>
#include <oscfg.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
#pragma pack(push, 1)
typedef struct
{
    struct ListNode nlist;
    TASK_ID taskid;
    int16_t count;
} dmn_t;
#pragma pack(pop)

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
#ifndef DMN_MAX_CHECK_TIME
# define DMN_MAX_CHECK_TIME         (6u)    /**< 默认喂狗超时时间（6*10秒） */
#endif

#ifndef TASK_PRIORITY_DMN
# define TASK_PRIORITY_DMN           (3u)    /**< 默认任务优先级 */
#endif

#ifndef TASK_STK_SIZE_DMN
# define TASK_STK_SIZE_DMN         (512u)    /**< 默认任务堆栈 */
#endif

#if (DMN_MAX_CHECK_TIME > 10u)
# error "Plesase set DMN_MAX_CHECK_TIME <= (10u)"
#endif
/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
bool_e theRebootSignal = FALSE;

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static struct ListNode the_registed_list;
static SEM_ID the_dmn_sem = NULL;
static TASK_ID the_dmn_id = NULL;

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
 ----------------------------------------------------------------------------*/
static bool_e
is_taskname_used(TASK_ID task);

static void
dmn_loop(void);

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   dmnLib初始化
 * @param[in]  None
 * @param[out] None
 * @retval  OK      : 初始化成功
 * @retval  ERROR   : 初始化失败
 ******************************************************************************
 */
status_t
dmn_init(uint32_t stacksize)
{
    if (the_dmn_id != NULL)
    {
        return ERROR;
    }
    stacksize = (stacksize == 0) ? TASK_STK_SIZE_DMN : stacksize;
    InitListHead(&the_registed_list);
    the_dmn_sem = semBCreate(1);
    D_ASSERT(the_dmn_sem != NULL);
    the_dmn_id = taskSpawn((const signed char * const )"daemon",
            TASK_PRIORITY_DMN, stacksize, (OSFUNCPTR)dmn_loop, 0u);
    D_ASSERT(the_dmn_id != NULL);

    return OK;
}

/**
 ******************************************************************************
 * @brief      在守护任务链表中寻找是否重复任务
 * @param[in]  None
 * @param[out] None
 * @retval     TRUE : 已存在
 * @retval     FALSE: 不存在
 ******************************************************************************
 */
static bool_e
is_taskname_used(TASK_ID taskid)
{
    dmn_t* pdmn = NULL;
    struct ListNode *piter = NULL;

    LIST_FOR_EACH(piter, &the_registed_list)
    {
        /* 取得遍历到的对象 */
        pdmn = MemToObj(piter, dmn_t, nlist);
        if (pdmn->taskid == taskid)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 ******************************************************************************
 * @brief   向守护任务注册
 * @param[in]  None
 * @param[out] None
 *
 * @retval     NULL : 失败
 * @retval  !  NULL : 注册ID
 ******************************************************************************
 */
DMN_ID
dmn_register(void)
{
    TASK_ID taskid = taskIdSelf();
    if (TRUE == is_taskname_used(taskid))
    {
        printf("err %s already registered!\n", taskName(taskid));
        return NULL;
    }
    dmn_t *pnew = malloc(sizeof(dmn_t));
    if (NULL == pnew)
    {
        return NULL;
    }
    pnew->taskid = taskid;
    pnew->count = DMN_MAX_CHECK_TIME;
    semTake(the_dmn_sem, WAIT_FOREVER);
    ListAddTail(&pnew->nlist, &the_registed_list);
    semGive(the_dmn_sem);

    return (DMN_ID)pnew;
}

/**
 ******************************************************************************
 * @brief   向守护任务喂狗
 * @param[in]  id   : 喂狗注册时返回的id
 *
 * @retval     OK   : 喂狗成功
 * @retval     ERROR: 喂狗失败
 ******************************************************************************
 */
status_t
dmn_sign(DMN_ID id)
{
    dmn_t *pdmn = (dmn_t *)id;

    semTake(the_dmn_sem, WAIT_FOREVER);

#if 1
    if (FALSE == is_taskname_used(pdmn->taskid))
    {
        semGive(the_dmn_sem);
        return ERROR;
    }
#endif

    if (pdmn->count > 0u)
    {
        pdmn->count = DMN_MAX_CHECK_TIME;
    }
    semGive(the_dmn_sem);

    return OK;
}

/**
 ******************************************************************************
 * @brief     向守护任务注销
 * @param[in]  id   : 喂狗注册时返回的id
 *
 * @retval     None
 ******************************************************************************
 */
status_t
dmn_unregister(DMN_ID id)
{
    dmn_t *pdmn = (dmn_t *)id;

    semTake(the_dmn_sem, WAIT_FOREVER);
    if (FALSE == is_taskname_used(id))
    {
        semGive(the_dmn_sem);
        return ERROR;
    }

    ListDelNode(&pdmn->nlist);
    semGive(the_dmn_sem);
    free(pdmn);

    return OK;
}

/**
 ******************************************************************************
 * @brief   输出dmn信息
 * @param[in]  is_sem   : 是否需要信号量保护
 *
 * @retval     None
 ******************************************************************************
 */
void
dmn_info(bool_e is_sem)
{
    dmn_t *pdmn = NULL;
    struct ListNode *piter = NULL;

    printf("\n  Name\r\t\t\tRemainCounts\n");

    if (is_sem == TRUE)
    {
        semTake(the_dmn_sem, WAIT_FOREVER);
    }
    LIST_FOR_EACH(piter, &the_registed_list)
    {
        /* 取得遍历到的对象 */
        pdmn = MemToObj(piter, dmn_t, nlist);
        printf("  %s\r\t\t\t%d\n", taskName(pdmn->taskid), pdmn->count);
    }
    if (is_sem == TRUE)
    {
        semGive(the_dmn_sem);
    }
}

/**
 ******************************************************************************
 * @brief   向dmn请求复位
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
dmn_reboot(void)
{
    theRebootSignal = TRUE;
}

/**
 ******************************************************************************
 * @brief   dmn任务执行体
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
static void
dmn_loop(void)
{
    uint32_t tick = tickGet();
    dmn_t *pdmn = NULL;
    struct ListNode *piter = NULL;

    theRebootSignal = FALSE;

    FOREVER
    {
        taskDelay(TICKS_PER_SECOND);
        if (_func_feedDogHook != NULL)
        {
            _func_feedDogHook();    /* 喂硬件狗 */
        }
        if ((tickGet() - tick) < (TICKS_PER_SECOND * 10))
        {
            continue;
        }
        tick = tickGet();
        semTake(the_dmn_sem, WAIT_FOREVER);
        LIST_FOR_EACH(piter, &the_registed_list)
        {
            /* 取得遍历到的对象 */
            pdmn = MemToObj(piter, dmn_t, nlist);
            if (pdmn->count >= 0)
            {
                pdmn->count--;
            }
            if (pdmn->count == 0)
            {
                printf("\n  TASK(%s) Sign Out Of Time!\n",
                        taskName(pdmn->taskid));
                dmn_info(FALSE);
                printf("  dmn reboot system...\n");
                if (_func_dmnRestHook != NULL)
                {
                    _func_dmnRestHook();    /* 任务间喂狗异常 */
                }
                if (_func_cpuRestHook != NULL)
                {
                    _func_cpuRestHook();    /* reset CPU */
                }
                bsp_reboot();
            }

        }
        semGive(the_dmn_sem);
    }
    printf("dmn will reboot system after 10s...\n");
    /* wait a moment */
    taskDelay(TICKS_PER_SECOND * 10u);
    if (_func_cpuRestHook != NULL)
    {
        _func_cpuRestHook();    /* reset CPU */
    }
    bsp_reboot();
}

/*--------------------------------dmnLib.c-----------------------------------*/
