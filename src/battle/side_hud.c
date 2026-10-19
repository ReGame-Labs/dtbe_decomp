/* A side's HUD at the bottom of the screen: the life and evolution gauges, the score, the timer, the round wins,
 * the hit counter and the portrait; drawn on the left and mirrored on the right. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/sin_cos.h"
#include "engine/text/console.h"

/* A gauge of the side HUD (its life and its evolution gauge), each set up by sideGaugeInit with 0x10000 at 0xC. */
typedef struct SideGauge {
    /* 0x0 */ s32 value;  /* 0 to 0x10000 for all of it */
    /* 0x4 */ s32 target; /* the length the value gives */
    /* 0x8 */ s32 shown;  /* the length drawn, a step a frame towards target */
    /* 0xC */ s32 unkC;
} SideGauge;

/* The side HUD's combo counter: shown when the hit count rises to 2 or more, fading with its timer. */
typedef struct HitCounter {
    /* 0x0 */ s32 value; /* the count shown */
    /* 0x4 */ s32 last;  /* the count of the last frame */
    /* 0x8 */ s32 timer; /* 0x200 when shown, down to 0 */
} HitCounter;

/* A side's HUD (SideHud_init, made twice by Fight): its tick reads the side each frame, its draw draws it. */
typedef struct SideHud {
    /* 0x00 */ Entity entity;
    /* 0x18 */ SideGauge life;           /* the side's life, 120 px */
    /* 0x28 */ SideGauge evolutionGauge; /* the side's gauge, 70 px */
    /* 0x38 */ BattleSide *owner;
    /* 0x3C */ s32 character;         /* Fighter_getCharacter of the side's current fighter */
    /* 0x40 */ s32 lastCharacter;     /* the character of the last frame */
    /* 0x44 */ s32 previousCharacter; /* the one before the last change */
    /* 0x48 */ s32 unk48;             /* SideHud_init's 4th argument */
    /* 0x4C */ s32 changeFlash;       /* 0x80 when the character changes, -8 a frame */
    /* 0x50 */ s32 side; /* 0: SideHud_drawLeft draws it, else SideHud_drawRight (SideHud_draw) */
    /* 0x54 */ s32 timeLeft;  /* the fight's time (0xD28) */
    /* 0x58 */ s32 score;     /* Fight_getScore */
    /* 0x5C */ s32 roundWins; /* Fight_getWins */
    /* 0x60 */ s32 frame;     /* counted every tick */
    /* 0x64 */ HitCounter hitCounter;
    /* 0x70 */ s32 visible; /* !Fight_isRoundOver; SideHud_draw draws nothing while 0 */
    /* 0x74 */ s32 pulse;   /* 0x80 to 0x100 by 8 */
} SideHud;

extern EntityVtable SIDE_HUD_VTABLE;

void sideGaugeInit(SideGauge *this, s32 value, s32 unkC);
void hitCounterClear(HitCounter *counter);
void sideGaugeUpdate(SideGauge *this, s32 length);
void hitCounterTick(HitCounter *counter);
void hitCounterSet(HitCounter *counter, s32 count);

/* Inline copies of game_0e.c's BattleSide getters (BattleSide_getLife, _getLifeMax, _getGauge: no callers, the
 * out-of-line copies of inline methods). */
static inline s32 getLifeInline(BattleSide *side) {
    return side->life;
}

static inline s32 getLifeMaxInline(BattleSide *side) {
    return side->lifeMax;
}

static inline s32 getGaugeInline(BattleSide *side) {
    return side->gauge;
}

/* Constructs the HUD of owner's side: its gauges from the side's life (unkC the life's maximum, 0x10000 at most)
 * and evolution gauge, the current fighter's character, a cleared hit counter, shown, pulse 0x80.
 * The match depends on the life read through the getter inline: a call in the arguments is evaluated first, so
 * the life loads before the maximum's test (the field read loads it after: 2 diffs). */
