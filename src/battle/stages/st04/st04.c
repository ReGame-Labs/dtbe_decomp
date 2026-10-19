#include "common.h"
#include <abs.h>
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "psyq.h"
#include "gte.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "engine/system/memory.h"

/* the virtual tables the functions below install, and the data they read */
extern VtableEntry STAGE_HIT_VTABLE[];
extern EntityVtable ST04_GLOW_VTABLE;
extern EntityVtable D_800D395C;
extern EntityVtable ST04_PLUNGE_VTABLE;
extern EntityVtable ST04_SIDE_SQUARE_VTABLE;
extern EntityVtable D_800D39D4;
extern EntityVtable D_800D39FC;
extern EntityVtable D_800D3A24;

/* Arena 4's stage, "Iceberg" in STAGE_NAMES (StageIceberg_init, virtual table STAGE_ICEBERG_VTABLE): a Fight whose
 * scenery scrolls and cycles textures, swings mesh nodes and throws objects; a fighter who goes below the water
 * (y 0) plunges (st04_checkPlunge) and is lifted back (st04_tickLiftBack). */
typedef struct StageIceberg {
    /* 0x000 */ Fight base;
    /* 0xD38 */ s32 frames; /* the scenery's frames (StageIceberg_tickScenery) */
    /* 0xD3C */ s32 roundsStarted; /* StageIceberg_startRound copies VRAM with func_800D5598 the first time */
    /* 0xD40 */ RECT imageRects[2]; /* two 32 by 32 texel 8-bit images in VRAM (func_800D3F04) */
    /* 0xD50 */ u8 images[2][32][32]; /* sourceImages with each row shifted, loaded each frame (func_800D3FC8) */
    /* 0x1550 */ u8 sourceImages[2][32][32]; /* the images as read back from VRAM (func_800D3F04) */
    /* 0x1D50 */ u8 unk1D50[0x1D68 - 0x1D50];
    /* 0x1D68 */ RECT clutRect;     /* 16 by 1 pixels at (0x330, 0xE6): a CLUT */
    /* 0x1D70 */ u16 shownClut[16]; /* clut with entry 15 pulsing, loaded to VRAM each frame (st04_loadPulsingClut) */
    /* 0x1D90 */ u16 clut[16]; /* that CLUT, read back from VRAM by st04_readClut */
    /* 0x1DB0 */ u8 unk1DB0[0x1DC8 - 0x1DB0];
    /* 0x1DC8 */ MeshScene *meshScene; /* Fight_getMeshScene's, at init (StageIceberg_startRound) */
    /* 0x1DCC */ VECTOR fighterPositions[2]; /* by side, each frame (StageIceberg_tickScenery) */
    /* 0x1DEC */ s32 unk1DEC[2]; /* by side, as the next two (st04_tickLiftBack) */
    /* 0x1DF4 */ s32 unk1DF4[2];
    /* 0x1DFC */ VECTOR unk1DFC[2];
    /* 0x1E1C */ s32 plunged[2]; /* by side: set once st04_checkPlunge has made the side's St04Plunge */
    /* the state of the six parts func_800D4BE0 sets up (func_800D4DEC resets one) */
    /* 0x1E24 */ s32 unk1E24[6];
    /* 0x1E3C */ s32 unk1E3C[6];
    /* 0x1E54 */ s16 unk1E54[6];
    /* 0x1E60 */ u16 unk1E60[6]; /* a random scale, 0 to 0xFFF (Fight_random) */
    /* 0x1E6C */ VECTOR partPositions[6]; /* starts at the next one; st04_stepPart makes it fall */
    /* 0x1ECC */ VECTOR unk1ECC[6]; /* D_800D3824's row at set-up */
    /* 0x1F2C */ VECTOR partForces[6]; /* gravity 0x71C a step (st04_stepPart) */
    /* 0x1F8C */ VECTOR partVelocities[6]; /* y cleared when the part lands */
    /* 0x1FEC */ s32 fallStates[2]; /* by side: the state func_800D58C0 switches on (0 to 4); 0 at round start */
    /* the per-side state func_800D56E4 resets */
    /* 0x1FF4 */ s32 unk1FF4[2];
    /* 0x1FFC */ s16 unk1FFC[2]; /* 0x1000 at reset, less mass >> 13 of each fighter in its area (func_800D5770) */
    /* 0x2000 */ s16 unk2000[2];
    /* 0x2004 */ s16 unk2004[2];
    /* by side, what func_800D58C0 drops (st04_stepFall steps it, func_800D56E4 resets it): */
    /* 0x2008 */ VECTOR fallPosition[2]; /* reset to fallHome */
    /* 0x2028 */ VECTOR fallHome[2];
    /* 0x2048 */ VECTOR fallForce[2];    /* gravity 0x71C a step */
    /* 0x2068 */ VECTOR fallVelocity[2]; /* y cleared below the floor */
    /* 0x2088 */ u16 continueShown; /* Fight_isContinueShown, each frame (StageIceberg_tickScenery) */
} StageIceberg;

/* A square around a side's fighter, which follows him (St04SideSquare_init, virtual table ST04_SIDE_SQUARE_VTABLE). */
typedef struct St04SideSquare {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageIceberg *stage;
    /* 0x1C */ u16 side;
    /* 0x20 */ u32 color; /* of the drawn square: 0xF08080 at construction */
    /* 0x24 */ Vec3 position; /* the fighter's (func_8007A650), each tick */
    /* 0x30 */ u8 unk30[0x34 - 0x30];
    /* 0x34 */ SVECTOR corners[4]; /* in the z = 0 plane */
} St04SideSquare;

