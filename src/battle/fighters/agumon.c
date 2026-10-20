/* Agumon (CHARACTER_AGUMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's projectile and its burst, and the special move 2's effect. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/prim/build_line_g2.h"

struct AgumonSpecial2Effect *AgumonSpecial2Effect_init(struct AgumonSpecial2Effect *this, Fighter *fighter);
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
AgumonSpecial1Projectile *AgumonSpecial1Projectile_init(AgumonSpecial1Projectile *this, Fighter *fighter,
                                                        Vec3 *position, Vec3 *velocity);
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

/* Spawns the special move 1's projectile 64 in front of the fighter and 0xA0 up, 96 a frame forward, calls
 * func_80079B0C(15), and puts its burst 64 further ahead. */
void Agumon_spawnSpecial1Projectile(Fighter *this) {
    s32 facing = Fighter_getFacing(this);
    Vec3 position;
    Vec3 velocity;
    /* The match depends on a variable of its own for each spawn */
    Entity *projectile;
    Entity *burst;

    Fighter_getPosition(this, &position);
    velocity.vx = facing * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 64;
    position.vy -= 0xA0;
    projectile = allocEntityMemory(sizeof(AgumonSpecial1Projectile));
    if (projectile != NULL) {
        AgumonSpecial1Projectile_init(entityOperatorNewAt(sizeof(AgumonSpecial1Projectile), projectile), this,
                                      &position, &velocity);
    }
    Fighter_addEntity(this, projectile);
    func_80079B0C(this, 15);
    position.vx += facing * 64;
    burst = allocEntityMemory(0x38);
    if (burst != NULL) {
        AgumonSpecial1Effect_init(entityOperatorNewAt(0x38, burst), &position, 0x40, 0x80, Fighter_getVramY(this));
    }
    Fighter_addEntity(this, burst);
}

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
typedef struct AgumonHitParams {
    /* 0x00 */ s32 forceX;
    /* 0x04 */ s32 forceY;
    /* 0x08 */ s32 damage;
    /* 0x0C */ s32 kind;
    /* 0x10 */ s32 knockbackX;
    /* 0x14 */ s32 knockbackY;
} AgumonHitParams;

extern AgumonHitParams AGUMON_SPECIAL2_HITS[3];

/* Starts hit of the special move 2: adds its force (forward and up), sets its damage, kind and knockback, and spawns
 * dust. */
void Agumon_startSpecial2Hit(Fighter *this, s32 hitIndex) {
    AgumonHitParams *hits = AGUMON_SPECIAL2_HITS;
    AgumonHitParams *hit = &hits[hitIndex];
    s32 facing;
    s32 vx;

    Fighter_accelerateTo(this, 0);
    facing = Fighter_getFacing(this);
    this->force.vx += hit->forceX * facing;
    this->force.vy += hit->forceY;
    this->hitDamage = hit->damage;
    this->hitKind = hit->kind;
    vx = hit->knockbackX * Fighter_getFacing(this);
    this->hitKnockback.vy = hit->knockbackY;
    this->hitKnockback.vz = 0;
    this->hitKnockback.vx = vx;
    Fighter_spawnDust(this, -1, 0x60, 4);
}

/* Spawns the special move 2's effect. */
void Agumon_spawnSpecial2Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x4C);

    if (child != NULL) {
        AgumonSpecial2Effect_init(entityOperatorNewAt(0x4C, child), this);
    }
    Fighter_addEntity(this, child);
}

extern EntityVtable THROW_LAUNCH_VTABLE;

/* Builds the launch for fighter's throw: takes hold of its opponent's handle and height, stops it (velocity and force
 * 0), hits it with 0x40000000 and vel, and turns it by side. */
