/* A side's HUD at the bottom of the screen: the life and evolution gauges, the score, the timer, the round wins,
 * the hit counter and the portrait; drawn on the left and mirrored on the right. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/text/console.h"

/* A gauge of the side HUD (its life and its evolution gauge), each set up by sideGaugeInit with 0x10000 at 0xC. */
typedef struct SideGauge {
    /* 0x0 */ s32 value;  /* 0 to 0x10000 for all of it */
    /* 0x4 */ s32 target; /* the length the value gives */
    /* 0x8 */ s32 shown;  /* the length drawn, a step a frame towards target */
    /* 0xC */ s32 unkC;
} SideGauge;

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_init);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_tick);

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

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawTimer);

INCLUDE_RODATA("game/nonmatchings/side_hud", SIDE_HUD_DECIMAL_PLACES);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawScore);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065060);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065064);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawHealthGauge);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065074);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065078);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065088);

INCLUDE_ASM("game/nonmatchings/side_hud", SideHud_drawEvolutionGauge);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800747F8);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800749BC);

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