/* The class of vtable D_800D39D4 (func_800D6A9C), made by func_800D4FE0: a quad standing at a position, whose top
 * edge func_800D6BB8 moves by state. */
typedef struct Unk800D6A9C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ u8 unk28; /* 0x80 at construction; func_800D6BB8 fades it into unk29 */
    /* 0x29 */ u8 unk29;
    /* 0x2A */ u16 state; /* func_800D6BB8's switch */
    /* 0x2C */ u16 timer; /* frames in the state */
    /* 0x2E */ u16 unk2E; /* set to 1 and 2 by func_800D6BB8, read by the draw */
    /* 0x30 */ SVECTOR unk30; /* the middle of the bottom edge */
    /* 0x38 */ SVECTOR corners[4]; /* top left, top right, bottom left, bottom right */
} Unk800D6A9C;

extern VtableEntry STAGE_ICEBERG_VTABLE[];

/* Constructs the stage: Fight, with virtual table STAGE_ICEBERG_VTABLE. */
StageIceberg *StageIceberg_init(StageIceberg *this) {
    Fight_init(&this->base);
    this->base.vtable = STAGE_ICEBERG_VTABLE;
    this->roundsStarted = 0;
    return this;
}

/* The stage's destructor: virtual table STAGE_ICEBERG_VTABLE, then Fight's destructor. */
void StageIceberg_destroy(StageIceberg *this, s32 flags) {
    this->base.vtable = STAGE_ICEBERG_VTABLE;
    Fight_destroy(&this->base, flags);
}

/* the table of createItemSpawner's entity, in the stage's .data */
extern u8 D_800D89A8[];

/* what the stage's init calls */
void st04_copyTexturesB(StageIceberg *this);
void st04_copyTexturesA(StageIceberg *this);
void func_800D3F04(StageIceberg *this);
void st04_readClut(StageIceberg *this);
void st04_clearPlunged(StageIceberg *this);
void st04_clearLiftBack(StageIceberg *this);
void func_800D42B4(StageIceberg *this);
void func_800D43D4(StageIceberg *this);
void func_800D4BE0(StageIceberg *this);
void func_800D5598(StageIceberg *this);
Entity *func_800D6520(Entity *this, StageIceberg *stage, s32 side);
Entity *St04Glow_init(Entity *this, StageIceberg *stage);

/* The stage's init: Fight_startRound, Fight_setBounds's bounds, its own state and VRAM copies (func_800D5598 only
 * the first time, counted in roundsStarted), the two func_800D6520 entities (0 through Fight_addStageEntity, 1 through
 * Fight_addEntity), the pulsing St04Glow and createItemSpawner's entity for D_800D89A8.
 * The match depends on the first two sharing `entity` and the third having a block variable of its own: a
 * variable set again later keeps the copy of the new value and the test after the constructor (a plain move to
 * v0 the first time, s0 the last time), a block-local one lets the allocation test jump past that test. */
void StageIceberg_startRound(StageIceberg *this) {
    Entity *entity;

    Fight_startRound(&this->base);
    Fight_setBounds(&this->base, -0x1000, 0x1000, -0x2000, 0x320, 0);
    this->frames = 0;
    this->continueShown = 0;
    this->meshScene = Fight_getMeshScene(&this->base);
    func_800D4BE0(this);
    st04_copyTexturesA(this);
    if (this->roundsStarted == 0) {
        func_800D5598(this);
    }
    func_800D3F04(this);
    st04_readClut(this);
    st04_clearLiftBack(this);
    func_800D42B4(this);
    func_800D43D4(this);
    st04_clearPlunged(this);
    st04_copyTexturesB(this);
    entity = NEW_ENTITY(0x320, func_800D6520, this, 0);
    if (entity != NULL) {
        Fight_addStageEntity(&this->base, entity);
    }
    entity = NEW_ENTITY(0x320, func_800D6520, this, 1);
    if (entity != NULL) {
        Fight_addEntity(&this->base, entity);
    }
    {
        Entity *pulse = NEW_ENTITY(0x3C8, St04Glow_init, this);

        if (pulse != NULL) {
            Fight_addStageEntity(&this->base, pulse);
        }
    }
    createItemSpawner(&this->base, D_800D89A8, 2);
    this->roundsStarted++;
}

/* the scenery's own steps */
void st04_scrollTexture(StageIceberg *this);
void func_800D3FC8(StageIceberg *this);
void st04_loadPulsingClut(StageIceberg *this);
void st04_checkPlunge(StageIceberg *this);
void func_800D44D4(StageIceberg *this);
void func_800D4700(StageIceberg *this);
void st04_tickLiftBack(StageIceberg *this);
void func_800D4FE0(StageIceberg *this);
void func_800D58C0(StageIceberg *this);

/* The stage's override of Fight_tickScenery: the base step, both fighters' positions, continueShown; unless the
 * continue prompt is shown or func_8006F144 holds before the round is over, its nine steps, counted in frames
 * (st00's StageMachine_tickScenery). */
void StageIceberg_tickScenery(StageIceberg *this) {
    Fight_tickScenery(&this->base);
    Fighter_getPosition(Fight_getFighter(&this->base, 0), (Vec3 *)&this->fighterPositions[0]);
    Fighter_getPosition(Fight_getFighter(&this->base, 1), (Vec3 *)&this->fighterPositions[1]);
    if (Fight_isContinueShown(&this->base)) {
        this->continueShown = 1;
    } else {
        this->continueShown = 0;
        if (!func_8006F144(&this->base) || Fight_isRoundOver(&this->base)) {
            func_800D4FE0(this);
            func_800D58C0(this);
            st04_scrollTexture(this);
            func_800D3FC8(this);
            st04_loadPulsingClut(this);
            st04_tickLiftBack(this);
            st04_checkPlunge(this);
            func_800D4700(this);
            func_800D44D4(this);
            this->frames++;
        }
    }
}

