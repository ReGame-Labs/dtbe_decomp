#include "common.h"
#include "engine/gfx/vram_cache.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/vram_cache", CACHE_VTABLE);

/*
 * Cache constructor: cuts the areas of VRAM at the texture pages, then the
 * pages into cellWidth x cellHeight cells, a slot each (the last cells
 * first), and empties the cache. The pages are kept in a variable-length
 * array, which is why this file is C++.
 */
Cache *cacheInit(Cache *cache, RECT *areas, s32 count, s32 cellWidth, s32 cellHeight) {
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
        slotCount += (page->w / cellWidth) * (page->h / cellHeight);
    }
    cache->count = slotCount;
    cache->slots = new CacheSlot[slotCount];
    CacheSlot *slot = cache->slots + slotCount;
    page = pages;
    for (i = pageCount; i != 0; i--, page++) {
        s32 cols = page->w / cellWidth;
        s32 rows = page->h / cellHeight;
        s32 row;
        s32 col;

        if (cols != 0 && rows != 0) {
            for (row = 0; row < rows; row++) {
                for (col = 0; col < cols; col++) {
                    slot--;
                    slot->x = page->x + col * cellWidth;
                    slot->y = page->y + cellHeight * row;
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
    u32 left = cache->count;
    CacheSlot *next = slot + 1;

    cache->used.next = &slot->link;
    while (left--) {
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

/* Destroys the cache, freeing its slots. */
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

/*
 * Takes slot, found by its key, out of the tree: a leaf goes, a node with one
 * subtree is replaced by it, and a node with two by the highest node of its
 * lower subtree. The address of node->lower is taken before its test, as the
 * call needs it: that keeps node's a0 preference off the load.
 */
void cacheRemoveSlot(Cache *cache, CacheSlot *slot) {
    CacheSlot **link = &cache->root;
    u32 key = slot->key;
    CacheSlot *node;

    while (*link != NULL && (*link)->key != key) {
        if ((*link)->key < key) {
            link = &(*link)->higher;
        } else {
            link = &(*link)->lower;
        }
    }
    node = *link;
    if (node != slot) {
        return;
    }
    if (node->higher == NULL && node->lower == NULL) {
        *link = NULL;
    } else if (node->higher == NULL) {
        *link = node->lower;
    } else {
        CacheSlot **lower = &node->lower;

        if (*lower == NULL) {
            *link = node->higher;
        } else {
            *link = removeHighestSlot(lower);
            (*link)->lower = *lower;
            (*link)->higher = node->higher;
        }
    }
}

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

/* rectCountCells again: the number of (1 << widthShift) x (1 << heightShift)
 * aligned cells that rect touches. */
s32 func_8002EB94(RECT *rect, s32 widthShift, s32 heightShift) {
    s32 cellWidth = 1 << widthShift;
    s32 cellHeight = 1 << heightShift;
    s32 cols = ((-cellWidth & (rect->x + rect->w + cellWidth - 1)) - (-cellWidth & rect->x)) >> widthShift;
    s32 rows = ((-cellHeight & (rect->y + rect->h + cellHeight - 1)) - (-cellHeight & rect->y)) >> heightShift;

    return cols * rows;
}

/* rectSplitCells again: splits rect at the
 * (1 << widthShift) x (1 << heightShift) aligned cells it touches, column by
 * column, into out; returns the end of what it wrote. */
RECT *func_8002EC00(RECT *out, RECT *rect, s32 widthShift, s32 heightShift) {
    s32 cellWidth = 1 << widthShift;
    s32 cellHeight = 1 << heightShift;
    s32 cols = ((-cellWidth & (rect->x + rect->w + cellWidth - 1)) - (-cellWidth & rect->x)) >> widthShift;
    s32 rows = ((-cellHeight & (rect->y + rect->h + cellHeight - 1)) - (-cellHeight & rect->y)) >> heightShift;
    s32 x = rect->x;
    s32 nextX;
    s32 col;

    for (col = cols - 1; col >= 0; col--, x = nextX) {
        s32 width;
        s32 y;
        s32 row;

        if (col == 0) {
            width = rect->x + rect->w - x;
        } else {
            width = (-cellWidth & (x + cellWidth)) - x;
        }
        y = rect->y;
        nextX = x + width;
        for (row = rows - 1; row >= 0; row--) {
            s32 height;

            if (row == 0) {
                height = rect->y + rect->h - y;
            } else {
                height = (-cellHeight & (y + cellHeight)) - y;
            }
            setRECT(out, x, y, width, height);
            y += height;
            out++;
        }
    }
    return out;
}

/* Where the slot is in VRAM, across. */
s16 cacheSlotGetX(CacheSlot *cacheSlot) {
    return cacheSlot->x;
}

/* Where the slot is in VRAM, down. */
s16 cacheSlotGetY(CacheSlot *cacheSlot) {
    return cacheSlot->y;
}
