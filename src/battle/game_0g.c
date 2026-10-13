/* Entity classes the fighters derive from (virtual tables D_8006D29C to D_8006D59C). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/game/character.h"
#include "engine/gfx/sprite.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_line_g2.h"

extern EntityVtable D_8006D54C;
extern EntityVtable D_8006D524;
extern EntityVtable D_8006D29C;
extern EntityVtable D_8006D2C4;
extern EntityVtable D_8006D2EC;
extern EntityVtable D_8006D45C;
extern EntityVtable D_8006D484;
extern EntityVtable D_8006D4AC;

/* The entity func_800C4204 constructs (virtual table D_8006D3E4), with a Sprite at 0x28. */
typedef struct Unk800C4204 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x28 - 0x18];
    /* 0x28 */ Sprite sprite;
} Unk800C4204;
void func_800C2E84(Unk800C2CCC *sprite, PolyFT4Shape *shape, SVECTOR *corners);

void func_800C5834(Unk800C5680 *this);
void func_800C59D4(Unk800C5680 *this);

Entity *func_800C38D8(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4);

/* The constructors of the particle classes of vtables D_8006D54C, D_8006D524 and D_8006D4D4, which
 * func_800C28B8 gets inlined like func_8006D574_init (out-of-line copies: func_800C69C4, func_800C6A74; the
 * D_8006D4D4 one with size and color, unlike func_8006D4D4_init). */
static inline void func_8006D54C_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame + 3, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D54C;
}

static inline void func_8006D524_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    func_800C2FBC((Unk800C2F14 *)this, 6, pos, vel, size, color, 0);
    this->vtable = &D_8006D524;
}

static inline void func_8006D4D4_initSized(Entity *this, VECTOR *pos, s32 size, u32 color) {
    func_800C2FBC((Unk800C2F14 *)this, 8, pos, &D_80010814, size, color, 0);
    this->vtable = &D_8006D4D4;
}

/* Creates a particle entity of a kind (0 to 11) at pos moving by vel, with size and color: 0 to 6 and 8 are
 * func_800C2FBC's sprites on that frame (0 to 5 grey, 8 still) with the inlined constructors of D_8006D574,
 * D_8006D54C, D_8006D524 and D_8006D4D4, 7 func_800C32E0's, 9 to 11 func_800C3654's. NULL for any other kind.
 * pos and vel are VECTORs as func_800C2FBC takes them; the constructors that take a Vec3 get them cast.
 * The match depends on a child per case (a new-expression each): the inlined-constructor cases keep it in s1
 * and the others in s0, so they are not one variable. */
Entity *func_800C28B8(s32 kind, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
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
            func_800C32E0(entityOperatorNewAt(0x48, child), (Vec3 *)pos, vel, size, color);
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
            func_800C3654(entityOperatorNewAt(0x5C, child), (Vec3 *)pos, (Vec3 *)vel, size, color, 9);
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

void func_800C2D04(Unk800C2CCC *this);

/* Constructs an Unk800C2CCC on frame: keeps the 0x100 bit of flags as 0x10, then its corners (func_800C2D04). */
Unk800C2CCC *func_800C2CCC(Unk800C2CCC *this, u16 *frame, s32 flags) {
    this->frame = frame;
    this->flags = (flags & 0x100) >> 4;
    func_800C2D04(this);
    return this;
}

/* Sets the corners of the sprite from its frame's x, y, w and h. */
void func_800C2D04(Unk800C2CCC *this) {
    u16 *frame = this->frame;

    this->left = frame[0];
    this->top = frame[1];
    this->right = frame[0] + frame[2] - 1;
    this->bottom = frame[1] + frame[3] - 1;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C2D58);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C2DF4);

/* Makes shape a PolyFT4Shape of the sprite: the four corners, its color, the uvs from its corners (their low
 * bytes) and the clut and tpage words of the frame (halfwords 8 and 9) with the flags of the sprite. The match
 * depends on the frame local (this->frame twice reloads it after the clut store) and on reading the corners
 * top, left, right, bottom first. */
void func_800C2E84(Unk800C2CCC *this, PolyFT4Shape *shape, SVECTOR *corners) {
    s32 top, left, right, bottom;
    u16 *frame;

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
    frame = this->frame;
    shape->clut = frame[8] + (this->flags << 10);
    shape->tpage = frame[9] | this->flags;
}

