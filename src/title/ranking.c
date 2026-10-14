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

extern TitleImage RANKING_IMAGES[];

/* Draws image at (x, y): a SPRT, then the DR_TPAGE of its texture page, added to ot. Returns the SPRT. */
SprtColorWords *drawImage(u_long *ot, s32 x, s32 y, TitleImage *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut.word;
    sprt->wh = image->wh.word;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* Draws two columns of six 16 by 64 strips (texture (0..15, 0x48..0x88)) that scroll with frame: down at x 0x12E,
 * up at x 2. */
/* The match depends on frame & 0x3F written in each loop (GCSE shares the two) and on y declared per loop
 * (a function-wide y keeps the second loop's counter from being reversed). */
void drawSideStrips(u_long *ot, s32 frame) {
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

/* One line of the ranking: a record's rank, name and score as RankingTask_initLines prints them. */
typedef struct RankingLine {
    /* 0x00 */ s16 x;         /* slides in to 16 (even lines) or 32 (odd ones) */
    /* 0x02 */ s16 character; /* the record's, whose face RankingTask_drawFace draws */
    /* 0x04 */ u8 text[0x50]; /* STR_RANKING_LINE_FORMAT, its digits full-width */
} RankingLine; /* size 0x54 */

/* The ranking's task (RANKING_TASK_VTABLE, 0x474 bytes as the sequencer allocates it), which
 * RankingTask_tick runs: it shows the records' lines one by one and fades out at frame 0x1CC. */
typedef struct RankingTask {
    /* 0x000 */ Task task;
    /* 0x020 */ s32 updateArg;
    /* 0x024 */ RankingTable records;
    /* 0x038 */ NameEntry entry;
    /* 0x108 */ s32 enterName;  /* rankingTaskInit's last argument (not in the demo): opens the name entry */
    /* 0x10C */ s32 frame;
    /* 0x110 */ s32 shownLines; /* the lines drawn: one more every 16 frames, up to 7 */
    /* 0x114 */ RankingLine lines[8];
    /* 0x3B4 */ u8 unk3B4[0x474 - 0x3B4];
} RankingTask; /* size 0x474 */


extern TickTaskVtable RANKING_TASK_VTABLE;

/* name_entry.c's, as C sees them */
void NameEntry_update(NameEntry *entry);
void NameEntry_draw(NameEntry *entry);
void NameEntry_storeRecord(NameEntry *entry, void *arg);

void RankingTask_initLines(RankingTask *this);
void RankingTask_drawLines(RankingTask *this);
s32 RankingTask_isSkipPressed(RankingTask *this);
void RankingTask_draw(RankingTask *this);

INCLUDE_ASM("title/nonmatchings/ranking", rankingTaskInit);

/* The destructor of the ranking's task: clears the screen to black, unloads the sounds, then destroys the
 * name entry, the records and the task. */
void RankingTask_destroy(RankingTask *this, s32 flags) {
    this->task.vtable = &RANKING_TASK_VTABLE.task;
    displaySetClearColor(CURRENT_DISPLAY, 0, 0, 0);
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    NameEntry_destroy(&this->entry, DESTROY_BASES);
    RankingTable_destroy(&this->records, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

/* The ranking's frame: runs the name entry while it is open; else draws the ranking, fades out at frame 0x1CC
 * (or on Start or the confirm button) and kills the task once the fade is done. */
void RankingTask_tick(RankingTask *this) {
    NameEntry *entry = &this->entry;

    if (this->entry.active != 0) {
        NameEntry_update(entry);
        NameEntry_draw(entry);
        return;
    }
    if (this->frame == 0) {
        if (this->entry.finished != 0) {
            NameEntry_storeRecord(entry, &this->records);
        }
        RankingTask_initLines(this);
    }
    RankingTask_drawLines(this);
    RankingTask_draw(this);
    if (++this->frame == 0x1CC) {
        fadeControlFadeOut(&FADE_CONTROL, 0x14);
    }
    if (this->frame < 0x1E1 && RankingTask_isSkipPressed(this) == 0) {
        return;
    }
    if (fadeControlIsDone(&FADE_CONTROL)) {
        fadeControlFadeIn(&FADE_CONTROL, 0x14);
        taskKill(&this->task);
    }
}

/* Whether Start or the confirm button was pressed on either pad. */
s32 RankingTask_isSkipPressed(RankingTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    return (padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & (PADstart | PAD_CONFIRM);
}

/* Loads "title/ranking.tim", uploads its TIMs to VRAM and frees it. */
void loadRankingTexture(void) {
    u32 *file = loadFile("title/ranking.tim");

    uploadTims(file);
    mainHeapFree(file);
}

/* Writes value with digits digits, zero padded, as two-byte (full-width) digits into text.
 * The match depends on reading each digit in the loop's test and storing both bytes before text += 2. */
void writeWideNumber(u8 *text, s32 value, s32 digits) {
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

INCLUDE_ASM("title/nonmatchings/ranking", RankingTask_initLines);

INCLUDE_ASM("title/nonmatchings/ranking", RankingTask_drawLines);

INCLUDE_ASM("title/nonmatchings/ranking", RankingTask_drawFace);

INCLUDE_ASM("title/nonmatchings/ranking", RankingTask_drawLineBar);

/* An empty function that nothing calls. */
void func_80073F8C(void) {
}

/* Draws RankingTask_tick's screen: drawSideStrips's part, two of RANKING_IMAGES's sprites, then the 0x140 x 0xF0
 * picture at (0, 0) of texture pages 7 and 8. */
void RankingTask_draw(RankingTask *this) {
    u_long *ot = FRAME_OT.firstDrawn + 5;
    SPRT *sprt;
    DR_TPAGE *tpage;

    drawSideStrips(ot, this->frame);
    sprt = (SPRT *)drawImage(ot, 0x2F, 0xD, &RANKING_IMAGES[0]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)drawImage(ot, 0x5E, 0xD0, &RANKING_IMAGES[2]);
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

/* Update: keeps arg and runs the task's tick. */
void RankingTask_update(TickTask *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/ranking", RANKING_TASK_VTABLE);
INCLUDE_RODATA("title/nonmatchings/ranking", RANKING_IMAGES);
