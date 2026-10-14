/* Imperialdramon (CHARACTER_IMPERIALDRAMON): the fighter createFighter builds for it, its moves (Paladin Mode's class
 * derives from it, imperialdramon_paladin_mode.c), and the entities they spawn: the special move 1's hitbox and
 * effect, the special move 2's hitbox and trail, and the special move 3's hold, effect and hitbox. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"

Entity *ImperialdramonSpecial1Effect_init(Entity *this, Fighter *chara, VECTOR *pos, Fight *fight);
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
    /* 0x18 */ u8 unk18[0x38 - 0x18];
    /* 0x38 */ Vec3 unk38; /* the centre, ImperialdramonSpecial1Effect_init's third argument */
    /* 0x44 */ s32 unk44;
    /* 0x48 */ VECTOR unk48[10];
    /* 0xE8 */ s32 unkE8[10];
} ImperialdramonSpecial1Effect;
void Imperialdramon_spawnSpecial1Hitbox(Fighter *this);
void Imperialdramon_spawnSpecial1Effect(Fighter *this, s32 air);
void Imperialdramon_spawnAttackBurst(Fighter *this);
/* the constructors of the entities spawned below */
Entity *ImperialdramonSpecial1Hitbox_init(void *this, Fighter *chara, Vec3 *position, Vec3 *velocity, Fight *unk228);
Entity *ImperialdramonSpecial3Effect_init(void *this, Fighter *chara, Vec3 *position, Vec3 *velocity, Fight *unk228);
struct Unk80091498 *func_80091498(struct Unk80091498 *this, Fighter *chara, Vec3 *position, Fight *obj);
extern VECTOR D_800690D0;

/* Builds Imperialdramon: a Fighter with its own vtable and its hold state from 0x4A6 cleared. */
Imperialdramon *Imperialdramon_init(Imperialdramon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &IMPERIALDRAMON_VTABLE;
    this->special3Parts = 0;
    this->holding = 0;
    this->hitPending = 0;
    this->unk4B0 = 0;
    this->unk4B4 = 0;
    this->unk4B8 = 0;
    this->unk4BC = 0;
    this->unk4C0 = 0;
    this->unk4C4 = 0;
    this->unk4C8 = 0;
    this->unk4CC = 0;
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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", Imperialdramon_throw);

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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", Imperialdramon_special3);

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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", func_8008DD50);

s32 Fighter_isInvulnerable(Fighter *this);
Fighter *Fighter_getOpponentInReach(Fighter *this, s32 index);
VECTOR *func_8008DD50(VECTOR *out, Imperialdramon *this);
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size);
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Fighter *owner, Fight *unk228, Vec3 *pos, s32 size);

/* The entity class of vtable D_800691C4: HitSpark_init's (0x48 bytes) with a halfword more; its draw is
 * func_80091328 and its destructor func_80091998. */
typedef struct Unk800919EC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x48 - 0x18];
    /* 0x48 */ u16 vramY; /* Fighter_getVramY of the fighter that spawns it */
} Unk800919EC;

extern EntityVtable D_800691C4;

/* func_800919EC's body, which Imperialdramon_special3Hold has inline. */
static inline void unk800919ECConstruct(Unk800919EC *this, u16 vramY, Vec3 *position, VECTOR *unk, s32 unk3,
                                        u32 color) {
    HitSpark_init((struct HitSpark *)this, position, unk, unk3, color);
    this->entity.vtable = &D_800691C4;
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
            this->unk4C0 = x;
            this->unk4C4 = y;
            this->unk4C8 = z;
            this->unk4B0 = x;
            this->unk4B4 = y;
            this->unk4B8 = z;
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

Entity *ImperialdramonSpecial2Hitbox_init(void *this, Fighter *chara, Vec3 *position, Vec3 *velocity, Fight *unk228);

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
Entity *ImperialdramonSpecial3Hitbox_init(void *this, Fighter *chara, Vec3 *position, Vec3 *velocity, Vec3 *unk4);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069138);

/* Spawns the special move 3's hitbox at the fighter's node 1 moved by D_80069138 and 256 forward, 256 a frame forward.
 */