/* Constructs a sprite entity: the given frame, moved from pos by vel (func_800C3078), size and color. */
Unk800C2F14 *func_800C2F14(Unk800C2F14 *this, u16 *frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color,
                           s32 flags) {
    entityInit(&this->entity, 0);
    func_800C2CCC(&this->sprite, frame, flags);
    this->entity.vtable = &D_8006D59C;
    func_800C3078(this, pos, vel, size, color);
    return this;
}

/* the sprite frames of func_800C2FBC: x, y, w, h, ..., clut and tpage at 8 and 9 */
extern u16 D_8006D5C4[][10];

/* Constructs a sprite entity like func_800C2F14, with entry frame of D_8006D5C4. The match depends on the table
 * taken into a local first (its address is loaded before the index is scaled). */
Unk800C2F14 *func_800C2FBC(Unk800C2F14 *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags) {
    u16 (*frames)[10];

    entityInit(&this->entity, 0);
    frames = D_8006D5C4;
    func_800C2CCC(&this->sprite, frames[frame], flags);
    this->entity.vtable = &D_8006D59C;
    func_800C3078(this, pos, vel, size, color);
    return this;
}

/* Sets the sprite's motion: at pos with velocity vel, size and color, counters at 0. */
void func_800C3078(Unk800C2F14 *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    s32 x, y, z;
    s32 dx, dy, dz;

    this->unk40 = 0;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->sprite.pos.vx = x;
    this->sprite.pos.vy = y;
    this->sprite.pos.vz = z;
    dx = vel->vx;
    dy = vel->vy;
    dz = vel->vz;
    this->sprite.size = size;
    this->sprite.unk6 = 0;
    this->unk54 = 0;
    this->sprite.color = color;
    this->unk44.vx = dx;
    this->unk44.vy = dy;
    this->unk44.vz = dz;
}

void func_800C2D58(Unk800C2CCC *this);

/* Steps the sprite's frame (func_800C2D58) and returns its frame count, one more. */
s32 func_800C30C4(Unk800C2F14 *this) {
    func_800C2D58(&this->sprite);
    return ++this->unk40;
}

/* Has func_800C2E84 make shape of the sprite with these corners, then sets bit 3 of its flags. */
void func_800C3100(Unk800C2F14 *this, PolyFT4Shape *shape, SVECTOR *corners) {
    func_800C2E84(&this->sprite, shape, corners);
    shape->flags |= 8;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C3134);

void func_800C2DF4(Unk800C2CCC *sprite, PolyFT4Shape *shape);

/* The draw method of the sprite entities (D_8006D59C and the classes derived from it): func_800C2DF4 makes the
 * shape of the sprite, bit 3 set, and buildPolyFT4 puts it in the camera's ordering table. The match depends on
 * the shape's address kept in a pointer (it stays in a saved register across func_800C2DF4). */
void func_800C3238(Unk800C2F14 *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape = &buffer;

    shape->flags = 0;
    func_800C2DF4(&this->sprite, shape);
    shape->flags |= 8;
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

/* The first method of D_8006D54C and D_8006D574: turns the sprite by twice the low half of the velocity's x, grows
 * it by 8, then the base's (func_800C3134) with arg. The game reads that x with lhu: only its low half counts. */
void func_800C32A4(Unk800C2F14 *this, s32 arg) {
    this->sprite.unk6 += (u16)this->unk44.vx * 2;
    this->sprite.size += 8;
    func_800C3134(this, arg);
}

/* Constructs an Unk800C32E0 at position, turned the way of dir (its y and x, ratan2), with color. The match depends
 * on the frame table taken into a local first, as in func_800C2FBC. */
Unk800C32E0 *func_800C32E0(Unk800C32E0 *this, Vec3 *position, VECTOR *dir, s32 unk3, u32 color) {
    u16 (*frames)[10];
    s32 x, y, z;

    entityInit(&this->entity, 0);
    frames = D_8006D5C4;
    func_800C2CCC(&this->sprite, frames[7], 0);
    this->entity.vtable = &D_8006D4FC;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->sprite.pos.vx = x;
    this->sprite.pos.vy = y;
    this->sprite.pos.vz = z;
    this->sprite.unk6 = ratan2(dir->vy, dir->vx);
    this->unk40 = unk3;
    this->sprite.color = color;
    this->sprite.size = 0;
    this->unk44 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C339C);

INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D1AC);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C347C);

