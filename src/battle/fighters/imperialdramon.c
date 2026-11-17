/* Imperialdramon (CHARACTER_IMPERIALDRAMON): the fighter createFighter builds for it, its moves (Paladin Mode's class
 * derives from it, imperialdramon_paladin_mode.c), and the entities they spawn: the special move 1's hitbox and
 * effect, the special move 2's hitbox and trail, and the special move 3's hold, effect and hitbox. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"

Entity *ImperialdramonSpecial1Effect_init(Entity *this, Fighter *fighter, VECTOR *pos, Fight *fight);
extern VECTOR D_800690E0;
extern TrailPoint D_800690F0[];
extern EntityVtable D_800691EC;
extern VtableEntry IMPERIALDRAMON_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable IMPERIALDRAMON_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable IMPERIALDRAMON_SPECIAL3_EFFECT_VTABLE;
extern VtableEntry IMPERIALDRAMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable IMPERIALDRAMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE;
extern VtableEntry IMPERIALDRAMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable IMPERIALDRAMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable D_8006933C;
extern EntityVtable D_80069364;
extern EntityVtable IMPERIALDRAMON_VTABLE;

/* The entity ImperialdramonSpecial1Effect_init constructs (virtual table IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE): ten
 * points scattered around a centre, each with an angle. */
typedef struct ImperialdramonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *fight;
    /* 0x20 */ s16 unk20;      /* the cap of unk28: 0xC00 in the air (the init's pad), else 0xF00 */
    /* 0x22 */ u8 shade;       /* 0x80, halved each tick once unk28 is reached; the draw's color */
    /* 0x24 */ s16 facing;     /* Fighter_getFacing */
    /* 0x26 */ u8 unk26[0x28 - 0x26];
    /* 0x28 */ s32 unk28;      /* eases halfway to 0x1000 each tick, up to unk20 */
    /* 0x2C */ u16 frames;
    /* 0x2E */ u16 state;
    /* 0x30 */ u16 unk30;
    /* 0x32 */ s16 vramY;      /* Fighter_getVramY */
    /* 0x34 */ s16 pointCount; /* points moved by func_8008F7F8: 4 more each tick, up to 10 */
    /* 0x36 */ u8 unk36[0x38 - 0x36];
    /* 0x38 */ VECTOR centre;  /* the init's position, then the fighter's node 12 + D_80069148 every tick; pad 0x1000 */
    /* 0x48 */ VECTOR unk48[10];
    /* 0xE8 */ s32 unkE8[10];
    /* 0x110 */ SVECTOR corners[8]; /* a quad of +-32 and a fan from (0, +-256) to 3 * 128 forward, z -32 */
} ImperialdramonSpecial1Effect;
void Imperialdramon_spawnSpecial1Hitbox(Fighter *this);
void Imperialdramon_spawnSpecial1Effect(Fighter *this, s32 air);
void Imperialdramon_spawnAttackBurst(Fighter *this);
/* the constructors of the entities spawned below */
Entity *ImperialdramonSpecial1Hitbox_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228);
Entity *ImperialdramonSpecial3Effect_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228);
struct Unk80091498 *func_80091498(struct Unk80091498 *this, Fighter *fighter, Vec3 *position, Fight *obj);
extern VECTOR D_800690D0;

/* Builds Imperialdramon: a Fighter with its own vtable and its hold state from 0x4A6 cleared. */
Imperialdramon *Imperialdramon_init(Imperialdramon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &IMPERIALDRAMON_VTABLE;
    this->special3Parts = 0;
    this->holding = 0;
    this->hitPending = 0;
    this->carryPosition.vx = 0;
    this->carryPosition.vy = 0;
    this->carryPosition.vz = 0;
    this->carryPosition.pad = 0;
    this->carryLastPosition.vx = 0;
    this->carryLastPosition.vy = 0;
    this->carryLastPosition.vz = 0;
    this->carryLastPosition.pad = 0;
    return this;
}

/* Imperialdramon's destructor. */
void Imperialdramon_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move (both classes): clears the pending hit and, while the special move 3 runs, lets the opponent go
 * (Fighter_hold(1)) and drops the hold. */
void Imperialdramon_endMove(Imperialdramon *this) {
    this->hitPending = 0;
    if (this->special3Active != 0) {
        Fighter_hold(Fighter_getOpponent(&this->base), 1);
        this->special3Active = 0;
        this->holding = 0;
    }
}

/* Dash attack (both classes): Fighter's, with dust at frame 17. */
void Imperialdramon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 17) {
        Fighter_spawnDust(this, 1, 80, 8);
    }
}

void Imperialdramon_spawnThrowSpark(Fighter *this);

/* Throw (both classes): Fighter's; frame 0 holds the opponent (7), sets clip 13 and stops both fighters; frame 7
 * if on the ground holds the opponent (46), hits and throws upward, spawning a spark; frame 20 lifts this fighter;
 * frame 34 hits downward, spawning a spark and stopping; from frame 8 to 33 brakes falling opponent. */
void Imperialdramon_throw(Fighter *this, s32 frame) {
    Fighter *opponent;
    Vec3 force;
    s32 x, y, z;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_hold(opponent, 7);
        Fighter_setClip(opponent, 13, 0, 0);
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->velocity.vz = 0;
        this->velocityPad = 0;
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        break;
    case 7:
        if (Fighter_isOnGround(this)) {
            Fighter_takeHit(opponent, 0x80000000, (Vec3 *)&ZERO_VECTOR, 0);
            force.vx = 0;
            force.vy = -0x6AA4;
            force.vz = 0;
            Fighter_hold(opponent, 46);
            Fighter_takeHit(opponent, 0x598C1A00, (Vec3 *)&ZERO_VECTOR, 1);
            x = force.vx;
            y = force.vy;
            z = force.vz;
            opponent->velocity.vy = 0;
            opponent->velocity.vx = 0;
            opponent->force.vx = x;
            opponent->force.vy = y;
            opponent->force.vz = z;
            Imperialdramon_spawnThrowSpark(this);
        } else {
            Fighter_stopActionClip(this);
            return;
        }
        break;
    case 20:
        this->force.vy -= ((s64)this->mass * 0x6388) >> 16;
        break;
    case 34:
        force.vx = 0;
        force.vy = 0x71C0;
        force.vz = 0;
        Fighter_takeHit(opponent, 0x598C1A00, (Vec3 *)&ZERO_VECTOR, 1);
        x = force.vx;
        y = force.vy;
        z = force.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
        this->velocity.vx = 0;
        Imperialdramon_spawnThrowSpark(this);
        break;
    }
    if ((u32)(frame - 8) < 26) {
        if (opponent->velocity.vy > 0) {
            opponent->force.vy -= opponent->velocity.vy;
        }
    }
}

/* Attack 1 (both classes): Fighter's; at frame 4 stops and pushes forward by mass * 0x71C0 (16.16), at frame 7 a burst;
 * from frame 5 on the force brakes a sixteenth of the horizontal velocity. The match depends on the impulse being an
 * s64 local: the game keeps both halves of the product across the call. */
void Imperialdramon_attack1(Fighter *this, s32 frame) {
    Fighter_attack1(this, frame);
    switch (frame) {
    case 4: {
        s64 impulse = ((s64)this->mass * 0x71C0) >> 16;

        this->velocity.vx = 0;
        this->force.vx += (s32)impulse * Fighter_getFacing(this);
        break;
    }
    case 7:
        Imperialdramon_spawnAttackBurst(this);
        break;
    }
    if (frame >= 5) {
        this->force.vx -= this->velocity.vx >> 4;
    }
}

/* Attack 2 (both classes): Fighter's, with a burst at frame 8. */
void Imperialdramon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 8) {
        Imperialdramon_spawnAttackBurst(this);
    }
}

/* Attack 3 (both classes): Fighter's, with dust at frame 22. */
void Imperialdramon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 22) {
        Fighter_spawnDust(this, 1, 80, 8);
    }
}

/* Special move 1 (both classes): at frame 0 the clip, the effect, func_80079B0C(30) and the voice (another one for
 * Paladin Mode); the hitbox at frame 10. */
