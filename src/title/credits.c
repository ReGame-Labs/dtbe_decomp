/* The credits (/credit.txt, /credit.k15, /title/credit.tim). */

#include "common.h"
#include "engine/cd/read.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "title/title_classes.h"
#include "psyq.h"
#include "vtable.h"

/* The credits task (CREDITS_TASK_VTABLE), as creditsTaskInit builds it: /credit.txt's lines scroll up over
 * the slides of /title/credit.tim until Start or frame 0x1480. */
typedef struct CreditsTask {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 updateArg; /* the argument of the last update */
    /* 0x24 */ u8 *text;   /* /credit.txt, its lines cut apart by splitLines */
    /* 0x28 */ u8 **lines; /* the start of each line of text */
    /* 0x2C */ Font *font;
    /* 0x30 */ u16 *fontData; /* /credit.k15 */
    /* 0x34 */ s32 lineCount; /* creditsCountLines' count of text */
    /* 0x38 */ s32 frame;     /* the slides, the pictures and the end are timed by it */
    /* 0x3C */ s32 leaving;   /* set on Start or at frame 0x1480: the task ends once the fade out is done */
    /* 0x40 */ s32 scroll;    /* in lines, 16.16 (the pixel is (scroll >> 12) & 0xF): 0x800 more a frame */
} CreditsTask; /* size 0x44 */

extern TickTaskVtable CREDITS_TASK_VTABLE;

/* the path of the credits' text, a pointer to the "/credit.txt" kept after STR_CREDITS_TASK_NAME */
extern char *CREDITS_TEXT_PATH;

/* creditsTaskInit's strings, kept as data: "no name" is followed by the "/credit.txt" of CREDITS_TEXT_PATH, and
 * "/title/credit.tim" by four zero bytes nothing reads, neither of which the literals of a C function give. */
INCLUDE_RODATA("title/nonmatchings/credits", STR_CREDITS_TASK_NAME);
extern char STR_CREDITS_TASK_NAME[]; /* "no name" */
INCLUDE_RODATA("title/nonmatchings/credits", PATH_CREDIT_K15);
extern char PATH_CREDIT_K15[]; /* "/credit.k15" */
INCLUDE_RODATA("title/nonmatchings/credits", PATH_CREDIT_TIM);
extern char PATH_CREDIT_TIM[]; /* "/title/credit.tim" */

s32 creditsCountLines(const u8 *text);
void splitLines(u8 **lines, u8 *text);
void setTimClutsStp(u32 *tims);

/* The credits' task: a 512 by 240 black screen, the text of CREDITS_TEXT_PATH cut into lines, the font
 * /credit.k15 and the pictures of /title/credit.tim, a fade in over 90 frames and music track 16 from its start. */
CreditsTask *creditsTaskInit(CreditsTask *this) {
    Display *display;
    u32 size;
    u32 *tims;

    taskInit(&this->task, 0, STR_CREDITS_TASK_NAME);
    this->task.vtable = &CREDITS_TASK_VTABLE.task;
    display = CURRENT_DISPLAY;
    displaySetSize(display, 512, 240);
    displaySetVsyncMode(display, 0);
    displaySetClearColor(display, 0, 0, 0);
    displayClear(display);
    size = getFileSize(CREDITS_TEXT_PATH);
    this->text = operatorVecNew(size + 1);
    loadFileSync(this->text, CREDITS_TEXT_PATH, 0, 0);
    this->text[size] = '\0';
    this->lineCount = creditsCountLines(this->text);
    this->lines = operatorVecNew(this->lineCount * sizeof(u8 *));
    splitLines(this->lines, this->text);
    this->fontData = loadFile(PATH_CREDIT_K15);
    this->font = fontInit(operatorNew(sizeof(Font)), this->fontData);
    cacheReset(&getFontCache()->cache);
    tims = loadFile(PATH_CREDIT_TIM);
    setTimClutsStp(tims);
    uploadTims(tims);
    mainHeapFree(tims);
    fadeControlFadeOut(&FADE_CONTROL, 0);
    fadeControlFadeIn(&FADE_CONTROL, 90);
    this->scroll = 0xFFF10000;
    this->frame = 0;
    this->leaving = 0;
    seekMusicTrack(16, 1);
    soundControlResumeMusic(&SOUND_CONTROL);
    return this;
}