/* Copies the 16 by 32 pixels at (0x2E0, 0x80) and at (0x2E0, 0xC0) in VRAM to (0x2E0, 0xA0) and (0x2E0, 0xE0);
 * the stage's init calls it. */
void st04_copyTexturesB(StageIceberg *this) {
    RECT rect;

    rect.x = 0x2E0;
    rect.y = 0x80;
    rect.w = 16;
    rect.h = 32;
    MoveImage(&rect, 0x2E0, 0xA0);
    rect.x = 0x2E0;
    rect.y = 0xC0;
    rect.w = 16;
    rect.h = 32;
    MoveImage(&rect, 0x2E0, 0xE0);
}

/* Copies the 16 by 64 pixels at (0x358, 0x30) in VRAM to (0x2E0, 0) and to (0x2E0, 0x40); the stage's init
 * calls it first. */
void st04_copyTexturesA(StageIceberg *this) {
    RECT rect;

    rect.x = 0x358;
    rect.y = 0x30;
    rect.w = 16;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0);
    rect.x = 0x358;
    rect.y = 0x30;
    rect.w = 16;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0x40);
}

/* On even ticks, copies the 16 by 64 pixels at (0x2E0, 63 - update % 64) in VRAM to (0x358, 0x30): a
 * texture that scrolls one line per two ticks. The even test is written "(count ^ 1) & 1" (xori, andi). */
void st04_scrollTexture(StageIceberg *this) {
    s32 count = this->frames;
    RECT rect;

    if ((count ^ 1) & 1) {
        rect.x = 0x2E0;
        rect.y = ~count & 0x3F;
        rect.w = 16;
        rect.h = 64;
        MoveImage(&rect, 0x358, 0x30);
    }
}

