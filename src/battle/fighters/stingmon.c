/* Stingmon (CHARACTER_STINGMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's projectile, the special move 2's hitboxes and effects, the special move 3's hitbox, effect and
 * sparks, and the attack 3's hitbox. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/matrix.h"

void Stingmon_spawnSpecial1Projectile(Fighter *this, s32 air);
extern VtableEntry STINGMON_ATTACK3_HITBOX_HIT_VTABLE[];
extern EntityVtable STINGMON_ATTACK3_HITBOX_VTABLE;
extern EntityVtable STINGMON_SPECIAL3_EFFECT_VTABLE;
extern EntityVtable D_8006BBE4;
extern EntityVtable STINGMON_SPECIAL2_EFFECT_VTABLE;
extern VtableEntry STINGMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable STINGMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable STINGMON_VTABLE;

/* Stingmon's class (CHARACTER_STINGMON). */
typedef struct Stingmon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s16 unk4A4;
} Stingmon;

/* The special move 3's hitbox: sixteen points around a centre; it can hold the opponent. */
typedef struct StingmonSpecial3Hitbox {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Hit unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ CollisionSphere part;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ u8 unk84[0x8A - 0x84];
    /* 0x08A */ u16 holdingOpponent; /* while set, the destructor releases the opponent (Fighter_hold(0)) */
    /* 0x08C */ u8 unk8C[0xAC - 0x8C];
    /* 0x0AC */ Vec3 unkAC;
    /* 0x0B8 */ u8 unkB8[4];
    /* 0x0BC */ Vec3 unkBC; /* StingmonSpecial3Hitbox_init's third argument */
    /* 0x0C8 */ u8 unkC8[4];
    /* 0x0CC */ VECTOR unkCC[16];
} StingmonSpecial3Hitbox;
void Stingmon_spawnAttack3Hitbox(Stingmon *this);

/* Builds Stingmon: a Fighter with its own vtable, 0x4A4 0. */
Stingmon *Stingmon_init(Stingmon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &STINGMON_VTABLE;
    this->unk4A4 = 0;
    return this;
}

/* Stingmon's destructor. */
void Stingmon_destroy(Entity *this, s32 flags) {
    this->vtable = &STINGMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: clears 0x4A4. */
void Stingmon_endMove(Stingmon *this) {
    this->unk4A4 = 0;
}

/* Stingmon_dashAttack's local table of four entries, copied whole from its constant (.rodata stays in asm). */
typedef struct Unk8006B87C {
    TrailPoint entries[4];
} Unk8006B87C;

extern Unk8006B87C D_8006B87C;

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006B87C);

/* Dash attack: Fighter's, with a trail (a copy of D_8006B87C, 4, 5, 5) at frame 8. */
void Stingmon_dashAttack(Fighter *this, s32 frame) {
    Unk8006B87C table;
    Entity *child;

    table = D_8006B87C;
    Fighter_dashAttack(this, frame);
    if (frame == 8) {
        child = allocEntityMemory(0x690);
        if (child != NULL) {
            Trail_init(entityOperatorNewAt(0x690, child), this, table.entries, 4, 5, 5);
        }
        Fighter_addEntity(this, child);
    }
}

/* Attack 3: Fighter's, with the hitbox at frame 5. */
void Stingmon_attack3(Stingmon *this, s32 frame) {
    Fighter_attack3(&this->base, frame);
    if (frame == 5) {
        Stingmon_spawnAttack3Hitbox(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_throw);

/* Special move 1: its clip and the voice at frame 0, the projectile at frame 10. */
void Stingmon_special1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        Fighter_playVoice(this, 0xEF);
        break;
    case 10:
        Stingmon_spawnSpecial1Projectile(this, 0);
        break;
    }
}

void Stingmon_spawnSpecial2Hitbox(Fighter *this, s32 air);
void Stingmon_spawnSpecial2Effect(Fighter *this, s32 arg);
struct Unk800B25CC *func_800B25CC(struct Unk800B25CC *this, Fighter *fighter, Vec3 *position);

