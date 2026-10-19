#include "common.h"
#include "engine/task/entity.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/vector.h"
#include "engine/system/memory.h"
#include "engine/sound/sound_control.h"

/* the constructor st07_createThrowers calls, and what that constructor calls last */
BonusThrower *BonusThrower_init(BonusThrower *this, Fight *owner, s32 side);
BonusLandingMark *BonusLandingMark_init(BonusLandingMark *this, Fight *owner, Vec3 *pos);
void BonusThrower_place(BonusThrower *this);
void BonusThrower_throw(BonusThrower *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable ST07_BOARD_VTABLE;
extern EntityVtable BONUS_LANDING_MARK_VTABLE;
extern EntityVtable BONUS_THROWER_VTABLE;
extern EntityVtable ST07_BALL_VTABLE;
extern EntityVtable D_800D3914;

/* The stage's computer player (StageBallShoot_createController), derived from Controller: it holds the button, or
 * lets it go, for random lengths. */
typedef struct BallShootController {
    /* 0x00 */ Controller base;
    /* 0x08 */ s32 unk8;     /* 0 when a new spell starts; one more at its start (BallShootController_getButtons) */
    /* 0x0C */ s32 frames;   /* of the spell */
    /* 0x10 */ s32 period;   /* the spell's length */
    /* 0x14 */ s32 released; /* the spell lets the button go (else it holds button 0x80) */
} BallShootController;

/* The goals that win the bonus game (st07_checkEnd); St07Ball_testGoal stops counting there. */
#define ST07_WINNING_HITS 10

/* Bonus game 0's stage (StageBallShoot_init, virtual table STAGE_BALL_SHOOT_VTABLE), derived from BonusFight: each
 * side holds its button to charge a throw at the goal; a hit counts, up to 10. */
typedef struct StageBallShoot {
    /* 0x000 */ BonusFight base;
    /* 0xF70 */ s16 frames; /* the scenery's frames (StageBallShoot_tickScenery) */
    /* 0xF72 */ u8 unkF72[0xF78 - 0xF72];
    /* 0xF78 */ MeshScene *meshScene; /* Fight_getMeshScene's, at the round's start */
    /* 0xF7C */ s16 hits[2]; /* by side: the goals St07Ball_testGoal counts, up to ST07_WINNING_HITS */
    /* 0xF80 */ s16 unkF80;
    /* 0xF82 */ s16 unkF82;
    /* 0xF84 */ u16 holdFrames[2]; /* by side: frames button 0x80 has been held, up to BONUS_CHARGE_FRAMES */
    /* 0xF88 */ u16 heldCharge[2]; /* by side: how long it was held, when let go (st07_updateCharge) */
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
    this->frames = 0;
    this->period = 0;
    this->released = 0;
    return this;
}

/* BallShootController's getButtons: in spells of random length, holds button 0x80 (15 to 30 frames) or lets it go
 * (15 to 46 frames on easy, 15 to 22 on normal; never on hard), each picked at random at the spell's start; the
 * spell's last frame returns 0. */
s32 BallShootController_getButtons(BallShootController *this) {
    s32 level = GAME_STATE.level;
    s32 buttons = 0;
    s32 frames;

    if (this->unk8 == 0) {
        if (level != LEVEL_HARD) {
            this->released = mersenneTwisterGenerate(&RANDOM) & 1;
        } else {
            this->released = 0;
        }
        switch (this->released) {
        case 0:
            this->period = (mersenneTwisterGenerate(&RANDOM) & 0xF) + 0xF;
            break;
        case 1:
            switch (level) {
            case LEVEL_EASY:
                this->period = (mersenneTwisterGenerate(&RANDOM) & 0x1F) + 0xF;
                break;
            case LEVEL_NORMAL:
                this->period = (mersenneTwisterGenerate(&RANDOM) & 7) + 0xF;
                break;
            }
            break;
        }
        this->frames = 0;
        this->unk8++;
    }
    frames = ++this->frames;
    if (this->released == 0) {
        buttons = 0x80;
    }
    if (frames >= this->period) {
        this->unk8 = 0;
        return 0;
    }
    return buttons;
}

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
void st07_checkEnd(StageBallShoot *this);
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
    st07_checkEnd(this);
    this->frames++;
}

/* Once per fight: a side with ST07_WINNING_HITS goals wins; at the end of the time (timeLeft 1) the side with more
 * goals wins, equal goals end in a draw. Then the bonus HUD is hidden. */
