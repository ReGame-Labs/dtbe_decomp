/* The options (the task "Option", /title/option.tim). */

#include "common.h"
#include "engine/math/sin_cos.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_sprt_16.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_line_f2.h"
#include "engine/gfx/prim/alloc_dr_mode.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/ordering_table.h"
#include "engine/sound/sound_control.h"
#include "engine/game/game_state.h"
#include "engine/game/sequencer.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/pad/pad.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "engine/cd/file_load.h"
#include "overlay.h"
#include "title/option.h"

EXTERN_C_BEGIN
/* name_entry.cpp's (ranking.c declares them too) */
void NameEntry_open(NameEntry *entry);
void NameEntry_update(NameEntry *entry);
void NameEntry_draw(NameEntry *entry);
NameEntry *NameEntry_init(NameEntry *entry, s32 mode, s32 maxLength);
/* name_entry.cpp defines these two without a parameter, but the options pass them the entry */
void loadZasciiTim(NameEntry *entry);
void loadInputTim(NameEntry *entry);

void optionThreadEntry(s32 unused, void *object);

/* the image OptionButtonMenu's draw puts right of the menu's title row */
extern TitleImage OPTION_BUTTON_MENU_TITLE;
/* the two images of the options' screen's header */
extern TitleImage OPTION_HEADER_IMAGES[2];
/* the texture windows of the background strips (the whole page) and of the header images */
extern RECT OPTION_STRIP_WINDOW;
extern RECT OPTION_HEADER_WINDOW;
/* the buttons that step through the options' menus' rows */
extern StepperButtons OPTION_MENU_BUTTONS;
/* the button menu's images: the labels of its rows ([0], [7], [8], [9], [4], [5] the six button rows, [10] the
 * vibration, [6] the reset), and what OptionButtonItem's draw shows ([1], and [2], the icons, by cell) */
extern TitleImage OPTION_BUTTON_MENU_IMAGES[11];
/* the vibration row's two values */
extern TitleImage *OPTION_VIBRATION_IMAGES[2];
/* the labels of the options' menu's rows */
extern TitleImage OPTION_ROW_IMAGES[9];
/* the values of the options' menu's first four rows: the level (three), the sound, row 2's and the auto save */
extern TitleImage *OPTION_LEVEL_IMAGES[3];
extern TitleImage *OPTION_SOUND_IMAGES[2];
extern TitleImage *D_80066B48[2];
extern TitleImage *OPTION_ON_OFF_IMAGES[2];
/* the background picture */
extern TIM_IMAGE OPTION_BACKGROUND_TIM;

/* a line from (x0, y0) to (x1, y1) */
typedef struct ScreenPoint {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
} ScreenPoint;

typedef struct ScreenLine {
    /* 0x0 */ ScreenPoint from;
    /* 0x4 */ ScreenPoint to;
} ScreenLine;

/* the guides OptionScreenItem's draw shows while the screen moves: x 16 and 303, y 16 and 223 */
#define OPTION_SCREEN_LINE_COUNT 4
extern ScreenLine OPTION_SCREEN_LINES[OPTION_SCREEN_LINE_COUNT];
EXTERN_C_END

/* The x of the menu row index: 45, or 35 on every other row. */
s32 getOptionRowX(s32 index) {
    return 0x2D - ((index + OPTION_ROW_OFFSET) & 1) * 10;
}

/* The y of the menu row index: 17 pixels a row, from 51. */
s32 getOptionRowY(s32 index) {
    return (index + OPTION_ROW_OFFSET) * 17 + 0x33;
}

/* The same as drawImage. */
SprtColorWords *optionDrawImage(u_long *ot, s32 x, s32 y, TitleImage *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut.word;
    sprt->wh = image->wh.word;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* optionDrawImage's 16 by 16 version: draws cell index of image (u moved by 16 per cell) at (x, y). */
SprtColorWords *optionDrawImageCell(u_long *ot, s32 x, s32 y, TitleImage *image, s32 index) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt16();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut.word + (index << 4);
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* Steps the value up, playing the step sound when it moved, then applies it. */
void OptionItem::stepUp(s32 arg, s32 held, u32 buttons) {
    if (stepperStepUpClamp() == 0) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_STEP, 0);
    }
    apply();
}

/* Steps the value down, playing the step sound when it moved (and shaking pad 1 on the vibration row, which
 * turns it on), then applies it. */