/* Special move 2: frame 0 plays its clip and the voice, stops the horizontal velocity and spawns the hitbox; frame 4
 * the effect func_800B25CC at the fighter's position; even frames 6-14 the effect; frames 5-17 accelerate by 0x71C0
 * forward; the horizontal force and velocity stop once Fighter_isActionClipDone holds. */
void Stingmon_special2(Fighter *this, s32 frame) {
    Vec3 position;

    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL2, 0, 0, -1);
        this->velocity.vx = 0;
        Fighter_playVoice(this, 0xF0);
        Stingmon_spawnSpecial2Hitbox(this, 0);
        break;
    case 4:
        Fighter_getPosition(this, &position);
        Fighter_addEntity(this, NEW_ENTITY(0x3C, func_800B25CC, this, &position));
        break;
    }
    if (frame >= 6 && frame < 16) {
        /* The match depends on even being a value (do_store_flag's xori/andi), not a test in the if. */
        s32 even = (frame & 1) == 0;

        if (even) {
            Stingmon_spawnSpecial2Effect(this, 0);
        }
    }
    if (frame >= 5 && frame < 18) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 0x71C0);
    }
    if (Fighter_isActionClipDone(this)) {
        this->force.vx = 0;
        this->velocity.vx = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_special3);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_airUpAttack);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_airSpecial1);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_airSpecial2);

/* Up attack: frame 0 its clip, the velocity stopped; frame 4 a push forward of 0xE38 times the mass (16.16) and attack
 * parts 0x20 with hit 0x439012E6 and an upward knockback of 0x6388 times the opponent's mass; frame 10 a trail (a copy
 * of D_8006B87C, 4, 6, 6); frame 12 the same parts with 0x4B9012E6 and a downward knockback; frame 19 clears the attack
 * parts; from frame 17 the force brakes a quarter of the horizontal velocity. The match depends on hitKnockback.vy
 * being set before hitKind and the rest of hitKnockback: the game loads opponent->mass before those stores, which a
 * store through this (it may alias opponent) would keep it after. */
void Stingmon_upAttack(Fighter *this, s32 frame) {
    Unk8006B87C table;
    Fighter *opponent;
    Entity *child;

    table = D_8006B87C;
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_UP_ATTACK, 0, 0, -1);
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->velocity.vz = 0;
        this->velocityPad = 0;
        break;
    case 4: {
        s64 speed;

        this->velocity.vx = 0;
        speed = ((s64)this->mass * 0xE38) >> 16;
        this->force.vx += (s32)speed * Fighter_getFacing(this);
        this->hitDamage = 0x439012E6;
        Fighter_setAttackParts(this, 0x20);
        this->hitKnockback.vy = -(s32)(((s64)opponent->mass * 0x6388) >> 16);
        this->hitKnockback.vx = 0;
        this->hitKnockback.vz = 0;
        this->hitKind = 1;
        break;
    }
    case 10:
        child = allocEntityMemory(0x690);
        if (child != NULL) {
            Trail_init(entityOperatorNewAt(0x690, child), this, table.entries, 4, 6, 6);
        }
        Fighter_addEntity(this, child);
        break;
    case 12:
        this->hitDamage = 0x4B9012E6;
        Fighter_setAttackParts(this, 0x20);
        this->hitKnockback.vy = ((s64)opponent->mass * 0x6388) >> 16;
        this->hitKnockback.vx = 0;
        this->hitKnockback.vz = 0;
        this->hitKind = 1;
        break;
    case 19:
        this->attackParts = 0;
        break;
    }
    if (frame >= 17) {
        this->force.vx += -this->velocity.vx >> 2;
    }
}

extern EntityVtable D_8006BC7C;

