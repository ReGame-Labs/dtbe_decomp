/* Wormmon (CHARACTER_WORMMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * guard's effect, the special move 1's hitbox and its hit's effect, and the special move 2's trail. */

#include "common.h"
#include "battle/game_classes.h"

void Wormmon_spawnSpecial1Hitbox(Fighter *this, s32 arg);
extern FaceEntry WORMMON_FACES[];
extern VtableEntry WORMMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable WORMMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable WORMMON_VTABLE;
struct WormmonSpecial1Hitbox *WormmonSpecial1Hitbox_init(struct WormmonSpecial1Hitbox *this, Fighter *fighter,
                                                         Vec3 *position, Vec3 *velocity, Fight *fight);
struct WormmonGuardEffect *WormmonGuardEffect_init(struct WormmonGuardEffect *this, Fighter *fighter,
                                                   s32 crouching);

/* Builds Wormmon: a Fighter with its own vtable and faces, no guard effect out. */
Wormmon *Wormmon_init(Wormmon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &WORMMON_VTABLE;
    Fighter_setFaceTable(&this->base, WORMMON_FACES);
    this->guardEffectActive = 0;
    return this;
}

/* Wormmon's destructor. */
void Wormmon_destroy(Entity *this, s32 flags) {
    this->vtable = &WORMMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Guard: Fighter's, with the guard effect (0) unless one is out. */
void Wormmon_guard(Wormmon *this, s32 frame) {
    Fighter_guard(&this->base, frame);
    if (this->guardEffectActive == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            WormmonGuardEffect_init(entityOperatorNewAt(0x4C, child), &this->base, 0);
        }
        Fighter_addEntity(&this->base, child);
        this->guardEffectActive = 1;
    }
}

/* Crouching guard: Fighter's, with the guard effect (1) unless one is out. */
void Wormmon_crouchGuard(Wormmon *this, s32 frame) {
    Fighter_crouchGuard(&this->base, frame);
    if (this->guardEffectActive == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            WormmonGuardEffect_init(entityOperatorNewAt(0x4C, child), &this->base, 1);
        }
        Fighter_addEntity(&this->base, child);
        this->guardEffectActive = 1;
    }
}

/* Throw: Fighter's; at frame 0 the opponent is held (8) flinching and both fighters stop; at frame 8, on the ground,
 * hits the opponent forward and up with a spark at its position + (0, -0x100, -0x100), else lets go
 * (Fighter_stopActionClip). */
void Wormmon_throw(Fighter *this, s32 frame) {
    Fighter *opponent;
    Vec3 velocity;
    Vec3 position;
    VECTOR direction;
    Entity *child;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    if (frame == 0) {
        Fighter_hold(opponent, 8);
        Fighter_setClip(opponent, FIGHTER_CLIP_FLINCH, 0, 0);
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->velocity.vz = 0;
        this->velocityPad = 0;
    }
    if (frame == 8) {
        if (Fighter_isOnGround(this)) {
            velocity.vx = (s16)Fighter_getFacing(this) * 9100;
            velocity.vy = -0x38E0;
            velocity.vz = 0;
            Fighter_takeHit(opponent, 0x5B8C1A00, &velocity, 2);
            direction.vx = 0;
            direction.vy = 0;
            direction.vz = 0;
            direction.pad = 0;
            Fighter_getPosition(opponent, &position);
            position.vy -= 0x100;
            position.vz -= 0x100;
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                HitSpark_init(entityOperatorNewAt(0x48, child), &position, &direction, 0x60, 0x808080);
            }
            Fighter_addEntity(this, child);
        } else {
            Fighter_stopActionClip(this);
        }
    }
}

/* Special move 1: its clip and the voice at frame 0, the hitbox (spawnSpecial1Hitbox(0)) at frame 13. */
void Wormmon_special1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        Fighter_playVoice(this, 0x105);
        break;
    case 13:
        Wormmon_spawnSpecial1Hitbox(this, 0);
        break;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4AC);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4B4);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4BC);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4C4);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4CC);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4D4);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4DC);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C4EC);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", WORMMON_FACES);

void Wormmon_spawnSpecial2Trail(Fighter *this);