void Imperialdramon_special1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        Imperialdramon_spawnSpecial1Effect(this, 0);
        func_80079B0C(this, 0x1E);
        if (Fighter_getCharacter(this) == CHARACTER_IMPERIALDRAMON) {
            Fighter_playVoice(this, 0x9A);
        } else {
            Fighter_playVoice(this, 0xA3);
        }
        break;
    case 10:
        Imperialdramon_spawnSpecial1Hitbox(this);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", Imperialdramon_special2);

void Imperialdramon_spawnSpecial3Effect(Fighter *this);
void Imperialdramon_spawnSpecial3Hitbox(Fighter *this);
void Imperialdramon_special3Hold(Imperialdramon *this, s32 frame);
void Imperialdramon_special3(Imperialdramon *this, s32 frame) {
    Fighter *opponent = Fighter_getOpponent(&this->base);

    func_80079AE4(&this->base);
    Imperialdramon_special3Hold(this, frame);
    switch (frame) {
    case 58: {
        s32 forceX = ((s64)this->base.mass * 0x1C70) >> 16;
        this->base.force.vx += -forceX * Fighter_getFacing(&this->base);
        this->base.force.vy -= ((s64)this->base.mass * 0x4718) >> 16;
        if (this->holding != 0) {
            Vec3 knockback;
            knockback.vx = 0;
            knockback.vy = -0x71C0;
            knockback.vz = 0;
            Fighter_takeHit(opponent, 0x5B01500, (Vec3 *)&ZERO_VECTOR, 0);
            this->carryVelocity.vy = 0;
            this->hitPending = 1;
            this->carryForce = knockback;
        }
        this->special3Parts = 0;
        break;
    }
    case 78:
#if !VERSION_JP
        this->holding = 0;
#endif
        Imperialdramon_spawnSpecial3Effect(&this->base);
        Fighter_playVoice(&this->base, 0x9C);
        break;
    case 80:
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        break;
    case 113:
        this->base.velocity.vx = 0;
        break;
    }
    if (frame >= 81 && frame < 96 && ((frame ^ 1) & 1)) {
        Imperialdramon_spawnSpecial3Hitbox(&this->base);
    }
    if (frame >= 87 && frame < 112) {
        s32 forceX = ((s64)this->base.mass * 0x1C7) >> 16;
        this->base.force.vx += -forceX * Fighter_getFacing(&this->base);
    }
}

/* Up attack in the air (both classes): Fighter's; at frame 5 a push up by mass * 0x6388 (16.16); while falling the
 * force brakes the fall. */
void Imperialdramon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    if (frame == 5) {
        this->force.vy -= ((s64)this->mass * 0x6388) >> 16;
    }
    if (this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

/* Special move 1 in the air (both classes): as on the ground, with the air clip, the air effect and func_80079B0C(20).
 */
void Imperialdramon_airSpecial1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 2, 0, -1);
        Imperialdramon_spawnSpecial1Effect(this, 1);
        func_80079B0C(this, 0x14);
        if (Fighter_getCharacter(this) == CHARACTER_IMPERIALDRAMON) {
            Fighter_playVoice(this, 0x9A);
        } else {
            Fighter_playVoice(this, 0xA3);
        }
        break;
    case 10:
        Imperialdramon_spawnSpecial1Hitbox(this);
        break;
    }
}

/* Up attack (both classes): Fighter's; a push up by mass * 0x71C0 (16.16) at frame 6, cleared at frame 16. */
void Imperialdramon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    switch (frame) {
    case 6:
        this->force.vy -= ((s64)this->mass * 0x71C0) >> 16;
        break;
    case 16:
        this->force.vy = 0;
        break;
    }
}

/* How far in front of the fighter the hold carries the opponent while none is in reach. */
#if VERSION_JP
#define CARRY_AHEAD 320
#else
#define CARRY_AHEAD 288
#endif

/* A frame of the hold's carry: carryPosition falls (0x71C a frame) by carryVelocity, never below the fighter, and stays
 * CARRY_AHEAD in front of it while no opponent is in reach; returns it in out. */
VECTOR *func_8008DD50(VECTOR *out, Imperialdramon *this) {
    VECTOR move;
    VECTOR position;
    s32 x, y, z;

    x = this->carryPosition.vx;
    y = this->carryPosition.vy;
    z = this->carryPosition.vz;
    this->carryLastPosition.vx = x;
    this->carryLastPosition.vy = y;
    this->carryLastPosition.vz = z;
    this->carryForce.vy += 0x71C;
    func_80079C1C((Vec3 *)&move, &this->carryVelocity, &this->carryForce, 0x10000);
    this->carryForce.vx = 0;
    this->carryForce.vy = 0;
    this->carryForce.vz = 0;
    this->unk4DC = 0;
    this->carryPosition.vx += move.vx >> 8;
    this->carryPosition.vy += move.vy >> 8;
    this->carryPosition.vz += move.vz >> 8;
    Fighter_getPosition(&this->base, (Vec3 *)&position);
    if (position.vy < this->carryPosition.vy) {
        this->carryPosition.vy = position.vy;
    }
    if (Fighter_getOpponentInReach(&this->base, 0) == NULL) {
        this->carryPosition.vx = position.vx + Fighter_getFacing(&this->base) * CARRY_AHEAD;
    }
    *out = this->carryPosition;
    return out;
}

s32 Fighter_isInvulnerable(Fighter *this);
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size);
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size);

/* The entity class of vtable D_800691C4: HitSpark_init's (0x48 bytes) with a halfword more; its draw is
 * func_80091328 and its destructor func_80091998. */
typedef struct Unk800919EC {
    /* 0x00 */ HitSpark base;
    /* 0x48 */ u16 vramY; /* Fighter_getVramY of the fighter that spawns it */
} Unk800919EC;

extern EntityVtable D_800691C4;

/* func_800919EC's body, which Imperialdramon_special3Hold has inline. */
static inline void unk800919ECConstruct(Unk800919EC *this, u16 vramY, Vec3 *position, VECTOR *unk, s32 unk3,
                                        u32 color) {
    HitSpark_init(&this->base, position, unk, unk3, color);
    this->base.entity.vtable = &D_800691C4;
    this->vramY = vramY;
}

/* Ends the hold: clears the special move 3's state (0x4A6 to 0x4AC) and the attack parts. */
static inline void Imperialdramon_releaseHold(Imperialdramon *this) {
    this->special3Parts = 0;
    this->base.attackParts = 0;
    this->holding = 0;
    this->special3Active = 0;
    this->hitPending = 0;
}

/* A frame of the special move 3 (both classes' special3 run it): in the first 20 frames it grabs the opponent when it
 * can be held (Fighter_hold), then carries it along func_8008DD50's path; given frames spawn sparks and set the hit
 * (attack parts, 0x44C), and each pending hit spawns a spark and sparks at the opponent. The JP version lets go when
 * Fight_isRoundOver says so; the US version also calls Fighter_takeHit on the opponent when it grabs it and sets the
 * opponent's 0x48C to 2 after frame 20. */
