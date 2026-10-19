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
#include "stdio.h"

extern EntityVtable D_8006E014;
extern EntityVtable CONFETTI_VTABLE;
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

/* The confetti at the winner's position (Confetti_init, virtual table CONFETTI_VTABLE, 0x12E0 bytes): 128 spinning
 * triangles of 4 colors that fall from 0x400 above the ground and start again from the top at the winner's x. */
typedef struct Confetti {
    /* 0x0000 */ Entity entity;
    /* 0x0018 */ Fight *scene;
    /* 0x001C */ Vec3 origin;          /* the winner's position */
    /* 0x0028 */ u8 unk28[0x30 - 0x28];
    /* 0x0030 */ s32 angles[128];      /* random, turned by 0x40 a frame */
    /* 0x0230 */ u8 colors[128];       /* 0-3, random */
    /* 0x02B0 */ VECTOR positions[128];
    /* 0x0AB0 */ VECTOR velocities[128];
    /* 0x12B0 */ SVECTOR shape[6];     /* the triangle's corners */
} Confetti; /* size 0x12E0 */

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

void BonusFight_spawnModeHud(BonusFight *this, s32 mode);
void BonusFight_spawnTimer(Fight *this);
void func_800D0F7C(BonusFight *this);

/* Starts a bonus game's round: the arena's bounds, the fighters' start positions, the mode's HUD and timer, the
 * clock (3000 frames, 900 in the target game), and a full gauge for the fighters that evolve. */
