/**
 ******************************************************************************
 * @file      memLib.c
 * @brief     内存管理模块
 * @details   本文管理动态内存分配及释放(本文不能使用信号量)
 *
 * @copyright
 ******************************************************************************
 */

/*-----------------------------------------------------------------------------
 Section: Includes
 ----------------------------------------------------------------------------*/
#include <listLib.h>
#include <intLib.h>
#include <debug.h>
#include <memLib.h>
#include <logLib.h>
#include <maths.h>

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
typedef struct _heap
{
    uint32_t magic;         /**< 魔数 */
    uint32_t presize;       /**< 上一节点大小,最低位1表示used 0 free */
    uint32_t cursize;       /**< 当前节点大小,最低位1表示used 0 free */
    struct ListNode node;   /**< 通用链表节点 */
} heap_t;

/*-----------------------------------------------------------------------------
 Section: Constant Definitions
 ----------------------------------------------------------------------------*/
#define MAGIC_NUM               (0xdeaddead)
#define WORD_SIZE               sizeof(uint32_t)
#define ALIGN_UP(addr)          (((addr) + WORD_SIZE - 1) & ~(WORD_SIZE - 1))
#define ALIGN_DOWN(addr)        ((addr) & ~(WORD_SIZE - 1))

#define MIN_HEAP_LEN            1024

/* 判断空闲内存节点依据:最低位是否为1 */
#define IS_FREE(size)          (((size) & 0x01) == 0)
#define GET_SIZE(size)         (size & ~(WORD_SIZE - 1))

/*-----------------------------------------------------------------------------
 Section: Global Variables
 ----------------------------------------------------------------------------*/
/* NONE */

/*-----------------------------------------------------------------------------
 Section: Local Variables
 ----------------------------------------------------------------------------*/
static struct ListNode the_heap_list = {NULL, NULL};    /**< 指向内存链表 */
static uint32_t the_totle_size = 0u;

/*-----------------------------------------------------------------------------
 Section: Local Function Prototypes
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
 * @brief   获取相邻下一个节点地址
 * @param[in]  *pheap   : 当前节点
 *
 * @retval  下一个节点地址
 ******************************************************************************
 */
static inline heap_t *
heap_next(const heap_t *pheap)
{
    return (heap_t *)((uint8_t *)pheap + ALIGN_UP(MOFFSET(heap_t, node))
            + GET_SIZE(pheap->cursize));
}

/**
 ******************************************************************************
 * @brief   获取相邻上一个节点地址
 * @param[in]  *pheap   : 当前节点
 *
 * @retval  上一个节点地址
 ******************************************************************************
 */
static inline heap_t *
heap_pre(const heap_t *pheap)
{
    return (heap_t *)((uint8_t *)pheap - ALIGN_UP(MOFFSET(heap_t, node))
            - GET_SIZE(pheap->presize));
}

/**
 ******************************************************************************
 * @brief   重置节点大小
 * @param[in]  *pheap   : 当前节点
 * @param[in]  size     : 大小
 *
 * @return  None
 ******************************************************************************
 */
static inline void
region_set_size(heap_t *pheap, uint32_t size)
{
    heap_t *pnext;

    pheap->cursize = size;
    pnext = heap_next(pheap);
    pnext->presize = size;
}

/**
 ******************************************************************************
 * @brief   初始化堆
 * @param[in]  start    : 堆起始地址
 * @param[in]  end      : 堆末地址
 *
 * @retval     OK
 * @retval     ERROR
 *
 * @details  初始化后空余内存链表会有两个结点，头结点和尾节点
 ******************************************************************************
 */
status_t
memlib_add(uint32_t start, uint32_t end)
{
    heap_t *pfirst = NULL;
    heap_t *ptail = NULL;

    //todo: 防止重复调用
    start = ALIGN_UP(start);   /* malloc分配首地址，up字节对齐 */
    end   = ALIGN_DOWN(end);   /* malloc分配末地址，down字节对齐 */

    if (start + MIN_HEAP_LEN >= end)    /* 若可分配的堆空间小于最小堆尺寸，则返回 错误 */
    {
        return ERROR;
    }

    pfirst = (heap_t *)start;
    ptail = (heap_t *)(end - ALIGN_UP(MOFFSET(heap_t, node)));

    pfirst->magic = MAGIC_NUM;
    pfirst->cursize = end - start - 2 * ALIGN_UP(MOFFSET(heap_t, node));
    pfirst->presize = 0x01;

    ptail->magic = MAGIC_NUM;
    ptail->presize = pfirst->cursize;
    ptail->cursize = 0x01;

    if (the_heap_list.pNextNode == NULL)
    {
        InitListHead(&the_heap_list);
    }
    intLock();
    ListAddTail(&pfirst->node, &the_heap_list);
    intUnlock();
    the_totle_size += end - start;

    return OK;
}

/**
 ******************************************************************************
 * @brief   malloc函数的实现
 * @param[in]  nSize    : 需要分配的大小
 *
 * @retval  申请成功返回地址，失败返回NULL
 ******************************************************************************
 */