/* The draw method of D_8006D4D4: like func_800C3238, without bit 3, one OT slot further while func_8006F104 of
 * D_800D377C gives 0. */
void func_800C35D4(Unk800C2F14 *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape;
    s32 further = func_8006F104(D_800D377C) == 0;

    shape = &buffer;
    shape->flags = 0;
    func_800C2DF4(&this->sprite, shape);
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + further);
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C3654);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C374C);

/* The entity func_800C3654 constructs (virtual tables D_8006D45C, D_8006D484, D_8006D4AC): a sprite with a
 * velocity at 0x40 and its color kept as hue, saturation and value at 0x50. */
typedef struct Unk800C3654 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800C2CCC sprite;
    /* 0x40 */ Vec3 unk40;
    /* 0x4C */ u8 unk4C[0x50 - 0x4C];
    /* 0x50 */ HsvColor hsv;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
} Unk800C3654; /* size 0x5C */

/* The draw method of func_800C3654's classes: the sprite's color from its hsv, then like func_800C3238 with bits 0
 * and 3 set and bit 1 cleared. */
void func_800C384C(Unk800C3654 *this, Camera *camera) {
    PolyFT4Shape buffer;
    PolyFT4Shape *shape = &buffer;

    shape->flags = 0;
    convertHsvToRgb((CVECTOR *)&this->sprite.color, this->hsv.hue, this->hsv.saturation, this->hsv.value);
    func_800C2DF4(&this->sprite, shape);
    shape->flags = (shape->flags & ~3) | 9;
    PRIM_BUFFER_FREE = buildPolyFT4(shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift);
}

/* Constructs an entity of D_8006D484: func_800C3654 with entry 10 of D_8006D5C4. */
Entity *func_800C38D8(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    func_800C3654(this, arg1, arg2, arg3, arg4, 10);
    this->vtable = &D_8006D484;
    return this;
}

/* Constructs an entity of D_8006D45C: func_800C3654 with entry 11 of D_8006D5C4. */
Entity *func_800C391C(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4) {
    func_800C3654(this, arg1, arg2, arg3, arg4, 11);
    this->vtable = &D_8006D45C;
    return this;
}

extern EntityVtable D_8006D434;

/* The entity of func_800C3960 (vtable D_8006D434, 0xE4 bytes) for a character, by its handle. */
typedef struct Unk800C3960 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 chara; /* the character's handle */
    /* 0x1C */ u8 unk1C[0xDC - 0x1C];
    /* 0xDC */ s32 unkDC; /* whether the character's func_80079E2C is 15 */
    /* 0xE0 */ s32 unkE0;
} Unk800C3960; /* size 0xE4 */

/* Constructs the character's entity of vtable D_8006D434. */
Unk800C3960 *func_800C3960(Unk800C3960 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D434;
    this->chara = entityGetHandle(&chara->entity);
    this->unkE0 = 0;
    this->unkDC = func_80079E2C(chara) == 15;
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

INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D1EC);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D1FC);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4204);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C42D0);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4480);

/* The entity class of virtual table D_8006D3BC (constructor func_800C4860). */
typedef struct Unk800C4860 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18; /* unk1C + unk24 */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
} Unk800C4860;

extern EntityVtable D_8006D3BC;

/* Constructs the entity of virtual table D_8006D3BC with its three values, and the sum of the first and
 * the third at 0x18. */
Unk800C4860 *func_800C4860(Unk800C4860 *this, s32 arg1, s32 arg2, s32 arg3) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D3BC;
    this->unk18 = arg1 + arg3;
    this->unk1C = arg1;
    this->unk20 = arg2;
    this->unk24 = arg3;
    this->unk28 = 0;
    this->unk2C = 0;
    return this;
}

/* The first method of Unk800C4860: moves 0x18 by 0x24 and 0x1C by half of it, and deletes the entity once 0x1C
 * leaves 0 to 320. */
