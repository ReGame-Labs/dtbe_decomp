#include "common.h"
#include "engine/system/main.h"
#include "engine/cd/cdfs.h"
#include "engine/cd/file_load.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/debug/system_menu.h"
#include "engine/game/game_state.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/lib/list.h"
#include "engine/lib/md5.h"
#include "engine/math/random.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"
#include "vtable.h"
#include "libetc.h"
#include "libsnd.h"
#include "memory.h"
#include "psyq.h"

/* what the random seed is the MD5 of */
#define RAM ((u8 *)0x80000000)
#define RAM_SEED_SIZE 0x10000
#define SCRATCHPAD ((u8 *)0x1F800000)
#define SCRATCHPAD_SIZE 0x400

/* Builds (initialize) or destroys the global objects: the scheduler, the
 * random numbers, the ordering table, the sound task, the fade task and the
 * game state. */
void initOrDestroyGlobals(s32 initialize, s32 priority) {
    SoundControl *sound;
    ListNode *seps;
    ListNode *vabs;

    if (priority == 0xFFFF) {
        if (initialize) {
            schedulerInit(&SCHEDULER);
            mersenneTwisterInit(&RANDOM);
            orderingTableInit(&FRAME_OT, 8);
            soundControlInit(&SOUND_CONTROL);
            fadeControlInit(&FADE_CONTROL);
            gameStateInit(&GAME_STATE);
            return;
        }
        taskDestroy(&FADE_CONTROL.task, DESTROY_BASES);
        /* the sound task's destructor, inlined: listRemove of both lists, in
         * the access order the game's compiler used */
        sound = &SOUND_CONTROL;
        seps = &sound->seps;
        sound->seps.next->prev = sound->seps.prev;
        seps->prev->next = sound->seps.next;
        sound->seps.next = seps;
        sound->seps.prev = seps;
        vabs = &sound->vabs;
        sound->vabs.next->prev = sound->vabs.prev;
        vabs->prev->next = sound->vabs.next;
        sound->vabs.next = vabs;
        sound->vabs.prev = vabs;
        /* SoundControl starts with a Task (see engine/sound/sound_control.h) */
        taskDestroy((Task *)sound, DESTROY_BASES);
        schedulerDestroy(&SCHEDULER, DESTROY_BASES);
    }
}

/* Builds the global objects. */
void initGlobals(void) {
    initOrDestroyGlobals(1, 0xFFFF);
}

/* Destroys the global objects. */
void destroyGlobals(void) {
    initOrDestroyGlobals(0, 0xFFFF);
}

/* Stops on a break instruction forever, which C cannot express. */
INCLUDE_ASM("asm/jp/main/nonmatchings/system/main", breakForever);

INCLUDE_RODATA("asm/jp/main/nonmatchings/system/main", STATIC_CONSTRUCTORS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/system/main", STATIC_DESTRUCTORS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/system/main", BUILD_NAME);

/* The game's entry point: starts the system, then runs the frames in a
 * thread. It passes $fp to func_800403C0, which needs inline asm, and splat
 * merged the thread's function (D_8001B090) into it. */
INCLUDE_ASM("asm/jp/main/nonmatchings/system/main", main);

/* Runs a frame: waits for the vertical blank, starts the next primitive
 * buffer and ordering table, reads the pads, services the CD, runs the tasks
 * and has the frame's ordering table drawn. */
void runFrame(void) {
    SYSTEM_CONTEXT.unk24 = displayWaitFrame(&DISPLAY);
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

    func_800383B4(&md5);
    MD5Update(&md5, RAM, RAM_SEED_SIZE);
    MD5Update(&md5, SCRATCHPAD, SCRATCHPAD_SIZE);
    /* the digest is read back as words */
    MD5Final(&md5, (u8 *)digest);
    mersenneTwisterSeed(random, digest[0] ^ digest[1] ^ digest[2] ^ digest[3]);
}

/* the file system startSystem opens, and the directory it reads it into */
char VFS_PATH[] = "/a.vfs";
char VFS_MOUNT_POINT[] = "/";

/* the size of the stack, which crt0 and initRamHeap leave out of the heap
 * (PsyQ's _stacksize) */
u32 STACK_SIZE = 0x800;
/* the size of the RAM, which crt0 puts the stack at the end of (PsyQ's
 * _ramsize) */
u32 RAM_SIZE = 0x200000;

/* Starts the system: the libraries, the CD file system and its allocator,
 * the display, the memory cards, the pads, the primitive buffers and the
 * entity heap; mounts /a.vfs and starts the sound. */
void startSystem(void) {
    VfsInfo info;
    s32 size;
    s32 i;

    func_8003F780();
    InitGeom();
    initCdfs(0);
    setVfsAllocator(allocForCdfs, freeForCdfs, NULL);
    displayStart(&DISPLAY);
    func_80058740(1);
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
void *allocForCdfs(void *this, s32 count, s32 size) {
    return mainHeapAllocPrev(size * count);
}

/* the memory release handed to the CD file system */
void freeForCdfs(void *this, void *ptr) {
    mainHeapFree(ptr);
}

/* Builds (initialize) or destroys the global objects of the display, the pads
 * and the handles. */
void initOrDestroyMainGlobals(s32 initialize, s32 priority) {
    if (priority == 0xFFFF) {
        if (initialize) {
            displayInit(&DISPLAY, 320, 240, 1);
            padManagerInit(&PAD_MANAGER);
            handleTableInit(&ENTITY_HANDLE_TABLE, 0x200);
            return;
        }
        handleTableDestroy(&ENTITY_HANDLE_TABLE, DESTROY_BASES);
        padManagerDestroy(&PAD_MANAGER, DESTROY_BASES);
    }
}

/* Builds the global objects of the display, the pads and the handles. */
void initMainGlobals(void) {
    initOrDestroyMainGlobals(1, 0xFFFF);
}

/* Destroys the global objects of the display, the pads and the handles. */
void destroyMainGlobals(void) {
    initOrDestroyMainGlobals(0, 0xFFFF);
}

/* Gives the heap the RAM from the end of the executable up to the stack. */
void initRamHeap(void) {
    u8 *base = BSS_END;
    u8 *end;

    /* D_1 is a symbol the linker put at 1: the test always passes */
    if (D_1 != NULL) {
        end = (u8 *)0x80200000 - STACK_SIZE;
    } else {
        end = (u8 *)(RAM_SIZE | 0x80000000) - STACK_SIZE;
    }
    mainHeapInit(base, end - base);
}
