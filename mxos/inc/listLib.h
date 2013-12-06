/**
 ******************************************************************************
 * @file       listLib.h
 * @brief      listģ��.
 * @details    �ṩͳһ�������������.
 * @copyright
 *
 ******************************************************************************
 */
#ifndef __LISTLIB_H__
#define __LISTLIB_H__

/*-----------------------------------------------------------------------------
 Section: Type Definitions
 ----------------------------------------------------------------------------*/
#pragma pack(push, 1)
struct ListNode
{
	struct ListNode *pNextNode;
	struct ListNode *pPrevNode;
};

#pragma pack(pop)

#define LIST_FOR_EACH(iter, pHead) \
	for (iter = (pHead)->pNextNode; iter != (pHead); iter = iter->pNextNode)

#define MemToObj(ptr, type, member) \
    (type *)((char *)ptr - (char *)(&((type *)0)->member))

/*-----------------------------------------------------------------------------
 Section: Function Prototypes
 ----------------------------------------------------------------------------*/
extern void
InitListHead(struct ListNode *pHead);

extern void
ListAddHead(struct ListNode *pNew,
        struct ListNode *pHead);
extern void
ListAddTail(struct ListNode *pNew,
        struct ListNode *pHead);

extern void
ListDelNode(struct ListNode *pNode);

extern int
ListIsEmpty(const struct ListNode *pHead);

#endif /* __LISTLIB_H__ */
/*-----------------------------End of listLib.h------------------------------*/