void Imperialdramon_special3Hold(Imperialdramon *this, s32 frame) {
    Vec3 above;
    Vec3 vec;
    Vec3 ownPosition;
    Fighter *opponent = Fighter_getOpponent(&this->base);
    Vec3 *p;
    Entity *child;
    s32 i;
    s32 dx;
    s32 x, y, z;

#if VERSION_JP
    if (Fight_isRoundOver(Fighter_getScene(&this->base))) {
        if (this->holding != 0) {
            Fighter_getPosition(opponent, &vec);
            vec.vx = this->opponentStartX;
            Fighter_setPosition(opponent, &vec);
        }
        Fighter_stopActionClip(&this->base);
        Imperialdramon_releaseHold(this);
        return;
    }
#endif
    this->special3Active = 1;
    /* The match depends on the range test: the game compares the frame unsigned. */
    if (frame >= 0 && frame < 20 && this->holding == 0) {
        if (!Fighter_isGuarding(opponent) && !Fighter_isInvulnerable(opponent)
            && Fighter_getOpponentInReach(&this->base, 0) != NULL) {
#if !VERSION_JP
            Fighter_takeHit(opponent, 0x40000001, (Vec3 *)&ZERO_VECTOR, 0);
#endif
            Fighter_hold(opponent, 60 - frame);
            this->holding = 1;
            opponent->velocity.vx = 0;
            opponent->velocity.vy = 0;
            opponent->velocity.vz = 0;
            opponent->velocityPad = 0;
            this->carryForce.vx = 0;
            this->carryForce.vy = 0;
            this->carryForce.vz = 0;
            this->unk4DC = 0;
            this->carryVelocity.vx = 0;
            this->carryVelocity.vy = 0;
            this->carryVelocity.vz = 0;
            this->unk4EC = 0;
            /* The match depends on reading the position's three words before any store, as vecCopy does. */
            p = Fighter_getPosition(opponent, &vec);
            x = p->vx;
            y = p->vy;
            z = p->vz;
            this->base.velocity.vx = 0;
            this->base.velocity.vy = 0;
            this->base.velocity.vz = 0;
            this->base.velocityPad = 0;
            this->carryLastPosition.vx = x;
            this->carryLastPosition.vy = y;
            this->carryLastPosition.vz = z;
            this->carryPosition.vx = x;
            this->carryPosition.vy = y;
            this->carryPosition.vz = z;
            Fighter_getPosition(&this->base, &ownPosition);
            dx = ownPosition.vx - vec.vx;
            if (ABS(dx) < 0x140) {
                this->base.force.vx = dx >= 0 ? 0x1C70 : -0x1C70;
            }
        }
    }
    if (this->holding != 0 && this->unk4A4 == 0) {
#if !VERSION_JP
        if (frame > 20) {
            opponent->unk48C = 2;
        }
#endif
        func_8008DD50((VECTOR *)&vec, this);
        Fighter_setPosition(opponent, &vec);
    }
    switch (frame) {
    case 0: {
        /* The match depends on unk228 and spark being set once in their block: sched1 then sinks the copy
         * of the result next to the call that follows. */
        Fight *unk228;

        Fighter_getPosition(opponent, &vec);
        this->unk4A4 = 0;
        this->opponentStartX = vec.vx;
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_SPECIAL3, 0, 0, -1);
        Fighter_addGauge(&this->base, -0x10000);
        unk228 = Fighter_getScene(&this->base);
        Fighter_getPosition(&this->base, &vec);
        for (i = 0; i < 16; i++) {
            Entity *spark = allocEntityMemory(0x58);

            if (spark != NULL) {
                func_8008E9B4(entityOperatorNewAt(0x58, spark), &this->base, unk228, &vec, 0x400 + i * 0x180);
            }
            Fighter_addEntity(&this->base, spark);
        }
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.velocity.vz = 0;
        this->base.velocityPad = 0;
        this->base.invulnerableFrames = 10;
        break;
    }
    case 3: {
        /* The match depends on push being an s64 local: the game keeps both halves of it across the call. */
        s64 push = ((s64)this->base.mass * 0x38E0) >> 16;

        this->base.force.vx += (s32)push * Fighter_getFacing(&this->base);
        break;
    }
    case 6:
        this->special3Parts = 4;
        this->base.hitDamage = 0x5B01500;
        this->hitPending = 1;
        break;
    case 12:
        this->special3Parts = 8;
        this->base.hitDamage = 0x5B01500;
        this->hitPending = 1;
        break;
    case 19: {
        s64 push = ((s64)this->base.mass * 0x1C70) >> 16;

        this->base.force.vx += (s32)push * Fighter_getFacing(&this->base);
        this->base.force.vy -= ((s64)this->base.mass * 0x71C0) >> 16;
        break;
    }
    case 20:
        if (this->holding != 0) {
            vec.vx = 0;
            vec.vy = -0x78DC;
            vec.vz = 0;
            this->carryForce = vec;
            this->carryVelocity.vx = 0;
            this->carryVelocity.vy = 0;
        }
        this->special3Parts = 0x10;
        this->base.hitDamage = 0x5B01500;
        this->hitPending = 1;
        break;
    case 10:
    case 27:
        this->base.velocity.vx = 0;
        break;
    case 28:
        this->special3Parts = 0x20;
        this->carryVelocity.vx = 0;
        this->base.hitDamage = 0x5B01500;
        this->hitPending = 1;
        break;
    case 42:
        this->base.force.vy = 0;
        break;
    case 50:
        if (this->holding != 0) {
            vec.vx = 0;
            vec.vy = 0x1C70;
            vec.vz = 0;
            Fighter_takeHit(opponent, 0x45B01500, (Vec3 *)&ZERO_VECTOR, 0);
            this->base.velocity.vx = 0;
            this->hitPending = 1;
            this->carryForce = vec;
        }
        this->special3Parts = 0;
        break;
    }
    if (this->special3Parts != 0) {
        Fighter_setAttackParts(&this->base, this->special3Parts);
    } else {
        this->base.attackParts = 0;
    }
    if (!Fighter_isActionClipDone(&this->base)) {
#if VERSION_JP
        if (this->base.hitDamage == 0 && this->hitPending != 0) {
#else
        if (!Fighter_isGuarding(opponent) && this->hitPending != 0 && this->holding != 0) {
#endif
            Fight *unk228 = Fighter_getScene(&this->base);

            func_8007A650(opponent, &above);
            child = allocEntityMemory(0x4C);
            if (child != NULL) {
                unk800919ECConstruct(entityOperatorNewAt(0x4C, child), Fighter_getVramY(&this->base), &above,
                                     &ZERO_VECTOR, 0x180, 0xFFFFFF);
            }
            Fighter_addEntity(&this->base, child);
            for (i = 0; i < 8; i++) {
                Entity *spark = allocEntityMemory(0x58);

                if (spark != NULL) {
                    func_8008EA58(entityOperatorNewAt(0x58, spark), opponent, unk228, &above, 0x100 + i * 0x40);
                }
                Fighter_addEntity(&this->base, spark);
            }
            this->hitPending = 0;
        }
    } else {
        Imperialdramon_releaseHold(this);
    }
}

