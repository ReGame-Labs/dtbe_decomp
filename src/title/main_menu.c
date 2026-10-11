/* The main menu (the task "MainMenu", /title/mainmenu.tim). */

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/gfx/sprite.h"
#include "title/title_classes.h"

/* A task with a thread, a StepperGroup and a sprite, named after its destructor. */
typedef struct Unk8006A62C {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ u8 steppers[0x44]; /* a StepperGroup */
    /* 0x70 */ Sprite sprite;
} Unk8006A62C;

extern TaskVtable D_8006471C;

/* Stepper's virtual table, which C reaches by its g++ name */
extern VtableEntry STEPPER_VTABLE[] __asm__("_vt.7Stepper");

/* The title's Stepper class of virtual table D_800646BC (func_8006A11C), as C
 * sees it: a Stepper's list node, and its virtual table pointer at 0x1C. */
typedef struct Unk8006A11C {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *group;
    /* 0x0C */ s32 index; /* among the children of its group */
    /* 0x10 */ u8 unk10[0x1C - 0x10];
    /* 0x1C */ VtableEntry *vtable;
    /* 0x20 */ s32 unk20; /* func_8006A11C's first argument */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* 0x2000 from func_8006A11C */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* func_8006A11C's second argument */
} Unk8006A11C;

/* Stepper's constructor, as C sees it */
void *stepperInit(void *this, s32 value);

/* Stepper::stepperCloseGroup, as C sees it */
void stepperCloseGroup(void *this, s32 value);

/* the count of the main menu's items (mainMenuTaskInit counts its StepperGroup's children) */
extern s32 D_8007B33C;

extern VtableEntry D_800646BC[];

/* Spreads the menu's items over 180 units from 0x8F: the place of item index. */
s32 func_80069944(s32 index) {
    return index * 180 / (D_8007B33C - 1) + 0x8F;
}

/* Draws the image at (x, y) as a SPRT with its texture page, semi-transparent when semiTrans is set. */
SprtColorWords *func_80069980(u_long *ot, s32 x, s32 y, Unk80073390Image *image, s32 semiTrans) {
    DR_TPAGE *tpage = allocDrTpage(image->tpage);
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
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

/* An image in VRAM (D_80064768): its texture page, its corner in the page, its CLUT and its size. */
typedef struct Unk80069CFC {
    /* 0x0 */ u16 tpage;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u8 u;
    /* 0x5 */ u8 v;
    /* 0x6 */ u16 clut;
    /* 0x8 */ s16 w;
    /* 0xA */ s16 h;
} Unk80069CFC;

/* Draws the image at (x, y) as a POLY_FT4, mirrored left to right when notMirrored is 0. */
void func_80069CFC(u_long *ot, s32 x, s32 y, Unk80069CFC *image, s32 notMirrored) {
    s32 u = image->u;
    s32 v = image->v;
    s32 w = image->w;
    s16 h = image->h - 1;
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
    poly->clut = image->clut;
    SetSemiTrans(poly, 0);
    poly->tpage = image->tpage;
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    AddPrim(ot, poly);
}

/* Draws the image's shadow at (x, y): func_80069CFC's quad, half bright and semi-transparent, with the CLUT
 * 0x40 further and texture page 0x58. */
void func_80069E4C(u_long *ot, s32 x, s32 y, Unk80069CFC *image, s32 notMirrored) {
    s32 u = image->u;
    s32 v = image->v;
    s32 w = image->w;
    s16 h = image->h - 1;
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
    poly->clut = image->clut + 0x40;
    SetSemiTrans(poly, 1);
    poly->tpage = 0x58;
    poly->r0 = 0x40;
    poly->g0 = 0x40;
    poly->b0 = 0x40;
    AddPrim(ot, poly);
}

INCLUDE_ASM("title/nonmatchings/main_menu", func_80069FA0);

/* Entry 0x1C of D_800646BC: plays sound 0xF. */
void func_8006A0B4(void) {
    soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
}

/* The cancel method (entry 0x10) of D_800646BC: refused (-1) when own is set, else plays sound 0x11
 * and accepts. */
s32 func_8006A0E0(Unk8006A11C *this, s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x11, 0);
    return 0;
}

/* Constructs the title's Stepper of virtual table D_800646BC with its two values (0x20 and 0x30). */
Unk8006A11C *func_8006A11C(Unk8006A11C *this, s32 arg1, s32 arg2) {
    stepperInit(this, 0);
    this->vtable = D_800646BC;
    /* The match depends on storing arg2 first (its shorter life gives it s1). */
    this->unk30 = arg2;
    this->unk20 = arg1;
    this->unk24 = 0;
    this->unk28 = 0x2000;
    this->unk2C = 0;
    return this;
}

/* The confirm method (entry 0x8) of D_800646BC: accepts silently when own is set; else refuses (-1) with
 * sound 0x12 the item whose 0x30 is 1, and accepts any other with sound 0x10. */
s32 func_8006A184(Unk8006A11C *this, s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return 0;
    }
    if (this->unk30 == 1) {
        soundControlPlaySound(&SOUND_CONTROL, 0x12, 0);
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    return 0;
}

/* The input method (entry 0x38) of D_800646BC: advances 0x24 by 0x180 a frame and closes the group with its
 * index once it reaches 0x5A00. */
s32 func_8006A1E4(Unk8006A11C *this) {
    this->unk24 += 0x180;
    if (this->unk24 >= 0x5A00) {
        stepperCloseGroup(this, this->index);
    }
    return 1;
}

INCLUDE_ASM("title/nonmatchings/main_menu", func_8006A224);

/* Whether any of the save data's bits 0x380 of unk18 is set. */
s32 func_8006A348(void) {
    return (GAME_STATE.save.unk18 & 0x380) != 0;
}

INCLUDE_ASM("title/nonmatchings/main_menu", mainMenuTaskInit);

/* The destructor of Unk8006A62C (task vtable D_8006471C): fades the screen in, then destroys the sprite,
 * the steppers, the thread and the task. */
void func_8006A62C(Unk8006A62C *this, s32 flags) {
    this->task.vtable = &D_8006471C;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->sprite, DESTROY_BASES);
    stepperGroupDestroy(this->steppers, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("title/nonmatchings/main_menu", func_8006A6A4);

INCLUDE_ASM("title/nonmatchings/main_menu", func_8006A7EC);

/* Unk8006A11C's destructor: Stepper's, inlined (it takes the stepper out of its group's list). */
void func_8006AAA0(Unk8006A11C *this, s32 flags) {
    this->vtable = STEPPER_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}
