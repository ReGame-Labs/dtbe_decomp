#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "psyq.h"
#include "engine/math/random.h"
#include "engine/system/memory.h"

/* The stage's computer player (StageEvolution_createController), derived from Controller: it presses the button
 * once every random period. */
typedef struct EvolutionController {
    /* 0x00 */ Controller base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
} EvolutionController;

/* Bonus game 1's stage (StageEvolution_init, virtual table STAGE_EVOLUTION_VTABLE), derived from BonusFight: a
 * button-mash race; the side whose meter reaches 0 first evolves (st08_checkWinner). */
typedef struct StageEvolution {
    /* 0x000 */ BonusFight base;
    /* 0xF70 */ RECT clutRect;     /* 16 by 1 pixels at (0x300, 0xFC): a CLUT */
    /* 0xF78 */ u16 clut[16];     /* that CLUT read back from VRAM (st08_readClut) */
    /* 0xF98 */ u16 clutCopy[16]; /* and a second copy */
    /* 0xFB8 */ s16 frames; /* the scenery's frames (StageEvolution_tickScenery) */
    /* 0xFBA */ s16 unkFBA;
    /* 0xFBC */ s16 winner; /* the side whose meter reached 0, -1 for none (st08_checkWinner) */
    /* 0xFBE */ s16 unkFBE;
    /* 0xFC0 */ u8 unkFC0[0xFC8 - 0xFC0];
    /* 0xFC8 */ s16 unkFC8;
    /* 0xFCA */ s16 unkFCA;
    /* 0xFCC */ s16 unkFCC;
    /* 0xFCE */ u8 unkFCE[0xFD0 - 0xFCE];
    /* 0xFD0 */ s32 unkFD0;
    /* 0xFD4 */ s32 unkFD4;
    /* 0xFD8 */ MeshScene *meshScene; /* Fight_getMeshScene's, at the round's start */
    /* 0xFDC */ s16 unkFDC;
    /* 0xFDE */ s16 unkFDE;
    /* 0xFE0 */ s16 meters[2]; /* by side: ST08_METER_START at the round's start, copied to BonusFight.counts */
} StageEvolution;

/* A side's meter at the round's start (st08_resetMeters), also BonusFight.countMax. */
#define ST08_METER_START 120

/* BonusFight.gaugeMax in this stage (st07 and st09: BONUS_CHARGE_FRAMES). */
#define ST08_GAUGE_MAX 15

extern VtableEntry EVOLUTION_CONTROLLER_VTABLE[];
extern VtableEntry STAGE_EVOLUTION_VTABLE[];

EvolutionController *EvolutionController_init(EvolutionController *this, BattleSide *side);

/* Constructs the stage's computer player for side (StageEvolution_createController). */
EvolutionController *EvolutionController_init(EvolutionController *this, BattleSide *side) {
    Controller_init(&this->base, side);
    this->base.vtable = EVOLUTION_CONTROLLER_VTABLE;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    return this;
}

/* Selects 0..2 in EvolutionController_getButtons (the period ranges 1-10, 2-7 and 3-6) */
extern s32 D_8005FB84;

/* EvolutionController's getButtons: returns 0x80 once every unk10 calls (counted at unkC), and draws a new random
 * period at the start of each one, from the range D_8005FB84 selects; returns 0 otherwise.
 * The match depends on the roll taking the random value before its own "%=" (an initializer with the
 * "%" keeps the remainder out of $v0), and on the count read into a local before the increment. */
s32 EvolutionController_getButtons(EvolutionController *this) {
    s32 count;

    if (this->unk8 == 0) {
        switch (D_8005FB84) {
        case 0: {
            s32 roll = mersenneTwisterGenerate(&RANDOM);

            roll %= 10;
            this->unk10 = (roll < 0 ? -roll : roll) + 1;
            break;
        }
        case 1: {
            s32 roll = mersenneTwisterGenerate(&RANDOM);

            roll %= 6;
            this->unk10 = (roll < 0 ? -roll : roll) + 2;
            break;
        }
        case 2:
            this->unk10 = (mersenneTwisterGenerate(&RANDOM) & 3) + 3;
            break;
        }
        this->unkC = 0;
        this->unk8++;
    }
    count = this->unkC;
    this->unkC = count + 1;
    if ((count + 2) % this->unk10 != 0) {
        return 0;
    }
    this->unk8 = 0;
    return 0x80;
}