SideHud *SideHud_init(SideHud *this, BattleSide *owner, s32 side, s32 unk48) {
    Fighter *fighter;
    s32 lifeMax;
    s32 character;

    entityInit(&this->entity, 0);
    this->entity.vtable = &SIDE_HUD_VTABLE;
    fighter = owner->current;
    lifeMax = getLifeMaxInline(owner);
    sideGaugeInit(&this->life, getLifeInline(owner), lifeMax > 0x10000 ? 0x10000 : lifeMax);
    sideGaugeInit(&this->evolutionGauge, getGaugeInline(owner), 0x10000);
    this->owner = owner;
    character = Fighter_getCharacter(fighter);
    this->previousCharacter = character;
    this->lastCharacter = character;
    this->character = character;
    this->unk48 = unk48;
    this->changeFlash = 0;
    this->side = side;
    this->timeLeft = 0;
    this->score = 0;
    this->roundWins = 0;
    this->frame = 0;
    hitCounterClear(&this->hitCounter);
    this->visible = 1;
    this->pulse = 0x80;
    return this;
}

/* Sets the life and the evolution gauge to the side's and steps their lengths (120 and 70 px).
 * The match depends on the gauges passed as pointers: the game keeps the evolution gauge's in a saved register. */
static inline void updateGauges(SideGauge *life, SideGauge *evolutionGauge, BattleSide *owner) {
    life->value = getLifeInline(owner);
    evolutionGauge->value = getGaugeInline(owner);
    sideGaugeUpdate(life, 120);
    sideGaugeUpdate(evolutionGauge, 70);
}

/* Reads the side each frame: the current fighter's character (a change starts the portrait's flash), the gauges,
 * the time, the score, the round wins and the hit count, whether the round goes on, the pulse and the frame. */
void SideHud_tick(SideHud *this) {
    Fighter *fighter = this->owner->current;
    Fight *fight = CURRENT_FIGHT;

    this->character = Fighter_getCharacter(fighter);
    if (this->character != this->lastCharacter) {
        this->previousCharacter = this->lastCharacter;
        this->changeFlash = 0x80;
    }
    this->lastCharacter = this->character;
    if (this->changeFlash > 0) {
        this->changeFlash -= 8;
    }
    updateGauges(&this->life, &this->evolutionGauge, this->owner);
    this->timeLeft = fight->timeLeft;
    this->score = Fight_getScore(fight, this->side);
    this->roundWins = Fight_getWins(fight, this->side);
    hitCounterSet(&this->hitCounter, Fighter_getComboHits(fighter));
    hitCounterTick(&this->hitCounter);
    this->visible = Fight_isRoundOver(fight) ^ 1;
    this->pulse += 8;
    if (this->pulse >= 0x100) {
        this->pulse = 0x80;
    }
    this->frame++;
}

/* Sets up a gauge with its value and unkC, its lengths at 0. */
void sideGaugeInit(SideGauge *this, s32 value, s32 unkC) {
    this->value = value;
    this->target = 0;
    this->shown = 0;
    this->unkC = unkC;
}

/* Sets the target to the gauge's full length scaled by the value (0x10000 for all of it), and moves the shown length
 * a step towards it; while the value is positive, both are 2 at least. */
void sideGaugeUpdate(SideGauge *this, s32 length) {
    this->target = length * this->value / 0x10000;
    if (this->shown != this->target) {
        s32 shown;

        if (this->shown < this->target) {
            shown = this->shown + 1;
        } else {
            shown = this->shown - 1;
        }
        this->shown = shown;
    }
    if (this->value > 0) {
        if (this->target < 2) {
            this->target = 2;
        }
        if (this->shown < 2) {
            this->shown = 2;
        }
    }
}

/* Clears the hit counter. */
void hitCounterClear(HitCounter *counter) {
    counter->value = 0;
    counter->last = 0;
    counter->timer = 0;
}

/* Runs the hit counter's timer down to 0: by an eighth above 0x100, by 4 below. The match depends on the step
 * stored through a pointer to the timer (the game computes its address before the test) and kept in a variable of
 * its own. */
void hitCounterTick(HitCounter *counter) {
    s32 *timer = &counter->timer;
    s32 value = *timer;
    s32 next;

    if (value > 0x100) {
        next = value - (value >> 3);
    } else {
        next = value - 4;
    }
    *timer = next;
    if (counter->timer <= 0) {
        counter->timer = 0;
    }
}

/* Takes the side's hit count of this frame: a count of 2 or more above the last one is shown (value) and restarts the
 * timer at 0x200. */
void hitCounterSet(HitCounter *counter, s32 count) {
    if (count >= 2 && counter->last < count) {
        counter->timer = 0x200;
        counter->value = count;
    }
    counter->last = count;
}

INCLUDE_ASM("game/nonmatchings/side_hud", hitCounterDraw);