void func_800C48D8(Unk800C4860 *this) {
    s32 step = this->unk24;

    this->unk18 += step;
    this->unk1C += step >> 1;
    if ((u32)this->unk1C > 320) {
        entityDelete(&this->entity);
    }
}

/* The draw method of Unk800C4860: a semi-transparent line from (0x18, 0x20) in grey 0xC0 to (0x1C, 0x20) in black,
 * after a tpage 0x29, two slots after the camera's last drawn one. The match depends on y read once into a local
 * (a word load, where the x reads are narrowed to halfwords). */
void func_800C4940(Unk800C4860 *this, Camera *camera) {
    u_long *ot = camera->ot.lastDrawn + 2;
    DR_TPAGE *tpage = allocDrTpage(0x29);
    LINE_G2 *line = allocLineG2();
    s32 y = this->unk20;

    line->x0 = this->unk18;
    line->y0 = y;
    line->x1 = this->unk1C;
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

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C49E4);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4ADC);

/* Spawns the entity of func_800C3CFC at the position, as a sibling. */
void func_800C4CD8(Unk800C49E4 *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_800C3CFC(entityOperatorNewAt(0x54, child), &this->position, 0x280, 0x403020);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of func_800C62EC at the position (radius 0x60, colour 0x8090A0), as a sibling. */
void func_800C4D40(Unk800C49E4 *this, s32 arg1, s32 count) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), arg1, &this->position, 0x60, 0x20, count, 0x8090A0);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D214);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4DD4);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C4F74);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C52C4);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5480);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5680);

/* The first method of Unk800C5680: counts 0x24 down, calling func_800C5834 until it runs out (it runs out at once
 * when func_8007AED0 of the character is set or its func_800792B8 is 0x13); then counts 0x20 down and deletes the
 * entity below 3. func_800C59D4 every frame it lives. */
void func_800C5778(Unk800C5680 *this, Unk80075EE4 *sum) {
    if (--this->unk24 < 0) {
        if (--this->unk20 < 3) {
            entityDelete(&this->entity);
            return;
        }
    } else {
        func_800C5834(this);
        if (func_8007AED0(this->chara) || func_800792B8(this->chara) == 0x13) {
            this->unk24 = 0;
        }
    }
    func_800C59D4(this);
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5834);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C59D4);

/* Copies up to 4 entries of the table, all but their word at 0x10, and keeps their count. The match depends on
 * the entries indexed and the table walked with a post-increment taken first (its pointer moves at the loop's top),
 * and on the vector read whole before it is stored. */
void func_800C5B18(Unk800C5680 *this, Unk800C5B18Entry *entries, s32 count) {
    s32 i;

    if (count > 4) {
        count = 4;
    }
    this->count = count;
    for (i = 0; i < count; i++) {
        Unk800C5B18Entry *entry = &this->entries[i];
        Unk800C5B18Entry *src = entries++;
        s32 x, y, z;

        entry->unk0 = src->unk0;
        x = src->unk4.vx;
        y = src->unk4.vy;
        z = src->unk4.vz;
        entry->unk4.vx = x;
        entry->unk4.vy = y;
        entry->unk4.vz = z;
        entry->color = src->color;
    }
}

/* Sets the colors of the entries in use, up to count of them. The match depends on colors walked with ++ while the
 * entries are indexed (the colors' pointer then moves first). */
void func_800C5B84(Unk800C5680 *this, CVECTOR *colors, s32 count) {
    s32 n = this->count;
    s32 i;

    if (count < n) {
        n = count;
    }
    for (i = 0; i < n; i++) {
        this->entries[i].color = *colors++;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5BD4);

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5C4C);

/* The entity class of virtual table D_8006D314 (constructor func_800C5DF4): two vectors and three values. */
typedef struct Unk800C5DF4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 unk2C;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Unk800C5DF4;

extern EntityVtable D_8006D314;

/* Constructs the entity of virtual table D_8006D314 with arg1, copies of the two vectors and three values.
 * The match depends on reading each vector's three words before storing any, as vecCopy does. */
Unk800C5DF4 *func_800C5DF4(Unk800C5DF4 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2, s32 arg4, s32 arg5, s32 arg6) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D314;
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
    this->unk3C = arg4;
    this->unk40 = arg5;
    this->unk44 = arg6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C5EB4);

