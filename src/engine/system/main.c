#include "common.h"
#include "engine/system/main.h"
#include "engine/cd/cdfs.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/debug/system_menu.h"
#include "engine/game/sequencer.h"
#include "engine/gfx/display.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/lib/md5.h"
#include "engine/math/random.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "engine/system/ram_heap.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"
#include "kernel.h"
#include "libetc.h"
#include "libmcrd.h"
#include "psyq.h"
#include "vtable.h"

/* what the random seed is the MD5 of: the start of the RAM */
#define RAM_SEED_SIZE 0x10000
#define SCRATCHPAD ((u8 *)0x1F800000)
#define SCRATCHPAD_SIZE 0x400

/* The static constructors and destructors of the files, in link order, that
 * __main and crt0 run: psylink's tables, ahead of the files' .rodata. They are
 * here as the first .rodata the executable has (system/globals and
 * system/break_stub have none). */
INCLUDE_RODATA("main/nonmatchings/system/main", STATIC_CONSTRUCTORS);

INCLUDE_RODATA("main/nonmatchings/system/main", STATIC_DESTRUCTORS);

INCLUDE_RODATA("main/nonmatchings/system/main", BUILD_NAME);

/* The game's entry point: starts the system, loads the font and the system
 * TIMs, then runs the frames in a thread (runFramesThread). It passes $fp to
 * SetConf, which needs inline asm. */
INCLUDE_ASM("main/nonmatchings/system/main", main);

/* The function of main's thread: every frame, makes the sequencer again if
 * its task is gone, runs the frame and switches back to main's thread. */
void runFramesThread(Thread *thread) {
    for (;;) {
        if (handleTableGet(TASK_HANDLES, APP_SEQUENCER_HANDLE) == NULL) {
            AppSequencer *sequencer = appSequencerInit(operatorNew(sizeof(AppSequencer)), 0);

            schedulerInsertTask(&SCHEDULER, &sequencer->task);
        }
        runFrame();
        threadSwitchBack(thread, 0);
    }
}

/* Runs a frame: waits for the vertical blank, starts the next primitive
 * buffer and ordering table, reads the pads, services the CD, runs the tasks
 * and has the frame's ordering table drawn. */
void runFrame(void) {
    SYSTEM_CONTEXT.vsyncTime = displayWaitFrame(&DISPLAY);
    swapPrimBuffers();
    orderingTableClear(&FRAME_OT);
    padManagerUpdate(&PAD_MANAGER);
    updateCdfs();
    updateXa();
    /* the tasks get the system context as their update word */
    schedulerRunTasks(&SCHEDULER, (s32)&SYSTEM_CONTEXT);
    displaySetOt(&DISPLAY, FRAME_OT.head);
}

/* Seeds the random numbers with the MD5 of the start of the RAM and of the
 * scratchpad, whatever they hold at boot. */
void seedRandom(MersenneTwister *random) {
    Md5 md5;
    u32 digest[4];

    md5Init(&md5);
    MD5Update(&md5, RAM, RAM_SEED_SIZE);
    MD5Update(&md5, SCRATCHPAD, SCRATCHPAD_SIZE);
    /* the digest is read back as words */
    MD5Final(&md5, (u8 *)digest);
    mersenneTwisterSeed(random, digest[0] ^ digest[1] ^ digest[2] ^ digest[3]);
}

/* the file system startSystem opens, and the directory it reads it into */
char VFS_PATH[] = "/a.vfs";
char VFS_MOUNT_POINT[] = "/";

/* Starts the system: the libraries, the CD file system and its allocator,
 * the display, the memory cards, the pads, the primitive buffers and the
 * entity heap; mounts /a.vfs and starts the sound. */
void startSystem(void) {
    VfsInfo info;
    s32 size;
    s32 i;

    ResetCallback();
    InitGeom();
    initCdfs(0);
    setVfsAllocator(allocForCdfs, freeForCdfs, NULL);
    displayStart(&DISPLAY);
    MemCardInit(1);
    padManagerStart(&PAD_MANAGER);
    initPrimBuffers(PRIM_BUFFER_MEMORY, sizeof(PRIM_BUFFER_MEMORY));
    initEntityHeap(sizeof(ENTITY_HEAP_MEMORY), ENTITY_HEAP_MEMORY, &ENTITY_HANDLE_TABLE);
    /* reads the pads over two frames, so that they have a previous state */
    for (i = 1; i != -1; i--) {
        VSync(0);
        padManagerUpdate(&PAD_MANAGER);
    }
    size = loadVfsInfo(&info, VFS_PATH);
    if (size != 0) {
        mountVfs(VFS_MOUNT_POINT, mainHeapAllocLargest(size), &info);
    }
    soundControlStart(&SOUND_CONTROL);
}

/* Does nothing. */
void func_8001B314(void) {
}

/* the memory allocator handed to the CD file system */
void *allocForCdfs(void *opaque, s32 items, s32 size) {
    return mainHeapAllocPrev(size * items);
}

/* the memory release handed to the CD file system */
void freeForCdfs(void *opaque, void *ptr) {
    mainHeapFree(ptr);
}

/*
 * Builds (initialize) or destroys the global objects of the display, the pads
 * and the handles. It is g++'s __static_initialization_and_destruction_0 of
 * this file, written in C. The objects are in .bss and only this file uses
 * them, as g++ places file-scope static objects; main is still asm and .bss is
 * not split, so the file stays C.
 */
void initOrDestroyMainGlobals(s32 initialize, s32 priority) {
    if (priority == INIT_PRIORITY_DEFAULT) {
        if (initialize) {
            /* a frame every vertical blank */
            displayInit(&DISPLAY, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
            padManagerInit(&PAD_MANAGER);
            handleTableInit(&ENTITY_HANDLE_TABLE, ENTITY_HANDLE_CAPACITY);
            return;
        }
        handleTableDestroy(&ENTITY_HANDLE_TABLE, DESTROY_BASES);
        padManagerDestroy(&PAD_MANAGER, DESTROY_BASES);
    }
}

/* Builds the global objects of the display, the pads and the handles: g++'s
 * _GLOBAL_.I of this file, the second of STATIC_CONSTRUCTORS. */
void initMainGlobals(void) {
    initOrDestroyMainGlobals(1, INIT_PRIORITY_DEFAULT);
}

/* Destroys the global objects of the display, the pads and the handles: g++'s
 * _GLOBAL_.D of this file, the second of STATIC_DESTRUCTORS. */
void destroyMainGlobals(void) {
    initOrDestroyMainGlobals(0, INIT_PRIORITY_DEFAULT);
}
