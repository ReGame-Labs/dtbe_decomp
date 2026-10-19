/* Seraphimon (CHARACTER_SERAPHIMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the special move 1's hitbox, the special move 2's hitboxes, effect and sparks, and the special move 3's effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"

SeraphimonSpecial2Effect *SeraphimonSpecial2Effect_init(SeraphimonSpecial2Effect *this, Fighter *fighter);
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Fighter *fighter, Fight *fight);
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Fighter *fighter, Fight *fight);
Unk800B050C *func_800B050C(Unk800B050C *this, Fighter *fighter, Fight *fight);
extern EntityVtable D_8006B5FC;
extern EntityVtable D_8006B624;
extern EntityVtable D_8006B64C;
extern VtableEntry SERAPHIMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable SERAPHIMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable D_8006B6BC;
extern EntityVtable SERAPHIMON_SPECIAL2_EFFECT_VTABLE;
extern VtableEntry SERAPHIMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable SERAPHIMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable SERAPHIMON_VTABLE;

/* Seraphimon's class (CHARACTER_SERAPHIMON). */
typedef struct Seraphimon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ s32 unk4A8;
} Seraphimon;
void Seraphimon_spawnSpecial1Hitbox(Fighter *this);

/* Builds Seraphimon: a Fighter with its own vtable, 0x4A8 0. */
Seraphimon *Seraphimon_init(Seraphimon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &SERAPHIMON_VTABLE;
    this->unk4A8 = 0;
    return this;
}

/* Seraphimon's destructor. */
void Seraphimon_destroy(Entity *this, s32 flags) {
    this->vtable = &SERAPHIMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: releases the opponent (Fighter_detach). */
void Seraphimon_endMove(Fighter *this) {
    Fighter_detach(Fighter_getOpponent(this));
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", Seraphimon_throw);

void Seraphimon_spawnSpecial2Effect(Fighter *this);
void Seraphimon_spawnSpecial2Hitbox(Fighter *this, s32 angle, s32 index);

/* Special move 2: as in the air, but the hitboxes get -0x200, with no push back, and until frame 11 the force brakes
 * only a sixteenth of the horizontal velocity. */
void Seraphimon_special2(Fighter *this, s32 frame) {
    s32 i;

    Fighter_special2(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Seraphimon_spawnSpecial2Effect(this);
        Fighter_playVoice(this, 0xE8);
    }
    if (frame == 10) {
        for (i = 0; i < 7; i++) {
            Seraphimon_spawnSpecial2Hitbox(this, -0x200, i);
        }
        Fighter_playSound(this, 0xE2);
    }
    if (frame < 11) {
        /* The match depends on adding the negated velocity (negu, sra, addu), not subtracting it. */
        this->force.vx += -this->velocity.vx >> 4;
    }
}

/* Special move 2 in the air: Fighter's; the effect and the voice at frame 1, seven hitboxes (0x200, 0 to 6), a push
 * back by 0x3000 and sound 0xE2 at frame 10; until frame 11 the force brakes a sixteenth of the horizontal velocity and
 * twice a fall. */
void Seraphimon_airSpecial2(Fighter *this, s32 frame) {
    s32 i;

    Fighter_airSpecial2(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Seraphimon_spawnSpecial2Effect(this);
        Fighter_playVoice(this, 0xE8);
    }
    if (frame == 10) {
        for (i = 0; i < 7; i++) {
            Seraphimon_spawnSpecial2Hitbox(this, 0x200, i);
        }
        this->force.vx -= Fighter_getFacing(this) * 0x3000;
        Fighter_playSound(this, 0xE2);
    }
    if (frame < 11) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy * 2;
        }
    }
}

struct SeraphimonSpecial2Hitbox *SeraphimonSpecial2Hitbox_init(struct SeraphimonSpecial2Hitbox *this, Fighter *fighter,
                                                               Vec3 *position, VECTOR *velocity, Fight *list, s32 arg5);

