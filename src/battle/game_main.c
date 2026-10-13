/* The battle's main task ("game main process"): loads the stage, /bin/st%02d.bin, and runs the battle. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/cd/read.h"
#include "engine/sound/sound_control.h"
#include "engine/task/task.h"
#include <libetc.h>

/* The task of virtual table D_80064434: its update stores the argument at
 * 0x20 and calls the task's own first virtual method (at 0x20). title.bin has
 * six tasks with the same update (Unk8006713C there). */
typedef struct Unk8006ECECVtable {
    /* 0x00 */ TaskVtable task;
    /* 0x20 */ VtableEntry unk20;
} Unk8006ECECVtable;

typedef struct Unk8006ECEC {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ Unk8006ECECVtable *vtable;
    /* 0x20 */ s32 unk20; /* the argument of the last update */
} Unk8006ECEC;
void func_8006E330(void *object);

/* gameMainTaskInit's task (virtual table D_80064434), as its first virtual method sees it: a thread at 0x28. */
typedef struct Unk8006E2EC {
    /* 0x00 */ Unk8006ECEC base;
    /* 0x24 */ Unk8006ED20 *unk24; /* deleted by the destructor (func_8006E23C) */
    /* 0x28 */ Thread thread;
} Unk8006E2EC;

struct SystemContext; /* overlay.h: the context every overlay's main task gets (unused here) */
/* the task's virtual table */
extern Unk8006ECECVtable D_80064434;
void func_8006E310(s32 unused, void *object);
void func_8006ECA4(s32 arg);

INCLUDE_RODATA("game/nonmatchings/game_main", D_800643E0);

/* Builds the battle's main task: a task with a thread of its own, and
 * func_8006ECA4 as the CD's idle callback. */
Task *gameMainTaskInit(Unk8006E2EC *this, struct SystemContext *context) {
    taskInit((Task *)this, 0x2000, "game main process");
    this->base.vtable = &D_80064434;
    threadInit(&this->thread, func_8006E310, 0x2000, this, 0);
    setCdfsIdle(func_8006ECA4, 0);
    this->unk24 = NULL;
    return (Task *)this;
}

/* Destroys the battle's main task: deletes the object at 0x24, if any, unloads the sounds and stops the music. */
void func_8006E23C(Unk8006E2EC *this, s32 flags) {
    Unk8006ED20 *object;

    this->base.vtable = &D_80064434;
    setCdfsIdle(NULL, 0);
    soundControlUnloadVabs(&SOUND_CONTROL);
    soundControlUnloadSeps(&SOUND_CONTROL);
    object = this->unk24;
    if (object != NULL) {
        /* its deleting destructor */
        object->vtable[13].func((u8 *)object + object->vtable[13].delta, DESTROY_DELETE);
    }
    stopMusic();
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy((Task *)this, flags);
}

/* The first virtual method of D_80064434 (at 0x20): switches to the task's thread with 0. */
void func_8006E2EC(Unk8006E2EC *this) {
    threadSwitchTo(&this->thread, 0);
}

/* The entry of the thread of gameMainTaskInit's object. */
void func_8006E310(s32 unused, void *object) {
    func_8006E330(object);
}

INCLUDE_ASM("game/nonmatchings/game_main", func_8006E330);

/* Shows the loading screen of virtual table D_8006DF3C for three frames of the task's thread. */
void func_8006E600(Unk8006E2EC *this) {
    LoadingScreen loading;
    s32 i;

    loadingScreenInit(&loading);
    loading.vtable = &D_8006DF3C;
    for (i = 2; i >= 0; i--) {
        loadingScreenUpdate(&loading);
        threadSwitchBack(&this->thread, 0);
    }
}

INCLUDE_ASM("game/nonmatchings/game_main", func_8006E660);

INCLUDE_ASM("game/nonmatchings/game_main", func_8006E948);

INCLUDE_ASM("game/nonmatchings/game_main", func_8006EA64);

INCLUDE_ASM("game/nonmatchings/game_main", func_8006EBB0);

/* a frame counter, modulo 16, of the CD's idle callback */
extern s32 D_800D32B4;
void func_8006EBB0(s32 frame);

/* The CD's idle callback (gameMainTaskInit): runs func_8006EBB0 on a frame counter that wraps at 16, then waits for
 * the vertical blank. */
void func_8006ECA4(s32 arg) {
    func_8006EBB0(D_800D32B4++);
    D_800D32B4 &= 0xF;
    VSync(0);
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8006ECEC(Unk8006ECEC *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_RODATA("game/nonmatchings/game_main", D_80064434);
INCLUDE_RODATA("game/nonmatchings/game_main", D_80064464);
INCLUDE_RODATA("game/nonmatchings/game_main", D_80064478);
INCLUDE_RODATA("game/nonmatchings/game_main", D_800644A8);
INCLUDE_RODATA("game/nonmatchings/game_main", D_800644C8);
