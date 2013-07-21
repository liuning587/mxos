/**
 ******************************************************************************
 * @file      memLib.c
 * @brief     内存管理模块
 * @details   本文管理动态内存分配及释放(本文不能使用信号量)
 * @copyright
 *
 ******************************************************************************
 */
 
/*-----------------------------------------------------------------------------
Section: Includes
-----------------------------------------------------------------------------*/
#include <listLib.h>
#include <intLib.h>
#include <debug.h>
#include <memLib.h>
#if 0
/*-----------------------------------------------------------------------------
Section: Type Definitions
-----------------------------------------------------------------------------*/
#define WORD_SIZE               sizeof(uint32_t)
#define WORD_ALIGN_UP(addr)     (((addr) + WORD_SIZE - 1) & ~(WORD_SIZE - 1))
#define WORD_ALIGN_DOWN(addr)   ((addr) & ~(WORD_SIZE - 1))

#define DWORD_SIZE              (WORD_SIZE << 1)

#define LIST_NODE_SIZE          WORD_ALIGN_UP(sizeof(struct ListNode))
#define LIST_NODE_ALIGN(nSize)  (((nSize) + LIST_NODE_SIZE - 1) & ~(LIST_NODE_SIZE - 1))
#define MIN_HEAP_LEN            1024
/* 判断空闲内存节点依据:最低位是否为1 */
#define IS_FREE(nSize)          (((nSize) & (WORD_SIZE - 1)) == 0)
#define GET_SIZE(pRegion)       ((pRegion)->nCurrSize & ~(WORD_SIZE - 1))

struct MemRegion
{
    size_t   nPrevSize;    /**< 上一个节点的大小 */
    size_t   nCurrSize;    /**< 当前点的大小 */
    struct ListNode lnMemRegion;    /**< 通用链表节点 */
};
/* 注意：考虑到下一个节点地址可以通过currsize计算得到，所以上面的结构体只有
   上一个节点的大小 */

static struct ListNode g_listFreeRegion;    /**< 指向内存链表 */
static uint32_t totleMemSize = 0u;
static bool_e is_heap_init = FALSE;
/*-----------------------------------------------------------------------------
Section: Function Definitions
-----------------------------------------------------------------------------*/
/* 获取下一个节点地址 */
static inline struct MemRegion *GetSuccessor(struct MemRegion *pRegion)
{
    return (struct MemRegion *)((uint8_t *)pRegion + DWORD_SIZE + GET_SIZE(pRegion));
}

/* 获取上一个节点地址 */
static inline struct MemRegion *GetPredeccessor(struct MemRegion *pRegion)
{
    return (struct MemRegion *)((uint8_t *)pRegion - (pRegion->nPrevSize & ~(WORD_SIZE - 1)) - DWORD_SIZE);
}

