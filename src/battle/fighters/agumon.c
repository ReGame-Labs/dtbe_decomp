/* Agumon (CHARACTER_AGUMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's projectile and its burst, and the special move 2's effect. */

#include "common.h"
#include "battle/game_classes.h"

struct AgumonSpecial2Effect *AgumonSpecial2Effect_init(struct AgumonSpecial2Effect *this, Fighter *chara);
struct AgumonSpecial1Effect *AgumonSpecial1Effect_init(struct AgumonSpecial1Effect *this, Vec3 *position, s32 arg2,
                                                       s32 maxSize, s32 vramY);
extern FaceEntry AGUMON_FACES[];
extern EntityVtable AGUMON_VTABLE;

/* The special move 1's projectile (a Projectile). */
typedef struct AgumonSpecial1Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ s32 cell; /* 1 to 4, stepped every other frame: the texture cell the draw uses */
} AgumonSpecial1Projectile;

extern EntityVtable AGUMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry AGUMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];
void Agumon_spawnSpecial1Projectile(Fighter *this);
void Agumon_startSpecial2Hit(Fighter *this, s32 hit);
void Agumon_spawnSpecial2Effect(Fighter *this);

/* Builds Agumon: a Fighter with its own vtable and faces. */
Fighter *Agumon_init(Fighter *this) {
    Fighter_init(this);
    this->entity.vtable = &AGUMON_VTABLE;
    Fighter_setFaceTable(this, AGUMON_FACES);
    return this;
}

/* Ends a move: back to face 0. */
void Agumon_endMove(Fighter *this) {
    Fighter_setFace(this, 0);
}

/* The throw: holds the opponent (frame 0) and carries it along with its own velocity and force until frame 5, when it
 * launches it up and forward (newLaunch). */
void Agumon_throw(Fighter *this, s32 frame) {
    Fighter *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    Fighter_throw(this, frame);
    other = Fighter_getOpponent(this);
    if (frame < 5) {
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
        Fighter_hold(other, 5);
        Fighter_setClip(other, FIGHTER_CLIP_STAND, 0, 0);
        break;
    case 5:
        side = Fighter_getFacing(this);
        vel.vx = side * 5460;
        vel.vy = -21840;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* Special move 1: voice and face 1 at frame 0; at frame 13 the shot sound, face 0 and the projectile. */
void Agumon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0x77);
        Fighter_setFace(this, 1);
        break;
    case 13:
        Fighter_playSound(this, 0x73);
        Fighter_setFace(this, 0);
        Agumon_spawnSpecial1Projectile(this);
        break;
    }
}

/* Special move 1 in the air: as on the ground, with the projectile at frame 7; until frame 8 the force brakes a
 * sixteenth of the horizontal velocity and all of a fall. */
void Agumon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0x77);
        Fighter_setFace(this, 1);
        break;
    case 7:
        Fighter_playSound(this, 0x73);
        Fighter_setFace(this, 0);
        Agumon_spawnSpecial1Projectile(this);
        break;
    }
    if (frame < 8) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", Agumon_spawnSpecial1Projectile);

INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_8006818C);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_80068194);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_8006819C);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_800681A4);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_800681AC);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_800681B4);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_800681BC);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", D_800681CC);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_FACES);

/* Special move 2, a three-hit combo timed by the model's frame: hits 0, 1 and 2 of AGUMON_SPECIAL2_HITS at frames 3, 16
 * and 35 (attack parts 4 while they last), the effect at frames 1, 16 and 39, invulnerable for 10 frames at 28; at
 * frames 15 and 32 it stops and gives the action clip 12 frames, cut short on landing. */