void st07_checkEnd(StageBallShoot *this) {
    if (this->base.hudHidden == 0) {
        if (this->hits[0] == ST07_WINNING_HITS) {
            BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 0),
                                     Fight_getFighter(&this->base.base, 1));
            this->base.hudHidden = 1;
        }
        if (this->hits[1] == ST07_WINNING_HITS) {
            BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 1),
                                     Fight_getFighter(&this->base.base, 0));
            this->base.hudHidden = 1;
        }
        if (this->base.base.timeLeft == 1) {
            /* The match depends on the store in each arm: there cse stores timeLeft's register, known to be 1. */
            if (this->hits[0] == this->hits[1]) {
                BonusFight_endInDraw(&this->base);
                this->base.hudHidden = 1;
            } else if (this->hits[0] > this->hits[1]) {
                BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 0),
                                         Fight_getFighter(&this->base.base, 1));
                this->base.hudHidden = 1;
            } else {
                BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 1),
                                         Fight_getFighter(&this->base.base, 0));
                this->base.hudHidden = 1;
            }
        }
    }
}

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

/* Unk800D449C's states (func_800D45E0). */
enum {
    GOAL_MARK_FLYING, /* to the target */
    GOAL_MARK_GROWING, /* at the target, after its mark is added */
    GOAL_MARK_DONE
};

/* The class func_800D449C constructs (virtual table D_800D3914, 0x58 bytes; st07_spawnGoalEffect makes one at a
 * goal): a sprite that flies on the screen from the goal to the side's hit counter, which then gets a mark. */
typedef struct Unk800D449C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageBallShoot *stage;
    /* 0x1C */ s16 side;
    /* 0x1E */ u16 frames;
    /* 0x20 */ u16 stateFrames;
    /* 0x22 */ u8 unk22[0x24 - 0x22];
    /* 0x24 */ VECTOR position; /* on the screen, in 16ths */
    /* 0x34 */ VECTOR step;     /* a sixteenth of the way to the target (x and y) */
    /* 0x44 */ VECTOR target;   /* in 16ths */
    /* 0x54 */ u16 state;       /* GOAL_MARK_FLYING, _GROWING, then _DONE */
    /* 0x56 */ s16 size;
} Unk800D449C;

/* Unk800D449C's constructor: from a screen point to another (x, y and z of each, in pixels) for a side. */
Unk800D449C *func_800D449C(Unk800D449C *this, StageBallShoot *stage, SVECTOR *from, SVECTOR *to, s16 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3914;
    this->stage = stage;
    /* The match depends on reading each point's three halves before storing any, each copy in its own block. */
    {
        s32 x = from->vx;
        s32 y = from->vy;
        s32 z = from->vz;

        this->position.vx = x << 4;
        this->position.vy = y << 4;
        this->position.vz = z << 4;
    }
    {
        s32 x = to->vx;
        s32 y = to->vy;
        s32 z = to->vz;

        this->target.vx = x << 4;
        this->target.vy = y << 4;
        this->target.vz = z << 4;
    }
    this->size = 0;
    this->step.vx = (this->target.vx - this->position.vx) / 16;
    this->step.vy = (this->target.vy - this->position.vy) / 16;
    this->side = side;
    this->frames = 0;
    this->state = 0;
    this->stateFrames = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3914, which func_800D449C installs. */
void func_800D458C(Entity *this, s32 flags) {
    this->vtable = &D_800D3914;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800D449C's tick: flies a sixteenth of the way a frame, growing up to size 8; within 3 pixels of the target
 * it gives its side a mark and grows faster for 10 frames, then plays sound 0x6E and is done. */
void func_800D45E0(Unk800D449C *this) {
    VECTOR offset;
    s32 x, y;

    switch (this->state) {
    case GOAL_MARK_FLYING:
        this->position.vx += this->step.vx;
        this->position.vy += this->step.vy;
        if (++this->size > 8) {
            this->size = 8;
        }
        x = this->position.vx;
        y = this->position.vy;
        offset.vx = x - this->target.vx;
        offset.vy = y - this->target.vy;
        offset.vz = 0;
        if ((vecGetLength(&offset) >> 4) >= 3) {
            break;
        }
        BonusFight_addMark(&this->stage->base, this->side);
        this->stateFrames = 0;
        this->state++;
        /* fall through */
    case GOAL_MARK_GROWING:
        this->size += this->stateFrames;
        if (++this->stateFrames >= 10) {
            soundControlPlaySound(&SOUND_CONTROL, 0x6E, 0);
            this->state++;
        }
        break;
    case GOAL_MARK_DONE: /* a case of its own in the game's switch (its compare tree tests above 1) */
        break;
    }
    this->frames++;
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4718);

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
    /* 0x78 */ s32 angle; /* the draw's rotation about z */
    /* 0x7C */ s32 spin;  /* added to the angle each tick */
} St07Ball;

/* Constructs a ball of side thrown from position with force (its x and y; unk1E gets -1 for a throw toward +x, else
 * 1), a 128 by 128 square spinning by force.vx / 32 a tick.
 * The match depends on each copy reading its words into its own block's locals first, and on the if/else for unk1E
 * (a ternary keeps force.vx in a register for the spin; the game reads it again). */
St07Ball *St07Ball_init(St07Ball *this, StageBallShoot *stage, Vec3 *position, VECTOR *force, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST07_BALL_VTABLE;
    this->stage = stage;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = force->vx;
        s32 y = force->vy;

        this->force.vx = x;
        this->force.vy = y;
    }
    this->force.vz = 0;
    this->unk50 = 0x10000;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->unk60 = 0;
    if (this->force.vx > 0) {
        this->unk1E = -1;
    } else {
        this->unk1E = 1;
    }
    this->side = side;
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->unk20 = 0;
    this->angle = 0;
    this->spin = this->force.vx >> 5;
    return this;
}