void OptionItem::stepDown(s32 arg, s32 held, u32 buttons) {
    if (stepperStepDownClamp() == 0) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_STEP, 0);
        if (this->index == BUTTON_MENU_ROW_VIBRATION) {
            /* The match depends on this local: written in the call, the constants take other registers. */
            PadManager *padManager = PAD_MANAGER_INSTANCE;

            padManagerStartVibration(padManager, 0, 0, 0xFF, 0xFF, 0xA, 0);
        }
    }
    apply();
}

/* Reads the option of the item's row from the save data into the value. */
void OptionItem::refresh(s32 arg) {
    switch (index) {
    case OPTION_ROW_LEVEL:
        value = GAME_STATE.save.level;
        return;
    case OPTION_ROW_SOUND:
        value = GAME_STATE.save.mono;
        return;
    case 2:
        value = GAME_STATE.save.unk8;
        return;
    case 3:
        value = GAME_STATE.save.autoSave == 0;
        return;
    case BUTTON_MENU_ROW_VIBRATION:
        value = GAME_STATE.save.vibration == 0;
        return;
    }
}

/* Writes the value into the option of the item's row in the save data, then applies the options. */
void OptionItem::apply() {
    switch (index) {
    case OPTION_ROW_LEVEL:
        GAME_STATE.save.level = value;
        break;
    case OPTION_ROW_SOUND:
        GAME_STATE.save.mono = value == 1;
        break;
    case 2:
        GAME_STATE.save.unk8 = value == 1;
        break;
    case 3:
        GAME_STATE.save.autoSave = value == 0;
        break;
    case BUTTON_MENU_ROW_VIBRATION:
        GAME_STATE.save.vibration = value == 0;
        break;
    }
    saveDataApplyOptions(&GAME_STATE.save);
}

/* Plays the cursor sound. */
void OptionItem::focus(s32 arg) {
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
}

/* Accepts, unless own is set. */
s32 OptionItem::cancel(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    return 0;
}

/* Constructs an item drawing image, not blinking. */
OptionItem::OptionItem(TitleImage *image) {
    this->image = image;
    this->blink = 0;
}

