#include "common.h"
#include "engine/debug/system_menu.h"
#include "engine/cd/file_load.h"
#include "engine/cd/xa_player.h"
#include "engine/game/game_state.h"
#include "engine/game/sequencer.h"
#include "engine/gfx/display.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/scene_graph.h"
#include "engine/menu/stepper.h"
#include "engine/menu/stepper_group.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound.h"
#include "engine/sound/sound_control.h"
#include "engine/system/handle_table.h"
#include "engine/system/load_exec.h"
#include "engine/system/memory.h"
#include "engine/task/task.h"
#include "engine/text/console.h"
#include "stdio.h"
#include "overlay.h"
#include "psyq.h"

/* the sound of a menu entry stepping */
#define SOUND_STEP 0x13
/* the console column the menu entries start at */
#define MENU_COLUMN 2
/* the gray of the current menu entry and of the others */
#define MENU_BRIGHT 0x80
#define MENU_DIM 0x40

/*
 * The buttons of the system menu's pages: up and down to move, circle or
 * start to confirm, cross to cancel, right and left to step. g++ writes a
 * file-scope static const object at the end of the file's .rodata, where
 * this one is (0x8001914C).
 */
static const StepperButtons MENU_BUTTONS = {
    PADLup, PADLdown, PADRright | PADstart, PADRdown, PADLright, PADLleft,
};

/*
 * The character names of the debug menu's table (CHARACTER_NAMES) too long for
 * .sdata; the short ones are defined further down. They start the file's
 * .rodata, ahead of the strings of the functions. extern: the table, in .data
 * and still assembly, points at them.
 */
extern const char STR_HOLYDRAMON[] = "HOLYDRAMON";
extern const char STR_IMPERIALDRAMON[] = "IMPERIALDRAMON";
extern const char STR_IMPERIALDRAMON_PALADIN_MODE[] = "IMPERIALDRAMON(Paladin Mode)";
extern const char STR_BEELZEBUMON[] = "BEELZEBUMON";
extern const char STR_METALGARURUMON[] = "METALGARURUMON";
extern const char STR_OMEGAMON[] = "OMEGAMON";
extern const char STR_SAKUYAMON[] = "SAKUYAMON";
extern const char STR_SAINTGARGOMON[] = "SAINTGARGOMON";
extern const char STR_SERAPHIMON[] = "SERAPHIMON";
extern const char STR_STINGMON[] = "STINGMON";
extern const char STR_TERRIERMON[] = "TERRIERMON";
extern const char STR_WARGREYMON[] = "WARGREYMON";
extern const char STR_BLACKWARGREYMON[] = "BLACKWARGREYMON";

/* Creates the sequencer, without random fights (the "SEQUENCER (REAL THING MODE)" command). */
Task *createSequencerTask(void) {
    return &appSequencerInit((AppSequencer *)operatorNew(sizeof(AppSequencer)), 0)->task;
}

/* Creates the sequencer with random fights (the "BATTLE AGEING SEQUENCER" command). */
Task *createAgeingSequencerTask(void) {
    return &appSequencerInit((AppSequencer *)operatorNew(sizeof(AppSequencer)), 1)->task;
}

/* Creates the title screen (the "TITLE" command). */
Task *createTitleTask(void) {
    return func_80067170(operatorNew(0x110C), &SYSTEM_CONTEXT);
}

/* Creates the minigame guide (the "MINIGAME GUIDE" command). */
Task *createMinigameGuideTask(void) {
    return func_80071F28(operatorNew(0xA90), &SYSTEM_CONTEXT, 0);
}

/* Creates the ranking screen (the "RANKING" command). */
Task *createRankingTask(void) {
    return func_8007360C(operatorNew(0x474), &SYSTEM_CONTEXT, 1);
}

/* Creates the credits (the "CREDIT" command). */
Task *createCreditsTask(void) {
    return func_80076504(operatorNew(0x44), &SYSTEM_CONTEXT);
}