void *
malloc(size_t size)
{
    void *p = NULL;
    struct ListNode *piter;

    size_t alloc_size;
    size_t rest_size;
    heap_t *pheap;
    heap_t *pnext;

    if (the_heap_list.pNextNode == NULL)
    {
        return NULL;
    }

    /* 计算实际需要的大小(4字节对齐)  */
    alloc_size = ALIGN_UP(size);

    intLock();    /* 进入临界区 */

    LIST_FOR_EACH(piter, &the_heap_list)
    {
        pheap = MemToObj(piter, heap_t, node);
        if (pheap->magic != MAGIC_NUM)
        {
            logmsg("Warning: mem over write at[0x%08x].\n", &pheap->node);
        }
        if (GET_SIZE(pheap->cursize) >= alloc_size)
        {
            goto do_alloc;
        }
    }

    intUnlock();  /* 退出临界区 */

    return NULL;

do_alloc:
    ListDelNode(piter);  /* 首先删除当前节点 */

    /* 计算分配内存后剩余值 */
    rest_size = GET_SIZE(pheap->cursize) - alloc_size;

    /* 若分配后，剩余空间不足以再分配节点 */
    if (rest_size <= ALIGN_UP(MOFFSET(heap_t, node)))
    {
        /* 当分配剩余内存小于结点大小时，全部内存分配*/
        region_set_size(pheap, pheap->cursize | 0x01);
    }
    else
    {
        /* 在当前节点分配空间,并标志使用位 */
        region_set_size(pheap, alloc_size | 0x01);

        /* 获得下一节点地址 */
        pnext = heap_next(pheap);
        pnext->magic = MAGIC_NUM;

        /* 重新计算下一节点可分配内存 */
        region_set_size(pnext, rest_size - ALIGN_UP(MOFFSET(heap_t, node)));

        /* 将空余节点增加到内存free空闲链表中 */
        ListAddTail(&pnext->node, &the_heap_list);
    }
    intUnlock();  /* 退出临界区 */

    /* 获得内存地址 脱离链表,注意node空间可以写*/
    p = &pheap->node;

    return p;
}

/**
 ******************************************************************************
 * @brief   内存释放
 * @param[in]  None
 *
 * @retval     None
 ******************************************************************************
 */
void
free(void *p)
{
    heap_t *pheap;
    heap_t *ptmp;

    if (the_heap_list.pNextNode == NULL)
    {
        return;
    }

    if (!IS_FREE((uint32_t)p))
    {
        logmsg("Warning: can not free block at[0x%08x].\n", p);
        return;
    }

    pheap = (heap_t *)((size_t)p - ALIGN_UP(MOFFSET(heap_t, node)));
    if (pheap->magic != MAGIC_NUM)
    {
        logmsg("Warning: mem over write, can not free at[0x%08x].\n",
                &pheap->node);
        return;
    }

    intLock();    /* 进入临界区 */

    /* 若下一个节点为free状态 */
    ptmp = heap_next(pheap);

    if (ptmp->magic != MAGIC_NUM)
    {
        logmsg("Warning: mem over write at[0x%08x].\n", &pheap->node);
    }

    if (IS_FREE(ptmp->cursize))
    {
        /* 则合并当前节点和下一个节点，计算总空余空间 */
        region_set_size(pheap, GET_SIZE(pheap->cursize)
                + ALIGN_UP(MOFFSET(heap_t, node)) + ptmp->cursize);
        /* 删除下一个节点 */
        ListDelNode(&ptmp->node);
    }
    else
    {
        /* 将低位置为0，表示空闲状态 */
        region_set_size(pheap, GET_SIZE(pheap->cursize));
    }

    /* 若上一个节点是空闲状态 */
    if (IS_FREE(pheap->presize))
    {
        ptmp = heap_pre(pheap);
        region_set_size(ptmp, pheap->cursize + pheap->presize
                + ALIGN_UP(MOFFSET(heap_t, node)));
    }
    else
    {
        /* 将本节的添加到内存free链表中 */
        ListAddTail(&pheap->node, &the_heap_list);
    }

    intUnlock();   /* 退出临界区 */
}

/**
 ******************************************************************************
 * @brief      输出内存使用信息
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
showMenInfo(void)
{
    struct ListNode *piter;
    heap_t *pheap;
    uint32_t TotalFreeSize = 0u;
    uint32_t MaxSize = 0u;
    uint32_t MinSize = 0xffffffff;

    if (the_heap_list.pNextNode == NULL)
    {
        printf(" Heap not initialized! Please call 'mem_init()'.\n");
        return ;
    }

    intLock();    /* 进入临界区 */
    /* 遍历free内存链表 */
    LIST_FOR_EACH(piter, &the_heap_list)
    {
        /* 取得遍历到的对象 */
        pheap = MemToObj(piter, heap_t, node);
        if (pheap->magic != MAGIC_NUM)
        {
            logmsg("Warning: mem over write at[0x%08x].\n", &pheap->node);
        }
        if (pheap->cursize > MaxSize)
        {
            MaxSize = pheap->cursize;
        }
        if (pheap->cursize < MinSize)
        {
            MinSize = pheap->cursize;
        }
        TotalFreeSize += pheap->cursize;
    }
    intUnlock();   /* 退出临界区 */

    printf("********** Heap Monitor ***********\n");
    printf(" TotalHeapMem = %4d Kb  %4d Byte\n", the_totle_size / 1024, the_totle_size % 1024);
    printf(" TotalFreeMem = %4d Kb  %4d Byte\n", TotalFreeSize / 1024, TotalFreeSize % 1024);
    printf(" MaxFreeMem   = %4d Kb  %4d Byte\n", MaxSize / 1024, MaxSize % 1024);
    printf(" MinFreeMem   = %4d Kb  %4d Byte\n", MinSize / 1024, MinSize % 1024);
    //printf(" Fragindices  = %.2f\n", 1-(float)MaxSize / (float)TotalFreeSize);
    printf("***********************************\n");
}
/*--------------------------------memLib.c-----------------------------------*/
