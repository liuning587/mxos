/**
 ******************************************************************************
 * @file      logLib.c
 * @brief     ����ʵ����־������ܣ����ڵ�����δ���л��жϷ�������еĴ�ӡ
 * @details   This file including all API functions's implement of logLib.c.
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <types.h>
#include <maths.h>
#include <taskLib.h>
#include <debug.h>
#include <intLib.h>
#include <dmnLib.h>
#include <oscfg.h>

#ifndef INCLUDE_LOGMSG_SUPPORT
# define INCLUDE_LOGMSG_SUPPORT (0u)
#endif

#if (INCLUDE_LOGMSG_SUPPORT == 1u)
/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
#ifndef MAX_MSGS
# define MAX_MSGS                  (10u)    /**< ��Ϣ�����е������Ϣ���� */
#endif

#ifndef MAX_BYTES_IN_A_MSG
# define MAX_BYTES_IN_A_MSG       (200u)    /**< 1��logMsg����ӡ���ֽ��� */
#endif

#ifndef TASK_PRIORITY_LOGMSG
# define TASK_PRIORITY_LOGMSG        (1u)    /**< logMsg��������� */
#endif

#ifndef TASK_STK_SIZE_LOGMSG
# define TASK_STK_SIZE_LOGMSG     (1024u)    /**< logMsg����Ķ�ջ��С */
#endif

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
#pragma pack(push, 1)

/** log msg �ṹ�嶨�� */
typedef struct
{
    int32_t id;     /**< ID of sending task */
    int32_t len;    /**< len of logmsg buf */
    uint8_t buf[MAX_BYTES_IN_A_MSG]; /**< format logmsg buf */
} log_msg_t;

#pragma pack(pop)

/*-----------------------------------------------------------------------------
 Section: Globals Function
 ----------------------------------------------------------------------------*/
extern int print(char **out, const char *format, va_list args );
extern void printstr(const char *pStr, int len);

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static TASK_ID the_logmsg_taskid = NULL;
static MSG_Q_ID the_logmsg_qid = NULL;
static int32_t the_logmsgs_lost = 0;
static int32_t the_logmsgs_outoflen = 0;

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief   logMsg ����ִ����
 * @param[in]  None
 * @param[out] None
 *
 * @retval     None
 ******************************************************************************
 */
static void
loglib_loop(void)
{
    int32_t new_msgs_lost = 0;
    int32_t new_outoflen = 0;
    log_msg_t *pmsg;
    DMN_ID dmnid = dmn_register();
    D_ASSERT(dmnid != NULL);

    while(1)
    {
        dmn_sign(dmnid);
        if (OK == msgQReceive(the_logmsg_qid, 30 * TICKS_PER_SECOND, (void **) &pmsg))
        {
            /* print task ID */
            if (pmsg->id == -1)
            {
                printf("interrupt: ");
            }
            else
            {
                //printf("%#x (%s): ", msg->id, checkName);
            }

            printstr((const char *)pmsg->buf, pmsg->len);
#if 0
            /* ���ù��Ӻ���������Ϣ���ݴ洢 */
            if (_func_logSaveHook != NULL)
            {
                (*_func_logSaveHook)(pmsg->buf, pmsg->len);
            }
#endif

            free(pmsg);
        }

        /* check for any more messages lost */
        if (new_msgs_lost != the_logmsgs_lost)
        {
            printf("loglib: %d log messages lost.\n",
                    the_logmsgs_lost - new_msgs_lost);

            new_msgs_lost = the_logmsgs_lost;
        }
        if (new_outoflen != the_logmsgs_outoflen)
        {
            printf("loglib: %d out of buf length.\n",
                    the_logmsgs_outoflen - new_outoflen);
        }

    }
}

/**
 ******************************************************************************
 * @brief   logmsg �����ʼ��
 * @param[in]  stacksize : ����ջ��С
 *
 * @retval  OK      : ��ʼ���ɹ�
 * @retval  ERROR   : ��ʼ��ʧ��
 ******************************************************************************
 */
status_t
loglib_init(uint32_t stacksize)
{
    if (the_logmsg_taskid != NULL)
    {
        return OK; /* already called */
    }

    stacksize = (stacksize == 0) ? TASK_STK_SIZE_LOGMSG : stacksize;
    the_logmsg_qid = msgQCreate(MAX_MSGS);

    D_ASSERT(the_logmsg_qid != NULL);

    the_logmsg_taskid = taskSpawn((const signed char * const ) "LogMsg",
            TASK_PRIORITY_LOGMSG, stacksize, (OSFUNCPTR) loglib_loop, 0);

    D_ASSERT(the_logmsg_taskid != NULL);
    return (OK);
}

