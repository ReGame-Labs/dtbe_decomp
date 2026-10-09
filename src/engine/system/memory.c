#include "common.h"
#include "engine/system/memory.h"
#include "engine/lib/heap.h"
#include "stdio.h"

/* die's message when the main heap is out of memory, which OUT_OF_MEMORY_FORMAT points
 * at */
const char STR_NOT_ENOUGH_MEMORY[] = "Not enough memory. %dBytes.\n";

/* Sets up the heap DEBUG_HEAP in size bytes at base. */
void debugHeapInit(void *base, s32 size) {
    heapInit(&DEBUG_HEAP, base, size);
}

/* Allocates from the heap DEBUG_HEAP, crashing when it is full. */
void *debugHeapAlloc(s32 size) {
    void *ptr;

    heapCheck(&DEBUG_HEAP);
    ptr = heapAllocLargest(&DEBUG_HEAP, size);
    if (ptr == NULL) {
        /* crash on purpose: an unaligned store */
        *(s32 *)1 = 1;
    }
    return ptr;
}

/* Frees into the heap DEBUG_HEAP, checking it before and after. */
void debugHeapFree(void *ptr) {
    heapCheck(&DEBUG_HEAP);
    heapFree(&DEBUG_HEAP, ptr);
    heapCheck(&DEBUG_HEAP);
}

/* Sets up the main heap in size bytes at base and logs it. */
void mainHeapInit(void *base, u32 size) {
    heapInit(&MAIN_HEAP, base, size);
    printf("Heap: %d KBytes (%08X-%08X)\n", size >> 10, (u32)base, (u32)base + size);
}

/* Allocates from the main heap, the best fit; dies when it is full. */
void *mainHeapAllocBest(s32 size) {
    void *ptr = heapAllocBest(&MAIN_HEAP, size);

    if (ptr == NULL) {
        die(OUT_OF_MEMORY_FORMAT[0], heapGetLargestFree(&MAIN_HEAP) - size);
    }
    return ptr;
}

/* Allocates from the main heap's largest free block; dies when it is full. */
void *mainHeapAllocLargest(s32 size) {
    void *ptr = heapAllocLargest(&MAIN_HEAP, size);

    if (ptr == NULL) {
        die(OUT_OF_MEMORY_FORMAT[0], heapGetLargestFree(&MAIN_HEAP) - size);
    }
    return ptr;
}

/* Allocates from the main heap with heapAllocPrev; dies when it is full. */
void *mainHeapAllocPrev(s32 size) {
    void *ptr = heapAllocPrev(&MAIN_HEAP, size);

    if (ptr == NULL) {
        die(OUT_OF_MEMORY_FORMAT[0], heapGetLargestFree(&MAIN_HEAP) - size);
    }
    return ptr;
}

/* Frees into the main heap. */
void mainHeapFree(void *ptr) {
    heapFree(&MAIN_HEAP, ptr);
}

/* Returns the size of the main heap's largest free block. */
u32 mainHeapGetLargestFree(void) {
    return heapGetLargestFree(&MAIN_HEAP);
}

/* Returns the number of blocks of the main heap. */
s32 mainHeapGetBlockCount(void) {
    return MAIN_HEAP.count;
}

/* Returns the free bytes of the main heap. */
s32 mainHeapGetTotalFree(void) {
    return heapGetTotalFree(&MAIN_HEAP);
}

/* Shrinks an allocation of the main heap to size bytes. */
void mainHeapShrink(void *ptr, s32 size) {
    heapShrink(&MAIN_HEAP, ptr, size);
}

/* Allocates from the main heap, the best fit, as `new` does; dies when it is full. */
void *operatorNew(s32 size) {
    void *ptr = heapAllocBest(&MAIN_HEAP, size);

    if (ptr == NULL) {
        die(OUT_OF_MEMORY_FORMAT[0], heapGetLargestFree(&MAIN_HEAP) - size);
    }
    return ptr;
}

/* Frees into the main heap, as `delete` does. */
void operatorDelete(void *ptr) {
    heapFree(&MAIN_HEAP, ptr);
}

/* Frees an array into the main heap, as `delete[]` does. */
void operatorVecDelete(void *ptr) {
    heapFree(&MAIN_HEAP, ptr);
}

/* Stops the game on an error: shows the build, the address die was called
 * from and the message (a printf format) on a cleared screen, then crashes on
 * purpose. It reads its return address (move s1, ra), which only inline asm
 * gives, so it stays assembly. */
INCLUDE_ASM("asm/jp/main/nonmatchings/system/memory", die);
