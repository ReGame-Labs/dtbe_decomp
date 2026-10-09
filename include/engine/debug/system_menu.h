#ifndef DTBE_DEBUG_SYSTEM_MENU_H
#define DTBE_DEBUG_SYSTEM_MENU_H

/* The debug system menu: its task, pages and entries, and what the tasks are given. */

#include "common.h"
#include <libgpu.h>
#include "engine/game/game_state.h"
#include "engine/gfx/display.h"
#include "engine/math/random.h"
#include "engine/menu/stepper.h"
#include "engine/pad/pad.h"
#include "engine/task/task.h"
#include "engine/text/console.h"

/*
 * The debug flags of the system menu's flags page (SystemContext.flags), a
 * bit by SystemFlag index: the page sets them as 1 << index, the system task
 * reads them as (flags >> bit) & 1.
 */
#define SYSTEM_FLAG_FONTDISP 0 /* the console and the debug lines */
#define SYSTEM_FLAG_HSYNCDISP 1
#define SYSTEM_FLAG_HEAPDISP 2
#define SYSTEM_FLAG_POWERDISP 3
#define SYSTEM_FLAG_POLLHOST 4
#define SYSTEM_FLAG_DISPMAPHIT 5
#define SYSTEM_FLAG_CLIPDISP 6 /* the meshes culled */
#define SYSTEM_FLAG_COLLIDISP 7

/* What the system hands to the tasks it starts (SYSTEM_CONTEXT). */
typedef struct SystemContext {
    /* 0x00 */ struct Display *display;
    /* 0x04 */ struct PadManager *pads;
    /* 0x08 */ Console *console;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ struct GameState *gameState;
    /* 0x14 */ MersenneTwister *random;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 resetDisabled; /* the buttons that end the running task do nothing */
    /* 0x20 */ u32 flags; /* a bit by SYSTEM_FLAG_* */
    /* 0x24 */ s32 vsyncTime; /* the frame's, from displayWaitFrame */
} SystemContext;

/* who plays a side in the game debug menu: its "ctrl" choices */
#define GAME_CONTROL_PAD 0
#define GAME_CONTROL_COMPUTER 1
#define GAME_CONTROL_COUNT 2
/* the arenas and the bonus games the game debug menu can start (STAGE_NAMES) */
#define GAME_STAGE_COUNT (ARENA_BONUS + BONUS_GAME_COUNT)

#ifdef __cplusplus

class SystemTask;

/*
 * A system menu entry; the classes below derive from it. Like the menu pages
 * and the menu task, it lives in the debug heap (debugHeapAlloc).
 */
class SystemEntry : public Stepper {
public:
    /* 0x20 */ s32 unk20;

    static void *operator new(u32 size) __asm__("debugHeapAlloc");
    static void operator delete(void *ptr) __asm__("debugHeapFree");
    void stepUp(s32 arg, s32 held, u32 buttons) __asm__("systemEntryStepUp");
    void stepDown(s32 arg, s32 held, u32 buttons) __asm__("systemEntryStepDown");
};

/*
 * A page of the system menu: a StepperGroup with a title. Its virtual base
 * Stepper follows it (at 0x2C in a plain SystemMenu).
 */
class SystemMenu : public StepperGroup {
public:
    /* 0x24 */ s32 unk24;
    /* 0x28 */ char *title;

    static void *operator new(u32 size) __asm__("debugHeapAlloc");
    static void operator delete(void *ptr) __asm__("debugHeapFree");
    SystemMenu(const StepperButtons *buttons, char *title) : StepperGroup(buttons) {
        this->title = title;
    }
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemMenuDraw");
};

/* The system menu task: the main page and the debug flags page. */
class SystemTask : public Task {
public:
    /* 0x20 */ s32 unk20;
    /* 0x24 */ SystemMenu menu;
    /* 0x70 */ SystemMenu flagsMenu;
    /* 0xBC */ s32 started; /* the tasks it started that still run: 0 or 1 */
    /* 0xC0 */ s32 flagsMenuOpen;
    /* 0xC4 */ u32 taskHandle; /* of the last task it started */

    static void *operator new(u32 size) __asm__("debugHeapAlloc");
    static void operator delete(void *ptr) __asm__("debugHeapFree");
    SystemTask() __asm__("systemTaskInit");
    ~SystemTask();
    void update(s32 arg) __asm__("systemTaskUpdate");
    void systemTaskSetUpDisplay() __asm__("systemTaskSetUpDisplay");
    void systemTaskStartTask(Task *(*create)(void), char *overlay) __asm__("systemTaskStartTask");
};

/* A system menu entry that starts a task, loading its overlay first if it names one. */
class SystemCommand : public SystemEntry {
public:
    /* 0x24 */ char *name;
    /* 0x28 */ Task *(*create)(void);
    /* 0x2C */ char *overlay; /* the name of its file in /bin, or NULL */

