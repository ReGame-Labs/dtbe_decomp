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

/* The task of virtual table GAME_MAIN_TASK_VTABLE: its update stores the argument at
 * 0x20 and calls the task's own first virtual method (at 0x20), its tick. title.bin has
 * six tasks with the same update (TickTask there). */
typedef struct Unk8006ECECVtable {
    /* 0x00 */ TaskVtable task;
    /* 0x20 */ VtableEntry tick;
} Unk8006ECECVtable;

typedef struct Unk8006ECEC {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ Unk8006ECECVtable *vtable;
    /* 0x20 */ s32 updateArg; /* the argument of the last update */
} Unk8006ECEC;
void GameMainTask_run(void *task);

/* gameMainTaskInit's task (GAME_MAIN_TASK_VTABLE), as its first virtual method sees it: a thread at 0x28. */
typedef struct GameMainTask {
    /* 0x00 */ Unk8006ECEC base;
    /* 0x24 */ Fight *fight; /* the stage's, deleted by the destructor */
    /* 0x28 */ Thread thread;
    /* 0x34 */ s32 stageSound; /* GameMainTask_pickSounds: by arena (ARENA_SOUND_IDS) */
    /* 0x38 */ struct {
        s32 unk0; /* GameMainTask_pickSounds: by character (CHARACTER_SOUND_IDS) */
        s32 unk4; /* GameMainTask_pickSounds: gameStateGetSound */
    } characterSounds[4]; /* by character of GAME_STATE (0x324, 0x33C and their evolutions) */
    /* 0x58 */ s32 fighterSounds[2]; /* GameMainTask_pickSounds: gameStateGetFighterSound, by side */
} GameMainTask;

struct SystemContext; /* overlay.h: the context every overlay's main task gets (unused here) */
/* the task's virtual table */
extern Unk8006ECECVtable GAME_MAIN_TASK_VTABLE;
void runGameMainThread(s32 unused, void *task);
void onCdIdle(s32 arg);

/* Builds the battle's main task: a task with a thread of its own, and
 * onCdIdle as the CD's idle callback. */
Task *gameMainTaskInit(GameMainTask *this, struct SystemContext *context) {
    taskInit((Task *)this, 0x2000, "game main process");
    this->base.vtable = &GAME_MAIN_TASK_VTABLE;
    threadInit(&this->thread, runGameMainThread, 0x2000, this, 0);
    setCdfsIdle(onCdIdle, 0);
    this->fight = NULL;
    return (Task *)this;
}

/* Destroys the battle's main task: deletes the fight, if any, unloads the sounds and stops the music. */
void GameMainTask_destroy(GameMainTask *this, s32 flags) {
    Fight *fight;

    this->base.vtable = &GAME_MAIN_TASK_VTABLE;
    setCdfsIdle(NULL, 0);
    soundControlUnloadVabs(&SOUND_CONTROL);
    soundControlUnloadSeps(&SOUND_CONTROL);
    fight = this->fight;
    if (fight != NULL) {
        /* its deleting destructor */
        fight->vtable[13].func((u8 *)fight + fight->vtable[13].delta, DESTROY_DELETE);
    }
    stopMusic();
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy((Task *)this, flags);
}

/* The tick (the method at 0x20): runs the task's thread for a frame. */
void GameMainTask_tick(GameMainTask *this) {
    threadSwitchTo(&this->thread, 0);
}

/* The entry of the battle's main task's thread. */
void runGameMainThread(s32 unused, void *task) {
    GameMainTask_run(task);
}

INCLUDE_ASM("game/nonmatchings/game_main", GameMainTask_run);

/* Shows the loading screen of virtual table FIGHT_LOADING_SCREEN_VTABLE for three frames of the task's thread. */
void GameMainTask_showLoading(GameMainTask *this) {
    LoadingScreen loading;
    s32 i;

    loadingScreenInit(&loading);
    loading.vtable = &FIGHT_LOADING_SCREEN_VTABLE;
    for (i = 2; i >= 0; i--) {
        loadingScreenUpdate(&loading);
        threadSwitchBack(&this->thread, 0);
    }
}

/* where the stage overlay is loaded (the vram of the st*.bin overlays) */
extern u8 STAGE_OVERLAY_MEMORY[];

/* the stage overlays' constructors, by arena */
Fight *StageMachine_init(void *memory);
Fight *StageNature_init(void *memory);
Fight *StageTurn_init(void *memory);
Fight *StageShrine_init(void *memory);
Fight *StageIceberg_init(void *memory);
Fight *StageVolcano_init(void *memory);
Fight *StageBoss_init(void *memory);
Fight *StageBallShoot_init(void *memory);
Fight *StageEvolution_init(void *memory);
Fight *StageJewelHunt_init(void *memory);
void GameMainTask_pickSounds(GameMainTask *this);

/* Starts a fight: clears the screen, loads the arena's overlay (/bin/st%02d.bin) and constructs its stage object (kept
 * as the task's fight and returned), then swaps the sounds of the last fight for the ones it picks for this one. */
