/**
 ******************************************************************************
 * @file       shell.c
 * @brief      本文实现了shell功能.
 * @details    This file including all API functions's implement of shell.
 * @copyright
 *
 ******************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include <shell.h>
#include <debug.h>
#include <taskLib.h>
#include <dmnLib.h>
#include <devLib.h>
#include <oscfg.h>

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
#ifndef TASK_PRIORITY_SHELL
# define TASK_PRIORITY_SHELL        (1u)    /**< shell任务优先级 */
#endif

#ifndef TASK_STK_SIZE_SHELL
# define TASK_STK_SIZE_SHELL     (1024u)    /**< shell任务堆栈 */
#endif

#ifndef CFG_MAXARGS
# define CFG_MAXARGS                (8u)    /**< 命令行最大参数数量 */
#endif

#ifndef CFG_CBSIZE
# define CFG_CBSIZE                (50u)    /**< 命令行字节数 */
#endif
#define SHELL_PRINTF        printf          /**< 字符串输出 */

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
extern cmd_tbl_t  __shell_cmd_start;
extern cmd_tbl_t  __shell_cmd_end;
extern int32_t _the_console_fd;

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static uint8_t console_buffer[CFG_CBSIZE]; /* console I/O buffer   */
static const char_t erase_seq[] = "\b \b";
static const char_t const *prompt = "~#";
static cmd_tbl_t *pmatch_cmd = NULL;
static TASK_ID shellTaskId = 0;
static DMN_ID the_dmnid = NULL;

/*-----------------------------------------------------------------------------
 Section: static Function Prototypes
 ----------------------------------------------------------------------------*/
static bool_e readline(void);
static cmd_tbl_t *find_cmd(const char *cmd);
static int parse_line(char_t *line, char_t *argv[]);
static int run_command(char_t *cmd);

/*-----------------------------------------------------------------------------
 Section: Function Definitions
 ----------------------------------------------------------------------------*/
/**
 ******************************************************************************
 * @brief      find command table entry for a command
 * @param[in]  const char_t *cmd
 * @param[out] None
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static cmd_tbl_t *
find_cmd(const char_t *cmd)
{
    cmd_tbl_t *cmdtp;
    cmd_tbl_t *cmdtp_temp = &__shell_cmd_start; /*Init value */
    const char_t *p;
    int32_t len;
    int32_t n_found = 0;

    /*
     * Some commands allow length modifiers (like "cp.b");
     * compare command name only until first dot.
     */
    len = ((p = strchr(cmd, '.')) == NULL) ? strlen(cmd) : (p - cmd);

    for (cmdtp = &__shell_cmd_start; cmdtp != &__shell_cmd_end; cmdtp++)
    {
        if (strncmp(cmd, (char *) cmdtp->name, len) == 0)
        {
            if (len == strlen(cmdtp->name))
            {
                return cmdtp; /* full match */
            }

            cmdtp_temp = cmdtp; /* abbreviated command ? */
            n_found++;
        }
    }
    if (n_found == 1)
    {
        /* exactly one match */
        return cmdtp_temp;
    }

    return NULL; /* not found or ambiguous command */
}