/* Reads the two images at (0x340, 0x90) and (0x340, 0xB0) back from VRAM into sourceImages. */
void func_800D3F04(StageIceberg *this) {
    s32 i;

    for (i = 0; i < 2; i++) {
        this->imageRects[i].x = 0x340;
        this->imageRects[i].y = 0x90 + i * 0x20;
        this->imageRects[i].w = 0x10;
        this->imageRects[i].h = 0x20;
        StoreImage(&this->imageRects[i], (u_long *)this->sourceImages[i]);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D3FC8);

/* Reads the 16-colour CLUT at (0x330, 0xE6) back from VRAM into clut. */
void st04_readClut(StageIceberg *this) {
    this->clutRect.x = 0x330;
    this->clutRect.y = 0xE6;
    this->clutRect.w = 16;
    this->clutRect.h = 1;
    StoreImage(&this->clutRect, (u_long *)this->clut);
}

/* Loads the CLUT back to VRAM with entry 15 pulsing: a grey-blue level that follows the sine of frames * 64. The
 * copy goes a word (two colours) at a time; the match depends on the do/while and on end set before dst. */
void st04_loadPulsingClut(StageIceberg *this) {
    u32 *src = (u32 *)this->clut;
    u32 *end = (u32 *)&this->clut[16];
    u32 *dst = (u32 *)this->shownClut;
    u8 level;

    do {
        *dst++ = *src++;
    } while (src != end);
    level = (COS_SIN_TABLE[(this->frames << 6) & 0xFFF].sin >> 9) + 0x10;
    this->shownClut[15] = level | ((level >> 2) << 5) | ((level >> 1) << 10);
    LoadImage(&this->clutRect, (u_long *)this->shownClut);
}

/* Clears the stage's plunged flags. */
void st04_clearPlunged(StageIceberg *this) {
    this->plunged[1] = 0;
    this->plunged[0] = 0;
}

/* The splash of a fighter who goes below the water: four quads 0x200 wide around random points near where he went
 * in. */
typedef struct St04Plunge {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageIceberg *stage;
    /* 0x1C */ SVECTOR origin; /* the fighter's x and z when he went in, y 0 */
    /* 0x24 */ u16 side;
    /* 0x26 */ u16 frames;
    /* 0x28 */ s16 unk28[4];
    /* 0x30 */ u8 shades[4]; /* the draw's color, 0x80 at first */
    /* 0x34 */ s16 unk34[4]; /* random, 0x1F to 0x22 */
    /* 0x3C */ s16 unk3C[4];
    /* 0x44 */ GridVertex centers[4]; /* within 0x80 of the origin in x and z; word-aligned like the grids' */
    /* 0x64 */ GridVertex corners[4][4];
} St04Plunge; /* size 0xE4 */

St04Plunge *St04Plunge_init(St04Plunge *this, StageIceberg *stage, u16 side);

/* The sound of a fighter who goes below the water. */
#define ST04_SOUND_PLUNGE 0x64

/* A stage step: once per side, when the fighter goes below the water (his y not negative), adds an St04Plunge for
 * that side to the stage and plays ST04_SOUND_PLUNGE. */
void st04_checkPlunge(StageIceberg *this) {
    s32 side;

    for (side = 0; side < 2; side++) {
        if (this->plunged[side] == 0 && this->fighterPositions[side].vy >= 0) {
            Entity *entity = NEW_ENTITY(0xE4, St04Plunge_init, this, side);

            if (entity != NULL) {
                Fight_addStageEntity(&this->base, entity);
            }
            soundControlPlaySound(&SOUND_CONTROL, ST04_SOUND_PLUNGE, 0);
            this->plunged[side] = 1;
        }
    }
}

/* Clears st04_tickLiftBack's state: the per-side words at 0x1DEC and 0x1DF4 and the two vectors at 0x1DFC. */
void st04_clearLiftBack(StageIceberg *this) {
    /* The match depends on the chained clears: the game stores [1] before [0]. */
    this->unk1DEC[0] = this->unk1DEC[1] = 0;
    this->unk1DF4[0] = this->unk1DF4[1] = 0;
    this->unk1DFC[0].vx = 0;
    this->unk1DFC[0].vy = 0;
    this->unk1DFC[0].vz = 0;
    this->unk1DFC[0].pad = 0;
    this->unk1DFC[1].vx = 0;
    this->unk1DFC[1].vy = 0;
    this->unk1DFC[1].vz = 0;
    this->unk1DFC[1].pad = 0;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D42B4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D43D4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D44D4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4700);

St04SideSquare *St04SideSquare_init(St04SideSquare *this, StageIceberg *stage, u16 side);

/* Sets the three coordinates of a VECTOR. The match depends on this helper being a function: its arguments
 * (the address, then the values) are all evaluated before the stores, the address as base + index + offset. */
static inline void setVectorXYZ(VECTOR *v, s32 x, s32 y, s32 z) {
    v->vx = x;
    v->vy = y;
    v->vz = z;
}

/* A stage step, per side: state 0 waits for the fighter's y not to be negative, then runs
 * Fighter_takeHit on it, adds an St04SideSquare for the side and keeps its position at 0x1DFC; state 1 moves
 * that position by twice the frame count (0x1DF4) each frame and puts the character there, and ends after 60
 * frames. JP puts it there with Fighter_setPosition, Fighter_hold and func_80079AE4; US with Fight_unfreezeFighters and
 * Fighter_setPosition, unless Fight_isRoundOver holds, which ends state 1 at once. */
void st04_tickLiftBack(StageIceberg *this) {
    s32 side;

    for (side = 0; side < 2; side++) {
#if VERSION_US
        Fighter *fighter = Fight_getFighter(&this->base, side);

#endif
        switch (this->unk1DEC[side]) {
        case 0:
            if (this->fighterPositions[side].vy < 0) {
                break;
            }
            Fighter_takeHit(Fight_getFighter(&this->base, side), 0x2000001, (Vec3 *)&ZERO_VECTOR, 0);
            {
                Entity *entity = NEW_ENTITY(0x54, St04SideSquare_init, this, side);

                if (entity != NULL) {
                    Fight_addEntity(&this->base, entity);
                }
            }
            this->unk1DF4[side] = 0;
            setVectorXYZ(&this->unk1DFC[side], this->fighterPositions[side].vx, 0, this->fighterPositions[side].vz);
            this->unk1DEC[side]++;
            /* fallthrough */
        case 1: {
#if VERSION_US
            this->unk1DFC[side].vy += this->unk1DF4[side] * 2;
            if (!Fight_isRoundOver(&this->base)) {
                Fight_unfreezeFighters(&this->base);
                Fighter_setPosition(fighter, (Vec3 *)&this->unk1DFC[side]);
            } else {
                this->unk1DEC[side]++;
            }
#else
            Fighter *fighter;
            Vec3 position;

            this->unk1DFC[side].vy += this->unk1DF4[side] * 2;
            fighter = Fight_getFighter(&this->base, side);
            position.vx = this->unk1DFC[side].vx;
            position.vy = this->unk1DFC[side].vy;
            position.vz = this->unk1DFC[side].vz;
            Fighter_setPosition(fighter, &position);
            Fighter_hold(Fight_getFighter(&this->base, side), 1);
            func_80079AE4(Fight_getFighter(&this->base, side));
#endif
            if (++this->unk1DF4[side] >= 60) {
                this->unk1DEC[side]++;
            }
            break;
        }
        }
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4BE0);

/* Clears the four words of a VECTOR. The match depends on this helper: it takes the vector's address once,
 * where the fields written out each add the offset to their store. */
static inline void clearVector(VECTOR *v) {
    v->vx = 0;
    v->vy = 0;
    v->vz = 0;
    v->pad = 0;
}

/* Resets one part's state: its force, velocity and three counters cleared, its position set back to 0x1ECC. */
void func_800D4DEC(StageIceberg *this, s32 part) {
    clearVector(&this->partForces[part]);
    clearVector(&this->partVelocities[part]);
    this->unk1E60[part] = 0;
    this->unk1E54[part] = 0;
    this->unk1E3C[part] = 0;
    this->partPositions[part].vx = this->unk1ECC[part].vx;
    this->partPositions[part].vy = this->unk1ECC[part].vy;
    this->partPositions[part].vz = this->unk1ECC[part].vz;
}

/* Whether a fighter is under a part: within 0x100 of it in x and between its y and 0x200 below. Part 5 rides on
 * side 1's fall (its position is offset by that fallPosition) and counts only while that fall's state is below 4. */
s32 st04_isFighterUnderPart(StageIceberg *this, s32 part) {
    s32 i;

    if (part != 5) {
        for (i = 0; i < 2; i++) {
            if (ABS(this->fighterPositions[i].vx - this->partPositions[part].vx) < 0x100
                && ABS(this->fighterPositions[i].vy - (this->partPositions[part].vy + 0x100)) < 0x100) {
                return 1;
            }
        }
    } else {
        if (this->fallStates[1] >= 4) {
            return 0;
        }
        for (i = 0; i < 2; i++) {
            if (ABS(this->fighterPositions[i].vx - (this->partPositions[part].vx + this->fallPosition[1].vx)) < 0x100
                && ABS(this->fighterPositions[i].vy - (this->partPositions[part].vy + 0x100 + this->fallPosition[1].vy))
                       < 0x100) {
                return 1;
            }
        }
    }
    return 0;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4FE0);

/* One fall step of a part (st04_stepFall's twin, with a mass of 2): gravity into its force, the force into the
 * velocity and the step (in 256ths) into the position. Then the segment from the old position to 0x100 below the new
 * one (for part 5, from its position plus side 1's fallPosition down 0x100) is cast at the map (func_80072FE0); on a
 * hit the part goes back to the segment's start and its fall stops. Returns whether it hit. */
s32 st04_stepPart(StageIceberg *this, s32 part) {
    Vec3 start;
    Vec3 end;
    Vec3 step;

    start.vx = this->partPositions[part].vx;
    start.vy = this->partPositions[part].vy;
    start.vz = 0;
    this->partForces[part].vy += 0x71C;
    func_80079C1C(&step, (Vec3 *)&this->partVelocities[part], (Vec3 *)&this->partForces[part], 0x20000);
    clearVector(&this->partForces[part]);
    this->partPositions[part].vx += step.vx >> 8;
    this->partPositions[part].vy += step.vy >> 8;
    if (part != 5) {
        end.vx = this->partPositions[part].vx;
        end.vy = this->partPositions[part].vy + 0x100;
    } else {
        start.vx = this->partPositions[5].vx + this->fallPosition[1].vx;
        start.vy = this->partPositions[5].vy + this->fallPosition[1].vy;
        end.vx = this->partPositions[5].vx + this->fallPosition[1].vx;
        /* The match depends on this order of the sum (fold turns it into pos + (fall + 0x100)). */
        end.vy = this->partPositions[5].vy + 0x100 + this->fallPosition[1].vy;
    }
    end.vz = 0;
    if (func_80072FE0(&start, &end)) {
        this->partPositions[part].vx = start.vx;
        this->partPositions[part].vy = start.vy;
        this->partVelocities[part].vy = 0;
        return 1;
    }
    return 0;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5598);

/* Resets one side's other state: the fall's force and velocity and 0x1FF4 cleared, 0x2004 and 0x1FFC set to
 * 0x1000, the fall's position set back to its home. */
void func_800D56E4(StageIceberg *this, s32 side) {
    clearVector(&this->fallForce[side]);
    clearVector(&this->fallVelocity[side]);
    this->unk2004[side] = 0x1000;
    this->unk2000[side] = 0;
    this->unk1FF4[side] = 0;
    this->unk1FFC[side] = 0x1000;
    this->fallPosition[side].vx = this->fallHome[side].vx;
    this->fallPosition[side].vy = this->fallHome[side].vy;
    this->fallPosition[side].vz = this->fallHome[side].vz;
}

extern VECTOR D_800D89E4[2][2];

/* Lowers side's 0x1FFC by mass >> 13 of each fighter on the ground in the side's area of D_800D89E4 (in x past
 * [side][0] toward the side, in y between [side][1] and [side][0]) and returns it. */
s32 func_800D5770(StageIceberg *this, s32 side) {
    s32 level = this->unk1FFC[side];
    s32 i;

    for (i = 0; i < 2; i++) {
        if (side == 0) {
            if (this->fighterPositions[i].vx > D_800D89E4[side][0].vx) {
                continue;
            }
        } else if (this->fighterPositions[i].vx < D_800D89E4[side][0].vx) {
            continue;
        }
        if (this->fighterPositions[i].vy > D_800D89E4[side][0].vy
            || this->fighterPositions[i].vy < D_800D89E4[side][1].vy) {
            continue;
        }
        if (Fighter_isOnGround(Fight_getFighter(&this->base, i))) {
            level -= Fight_getFighter(&this->base, i)->mass >> 13;
        }
    }
    this->unk1FFC[side] = level;
    return level;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D58C0);

/* One fall step of a side's fallPosition: gravity into the force, the force over a mass of 3 into the velocity and
 * the step (in 256ths) into the position. Returns whether it is below the floor (y > 0), stopping its fall there. */
s32 st04_stepFall(StageIceberg *this, s32 side) {
    /* The match depends on landed's initialisation here (the game keeps it in a saved register across the call). */
    Vec3 step;
    s32 landed = 0;

    this->fallForce[side].vy += 0x71C;
    func_80079C1C(&step, (Vec3 *)&this->fallVelocity[side], (Vec3 *)&this->fallForce[side], 0x30000);
    clearVector(&this->fallForce[side]);
    this->fallPosition[side].vx += step.vx >> 8;
    this->fallPosition[side].vy += step.vy >> 8;
    this->fallPosition[side].vz += step.vz >> 8;
    if (this->fallPosition[side].vy > 0) {
        this->fallVelocity[side].vy = 0;
        landed = 1;
    }
    return landed;
}

/* The entity class of virtual table D_800D3A24 (func_800D5F94, 0xE4 bytes): four 0x80-wide quads around a position,
 * each at a random x within 0x40 of it. */
typedef struct Unk800D5F94 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR position; /* cut to 16 bits at construction */
    /* 0x28 */ u16 states[4];   /* by quad: drawn while below 2 */
    /* 0x30 */ s16 greys[4];    /* by quad: its color's grey (the low byte) */
    /* 0x38 */ s16 unk38;
    /* 0x3A */ s16 unk3A[4];    /* by quad: 3 to 10 at random */
    /* 0x42 */ s16 unk42;
    /* 0x44 */ SVECTOR centers[4];
    /* 0x64 */ SVECTOR corners[4][4];
} Unk800D5F94; /* size 0xE4 */

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5F94);