void SideHud_drawLeft(SideHud *this, u_long *ot);
void SideHud_drawRight(SideHud *this, u_long *ot);
void func_80074A88(SideHud *this, Camera *camera);

/* Draws the side's HUD after the last slot of the camera's ordering table, the left or the mirrored right layout,
 * then its fighter's marker (func_80074A88) while it is still visible. */
void SideHud_draw(SideHud *this, Camera *camera) {
    u_long *ot;

    if (this->visible) {
        ot = camera->ot.lastDrawn + 1;
        if (this->side == 0) {
            SideHud_drawLeft(this, ot);
        } else {
            SideHud_drawRight(this, ot);
        }
        if (this->visible) {
            func_80074A88(this, camera);
        }
    }
}

/* the parts of the HUD, each drawn at (x, y) */
void func_80073C80(SideHud *this, u_long *ot, s32 x, s32 y);
void SideHud_drawRoundWins(SideHud *this, u_long *ot, s32 x, s32 y);
void SideHud_drawTimer(SideHud *this, u_long *ot, s32 x, s32 y);
void SideHud_drawScore(SideHud *this, u_long *ot, s32 x, s32 y);
void SideHud_drawHealthGauge(SideHud *this, u_long *ot, s32 x, s32 y);
void SideHud_drawEvolutionGauge(SideHud *this, u_long *ot, s32 x, s32 y);
void func_800749BC(SideHud *this, u_long *ot, s32 x, s32 y);
void hitCounterDraw(HitCounter *counter, u_long *ot, s32 x, s32 y);

/* Draws the left side's HUD: the timer, the gauges, the round wins and the score, then the hit counter at x 0xC8
 * (0xBC from 10 up), y 0x30, and the portrait (func_80073C80). */
void SideHud_drawLeft(SideHud *this, u_long *ot) {
    HitCounter *counter;
    s32 x;

    SideHud_drawTimer(this, ot, 0x94, 0xCD);
    SideHud_drawHealthGauge(this, ot, 0xF, 0xD8);
    SideHud_drawEvolutionGauge(this, ot, 0x41, 0xCC);
    SideHud_drawRoundWins(this, ot, 0x76, 0xBE);
    SideHud_drawScore(this, ot, 0x43, 0xBE);
    func_800749BC(this, ot, 4, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    /* The match depends on the counter pointer: its address is computed before the test, as in the game. */
    counter = &this->hitCounter;
    if (counter->value < 10) {
        x = 0xC8;
    } else {
        x = 0xBC;
    }
    hitCounterDraw(counter, ot + 2, x, 0x30);
    func_80073C80(this, ot, 0xF, 0xBD);
}

/* Draws the right side's HUD: SideHud_drawLeft's parts but the timer, mirrored, and the hit counter at x 0x1C. */
void SideHud_drawRight(SideHud *this, u_long *ot) {
    SideHud_drawHealthGauge(this, ot, 0xB9, 0xD8);
    SideHud_drawEvolutionGauge(this, ot, 0xB9, 0xCC);
    SideHud_drawRoundWins(this, ot, 0xBF, 0xBE);
    SideHud_drawScore(this, ot, 0xCD, 0xBE);
    func_800749BC(this, ot, 0xA0, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    hitCounterDraw(&this->hitCounter, ot + 2, 0x1C, 0x30);
    func_80073C80(this, ot, 0x102, 0xBD);
}

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073C80);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073D78);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawRoundWins);

/* Draws the round's seconds left (timeLeft / 30, at most 99) at (x, y): two 12x16 digits of the HUD texture's row at
 * v 0x58. The match depends on the digits' x copied from x after the clamp (the game copies the parameter there),
 * and on each digit's u computed before allocSprt. */
void SideHud_drawTimer(SideHud *this, u_long *ot, s32 x, s32 y) {
    s32 digits[2];
    s32 seconds = this->timeLeft / 30;
    s32 digitX;
    s32 i;

    if (seconds > 99) {
        seconds = 99;
    }
    digitX = x;
    digits[0] = seconds / 10;
    digits[1] = seconds % 10;
    for (i = 0; i < 2; i++) {
        s32 u = digits[i] * 12;
        SPRT *sprt = allocSprt();

        sprt->r0 = 0x80;
        sprt->g0 = 0x80;
        sprt->b0 = 0x80;
        sprt->x0 = digitX;
        sprt->y0 = y;
        sprt->u0 = u;
        sprt->v0 = 0x58;
        sprt->clut = getClut(0x3C0, 0x7B);
        sprt->w = 12;
        sprt->h = 16;
        AddPrim(ot, sprt);
        digitX += 12;
    }
}

