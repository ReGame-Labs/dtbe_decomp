#ifndef DTBE_LIB_NODE_POOL_INLINE_H
#define DTBE_LIB_NODE_POOL_INLINE_H

/* The node pool's operations as inline functions, which the code after them inlines. */

#include "common.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"

/* A free node of pool, or NULL when there is none. nodePoolAlloc is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline LinkNode *nodePoolAllocInline(NodePool *pool) {
    LinkNode *node = pool->free;
    LinkNode *result;

    if (node == NULL) {
        result = NULL;
    } else {
        pool->free = node->next;
        result = node;
    }
    return result;
}

/* Gives node back to the free nodes of pool. nodePoolFree is its out-of-line
 * copy; the code after it inlines it, as the game did. */
static inline void nodePoolFreeInline(NodePool *pool, LinkNode *node) {
    node->next = pool->free;
    pool->free = node;
}

/* Takes node out of the list of pool and frees it. nodePoolRelease is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolReleaseInline(NodePool *pool, LinkNode *node) {
    linkListRemove(&pool->used, node);
    nodePoolFreeInline(pool, node);
}

/* Hands node out of pool at the head of its list. nodePoolPushFront is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolPushFrontInline(NodePool *pool, LinkNode *node) {
    linkListInsertBefore(&pool->used, pool->used.head, node);
}

/* Hands node out of pool at the tail of its list. nodePoolPushBack is its
 * out-of-line copy; the code after it inlines it, as the game did. */
static inline void nodePoolPushBackInline(NodePool *pool, LinkNode *node) {
    linkListInsertAfter(&pool->used, pool->used.tail, node);
}

#endif /* DTBE_LIB_NODE_POOL_INLINE_H */
