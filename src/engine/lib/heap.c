#include "common.h"
#include "engine/lib/heap.h"

s32 heapLargestFree(Heap *heap) {
    HeapBlock *block = heap->rover;
    s32 largest = 0;

    do {
        if (largest < block->size) {
            largest = block->size;
        }
        block = block->next;
    } while (block != heap->rover);
    return largest - sizeof(HeapBlock);
}

s32 heapCheck(Heap *heap) {
    HeapBlock *block = heap->rover;
    s32 count = heap->count;
    HeapBlock *first = heap->first;
    HeapBlock *end = heap->end;
    HeapBlock *next;

    do {
        if (block < first || block >= end || ((u32)block & 3)) {
            return -1;
        }
        next = block->next;
        if (next < first || next >= end || ((u32)next & 3)) {
            return -1;
        }
        if (next->prev != block) {
            return -1;
        }
        block = next;
        count--;
    } while (block != heap->rover);
    if (count != 0) {
        return -1;
    }
    return 0;
}

void heapFree(Heap *heap, void *ptr) {
    HeapBlock *block = ptr;
    HeapBlock *neighbor;
    HeapBlock *next;
    HeapBlock *rover;
    s32 size;

    if (block != NULL) {
        block--;
        neighbor = block->next;
        size = -block->size;
        block->size = size;
        rover = heap->rover;
        if (neighbor->size > 0 && (HeapBlock *)((u8 *)block + size) == neighbor) {
            block->size = size + neighbor->size;
            block->next = neighbor->next;
            neighbor->next->prev = block;
            if (rover == neighbor) {
                rover = block;
            }
            heap->count--;
        }
        neighbor = block->prev;
        if (neighbor->size > 0 && (HeapBlock *)((u8 *)block - neighbor->size) == neighbor) {
            neighbor->size += block->size;
            next = block->next;
            neighbor->next = next;
            next->prev = neighbor;
            if (rover == block) {
                rover = neighbor;
            }
            heap->count--;
        }
        heap->rover = rover;
    }
}

void heapInit(Heap *heap, void *base, s32 size) {
    HeapBlock *first = (HeapBlock *)(((u32)base + 3) & ~3);

    size -= (u32)first - (u32)base;
    size &= ~3;
    heap->count = 1;
    heap->rover = first;
    heap->first = first;
    heap->end = (HeapBlock *)((u8 *)first + size);
    first->size = size;
    first->next = first;
    first->prev = first;
}

/* best fit: the free block that leaves the least over */
void *heapAllocBest(Heap *heap, s32 size) {
    HeapBlock *block = heap->first;
    HeapBlock *best = NULL;
    s32 bestLeft = 0x7FFFFFFF;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    do {
        left = block->size - size;
        if (left == 0) {
            goto exact;
        }
        if (left > 0 && left < bestLeft) {
            bestLeft = left;
            best = block;
        }
        block = block->next;
    } while (block != heap->first);
    if (best == NULL) {
        return NULL;
    }
    if (bestLeft <= HEAP_MIN_SPLIT) {
        goto whole;
    }
    /* the rest of the block becomes a free block after it */
    block = (HeapBlock *)((u8 *)best + size);
    heap->rover = block;
    heap->count++;
    block->size = bestLeft;
    best->size = -size;
    best->next->prev = block;
    block->next = best->next;
    block->prev = best;
    best->next = block;
    return best + 1;
exact:
    heap->rover = block->next;
    block->size = -block->size;
    return block + 1;
whole:
    best->size = -best->size;
    return best + 1;
}

/* the largest free block, from its end */
void *heapAllocLargest(Heap *heap, s32 size) {
    HeapBlock *block;
    HeapBlock *best = NULL;
    s32 bestLeft = 0;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    block = heap->first->prev;
    do {
        left = block->size - size;
        if (left == 0) {
            goto exact;
        }
        if (left > 0 && bestLeft < left) {
            bestLeft = left;
            best = block;
        }
        block = block->prev;
    } while (block != heap->first->prev);
    if (best == NULL) {
        return NULL;
    }
    if (bestLeft <= HEAP_MIN_SPLIT) {
        goto whole;
    }
    /* the block keeps its start, free, and hands out its end */
    block = (HeapBlock *)((u8 *)best + bestLeft);
    heap->rover = best;
    heap->count++;
    best->size = bestLeft;
    block->size = -size;
    best->next->prev = block;
    block->next = best->next;
    best->next = block;
    block->prev = best;
    return block + 1;
exact:
    heap->rover = block->prev;
    block->size = -block->size;
    return block + 1;
whole:
    best->size = -best->size;
    return best + 1;
}

/* Only matches when heap->rover is read again after the store, a fake form. */
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/heap", heapAllocNext);

/* Same as heapAllocNext: only the fake reload of heap->rover matches. */
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/heap", heapAllocPrev);

s32 heapTotalFree(Heap *heap) {
    HeapBlock *block = heap->rover;
    s32 total = 0;

    do {
        if (block->size > 0) {
            total += block->size;
        }
        block = block->next;
    } while (block != heap->rover);
    return total;
}

/* Shrinks the allocation at ptr to size bytes. What it gives back joins the
 * next block when that one is free and follows it, or else becomes a free
 * block of its own when it is big enough. */
void heapShrink(Heap *heap, void *ptr, s32 size) {
    HeapBlock *block = (HeapBlock *)ptr - 1;
    HeapBlock *rest;
    HeapBlock *next;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    rest = (HeapBlock *)((u8 *)block + size);
    next = block->next;
    left = -HEAP_SIZE_BEFORE(ptr) - size;
    if (block < next && next->size > 0) {
        if (heap->rover == next) {
            heap->rover = rest;
        }
        HEAP_SIZE_BEFORE(ptr) = -size;
        rest->size = left + block->next->size;
        rest->next = block->next->next;
        rest->next->prev = rest;
        rest->prev = block;
        block->next = rest;
    } else if (left > HEAP_MIN_SPLIT) {
        block->size = -size;
        rest->next = block->next;
        rest->next->prev = rest;
        rest->prev = block;
        block->next = rest;
        rest->size = left;
        heap->count++;
    }
}