/* Constructs the stage: BonusFight's, with virtual table STAGE_EVOLUTION_VTABLE and its eight mesh nodes, and
 * clears 0xFD0 and 0xFD4. */
StageEvolution *StageEvolution_init(StageEvolution *this) {
    BonusFight_init(&this->base);
    this->base.base.vtable = STAGE_EVOLUTION_VTABLE;
    this->base.nodes[0] = 9;
    this->base.nodes[1] = 10;
    this->base.nodes[2] = 14;
    this->base.nodes[3] = 11;
    this->base.nodes[4] = 12;
    this->base.nodes[5] = 8;
    this->base.nodes[6] = 15;
    this->base.nodes[7] = 16;
    this->unkFD0 = this->unkFD4 = 0;
    return this;
}

/* The stage's destructor: virtual table STAGE_EVOLUTION_VTABLE, then BonusFight's. */
void StageEvolution_destroy(StageEvolution *this, s32 flags) {
    this->base.base.vtable = STAGE_EVOLUTION_VTABLE;
    BonusFight_destroy(&this->base, flags);
}

/* what StageEvolution_startRound calls */
void st08_clearState(StageEvolution *this);
void st08_readClut(StageEvolution *this);
void st08_resetCounts(StageEvolution *this);
void func_800D4390(StageEvolution *this);
void st08_resetMeters(StageEvolution *this);

/* The stage's override of Fight_startRound: BonusFight's, then its own state and objects; the result waits 120
 * frames, 0xFBA and the winner start at -1 (as st07's StageBallShoot_startRound). */
void StageEvolution_startRound(StageEvolution *this) {
    BonusFight_startRound(&this->base);
    st08_clearState(this);
    Fight_setBounds(&this->base.base, -0x500, 0x500, -0xC80, 0x400, 0);
    this->meshScene = Fight_getMeshScene(&this->base.base);
    st08_resetCounts(this);
    func_800D4390(this);
    st08_resetMeters(this);
    st08_readClut(this);
    this->base.resultDelay = 120;
    this->unkFBA = -1;
    this->winner = -1;
}

/* The stage's override of Fight_createController: an EvolutionController for the side. */
EvolutionController *StageEvolution_createController(void *this, s32 unk, BattleSide *battleSide) {
    return EvolutionController_init(operatorNew(0x14), battleSide);
}

/* Clears the frames, hudHidden and 0xFBE (StageEvolution_startRound calls it after BonusFight_startRound). */
void st08_clearState(StageEvolution *this) {
    this->frames = 0;
    this->base.hudHidden = 0;
    this->unkFBE = 0;
}

INCLUDE_ASM("st08/nonmatchings/st08", StageEvolution_tick);

/* the stage's own steps */
void st08_checkWinner(StageEvolution *this);
void func_800D4018(StageEvolution *this);
void func_800D40DC(StageEvolution *this);
void func_800D4590(StageEvolution *this);
void st08_copyMeters(StageEvolution *this);

/* The stage's override of Fight_tickScenery: BonusFight's, then its own five steps, and counts the frame. */
void StageEvolution_tickScenery(StageEvolution *this) {
    BonusFight_tickScenery(&this->base);
    func_800D40DC(this);
    func_800D4590(this);
    func_800D4018(this);
    st08_copyMeters(this);
    st08_checkWinner(this);
    this->frames++;
}

