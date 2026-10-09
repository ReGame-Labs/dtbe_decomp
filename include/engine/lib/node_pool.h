#ifndef DTBE_LIB_NODE_POOL_H
#define DTBE_LIB_NODE_POOL_H

/* Pools of nodes of one size: the free ones in a chain, the ones handed out in a list. */

#include "common.h"
#include "engine/lib/list.h"

EXTERN_C_BEGIN

/* A pool of nodes of one size: the free ones are chained by their next
 * links, the ones handed out make a list. */
typedef struct {
    /* 0x00 */ LinkNode *free;
    /* 0x04 */ LinkList used;
    /* 0x10 */ void *nodes; /* the memory the nodes were cut from */
} NodePool;

void nodePoolInit(NodePool *nodePool, void *nodes, s32 size, s32 count);
void nodePoolInitFree(NodePool *nodePool, void *nodes, s32 size, s32 count);
LinkNode *nodePoolAlloc(NodePool *nodePool);
void nodePoolFree(NodePool *nodePool, LinkNode *node);
void nodePoolRelease(NodePool *nodePool, LinkNode *node);
void nodePoolPushFront(NodePool *nodePool, LinkNode *node);
void nodePoolPushBack(NodePool *nodePool, LinkNode *node);
void linkListInsertAfter(LinkList *linkList, LinkNode *after, LinkNode *node);

EXTERN_C_END

#endif /* DTBE_LIB_NODE_POOL_H */