/* Plays the confirm sound unless own is set; always accepts. */
s32 OptionItem::confirm(s32 arg, s32 held, s32 own) {
    /* The match depends on the early return, MainMenuItem_cancel's shape: the return label it leaves keeps
     * the final v0 = 0 out of the epilogue's load delay slot. */
    if (own != 0) {
        return 0;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    return 0;
}

/* Moves the blink on; always takes the input. */
s32 OptionItem::input(s32 arg, s32 held, u32 buttons) {
    this->blink += 0x20;
    return 1;
}

/* engine getCos, inlined.
 * The match depends on this s16 inline: a table read in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* The item's brightness: 128 while it is current, 64 otherwise, swung by 1/128 of the blink's cosine. */
s32 OptionItem::getBrightness(s32 isCurrent) {
    s32 swing = getCosInline(this->blink) >> 7;

    /* The match depends on testing !isCurrent: it keeps the 128 return on the fall-through path. */
    if (!isCurrent) {
        return swing + 0x40;
    }
    return swing + 0x80;
}

/* Draws the item's image at its row, its brightness from getBrightness; an inactive item stops blinking.
 * Stepper's draw passes the ordering table as ot. */
void OptionItem::draw(s32 ot, s32 isCurrent, s32 active) {
    s32 index = this->index;
    SPRT *sprt = (SPRT *)optionDrawImage((u_long *)ot, getOptionRowX(index), getOptionRowY(index), this->image);
    s32 brightness;

    if (!active) {
        this->blink = 0;
    }
    brightness = getBrightness(isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

/* Constructs an item drawing image, and one of the count images for its value. */
OptionChoiceItem::OptionChoiceItem(TitleImage *image, TitleImage **images, s32 count) : OptionItem(image) {
    this->max = count - 1;
    this->images = images;
}

/* Draws OptionItem's, then the image of the value 128 pixels right of it, as bright. */
void OptionChoiceItem::draw(s32 ot, s32 isCurrent, s32 active) {
    TitleImage **image;
    s32 index;
    SPRT *sprt;
    s32 brightness;

    OptionItem::draw(ot, isCurrent, active);
    image = &this->images[this->value];
    index = this->index;
    sprt = (SPRT *)optionDrawImage((u_long *)ot, getOptionRowX(index) + 0x80, getOptionRowY(index), *image);
    brightness = getBrightness(isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

/* Constructs an item drawing image that starts a memory card task in mode, with no task yet.
 * The match depends on mode being a const member set by the initializer: g++ marks that store unchanging,
 * so it may alias the epilogue's register reloads and stays above them. */
OptionMemoryCardItem::OptionMemoryCardItem(TitleImage *image, s32 mode) : OptionItem(image), mode(mode) {
    this->task = NULL;
    this->taskHandle = 0;
}

/* Starts the memory card task in mode, unless the one it started still runs. */
s32 OptionMemoryCardItem::confirm(s32 arg, s32 held, s32 own) {
    Task *task;

    if (handleTableGet(TASK_HANDLES, this->taskHandle) != NULL) {
        return -1;
    }
    task = memoryCardTaskInit(operatorNew(0x58), this->mode);
    schedulerInsertTask(&SCHEDULER, task);
    this->task = task;
    this->taskHandle = task->handle;
    return 0;
}

/* OptionItem's; keeps the input while the memory card task runs, then forgets the task (and opens the
 * group again after a load). */
s32 OptionMemoryCardItem::input(s32 arg, s32 held, u32 buttons) {
    OptionItem::input(arg, held, buttons);
    if (handleTableGet(TASK_HANDLES, this->taskHandle) != NULL) {
        return 1;
    }
    this->task = NULL;
    this->taskHandle = 0;
    if (this->mode == MEMORY_CARD_OPTION_LOAD) {
        stepperGetGroup()->stepperGroupOpen(0);
    }
    return 0;
}

/* Does nothing. */
s32 OptionMemoryCardItem::cancel(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Kills the memory card task it started, if it still runs. */
OptionMemoryCardItem::~OptionMemoryCardItem() {
    killTaskByHandle(this->taskHandle);
}

/* Constructs an item drawing image, reading the screen position of the save data (refresh, which the
 * constructor calls directly and passes the item itself: a1 is this). */
OptionScreenItem::OptionScreenItem(TitleImage *image) : OptionItem(image) {
    refresh((s32)this);
}

/* Reads the screen position of the save data, as shown and as kept. The match depends on the assignment
 * chains, which store their right-hand field first. */
void OptionScreenItem::refresh(s32 arg) {
    this->keptX = this->screenX = GAME_STATE.save.screenX;
    this->keptY = this->screenY = GAME_STATE.save.screenY;
}

/* Puts the kept screen position back, into the save data too, applies it and plays the back sound. */
s32 OptionScreenItem::cancel(s32 arg, s32 held, s32 own) {
    GAME_STATE.save.screenX = this->screenX = this->keptX;
    GAME_STATE.save.screenY = this->screenY = this->keptY;
    saveDataApplyOptions(&GAME_STATE.save);
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
    return 0;
}

/* Plays the confirm sound and starts moving the screen; own keeps the position moved to and stops. */
s32 OptionScreenItem::confirm(s32 arg, s32 held, s32 own) {
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    if (own != 0) {
        this->moving = 0;
        this->keptX = this->screenX;
        this->keptY = this->screenY;
        return -1;
    }
    this->moving = 1;
    return 0;
}

/* how far the screen moves from its place: x -8 to 8, y 0 to 8 */
#define OPTION_SCREEN_LIMIT 8

/* OptionItem's, then the d-pad moves the screen a pixel a frame (x -8 to 8, y 0 to 8) and applies it through the
 * save data; returns moving, so the input stays here while the screen is moved. */
s32 OptionScreenItem::input(s32 arg, s32 held, u32 buttons) {
    OptionItem::input(arg, held, buttons);
    if (buttons & PAD_LEFT) {
        this->screenX--;
    }
    if (buttons & PAD_RIGHT) {
        this->screenX++;
    }
    if (buttons & PAD_UP) {
        this->screenY--;
    }
    if (buttons & PAD_DOWN) {
        this->screenY++;
    }
    /* The match depends on a variable per bound, and on the first bound kept in the variable it is tested
     * against (slt): a constant test, as in the other three, folds to slti -7. */
    {
        s32 x = -OPTION_SCREEN_LIMIT;
        if (x < this->screenX) {
            x = this->screenX;
        }
        this->screenX = x;
    }
    {
        s32 x = OPTION_SCREEN_LIMIT;
        if (this->screenX < OPTION_SCREEN_LIMIT) {
            x = this->screenX;
        }
        this->screenX = x;
    }
    {
        s32 y = 0;
        if (this->screenY > 0) {
            y = this->screenY;
        }
        this->screenY = y;
    }
    {
        s32 y = OPTION_SCREEN_LIMIT;
        if (this->screenY < OPTION_SCREEN_LIMIT) {
            y = this->screenY;
        }
        this->screenY = y;
    }
    GAME_STATE.save.screenX = this->screenX;
    GAME_STATE.save.screenY = this->screenY;
    saveDataApplyOptions(&GAME_STATE.save);
    return this->moving;
}

INCLUDE_RODATA("title/nonmatchings/option", OPTION_SCREEN_LINES);

/* Draws OptionItem's, and while active the four white guide lines 16 pixels inside the screen's edges
 * (OPTION_SCREEN_LINES), near the front of the frame. */
void OptionScreenItem::draw(s32 ot, s32 isCurrent, s32 active) {
    s32 i;

    OptionItem::draw(ot, isCurrent, active);
    if (active) {
        u_long *front = FRAME_OT.firstDrawn + 2;

        for (i = 0; i < OPTION_SCREEN_LINE_COUNT; i++) {
            LINE_F2 *line = allocLineF2();
            /* The match depends on reading the four coordinates into u16 locals before the color: the loads
             * come first, as lhu. */
            u16 x0 = OPTION_SCREEN_LINES[i].from.x;
            u16 y0 = OPTION_SCREEN_LINES[i].from.y;
            u16 x1 = OPTION_SCREEN_LINES[i].to.x;
            u16 y1 = OPTION_SCREEN_LINES[i].to.y;

            setRGB0(line, 0xFF, 0xFF, 0xFF);
            line->x0 = x0;
            line->y0 = y0;
            line->x1 = x1;
            line->y1 = y1;
            AddPrim(front, line);
        }
    }
}

/* Constructs an item drawing image, with a name entry on the second character grid whose maxLength goes
 * from 5 to 6, and loads the entry's images ("/zascii.tim", "/title/input.tim"). */
OptionCodeItem::OptionCodeItem(TitleImage *image) : OptionItem(image) {
    NameEntry_init(&this->entry, NAME_ENTRY_MODE_CODE, 5);
    this->entry.maxLength = 6;
    loadZasciiTim(&this->entry);
    loadInputTim(&this->entry);
}

/* Plays the confirm sound and opens the name entry, which the group then draws alone. */
s32 OptionCodeItem::confirm(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    NameEntry_open(&this->entry);
    stepperSetGroupCurrentOnly(0);
    return 0;
}

/* Runs the name entry while it is open; once it closed, the group draws all its items again. */
s32 OptionCodeItem::input(s32 arg, s32 held, u32 buttons) {
    if (this->entry.active) {
        NameEntry_update(&this->entry);
        NameEntry_draw(&this->entry);
        return 1;
    }
    stepperSetGroupCurrentOnly(1);
    return 0;
}

/* Draws OptionItem's. */
void OptionCodeItem::draw(s32 ot, s32 isCurrent, s32 active) {
    OptionItem::draw(ot, isCurrent, active);
}

/* Constructs an item drawing image. */
OptionResetButtonsItem::OptionResetButtonsItem(TitleImage *image) : OptionItem(image) {
}

/* Constructs an item drawing image. */
OptionButtonItem::OptionButtonItem(TitleImage *image) : OptionItem(image) {
}

/* Plays the confirm sound; starts changing the two players' button slots of this row, or, with own, stores them in
 * the save data and opens the group again. */
s32 OptionButtonItem::confirm(s32 arg, s32 held, s32 own) {
    SaveData *save = &GAME_STATE.save;
    s32 slot = this->index;

    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    if (own != 0) {
        saveDataSetButtonSlot(save, 0, slot, this->slots[0]);
        saveDataSetButtonSlot(save, 1, slot, this->slots[1]);
        stepperGetGroup()->stepperGroupOpen(0);
        this->editing = 0;
    } else {
        this->slots[0] = saveDataGetButtonSlot(save, 0, slot);
        this->slots[1] = saveDataGetButtonSlot(save, 1, slot);
        this->editing = 1;
    }
    return 0;
}

/* With own, reads the slots again (refresh) and plays the back sound. */
s32 OptionButtonItem::cancel(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        refresh(0);
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
    }
    return 0;
}

/* Reads the two players' button slots of this row from the save data. */
void OptionButtonItem::refresh(s32 arg) {
    SaveData *save = &GAME_STATE.save;
    s32 slot = this->index;

    this->slots[0] = saveDataGetButtonSlot(save, 0, slot);
    this->slots[1] = saveDataGetButtonSlot(save, 1, slot);
}

/* OptionItem's, then left and right on each player's pad step that player's slot of this row (wrapping over the
 * six), with the step sound for the given buttons or pad 2's; returns editing. */
s32 OptionButtonItem::input(s32 arg, s32 held, u32 buttons) {
    PadManager *padManager;
    s32 slot0;
    s32 slot1;
    u32 stepped;

    OptionItem::input(arg, held, buttons);
    /* The match depends on buttons taking each pad's repeated buttons in turn (a separate variable takes other
     * registers). */
    stepped = buttons;
    padManager = PAD_MANAGER_INSTANCE;
    slot0 = this->slots[0];
    slot1 = this->slots[1];
    buttons = padManagerGetRepeated(padManager, 0);
    if (buttons & PAD_LEFT) {
        slot0--;
    }
    if (buttons & PAD_RIGHT) {
        slot0++;
    }
    buttons = padManagerGetRepeated(padManager, 1);
    stepped |= buttons;
    if (stepped & (PAD_LEFT | PAD_RIGHT)) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_STEP, 0);
    }
    if (buttons & PAD_LEFT) {
        slot1--;
    }
    if (buttons & PAD_RIGHT) {
        slot1++;
    }
    if (slot0 < 0) {
        slot0 += BUTTON_SLOT_COUNT;
    }
    if (slot0 >= BUTTON_SLOT_COUNT) {
        slot0 = 0;
    }
    if (slot1 < 0) {
        slot1 += BUTTON_SLOT_COUNT;
    }
    if (slot1 >= BUTTON_SLOT_COUNT) {
        slot1 = 0;
    }
    this->slots[0] = slot0;
    this->slots[1] = slot1;
    return this->editing;
}

INCLUDE_ASM("title/nonmatchings/option", OptionButtonItem_draw);

/* The button settings submenu: a group stepped by OPTION_MENU_BUTTONS, wrapping, open, of the six button rows, the
 * vibration (two values) and the reset. */
OptionButtonMenu::OptionButtonMenu(TitleImage *image) : StepperGroup(&OPTION_MENU_BUTTONS), OptionItem(image) {
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[0]));
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[7]));
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[8]));
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[9]));
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[4]));
    stepperGroupAddChild(new OptionButtonItem(&OPTION_BUTTON_MENU_IMAGES[5]));
    stepperGroupAddChild(new OptionChoiceItem(&OPTION_BUTTON_MENU_IMAGES[10], OPTION_VIBRATION_IMAGES, 2));
    stepperGroupAddChild(new OptionResetButtonsItem(&OPTION_BUTTON_MENU_IMAGES[6]));
    stepperGroupOpen(0);
    wraps = 1;
}