/* Creates the character select screen (the "CHARACTER SELECT" command). */
Task *createCharacterSelectTask(void) {
    return func_8006AAF8(operatorNew(0x234), &SYSTEM_CONTEXT, 0x25);
}

/* Creates the task of an overlay's func_8006E1CC (a system menu command). */
Task *createGameTask(void) {
    return func_8006E1CC(operatorNew(0x64), &SYSTEM_CONTEXT);
}

/* Creates the task of an overlay's func_80065AE4 (a system menu command). */
Task *func_800283DC(void) {
    return func_80065AE4(operatorNew(0xF4C), &SYSTEM_CONTEXT);
}

/* Creates the task of an overlay's func_80066084 (a system menu command). */
Task *func_8002840C(void) {
    return func_80066084(operatorNew(0x14C), &SYSTEM_CONTEXT);
}

/* Creates the task of an overlay's func_800673A4 (a system menu command). */
Task *func_8002843C(void) {
    return func_800673A4(operatorNew(0xA1C), &SYSTEM_CONTEXT);
}

/* Creates the task of an overlay's func_80066F74 (a system menu command). */
Task *func_8002846C(void) {
    return func_80066F74(operatorNew(0xA5C), &SYSTEM_CONTEXT);
}

/* Creates the model viewer (the "ModelView" command). */
Task *createModelViewTask(void) {
    return func_800681A4(operatorNew(0x30), &SYSTEM_CONTEXT);
}

/* Creates the map viewer (the "MapView" command). */
Task *createMapViewTask(void) {
    return func_8006A43C(operatorNew(0x30), &SYSTEM_CONTEXT);
}

/* Runs the game's executable again (the "LoadExec" command). */
Task *reloadExecutable(void) {
    runExecutable("/slps_999.99");
    return NULL;
}

/* Creates the option screen (the "OPTION" command). */
Task *createOptionTask(void) {
    return func_800794E0(operatorNew(0x8C));
}

/* Creates the main menu (the "MAIN MENU" command). */
Task *createMainMenuTask(void) {
    return func_8006A360(operatorNew(0x90), 0);
}

/* Steps an entry up: 16 at a time with L1 held, 128 with L2, else once. */
void SystemEntry::stepUp(s32 arg, s32 held, u32 pressed) {
    s32 steps = 1;

    if (held & PADL1) {
        steps = 16;
    }
    if (held & PADL2) {
        steps = 128;
    }
    while (--steps != -1) {
        stepperStepUpWrap();
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_STEP, 0);
}

/* Steps an entry down: 16 at a time with L1 held, 128 with L2, else once. */
void SystemEntry::stepDown(s32 arg, s32 held, u32 pressed) {
    s32 steps = 1;

    if (held & PADL1) {
        steps = 16;
    }
    if (held & PADL2) {
        steps = 128;
    }
    while (--steps != -1) {
        stepperStepDownWrap();
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_STEP, 0);
}

/* Starts the line of the entry of an index on the console, bright if it is the current one. */
Console *startEntryLine(s32 arg, s32 isCurrent, s32 index) {
    Console *console = SYSTEM_CONTEXT.console;

    consoleLocate(console, MENU_COLUMN, index + 1);
    if (isCurrent) {
        consoleSetColor(console, MENU_BRIGHT, MENU_BRIGHT, MENU_BRIGHT);
    } else {
        consoleSetColor(console, MENU_DIM, MENU_DIM, MENU_DIM);
    }
    return console;
}

/* Draws a menu page: its title in yellow at the top when it is active, then its entries. */
void SystemMenu::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console;

    if (active) {
        console = SYSTEM_CONTEXT.console;
        consoleSetColor(console, 0xFF, 0xFF, 0);
        consoleLocate(console, 0, 0);
        consolePrint(console, "--- %s ---", title);
    }
    StepperGroup::draw(arg, isCurrent, active);
}

/* Constructs an entry that starts a task, loading an overlay first unless it is NULL. */
SystemCommand::SystemCommand(char *name, Task *(*create)(void), char *overlay) {
    this->name = name;
    this->create = create;
    this->overlay = overlay;
}

