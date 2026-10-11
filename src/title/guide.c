/* The guide (the task "Guide Logo", title/guide.tim): the minigames' explanations. */

#include "common.h"
#include <libetc.h>
#include "engine/gfx/camera.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

/* The guide logo task, which guideLogoTaskInit sets up. */
typedef struct GuideLogoTask {
    /* 0x000 */ Task task;
    /* 0x020 */ s32 unk20;
    /* 0x024 */ EntityGroup entities;
    /* 0x034 */ struct Unk80076E24 *model; /* a character model entity (title_0a.c) */
    /* 0x038 */ u32 sound; /* a sound's handle (stopSound) */
    /* 0x03C */ Camera camera;
    /* 0x078 */ u8 unk78[0xA6C - 0x78]; /* 0xA8: a Mersenne twister */
    /* 0xA6C */ u16 unkA6C; /* a frame count (func_800721C0) */
    /* 0xA6E */ s16 leaving; /* -1 once the guide fades out to leave */
    /* 0xA70 */ s16 leaveFrame; /* frames since it started leaving */
    /* 0xA72 */ s16 unkA72;
    /* 0xA74 */ u16 unkA74; /* guideLogoTaskInit's argument */
    /* 0xA76 */ s16 unkA76;
    /* 0xA78 */ s16 unkA78;
    /* 0xA7A */ s16 unkA7A;
    /* 0xA7C */ s16 unkA7C;
    /* 0xA7E */ u8 unkA7E[0xA80 - 0xA7E];
    /* 0xA80 */ s32 unkA80;
    /* 0xA84 */ s32 unkA84;
    /* 0xA88 */ s16 unkA88; /* func_80073210 draws its blinking mark while it is set */
    /* 0xA8A */ s16 unkA8A;
} GuideLogoTask;

/* The buttons that confirm and that leave the guide: circle and cross in Japan, cross and triangle in the
 * US. */
#if VERSION_JP
#define PAD_CONFIRM PADRright
#define PAD_CANCEL PADRdown
#else
#define PAD_CONFIRM PADRdown
#define PAD_CANCEL PADRup
#endif

extern Unk8006713CVtable D_80065EC4;
extern s32 D_8007B244[]; /* by 0xA74 */

/* title_0a.c's, as C sees them */
void func_800773C8(struct Unk80076E24 *model, s32 value);
s32 func_800773F0(struct Unk80076E24 *model);

void func_80072CF0(GuideLogoTask *this);
void func_80073208(void);
void func_800726EC(GuideLogoTask *this);
void func_800728D8(GuideLogoTask *this);
void func_80072D2C(GuideLogoTask *this);
void func_80073210(GuideLogoTask *this);