/* The destructor of the entity class of vtable D_800D3A24, which func_800D5F94 installs. */
void func_800D60E8(Entity *this, s32 flags) {
    this->vtable = &D_800D3A24;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D613C);

/* Unk800D5F94's draw, when a sphere of radius 0x5A at its position is in view: each quad whose state is below 2,
 * in its grey, with the 32 by 32 picture at (0, 0x20) of the texture page at x 0x200 and the CLUT at (0x230, 0xF6),
 * semi-transparent (abr 3: a quarter of it added). */
void func_800D6384(Unk800D5F94 *this, Camera *camera) {
    PolyFT4Shape shape;
    u8 grey;
    s32 i;

    shape.flags = 0;
    if (cameraIsSphereVisible(camera, &this->position, 0x5A)) {
        gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
        UV_BYTES(shape.uv0)[0] = 0;
        UV_BYTES(shape.uv0)[1] = 0x20;
        UV_BYTES(shape.uv1)[0] = 0x1F;
        UV_BYTES(shape.uv1)[1] = 0x20;
        UV_BYTES(shape.uv2)[0] = 0;
        UV_BYTES(shape.uv2)[1] = 0x3F;
        UV_BYTES(shape.uv3)[0] = 0x1F;
        UV_BYTES(shape.uv3)[1] = 0x3F;
        shape.tpage = getTPage(0, 3, 0x200, 0);
        shape.clut = getClut(0x230, 0xF6);
        shape.flags |= SPRITE_SEMI_TRANS | 3;
        for (i = 0; i < 4; i++) {
            if (this->states[i] < 2) {
                shape.corners[0] = &this->corners[i][0];
                shape.corners[1] = &this->corners[i][1];
                shape.corners[2] = &this->corners[i][2];
                shape.corners[3] = &this->corners[i][3];
                grey = this->greys[i];
                shape.color = grey | grey << 8 | grey << 16;
                PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
            }
        }
    }
}