    SystemCommand(char *name, Task *(*create)(void), char *overlay) __asm__("systemCommandInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemCommandConfirm");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemCommandDraw");
};

/* A system menu entry that picks one of a list of names. */
class SystemChoice : public SystemEntry {
public:
    /* 0x24 */ char *name;
    /* 0x28 */ char **choices;

    SystemChoice(char *name, char **choices, s32 count) __asm__("systemChoiceInit");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemChoiceDraw");
};

/* A system menu entry that sets a number between bounds. */
class SystemNumber : public SystemEntry {
public:
    /* 0x24 */ char *name;

    SystemNumber(char *name, s32 min, s32 max) __asm__("systemNumberInit");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemNumberDraw");
};

/*
 * The game debug menu ("GAME DEBUG MENU"): a page that sets a fight up and
 * starts the game overlay. Its virtual base Stepper follows it, at 0x48.
 */
class SystemGameMenu : public SystemMenu {
public:
    /* 0x2C */ SystemChoice *stage; /* the arena, or a bonus game */
    /* 0x30 */ SystemChoice *character1;
    /* 0x34 */ SystemChoice *control1;
    /* 0x38 */ SystemChoice *character2;
    /* 0x3C */ SystemChoice *control2;
    /* 0x40 */ SystemChoice *level;
    /* 0x44 */ SystemNumber *phase;

    SystemGameMenu() __asm__("systemGameMenuInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemGameMenuConfirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("systemGameMenuCancel");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemGameMenuDraw");
};

/* The system menu entry that plays a BGM. */
class SystemBgm : public SystemEntry {
public:
    SystemBgm() __asm__("systemBgmInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemBgmConfirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("systemBgmCancel");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemBgmDraw");
};

/* The system menu entry that plays a sound effect. */
class SystemSound : public SystemEntry {
public:
    /* 0x24 */ s32 handle; /* of the effect it played last, to stop it */

    SystemSound() __asm__("systemSoundInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemSoundConfirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("systemSoundCancel");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemSoundDraw");
};

/* The system menu entry that plays an XA track. */
class SystemXa : public SystemEntry {
public:
    SystemXa() __asm__("systemXaInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemXaConfirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("systemXaCancel");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemXaDraw");
};

/* A system menu entry that toggles the bit of its index in SYSTEM_CONTEXT's flags. */
class SystemFlag : public SystemEntry {
public:
    /* 0x24 */ char *name;

    SystemFlag(char *name) {
        this->name = name;
        min = 0;
        max = 1;
    }
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemFlagConfirm");
    void stepUp(s32 arg, s32 held, u32 buttons) __asm__("systemFlagStepUp");
    void stepDown(s32 arg, s32 held, u32 buttons) __asm__("systemFlagStepDown");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("systemFlagDraw");
    void refresh(s32 arg) __asm__("systemFlagRefresh");
};

/* The VRAM viewer entry of the system menu: a flag, and the area of VRAM it shows. */
class SystemVramViewer : public SystemFlag {
public:
    /* 0x28 */ DISPENV env;

    SystemVramViewer() __asm__("systemVramViewerInit");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("systemVramViewerConfirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("systemVramViewerCancel");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("systemVramViewerInput");
};

#endif /* __cplusplus */

EXTERN_C_BEGIN

/* 14 zero words: systemXaDraw takes the first as the number of XA tracks;
 * the rest is unknown */
extern s32 D_8005F870[14];

extern struct SystemContext SYSTEM_CONTEXT; /* what the tasks are given */

EXTERN_C_END

/* the names the game debug menu picks from: by character, level and stage */
extern char *CHARACTER_NAMES[CHARACTER_COUNT];
extern char *LEVEL_NAMES[LEVEL_COUNT];
extern char *STAGE_NAMES[GAME_STAGE_COUNT];

EXTERN_C_BEGIN

/* the commands' task creators; the scenes among them are the sequencer's
 * (createTitleTask, ...), with the select flags, the bonus game and the
 * ranking's argument fixed */
Task *debugCreateSequencerTask(void);
Task *debugCreateAgeingSequencerTask(void);
Task *debugCreateTitleTask(void);
Task *debugCreateBonusGuideTask(void);
Task *debugCreateRankingTask(void);
Task *debugCreateCreditsTask(void);
Task *debugCreateCharacterSelectTask(void);
Task *debugCreateFightTask(void);
Task *debugCreateHanamasuTestTask(void);
Task *debugCreateIwanagaTestTask(void);
Task *debugCreateShohyamaTestTask(void);
Task *debugCreateTeradaTestTask(void);
Task *debugCreateModelViewTask(void);
Task *debugCreateMapViewTask(void);
Task *reloadExecutable(void);
Task *debugCreateOptionTask(void);
Task *debugCreateMainMenuTask(void);
Console *startEntryLine(s32 arg, s32 isCurrent, s32 index);
void loadDebugTim(void);

EXTERN_C_END

#endif /* DTBE_DEBUG_SYSTEM_MENU_H */
