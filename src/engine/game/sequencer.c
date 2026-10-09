#include "common.h"
#include "engine/game/sequencer.h"
#include "engine/debug/system_menu.h"
#include "engine/game/game_state.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/task.h"
#include "libetc.h"
#include "memory.h"
#include "strings.h"
#include "overlay.h"

/* The scenes the sequencer runs (appSequencerRunScene), each a task built in an overlay. */

Task *func_8001C36C(void) {
    return func_80066D58(operatorNew(0x34), &SYSTEM_CONTEXT);
}

Task *createTitleScene(void) {
    return func_80067170(operatorNew(0x110C), &SYSTEM_CONTEXT);
}

Task *createCharacterSelectScene(void) {
    return func_8006AAF8(operatorNew(0x234), &SYSTEM_CONTEXT, D_8005FB80[0]);
}

Task *createFightScene(void) {
    return func_8006E1CC(operatorNew(0x64), &SYSTEM_CONTEXT);
}

Task *createMinigameGuideScene(void) {
    return func_80071F28(operatorNew(0xA90), &SYSTEM_CONTEXT, D_8005F92C[0]);
}

Task *createCreditsScene(void) {
    return func_80076504(operatorNew(0x44), &SYSTEM_CONTEXT);
}

Task *createRankingScene(void) {
    return func_8007360C(operatorNew(0x474), &SYSTEM_CONTEXT, D_8005FB90[0] == 0);
}

Task *createMovieScene(void) {
    return func_80064494(operatorNew(0x28));
}

Task *createMainMenuScene(void) {
    return func_8006A360(operatorNew(0x90), 0);
}

Task *createOptionScene(void) {
    return func_800794E0(operatorNew(0x8C));
}

Task *func_8001C554(void) {
    return func_800709B0(operatorNew(0x58), 2);
}

Task *createSaveScene(void) {
    return func_800709B0(operatorNew(0x58), 3);
}

/* Builds the sequencer. With randomFights, it only runs random fights. */
AppSequencer *appSequencerInit(AppSequencer *this, s32 randomFights) {
    taskInit(&this->task, APP_SEQUENCER_PRIORITY, "Application Sequencer");
    this->task.vtable = &APP_SEQUENCER_VTABLE;
    threadInit(&this->thread, runAppSequencerThread, APP_SEQUENCER_STACK_SIZE, this, 0);
    this->scene = 0;
    APP_SEQUENCER_HANDLE = this->task.handle;
    if (randomFights) {
        this->state = SEQUENCER_RANDOM_FIGHTS;
    } else {
        this->state = 0;
    }
    return this;
}

/* The entry of the sequencer's thread. */
void runAppSequencerThread(s32 unused, AppSequencer *this) {
    appSequencerRunScenes(this);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/sequencer", OVERLAY_PATH_FORMAT);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/sequencer", APP_SEQUENCER_VTABLE);

/* whether the game just started: the scene of func_8001C554 only runs then */
static s32 JUST_BOOTED = 1;

/* the overlays of the title, the movie and the game scenes */
char TITLE_OVERLAY[] = "title";
char MOVIE_OVERLAY[] = "movie";
char GAME_OVERLAY[] = "game";

/* The sequencer's thread: runs the scenes, state after state, for good. The
 * loop is a goto: as a for (;;), the check of the state that matches no case
 * branches elsewhere. */
