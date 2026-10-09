#ifndef DTBE_GFX_ORDERING_TABLE_H
#define DTBE_GFX_ORDERING_TABLE_H

/* Ordering tables: runs of linked primitive slots, and the one the frame is drawn with. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

/* An ordering table: a run of linked GPU primitive slots and the pointers into it. */
typedef struct {
    /* 0x00 */ s32 depth;          /* number of slots; negative when the table runs in reverse */
    /* 0x04 */ u_long *head;       /* where drawing starts */
    /* 0x08 */ u_long *slots;      /* the lowest slot of the depth range */
    /* 0x0C */ u_long *tail;       /* where drawing ends */
    /* 0x10 */ u_long *firstDrawn; /* the slot drawn first */
    /* 0x14 */ u_long *lastDrawn;  /* the slot drawn last */
} OrderingTable; /* size 0x18 */

/* the ordering table the frame is drawn with */
extern OrderingTable FRAME_OT;

OrderingTable *orderingTableInit(OrderingTable *orderingTable, s32 depth);
void orderingTableClear(OrderingTable *orderingTable);
void orderingTableLinkInto(OrderingTable *orderingTable, OrderingTable *other);
void orderingTableLinkAfter(OrderingTable *orderingTable, u_long *prim);
u_long *orderingTableGetHead(OrderingTable *orderingTable);
u_long *orderingTableGetLastDrawn(OrderingTable *orderingTable);
u_long *orderingTableGetFirstDrawn(OrderingTable *orderingTable);
u_long *orderingTableGetSlots(OrderingTable *orderingTable);
u_long *func_80026BC0(OrderingTable *orderingTable);
u_long *orderingTableGetTail(OrderingTable *orderingTable);
u32 orderingTableIsReversed(OrderingTable *orderingTable);
s32 orderingTableGetSlotCount(OrderingTable *orderingTable);

EXTERN_C_END

#endif /* DTBE_GFX_ORDERING_TABLE_H */