ThrowLaunch *ThrowLaunch_init(ThrowLaunch *this, Fighter *fighter, Vec3 *vel, u32 hit, s32 side) {
    Fighter *target;
    Vec3 position;

    entityInit(&this->entity, 0);
    this->entity.vtable = &THROW_LAUNCH_VTABLE;
    target = Fighter_getOpponent(fighter);
    this->targetHandle = entityGetHandle(&target->entity);
    this->target = target;
    this->fighter = fighter;
    this->side = side;
    this->hit = hit;
    Fighter_getPosition(target, &position);
    this->delay = 1;
    this->lastY = position.vy;
    target->velocity.vx = 0;
    target->velocity.vy = 0;
    target->velocity.vz = 0;
    target->velocityPad = 0;
    target->force.vx = 0;
    target->force.vy = 0;
    target->force.vz = 0;
    target->forcePad = 0;
    Fighter_takeHit(target, 0x40000000, vel, 0);
    if (side > 0) {
        Fighter_faceLeft(target, 0);
    } else {
        Fighter_faceRight(target, 0);
    }
    return this;
}

/* Turns a thrown target's angle by side: 384 a tick when it can evolve, else 192. The match depends on this inline
 * taking the angle's address: the game keeps it in a saved register across Fighter_canEvolve. */
static inline void spinThrown(s32 *angle, Fighter *target, s32 side) {
    *angle += side * (Fighter_canEvolve(target) ? 384 : 192);
}

/* After the delay, follows the thrown target: while it rises or falls less than 0x800 a tick it spins (spinThrown)
 * until it lands, which hits it (hit, kind 2). A target gone, or a fall of 0x800 or more, also ends it. At the end
 * the target's spin stops and the launch deletes itself. */
void ThrowLaunch_tick(ThrowLaunch *this) {
    Fighter *target;
    Vec3 position;

    if (this->delay > 0) {
        this->delay--;
        return;
    }
    target = (Fighter *)getEntityByHandle(this->targetHandle);
    if (target != NULL) {
        Fighter_getPosition(target, &position);
        if (position.vy - this->lastY >= -0x800) {
            this->lastY = position.vy;
            if (Fighter_isOnGround(target)) {
                /* ZERO_VECTOR is a VECTOR; only its first three words are read here */
                Fighter_takeHit(target, this->hit, (Vec3 *)&ZERO_VECTOR, 2);
            } else {
                spinThrown(&target->targetRotation.vz, target, this->side);
                return;
            }
        }
    }
    this->target->targetRotation.vz = 0;
    entityDelete(&this->entity);
}

/* Builds the special move 1's projectile, a Projectile of radius 0x80 and speed 0xA00. */
AgumonSpecial1Projectile *AgumonSpecial1Projectile_init(AgumonSpecial1Projectile *this, Fighter *fighter,
                                                        Vec3 *position, Vec3 *velocity) {
    Projectile_init(&this->base, fighter, position, velocity, 0x80, 0x5A81C00, 0xA00, 3);
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
        AgumonSpecial1Effect_init(entityOperatorNewAt(0x38, child), &this->base.position, 0x180, 0xC0,
                                  this->base.unkBC);
    }
    entityAddSibling(&this->base.parts.entity, child);
    Projectile_hit(&this->base, sum);
}