Fight *GameMainTask_startFight(GameMainTask *this) {
    char path[64];
    Fight *stage = NULL;
    s32 i;
    s32 j;

    displaySetClearColor(SYSTEM_CONTEXT.display, 0, 0, 0);
    displaySetVsyncMode(SYSTEM_CONTEXT.display, 2);
    displayEnable(SYSTEM_CONTEXT.display, 0);
    displayDisable(SYSTEM_CONTEXT.display, 4);
    sprintf(path, "/bin/st%02d.bin", GAME_STATE.arena);
    loadCompressedFileInto(STAGE_OVERLAY_MEMORY, path);
    switch (GAME_STATE.arena) {
    case 0:
        stage = StageMachine_init(operatorNew(0x10C4));
        break;
    case 1:
        stage = StageNature_init(operatorNew(0xFD8));
        break;
    case 2:
        stage = StageTurn_init(operatorNew(0xD7C));
        break;
    case 3:
        stage = StageShrine_init(operatorNew(0xDD8));
        break;
    case 4:
        stage = StageIceberg_init(operatorNew(0x208C));
        break;
    case 5:
        stage = StageVolcano_init(operatorNew(0xF58));
        break;
    case 6:
        stage = StageBoss_init(operatorNew(0xD84));
        break;
    case 7:
        stage = StageBallShoot_init(operatorNew(0xFC4));
        break;
    case 8:
        stage = StageEvolution_init(operatorNew(0x1004));
        break;
    case 9:
        stage = StageJewelHunt_init(operatorNew(0x15D8));
        break;
    }
    this->fight = stage;

    soundControlUnloadSound(&SOUND_CONTROL, this->stageSound);
    for (i = 0; i < 4; i++) {
        soundControlUnloadSound(&SOUND_CONTROL, this->characterSounds[i].unk0);
        soundControlUnloadSound(&SOUND_CONTROL, this->characterSounds[i].unk4);
    }
    soundControlUnloadSound(&SOUND_CONTROL, this->fighterSounds[0]);
    soundControlUnloadSound(&SOUND_CONTROL, this->fighterSounds[1]);
    gameStatePickSounds(&GAME_STATE);
    gameStatePickFighterSounds(&GAME_STATE);
    GameMainTask_pickSounds(this);
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

/* the sound of each arena and of each character, by GAME_STATE's arena and character */
extern s16 ARENA_SOUND_IDS[];
extern s16 CHARACTER_SOUND_IDS[];

/* Picks the fight's sounds: the arena's, two for each of the four characters that can show up (the two fighters
 * and their evolutions: CHARACTER_SOUND_IDS and gameStateGetSound) and one for each side's fighter. The match
 * depends on &GAME_STATE kept in a local (one saved register for all its uses). */
void GameMainTask_pickSounds(GameMainTask *this) {
    GameState *state = &GAME_STATE;
    s32 characters[4];
    s32 i;

    this->stageSound = ARENA_SOUND_IDS[state->arena];
    characters[0] = state->fighters[0].character;
    characters[1] = state->fighters[1].character;
    characters[2] = CHARACTER_EVOLUTIONS[characters[0]];
    characters[3] = CHARACTER_EVOLUTIONS[characters[1]];
    for (i = 0; i < 4; i++) {
        this->characterSounds[i].unk0 = CHARACTER_SOUND_IDS[characters[i]];
        this->characterSounds[i].unk4 = gameStateGetSound(state, characters[i]);
    }
    this->fighterSounds[0] = gameStateGetFighterSound(state, 0);
    this->fighterSounds[1] = gameStateGetFighterSound(state, 1);
}

INCLUDE_ASM("game/nonmatchings/game_main", GameMainTask_stepFight);

INCLUDE_ASM("game/nonmatchings/game_main", drawLoadingIcon);

/* a frame counter, modulo 16, of the CD's idle callback */
extern s32 LOADING_ICON_FRAME;
void drawLoadingIcon(s32 frame);

/* The CD's idle callback (gameMainTaskInit): runs drawLoadingIcon on a frame counter that wraps at 16, then waits for
 * the vertical blank. */
void onCdIdle(s32 arg) {
    drawLoadingIcon(LOADING_ICON_FRAME++);
    LOADING_ICON_FRAME &= 0xF;
    VSync(0);
}

/* Update: keeps arg and runs the task's own tick. */
void GameMainTask_update(Unk8006ECEC *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

INCLUDE_RODATA("game/nonmatchings/game_main", GAME_MAIN_TASK_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_main", ARENA_SOUND_IDS);
INCLUDE_RODATA("game/nonmatchings/game_main", CHARACTER_SOUND_IDS);
INCLUDE_RODATA("game/nonmatchings/game_main", LOADING_ICON_PALETTE);
INCLUDE_RODATA("game/nonmatchings/game_main", LOADING_ICON_FRAMES);
