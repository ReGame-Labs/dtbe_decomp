/* The class of the constructor func_800D02C8 (virtual table D_8006E154), derived from the battle scene's,
 * and the figures it prints (%06d, %03d, %02d). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/display.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/text/font.h"

extern EntityVtable D_8006E014;
extern EntityVtable D_8006E03C;
extern EntityVtable D_8006E064;
extern EntityVtable D_8006E08C;
extern EntityVtable D_8006E0B4;
extern EntityVtable D_8006E0DC;
extern EntityVtable D_8006E104;
extern EntityVtable D_8006E12C;
extern VtableEntry D_8006E154[];
extern u8 D_8006DFCC[];
extern u8 D_8006DFD4[]; /* "%03d" */

/* The entity classes that func_800D0CF8 and func_800D0F1C make for
 * func_8006ED20's object: D_8006E12C, D_8006E104, D_8006E0DC, and D_8006E0B4 with a
 * field more. */
typedef struct Unk800D1328 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *unk18;
} Unk800D1328;
typedef struct Unk800D1C50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *unk18;
    /* 0x1C */ s16 unk1C;
} Unk800D1C50;

/* Constructs an object of the class of vtable D_8006E154, which derives from func_8006ED20's. */
Unk800D02C8 *func_800D02C8(Unk800D02C8 *this) {
    func_8006ED20(&this->base);
    this->base.vtable = D_8006E154;
    return this;
}

/* The destructor: clears the screen to black, stops the sound at 0xDA4, then destroys the battle object's part
 * (func_8006F254). */
void func_800D02FC(Unk800D02C8 *this, s32 flags) {
    this->base.vtable = D_8006E154;
    displaySetClearColor(CURRENT_DISPLAY, 0, 0, 0);
    stopSound(this->unkDA4);
    func_8006F254(&this->base, flags);
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0364);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0504);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D06D0);

/* Stops the sound at 0xDA4. */
void func_800D07FC(Unk800D02C8 *this) {
    stopSound(this->unkDA4);
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0820);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D098C);

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DF5C);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0ABC);

/* func_80071BF8's start without its wait: plays sounds 0x8009 and 0x42, runs func_80079268 on both sides'
 * characters, adds a text entity of class 9 and keeps its handle, then sets bits 5 and 6 of 0x2C8 and 0xD9C to 2. */
void func_800D0B80(Unk800D02C8 *this) {
    Unk80076F34 *chara0;
    Unk80076F34 *chara1;

    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    chara0 = func_800700C8(&this->base, 0);
    chara1 = func_800700C8(&this->base, 1);
    func_80079268(chara0);
    func_80079268(chara1);
    this->unkDA0 = entityGetHandle(func_8006F0B4(&this->base, func_800CB7B0(entityOperatorNew(0x59C), 9, 0)));
    func_80073288(&this->base);
    func_800732B0(&this->base);
    this->unkD9C = 2;
}

/* The part of st07's stage (its Unk800D3B14) that func_800D0C50 and func_800D0C9C use; Unk800D3284, further on,
 * types more of the same object. */
typedef struct Unk800D0C50 {
    /* 0x000 */ Unk800D02C8 base;
    /* 0xDA8 */ u8 unkDA8[0xF2C - 0xDA8];
    /* 0xF2C */ s16 unkF2C[2];     /* by side: cleared with the counts (func_800D0CF8) */
    /* 0xF30 */ u8 unkF30[0xF38 - 0xF30];
    /* 0xF38 */ s16 unkF38[2];     /* by side: how many entries of unkF3C are set */
    /* 0xF3C */ s16 unkF3C[2][10]; /* by side: func_800D0C50 sets the next one to 1 */
    /* 0xF64 */ u16 unkF64[2];     /* by func_80077574's index: func_800D0C9C adds it to its argument */
} Unk800D0C50;

/* Sets the side's next entry of 0xF3C to 1 and counts it. */
void func_800D0C50(Unk800D0C50 *this, s16 side) {
    this->unkF3C[side][this->unkF38[side]] = 1;
    this->unkF38[side]++;
}

/* func_800773F4 on the character with arg plus the 0xF64 entry of its index (func_80077574). */
void func_800D0C9C(Unk800D0C50 *this, Unk80076F34 *chara, u16 arg) {
    func_800773F4(chara, this->unkF64[(u16)func_80077574(chara)] + arg);
}

