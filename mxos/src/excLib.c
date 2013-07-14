/**
 ******************************************************************************
 * @file      excLib.c
 * @brief     系统异常处理模块.
 * @details   发生异常时，输出寄存器的值 便于调试。
 *
 * @copyright
 *
 ******************************************************************************
 */
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <taskLib.h>
#include <FreeRTOS.h>
#include <task.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#define MEM_FALUT_FSR   (0xE000ED28)
#define BUS_FALUT_FSR   (0xE000ED29)
#define USGE_FALUT_FSR  (0xE000ED2A)
#define HARD_FALUT_FSR  (0xE000ED2C)

#define MEM_FALUT_FAR   (0xE000ED34)
#define BUS_FALUT_FAR   (0xE000ED38)

/*-----------------------------------------------------------------------------
 Section: Private Type Definitions
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
extern int32_t intCnt;

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief      异常中断入口
 * @param[in]  None
 *
 * @return     None
 ******************************************************************************
 */
void
excEnterCommon(void)
{
    __asm volatile (
        ".extern  pxCurrentTCB          \n"
        ".extern  excExcHandle          \n"
        ".extern  vTaskSwitchContext    \n"
        "   CPSID   F                   \n"
        "   CPSID   F                   \n" /* Disable 异常中断 */
        "   MRS     R1, IPSR            \n" /* 将异常号存入R1 */
        "   TST     LR, #0x4            \n" /* 测试EXC_RETURN的bit2 */
        "   ITE     EQ                  \n" /* 如果为0,下面包括2条指令 (P76) */
        "   MRSEQ   R0, MSP             \n" /* 如果EXC_RETURN的bit2==0 则使用MSP */
        "   MRSNE   R0, PSP             \n" /* 否则使用PSP */
        "   SUBS    R0, R0, #0x20       \n"
        "   STM     R0, {R4-R11}        \n" /* 保存R4-R11 */
        "   BL      excExcHandle        \n"
        "   BL      vTaskSwitchContext  \n"
        "   LDR     R2, =pxCurrentTCB   \n"
        "   LDR     R3, [R2]            \n"
        "   LDR     R0, [R3]            \n" /* R0 is new process SP; SP = OSTCBHighRdy->OSTCBStkPtr;*/
        "   LDM     R0, {R4-R11}        \n" /* Restore r4-11 from new process stack*/
        "   ADDS    R0, R0, #0x20       \n"
        "   MSR     PSP, R0             \n" /* Load PSP with new process SP*/
        "   LDR     LR, =0xFFFFFFFD     \n"
        "   CPSIE   F                   \n"
        "   BX      LR                  \n" /* Exception return will restore remaining context*/
        );
}

/**
 ******************************************************************************
 * @brief    interrupt level handling of exceptions
 * @param[in]  pregs    : 发生异常时寄存器值
 * @param[in]  excno    : 异常中断号
 *
 * @return None
 ******************************************************************************
 */
void
excExcHandle(void* pregs,
        uint32_t excno)
{
    uint32_t regs[16];
    uint32_t hfsr = *(uint32_t*)HARD_FALUT_FSR;
    uint8_t mfsr = *(uint8_t*)MEM_FALUT_FSR;
    uint8_t bfsr = *(uint8_t*)BUS_FALUT_FSR;
    uint16_t ufsr = *(uint16_t*)USGE_FALUT_FSR;

    memcpy(regs, pregs, 64);
    uint32_t sp = (uint32_t)pregs;

    intCnt++;
    printf("\r\n");
    switch (excno)
    {
        case 3:
            /* 输出hfsr信息 */
            printf("=====硬fault=====\r\n");
            if (hfsr & 0x00000002) printf("取向量时发生硬fault\r\n");
            if (hfsr & 0x40000000) printf("上访产生硬fault\r\n");
            if (hfsr & 0x80000000) printf("调试产生硬fault\r\n");
            break;
        case 4:
            /* 输出mfsr信息 */
            printf("=====存储器fault=====\r\n");
            if (mfsr & 0x01) printf("取指令访问违规\r\n");
            if (mfsr & 0x02) printf("数据访问违规\r\n");
            if (mfsr & 0x08) printf("出栈错误\r\n");
            if (mfsr & 0x10) printf("入栈错误\r\n");
            if (mfsr & 0x80) printf("由0x%08x处的指令引起\r\n", *(uint32_t*)(MEM_FALUT_FAR));
            break;
        case 5:
            /* 输出bsfr信息 */
            printf("=====总线fault=====\r\n");
            if (bfsr & 0x01) printf("取指令访问违规\r\n");
            if (bfsr & 0x02) printf("精确的数据访问违规\r\n");
            if (bfsr & 0x04) printf("不精确的数据访问违规\r\n");
            if (bfsr & 0x08) printf("出栈错误\r\n");
            if (bfsr & 0x10) printf("入栈错误\r\n");
            if (bfsr & 0x80) printf("由0x%08x处的指令引起\r\n", *(uint32_t*)(BUS_FALUT_FAR));
            else printf("无法追踪违规指令\r\n");
            break;
        case 6:
            /* 输出ufsr信息 */
            printf("=====用法fault=====\r\n");
            if (ufsr & 0x0001) printf("指令解码错误\r\n");
            if (ufsr & 0x0002) printf("程序视图切入ARM状态\r\n");
            if (ufsr & 0x0008) printf("异常返回时视图非法的加载EXC_RETURN到PC\r\n");
            if (ufsr & 0x0010) printf("程序视图执行协处理器相关指令\r\n");
            if (ufsr & 0x0100) printf("未对齐\r\n");
            if (ufsr & 0x0200) printf("除数为零\r\n");
            break;
        default:
            printf("=====无效的错误中断:%d=====\r\n", excno);
            intCnt--;
            return;
    }

    printf("exception occur regs : \r\n");
    printf("    r0  =%08x    r1  =%08x    r2  =%08x    r3  =%08x \r\n",
            regs[8], regs[9], regs[10], regs[11]);
    printf("    r4  =%08x    r5  =%08x    r6  =%08x    r7  =%08x \r\n",
            regs[0], regs[1], regs[2], regs[3]);
    printf("    r8  =%08x    r9  =%08x    r10 =%08x    r11 =%08x \r\n",
            regs[4], regs[5], regs[6], regs[7]);
    printf("    sp  =%08x    lr  =%08x    pc  =%08x    xpsr=%08x \r\n",
            sp, regs[13], regs[14], regs[15]);

    TASK_ID taskid = taskIdSelf();
    printf("exception occur in task: 0x%x \n\r", (uint32_t)taskid);
    printf("exception task name: %-16s \n\r", taskName(taskid));
    taskSuspend(taskid);    /* 挂起产生异常的任务 */

    time_t curtm = time(NULL);
    struct tm daytime;
    (void)localtime_r(&curtm, &daytime);
    printf("exception occur time: %04d-%02d-%02d %02d:%02d:%02d\r\n",
            daytime.tm_year + 1900, daytime.tm_mon + 1, daytime.tm_mday,
            daytime.tm_hour, daytime.tm_min, daytime.tm_sec);
    /* 若此时调度器关闭，则打开调度器 */
    if (xTaskGetSchedulerState() == taskSCHEDULER_SUSPENDED)
    {
        xTaskResumeAll();
    }
    intCnt--;
}

/*----------------------------End of excLib.c--------------------------------*/