/* Constructs an Unk8008E9B4 with vtable D_8006BC7C (see func_8008E9B4), with 12 in unk4C and 24 in unk50. */
Unk8008E9B4 *func_800B146C(Unk8008E9B4 *this, Fighter *owner, s32 arg2, Vec3 *pos, s32 arg4) {
    func_800C3CFC(&this->base, pos, arg4, 0xFFC080);
    this->base.entity.vtable = &D_8006BC7C;
    this->unk54 = owner;
    this->base.unk4C = 12;
    this->base.unk50 = 24;
    return this;
}

/* The tick of the Unk8008E9B4 of D_8006BC7C: func_800C3E64, then puts the world position of the
 * owner's point 3 (Fighter_getNodePosition) at 0x28. */
void func_800B14D0(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    Fighter_getNodePosition(this->unk54, &this->base.unk28, 3, &origin);
}

extern VECTOR STINGMON_SPECIAL1_PROJECTILE_OFFSET;

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL1_PROJECTILE_OFFSET);

struct StingmonSpecial1Projectile *StingmonSpecial1Projectile_init(struct StingmonSpecial1Projectile *this,
                                                                   Fighter *fighter, Vec3 *position, Vec3 *velocity);

/* Spawns the special move 1's projectile at the fighter's point 20 (moved by STINGMON_SPECIAL1_PROJECTILE_OFFSET):
 * forward at 112 per frame, or in the air (1 from airSpecial1) forward and down at 90. */
void Stingmon_spawnSpecial1Projectile(Fighter *this, s32 air) {
    VECTOR offset = STINGMON_SPECIAL1_PROJECTILE_OFFSET;
    VECTOR position;
    Vec3 velocity;
    Entity *child;

    Fighter_getNodePosition(this, &position, 20, &offset);
    if (air == 0) {
        velocity.vx = Fighter_getFacing(this) * 112;
        velocity.vy = 0;
    } else {
        velocity.vx = Fighter_getFacing(this) * 90;
        velocity.vy = 90;
    }
    velocity.vz = 0;
    child = allocEntityMemory(0xE0);
    if (child != NULL) {
        StingmonSpecial1Projectile_init(entityOperatorNewAt(0xE0, child), this, (Vec3 *)&position, &velocity);
    }
    Fighter_addEntity(this, child);
}

struct StingmonSpecial2Hitbox *StingmonSpecial2Hitbox_init(struct StingmonSpecial2Hitbox *this, Fighter *fighter,
                                                           Vec3 *position, VECTOR *velocity, Fight *list);

/* Spawns the special move 2's hitbox at the fighter's position, flying forward at 0x4718 times the mass (16.16). Its
 * velocity's z carries air (1 from airSpecial2), which StingmonSpecial2Hitbox_init reads. */
void Stingmon_spawnSpecial2Hitbox(Fighter *this, s32 air) {
    Vec3 position;
    VECTOR velocity;
    Fight *list;
    s64 speed;
    Entity *child;

    list = Fighter_getScene(this);
    Fighter_getPosition(this, &position);
    speed = ((s64)this->mass * 0x4718) >> 16;
    velocity.vx = (s32)speed * Fighter_getFacing(this);
    velocity.vy = 0;
    velocity.vz = air;
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        StingmonSpecial2Hitbox_init(entityOperatorNewAt(0xC0, child), this, &position, &velocity, list);
    }
    Fighter_addEntity(this, child);
}

struct StingmonSpecial2Effect *StingmonSpecial2Effect_init(struct StingmonSpecial2Effect *this, Fighter *fighter,
                                                           Vec3 *position, s32 arg3);