/* Spawns a spark (HitSpark_init, 0x60, grey) at the opponent, for the throw. */
void Imperialdramon_spawnThrowSpark(Fighter *this) {
    Fighter *opponent = Fighter_getOpponent(this);
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    velocity.vx = 0;
    velocity.vy = 0;
    velocity.vz = 0;
    velocity.pad = 0;
    Fighter_getPosition(opponent, &position);
    child = allocEntityMemory(0x48);
    if (child != NULL) {
        HitSpark_init(entityOperatorNewAt(0x48, child), &position, &velocity, 0x60, 0x808080);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800690D0);

/* Spawns the special move 1's hitbox at an offset from the fighter's node 12, 64 a frame forward. */
void Imperialdramon_spawnSpecial1Hitbox(Fighter *this) {
    Fight *unk228 = Fighter_getScene(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getNodePosition(this, (VECTOR *)&position, 0xC, &D_800690D0);
    velocity.vx = Fighter_getFacing(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        ImperialdramonSpecial1Hitbox_init(entityOperatorNewAt(0x160, child), this, &position, &velocity, unk228);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800690E0);

/* Spawns the special move 1's effect at the fighter's node 12 moved by D_800690E0, with air (1 in the air) in the
 * vector's pad. */
void Imperialdramon_spawnSpecial1Effect(Fighter *this, s32 air) {
    Fight *obj = Fighter_getScene(this);
    VECTOR pos;
    Entity *child;

    Fighter_getNodePosition(this, &pos, 12, &D_800690E0);
    pos.pad = air;
    child = allocEntityMemory(0x150);
    if (child != NULL) {
        ImperialdramonSpecial1Effect_init(entityOperatorNewAt(0x150, child), this, &pos, obj);
    }
    Fighter_addEntity(this, child);
}

Entity *ImperialdramonSpecial2Hitbox_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228);

/* Spawns the special move 2's hitbox at the fighter's node 12, 64 a frame forward. */
void Imperialdramon_spawnSpecial2Hitbox(Fighter *this) {
    Fight *unk228 = Fighter_getScene(this);
    Vec3 position;
    Vec3 velocity;
    VECTOR offset;
    Entity *child;

    velocity.vx = Fighter_getFacing(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    offset.vx = 0;
    offset.vy = 0;
    offset.vz = 0;
    offset.pad = 0;
    Fighter_getNodePosition(this, (VECTOR *)&position, 0xC, &offset);
    child = allocEntityMemory(0xA4);
    if (child != NULL) {
        ImperialdramonSpecial2Hitbox_init(entityOperatorNewAt(0xA4, child), this, &position, &velocity, unk228);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800690F0);

/* Spawns the special move 2's trail (Trail_init with the table D_800690F0). */
void Imperialdramon_spawnSpecial2Trail(Fighter *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, D_800690F0, 3, 8, 14);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 3's effect at the fighter, moving 256 forward and 64 up. */
void Imperialdramon_spawnSpecial3Effect(Fighter *this) {
    Fight *unk228 = Fighter_getScene(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    velocity.vx = Fighter_getFacing(this) * 256;
    velocity.vy = -64;
    velocity.vz = 0;
    child = allocEntityMemory(0x920);
    if (child != NULL) {
        ImperialdramonSpecial3Effect_init(entityOperatorNewAt(0x920, child), this, &position, &velocity, unk228);
    }
    Fighter_addEntity(this, child);
}

extern VECTOR D_80069138;
struct ImperialdramonSpecial3Hitbox *ImperialdramonSpecial3Hitbox_init(struct ImperialdramonSpecial3Hitbox *this,
                                                                       Fighter *fighter, Vec3 *position,
                                                                       Vec3 *velocity, Vec3 *knockback);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069138);

/* Spawns the special move 3's hitbox at the fighter's node 1 moved by D_80069138 and 256 forward, 256 a frame forward.
 */
void Imperialdramon_spawnSpecial3Hitbox(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Vec3 knockback;
    Entity *child;

    Fighter_getNodePosition(this, (VECTOR *)&position, 1, &D_80069138);
    position.vx += Fighter_getFacing(this) * 256;
    velocity.vx = Fighter_getFacing(this) * 256;
    velocity.vy = 0;
    velocity.vz = 0;
    knockback.vx = Fighter_getFacing(this) * 0x71C0;
    knockback.vy = -0x38E0;
    knockback.vz = 0;
    child = allocEntityMemory(0xAC);
    if (child != NULL) {
        ImperialdramonSpecial3Hitbox_init(entityOperatorNewAt(0xAC, child), this, &position, &velocity, &knockback);
    }
    Fighter_addEntity(this, child);
}

/* Spawns a burst of 8 (ParticleBurst_init, kind 6) 0xC0 in front of the fighter and 0x20 up, for attacks 1 and 2. */
void Imperialdramon_spawnAttackBurst(Fighter *this) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vx += Fighter_getFacing(this) * 0xC0;
    position.vy -= 0x20;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 6, &position, 0x10, 0x50, 8, 0xA0A0A0);
    }
    Fighter_addEntity(this, child);
}

/* Constructs an Unk8008E9B4 (vtable D_80069364) for owner: func_800C3CFC with pos, arg4 and 0xFFC080,
 * then 6 in unk50. */
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size) {
    func_800C3CFC(&this->base, pos, size, 0xFFC080);
    /* The match depends on this order: vtable, unk54, unk50. */
    this->base.entity.vtable = &D_80069364;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

/* The gather of the Unk8008E9B4 of D_80069364: func_800C3E64, then puts the world position of the
 * owner's point 12 (Fighter_getNodePosition) at 0x28. */
void func_8008EA10(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    Fighter_getNodePosition(this->unk54, &this->base.unk28, 12, &origin);
}

/* Constructs an Unk8008E9B4 with vtable D_8006933C (see func_8008E9B4). */
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size) {
    func_800C3CFC(&this->base, pos, size, 0xFFC080);
    /* The match depends on this order, as in func_8008E9B4. */
    this->base.entity.vtable = &D_8006933C;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

typedef struct ImperialdramonSpecial1Hitbox {
    /* 0x000 */ UnkPartsEntity38 parts;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ Fight *fight;
    /* 0x088 */ s16 side;
    /* 0x08A */ s16 unk8A;
    /* 0x08C */ s32 unk8C;
    /* 0x090 */ s16 unk90;
    /* 0x092 */ s16 unk92;
    /* 0x094 */ s16 unk94;
    /* 0x096 */ s16 vramY;
    /* 0x098 */ s32 speed;
    /* 0x09C */ s32 unk9C;
    /* 0x0A0 */ Vec3 velocity;
    /* 0x0AC */ u8 unkAC[4];
    /* 0x0B0 */ Vec3 position;
    /* 0x0BC */ u8 unkBC[4];
    /* 0x0C0 */ SVECTOR unkC0[4][4];
    /* 0x140 */ SVECTOR corners[4];
} ImperialdramonSpecial1Hitbox;

/* Builds Imperialdramon's special move 1 hitbox (0x160 bytes): a 0xF0 sphere with knockback (vx << 6, -|vx << 6|, 0),
 * four 4-cornered quads and a 4-cornered head quad, and plays sound 0x95 (Imperialdramon) or 0x9E (Fighter Mode). */
Entity *ImperialdramonSpecial1Hitbox_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228) {
    ImperialdramonSpecial1Hitbox *h = (ImperialdramonSpecial1Hitbox *)this;
    VECTOR knockback;
    s32 i;
    ImperialdramonSpecial1Hitbox *p;
    CollisionSphere *part;

    entityInit(this, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&h->parts.base, fighter, 0x5A81F33, velocity, 3);
    h->parts.base.vtable = IMPERIALDRAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    h->parts.entity.vtable = &IMPERIALDRAMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&h->parts.part, position, 0xF0, &h->parts.base);
    knockback = *(VECTOR *)velocity;
    knockback.vx <<= 6;
    knockback.vz = 0;
    knockback.vy = -abs(knockback.vx);
    Hit_setKnockback(&h->parts.base, (Vec3 *)&knockback);
    h->vramY = Fighter_getVramY(fighter);
    h->fighter = fighter;
    h->fight = unk228;
    h->unk92 = 0;
    h->unk94 = 0;
    h->speed = vecGetLength((VECTOR *)velocity);
    h->unk9C = 0;
    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;

        h->unk90 = 0;
        h->unk8A = 0;
        h->velocity.vx = x;
        h->velocity.vy = y;
        h->velocity.vz = z;
        h->side = x < 0 ? -1 : 1;
    }
    h->parts.part.entity = &h->parts.entity;
    part = &h->parts.part;
    if (Fighter_getSide(h->fighter) == 0) {
        Fight_addSide0Shape(unk228, &part->link);
    } else {
        Fight_addSide1Shape(unk228, &part->link);
    }
    h->unk8C = 0;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        h->position.vx = x;
        h->position.vy = y;
        h->position.vz = z;
    }
    {
        s16 neg = -0x80;
        s16 pos = 0x80;

        p = h;
        for (i = 3; i >= 0; i--) {
            p->unkC0[0][3].vx = 0;
            p->unkC0[0][1].vx = 0;
            p->unkC0[0][2].vx = 0;
            p->unkC0[0][0].vx = 0;
            p->unkC0[0][1].vy = neg;
            p->unkC0[0][0].vy = neg;
            p->unkC0[0][3].vy = pos;
            p->unkC0[0][2].vy = pos;
            p->unkC0[0][3].vz = 0;
            p->unkC0[0][2].vz = 0;
            p->unkC0[0][1].vz = 0;
            p->unkC0[0][0].vz = 0;
            p = (ImperialdramonSpecial1Hitbox *)((u8 *)p + 0x20);
        }
    }
    {
        s32 side = h->side;

        h->corners[1].vy = -0x100;
        h->corners[0].vy = -0x100;
        h->corners[3].vy = 0x100;
        h->corners[2].vy = 0x100;
        h->corners[3].vz = -0x40;
        h->corners[2].vz = -0x40;
        h->corners[1].vz = -0x40;
        h->corners[0].vz = -0x40;
        h->corners[2].vx = -side << 7;
        h->corners[0].vx = -side << 7;
        h->corners[3].vx = side << 8;
        h->corners[1].vx = side << 8;
    }
    if (Fighter_getCharacter(h->fighter) == 5) {
        Fighter_playSound(h->fighter, 0x95);
    } else {
        Fighter_playSound(h->fighter, 0x9E);
    }
    return this;
}

