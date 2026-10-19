/* The battle's sprite effects: particles and their emitters, hit sparks, sparkles, the evolution's cut-in and
 * flash, and the fighters' motion trails (virtual tables PARTICLE_IMPLOSION_VTABLE to PARTICLE_VTABLE). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_f3.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/game/character.h"
#include "engine/gfx/sprite.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_line_g2.h"
#include "engine/gfx/camera.h"
#include "gte.h"

extern EntityVtable D_8006D54C;
extern EntityVtable D_8006D524;
extern EntityVtable PARTICLE_IMPLOSION_VTABLE;
extern EntityVtable PARTICLE_BURST_VTABLE;
extern EntityVtable D_8006D2EC;
extern EntityVtable D_8006D45C;
extern EntityVtable D_8006D484;
extern EntityVtable SPARKLE_VTABLE;

/* The cut-in of an evolving side (CutIn_init, virtual table CUTIN_VTABLE): the side's image, already in VRAM,
 * slides in with speed lines for 60 frames, then fades. */
typedef struct CutIn {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 side;
    /* 0x1C */ s32 slide; /* eases to 0xAD */
    /* 0x20 */ s32 alpha; /* 0xFF, then 0x20 less a frame */
    /* 0x24 */ s32 frame; /* 60 frames of speed lines */
    /* 0x28 */ Sprite sprite;
} CutIn;
void spriteAnimFillShape(SpriteAnim *sprite, PolyFT4Shape *shape, SVECTOR *corners);

void Trail_record(Trail *this);
void Trail_fadeColors(Trail *this);

Entity *func_800C38D8(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4);

/* The constructors of the particle classes of vtables D_8006D54C, D_8006D524 and D_8006D4D4, which
 * createParticle gets inlined like func_8006D574_init (out-of-line copies: func_800C69C4, func_800C6A74; the
 * D_8006D4D4 one is func_8006D4D4_initSized, game_classes.h). */
static inline void func_8006D54C_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    Particle_initById((Particle *)this, frame + 3, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D54C;
}

static inline void func_8006D524_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    Particle_initById((Particle *)this, 6, pos, vel, size, color, 0);
    this->vtable = &D_8006D524;
}

/* Creates a particle entity of a kind (0 to 11) at pos moving by vel, with size and color: 0 to 6 and 8 are
 * Particle_initById's sprites on that frame (0 to 5 grey, 8 still) with the inlined constructors of
 * SPIN_PARTICLE_VTABLE, D_8006D54C, D_8006D524 and D_8006D4D4, 7 HitSpark_init's, 9 to 11 Sparkle_init's. NULL for any
 * other kind. pos and vel are VECTORs as Particle_initById takes them; the constructors that take a Vec3 get them cast.
 * The match depends on a child per case (a new-expression each): the inlined-constructor cases keep it in s1 and the
 * others in s0, so they are not one variable. */
Entity *createParticle(s32 kind, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    switch (kind) {
    case 0: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D574_init(entityOperatorNewAt(0x58, child), pos, vel, size, 0);
        }
        return child;
    }
    case 1: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D574_init(entityOperatorNewAt(0x58, child), pos, vel, size, 1);
        }
        return child;
    }
    case 2: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D574_init(entityOperatorNewAt(0x58, child), pos, vel, size, 2);
        }
        return child;
    }
    case 3: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D54C_init(entityOperatorNewAt(0x58, child), pos, vel, size, 0);
        }
        return child;
    }
    case 4: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D54C_init(entityOperatorNewAt(0x58, child), pos, vel, size, 1);
        }
        return child;
    }
    case 5: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D54C_init(entityOperatorNewAt(0x58, child), pos, vel, size, 2);
        }
        return child;
    }
    case 6: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D524_init(entityOperatorNewAt(0x58, child), pos, vel, size, color);
        }
        return child;
    }
    case 7: {
        Entity *child = allocEntityMemory(0x48);

        if (child != NULL) {
            HitSpark_init(entityOperatorNewAt(0x48, child), (Vec3 *)pos, vel, size, color);
        }
        return child;
    }
    case 8: {
        Entity *child = allocEntityMemory(0x58);

        if (child != NULL) {
            func_8006D4D4_initSized(entityOperatorNewAt(0x58, child), pos, size, color);
        }
        return child;
    }
    case 9: {
        Entity *child = allocEntityMemory(0x5C);

        if (child != NULL) {
            Sparkle_init(entityOperatorNewAt(0x5C, child), (Vec3 *)pos, (Vec3 *)vel, size, color, 9);
        }
        return child;
    }
    case 10: {
        Entity *child = allocEntityMemory(0x5C);

        if (child != NULL) {
            func_800C38D8(entityOperatorNewAt(0x5C, child), (Vec3 *)pos, (Vec3 *)vel, size, color);
        }
        return child;
    }
    case 11: {
        Entity *child = allocEntityMemory(0x5C);

        if (child != NULL) {
            func_800C391C(entityOperatorNewAt(0x5C, child), (Vec3 *)pos, (Vec3 *)vel, size, color);
        }
        return child;
    }
    }
    return NULL;
}

void spriteAnimRewind(SpriteAnim *this);

/* Constructs a sprite animation on its row of PARTICLE_ANIMS: keeps the 0x100 bit of flags as 0x10, then starts
 * at the first cell (spriteAnimRewind). */
SpriteAnim *spriteAnimInit(SpriteAnim *this, SpriteAnimRow *anim, s32 flags) {
    this->anim = anim;
    this->flags = (flags & 0x100) >> 4;
    spriteAnimRewind(this);
    return this;
}

