#include "common.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim_buffer.h"
#include "libgpu.h"

/* Sets the depth of an ordering table, negative to make it run in reverse. */
OrderingTable *orderingTableInit(OrderingTable *ot, s32 depth) {
    ot->depth = depth;
    return ot;
}

/* Takes the table's slots from the frame's primitive buffer and clears them. */
void orderingTableClear(OrderingTable *ot) {
    u_long *base = (u_long *)PRIM_BUFFER_FREE;
    s32 depth = ot->depth;
    s32 count = depth < 0 ? -depth : depth;
    s32 size;
    u_long *slot;

    size = count * sizeof(u_long);
    /* the depth range plus a head and a tail slot */
    PRIM_BUFFER_FREE = (u8 *)base + (size + 2 * sizeof(u_long));
    slot = base;
    if (depth < 0) {
        ClearOTagR(slot, count + 2);
        ot->tail = slot;
        slot++;
        ot->lastDrawn = slot;
        ot->slots = slot;
        slot = (u_long *)((u8 *)slot + size);
        ot->firstDrawn = slot - 1;
        ot->head = slot;
    } else {
        ClearOTag(base, count + 2);
        slot = base + 1;
        ot->firstDrawn = slot;
        ot->slots = slot;
        slot = (u_long *)((u8 *)slot + size);
        ot->head = base;
        ot->lastDrawn = slot - 1;
        ot->tail = slot;
    }
}

/* Links this table's chain into another table's, right after its head. */
void orderingTableLinkInto(OrderingTable *ot, OrderingTable *other) {
    orderingTableLinkAfter(ot, other->head);
}

/* Links the table's tail to what follows prim, and prim to the table's head. */
void orderingTableLinkAfter(OrderingTable *ot, u_long *prim) {
    u_long *tail = ot->tail;
    u_long *head = ot->head;

    *tail = (*tail & 0xFF000000) | (*prim & 0xFFFFFF);
    *prim = (*prim & 0xFF000000) | ((u32)head & 0xFFFFFF);
}

/* Returns where drawing of the table starts. */
u_long *orderingTableGetHead(OrderingTable *ot) {
    return ot->head;
}

/* Returns the slot drawn last. */
u_long *orderingTableGetLastDrawn(OrderingTable *ot) {
    return ot->lastDrawn;
}

/* Returns the slot drawn first. */
u_long *orderingTableGetFirstDrawn(OrderingTable *ot) {
    return ot->firstDrawn;
}

/* Returns the lowest slot of the depth range. */
u_long *orderingTableGetSlots(OrderingTable *ot) {
    return ot->slots;
}

/* Returns the lowest slot of the depth range. */
u_long *func_80026BC0(OrderingTable *ot) {
    return ot->slots;
}

/* Returns where drawing of the table ends. */
u_long *orderingTableGetTail(OrderingTable *ot) {
    return ot->tail;
}

/* Returns whether the table runs in reverse. */
u32 orderingTableIsReversed(OrderingTable *ot) {
    return (u32)ot->depth >> 31;
}

/* Returns the number of slots of the table. */
s32 orderingTableGetSlotCount(OrderingTable *ot) {
    s32 depth = ot->depth;

    if (depth < 0) {
        depth = -depth;
    }
    return depth;
}
