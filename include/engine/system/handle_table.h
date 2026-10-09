#ifndef DTBE_SYSTEM_HANDLE_TABLE_H
#define DTBE_SYSTEM_HANDLE_TABLE_H

/* Handle tables: numbers that stand for objects, and stop finding them once given back. */

#include "common.h"

EXTERN_C_BEGIN

/* A handle: the index of its node in the table, and a serial number that
 * changes every time the node is handed out, so that a stale handle no
 * longer finds the node. 0 is never a valid handle. */
typedef union {
    struct {
        u32 index : 12;
        u32 serial : 20;
    } f;
    u32 word;
} Handle;

#define HANDLE_TABLE_MAX 0x1000 /* what a 12-bit index can address */

typedef struct HandleNode {
    /* 0x0 */ struct HandleNode *next;
    /* 0x4 */ struct HandleNode *prev;
    /* 0x8 */ void *owner; /* what the handle stands for; NULL while free */
    /* 0xC */ Handle handle;
} HandleNode;

/* A table of handles. Its nodes make two circular lists: nodes[0] heads
 * the free nodes and usedList (nodes[capacity]) heads the ones in use. */
typedef struct {
    /* 0x0 */ HandleNode *nodes;
    /* 0x4 */ HandleNode *usedList;
    /* 0x8 */ s32 capacity;
} HandleTable;

HandleTable *handleTableInit(HandleTable *table, s32 capacity);
void handleTableDestroy(HandleTable *table, s32 flags);
u32 handleTableAdd(HandleTable *table, void *owner);

void *handleTableRemove(HandleTable *table, u32 handle);
void *handleTableGet(HandleTable *table, u32 handle);
s32 handleTableCountList(HandleTable *table, HandleNode *list);
s32 handleTableCountUsed(HandleTable *table);
s32 handleTableCountFree(HandleTable *table);

EXTERN_C_END

#endif /* DTBE_SYSTEM_HANDLE_TABLE_H */