void AgumonSpecial1Projectile_draw(AgumonSpecial1Projectile *this, Camera *camera) {
    s32 doubleRadius;
    s32 prod1;
    s32 prod2;
    MatrixWords m;
    SVECTOR corners[4];
    int new_var;
    PolyGT4Shape shape;
    Vec3 *pos;
    u_long *ot;
    s32 zShift;
    s32 dir;
    s32 radius;
    s32 vramY;
    u8 u;
    s32 x;
    s32 y;
    s32 z;
    s16 vx;
    s16 vx2;
    s32 tpage;

    pos = &this->base.position;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    m.words[0] = 4096;
    new_var = 0x1C0 & 0x3ff;
    m.words[1] = 0;
    m.words[2] = 4096;
    m.words[3] = 0;
    m.words[4] = 4096;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    setModelMatrix(&m.m);

    dir = 1;
    if (this->base.velocity.vx < 0) {
        dir = -1;
    }

    corners[0].vz = 0;
    corners[1].vz = 0;
    corners[2].vz = 0;
    corners[3].vz = 0;
    shape.flags = 0;
    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    shape.v1 = 0;
    shape.v0 = 0;
    shape.v3 = 0x1F;
    shape.v2 = 0x1F;

    radius = this->base.radius;
    shape.colors[0] = (shape.colors[2] = 0x80);
    shape.colors[1] = (shape.colors[3] = 0xC0C0C0);
    doubleRadius = radius * 2;
    corners[0].vy = ((u32) (-radius)) >> 1;
    corners[1].vy = -radius;
    prod1 = dir * doubleRadius;
    corners[2].vy = ((u32) radius) >> 1;
    corners[3].vy = radius;

    u = (this->cell << 5) & 0xFF;
    shape.u0 = (shape.u2 = u);
    vramY = this->base.unkBC;
    shape.u1 = (shape.u3 = u + 0x1F);
    prod1 = -prod1;
    shape.clut = ((vramY + 0xFC) << 6) | ((0x1D0 >> 4) & 0x3f);
    prod2 = dir * radius;
    tpage = (((vramY & 0x100) >> 4) | (new_var >> 6)) | ((vramY & 0x200) << 2);
    vx = prod1;
    vx2 = prod2;
    corners[2].vx = (corners[0].vx = vx);
    corners[3].vx = (corners[1].vx = vx2);
    shape.tpage = tpage;

    PRIM_BUFFER_FREE = buildPolyGT4(&shape, PRIM_BUFFER_FREE, ot, zShift);
}

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
/* Draws the burst using two additive PolyFT4 quads rotated in opposite directions around Z. */
void AgumonSpecial1Effect_draw(AgumonSpecial1Effect *this, Camera *camera) {
    VECTOR pos;
    MatrixWords m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    s32 zShift;
    u_long *ot;
    void *packet;
    s32 angle;
    s32 alpha;
    s16 size;
    s32 maxSize;
    s32 vramY;
    s16 size2;
    VECTOR *v;
    s32 x, y, z;

    packet = PRIM_BUFFER_FREE;
    angle = this->unk30;
    pos = *(VECTOR *) &this->position;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m.m, -angle);
    v = &pos;
    x = v->vx;
    y = v->vy;
    z = v->vz;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    setModelMatrix(&m.m);

    size = this->size;
    corners[3].vz = 0;
    corners[2].vz = 0;
    corners[1].vz = 0;
    corners[0].vz = 0;

    corners[1].vx = (corners[3].vx = size);
    size2 = (s16) (size * 2);
    corners[0].vx = (corners[2].vx = -size);
    corners[0].vy = (corners[1].vy = -size2);
    corners[2].vy = (corners[3].vy = size2);

    maxSize = this->maxSize;
    alpha = (maxSize - size) * 255 / maxSize;

    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    shape.color = alpha | ((alpha >> 1) << 8) | ((alpha >> 2) << 16);

    ((u8 *) (&shape.uv0))[0] = (((u8 *) (&shape.uv2))[0] = 0xC0);
    ((u8 *) (&shape.uv1))[0] = (((u8 *) (&shape.uv3))[0] = 0xEF);
    ((u8 *) (&shape.uv0))[1] = (((u8 *) (&shape.uv1))[1] = 0x20);
    ((u8 *) (&shape.uv2))[1] = (((u8 *) (&shape.uv3))[1] = 0x5F);

    vramY = this->vramY;
    shape.flags = 9;
    shape.clut = ((vramY + 0xFD) << 6) | ((0x1D0 >> 4) & 0x3f);
    shape.tpage = (((((0 & 0x3) << 7) | ((1 & 0x3) << 5)) | ((vramY & 0x100) >> 4)) | ((0x1C0 & 0x3ff) >> 6)) | ((vramY & 0x200) << 2);

    packet = buildPolyFT4(&shape, packet, ot, zShift);
    matrixSetRotationZ(&m.m, angle);
    setModelMatrix(&m.m);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, zShift);
}