/* The entity class func_800D6520 constructs (virtual table D_800D39FC), one per side (StageIceberg_init): 32 points
 * with their own phases, and a state its tick (func_800D66D0) steps. */
typedef struct Unk800D6520 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ StageIceberg *stage;
    /* 0x01C */ u16 timer; /* the state's frames (0x200 in state 0, 0x400 in state 2) */
    /* 0x01E */ u16 state; /* 0 to 3 (func_800D66D0) */
    /* 0x020 */ VECTOR unk20; /* random x and y in state 1, which unk30 eases toward */
    /* 0x030 */ VECTOR unk30;
    /* 0x040 */ u16 phases[32];
    /* 0x080 */ u8 greys[32]; /* each point's grey (func_800D6938) */
    /* 0x0A0 */ s16 unkA0[32]; /* 8 to 39 at random */
    /* 0x0E0 */ s16 unkE0[32]; /* each point's start x */
    /* 0x120 */ VECTOR points[32];
} Unk800D6520; /* size 0x320 */

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6520);

/* The destructor of the entity class of vtable D_800D39FC, which func_800D6520 installs. */
void func_800D667C(Entity *this, s32 flags) {
    this->vtable = &D_800D39FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D66D0);

extern SVECTOR D_800D38B0[4];

/* func_800D6520's draw: unless the continue prompt is shown, each of the 32 points as a square with the 32 by 32
 * picture at (0, 0) of the texture page at x 0x200 and the CLUT at (0x230, 0xFF), added in the point's grey.
 * The match depends on the camera parameter typed Camera * and on matrix taken inside the loop (loop hoists it into
 * a saved register after i's start; the translation is stored straight into m). */
void func_800D6938(Unk800D6520 *this, Camera *camera) {
    MatrixWords m;
    MATRIX *matrix;
    PolyFT4Shape shape;
    VECTOR *point;
    s32 x, y, z;
    s32 i;

    if (this->stage->continueShown == 0) {
        shape.corners[0] = &D_800D38B0[0];
        shape.corners[1] = &D_800D38B0[1];
        shape.corners[2] = &D_800D38B0[2];
        shape.corners[3] = &D_800D38B0[3];
        UV_BYTES(shape.uv0)[0] = 0;
        UV_BYTES(shape.uv0)[1] = 0;
        UV_BYTES(shape.uv1)[0] = 0x1F;
        UV_BYTES(shape.uv1)[1] = 0;
        UV_BYTES(shape.uv2)[0] = 0;
        UV_BYTES(shape.uv2)[1] = 0x1F;
        UV_BYTES(shape.uv3)[0] = 0x1F;
        UV_BYTES(shape.uv3)[1] = 0x1F;
        shape.tpage = getTPage(0, 1, 0x200, 0);
        shape.clut = getClut(0x230, 0xFF);
        shape.flags = SPRITE_SEMI_TRANS | 1;
        for (i = 0; i < 32; i++) {
            u8 grey;

            matrix = &m.m;
            point = &this->points[i];
            x = point->vx;
            y = point->vy;
            z = point->vz;
            m.words[0] = ONE;
            m.words[1] = 0;
            m.words[2] = ONE;
            m.words[3] = 0;
            m.words[4] = ONE;
            m.m.t[0] = x;
            m.m.t[1] = y;
            m.m.t[2] = z;
            setModelMatrix(matrix);
            grey = this->greys[i];
            shape.color = grey | grey << 8 | grey << 16;
            PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
        }
    }
}

