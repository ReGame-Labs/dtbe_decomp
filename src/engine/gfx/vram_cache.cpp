#include "common.h"
#include "engine/gfx/vram_cache.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/vram_cache", CACHE_VTABLE);

/* a texture page of VRAM is 1 << 6 by 1 << 8 pixels */
#define TPAGE_WSHIFT 6
#define TPAGE_HSHIFT 8

/*
 * Cache constructor: cuts the areas of VRAM at the texture pages, then the
 * pages into cellW x cellH cells, a slot each (the last cells first), and
 * empties the cache. The pages are kept in a variable-length array, which
 * is why this file is C++.
 */
Cache *cacheInit(Cache *cache, RECT *areas, s32 count, s32 cellW, s32 cellH) {
    u32 pageCount = 0;
    s32 i;

    cache->vtable = &CACHE_VTABLE;
    for (i = 0; i < count; i++) {
        pageCount += func_8002EB94(&areas[i], TPAGE_WSHIFT, TPAGE_HSHIFT);
    }
    RECT pages[pageCount];
    RECT *page = pages;
    for (i = 0; i < count; i++) {
        page = func_8002EC00(page, areas++, TPAGE_WSHIFT, TPAGE_HSHIFT);
    }
    page = pages;
    u32 slotCount = 0;
    for (i = pageCount; i != 0; i--, page++) {
        slotCount += (page->w / cellW) * (page->h / cellH);
    }
    cache->count = slotCount;
    cache->slots = new CacheSlot[slotCount];
    CacheSlot *slot = cache->slots + slotCount;
    page = pages;
    for (i = pageCount; i != 0; i--, page++) {
        s32 cols = page->w / cellW;
        s32 rows = page->h / cellH;
        s32 row;
        s32 col;

        if (cols != 0 && rows != 0) {
            for (row = 0; row < rows; row++) {
                for (col = 0; col < cols; col++) {
                    slot--;
                    slot->x = page->x + col * cellW;
                    slot->y = page->y + cellH * row;
                }
            }
        }
    }
    cacheReset(cache);
    return cache;
}

/* Empties the cache: links all its slots into the use list, in order, and
 * drops the tree. Each slot is linked to the one after it, kept in next. */
void cacheReset(Cache *cache) {
    CacheSlot *slot = cache->slots;
    ListNode *prev = &cache->used;
    u32 n = cache->count;
    CacheSlot *next = slot + 1;

    cache->used.next = &slot->link;
    while (n--) {
        slot->link.prev = prev;
        slot->link.next = &next->link;
        prev = &slot->link;
        slot++;
        next = slot + 1;
    }
    cache->used.prev = prev;
    prev->next = &cache->used;
    cache->root = NULL;
}

void cacheDestroy(Cache *cache, s32 flags) {
    cache->vtable = &CACHE_VTABLE;
    if (cache->slots != NULL) {
        operatorVecDelete(cache->slots);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(cache);
    }
}

/* Loads the keys from first to last, middle first so that the tree stays
 * balanced. */
void cacheLoadRange(Cache *cache, u32 first, u32 last) {
    u32 middle;

    if (first <= last) {
        if (first == last) {
            cacheGetSlot(cache, last);
        } else {
            middle = (first + last) >> 1;
            cacheGetSlot(cache, middle);
            cacheLoadRange(cache, first, middle - 1);
            cacheLoadRange(cache, middle + 1, last);
        }
    }
}

/* Returns the slot holding key, loading it into the least recently used slot
 * if it is not there. */
CacheSlot *cacheGetSlot(Cache *cache, u32 key) {
    CacheSlot *slot = cacheFindSlot(cache, key);

    if (slot == NULL) {
        slot = (CacheSlot *)cache->used.prev;
        cacheRemoveSlot(cache, slot);
        cacheAddSlot(cache, slot, key);
        cache->vtable->load.func((u8 *)cache + cache->vtable->load.delta, slot->x, slot->y, key);
    }
    cacheMarkSlotUsed(cache, slot);
    return slot;
}

/* Finds the slot holding key. */
CacheSlot *cacheFindSlot(Cache *cache, u32 key) {
    CacheSlot *node = cache->root;

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
void cacheMarkSlotUsed(Cache *cache, CacheSlot *slot) {
    ListNode *head = &cache->used;

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
void cacheAddSlot(Cache *cache, CacheSlot *slot, u32 key) {
    CacheSlot **link = &cache->root;

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
