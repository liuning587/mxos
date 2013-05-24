/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs R0.08b                  */
/* (C)ChaN, 2011                                                          */
/*------------------------------------------------------------------------*/

#include <stdlib.h>		/* ANSI memory controls */
#include <malloc.h>		/* ANSI memory controls */
#include <time.h>

#include <types.h>
#include <FreeRTOS.h>
#include <semphr.h>

#include "../ff.h"

#if !_FS_READONLY
DWORD get_fattime (void)
{
    time_t curtime = time(NULL);
    struct tm daytime;

    (void)localtime_r(&curtime, &daytime);
    int32_t year = (daytime.tm_year - 80 < 0) ? 0 : daytime.tm_year - 80;
    return (year << 25)                 /* Year  */
            | (daytime.tm_mon << 21)    /* Month */
            | (daytime.tm_mday << 16)   /* Day   */
            | (daytime.tm_hour << 11)   /* Hour  */
            | (daytime.tm_min << 5)     /* Min   */
            | (daytime.tm_sec >> 1);    /* Sec   */
}
#endif

#if _FS_REENTRANT
/*------------------------------------------------------------------------*/
/* Create a Synchronization Object */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to create a new
/  synchronization object, such as semaphore and mutex. When a zero is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* TRUE:Function succeeded, FALSE:Could not create due to any error */
	BYTE vol,			/* Corresponding logical drive being processed */
	_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
	int ret;

	*sobj = xSemaphoreCreateMutex();		/* FreeRTOS */
	ret = (*sobj != NULL);

	return ret;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a synchronization
/  object that created with ff_cre_syncobj function. When a zero is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* TRUE:Function succeeded, FALSE:Could not delete due to any error */
	_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
    bool_e ret;

    if( sobj != NULL )
    {
        vQueueDelete( sobj );
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a zero is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* TRUE:Got a grant to access the volume, FALSE:Could not get a grant */
	_SYNC_t sobj	/* Sync object to wait */
)
{
	int ret;

	ret = (xSemaphoreTake(sobj, _FS_TIMEOUT) == pdTRUE);	/* FreeRTOS */

	return ret;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t sobj	/* Sync object to be signaled */
)
{
	xSemaphoreGive(sobj);	/* FreeRTOS */

}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT size		/* Number of bytes to allocate */
)
{
	return malloc(size);
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
	void* mblock	/* Pointer to the memory block to free */
)
{
	free(mblock);
}

#endif