/* St07Ball's destructor. */
void St07Ball_destroy(Entity *this, s32 flags) {
    this->vtable = &ST07_BALL_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

s32 St07Ball_testGoal(St07Ball *this, s32 arg);
void st07_spawnGoalEffect(St07Ball *this, u16 index, s32 arg);

/* St07Ball's tick: falls (0x71C added to the force) and moves by func_80079C1C's step (in 1/256ths); it deletes
 * itself once St07Ball_testGoal says so, or once it is below the ground (y > 0), leaving a BonusLandingMark at
 * its position in the latter case. */
void St07Ball_tick(St07Ball *this, s32 arg) {
    Vec3 *pos;
    Vec3 move;

    this->angle += this->spin;
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

    if (this->stage->hits[this->side] >= ST07_WINNING_HITS) {
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

/* St07Ball's draw: the square turned about z by its angle at its position, with the 64 by 64 picture at
 * (0x28, 0) of the 8-bit texture page at x 640 and the CLUT at (0, 480), opaque. The match depends on the matrix
 * passed to both calls through one pointer (the game keeps it in a saved register) and on this order of the shape's
 * fields. */
void St07Ball_draw(St07Ball *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    MatrixWords m;
    MATRIX *matrix;
    PolyFT4Shape shape;
    Vec3 *position;
    s32 x, y, z;

    matrix = &m.m;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.words[5] = 0;
    m.words[6] = 0;
    m.words[7] = 0;
    matrixRotateZ(matrix, this->angle);
    position = &this->position;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    setModelMatrix(matrix);
    shape.flags = 0;
    shape.color = 0x808080;
    UV_BYTES(shape.uv0)[0] = 0x28;
    UV_BYTES(shape.uv0)[1] = 0;
    UV_BYTES(shape.uv1)[0] = 0x67;
    UV_BYTES(shape.uv1)[1] = 0;
    UV_BYTES(shape.uv2)[0] = 0x28;
    UV_BYTES(shape.uv2)[1] = 0x3F;
    UV_BYTES(shape.uv3)[0] = 0x67;
    UV_BYTES(shape.uv3)[1] = 0x3F;
    shape.tpage = getTPage(1, 0, 640, 0);
    shape.clut = getClut(0, 480);
    shape.corners[0] = &this->corners[0];
    shape.corners[1] = &this->corners[1];
    shape.corners[2] = &this->corners[2];
    shape.corners[3] = &this->corners[3];
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + 1);
}

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

/* The stage that owns a thrower. */
static inline StageBallShoot *throwerStage(BonusThrower *thrower) {
    return (StageBallShoot *)thrower->owner;
}

/* A side's hold count, as an s16. The match depends on the s16 result: BonusThrower_tick loads the u16 count with
 * lh (as setGauge's s16 parameter does). */
static inline s16 getHoldFrames(StageBallShoot *stage, s32 side) {
    return stage->holdFrames[side];
}

/* BonusThrower's tick: while its side holds the button, the scale eases a quarter of the way to 0x1000 each frame
 * (else it drops to 0); then BonusThrower_place, BonusThrower_throw, and counts its frames. */
void BonusThrower_tick(BonusThrower *this, s32 arg) {
    if (getHoldFrames(throwerStage(this), this->side) != 0) {
        this->scale += (0x1000 - this->scale) >> 2;
    } else {
        this->scale = 0;
    }
    BonusThrower_place(this);
    BonusThrower_throw(this);
    this->frame++;
}

/* Each frame for its side: when the button is let go after a hold (heldCharge) and the stage allows it, throws an
 * St07Ball from position, harder the longer the hold (the launch's pad gets the charge times 0xCC, which St07Ball_init
 * does not read), and holds the side's fighter (BonusFight_holdFighter). As st09's, without its slots and sound. */
void BonusThrower_throw(BonusThrower *this) {
    VECTOR vec;
    Entity *entity;
    s32 power;
    s32 x;
    s32 y;

    if (throwerStage(this)->base.holding[this->side] == 0 && throwerStage(this)->base.hudHidden == 0
        && throwerStage(this)->heldCharge[this->side] != 0) {
        /* The match depends on vec's address taken here: the game keeps it in a saved register from the start
         * of the block. */
        VECTOR *launch = &vec;

        vec.vx = 0;
        vec.vy = 0;
        vec.vz = 0;
        vec.pad = 0;
        power = ((throwerStage(this)->heldCharge[this->side] + 8) >> 1) * 0x71C;
        x = ((s64)power * 0x4000) >> 16;
        vec.vx += x * Fighter_getFacing(Fight_getFighter(&throwerStage(this)->base.base, this->side));
        y = ((s64)power * 0x10000) >> 16;
        vec.vy -= y;
        vec.pad = throwerStage(this)->heldCharge[this->side] * 0xCC;
        /* The match depends on the owner cast here: throwerStage's inline result takes another register. */
        entity = NEW_ENTITY(sizeof(St07Ball), St07Ball_init, (StageBallShoot *)this->owner, &this->position, launch,
                            this->side);
        entityAddSibling(&this->entity, entity);
        BonusFight_holdFighter(&throwerStage(this)->base, this->side);
    }
}

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

/* The board st07_createBoard puts on the stage's mesh: sixteen pieces, two more every 8 frames, each falling (its
 * offset's vy grows by speed / 4) and fading until it is 0x100 down, then starting again (St07Board_tick). */
typedef struct St07Board {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageBallShoot *owner;
    /* 0x1C */ MeshScene *meshScene; /* the stage's, the board drawn on its node 3 */
    /* 0x20 */ u16 frames;
    /* 0x22 */ u16 count;           /* the pieces in play: + 2 every 8 frames, up to 16 */
    /* 0x24 */ s16 speeds[16];      /* + 1 a frame while falling */
    /* 0x44 */ u8 shades[16];       /* -0x80 - vy / 2: the piece's grey */
    /* 0x54 */ u16 unk54[16];       /* while set, the piece does not fall */
    /* 0x74 */ VECTOR offsets[16];
} St07Board; /* size 0x174 */

/* vector_ops.c's vecClear, inlined: the match depends on clearing through a pointer. */
static inline VECTOR *clearVector(VECTOR *v) {
    v->vx = 0;
    v->vy = 0;
    v->vz = 0;
    v->pad = 0;
    return v;
}

/* Constructs the board of owner on meshScene, every piece at rest. */
St07Board *St07Board_init(St07Board *this, StageBallShoot *owner, MeshScene *meshScene) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST07_BOARD_VTABLE;
    this->owner = owner;
    this->meshScene = meshScene;
    for (i = 0; i < 16; i++) {
        clearVector(&this->offsets[i]);
        this->shades[i] = 0;
        this->speeds[i] = 0;
        this->unk54[i] = 0;
    }
    this->frames = 0;
    this->count = 0;
    return this;
}

/* St07Board's destructor. */
void St07Board_destroy(Entity *this, s32 flags) {
    this->vtable = &ST07_BOARD_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The board's tick: every 8 frames two more pieces come into play (up to 16); each piece not held by unk54 falls
 * faster and fades, and starts again at rest once it is 0x100 down. */
void St07Board_tick(St07Board *this, s32 arg) {
    s32 i;

    if ((this->frames & 7) == 0) {
        this->count += 2;
        if (this->count > 16) {
            this->count = 16;
        }
    }
    for (i = 0; i < this->count; i++) {
        if (this->unk54[i] == 0) {
            this->offsets[i].vy += ++this->speeds[i] >> 2;
            this->shades[i] = -0x80 - (this->offsets[i].vy >> 1);
            if (this->offsets[i].vy > 0x100) {
                clearVector(&this->offsets[i]);
                this->shades[i] = 0;
                this->speeds[i] = 0;
            }
        }
    }
    this->frames++;
}

INCLUDE_ASM("st07/nonmatchings/st07", St07Board_draw);

/* BallShootController's destructor: Controller's. */
void BallShootController_destroy(BallShootController *this, s32 flags) {
    Controller_destroy(&this->base, flags);
}