OptionButtonMenu::~OptionButtonMenu() {
}

/* Starts the menu from row 1 from now on; plays the confirm sound and opens the group, drawn alone, unless own. */
s32 OptionButtonMenu::confirm(s32 arg, s32 held, s32 own) {
    OPTION_ROW_OFFSET = 1;
    if (own != 0) {
        return 0;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    stepperSetGroupCurrentOnly(0);
    stepperGetGroup()->stepperGroupOpen(0);
    return 0;
}

/* StepperGroup's. */
s32 OptionButtonMenu::input(s32 arg, s32 held, u32 buttons) {
    return StepperGroup::input(arg, held, buttons);
}

/* While open, StepperGroup's and OPTION_BUTTON_MENU_TITLE at full brightness right of the title row
 * (index -1); else OptionItem's, not blinking. */
void OptionButtonMenu::draw(s32 ot, s32 isCurrent, s32 active) {
    SPRT *sprt;

    if (!active) {
        OptionItem::draw(ot, isCurrent, 0);
    } else {
        StepperGroup::draw(ot, isCurrent, active);
        sprt = (SPRT *)optionDrawImage((u_long *)ot, getOptionRowX(-1) + 0x84, getOptionRowY(-1),
                                       &OPTION_BUTTON_MENU_TITLE);
        setRGB0(sprt, 0x80, 0x80, 0x80);
    }
}

/* Constructs the options' task "Option": its thread, the options' menu (the level, the sound, row 2, the auto save,
 * the memory card's two rows, the button menu, the screen position and the code), wrapping and open, and the
 * background; loads the options' pictures and sound 0x8003 and plays it. The match depends on reaching the menu
 * through this->menu each time (with a local pointer the wraps store goes through it, not this). */
OptionTask::OptionTask() : Task(0x2000, "Option") {
    u32 *file;

    threadInit(&this->thread, optionThreadEntry, 0x1000, this, 0);
    stepperGroupInit(this->menu, 1, &OPTION_MENU_BUTTONS);
    spriteInit(&this->background);
    file = (u32 *)loadFile("/title/option.tim");
    uploadTims(file);
    mainHeapFree(file);
    cacheReset(&getFontCache()->cache);
    soundControlLoadSound(&SOUND_CONTROL, 0x8003, 1);
    soundControlPlaySound(&SOUND_CONTROL, 0x8003, 0);
    ((StepperGroup *)this->menu)->stepperGroupAddChild(
        new OptionChoiceItem(&OPTION_ROW_IMAGES[0], OPTION_LEVEL_IMAGES, 3));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(
        new OptionChoiceItem(&OPTION_ROW_IMAGES[1], OPTION_SOUND_IMAGES, 2));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(new OptionChoiceItem(&OPTION_ROW_IMAGES[2], D_80066B48, 2));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(
        new OptionChoiceItem(&OPTION_ROW_IMAGES[3], OPTION_ON_OFF_IMAGES, 2));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(
        new OptionMemoryCardItem(&OPTION_ROW_IMAGES[4], MEMORY_CARD_OPTION_SAVE));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(
        new OptionMemoryCardItem(&OPTION_ROW_IMAGES[5], MEMORY_CARD_OPTION_LOAD));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(new OptionButtonMenu(&OPTION_ROW_IMAGES[6]));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(new OptionScreenItem(&OPTION_ROW_IMAGES[7]));
    ((StepperGroup *)this->menu)->stepperGroupAddChild(new OptionCodeItem(&OPTION_ROW_IMAGES[8]));
    ((StepperGroup *)this->menu)->stepperGroupOpen(0);
    ((StepperGroup *)this->menu)->wraps = 1;
    spriteSetTim(&this->background, &OPTION_BACKGROUND_TIM, 0);
    this->frame = 0;
}

/* Fades the screen in and destroys the members; the thread, the menu's StepperGroup and the sprite are C
 * structs here, so their destructors are called by hand. */
OptionTask::~OptionTask() {
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->background, DESTROY_BASES);
    stepperGroupDestroy(this->menu, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
}

/* The task's thread: fades the screen in (10 frames), runs the options' menu until it closes (input from both
 * pads), fades out, kills the task and stops the songs, then hands back -1 every frame. */
void OptionTask::run() {
    s32 closed;
    Thread *thread;

    fadeControlFadeOut(&FADE_CONTROL, 0);
    fadeControlFadeIn(&FADE_CONTROL, 10);
    while (!fadeControlIsDone(&FADE_CONTROL)) {
        threadSwitchBack(&this->thread, 0);
    }
    /* The match depends on the thread taken into this variable only after the fade-in's wait, which uses the
     * field directly. */
    thread = &this->thread;
    do {
        PadManager *pads = PAD_MANAGER_INSTANCE;
        u32 held = padManagerGetHeld(pads, -1);
        u32 repeated = padManagerGetRepeated(pads, -1);

        closed = ((StepperGroup *)this->menu)->stepperGroupInputIfOpen(0, held, repeated) == 0;
        threadSwitchBack(thread, 0);
    } while (!closed);
    fadeControlFadeOut(&FADE_CONTROL, 10);
    while (!fadeControlIsDone(&FADE_CONTROL)) {
        threadSwitchBack(thread, 0);
    }
    taskKill(this);
    stopSongs();
    for (;;) {
        threadSwitchBack(thread, -1);
    }
}

/* The entry of the thread of optionTaskInit's task. */
void optionThreadEntry(s32 unused, void *object) {
    ((OptionTask *)object)->run();
}

/* The task's update: runs its thread for a frame, then draws the scrolling background strips at the screen's
 * sides, the two header images, the background sprite and the menu. */
void OptionTask::update(s32 arg) {
    u_long *ot;
    SprtColorWords *sprt;

    if (threadSwitchTo(&this->thread, 0) < 0) {
        return;
    }
    ot = FRAME_OT.firstDrawn + 5;
    AddPrim(ot, allocDrMode(0, &OPTION_STRIP_WINDOW));
    sprt = (SprtColorWords *)allocSprt();
    sprt->xy = 2;
    sprt->rgbCode = (((SPRT *)sprt)->code << 24) | (PRIM_SEMI_TRANS << 24) | 0x808080;
    sprt->uvClut = (GetClut(320, 64) << 16) | ((u8)this->frame << 8);
    sprt->wh = (240 << 16) | 16;
    AddPrim(ot, sprt);
    sprt = (SprtColorWords *)allocSprt();
    sprt->xy = 302;
    sprt->rgbCode = (((SPRT *)sprt)->code << 24) | (PRIM_SEMI_TRANS << 24) | 0x808080;
    sprt->uvClut = (GetClut(320, 64) << 16) | ((u8)-this->frame << 8);
    sprt->wh = (240 << 16) | 16;
    AddPrim(ot, sprt);
    this->frame++;
    AddPrim(ot, allocDrMode(5, &OPTION_HEADER_WINDOW));
    sprt = optionDrawImage(ot, 47, 13, &OPTION_HEADER_IMAGES[0]);
    setRGB0((SPRT *)sprt, 0x80, 0x80, 0x80);
    sprt = optionDrawImage(ot, 94, 208, &OPTION_HEADER_IMAGES[1]);
    setRGB0((SPRT *)sprt, 0x80, 0x80, 0x80);
    PRIM_BUFFER_FREE = (u8 *)spriteDraw(&this->background, PRIM_BUFFER_FREE, FRAME_OT.firstDrawn + 1);
    ((StepperGroup *)this->menu)->stepperGroupDrawIfOpen((s32)(FRAME_OT.firstDrawn + 4));
}

OptionChoiceItem::~OptionChoiceItem() {
}

OptionScreenItem::~OptionScreenItem() {
}

/* The name entry is a C struct (name_entry.cpp keeps it one, with C names), so its destructor is called
 * by hand, where g++ calls a member's. */
OptionCodeItem::~OptionCodeItem() {
    NameEntry_destroy(&this->entry, DESTROY_BASES);
}

OptionResetButtonsItem::~OptionResetButtonsItem() {
}

OptionButtonItem::~OptionButtonItem() {
}

OptionItem::~OptionItem() {
}

/* Does nothing. */
s32 OptionChoiceItem::confirm(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Does nothing. */
s32 OptionCodeItem::cancel(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Plays the confirm sound, puts the save data's buttons back to the defaults and opens the item's group. */
s32 OptionResetButtonsItem::confirm(s32 arg, s32 held, s32 own) {
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    saveDataResetButtons(&GAME_STATE.save);
    stepperGetGroup()->stepperGroupOpen(0);
    return -1;
}

/* StepperGroup's; a menu that was cancelled starts again from the first row. */
s32 OptionButtonMenu::cancel(s32 arg, s32 held, s32 own) {
    s32 result = StepperGroup::cancel(arg, held, own);

    if (result == 0) {
        OPTION_ROW_OFFSET = 0;
    }
    return result;
}

/* StepperGroup's. */
void OptionButtonMenu::refresh(s32 arg) {
    StepperGroup::refresh(arg);
}

INCLUDE_RODATA("title/nonmatchings/option", OPTION_BACKGROUND_CLUT_RECT);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_BACKGROUND_PIXEL_RECT);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_BACKGROUND_TIM);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_STRIP_WINDOW);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_HEADER_WINDOW);
INCLUDE_RODATA("title/nonmatchings/option", _vt.16OptionButtonMenu.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.16OptionButtonItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.22OptionResetButtonsItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.14OptionCodeItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.16OptionScreenItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.20OptionMemoryCardItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.16OptionChoiceItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.10OptionItem.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.10OptionTask);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_MENU_BUTTONS);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_HEADER_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_ROW_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A2C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A38);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A44);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A50);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A5C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A68);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A74);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_IMAGE_ON);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_IMAGE_OFF);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_BUTTON_MENU_TITLE);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_BUTTON_MENU_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_LEVEL_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_SOUND_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B48);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_ON_OFF_IMAGES);
INCLUDE_RODATA("title/nonmatchings/option", OPTION_VIBRATION_IMAGES);
