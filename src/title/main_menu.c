/* The main menu (the task "MainMenu", /title/mainmenu.tim). */

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/debug/system_menu.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/gfx/sprite.h"
#include "title/title_classes.h"

/* The StepperGroup (engine/menu/stepper.h) of the main menu's items, as C sees it. */
typedef struct MainMenuGroup {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u32 wraps : 1; /* moving past the last item goes to the first */
    /* 0x14 */ u8 unk14[0x44 - 0x14];
} MainMenuGroup;

/* The main menu's task (mainMenuTaskInit, virtual table MAIN_MENU_TASK_VTABLE): its thread runs the
 * StepperGroup of the items until one closes it; the update draws the background, the items, the arrows,
 * the rails and the banners. */
typedef struct MainMenuTask {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ MainMenuGroup items;
    /* 0x70 */ Sprite background;
    /* 0x88 */ s32 frame;
    /* 0x8C */ s16 leftX;  /* of the left arrow and rail, in 1/16 pixels: slides in after 0x10 frames */
    /* 0x8E */ s16 rightX;
} MainMenuTask;

extern TaskVtable MAIN_MENU_TASK_VTABLE;

/* Stepper's virtual table, which C reaches by its g++ name */
extern VtableEntry STEPPER_VTABLE[] __asm__("_vt.7Stepper");

/* An item of the main menu (virtual table MAIN_MENU_ITEM_VTABLE), a Stepper as C sees it: a Stepper's list
 * node, and its virtual table pointer at 0x1C. Its label slides up into place; once confirmed it flashes,
 * then closes the menu with its index, the menu's choice. */
typedef struct MainMenuItem {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *group;
    /* 0x0C */ s32 index; /* among the children of its group */
    /* 0x10 */ u8 unk10[0x1C - 0x10];
    /* 0x1C */ VtableEntry *vtable;
    /* 0x20 */ TitleImage *label;
    /* 0x24 */ s32 flashPhase; /* runs once the item is confirmed */
    /* 0x28 */ s32 y;          /* in 1/16 pixels, from 0x2000 up to its place */
    /* 0x2C */ s32 frame;      /* the items start to move one after the other */
    /* 0x30 */ s32 locked;     /* a "???" item, which refuses the confirm */
} MainMenuItem;

/* The pictures of /title/mainmenu.tim the main menu draws, in MAIN_MENU_IMAGES. */
enum MainMenuImage {
    MAIN_MENU_IMAGE_BACKGROUND = 0, /* 512x480; the background is drawn from MAIN_MENU_BACKGROUND_TIM instead */
    MAIN_MENU_IMAGE_RAIL = 1,       /* 32x256, drawn 312 high (mainMenuDrawRail) */
    MAIN_MENU_IMAGE_ARROW = 2,      /* at the cursor's item, beside each rail */
    MAIN_MENU_IMAGE_UNK3 = 3,       /* 32x256 as the rail; nothing draws it */
    MAIN_MENU_IMAGE_LABELS = 4,     /* the items' labels, 7 (MAIN_MENU_LABELS) */
    MAIN_MENU_IMAGE_LABEL_BAR = 11, /* drawn semi-transparent and dark behind every label */
    MAIN_MENU_IMAGE_HEADER = 12,    /* at the top of the screen */
    MAIN_MENU_IMAGE_FOOTER = 13,    /* at the bottom */
    MAIN_MENU_IMAGE_COUNT = 14
};

extern TitleImage MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_COUNT];

/* Stepper's constructor, as C sees it */
void *stepperInit(void *this, s32 value);

/* Stepper::stepperCloseGroup, as C sees it */
void stepperCloseGroup(void *this, s32 value);

/* the count of the main menu's items (mainMenuTaskInit counts its StepperGroup's children) */
extern s32 MAIN_MENU_ITEM_COUNT;

extern VtableEntry MAIN_MENU_ITEM_VTABLE[];

/* Spreads the menu's items over 180 units from 0x8F: the place of item index. */
s32 getMainMenuItemY(s32 index) {
    return index * 180 / (MAIN_MENU_ITEM_COUNT - 1) + 0x8F;
}