/* Sets the STP bit of every CLUT color of a run of TIMs (/title/credit.tim), before they go to VRAM. */
void setTimClutsStp(u32 *tims) {
    TIM_IMAGE image;

    OpenTIM((u_long *)tims);
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
s32 creditsCountLines(const u8 *text) {
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
void splitLines(u8 **lines, u8 *text) {
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
void CreditsTask_destroy(CreditsTask *this, s32 flags) {
    this->task.vtable = &CREDITS_TASK_VTABLE.task;
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

INCLUDE_ASM("title/nonmatchings/credits", CreditsTask_tick);

#if VERSION_JP
/* Draws a 408 x 88 picture of texture page 0x9A, only in the Japanese release, as two 204 x 88 halves stacked
 * in the texture (v 0 and 88) side by side from x - 204, y, as bright as brightness; nothing at brightness 0. */
void func_80076B0C(u_long *ot, s32 x, s32 y, s32 brightness) {
    if (brightness != 0) {
        DR_TPAGE *tpage = allocDrTpage(0x9A);
        SPRT *sprt = allocSprt();

        sprt->x0 = x - 204;
        sprt->r0 = brightness;
        sprt->g0 = brightness;
        sprt->b0 = brightness;
        sprt->y0 = y;
        sprt->u0 = 0;
        sprt->v0 = 0;
        sprt->w = 204;
        sprt->h = 88;
        sprt->clut = 0x3E20;
        AddPrim(ot, sprt);
        sprt = allocSprt();
        sprt->r0 = brightness;
        sprt->g0 = brightness;
        sprt->b0 = brightness;
        sprt->x0 = x;
        sprt->y0 = y;
        sprt->u0 = 0;
        sprt->v0 = 88;
        sprt->w = 204;
        sprt->h = 88;
        sprt->clut = 0x3DE0;
        AddPrim(ot, sprt);
        AddPrim(ot, tpage);
    }
}
#endif

/* Draws the credits' end picture (256 x 120 from texture page 13) at x - 128, y, as bright as brightness;
 * nothing at brightness 0. */
void drawCreditsEndPicture(u_long *ot, s32 x, s32 y, s32 brightness) {
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

/* Draws slide (0 to 6) of /title/credit.tim at x, y: its 112 x 240 texture stretched to 222 x 239, as bright as
 * brightness and semi-transparent; in VRAM slides 0 to 4 are texture pages from x 0x200 at y 0, 5 and 6 from
 * x 0x200 at y 0x100, each with its clut on row 0xFF - slide. Nothing at brightness 0. */
void drawCreditsSlide(u_long *ot, s32 x, s32 y, s32 slide, s32 brightness) {
    if (brightness != 0) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 tx;
        u16 ty;

        setRGB0(poly, brightness, brightness, brightness);
        poly->x0 = x;
        poly->y0 = y;
        poly->x1 = x + 0xDE;
        poly->y1 = y;
        poly->x2 = x;
        poly->y2 = y + 0xEF;
        poly->x3 = x + 0xDE;
        poly->y3 = y + 0xEF;
        setUV4(poly, 0, 0, 0x6F, 0, 0, 0xEF, 0x6F, 0xEF);
        if (slide < 5) {
            tx = slide * 64 + 0x200;
            ty = 0;
        } else {
            tx = slide * 64 + 0xC0;
            ty = 0x100;
        }
        /* The match depends on ty being a u16 and on the x bits or-ed with the modes before ty's bit (srl, then
         * the ori on the x bits). */
        poly->tpage = (ty >> 4) | (((tx & 0x3FF) >> 6) | getTPage(1, 1, 0, 0));
        poly->clut = getClut(0x200, 0xFF - slide);
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
}

/* Update: keeps arg and runs the task's tick. */
void CreditsTask_update(TickTask *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/credits", CREDITS_TASK_VTABLE);
