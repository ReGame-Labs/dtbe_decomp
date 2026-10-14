#include "common.h"
#include "engine/task/entity.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/math/vector.h"
#include "engine/system/memory.h"
#include "engine/sound/sound_control.h"

/* the constructor st07_createThrowers calls, and what that constructor calls last */
BonusThrower *BonusThrower_init(BonusThrower *this, Fight *owner, s32 side);
BonusLandingMark *BonusLandingMark_init(BonusLandingMark *this, Fight *owner, Vec3 *pos);
void BonusThrower_place(BonusThrower *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3874;
extern EntityVtable BONUS_LANDING_MARK_VTABLE;
extern EntityVtable BONUS_THROWER_VTABLE;
extern EntityVtable ST07_BALL_VTABLE;
extern EntityVtable D_800D3914;

/* The stage's computer player (StageBallShoot_createController), derived from Controller: it holds the button, or
 * lets it go, for random lengths. */
typedef struct BallShootController {
    /* 0x00 */ Controller base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} BallShootController;

/* Bonus game 0's stage (StageBallShoot_init, virtual table STAGE_BALL_SHOOT_VTABLE), derived from BonusFight: each
 * side holds its button to charge a throw at the goal; a hit counts, up to 10. */
typedef struct StageBallShoot {
    /* 0x000 */ BonusFight base;
    /* 0xF70 */ s16 frames; /* the scenery's frames (StageBallShoot_tickScenery) */
    /* 0xF72 */ u8 unkF72[0xF78 - 0xF72];
    /* 0xF78 */ MeshScene *meshScene; /* Fight_getMeshScene's, at the round's start */
    /* 0xF7C */ s16 hits[2]; /* by side: the goals St07Ball_testGoal counts, up to 10 */
    /* 0xF80 */ s16 unkF80;
    /* 0xF82 */ s16 unkF82;
    /* 0xF84 */ u16 holdFrames[2]; /* by side: frames button 0x80 has been held, up to BONUS_CHARGE_FRAMES */
    /* 0xF88 */ s16 heldCharge[2]; /* by side: how long it was held, when let go (st07_updateCharge) */
    /* 0xF8C */ s16 unkF8C;    /* set to 1 by St07Ball_testGoal */
    /* 0xF8E */ u8 unkF8E[0xF94 - 0xF8E];
    /* 0xF94 */ VECTOR goalPoints[3]; /* St07Ball_testGoal tests against [1] */
} StageBallShoot;

extern VtableEntry BALL_SHOOT_CONTROLLER_VTABLE[];
extern VtableEntry STAGE_BALL_SHOOT_VTABLE[];

BallShootController *BallShootController_init(BallShootController *this, BattleSide *side);

/* Constructs the stage's computer player for side (StageBallShoot_createController). */
BallShootController *BallShootController_init(BallShootController *this, BattleSide *side) {
    Controller_init(&this->base, side);
    this->base.vtable = BALL_SHOOT_CONTROLLER_VTABLE;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    this->unk14 = 0;
    return this;
}

INCLUDE_ASM("st07/nonmatchings/st07", BallShootController_getButtons);

/* Constructs the stage: BonusFight's, with virtual table STAGE_BALL_SHOOT_VTABLE and its eight mesh nodes. */
StageBallShoot *StageBallShoot_init(StageBallShoot *this) {
    BonusFight_init(&this->base);
    this->base.base.vtable = STAGE_BALL_SHOOT_VTABLE;
    this->base.nodes[0] = 9;
    this->base.nodes[1] = 10;
    this->base.nodes[2] = 11;
    this->base.nodes[3] = 13;
    this->base.nodes[4] = 14;
    this->base.nodes[5] = 15;
    this->base.nodes[6] = 16;
    this->base.nodes[7] = 12;
    return this;
}

/* The stage's destructor: virtual table STAGE_BALL_SHOOT_VTABLE, then BonusFight's. */
void StageBallShoot_destroy(StageBallShoot *this, s32 flags) {
    this->base.base.vtable = STAGE_BALL_SHOOT_VTABLE;
    BonusFight_destroy(&this->base, flags);
}

/* what the stage's startRound and tickScenery call */
void st07_clearState(StageBallShoot *this);
void st07_resetCharge(StageBallShoot *this);
void st07_createThrowers(Fight *this);
void st07_createBoard(StageBallShoot *this);
void func_800D3D58(StageBallShoot *this);
void st07_updateCharge(StageBallShoot *this);
void func_800D4140(StageBallShoot *this);

/* The stage's override of Fight_startRound: BonusFight's, then its own state and objects; the result waits 90
 * frames. */
void StageBallShoot_startRound(StageBallShoot *this) {
    BonusFight_startRound(&this->base);
    st07_clearState(this);
    this->meshScene = Fight_getMeshScene(&this->base.base);
    st07_resetCharge(this);
    st07_createThrowers(&this->base.base);
    st07_createBoard(this);
    this->base.resultDelay = 90;
}

/* The stage's override of Fight_createController: a BallShootController for the side. */
BallShootController *StageBallShoot_createController(void *this, s32 unk, BattleSide *battleSide) {
    return BallShootController_init(operatorNew(0x18), battleSide);
}

/* Clears six of the stage's halfwords; StageBallShoot_startRound calls it first. */
void st07_clearState(StageBallShoot *this) {
    this->frames = 0;
    this->base.hudHidden = 0;
    this->hits[1] = 0;
    this->hits[0] = 0;
    this->unkF82 = 0;
    this->unkF80 = 0;
}

/* where the stage's view looks at, and from */
extern Vec3 ST07_VIEW_TARGET;
extern Vec3 ST07_VIEW_EYE;

/* look_at.c's lookAtSetTarget and lookAtSetEye, which the game inlined here (the second dirty store is merged
 * into the first) */
static inline void setTarget(LookAt *this, Vec3 *target) {
    s32 x, y, z;

    this->dirty = 1;
    x = target->vx;
    y = target->vy;
    z = target->vz;
    this->target.vx = x;
    this->target.vy = y;
    this->target.vz = z;
}

static inline void setEye(LookAt *this, Vec3 *eye) {
    s32 x, y, z;

    this->dirty = 1;
    x = eye->vx;
    y = eye->vy;
    z = eye->vz;
    this->eye.vx = x;
    this->eye.vy = y;
    this->eye.vz = z;
}

/* The stage's override of Fight_tick: BonusFight's, then, while the HUD is shown, both fighters kept at their
 * start positions and the fixed view from ST07_VIEW_EYE at ST07_VIEW_TARGET. */
void StageBallShoot_tick(StageBallShoot *this) {
    LookAt *lookAt;

    BonusFight_tick(&this->base);
    if (this->base.hudHidden == 0) {
        /* Fighter_setPosition reads the first three words of each VECTOR */
        Fighter_setPosition(Fight_getFighter(&this->base.base, 0), (Vec3 *)&this->base.startPos[0]);
        Fighter_setPosition(Fight_getFighter(&this->base.base, 1), (Vec3 *)&this->base.startPos[1]);
        lookAt = Fight_getLookAt(&this->base.base);
        setTarget(lookAt, &ST07_VIEW_TARGET);
        setEye(lookAt, &ST07_VIEW_EYE);
        Fight_applyView(&this->base.base);
    }
}

/* The stage's override of Fight_tickScenery: BonusFight's, then its own three steps, and counts the frame. */
void StageBallShoot_tickScenery(StageBallShoot *this) {
    BonusFight_tickScenery(&this->base);
    st07_updateCharge(this);
    func_800D4140(this);
    func_800D3D58(this);
    this->frames++;
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D3D58);

/* Clears the hold counts and sets the gauge's maximum to BONUS_CHARGE_FRAMES; called by StageBallShoot_startRound. */
void st07_resetCharge(StageBallShoot *this) {
    this->holdFrames[0] = 0;
    this->holdFrames[1] = 0;
    this->heldCharge[1] = 0;
    this->heldCharge[0] = 0;
    this->base.gaugeMax = BONUS_CHARGE_FRAMES;
}

/* g++'s `new` of a BonusThrower: NULL when the allocation fails. The match depends
 * on this helper: written out in the loop, the second NULL check folds into the first. */
static inline Entity *newThrower(Fight *owner, s32 side) {
    Entity *child = allocEntityMemory(sizeof(BonusThrower));

    if (child != NULL) {
        BonusThrower_init(entityOperatorNewAt(sizeof(BonusThrower), child), owner, side);
    }
    return child;
}

/* Creates the stage's two BonusThrowers (side 0 and 1), each put into the entity group of this. */
void st07_createThrowers(Fight *this) {
    s32 i;

    for (i = 0; i < 2; i++) {
        Entity *child = newThrower(this, i);

        if (child != NULL) {
            Fight_addEntity(this, child);
        }
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", st07_createBoard);


/* Sets a side's gauge: BonusFight's setter, out of line in game.bin as func_800D3284. The match
 * depends on its s16 value parameter: promoted to int, it loads the u16 count with lh before the store. */
static inline void setGauge(StageBallShoot *this, s32 index, s16 value) {
    this->base.gauges[index] = value;
}

/* Each frame, for both sides: counts the frames their button 0x80 is held (up to BONUS_CHARGE_FRAMES), keeps the
 * count in heldCharge when it is let go, and copies the count to the side's gauge. */
void st07_updateCharge(StageBallShoot *this) {
    struct InputHistory *pads[2];
    u32 buttons[2];
    s32 i;

    for (i = 0; i < 2; i++) {
        pads[i] = Fight_getInput(&this->base.base, i);
        buttons[i] = InputHistory_getHeld(pads[i]);
        if (buttons[i] & 0x80) {
            if (++this->holdFrames[i] > BONUS_CHARGE_FRAMES) {
                this->holdFrames[i] = BONUS_CHARGE_FRAMES;
            }
            this->heldCharge[i] = 0;
        } else {
            this->heldCharge[i] = this->holdFrames[i];
            this->holdFrames[i] = 0;
        }
        setGauge(this, i, this->holdFrames[i]);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4140);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D449C);

/* The destructor of the entity class of vtable D_800D3914, which func_800D449C installs. */
void func_800D458C(Entity *this, s32 flags) {
    this->vtable = &D_800D3914;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D45E0);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4718);

INCLUDE_ASM("st07/nonmatchings/st07", St07Ball_init);

/* St07Ball's destructor. */
void St07Ball_destroy(Entity *this, s32 flags) {
    this->vtable = &ST07_BALL_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The ball a BonusThrower throws (St07Ball_init, virtual table ST07_BALL_VTABLE, 0x80 bytes). */
typedef struct St07Ball {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageBallShoot *stage;
    /* 0x1C */ u16 side; /* indexes the stage's tables by side (lhu) */
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ u16 unk20; /* counts the ticks spent above the ground (St07Ball_tick) */
    /* 0x22 */ u8 unk22[0x24 - 0x22];
    /* 0x24 */ SVECTOR corners[4]; /* a 128 by 128 square */
    /* 0x44 */ Vec3 force;         /* func_80079C1C's force, cleared after each tick */
    /* 0x50 */ s32 unk50;
    /* 0x54 */ Vec3 velocity;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ Vec3 position;
    /* 0x70 */ u8 unk70[0x78 - 0x70];
    /* 0x78 */ s32 unk78;
    /* 0x7C */ s32 unk7C; /* added to 0x78 each tick */
} St07Ball;

s32 St07Ball_testGoal(St07Ball *this, s32 arg);
void st07_spawnGoalEffect(St07Ball *this, u16 index, s32 arg);

/* St07Ball's tick: falls (0x71C added to the force) and moves by func_80079C1C's step (in 1/256ths); it deletes
 * itself once St07Ball_testGoal says so, or once it is below the ground (y > 0), leaving a BonusLandingMark at
 * its position in the latter case. */
void St07Ball_tick(St07Ball *this, s32 arg) {
    Vec3 *pos;
    Vec3 move;

    this->unk78 += this->unk7C;
    this->force.vy += 0x71C;
    func_80079C1C(&move, &this->velocity, &this->force, 0x10000);
    /* The match depends on taking &this->position here, after the step: the game keeps it in a saved register from
     * this point for the constructor, while the updates below still address the fields from this. */
    pos = &this->position;
    this->force.vx = 0;
    this->force.vy = 0;
    this->force.vz = 0;
    this->unk50 = 0;
    this->position.vx += move.vx >> 8;
    this->position.vy += move.vy >> 8;
    this->position.vz += move.vz >> 8;
    if (St07Ball_testGoal(this, arg)) {
        entityDelete(&this->entity);
    } else if (this->position.vy > 0) {
        entityAddSibling(&this->entity,
                         NEW_ENTITY(sizeof(BonusLandingMark), BonusLandingMark_init, &this->stage->base.base, pos));
        entityDelete(&this->entity);
    } else {
        this->force.vx = 0;
        this->force.vy = 0;
        this->force.vz = 0;
        this->unk50 = 0;
        this->unk20++;
    }
}

/* St07Ball_testGoal's results. */
enum {
    BALL_GOAL_MISS = 0, /* nowhere near the goal: the ball goes on */
    BALL_GOAL_OUT = 1,  /* at the goal but below it: it leaves a BonusLandingMark */
    BALL_GOAL_HIT = 2   /* in the goal: the side's hits go up */
};

/* The sound of a goal. */
#define ST07_SOUND_GOAL 0x6C

/* The ball's test against the stage's goal point goalPoints[1]: BALL_GOAL_MISS when the side has 10 hits or
 * more, when the ball goes up (velocity.vy < 0) or when it is 0x80 or more away from the point in x or in y (0x180
 * off in y); otherwise BALL_GOAL_HIT when position.vy is less than the point's + 0x180 (the hits go up,
 * st07_spawnGoalEffect and ST07_SOUND_GOAL), or BALL_GOAL_OUT when it is not, with a BonusLandingMark left at its
 * position. */
s32 St07Ball_testGoal(St07Ball *this, s32 arg) {
    s32 y;

    if (this->stage->hits[this->side] >= 10) {
        return BALL_GOAL_MISS;
    }
    if (this->velocity.vy < 0) {
        return BALL_GOAL_MISS;
    }
    /* The match depends on the nested ifs (with early returns the two arms' tests cross-jump into one), on each
     * distance written out in both arms (with a local d, fold swaps the arms) and on the last test's form
     * (written position.vy - 0x180 - point, it computes point + 0x180 first). */
    if ((this->position.vx - this->stage->goalPoints[1].vx >= 0
             ? this->position.vx - this->stage->goalPoints[1].vx
             : this->stage->goalPoints[1].vx - this->position.vx) < 0x80) {
        y = this->position.vy - 0x180;
        if ((y - this->stage->goalPoints[1].vy >= 0 ? y - this->stage->goalPoints[1].vy
                                                 : this->stage->goalPoints[1].vy - y) < 0x80) {
            if (this->position.vy - (this->stage->goalPoints[1].vy + 0x180) < 0) {
                this->stage->hits[this->side]++;
                this->stage->unkF8C = 1;
                st07_spawnGoalEffect(this, this->side, arg);
                soundControlPlaySound(&SOUND_CONTROL, ST07_SOUND_GOAL, 0);
                return BALL_GOAL_HIT;
            }
            entityAddSibling(&this->entity, NEW_ENTITY(sizeof(BonusLandingMark), BonusLandingMark_init,
                                                       &this->stage->base.base, &this->position));
            return BALL_GOAL_OUT;
        }
    }
    return BALL_GOAL_MISS;
}

INCLUDE_ASM("st07/nonmatchings/st07", st07_spawnGoalEffect);

INCLUDE_ASM("st07/nonmatchings/st07", St07Ball_draw);

/* Constructs the BonusThrower of side for owner: the corners of a 128 by 128 square, then BonusThrower_place. */
BonusThrower *BonusThrower_init(BonusThrower *this, Fight *owner, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &BONUS_THROWER_VTABLE;
    this->owner = owner;
    this->side = side;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->frame = 0;
    this->scale = 0;
    BonusThrower_place(this);
    return this;
}

/* BonusThrower's destructor. */
void BonusThrower_destroy(Entity *this, s32 flags) {
    this->vtable = &BONUS_THROWER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", BonusThrower_tick);

INCLUDE_ASM("st07/nonmatchings/st07", BonusThrower_throw);

INCLUDE_ASM("st07/nonmatchings/st07", BonusThrower_place);

INCLUDE_ASM("st07/nonmatchings/st07", BonusThrower_draw);

/* Constructs a BonusLandingMark of owner at pos: a 512 by 512 square. */
BonusLandingMark *BonusLandingMark_init(BonusLandingMark *this, Fight *owner, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &BONUS_LANDING_MARK_VTABLE;
    this->owner = owner;
    /* The match depends on reading pos into x, y and z here and storing them last. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x100;
    this->corners[1].vx = this->corners[3].vx = 0x100;
    this->corners[0].vy = this->corners[1].vy = -0x100;
    this->corners[2].vy = this->corners[3].vy = 0x100;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->frame = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* BonusLandingMark's destructor. */
void BonusLandingMark_destroy(Entity *this, s32 flags) {
    this->vtable = &BONUS_LANDING_MARK_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* BonusLandingMark's tick: counts its frames, and deletes it after BONUS_LANDING_MARK_FRAMES. */
void BonusLandingMark_tick(BonusLandingMark *this, s32 arg) {
    if (++this->frame > BONUS_LANDING_MARK_FRAMES) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", BonusLandingMark_draw);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5910);

/* The destructor of the entity class of vtable D_800D3874, which func_800D5910 installs. */
void func_800D59B4(Entity *this, s32 flags) {
    this->vtable = &D_800D3874;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5A08);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5AFC);

/* BallShootController's destructor: Controller's. */
void BallShootController_destroy(BallShootController *this, s32 flags) {
    Controller_destroy(&this->base, flags);
}