/* Goes back to the first cell: the anim's x and y, one cell wide and high. */
void spriteAnimRewind(SpriteAnim *this) {
    SpriteAnimRow *anim = this->anim;

    this->left = anim->x;
    this->top = anim->y;
    this->right = anim->x + anim->cellWidth - 1;
    this->bottom = anim->y + anim->cellHeight - 1;
}

/* Moves to the next cell: one cell right along the strip, or back to the strip's x one cell lower at its end. The
 * match depends on the end of the strip tested first (the other order inverts the branch). */
void spriteAnimStep(SpriteAnim *this) {
    s16 left = this->left;
    s16 right = this->right;
    SpriteAnimRow *anim = this->anim;
    s16 top = this->top;
    s16 bottom = this->bottom;
    s16 width = anim->cellWidth;
    s16 height = anim->cellHeight;

    right += width;
    if (right >= (s16)(anim->x + anim->stripWidth)) {
        left = anim->x;
        right = left + width - 1;
        top += height;
        bottom = top + height - 1;
    } else {
        left += width;
    }
    this->left = left;
    this->right = right;
    this->top = top;
    this->bottom = bottom;
}

/* Builds the sprite's shape for buildPolyFT4: a model matrix turned by its angle, scaled by its size and moved to
 * its position, then the square of SQUARE_CORNERS. The match depends on the matrix and the position taken by
 * pointer (the calls get model; the translation is stored into m itself). */
void spriteAnimBuildShape(SpriteAnim *this, PolyFT4Shape *shape) {
    MATRIX m;
    MATRIX *model = &m;
    VECTOR *position = &this->position;

    m.t[0] = position->vx;
    m.t[1] = position->vy;
    m.t[2] = position->vz;
    matrixSetRotationZ(model, this->angle);
    matrixScale(model, this->size, this->size, this->size);
    setModelMatrix(model);
    spriteAnimFillShape(this, shape, SQUARE_CORNERS);
}

/* Makes shape a PolyFT4Shape of the sprite: the four corners, its color, the uvs from its corners (their low
 * bytes) and the clut and tpage words of the anim (halfwords 8 and 9) with the flags of the sprite. The match
 * depends on the anim local (this->anim twice reloads it after the clut store) and on reading the corners
 * top, left, right, bottom first. */
void spriteAnimFillShape(SpriteAnim *this, PolyFT4Shape *shape, SVECTOR *corners) {
    s32 top, left, right, bottom;
    SpriteAnimRow *anim;

    shape->corners[0] = &corners[0];
    shape->corners[1] = &corners[1];
    shape->corners[2] = &corners[2];
    shape->corners[3] = &corners[3];
    shape->color = this->color;
    top = (u8)this->top;
    left = (u8)this->left;
    right = (u8)this->right;
    bottom = (u8)this->bottom;
    shape->uv0 = left | (top << 8);
    shape->uv1 = left | (bottom << 8);
    shape->uv2 = right | (top << 8);
    shape->uv3 = right | (bottom << 8);
    anim = this->anim;
    shape->clut = anim->clut + (this->flags << 10);
    shape->tpage = anim->tpage | this->flags;
}

/* Constructs a particle on an anim row, at pos moving by vel (Particle_setMotion), with size and color. */
Particle *Particle_init(Particle *this, SpriteAnimRow *anim, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags) {
    entityInit(&this->entity, 0);
    spriteAnimInit(&this->sprite, anim, flags);
    this->entity.vtable = &PARTICLE_VTABLE;
    Particle_setMotion(this, pos, vel, size, color);
    return this;
}

/* the particles' anims */
extern SpriteAnimRow PARTICLE_ANIMS[];

/* Constructs a particle like Particle_init, on row anim of PARTICLE_ANIMS. The match depends on the table taken
 * into a local first (its address is loaded before the index is scaled). */
Particle *Particle_initById(Particle *this, s32 anim, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags) {
    SpriteAnimRow *anims;

    entityInit(&this->entity, 0);
    anims = PARTICLE_ANIMS;
    spriteAnimInit(&this->sprite, &anims[anim], flags);
    this->entity.vtable = &PARTICLE_VTABLE;
    Particle_setMotion(this, pos, vel, size, color);
    return this;
}

/* Sets the sprite's motion: at pos with velocity vel, size and color, counters at 0. */
void Particle_setMotion(Particle *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    s32 x, y, z;
    s32 dx, dy, dz;

    this->cell = 0;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->sprite.position.vx = x;
    this->sprite.position.vy = y;
    this->sprite.position.vz = z;
    dx = vel->vx;
    dy = vel->vy;
    dz = vel->vz;
    this->sprite.size = size;
    this->sprite.angle = 0;
    this->age = 0;
    this->sprite.color = color;
    this->velocity.vx = dx;
    this->velocity.vy = dy;
    this->velocity.vz = dz;
}

void spriteAnimStep(SpriteAnim *this);

/* Steps the sprite to its next cell (spriteAnimStep) and returns the cells stepped, one more. */
s32 Particle_stepAnim(Particle *this) {
    spriteAnimStep(&this->sprite);
    return ++this->cell;
}

/* Has spriteAnimFillShape make shape of the sprite with these corners, then sets bit 3 of its flags. */
void Particle_buildShape(Particle *this, PolyFT4Shape *shape, SVECTOR *corners) {
    spriteAnimFillShape(&this->sprite, shape, corners);
    shape->flags |= 8;
}