/* Spawns the special move 2's effect at the fighter's point 20, 128 ahead and 0x40 up (with arg, 0x1B down), z 0. */
void Stingmon_spawnSpecial2Effect(Fighter *this, s32 arg) {
    VECTOR position;
    VECTOR origin;
    Entity *child;
    s32 y;

    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    Fighter_getNodePosition(this, &position, 20, &origin);
    position.vx += Fighter_getFacing(this) * 128;
    y = position.vy;
    position.vz = 0;
    /* The match depends on storing the default height first and overwriting it with arg */
    position.vy = y - 0x40;
    if (arg != 0) {
        position.vy = y + 0x1B;
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        StingmonSpecial2Effect_init(entityOperatorNewAt(0x3C, child), this, (Vec3 *)&position, arg);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006B93C);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", Stingmon_spawnSpecial3Hitbox);

Entity *StingmonSpecial3Effect_init(Entity *this, Fighter *fighter, Vec3 *position);

/* Spawns the special move 3's effect 640 ahead of the fighter and 0xF0 up. */
void Stingmon_spawnSpecial3Effect(Fighter *this) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vx += Fighter_getFacing(this) * 640;
    position.vy -= 0xF0;
    child = allocEntityMemory(0x1F4);
    if (child != NULL) {
        StingmonSpecial3Effect_init(entityOperatorNewAt(0x1F4, child), this, &position);
    }
    Fighter_addEntity(this, child);
}

UnkPartsEntity38 *StingmonAttack3Hitbox_init(UnkPartsEntity38 *this, Fighter *fighter, Vec3 *position, VECTOR *velocity,
                                             Fight *list);

/* Spawns the attack 3's hitbox at the fighter's position, flying forward at 0x38E0 and up at 0x1C70 times the mass
 * (16.16). */
void Stingmon_spawnAttack3Hitbox(Stingmon *this) {
    Vec3 position;
    VECTOR velocity;
    Fight *list;
    s64 speed;
    Entity *child;

    list = Fighter_getScene(&this->base);
    Fighter_getPosition(&this->base, &position);
    speed = ((s64)this->base.mass * 0x38E0) >> 16;
    velocity.vx = (s32)speed * Fighter_getFacing(&this->base);
    velocity.vy = -(s32)(((s64)this->base.mass * 0x1C70) >> 16);
    velocity.vz = 0;
    child = allocEntityMemory(0xBC);
    if (child != NULL) {
        StingmonAttack3Hitbox_init(entityOperatorNewAt(0xBC, child), &this->base, &position, &velocity, list);
    }
    Fighter_addEntity(&this->base, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial2Hitbox_init);

/* The special move 2 hitbox's destructor. */
void StingmonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = STINGMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &STINGMON_SPECIAL2_HITBOX_VTABLE;
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

/* The UnkPartsEntity38 class StingmonSpecial2Hitbox_init constructs (virtual tables STINGMON_SPECIAL2_HITBOX_VTABLE and
 * STINGMON_SPECIAL2_HITBOX_HIT_VTABLE). */
typedef struct StingmonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *fighter;
    /* 0x84 */ u16 frame;
    /* 0x86 */ s16 facing; /* the character's (Fighter_getFacing) */
    /* 0x88 */ u8 unk88;   /* 10, or 3: the frames state 2 waits */
    /* 0x89 */ u8 unk89;   /* rises to 0x80 in state 1, falls to 0 in state 3 */
    /* 0x8A */ u16 state;
    /* 0x8C */ u16 unk8C;  /* state 2's frame count */
    /* 0x8E */ s16 vramY;  /* the character's 0x440 (Fighter_getVramY) */
    /* 0x90 */ VECTOR position; /* the character's point 20 (Fighter_getNodePosition) */
} StingmonSpecial2Hitbox;

extern VECTOR D_800D344C;

/* The special move 2 hitbox's tick: states 0-3 fade unk89 in, hold it unk88 frames and fade it out, then it
 * deletes itself; with unk88 10, frame 7 launches it (Hit_set and a velocity from the character's unk3C4
 * and side). Its part follows the character's point 20; flag 1 of the part is set from frame 16 (23 with 10).
 * The match depends on: the two flag clears as two statements (the game ands with -3 and -2, one store); vx and
 * vy computed before the stores (the game reads the character again before storing vx); the flag 1 test as one
 * branch per threshold (cross-jumping shares their tail; one ternary condition lays the branches out otherwise). */