/* Starts the entry's task; arg is the system menu task. */
s32 SystemCommand::confirm(s32 arg, s32 held, s32 own) {
    ((SystemTask *)arg)->systemTaskStartTask(create, overlay);
    return -1;
}

/* Draws the entry: its index and name. */
void SystemCommand::draw(s32 arg, s32 isCurrent, s32 active) {
    consolePrint(startEntryLine(arg, isCurrent, index), "#%02d %s", index, name);
}

/* Constructs an entry that picks one of count names. */
SystemChoice::SystemChoice(char *name, char **choices, s32 count) {
    this->name = name;
    this->choices = choices;
    max = count - 1;
}

/* Draws the entry: its index, its name, the value and its choice. */
void SystemChoice::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console = startEntryLine(arg, isCurrent, index);
    s32 choice = value;
    s32 index = this->index;

    consolePrint(console, "#%02d %s %2d:%s", index, name, choice, choices[choice]);
}

/* Constructs an entry that steps a number from min to max. */
SystemNumber::SystemNumber(char *name, s32 min, s32 max) {
    this->min = min;
    this->max = max;
    this->name = name;
}

/* Draws the entry: its index, its name and the number. */
void SystemNumber::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console = startEntryLine(arg, isCurrent, index);
    s32 value = this->value;
    s32 index = this->index;

    consolePrint(console, "#%02d %s %2d", index, name, value);
}

/* Constructs the game debug menu, empty until it is opened. */
SystemGameMenu::SystemGameMenu() : SystemMenu(&MENU_BUTTONS, "GAME DEBUG MENU") {
}

/*
 * The names the game debug menu picks from, in the tables in .data that point
 * at them (CHARACTER_NAMES, LEVEL_NAMES, STAGE_NAMES): characters, levels and arenas.
 */
char STR_AGUMON[] = "AGUMON";
char STR_DUKEMON[] = "DUKEMON";
char STR_GABUMON[] = "GABUMON";
char STR_GUILMON[] = "GUILMON";
char STR_IMPMON[] = "IMPMON";
char STR_PATAMON[] = "PATAMON";
char STR_RENAMON[] = "RENAMON";
char STR_TAILMON[] = "TAILMON";
char STR_V_MON[] = "V-MON";
char STR_WORMMON[] = "WORMMON";
char STR_GOKUMON[] = "GOKUMON";

/*
 * Who plays a side, by GAME_CONTROL_*. The names are arrays of their own: the
 * compiler would put string literals of the table after it, the other way round.
 */
static char PAD_NAME[] = "PAD";
static char COMPUTER_NAME[] = "COM";
static char *CONTROL_NAMES[GAME_CONTROL_COUNT] = { PAD_NAME, COMPUTER_NAME };

char STR_EASY[] = "EASY";
char STR_NORMAL[] = "NORMAL";
char STR_HARD[] = "HARD";
char STR_MACHINE[] = "Machine";
char STR_NATURE[] = "Nature";
char STR_TURN[] = "Turn";
char STR_SHRINE[] = "Shrine";
char STR_ICEBERG[] = "Iceberg";
char STR_VOLCANO[] = "Volcano";
char STR_BOSS[] = "Boss";

/*
 * The names of the bonus games, the last of the stage names (STAGE_NAMES), too
 * long for .sdata. Most likely they were the string literals of that table,
 * defined here with the other name tables; while .data is not split by file,
 * the tables stay in assembly and point at these by address.
 */
extern const char STR_BONUS_0_BALL_SHOOT[] = "Bonus #0 BallShoot";
extern const char STR_BONUS_1_EVOLUTION_COMPETITION[] = "Bonus #1 Evolution competition";
extern const char STR_BONUS_2_JEWELRY_HUNTING[] = "Bonus #2 Jewelry hunting";

/*
 * Confirms the game debug menu: opening it (re)builds its entries and takes
 * the buttons; confirming while it is open sets the next fight up as picked.
 */
