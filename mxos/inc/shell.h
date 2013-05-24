/**
 ******************************************************************************
 * @file       shell.h
 * @brief      shell模块.
 * @details    This file including all API functions's implement of shell.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __SHELL_H__
#define __SHELL_H__

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <types.h>

/*-----------------------------------------------------------------------------
 Section: Macro Definitions
 ----------------------------------------------------------------------------*/
# define CFG_MAXARGS                (8u)    /**< 命令行最大参数数量 */
#define INCLUDE_SHELL

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
/**
 * Monitor Command Table
 */
struct cmd_tbl_s
{
	char_t      *name;		/**< Command Name			        */
	uint32_t    maxargs;	/**< maximum number of arguments	*/
	uint32_t    (*cmd)(struct cmd_tbl_s *, uint32_t, const uint8_t *[]);
	char_t      *usage;		/**< Usage message	(short)	    */
};
typedef struct cmd_tbl_s	cmd_tbl_t;

#define Struct_Section  __attribute__ ((unused,section (".shell_cmd")))

#ifdef INCLUDE_SHELL
#define SHELL_CMD(name, maxargs, cmd, usage) \
const cmd_tbl_t __shell_cmd_##name Struct_Section = {#name, maxargs, cmd, usage}
#else
#define SHELL_CMD(name,maxargs,cmd,usage) 
#endif
/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern status_t shell_init(uint32_t stacksize);

#endif /* __SHELL_H__ */

/*------------------------------End of shell.h-------------------------------*/

