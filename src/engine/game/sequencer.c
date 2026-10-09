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
#include "vtable.h"
#include "overlay.h"

/* The scenes the sequencer runs (appSequencerRunScene), each a task built in
 * an overlay; the names in quotes are the tasks'. */

/* Creates the boot logo ("Boot Logo"). */
Task *createBootLogoTask(void) {
    return func_80066D58(operatorNew(0x34), &SYSTEM_CONTEXT);
}

/* Creates the title ("Title Logo"); a result other than 0 starts the demo. */
Task *createTitleTask(void) {
    return func_80067170(operatorNew(0x110C), &SYSTEM_CONTEXT);
}

/* Creates the select scene ("Character Select"): the characters, and the
 * arena or the bonus game, as GameState.selectFlags says. */
Task *createCharacterSelectTask(void) {
    return func_8006AAF8(operatorNew(0x234), &SYSTEM_CONTEXT, GAME_STATE.selectFlags);
}

/* Creates the fight scene ("game main process"): fights and bonus games; its
 * result is a FIGHT_RESULT_*. */
Task *createFightTask(void) {
    return func_8006E1CC(operatorNew(0x64), &SYSTEM_CONTEXT);
}

/* Creates the guide of the bonus game ("Guide Logo"). */
Task *createBonusGuideTask(void) {
    return func_80071F28(operatorNew(0xA90), &SYSTEM_CONTEXT, GAME_STATE.bonusGame);
}

/* Creates the credits. */
Task *createCreditsTask(void) {
    return func_80076504(operatorNew(0x44), &SYSTEM_CONTEXT);
}

/* Creates the ranking; its argument is 0 after the demo. */
Task *createRankingTask(void) {
    return func_8007360C(operatorNew(0x474), &SYSTEM_CONTEXT, !GAME_STATE.demo);
}

/* Creates the movie scene, which plays /a.str. */
Task *createMovieTask(void) {
    return func_80064494(operatorNew(0x28));
}

/* Creates the main menu ("MainMenu"): its result is the choice (0 to 5), or
 * another to go back. */
Task *createMainMenuTask(void) {
    return func_8006A360(operatorNew(0x90), 0);
}

/* Creates the options ("Option"). */
Task *createOptionTask(void) {
    return func_800794E0(operatorNew(0x8C));
}

/* Creates the memory card scene that loads the save. */
Task *createLoadTask(void) {
    return func_800709B0(operatorNew(0x58), MEMORY_CARD_LOAD);
}

/* Creates the memory card scene that saves. */
Task *createSaveTask(void) {
    return func_800709B0(operatorNew(0x58), MEMORY_CARD_SAVE);
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
        this->state = SEQUENCER_BOOT;
    }
    return this;
}

/* The entry of the sequencer's thread. */
void runAppSequencerThread(s32 unused, AppSequencer *this) {
    appSequencerRunScenes(this);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/sequencer", OVERLAY_PATH_FORMAT);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/sequencer", APP_SEQUENCER_VTABLE);

