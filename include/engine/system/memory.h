#ifndef DTBE_SYSTEM_MEMORY_H
#define DTBE_SYSTEM_MEMORY_H

/* The main heap and the debug heap, and the global `new` and `delete` on the main heap. */

#include "common.h"
#include <libgpu.h>
#include "engine/lib/heap.h"

EXTERN_C_BEGIN

/* the heaps of debugHeapAlloc and of mainHeapInit, mainHeapAllocBest and the other main heap functions */
extern Heap DEBUG_HEAP;
extern Heap MAIN_HEAP;

/* "Not enough memory. %dBytes.\n" */
extern char *OUT_OF_MEMORY_FORMAT[];
/* the display die shows its message on */
extern DISPENV DIE_DISPENV;

void die(const char *fmt, ...);

void debugHeapInit(void *base, s32 size);
void *debugHeapAlloc(s32 size);
void debugHeapFree(void *ptr);
void mainHeapInit(void *base, u32 size);
void *mainHeapAllocBest(s32 size);
void *mainHeapAllocLargest(s32 size);
void *mainHeapAllocPrev(s32 size);
void mainHeapFree(void *ptr);
u32 mainHeapGetLargestFree(void);
s32 mainHeapGetBlockCount(void);
s32 mainHeapGetTotalFree(void);
void mainHeapShrink(void *ptr, s32 size);
/* the global operator new, which g++ 2.95 calls __builtin_new */
void *operatorNew(s32 size) __asm__("__builtin_new");
/* the global operator delete, which g++ 2.95 calls __builtin_delete */
void operatorDelete(void *ptr) __asm__("__builtin_delete");
/* the global operator new[], libgcc's (it only calls new), which g++ 2.95
 * calls __builtin_vec_new */
void *operatorVecNew(s32 size) __asm__("__builtin_vec_new");
/* the global operator delete[], which g++ 2.95 calls __builtin_vec_delete */
void operatorVecDelete(void *ptr) __asm__("__builtin_vec_delete");

EXTERN_C_END

#endif /* DTBE_SYSTEM_MEMORY_H */
