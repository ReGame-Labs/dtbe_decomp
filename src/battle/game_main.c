/* The battle's main task ("game main process"): loads the stage, /bin/st%02d.bin, and runs the battle. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/cd/file_load.h"
#include "engine/cd/read.h"
#include "engine/debug/system_menu.h"
#include "engine/game/game_state.h"
#include "engine/gfx/display.h"
#include "engine/system/memory.h"
#include "engine/sound/sound_control.h"
#include "engine/task/task.h"
#include <libetc.h>
#include <stdio.h>

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
    /* 0x34 */ s32 stageSound; /* func_8006E948: by arena (D_80064464) */
    /* 0x38 */ struct {
        s32 unk0; /* func_8006E948: by character (D_80064478) */
        s32 unk4; /* func_8006E948: gameStateGetSound */
    } characterSounds[4]; /* by character of GAME_STATE (0x324, 0x33C and their evolutions) */
    /* 0x58 */ s32 fighterSounds[2]; /* func_8006E948: gameStateGetFighterSound, by side */
} Unk8006E2EC;

struct SystemContext; /* overlay.h: the context every overlay's main task gets (unused here) */
/* the task's virtual table */
extern Unk8006ECECVtable D_80064434;
void func_8006E310(s32 unused, void *object);
void func_8006ECA4(s32 arg);

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

/* where the stage overlay is loaded (the vram of the st*.bin overlays) */
extern u8 D_800D3820[];

/* the stage overlays' constructors, by arena */
Unk8006ED20 *func_800D3B74(void *memory);
Unk8006ED20 *func_800D3DBC(void *memory);
Unk8006ED20 *func_800D41E0(void *memory);
Unk8006ED20 *func_800D3E18(void *memory);
Unk8006ED20 *func_800D3AC4(void *memory);
Unk8006ED20 *func_800D3CEC(void *memory);
Unk8006ED20 *func_800D3C10(void *memory);
Unk8006ED20 *func_800D3B14(void *memory);
Unk8006ED20 *func_800D3AA8(void *memory);
Unk8006ED20 *func_800D3D58(void *memory);
void func_8006E948(Unk8006E2EC *this);

/* Starts a fight: clears the screen, loads the arena's overlay (/bin/st%02d.bin) and constructs its stage object
 * (kept at 0x24 and returned), then swaps the sounds of the last fight for the ones it picks for this one. */
Unk8006ED20 *func_8006E660(Unk8006E2EC *this) {
    char path[64];
    Unk8006ED20 *stage = NULL;
    s32 i;
    s32 j;

    displaySetClearColor(SYSTEM_CONTEXT.display, 0, 0, 0);
    displaySetVsyncMode(SYSTEM_CONTEXT.display, 2);
    displayEnable(SYSTEM_CONTEXT.display, 0);
    displayDisable(SYSTEM_CONTEXT.display, 4);
    sprintf(path, "/bin/st%02d.bin", GAME_STATE.arena);
    loadCompressedFileInto(D_800D3820, path);
    switch (GAME_STATE.arena) {
    case 0:
        stage = func_800D3B74(operatorNew(0x10C4));
        break;
    case 1:
        stage = func_800D3DBC(operatorNew(0xFD8));
        break;
    case 2:
        stage = func_800D41E0(operatorNew(0xD7C));
        break;
    case 3:
        stage = func_800D3E18(operatorNew(0xDD8));
        break;
    case 4:
        stage = func_800D3AC4(operatorNew(0x208C));
        break;
    case 5:
        stage = func_800D3CEC(operatorNew(0xF58));
        break;
    case 6:
        stage = func_800D3C10(operatorNew(0xD84));
        break;
    case 7:
        stage = func_800D3B14(operatorNew(0xFC4));
        break;
    case 8:
        stage = func_800D3AA8(operatorNew(0x1004));
        break;
    case 9:
        stage = func_800D3D58(operatorNew(0x15D8));
        break;
    }
    this->unk24 = stage;

    soundControlUnloadSound(&SOUND_CONTROL, this->stageSound);
    for (i = 0; i < 4; i++) {
        soundControlUnloadSound(&SOUND_CONTROL, this->characterSounds[i].unk0);
        soundControlUnloadSound(&SOUND_CONTROL, this->characterSounds[i].unk4);
    }
    soundControlUnloadSound(&SOUND_CONTROL, this->fighterSounds[0]);
    soundControlUnloadSound(&SOUND_CONTROL, this->fighterSounds[1]);
    gameStatePickSounds(&GAME_STATE);
    gameStatePickFighterSounds(&GAME_STATE);
    func_8006E948(this);
    soundControlLoadSound(&SOUND_CONTROL, this->stageSound, 1);
    /* The match depends on this loop having a counter of its own (with i the two counters share a register). */
    for (j = 0; j < 4; j++) {
        soundControlLoadSound(&SOUND_CONTROL, this->characterSounds[j].unk0, 1);
        soundControlLoadSound(&SOUND_CONTROL, this->characterSounds[j].unk4, 1);
    }
    soundControlLoadSound(&SOUND_CONTROL, this->fighterSounds[0], 1);
    soundControlLoadSound(&SOUND_CONTROL, this->fighterSounds[1], 1);
    return stage;
}

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
