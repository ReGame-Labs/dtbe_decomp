/* The bonus games' fight (BonusFight, arenas 7 to 9), derived from Fight: its rounds, the winner and draw ends, and
 * the HUD of each mode (gauges, marks, score boards, the timer) with the figures it prints (%06d, %03d, %02d). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
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
extern EntityVtable BONUS_TIMER_VTABLE;
extern EntityVtable D_8006E104;
extern EntityVtable D_8006E12C;
extern VtableEntry BONUS_FIGHT_VTABLE[];
extern u8 STR_FORMAT_6D[];
extern u8 STR_FORMAT_3D[]; /* "%03d" */

/* The entity classes that BonusFight_spawnModeHud and BonusFight_spawnTimer make for
 * the fight: D_8006E12C, D_8006E104, BONUS_TIMER_VTABLE, and D_8006E0B4 with a
 * field more. */
typedef struct Unk800D1328 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *unk18;
} Unk800D1328;
typedef struct Unk800D1C50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *unk18;
    /* 0x1C */ s16 unk1C;
} Unk800D1C50;

/* Constructs the bonus fight on Fight. */
BonusFight *BonusFight_init(BonusFight *this) {
    Fight_init(&this->base);
    this->base.vtable = BONUS_FIGHT_VTABLE;
    return this;
}

/* Destroys the bonus fight: clears the screen to black, stops the win jingle, then destroys the Fight part. */
void BonusFight_destroy(BonusFight *this, s32 flags) {
    this->base.vtable = BONUS_FIGHT_VTABLE;
    displaySetClearColor(CURRENT_DISPLAY, 0, 0, 0);
    stopSound(this->jingleHandle);
    Fight_destroy(&this->base, flags);
}

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_startRound);

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_tick);

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_tickScenery);

/* Before the next round (slot 0x38): stops the win jingle. */
void BonusFight_prepareNextRound(BonusFight *this) {
    stopSound(this->jingleHandle);
}

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_endWithWinner);

INCLUDE_ASM("game/nonmatchings/game_0k", bonusShowWinner);

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DF5C);

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_holdFighter);

/* Ends the bonus game in a draw, as Fight_playDraw without its wait: plays sounds 0x8009 and 0x42, runs
 * func_80079268 on both sides' fighters, shows the DRAW GAME message and keeps its handle, freezes the fighters,
 * holds the clock and sets the result. */
void BonusFight_endInDraw(BonusFight *this) {
    Fighter *fighter0;
    Fighter *fighter1;

    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    fighter0 = Fight_getFighter(&this->base, 0);
    fighter1 = Fight_getFighter(&this->base, 1);
    func_80079268(fighter0);
    func_80079268(fighter1);
    this->messageHandle = entityGetHandle(
        Fight_addEntity(&this->base, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_DRAW_GAME, 0)));
    Fight_freezeFighters(&this->base);
    Fight_holdClock(&this->base);
    this->result = BONUS_RESULT_DRAW;
}

/* Sets the side's next mark and counts it (st07). */
void BonusFight_addMark(BonusFight *this, s16 side) {
    this->marks[side][this->markCounts[side]] = 1;
    this->markCounts[side]++;
}

/* Adds points plus the fighter's side's bonus score to the fighter's score (Fighter_addScore); nothing calls it. */
void func_800D0C9C(BonusFight *this, Fighter *fighter, u16 points) {
    Fighter_addScore(fighter, this->scores[(u16)Fighter_getSide(fighter)] + points);
}

Unk800D1328 *func_800D1328(Unk800D1328 *this, Fight *arg);
Unk800D1328 *func_800D17E4(Unk800D1328 *this, Fight *arg);
Unk800D1C50 *func_800D1C50(Unk800D1C50 *this, Fight *arg);
Unk800D1F8C *func_800D1F8C(Unk800D1F8C *this, Fight *owner);
Unk800D1F8C *func_800D23C4(Unk800D1F8C *this, Fight *owner);