s32 SystemGameMenu::confirm(s32 arg, s32 held, s32 own) {
    if (!own) {
        char **controls = CONTROL_NAMES;

        stepperSetGroupCurrentOnly(0);
        stage = new SystemChoice("STAGE", STAGE_NAMES, GAME_STAGE_COUNT);
        stepperGroupAddChild(stage);
        character1 = new SystemChoice("   1P", CHARACTER_NAMES, CHARACTER_COUNT);
        stepperGroupAddChild(character1);
        control1 = new SystemChoice(" ctrl", controls, GAME_CONTROL_COUNT);
        stepperGroupAddChild(control1);
        character2 = new SystemChoice("   2P", CHARACTER_NAMES, CHARACTER_COUNT);
        stepperGroupAddChild(character2);
        control2 = new SystemChoice(" ctrl", controls, GAME_CONTROL_COUNT);
        stepperGroupAddChild(control2);
        level = new SystemChoice("level", LEVEL_NAMES, GAME_LEVEL_COUNT);
        stepperGroupAddChild(level);
        phase = new SystemNumber("phase", 0, GAME_PHASE_MAX);
        stepperGroupAddChild(phase);
        stepperGroupAddChild(new SystemCommand("GAME START", createGameTask, "game"));
        return 0;
    }
    GAME_STATE.arena = stage->value;
    GAME_STATE.fighters[0].character = character1->value;
    GAME_STATE.fighters[0].computer = control1->value == GAME_CONTROL_COMPUTER;
    GAME_STATE.fighters[0].unk14 = 7;
    GAME_STATE.fighters[1].character = character2->value;
    GAME_STATE.fighters[1].computer = control2->value == GAME_CONTROL_COMPUTER;
    GAME_STATE.fighters[1].unk14 = 7;
    GAME_STATE.unk2A8 = level->value;
    GAME_STATE.unk2AC = phase->value;
    return 0;
}

/* Leaves the page. */
s32 SystemGameMenu::cancel(s32 arg, s32 held, s32 own) {
    stepperGroupRemoveChildren();
    return 0;
}

/* Draws the page: as a menu while it is active, else as an entry of the page holding it. */
void SystemGameMenu::draw(s32 arg, s32 isCurrent, s32 active) {
    if (active) {
        SystemMenu::draw(arg, isCurrent, active);
    } else {
        consolePrint(startEntryLine(arg, isCurrent, index), "#%02d %s", index, title);
    }
}

/* Constructs the entry that plays a BGM. */
SystemBgm::SystemBgm() {
    min = 0;
    max = getSongCount() - 1;
}

/* Unloads the sequences and the VABs that are not resident. */
s32 SystemBgm::cancel(s32 arg, s32 held, s32 own) {
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    return 0;
}

/* Plays the BGM of the entry's number. */
s32 SystemBgm::confirm(s32 arg, s32 held, s32 own) {
    s32 song = value;

    soundControlLoadAndPlaySong(&SOUND_CONTROL, song);
    return -1;
}

/* Draws the entry: its index, the BGM number and the name of its .sep. */
void SystemBgm::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console = startEntryLine(arg, isCurrent, index);
    s32 song = value;
    s32 index = this->index;

    consolePrint(console, "#%02d BGM 0x%03x '%s'", index, song, getSepName(getSongSep(song)));
}

/* Constructs the entry that plays a sound effect. */
SystemSound::SystemSound() {
    min = 0;
    max = getSoundEffectCount() - 1;
    handle = 0;
}

/* Unloads the VABs that are not resident. */
s32 SystemSound::cancel(s32 arg, s32 held, s32 own) {
    soundControlUnloadVabs(&SOUND_CONTROL);
    return 0;
}

/*
 * Select hands the stepper's value to func_8001BEC8, L1 passes the handle of
 * the last effect played to stopSound, any other button plays the sound
 * effects of the stepper's value and keeps the handle.
 */