/* Spawns one of the special move 2's hitboxes at the fighter's node 18, moving at angle (mirrored when the fighter
 * faces left) at 1/32 of the unit. */
void Seraphimon_spawnSpecial2Hitbox(Fighter *this, s32 angle, s32 index) {
    VECTOR position;
    VECTOR velocity;
    VECTOR offset;
    Fight *fight;
    Entity *child;

    memset(&offset, 0, sizeof(offset));
    Fighter_getNodePosition(this, &position, 18, &offset);
    if (Fighter_getFacing(this) > 0) {
        velocity.vx = COS_SIN_TABLE[angle & 0xFFF].cos >> 5;
        velocity.vy = COS_SIN_TABLE[angle & 0xFFF].sin >> 5;
        velocity.vz = 0;
    } else {
        /* The match depends on this local: written in the index, the subtraction is scheduled after the table's
         * address. */
        s32 mirrored = 0x800 - angle;

        velocity.vx = COS_SIN_TABLE[mirrored & 0xFFF].cos >> 5;
        velocity.vy = COS_SIN_TABLE[mirrored & 0xFFF].sin >> 5;
        velocity.vz = 0;
    }
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0xD0);
    if (child != NULL) {
        /* the hitbox reads only the position's first three words */
        SeraphimonSpecial2Hitbox_init(entityOperatorNewAt(0xD0, child), this, (Vec3 *)&position, &velocity, fight,
                                      index);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 2's effect for this fighter. */
void Seraphimon_spawnSpecial2Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        SeraphimonSpecial2Effect_init(entityOperatorNewAt(0x44, child), this);
    }
    Fighter_addEntity(this, child);
}

/* Special move 1: Fighter's; the hitbox and the voice at frame 1, sound 0xE3 at frame 20; until frame 21 the force
 * brakes a sixteenth of the horizontal velocity. */
void Seraphimon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Seraphimon_spawnSpecial1Hitbox(this);
        Fighter_playVoice(this, 0xE7);
    }
    if (frame == 20) {
        Fighter_playSound(this, 0xE3);
    }
    if (frame < 21) {
        this->force.vx += -this->velocity.vx >> 4;
    }
}

/* Special move 1 in the air: Fighter's; the hitbox and the voice at frame 1, sound 0xE3 at frame 20; until frame 21 the
 * force brakes a sixteenth of the horizontal velocity and all of a fall. */
void Seraphimon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Seraphimon_spawnSpecial1Hitbox(this);
        Fighter_playVoice(this, 0xE7);
    }
    if (frame == 20) {
        Fighter_playSound(this, 0xE3);
    }
    if (frame < 21) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

/* The special move 1's hitbox (Seraphimon_spawnSpecial1Hitbox); its sphere starts disabled. */
typedef struct SeraphimonSpecial1Hitbox {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Hit unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ CollisionSphere unk38;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ Fight *list;
    /* 0x088 */ Vec3 position;
    /* 0x094 */ u8 unk94[0x98 - 0x94];
    /* 0x098 */ Vec3 velocity;
    /* 0x0A4 */ u8 unkA4[0x138 - 0xA4];
    /* 0x138 */ s32 unk138;
    /* 0x13C */ s32 unk13C;
    /* 0x140 */ s32 facing; /* the character's (Fighter_getFacing) */
    /* 0x144 */ s32 speed; /* the velocity's length */
    /* 0x148 */ s32 unk148;
    /* 0x14C */ s32 unk14C;
    /* 0x150 */ s16 vramY; /* the character's 0x440 (Fighter_getVramY) */
    /* 0x152 */ u8 unk152[0x154 - 0x152];
    /* 0x154 */ s32 unk154;
} SeraphimonSpecial1Hitbox;

SeraphimonSpecial1Hitbox *SeraphimonSpecial1Hitbox_init(SeraphimonSpecial1Hitbox *this, Fighter *fighter,
                                                        Vec3 *position, VECTOR *velocity, Fight *list);
extern VECTOR SERAPHIMON_SPECIAL1_HITBOX_OFFSET;

INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL1_HITBOX_OFFSET);

