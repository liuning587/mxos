/**
 ******************************************************************************
 * @file      intLib.c
 * @brief     �жϺ������ʵ��.
 * @details
 *          ����������ļ��ṩ��Ӳ��ƽ̨��ص��жϽӿں��������Թ���
 *          ������ϵͳ���жϴ����κ�C ���Գ����ܹ�ͨ�����ú���
 *          intConnect()��ָ���ĺ����ҽӵ�ָ�����ж��ϡ�
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
/* �ж�ʵ�ֺ�����ڱ� */
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
 * @brief      ���ݴ�����жϺ��ҵ���������Ӧ��ISR.
 * @param[in]  irq_num: �жϺ�
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
 *  @brief  �жϴ�������
 *
 *          ����������ļ��ṩ��Ӳ��ƽ̨��ص��жϽӿں��������Թ���
 *          ������ϵͳ���жϴ����κ�C ���Գ����ܹ�ͨ�����ú���
 *          intConnect()��ָ���ĺ����ҽӵ�ָ�����ж��ϡ�
 *          �����ܹ�ͨ������intLock()��intUnlock()���򿪺͹ر��жϡ�
 *
 * @{
 */

/**
 ******************************************************************************
 * @brief      ���������жϺŹ���.
 * @param[in]    irq_num   : �жϺ�(16 ~ MAX_INT_COUNT)
 * @param[in]    routine   : �жϷ���
 * @param[in]    parameter : �жϲ���
 *
 * @retval  OK   : �ɹ�
 * @retval  ERROR: ʧ��
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
 * @brief      ע���ж�ROUTINE.
 * @param[in]   irq_num   : �жϺ�(16 ~ MAX_INT_COUNT)
 *
 * @retval  OK   : �ɹ�
 * @retval  ERROR: ʧ��
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
 * @brief      �����ж����ȼ�.
 * @param[in]   irq_num   : �жϺ�(16 ~ MAX_INT_COUNT)
 * @param[in]   prio      : �ж����ȼ�
 *
 * @retval  OK   : �ɹ�
 * @retval  ERROR: ʧ��
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
 * @brief      ʹ��ָ�����ж�.
 * @param[in]    irq_num   : �жϺ�(16 ~ MAX_INT_COUNT)
 * @retval
 *             status_t: �ɹ�-OK, ʧ��-ERROR
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
 * @brief   ʹ��ָ�����ж�
 * @param[in]  irq_num   : �жϺ�(16 ~ MAX_INT_COUNT)
 *
 * @retval  OK   : �ɹ�
 * @retval  ERROR: ʧ��
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
 * @brief   �Ƿ�Ϊ�ж�״̬
 * @param[in]  None
 *
 * @retval  TRUE : ��
 * @retval  FALSE: ��
 ******************************************************************************
 */
bool_e
intContext (void)
{
    return (intCnt > 0) ? TRUE : FALSE;
}

/**
 ******************************************************************************
 * @brief      �жϺ�����ʼ��.
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
        // ��ʼ���жϱ�
        for (irq_num = 16; irq_num < MAX_INT_COUNT; irq_num++)
        {
            intConnect(irq_num, dummy, 0);
        }

        // ʹ��BusFault��memFault��usgFault �ɿ�����excLib��ʼ��ʱʹ��
        SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk
                | SCB_SHCSR_USGFAULTENA_Msk
                | SCB_SHCSR_MEMFAULTENA_Msk);
        SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;  /* ʹ�ܳ���Ϊ0�쳣 */

    }
    return OK;
}


/**
 ******************************************************************************
 * @brief      ���ж�
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
 * @brief      ���ж�.
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