void Imperialdramon_spawnSpecial3Hitbox(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Vec3 unk4;
    Entity *child;

    Fighter_getNodePosition(this, (VECTOR *)&position, 1, &D_80069138);
    position.vx += Fighter_getFacing(this) * 256;
    velocity.vx = Fighter_getFacing(this) * 256;
    velocity.vy = 0;
    velocity.vz = 0;
    unk4.vx = Fighter_getFacing(this) * 0x71C0;
    unk4.vy = -0x38E0;
    unk4.vz = 0;
    child = allocEntityMemory(0xAC);
    if (child != NULL) {
        ImperialdramonSpecial3Hitbox_init(entityOperatorNewAt(0xAC, child), this, &position, &velocity, &unk4);
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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Hitbox_init);

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
    Entity *chara;
    Vec3 position;

    Hit_apply(&this->base, sum);
    chara = getHitFighter(sum);
    if (chara != NULL) {
        Fighter_getPosition((Fighter *)chara, &position);
        this->part.flags |= COLLISION_SHAPE_DISABLED;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Effect_init);

/* The special move 1 effect's destructor. */
void ImperialdramonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069148);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", func_8008F7F8);

/* The direction (ratan2) from the centre at 0x38 to point index, on x and y. */
s16 func_8008F9B0(ImperialdramonSpecial1Effect *this, s32 index) {
    return ratan2(this->unk48[index].vx - this->unk38.vx, this->unk48[index].vy - this->unk38.vy);
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial1Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial2Hitbox_init);

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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial2Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial2Hitbox_hit);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069158);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial2Hitbox_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon", D_80069178);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial3Effect_init);

/* The special move 3 effect's destructor. */
void ImperialdramonSpecial3Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_SPECIAL3_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 3's effect (0x920 bytes): 64 particles drawn to a point of the character, with a square of four
 * corners. */
typedef struct ImperialdramonSpecial3Effect {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Fighter *chara;
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

    Fighter_getNodePosition(this->chara, &this->position, 1, &offset);
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
            func_80091498(entityOperatorNewAt(0x38, child), this->chara, (Vec3 *)&this->position, this->obj);
        }
        Fighter_addEntity(this->chara, child);
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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial3Hitbox_init);

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

/* The special move 3's hitbox (0xAC bytes): parts that move from pos by vel every frame until they have gone
 * 0x3000. */
typedef struct ImperialdramonSpecial3Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Imperialdramon *owner;
    /* 0x84 */ s32 speed;    /* vel's length (vecGetLength) */
    /* 0x88 */ s32 distance; /* grows by speed every frame */
    /* 0x8C */ Vec3 vel;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 pos;
    /* 0xA8 */ s32 unkA8;
} ImperialdramonSpecial3Hitbox;

/* Flies by its velocity, the sphere with it; deletes itself when the owner's 0x4A4 is set or its special move 3 has
 * ended, or once it has gone over 0x3000. */
void ImperialdramonSpecial3Hitbox_tick(ImperialdramonSpecial3Hitbox *this) {
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    s32 x = this->pos.vx;
    s32 y = this->pos.vy;
    s32 z = this->pos.vz;

    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
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

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", ImperialdramonSpecial3Hitbox_hit);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon", func_80091328);

/* The entity class of vtable D_800691EC (0x38 bytes), made for a character 0x40 below a position. */
typedef struct Unk80091498 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *chara;
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
Unk80091498 *func_80091498(Unk80091498 *this, Fighter *chara, Vec3 *position, Fight *obj) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800691EC;
    this->vramY = Fighter_getVramY(chara);
    this->chara = chara;
    this->age = 0;
    this->unk20 = 0;
    this->facing = Fighter_getFacing(chara);
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

/* Its update: unk20 eases a quarter of the way to 0x80, after 10 frames unk24 eases a quarter of the way to 0, and
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
    HitSpark_init((struct HitSpark *)this, position, unk, unk3, color); /* the base class constructor */
    this->vramY = vramY;
    this->entity.vtable = &D_800691C4;
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