/* Spawns the special move 1's hitbox at the character's point 9 (moved by SERAPHIMON_SPECIAL1_HITBOX_OFFSET), flying
 * forward at 128 per frame. */
void Seraphimon_spawnSpecial1Hitbox(Fighter *this) {
    Vec3 position;
    VECTOR velocity;
    VECTOR offset = SERAPHIMON_SPECIAL1_HITBOX_OFFSET;
    Fight *list;
    Entity *child;

    Fighter_getNodePosition(this, (VECTOR *)&position, 9, &offset);
    velocity.vx = Fighter_getFacing(this) * 128;
    velocity.vy = 0;
    velocity.vz = 0;
    list = Fighter_getScene(this);
    child = allocEntityMemory(sizeof(SeraphimonSpecial1Hitbox));
    if (child != NULL) {
        SeraphimonSpecial1Hitbox_init(entityOperatorNewAt(sizeof(SeraphimonSpecial1Hitbox), child), this, &position,
                                      &velocity, list);
    }
    Fighter_addEntity(this, child);
    func_80079B0C(this, 20);
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", Seraphimon_special3);

/* Spawns a special move 3 effect (func_800AFD78) for this fighter and its fight. */
void func_800AE438(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0x60);

    if (child != NULL) {
        func_800AFD78(entityOperatorNewAt(0x60, child), this, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns a special move 3 effect (func_800B01AC) for this fighter and its fight. */
void func_800AE4A8(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B01AC(entityOperatorNewAt(0x40, child), this, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns a special move 3 effect (func_800B050C) for this fighter and its fight. */
void func_800AE518(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B050C(entityOperatorNewAt(0x40, child), this, fight);
    }
    Fighter_addEntity(this, child);
}

/* A hitbox of the special move 2 (seven at a time): like Unk800AD050, but it keeps its start position and the
 * velocity's length. */
typedef struct SeraphimonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *owner;
    /* 0x84 */ Fight *list; /* the part goes into one of its two lists (Fighter_getSide picks which) */
    /* 0x88 */ Vec3 position;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 start;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ Vec3 velocity;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 speed; /* vecGetLength of velocity */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s16 vramY; /* Fighter_getVramY of the owner */
    /* 0xCE */ s16 unkCE;
} SeraphimonSpecial2Hitbox;

/* Builds a special move 2 hitbox for fighter at position, moving by velocity (its knockback: velocity's x and z times
 * 8, y less 0xC00), and adds its sphere to fighter's side of list (Fighter_getSide). */
SeraphimonSpecial2Hitbox *SeraphimonSpecial2Hitbox_init(SeraphimonSpecial2Hitbox *this, Fighter *fighter,
                                                        Vec3 *position, VECTOR *velocity, Fight *list, s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, 0x7AC0800, (Vec3 *)velocity, 1);
    this->parts.base.vtable = SERAPHIMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &SERAPHIMON_SPECIAL2_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 3;
    v.vy -= 0xC00;
    v.vz <<= 3;
    Hit_setKnockback(&this->parts.base, &v);
    this->owner = fighter;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    sx = position->vx;
    sy = position->vy;
    sz = position->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->unkB8 = 0;
    this->unkBC = 0;
    this->speed = vecGetLength(velocity);
    this->unkC4 = 0;
    this->unkC8 = 0;
    this->unkCE = arg5;
    this->vramY = Fighter_getVramY(fighter);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(this->owner) == 0) {
        Fight_addSide0Shape(list, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(list, &this->parts.part.link);
    }
    return this;
}

/* The special move 2 hitbox's destructor. */
void SeraphimonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = SERAPHIMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &SERAPHIMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial2Hitbox_hit);

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial2Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial2Hitbox_draw);