/* The special move 1 hitbox's destructor. */
void ImperialdramonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = IMPERIALDRAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &IMPERIALDRAMON_SPECIAL1_HITBOX_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Hitbox_tick);

/* On a hit: Hit_apply; when it struck a fighter it takes its position (unused) and disables its sphere. */
void ImperialdramonSpecial1Hitbox_hit(UnkPartsEntity38 *this, CollisionSphere *sum) {
    Entity *fighter;
    Vec3 position;

    Hit_apply(&this->base, sum);
    fighter = getHitFighter(sum);
    if (fighter != NULL) {
        Fighter_getPosition((Fighter *)fighter, &position);
        this->part.flags.bits.disabled = 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Hitbox_draw);

/* Constructs the special move 1 effect for fighter in fight at pos (unk20 0xF00 on the ground, 0xC00 when pos->pad is
 * set): the ten points 0..0xFF around the centre with random angles, then the corners. The match needs the corners'
 * stores in this order (corners[2].vx first). */
Entity *ImperialdramonSpecial1Effect_init(Entity *this, Fighter *fighter, VECTOR *pos, Fight *fight) {
    ImperialdramonSpecial1Effect *e = (ImperialdramonSpecial1Effect *)this;
    s32 i;
    s32 *pa;
    long *pz, *py, *px;

    entityInit(this, 0);
    e->entity.vtable = &IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE;
    e->vramY = Fighter_getVramY(fighter);
    e->fighter = fighter;
    e->fight = fight;
    e->state = 0;
    e->unk30 = 0;
    e->frames = 0;
    e->pointCount = 0;
    e->unk28 = 0;
    if (pos->pad == 0) {
        e->unk20 = 0xF00;
    } else {
        e->unk20 = 0xC00;
    }
    i = 9;
    {
        s32 x = pos->vx, y = pos->vy, z = pos->vz;
        e->centre.pad = 0x1000;
        e->centre.vx = x;
        e->centre.vy = y;
        e->centre.vz = z;
    }
    e->facing = Fighter_getFacing(e->fighter);
    pa = e->unkE8;
    pz = &e->unk48[0].vz;
    py = &e->unk48[0].vy;
    px = &e->unk48[0].vx;
    e->shade = 0x80;
    for (; i >= 0; i--) {
        s32 r = Fight_random(e->fight);
        s32 c = e->centre.vx - 0x7F;
        *px = (r & 0xFF) + c;
        px += 4;
        r = Fight_random(e->fight);
        c = e->centre.vy - 0x7F;
        *py = (r & 0xFF) + c;
        *pz = 0;
        pz += 4;
        py += 4;
        *pa = Fight_random(e->fight) & 0xFFF;
        pa++;
    }
    e->corners[2].vx = -32;
    e->corners[3].vx = 32;
    e->corners[1].vx = 32;
    e->corners[3].vy = 32;
    e->corners[2].vy = 32;
    e->corners[0].vx = -32;
    e->corners[5].vy = -256;
    e->corners[4].vy = -256;
    e->corners[1].vy = -32;
    e->corners[0].vy = -32;
    e->corners[3].vz = 0;
    e->corners[2].vz = 0;
    e->corners[1].vz = 0;
    e->corners[0].vz = 0;
    e->corners[6].vx = 0;
    e->corners[4].vx = 0;
    e->corners[7].vy = 256;
    e->corners[6].vy = 256;
    e->corners[7].vz = -32;
    e->corners[6].vz = -32;
    e->corners[5].vz = -32;
    e->corners[4].vz = -32;
    e->corners[7].vx = e->facing * 3 << 7;
    e->corners[5].vx = e->facing * 3 << 7;
    return this;
}

/* The special move 1 effect's destructor. */
void ImperialdramonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069148);

/* The offset of the effect's centre from the fighter's node 12 (0x46, 0xAA, 0x14, pad 0x1000) */
extern VECTOR D_80069148;

void func_8008F7F8(ImperialdramonSpecial1Effect *this);

/* Follows the fighter's node 12 with 4 more points each tick (up to 10) and moves them (func_8008F7F8). From frame 6
 * it grows unk28 halfway to 0x1000 until unk20, then halves the shade until under 3, then deletes itself. */
void ImperialdramonSpecial1Effect_tick(ImperialdramonSpecial1Effect *this) {
    this->pointCount += 4;
    if (this->pointCount > 10) {
        this->pointCount = 10;
    }
    Fighter_getNodePosition(this->fighter, &this->centre, 12, &D_80069148);
    if (this->frames >= 6) {
        switch (this->state) {
        case 0:
            this->unk28 += (0x1000 - this->unk28) >> 1;
            if (this->unk28 < this->unk20) {
                break;
            }
            this->unk28 = this->unk20;
            this->state++;
            /* fall through */
        case 1:
            this->shade -= this->shade >> 1;
            if (this->shade >= 3) {
                break;
            }
            this->shade = 0;
            this->state++;
            /* fall through */
        case 2:
            entityDelete(&this->entity);
            return;
        }
    }
    func_8008F7F8(this);
    this->frames++;
}

s16 func_8008F9B0(ImperialdramonSpecial1Effect *this, s32 index);
s32 turnAngleToward(s32 target, s32 from, s32 step);

/* Steers each active point's angle toward func_8008F9B0 by up to 0x80, advances it by (-sin, -cos) >> 7, and in
 * state 0 respawns any point within 0x40 of centre to a random (-0x7F..0x80) offset with a random angle. */
void func_8008F7F8(ImperialdramonSpecial1Effect *this) {
    VECTOR diff;
    s32 i;

    for (i = 0; i < this->pointCount; i++) {
        VECTOR *v;
        s32 dx, dy;
        s32 idx;

        idx = turnAngleToward(func_8008F9B0(this, i), this->unkE8[i], 0x80);
        this->unkE8[i] = idx;
        dx = -COS_SIN_TABLE[idx & 0xFFF].sin >> 7;
        dy = -COS_SIN_TABLE[idx & 0xFFF].cos >> 7;
        this->unk48[i].vx += dx;
        this->unk48[i].vy += dy;
        v = &this->unk48[i];
        {
            s32 vy, vz;
            diff.vx = v->vx - this->centre.vx;
            vy = v->vy;
            vz = v->vz;
            diff.vy = vy - this->centre.vy;
            diff.vz = vz - this->centre.vz;
        }
        if (vecGetLength(&diff) < 0x40 && this->state == 0) {
            s32 r = Fight_random(this->fight);
            s32 c = this->centre.vx - 0x7F;
            this->unk48[i].vx = (r & 0xFF) + c;
            r = Fight_random(this->fight);
            c = this->centre.vy - 0x7F;
            this->unk48[i].vy = (r & 0xFF) + c;
            this->unk48[i].vz = 0;
            this->unkE8[i] = Fight_random(this->fight) & 0xFFF;
        }
    }
}

/* The direction (ratan2) from the centre at 0x38 to point index, on x and y. */
s16 func_8008F9B0(ImperialdramonSpecial1Effect *this, s32 index) {
    return ratan2(this->unk48[index].vx - this->centre.vx, this->unk48[index].vy - this->centre.vy);
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Effect_draw);

/* The special move 2's hitbox (0xA4 bytes): a sphere that grows to radius 0x200 while it slides forward, slowing
 * down, from the fighter's node 1. */