Unk800D1328 *func_800D1328(Unk800D1328 *this, Unk8006ED20 *arg);
Unk800D1328 *func_800D17E4(Unk800D1328 *this, Unk8006ED20 *arg);
Unk800D1C50 *func_800D1C50(Unk800D1C50 *this, Unk8006ED20 *arg);
Unk800D1F8C *func_800D1F8C(Unk800D1F8C *this, Unk8006ED20 *owner);
Unk800D1F8C *func_800D23C4(Unk800D1F8C *this, Unk8006ED20 *owner);

/* Clears both sides' 0xF2C, counts (0xF38), 0xF64 and entries (0xF3C), then adds this mode's two entities:
 * 0 those of D_8006E12C and D_8006E0B4, 1 those of D_8006E104 and func_800D23C4, 2 those of D_8006E12C and
 * func_800D1F8C. */
void func_800D0CF8(Unk800D0C50 *this, s32 mode) {
    Entity *entity;
    s32 side;
    s32 i;

    for (side = 0; side < 2; side++) {
        this->unkF2C[side] = 0;
        this->unkF38[side] = 0;
        this->unkF64[side] = 0;
    }
    for (side = 0; side < 2; side++) {
        for (i = 0; i < 10; i++) {
            this->unkF3C[side][i] = 0;
        }
    }
    switch (mode) {
    case 0:
        entity = NEW_ENTITY(0x1C, func_800D1328, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        entity = NEW_ENTITY(0x20, func_800D1C50, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        break;
    case 1:
        entity = NEW_ENTITY(0x1C, func_800D17E4, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        entity = NEW_ENTITY(0x70, func_800D23C4, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        break;
    case 2:
        entity = NEW_ENTITY(0x1C, func_800D1328, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        entity = NEW_ENTITY(0x70, func_800D1F8C, &this->base.base);
        if (entity != NULL) {
            func_8006F0B4(&this->base.base, entity);
        }
        break;
    }
}

Unk800D1328 *func_800D27E0(Unk800D1328 *this, Unk8006ED20 *arg);

/* Makes func_800D27E0's entity for this and, when it was allocated, adds it to this (func_8006F0B4). */
void func_800D0F1C(Unk8006ED20 *this) {
    Entity *entity = NEW_ENTITY(0x1C, func_800D27E0, this);

    if (entity != NULL) {
        func_8006F0B4(this, entity);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0F7C);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D110C);

/* Constructs an entity of the class of virtual table D_8006E12C, for func_8006ED20's object. */
Unk800D1328 *func_800D1328(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E12C;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E12C, which func_800D1328 installs. */
void func_800D1370(Entity *this, s32 flags) {
    this->vtable = &D_8006E12C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D13C4);

/* Constructs an entity of the class of virtual table D_8006E104, for func_8006ED20's object. */
Unk800D1328 *func_800D17E4(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E104;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E104, which func_800D17E4 installs. */
void func_800D182C(Entity *this, s32 flags) {
    this->vtable = &D_8006E104;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D1880);

/* Constructs an entity of the class of virtual table D_8006E0B4, for func_8006ED20's object. */
Unk800D1C50 *func_800D1C50(Unk800D1C50 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E0B4;
    this->unk18 = arg;
    this->unk1C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006E0B4, which func_800D1C50 installs. */
void func_800D1C9C(Entity *this, s32 flags) {
    this->vtable = &D_8006E0B4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Adds 1 to 0x1C each time its group gathers it (the gather of D_8006E0B4). */
void func_800D1CF0(Unk800D1C50 *this) {
    this->unk1C++;
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D1D04);

/* Constructs an Unk800D1F8C (vtable D_8006E08C) of owner: copies the 16-color palettes at x 640, y 245 and 244 of
 * VRAM. */
Unk800D1F8C *func_800D1F8C(Unk800D1F8C *this, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E08C;
    this->owner = owner;
    this->unk68 = 0;
    this->clutRects[0].x = 0x280;
    this->clutRects[0].y = 0xF5;
    this->clutRects[0].w = 0x10;
    this->clutRects[0].h = 1;
    StoreImage(&this->clutRects[0], (u_long *)this->cluts[0]);
    this->clutRects[1].x = 0x280;
    this->clutRects[1].y = 0xF4;
    this->clutRects[1].w = 0x10;
    this->clutRects[1].h = 1;
    StoreImage(&this->clutRects[1], (u_long *)this->cluts[1]);
    return this;
}

/* The destructor of the entity class of vtable D_8006E08C, which func_800D1F8C installs. */
void func_800D2034(Entity *this, s32 flags) {
    this->vtable = &D_8006E08C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The object of the setters func_800D3284-func_800D32AC (and of func_800D326C, which reads 0xD98), none of them
 * called. Probably func_800D02C8's class (the stage base of st07-st09: their stage objects have 0xF34 and
 * 0xF6A too); func_800D24C0 prints 0xF30 and 0xF32 for its owner, by side. */
typedef struct Unk800D3284 {
    /* 0x000 */ u8 unk0[0xD98];
    /* 0xD98 */ s16 unkD98[(0xF2C - 0xD98) / 2];
    /* 0xF2C */ s16 unkF2C[2];
    /* 0xF30 */ s16 unkF30[2];
    /* 0xF34 */ s16 unkF34;
    /* 0xF36 */ s16 unkF36;
    /* 0xF38 */ u8 unkF38[0xF64 - 0xF38];
    /* 0xF64 */ u16 unkF64[2]; /* by side: func_800D2088 prints them and flashes the higher one's palette */
    /* 0xF68 */ u8 unkF68[0xF6A - 0xF68];
    /* 0xF6A */ u16 unkF6A; /* func_800D2088 and func_800D24C0 draw nothing while it is set */
} Unk800D3284;

/* The owner of an Unk800D1F8C as the stage it is (game_classes.h types it Unk8006ED20). */
static inline Unk800D3284 *paletteOwner(Unk800D1F8C *this) {
    return (Unk800D3284 *)this->owner;
}

/* A TextPrinter's virtual calls, inlined as the game has them: printerSetColor and func_80032900 on the
 * printer object itself (its virtual table read from the stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)
#define TEXT_SET_UNK18(printer, value) \
    (printer).printer.vtable->unk18.func((u8 *)&(printer) + (printer).printer.vtable->unk18.delta, value)

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DFCC);

/* The update of D_8006E08C's entity: unless the owner's 0xF6A is set, prints the two sides' 0xF64 counts
 * (shadowed); if 0xF6A got set meanwhile, pulses entries 7 and 8 of the leading side's palette; then draws two
 * 120 by 24 sprites of the palettes. */
void func_800D2088(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->unkF6A != 0) {
        return;
    }
    ot = camera->ot.slots;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 77;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[0]);
    printer.printer.x = 236;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 78;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[0]);
    printer.printer.x = 237;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFCC, paletteOwner(this)->unkF64[1]);

    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->unkF6A != 0) {
        /* The match depends on the operand order of both compares (0xF64 loads first, then 0xF66). */
        if (paletteOwner(this)->unkF64[0] > paletteOwner(this)->unkF64[1]) {
            this->cluts[0][7] = color;
            this->cluts[0][8] = brightColor;
            LoadImage(&this->clutRects[0], (u_long *)this->cluts[0]);
        }
        if (paletteOwner(this)->unkF64[1] > paletteOwner(this)->unkF64[0]) {
            this->cluts[1][7] = color;
            this->cluts[1][8] = brightColor;
            LoadImage(&this->clutRects[1], (u_long *)this->cluts[1]);
        }
    }

    tpage = allocDrTpage(10);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 20;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3D68;
    sprt->w = 120;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 180;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3D28;
    sprt->w = 120;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    this->unk68++;
    /* The match depends on the destructor taking the printer through a variable set here: gcse then reuses
     * the first &printer in a second register (fp), as g++ passes a local's address to its destructor. */
    base = &printer.printer;
    printerDestroy(base, DESTROY_BASES);
}

/* Constructs an Unk800D1F8C (vtable D_8006E064) of owner: copies the 16-color palettes at x 640, y 250 and 248 of
 * VRAM. */
Unk800D1F8C *func_800D23C4(Unk800D1F8C *this, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E064;
    this->owner = owner;
    this->unk68 = 0;
    this->clutRects[0].x = 0x280;
    this->clutRects[0].y = 0xFA;
    this->clutRects[0].w = 0x10;
    this->clutRects[0].h = 1;
    StoreImage(&this->clutRects[0], (u_long *)this->cluts[0]);
    this->clutRects[1].x = 0x280;
    this->clutRects[1].y = 0xF8;
    this->clutRects[1].w = 0x10;
    this->clutRects[1].h = 1;
    StoreImage(&this->clutRects[1], (u_long *)this->cluts[1]);
    return this;
}

/* The destructor of the entity class of vtable D_8006E064, which func_800D23C4 installs. */
void func_800D246C(Entity *this, s32 flags) {
    this->vtable = &D_8006E064;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DFD4);

/* The update of D_8006E064's entity: unless the owner's 0xF6A is set, prints the two sides' 0xF30 counts
 * (shadowed), pulses entries 7 and 8 of each palette (the side's count still 0) and draws two 96 by 24
 * sprites of the palettes. */
void func_800D24C0(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->unkF6A != 0) {
        return;
    }
    ot = camera->ot.head;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 90;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[0]);
    printer.printer.x = 250;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 89;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[0]);
    printer.printer.x = 249;
    printer.printer.y = 20;
    printerPrint(&printer.printer, D_8006DFD4, paletteOwner(this)->unkF30[1]);

    /* The match depends on green and blue in their own variables (the bright color adds 4 to each). */
    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->unkF30[0] == 0) {
        this->cluts[0][7] = color;
        this->cluts[0][8] = brightColor;
        LoadImage(&this->clutRects[0], (u_long *)this->cluts[0]);
    }
    if (paletteOwner(this)->unkF30[1] == 0) {
        this->cluts[1][7] = color;
        this->cluts[1][8] = brightColor;
        LoadImage(&this->clutRects[1], (u_long *)this->cluts[1]);
    }

    tpage = allocDrTpage(10);
    /* The match depends on this field order, which sets the sprite registers. */
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 32;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3EA8;
    sprt->w = 96;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = 192;
    sprt->y0 = 16;
    sprt->u0 = 0x50;
    sprt->v0 = 0;
    sprt->clut = 0x3E28;
    sprt->w = 96;
    sprt->h = 24;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    this->unk68++;
    base = &printer.printer; /* as in func_800D2088 */
    printerDestroy(base, DESTROY_BASES);
}