/* Constructs an Unk800D6A9C at a part's position (the stage is not kept): a 0x200 by 0x200 quad whose bottom edge
 * is 0x180 below the position. The position is cut to 16 bits, as the quad's SVECTORs hold it. */
Unk800D6A9C *func_800D6A9C(Unk800D6A9C *this, StageIceberg *stage, VECTOR position) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D39D4;
    this->state = this->timer = 0;
    this->unk2E = 0;
    this->position.vx = (s16)position.vx;
    this->position.vy = (s16)position.vy;
    this->position.vz = (s16)position.vz;
    this->unk28 = 0x80;
    this->unk29 = 0;
    this->unk30.vx = this->position.vx;
    this->unk30.vy = this->position.vy + 0x180;
    this->unk30.vz = this->position.vz;
    this->corners[0].vx = this->corners[2].vx = this->position.vx - 0x100;
    this->corners[1].vx = this->corners[3].vx = this->position.vx + 0x100;
    this->corners[0].vy = this->corners[1].vy = this->unk30.vy - 0x200;
    this->corners[2].vy = this->corners[3].vy = this->unk30.vy;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = this->unk30.vz;
    return this;
}

/* The destructor of the entity class of vtable D_800D39D4, which func_800D6A9C installs. */
void func_800D6B64(Entity *this, s32 flags) {
    this->vtable = &D_800D39D4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6BB8);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6DC0);

/* Constructs an St04SideSquare for a side: its corners are +-2 * 0x2 of the character's record.
 * The match depends on the record taken as "table + index" (an index or a record pointer local swaps
 * the address registers) and on the chained corner stores (the game stores the last first). */
St04SideSquare *St04SideSquare_init(St04SideSquare *this, StageIceberg *stage, u16 side) {
    Vec3 position;
    s32 size;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST04_SIDE_SQUARE_VTABLE;
    this->stage = stage;
    this->side = side;
    this->color = 0xF08080;
    func_8007A650(Fight_getFighter(&stage->base, side), &position);
    size = (FIGHTER_PARAMS + Fighter_getCharacter(Fight_getFighter(&this->stage->base, this->side)))->height * 2;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->corners[0].vx = this->corners[2].vx = -size;
    this->corners[1].vx = this->corners[3].vx = size;
    this->corners[0].vy = this->corners[1].vy = -size;
    this->corners[2].vy = this->corners[3].vy = size;
    return this;
}

/* St04SideSquare's destructor. */
void St04SideSquare_destroy(Entity *this, s32 flags) {
    this->vtable = &ST04_SIDE_SQUARE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", St04SideSquare_tick);

/* St04SideSquare's draw: unless the continue prompt is shown, the square at the fighter's position, with the 64 by
 * 64 picture at (0x20, 0) of the texture page at x 0x200 and the CLUT at (0x230, 0xFC), semi-transparent. */
void St04SideSquare_draw(St04SideSquare *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    MatrixWords m;
    PolyFT4Shape shape;
    Vec3 *position;
    s32 x, y, z;

    if (this->stage->continueShown == 0) {
        position = &this->position;
        x = position->vx;
        y = position->vy;
        z = position->vz;
        m.words[0] = ONE;
        m.words[1] = 0;
        m.words[2] = ONE;
        m.words[3] = 0;
        m.words[4] = ONE;
        m.m.t[0] = x;
        m.m.t[1] = y;
        m.m.t[2] = z;
        setModelMatrix(&m.m);
        shape.corners[0] = &this->corners[0];
        shape.corners[1] = &this->corners[1];
        shape.corners[2] = &this->corners[2];
        shape.corners[3] = &this->corners[3];
        UV_BYTES(shape.uv0)[0] = 0x20;
        UV_BYTES(shape.uv0)[1] = 0;
        UV_BYTES(shape.uv1)[0] = 0x5F;
        UV_BYTES(shape.uv1)[1] = 0;
        UV_BYTES(shape.uv2)[0] = 0x20;
        UV_BYTES(shape.uv2)[1] = 0x3F;
        UV_BYTES(shape.uv3)[0] = 0x5F;
        UV_BYTES(shape.uv3)[1] = 0x3F;
        shape.tpage = getTPage(0, 0, 0x200, 0);
        shape.clut = getClut(0x230, 0xFC);
        shape.flags = SPRITE_SEMI_TRANS;
        shape.color = this->color;
        PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + 1);
    }
}

/* Constructs the splash for the side's fighter at his x and z, with its quads at random points around them. */
St04Plunge *St04Plunge_init(St04Plunge *this, StageIceberg *stage, u16 side) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST04_PLUNGE_VTABLE;
    this->side = side;
    this->stage = stage;
    this->origin.vx = stage->fighterPositions[this->side].vx;
    this->origin.vz = stage->fighterPositions[this->side].vz;
    this->origin.vy = 0;
    this->frames = 0;
    for (i = 0; i < 4; i++) {
        this->unk28[i] = 0;
        this->unk34[i] = (Fight_random(&stage->base) & 3) + 0x1F;
        this->shades[i] = 0x80;
        this->unk3C[i] = 0;
        /* The match depends on a signed random byte (with a u32 one, the sum narrowed to a halfword loads 0xFF80 into a register) and on
         * this order of the sum (fold turns it into random + (origin - 0x80)). */
        this->centers[i].vx = ((s32)Fight_random(&stage->base) & 0xFF) - 0x80 + this->origin.vx;
        this->centers[i].vy = 0;
        this->centers[i].vz = ((s32)Fight_random(&stage->base) & 0xFF) - 0x80 + this->origin.vz;
        this->corners[i][0].vx = this->corners[i][2].vx = this->centers[i].vx - 0x100;
        this->corners[i][1].vx = this->corners[i][3].vx = this->centers[i].vx + 0x100;
        this->corners[i][0].vy = this->corners[i][1].vy = this->corners[i][2].vy = this->corners[i][3].vy =
            this->centers[i].vy;
        this->corners[i][0].vz = this->corners[i][1].vz = this->corners[i][2].vz = this->corners[i][3].vz =
            this->centers[i].vz;
    }
    return this;
}

