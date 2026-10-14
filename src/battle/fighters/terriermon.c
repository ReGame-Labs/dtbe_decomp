/* Terriermon (CHARACTER_TERRIERMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the special move 1's projectile and effect, and the special move 2's hitboxes and effect. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"

void Terriermon_spawnSpecial2Effect(Fighter *this);
struct TerriermonSpecial2Hitbox *TerriermonSpecial2Hitbox_init(struct TerriermonSpecial2Hitbox *this,
                                                               Terriermon *owner, VECTOR *pos, Fight *list, u32 damage);
TerriermonSpecial1Effect *TerriermonSpecial1Effect_init(TerriermonSpecial1Effect *this, Fighter *arg);
Projectile *TerriermonSpecial1Projectile_init(Projectile *this, Fighter *fighter, Vec3 *position, Vec3 *velocity,
                                              Fight *fight);
Entity *TerriermonSpecial2Effect_init(Entity *this, Fighter *arg);
extern FaceEntry TERRIERMON_FACES[];
extern EntityVtable TERRIERMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable TERRIERMON_SPECIAL1_EFFECT_VTABLE;
extern VtableEntry TERRIERMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable TERRIERMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable TERRIERMON_VTABLE;
extern EntityVtable TERRIERMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry TERRIERMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];
void Terriermon_spawnSpecial1Projectile(Fighter *this);
void Terriermon_spawnSpecial1Effect(Fighter *this);
/* offset's initializer in Terriermon_throw: GCC keeps it as an anonymous constant (.rodata stays in asm) */
extern VECTOR D_8006C06C;

/* Builds Terriermon: a Fighter with its own vtable and faces, 0x4A4 and 0x4A8 0. */
Terriermon *Terriermon_init(Terriermon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &TERRIERMON_VTABLE;
    Fighter_setFaceTable(&this->base, TERRIERMON_FACES);
    this->unk4A4 = this->unk4A8 = 0;
    return this;
}

/* Terriermon's destructor. */
void Terriermon_destroy(Entity *this, s32 flags) {
    this->vtable = &TERRIERMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Crouching attack, from Fighter_getParams's attack 5: frame 0 its clip, the attack's hit and knockback (forward), and
 * dust; until frame 17 an acceleration of (16 - frame) * 0x600 forward; the attack's parts from its first to its last
 * active frame, else none. */
void Terriermon_crouchAttack(Fighter *this, s32 frame) {
    FighterParams *table;
    s32 vx;

    func_80079AE4(this);
    table = Fighter_getParams(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_CROUCH_ATTACK, 0, 0, -1);
        this->hitDamage = table->attacks[5].hitDamage;
        /* The match depends on the product computed before the stores of 0x458 and 0x45C and stored last */
        vx = table->attacks[5].knockbackX * Fighter_getFacing(this);
        this->hitKnockback.vy = -table->attacks[5].knockbackY;
        this->hitKnockback.vz = 0;
        this->hitKnockback.vx = vx;
        Fighter_spawnDust(this, -1, 0x60, 3);
    }
    if (frame < 17) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * (16 - frame) * 0x600);
    }
    if (frame >= table->attacks[5].firstActiveFrame && table->attacks[5].lastActiveFrame >= frame) {
        Fighter_setAttackParts(this, table->attacks[5].parts);
    } else {
        this->attackParts = 0;
    }
}

/* Throw: Fighter's; until frame 10 the fighter stops (Fighter_accelerateTo(0)) and the opponent takes its velocity and
 * force; frame 0 holds the opponent in FIGHTER_CLIP_FLINCH; frame 10, when the opponent is within 0x180 by 0x80, throws
 * it (Fighter_takeHit) with a spark at node 12, else lets it go (Fighter_stopActionClip). */
void Terriermon_throw(Fighter *this, s32 frame) {
    Vec3 position;
    Vec3 opponentPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Fighter *opponent;
    Entity *child;
    s32 dx, dy;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    if (frame < 10) {
        s32 x, y, z;

        Fighter_accelerateTo(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->velocity.vx;
        y = this->velocity.vy;
        z = this->velocity.vz;
        opponent->velocity.vx = x;
        opponent->velocity.vy = y;
        opponent->velocity.vz = z;
        x = this->force.vx;
        y = this->force.vy;
        z = this->force.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
    }
    switch (frame) {
    case 0:
        Fighter_hold(opponent, 10);
        Fighter_setClip(opponent, FIGHTER_CLIP_FLINCH, 0, 0);
        break;
    case 10:
        Fighter_getPosition(this, &position);
        Fighter_getOpponentPosition(this, &opponentPosition);
        dx = ABS(position.vx - opponentPosition.vx);
        dy = ABS(position.vy - opponentPosition.vy);
        if (dx < 0x180 && dy < 0x80) {
            velocity.vx = (s16)Fighter_getFacing(this) * 14560;
            velocity.vy = -0x2AA8;
            velocity.vz = 0;
            Fighter_takeHit(opponent, 0xC38C1B58, &velocity, 0);
            offset = D_8006C06C;
            Fighter_getNodePosition(this, (VECTOR *)&node, 12, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                HitSpark_init(entityOperatorNewAt(0x48, child), &node, &ZERO_VECTOR, 0xC0, 0x808080);
            }
            Fighter_addEntity(this, child);
        } else {
            Fighter_hold(opponent, 0);
            Fighter_stopActionClip(this);
        }
        break;
    }
}

/* Special move 1: Fighter's; the effect and the voice at frame 1, the projectile and sound 0xF7 at frame 13; until
 * frame 14 the force brakes a sixteenth of the horizontal velocity. */
void Terriermon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Terriermon_spawnSpecial1Effect(this);
        Fighter_playVoice(this, 0xFB);
    }
    if (frame == 13) {
        Terriermon_spawnSpecial1Projectile(this);
        Fighter_playSound(this, 0xF7);
    }
    if (frame < 14) {
        this->force.vx += -this->velocity.vx >> 4;
    }
}

