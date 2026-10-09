#include "common.h"
#include "engine/lib/heap.h"

/* Returns the size the largest free block can hand out (its size without the header). */
s32 heapGetLargestFree(Heap *this) {
    HeapBlock *block = this->rover;
    s32 largest = 0;

    do {
        if (largest < block->size) {
            largest = block->size;
        }
        block = block->next;
    } while (block != this->rover);
    return largest - sizeof(HeapBlock);
}

/* Checks that the blocks are inside the heap, aligned, linked both ways and
 * as many as it counts: 0 if so, else -1. */
s32 heapCheck(Heap *this) {
    HeapBlock *block = this->rover;
    s32 count = this->count;
    HeapBlock *first = this->first;
    HeapBlock *end = this->end;
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
    } while (block != this->rover);
    if (count != 0) {
        return -1;
    }
    return 0;
}

/* Frees the allocation at ptr (nothing for NULL), merging its block with the
 * free blocks right before and after it. */
void heapFree(Heap *this, void *ptr) {
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
        rover = this->rover;
        if (neighbor->size > 0 && (HeapBlock *)((u8 *)block + size) == neighbor) {
            block->size = size + neighbor->size;
            block->next = neighbor->next;
            neighbor->next->prev = block;
            if (rover == neighbor) {
                rover = block;
            }
            this->count--;
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
            this->count--;
        }
        this->rover = rover;
    }
}

/* Sets up a heap in size bytes at base, aligned to words: one free block. */
void heapInit(Heap *this, void *base, s32 size) {
    HeapBlock *first = (HeapBlock *)(((u32)base + 3) & ~3);

    size -= (u32)first - (u32)base;
    size &= ~3;
    this->count = 1;
    this->rover = first;
    this->first = first;
    this->end = (HeapBlock *)((u8 *)first + size);
    first->size = size;
    first->next = first;
    first->prev = first;
}

/* Allocates size bytes from the best fit: the free block that leaves the
 * least over; its start is handed out. NULL when none is big enough. */
void *heapAllocBest(Heap *this, s32 size) {
    HeapBlock *block = this->first;
    HeapBlock *best = NULL;
    s32 bestLeft = 0x7FFFFFFF;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    do {
        left = block->size - size;
        if (left == 0) {
            this->rover = block->next;
            block->size = -block->size;
            return block + 1;
        }
        if (left > 0 && left < bestLeft) {
            bestLeft = left;
            best = block;
        }
        block = block->next;
    } while (block != this->first);
    if (best == NULL) {
        return NULL;
    }
    if (bestLeft <= HEAP_MIN_SPLIT) {
        best->size = -best->size;
        return best + 1;
    }
    /* the rest of the block becomes a free block after it */
    block = (HeapBlock *)((u8 *)best + size);
    this->rover = block;
    this->count++;
    block->size = bestLeft;
    best->size = -size;
    best->next->prev = block;
    block->next = best->next;
    block->prev = best;
    best->next = block;
    return best + 1;
}

/* Allocates size bytes from a free block that fits exactly, else from the
 * end of the largest one; searches backward from the last block. NULL when
 * none is big enough. */
void *heapAllocLargest(Heap *this, s32 size) {
    HeapBlock *block;
    HeapBlock *best = NULL;
    s32 bestLeft = 0;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    block = this->first->prev;
    do {
        left = block->size - size;
        if (left == 0) {
            this->rover = block->prev;
            block->size = -block->size;
            return block + 1;
        }
        if (left > 0 && bestLeft < left) {
            bestLeft = left;
            best = block;
        }
        block = block->prev;
    } while (block != this->first->prev);
    if (best == NULL) {
        return NULL;
    }
    if (bestLeft <= HEAP_MIN_SPLIT) {
        best->size = -best->size;
        return best + 1;
    }
    /* the block keeps its start, free, and hands out its end */
    block = (HeapBlock *)((u8 *)best + bestLeft);
    this->rover = best;
    this->count++;
    best->size = bestLeft;
    block->size = -size;
    best->next->prev = block;
    block->next = best->next;
    best->next = block;
    block->prev = best;
    return block + 1;
}

/*
 * Allocates size bytes from the first free block big enough, searching
 * forward from the rover: a block that fits exactly, or leaves no more than
 * HEAP_MIN_SPLIT, is taken whole; a bigger one is split, its start given
 * out and the rover left on the rest. NULL when the search comes back to the
 * rover. The exact fit is handled inside the loop: that keeps the load of
 * this->rover in it, as in the game.
 */
void *heapAllocNext(Heap *this, s32 size) {
    HeapBlock *block = this->rover;
    HeapBlock *rest;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    for (;;) {
        left = block->size - size;
        if (left == 0) {
            this->rover = block->next;
            block->size = -block->size;
            return block + 1;
        }
        if (left > 0) {
            break;
        }
        block = block->next;
        if (block == this->rover) {
            return NULL;
        }
    }
    if (left <= HEAP_MIN_SPLIT) {
        block->size = -block->size;
        return block + 1;
    }
    rest = (HeapBlock *)((u8 *)block + size);
    this->rover = rest;
    this->count++;
    rest->size = left;
    rest->prev = block;
    block->size = -size;
    block->next->prev = rest;
    rest->next = block->next;
    block->next = rest;
    return block + 1;
}

/* heapAllocNext searching backward, and giving out the end of a block it
 * splits, the rover left on the start. */
void *heapAllocPrev(Heap *this, s32 size) {
    HeapBlock *block = this->rover;
    HeapBlock *rest;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    for (;;) {
        left = block->size - size;
        if (left == 0) {
            this->rover = block->prev;
            block->size = -block->size;
            return block + 1;
        }
        if (left > 0) {
            break;
        }
        block = block->prev;
        if (block == this->rover) {
            return NULL;
        }
    }
    if (left <= HEAP_MIN_SPLIT) {
        block->size = -block->size;
        return block + 1;
    }
    rest = (HeapBlock *)((u8 *)block + left);
    this->rover = block;
    this->count++;
    rest->size = -size;
    rest->prev = block;
    block->size = left;
    block->next->prev = rest;
    rest->next = block->next;
    block->next = rest;
    return rest + 1;
}

/* Returns the size of the free blocks, headers included. */
s32 heapGetTotalFree(Heap *this) {
    HeapBlock *block = this->rover;
    s32 total = 0;

    do {
        if (block->size > 0) {
            total += block->size;
        }
        block = block->next;
    } while (block != this->rover);
    return total;
}

/* Shrinks the allocation at ptr to size bytes. What it gives back joins the
 * next block when that one is free and follows it, or else becomes a free
 * block of its own when it is big enough. */
void heapShrink(Heap *this, void *ptr, s32 size) {
    HeapBlock *block = (HeapBlock *)ptr - 1;
    HeapBlock *rest;
    HeapBlock *next;
    s32 left;

    size = HEAP_BLOCK_SIZE(size);
    rest = (HeapBlock *)((u8 *)block + size);
    next = block->next;
    left = -HEAP_SIZE_BEFORE(ptr) - size;
    if (block < next && next->size > 0) {
        if (this->rover == next) {
            this->rover = rest;
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
        this->count++;
    }
}