/* The particle's frame: moves by its velocity, loses a sixteenth of it to drag, and after the first frame steps
 * its cell when age & stepMask is 0, deleting itself once it has stepped all the cells. The match depends on the
 * position sum reading everything before writing (as vecAddInPlace) and the velocity's added field by field. */
void Particle_tick(Particle *this, s32 arg) {
    Vec3 drag;
    s32 x, y, z;
    SpriteAnimRow *anim;

    x = this->sprite.position.vx;
    y = this->sprite.position.vy;
    z = this->sprite.position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->sprite.position.vx = x;
    this->sprite.position.vy = y;
    this->sprite.position.vz = z;
    drag.vx = -this->velocity.vx >> 4;
    drag.vy = -this->velocity.vy >> 4;
    drag.vz = -this->velocity.vz >> 4;
    this->velocity.vx += drag.vx;
    this->velocity.vy += drag.vy;
    this->velocity.vz += drag.vz;
    if (++this->age != 1) {
        anim = this->sprite.anim;
        if (!(this->age & anim->stepMask) && Particle_stepAnim(this) >= anim->cellCount) {
            entityDelete(&this->entity);
        }
    }
}

void spriteAnimBuildShape(SpriteAnim *sprite, PolyFT4Shape *shape);

/* The draw method of the sprite entities (PARTICLE_VTABLE and the classes derived from it): spriteAnimBuildShape makes
 * the shape of the sprite, bit 3 set, and buildPolyFT4 puts it in the camera's ordering table. The match depends on the
 * shape's address kept in a pointer (it stays in a saved register across spriteAnimBuildShape). */
void Particle_draw(Particle *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape = &buffer;

    shape->flags = 0;
    spriteAnimBuildShape(&this->sprite, shape);
    shape->flags |= 8;
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

/* The first method of D_8006D54C and SPIN_PARTICLE_VTABLE: turns the sprite by twice the low half of the velocity's x,
 * grows it by 8, then the base's (Particle_tick) with arg. The game reads that x with lhu: only its low half counts. */
void SpinParticle_tick(Particle *this, s32 arg) {
    this->sprite.angle += (u16)this->velocity.vx * 2;
    this->sprite.size += 8;
    Particle_tick(this, arg);
}

/* Constructs an HitSpark at position, turned the way of dir (its y and x, ratan2), with color. The match depends
 * on the frame table taken into a local first, as in Particle_initById. */
HitSpark *HitSpark_init(HitSpark *this, Vec3 *position, VECTOR *dir, s32 unk3, u32 color) {
    SpriteAnimRow *frames;
    s32 x, y, z;

    entityInit(&this->entity, 0);
    frames = PARTICLE_ANIMS;
    spriteAnimInit(&this->sprite, &frames[7], 0);
    this->entity.vtable = &HIT_SPARK_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->sprite.position.vx = x;
    this->sprite.position.vy = y;
    this->sprite.position.vz = z;
    this->sprite.angle = ratan2(dir->vy, dir->vx);
    this->targetSize = unk3;
    this->sprite.color = color;
    this->sprite.size = 0;
    this->age = 0;
    return this;
}

/* On its first frame bursts 4 kind 9 particles from the spark (spread 0x60, an eighth of its target size each);
 * every frame its size moves halfway to the target, and the spark is deleted at age 5. */
void HitSpark_tick(HitSpark *this) {
    Entity *burst;

    if (this->age == 0) {
        burst = allocEntityMemory(0x3C);
        if (burst != NULL) {
            ParticleBurst_init(entityOperatorNewAt(0x3C, burst), 9, (Vec3 *)&this->sprite.position, 0x60,
                               this->targetSize >> 3, 4, 0xA0FF);
        }
        entityAddSibling(&this->entity, burst);
    }
    this->sprite.size += (this->targetSize - this->sprite.size) >> 1;
    this->age++;
    if (this->age >= 5) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0g", HIT_SPARK_CORNERS);

/* The spark's corners: x from -0x800 to 0x1000, y from -0x1000 to 0x1000 (HitSpark_draw). */
extern SVECTOR HIT_SPARK_CORNERS[4];

/* The draw method of HitSpark: a model matrix at the spark's position, turned by its angle and scaled by its size,
 * then the spark's quad twice: in its color (flags 9, one OT slot further while func_8006F104 of CURRENT_FIGHT
 * gives 0), and again with clut 0x7F20 and tpage 0x58 (flags 0xA). */
void HitSpark_draw(HitSpark *this, Camera *camera) {
    u_long *ot = camera->ot.slots;
    s32 shift = camera->zShift;
    u8 *prim = PRIM_BUFFER_FREE;
    /* The match depends on further being declared before sprite (their saved registers). */
    s32 further = func_8006F104(CURRENT_FIGHT) == 0;
    SpriteAnim *sprite = &this->sprite;
    MatrixWords m;
    PolyFT4Shape shape;
    MATRIX *matrix;
    VECTOR *pos;
    s32 x, y, z;

    pos = &this->sprite.position;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    matrix = &m.m;
    matrixSetRotationZ(matrix, this->sprite.angle);
    matrixScale(matrix, this->sprite.size, this->sprite.size, this->sprite.size);
    setModelMatrix(matrix);
    shape.flags = 0;
    spriteAnimFillShape(sprite, &shape, HIT_SPARK_CORNERS);
    shape.flags = (shape.flags & ~3) | 9;
    prim = buildPolyFT4(&shape, prim, ot, shift + further);
    shape.clut = 0x7F20;
    shape.tpage = 0x58;
    shape.flags = (shape.flags & ~3) | 0xA;
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, prim, ot, shift);
}

/* The draw method of D_8006D4D4: like Particle_draw, without bit 3, one OT slot further while func_8006F104 of
 * CURRENT_FIGHT gives 0. */
void func_800C35D4(Particle *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape;
    s32 further = func_8006F104(CURRENT_FIGHT) == 0;

    shape = &buffer;
    shape->flags = 0;
    spriteAnimBuildShape(&this->sprite, shape);
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + further);
}

