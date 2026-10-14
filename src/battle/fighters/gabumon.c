/* Gabumon (CHARACTER_GABUMON): the fighter createFighter builds for it, its moves, the special move 1's projectile
 * and the special move 2's effect. */

#include "common.h"
#include "battle/game_classes.h"

extern FaceEntry GABUMON_FACES[];
extern EntityVtable GABUMON_VTABLE;

/* The special move 2's effect, on the fighter: its tick spawns the sparks of vtable D_8006895C. */
typedef struct GabumonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *chara;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 vramY; /* Fighter_getVramY of the character */
} GabumonSpecial2Effect;

GabumonSpecial2Effect *GabumonSpecial2Effect_init(GabumonSpecial2Effect *this, Fighter *chara);

extern EntityVtable GABUMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable GABUMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry GABUMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];
void Gabumon_spawnSpecial1Projectile(Fighter *this);
Projectile *GabumonSpecial1Projectile_init(Projectile *this, Fighter *chara, Vec3 *position, Vec3 *velocity);

/* Builds Gabumon: a Fighter with its own vtable and faces. */
Gabumon *Gabumon_init(Gabumon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &GABUMON_VTABLE;
    Fighter_setFaceTable(&this->base, GABUMON_FACES);
    this->unk4A4 = 0;
    return this;
}

/* Ends a move: back to face 0. */
void Gabumon_endMove(Fighter *this) {
    Fighter_setFace(this, 0);
}

/* The throw: holds the opponent (frame 0) and carries it along with its own velocity and force until frame 16, when it
 * launches it up and backward (newLaunch). */
void Gabumon_throw(Fighter *this, s32 frame) {
    Fighter *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    Fighter_throw(this, frame);
    other = Fighter_getOpponent(this);
    if (frame < 16) {
        Fighter_accelerateTo(this, 0);
        /* The match depends on each vector read into x, y and z before it is stored. */
        x = this->velocity.vx;
        y = this->velocity.vy;
        z = this->velocity.vz;
        other->velocity.vx = x;
        other->velocity.vy = y;
        other->velocity.vz = z;
        x = this->force.vx;
        y = this->force.vy;
        z = this->force.vz;
        other->force.vx = x;
        other->force.vy = y;
        other->force.vz = z;
    }
    switch (frame) {
    case 0:
        Fighter_hold(other, 16);
        Fighter_setClip(other, FIGHTER_CLIP_STAND, 0, 0);
        break;
    case 16:
        side = -Fighter_getFacing(this);
        vel.vx = side * 10920;
        vel.vy = -18200;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* Special move 1: voice and face 1 at frame 0; at frame 14 the shot sound, face 0 and the projectile. */
void Gabumon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0x85);
        Fighter_setFace(this, 1);
        break;
    case 14:
        Fighter_playSound(this, 0x82);
        Fighter_setFace(this, 0);
        Gabumon_spawnSpecial1Projectile(this);
        break;
    }
}

/* Special move 1 in the air: as on the ground; until frame 15 the force brakes a sixteenth of the horizontal velocity
 * and all of a fall. */
void Gabumon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0x85);
        Fighter_setFace(this, 1);
        break;
    case 14:
        Fighter_playSound(this, 0x82);
        Fighter_setFace(this, 0);
        Gabumon_spawnSpecial1Projectile(this);
        break;
    }
    if (frame < 15) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

/* Fires the special move 1's projectile, 0xC0 above the fighter, 96 a frame forward. */
void Gabumon_spawnSpecial1Projectile(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vy -= 0xC0;
    velocity.vx = Fighter_getFacing(this) * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        GabumonSpecial1Projectile_init(entityOperatorNewAt(0xC0, child), this, &position, &velocity);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", Gabumon_special2);

/* Spawns the special move 2's effect. */
void Gabumon_spawnSpecial2Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        GabumonSpecial2Effect_init(entityOperatorNewAt(0x54, child), this);
    }
    Fighter_addEntity(this, child);
}

/* Builds the special move 1's projectile, a Projectile of radius 0x60 and speed 0x900. */
Projectile *GabumonSpecial1Projectile_init(Projectile *this, Fighter *chara, Vec3 *position, Vec3 *velocity) {
    Projectile_init(this, chara, position, velocity, 0x60, 0x6A81E00, 0x900, 3);
    this->parts.base.vtable = GABUMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->parts.entity.vtable = &GABUMON_SPECIAL1_PROJECTILE_VTABLE;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", GabumonSpecial1Projectile_draw);

/* Builds the special move 2's effect for chara: three 0x1000 scales (0x1C, 0x24, 0x2C) and 0x80 at 0x44. */
GabumonSpecial2Effect *GabumonSpecial2Effect_init(GabumonSpecial2Effect *this, Fighter *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &GABUMON_SPECIAL2_EFFECT_VTABLE;
    this->chara = chara;
    this->unk1C = 0x1000;
    this->unk20 = 0;
    this->unk24 = 0x1000;
    this->unk28 = 0;
    this->unk2C = 0x1000;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = 0x80;
    this->unk48 = 0;
    this->unk4C = 0;
    this->vramY = Fighter_getVramY(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", GabumonSpecial2Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", GabumonSpecial2Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", func_80087E14);

/* The entity class of virtual table D_8006895C (func_80087E14): a matrix copied in, a value easing to
 * 0x280 at 0x38 and a life at 0x3C. */
typedef struct Unk80087E14 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MATRIX matrix;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Unk80087E14;

/* The update of D_8006895C's entity: turns its matrix by 0xB0 about z, eases 0x38 an eighth of the way to
 * 0x280, and deletes itself once 0x3C, down 0x10 a frame, runs out. */
void func_80087EC4(Unk80087E14 *this) {
    matrixRotateZ(&this->matrix, 0xB0);
    this->unk38 += (0x280 - this->unk38) >> 3;
    this->unk3C -= 0x10;
    if (this->unk3C <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/gabumon", func_80087F3C);

/* Gabumon's destructor. */
void Gabumon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void GabumonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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

/* The special move 2 effect's destructor. */
void GabumonSpecial2Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the special move 2's sparks (vtable D_8006895C). */
void func_800881C4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688AC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688B4);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688BC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688C4);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688CC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688D4);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688DC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688E4);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688EC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688F4);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_800688FC);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_80068904);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_8006890C);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_8006892C);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", GABUMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", D_8006895C);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", GABUMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", GABUMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", GABUMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gabumon", GABUMON_VTABLE);