/* Special move 1 in the air: Fighter's; the effect and the voice at frame 1, the projectile, sound 0xF7 and a push back
 * by 0x3000 at frame 13; until frame 14 the force brakes a sixteenth of the horizontal velocity and all of a fall. */
void Terriermon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Terriermon_spawnSpecial1Effect(this);
        Fighter_playVoice(this, 0xFB);
    }
    if (frame == 13) {
        Terriermon_spawnSpecial1Projectile(this);
        Fighter_playSound(this, 0xF7);
        this->force.vx -= Fighter_getFacing(this) * 0x3000;
    }
    if (frame < 14) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

/* Spawns the special move 1's projectile 0x80 above the fighter, flying 128 a frame forward. */
void Terriermon_spawnSpecial1Projectile(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Fight *fight;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vy -= 0x80;
    velocity.vx = Fighter_getFacing(this) * 128;
    velocity.vy = 0;
    velocity.vz = 0;
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        TerriermonSpecial1Projectile_init(entityOperatorNewAt(0xC0, child), this, &position, &velocity, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 1's effect on the fighter. */
void Terriermon_spawnSpecial1Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        TerriermonSpecial1Effect_init(entityOperatorNewAt(0x44, child), this);
    }
    Fighter_addEntity(this, child);
}

/* Special move 2: Fighter's; frame 0 the voice and sound 0xF8, 0x4A4 and 0x4A8 cleared; 0x4A4 counts down to 0; the
 * effect on odd frames before 16; on frames 4 to 29 while 0x4A4 is 0, a hitbox at point 5 (odd frames) or 12, its
 * damage 0xC7AC1000 instead of 0x7AC0800 once 0x4A8 reaches 2; before frame 31 an acceleration forward and an upward
 * velocity of frame * 256, else a stop; dust before frame 3. */
