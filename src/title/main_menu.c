/* The main menu (the task "MainMenu", /title/mainmenu.tim). */

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/gfx/sprite.h"
#include "title/title_classes.h"

/* The main menu's task (mainMenuTaskInit, virtual table MAIN_MENU_TASK_VTABLE): its thread runs the
 * StepperGroup of the items until one closes it; the update draws the background, the items, the arrows,
 * the rails and the banners. */
typedef struct MainMenuTask {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ u8 items[0x44]; /* a StepperGroup */
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

INCLUDE_ASM("title/nonmatchings/main_menu", func_80069A58);

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

/* the bar drawn semi-transparent and dark behind every label */
extern TitleImage MAIN_MENU_DECOR_IMAGES[];

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
    sprt = (SPRT *)mainMenuDrawImage(front, 0x82, (this->y >> 4) + 4, MAIN_MENU_DECOR_IMAGES, 1);
    setRGB0(sprt, 0x40, 0x40, 0x40);
    this->frame++;
}

/* Whether a bonus game is unlocked (SaveData.bonusGamesUnlocked): else the two mini game items are locked "???"
 * ones. */
s32 hasBonusGame(void) {
    return (GAME_STATE.save.bonusGamesUnlocked & BONUS_GAME_ARENA_BITS) != 0;
}

INCLUDE_ASM("title/nonmatchings/main_menu", mainMenuTaskInit);

/* The destructor of MainMenuTask (task vtable MAIN_MENU_TASK_VTABLE): fades the screen in, then destroys the
 * background, the items, the thread and the task. */
void MainMenuTask_destroy(MainMenuTask *this, s32 flags) {
    this->task.vtable = &MAIN_MENU_TASK_VTABLE;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->background, DESTROY_BASES);
    stepperGroupDestroy(this->items, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("title/nonmatchings/main_menu", MainMenuTask_run);

INCLUDE_ASM("title/nonmatchings/main_menu", MainMenuTask_update);

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
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_SIDE_IMAGES);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_LABELS);
INCLUDE_RODATA("title/nonmatchings/main_menu", D_800647BC);
INCLUDE_RODATA("title/nonmatchings/main_menu", MAIN_MENU_DECOR_IMAGES);