/* 重新设置pRegion节点的大小 */
static inline void RegionSetSize(struct MemRegion *pRegion, size_t nSize)
{
    struct MemRegion *pSuccRegion;

    pRegion->nCurrSize = nSize;     /* 当前节点尺寸赋值 */

    pSuccRegion = GetSuccessor(pRegion);    /* 获取下一节点 */
    pSuccRegion->nPrevSize = nSize; /* 下一节点的上一节点赋值 */
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
memlib_init(uint32_t start, uint32_t end)
{
    struct MemRegion *pFirst, *pTail;

    start = WORD_ALIGN_UP(start);   /* malloc分配首地址，up字节对齐 */
    end   = WORD_ALIGN_DOWN(end);   /* malloc分配末地址，down字节对齐 */

    if (start + MIN_HEAP_LEN >= end)    /* 若可分配的堆空间小于最小堆尺寸，则返回 错误 */
    {
        return ERROR;
    }

    /* 初始化时有头结点和尾节点，物理上为堆头和堆尾位置 */
    pFirst = (struct MemRegion *)start;
    pTail  = (struct MemRegion *)(end - DWORD_SIZE);    /* 不知道为何-DWORD_SIZE */

    totleMemSize = (size_t)pTail - (size_t)pFirst;
    pFirst->nPrevSize = 1;  /* 将上一个节点尺寸赋1表示上一节点为使用状态 */
    /* 计算pFirst节点可分配内存 */
    pFirst->nCurrSize = (size_t)pTail - (size_t)pFirst - DWORD_SIZE;

    pTail->nPrevSize = pFirst->nCurrSize;   /* pTail上一结点为pTail */
    pTail->nCurrSize = 1;   /* pTail下一结点为使用状态 */

    InitListHead(&g_listFreeRegion);    /* 初始化内存free链表 */

    /* 将初始化的free节点添加到内存free链表中 */
    ListAddTail(&pFirst->lnMemRegion, &g_listFreeRegion);
    is_heap_init = TRUE;

    return OK;
}


/**
 ******************************************************************************
 * @brief      malloc函数的实现
 * @param[in]  nSize    : 需要分配的大小
 * @retval     申请成功返回地址，失败返回NULL
 ******************************************************************************
 */
void *
malloc(size_t nSize)
{
    void *p = NULL;
    struct ListNode *iter;
    size_t nAllocSize, nRestSize;
    struct MemRegion *pCurrRegion, *pSuccRegion;    // *pPrevRegion,

    if (is_heap_init != TRUE)
    {
        return NULL;
    }

    /* 计算实际需要的大小（4字节对齐）= 要申请的内存大小 + 节点信息大小 */
    nAllocSize = LIST_NODE_ALIGN(nSize);

    intLock();    /* 进入临界区 */
    /* 遍历free内存链表 */
    LIST_FOR_EACH(iter, &g_listFreeRegion)
    {
        /* 取得遍历到的对象 */
        pCurrRegion = MemToObj(iter, struct MemRegion, lnMemRegion);
        // printf("%d <--> %d\n", pCurrRegion->nCurrSize, nAllocSize);
        /* 该节点有足够空间分配nAllocSize,则进入do_alloc */
        if (pCurrRegion->nCurrSize >= nAllocSize)
            goto do_alloc;
    }
    /* 若没有空间分配了，则返回空指针 */
    intUnlock();  /* 退出临界区 */

    return NULL;

do_alloc:
    ListDelNode(iter);  /* 首先删除当前节点 */

    /* 计算分配内存后剩余值 */
    nRestSize = pCurrRegion->nCurrSize - nAllocSize;

    /* 若分配后，剩余空间不足以再分配节点 */
    if (nRestSize < sizeof(struct MemRegion))
    {
        /* 当分配剩余内存小于结点大小时，全部内存分配。标志当前节点内存使用位*/
        RegionSetSize(pCurrRegion, pCurrRegion->nCurrSize | 1);
    }
    else
    {
        /* 在当前节点分配空间,并标志使用位 */
        RegionSetSize(pCurrRegion, nAllocSize | 1);

        /* 获得下一节点地址 */
        pSuccRegion = GetSuccessor(pCurrRegion);
        /* 重新计算下一节点可分配内存 */
        RegionSetSize(pSuccRegion, nRestSize - DWORD_SIZE);
        /* 将空余节点增加到内存free空闲链表中 */
        ListAddTail(&pSuccRegion->lnMemRegion, &g_listFreeRegion);
    }

    /* 获得内存地址 脱离链表,可直接写lnMemRegion*/
    p = &pCurrRegion->lnMemRegion;

    intUnlock();  /* 退出临界区 */

    return p;
}

/**
 ******************************************************************************
 * @brief      内存释放
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
free(void *p)
{
    struct MemRegion *pCurrRegion, *pSuccRegion;

    if (is_heap_init != TRUE)
    {
        return ;
    }
    intLock();    /* 进入临界区 */

    pCurrRegion = (struct MemRegion *)((size_t)p - DWORD_SIZE);
    pSuccRegion = GetSuccessor(pCurrRegion);

    /* 若下一个节点为free状态 */
    if (IS_FREE(pSuccRegion->nCurrSize))
    {
        /* 则合并当前节点和下一个节点，计算总空余空间 */
        RegionSetSize(pCurrRegion, GET_SIZE(pCurrRegion) + pSuccRegion->nCurrSize + DWORD_SIZE);
        /* 删除下一个节点 */
        ListDelNode(&pSuccRegion->lnMemRegion);
    }
    else
    {
        /* 将低位置为0，表示空闲状态 */
        RegionSetSize(pCurrRegion, GET_SIZE(pCurrRegion));
    }

    /* 若上一个节点是空闲状态 */
    if (IS_FREE(pCurrRegion->nPrevSize))
    {
        struct MemRegion *pPrevRegion;

        /* 获取上一个节点地址 */
        pPrevRegion = GetPredeccessor(pCurrRegion);
        /* 合并本节的与上一个节点 */
        RegionSetSize(pPrevRegion, pPrevRegion->nCurrSize + pCurrRegion->nCurrSize + DWORD_SIZE);
    }
    else
    {
        /* 将本节的添加到内存free链表中 */
        ListAddTail(&pCurrRegion->lnMemRegion, &g_listFreeRegion);
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
    struct ListNode *iter;
    struct MemRegion *pCurrRegion;
    uint32_t TotalFreeSize = 0u;
    uint32_t MaxSize = 0u;
    uint32_t MinSize = 0xffffffff;
    if (is_heap_init != TRUE)
    {
        printf(" Heap not initialized! Please call 'mem_init()'.\n");
        return ;
    }

    intLock();    /* 进入临界区 */
    /* 遍历free内存链表 */
    LIST_FOR_EACH(iter, &g_listFreeRegion)
    {
        /* 取得遍历到的对象 */
        pCurrRegion = MemToObj(iter, struct MemRegion, lnMemRegion);
        if (pCurrRegion->nCurrSize > MaxSize)
            MaxSize = pCurrRegion->nCurrSize;
        if (pCurrRegion->nCurrSize < MinSize)
            MinSize = pCurrRegion->nCurrSize;
        TotalFreeSize += pCurrRegion->nCurrSize;
    }
    intUnlock();   /* 退出临界区 */

    printf("********** Heap Monitor ***********\n");
    printf(" TotalHeapMem = %4d Kb  %4d Byte\n", totleMemSize / 1024, totleMemSize % 1024);
    printf(" TotalFreeMem = %4d Kb  %4d Byte\n", TotalFreeSize / 1024, TotalFreeSize % 1024);
    printf(" MaxFreeMem   = %4d Kb  %4d Byte\n", MaxSize / 1024, MaxSize % 1024);
    printf(" MinFreeMem   = %4d Kb  %4d Byte\n", MinSize / 1024, MinSize % 1024);
    //printf(" Fragindices  = %.2f\n", 1-(float)MaxSize / (float)TotalFreeSize);
    printf("***********************************\n");
}
#endif
/*------------------------------- memLib.c ----------------------------------*/