void BonusFight_startRound(BonusFight *this) {
    Fighter *fighters[2];
    s32 i;

    Fight_startRound(&this->base);
    this->holding[0] = 0;
    this->holding[1] = 0;
    this->holdPos[0].vx = 0;
    this->holdPos[0].vy = 0;
    this->holdPos[0].vz = 0;
    this->holdPos[0].pad = 0;
    this->holdPos[1].vx = 0;
    this->holdPos[1].vy = 0;
    this->holdPos[1].vz = 0;
    this->holdPos[1].pad = 0;
    Fight_setBounds(&this->base, -0xABE, 0xABE, -0xC80, 0x400, -0x200);
    this->meshScene = Fight_getMeshScene(&this->base);
    fighters[0] = Fight_getFighter(&this->base, 0);
    fighters[1] = Fight_getFighter(&this->base, 1);
    Fighter_getPosition(fighters[0], (Vec3 *)&this->startPos[0]);
    Fighter_getPosition(fighters[1], (Vec3 *)&this->startPos[1]);
    this->base.hasBackground = 0;
    this->hudHidden = 0;
    this->endWait = 0;
    this->gauges[1] = 0;
    this->gauges[0] = 0;
    this->counts[1] = 0;
    this->counts[0] = 0;
    this->base.modeFlags |= FIGHT_MODE_BONUS;
    func_800D0F7C(this);
    BonusFight_spawnTimer(&this->base);
    BonusFight_spawnModeHud(this, (u16)(this->base.arena - ARENA_BONUS)); /* the bonus game */
    this->result = BONUS_RESULT_PLAYING;
    this->resultFrames = 0;
    this->messageHandle = 0;
    switch (this->base.arena) {
    case ARENA_BONUS + BONUS_GAME_BASKETBALL:
    case ARENA_BONUS + BONUS_GAME_EVOLUTION_RACE:
        this->base.timeLeft = 3000;
        break;
    case ARENA_BONUS + BONUS_GAME_TARGET:
        this->base.timeLeft = 900;
        break;
    }
    for (i = 0; i < 2; i++) {
        if (FIGHTER_PARAMS[Fighter_getCharacter(fighters[i])].flags & FIGHTER_PARAM_EVOLVED) {
            Fighter_addGauge(fighters[i], 0x10000);
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0k", BonusFight_tick);

void func_800D110C(BonusFight *this);

/* The bonus fight's own part of a frame (FIGHT_VTABLE's tickScenery): a light grey backdrop, the base's step and
 * func_800D110C; once there is a result the clock holds at 1 and, after resultDelay frames, the result runs: a win
 * stops the round when its message is gone (and then goes on as a draw), a draw goes to the ending when its message
 * is gone, the ending waits 60 frames, then the round stops. */
void BonusFight_tickScenery(BonusFight *this) {
    displaySetClearColor(CURRENT_DISPLAY, 0xF0, 0xF0, 0xF0);
    Fight_tickScenery(&this->base);
    func_800D110C(this);
    if (this->result == BONUS_RESULT_PLAYING) {
        return;
    }
    this->base.timeLeft = 1;
    if (++this->resultFrames < this->resultDelay) {
        return;
    }
    switch (this->result) {
    case BONUS_RESULT_WON:
        if (getEntityByHandle(this->messageHandle) == NULL) {
            Fight_stopRound(&this->base);
            this->result = BONUS_RESULT_OVER;
        }
        /* fall through */
    case BONUS_RESULT_DRAW:
        if (getEntityByHandle(this->messageHandle) == NULL) {
            this->result = BONUS_RESULT_ENDING;
        }
        /* fall through */
    case BONUS_RESULT_ENDING:
        if (++this->endWait < 60) {
            break;
        }
        this->result = BONUS_RESULT_OVER;
        /* fall through */
    case BONUS_RESULT_OVER:
        Fight_stopRound(&this->base);
        break;
    }
}

/* Before the next round (slot 0x38): stops the win jingle. */
void BonusFight_prepareNextRound(BonusFight *this) {
    stopSound(this->jingleHandle);
}

Confetti *Confetti_init(Confetti *this, Fight *scene, Vec3 *origin);
Entity *bonusShowWinner(Fight *fight, s32 side);

/* Ends the bonus game with a winner: the winner's win pose (func_80079268 on the loser), confetti at its position,
 * its win message (bonusShowWinner) whose handle it keeps, the fighters frozen and the clock held, the result set,
 * the arena's bonus game unlocked (USA: and the winner's bonus added to its score), the camera on the winner, and
 * the arena's win jingle. */
void BonusFight_endWithWinner(BonusFight *this, Fighter *winner, Fighter *loser) {
    Vec3 position;
    s32 side;

    Fighter_requestWinPose(winner);
    func_80079268(loser);
    side = Fighter_getSide(winner);
    Fight_addStageEntity(&this->base,
                         NEW_ENTITY(sizeof(Confetti), Confetti_init, &this->base, Fighter_getPosition(winner, &position)));
    this->messageHandle = entityGetHandle(bonusShowWinner(&this->base, side));
    Fight_freezeFighters(&this->base);
    Fight_holdClock(&this->base);
    this->result = BONUS_RESULT_WON;
    unlockBonusGame(this->base.arena);
#if !VERSION_JP
    Fight_addScore(&this->base, side, Fight_computeBonus(&this->base, side));
#endif
    if (Fighter_getSide(winner) == 0) {
        this->base.cameraFocus = 1;
    } else {
        this->base.cameraFocus = 2;
    }
    switch (this->base.arena) {
    case ARENA_BONUS:
        this->jingleHandle = soundControlPlaySound(&SOUND_CONTROL, 0x6D, 0);
        break;
    case ARENA_BONUS + 1:
        this->jingleHandle = soundControlPlaySound(&SOUND_CONTROL, 0x6F, 0);
        break;
    case ARENA_BONUS + 2:
        this->jingleHandle = soundControlPlaySound(&SOUND_CONTROL, 0x71, 0);
        break;
    }
}

/* Shows the bonus game's winner: YOU WIN to a human against the computer, YOU LOSE when the computer beat a human
 * (with sound 0x8009 instead of 0x8008), else P1/P2 WIN; a human winner also gets the score tally. Returns the
 * tally, or the message for a computer winner. */
Entity *bonusShowWinner(Fight *fight, s32 side) {
    s32 otherComputer = GAME_STATE.fighters[side ^ 1].computer;
    s32 computer = GAME_STATE.fighters[side].computer;
    s32 kind;
    Entity *entity;

    if (!computer && otherComputer) {
        kind = BATTLE_MESSAGE_YOU_WIN;
        soundControlPlaySound(&SOUND_CONTROL, 0x8008, 0);
    } else if (computer && !otherComputer) {
        kind = BATTLE_MESSAGE_YOU_LOSE;
        soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    } else {
        kind = BATTLE_MESSAGE_P1_WIN + side;
        soundControlPlaySound(&SOUND_CONTROL, 0x8008, 0);
    }
    entity = BattleMessage_init(entityOperatorNew(0x59C), kind, 0);
    Fight_addEntity(fight, entity);
    if (!computer) {
        entity = ScoreTally_init(entityOperatorNew(0x64), fight, side);
        Fight_addEntity(fight, entity);
    }
    return entity;
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DF5C);

extern s16 D_8006DF5C[]; /* by character: the clip BonusFight_holdFighter plays (5 for every one) */

/* Holds the side's fighter where it stands (BonusFight_tick keeps it at holdPos until the clip ends) and plays its
 * character's clip of D_8006DF5C once. */
void BonusFight_holdFighter(BonusFight *this, u16 side) {
    Fighter *fighters[2];
    s32 character;

    fighters[0] = Fight_getFighter(&this->base, 0);
    fighters[1] = Fight_getFighter(&this->base, 1);
    character = Fighter_getCharacter(fighters[side]);
    /* holdPos is a VECTOR; the position fills its first three words */
    Fighter_getPosition(fighters[side], (Vec3 *)&this->holdPos[side]);
    Fighter_playActionClip(fighters[side], D_8006DF5C[character], 0, 0, -1);
    this->holding[side] = 1;
}

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

/* A TextPrinter's func_80032900, inlined as the game has it (its virtual table read from the printer), like
 * TEXT_SET_COLOR. */
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

/* Draws the bonus game's seconds left (timeLeft / 30, 0 to 99) printed "%02d" as two 12x16 digits of the HUD
 * texture's row at v 0x58, from (148, 203); nothing while the bonus HUD is hidden. */
void BonusTimer_draw(Unk800D1328 *this, Camera *camera) {
    u8 text[8];
    BonusFight *fight = (BonusFight *)this->unk18;
    u_long *ot;
    DR_TPAGE *mode;
    s32 seconds;
    s32 x;
    s32 i;

    if (fight->hudHidden) {
        return;
    }
    ot = camera->ot.slots;
    seconds = fight->base.timeLeft / 30;
    x = 148;
    if (seconds > 99) {
        seconds = 99;
    }
    if (seconds < 0) {
        seconds = 0;
    }
    sprintf((char *)text, "%02d", seconds);
    mode = allocDrTpage(15);
    for (i = 0; i < 2; i++) {
        s32 u = (text[i] - '0') * 12;
        SPRT *sprt = allocSprt();

        sprt->r0 = 0x80;
        sprt->g0 = 0x80;
        sprt->b0 = 0x80;
        sprt->x0 = x;
        sprt->y0 = 203;
        sprt->u0 = u;
        sprt->v0 = 0x58;
        sprt->clut = getClut(0x3C0, 0x7B);
        sprt->w = 12;
        sprt->h = 16;
        AddPrim(ot, sprt);
        x += 12;
    }
    AddPrim(ot, mode);
}

INCLUDE_ASM("game/nonmatchings/game_0k", Confetti_init);

/* Destroys the confetti. */
void Confetti_destroy(Entity *this, s32 flags) {
    this->vtable = &CONFETTI_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The confetti's tick: every piece falls by its velocity and starts again at the top (0x400 above the ground, at
 * the winner's x) once it passes the ground, and turns by 0x40. */
void Confetti_tick(Confetti *this) {
    s32 i;

    for (i = 0; i < 128; i++) {
        VECTOR *position = &this->positions[i];
        /* The match depends on the position read into locals before the velocity is added (all loads first). */
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        x += this->velocities[i].vx;
        y += this->velocities[i].vy;
        z += this->velocities[i].vz;

        position->vx = x;
        position->vy = y;
        position->vz = z;
        if (y > 0) {
            position->vx = this->origin.vx;
            position->vy = -0x400;
            position->vz = 0;
        }
        this->angles[i] += 0x40;
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006DFE4);

INCLUDE_ASM("game/nonmatchings/game_0k", Confetti_draw);

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
INCLUDE_RODATA("game/nonmatchings/game_0k", CONFETTI_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E064);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E08C);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E0B4);
INCLUDE_RODATA("game/nonmatchings/game_0k", BONUS_TIMER_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E104);
INCLUDE_RODATA("game/nonmatchings/game_0k", D_8006E12C);
INCLUDE_RODATA("game/nonmatchings/game_0k", BONUS_FIGHT_VTABLE);
