#ifndef HEAP_H
#define HEAP_H

#include "common.h"

/* A block of a heap: a header, then the memory it hands out. The blocks
 * of a heap make a circular list in address order. */
typedef struct HeapBlock {
    /* 0x0 */ struct HeapBlock *next;
    /* 0x4 */ struct HeapBlock *prev;
    /* 0x8 */ s32 size; /* with the header; negated while the block is in use */
} HeapBlock;

typedef struct {
    /* 0x0 */ HeapBlock *rover; /* where the next search starts */
    /* 0x4 */ HeapBlock *first;
    /* 0x8 */ HeapBlock *end;
    /* 0xC */ s32 count; /* blocks, free or in use */
} Heap;

/* the size of the block that holds size bytes */
#define HEAP_BLOCK_SIZE(size) (((size) + sizeof(HeapBlock) + 3) & ~3)
/* what is left of a free block only becomes a block of its own when it is
 * bigger than a header */
#define HEAP_MIN_SPLIT ((s32)sizeof(HeapBlock))

s32 heapLargestFree(Heap *heap);
s32 heapCheck(Heap *heap);
void heapFree(Heap *heap, void *ptr);
void heapInit(Heap *heap, void *base, s32 size);
void *heapAllocBest(Heap *heap, s32 size);
void *heapAllocLargest(Heap *heap, s32 size);
void *heapAllocNext(Heap *heap, s32 size);
void *heapAllocPrev(Heap *heap, s32 size);
s32 heapTotalFree(Heap *heap);
void heapShrink(Heap *heap, void *ptr, s32 size);

extern Heap D_80114468;
extern Heap mainHeap;
extern Heap D_80114490;

/* "Not enough memory. %dBytes.\n" */
extern char *D_8005FE10;

void die(const char *fmt, ...);

#endif /* HEAP_H */