/* A sparkle (Sparkle_init, virtual tables D_8006D45C, D_8006D484, SPARKLE_VTABLE): a 16-pixel glint that floats
 * up and grows, its color kept as hue, saturation and value, the value fading over its life of 16 frames. */
typedef struct Sparkle {
    /* 0x00 */ Entity entity;
    /* 0x18 */ SpriteAnim sprite;
    /* 0x40 */ Vec3 velocity;
    /* 0x4C */ u8 velocityPad[0x50 - 0x4C];
    /* 0x50 */ HsvColor hsv;
    /* 0x54 */ s32 startValue; /* the value is startValue * life >> 4 */
    /* 0x58 */ s32 life;       /* 16 frames, counted down */
} Sparkle; /* size 0x5C */

INCLUDE_ASM("game/nonmatchings/game_0g", Sparkle_init);

/* The tick of Sparkle_init's classes: the sprite grows, drifts by its velocity, slows down by 1/8 each frame and
 * rises at random, its brightness fades with its life and it is deleted when the 16 frames are over.
 * The match depends on the position sum read whole (as in vecAddInPlace) and the drag added field by field, the
 * same mixed shape as Particle_tick. */
void Sparkle_tick(Sparkle *this) {
    Vec3 drag;
    s32 x, y, z;

    this->sprite.size += 2;
    x = this->sprite.position.vx;
    y = this->sprite.position.vy;
    z = this->sprite.position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->sprite.position.vx = x;
    this->sprite.position.vy = y;
    this->sprite.position.vz = z;
    drag.vx = -this->velocity.vx >> 3;
    drag.vy = -this->velocity.vy >> 3;
    drag.vz = -this->velocity.vz >> 3;
    this->velocity.vx += drag.vx;
    this->velocity.vy += drag.vy;
    this->velocity.vz += drag.vz;
    this->velocity.vy -= mersenneTwisterGenerate(&RANDOM) & 1;
    this->hsv.value = this->startValue * this->life >> 4;
    this->life--;
    if (this->life <= 0) {
        entityDelete(&this->entity);
    }
}

/* The draw method of Sparkle_init's classes: the sprite's color from its hsv, then like Particle_draw with bits 0
 * and 3 set and bit 1 cleared. */
void Sparkle_draw(Sparkle *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape = &buffer;

    shape->flags = 0;
    convertHsvToRgb((CVECTOR *)&this->sprite.color, this->hsv.hue, this->hsv.saturation, this->hsv.value);
    spriteAnimBuildShape(&this->sprite, shape);
    shape->flags = (shape->flags & ~3) | 9;
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

/* Constructs an entity of D_8006D484: Sparkle_init with entry 10 of PARTICLE_ANIMS. */
Entity *func_800C38D8(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    Sparkle_init(this, arg1, arg2, arg3, arg4, 10);
    this->vtable = &D_8006D484;
    return this;
}

/* Constructs an entity of D_8006D45C: Sparkle_init with entry 11 of PARTICLE_ANIMS. */
Entity *func_800C391C(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    Sparkle_init(this, arg1, arg2, arg3, arg4, 11);
    this->vtable = &D_8006D45C;
    return this;
}

extern EntityVtable D_8006D434;

/* The entity of func_800C3960 (vtable D_8006D434, 0xE4 bytes) for a fighter, by its handle: a ring of 12 points
 * turning above its head while it is dizzy (func_800C39C8). */
typedef struct Unk800C3960 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 fighterHandle; /* the fighter's (entityGetHandle) */
    /* 0x1C */ VECTOR points[12]; /* the ring's, in world space */
    /* 0xDC */ s32 unkDC; /* whether the character's func_80079E2C is 15 */
    /* 0xE0 */ s32 angle; /* the ring's turn: the fighter's facing * 0x40 a tick */
} Unk800C3960; /* size 0xE4 */

/* Constructs the character's entity of vtable D_8006D434. */
Unk800C3960 *func_800C3960(Unk800C3960 *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D434;
    this->fighterHandle = entityGetHandle(&fighter->entity);
    this->angle = 0;
    this->unkDC = func_80079E2C(fighter) == 15;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C39C8);

INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D1CC);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C3B40);

extern EntityVtable D_8006D40C;

/* Constructs an Unk800C3CFC at pos: a random normalized direction, size and color, a random angle and turn, a
 * random span from size / 32 + 1 and a random life of 16 to 47 frames. */
Unk800C3CFC *func_800C3CFC(Unk800C3CFC *this, Vec3 *pos, s32 size, s32 color) {
    s32 span;
    s32 dx, dy, dz;
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D40C;
    span = (size >> 5) + 1;
    dx = (s32)mersenneTwisterGenerate(&RANDOM) >> 1;
    dy = mersenneTwisterGenerate(&RANDOM);
    dz = mersenneTwisterGenerate(&RANDOM);
    this->unk18.vx = dx;
    this->unk18.vy = dy;
    this->unk18.vz = dz;
    vecNormalize(&this->unk18, &this->unk18);
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->unk38 = size;
    this->unk28.vx = x;
    this->unk28.vy = y;
    this->unk28.vz = z;
    this->unk3C = mersenneTwisterGenerate(&RANDOM) & 0xFFF;
    this->unk40 = (s32)mersenneTwisterGenerate(&RANDOM) % 48;
    this->unk44 = mersenneTwisterGenerate(&RANDOM) % span + span;
    this->unk48 = color;
    this->unk4C = 0;
    this->unk50 = (mersenneTwisterGenerate(&RANDOM) & 0x1F) + 0x10;
    return this;
}

