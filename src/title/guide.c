/* The guide (the task "Guide Logo", title/guide.tim): the minigames' explanations. */

#include "common.h"
#include <libetc.h>
#include "engine/gfx/camera.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/lib/string.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

/* The guide logo task, which guideLogoTaskInit sets up: the host explains a bonus game in a window, typing
 * its text a character a frame, four lines a page. */
typedef struct GuideLogoTask {
    /* 0x000 */ Task task;
    /* 0x020 */ s32 updateArg; /* the argument of the last update */
    /* 0x024 */ EntityGroup entities;
    /* 0x034 */ TitleChara *chara; /* the guide's host (title_chara.c) */
    /* 0x038 */ u32 sound; /* a sound's handle (stopSound) */
    /* 0x03C */ Camera camera;
    /* 0x078 */ MatrixWords view; /* GuideLogoTask_tick looks at the guide through it */
    /* 0x098 */ VECTOR unk98;
    /* 0x0A8 */ MersenneTwister random;
    /* 0xA6C */ u16 frame; /* counted every tick: the strips and the page mark follow it */
    /* 0xA6E */ s16 leaving; /* -1 once the guide fades out to leave */
    /* 0xA70 */ s16 leaveFrame; /* frames since it started leaving */
    /* 0xA72 */ s16 typeDelay; /* counts to 0x21 before the typing starts */
    /* 0xA74 */ u16 bonusGame; /* guideLogoTaskInit's argument: which text (GUIDE_TEXTS) and pictures */
    /* 0xA76 */ u16 typeAll; /* the confirm button set it: type the rest of the page at once */
    /* 0xA78 */ u16 typed; /* characters typed so far */
    /* 0xA7A */ u16 nextPageCount; /* the next page's first character count, and nextPageText its text */
    /* 0xA7C */ u16 pageCount; /* the page's, and pageText its text */
    /* 0xA7E */ u8 unkA7E[0xA80 - 0xA7E];
    /* 0xA80 */ u8 *nextPageText; /* in the text of bonusGame */
    /* 0xA84 */ u8 *pageText;
    /* 0xA88 */ s16 pageFull; /* GuideLogoTask_drawPageMark draws its blinking mark while it is set */
    /* 0xA8A */ s16 textDone; /* the whole text is out: the confirm button leaves */
    /* 0xA8C */ u32 playedVoices; /* a bit per voice tag already played (GuideLogoTask_typeText) */
} GuideLogoTask;


extern TickTaskVtable GUIDE_LOGO_TASK_VTABLE;
extern u8 *GUIDE_TEXTS[]; /* the guides' texts, by bonus game */

void GuideLogoTask_resetText(GuideLogoTask *this);
void func_80073208(void);
void GuideLogoTask_drawBonusGamePicture(GuideLogoTask *this);
void GuideLogoTask_drawTextWindow(GuideLogoTask *this);
void GuideLogoTask_typeText(GuideLogoTask *this);
void GuideLogoTask_drawPageMark(GuideLogoTask *this);

/* The minigames' explanations, which GUIDE_TEXTS points into. */
INCLUDE_RODATA("title/nonmatchings/guide", STR_GUIDE_TEXTS);