/* The special move 2's effect, at the fighter's node 2. */
typedef struct AgumonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ VECTOR position; /* the character's node 2 (Fighter_getNodePosition) */
    /* 0x2C */ s32 unk2C;  /* 0x80, down by 0x2B a tick in state 0 */
    /* 0x30 */ s32 unk30;  /* 0x180, down by 0x40 a tick in state 1, to 0x40 */
    /* 0x34 */ s32 ticks;
    /* 0x38 */ s32 unk38;  /* up by 0x20 a tick in state 1; at 0xC0 the effect ends */
    /* 0x3C */ s32 unk3C;  /* unk38 / 48, at most 3 */
    /* 0x40 */ s32 state;  /* 0 until unk2C runs out, then 1 */
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48; /* Fighter_getVramY of the character */
} AgumonSpecial2Effect;

extern EntityVtable AGUMON_SPECIAL2_EFFECT_VTABLE;

/* Builds the special move 2's effect for fighter, at its node 2. */
AgumonSpecial2Effect *AgumonSpecial2Effect_init(AgumonSpecial2Effect *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &AGUMON_SPECIAL2_EFFECT_VTABLE;
    this->fighter = fighter;
    Fighter_getNodePosition(fighter, &this->position, 2, &ZERO_VECTOR);
    this->unk2C = 0x80;
    this->unk30 = 0x180;
    this->ticks = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->state = 0;
    this->unk44 = 0;
    this->unk48 = Fighter_getVramY(fighter);
    return this;
}

/* State 0 brings unk2C down to 0, then state 1 shrinks unk30 and grows unk38 (unk3C its step) until 0xC0, when the
 * effect deletes itself. While it lives it follows the character's node 2. */
void AgumonSpecial2Effect_tick(AgumonSpecial2Effect *this) {
    switch (this->state) {
    case 0:
        this->unk2C -= 0x2B;
        if (this->unk2C <= 0) {
            this->unk2C = 0;
            this->state = 1;
        }
        break;
    case 1: {
        s32 step;

        this->unk30 -= 0x40;
        if (this->unk30 < 0x40) {
            this->unk30 = 0x40;
        }
        step = this->unk38 / 48;
        if (step > 3) {
            step = 3;
        }
        this->unk3C = step;
        this->unk38 += 0x20;
        if (this->unk38 >= 0xC0) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    }
    this->ticks++;
    Fighter_getNodePosition(this->fighter, &this->position, 2, &ZERO_VECTOR);
}

void func_8008391C(AgumonSpecial2Effect *this, Camera *camera);
void func_80083B90(AgumonSpecial2Effect *this, Camera *camera);

/* Draws by its state: func_8008391C's part, and in state 1 func_80083B90's too. */
void AgumonSpecial2Effect_draw(AgumonSpecial2Effect *this, Camera *camera) {
    switch (this->state) {
    case 0:
        func_8008391C(this, camera);
        break;
    case 1:
        func_8008391C(this, camera);
        func_80083B90(this, camera);
        break;
    }
}

void func_8008391C(AgumonSpecial2Effect *this, Camera *camera) {
    VECTOR temp;
    VECTOR pos;
    MatrixWords m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    s32 zShift;
    u_long *ot;
    void *packet;
    s32 facing;
    s32 offset;
    s32 cell;
    u8 u;
    VECTOR *v;
    s32 x;
    s32 y;
    s32 z;
    s32 vramY;
    u16 unk2C;
    s16 unk30;
    s32 prod;
    s32 vx;

    if ((this->ticks / 3) >= 3) {
        return;
    }

    zShift = camera->zShift + 1;
    ot = camera->ot.slots;
    packet = PRIM_BUFFER_FREE;
    facing = Fighter_getFacing(this->fighter);
    offset = facing * 160;
    memset(&pos, 0, sizeof(pos));
    pos.vx = this->position.vx + offset;
    pos.vy = this->position.vy;
    pos.vz = this->position.vz;
    temp = pos;
    v = &temp;
    x = v->vx;
    y = v->vy;
    z = v->vz;
    m.words[0] = 4096;
    m.words[1] = 0;
    m.words[2] = 4096;
    m.words[3] = 0;
    m.words[4] = 4096;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    setModelMatrix(&m.m);

    corners[3].vx = 0;
    corners[1].vx = 0;
    corners[3].vz = 0;
    corners[2].vz = 0;
    corners[1].vz = 0;
    corners[0].vz = 0;

    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    ((u8 *) (&shape.uv1))[1] = 0x20;
    ((u8 *) (&shape.uv0))[1] = 0x20;
    ((u8 *) (&shape.uv3))[1] = 0x5F;
    ((u8 *) (&shape.uv2))[1] = 0x5F;
    shape.color = 0x505050;
    shape.flags = 9;

    unk2C = this->unk2C;
    unk30 = (s16) this->unk30;
    vramY = this->unk48;
    prod = this->unk30 * facing;
    corners[0].vy = (corners[1].vy = (-unk2C) - unk30);
    corners[2].vy = (corners[3].vy = -unk2C);

    shape.clut = ((vramY + 0xFE) << 6) | ((0x1D0 >> 4) & 0x3f);
    shape.tpage = (((((0 & 0x3) << 7) | ((1 & 0x3) << 5)) | ((vramY & 0x100) >> 4)) | ((0x1C0 & 0x3ff) >> 6)) | ((vramY & 0x200) << 2);

    vx = -((s16) prod);
    corners[0].vx = (corners[2].vx = vx);

    cell = this->ticks / 3;
    u = (cell << 6) & 0xFF;
    ((u8 *) (&shape.uv0))[0] = (((u8 *) (&shape.uv2))[0] = u);
    ((u8 *) (&shape.uv1))[0] = (((u8 *) (&shape.uv3))[0] = u + 0x3F);

    packet = buildPolyFT4(&shape, packet, ot, zShift);
    corners[0].vy = (corners[1].vy = -((u16) corners[0].vy));
    corners[2].vy = (corners[3].vy = -((u16) corners[2].vy));
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, zShift);
}

