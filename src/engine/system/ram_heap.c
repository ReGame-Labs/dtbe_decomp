#include "common.h"
#include "engine/system/ram_heap.h"
#include "engine/system/memory.h"

/* the size of the stack, which crt0 and initRamHeap leave out of the heap
 * (PsyQ's _stacksize) */
u32 STACK_SIZE = 0x800;
/* the size of the RAM, which crt0 puts the stack at the end of (PsyQ's
 * _ramsize) */
u32 RAM_SIZE = 0x200000;

/* Gives the heap the RAM from the end of the executable up to the stack. */
void initRamHeap(void) {
    u8 *base = BSS_END;
    u8 *end;

    /* D_1 is a symbol the linker put at 1: the test always passes */
    if (D_1 != NULL) {
        end = RAM_END - STACK_SIZE;
    } else {
        end = (u8 *)(RAM_SIZE | (u32)RAM) - STACK_SIZE;
    }
    mainHeapInit(base, end - base);
}