/* Special move 2: frame 0 its clip, attack parts 0x202 with hit 0x7AC1400 and a forward knockback of 0x4718 times the
 * opponent's mass (16.16), and the voice; frame 6 the trail; frame 7 a push up of 0x31C4 times the mass; frame 10 the
 * same parts with 0x87AC1400 and 0x2AA8; frame 29 clears the attack parts; frames 8 to 28 an acceleration forward; a
 * fall is braked whole. The match depends on a block-scoped speed in each case: one s64 for both cases is set in two
 * blocks, becomes a global pseudo and moves the product out of $s0/$s1. */
void Wormmon_special2(Fighter *this, s32 frame) {
    Fighter *opponent;

    func_80079AE4(this);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0: {
        s64 speed;

        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL2, 0, 0, -1);
        Fighter_setAttackParts(this, 0x202);
        this->hitDamage = 0x7AC1400;
        this->hitKind = 1;
        speed = ((s64)opponent->mass * 0x4718) >> 16;
        this->hitKnockback.vx = (s32)speed * Fighter_getFacing(this);
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        Fighter_playVoice(this, 0x106);
        break;
    }
    case 6:
        Wormmon_spawnSpecial2Trail(this);
        break;
    case 7:
        this->force.vy -= ((s64)this->mass * 0x31C4) >> 16;
        break;
    case 10: {
        s64 speed;

        Fighter_setAttackParts(this, 0x202);
        this->hitDamage = 0x87AC1400;
        this->hitKind = 1;
        speed = ((s64)opponent->mass * 0x2AA8) >> 16;
        this->hitKnockback.vx = (s32)speed * Fighter_getFacing(this);
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        break;
    }
    case 29:
        this->attackParts = 0;
        break;
    }
    if (frame >= 8 && frame <= 28) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 10920);
    }
    if (this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

/* Up attack in the air: Fighter's; frame 0 its clip and the vertical velocity stopped; frame 5 a push up of 0x4718
 * times the mass (16.16); a fall's velocity is braked whole. */
void Wormmon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_UP_ATTACK, 2, 0, -1);
        this->velocity.vy = 0;
        break;
    case 5:
        this->force.vy -= ((s64)this->mass * 0x4718) >> 16;
        break;
    }
    if (this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

/* Up attack: Fighter's; frame 4 the horizontal velocity stopped and a push forward of 0x38E0 times the mass (16.16);
 * from frame 5 the force brakes a quarter of the horizontal velocity. */
void Wormmon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 4) {
        s32 push;

        this->velocity.vx = 0;
        push = ((s64)this->mass * 0x38E0) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
    }
    if (frame >= 5) {
        this->force.vx += -this->velocity.vx >> 2;
    }
}

/* Special move 1 in the air: at frame 0 its clip, a stop and the voice; at frame 5 a push back; the hitbox at frame
 * 11; from frame 6 a rise is braked and the horizontal velocity loses 1/128 a frame. */
void Wormmon_airSpecial1(Fighter *this, s32 frame) {
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 0, 0, -1);
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        Fighter_playVoice(this, 0x105);
        break;
    case 5: {
        s32 push = ((s64)this->mass * 0x1C70) >> 16;

        this->force.vx += push * -Fighter_getFacing(this);
        break;
    }
    case 11:
        Wormmon_spawnSpecial1Hitbox(this, 1);
        break;
    }
    if (frame >= 6) {
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
        this->force.vx -= this->velocity.vx >> 7;
    }
}

/* Special move 2 in the air: as Wormmon_special2, with its own clip (the vertical velocity stopped first) and
 * 0x47AC1400 at frame 10. The match depends on the same block-scoped speeds. */