/* Draws the image at (x, y) as a SPRT with its texture page, semi-transparent when semiTrans is set. */
SprtColorWords *mainMenuDrawImage(u_long *ot, s32 x, s32 y, TitleImage *image, s32 semiTrans) {
    DR_TPAGE *tpage = allocDrTpage(image->tpage);
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut.word;
    sprt->wh = image->wh.word;
    if (semiTrans == 0) {
        SetSemiTrans(sprt, 0);
    } else {
        SetSemiTrans(sprt, 1);
    }
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    return sprt;
}

/* Draws a rail at (x, y), or its shadow (shadow set: half bright, subtractive, the CLUT 0x40 further), mirrored
 * unless notMirrored: the image's 256 rows as two quads, 0x9C rows over 0x9C pixels, then the other 0x64 over
 * another 0x9C. */
INCLUDE_ASM("title/nonmatchings/main_menu", mainMenuDrawRail);
void mainMenuDrawRail(u_long *ot, s32 x, s32 y, TitleImage *image, s32 notMirrored, s32 shadow);

/* Draws the image at (x, y) as a POLY_FT4, mirrored left to right when notMirrored is 0. */
void mainMenuDrawImageQuad(u_long *ot, s32 x, s32 y, TitleImage *image, s32 notMirrored) {
    s32 u = image->uvClut.part.u;
    s32 v = image->uvClut.part.v;
    s32 w = image->wh.part.w;
    s16 h = image->wh.part.h - 1;
    POLY_FT4 *poly = allocPolyFT4();

    if (notMirrored == 0) {
        poly->u0 = u + (w - 1);
        poly->v0 = v;
        poly->u1 = u;
        poly->v1 = v;
        poly->u2 = u + (w - 1);
        poly->v2 = v + h;
        poly->u3 = u;
        poly->v3 = v + h;
    } else {
        poly->u0 = u;
        poly->v0 = v;
        poly->u1 = u + w;
        poly->v1 = v;
        poly->u2 = u;
        poly->v2 = v + h;
        poly->u3 = u + w;
        poly->v3 = v + h;
    }
    poly->x0 = x;
    poly->y0 = y;
    poly->x1 = w + x;
    poly->y1 = y;
    poly->x2 = x;
    poly->y2 = h + y;
    poly->x3 = w + x;
    poly->y3 = h + y;
    poly->clut = image->uvClut.part.clut;
    SetSemiTrans(poly, 0);
    poly->tpage = image->tpage;
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    AddPrim(ot, poly);
}

/* Draws the image's shadow at (x, y): mainMenuDrawImageQuad's quad, half bright and semi-transparent, with
 * the CLUT 0x40 further and texture page 0x58. */
void mainMenuDrawImageShadow(u_long *ot, s32 x, s32 y, TitleImage *image, s32 notMirrored) {
    s32 u = image->uvClut.part.u;
    s32 v = image->uvClut.part.v;
    s32 w = image->wh.part.w;
    s16 h = image->wh.part.h - 1;
    POLY_FT4 *poly = allocPolyFT4();

    if (notMirrored == 0) {
        poly->u0 = u + (w - 1);
        poly->v0 = v;
        poly->u1 = u;
        poly->v1 = v;
        poly->u2 = u + (w - 1);
        poly->v2 = v + h;
        poly->u3 = u;
        poly->v3 = v + h;
    } else {
        poly->u0 = u;
        poly->v0 = v;
        poly->u1 = u + w;
        poly->v1 = v;
        poly->u2 = u;
        poly->v2 = v + h;
        poly->u3 = u + w;
        poly->v3 = v + h;
    }
    poly->x0 = x;
    poly->y0 = y;
    poly->x1 = w + x;
    poly->y1 = y;
    poly->x2 = x;
    poly->y2 = h + y;
    poly->x3 = w + x;
    poly->y3 = h + y;
    poly->clut = image->uvClut.part.clut + 0x40;
    SetSemiTrans(poly, 1);
    poly->tpage = 0x58;
    poly->r0 = 0x40;
    poly->g0 = 0x40;
    poly->b0 = 0x40;
    AddPrim(ot, poly);
}

/* Draws the banners at the menu's sides: two 32x480 strips (u 0x6C of the 4-bit page at x 768, clut (768, 253)) at
 * x 16 and 464, the left one scrolled down the page by scroll and the right one up. */