/* Constructs an entity of the class of virtual table D_8006E0DC, for func_8006ED20's object. */
Unk800D1328 *func_800D27E0(Unk800D1328 *this, Unk8006ED20 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006E0DC;
    this->unk18 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006E0DC, which func_800D27E0 installs. */
void func_800D2828(Entity *this, s32 flags) {
    this->vtable = &D_8006E0DC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D287C);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D29C8);

/* The destructor of the entity class of vtable D_8006E03C, which func_800D29C8 installs. */
void func_800D2B94(Entity *this, s32 flags) {
    this->vtable = &D_8006E03C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D2BE8);

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DFE4);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D2C64);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D2E98);

/* The destructor of the entity class of vtable D_8006E014, which func_800D2E98 installs. */
void func_800D2F8C(Entity *this, s32 flags) {
    this->vtable = &D_8006E014;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D2FE0(void) {
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DFF4);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D2FE8);

/* Returns entry index of the array at 0xD98; no caller found. */
s16 func_800D326C(Unk800D3284 *this, u16 index) {
    return this->unkD98[index];
}

/* Sets entry index of the array at 0xF2C; no caller found. */
void func_800D3284(Unk800D3284 *this, s32 index, s32 value) {
    this->unkF2C[index] = value;
}

/* Sets entry index of the array at 0xF30; no caller found. */
void func_800D3294(Unk800D3284 *this, s32 index, s32 value) {
    this->unkF30[index] = value;
}

/* Sets 0xF34; nothing calls it. */
void func_800D32A4(Unk800D3284 *this, s32 value) {
    this->unkF34 = value;
}

/* Sets 0xF36; nothing calls it. */
void func_800D32AC(Unk800D3284 *this, s32 value) {
    this->unkF36 = value;
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E014);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E03C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E064);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E08C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E0B4);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E0DC);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E104);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E12C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E154);
