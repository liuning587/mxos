/**
 ******************************************************************************
 * @file       oscfg.h
 * @brief      系统配置文件.
 * @details    This file including all API functions's declare of oscfg.h.
 *
 ******************************************************************************
 */
 

#ifndef __OSCFG_H__
#define __OSCFG_H__
/*-----------------------------------------------------------------------------
Section: Includes
-----------------------------------------------------------------------------*/
/* None */

/*-----------------------------------------------------------------------------
Section: Macro Definitions
-----------------------------------------------------------------------------*/
#define OS_RELEASE  "0.1.2-beta1"

#define CORE_CM3    1
#define CORE_CM4    2

#define CORE_TYPE   CORE_CM3    /**< 核心配置 */
#define SUPPORT_FPU 0           /**< 是否支持FPU */

#define MCU_CLOCK      bsp_get_mcu_clk()        /**< 获取MCU的主频 */
#define MAX_INT_COUNT  bsp_get_max_int_count()  /**< 获取MCU最大中断数量 */

#define TICKS_PER_SECOND          (100u)    /**< 定义时钟周期终端的频率 */
#define MAX_TASK_PRIORITIES         (8u)    /**< 定义操作系统任务的最高优先级 */

/* shell任务配置 */
#define TASK_PRIORITY_SHELL         (2u)    /**< shell任务优先级 */
#define TASK_STK_SIZE_SHELL      (2048u)    /**< shell任务堆栈 */

/* dmn任务配置 */
#define TASK_PRIORITY_DMN           (1u)    /**< DMN任务优先级 */
#define TASK_STK_SIZE_DMN         (512u)    /**< DMN任务堆栈 */
#define DMN_MAX_CHECK_TIME          (6u)    /**< 默认喂狗超时时间（6*10秒） */

/* logMsg任务配置 */
#define INCLUDE_LOGMSG_SUPPORT      (1u)    /**< 支持logMsg */
#define MAX_MSGS                   (10u)    /**< 消息队列中的最大消息个数 */
#define MAX_BYTES_IN_A_MSG        (200u)    /**< 1个logMsg最大打印的字节数 */
#define TASK_PRIORITY_LOGMSG        (1u)    /**< logMsg任务的优先 */
#define TASK_STK_SIZE_LOGMSG     (1024u)    /**< logMsg任务的堆栈大小 */

#if (CORE_TYPE == CORE_CM4) && (SUPPORT_FPU == 1)
# define __FPU_PRESENT        1
#endif

#endif /* __OSCFG_H__ */
/*------------------------------End of oscfg.h-------------------------------*/