void Wormmon_airSpecial2(Fighter *this, s32 frame) {
    Fighter *opponent;

    func_80079AE4(this);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0: {
        s64 speed;

        this->velocity.vy = 0;
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL2, 0, 0, -1);
        Fighter_setAttackParts(this, 0x202);
        this->hitDamage = 0x7AC1400;
        this->hitKind = 1;
        speed = ((s64)opponent->mass * 0x4718) >> 16;
        this->hitKnockback.vx = (s32)speed * Fighter_getFacing(this);
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        Fighter_playVoice(this, 0x106);
        break;
    }
    case 6:
        Wormmon_spawnSpecial2Trail(this);
        break;
    case 7:
        this->force.vy -= ((s64)this->mass * 0x31C4) >> 16;
        break;
    case 10: {
        s64 speed;

        Fighter_setAttackParts(this, 0x202);
        this->hitDamage = 0x47AC1400;
        this->hitKind = 1;
        speed = ((s64)opponent->mass * 0x2AA8) >> 16;
        this->hitKnockback.vx = (s32)speed * Fighter_getFacing(this);
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        break;
    }
    case 29:
        this->attackParts = 0;
        break;
    }
    if (frame >= 8 && frame <= 28) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 10920);
    }
    if (this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

/* Wormmon_spawnSpecial2Trail's local table of three entries, copied whole from its constant (.rodata stays in asm). */
typedef struct Unk8006C5FC {
    TrailPoint entries[3];
} Unk8006C5FC;

extern Unk8006C5FC D_8006C5FC;

INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C5FC);

/* Spawns the special move 2's trail (a copy of D_8006C5FC, 3, 8, 23) and plays sound 0x102. */
void Wormmon_spawnSpecial2Trail(Fighter *this) {
    Unk8006C5FC table;
    Entity *child;

    table = D_8006C5FC;
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, table.entries, 3, 8, 23);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0x102);
}