void StingmonSpecial2Hitbox_tick(StingmonSpecial2Hitbox *this) {
    switch (this->state) {
    case 0:
        if (this->unk88 == 10 && this->frame < 3) {
            break;
        }
        this->parts.part.flags &= ~2;
        this->parts.part.flags &= ~COLLISION_SHAPE_DISABLED;
        this->state++;
    case 1:
        this->unk89 += (0x84 - this->unk89) >> 2;
        if ((s8)this->unk89 >= 0) {
            break;
        }
        this->unk89 = 0x80;
        this->unk8C = 0;
        this->state++;
    case 2:
        if (++this->unk8C < this->unk88) {
            break;
        }
        this->state++;
    case 3:
        this->unk89 -= (this->unk89 + 4) >> 2;
        if (this->unk89 != 0) {
            break;
        }
        this->state++;
    case 4:
        entityDelete(&this->parts.entity);
        return;
    }
    if (this->frame == 7 && this->unk88 == 10) {
        VECTOR velocity;
        s64 speed;
        s32 vx, vy;

        this->parts.part.flags &= ~2;
        Hit_set(&this->parts.base, 0x47AC1FD9, 2);
        speed = ((s64)this->fighter->mass * 0x1554) >> 16;
        vx = (s32)speed * Fighter_getFacing(this->fighter);
        vy = -(s32)(((s64)this->fighter->mass * 0x4718) >> 16);
        velocity.vx = vx;
        velocity.vy = vy;
        velocity.vz = 0;
        Hit_setKnockback(&this->parts.base, &velocity);
        this->parts.part.flags &= ~COLLISION_SHAPE_DISABLED;
    }
    if (this->unk88 == 10) {
        if (this->frame >= 23) {
            this->parts.part.flags |= COLLISION_SHAPE_DISABLED;
        }
    } else if (this->frame >= 16) {
        this->parts.part.flags |= COLLISION_SHAPE_DISABLED;
    }
    Fighter_getNodePosition(this->fighter, &this->position, 20, &D_800D344C);
    this->parts.part.position.vx = this->position.vx;
    this->parts.part.position.vy = this->position.vy;
    this->parts.part.position.vz = this->position.vz;
    this->frame++;
}

/* On a hit: Hit_apply with arg, then sets bit 0 of 0x44 when getHitFighter finds a fighter in arg. */
void StingmonSpecial2Hitbox_hit(Unk800AD5BC *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial2Hitbox_draw);

/* The entity class of vtable STINGMON_SPECIAL2_EFFECT_VTABLE (0x3C bytes), made for a character at a position. */
typedef struct StingmonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u16 frames; /* counts the frames of StingmonSpecial2Effect_tick */
    /* 0x20 */ s16 facing; /* Fighter_getFacing of the character: 1 or -1 */
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u16 phase; /* StingmonSpecial2Effect_tick's */
    /* 0x26 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} StingmonSpecial2Effect;

/* Builds the special move 2's effect for fighter at position, with arg3 at 0x1C. */
StingmonSpecial2Effect *StingmonSpecial2Effect_init(StingmonSpecial2Effect *this, Fighter *fighter, Vec3 *position,
                                                    s32 arg3) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &STINGMON_SPECIAL2_EFFECT_VTABLE;
    this->vramY = Fighter_getVramY(fighter);
    this->facing = Fighter_getFacing(fighter);
    this->fighter = fighter;
    this->phase = 0;
    this->unk22 = 0x80;
    this->frames = 0;
    this->unk28 = 0x1000;
    this->unk1C = arg3;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The special move 2 effect's destructor. */
void StingmonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &STINGMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 2 effect's tick: in phase 0, 0x28 grows by frames * 128 up to 0x2000 while 0x22 fades
 * by an eighth; once 0x22 is down to 8 the entity deletes itself. */
