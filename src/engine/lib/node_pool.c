#include "common.h"
#include "engine/lib/node_pool.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool_inline.h"
#include "libsnd.h"
#include "memory.h"

/* Sets up pool with count nodes of size bytes cut from nodes, all free. */
void nodePoolInit(NodePool *pool, void *nodes, s32 size, s32 count) {
    pool->nodes = nodes;
    pool->used.head = NULL;
    pool->used.tail = NULL;
    pool->used.count = 0;
    nodePoolInitFree(pool, nodes, size, count);
}

/* Chains count nodes of size bytes from nodes into the free nodes of pool. */
void nodePoolInitFree(NodePool *pool, void *nodes, s32 size, s32 count) {
    LinkNode *node;
    LinkNode *next;
    LinkNode *end;

    pool->free = nodes;
    if (count != 0) {
        end = (LinkNode *)((u8 *)nodes + size * count);
        node = nodes;
        do {
            next = (LinkNode *)((u8 *)node + size);
            node->next = next;
            node = next;
        } while (node != end);
        node = (LinkNode *)((u8 *)node - size);
        node->next = NULL;
    }
}

/* a free node of pool, or NULL when there is none */
LinkNode *nodePoolAlloc(NodePool *pool) {
    return nodePoolAllocInline(pool);
}

/* Gives node back to the free nodes of pool. */
void nodePoolFree(NodePool *pool, LinkNode *node) {
    nodePoolFreeInline(pool, node);
}

/* Takes node out of the list of pool and frees it. */
void nodePoolRelease(NodePool *pool, LinkNode *node) {
    nodePoolReleaseInline(pool, node);
}

/* Hands node out of pool at the head of its list. */
void nodePoolPushFront(NodePool *pool, LinkNode *node) {
    nodePoolPushFrontInline(pool, node);
}

/* Hands node out of pool at the tail of its list. */
void nodePoolPushBack(NodePool *pool, LinkNode *node) {
    nodePoolPushBackInline(pool, node);
}

/* Inserts node into list after the node after; into an empty list, as its
 * only node. */
void linkListInsertAfter(LinkList *list, LinkNode *after, LinkNode *node) {
    LinkNode *next;

    list->count++;
    if (list->head == NULL) {
        node->next = NULL;
        node->prev = NULL;
        list->head = node;
        list->tail = node;
        return;
    }
    node->prev = after;
    node->next = after->next;
    next = after->next;
    if (next == NULL) {
        list->tail = node;
    } else {
        next->prev = node;
    }
    after->next = node;
}
