#ifndef DTBE_SYSTEM_RAM_HEAP_H
#define DTBE_SYSTEM_RAM_HEAP_H

/* The RAM and the stack's sizes, the linker's symbols, and the main heap over the RAM between the executable and the stack. */

#include "common.h"

EXTERN_C_BEGIN

/* the RAM, where KSEG0 maps it */
#define RAM ((u8 *)0x80000000)
/* the end of the console's 2 MB of RAM */
#define RAM_END (RAM + 0x200000)

extern u32 STACK_SIZE; /* the size of the stack (PsyQ's _stacksize) */
extern u32 RAM_SIZE; /* the size of the RAM (PsyQ's _ramsize) */
/* the end of the executable's .bss, where the heap starts */
extern u8 BSS_END[];
/* a symbol the linker puts at address 1, so its address is never NULL */
extern u8 D_1[];

void initRamHeap(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_RAM_HEAP_H */
