#ifndef DTBE_SYSTEM_MAIN_H
#define DTBE_SYSTEM_MAIN_H

/* The game's entry point: starting the system, running the frames, the global objects. */

#include "common.h"
#include "engine/math/random.h"
#include "engine/system/handle_table.h"

EXTERN_C_BEGIN

extern char BUILD_NAME[]; /* "build0110291339", the build the game was made from */

/* the memory of the two primitive buffers (engine/gfx/prim_buffer.h) */
extern u8 PRIM_BUFFER_MEMORY[0x2D000];
/* the heap the entities are allocated from (engine/task/entity.h) */
extern u8 ENTITY_HEAP_MEMORY[0xC000];
/* a table of 0x200 handles, built by initOrDestroyMainGlobals */
extern HandleTable ENTITY_HANDLE_TABLE;

extern char VFS_PATH[]; /* "/a.vfs" */
extern char VFS_MOUNT_POINT[]; /* "/" */
extern u32 STACK_SIZE; /* the size of the stack (PsyQ's _stacksize) */
extern u32 RAM_SIZE; /* the size of the RAM (PsyQ's _ramsize) */
/* the end of the executable's .bss, where the heap starts */
extern u8 BSS_END[];

void initOrDestroyGlobals(s32 initialize, s32 priority);
void initGlobals(void);
void destroyGlobals(void);
void breakForever(void);
void main(void);
void runFrame(void);
void seedRandom(MersenneTwister *random);
void startSystem(void);
void func_8001B314(void);
void *allocForCdfs(void *context, s32 count, s32 size);
void freeForCdfs(void *context, void *ptr);
void initOrDestroyMainGlobals(s32 initialize, s32 priority);
void initMainGlobals(void);
void destroyMainGlobals(void);
void initRamHeap(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_MAIN_H */