typedef struct ImperialdramonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *fighter;
    /* 0x84 */ u8 speed;     /* 0x80 at first, loses an eighth every frame; the hitbox goes under 9 */
    /* 0x88 */ u32 size;     /* 16 times the sphere's radius: 0 at first, half way to 0x2020 every frame, up to 0x2000 */
    /* 0x8C */ s16 facing;   /* Fighter_getFacing of the fighter */
    /* 0x8E */ s16 unk8E;
    /* 0x90 */ s16 vramY;    /* Fighter_getVramY of the fighter */
    /* 0x92 */ s16 unk92;
    /* 0x94 */ Vec3 position;
} ImperialdramonSpecial2Hitbox;

/* Builds Imperialdramon's special move 2 hitbox (0xA4 bytes): a 0x80 sphere with knockback (vx << 6, -|vx << 6|, 0),
 * at the fighter's node 1, and plays sound 0x96 (Imperialdramon) or 0x9D (Fighter Mode). */
Entity *ImperialdramonSpecial2Hitbox_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228) {
    ImperialdramonSpecial2Hitbox *h = (ImperialdramonSpecial2Hitbox *)this;
    VECTOR offset;
    VECTOR knockback;
    VECTOR *pos;
    VECTOR *off;
    CollisionSphere *part;
    s32 x, y, z;

    entityInit(this, 0);
    Hit_init(&h->parts.base, fighter, 0x5AC118C, velocity, 3);
    h->parts.base.vtable = IMPERIALDRAMON_SPECIAL2_HITBOX_HIT_VTABLE;
    h->parts.entity.vtable = &IMPERIALDRAMON_SPECIAL2_HITBOX_VTABLE;
    CollisionSphere_init(&h->parts.part, position, 0x80, &h->parts.base);
    knockback = *(VECTOR *)velocity;
    off = &offset;
    offset.vx = 0;
    offset.vy = 0;
    offset.vz = 0;
    offset.pad = 0;
    knockback.vz = 0;
    knockback.vx <<= 6;
    knockback.vy = -abs(knockback.vx);
    Hit_setKnockback(&h->parts.base, &knockback);
    h->vramY = Fighter_getVramY(fighter);
    h->fighter = fighter;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    pos = (VECTOR *)&h->position;
    h->unk8E = 0;
    h->speed = 0x80;
    h->size = 0;
    h->position.vx = x;
    h->position.vy = y;
    h->position.vz = z;
    h->facing = Fighter_getFacing(fighter);
    Fighter_getNodePosition(h->fighter, pos, 1, off);
    h->parts.part.entity = &h->parts.entity;
    h->position.vy += 0x100;
    part = &h->parts.part;
    if (Fighter_getSide(h->fighter) == 0) {
        Fight_addSide0Shape(unk228, &part->link);
    } else {
        Fight_addSide1Shape(unk228, &part->link);
    }
    if (Fighter_getCharacter(h->fighter) == 5) {
        Fighter_playSound(h->fighter, 0x96);
    } else {
        Fighter_playSound(h->fighter, 0x9D);
    }
    return (Entity *)this;
}

/* The special move 2 hitbox's destructor. */
void ImperialdramonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = IMPERIALDRAMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &IMPERIALDRAMON_SPECIAL2_HITBOX_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Slides forward by 1.5 times its speed and grows; it slows down by an eighth a frame and deletes
 * itself under speed 9. */
void ImperialdramonSpecial2Hitbox_tick(ImperialdramonSpecial2Hitbox *this) {
    this->position.vx += (this->speed * 3 >> 1) * this->facing;
    this->size += (0x2020 - this->size) >> 1;
    if (this->size > 0x2000) {
        this->size = 0x2000;
    }
    if (this->speed < 9) {
        entityDelete(&this->parts.entity);
        return;
    }
    this->speed += -this->speed >> 3;
    this->parts.part.position = this->position;
    this->parts.part.radius = this->size >> 4;
}

/* Applies hit, and when it struck a fighter (getHitFighter) disables its sphere and adds a hit spark (HitSpark_init,
 * size 0x60, color 0x808080) at the fighter's position (Fighter_getPosition) with a zero VECTOR as the direction. */
