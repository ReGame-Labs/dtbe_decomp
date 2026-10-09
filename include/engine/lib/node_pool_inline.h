#ifndef DTBE_LIB_NODE_POOL_INLINE_H
#define DTBE_LIB_NODE_POOL_INLINE_H

/* The node pool's operations as inline functions, which the code after them inlines. */

#include "common.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"

/* A free node of nodePool, or NULL when there is none. nodePoolAlloc is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline LinkNode *nodePoolAllocInline(NodePool *nodePool) {
    LinkNode *node = nodePool->free;
    LinkNode *result;

    if (node == NULL) {
        result = NULL;
    } else {
        nodePool->free = node->next;
        result = node;
    }
    return result;
}

/* Gives node back to the free nodes of nodePool. nodePoolFree is its out-of-line
 * copy; the code after it inlines it, as the game did. */
static inline void nodePoolFreeInline(NodePool *nodePool, LinkNode *node) {
    node->next = nodePool->free;
    nodePool->free = node;
}

/* Takes node out of the list of nodePool and frees it. nodePoolRelease is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolReleaseInline(NodePool *nodePool, LinkNode *node) {
    linkListRemove(&nodePool->used, node);
    nodePoolFreeInline(nodePool, node);
}

/* Hands node out of nodePool at the head of its list. nodePoolPushFront is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolPushFrontInline(NodePool *nodePool, LinkNode *node) {
    linkListInsertBefore(&nodePool->used, nodePool->used.head, node);
}

/* Hands node out of nodePool at the tail of its list. nodePoolPushBack is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolPushBackInline(NodePool *nodePool, LinkNode *node) {
    linkListInsertAfter(&nodePool->used, nodePool->used.tail, node);
}

#endif /* DTBE_LIB_NODE_POOL_INLINE_H */