/* Clears both sides' gauges, marks and scores, then adds the HUD entities of the mode, the bonus game (the arena
 * less ARENA_BONUS): st07's the gauges of D_8006E12C and the marks of D_8006E0B4, st08's the count gauges of
 * D_8006E104 and the count board of func_800D23C4, st09's the gauges of D_8006E12C and the score board of
 * func_800D1F8C. */
void BonusFight_spawnModeHud(BonusFight *this, s32 mode) {
    Entity *entity;
    s32 side;
    s32 i;

    for (side = 0; side < 2; side++) {
        this->gauges[side] = 0;
        this->markCounts[side] = 0;
        this->scores[side] = 0;
    }
    for (side = 0; side < 2; side++) {
        for (i = 0; i < 10; i++) {
            this->marks[side][i] = 0;
        }
    }
    switch (mode) {
    case BONUS_GAME_BASKETBALL:
        entity = NEW_ENTITY(0x1C, func_800D1328, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        entity = NEW_ENTITY(0x20, func_800D1C50, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        break;
    case BONUS_GAME_EVOLUTION_RACE:
        entity = NEW_ENTITY(0x1C, func_800D17E4, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        entity = NEW_ENTITY(0x70, func_800D23C4, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        break;
    case BONUS_GAME_TARGET:
        entity = NEW_ENTITY(0x1C, func_800D1328, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        entity = NEW_ENTITY(0x70, func_800D1F8C, &this->base);
        if (entity != NULL) {
            Fight_addEntity(&this->base, entity);
        }
        break;
    }
}

Unk800D1328 *BonusTimer_init(Unk800D1328 *this, Fight *fight);

/* Adds the bonus game's timer (BonusTimer) to the fight, when it was allocated. */
void BonusFight_spawnTimer(Fight *this) {
    Entity *entity = NEW_ENTITY(0x1C, BonusTimer_init, this);

    if (entity != NULL) {
        Fight_addEntity(this, entity);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D0F7C);

INCLUDE_ASM("game/nonmatchings/game_0k", func_800D110C);

/* Constructs an entity of the class of virtual table D_8006E12C, for the fight. */
Unk800D1328 *func_800D1328(Unk800D1328 *this, Fight *arg) {
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

/* Constructs an entity of the class of virtual table D_8006E104, for the fight. */
Unk800D1328 *func_800D17E4(Unk800D1328 *this, Fight *arg) {
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

/* Constructs an entity of the class of virtual table D_8006E0B4, for the fight. */
Unk800D1C50 *func_800D1C50(Unk800D1C50 *this, Fight *arg) {
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
Unk800D1F8C *func_800D1F8C(Unk800D1F8C *this, Fight *owner) {
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

/* The owner of a score board, as the BonusFight it is (game_classes.h types it Fight). */
static inline BonusFight *paletteOwner(Unk800D1F8C *this) {
    return (BonusFight *)this->owner;
}

/* A TextPrinter's virtual calls, inlined as the game has them: printerSetColor and func_80032900 on the
 * printer object itself (its virtual table read from the stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)
#define TEXT_SET_UNK18(printer, value) \
    (printer).printer.vtable->unk18.func((u8 *)&(printer) + (printer).printer.vtable->unk18.delta, value)

INCLUDE_RODATA("game/nonmatchings/game_0k", STR_FORMAT_6D);

/* Draws mode 2's (st09) score board: unless the HUD is hidden, prints the two sides' scores (shadowed); once it is
 * hidden, pulses entries 7 and 8 of the leading side's palette; then draws two 120 by 24 sprites of the
 * palettes. */
void func_800D2088(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->hudHidden != 0) {
        return;
    }
    ot = camera->ot.slots;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 77;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_6D, paletteOwner(this)->scores[0]);
    printer.printer.x = 236;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_6D, paletteOwner(this)->scores[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 78;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_6D, paletteOwner(this)->scores[0]);
    printer.printer.x = 237;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_6D, paletteOwner(this)->scores[1]);

    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->hudHidden != 0) {
        /* The match depends on the operand order of both compares (scores[0] loads first, then scores[1]). */
        if (paletteOwner(this)->scores[0] > paletteOwner(this)->scores[1]) {
            this->cluts[0][7] = color;
            this->cluts[0][8] = brightColor;
            LoadImage(&this->clutRects[0], (u_long *)this->cluts[0]);
        }
        if (paletteOwner(this)->scores[1] > paletteOwner(this)->scores[0]) {
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
Unk800D1F8C *func_800D23C4(Unk800D1F8C *this, Fight *owner) {
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

INCLUDE_RODATA("game/nonmatchings/game_0k", STR_FORMAT_3D);

/* Draws mode 1's (st08) count board: unless the HUD is hidden, prints the two sides' counts (shadowed), pulses
 * entries 7 and 8 of the palette of a side whose count is 0, and draws two 96 by 24 sprites of the palettes. */
void func_800D24C0(Unk800D1F8C *this, Camera *camera) {
    TextPrinter printer;
    u_long *ot;
    u32 level, green, blue; /* a grey pulsing with the counter at 0x68: red, green and blue */
    u32 color, brightColor;
    DR_TPAGE *tpage;
    SPRT *sprt;
    Printer *base; /* the printer for its destructor */

    if (paletteOwner(this)->hudHidden != 0) {
        return;
    }
    ot = camera->ot.head;
    textPrinterInit(&printer, ot);
    TEXT_SET_COLOR(printer, 0x6060FF);
    TEXT_SET_UNK18(printer, 16);
    TEXT_SET_COLOR(printer, 0x606080);
    printer.printer.x = 90;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_3D, paletteOwner(this)->counts[0]);
    printer.printer.x = 250;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_3D, paletteOwner(this)->counts[1]);
    TEXT_SET_COLOR(printer, 0x80);
    printer.printer.x = 89;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_3D, paletteOwner(this)->counts[0]);
    printer.printer.x = 249;
    printer.printer.y = 20;
    printerPrint(&printer.printer, STR_FORMAT_3D, paletteOwner(this)->counts[1]);

    /* The match depends on green and blue in their own variables (the bright color adds 4 to each). */
    level = (u8)((COS_SIN_TABLE[((u16)this->unk68 << 7) & 0xFFF].sin >> 9) + 23);
    green = level >> 1;
    blue = level >> 2;
    color = level | (green << 5) | (blue << 10);
    brightColor = level | ((green + 4) << 5) | ((blue + 4) << 10);
    if (paletteOwner(this)->counts[0] == 0) {
        this->cluts[0][7] = color;
        this->cluts[0][8] = brightColor;
        LoadImage(&this->clutRects[0], (u_long *)this->cluts[0]);
    }
    if (paletteOwner(this)->counts[1] == 0) {
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

/* Constructs the bonus game's timer, which prints the fight's seconds left. */
Unk800D1328 *BonusTimer_init(Unk800D1328 *this, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &BONUS_TIMER_VTABLE;
    this->unk18 = fight;
    return this;
}

/* Destroys the bonus game's timer. */
void BonusTimer_destroy(Entity *this, s32 flags) {
    this->vtable = &BONUS_TIMER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", BonusTimer_draw);

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

/* Returns whether the side's fighter is held; no caller found. */
s16 func_800D326C(BonusFight *this, u16 side) {
    return this->holding[side];
}

/* Sets the side's gauge; no caller found (st09 inlines it). */
void func_800D3284(BonusFight *this, s32 side, s32 value) {
    this->gauges[side] = value;
}

/* Sets the side's count; no caller found. */
void func_800D3294(BonusFight *this, s32 side, s32 value) {
    this->counts[side] = value;
}

/* Sets the gauges' maximum; nothing calls it. */
void func_800D32A4(BonusFight *this, s32 value) {
    this->gaugeMax = value;
}

/* Sets the counts' maximum; nothing calls it. */
void func_800D32AC(BonusFight *this, s32 value) {
    this->countMax = value;
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E014);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E03C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E064);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E08C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E0B4);
INCLUDE_RODATA("game/nonmatchings/game_0k", BONUS_TIMER_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E104);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E12C);
INCLUDE_RODATA("game/nonmatchings/game_0k", BONUS_FIGHT_VTABLE);