void func_80083B90(AgumonSpecial2Effect *this, Camera *camera) {
    VECTOR temp;
    VECTOR pos;
    MatrixWords m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    s32 zShift;
    u_long *ot;
    s32 offset;
    s32 size;
    u8 u;
    VECTOR *v;
    s32 x;
    s32 y;
    s32 z;
    s32 vramY;

    zShift = camera->zShift + 1;
    ot = camera->ot.slots;
    offset = Fighter_getFacing(this->fighter) * 160;
    memset(&pos, 0, sizeof(pos));
    pos.vx = this->position.vx + offset;
    pos.vy = this->position.vy;
    pos.vz = this->position.vz;
    temp = pos;
    v = &temp;
    x = v->vx;
    y = v->vy;
    z = v->vz;
    m.words[0] = 4096;
    m.words[1] = 0;
    m.words[2] = 4096;
    m.words[3] = 0;
    m.words[4] = 4096;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    setModelMatrix(&m.m);
    corners[3].vz = 0;
    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    corners[2].vz = 0;
    corners[1].vz = 0;
    corners[0].vz = 0;
    shape.flags = 0;
    ((u8 *) (&shape.uv1))[1] = 0x60;
    ((u8 *) (&shape.uv0))[1] = 0x60;
    ((u8 *) (&shape.uv3))[1] = 0x7F;
    ((u8 *) (&shape.uv2))[1] = 0x7F;
    shape.color = 0x808080;
    size = (s16) this->unk38;
    u = ((u8) this->unk3C << 5) & 0xFF;
    corners[0].vx = (corners[2].vx = -size);
    corners[1].vx = (corners[3].vx = size);
    corners[0].vy = (corners[1].vy = -size);
    corners[2].vy = (corners[3].vy = size);
    ((u8 *) (&shape.uv0))[0] = (((u8 *) (&shape.uv2))[0] = u);
    vramY = this->unk48;
    ((u8 *) (&shape.uv1))[0] = (((u8 *) (&shape.uv3))[0] = u + 0x1F);
    shape.clut = ((vramY + 0xFF) << 6) | ((0x1D0 >> 4) & 0x3f);
    shape.tpage = (((((0 & 0x3) << 7) | ((0 & 0x3) << 5)) | ((vramY & 0x100) >> 4)) | ((0x1C0 & 0x3ff) >> 6)) | ((vramY & 0x200) << 2);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, zShift);
}

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