/* The first method of Unk800C3CFC: turns the angle, and deletes the entity once it has lived longer than 0x50. */
void func_800C3E64(Unk800C3CFC *this) {
    this->unk3C += this->unk40;
    if (++this->unk4C > this->unk50) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C3ED0);

INCLUDE_RODATA("game/nonmatchings/game_0g", CUTIN_CLUT_RECTS);
INCLUDE_RODATA("game/nonmatchings/game_0g", CUTIN_IMAGE_RECTS);

INCLUDE_ASM("game/nonmatchings/game_0g", CutIn_init);

INCLUDE_ASM("game/nonmatchings/game_0g", CutIn_tick);

INCLUDE_ASM("game/nonmatchings/game_0g", CutIn_draw);

/* A speed line of the cut-in (SpeedLine_init, virtual table SPEED_LINE_VTABLE): a horizontal line whose front
 * moves at speed and its back at half of it. */
typedef struct SpeedLine {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 frontX; /* backX + speed at first */
    /* 0x1C */ s32 backX;
    /* 0x20 */ s32 y;
    /* 0x24 */ s32 speed;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
} SpeedLine;

extern EntityVtable SPEED_LINE_VTABLE;

/* Constructs a speed line at (x, y) moving at speed, its front one step ahead of its back. */
SpeedLine *SpeedLine_init(SpeedLine *this, s32 x, s32 y, s32 speed) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &SPEED_LINE_VTABLE;
    this->frontX = x + speed;
    this->backX = x;
    this->y = y;
    this->speed = speed;
    this->unk28 = 0;
    this->unk2C = 0;
    return this;
}

/* The speed line's tick: the front moves at speed and the back at half of it; deleted once the back leaves 0 to
 * 320. */
void SpeedLine_tick(SpeedLine *this) {
    s32 step = this->speed;

    this->frontX += step;
    this->backX += step >> 1;
    if ((u32)this->backX > 320) {
        entityDelete(&this->entity);
    }
}

/* The speed line's draw: a semi-transparent line from its front in grey 0xC0 to its back in black,
 * after a tpage 0x29, two slots after the camera's last drawn one. The match depends on y read once into a local
 * (a word load, where the x reads are narrowed to halfwords). */
void SpeedLine_draw(SpeedLine *this, Camera *camera) {
    u_long *ot = camera->ot.lastDrawn + 2;
    DR_TPAGE *tpage = allocDrTpage(0x29);
    LINE_G2 *line = allocLineG2();
    s32 y = this->y;

    line->x0 = this->frontX;
    line->y0 = y;
    line->x1 = this->backX;
    line->y1 = y;
    line->r0 = 0xC0;
    line->g0 = 0xC0;
    line->b0 = 0xC0;
    line->r1 = 0;
    line->g1 = 0;
    line->b1 = 0;
    setSemiTrans(line, 1);
    AddPrim(ot, line);
    AddPrim(ot, tpage);
}

extern EntityVtable TRANSFORM_FLASH_VTABLE;
/* each character's other form (its evolution, or the form it evolved from), by CHARACTER_* */
extern s32 CHARA_OTHER_FORM[];

/* Sets *target to the height of fighter's other form, or to *ownHeight when func_800777BC holds.
 * The match depends on the store through target: its address is taken before the call and kept in a register. */
static inline void TransformFlash_setFormHeight(s32 *target, Fighter *fighter, s32 *ownHeight) {
    *target = func_800777BC(fighter) ? *ownHeight
                                     : FIGHTER_PARAMS[CHARA_OTHER_FORM[Fighter_getCharacter(fighter)]].height;
}

/* Constructs the flash of owner's evolution at its fighter's position: from the fighter's height to the other
 * form's (unless func_800777BC holds: then the fighter's own), radius 0 out to twice that height, opaque. */
TransformFlash *TransformFlash_init(TransformFlash *this, BattleSide *owner) {
    Fighter *fighter;

    entityInit(&this->entity, 0);
    this->entity.vtable = &TRANSFORM_FLASH_VTABLE;
    fighter = owner->current;
    this->side = owner;
    Fighter_getPosition(fighter, &this->position);
    this->height = Fighter_getParams(fighter)->height;
    TransformFlash_setFormHeight(&this->targetHeight, fighter, &this->height);
    this->radius = 0;
    this->maxRadius = this->targetHeight * 2;
    this->alpha = 0xFF;
    this->phase = 0;
    this->frame = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", TransformFlash_tick);

/* Spawns the entity of func_800C3CFC at the position, as a sibling. */
void TransformFlash_spawnFlare(TransformFlash *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_800C3CFC(entityOperatorNewAt(0x54, child), &this->position, 0x280, 0x403020);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of ParticleBurst_init at the position (radius 0x60, colour 0x8090A0), as a sibling. */
void TransformFlash_spawnBurst(TransformFlash *this, s32 arg1, s32 count) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), arg1, &this->position, 0x60, 0x20, count, 0x8090A0);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_RODATA("game/nonmatchings/game_0g", TRANSFORM_FLASH_DISC);

/* the flash's disc: its center, then 17 points round a circle of radius 0x1000 in x and y (the last is the first) */
extern SVECTOR TRANSFORM_FLASH_DISC[18];