s32 SystemSound::confirm(s32 arg, s32 held, s32 own) {
    if (held & PADselect) {
        func_8001BEC8(&SOUND_CONTROL, value);
        return -1;
    }
    if (held & PADL1) {
        stopSound(handle);
    } else {
        handle = soundControlLoadAndPlaySoundEffect(&SOUND_CONTROL, value);
    }
    return -1;
}

/* Draws the entry: its index, the sound number and the channel playing it. */
void SystemSound::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console = startEntryLine(arg, isCurrent, index);
    s32 index = this->index;
    s32 sound = value;

    consolePrint(console, "#%02d SE 0x%03x channel:%d", index, sound, getSoundEffectVoice(handle));
}

/* Constructs the entry that plays an XA track. */
SystemXa::SystemXa() {
    min = 0;
}

/* Turns the XA player off. */
s32 SystemXa::cancel(s32 arg, s32 held, s32 own) {
    turnXaOff();
    return 0;
}

/*
 * Plays the XA track of the entry's number, or with R2 held resumes the one
 * playing; with R1 held pauses it.
 */
s32 SystemXa::confirm(s32 arg, s32 held, s32 own) {
    if (held & PADR1) {
        pauseXa();
        return -1;
    }
    if (!(held & PADR2)) {
        seekXaTrack(value);
        setXaLoops(0);
    }
    playXa();
    return -1;
}

/* Draws the entry: its index and the XA track number, up to the number of tracks. */
void SystemXa::draw(s32 arg, s32 isCurrent, s32 active) {
    max = D_8005F870[0] - 1;
    consolePrint(startEntryLine(arg, isCurrent, index), "#%02d XA %d", index, value);
}

/* Toggles the flag; on is both its value and, shifted by its index, its bit. */
s32 SystemFlag::confirm(s32 arg, s32 held, s32 own) {
    s32 on = 1;

    SYSTEM_CONTEXT.flags ^= on << index;
    if (SYSTEM_CONTEXT.flags & (on << index)) {
        value = on;
    } else {
        value = 0;
    }
    return -1;
}

/* Sets the flag. */
void SystemFlag::stepUp(s32 arg, s32 held, u32 pressed) {
    Stepper::stepUp(arg, held, pressed);
    SYSTEM_CONTEXT.flags |= 1 << index;
}

/* Clears the flag. */
void SystemFlag::stepDown(s32 arg, s32 held, u32 pressed) {
    Stepper::stepDown(arg, held, pressed);
    SYSTEM_CONTEXT.flags &= ~(1 << index);
}

/* Shows the flag as it is. */
void SystemFlag::refresh(s32 arg) {
    if (SYSTEM_CONTEXT.flags & (1 << index)) {
        value = 1;
    } else {
        value = 0;
    }
}

/* Draws the flag: its index, its name and ON or OFF. */
void SystemFlag::draw(s32 arg, s32 isCurrent, s32 active) {
    Console *console = startEntryLine(arg, isCurrent, index);
    s32 on = value;
    s32 index = this->index;
    char *state;

    if (on) {
        state = "ON";
    } else {
        state = "OFF";
    }
    consolePrint(console, "#%02d %-16s%s", index, name, state);
}

/* Constructs the VRAM viewer entry, showing the right of VRAM at 256x240. */
SystemVramViewer::SystemVramViewer() : SystemFlag("VRAM VIEWER") {
    env.disp.x = 384;
    env.disp.y = 0;
    env.disp.w = 640;
    env.disp.h = 480;
    env.screen.x = 0;
    env.screen.y = 0;
    env.screen.w = 256;
    env.screen.h = 240;
    env.isinter = 1;
    env.isrgb24 = 0;
}

/*
 * Moves the area of VRAM shown by 8 pixels with the d-pad, inside VRAM
 * (virtual input). The C is right but not the allocation: the original keeps
 * this in a1 and the bound in a0, and loads x before w.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/debug/system_menu", systemVramViewerInput);

/* Shows the area of VRAM. */
s32 SystemVramViewer::confirm(s32 arg, s32 held, s32 own) {
    displaySetDispEnv(SYSTEM_CONTEXT.display, &env);
    return 0;
}