/**
 ******************************************************************************
 * @brief      匹配命令
 * @param[in]  char_t *buf
 * @param[in]  int32_t len
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static int32_t
match_cmd(char_t *buf, int32_t len)
{
    cmd_tbl_t *pcmd = NULL;
    int32_t i = 0;
    uint8_t flag = 0u;

    for (i = 0; i < len; i++)
    {
        if (buf[i] == ' ')
        {
            return -1;
        }
    }

    (pmatch_cmd != NULL) ? (pcmd = pmatch_cmd + 1)
            : (pmatch_cmd = pcmd = &__shell_cmd_start);

    do {
        if (pcmd >= &__shell_cmd_end)
        {
            pcmd = &__shell_cmd_start;
            flag = 1u;
        }
        if (strncmp(pcmd->name, buf, len) == 0)
        {
            pmatch_cmd = pcmd;
            strncpy(buf, pcmd->name, CFG_CBSIZE);
            strncat(buf, " ", CFG_CBSIZE);
            return strlen(buf);
        }
        if (flag == 1u)
        {
            flag = 0u;
        }
        else
        {
            pcmd++;
        }
    } while (pcmd != pmatch_cmd);

    return -2;
}

/**
 ******************************************************************************
 * @brief      读取一行命令
 * @param[in]  None
 * @param[out] None
 * @retval     TRUE     : 成功
 * @retval     FALSE    : 失败
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static bool_e
readline(void)
{
    uint8_t c;
    uint8_t *p = console_buffer;
    int32_t n = 0;
    int32_t n_pos = 0;
    int32_t ret;

    // 打印提示符
    SHELL_PRINTF(prompt);
    //fflush(stdout);
    while (TRUE)
    {
        dmn_sign(the_dmnid);
        taskDelay(1);
        // 检测输入
        if (_the_console_fd <= 0)
        {
            if ((c = bsp_getchar()) == 0)
                continue;
        }
        else
        {
            if (dev_read(_the_console_fd, &c, 1) != 1)
                continue;
        }
        // 处理输入字符
        switch (c)
        {
        case '\r': // 回车
        case '\n': // 换行，命令结束输入
            p[n] = '\0';
             SHELL_PRINTF ("\r\n");
            pmatch_cmd = NULL;
            return TRUE;

        case 0x03: // Ctrl + C
            // 清空缓冲区
            console_buffer[0] = '\0';
            SHELL_PRINTF("\r\n");
            return FALSE;

        case 0x08: //退格键
            if (0 >= n)
            {
                continue;
            }
            // 删除最后一个字符
            p[n] = '\0';
            //p--;
            n--;
            n_pos = n;
            pmatch_cmd = NULL;
            SHELL_PRINTF(erase_seq);
            continue;

        case 0x09: // TAB
            ret = match_cmd((char_t *)console_buffer, n_pos);
            n = ret > 0 ? ret : n;
            SHELL_PRINTF("\r");
            SHELL_PRINTF(prompt);
            SHELL_PRINTF("%s", console_buffer);
            SHELL_PRINTF("\033[0K"); // Clear from cursor to end of line
            continue;

        case 0x7F: // DEL
            continue;

        default: // 可接受的字符
            // Buffer full
            if (n >= CFG_CBSIZE - 2)
            {
                SHELL_PRINTF("\a");//超出命令最大字节数响铃
                continue;
            }
            SHELL_PRINTF("%c", c);
            // 保存到缓冲区
            p[n] = c;
            n++;
            n_pos = n;
            break;
        }
    }
    return TRUE;
}

/**
 ******************************************************************************
 * @brief      解析命令行
 * @param[in]  char_t *line     : 输入命令行
 * @param[out] char_t *argv[]   : 输出命令行参数
 *
 * @retval 1  - command executed, repeatable
 * @retval 0  - command executed but not repeatable, interrupted commands are
 *              always considered not repeatable
 * @retval -1 - not executed (unrecognized, bootd recursion or too many args)
 *              (If cmd is NULL or "" or longer than CFG_CBSIZE-1 it is
 *              considered unrecognized)
 * @note
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 *
 ******************************************************************************
 */
static int32_t
parse_line(char_t *line, char_t *argv[])
{
    int32_t nargs = 0;

    while (nargs < CFG_MAXARGS)
    {
        /* skip any white space */
        while ((*line == ' ') || (*line == '\t'))
        {
            ++line;
        }

        if (*line == '\0')
        {
            /* end of line, no more args    */
            argv[nargs] = NULL;
            return (nargs);
        }

        argv[nargs++] = line; /* begin of argument string */

        /* find end of string */
        while (*line && (*line != ' ') && (*line != '\t'))
        {
            ++line;
        }

        if (*line == '\0')
        {
            /* end of line, no more args    */
            argv[nargs] = NULL;

            return (nargs);
        }

        *line++ = '\0'; /* terminate current arg     */
    }

    SHELL_PRINTF("** Too many args (max. %d) **\n", CFG_MAXARGS);
    return (nargs);
}

