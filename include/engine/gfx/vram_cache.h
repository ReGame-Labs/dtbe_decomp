#ifndef DTBE_GFX_VRAM_CACHE_H
#define DTBE_GFX_VRAM_CACHE_H

/* A cache of the most recently used images in equal VRAM cells, found by key in a tree. */

#include "common.h"
#include <libgpu.h>
#include "vtable.h"
#include "engine/lib/list.h"

EXTERN_C_BEGIN

/* A cell of VRAM the cache can hold one image in. */
typedef struct CacheSlot {
    /* 0x00 */ ListNode link; /* in the cache's use order */
    /* 0x08 */ struct CacheSlot *higher; /* the subtree of greater keys */
    /* 0x0C */ struct CacheSlot *lower;  /* the subtree of smaller keys */
    /* 0x10 */ u32 key;
    /* 0x14 */ s16 x;
    /* 0x16 */ s16 y;
} CacheSlot;

/* the virtual table of Cache */
typedef struct CacheVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry load;    /* (Cache *, s32 x, s32 y, u32 key) */
    /* 0x10 */ VtableEntry destroy; /* (Cache *, s32 flags) */
} CacheVtable;

/* Keeps the most recently used images in a set of equal VRAM cells, found
 * by key through a binary tree. */
typedef struct Cache {
    /* 0x00 */ CacheSlot *root;
    /* 0x04 */ CacheSlot *slots;
    /* 0x08 */ s32 count;
    /* 0x0C */ ListNode used; /* most recently used first */
    /* 0x14 */ CacheVtable *vtable;
} Cache;

extern struct CacheVtable CACHE_VTABLE; /* of Cache */

Cache *cacheInit(Cache *cache, RECT *areas, s32 count, s32 cellWidth, s32 cellHeight);
void cacheReset(Cache *cache);
void cacheDestroy(Cache *cache, s32 flags);
void cacheLoadRange(Cache *cache, u32 first, u32 last);
CacheSlot *cacheGetSlot(Cache *cache, u32 key);
CacheSlot *cacheFindSlot(Cache *cache, u32 key);
void cacheMarkSlotUsed(Cache *cache, CacheSlot *slot);
void cacheAddSlot(Cache *cache, CacheSlot *slot, u32 key);
void cacheRemoveSlot(Cache *cache, CacheSlot *slot);
CacheSlot *removeHighestSlot(CacheSlot **link);
s32 func_8002EB94(RECT *rect, s32 widthShift, s32 heightShift);
RECT *func_8002EC00(RECT *out, RECT *rect, s32 widthShift, s32 heightShift);

s16 cacheSlotGetX(CacheSlot *cacheSlot);
s16 cacheSlotGetY(CacheSlot *cacheSlot);

EXTERN_C_END

#endif /* DTBE_GFX_VRAM_CACHE_H */