void StingmonSpecial2Effect_tick(StingmonSpecial2Effect *this) {
    switch (this->phase) {
    case 0:
        this->unk28 += this->frames << 7;
        if (this->unk28 > 0x2000) {
            this->unk28 = 0x2000;
        }
        this->unk22 += -this->unk22 >> 3;
        if (this->unk22 > 8) {
            break;
        }
        this->phase++;
        /* fallthrough */
    case 1:
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BA10);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial2Effect_draw);

/* The entity class of vtable D_8006BBE4 (0x3C bytes), made for a character at a position. */
typedef struct Unk800B25CC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ u16 frames; /* counts the frames of func_800B26B8 */
    /* 0x1E */ s16 facing; /* Fighter_getFacing of the character: 1 or -1 */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x26 */ u8 unk26[2];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} Unk800B25CC;

/* Constructs the entity of vtable D_8006BBE4 for a character at a position. */
Unk800B25CC *func_800B25CC(Unk800B25CC *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BBE4;
    this->vramY = Fighter_getVramY(fighter);
    this->facing = Fighter_getFacing(fighter);
    this->fighter = fighter;
    this->unk22 = 0;
    this->unk20 = 0x80;
    this->frames = 0;
    this->unk28 = 0;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BBE4, which func_800B25CC installs. */
void func_800B2664(Entity *this, s32 flags) {
    this->vtable = &D_8006BBE4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of func_800B25CC's entity: 0x28 eases halfway to 0x1020 (at most 0x1000) every frame while 0x20
 * fades by an eighth; once 0x20 is down to 8 the entity deletes itself. */
void func_800B26B8(Unk800B25CC *this) {
    this->unk28 += (0x1020 - this->unk28) >> 1;
    if (this->unk28 > 0x1000) {
        this->unk28 = 0x1000;
    }
    if (this->unk20 < 9) {
        entityDelete(&this->entity);
    } else {
        this->unk20 += -this->unk20 >> 3;
        this->frames++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BA30);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", func_800B275C);

extern VtableEntry STINGMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];
extern EntityVtable STINGMON_SPECIAL1_PROJECTILE_VTABLE;

/* The special move 1's projectile (a Projectile, 0xE0 bytes). */
typedef struct StingmonSpecial1Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ s32 cell;   /* the tick counts it up to 6, then loops 4 to 6 */
    /* 0xC4 */ s32 angle;  /* ratan2 of the velocity */
    /* 0xC8 */ Vec3 start; /* the position it was fired from */
    /* 0xD4 */ s32 unkD4;
    /* 0xD8 */ s32 fade;   /* 0x80, down by 16 a tick to 0 */
    /* 0xDC */ s32 length; /* 0, up by 0x30 a tick */
} StingmonSpecial1Projectile;

/* Builds the special move 1's projectile, a Projectile of radius 0x90 and speed 0xD00, aimed by its velocity. */
StingmonSpecial1Projectile *StingmonSpecial1Projectile_init(StingmonSpecial1Projectile *this, Fighter *fighter,
                                                            Vec3 *position, Vec3 *velocity) {
    s32 x, y, z;

    Projectile_init(&this->base, fighter, position, velocity, 0x90, 0x7A82100, 0xD00, 3);
    this->base.parts.base.vtable = STINGMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &STINGMON_SPECIAL1_PROJECTILE_VTABLE;
    this->cell = 0;
    this->angle = ratan2(velocity->vy, velocity->vx);
    /* The match depends on reading the position whole before storing it (a struct copy: 13 diffs). */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->start.vx = x;
    this->start.vy = y;
    this->start.vz = z;
    this->fade = 0x80;
    this->length = 0;
    return this;
}

/* Loops the cell, fades and lengthens; once faded out with its sphere in no list it deletes itself, else it flies
 * as a Projectile. */
void StingmonSpecial1Projectile_tick(StingmonSpecial1Projectile *this, s32 arg) {
    s32 alone;

    if (++this->cell >= 7) {
        this->cell = 4;
    }
    if ((this->fade -= 16) <= 0) {
        this->fade = 0;
    }
    this->length += 0x30;
    if (this->fade <= 0) {
        /* listIsAlone(&this->base.parts.part.link), as in the draw */
        alone = this->base.parts.part.link.next == &this->base.parts.part.link
                && &this->base.parts.part.link == this->base.parts.part.link.prev;
        if (alone) {
            entityDelete(&this->base.parts.entity);
            return;
        }
    }
    Projectile_tick(&this->base, arg);
}

void StingmonSpecial1Projectile_drawHead(StingmonSpecial1Projectile *this, Camera *camera);
void StingmonSpecial1Projectile_drawStart(StingmonSpecial1Projectile *this, Camera *camera);

/* Draws the projectile: its head while its sphere is in a list, then its start. */
void StingmonSpecial1Projectile_draw(Projectile *this, s32 arg) {
    s32 alone;

    /* listIsAlone(&this->parts.part.node), reading at offsets from this (as entity.c's tick) */
    alone = this->parts.part.link.next == &this->parts.part.link
            && &this->parts.part.link == this->parts.part.link.prev;
    if (!alone) {
        StingmonSpecial1Projectile_drawHead((StingmonSpecial1Projectile *)this, (Camera *)arg);
    }
    StingmonSpecial1Projectile_drawStart((StingmonSpecial1Projectile *)this, (Camera *)arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS);

/* PolyFT4Shape's uv words as their u and v bytes (as in saintgargomon.c): game_classes.h types them u16, and the game
 * stores the bytes. */
#define UV_BYTES(uv) ((u8 *)&(uv))

/* The corners of the rectangle StingmonSpecial1Projectile_drawHead draws. */
extern SVECTOR STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS[4];

/* Draws the projectile's head: the rectangle STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS turned by its angle, scaled
 * by its radius and placed at its position, with the 48 by 48 picture of cell (cell >> 1) & 3 in grey,
 * semi-transparent, with buildPolyFT4.
 * The match depends on the matrix built as matrixInitTranslationVec does, reading the position through a pointer,
 * and on a pointer to it for the three calls (the game keeps it in $s1). */
void StingmonSpecial1Projectile_drawHead(StingmonSpecial1Projectile *this, Camera *camera) {
    MatrixWords m;
    MATRIX *matrix;
    PolyFT4Shape shape;
    Vec3 *position;
    s32 x, y, z;
    u8 u;

    position = &this->base.position;
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
    matrix = &m.m;
    matrixSetRotationZ(matrix, this->angle);
    matrixScale(matrix, this->base.radius, this->base.radius, this->base.radius);
    setModelMatrix(matrix);
    shape.corners[0] = &STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS[0];
    shape.corners[1] = &STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS[1];
    shape.corners[2] = &STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS[2];
    shape.corners[3] = &STINGMON_SPECIAL1_PROJECTILE_HEAD_CORNERS[3];
    shape.color = 0x808080;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0xA0;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xCF;
    shape.flags = 9;
    u = ((this->cell >> 1) & 3) * 48 + 0x40;
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = u;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = u + 0x2F;
    shape.tpage = getTPage(0, 1, 0x1C0, this->base.unkBC);
    shape.clut = getClut(0x1C0, this->base.unkBC + 0xFA);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL1_PROJECTILE_START_CORNERS);

/* The corners of the square StingmonSpecial1Projectile_drawStart draws. */
extern SVECTOR STINGMON_SPECIAL1_PROJECTILE_START_CORNERS[4];

/* Draws the projectile's start: the square STINGMON_SPECIAL1_PROJECTILE_START_CORNERS turned by its angle, scaled by
 * its length and placed where it was fired from, with a 64 by 32 picture in its fade's grey (red at half),
 * semi-transparent, with buildPolyFT4.
 * The match depends on the matrix built as matrixInitTranslationVec does, reading the start through a pointer, and on
 * a pointer to it for the three calls (the game keeps it in $s1). */
void StingmonSpecial1Projectile_drawStart(StingmonSpecial1Projectile *this, Camera *camera) {
    MatrixWords m;
    MATRIX *matrix;
    PolyFT4Shape shape;
    Vec3 *start;
    s32 x, y, z;

    start = &this->start;
    x = start->vx;
    y = start->vy;
    z = start->vz;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    matrix = &m.m;
    matrixSetRotationZ(matrix, this->angle);
    matrixScale(matrix, this->length, this->length, this->length);
    setModelMatrix(matrix);
    shape.corners[0] = &STINGMON_SPECIAL1_PROJECTILE_START_CORNERS[0];
    shape.corners[1] = &STINGMON_SPECIAL1_PROJECTILE_START_CORNERS[1];
    shape.corners[2] = &STINGMON_SPECIAL1_PROJECTILE_START_CORNERS[2];
    shape.corners[3] = &STINGMON_SPECIAL1_PROJECTILE_START_CORNERS[3];
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv1)[0] = 0xBF;
    UV_BYTES(shape.uv2)[0] = UV_BYTES(shape.uv3)[0] = 0x80;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv2)[1] = 0;
    UV_BYTES(shape.uv1)[1] = UV_BYTES(shape.uv3)[1] = 0x1F;
    shape.flags = 9;
    shape.color = (this->fade >> 1) | (this->fade << 8) | (this->fade << 16);
    shape.tpage = getTPage(0, 1, 0x1C0, this->base.unkBC);
    shape.clut = getClut(0x1C0, this->base.unkBC + 0x7C);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Hitbox_init);

