/* The ranking (title/ranking.tim). */

#include "common.h"
#include <libetc.h>
#include <stdio.h>
#include "engine/cd/file_load.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

extern Unk80073390Image D_80066024[];

/* Draws image at (x, y): a SPRT, then the DR_TPAGE of its texture page, added to ot. Returns the SPRT. */
SprtColorWords *func_80073390(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* Draws two columns of six 16 by 64 strips (texture (0..15, 0x48..0x88)) that scroll with frame: down at x 0x12E,
 * up at x 2. */
/* The match depends on frame & 0x3F written in each loop (GCSE shares the two) and on y declared per loop
 * (a function-wide y keeps the second loop's counter from being reversed). */
void func_8007342C(u_long *ot, s32 frame) {
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
        setUV4(poly, 0, 0x48, 0x10, 0x48, 0, 0x88, 0x10, 0x88);
        poly->tpage = 0x17;
        poly->clut = 0x7F5C;
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
        setUV4(poly, 0, 0x48, 0x10, 0x48, 0, 0x88, 0x10, 0x88);
        poly->tpage = 0x17;
        poly->clut = 0x7F5C;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
}

/* The ranking's task (virtual table D_80065FE8), which func_80073818 runs: 0x10C counts its frames (it fades
 * out at 0x1CC). */
typedef struct Unk80073818 {
    /* 0x000 */ Task task;
    /* 0x020 */ s32 unk20;
    /* 0x024 */ Unk800764BC records;
    /* 0x038 */ Unk80074220 entry;
    /* 0x108 */ u8 unk108[0x10C - 0x108];
    /* 0x10C */ s32 frame;
} Unk80073818;

/* The button that confirms: circle in Japan, cross in the US. */
#if VERSION_JP
#define PAD_CONFIRM PADRright
#else
#define PAD_CONFIRM PADRdown
#endif

extern Unk8006713CVtable D_80065FE8;

/* name_entry.c's, as C sees them */
void func_800744BC(Unk80074220 *entry);
void func_8007460C(Unk80074220 *entry);
void func_800747F8(Unk80074220 *entry, void *arg);

void func_80073A04(Unk80073818 *this);
void func_80073B34(Unk80073818 *this);
s32 func_80073918(Unk80073818 *this);
void func_80073F94(Unk80073818 *this);

INCLUDE_ASM("title/nonmatchings/ranking", rankingTaskInit);

/* The destructor of the ranking's task: clears the screen to black, unloads the sounds, then destroys the
 * name entry, 0x24 and the task. */
void func_80073784(Unk80073818 *this, s32 flags) {
    this->task.vtable = &D_80065FE8.task;
    displaySetClearColor(CURRENT_DISPLAY, 0, 0, 0);
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    func_80074220(&this->entry, DESTROY_BASES);
    func_80076040(&this->records, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

/* The ranking's frame: runs the name entry while it is open; else draws the ranking, fades out at frame 0x1CC
 * (or on Start or the confirm button) and kills the task once the fade is done. */
void func_80073818(Unk80073818 *this) {
    Unk80074220 *entry = &this->entry;

    if (this->entry.active != 0) {
        func_800744BC(entry);
        func_8007460C(entry);
        return;
    }
    if (this->frame == 0) {
        if (this->entry.finished != 0) {
            func_800747F8(entry, &this->records);
        }
        func_80073A04(this);
    }
    func_80073B34(this);
    func_80073F94(this);
    if (++this->frame == 0x1CC) {
        fadeControlFadeOut(&FADE_CONTROL, 0x14);
    }
    if (this->frame < 0x1E1 && func_80073918(this) == 0) {
        return;
    }
    if (fadeControlIsDone(&FADE_CONTROL)) {
        fadeControlFadeIn(&FADE_CONTROL, 0x14);
        taskKill(&this->task);
    }
}

/* Whether Start or the confirm button was pressed on either pad. */
s32 func_80073918(Unk80073818 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    return (padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & (PADstart | PAD_CONFIRM);
}

/* Loads "title/ranking.tim", uploads its TIMs to VRAM and frees it. */
void func_80073960(void) {
    u32 *file = loadFile("title/ranking.tim");

    uploadTims(file);
    mainHeapFree(file);
}

/* Writes value with digits digits, zero padded, as two-byte (full-width) digits into text.
 * The match depends on reading each digit in the loop's test and storing both bytes before text += 2. */
void func_8007399C(u8 *text, s32 value, s32 digits) {
    u8 buffer[0x20];
    u8 *digit = buffer;
    u8 c;

    sprintf(buffer, "%0*d", digits, value);
    while ((c = *digit++) != 0) {
        text[0] = 0x82;
        text[1] = c + 0x1F;
        text += 2;
    }
}

INCLUDE_ASM("title/nonmatchings/ranking", func_80073A04);

INCLUDE_ASM("title/nonmatchings/ranking", func_80073B34);

INCLUDE_ASM("title/nonmatchings/ranking", func_80073D40);

INCLUDE_ASM("title/nonmatchings/ranking", func_80073E34);

void func_80073F8C(void) {
}

/* Draws func_80073818's screen: func_8007342C's part, two of D_80066024's sprites, then the 0x140 x 0xF0
 * picture at (0, 0) of texture pages 7 and 8. */
void func_80073F94(Unk80073818 *this) {
    u_long *ot = FRAME_OT.firstDrawn + 5;
    SPRT *sprt;
    DR_TPAGE *tpage;

    func_8007342C(ot, this->frame);
    sprt = (SPRT *)func_80073390(ot, 0x2F, 0xD, &D_80066024[0]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)func_80073390(ot, 0x5E, 0xD0, &D_80066024[2]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    tpage = allocDrTpage(7);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0, 0);
    setWH(sprt, 0x100, 0xF0);
    sprt->clut = getClut(0x1C0, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    tpage = allocDrTpage(8);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0x100, 0);
    setWH(sprt, 0x40, 0xF0);
    sprt->clut = getClut(0x1C0, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_800740FC(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/ranking", D_80065FE8);
INCLUDE_RODATA("title/nonmatchings/ranking", D_80066024);