/* Spawns the special move 1's hitbox 0x60 above the fighter (0x70 with arg), flying 48 a frame forward. */
void Wormmon_spawnSpecial1Hitbox(Fighter *this, s32 arg) {
    Fight *fight = Fighter_getScene(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    if (arg == 0) {
        position.vy -= 0x60;
    } else {
        position.vy -= 0x70;
    }
    velocity.vx = Fighter_getFacing(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x184);
    if (child != NULL) {
        WormmonSpecial1Hitbox_init(entityOperatorNewAt(0x184, child), this, &position, &velocity, fight);
    }
    Fighter_addEntity(this, child);
}

/* The special move 1's hitbox (Wormmon_spawnSpecial1Hitbox). */
typedef struct WormmonSpecial1Hitbox {
    /* 0x000 */ UnkPartsEntity38 parts;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ Fight *fight;
    /* 0x088 */ u8 unk88[0xB4 - 0x88];
    /* 0x0B4 */ Vec3 velocity;
    /* 0x0C0 */ u8 unkC0[0x184 - 0xC0];
} WormmonSpecial1Hitbox;

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", WormmonSpecial1Hitbox_init);

/* The special move 1 hitbox's destructor. */
void WormmonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = WORMMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &WORMMON_SPECIAL1_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", WormmonSpecial1Hitbox_tick);

struct Unk800BA734 *func_800BA734(struct Unk800BA734 *this, Fighter *fighter, BattleSide *owner);

/* On a hit: Hit_apply with sum; the fighter it hit, unless it guards, is held (Fighter_hold(25)) and gets
 * func_800BA734's entity for its side; a guarding one gets a hit spark. Then the hitbox deletes itself. */
void WormmonSpecial1Hitbox_hit(WormmonSpecial1Hitbox *this, CollisionSphere *sum) {
    Fighter *target;
    Vec3 position;
    BattleSide *side;
    Entity *child;

    Hit_apply(&this->parts.base, sum);
    target = (Fighter *)getHitFighter(sum);
    if (target != NULL) {
        /* the position is never read */
        Fighter_getPosition(target, &position);
        if (!Fighter_isGuarding(target)) {
            Fighter_hold(target, 25);
            side = func_8007758C(target);
            child = allocEntityMemory(0x4AC);
            if (child != NULL) {
                func_800BA734(entityOperatorNewAt(0x4AC, child), this->fighter, side);
            }
            entityAddSibling(&this->parts.entity, child);
        } else {
            entityAddSibling(&this->parts.entity, createHitSpark(&target->entity, &this->velocity, 0x60));
        }
    }
    entityDelete(&this->parts.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", WormmonSpecial1Hitbox_draw);

/* The entity class func_800BA734 constructs (virtual table D_8006C6F4): an Unk800C4F74 with 0x488 at 5/6. */
typedef struct Unk800BA734 {
    /* 0x000 */ Unk800C4F74 base;
    /* 0x4A8 */ s16 vramY; /* Fighter_getVramY of the character */
} Unk800BA734;
extern EntityVtable D_8006C6F4;

/* Constructs an Unk800BA734 for fighter and owner: func_800C4F74's entity, with 0x488 cut to 5/6. */
Unk800BA734 *func_800BA734(Unk800BA734 *this, Fighter *fighter, BattleSide *owner) {
    func_800C4F74(&this->base, owner);
    this->base.entity.vtable = &D_8006C6F4;
    this->vramY = Fighter_getVramY(fighter);
    this->base.unk488 = this->base.unk488 * 5 / 6;
    return this;
}

/* The tick of func_800BA734's entity: follows the side's current fighter (func_8007A650); in state 0 0x488 grows by
 * 8 until tick 5, state 1 waits until tick 9, state 2 takes 8 off 0x494 a tick and deletes the entity at 0. */
void func_800BA7AC(Unk800BA734 *this) {
    func_8007A650(this->base.owner->current, &this->base.position);
    switch (this->base.unk498) {
    case 0:
        this->base.unk488 += 8;
        if (this->base.unk49C >= 5) {
            this->base.unk498 = 1;
        }
        break;
    case 1:
        if (this->base.unk49C >= 9) {
            this->base.unk498 = 2;
        }
        break;
    case 2:
        this->base.unk494 -= 8;
        if (this->base.unk494 <= 0) {
            entityDelete(&this->base.entity);
            return;
        }
        break;
    }
    this->base.unk4A0 += 10;
    this->base.unk49C++;
}

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", func_800BA8AC);

/* The guard's effect, one at a time for a Wormmon (guardEffectActive). */
typedef struct WormmonGuardEffect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Wormmon *fighter;
    /* 0x1C */ u32 handle; /* the character's (entityGetHandle) */
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 facing; /* Fighter_getFacing */
    /* 0x24 */ s16 vramY;  /* Fighter_getVramY */
    /* 0x26 */ s16 unk26;
    /* 0x28 */ VECTOR fighterPosition; /* the fighter's when made (Fighter_getPosition) */
    /* 0x38 */ VECTOR position; /* where the draw puts the rectangle */
    /* 0x48 */ s32 scale;       /* of the rectangle, in 4096ths; 0 draws nothing */
} WormmonGuardEffect;

extern EntityVtable WORMMON_GUARD_EFFECT_VTABLE;

/* Builds the guard effect for fighter: it keeps its handle and facing, and sits 200 in front of it and 0x84 up when
 * standing (crouching 0), 0xE8 in front and 0x60 up when crouching (1), 0x80 toward the camera. */
WormmonGuardEffect *WormmonGuardEffect_init(WormmonGuardEffect *this, Fighter *fighter, s32 crouching) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &WORMMON_GUARD_EFFECT_VTABLE;
    this->handle = entityGetHandle(&fighter->entity);
    this->facing = Fighter_getFacing(fighter);
    {
        VECTOR offsets[2] = { { this->facing * 200, -0x84 }, { this->facing * 0xE8, -0x60 } };

        Fighter_getPosition(fighter, (Vec3 *)&this->fighterPosition);
        this->position.vz = -0x80;
        /* The match depends on the offset being the first operand */
        this->position.vx = offsets[crouching].vx + this->fighterPosition.vx;
        this->position.vy = offsets[crouching].vy + this->fighterPosition.vy;
    }
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = (Wormmon *)fighter;
    this->unk26 = 0;
    this->scale = 0;
    this->unk20 = 0;
    return this;
}

/* The guard effect's destructor: clears the character's guardEffectActive, so that it can make another. */
void WormmonGuardEffect_destroy(WormmonGuardEffect *this, s32 flags) {
    this->entity.vtable = &WORMMON_GUARD_EFFECT_VTABLE;
    this->fighter->guardEffectActive = 0;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", WormmonGuardEffect_tick);

INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C644);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C664);

INCLUDE_ASM("game/nonmatchings/fighters/wormmon", WormmonGuardEffect_draw);

/* The destructor of the entity class of vtable D_8006C6F4, which func_800BA734 installs. */
void func_800BAFEC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", WORMMON_GUARD_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", WORMMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", WORMMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", D_8006C6F4);
INCLUDE_RODATA("game/nonmatchings/fighters/wormmon", WORMMON_VTABLE);
