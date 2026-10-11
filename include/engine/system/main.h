#ifndef DTBE_SYSTEM_MAIN_H
#define DTBE_SYSTEM_MAIN_H

/* The game's entry point: starting the system, running the frames; the display, the pads and the entity handles. */

#include "common.h"
#include "engine/math/random.h"
#include "engine/system/handle_table.h"
#include "engine/system/thread.h"

EXTERN_C_BEGIN

extern char BUILD_NAME[]; /* "build0110291339", the build the game was made from */

/* the memory of the two primitive buffers (engine/gfx/prim_buffer.h) */
extern u8 PRIM_BUFFER_MEMORY[0x2D000];
/* the heap the entities are allocated from (engine/task/entity.h) */
extern u8 ENTITY_HEAP_MEMORY[0xC000];
/* the handles of the entities (engine/task/entity.h's ENTITY_HANDLES), built
 * by initOrDestroyMainGlobals */
extern HandleTable ENTITY_HANDLE_TABLE;
/* how many handles ENTITY_HANDLE_TABLE holds */
#define ENTITY_HANDLE_CAPACITY 0x200

extern char VFS_PATH[]; /* "/a.vfs" */
extern char VFS_MOUNT_POINT[]; /* "/" */

void main(void);
void runFramesThread(Thread *thread);
void runFrame(void);
void seedRandom(MersenneTwister *random);
void startSystem(void);
void func_8001B314(void);
void *allocForCdfs(void *opaque, s32 items, s32 size);
void freeForCdfs(void *opaque, void *ptr);
void initOrDestroyMainGlobals(s32 initialize, s32 priority);
void initMainGlobals(void);
void destroyMainGlobals(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_MAIN_H */