/**
 ******************************************************************************
 * @brief      执行命令
 * @param[in]  char_t *cmd
 * @param[out] None
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static int32_t
run_command(char_t *cmd)
{
    cmd_tbl_t *cmdtp;
    char_t cmdbuf[CFG_CBSIZE]; /* working copy of cmd      */

    char_t *token; /* start of token in cmdbuf */
    char_t *sep; /* end of token (separator) in cmdbuf */
    char_t *str = cmdbuf;
    char_t *argv[CFG_MAXARGS + 1]; /* NULL terminated  */
    int32_t argc, inquotes;
    int32_t rc = 0;
    int32_t len = 0;

    len = 0;
    len = strlen(cmd);
    if (len >= CFG_CBSIZE)
    {
        SHELL_PRINTF("Command too long!\n\r");
        return -1;
    }
    memset(cmdbuf, 0, CFG_CBSIZE);
    memset(argv, 0, CFG_MAXARGS);
    strcpy(&cmdbuf[0], cmd);

    /* Process separators and check for invalid
     * repeatable commands
     */
    while (*str)
    {
        /*
         * Find separator, or string end
         * Allow simple escape of ';' by writing "\;"
         */
        for (inquotes = 0, sep = str; *sep; sep++)
        {
            if ((*sep == '\'') && (*(sep - 1) != '\\'))
            {
                inquotes = !inquotes;
            }

            if (!inquotes && (*sep == ';') && /* separator        */
                    (sep != str) && /* past string start    */
                    (*(sep - 1) != '\\')) /* and NOT escaped  */
            {
                break;
            }
        }

        /*
         * Limit the token to data between separators
         */
        token = str;
        if (*sep)
        {
            str = sep + 1; /* start of command for next pass */
            *sep = '\0';
        }
        else
        {
            str = sep; /* no more commands for next pass */
        }

        /* Extract arguments */
        if ((argc = parse_line(token, argv)) == 0)
        {
            rc = -1; /* no command at all */
            continue;
        }

        /* Look up command in command table */
        if ((cmdtp = find_cmd((char*) argv[0])) == NULL)
        {
            SHELL_PRINTF("Unknown command '%s' - try 'help'\r\n", argv[0]);
            rc = -1; /* give up after bad command */
            continue;
        }

        /* found - check max args */
        if (argc > cmdtp->maxargs)
        {
            SHELL_PRINTF("Usage:\n%s\r\n", cmdtp->usage);
            rc = -1;
            continue;
        }

        /* OK - call function to do the command */
        if ((cmdtp->cmd)(cmdtp, argc, (const uint8_t **)argv) != 0)
        {
            rc = -1;
        }
    }

    return rc;
}

/**
 ******************************************************************************
 * @brief      打印帮助信息help
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @details
 *
 * @note
 ******************************************************************************
 */
static uint32_t
help(void)
{
    uint8_t *usage;
    cmd_tbl_t *cmdtptemp;

    SHELL_PRINTF("\n");
    /* print short help (usage) */
    for (cmdtptemp = &__shell_cmd_start; cmdtptemp != &__shell_cmd_end;
            cmdtptemp++)
    {
        usage = (uint8_t *)cmdtptemp->name;
        if (usage == NULL)
        {
            continue;
        }
        SHELL_PRINTF((char*) usage);
        SHELL_PRINTF("\r\t\t\t");
        usage = (uint8_t *)cmdtptemp->usage;
        /* allow user abort */
        if (usage == NULL)
        {
            continue;
        }
        SHELL_PRINTF((char*) usage);
    }
    return 0;

}
/*SHELL CMD FOR SHELL*/
uint32_t do_help(cmd_tbl_t * cmdtp, uint32_t argc, const uint8_t *argv[])
{
    return help();
}

SHELL_CMD(help, CFG_MAXARGS, do_help, "Print this list\r\n");

/**
 ******************************************************************************
 * @brief      shell任务执行体
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
shell_loop(void)
{
    the_dmnid = dmn_register();

    D_ASSERT(the_dmnid != NULL);

    while (1)
    {
        if (readline())
            run_command((char_t *) console_buffer);
    }
}

/**
 ******************************************************************************
 * @brief      shell模块初始化
 * @param[in]  stacksize    : shell任务栈大小
 *
 * @retval  OK      :   初始化成功
 * @retval  ERROR   :   初始化失败
 ******************************************************************************
 */
status_t
shell_init(uint32_t stacksize)
{
    if (shellTaskId != 0)
        return ERROR; /* already called */

    stacksize = (stacksize == 0) ? TASK_STK_SIZE_SHELL : stacksize;
    shellTaskId = taskSpawn((const signed char * const )"shell",
            TASK_PRIORITY_SHELL, stacksize,
            (OSFUNCPTR)shell_loop, 0);

    D_ASSERT(shellTaskId != NULL);
    return OK;
}