/* The stage's last step, until hudHidden is set: a side whose meter reached 0 gets Fighter_requestEvolution
 * (when bit 0 of its record is clear, counted at 0xFBE) and Fighter_addGauge(0x10000), and winner names it;
 * with both, winner and 0xFBA go to -1. 0xD28 == 1 calls BonusFight_endInDraw and sets 0xFBA to 170.
 * The match depends on the bit tests written "(x ^ 1) & 1" (xori, andi; "!(x & 1)" branches on andi). */
void st08_checkWinner(StageEvolution *this) {
    s16 done = 0;
    u16 first;
    u16 second;

    if (this->base.hudHidden != 0) {
        return;
    }
    first = FIGHTER_PARAMS[Fighter_getCharacter(Fight_getFighter(&this->base.base, 0))].flags;
    second = FIGHTER_PARAMS[Fighter_getCharacter(Fight_getFighter(&this->base.base, 1))].flags;
    if (this->meters[0] == 0) {
        this->winner = 0;
        if ((first ^ 1) & 1) {
            Fighter_requestEvolution(Fight_getFighter(&this->base.base, 0));
            this->unkFBE++;
        }
        Fighter_addGauge(Fight_getFighter(&this->base.base, 0), 0x10000);
        this->base.hudHidden = 1;
        done = 1;
    }
    if (this->meters[1] == 0) {
        this->winner = 1;
        if ((second ^ 1) & 1) {
            Fighter_requestEvolution(Fight_getFighter(&this->base.base, 1));
            this->unkFBE++;
        }
        Fighter_addGauge(Fight_getFighter(&this->base.base, 1), 0x10000);
        this->base.hudHidden = 1;
        done++;
    }
    if (done == 2) {
        this->base.hudHidden = 1;
        this->winner = -1;
        this->unkFBA = -1;
    }
    if (this->base.base.timeLeft == 1) {
        BonusFight_endInDraw(&this->base);
        this->base.hudHidden = 1;
        this->winner = -1;
        this->unkFBA = 170;
    }
}

/* Reads the 16-colour CLUT at (0x300, 0xFC) back from VRAM twice, into clut and clutCopy. */
void st08_readClut(StageEvolution *this) {
    this->clutRect.x = 0x300;
    this->clutRect.y = 0xFC;
    this->clutRect.w = 16;
    this->clutRect.h = 1;
    StoreImage(&this->clutRect, (u_long *)this->clut);
    StoreImage(&this->clutRect, (u_long *)this->clutCopy);
}

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4018);

/* Clears the halfwords at 0xFDC, 0xFDE and 0xFC8-0xFCC and sets the gauge's maximum (as st07_resetCharge). */
void st08_resetCounts(StageEvolution *this) {
    this->unkFDC = 0;
    this->unkFDE = 0;
    this->unkFC8 = 0;
    this->unkFCC = 0;
    this->unkFCA = 0;
    this->base.gaugeMax = ST08_GAUGE_MAX;
}

INCLUDE_ASM("st08/nonmatchings/st08", func_800D40DC);

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4390);

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4590);

/* Sets both sides' meters and BonusFight.countMax to ST08_METER_START (StageEvolution_startRound calls it).
 * The match depends on the sides' value in a local set before the loop (its 120 is loaded first). */
void st08_resetMeters(StageEvolution *this) {
    s16 start = ST08_METER_START;
    s32 side;

    for (side = 1; side >= 0; side--) {
        this->meters[side] = start;
    }
    this->base.countMax = ST08_METER_START;
}

/* Copies both sides' meters into BonusFight.counts (the last of the stage's steps before st08_checkWinner).
 * The match depends on the value going through an int (lh, not lhu); gcc turns the loop into a countdown. */
void st08_copyMeters(StageEvolution *this) {
    s32 side;
    s32 value;

    for (side = 0; side < 2; side++) {
        value = this->meters[side];
        this->base.counts[side] = value;
    }
}

/* EvolutionController's destructor: Controller's. */
void EvolutionController_destroy(EvolutionController *this, s32 flags) {
    Controller_destroy(&this->base, flags);
}