void appSequencerRunScenes(AppSequencer *this) {
    soundControlUnloadVabs(&SOUND_CONTROL);
    soundControlUnloadSeps(&SOUND_CONTROL);
    stopMusic();
next:
    switch (this->state) {
    case 0:
        appSequencerRunScene(this, func_8001C36C, TITLE_OVERLAY, 0);
        if (JUST_BOOTED) {
            JUST_BOOTED = 0;
            appSequencerRunScene(this, func_8001C554, TITLE_OVERLAY, 0);
        }
        /* fall through */
    case 1:
        appSequencerRunScene(this, createMovieScene, MOVIE_OVERLAY, 0);
        /* fall through */
    case 2:
        if (appSequencerRunScene(this, createTitleScene, TITLE_OVERLAY, 1)) {
            func_8001E264(&GAME_STATE);
            appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0);
            appSequencerRunScene(this, createRankingScene, TITLE_OVERLAY, 1);
            this->state = 0;
            break;
        }
        /* fall through */
    case 3:
        func_8001D1C8(&GAME_STATE);
        switch (appSequencerRunScene(this, createMainMenuScene, TITLE_OVERLAY, 0)) {
        case 0:
            this->state = 4;
            break;
        case 1:
            this->state = 5;
            break;
        case 2:
            this->state = 6;
            break;
        case 3:
            this->state = 7;
            break;
        case 4:
            this->state = 8;
            break;
        case 5:
            this->state = 9;
            break;
        default:
            this->state = 2;
            break;
        }
        break;
    case 4:
        this->state = appSequencerRunVsComputer(this);
        break;
    case 5:
        this->state = appSequencerRunVersus(this, 0);
        break;
    case 6:
        this->state = appSequencerRunVersus(this, 1);
        break;
    case 7:
        this->state = appSequencerRunMinigameVsComputer(this);
        break;
    case 8:
        this->state = appSequencerRunMinigameVersus(this);
        break;
    case 9:
        this->state = appSequencerRunOptions(this);
        break;
    case SEQUENCER_RANDOM_FIGHTS:
        appSequencerRunRandomFights(this);
        break;
    }
    goto next;
}

/* Runs a run against the computer (gameStateSetUpVsComputer): the select scene
 * (createCharacterSelectScene), then the fights until the run ends, recording a cleared run
 * (gameStateRecordClear). Returns the state the sequencer runs next. */
s32 appSequencerRunVsComputer(AppSequencer *this) {
    s32 result;
    s32 next = 2;

start:
    gameStateSetUpVsComputer(&GAME_STATE);
    result = appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1);
    if (result < 0) {
        return 3;
    }
    if (result == 1) {
        func_8001DF70(&GAME_STATE);
    }
step:
    if (func_8001DF34(&GAME_STATE)) {
        if (appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0) == 5) {
            goto start;
        }
        gameStateRestoreFighters(&GAME_STATE);
    }
fight:
    switch (appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0)) {
    case 0: /* listed in the game: its jump table starts at 0 */
        break;
    case 1:
        appSequencerRunScene(this, createMinigameGuideScene, TITLE_OVERLAY, 1);
        goto fight;
    case 2:
        if (!GAME_STATE.unk2B8) {
            gameStateRecordClear(&GAME_STATE);
            next = 0;
            appSequencerRunScene(this, createCreditsScene, TITLE_OVERLAY, 0);
        }
        break;
    case 3:
        func_8001DF70(&GAME_STATE);
        while (appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1) < 0) {
        }
        goto step;
    case 4:
        goto fight;
    case 5:
        goto start;
    }
    if (!GAME_STATE.unk2B8) {
        appSequencerRunScene(this, createRankingScene, TITLE_OVERLAY, 1);
    }
    if (gameStateHasUnsavedProgress(&GAME_STATE)) {
        appSequencerRunScene(this, createSaveScene, TITLE_OVERLAY, 1);
    }
    return next;
}

/* Runs fights of two sides (gameStateSetUpVersus; arg1 gives side 2 to the
 * computer), back to the select scene after each, until it is left. The
 * fight loop keeps its head on top, as in the game, only with a goto. */
s32 appSequencerRunVersus(AppSequencer *this, s32 arg1) {
    s32 result;

    for (;;) {
        gameStateSetUpVersus(&GAME_STATE, arg1);
        result = appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1);
        if (result < 0) {
            break;
        }
        if (result == 1) {
            func_8001DF70(&GAME_STATE);
        }
    fight:
        if (GAME_STATE.unk2B8) {
            GAME_STATE.unk2B8 = 0;
            if (appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0) == 5) {
                continue;
            }
            gameStateRestoreFighters(&GAME_STATE);
        }
        if (appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0) != 3) {
            continue;
        }
        func_8001DF70(&GAME_STATE);
        while (appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1) < 0) {
        }
        goto fight;
    }
    return 3;
}