/* The same as drawImage. */
SprtColorWords *guideDrawImage(u_long *ot, s32 x, s32 y, TitleImage *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut.word;
    sprt->wh = image->wh.word;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* drawSideStrips with texture (0..15, 0x40..0x80), page 0x16 and CLUT 0x7F18: two columns of six scrolling
 * strips. The match depends on the same forms as drawSideStrips's. */
void guideDrawSideStrips(u_long *ot, s32 frame) {
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

/* The destructor of the guide logo task: stops its sound, unloads the sounds, deletes the host, fades the
 * screen in, then destroys the camera, the entities and the task. */
void GuideLogoTask_destroy(GuideLogoTask *this, s32 flags) {
    this->task.vtable = &GUIDE_LOGO_TASK_VTABLE.task;
    stopSound(this->sound);
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    entityDelete((Entity *)this->chara);
    fadeControlFadeIn(&FADE_CONTROL, 0);
    cameraDestroy(&this->camera, DESTROY_BASES);
    entityGroupDestroy(&this->entities, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_CAMERA_UP);
INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_CAMERA_TARGET);
INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_CAMERA_EYE);

INCLUDE_ASM("title/nonmatchings/guide", GuideLogoTask_tick);

/* Reads both pads: the cancel button starts leaving (fade out, the host's action 1); the confirm button sets
 * typeAll or plays the host's action 9, and leaves too once textDone is set. */
void GuideLogoTask_readPads(GuideLogoTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 port;

    for (port = 0; port < 2; port++) {
        u32 pressed = padManagerGetPressed(pads, port);

        if ((pressed & PAD_CANCEL) && this->leaving == 0) {
            this->leaving = -1;
            fadeControlFadeOut(&FADE_CONTROL, 0x1E);
            TitleChara_setAction(this->chara, 1);
        }
        if (pressed & PAD_CONFIRM) {
            if (this->pageFull == 0) {
                this->typeAll = 1;
            } else if (TitleChara_getAction(this->chara) == TITLE_CHARA_IDLE) {
                TitleChara_setAction(this->chara, 9);
            }
            if (this->textDone != 0 && this->leaving == 0) {
                this->leaving = -1;
                fadeControlFadeOut(&FADE_CONTROL, 0x1E);
                TitleChara_setAction(this->chara, 1);
            }
        }
    }
}

/* Makes the guide's host (TitleChara_init), seen by its camera, at position turned by rotation. */
static inline TitleChara *guideNewChara(GuideLogoTask *this, Vec3 *position, Vec3 *rotation) {
    return (TitleChara *)NEW_ENTITY(sizeof(TitleChara), TitleChara_init, &this->camera, position, rotation);
}

/* Sets the view to the identity and 0x98 to (0xC00, 0xC00, 0xC00), then makes the host at
 * (-0x100, 0, 0), turned 0x78E about y, adds it to the entities and makes it interruptible. The match depends on
 * guideNewChara taking the position as a parameter: its copy keeps the address in $s2 across the allocation
 * (the rotation, at frame offset 0x10, gets integrate.c's constant equivalence and is folded back into
 * `sp + 0x20`; the position, at offset 0, does not). */
void GuideLogoTask_createModel(GuideLogoTask *this) {
    Vec3 position;
    Vec3 rotation;
    TitleChara *chara;

    this->view.words[0] = ONE;
    this->view.words[1] = 0;
    this->view.words[2] = ONE;
    this->view.words[3] = 0;
    this->view.words[4] = ONE;
    this->view.m.t[0] = 0;
    this->view.m.t[1] = 0;
    this->view.m.t[2] = 0;
    this->unk98.vx = 0xC00;
    this->unk98.vy = 0xC00;
    this->unk98.vz = 0xC00;
    this->unk98.pad = 0;
    position.vx = -0x100;
    position.vy = 0;
    position.vz = 0;
    rotation.vx = 0;
    rotation.vy = 0x78E;
    rotation.vz = 0;
    chara = guideNewChara(this, &position, &rotation);
    this->chara = chara;
    if (chara != NULL) {
        entityGroupAppend(&this->entities, &chara->entity);
    }
    this->chara->interruptible = 1;
}

/* Calls the empty func_80073208, then GuideLogoTask_resetText, which resets the typing (typeAll to textDone). */
void GuideLogoTask_startText(void *this) {
    func_80073208();
    GuideLogoTask_resetText(this);
}

/* A frame of the guide: draws the blinking mark, counts typeDelay up to 0x21 and then runs
 * GuideLogoTask_typeText, then runs GuideLogoTask_drawBonusGamePicture and GuideLogoTask_drawTextWindow. */
void GuideLogoTask_drawExplanation(GuideLogoTask *this) {
    GuideLogoTask_drawPageMark(this);
    if (this->typeDelay > 0x20) {
        GuideLogoTask_typeText(this);
    } else {
        this->typeDelay++;
    }
    GuideLogoTask_drawBonusGamePicture(this);
    GuideLogoTask_drawTextWindow(this);
}

/* An empty function that guideLogoTaskInit calls. */void func_80072560(void) {
}

extern TitleImage GUIDE_IMAGES[];

/* Draws the guide logo screen: guideDrawSideStrips's strips at the frame count, picture 4 + bonusGame (the
 * game's title) and picture 14 of GUIDE_IMAGES, then the 0x140 x 0xF0 picture at (0, 0) of texture pages 6
 * and 7 (RankingTask_draw's shape). */
void GuideLogoTask_drawScreen(GuideLogoTask *this) {
    u_long *ot = this->camera.ot.head;
    TitleImage *pictures;
    SPRT *sprt;
    DR_TPAGE *tpage;

    guideDrawSideStrips(ot, this->frame);
    /* The match depends on the table in a local, set after the first call (it then keeps s1). */
    pictures = GUIDE_IMAGES;
    sprt = (SPRT *)guideDrawImage(ot, 0x2F, 0xD, &pictures[(u16)(this->bonusGame + 4)]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)guideDrawImage(ot, 0x46, 0xD1, &pictures[14]);
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

/* An empty function that nothing calls. */void func_800726E4(void) {
}

INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_PICTURE_UVS);
INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_PICTURE_CLUTS);