void ImperialdramonSpecial2Hitbox_hit(ImperialdramonSpecial2Hitbox *this, void *hit) {
    Entity *fighter;

    Hit_apply(&this->parts.base, hit);
    fighter = getHitFighter(hit);
    if (fighter != NULL) {
        VECTOR velocity;
        Vec3 position;
        Entity *child;
        VECTOR *vel;

        velocity.vx = 0;
        velocity.vy = 0;
        velocity.vz = 0;
        velocity.pad = 0;
        vel = &velocity;
        this->parts.part.flags.bits.disabled = 1;
        Fighter_getPosition((Fighter *)fighter, &position);
        child = NEW_ENTITY(0x48, HitSpark_init, &position, vel, 0x60, 0x808080);
        entityAddSibling(&this->parts.entity, child);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069158);

extern SVECTOR D_80069158[4];

void ImperialdramonSpecial2Hitbox_draw(ImperialdramonSpecial2Hitbox *this, Camera *arg) {
    Camera *camera = arg;
    MatrixWords m;
    PolyFT4Shape shape;
    s32 i;
    s32 color;
    u16 vramY;
    MATRIX *mp;
    Vec3 *pos;

    i = 0;
    shape.corners[0] = &D_80069158[0];
    shape.corners[1] = &D_80069158[1];
    shape.corners[2] = &D_80069158[2];
    shape.corners[3] = &D_80069158[3];
    UV_BYTES(shape.uv0)[0] = 0xBF;
    UV_BYTES(shape.uv1)[0] = 0x80;
    UV_BYTES(shape.uv2)[0] = 0xBF;
    UV_BYTES(shape.uv3)[0] = 0x80;
    UV_BYTES(shape.uv0)[1] = 0x20;
    UV_BYTES(shape.uv1)[1] = 0x20;
    UV_BYTES(shape.uv2)[1] = 0x5F;
    UV_BYTES(shape.uv3)[1] = 0x5F;
    color = this->speed;
    shape.flags = 9;
    vramY = this->vramY;
    shape.color = color | (color << 8) | (color << 16);
    shape.tpage = getTPage(0, 1, 0x1C0, vramY);
    shape.clut = getClut(0x1C0, vramY + 0xF2);
    mp = &m.m;
    for (; i < 2; i++) {
        s32 size = (u16)this->size;

        m.m.t[0] = 0;
        m.m.t[1] = 0;
        m.m.t[2] = 0;
        m.words[1] = 0;
        m.words[3] = 0;
        m.words[0] = size;
        m.words[2] = size;
        m.words[4] = size;
        if (i != 0) {
            matrixRotateX(mp, 0x800);
        }
        if (this->facing < 0) {
            matrixRotateY(mp, 0x800);
        }
        pos = &this->position;
        m.m.t[0] = pos->vx;
        m.m.t[1] = pos->vy;
        m.m.t[2] = pos->vz;
        setModelMatrix(mp);
        PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + 1);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069178);

/* The special move 3's effect (0x920 bytes): 64 particles drawn to a point of the character, with a square of four
 * corners. */
typedef struct ImperialdramonSpecial3Effect {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Fighter *fighter;
    /* 0x01C */ Fight *obj;    /* ImperialdramonSpecial3Effect_init's fifth argument; Fight_random draws numbers from
                                * it */
    /* 0x020 */ s16 unk20;           /* frames in state 2 */
    /* 0x022 */ u8 unk22;            /* 0x80 from ImperialdramonSpecial3Effect_init, eased to 0 */
    /* 0x023 */ u8 unk23;            /* 0 from ImperialdramonSpecial3Effect_init, eased to 0x84 */
    /* 0x024 */ s16 side;            /* the sign of the velocity's x */
    /* 0x026 */ u16 timer;           /* counts the frames */
    /* 0x028 */ u16 state;
    /* 0x02A */ s16 vramY;           /* Fighter_getVramY of the character */
    /* 0x02C */ u16 count;           /* particles in use: 16 more every frame, up to 64 */
    /* 0x02E */ u8 unk2E[0x34 - 0x2E];
    /* 0x034 */ s32 unk34;
    /* 0x038 */ u8 unk38[0x48 - 0x38];
    /* 0x048 */ VECTOR position;     /* the character's point 1 moved by D_80069188 (Fighter_getNodePosition) */
    /* 0x058 */ VECTOR unk58;        /* position, 128 toward side */
    /* 0x068 */ u8 unk68[0x78 - 0x68];
    /* 0x078 */ SVECTOR corners[4];  /* (+-0x80, +-0x40, 0) */
    /* 0x098 */ s32 unk98;           /* 0x1000 from ImperialdramonSpecial3Effect_init, eased to 0x3200 */
    /* 0x09C */ s32 unk9C;           /* 0x1000 from ImperialdramonSpecial3Effect_init, halved every frame of state 3 */
    /* 0x0A0 */ s16 unkA0[16][4];
    /* 0x120 */ VECTOR particles[64]; /* positions */
    /* 0x520 */ VECTOR velocities[64];
} ImperialdramonSpecial3Effect; /* size 0x920 */

/* Offset from fighter node 1 used by ImperialdramonSpecial3Effect_init. */
extern VECTOR D_80069178;

/* Constructs the special move 3 effect for fighter at node 1 + D_80069178: scatters 64 particles, sets the quad's
 * corners (+-0x80, +-0x40, 0) and initializes the 16 trail segments. */
Entity *ImperialdramonSpecial3Effect_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *unk228) {
    ImperialdramonSpecial3Effect *e = (ImperialdramonSpecial3Effect *)this;
    long *vz, *vy, *vx;
    long *pz, *py, *px;
    s32 i;
    ImperialdramonSpecial3Effect *p;

    entityInit(this, 0);
    e->entity.vtable = &IMPERIALDRAMON_SPECIAL3_EFFECT_VTABLE;
    e->vramY = Fighter_getVramY(fighter);
    e->unk22 = 0x80;
    e->fighter = fighter;
    e->state = 0;
    e->unk23 = 0;
    e->unk34 = 0;
    e->timer = 0;
    e->count = 0;
    e->unk98 = 0x1000;
    e->unk9C = 0x1000;
    e->unk20 = 0;
    e->obj = unk228;
    if (velocity->vx >= 0) {
        e->side = 1;
    } else {
        e->side = -1;
    }
    Fighter_getNodePosition(e->fighter, &e->position, 1, &D_80069178);
    vz = &e->velocities[0].vz;
    vy = &e->velocities[0].vy;
    vx = &e->velocities[0].vx;
    pz = &e->particles[0].vz;
    py = &e->particles[0].vy;
    px = &e->particles[0].vx;
    e->corners[2].vx = -0x80;
    e->corners[0].vx = -0x80;
    e->corners[3].vx = 0x80;
    e->corners[1].vx = 0x80;
    e->corners[1].vy = -0x40;
    e->corners[0].vy = -0x40;
    e->corners[3].vy = 0x40;
    e->corners[2].vy = 0x40;
    e->corners[3].vz = 0;
    e->corners[2].vz = 0;
    e->corners[1].vz = 0;
    e->corners[0].vz = 0;
    e->unk58.vy = e->position.vy;
    e->unk58.vz = e->position.vz;
    e->unk58.vx = e->position.vx + (e->side << 7);
    for (i = 0x3F; i >= 0; i--) {
        *px = (Fight_random(e->obj) & 0xFFF) - 0x800;
        *py = (Fight_random(e->obj) & 0xFFF) - 0x800;
        *pz = 0;
        pz += 4;
        *vx = -*px >> 2;
        px += 4;
        vx += 4;
        *vy = -*py >> 2;
        py += 4;
        *vz = 0;
        vz += 4;
        vy += 4;
    }
    i = 0;
    p = e;
    for (; i < 16; i++) {
        s16 v = i * (e->side * 3 << 6);

        p->unkA0[0][1] = 0;
        p->unkA0[0][2] = 0;
        p->unkA0[0][0] = v;
        p = (ImperialdramonSpecial3Effect *)((u8 *)p + 8);
    }
    Fighter_playSound(e->fighter, 0x97);
    return this;
}

/* The special move 3 effect's destructor. */
void ImperialdramonSpecial3Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_SPECIAL3_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

extern VECTOR D_80069188;

/* vecAddInPlace (engine/math/vector_ops.c), inlined */
static inline VECTOR *vecAddInPlaceInline(VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x += b->vx;
    y += b->vy;
    z += b->vz;
    a->vx = x;
    a->vy = y;
    a->vz = z;
    return a;
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069188);

/* Follows the character's point 1 moved by D_80069188; moves the particles, and puts one that came within 32 of the
 * point back at random, a quarter of its offset as velocity; eases unk22 and unk23. Then by state: after 6 frames makes
 * func_80091498's entity, grows unk98 toward 0x3200 while unk58 bobs, waits 12 frames, halves unk9C below 5 and deletes
 * itself. */
void ImperialdramonSpecial3Effect_tick(ImperialdramonSpecial3Effect *this) {
    VECTOR offset = D_80069188;
    VECTOR distance;
    VECTOR *particle;
    VECTOR *velocity;
    Entity *child;
    s32 i;

    Fighter_getNodePosition(this->fighter, &this->position, 1, &offset);
    this->unk58.vy = this->position.vy;
    this->unk58.vz = this->position.vz;
    this->unk58.vx = this->position.vx + (this->side << 7);
    this->count += 16;
    if (this->count > 64) {
        this->count = 64;
    }
    for (i = 0; i < this->count; i++) {
        /* The match depends on velocity's address taken before particle's (loop makes the later one the base
         * register) and on vecAddInPlace inlined; the reset below indexes the arrays again. */
        velocity = &this->velocities[i];
        particle = &this->particles[i];
        vecAddInPlaceInline(particle, velocity);
        distance.vx = particle->vx - this->position.vx;
        distance.vy = particle->vy - this->position.vy;
        distance.vz = particle->vz - this->position.vz;
        if (vecGetLength(&distance) < 32) {
            this->particles[i].vx = (Fight_random(this->obj) & 0xFFF) - 0x800;
            this->particles[i].vy = (Fight_random(this->obj) & 0xFFF) - 0x800;
            this->velocities[i].vx = -this->particles[i].vx >> 2;
            this->velocities[i].vy = -this->particles[i].vy >> 2;
        }
    }
    if (this->unk23 != 0x80) {
        this->unk22 += (0 - this->unk22) >> 2;
        this->unk23 += (0x84 - this->unk23) >> 2;
    } else {
        this->unk22 = 0;
    }
    switch (this->state) {
    case 0:
        if (this->timer < 6) {
            break;
        }
        child = allocEntityMemory(0x38);
        if (child != NULL) {
            /* position is a VECTOR here, a Vec3 to func_80091498 (it reads the three words) */
            func_80091498(entityOperatorNewAt(0x38, child), this->fighter, (Vec3 *)&this->position, this->obj);
        }
        Fighter_addEntity(this->fighter, child);
        this->state++;
        /* fallthrough */
    case 1:
        /* The match depends on the sine as the first operand (unk58.vy += ... swaps the registers). */
        this->unk58.vy = (COS_SIN_TABLE[(this->timer << 10) & 0xC00].sin >> 7) + this->unk58.vy;
        this->unk98 += (0x3200 - this->unk98) >> 1;
        if (this->unk98 < 0x3100) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 2:
        this->unk20++;
        if (this->unk20 < 12) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 3:
        this->unk9C -= this->unk9C >> 1;
        if (this->unk9C >= 5) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 4:
        entityDelete(&this->entity);
        return;
    }
    this->timer++;
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial3Effect_draw);

/* The special move 3's hitbox (0xAC bytes): parts that move from position by velocity every frame until they have gone
 * 0x3000. */
typedef struct ImperialdramonSpecial3Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Imperialdramon *owner;
    /* 0x84 */ s32 speed;    /* velocity's length (vecGetLength) */
    /* 0x88 */ s32 distance; /* grows by speed every frame */
    /* 0x8C */ Vec3 velocity;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 position;
    /* 0xA8 */ s32 unkA8;
} ImperialdramonSpecial3Hitbox;