/* whether the game just started: the save is only loaded then */
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
    case SEQUENCER_BOOT:
        appSequencerRunScene(this, createBootLogoTask, TITLE_OVERLAY, 0);
        if (JUST_BOOTED) {
            JUST_BOOTED = 0;
            appSequencerRunScene(this, createLoadTask, TITLE_OVERLAY, 0);
        }
        /* fall through */
    case SEQUENCER_MOVIE:
        appSequencerRunScene(this, createMovieTask, MOVIE_OVERLAY, 0);
        /* fall through */
    case SEQUENCER_TITLE:
        if (appSequencerRunScene(this, createTitleTask, TITLE_OVERLAY, 1)) {
            gameStateSetUpDemo(&GAME_STATE);
            appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0);
            appSequencerRunScene(this, createRankingTask, TITLE_OVERLAY, 1);
            this->state = SEQUENCER_BOOT;
            break;
        }
        /* fall through */
    case SEQUENCER_MAIN_MENU:
        gameStateResetLastPicks(&GAME_STATE);
        switch (appSequencerRunScene(this, createMainMenuTask, TITLE_OVERLAY, 0)) {
        case 0:
            this->state = SEQUENCER_VS_COMPUTER;
            break;
        case 1:
            this->state = SEQUENCER_VERSUS;
            break;
        case 2:
            this->state = SEQUENCER_VERSUS_COMPUTER;
            break;
        case 3:
            this->state = SEQUENCER_BONUS_VS_COMPUTER;
            break;
        case 4:
            this->state = SEQUENCER_BONUS_VERSUS;
            break;
        case 5:
            this->state = SEQUENCER_OPTIONS;
            break;
        default:
            this->state = SEQUENCER_TITLE;
            break;
        }
        break;
    case SEQUENCER_VS_COMPUTER:
        this->state = appSequencerRunVsComputer(this);
        break;
    case SEQUENCER_VERSUS:
        this->state = appSequencerRunVersus(this, 0);
        break;
    case SEQUENCER_VERSUS_COMPUTER:
        this->state = appSequencerRunVersus(this, 1);
        break;
    case SEQUENCER_BONUS_VS_COMPUTER:
        this->state = appSequencerRunBonusVsComputer(this);
        break;
    case SEQUENCER_BONUS_VERSUS:
        this->state = appSequencerRunBonusVersus(this);
        break;
    case SEQUENCER_OPTIONS:
        this->state = appSequencerRunOptions(this);
        break;
    case SEQUENCER_RANDOM_FIGHTS:
        appSequencerRunRandomFights(this);
        break;
    }
    goto next;
}

/* Runs a run against the computer (gameStateSetUpVsComputer): the select scene
 * (createCharacterSelectTask), then the fights until the run ends, recording a cleared run
 * (gameStateRecordClear). Returns the state the sequencer runs next. */
s32 appSequencerRunVsComputer(AppSequencer *this) {
    s32 result;
    s32 next = SEQUENCER_TITLE;

start:
    gameStateSetUpVsComputer(&GAME_STATE);
    result = appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1);
    if (result < 0) {
        return SEQUENCER_MAIN_MENU;
    }
    if (result == CHARACTER_SELECT_CHALLENGE) {
        gameStateSetUpChallenge(&GAME_STATE);
    }
step:
    if (gameStateIsOutsideRun(&GAME_STATE)) {
        if (appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0) == FIGHT_RESULT_SELECT) {
            goto start;
        }
        gameStateRestoreFighters(&GAME_STATE);
    }
fight:
    switch (appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0)) {
    case FIGHT_RESULT_RUN_OVER: /* listed in the game: its jump table starts at 0 */
        break;
    case FIGHT_RESULT_BONUS_GAME:
        appSequencerRunScene(this, createBonusGuideTask, TITLE_OVERLAY, 1);
        goto fight;
    case FIGHT_RESULT_RUN_CLEARED:
        if (!GAME_STATE.challenge) {
            gameStateRecordClear(&GAME_STATE);
            next = SEQUENCER_BOOT;
            appSequencerRunScene(this, createCreditsTask, TITLE_OVERLAY, 0);
        }
        break;
    case FIGHT_RESULT_CHALLENGE:
        gameStateSetUpChallenge(&GAME_STATE);
        while (appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1) < 0) {
        }
        goto step;
    case FIGHT_RESULT_FIGHT_AGAIN:
        goto fight;
    case FIGHT_RESULT_SELECT:
        goto start;
    }
    if (!GAME_STATE.challenge) {
        appSequencerRunScene(this, createRankingTask, TITLE_OVERLAY, 1);
    }
    if (gameStateHasUnsavedProgress(&GAME_STATE)) {
        appSequencerRunScene(this, createSaveTask, TITLE_OVERLAY, 1);
    }
    return next;
}