void mainMenuDrawBanners(u_long *ot, s32 scroll) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->rgbCode = (((SPRT *)sprt)->code << 24) | (PRIM_SEMI_TRANS << 24) | 0x808080;
    sprt->xy = 16;
    sprt->uvClut = (GetClut(768, 253) << 16) | ((scroll & 0xFF) << 8) | 0x6C;
    sprt->wh = (480 << 16) | 32;
    AddPrim(ot, sprt);
    sprt = (SprtColorWords *)allocSprt();
    sprt->rgbCode = (((SPRT *)sprt)->code << 24) | (PRIM_SEMI_TRANS << 24) | 0x808080;
    sprt->xy = 464;
    sprt->uvClut = (GetClut(768, 253) << 16) | ((-scroll & 0xFF) << 8) | 0x6C;
    sprt->wh = (480 << 16) | 32;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(getTPage(0, 0, 768, 0)));
}

/* The focus method (entry 0x18) of MAIN_MENU_ITEM_VTABLE: plays the cursor sound. */
void MainMenuItem_focus(void) {
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
}

/* The cancel method (entry 0x10) of MAIN_MENU_ITEM_VTABLE: refused (-1) when own is set, else plays the back sound
 * and accepts. */
s32 MainMenuItem_cancel(MainMenuItem *this, s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
    return 0;
}

/* Constructs a main menu item showing label, locked or not, below the screen. */
MainMenuItem *MainMenuItem_init(MainMenuItem *this, TitleImage *label, s32 locked) {
    stepperInit(this, 0);
    this->vtable = MAIN_MENU_ITEM_VTABLE;
    /* The match depends on storing locked first (its shorter life gives it s1). */
    this->locked = locked;
    this->label = label;
    this->flashPhase = 0;
    this->y = 0x2000;
    this->frame = 0;
    return this;
}

/* The confirm method (entry 0x8) of MAIN_MENU_ITEM_VTABLE: accepts silently when own is set; else refuses
 * (-1) a locked item with the refusal sound, and accepts any other with the confirm sound. */
s32 MainMenuItem_confirm(MainMenuItem *this, s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return 0;
    }
    if (this->locked == 1) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_REFUSE, 0);
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
    return 0;
}

/* The input method (entry 0x38) of MAIN_MENU_ITEM_VTABLE: advances the flash by 0x180 a frame and closes
 * the group with the item's index once it reaches 0x5A00. */
s32 MainMenuItem_input(MainMenuItem *this) {
    this->flashPhase += 0x180;
    if (this->flashPhase >= 0x5A00) {
        stepperCloseGroup(this, this->index);
    }
    return 1;
}

/* the item the main menu's cursor is on, set by the current item's draw (MainMenuTask_update reads it) */
extern s32 MAIN_MENU_CURSOR;


/* The cosine of angle (0x1000 a turn), 4.12. The match depends on this s16 inline, as in option.cpp: a table read
 * in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* The draw method of MAIN_MENU_ITEM_VTABLE: once the menu's frame passes 8 per index, eases the label an eighth
 * of the way to its place each frame; draws it at x 128 pulsing with the flash (from 0x40, or 0x80 while current,
 * which also sets MAIN_MENU_CURSOR), and the dark semi-transparent bar 2 right and 4 down; counts the frame. */
void MainMenuItem_draw(MainMenuItem *this, s32 ot, s32 isCurrent, s32 active) {
    u_long *front = FRAME_OT.firstDrawn + 5;
    s32 index = this->index;
    SPRT *sprt;
    s32 brightness;

    if (index * 8 < this->frame) {
        this->y += ((getMainMenuItemY(index) << 4) - this->y) >> 3;
    }
    sprt = (SPRT *)mainMenuDrawImage(front, 0x80, this->y >> 4, this->label, 0);
    brightness = getCosInline(this->flashPhase) >> 7;
    if (isCurrent) {
        brightness += 0x80;
    } else {
        brightness += 0x40;
    }
    setRGB0(sprt, brightness, brightness, brightness);
    if (isCurrent) {
        MAIN_MENU_CURSOR = index;
    }
    sprt = (SPRT *)mainMenuDrawImage(front, 0x82, (this->y >> 4) + 4, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_LABEL_BAR], 1);
    setRGB0(sprt, 0x40, 0x40, 0x40);
    this->frame++;
}

/* Whether a bonus game is unlocked (SaveData.bonusGamesUnlocked): else the two mini game items are locked "???"
 * ones. */
s32 hasBonusGame(void) {
    return (GAME_STATE.save.bonusGamesUnlocked & BONUS_GAME_ARENA_BITS) != 0;
}

