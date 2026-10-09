#ifndef DTBE_GFX_ORDERING_TABLE_H
#define DTBE_GFX_ORDERING_TABLE_H

/* Ordering tables: runs of linked primitive slots, and the one the frame is drawn with. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

/* An ordering table: a run of linked GPU primitive slots and the pointers into it. */
typedef struct {
    s32 depth;       /* number of slots; negative when the table runs in reverse */
    u_long *head;       /* where drawing starts */
    u_long *slots;      /* the lowest slot of the depth range */
    u_long *tail;       /* where drawing ends */
    u_long *firstDrawn; /* the slot drawn first */
    u_long *lastDrawn;  /* the slot drawn last */
} OrderingTable;

/* the ordering table the frame is drawn with */
extern OrderingTable FRAME_OT;

OrderingTable *orderingTableInit(OrderingTable *ot, s32 depth);
void orderingTableClear(OrderingTable *ot);
void orderingTableLinkInto(OrderingTable *ot, OrderingTable *other);
void orderingTableLinkAfter(OrderingTable *ot, u_long *prim);
u_long *orderingTableGetHead(OrderingTable *ot);
u_long *orderingTableGetLastDrawn(OrderingTable *ot);
u_long *orderingTableGetFirstDrawn(OrderingTable *ot);
u_long *orderingTableGetSlots(OrderingTable *ot);
u_long *func_80026BC0(OrderingTable *ot);
u_long *orderingTableGetTail(OrderingTable *ot);
u32 orderingTableIsReversed(OrderingTable *ot);
s32 orderingTableGetSlotCount(OrderingTable *ot);

EXTERN_C_END

#endif /* DTBE_GFX_ORDERING_TABLE_H */
