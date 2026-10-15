#ifndef DTBE_TITLE_OPTION_H
#define DTBE_TITLE_OPTION_H

/* The options' rows, the button menu and the options' task (title/option.cpp). */

#ifdef __cplusplus
/* Their virtual tables stay in the overlay's assembly (.rodata), so option.cpp
 * writes none of them out. */
#pragma interface
#endif

#include "common.h"
#include "engine/menu/stepper.h"
#include "engine/system/thread.h"
#include "engine/gfx/sprite.h"
#include "engine/task/task.h"
#include "title/title_classes.h"

#ifdef __cplusplus

/*
 * The member functions keep the names of the symbol files through asm labels
 * (g++ would mangle them); the destructors, which take no asm label, have
 * g++'s names there.
 */

/* A row of the options' menus: its label image drawn at its row, blinking while it is current; it reads and
 * writes the save option of its row (refresh, apply). The other rows derive from it; Stepper is its virtual
 * base, reached through the pointer at 0x00 and placed after the members of the most derived class. */
class OptionItem : public virtual Stepper {
public:
    /* 0x04 */ TitleImage *image;
    /* 0x08 */ s32 blink; /* the blink's angle; 0 while the item is not active */

    void stepUp(s32 arg, s32 held, u32 buttons) __asm__("OptionItem_stepUp");
    void stepDown(s32 arg, s32 held, u32 buttons) __asm__("OptionItem_stepDown");
    void refresh(s32 arg) __asm__("OptionItem_refresh");
    void apply() __asm__("OptionItem_apply");
    void focus(s32 arg) __asm__("OptionItem_focus");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionItem_cancel");
    OptionItem(TitleImage *image) __asm__("OptionItem_init");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionItem_confirm");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionItem_input");
    s32 getBrightness(s32 isCurrent) __asm__("OptionItem_getBrightness");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionItem_draw");
    ~OptionItem();
};

/* A row with a choice: OptionItem that also draws the image of its value, from a table: the level, the sound,
 * two ON/OFF options and the vibration. */
class OptionChoiceItem : public OptionItem {
public:
    /* 0x0C */ TitleImage **images; /* one per value */

    OptionChoiceItem(TitleImage *image, TitleImage **images, s32 count) __asm__("OptionChoiceItem_init");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionChoiceItem_draw");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionChoiceItem_confirm");
    ~OptionChoiceItem();
};

/* The save and load rows: confirm starts a memory card task (memoryCardTaskInit) in mode, and the item keeps
 * that task while it runs. */
class OptionMemoryCardItem : public OptionItem {
public:
    /* 0x0C */ const s32 mode; /* MEMORY_CARD_OPTION_SAVE or _LOAD, fixed by the constructor */
    /* 0x10 */ Task *task;
    /* 0x14 */ u32 taskHandle; /* the task's handle, in TASK_HANDLES */

    OptionMemoryCardItem(TitleImage *image, s32 mode) __asm__("OptionMemoryCardItem_init");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionMemoryCardItem_confirm");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionMemoryCardItem_input");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionMemoryCardItem_cancel");
    ~OptionMemoryCardItem();
};

/* The screen position row: moves the screen (the save data's screenX and screenY). */
class OptionScreenItem : public OptionItem {
public:
    /* 0x0C */ s32 screenX; /* the screen position as shown (SaveData.screenX, screenY) */
    /* 0x10 */ s32 screenY;
    /* 0x14 */ s32 keptX;   /* and as it was when the item was confirmed; cancel goes back to it */
    /* 0x18 */ s32 keptY;
    /* 0x1C */ s32 moving;  /* the screen is being moved (confirm without own) */

    OptionScreenItem(TitleImage *image) __asm__("OptionScreenItem_init");
    void refresh(s32 arg) __asm__("OptionScreenItem_refresh");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionScreenItem_cancel");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionScreenItem_confirm");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionScreenItem_input");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionScreenItem_draw");
    ~OptionScreenItem();
};

/* The code row: a name entry on the code grid, whose codes unlock things (NameEntry_tryCode). */
class OptionCodeItem : public OptionItem {
public:
    /* 0x0C */ NameEntry entry;

    OptionCodeItem(TitleImage *image) __asm__("OptionCodeItem_init");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionCodeItem_confirm");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionCodeItem_input");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionCodeItem_draw");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionCodeItem_cancel");
    ~OptionCodeItem();
};

/* The button menu's row that puts the buttons back to the defaults. */
class OptionResetButtonsItem : public OptionItem {
public:
    OptionResetButtonsItem(TitleImage *image) __asm__("OptionResetButtonsItem_init");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionResetButtonsItem_confirm");
    ~OptionResetButtonsItem();
};

/* A button row of the button menu: the button slot of its row (its index) for both players, in the save
 * data. */
class OptionButtonItem : public OptionItem {
public:
    /* 0x0C */ s32 editing;  /* confirm without own started a change */
    /* 0x10 */ s32 slots[2]; /* by player, while editing */

    OptionButtonItem(TitleImage *image) __asm__("OptionButtonItem_init");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionButtonItem_confirm");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionButtonItem_cancel");
    void refresh(s32 arg) __asm__("OptionButtonItem_refresh");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionButtonItem_input");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionButtonItem_draw");
    ~OptionButtonItem();
};

/* The button settings submenu, a row of the options' menu: a StepperGroup of the six button rows, the
 * vibration and the reset, and an OptionItem (at 0x24) on one Stepper. It overrides what both of them
 * override, mostly by calling StepperGroup's. */
class OptionButtonMenu : public StepperGroup, public OptionItem {
public:
    OptionButtonMenu(TitleImage *image) __asm__("OptionButtonMenu_init");
    ~OptionButtonMenu();
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("OptionButtonMenu_confirm");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("OptionButtonMenu_input");
    void draw(s32 ot, s32 isCurrent, s32 active) __asm__("OptionButtonMenu_draw");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("OptionButtonMenu_cancel");
    void refresh(s32 arg) __asm__("OptionButtonMenu_refresh");
};

/* The options' task "Option", like main_menu.c's MainMenuTask with a frame count at 0x2C: its thread runs the
 * options' menu over the background. */
class OptionTask : public Task {
public:
    /* 0x20 */ Thread thread;
    /* 0x2C */ s32 frame;
    /* 0x30 */ u8 menu[0x44]; /* a StepperGroup: the options' menu */
    /* 0x74 */ Sprite background;

    OptionTask() __asm__("optionTaskInit");
    ~OptionTask();
    void run() __asm__("OptionTask_run");
    void update(s32 arg) __asm__("OptionTask_update");
};

#endif /* __cplusplus */

/* The rows (Stepper.index) whose save option OptionItem reads and writes: the options' menu's first two,
 * and the button menu's row after its six button rows. */
#define OPTION_ROW_LEVEL 0
#define OPTION_ROW_SOUND 1
#define BUTTON_MENU_ROW_VIBRATION 6

EXTERN_C_BEGIN

/* the first row of the options' menus on the screen: 1 while the button menu is open */
extern s32 OPTION_ROW_OFFSET;

s32 getOptionRowX(s32 index);
s32 getOptionRowY(s32 index);
SprtColorWords *optionDrawImage(u_long *ot, s32 x, s32 y, TitleImage *image);
SprtColorWords *optionDrawImageCell(u_long *ot, s32 x, s32 y, TitleImage *image, s32 index);

EXTERN_C_END

#endif /* DTBE_TITLE_OPTION_H */
