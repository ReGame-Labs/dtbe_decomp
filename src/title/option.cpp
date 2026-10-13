/* The options (the task "Option", /title/option.tim). */

#include "common.h"
#include "engine/math/sin_cos.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_sprt_16.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/sound/sound_control.h"
#include "engine/game/game_state.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/pad/pad.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "overlay.h"
#include "title/option.h"

EXTERN_C_BEGIN
/* name_entry.c's (ranking.c declares them too) */
void func_80074318(Unk80074220 *entry);
void func_800744BC(Unk80074220 *entry);
void func_8007460C(Unk80074220 *entry);
Unk80074220 *func_80074130(Unk80074220 *entry, s32 grid, s32 maxLength);
/* name_entry.c defines these two without a parameter, but the options pass them the entry */
void func_8007434C(Unk80074220 *entry);
void func_80074388(Unk80074220 *entry);

/* the image Unk80079254's draw puts right of the menu's title row */
extern Unk80073390Image D_80066AA4;
EXTERN_C_END

/* The x of the menu row index: 45, or 35 on every other row. */
s32 func_80077690(s32 index) {
    return 0x2D - ((index + D_8007B32C) & 1) * 10;
}

/* The y of the menu row index: 17 pixels a row, from 51. */
s32 func_800776BC(s32 index) {
    return (index + D_8007B32C) * 17 + 0x33;
}