/* Runs fights set up by gameStateSetUpMinigameVsComputer until the select scene is left. */
s32 appSequencerRunMinigameVsComputer(AppSequencer *this) {
    s32 result;

    for (;;) {
        gameStateSetUpMinigameVsComputer(&GAME_STATE);
        result = appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1);
        if (result < 0) {
            break;
        }
        if (result == 1) {
            func_8001DF70(&GAME_STATE);
        }
        GAME_STATE.unk50 = GAME_STATE.arena - ARENA_RANDOM_COUNT;
        appSequencerRunScene(this, createMinigameGuideScene, TITLE_OVERLAY, 1);
        appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0);
    }
    return 3;
}

/* Runs fights set up by gameStateSetUpMinigameVersus until the select scene is left. */
s32 appSequencerRunMinigameVersus(AppSequencer *this) {
    for (;;) {
        gameStateSetUpMinigameVersus(&GAME_STATE);
        if (appSequencerRunScene(this, createCharacterSelectScene, TITLE_OVERLAY, 1) < 0) {
            break;
        }
        GAME_STATE.unk50 = GAME_STATE.arena - ARENA_RANDOM_COUNT;
        appSequencerRunScene(this, createMinigameGuideScene, TITLE_OVERLAY, 1);
        appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0);
    }
    return 3;
}

/* Runs the option scene, and the save scene if the progress changed. */
s32 appSequencerRunOptions(AppSequencer *this) {
    appSequencerRunScene(this, createOptionScene, TITLE_OVERLAY, 0);
    if (gameStateHasUnsavedProgress(&GAME_STATE)) {
        appSequencerRunScene(this, createSaveScene, TITLE_OVERLAY, 0);
    }
    return 3;
}

/* Runs random fights, forever. */
void appSequencerRunRandomFights(AppSequencer *this) {
    for (;;) {
        gameStateSetUpRandomFight(&GAME_STATE);
        appSequencerRunScene(this, createFightScene, GAME_OVERLAY, 0);
    }
}

/* Runs a scene: loads its overlay, behind the loading screen when asked, and
 * waits for its task to end. The game keeps the path buffer and the loading
 * screen, locals of two different blocks, in one stack slot, which g++ does
 * and gcc does not. Under cc1plus the C matches whole: a counter declared in
 * each for (the loading screen's counts 3 down to 0), and scene's handle
 * stored before schedulerInsertTask. This file can't be C++ as it is laid
 * out, though: APP_SEQUENCER_VTABLE sits in the middle of its .rodata, before
 * appSequencerRunScenes' jump tables, where g++ writes a class's vtable at
 * the end of the file. */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/sequencer", appSequencerRunScene);

/* The update of the sequencer task: resets the game on L1 R1 Select Start.
 * Once killed, it only forgets its scene when the scene's task is gone; else
 * it resumes its thread. */
void appSequencerUpdate(AppSequencer *this) {
    PadManager *pads;
    u32 held;
    u32 pressed;
    s32 port;

    if (this->task.killed) {
        if (handleTableGet(TASK_HANDLES, this->scene) == NULL) {
            this->scene = 0;
        }
        return;
    }
    if (!SYSTEM_CONTEXT.unk1C) {
        pads = PAD_MANAGER_INSTANCE;
        for (port = 0; port < 2; port++) {
            held = padManagerGetHeld(pads, port);
            pressed = padManagerGetPressed(pads, port);
            /* the buttons held since an earlier frame */
            held ^= pressed;
            if ((held & RESET_BUTTONS) == RESET_BUTTONS && (pressed & PADstart)) {
                taskKill(&this->task);
                break;
            }
        }
    }
    threadSwitchTo(&this->thread, 0);
}

/* Kills the scene running. Returns -1 if there was one, else 0. */
s32 appSequencerSurvives(AppSequencer *this) {
    s32 result = 0;

    if (this->scene) {
        killTaskByHandle(this->scene);
        result = -1;
    }
    return result;
}

/* Destroys the sequencer. */
void appSequencerDestroy(AppSequencer *this, s32 flags) {
    threadDestroy(&this->thread, 2);
    taskDestroy(&this->task, flags);
}
