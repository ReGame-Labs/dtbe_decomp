#ifndef DTBE_GAME_SEQUENCER_H
#define DTBE_GAME_SEQUENCER_H

/* The sequencer: the task that runs the game's scenes in turn, on a thread of its own. */

#include "common.h"
#include <libetc.h>
#include "engine/system/thread.h"
#include "engine/task/task.h"

EXTERN_C_BEGIN

/* The task that runs the game's scenes one after the other, on a thread of
 * its own. */
#define APP_SEQUENCER_PRIORITY -8000
#define APP_SEQUENCER_STACK_SIZE 0x1800
/* AppSequencer.state: the endless random fights of the attract mode */
#define SEQUENCER_RANDOM_FIGHTS 10

typedef struct AppSequencer {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ u32 scene; /* the handle of the scene task running, or 0 */
    /* 0x30 */ s32 state; /* what to run next */
} AppSequencer; /* size 0x34 */

/* held together with Start, they reset the game */
#define RESET_BUTTONS (PADselect | PADR1 | PADL1)

extern u16 D_8005F92C[298];
extern s32 D_8005FB80[4];
extern s32 D_8005FB90[65];

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

Task *func_8001C36C(void);
Task *createTitleScene(void);
Task *createCharacterSelectScene(void);
Task *createFightScene(void);
Task *createMinigameGuideScene(void);
Task *createCreditsScene(void);
Task *createRankingScene(void);
Task *createMovieScene(void);
Task *createMainMenuScene(void);
Task *createOptionScene(void);
Task *func_8001C554(void);
Task *createSaveScene(void);
AppSequencer *appSequencerInit(AppSequencer *appSequencer, s32 randomFights);
void runAppSequencerThread(s32 unused, AppSequencer *appSequencer);
void appSequencerRunScenes(AppSequencer *appSequencer);
s32 appSequencerRunVsComputer(AppSequencer *appSequencer);
s32 appSequencerRunVersus(AppSequencer *appSequencer, s32 arg1);
s32 appSequencerRunMinigameVsComputer(AppSequencer *appSequencer);
s32 appSequencerRunMinigameVersus(AppSequencer *appSequencer);
s32 appSequencerRunOptions(AppSequencer *appSequencer);
void appSequencerRunRandomFights(AppSequencer *appSequencer);
s32 appSequencerRunScene(AppSequencer *appSequencer, Task *(*create)(void), char *overlay, s32 loading);
void appSequencerUpdate(AppSequencer *appSequencer);
s32 appSequencerSurvives(AppSequencer *appSequencer);
void appSequencerDestroy(AppSequencer *appSequencer, s32 flags);

EXTERN_C_END

#endif /* DTBE_GAME_SEQUENCER_H */