/* St04Plunge's destructor. */
void St04Plunge_destroy(Entity *this, s32 flags) {
    this->vtable = &ST04_PLUNGE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", St04Plunge_tick);

INCLUDE_ASM("st04/nonmatchings/st04", St04Plunge_draw);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7900);

/* The destructor of the entity class of vtable D_800D395C, which func_800D7900 installs. */
void func_800D7A64(Entity *this, s32 flags) {
    this->vtable = &D_800D395C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7AB8);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7CE4);

/* Semi-transparent Gouraud quads with a pulsing level (St04Glow_init, virtual table ST04_GLOW_VTABLE). */
typedef struct St04Glow {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 phase;
    /* 0x1A */ u8 level;
    /* 0x1B */ u8 unk1B;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E[0x20 - 0x1E];
    /* 0x20 */ SVECTOR vertices[3][3][13]; /* three grids of 3 rows of 13 points (St04Glow_init) */
} St04Glow; /* size 0x3C8 */

INCLUDE_ASM("st04/nonmatchings/st04", St04Glow_init);

/* St04Glow's destructor. */
void St04Glow_destroy(Entity *this, s32 flags) {
    this->vtable = &ST04_GLOW_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St04Glow's tick: the level follows a sine of the phase (0x6F +- 16), and the phase steps. */
void St04Glow_tick(St04Glow *this) {
    this->level = (COS_SIN_TABLE[(this->phase << 4) & 0xFFF].sin >> 8) + 0x6F;
    this->phase++;
}

INCLUDE_ASM("st04/nonmatchings/st04", St04Glow_draw);

/* Constructs a StageHit with damage and the knockback (0x60, 0, 0). */
StageHit *StageHit_init(StageHit *this, s32 damage) {
    Vec3 vec;

    /* The match depends on vec, a whole vector copied in, and on filling it after the vtable store. */
    this->vtable = STAGE_HIT_VTABLE;
    vec.vx = 0x60;
    vec.vy = 0;
    vec.vz = 0;
    this->damage = damage;
    this->knockback = vec;
    return this;
}

/* StageHit's slot 0x08 (collideShapes calls it with the sphere its own touched): adds its damage and knockback
 * into the sphere's. */
void StageHit_apply(StageHit *this, CollisionSphere *part) {
    s32 x, y, z;

    part->hitDamage += this->damage;
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = part->hitKnockback.vx;
    y = part->hitKnockback.vy;
    z = part->hitKnockback.vz;
    x += this->knockback.vx;
    y += this->knockback.vy;
    z += this->knockback.vz;
    part->hitKnockback.vx = x;
    part->hitKnockback.vy = y;
    part->hitKnockback.vz = z;
}

/* The entity class func_800D850C constructs (virtual table D_800D38EC) for one of the stage's six parts: a
 * StageHit at 0x18 and CollisionSphere_init's object at 0x30. */
typedef struct Unk800D850C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageHit hit;
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ CollisionSphere unk30;
    /* 0x78 */ StageIceberg *stage;
    /* 0x7C */ u16 index; /* the part, 0 to 5 */
    /* 0x7E */ s16 unk7E; /* set while unk30 is out of the stage's list (func_800D86AC) */
    /* 0x80 */ Vec3 unk80; /* the part's position (0x1E6C), 0x100 lower */
    /* 0x8C */ u8 unk8C[0x90 - 0x8C];
    /* 0x90 */ Vec3 unk90; /* (0x60, 0, 0) at construction: StageHit_init's knockback */
    /* 0x9C */ u8 unk9C[0xA0 - 0x9C];
} Unk800D850C;

extern EntityVtable D_800D38EC;

/* Constructs part index of the stage at position: a StageHit of damage 0x2D8 (and flag 0x2000000) whose knockback is
 * cleared and kept apart in unk90, the sphere of radius 0x80 owned by it, added to the stage's shapes disabled. It
 * also writes a zero VECTOR on the stack that nothing reads, which no clean C form has given yet. */
INCLUDE_ASM("st04/nonmatchings/st04", func_800D850C);

/* Unk800D850C's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D8618(Unk800D850C *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &D_800D38EC;
    this->unk30.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk30.link) written out: the match depends on this
     * access order (soundControlDestroy's); the inline gives 12 diffs */
    link = &this->unk30.link;
    this->unk30.link.next->prev = this->unk30.link.prev;
    link->prev->next = this->unk30.link.next;
    this->unk30.link.next = link;
    this->unk30.link.prev = link;
    this->hit.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D86AC);

/* StageHit's destructor: CollisionOwner's, inlined. */
void StageHit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the hit's knockback, reading all three components before writing any (as vecCopy does). */
void StageHit_setKnockback(StageHit *this, Vec3 *knockback) {
    s32 x, y, z;

    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}