/* The same as func_80073390. */
SprtColorWords *func_800776DC(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* func_800776DC's 16 by 16 version: draws cell index of image (u moved by 16 per cell) at (x, y). */
SprtColorWords *func_80077778(u_long *ot, s32 x, s32 y, Unk80073390Image *image, s32 index) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt16();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut + (index << 4);
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* Steps the value up, playing sound 0x13 when it moved, then func_8007799C. */
void Unk80077AC8::stepUp(s32 arg, s32 held, u32 buttons) {
    if (stepperStepUpClamp() == 0) {
        soundControlPlaySound(&SOUND_CONTROL, 0x13, 0);
    }
    func_8007799C();
}

/* Steps the value down, playing sound 0x13 when it moved (and shaking pad 1 on row 6), then func_8007799C. */
void Unk80077AC8::stepDown(s32 arg, s32 held, u32 buttons) {
    if (stepperStepDownClamp() == 0) {
        soundControlPlaySound(&SOUND_CONTROL, 0x13, 0);
        if (this->index == 6) {
            /* The match depends on this local: written in the call, the constants take other registers. */
            PadManager *padManager = PAD_MANAGER_INSTANCE;

            padManagerStartVibration(padManager, 0, 0, 0xFF, 0xFF, 0xA, 0);
        }
    }
    func_8007799C();
}

/* Reads the option of the item's row from the save data into the value. */
void Unk80077AC8::refresh(s32 arg) {
    switch (index) {
    case 0:
        value = GAME_STATE.save.level;
        return;
    case 1:
        value = GAME_STATE.save.mono;
        return;
    case 2:
        value = GAME_STATE.save.unk8;
        return;
    case 3:
        value = GAME_STATE.save.unk4 == 0;
        return;
    case 6:
        value = GAME_STATE.save.unkC == 0;
        return;
    }
}

/* Writes the value into the option of the item's row in the save data, then applies the options. */
void Unk80077AC8::func_8007799C() {
    switch (index) {
    case 0:
        GAME_STATE.save.level = value;
        break;
    case 1:
        GAME_STATE.save.mono = value == 1;
        break;
    case 2:
        GAME_STATE.save.unk8 = value == 1;
        break;
    case 3:
        GAME_STATE.save.unk4 = value == 0;
        break;
    case 6:
        GAME_STATE.save.unkC = value == 0;
        break;
    }
    saveDataApplyOptions(&GAME_STATE.save);
}

/* Plays sound 0xF. */
void Unk80077AC8::focus(s32 arg) {
    soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
}

/* Accepts, unless own is set. */
s32 Unk80077AC8::cancel(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    return 0;
}

/* Constructs an item drawing image, not blinking. */
Unk80077AC8::Unk80077AC8(Unk80073390Image *image) {
    this->image = image;
    this->blink = 0;
}

/* Plays sound 0x10 unless own is set; always accepts. */
s32 Unk80077AC8::confirm(s32 arg, s32 held, s32 own) {
    /* The match depends on the early return, func_8006A0E0's shape: the return label it leaves keeps
     * the final v0 = 0 out of the epilogue's load delay slot. */
    if (own != 0) {
        return 0;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    return 0;
}

/* Moves the blink on; always takes the input. */
s32 Unk80077AC8::input(s32 arg, s32 held, u32 buttons) {
    this->blink += 0x20;
    return 1;
}

/* engine getCos, inlined.
 * The match depends on this s16 inline: a table read in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* The item's brightness: 128 while it is current, 64 otherwise, swung by 1/128 of the blink's cosine. */
s32 Unk80077AC8::func_80077C44(s32 isCurrent) {
    s32 swing = getCosInline(this->blink) >> 7;

    /* The match depends on testing !isCurrent: it keeps the 128 return on the fall-through path. */
    if (!isCurrent) {
        return swing + 0x40;
    }
    return swing + 0x80;
}

/* Draws the item's image at its row, its brightness from func_80077C44; an inactive item stops blinking.
 * Stepper's draw passes the ordering table as arg. */
void Unk80077AC8::draw(s32 arg, s32 isCurrent, s32 active) {
    s32 index = this->index;
    SPRT *sprt = (SPRT *)func_800776DC((u_long *)arg, func_80077690(index), func_800776BC(index), this->image);
    s32 brightness;

    if (!active) {
        this->blink = 0;
    }
    brightness = func_80077C44(isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

/* Constructs an item drawing image, and one of the count images for its value. */
Unk80077D20::Unk80077D20(Unk80073390Image *image, Unk80073390Image **images, s32 count) : Unk80077AC8(image) {
    this->max = count - 1;
    this->images = images;
}

/* Draws Unk80077AC8's, then the image of the value 128 pixels right of it, as bright. */
void Unk80077D20::draw(s32 arg, s32 isCurrent, s32 active) {
    Unk80073390Image **image;
    s32 index;
    SPRT *sprt;
    s32 brightness;

    Unk80077AC8::draw(arg, isCurrent, active);
    image = &this->images[this->value];
    index = this->index;
    sprt = (SPRT *)func_800776DC((u_long *)arg, func_80077690(index) + 0x80, func_800776BC(index), *image);
    brightness = func_80077C44(isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

/* Constructs an item drawing image that starts a memory card task in mode, with no task yet.
 * The match depends on mode being a const member set by the initializer: g++ marks that store unchanging,
 * so it may alias the epilogue's register reloads and stays above them. */
Unk80077F28::Unk80077F28(Unk80073390Image *image, s32 mode) : Unk80077AC8(image), mode(mode) {
    this->task = NULL;
    this->taskHandle = 0;
}

/* Starts the memory card task in mode, unless the one it started still runs. */
s32 Unk80077F28::confirm(s32 arg, s32 held, s32 own) {
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

/* Unk80077AC8's; keeps the input while the memory card task runs, then forgets the task (and opens the
 * group again in mode 0). */
s32 Unk80077F28::input(s32 arg, s32 held, u32 buttons) {
    Unk80077AC8::input(arg, held, buttons);
    if (handleTableGet(TASK_HANDLES, this->taskHandle) != NULL) {
        return 1;
    }
    this->task = NULL;
    this->taskHandle = 0;
    if (this->mode == 0) {
        stepperGetGroup()->stepperGroupOpen(0);
    }
    return 0;
}

/* Does nothing. */
s32 Unk80077F28::cancel(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Kills the memory card task it started, if it still runs. */
Unk80077F28::~Unk80077F28() {
    killTaskByHandle(this->taskHandle);
}

/* Constructs an item drawing image, reading the screen position of the save data (refresh, which the
 * constructor calls directly and passes the item itself: a1 is this). */
Unk800782D8::Unk800782D8(Unk80073390Image *image) : Unk80077AC8(image) {
    refresh((s32)this);
}

/* Reads the screen position of the save data, as shown and as kept. The match depends on the assignment
 * chains, which store their right-hand field first. */
void Unk800782D8::refresh(s32 arg) {
    this->keptX = this->screenX = GAME_STATE.save.screenX;
    this->keptY = this->screenY = GAME_STATE.save.screenY;
}

/* Puts the kept screen position back, into the save data too, applies it and plays sound 0x11. */
s32 Unk800782D8::cancel(s32 arg, s32 held, s32 own) {
    GAME_STATE.save.screenX = this->screenX = this->keptX;
    GAME_STATE.save.screenY = this->screenY = this->keptY;
    saveDataApplyOptions(&GAME_STATE.save);
    soundControlPlaySound(&SOUND_CONTROL, 0x11, 0);
    return 0;
}

/* Plays sound 0x10 and starts moving the screen; own keeps the position moved to and stops. */
s32 Unk800782D8::confirm(s32 arg, s32 held, s32 own) {
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    if (own != 0) {
        this->moving = 0;
        this->keptX = this->screenX;
        this->keptY = this->screenY;
        return -1;
    }
    this->moving = 1;
    return 0;
}

INCLUDE_ASM("title/nonmatchings/option", func_8007850C);

INCLUDE_RODATA("title/nonmatchings/option", D_80066600);

INCLUDE_ASM("title/nonmatchings/option", func_80078628);

/* Constructs an item drawing image, with a name entry on the second character grid whose maxLength goes
 * from 5 to 6, and loads the entry's images ("/zascii.tim", "/title/input.tim"). */
Unk800786E4::Unk800786E4(Unk80073390Image *image) : Unk80077AC8(image) {
    func_80074130(&this->entry, 1, 5);
    this->entry.maxLength = 6;
    func_8007434C(&this->entry);
    func_80074388(&this->entry);
}

/* Plays sound 0x10 and opens the name entry, which the group then draws alone. */
s32 Unk800786E4::confirm(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    func_80074318(&this->entry);
    stepperSetGroupCurrentOnly(0);
    return 0;
}

/* Runs the name entry while it is open; once it closed, the group draws all its items again. */
s32 Unk800786E4::input(s32 arg, s32 held, u32 buttons) {
    if (this->entry.active) {
        func_800744BC(&this->entry);
        func_8007460C(&this->entry);
        return 1;
    }
    stepperSetGroupCurrentOnly(1);
    return 0;
}

/* Draws Unk80077AC8's. */
void Unk800786E4::draw(s32 arg, s32 isCurrent, s32 active) {
    Unk80077AC8::draw(arg, isCurrent, active);
}

/* Constructs an item drawing image. */
Unk80078914::Unk80078914(Unk80073390Image *image) : Unk80077AC8(image) {
}

/* Constructs an item drawing image. */
Unk80078A4C::Unk80078A4C(Unk80073390Image *image) : Unk80077AC8(image) {
}

/* Plays sound 0x10; starts changing the two players' button slots of this row, or, with own, stores them in
 * the save data and opens the group again. */
s32 Unk80078A4C::confirm(s32 arg, s32 held, s32 own) {
    SaveData *save = &GAME_STATE.save;
    s32 slot = this->index;

    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
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

/* With own, reads the slots again (refresh) and plays sound 0x11. */
s32 Unk80078A4C::cancel(s32 arg, s32 held, s32 own) {
    if (own != 0) {
        refresh(0);
        soundControlPlaySound(&SOUND_CONTROL, 0x11, 0);
    }
    return 0;
}

/* Reads the two players' button slots of this row from the save data. */
void Unk80078A4C::refresh(s32 arg) {
    SaveData *save = &GAME_STATE.save;
    s32 slot = this->index;

    this->slots[0] = saveDataGetButtonSlot(save, 0, slot);
    this->slots[1] = saveDataGetButtonSlot(save, 1, slot);
}

INCLUDE_ASM("title/nonmatchings/option", func_80078D20);

INCLUDE_ASM("title/nonmatchings/option", func_80078E3C);

INCLUDE_ASM("title/nonmatchings/option", func_80078F5C);

Unk80079254::~Unk80079254() {
}

/* Starts the menu from row 1 from now on; plays sound 0x10 and opens the group, drawn alone, unless own. */
s32 Unk80079254::confirm(s32 arg, s32 held, s32 own) {
    D_8007B32C = 1;
    if (own != 0) {
        return 0;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    stepperSetGroupCurrentOnly(0);
    stepperGetGroup()->stepperGroupOpen(0);
    return 0;
}

/* StepperGroup's. */
s32 Unk80079254::input(s32 arg, s32 held, u32 buttons) {
    return StepperGroup::input(arg, held, buttons);
}

/* While open, StepperGroup's and D_80066AA4 at full brightness right of the title row (index -1); else
 * Unk80077AC8's, not blinking. */
void Unk80079254::draw(s32 arg, s32 isCurrent, s32 active) {
    SPRT *sprt;

    if (!active) {
        Unk80077AC8::draw(arg, isCurrent, 0);
    } else {
        StepperGroup::draw(arg, isCurrent, active);
        sprt = (SPRT *)func_800776DC((u_long *)arg, func_80077690(-1) + 0x84, func_800776BC(-1), &D_80066AA4);
        setRGB0(sprt, 0x80, 0x80, 0x80);
    }
}

INCLUDE_ASM("title/nonmatchings/option", optionTaskInit);

/* Fades the screen in and destroys the members; the thread, the StepperGroup and the sprite are C
 * structs here, so their destructors are called by hand. */
Unk800797D8::~Unk800797D8() {
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->sprite, DESTROY_BASES);
    stepperGroupDestroy(this->steppers, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
}

INCLUDE_ASM("title/nonmatchings/option", func_80079850);

INCLUDE_ASM("title/nonmatchings/option", func_80079978);

Unk80077D20::~Unk80077D20() {
}

Unk800782D8::~Unk800782D8() {
}

/* The name entry is a C struct here (name_entry.c is C), so its destructor is called by hand, where g++
 * calls a member's. */
Unk800786E4::~Unk800786E4() {
    func_80074220(&this->entry, DESTROY_BASES);
}

Unk80078914::~Unk80078914() {
}

Unk80078A4C::~Unk80078A4C() {
}

Unk80077AC8::~Unk80077AC8() {
}

/* Does nothing. */
s32 Unk80077D20::confirm(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Does nothing. */
s32 Unk800786E4::cancel(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Plays sound 0x10, puts the save data's buttons back to the defaults and opens the item's group. */
s32 Unk80078914::confirm(s32 arg, s32 held, s32 own) {
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    saveDataResetButtons(&GAME_STATE.save);
    stepperGetGroup()->stepperGroupOpen(0);
    return -1;
}

/* StepperGroup's; a menu that was cancelled starts again from the first row. */
s32 Unk80079254::cancel(s32 arg, s32 held, s32 own) {
    s32 result = StepperGroup::cancel(arg, held, own);

    if (result == 0) {
        D_8007B32C = 0;
    }
    return result;
}

/* StepperGroup's. */
void Unk80079254::refresh(s32 arg) {
    StepperGroup::refresh(arg);
}

INCLUDE_RODATA("title/nonmatchings/option", D_8006663C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066644);
INCLUDE_RODATA("title/nonmatchings/option", D_8006664C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066660);
INCLUDE_RODATA("title/nonmatchings/option", D_80066668);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80079254.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80078A4C.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80078914.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk800786E4.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk800782D8.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80077F28.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80077D20.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk80077AC8.7Stepper);
INCLUDE_RODATA("title/nonmatchings/option", _vt.11Unk800797D8);
INCLUDE_RODATA("title/nonmatchings/option", D_8006699C);
INCLUDE_RODATA("title/nonmatchings/option", D_800669A8);
INCLUDE_RODATA("title/nonmatchings/option", D_800669C0);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A2C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A38);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A44);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A50);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A5C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A68);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A74);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A80);
INCLUDE_RODATA("title/nonmatchings/option", D_80066A8C);
INCLUDE_RODATA("title/nonmatchings/option", D_80066AA4);
INCLUDE_RODATA("title/nonmatchings/option", D_80066AB0);
INCLUDE_RODATA("title/nonmatchings/option", D_80066AC8);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B34);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B40);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B48);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B50);
INCLUDE_RODATA("title/nonmatchings/option", D_80066B58);