/* Builds the special move 2's effect for fighter at its position (Fighter_getPosition). */
SeraphimonSpecial2Effect *SeraphimonSpecial2Effect_init(SeraphimonSpecial2Effect *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &SERAPHIMON_SPECIAL2_EFFECT_VTABLE;
    this->fighter = fighter;
    Fighter_getPosition(fighter, &this->position);
    this->unk2C = -0x180;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->ticks = 0;
    this->unk3C = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The special move 2 effect's destructor. */
void SeraphimonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &SERAPHIMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Follows the character's position for 16 ticks, then deletes itself. */
void SeraphimonSpecial2Effect_tick(SeraphimonSpecial2Effect *this, s32 arg) {
    if (this->ticks >= 16) {
        entityDelete(&this->entity);
    } else {
        this->ticks++;
        Fighter_getPosition(this->fighter, &this->position);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial2Effect_draw);

/* The sparks of a special move 2 hitbox (virtual table D_8006B6BC, spawned by SeraphimonSpecial2Hitbox_tick). */
typedef struct Unk800AF0A0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 unk2C;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
} Unk800AF0A0;

/* Constructs the entity of virtual table D_8006B6BC with arg1 and copies of the two vectors. The match
 * depends on reading each vector's three words before storing any, as vecCopy does. */
Unk800AF0A0 *func_800AF0A0(Unk800AF0A0 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B6BC;
    this->unk18 = arg1;
    x = vec1->vx;
    y = vec1->vy;
    z = vec1->vz;
    this->unk1C.vx = x;
    this->unk1C.vy = y;
    this->unk1C.vz = z;
    x2 = vec2->vx;
    y2 = vec2->vy;
    z2 = vec2->vz;
    this->unk2C.vx = x2;
    this->unk2C.vy = y2;
    this->unk2C.vz = z2;
    this->unk3C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B6BC, which func_800AF0A0 installs. */
void func_800AF134(Entity *this, s32 flags) {
    this->vtable = &D_8006B6BC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of func_800AF0A0's entity: for 8 frames the vector at 0x1C moves by the one at 0x2C, then the
 * entity deletes itself. */
void func_800AF188(Unk800AF0A0 *this) {
    s32 x, y, z;

    if (this->unk3C < 8) {
        this->unk3C++;
        /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
        x = this->unk1C.vx;
        y = this->unk1C.vy;
        z = this->unk1C.vz;
        x += this->unk2C.vx;
        y += this->unk2C.vy;
        z += this->unk2C.vz;
        this->unk1C.vx = x;
        this->unk1C.vy = y;
        this->unk1C.vz = z;
    } else {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", func_800AF214);

/* Builds the special move 1's hitbox for fighter at position, flying at velocity, and adds its sphere, disabled, to
 * fighter's side of list (Fighter_getSide). */
SeraphimonSpecial1Hitbox *SeraphimonSpecial1Hitbox_init(SeraphimonSpecial1Hitbox *this, Fighter *fighter,
                                                        Vec3 *position, VECTOR *velocity, Fight *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->unk18, fighter, 0x7A83000, (Vec3 *)velocity, 3);
    this->unk18.vtable = SERAPHIMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &SERAPHIMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&this->unk38, position, 0x100, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    Hit_setKnockback(&this->unk18, &v);
    this->fighter = fighter;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->facing = Fighter_getFacing(fighter);
    this->unk138 = 0;
    this->unk13C = 0;
    this->speed = vecGetLength(velocity);
    this->unk148 = 0;
    this->unk14C = 0;
    this->vramY = Fighter_getVramY(fighter);
    this->unk154 = 0;
    this->unk38.entity = &this->entity;
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(list, &this->unk38.link);
    } else {
        Fight_addSide1Shape(list, &this->unk38.link);
    }
    this->unk38.flags.bits.disabled = 1;
    return this;
}

/* The special move 1 hitbox's destructor. */
void SeraphimonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = SERAPHIMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &SERAPHIMON_SPECIAL1_HITBOX_VTABLE;
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

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes) at owner's position: NULL when the allocation
 * fails. */
static inline Entity *func_800AF5A4_newFlash(SeraphimonSpecial1Hitbox *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of ParticleBurst_init's burst (0x3C bytes, kind 4, eight particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *func_800AF5A4_newBurst(SeraphimonSpecial1Hitbox *owner, s32 radius) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 4, &owner->position, radius, 0xA0, 8, 0x808080);
    }
    return child;
}

/* On a hit: Hit_apply with sum; when sum belongs to an entity of kind 0x10000 (entityGetKind), sets 0x154, disables
 * the sphere and adds a flash at the position if the opponent guards (Fighter_isGuarding), else a burst of 8 of
 * radius velocity.vx / 4. */
void SeraphimonSpecial1Hitbox_hit(SeraphimonSpecial1Hitbox *this, CollisionSphere *sum) {
    Hit_apply(&this->unk18, sum);
    if (sum->entity != NULL && entityGetKind(sum->entity) == ENTITY_KIND_FIGHTER) {
        this->unk154 = 1;
        this->unk38.flags.bits.disabled = 1;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, Fighter_isGuarding(Fighter_getOpponent(this->fighter))
                                            ? func_800AF5A4_newFlash(this)
                                            : func_800AF5A4_newBurst(this, this->velocity.vx >> 2));
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial1Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", SeraphimonSpecial1Hitbox_draw);

/* Builds a special move 3 effect (virtual table D_8006B64C) for fighter and its fight. */
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B64C;
    this->fighter = fighter;
    this->unk1C = fight;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_8006B64C, which func_800AFD78 installs. */
void func_800AFDE8(Entity *this, s32 flags) {
    this->vtable = &D_8006B64C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of func_800AFD78's effect: for 24 ticks it rerolls its nine offsets (-0x400..0x3FF, times 1 to 9, an
 * eighth of it) and its eight values (0, 1 or 2, a 3 counting as 2) from the fight's random numbers, then deletes
 * itself. */
void func_800AFE3C(Unk800AFD78 *this) {
    s32 i;
    s32 value;

    if (this->unk34 < 24) {
        for (i = 0; i < 9; i++) {
            this->unk4C[i] = ((s32)((mersenneTwisterGenerate(&this->unk1C->random) & 0x7FF) - 0x400) * (i + 1)) >> 3;
        }
        for (i = 0; i < 8; i++) {
            value = mersenneTwisterGenerate(&this->unk1C->random) & 3;
            if (value == 3) {
                value = 2;
            }
            this->unk3C[i] = value;
        }
        this->unk34++;
    } else {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", func_800AFF40);

/* Builds a special move 3 effect (virtual table D_8006B624) for fighter and its fight. */
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B624;
    this->fighter = fighter;
    this->unk1C = fight;
    this->unk30 = 0;
    this->ticks = 0;
    this->unk38 = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_8006B624, which func_800B01AC installs. */
void func_800B021C(Entity *this, s32 flags) {
    this->vtable = &D_8006B624;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves 0x24 down by 0x120 for 24 ticks, then deletes itself. */
void func_800B0270(Unk800B01AC *this, s32 arg) {
    if (this->ticks >= 24) {
        entityDelete(&this->entity);
    } else {
        this->ticks++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", func_800B02E0);

/* Builds a special move 3 effect (virtual table D_8006B5FC) for fighter and its fight; the same fields as
 * func_800B01AC's. */
Unk800B050C *func_800B050C(Unk800B050C *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B5FC;
    this->fighter = fighter;
    this->unk1C = fight;
    this->unk30 = 0;
    this->ticks = 0;
    this->unk38 = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_8006B5FC, which func_800B050C installs. */
void func_800B057C(Entity *this, s32 flags) {
    this->vtable = &D_8006B5FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves 0x24 down by 0x120 for 16 ticks, then deletes itself. */
void func_800B05D0(Unk800B01AC *this, s32 arg) {
    if (this->ticks >= 16) {
        entityDelete(&this->entity);
    } else {
        this->ticks++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/seraphimon", func_800B0640);

INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", D_8006B5FC);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", D_8006B624);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", D_8006B64C);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", D_8006B6BC);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/seraphimon", SERAPHIMON_VTABLE);