/* Shows the game's display again when the cancel is the entry's own. */
s32 SystemVramViewer::cancel(s32 arg, s32 held, s32 own) {
    if (!own) {
        return 0;
    }
    displaySetDispEnv(SYSTEM_CONTEXT.display, NULL);
    return 0;
}

/* the system menu task's priority */
#define SYSTEM_PRIORITY -9999
/*
 * The "Testing ground for ..." commands, left out of the menu: their strings
 * are in the file's .rodata, among the menu's, but no code adds them.
 */
#define TESTING_GROUNDS 0

/* Constructs the system menu task and the entries of its two pages. */
SystemTask::SystemTask()
    : Task(SYSTEM_PRIORITY, "DebugMenu"), menu(&MENU_BUTTONS, "DEBUG MODE MAIN MENU"),
      flagsMenu(&MENU_BUTTONS, "DEBUG FLAGS SETTING") {
    started = 0;
    SYSTEM_CONTEXT.flags |= 1 << SYSTEM_FLAG_FONTDISP;
    systemTaskSetUpDisplay();
    flagsMenuOpen = 0;
    menu.stepperGroupSetKeepOpen(1);
    menu.stepperGroupAddChild(new SystemCommand("SEQUENCER (REAL THING MODE)", createSequencerTask, NULL));
    menu.stepperGroupAddChild(new SystemGameMenu);
    menu.stepperGroupAddChild(new SystemCommand("BATTLE AGEING SEQUENCER", createAgeingSequencerTask, NULL));
    menu.stepperGroupAddChild(new SystemCommand("TITLE", createTitleTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("MAIN MENU", createMainMenuTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("OPTION", createOptionTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("CHARACTER SELECT", createCharacterSelectTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("RANKING", createRankingTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("CREDIT", createCreditsTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("MINIGAME GUIDE", createMinigameGuideTask, "title"));
    menu.stepperGroupAddChild(new SystemCommand("ModelView", createModelViewTask, "test"));
    menu.stepperGroupAddChild(new SystemCommand("MapView", createMapViewTask, "test"));
    menu.stepperGroupAddChild(new SystemCommand("LoadExec", reloadExecutable, NULL));
    if (TESTING_GROUNDS) {
        menu.stepperGroupAddChild(new SystemCommand("Testing ground for HANAMASU", func_800283DC, "test"));
        menu.stepperGroupAddChild(new SystemCommand("Testing ground for IWANAGA", func_8002840C, "test"));
        menu.stepperGroupAddChild(new SystemCommand("Testing ground for SHOHYAMA", func_8002843C, "test"));
        menu.stepperGroupAddChild(new SystemCommand("Testing ground for TERADA", func_8002846C, "test"));
    }
    menu.stepperGroupAddChild(new SystemBgm);
    menu.stepperGroupAddChild(new SystemSound);
    menu.stepperGroupAddChild(new SystemXa);
    flagsMenu.stepperGroupAddChild(new SystemFlag("fontdisp"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("hsyncdisp"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("heapdisp"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("powerdisp"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("pollhost"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("dispmaphit"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("clipdisp"));
    flagsMenu.stepperGroupAddChild(new SystemFlag("collidisp"));
    flagsMenu.stepperGroupAddChild(new SystemVramViewer);
    menu.wraps = 1;
}

/* Destroys the system menu task. */
SystemTask::~SystemTask() {
}

/* Sets the display and the pads up for the system menu. */
void SystemTask::systemTaskSetUpDisplay() {
    Display *display = SYSTEM_CONTEXT.display;

    padManagerSetRepeat(SYSTEM_CONTEXT.pads, 16, 4);
    displaySetSize(display, 512, 480);
    displaySetVsyncMode(display, 1);
    displaySetClearColor(display, 0, 0x40, 0);
    displayDisable(display, 1);
    displayEnable(display, 0);
}

/*
 * Runs the debug flags menu (opened with L2+R2), the menu of tasks while none
 * runs, ends the running task on select+start, then writes the debug text the
 * flags ask for.
 */
void SystemTask::update(s32 arg) {
    PadManager *pads = SYSTEM_CONTEXT.pads;
    Console *console = SYSTEM_CONTEXT.console;
    u32 repeat = padManagerGetRepeated(pads, -1);
    u32 held = padManagerGetHeld(pads, -1);
    u32 pressed = padManagerGetPressed(pads, -1);
    s32 culled;
    s32 total;

    if (flagsMenuOpen) {
        flagsMenuOpen = flagsMenu.stepperGroupInputIfOpen((s32)this, held, repeat) == 1;
        flagsMenu.stepperGroupDrawIfOpen((s32)this);
    }
    switch (started) {
    case 0:
        if (!flagsMenuOpen) {
            menu.stepperGroupInputIfOpen((s32)this, held, repeat);
            menu.stepperGroupDrawIfOpen((s32)this);
        }
        break;
    case 1:
        if (handleTableGet(TASK_HANDLES, taskHandle) != NULL) {
            if ((held & PADselect) && (pressed & PADstart) && SYSTEM_CONTEXT.unk1C == 0) {
                killTaskByHandle(taskHandle);
            }
        } else {
            started = 0;
            systemTaskSetUpDisplay();
        }
        break;
    }
    if ((held & (PADL2 | PADR2)) == (PADL2 | PADR2) && !flagsMenuOpen) {
        flagsMenuOpen = 1;
        flagsMenu.stepperGroupOpen((s32)this);
    }
    if (!started || flagsMenuOpen || (SYSTEM_CONTEXT.flags >> SYSTEM_FLAG_FONTDISP) & 1) {
        consoleSetColor(console, 0xFF, 0xFF, 0xFF);
        consolePrint(console, "\n");
        if ((SYSTEM_CONTEXT.flags >> SYSTEM_FLAG_HSYNCDISP) & 1) {
            consolePrint(console, "H-SYNC: %d %d\n", SYSTEM_CONTEXT.unk24, displayGetDrawEndCount(SYSTEM_CONTEXT.display));
        }
        if ((SYSTEM_CONTEXT.flags >> SYSTEM_FLAG_HEAPDISP) & 1) {
            consolePrint(console, "HEAP: AVAIL(%dK) BLOCK(%d)", mainHeapGetLargestFree() >> 10, mainHeapGetBlockCount());
        }
        if ((SYSTEM_CONTEXT.flags >> SYSTEM_FLAG_CLIPDISP) & 1) {
            culled = getMeshOutOfViewCount();
            total = getMeshCullCount();
            resetMeshCullCounts();
            consolePrint(console, "TMDCLIP: %d/%d", total - culled, total);
        }
        consoleDraw(console, FRAME_OT.lastDrawn);
    }
    consoleClear(console);
}

/* Reads the debug TIM's header (it is never shown). */
void loadDebugTim(void) {
    TIM_IMAGE image;
    u32 *file = (u32 *)loadFile("dbg.tim");

    func_800585B0(file);
    ReadTIM(&image);
    mainHeapFree(file);
}

/*
 * Starts a task made by create, after loading the overlay /bin/<overlay>.bin
 * (unless NULL) and setting the display up again; the system menu waits for
 * it to end.
 */
void SystemTask::systemTaskStartTask(Task *(*create)(void), char *overlay) {
    char path[64];
    Display *display;
    Task *task;

    if (overlay != NULL) {
        sprintf(path, "/bin/%s.bin", overlay);
        loadCompressedFileInto(D_800643E0, path);
    }
    display = SYSTEM_CONTEXT.display;
    padManagerSetRepeat(SYSTEM_CONTEXT.pads, 16, 4);
    displaySetSize(display, SCREEN_WIDTH, SCREEN_HEIGHT);
    displaySetVsyncMode(display, 1);
    displaySetClearColor(display, 0, 0, 0);
    displayDisable(display, 1);
    displayEnable(display, 0);
    task = create();
    taskHandle = task->handle;
    schedulerInsertTask(scheduler, task);
    started++;
}
