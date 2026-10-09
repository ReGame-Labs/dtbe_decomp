#include "common.h"
#include "engine/gfx/vram_cache.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/vram_cache", CACHE_VTABLE);

/* Cache constructor: sizes a local array from its argument after statements,
 * a g++ variable-length array. C needs an inner block, which saves and restores
 * sp; left for the C++ build. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/vram_cache", cacheInit);

/* Empties the cache: links all its slots into the use list, in order, and
 * drops the tree. Each slot is linked to the one after it, kept in next. */
void cacheReset(Cache *this) {
    CacheSlot *slot = this->slots;
    ListNode *prev = &this->used;
    u32 n = this->count;
    CacheSlot *next = slot + 1;

    this->used.next = &slot->link;
    while (n--) {
        slot->link.prev = prev;
        slot->link.next = &next->link;
        prev = &slot->link;
        slot++;
        next = slot + 1;
    }
    this->used.prev = prev;
    prev->next = &this->used;
    this->root = NULL;
}

void cacheDestroy(Cache *this, s32 flags) {
    this->vtable = &CACHE_VTABLE;
    if (this->slots != NULL) {
        operatorVecDelete(this->slots);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Loads the keys from first to last, middle first so that the tree stays
 * balanced. */
void cacheLoadRange(Cache *this, u32 first, u32 last) {
    u32 middle;

    if (first <= last) {
        if (first == last) {
            cacheGetSlot(this, last);
        } else {
            middle = (first + last) >> 1;
            cacheGetSlot(this, middle);
            cacheLoadRange(this, first, middle - 1);
            cacheLoadRange(this, middle + 1, last);
        }
    }
}

/* Returns the slot holding key, loading it into the least recently used slot
 * if it is not there. */
CacheSlot *cacheGetSlot(Cache *this, u32 key) {
    CacheSlot *slot = cacheFindSlot(this, key);

    if (slot == NULL) {
        slot = (CacheSlot *)this->used.prev;
        cacheRemoveSlot(this, slot);
        cacheAddSlot(this, slot, key);
        this->vtable->load.func((u8 *)this + this->vtable->load.delta, slot->x, slot->y, key);
    }
    cacheMarkSlotUsed(this, slot);
    return slot;
}

/* Finds the slot holding key. */
CacheSlot *cacheFindSlot(Cache *this, u32 key) {
    CacheSlot *node = this->root;

    while (node != NULL) {
        if (node->key == key) {
            return node;
        }
        if (node->key < key) {
            node = node->higher;
        } else {
            node = node->lower;
        }
    }
    return node;
}

/* Makes slot the most recently used. */
void cacheMarkSlotUsed(Cache *this, CacheSlot *slot) {
    ListNode *head = &this->used;

    if (head->next != &slot->link) {
        slot->link.prev->next = slot->link.next;
        slot->link.next->prev = slot->link.prev;
        slot->link.prev = head;
        slot->link.next = head->next;
        head->next->prev = &slot->link;
        head->next = &slot->link;
    }
}

/* Adds slot to the tree under key. */
void cacheAddSlot(Cache *this, CacheSlot *slot, u32 key) {
    CacheSlot **link = &this->root;

    slot->higher = NULL;
    slot->lower = NULL;
    slot->key = key;
    while (*link != NULL) {
        if ((*link)->key < key) {
            link = &(*link)->higher;
        } else {
            link = &(*link)->lower;
        }
    }
    *link = slot;
}

/* Near miss: the node->lower load goes to another register and delay slot. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/vram_cache", cacheRemoveSlot);

/* Takes the node with the greatest key out of the subtree at link. */
CacheSlot *removeHighestSlot(CacheSlot **link) {
    CacheSlot *node;

    while ((*link)->higher != NULL) {
        link = &(*link)->higher;
    }
    node = *link;
    *link = node->lower;
    return node;
}

/* rectCountCells again: the number of (1 << wshift) x (1 << hshift) aligned
 * cells that rect touches. */
s32 func_8002EB94(RECT *rect, s32 wshift, s32 hshift) {
    s32 cellW = 1 << wshift;
    s32 cellH = 1 << hshift;
    s32 cols = ((-cellW & (rect->x + rect->w + cellW - 1)) - (-cellW & rect->x)) >> wshift;
    s32 rows = ((-cellH & (rect->y + rect->h + cellH - 1)) - (-cellH & rect->y)) >> hshift;

    return cols * rows;
}

/* rectSplitCells again: splits rect at the (1 << wshift) x (1 << hshift)
 * aligned cells it touches, column by column, into out; returns the end of
 * what it wrote. */
RECT *func_8002EC00(RECT *out, RECT *rect, s32 wshift, s32 hshift) {
    s32 cellW = 1 << wshift;
    s32 cellH = 1 << hshift;
    s32 cols = ((-cellW & (rect->x + rect->w + cellW - 1)) - (-cellW & rect->x)) >> wshift;
    s32 rows = ((-cellH & (rect->y + rect->h + cellH - 1)) - (-cellH & rect->y)) >> hshift;
    s32 x = rect->x;
    s32 nextX;
    s32 col;

    for (col = cols - 1; col >= 0; col--, x = nextX) {
        s32 w;
        s32 y;
        s32 row;

        if (col == 0) {
            w = rect->x + rect->w - x;
        } else {
            w = (-cellW & (x + cellW)) - x;
        }
        y = rect->y;
        nextX = x + w;
        for (row = rows - 1; row >= 0; row--) {
            s32 h;

            if (row == 0) {
                h = rect->y + rect->h - y;
            } else {
                h = (-cellH & (y + cellH)) - y;
            }
            setRECT(out, x, y, w, h);
            y += h;
            out++;
        }
    }
    return out;
}

s16 cacheSlotGetX(CacheSlot *slot) {
    return slot->x;
}

s16 cacheSlotGetY(CacheSlot *slot) {
    return slot->y;
}
