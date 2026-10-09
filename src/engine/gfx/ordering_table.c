#include "common.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim_buffer.h"
#include "libgpu.h"

/* Sets the depth of an ordering table, negative to make it run in reverse. */
OrderingTable *orderingTableInit(OrderingTable *this, s32 depth) {
    this->depth = depth;
    return this;
}

/* Takes the table's slots from the frame's primitive buffer and clears them. */
void orderingTableClear(OrderingTable *this) {
    u_long *base = (u_long *)PRIM_BUFFER_FREE;
    s32 depth = this->depth;
    s32 count = depth < 0 ? -depth : depth;
    s32 size;
    u_long *slot;

    size = count * sizeof(u_long);
    /* the depth range plus a head and a tail slot */
    PRIM_BUFFER_FREE = (u8 *)base + (size + 2 * sizeof(u_long));
    slot = base;
    if (depth < 0) {
        ClearOTagR(slot, count + 2);
        this->tail = slot;
        slot++;
        this->lastDrawn = slot;
        this->slots = slot;
        slot = (u_long *)((u8 *)slot + size);
        this->firstDrawn = slot - 1;
        this->head = slot;
    } else {
        ClearOTag(base, count + 2);
        slot = base + 1;
        this->firstDrawn = slot;
        this->slots = slot;
        slot = (u_long *)((u8 *)slot + size);
        this->head = base;
        this->lastDrawn = slot - 1;
        this->tail = slot;
    }
}

/* Links this table's chain into another table's, right after its head. */
void orderingTableLinkInto(OrderingTable *this, OrderingTable *other) {
    orderingTableLinkAfter(this, other->head);
}

/* Links the table's tail to what follows prim, and prim to the table's head. */
void orderingTableLinkAfter(OrderingTable *this, u_long *prim) {
    u_long *tail = this->tail;
    u_long *head = this->head;

    *tail = (*tail & 0xFF000000) | (*prim & 0xFFFFFF);
    *prim = (*prim & 0xFF000000) | ((u32)head & 0xFFFFFF);
}

/* Returns where drawing of the table starts. */
u_long *orderingTableGetHead(OrderingTable *this) {
    return this->head;
}

/* Returns the slot drawn last. */
u_long *orderingTableGetLastDrawn(OrderingTable *this) {
    return this->lastDrawn;
}

/* Returns the slot drawn first. */
u_long *orderingTableGetFirstDrawn(OrderingTable *this) {
    return this->firstDrawn;
}

/* Returns the lowest slot of the depth range. */
u_long *orderingTableGetSlots(OrderingTable *this) {
    return this->slots;
}

/* Returns the lowest slot of the depth range. */
u_long *func_80026BC0(OrderingTable *this) {
    return this->slots;
}

/* Returns where drawing of the table ends. */
u_long *orderingTableGetTail(OrderingTable *this) {
    return this->tail;
}

/* Returns whether the table runs in reverse. */
u32 orderingTableIsReversed(OrderingTable *this) {
    return (u32)this->depth >> 31;
}

/* Returns the number of slots of the table. */
s32 orderingTableGetSlotCount(OrderingTable *this) {
    s32 depth = this->depth;

    if (depth < 0) {
        depth = -depth;
    }
    return depth;
}
