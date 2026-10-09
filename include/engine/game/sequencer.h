#ifndef DTBE_GAME_SEQUENCER_H
#define DTBE_GAME_SEQUENCER_H

/* The sequencer: the task that runs the game's scenes in turn, on a thread of its own. */

#include "common.h"
#include <libetc.h>
#include "engine/system/thread.h"
#include "engine/task/task.h"

EXTERN_C_BEGIN

/* the priority of the sequencer's task, and the stack size of its thread */
#define APP_SEQUENCER_PRIORITY -8000
#define APP_SEQUENCER_STACK_SIZE 0x1800

/* AppSequencer.state: what the sequencer runs next */
#define SEQUENCER_BOOT 0 /* the boot logo (and the save, the first time), then on */
#define SEQUENCER_MOVIE 1 /* the movie, then on */
#define SEQUENCER_TITLE 2 /* the title, then the main menu or the demo */
#define SEQUENCER_MAIN_MENU 3
/* the main menu's choices, in its order */
#define SEQUENCER_VS_COMPUTER 4
#define SEQUENCER_VERSUS 5
#define SEQUENCER_VERSUS_COMPUTER 6
#define SEQUENCER_BONUS_VS_COMPUTER 7
#define SEQUENCER_BONUS_VERSUS 8
#define SEQUENCER_OPTIONS 9
/* the endless random fights of the debug menu's "BATTLE AGEING SEQUENCER" */
#define SEQUENCER_RANDOM_FIGHTS 10

/* The task that runs the game's scenes one after the other, on a thread of
 * its own. */
typedef struct AppSequencer {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ u32 scene; /* the handle of the scene task running, or 0 */
    /* 0x30 */ s32 state; /* what to run next */
} AppSequencer; /* size 0x34 */

/* the results of the fight scene (createFightTask), as the sequencer runs
 * the scenes after them */
#define FIGHT_RESULT_RUN_OVER 0    /* the run is over: the ranking, then the title */
#define FIGHT_RESULT_BONUS_GAME 1  /* the bonus game is next: its guide, then the fight scene */
#define FIGHT_RESULT_RUN_CLEARED 2 /* the run was won: the clear is recorded, the credits run */
#define FIGHT_RESULT_CHALLENGE 3   /* a player joined in: the challenger picks a character */
#define FIGHT_RESULT_FIGHT_AGAIN 4 /* the fight scene runs again, with no other scene between */
#define FIGHT_RESULT_SELECT 5      /* back to the select scene, the mode set up anew */

/* the result of the select scene (createCharacterSelectTask) when a player
 * joined in; below 0, the scene was left */
#define CHARACTER_SELECT_CHALLENGE 1

/* the modes of the memory card scene (func_800709B0) that load or save
 * without asking first, as modes 0 and 1 do */
#define MEMORY_CARD_LOAD 2
#define MEMORY_CARD_SAVE 3

/* held together with Start, they reset the game */
#define RESET_BUTTONS (PADselect | PADR1 | PADL1)

/* the virtual table of AppSequencer */
extern struct TaskVtable APP_SEQUENCER_VTABLE;

/* the handle of the AppSequencer, for the scenes to find it */
extern u32 APP_SEQUENCER_HANDLE;
/* the path of a scene's overlay on the disc, "/bin/%s.bin" */
extern char OVERLAY_PATH_FORMAT[];

/* the overlays of the title, the movie and the game scenes */
extern char TITLE_OVERLAY[]; /* "title" */
extern char MOVIE_OVERLAY[]; /* "movie" */
extern char GAME_OVERLAY[]; /* "game" */

Task *createBootLogoTask(void);
Task *createTitleTask(void);
Task *createCharacterSelectTask(void);
Task *createFightTask(void);
Task *createBonusGuideTask(void);
Task *createCreditsTask(void);
Task *createRankingTask(void);
Task *createMovieTask(void);
Task *createMainMenuTask(void);
Task *createOptionTask(void);
Task *createLoadTask(void);
Task *createSaveTask(void);
AppSequencer *appSequencerInit(AppSequencer *appSequencer, s32 randomFights);
void runAppSequencerThread(s32 unused, AppSequencer *appSequencer);
void appSequencerRunScenes(AppSequencer *appSequencer);
s32 appSequencerRunVsComputer(AppSequencer *appSequencer);
s32 appSequencerRunVersus(AppSequencer *appSequencer, s32 computer);
s32 appSequencerRunBonusVsComputer(AppSequencer *appSequencer);
s32 appSequencerRunBonusVersus(AppSequencer *appSequencer);
s32 appSequencerRunOptions(AppSequencer *appSequencer);
void appSequencerRunRandomFights(AppSequencer *appSequencer);
s32 appSequencerRunScene(AppSequencer *appSequencer, Task *(*create)(void), char *overlay, s32 loading);
void appSequencerUpdate(AppSequencer *appSequencer);
s32 appSequencerSurvives(AppSequencer *appSequencer);
void appSequencerDestroy(AppSequencer *appSequencer, s32 flags);

EXTERN_C_END

#endif /* DTBE_GAME_SEQUENCER_H */
