/* The credits (/credit.txt, /credit.k15, /title/credit.tim). */

#include "common.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "title/title_classes.h"
#include "psyq.h"
#include "vtable.h"

/* The credits task (virtual table D_800664CC), as creditsTaskInit builds it. */
typedef struct Unk800767AC {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 unk20; /* the argument of the last update */
    /* 0x24 */ u8 *text;   /* /credit.txt, its lines cut apart by func_80076764 */
    /* 0x28 */ u8 **lines; /* the start of each line of text */
    /* 0x2C */ Font *font;
    /* 0x30 */ u16 *fontData; /* /credit.k15 */
} Unk800767AC;

extern Unk8006713CVtable D_800664CC;

INCLUDE_ASM("title/nonmatchings/credits", creditsTaskInit);

/* Sets the STP bit of every CLUT color of a run of TIMs (/title/credit.tim), before they go to VRAM. */
void func_800766A4(u32 *tims) {
    TIM_IMAGE image;

    func_800585B0(tims);
    while (ReadTIM(&image) != NULL) {
        u16 *color = (u16 *)image.caddr;

        if (color != NULL) {
            s32 count = image.crect->w * image.crect->h;
            s32 i;

            for (i = 0; i < count; i++) {
                color[i] |= 0x8000;
            }
        }
    }
}

/* The number of line breaks in text: creditsTaskInit's count of the credits' lines. */
s32 func_80076728(const u8 *text) {
    s32 lines = 0;
    u8 c;

    while ((c = *text++) != '\0') {
        if (c == '\n') {
            lines++;
        }
    }
    return lines;
}

/* Cuts text at its line breaks into lines, the start of each line that ends in one. */
void func_80076764(u8 **lines, u8 *text) {
    u8 *line = text;
    u8 c;

    while ((c = *text++) != '\0') {
        if (c == '\n') {
            *lines++ = line;
            line = text;
            text[-1] = '\0';
        }
    }
}

/* The destructor of the credits task: stops the music, frees the text, its lines, the font and its data, fades
 * the screen in and destroys the task. */
void func_800767AC(Unk800767AC *this, s32 flags) {
    this->task.vtable = &D_800664CC.task;
    stopMusic();
    if (this->text != NULL) {
        operatorVecDelete(this->text);
    }
    if (this->lines != NULL) {
        operatorVecDelete(this->lines);
    }
    if (this->font != NULL) {
        fontDestroy(this->font, DESTROY_DELETE);
    }
    mainHeapFree(this->fontData);
    fadeControlFadeIn(&FADE_CONTROL, 0);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("title/nonmatchings/credits", func_80076858);

#if VERSION_JP
INCLUDE_ASM("title/nonmatchings/credits", func_80076B0C);
#endif

/* Draws the credits picture (256 x 120 from texture page 13) at x - 128, y, as bright as brightness; nothing at
 * brightness 0. */
void func_80076C14(u_long *ot, s32 x, s32 y, s32 brightness) {
    if (brightness != 0) {
        DR_TPAGE *tpage = allocDrTpage(13);
        SPRT *sprt = allocSprt();

        sprt->x0 = x - 128;
        sprt->r0 = brightness;
        sprt->g0 = brightness;
        sprt->b0 = brightness;
        sprt->y0 = y;
        sprt->u0 = 0;
        sprt->v0 = 0;
        sprt->w = 256;
        sprt->h = 120;
        sprt->clut = 0x3DA0;
        AddPrim(ot, sprt);
        AddPrim(ot, tpage);
    }
}

INCLUDE_ASM("title/nonmatchings/credits", func_80076CC4);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_80076DF0(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}