void Agumon_special2(Fighter *this, s32 frame) {
    s32 modelFrame;

    Fighter_special2(this, frame);
    func_80079AE4(this);
    modelFrame = modelGetFrame(this->model);
    switch (modelFrame) {
    case 3:
        Agumon_startSpecial2Hit(this, 0);
        break;
    case 16:
        Agumon_startSpecial2Hit(this, 1);
        break;
    case 28:
        this->invulnerableFrames = 10;
        break;
    case 15:
    case 32:
        Fighter_accelerateTo(this, 0);
        if (this->unk49C <= 0) {
            this->unk49C = 12;
        }
        break;
    case 35:
        Agumon_startSpecial2Hit(this, 2);
        break;
    }
    switch (modelFrame) {
    case 0:
        Fighter_playVoice(this, 0x78);
        break;
    case 1:
    case 16:
    case 39:
        Agumon_spawnSpecial2Effect(this);
        break;
    case 9:
    case 24:
    case 47:
        Fighter_playSound(this, 0x74);
        break;
    }
    if ((modelFrame >= 3 && modelFrame <= 10) || (modelFrame >= 16 && modelFrame <= 25)
        || (modelFrame >= 36 && modelFrame <= 47)) {
        this->attackParts = 4;
    } else {
        this->attackParts = 0;
    }
    if (this->unk49C > 0) {
        if (Fighter_isOnGround(this)) {
            this->unk49C = 0;
        } else if (this->unk49C == 1) {
            Fighter_stopActionClip(this);
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_SPECIAL2_HITS);

INCLUDE_ASM("game/nonmatchings/fighters/agumon", Agumon_startSpecial2Hit);

/* Spawns the special move 2's effect. */
void Agumon_spawnSpecial2Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x4C);

    if (child != NULL) {
        AgumonSpecial2Effect_init(entityOperatorNewAt(0x4C, child), this);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", ThrowLaunch_init);

INCLUDE_ASM("game/nonmatchings/fighters/agumon", ThrowLaunch_tick);

/* Builds the special move 1's projectile, a Projectile of radius 0x80 and speed 0xA00. */
AgumonSpecial1Projectile *AgumonSpecial1Projectile_init(AgumonSpecial1Projectile *this, Fighter *chara,
                                                        Vec3 *position, Vec3 *velocity) {
    Projectile_init(&this->base, chara, position, velocity, 0x80, 0x5A81C00, 0xA00, 3);
    this->base.parts.base.vtable = AGUMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &AGUMON_SPECIAL1_PROJECTILE_VTABLE;
    this->cell = 0;
    return this;
}

/* Steps the texture cell every other frame, then flies as a Projectile. */
void AgumonSpecial1Projectile_tick(AgumonSpecial1Projectile *this, s32 arg) {
    if (this->base.age & 1) {
        if (++this->cell >= 5) {
            this->cell = 1;
        }
    }
    Projectile_tick(&this->base, arg);
}

/* On a hit: spawns the burst where it is, then Projectile_hit. */
void AgumonSpecial1Projectile_hit(AgumonSpecial1Projectile *this, CollisionSphere *sum) {
    Entity *child = allocEntityMemory(0x38);

    if (child != NULL) {
        AgumonSpecial1Effect_init(entityOperatorNewAt(0x38, child), &this->base.pos, 0x180, 0xC0, this->base.unkBC);
    }
    entityAddSibling(&this->base.parts.entity, child);
    Projectile_hit(&this->base, sum);
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", AgumonSpecial1Projectile_draw);

/* The burst of the special move 1, where the projectile starts and where it hits: it grows until it reaches its
 * maximum, then deletes itself. */
typedef struct AgumonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 size; /* + 16 a frame; the quad is 2 size by 4 size and fades as it grows */
    /* 0x2C */ s32 maxSize;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 vramY;
} AgumonSpecial1Effect;

extern EntityVtable AGUMON_SPECIAL1_EFFECT_VTABLE;

/* Builds the burst at position, growing up to maxSize. The match depends on reading the three words before storing any,
 * as vecCopy does. */
AgumonSpecial1Effect *AgumonSpecial1Effect_init(AgumonSpecial1Effect *this, Vec3 *position, s32 arg2, s32 maxSize,
                                                s32 vramY) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &AGUMON_SPECIAL1_EFFECT_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->size = 0;
    this->maxSize = maxSize;
    this->unk30 = arg2;
    this->vramY = vramY;
    return this;
}

/* Grows by 16 a frame and deletes itself at its maximum size. */
void AgumonSpecial1Effect_tick(AgumonSpecial1Effect *this) {
    this->size += 16;
    if (this->size >= this->maxSize) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", AgumonSpecial1Effect_draw);

/* The special move 2's effect, at the fighter's node 2. */
typedef struct AgumonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *chara;
    /* 0x1C */ VECTOR position; /* the character's node 2 (Fighter_getNodePosition) */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48; /* Fighter_getVramY of the character */
} AgumonSpecial2Effect;

extern EntityVtable AGUMON_SPECIAL2_EFFECT_VTABLE;

/* Builds the special move 2's effect for chara, at its node 2. */
AgumonSpecial2Effect *AgumonSpecial2Effect_init(AgumonSpecial2Effect *this, Fighter *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &AGUMON_SPECIAL2_EFFECT_VTABLE;
    this->chara = chara;
    Fighter_getNodePosition(chara, &this->position, 2, &ZERO_VECTOR);
    this->unk2C = 0x80;
    this->unk30 = 0x180;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = 0;
    this->unk48 = Fighter_getVramY(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", AgumonSpecial2Effect_tick);

void func_8008391C(AgumonSpecial2Effect *this, s32 arg);
void func_80083B90(AgumonSpecial2Effect *this, s32 arg);

/* Draws by its state (0x40): func_8008391C's part, and in state 1 func_80083B90's too. */
void AgumonSpecial2Effect_draw(AgumonSpecial2Effect *this, s32 arg) {
    switch (this->unk40) {
    case 0:
        func_8008391C(this, arg);
        break;
    case 1:
        func_8008391C(this, arg);
        func_80083B90(this, arg);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/agumon", func_8008391C);

INCLUDE_ASM("game/nonmatchings/fighters/agumon", func_80083B90);

/* Agumon's destructor. */
void Agumon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The throw launch's destructor. */
void ThrowLaunch_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void AgumonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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

/* The burst's destructor. */
void AgumonSpecial1Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 2 effect's destructor. */
void AgumonSpecial2Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", THROW_LAUNCH_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/agumon", AGUMON_VTABLE);