extern VtableEntry STINGMON_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable STINGMON_SPECIAL3_HITBOX_VTABLE;

/* The special move 3 hitbox's destructor: first releases the opponent while it holds it. */
void StingmonSpecial3Hitbox_destroy(StingmonSpecial3Hitbox *this, s32 flags) {
    ListNode *node;

    this->unk18.vtable = STINGMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &STINGMON_SPECIAL3_HITBOX_VTABLE;
    if (this->holdingOpponent != 0) {
        Fighter_hold(Fighter_getOpponent(this->fighter), 0);
    }
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->unk18.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Hitbox_hit);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", func_800B34D0);

/* The direction (ratan2) from the centre at 0xAC to point index, on x and y. */
s16 func_800B36B4(StingmonSpecial3Hitbox *this, s32 index) {
    return ratan2(this->unkCC[index].vx - this->unkAC.vx, this->unkCC[index].vy - this->unkAC.vy);
}

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BA90);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BAB0);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Effect_init);

/* The special move 3 effect's destructor. */
void StingmonSpecial3Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &STINGMON_SPECIAL3_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonSpecial3Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonAttack3Hitbox_init);

/* The attack 3 hitbox's destructor. */
void StingmonAttack3Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = STINGMON_ATTACK3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &STINGMON_ATTACK3_HITBOX_VTABLE;
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

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BAD0);

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonAttack3Hitbox_tick);

/* On a hit: Hit_apply with arg, then sets bit 0 of 0x44 when getHitFighter finds a fighter in arg. */
void StingmonAttack3Hitbox_hit(Unk800AD5BC *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/stingmon", StingmonAttack3Hitbox_draw);

/* The destructor of the entity class of vtable D_8006BC7C, which func_800B146C installs. */
void func_800B44D8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void StingmonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = PROJECTILE_HIT_VTABLE;
    this->entity.vtable = &PROJECTILE_VTABLE;
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

INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_ATTACK3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_ATTACK3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL3_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BBE4);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", D_8006BC7C);
INCLUDE_RODATA("game/nonmatchings/fighters/stingmon", STINGMON_VTABLE);