/* Constructs an Unk800C6088 (vtable D_8006D2EC): count particles around pos. */
Unk800C6088 *func_800C6088(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D2EC;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C6134);

/* Constructs an Unk800C6088 (vtable D_8006D2C4): count particles around pos. */
Unk800C6088 *func_800C62EC(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D2C4;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C6398);

/* Constructs an Unk800C6088 (vtable D_8006D29C): count particles around pos. */
Unk800C6088 *func_800C652C(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006D29C;
    this->unk18 = arg1;
    /* The match depends on reading pos into x, y and z first, and on storing radius before them. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->radius = radius;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk30 = arg4;
    this->count = count;
    this->unk38 = arg6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0g", func_800C65D8);

/* Returns the first word of what it is given. Nothing in game.bin calls it. */
s32 func_800C67C0(s32 *words) {
    return words[0];
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C67CC(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs a sprite entity of D_8006D59C on frame with flags, without the motion of func_800C3078. */
Unk800C2F14 *func_800C6820(Unk800C2F14 *this, u16 *frame, s32 flags) {
    entityInit(&this->entity, 0);
    func_800C2CCC(&this->sprite, frame, flags);
    this->entity.vtable = &D_8006D59C;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6880(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D574: func_800C2FBC with the arguments and 0. */
Entity *func_800C68D4(Entity *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    func_800C2FBC((Unk800C2F14 *)this, frame, pos, vel, size, color, 0);
    this->vtable = &D_8006D574;
    return this;
}

/* Constructs an entity of D_8006D574: func_800C2FBC with frame, color 0x808080 and no flags. */
Entity *func_800C691C(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D574;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6970(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D54C: func_800C2FBC with frame + 3, color 0x808080 and no flags. */
Entity *func_800C69C4(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame + 3, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D54C;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6A20(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D524: func_800C2FBC with frame 6 and no flags. */
Unk800C2F14 *func_800C6A74(Unk800C2F14 *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color) {
    func_800C2FBC(this, 6, pos, vel, size, color, 0);
    this->entity.vtable = &D_8006D524;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D4FC, which func_800C32E0
 * installs: game has 2 identical ones. */
void func_800C6AC8(Entity *this, s32 flags) {
    this->vtable = &D_8006D4FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C6B1C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an entity of D_8006D4D4: func_800C2FBC with frame 8, the velocity D_80010814 and no flags. */
Unk800C2F14 *func_800C6B70(Unk800C2F14 *this, VECTOR *pos, s32 size, u32 color) {
    func_800C2FBC(this, 8, pos, &D_80010814, size, color, 0);
    this->entity.vtable = &D_8006D4D4;
    return this;
}

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6BC0(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6C14(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4AC, which func_800C3654
 * installs: game has 3 identical ones. */
void func_800C6C68(Entity *this, s32 flags) {
    this->vtable = &D_8006D4AC;
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

/* The destructor of the entity class of vtable D_8006D3E4, which func_800C4204 installs:
 * its sprite, then the entity. */
void func_800C6D4C(Unk800C4204 *this, s32 flags) {
    spriteDestroy(&this->sprite, DESTROY_BASES);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D3BC, which func_800C4860 installs. */
void func_800C6DA4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D394, which func_800C49E4 installs. */
void func_800C6DEC(Entity *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_8006D33C, which func_800C5680 installs. */
void func_800C6E7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* When 0x24 is above 0, runs func_800C5834 and func_800C59D4. */
void func_800C6EC4(Unk800C5680 *this) {
    if (this->unk24 > 0) {
        func_800C5834(this);
        func_800C59D4(this);
    }
}

/* The destructor of the entity class of vtable D_8006D314, which func_800C5DF4 installs. */
void func_800C6F04(Entity *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_8006D2C4, which func_800C62EC installs. */
void func_800C6F94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D29C, which func_800C652C installs. */
void func_800C6FDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D29C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D2C4);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D2EC);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D314);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D33C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D36C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D394);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D3BC);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D3E4);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D40C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D434);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D45C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D484);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D4AC);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D4D4);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D4FC);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D524);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D54C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D574);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D59C);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D5C4);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D6C8);
INCLUDE_RODATA("game/nonmatchings/game_0g", D_8006D728);
