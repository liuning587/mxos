/**
 ******************************************************************************
 * @file      oshook.c
 * @brief     os外提给提供的钩子接口库.
 * @details   This file including all API functions's  implement of mxhook.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <oscfg.h>
#if CORE_TYPE == CORE_CM3
#include <core_cm3.h>
#elif  CORE_TYPE == CORE_CM4
#include <core_cm4.h>
#endif

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
/**
 *  @brief 喂硬件狗函数钩子
 */
FUNCPTR     _func_feedDogHook = NULL;

/**
 *  @brief 芯片复位函数钩子
 */
FUNCPTR     _func_cpuRestHook = NULL;

/**
 *  @brief 任务超时未喂狗异常时函数钩子
 */
FUNCPTR     _func_dmnRestHook = NULL;

/**
 *  @brief 内核任务堆栈溢出发生时，调用的钩子函数，
 *  可用于在文件中保存现场信息。
 *  _func_evtLogOverStackHook(taskId,pcTaskName);
 */
VOIDFUNCPTR _func_evtLogOverStackHook;

int32_t _the_console_fd = -1;

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
 * @brief   0. 默认关闭看门狗
 * @param[in]  None
 *
 * @retrun    None
 ******************************************************************************
 */
void
_default_bsp_close_watchdog(void)
{
    //空函数
}

/**
 ******************************************************************************
 * @brief   1. 默认uart底层输出字节
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_putchar(char_t c)
{

}

/**
 ******************************************************************************
 * @brief   2. 默认uart底层读取字节,查询模式，若无数据返回0
 * @param[in]  None
 *
 * @retval    0 :
 ******************************************************************************
 */
int32_t
_default_bsp_getchar(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief   3. 默认bsp底层实现复位函数
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_reboot(void)
{
    __asm volatile ("isb");
    SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
                   (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
                   SCB_AIRCR_SYSRESETREQ_Msk);
    __asm volatile ("dsb");
    while(1);
}

/**
 ******************************************************************************
 * @brief  4. 默认启动定时器
 * @param[in]  None
 *
 * @retval    None
 ******************************************************************************
 */
void
_default_bsp_timer_start(void)
{

}

/**
 ******************************************************************************
 * @brief  4. 默认获取计数器值
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_timer_get(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief  5. 默认获取MCU主频
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_mcu_clk(void)
{
    return 0;
}

/**
 ******************************************************************************
 * @brief  6. 默认获取MCU中断数量
 * @param[in]  None
 *
 * @retval    0
 ******************************************************************************
 */
uint32_t
_default_bsp_get_max_int_count(void)
{
    return 103;
}

/**
  *@brief MXOS默认钩子函数
  */
#pragma weak bsp_close_watchdog     = _default_bsp_close_watchdog
#pragma weak bsp_putchar            = _default_bsp_putchar
#pragma weak bsp_getchar            = _default_bsp_getchar
#pragma weak bsp_reboot             = _default_bsp_reboot
#pragma weak bsp_timer_start        = _default_bsp_timer_start
#pragma weak bsp_timer_get          = _default_bsp_timer_get
#pragma weak bsp_get_mcu_clk        = _default_bsp_mcu_clk
#pragma weak bsp_get_max_int_count  = _default_bsp_get_max_int_count


/*--------------------------------oshook.c-----------------------------------*/