/* StepperGroup::stepperGroupAddChild, as C sees it */
void *stepperGroupAddChild(void *this, void *child);

/* StepperGroup::stepperGroupCountChildren, as C sees it */
s32 stepperGroupCountChildren(void *this);

void mainMenuThreadEntry(s32 unused, void *object);

/* the buttons of the main menu's StepperGroup */
extern u8 MAIN_MENU_BUTTONS[];

/* the labels of the items; the two mini game items show the locked "???" (6) until a bonus game is unlocked */
#define MAIN_MENU_LABELS (&MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_LABELS])

/* the background picture */
extern TIM_IMAGE MAIN_MENU_BACKGROUND_TIM;

/* Constructs the main menu's task with the cursor on cursor: its thread, the items (the two mini games locked
 * while no bonus game is unlocked), the background, and the arrows and rails off the screen. */
MainMenuTask *mainMenuTaskInit(MainMenuTask *this, s32 cursor) {
    Display *display;
    u32 *file;

    taskInit(&this->task, 0x2000, "MainMenu");
    this->task.vtable = &MAIN_MENU_TASK_VTABLE;
    threadInit(&this->thread, mainMenuThreadEntry, 0x1000, this, 0);
    stepperGroupInit(&this->items, 1, MAIN_MENU_BUTTONS);
    spriteInit(&this->background);
    fadeControlFadeOut(&FADE_CONTROL, 0);
    display = CURRENT_DISPLAY;
    displaySetSize(display, 512, 480);
    displayClear(display);
    displaySetVsyncMode(display, 0);
    displayDisable(display, 0);
    displayWaitFrame(display);
    soundControlLoadSound(&SOUND_CONTROL, 0x8001, 1);
    file = loadFile("/title/mainmenu.tim");
    uploadTims(file);
    mainHeapFree(file);
    /* The match depends on writing every new item out: an inline helper for them moves the label loads. */
    stepperGroupAddChild(&this->items, MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[0], 0));
    stepperGroupAddChild(&this->items, MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[1], 0));
    stepperGroupAddChild(&this->items, MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[2], 0));
    if (hasBonusGame()) {
        stepperGroupAddChild(&this->items,
                             MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[3], 0));
        stepperGroupAddChild(&this->items,
                             MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[4], 0));
    } else {
        stepperGroupAddChild(&this->items,
                             MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[6], 1));
        stepperGroupAddChild(&this->items,
                             MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[6], 1));
    }
    stepperGroupAddChild(&this->items, MainMenuItem_init(operatorNew(sizeof(MainMenuItem)), &MAIN_MENU_LABELS[5], 0));
    this->items.wraps = 1;
    spriteSetTim(&this->background, &MAIN_MENU_BACKGROUND_TIM, 0);
    this->frame = 0;
    this->leftX = -0x400;
    this->rightX = 0x2400;
    MAIN_MENU_ITEM_COUNT = stepperGroupCountChildren(&this->items);
    MAIN_MENU_CURSOR = cursor;
    soundControlPlaySound(&SOUND_CONTROL, 0x8001, 0);
    return this;
}

/* The destructor of MainMenuTask (task vtable MAIN_MENU_TASK_VTABLE): fades the screen in, then destroys the
 * background, the items, the thread and the task. */