/**
 ******************************************************************************
 * @brief   ��־���
 * @param[in]  fmt  : ��־��Ϣ��printf��ͬ
 *
 * @retval     ERROR: ������־��Ϣʧ��
 * @retval     OK   : ������־��Ϣ�ɹ�
 ******************************************************************************
 */
status_t
logmsg(const char *fmt, ...)
{
    log_msg_t *msg;

    if (the_logmsg_taskid == NULL)
    {
        va_list args;

        va_start( args, fmt );
        print( 0, fmt, args );
        return OK;
    }

    if (strlen(fmt) > (MAX_BYTES_IN_A_MSG - 1))
    {
        return ERROR;
    }
    msg = malloc(sizeof(log_msg_t));

    /* �ж��Ƿ����ж��е��� */
    msg->id = (intContext() == TRUE) ? -1 : (int32_t)taskIdSelf();

    char *out = (char *)msg->buf;

    va_list args;
    va_start( args, fmt );
    msg->len = print( &out, fmt, args );
    va_end(args);

    if (msg->len > (MAX_BYTES_IN_A_MSG - 1))
    {
        the_logmsgs_outoflen++; /* shit out of buf length */
    }
    msg->buf[msg->len] = '\0';


    if (msgQSend(the_logmsg_qid, (void *) msg) != OK)
    {
        ++the_logmsgs_lost;
        return ERROR;
    }

    return OK;
}


/**
 ******************************************************************************
 * @brief   ��loglib�����ָ���Ļ���������
 * @param[in]  pbuf : ��Ҫ����Ļ�����ָ��
 * @param[in]  len  : ��Ҫ����Ļ��������ݳ���
 *
 * @retval     ERROR: ������־��Ϣʧ��
 * @retval     OK   : ������־��Ϣ�ɹ�
 ******************************************************************************
 */
status_t
logbuf(const uint8_t *pbuf, uint32_t len)
{
    log_msg_t *msg;

    if (the_logmsg_taskid == NULL)
    {
        printbuffer("", pbuf, len);
        return OK;
    }

    msg = malloc(sizeof(log_msg_t));

    /* �ж��Ƿ����ж��е��� */
    msg->id = (intContext() == TRUE) ? -1 : (int32_t)taskIdSelf();

    int32_t i;
    for (i = 0; i < MIN(len , ((MAX_BYTES_IN_A_MSG - 2) / 3)); i++)
    {
        (void)sprintf((char *)msg->buf + (i*3) , "%02x ", *pbuf);
        msg->len += 3;
        pbuf++;
    }
    msg->buf[i*3] = '\r';
    msg->buf[(i*3)+1] = '\n';
    msg->len += 2;
    if (msg->len > (MAX_BYTES_IN_A_MSG - 1))
    {
        the_logmsgs_outoflen++; /* shit out of buf length */
    }
    msg->buf[msg->len] = '\0';


    if (msgQSend(the_logmsg_qid, (void *) msg) != OK)
    {
        ++the_logmsgs_lost;
        return ERROR;
    }

    return OK;

}

#else

extern int print(char **out, const char *format, va_list args );
extern void printbuffer(char_t* format, const uint8_t* buffer, int32_t len);
/**
 ******************************************************************************
 * @brief   logmsg �����ʼ��
 * @param[in]  None
 * @param[out] None
 * @retval  OK      : ��ʼ���ɹ�
 * @retval  ERROR   : ��ʼ��ʧ��
 ******************************************************************************
 */
status_t
loglib_init(uint32_t stacksize)
{
    (void)stacksize;
    return OK;
}

/**
 ******************************************************************************
 * @brief   ��־���
 * @param[in]  fmt  : ��־��Ϣ��printf��ͬ
 *
 * @retval     ERROR: ������־��Ϣʧ��
 * @retval     OK   : ������־��Ϣ�ɹ�
 ******************************************************************************
 */
status_t
logmsg(const char *fmt, ...)
{
    va_list args;

    va_start( args, fmt );
    print( 0, fmt, args );
    return OK;
}

/**
 ******************************************************************************
 * @brief   ��loglib�����ָ���Ļ���������
 * @param[in]  pbuf : ��Ҫ����Ļ�����ָ��
 * @param[in]  len  : ��Ҫ����Ļ��������ݳ���
 *
 * @retval     ERROR: ������־��Ϣʧ��
 * @retval     OK   : ������־��Ϣ�ɹ�
 ******************************************************************************
 */
status_t
logbuf(const uint8_t *pbuf, uint32_t len)
{
    printbuffer("", pbuf, len);
    return OK;
}

#endif
/*---------------------------------logLib.c----------------------------------*/
