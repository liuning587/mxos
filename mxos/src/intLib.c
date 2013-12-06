/**
 ******************************************************************************
 * @file      intLib.c
 * @brief     中断函数库的实现.
 * @details
 *          这个函数库文件提供与硬件平台相关的中断接口函数，用以管理
 *          和链接系统的中断处理。任何C 语言程序能够通过调用函数
 *          intConnect()将指定的函数挂接到指定的中断上。
 * @copyright
 *
 ******************************************************************************
 */

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <stdlib.h>
#include <oscfg.h>
#if CORE_TYPE == CORE_CM3
#include <core_cm3.h>
#elif  CORE_TYPE == CORE_CM4
#include <core_cm4.h>
#endif

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
#pragma pack(push, 1)

typedef struct int_rtn
{
    VOIDFUNCPTR routine; /**< interrupt handler */
    int32_t parameter;   /**< parameter of the handler */
} INT_RTN;

#pragma pack(pop)

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* 中断实现函数入口表 */
INT_RTN *intRtnTbl = NULL;
/* Counter to tell if we are at interrupt (non-task) level. */
int32_t intCnt = 0;

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Function Prototypes
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief      根据传入的中断号找到并调用相应的ISR.
 * @param[in]  irq_num: 中断号
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
void
intHandler(uint32_t irq_num)
{
    intCnt++;
    INT_RTN int_rtn = intRtnTbl[irq_num - 16];

    if (0 != int_rtn.parameter)
        int_rtn.routine(int_rtn.parameter);
    else
        int_rtn.routine();
    if(intCnt > 0) intCnt--;
}

/* dummy interrupt handler */
extern void
dummy(void);

/** @defgroup group_intLib intLib
 *  @brief  中断处理函数库
 *
 *          这个函数库文件提供与硬件平台相关的中断接口函数，用以管理
 *          和链接系统的中断处理。任何C 语言程序能够通过调用函数
 *          intConnect()将指定的函数挂接到指定的中断上。
 *          任务能够通过调用intLock()和intUnlock()来打开和关闭中断。
 *
 * @{
 */

/**
 ******************************************************************************
 * @brief      将函数与中断号关联.
 * @param[in]    irq_num   : 中断号(16 ~ MAX_INT_COUNT)
 * @param[in]    routine   : 中断服务
 * @param[in]    parameter : 中断参数
 *
 * @retval  OK   : 成功
 * @retval  ERROR: 失败
 ******************************************************************************
 */
extern status_t
intConnect(uint32_t irq_num, VOIDFUNCPTR routine, uint32_t parameter)
{
    if (intRtnTbl == NULL)
    {
        return ERROR;
    }
    if ((irq_num < 16) || (irq_num > MAX_INT_COUNT))
    {
        return ERROR;
    }
    intRtnTbl[irq_num - 16].routine = routine;
    intRtnTbl[irq_num - 16].parameter = parameter;

    return OK;
}


/**
 ******************************************************************************
 * @brief      注销中断ROUTINE.
 * @param[in]   irq_num   : 中断号(16 ~ MAX_INT_COUNT)
 *
 * @retval  OK   : 成功
 * @retval  ERROR: 失败
 ******************************************************************************
 */
extern status_t
intDisconnect(uint32_t irq_num)
{
    if (intRtnTbl == NULL)
    {
        return ERROR;
    }
    if ((irq_num < 16) || (irq_num > MAX_INT_COUNT))
    {
        return ERROR;
    }

    intRtnTbl[irq_num - 16].routine = dummy;
    intRtnTbl[irq_num - 16].parameter = 0;

    return OK;
}

/**
 ******************************************************************************
 * @brief      设置中断优先级.
 * @param[in]   irq_num   : 中断号(16 ~ MAX_INT_COUNT)
 * @param[in]   prio      : 中断优先级
 *
 * @retval  OK   : 成功
 * @retval  ERROR: 失败
 ******************************************************************************
 */
extern status_t
intPrioSet(uint32_t irq_num, uint8_t prio)
{
    if ((irq_num < 16) || (irq_num > MAX_INT_COUNT))
    {
        return ERROR;
    }

    NVIC->IP[irq_num] = ((prio << (8 - __NVIC_PRIO_BITS)) & 0xff);

    return OK;
}

/**
 ******************************************************************************
 * @brief      使能指定的中断.
 * @param[in]    irq_num   : 中断号(16 ~ MAX_INT_COUNT)
 * @retval
 *             status_t: 成功-OK, 失败-ERROR
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
extern status_t
intEnable(uint32_t irq_num)
{
    if ((irq_num < 16) || (irq_num > MAX_INT_COUNT))
    {
        return ERROR;
    }

    uint32_t nirq = irq_num - 16;
    NVIC->ISER[(nirq >> 5)] = (1 << (nirq & 0x1F)); /* enable interrupt */

    return OK;
}

/**
 ******************************************************************************
 * @brief   使能指定的中断
 * @param[in]  irq_num   : 中断号(16 ~ MAX_INT_COUNT)
 *
 * @retval  OK   : 成功
 * @retval  ERROR: 失败
 ******************************************************************************
 */
extern status_t
intDisable(uint32_t irq_num)
{
    if ((irq_num < 16) || (irq_num > MAX_INT_COUNT ))
    {
        return ERROR;
    }

    uint32_t nirq = irq_num - 16;
    NVIC->ICER[(nirq >> 5)] = (1 << (nirq & 0x1F)); /* disable interrupt */

    return OK;
}

/**
 ******************************************************************************
 * @brief   是否为中断状态
 * @param[in]  None
 *
 * @retval  TRUE : 是
 * @retval  FALSE: 否
 ******************************************************************************
 */
bool_e
intContext (void)
{
    return (intCnt > 0) ? TRUE : FALSE;
}

/**
 ******************************************************************************
 * @brief      中断函数初始化.
 * @param[in]  None
 *
 * @retval     None
 ******************************************************************************
 */
extern status_t
intLibInit(void)
{
    uint32_t irq_num;

    if (intRtnTbl == NULL)
    {
        intRtnTbl = malloc(sizeof(INT_RTN) * (MAX_INT_COUNT - 15));
        if (intRtnTbl == NULL)
        {
            return ERROR;
        }
        // 初始化中断表
        for (irq_num = 16; irq_num < MAX_INT_COUNT; irq_num++)
        {
            intConnect(irq_num, dummy, 0);
        }

        // 使能BusFault、memFault、usgFault 可考虑在excLib初始化时使能
        SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk
                | SCB_SHCSR_USGFAULTENA_Msk
                | SCB_SHCSR_MEMFAULTENA_Msk);
        SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;  /* 使能除数为0异常 */

    }
    return OK;
}


/**
 ******************************************************************************
 * @brief      关中断
 * @param[in]  None
 *
 * @retval     None
 ******************************************************************************
 */
void
intLock(void)
{
    __asm(
            "CPSID   I\n"
            "CPSID   I\n"
         );
    intCnt++;
}

/**
 ******************************************************************************
 * @brief      开中断.
 * @param[in]  None
 *
 * @retval     None
 ******************************************************************************
 */
void
intUnlock(void)
{
    intCnt--;
    __asm(
            "CPSIE   I\n"
            "BX      LR\n"
         );
}
/**
* @}
*/

/*--------------------------------intLib.c-----------------------------------*/