/* The same as func_80073390. */
SprtColorWords *func_80071CB8(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* func_8007342C with texture (0..15, 0x40..0x80), page 0x16 and CLUT 0x7F18: two columns of six scrolling
 * strips. The match depends on the same forms as func_8007342C's. */
void func_80071D54(u_long *ot, s32 frame) {
    s32 i;

    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 + (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 0x12E;
        poly->y0 = y - 64;
        poly->x1 = 0x13E;
        poly->y1 = y - 64;
        poly->x2 = 0x12E;
        poly->y2 = y;
        poly->x3 = 0x13E;
        poly->y3 = y;
        setUV4(poly, 0, 0x40, 0x10, 0x40, 0, 0x80, 0x10, 0x80);
        poly->tpage = 0x16;
        poly->clut = 0x7F18;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 - (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 2;
        poly->y0 = y;
        poly->x1 = 0x12;
        poly->y1 = y;
        poly->x2 = 2;
        poly->y2 = y + 64;
        poly->x3 = 0x12;
        poly->y3 = y + 64;
        setUV4(poly, 0, 0x40, 0x10, 0x40, 0, 0x80, 0x10, 0x80);
        poly->tpage = 0x16;
        poly->clut = 0x7F18;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
}

INCLUDE_ASM("title/nonmatchings/guide", guideLogoTaskInit);

/* The destructor of the guide logo task: stops its sound, unloads the sounds, deletes the model, fades the
 * screen in, then destroys the camera, the entities and the task. */
void func_80072100(GuideLogoTask *this, s32 flags) {
    this->task.vtable = &D_80065EC4.task;
    stopSound(this->sound);
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    entityDelete((Entity *)this->model);
    fadeControlFadeIn(&FADE_CONTROL, 0);
    cameraDestroy(&this->camera, DESTROY_BASES);
    entityGroupDestroy(&this->entities, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("title/nonmatchings/guide", func_800721C0);

/* Reads both pads: the cancel button starts leaving (fade out, model clip 1); the confirm button sets 0xA76
 * or plays model clip 9, and leaves too once 0xA8A is set. */
void func_800722CC(GuideLogoTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 port;

    for (port = 0; port < 2; port++) {
        u32 pressed = padManagerGetPressed(pads, port);

        if ((pressed & PAD_CANCEL) && this->leaving == 0) {
            this->leaving = -1;
            fadeControlFadeOut(&FADE_CONTROL, 0x1E);
            func_800773C8(this->model, 1);
        }
        if (pressed & PAD_CONFIRM) {
            if (this->unkA88 == 0) {
                this->unkA76 = 1;
            } else if (func_800773F0(this->model) == 0) {
                func_800773C8(this->model, 9);
            }
            if (this->unkA8A != 0 && this->leaving == 0) {
                this->leaving = -1;
                fadeControlFadeOut(&FADE_CONTROL, 0x1E);
                func_800773C8(this->model, 1);
            }
        }
    }
}

INCLUDE_ASM("title/nonmatchings/guide", func_800723FC);

/* Calls the empty func_80073208, then func_80072CF0, which resets fields 0xA76-0xA8A. */
void func_800724D8(void *this) {
    func_80073208();
    func_80072CF0(this);
}

/* A frame of the guide: draws the blinking mark, counts 0xA72 up to 0x21 and then runs func_80072D2C, then
 * runs func_800726EC and func_800728D8. */
void func_80072504(GuideLogoTask *this) {
    func_80073210(this);
    if (this->unkA72 > 0x20) {
        func_80072D2C(this);
    } else {
        this->unkA72++;
    }
    func_800726EC(this);
    func_800728D8(this);
}

void func_80072560(void) {
}

extern Unk80073390Image D_80065EF4[];

/* Draws the guide logo screen: func_80071D54's strips at the frame count, picture 4 + 0xA74 and picture 14
 * of D_80065EF4, then the 0x140 x 0xF0 picture at (0, 0) of texture pages 6 and 7 (func_80073F94's shape). */
void func_80072568(GuideLogoTask *this) {
    u_long *ot = this->camera.ot.head;
    Unk80073390Image *pictures;
    SPRT *sprt;
    DR_TPAGE *tpage;

    func_80071D54(ot, this->unkA6C);
    /* The match depends on the table in a local, set after the first call (it then keeps s1). */
    pictures = D_80065EF4;
    sprt = (SPRT *)func_80071CB8(ot, 0x2F, 0xD, &pictures[(u16)(this->unkA74 + 4)]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)func_80071CB8(ot, 0x46, 0xD1, &pictures[14]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    tpage = allocDrTpage(6);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0, 0);
    setWH(sprt, 0x100, 0xF0);
    sprt->clut = getClut(0x180, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    tpage = allocDrTpage(7);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0x100, 0);
    setWH(sprt, 0x40, 0xF0);
    sprt->clut = getClut(0x180, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

void func_800726E4(void) {
}

INCLUDE_ASM("title/nonmatchings/guide", func_800726EC);

void func_800728D0(void) {
}

INCLUDE_ASM("title/nonmatchings/guide", func_800728D8);

/* Resets 0xA76-0xA8A, 0xA80 and 0xA84 from D_8007B244 by 0xA74. */
void func_80072CF0(GuideLogoTask *this) {
    s32 value;

    this->unkA76 = 0;
    this->unkA78 = 0;
    this->unkA7A = 0;
    this->unkA7C = 0;
    value = D_8007B244[this->unkA74];
    this->unkA88 = 0;
    this->unkA8A = 0;
    this->unkA80 = value;
    this->unkA84 = value;
}

INCLUDE_ASM("title/nonmatchings/guide", func_80072D2C);

void func_80073200(void) {
}

void func_80073208(void) {
}

/* While 0xA88 is set, draws a 16 by 16 mark at (0x110, 0xB4) of texture page 0x17 that blinks every 8 frames
 * (u 0x40 and 0x50). The match depends on the blink phase kept in a local: the test is then a computed
 * truth value (srl, xori, andi), not a branch on the bit. */
void func_80073210(GuideLogoTask *this) {
    u_long *ot = this->camera.ot.slots;
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 firstFrame;

    if (this->unkA88 == 0) {
        return;
    }
    tpage = allocDrTpage(0x17);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    firstFrame = (this->unkA6C & 8) == 0;
    if (firstFrame) {
        setUV0(sprt, 0x40, 0);
    } else {
        setUV0(sprt, 0x50, 0);
    }
    setXY0(sprt, 0x110, 0xB4);
    setWH(sprt, 0x10, 0x10);
    sprt->clut = getClut(0x1C0, 0x1FE);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

/* Clears leaving, leaveFrame and 0xA72. */
void func_800732D8(GuideLogoTask *this) {
    this->leaving = 0;
    this->leaveFrame = 0;
    this->unkA72 = 0;
}

/* While leaving: stops the sound at frame 0x20 and kills the task after frame 0x80. */
void func_800732E8(GuideLogoTask *this) {
    if (this->leaving != 0) {
        if (this->leaveFrame == 0x20) {
            stopSound(this->sound);
        }
        if (this->leaveFrame >= 0x80) {
            taskKill(&this->task);
        } else {
            this->leaveFrame++;
        }
    }
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8007335C(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}