/* Runs fights of two sides (gameStateSetUpVersus; computer gives side 2 to the
 * computer), back to the select scene after each, until it is left. The
 * fight loop keeps its head on top, as in the game, only with a goto. */
s32 appSequencerRunVersus(AppSequencer *this, s32 computer) {
    s32 result;

    for (;;) {
        gameStateSetUpVersus(&GAME_STATE, computer);
        result = appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1);
        if (result < 0) {
            break;
        }
        if (result == CHARACTER_SELECT_CHALLENGE) {
            gameStateSetUpChallenge(&GAME_STATE);
        }
    fight:
        if (GAME_STATE.challenge) {
            GAME_STATE.challenge = 0;
            if (appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0) == FIGHT_RESULT_SELECT) {
                continue;
            }
            gameStateRestoreFighters(&GAME_STATE);
        }
        if (appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0) != FIGHT_RESULT_CHALLENGE) {
            continue;
        }
        gameStateSetUpChallenge(&GAME_STATE);
        while (appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1) < 0) {
        }
        goto fight;
    }
    return SEQUENCER_MAIN_MENU;
}

/* Runs fights set up by gameStateSetUpBonusVsComputer until the select scene is left. */
s32 appSequencerRunBonusVsComputer(AppSequencer *this) {
    s32 result;

    for (;;) {
        gameStateSetUpBonusVsComputer(&GAME_STATE);
        result = appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1);
        if (result < 0) {
            break;
        }
        if (result == CHARACTER_SELECT_CHALLENGE) {
            gameStateSetUpChallenge(&GAME_STATE);
        }
        GAME_STATE.bonusGame = GAME_STATE.arena - ARENA_BONUS;
        appSequencerRunScene(this, createBonusGuideTask, TITLE_OVERLAY, 1);
        appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0);
    }
    return SEQUENCER_MAIN_MENU;
}

/* Runs fights set up by gameStateSetUpBonusVersus until the select scene is left. */
s32 appSequencerRunBonusVersus(AppSequencer *this) {
    for (;;) {
        gameStateSetUpBonusVersus(&GAME_STATE);
        if (appSequencerRunScene(this, createCharacterSelectTask, TITLE_OVERLAY, 1) < 0) {
            break;
        }
        GAME_STATE.bonusGame = GAME_STATE.arena - ARENA_BONUS;
        appSequencerRunScene(this, createBonusGuideTask, TITLE_OVERLAY, 1);
        appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0);
    }
    return SEQUENCER_MAIN_MENU;
}

/* Runs the option scene, and the save scene if the progress changed. */
s32 appSequencerRunOptions(AppSequencer *this) {
    appSequencerRunScene(this, createOptionTask, TITLE_OVERLAY, 0);
    if (gameStateHasUnsavedProgress(&GAME_STATE)) {
        appSequencerRunScene(this, createSaveTask, TITLE_OVERLAY, 0);
    }
    return SEQUENCER_MAIN_MENU;
}

/* Runs random fights, forever. */
void appSequencerRunRandomFights(AppSequencer *this) {
    for (;;) {
        gameStateSetUpRandomFight(&GAME_STATE);
        appSequencerRunScene(this, createFightTask, GAME_OVERLAY, 0);
    }
}

/* Runs a scene: loads its overlay, behind the loading screen when asked, and
 * waits for its task to end. The game keeps the path buffer and the loading
 * screen, locals of two different blocks, in one stack slot, which g++ does
 * and gcc does not. Under cc1plus the C matches whole: a counter declared in
 * each for (the loading screen's counts 3 down to 0), and scene's handle
 * stored before schedulerInsertTask, each `display` in a block of its own.
 * The whole file comes out of g++ as a class AppSequencer : Task (Init its
 * constructor, the others inline members under #pragma interface, written
 * after the vtable in the game's order) but for its destructor: only a
 * synthesized one leaves out the vtable store, as the game's does, and g++
 * writes that one right after the vtable, where the game has it last. */
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
    if (!SYSTEM_CONTEXT.resetDisabled) {
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
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}