INCLUDE_ASM("title/nonmatchings/guide", GuideLogoTask_drawBonusGamePicture);

/* An empty function that nothing calls. */void func_800728D0(void) {
}

INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_WINDOW_CORNERS);

INCLUDE_ASM("title/nonmatchings/guide", GuideLogoTask_drawTextWindow);

/* Resets the typing to the first page of the bonus game's text (GUIDE_TEXTS). */
void GuideLogoTask_resetText(GuideLogoTask *this) {
    u8 *value;

    this->typeAll = 0;
    this->typed = 0;
    this->nextPageCount = 0;
    this->pageCount = 0;
    value = GUIDE_TEXTS[this->bonusGame];
    this->pageFull = 0;
    this->textDone = 0;
    this->nextPageText = value;
    this->pageText = value;
}

/* A TextPrinter's virtual setColor and putChar, inlined as the game has them (the virtual table read from the
 * stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)
#define TEXT_PUT_CHAR(printer, x, y, code) \
    (printer).printer.vtable->putChar.func((u8 *)&(printer) + (printer).printer.vtable->putChar.delta, x, y, code)

/* Types out the guide's text (GUIDE_TEXTS by bonusGame) a character more each frame, four lines of 16 columns
 * a page, from (0x20, 0x86); the confirm button turns a full page. Tags: "<cN>" sets one of five colours,
 * "<i" and two more characters skip a column, "<vN>" plays voice N once (and the host's TITLE_CHARA_TALK).
 * The match depends on three forms: each colour case calls setColor itself, with its text++ after the call
 * (cross-jumping merges the calls' tails and leaves the virtual table read in each case); `c - '0'` switched
 * on, not stored (one register for the character and the index); the destructor's address taken into a local
 * (gcse then keeps the printer's address in $s4 across the loop). */