/* Builds the special move 3's hitbox for fighter: a sphere of radius 0x180 at position (damage word 0x45B03000, kind 2,
 * knockback), flying by velocity; disabled from the start when the owner's 0x4A4 is already set. */
ImperialdramonSpecial3Hitbox *ImperialdramonSpecial3Hitbox_init(ImperialdramonSpecial3Hitbox *this, Fighter *fighter,
                                                                Vec3 *position, Vec3 *velocity, Vec3 *knockback) {
    Fight *list;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, 0x45B03000, knockback, 2);
    this->parts.base.vtable = IMPERIALDRAMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &IMPERIALDRAMON_SPECIAL3_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0x180, &this->parts.base);
    this->owner = (Imperialdramon *)fighter;
    /* vecGetLength reads only the first three words */
    this->speed = vecGetLength((VECTOR *)velocity);
    this->distance = 0;
    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;

        this->velocity.vx = x;
        this->velocity.vy = y;
        this->velocity.vz = z;
    }
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        /* The match depends on this store sitting between the position's reads and its stores */
        this->parts.part.entity = &this->parts.entity;
        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    list = Fighter_getScene(fighter);
    if (Fighter_getSide(fighter) == 0) {
        Fight_addSide0Shape(list, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(list, &this->parts.part.link);
    }
    if (this->owner->unk4A4 != 0) {
        this->parts.part.flags.bits.disabled = 1;
    }
    return this;
}

/* The special move 3 hitbox's destructor. */
void ImperialdramonSpecial3Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = IMPERIALDRAMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &IMPERIALDRAMON_SPECIAL3_HITBOX_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Flies by its velocity, the sphere with it; deletes itself when the owner's 0x4A4 is set or its special move 3 has
 * ended, or once it has gone over 0x3000. */
void ImperialdramonSpecial3Hitbox_tick(ImperialdramonSpecial3Hitbox *this) {
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;

    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    if (this->owner->unk4A4 != 0 || this->owner->special3Active == 0) {
        entityDelete(&this->parts.entity);
    } else {
        this->distance += this->speed;
        if (this->distance > 0x3000) {
            entityDelete(&this->parts.entity);
        }
    }
}

/* On a hit: with no fighter hit, only Hit_apply. A fighter that does not guard stops (its velocity cleared) and sets
 * the owner's 0x4A4; a guarding one takes no knockback. Then Hit_apply, a burst of 4 (ParticleBurst_init) at the
 * fighter and the hitbox deletes itself. */
void ImperialdramonSpecial3Hitbox_hit(ImperialdramonSpecial3Hitbox *this, CollisionSphere *sum) {
    Fighter *target = (Fighter *)getHitFighter(sum);
    Vec3 position;
    Entity *child;

    if (target == NULL) {
        Hit_apply(&this->parts.base, sum);
        return;
    }
    if (!Fighter_isGuarding(target)) {
        this->owner->unk4A4 = 1;
        target->velocity.vx = 0;
        target->velocity.vy = 0;
        target->velocity.vz = 0;
        target->velocityPad = 0;
    } else {
        Hit_setKnockback(&this->parts.base, &ZERO_VECTOR);
    }
    Hit_apply(&this->parts.base, sum);
    Fighter_getPosition(target, &position);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 4, &position, this->speed >> 1, 0x100, 4, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

static inline void setTranslationImp(long *t, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    t[0] = x;
    t[1] = y;
    t[2] = z;
}

/* Draws Unk800919EC's spark quad using its SpriteAnim transform and vramY's sprite sheet. */
void func_80091328(Unk800919EC *this, Camera *camera) {
    MatrixWords m;
    PolyFT4Shape shape;
    MATRIX *mp = &m.m;
    void *packet;
    u_long *ot;
    s32 zShift;

    zShift = camera->zShift + 1;
    setTranslationImp(m.m.t, &this->base.sprite.position);
    packet = PRIM_BUFFER_FREE;
    m.words[0] = 0x1000;
    m.words[1] = 0;
    m.words[2] = 0x1000;
    m.words[3] = 0;
    m.words[4] = 0x1000;
    ot = camera->ot.slots;
    matrixSetRotationZ(mp, this->base.sprite.angle);
    matrixScale(mp, this->base.sprite.size, this->base.sprite.size, this->base.sprite.size);
    setModelMatrix(mp);
    shape.corners[0] = &SQUARE_CORNERS[0];
    shape.corners[1] = &SQUARE_CORNERS[1];
    shape.corners[2] = &SQUARE_CORNERS[2];
    shape.corners[3] = &SQUARE_CORNERS[3];
    shape.color = 0x808080;
    UV_BYTES(shape.uv0)[0] = 0x40;
    UV_BYTES(shape.uv0)[1] = 0x60;
    UV_BYTES(shape.uv1)[0] = 0x7F;
    UV_BYTES(shape.uv1)[1] = 0x60;
    UV_BYTES(shape.uv2)[0] = 0x40;
    UV_BYTES(shape.uv2)[1] = 0x9F;
    UV_BYTES(shape.uv3)[0] = 0x7F;
    UV_BYTES(shape.uv3)[1] = 0x9F;
    shape.flags = 9;
    shape.clut = getClut(0x1C0, this->vramY + 0xFA);
    shape.tpage = getTPage(0, 1, 0x1C0, this->vramY);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, zShift);
}

/* The entity class of vtable D_800691EC (0x38 bytes), made for a character 0x40 below a position. */
typedef struct Unk80091498 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ s16 facing; /* Fighter_getFacing of the character: 1 or -1 */
    /* 0x1E */ u16 age;   /* frames: func_80091584 counts it up to 41 */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ Vec3 position;
} Unk80091498;

/* Constructs the entity of vtable D_800691EC for a character, 0x40 below position. obj is not read
 * (ImperialdramonSpecial3Effect_tick passes it). */
Unk80091498 *func_80091498(Unk80091498 *this, Fighter *fighter, Vec3 *position, Fight *obj) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800691EC;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = fighter;
    this->age = 0;
    this->unk20 = 0;
    this->facing = Fighter_getFacing(fighter);
    this->unk24 = 0x1C00;
    x = position->vx;
    y = position->vy + 0x40;
    z = position->vz;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The destructor of the entity class of vtable D_800691EC, which func_80091498 installs. */
void func_80091530(Entity *this, s32 flags) {
    this->vtable = &D_800691EC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Its tick: unk20 eases a quarter of the way to 0x80, after 10 frames unk24 eases a quarter of the way to 0, and
 * after 40 frames it deletes itself. */
void func_80091584(Unk80091498 *this) {
    this->unk20 += (0x80 - this->unk20) >> 2;
    if (this->age > 10) {
        this->unk24 += -this->unk24 >> 2;
    }
    if (this->age > 40) {
        entityDelete(&this->entity);
    } else {
        this->age++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800691B0);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", func_80091620);

/* The destructor of the entity class of vtable D_80069364, which func_8008E9B4 installs. */
void func_80091908(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006933C, which func_8008EA58 installs. */
void func_80091950(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable HIT_SPARK_VTABLE, which HitSpark_init
 * installs: game has 2 identical ones. */
void func_80091998(Entity *this, s32 flags) {
    this->vtable = &HIT_SPARK_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an Unk800919EC: HitSpark_init's entity at position, then vramY and the vtable D_800691C4. */
Unk800919EC *func_800919EC(Unk800919EC *this, u16 vramY, Vec3 *position, VECTOR *unk, s32 unk3, u32 color) {
    /* The match depends on HitSpark_init being declared to return this (it does): its v0 sends the vtable to v1. */
    HitSpark_init(&this->base, position, unk, unk3, color); /* the base class constructor */
    this->vramY = vramY;
    this->base.entity.vtable = &D_800691C4;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800691C4);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_800691EC);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL3_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_8006933C);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069364);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", IMPERIALDRAMON_VTABLE);
