/* The guide (the task "Guide Logo", title/guide.tim): the minigames' explanations. */

#include "common.h"
#include <libetc.h>
#include "engine/gfx/camera.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/lib/string.h"
#include "engine/math/matrix.h"
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
    /* 0x034 */ Unk80076E24 *model; /* a character model entity (title_0a.c) */
    /* 0x038 */ u32 sound; /* a sound's handle (stopSound) */
    /* 0x03C */ Camera camera;
    /* 0x078 */ MatrixWords view; /* func_800721C0 looks at the guide through it */
    /* 0x098 */ VECTOR unk98;
    /* 0x0A8 */ u8 unkA8[0xA6C - 0xA8]; /* a Mersenne twister */
    /* 0xA6C */ u16 unkA6C; /* a frame count (func_800721C0) */
    /* 0xA6E */ s16 leaving; /* -1 once the guide fades out to leave */
    /* 0xA70 */ s16 leaveFrame; /* frames since it started leaving */
    /* 0xA72 */ s16 unkA72;
    /* 0xA74 */ u16 unkA74; /* guideLogoTaskInit's argument */
    /* 0xA76 */ u16 unkA76; /* the confirm button set it: type the rest of the page at once */
    /* 0xA78 */ u16 unkA78; /* characters typed so far */
    /* 0xA7A */ u16 unkA7A; /* the next page's first character count, and 0xA80 its text */
    /* 0xA7C */ u16 unkA7C; /* the page's, and 0xA84 its text */
    /* 0xA7E */ u8 unkA7E[0xA80 - 0xA7E];
    /* 0xA80 */ u8 *unkA80; /* in the text of 0xA74 */
    /* 0xA84 */ u8 *unkA84;
    /* 0xA88 */ s16 unkA88; /* the page is full; func_80073210 draws its blinking mark while it is set */
    /* 0xA8A */ s16 unkA8A; /* the whole text is out */
    /* 0xA8C */ u32 unkA8C; /* a bit per voice tag already played (func_80072D2C) */
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
extern u8 *D_8007B244[]; /* the guides' texts, by 0xA74 */

void func_80072CF0(GuideLogoTask *this);
void func_80073208(void);
void func_800726EC(GuideLogoTask *this);
void func_800728D8(GuideLogoTask *this);
void func_80072D2C(GuideLogoTask *this);
void func_80073210(GuideLogoTask *this);

/* The minigames' explanations, which D_8007B244 points into. */
INCLUDE_RODATA("title/nonmatchings/guide", D_800659BC);

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

INCLUDE_RODATA("title/nonmatchings/guide", D_80065D90);
INCLUDE_RODATA("title/nonmatchings/guide", D_80065DA0);
INCLUDE_RODATA("title/nonmatchings/guide", D_80065DB0);

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

/* Makes the guide's character model (func_80076E24), seen by its camera, at position turned by rotation. */
static inline Unk80076E24 *guideNewModel(GuideLogoTask *this, Vec3 *position, Vec3 *rotation) {
    return (Unk80076E24 *)NEW_ENTITY(sizeof(Unk80076E24), func_80076E24, &this->camera, position, rotation);
}

/* Sets the view to the identity and 0x98 to (0xC00, 0xC00, 0xC00), then makes the character model at
 * (-0x100, 0, 0), turned 0x78E about y, adds it to the entities and sets its 0x38. The match depends on
 * guideNewModel taking the position as a parameter: its copy keeps the address in $s2 across the allocation
 * (the rotation, at frame offset 0x10, gets integrate.c's constant equivalence and is folded back into
 * `sp + 0x20`; the position, at offset 0, does not). */
void func_800723FC(GuideLogoTask *this) {
    Vec3 position;
    Vec3 rotation;
    Unk80076E24 *model;

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
    model = guideNewModel(this, &position, &rotation);
    this->model = model;
    if (model != NULL) {
        entityGroupAppend(&this->entities, &model->entity);
    }
    this->model->unk38 = 1;
}

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

INCLUDE_RODATA("title/nonmatchings/guide", D_80065DC0);
INCLUDE_RODATA("title/nonmatchings/guide", D_80065DCC);

INCLUDE_ASM("title/nonmatchings/guide", func_800726EC);

void func_800728D0(void) {
}

INCLUDE_RODATA("title/nonmatchings/guide", D_80065DD8);

INCLUDE_ASM("title/nonmatchings/guide", func_800728D8);

/* Resets 0xA76-0xA8A, 0xA80 and 0xA84 from D_8007B244 by 0xA74. */
void func_80072CF0(GuideLogoTask *this) {
    u8 *value;

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

/* A TextPrinter's virtual setColor and putChar, inlined as the game has them (the virtual table read from the
 * stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)
#define TEXT_PUT_CHAR(printer, x, y, code) \
    (printer).printer.vtable->putChar.func((u8 *)&(printer) + (printer).printer.vtable->putChar.delta, x, y, code)

/* Types out the guide's text (D_8007B244 by 0xA74) a character more each frame, four lines of 16 columns a
 * page, from (0x20, 0x86); the confirm button turns a full page. Tags: "<cN>" sets one of five colours, "<i"
 * and two more characters skip a column, "<vN>" plays voice N once (and the model's clip 0xB). The match depends
 * on three forms: each colour case calls setColor itself, with its text++ after the call (cross-jumping merges
 * the calls' tails and leaves the virtual table read in each case); `c - '0'` switched on, not stored (one
 * register for the character and the index); the destructor's address taken into a local (gcse then keeps the
 * printer's address in $s4 across the loop). */
void func_80072D2C(GuideLogoTask *this) {
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

    length = strlen(D_8007B244[this->unkA74]);
    textPrinterInit(&printer, ot);
#if VERSION_JP
    textPrinterSetShadow(&printer, 1);
    textPrinterSetShadowColor(&printer, 0x404040);
#endif
    line = 0;
#if !VERSION_JP
    narrow = 0;
#endif
    if (this->unkA88 == 0) {
        this->unkA78++;
        if (this->unkA78 > length) {
#if VERSION_JP
            this->unkA78 = length >> 1;
#else
            this->unkA78 = length;
#endif
        }
    }
    if ((padManagerGetPressed(pads, -1) & PAD_CONFIRM) && this->unkA88 != 0) {
        this->unkA88 = 0;
        this->unkA76 = 0;
        this->unkA7C = this->unkA7A;
        this->unkA84 = this->unkA80;
    }
    count = this->unkA7C;
    text = this->unkA84;
    for (;;) {
        c = *text++;
        if (c == 0) {
            this->unkA8A = 1;
            break;
        }
        if (this->unkA76 == 0) {
            if (count >= this->unkA78) {
                break;
            }
        } else if (this->unkA88 != 0) {
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
                if (!(this->unkA8C & (1 << voice))) {
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
                    func_800773E8(this->model, this->sound);
                    func_800773C8(this->model, 0xB);
                    this->unkA8C |= 1 << voice;
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
            this->unkA88 = 1;
            this->unkA78 = count;
            this->unkA7A = count;
            this->unkA80 = text;
            this->unkA76 = 0;
        }
    }
    base = &printer.printer;
    printerDestroy(base, DESTROY_BASES);
}

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

INCLUDE_RODATA("title/nonmatchings/guide", D_80065EC4);
INCLUDE_RODATA("title/nonmatchings/guide", D_80065EF4);