INCLUDE_RODATA("game/nonmatchings/side_hud", SIDE_HUD_DECIMAL_PLACES);

/* 100000 down to 1, then 0: the decimal places of the score (SideHud_drawScore). */
extern s32 SIDE_HUD_DECIMAL_PLACES[];

/* The smaller of a and b. */
static inline s32 min(s32 a, s32 b) {
    return a < b ? a : b;
}

/* Draws the score (at most 999999) at (x, y): six 8x10 digits of the HUD texture's row at v 0x6B, the leading zeros
 * semi-transparent. */
void SideHud_drawScore(SideHud *this, u_long *ot, s32 x, s32 y) {
    /* The match depends on the clamp going through min: the game clamps in a temporary that starts at 999999 and
     * copies it to score; a ternary in place starts from the score. */
    s32 score = min(this->score, 999999);
    s32 leading = 1;
    s32 *places = SIDE_HUD_DECIMAL_PLACES;
    s32 place;

    while ((place = *places++) != 0) {
        s32 u = 0;
        SPRT *sprt;

        while (score >= place) {
            leading = 0;
            score -= place;
            u += 8;
        }
        if (place == 1) {
            leading = 0;
        }
        sprt = allocSprt();
        sprt->r0 = 0x80;
        sprt->g0 = 0x80;
        sprt->b0 = 0x80;
        sprt->x0 = x;
        sprt->y0 = y;
        sprt->u0 = u;
        sprt->v0 = 0x6B;
        sprt->clut = getClut(0x3C0, 0x7B);
        sprt->w = 8;
        sprt->h = 10;
        setSemiTrans(sprt, leading);
        AddPrim(ot, sprt);
        x += 8;
    }
}

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065060);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065064);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawHealthGauge);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065074);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065078);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065088);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawEvolutionGauge);

/* The evolution gauge's glow (SideHud_drawEvolutionGauge), while the side's fighter can evolve: a semi-transparent
 * 40x32 sprite centred on (x, y), 0x46 further right for side 1, its size pulsing by a quarter with the sine
 * of the frame (a full turn every 8 frames). The match depends on the sizes written as size * sin >> 14. */
void SideHud_drawEvolutionGlow(SideHud *this, u_long *ot, s32 x, s32 y) {
    RECT rect;
    s32 sin;
    POLY_FT4 *poly;

    if (!Fighter_canEvolve(this->owner->current)) {
        return;
    }
    sin = COS_SIN_TABLE[(this->frame << 9) & 0xE00].sin;
    rect.w = ((sin * 40) >> 14) + 40;
    rect.h = ((sin * 32) >> 14) + 32;
    rect.x = x - rect.w / 2 + this->side * 0x46;
    rect.y = y - rect.h / 2;
    poly = allocPolyFT4();
    setRGB0(poly, 0x80, 0x80, 0x80);
    setXYWH(poly, rect.x, rect.y, rect.w, rect.h);
    setUVWH(poly, 0x80, 0x98, 0x27, 0x1F);
    poly->tpage = 0xF;
    poly->clut = 0x1FBC;
    setSemiTrans(poly, 1);
    AddPrim(ot, poly);
}

/* Draws the side's 156 x 43 semi-transparent backdrop at x, y: side 0's image at u 4, v 0 of clut row 0x78, side
 * 1's at u 0, v 0x2C of row 0x79. */
void func_800749BC(SideHud *this, u_long *ot, s32 x, s32 y) {
    s32 clutY = this->side + 0x78;
    s32 u;
    s32 v;
    SPRT *sprt;

    if (this->side == 0) {
        u = 4;
        v = 0;
    } else {
        u = 0;
        v = 0x2C;
    }
    sprt = allocSprt();
    sprt->r0 = 0x80;
    sprt->g0 = 0x80;
    sprt->b0 = 0x80;
    sprt->x0 = x;
    sprt->y0 = y;
    sprt->u0 = u;
    sprt->v0 = v;
    sprt->clut = getClut(0x3C0, clutY);
    sprt->w = 0x9C;
    sprt->h = 0x2B;
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
}

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065098);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80074A88);

/* Destroys a side's HUD. */
void SideHud_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/side_hud", SIDE_HUD_VTABLE);