/* Sets a matrix's translation t to v. */
static inline void setTranslation(long *t, Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    t[0] = x;
    t[1] = y;
    t[2] = z;
}

/* Scales m (matrixScale) and makes it the GTE's model matrix (setModelMatrix). */
static inline void setScaledModelMatrix(MATRIX *m, s32 x, s32 y, s32 z) {
    matrixScale(m, x, y, z);
    setModelMatrix(m);
}

/* The flash's draw: a disc facing the camera at the position, radius wide and an eighth more high, as 8 POLY_G4
 * of two rim segments each, its center colored by alpha (red alpha / 2, green 3/4 alpha, blue alpha) and its rim
 * black. The match depends on the translation set by an inline taking m.t (the position read through its pointer
 * parameter, the stores sp-relative) and on the two matrix calls made by the setScaledModelMatrix inline (&m
 * kept in a saved register). */
void TransformFlash_draw(TransformFlash *this, Camera *camera) {
    MATRIX m;
    PolyG4Shape shape;
    u8 *packet = PRIM_BUFFER_FREE;
    u_long *ot;
    s32 zShift;
    u32 alpha;
    s32 i;

    m = camera->view->rotation;
    zShift = camera->zShift + 1;
    setTranslation(m.t, &this->position);
    ot = camera->ot.slots;
    setScaledModelMatrix(&m, this->radius, this->radius + (this->radius >> 3), this->radius);
    shape.corners[0] = &TRANSFORM_FLASH_DISC[1];
    shape.corners[1] = &TRANSFORM_FLASH_DISC[2];
    shape.corners[2] = &TRANSFORM_FLASH_DISC[0];
    shape.corners[3] = &TRANSFORM_FLASH_DISC[3];
    shape.colors[0] = shape.colors[1] = shape.colors[3] = 0;
    alpha = this->alpha;
    shape.flags = 9;
    shape.colors[2] = (alpha >> 1) | ((alpha - (alpha >> 2)) << 8) | (alpha << 16);
    for (i = 0; i < 8; i++) {
        packet = buildPolyG4(&shape, packet, ot, zShift);
        shape.corners[0] += 2;
        shape.corners[1] += 2;
        shape.corners[3] += 2;
    }
    PRIM_BUFFER_FREE = packet;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4F74);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C52C4);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5480);

INCLUDE_ASM("game/nonmatchings/game_0g", Trail_init);

/* The trail's tick: records (Trail_record) while recordTimer runs, which ends at once when the fighter stops
 * attacking or is held; then shortens, and is deleted below 3 samples. Its colors fade every frame it lives. */
void Trail_tick(Trail *this, CollisionSphere *sum) {
    if (--this->recordTimer < 0) {
        if (--this->length < 3) {
            entityDelete(&this->entity);
            return;
        }
    } else {
        Trail_record(this);
        if (Fighter_isAttackIdle(this->fighter) || Fighter_getBodyStateId(this->fighter) == FIGHTER_STATE_HELD) {
            this->recordTimer = 0;
        }
    }
    Trail_fadeColors(this);
}

/* Records the trail's points: moves head on in the ring, samples every point's model position (its node and
 * offset, Fighter_getNodePosition) and color into it, then counts the sample in validCount, which starts again
 * when the first point jumped more than 0x300 from the last sample. The match depends on the position narrowed
 * through s16 locals (lh loads), the point count read into a local, and the last sample's address taken before the
 * head's. */
void Trail_record(Trail *this) {
    s32 previous = this->head;
    Fighter *fighter = this->fighter;
    TrailPoint *point;
    TrailSample *sample;
    TrailSample *last;
    VECTOR position;
    s32 count;
    s32 head;
    s32 i;

    if (previous < 0) {
        previous = 0;
    }
    point = this->points;
    head = (this->head + 1) & 0x1F;
    count = this->pointCount;
    this->head = head;
    for (i = 0; i < count; i++) {
        VECTOR offset = point->offset;
        s16 x, y, z;

        Fighter_getNodePosition(fighter, &position, point->node, &offset);
        sample = &this->samples[i][head];
        x = position.vx;
        y = position.vy;
        z = position.vz;
        sample->x = x;
        sample->y = y;
        sample->z = z;
        /* one word: a CVECTOR assignment copies with lwl/lwr, the game with lw/sw */
        *(u32 *)&sample->color = *(u32 *)&point->color;
        point++;
    }
    last = &this->samples[0][previous];
    sample = &this->samples[0][head];
    position.vx = sample->x - last->x;
    position.vy = sample->y - last->y;
    position.vz = sample->z - last->z;
    if ((u32)(position.vx * position.vx + position.vy * position.vy + position.vz * position.vz) > 0x90000) {
        this->validCount = 0;
    }
    this->validCount++;
}

INCLUDE_ASM("game/nonmatchings/game_0g", Trail_fadeColors);

/* Copies up to 4 entries of the table, all but their word at 0x10, and keeps their count. The match depends on
 * the entries indexed and the table walked with a post-increment taken first (its pointer moves at the loop's top),
 * and on the vector read whole before it is stored. */
void Trail_setPoints(Trail *this, TrailPoint *entries, s32 count) {
    s32 i;

    if (count > 4) {
        count = 4;
    }
    this->pointCount = count;
    for (i = 0; i < count; i++) {
        TrailPoint *entry = &this->points[i];
        TrailPoint *src = entries++;
        s32 x, y, z;

        entry->node = src->node;
        x = src->offset.vx;
        y = src->offset.vy;
        z = src->offset.vz;
        entry->offset.vx = x;
        entry->offset.vy = y;
        entry->offset.vz = z;
        entry->color = src->color;
    }
}