void GuideLogoTask_typeText(GuideLogoTask *this) {
    TextPrinter printer;
    Printer *base;
    s32 x = 0x20;
    s32 y = 0x86;
    s32 column = 0;
    s32 line;
#if !VERSION_JP
    s32 narrow; /* characters printed on the line, each half a column wide */
#endif
    s32 length;
    s32 count;
    u8 *text;
    u32 c;
    u32 voice;
    u_long *ot = this->camera.ot.slots;
    PadManager *pads = PAD_MANAGER_INSTANCE;

    length = strlen(GUIDE_TEXTS[this->bonusGame]);
    textPrinterInit(&printer, ot);
#if VERSION_JP
    textPrinterSetShadow(&printer, 1);
    textPrinterSetShadowColor(&printer, 0x404040);
#endif
    line = 0;
#if !VERSION_JP
    narrow = 0;
#endif
    if (this->pageFull == 0) {
        this->typed++;
        if (this->typed > length) {
#if VERSION_JP
            this->typed = length >> 1;
#else
            this->typed = length;
#endif
        }
    }
    if ((padManagerGetPressed(pads, -1) & PAD_CONFIRM) && this->pageFull != 0) {
        this->pageFull = 0;
        this->typeAll = 0;
        this->pageCount = this->nextPageCount;
        this->pageText = this->nextPageText;
    }
    count = this->pageCount;
    text = this->pageText;
    for (;;) {
        c = *text++;
        if (c == 0) {
            this->textDone = 1;
            break;
        }
        if (this->typeAll == 0) {
            if (count >= this->typed) {
                break;
            }
        } else if (this->pageFull != 0) {
            break;
        }
        switch (c) {
        case '\n':
            x = 0x20;
            y += 0x10;
            count++;
            column = 0;
#if !VERSION_JP
            narrow = 0;
#endif
            line++;
            break;
        case '\t':
            x += 0x40;
            column += 4;
            count++;
            break;
        case '<':
            c = *text++;
            switch (c) {
            case 'c':
                c = *text++;
                switch (c - '0') {
                case 0:
                    TEXT_SET_COLOR(printer, 0x404080);
                    text++;
                    break;
                case 1:
                    TEXT_SET_COLOR(printer, 0x408040);
                    text++;
                    break;
                case 2:
                    TEXT_SET_COLOR(printer, 0x804040);
                    text++;
                    break;
                case 3:
                    TEXT_SET_COLOR(printer, 0x408080);
                    text++;
                    break;
                default:
                    TEXT_SET_COLOR(printer, 0x808080);
                    break;
                }
                break;
            case 'i':
                x += 0x10;
                column++;
                text += 2;
                break;
            case 'v':
                c = *text++;
                voice = c - '0';
                if (!(this->playedVoices & (1 << voice))) {
                    if (this->sound != 0) {
                        stopSound(this->sound);
                    }
                    switch (voice) {
                    case 0:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x2F, 0);
                        break;
                    case 1:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x30, 0);
                        break;
                    case 2:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x32, 0);
                        break;
                    case 3:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x33, 0);
                        break;
                    case 4:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x34, 0);
                        break;
                    case 5:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x35, 0);
                        break;
                    case 6:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x36, 0);
                        break;
                    case 7:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x37, 0);
                        break;
                    case 8:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x38, 0);
                        break;
                    case 9:
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x39, 0);
                        break;
                    case 'a' - '0':
                        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x3E, 0);
                        break;
                    }
                    TitleChara_setVoice(this->chara, this->sound);
                    TitleChara_setAction(this->chara, TITLE_CHARA_TALK);
                    this->playedVoices |= 1 << voice;
                }
                text++;
                break;
            }
            break;
        default:
            if (IS_SJIS_LEAD(c)) {
                c = (c << 8) | *text++;
            }
            printer.printer.x = x;
            printer.printer.y = y;
            TEXT_PUT_CHAR(printer, x, y, c);
#if VERSION_JP
            x += 0x10;
            count++;
            column++;
#else
            x += 8;
            count++;
            column++;
            narrow++;
#endif
            break;
        }
#if VERSION_JP
        if (column >= 0x10) {
#else
        if (column > (narrow + 0x22) / 2 - 1) {
#endif
            x = 0x20;
            y += 0x10;
            line++;
            column = 0;
#if !VERSION_JP
            narrow = 0;
#endif
        }
        if (line >= 4) {
            x = 0x20;
            y = 0x86;
            this->pageFull = 1;
            this->typed = count;
            this->nextPageCount = count;
            this->nextPageText = text;
            this->typeAll = 0;
        }
    }
    base = &printer.printer;
    printerDestroy(base, DESTROY_BASES);
}

/* An empty function that nothing calls. */void func_80073200(void) {
}

/* An empty function that GuideLogoTask_startText calls. */void func_80073208(void) {
}

/* While pageFull is set, draws a 16 by 16 mark at (0x110, 0xB4) of texture page 0x17 that blinks every 8 frames
 * (u 0x40 and 0x50). The match depends on the blink phase kept in a local: the test is then a computed
 * truth value (srl, xori, andi), not a branch on the bit. */
void GuideLogoTask_drawPageMark(GuideLogoTask *this) {
    u_long *ot = this->camera.ot.slots;
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 firstFrame;

    if (this->pageFull == 0) {
        return;
    }
    tpage = allocDrTpage(0x17);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    firstFrame = (this->frame & 8) == 0;
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

/* Clears leaving, leaveFrame and typeDelay. */
void GuideLogoTask_clearLeaving(GuideLogoTask *this) {
    this->leaving = 0;
    this->leaveFrame = 0;
    this->typeDelay = 0;
}

/* While leaving: stops the sound at frame 0x20 and kills the task after frame 0x80. */
void GuideLogoTask_tickLeaving(GuideLogoTask *this) {
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

/* Update: keeps arg and runs the task's tick. */
void GuideLogoTask_update(TickTask *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_LOGO_TASK_VTABLE);
INCLUDE_RODATA("title/nonmatchings/guide", GUIDE_IMAGES);