void Terriermon_special2(Terriermon *this, s32 frame) {
    Fight *fight;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 damage;
    s32 odd;

    Fighter_special2(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        Fighter_playVoice(&this->base, 0xFC);
        Fighter_playSound(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = frame & 1;
    if (odd && frame < 16) {
        Terriermon_spawnSpecial2Effect(&this->base);
    }
    if (frame < 30 && this->unk4A4 == 0 && frame >= 4) {
        fight = Fighter_getScene(&this->base);
        memset(&offset, 0, sizeof(offset));
        damage = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            damage = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before damage. */
        if (odd) {
            Entity *child;

            Fighter_getNodePosition(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                TerriermonSpecial2Hitbox_init(entityOperatorNewAt(0x94, child), this, &pos, fight, damage);
            }
            Fighter_addEntity(&this->base, child);
        } else {
            Entity *child;

            Fighter_getNodePosition(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                TerriermonSpecial2Hitbox_init(entityOperatorNewAt(0x94, child), this, &pos, fight, damage);
            }
            Fighter_addEntity(&this->base, child);
        }
    }
    if (frame < 31) {
        Fighter_accelerateTo(&this->base, Fighter_getFacing(&this->base) << 14);
        this->base.velocity.vy = -(frame << 8);
    } else {
        Fighter_accelerateTo(&this->base, 0);
    }
    if (frame < 3) {
        Fighter_spawnDust(&this->base, -1, 0x60, 5);
    }
}

/* Special move 2 in the air: as Terriermon_special2 from frame 0 and without the dust. */
void Terriermon_airSpecial2(Terriermon *this, s32 frame) {
    Fight *fight;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 damage;
    s32 odd;

    Fighter_special2(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        Fighter_playVoice(&this->base, 0xFC);
        Fighter_playSound(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = frame & 1;
    if (odd && frame < 16) {
        Terriermon_spawnSpecial2Effect(&this->base);
    }
    if (frame < 30 && this->unk4A4 == 0) {
        fight = Fighter_getScene(&this->base);
        memset(&offset, 0, sizeof(offset));
        damage = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            damage = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before damage. */
        if (odd) {
            Entity *child;

            Fighter_getNodePosition(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                TerriermonSpecial2Hitbox_init(entityOperatorNewAt(0x94, child), this, &pos, fight, damage);
            }
            Fighter_addEntity(&this->base, child);
        } else {
            Entity *child;

            Fighter_getNodePosition(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                TerriermonSpecial2Hitbox_init(entityOperatorNewAt(0x94, child), this, &pos, fight, damage);
            }
            Fighter_addEntity(&this->base, child);
        }
    }
    if (frame < 31) {
        Fighter_accelerateTo(&this->base, Fighter_getFacing(&this->base) << 14);
        this->base.velocity.vy = -(frame << 8);
    } else {
        Fighter_accelerateTo(&this->base, 0);
    }
}

/* Spawns the special move 2's effect on the fighter. */
void Terriermon_spawnSpecial2Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        TerriermonSpecial2Effect_init(entityOperatorNewAt(0x120, child), this);
    }
    Fighter_addEntity(this, child);
}

/* A hitbox of the special move 2: it lives one tick. */
typedef struct TerriermonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Terriermon *owner;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ s32 unk90;
} TerriermonSpecial2Hitbox;

/* Builds a special move 2 hitbox for owner at pos with damage, its knockback 0x2AA8 toward the opponent
 * (Fighter_getOpponentPosition), and adds its sphere to the owner's side of list (Fighter_getSide). */
TerriermonSpecial2Hitbox *TerriermonSpecial2Hitbox_init(TerriermonSpecial2Hitbox *this, Terriermon *owner,
                                                        VECTOR *pos, Fight *list, u32 damage) {
    Vec3 ownerPosition;
    Vec3 opponentPosition;
    VECTOR v;
    s32 push;
    s32 x, y, z;
    ListNode *node;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, &owner->base, damage, (Vec3 *)&ZERO_VECTOR, 1);
    this->parts.base.vtable = TERRIERMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &TERRIERMON_SPECIAL2_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, (Vec3 *)pos, 0x80, &this->parts.base);
    Fighter_getPosition(&owner->base, &ownerPosition);
    Fighter_getOpponentPosition(&owner->base, &opponentPosition);
    /* The match depends on one variable for the difference and the push */
    push = opponentPosition.vx - ownerPosition.vx;
    if (push > 0) {
        push = 0x2AA8;
    } else {
        push = -0x2AA8;
    }
    v.vx = push;
    v.vy = 0;
    v.vz = 0;
    Hit_setKnockback(&this->parts.base, &v);
    this->owner = owner;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    /* The match depends on the node's address taken here, kept in a saved register for the list calls */
    node = &this->parts.part.link;
    this->parts.part.entity = &this->parts.entity;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (Fighter_getSide(&owner->base) == 0) {
        Fight_addSide0Shape(list, node);
    } else {
        Fight_addSide1Shape(list, node);
    }
    return this;
}

/* The special move 2 hitbox's destructor. */
void TerriermonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = TERRIERMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &TERRIERMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial2Hitbox_hit);

/* Deletes the hitbox at its first tick. */
void TerriermonSpecial2Hitbox_tick(TerriermonSpecial2Hitbox *this, s32 arg) {
    entityDelete(&this->parts.entity);
}

/* Builds the special move 1's projectile: Projectile_init with 0x80, 0x7A81800, 0x2000 and 3, then its vtables. */
Projectile *TerriermonSpecial1Projectile_init(Projectile *this, Fighter *fighter, Vec3 *position, Vec3 *velocity,
                                              Fight *fight) {
    Projectile_init(this, fighter, position, velocity, 0x80, 0x7A81800, 0x2000, 3);
    this->parts.base.vtable = TERRIERMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->parts.entity.vtable = &TERRIERMON_SPECIAL1_PROJECTILE_VTABLE;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial1Projectile_draw);

/* Builds the special move 1's effect for fighter at its position (Fighter_getPosition). */
TerriermonSpecial1Effect *TerriermonSpecial1Effect_init(TerriermonSpecial1Effect *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &TERRIERMON_SPECIAL1_EFFECT_VTABLE;
    this->fighter = fighter;
    Fighter_getPosition(fighter, &this->position);
    this->unk2C = 0x40;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->ticks = 0;
    this->unk3C = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The special move 1 effect's destructor. */
void TerriermonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &TERRIERMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Follows the character's position, 0x60 above it, for 13 ticks, then deletes itself. */
void TerriermonSpecial1Effect_tick(TerriermonSpecial1Effect *this, s32 arg) {
    if (this->ticks >= 13) {
        entityDelete(&this->entity);
    } else {
        this->ticks++;
        Fighter_getPosition(this->fighter, &this->position);
        this->position.vy -= 0x60;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial1Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial2Effect_init);

/* The special move 2 effect's destructor. */
void TerriermonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &TERRIERMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial2Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/terriermon", TerriermonSpecial2Effect_draw);

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void TerriermonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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

INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C00C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C014);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C01C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C024);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C02C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C034);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C03C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C04C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", D_8006C06C);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/terriermon", TERRIERMON_VTABLE);