/* Sets the colors of the entries in use, up to count of them. The match depends on colors walked with ++ while the
 * entries are indexed (the colors' pointer then moves first). */
void Trail_setColors(Trail *this, CVECTOR *colors, s32 count) {
    s32 n = this->pointCount;
    s32 i;

    if (count < n) {
        n = count;
    }
    for (i = 0; i < n; i++) {
        this->points[i].color = *colors++;
    }
}

/* The trail's draw: loads the camera's matrix from the scratchpad into the GTE, then draws the strips through
 * the drawStrips slot. */
void Trail_draw(Trail *this, Camera *camera) {
    TrailVtable *vtable;

    gte_loadRotTransKeepArgs(SCRATCH_VIEW_MATRIX);
    vtable = (TrailVtable *)this->entity.vtable;
    vtable->drawStrips.func((u8 *)this + vtable->drawStrips.delta, camera);
}

/* Draws the trail (TrailVtable.drawStrips): a POLY_G4 between each two neighbouring points for each two samples in
 * a row, from the newest back over length - 1 samples (at most the valid ones), gouraud between the samples' colors;
 * the newest end of the first strip is black when there are two strips or more. A sample starts with an SVECTOR's
 * layout, so the corners point at the samples; the colors are copied as words (CVECTOR to the shape's u32). The
 * match depends on the four samples taken by pointer (the game reads the colors through the corners' registers). */
void Trail_drawStrips(Trail *this, Camera *camera) {
    PolyG4Shape shape;
    u8 *packet = PRIM_BUFFER_FREE;
    u_long *ot;
    s32 zShift;
    s32 strips;
    s32 edges;
    s32 cur;
    s32 prev;
    s32 i;
    s32 j;

    strips = this->length - 1;
    shape.flags = 9;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    if (this->validCount - 1 < strips) {
        strips = this->validCount - 1;
    }
    cur = this->head;
    edges = this->pointCount - 1;
    for (i = 0; i < strips; i++) {
        prev = (cur - 1) & 0x1F;
        for (j = 0; j < edges; j++) {
            TrailSample *newer = &this->samples[j][cur];
            TrailSample *older = &this->samples[j][prev];
            TrailSample *nextNewer = &this->samples[j + 1][cur];
            TrailSample *nextOlder = &this->samples[j + 1][prev];

            shape.corners[0] = (SVECTOR *)newer;
            shape.corners[1] = (SVECTOR *)older;
            shape.corners[2] = (SVECTOR *)nextNewer;
            shape.corners[3] = (SVECTOR *)nextOlder;
            if (i == 0 && strips >= 2) {
                shape.colors[0] = shape.colors[2] = 0;
            } else {
                shape.colors[0] = *(u32 *)&newer->color;
                shape.colors[2] = *(u32 *)&nextNewer->color;
            }
            shape.colors[1] = *(u32 *)&older->color;
            shape.colors[3] = *(u32 *)&nextOlder->color;
            packet = buildPolyG4(&shape, packet, ot, zShift);
        }
        cur = prev;
    }
    PRIM_BUFFER_FREE = packet;
}

/* A particle spray (ParticleSpray_init, virtual table PARTICLE_SPRAY_VTABLE): emits particles of createParticle's
 * kind at pos with vel, both jittered by half of vel's length (count > 0: one a frame; < 0: all at once). */
typedef struct ParticleSpray {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 kind;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 positionPad;
    /* 0x2C */ Vec3 velocity;
    /* 0x38 */ s32 velocityPad;
    /* 0x3C */ s32 size;
    /* 0x40 */ s32 count;
    /* 0x44 */ s32 color;
} ParticleSpray;

extern EntityVtable PARTICLE_SPRAY_VTABLE;

/* Constructs a particle spray of kind at pos moving by vel, with size, count and color. The match depends on
 * reading each vector's three words before storing any, as vecCopy does. */
ParticleSpray *ParticleSpray_init(ParticleSpray *this, s32 kind, Vec3 *pos, Vec3 *vel, s32 size, s32 count,
                                  s32 color) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &PARTICLE_SPRAY_VTABLE;
    this->kind = kind;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    x2 = vel->vx;
    y2 = vel->vy;
    z2 = vel->vz;
    this->velocity.vx = x2;
    this->velocity.vy = y2;
    this->velocity.vz = z2;
    this->size = size;
    this->count = count;
    this->color = color;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", ParticleSpray_tick);

/* Constructs an ParticleEmitter (vtable D_8006D2EC): count particles around pos. */
ParticleEmitter *func_800C6088(ParticleEmitter *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D2EC;
    this->kind = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->spread = radius;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->size = arg4;
    this->count = count;
    this->color = arg6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C6134);

