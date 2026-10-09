#include "common.h"
#include "engine/lib/node_pool.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool_inline.h"

/* Sets up the pool with count nodes of size bytes cut from nodes, all free. */
void nodePoolInit(NodePool *this, void *nodes, s32 size, s32 count) {
    this->nodes = nodes;
    this->used.head = NULL;
    this->used.tail = NULL;
    this->used.count = 0;
    nodePoolInitFree(this, nodes, size, count);
}

/* Chains count nodes of size bytes from nodes into the free nodes of the pool. */
void nodePoolInitFree(NodePool *this, void *nodes, s32 size, s32 count) {
    LinkNode *node;
    LinkNode *next;
    LinkNode *end;

    this->free = nodes;
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

/* Takes a free node of the pool; NULL when there is none. */
LinkNode *nodePoolAlloc(NodePool *this) {
    return nodePoolAllocInline(this);
}

/* Gives node back to the free nodes of the pool. */
void nodePoolFree(NodePool *this, LinkNode *node) {
    nodePoolFreeInline(this, node);
}

/* Takes node out of the list of the pool and frees it. */
void nodePoolRelease(NodePool *this, LinkNode *node) {
    nodePoolReleaseInline(this, node);
}

/* Hands node out of the pool at the head of its list. */
void nodePoolPushFront(NodePool *this, LinkNode *node) {
    nodePoolPushFrontInline(this, node);
}

/* Hands node out of the pool at the tail of its list. */
void nodePoolPushBack(NodePool *this, LinkNode *node) {
    nodePoolPushBackInline(this, node);
}

/* Inserts node into the list after the node after; into an empty list, as
 * its only node. A LinkList operation (linkListInsertBefore is in list.c),
 * but the game has it here, after nodePoolPushBack, its caller through
 * nodePoolPushBackInline. */
void linkListInsertAfter(LinkList *this, LinkNode *after, LinkNode *node) {
    LinkNode *next;

    this->count++;
    if (this->head == NULL) {
        node->next = NULL;
        node->prev = NULL;
        this->head = node;
        this->tail = node;
        return;
    }
    node->prev = after;
    node->next = after->next;
    next = after->next;
    if (next == NULL) {
        this->tail = node;
    } else {
        next->prev = node;
    }
    after->next = node;
}