void MainMenuTask_destroy(MainMenuTask *this, s32 flags) {
    this->task.vtable = &MAIN_MENU_TASK_VTABLE;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->background, DESTROY_BASES);
    stepperGroupDestroy(&this->items, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

/* StepperGroup::stepperGroupInputIfOpen, as C sees it */
s32 stepperGroupInputIfOpen(void *this, s32 arg, s32 held, u32 buttons);

/* StepperGroup::stepperGroupGetCloseValue, as C sees it */
s32 stepperGroupGetCloseValue(void *this);

/* The task's thread: waits 4 frames, fades the screen in (10 frames), runs the items until one closes the menu
 * (input from both pads), leaves the choice in SYSTEM_CONTEXT.sceneResult, fades out (30 frames), kills the task
 * and stops the songs, then hands back -1 every frame. */
void MainMenuTask_run(MainMenuTask *this) {
    s32 i;
    s32 closed;

    for (i = 4; i != 0; i--) {
        threadSwitchBack(&this->thread, -1);
    }
    fadeControlFadeIn(&FADE_CONTROL, 10);
    while (!fadeControlIsDone(&FADE_CONTROL)) {
        threadSwitchBack(&this->thread, 0);
    }
    do {
        PadManager *pads = PAD_MANAGER_INSTANCE;
        u32 held = padManagerGetHeld(pads, -1);
        u32 repeated = padManagerGetRepeated(pads, -1);

        closed = stepperGroupInputIfOpen(&this->items, 0, held, repeated) == 0;
        threadSwitchBack(&this->thread, 0);
    } while (!closed);
    SYSTEM_CONTEXT.sceneResult = stepperGroupGetCloseValue(&this->items);
    fadeControlFadeOut(&FADE_CONTROL, 30);
    while (!fadeControlIsDone(&FADE_CONTROL)) {
        threadSwitchBack(&this->thread, 0);
    }
    taskKill(&this->task);
    stopSongs();
    for (;;) {
        threadSwitchBack(&this->thread, -1);
    }
}

/* The entry of the thread of mainMenuTaskInit's task. */
void mainMenuThreadEntry(s32 unused, void *object) {
    MainMenuTask_run(object);
}

/* StepperGroup::stepperGroupDrawIfOpen, as C sees it */
void stepperGroupDrawIfOpen(void *this, s32 arg);

/* The update of MainMenuTask: runs the thread, then draws the background; once the fade is done, the items,
 * the arrows at the cursor's item and the rails (sliding in after 0x10 frames, an eighth of the way a frame),
 * their shadows, the banners, the header and the footer; counts the frame. */
void MainMenuTask_update(MainMenuTask *this, s32 arg) {
    u_long *ot;
    SPRT *sprt;

    if (threadSwitchTo(&this->thread, 0) < 0) {
        return;
    }
    /* The match depends on ot holding the background's slot first: it reads FRAME_OT before PRIM_BUFFER_FREE. */
    ot = FRAME_OT.firstDrawn + 1;
    PRIM_BUFFER_FREE = spriteDraw(&this->background, PRIM_BUFFER_FREE, ot);
    if (!fadeControlIsDone(&FADE_CONTROL)) {
        return;
    }
    stepperGroupDrawIfOpen(&this->items, 0);
    ot = FRAME_OT.firstDrawn + 5;
    if (this->frame >= 0x10) {
        this->leftX += (0x4C0 - this->leftX) >> 3;
        this->rightX += (0x1960 - this->rightX) >> 3;
    }
    mainMenuDrawImageQuad(ot, (this->leftX >> 4) - 13, getMainMenuItemY(MAIN_MENU_CURSOR) - 10,
                          &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_ARROW], 0);
    mainMenuDrawImageQuad(ot, (this->rightX >> 4) - 8, getMainMenuItemY(MAIN_MENU_CURSOR) - 10,
                          &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_ARROW], 1);
    mainMenuDrawRail(ot, this->leftX >> 4, 0x5C, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_RAIL], 0, 0);
    mainMenuDrawRail(ot, this->rightX >> 4, 0x5C, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_RAIL], 1, 0);
    mainMenuDrawImageShadow(ot, (this->leftX >> 4) - 11, getMainMenuItemY(MAIN_MENU_CURSOR) - 6,
                            &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_ARROW], 0);
    mainMenuDrawImageShadow(ot, (this->rightX >> 4) - 7, getMainMenuItemY(MAIN_MENU_CURSOR) - 6,
                            &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_ARROW], 1);
    mainMenuDrawRail(ot, (this->leftX >> 4) + 2, 0x60, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_RAIL], 0, 1);
    mainMenuDrawRail(ot, (this->rightX >> 4) + 2, 0x60, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_RAIL], 1, 1);
    mainMenuDrawBanners(ot, this->frame);
    sprt = (SPRT *)mainMenuDrawImage(ot, 0xA4, 0x21, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_HEADER], 1);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)mainMenuDrawImage(ot, 0xB9, 0x1AE, &MAIN_MENU_IMAGES[MAIN_MENU_IMAGE_FOOTER], 1);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    this->frame++;
}

/* MainMenuItem's destructor: Stepper's, inlined (it takes the stepper out of its group's list). */
void MainMenuItem_destroy(MainMenuItem *this, s32 flags) {
    this->vtable = STEPPER_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_BACKGROUND_CLUT_RECT);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_BACKGROUND_RECT);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_BACKGROUND_TIM);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_ITEM_VTABLE);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_TASK_VTABLE);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_BUTTONS);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_IMAGES);