/* Constructs a ParticleEmitter (vtable PARTICLE_BURST_VTABLE): count particles around pos. */
ParticleEmitter *ParticleBurst_init(ParticleEmitter *this, s32 kind, Vec3 *pos, s32 spread, s32 size, s32 count,
                                    s32 color) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &PARTICLE_BURST_VTABLE;
    this->kind = kind;
    /* The match depends on reading pos into x, y and z first, and on storing spread before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->spread = spread;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->size = size;
    this->count = count;
    this->color = color;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", ParticleBurst_tick);

/* Constructs a ParticleEmitter (vtable PARTICLE_IMPLOSION_VTABLE): count particles around pos. */
ParticleEmitter *ParticleImplosion_init(ParticleEmitter *this, s32 kind, Vec3 *pos, s32 spread, s32 size, s32 count,
                                        s32 color) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &PARTICLE_IMPLOSION_VTABLE;
    this->kind = kind;
    /* The match depends on reading pos into x, y and z first, and on storing spread before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->spread = spread;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->size = size;
    this->count = count;
    this->color = color;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", ParticleImplosion_tick);

/* Returns the first word of what it is given. Nothing in game.bin calls it. */
s32 func_800C67C0(s32 *words) {
    return words[0];
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void Particle_destroy(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs a sprite entity of PARTICLE_VTABLE on frame with flags, without the motion of Particle_setMotion. */
Particle *Particle_initStill(Particle *this, SpriteAnimRow *frame, s32 flags) {
    entityInit(&this->entity, 0);
    spriteAnimInit(&this->sprite, frame, flags);
    this->entity.vtable = &PARTICLE_VTABLE;
    return this;
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void SpinParticle_destroy(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of SPIN_PARTICLE_VTABLE: Particle_initById with the arguments and 0. */
Entity *SpinParticle_init(Entity *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    Particle_initById((Particle *)this, frame, pos, vel, size, color, 0);
    this->vtable = &SPIN_PARTICLE_VTABLE;
    return this;
}

/* Constructs an entity of SPIN_PARTICLE_VTABLE: Particle_initById with frame, color 0x808080 and no flags. */
Entity *SpinParticle_initGrey(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    Particle_initById((Particle *)this, frame, pos, vel, size, 0x808080, 0);
    this->vtable = &SPIN_PARTICLE_VTABLE;
    return this;
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800C6970(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D54C: Particle_initById with frame + 3, color 0x808080 and no flags. */
Entity *func_800C69C4(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    Particle_initById((Particle *)this, frame + 3, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D54C;
    return this;
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800C6A20(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D524: Particle_initById with frame 6 and no flags. */
Particle *func_800C6A74(Particle *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    Particle_initById(this, 6, pos, vel, size, color, 0);
    this->entity.vtable = &D_8006D524;
    return this;
}

/* A copy of the destructor of the entity class of vtable HIT_SPARK_VTABLE, which HitSpark_init
 * installs: game has 2 identical ones. */
void HitSpark_destroy(Entity *this, s32 flags) {
    this->vtable = &HIT_SPARK_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800C6B1C(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D4D4: Particle_initById with frame 8, the velocity ZERO_VECTOR and no flags. */
Particle *func_800C6B70(Particle *this, VECTOR *pos, s32 size, u32 color) {
    Particle_initById(this, 8, pos, &ZERO_VECTOR, size, color, 0);
    this->entity.vtable = &D_8006D4D4;
    return this;
}

/* A copy of the destructor of the entity class of vtable SPARKLE_VTABLE, which Sparkle_init
 * installs: game has 3 identical ones. */
void Sparkle_destroy(Entity *this, s32 flags) {
    this->vtable = &SPARKLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable SPARKLE_VTABLE, which Sparkle_init
 * installs: game has 3 identical ones. */
void func_800C6C14(Entity *this, s32 flags) {
    this->vtable = &SPARKLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable SPARKLE_VTABLE, which Sparkle_init
 * installs: game has 3 identical ones. */
void func_800C6C68(Entity *this, s32 flags) {
    this->vtable = &SPARKLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D434, which func_800C3960 installs. */
void func_800C6CBC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D40C, which func_800C3CFC installs. */
void func_800C6D04(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable CUTIN_VTABLE, which CutIn_init installs:
 * its sprite, then the entity. */
void CutIn_destroy(CutIn *this, s32 flags) {
    spriteDestroy(&this->sprite, DESTROY_BASES);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable SPEED_LINE_VTABLE, which SpeedLine_init installs. */
void SpeedLine_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable TRANSFORM_FLASH_VTABLE, which TransformFlash_init installs. */
void TransformFlash_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D36C, which func_800C4F74 installs. */
void func_800C6E34(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable TRAIL_VTABLE, which Trail_init installs. */
void Trail_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* While recordTimer runs, records and fades the trail (no caller found). */
void func_800C6EC4(Trail *this) {
    if (this->recordTimer > 0) {
        Trail_record(this);
        Trail_fadeColors(this);
    }
}

/* The destructor of the entity class of vtable PARTICLE_SPRAY_VTABLE, which ParticleSpray_init installs. */
void ParticleSpray_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D2EC, which func_800C6088 installs. */
void func_800C6F4C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable PARTICLE_BURST_VTABLE, which ParticleBurst_init installs. */
void ParticleBurst_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable PARTICLE_IMPLOSION_VTABLE, which ParticleImplosion_init installs. */
void ParticleImplosion_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0g", PARTICLE_IMPLOSION_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", PARTICLE_BURST_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D2EC);
INCLUDE_RODATA("game/nonmatchings/game_0g", PARTICLE_SPRAY_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", TRAIL_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D36C);
INCLUDE_RODATA("game/nonmatchings/game_0g", TRANSFORM_FLASH_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", SPEED_LINE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", CUTIN_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D40C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D434);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D45C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D484);
INCLUDE_RODATA("game/nonmatchings/game_0g", SPARKLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D4D4);
INCLUDE_RODATA("game/nonmatchings/game_0g", HIT_SPARK_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D524);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D54C);
INCLUDE_RODATA("game/nonmatchings/game_0g", SPIN_PARTICLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", PARTICLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0g", PARTICLE_ANIMS);
INCLUDE_RODATA("game/nonmatchings/game_0g", CHARA_OTHER_FORM);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D728);
