/* One of the 24 fighters: its constructor, func_8009DBBC, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "gte.h"

struct Unk8009E8F8 *func_8009E8F8(struct Unk8009E8F8 *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos,
                                  VECTOR *vel);
Entity *func_8009F76C(Entity *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, s32 side, s32 arg5);
struct Unk800A0054 *func_800A0054(struct Unk800A0054 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos,
                                  Vec3 *vel, s32 arg5);
struct Unk800A0830 *func_800A0830(struct Unk800A0830 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos,
                                  Vec3 *vel, s32 arg5);
Unk8009F248 *func_8009F248(Unk8009F248 *this, Unk80076F34 *owner);
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Unk80076F34 *owner);
extern VECTOR D_8006A43C;
extern VECTOR D_8006A44C[2];
extern VECTOR D_8006A46C[2];
extern EntityVtable D_8006A48C;
extern EntityVtable D_8006A4B4;
extern EntityVtable D_8006A4DC;
extern EntityVtable D_8006A504;
extern EntityVtable D_8006A52C;
extern EntityVtable D_8006A554;
extern EntityVtable D_8006A5C4;
extern u16 D_8006A6EC[];
extern u16 D_8006A700[];

/* The character class of vtable D_8006A5C4 (func_8009DBBC): func_8009E744 and func_8009E814 clear
 * 0x4A8 and 0x4AC before they launch their shots. */
typedef struct Unk8009DBBC {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
} Unk8009DBBC;

/* Constructs an object of the entity class of vtable D_8006A5C4, which derives from func_80076F34's. */
Unk80076F34 *func_8009DBBC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006A5C4;
    return this;
}

/* The destructor of the entity class of vtable D_8006A5C4, which func_8009DBBC installs. */
void func_8009DBF0(Entity *this, s32 flags) {
    this->vtable = &D_8006A5C4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_8009DC44(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009DC6C);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A3D4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009DE10);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009E050);

void func_8009E364(Unk80076F34 *this);
void func_8009E424(Unk80076F34 *this);
void func_8009E480(Unk80076F34 *this);

/* A virtual function of Unk80076F34: func_8007B9C8; at step 0 sound 0xC3, at step 8 func_8009E424, at step 9
 * func_8009E480 and then func_8009E364. */
void func_8009E244(Unk80076F34 *this, s32 step) {
    func_8007B9C8(this, step);
    if (step == 0) {
        func_800784F4(this, 0xC3);
    }
    if (step == 8) {
        func_8009E424(this);
    }
    if (step == 9) {
        func_8009E480(this);
        func_8009E364(this);
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006A5C4: func_8007BBE8; arg 0 plays
 * sound 0xC3, arg 8 calls func_8009E424, arg 9 func_8009E480 and func_8009E364, and below 10 the position
 * at 0x3B4 moves back by a sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_8009E2B8(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xC3);
    }
    if (arg == 8) {
        func_8009E424(this);
    }
    if (arg == 9) {
        func_8009E480(this);
        func_8009E364(this);
    }
    if (arg < 10) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A43C);

/* Creates an entity of func_8009E8F8's class (0xE4 bytes) at the owner's point 4 moved by D_8006A43C, moving
 * 192 a frame toward func_80079E10's side, hands it to func_8007AD4C and plays sound 0xBE. */
void func_8009E364(Unk80076F34 *this) {
    VECTOR pos;
    VECTOR vel;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(this, &pos, 4, &D_8006A43C);
    vel.vx = func_80079E10(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xE4);
    if (child != NULL) {
        func_8009E8F8(entityOperatorNewAt(0xE4, child), this, obj, &pos, &vel);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xBE);
}

/* Creates an entity of func_8009F248's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8009E424(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F248(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_8009F4D4's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8009E480(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F4D4(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009E4DC);

/* Creates an entity of func_8009F76C's class (0xC0 bytes) 0x180 in front of the character and 0x133
 * up, for func_80079E10's side and arg; hands it to func_8007AD4C and plays sound 0xBF. */
void func_8009E5EC(Unk80076F34 *this, s32 arg) {
    s32 side;
    Vec3 pos;
    Unk8006ED20 *obj;
    Entity *child;

    side = func_80079E10(this);
    func_800775D8(this, &pos);
    pos.vx += side * 0x180;
    pos.vy -= 0x133;
    obj = func_80077580(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_8009F76C(entityOperatorNewAt(0xC0, child), this, obj, &pos, side, arg);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xBF);
}

void func_8009E744(Unk8009DBBC *this);
void func_8009E814(Unk8009DBBC *this);

/* Overrides func_8007BA30: calls it; arg 0 sets 0x498 to 12, func_80077448(-0x10000) and plays sound 0xC5;
 * arg 18 launches func_8009E744's shot and arg 38 func_8009E814's. */
void func_8009E6C4(Unk8009DBBC *this, s32 arg) {
    func_8007BA30(&this->base, arg);
    if (arg == 0) {
        this->base.unk498 = 12;
        func_80077448(&this->base, -0x10000);
        func_800784F4(&this->base, 0xC5);
    }
    if (arg == 18) {
        func_8009E744(this);
    }
    if (arg == 38) {
        func_8009E814(this);
    }
}

/* Clears 0x4A8 and creates an entity of func_800A0054's class (0xD8 bytes) 0x120 in front of the
 * character, moving 128 a frame toward func_80079E10's side; hands it to func_8007AD4C and plays
 * sound 0xC0. */
void func_8009E744(Unk8009DBBC *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Unk8006ED20 *obj;
    Entity *child;

    this->unk4A8 = 0;
    side = func_80079E10(&this->base);
    func_800775D8(&this->base, &pos);
    vel.vx = side * 128;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x120;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0xD8);
    if (child != NULL) {
        func_800A0054(entityOperatorNewAt(0xD8, child), &this->base, obj, &pos, &vel, 0);
    }
    func_8007AD4C(&this->base, child);
    func_80078540(&this->base, 0xC0);
}

/* As func_8009E744 with 0x4AC, func_800A0830's class (0xD4 bytes), 0x180 in front and 0x180 up,
 * moving 160 a frame. */
void func_8009E814(Unk8009DBBC *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Unk8006ED20 *obj;
    Entity *child;

    this->unk4AC = 0;
    side = func_80079E10(&this->base);
    func_800775D8(&this->base, &pos);
    vel.vx = side * 160;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x180;
    pos.vy -= 0x180;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        func_800A0830(entityOperatorNewAt(0xD4, child), &this->base, obj, &pos, &vel, 0);
    }
    func_8007AD4C(&this->base, child);
    func_80078540(&this->base, 0xC0);
}

extern EntityVtable D_8006A59C;
extern VtableEntry D_8006A57C[];

/* The Unk800C7024 class of virtual tables D_8006A59C and D_8006A57C (constructor func_8009E8F8, size 0xE4):
 * a shot that keeps its owner, the owner's Unk8006ED20 and where it started. */
typedef struct Unk8009E8F8 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ Unk80076F34 *owner;
    /* 0xC4 */ Unk8006ED20 *obj;
    /* 0xC8 */ Vec3 start;
    /* 0xD4 */ s32 unkD4;
    /* 0xD8 */ s32 unkD8;
    /* 0xDC */ s32 unkDC; /* set by func_8009EB20 */
    /* 0xE0 */ s32 unkE0;
} Unk8009E8F8;

/* Constructs the shot of virtual table D_8006A59C for owner at pos, moving by vel. */
Unk8009E8F8 *func_8009E8F8(Unk8009E8F8 *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos, VECTOR *vel) {
    s32 x, y, z;

    func_800C7024(&this->base, owner, (Vec3 *)pos, (Vec3 *)vel, 0x80, 0x6A82400, 0x1000, 3);
    this->base.parts.base.vtable = D_8006A57C;
    this->base.parts.entity.vtable = &D_8006A59C;
    this->owner = owner;
    this->obj = obj;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->unkE0 = 0x80;
    this->unkD8 = 0;
    this->unkDC = 0;
    this->start.vx = x;
    this->start.vy = y;
    this->start.vz = z;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009E9BC);

/* The second base's first method of func_8009E8F8's shot: func_8007BDF0 with sum, then sets 0xDC and bit 0
 * of its part's flags and calls func_800C73A8(4). */
void func_8009EB20(Unk8009E8F8 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->base.parts.base, sum);
    this->unkDC = 1;
    this->base.parts.part.flags |= 1;
    func_800C73A8(&this->base, 4);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009EB68);

Entity *func_8009F014(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel);

/* Spawns func_8009F014's entity (0x58 bytes) at the shot's position as a sibling, moving back at a 16th of
 * the shot's x speed and up by 13 to 20 (a random of the owner's Unk8006ED20). */
void func_8009EF64(Unk8009E8F8 *this) {
    VECTOR velocity;
    Entity *child;

    /* the game copies all four words (0x9C too) before it overwrites three of them */
    velocity.vx = this->base.vel.vx;
    velocity.vy = this->base.vel.vy;
    velocity.vz = this->base.vel.vz;
    velocity.pad = this->base.unk9C;
    velocity.vy = (mersenneTwisterGenerate(&this->obj->random) & 7) - 20;
    velocity.vz = 0;
    velocity.vx = -this->base.vel.vx >> 4;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_8009F014(entityOperatorNewAt(0x58, child), (s32)this->owner, (VECTOR *)&this->base.pos, &velocity);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* Constructs an entity of D_8006A554: func_800C2FBC with entry 9 of D_8006D5C4, 0x19 and 0x802020. */
Entity *func_8009F014(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    func_800C2FBC((Unk800C2F14 *)this, 9, pos, vel, 0x19, 0x802020, 0);
    this->vtable = &D_8006A554;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F064);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F120);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A44C);

/* Constructs an Unk8009F248 (vtable D_8006A52C) of owner with the sprite frame D_8006A6EC, at the owner's point 4
 * moved by D_8006A44C[1] and moving by D_8006A44C[0]. */
Unk8009F248 *func_8009F248(Unk8009F248 *this, Unk80076F34 *owner) {
    s32 flags = func_8007768C(owner);
    /* The match depends on frame, read into a local before entityInit. */
    u16 *frame = D_8006A6EC;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    func_800C2CCC(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A52C;
    this->owner = owner;
    func_8007A4A4(owner, &pos, 4, &D_8006A44C[1]);
    func_800C3078(&this->base, &pos, &D_8006A44C[0], 0x133, 0x808080);
    this->base.sprite.size = 0x133;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F30C);

/* Entry 0x14 of vtable D_8006A52C: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (func_80079E10), one OT slot further, with buildPolyFT4. */
void func_8009F394(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = func_80079E10(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.pos;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    func_800C3100(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A46C);

/* Constructs an Unk8009F248 (vtable D_8006A504) of owner with the sprite frame D_8006A700, at the owner's point 4
 * moved by D_8006A46C[1] and moving by D_8006A46C[0]. */
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Unk80076F34 *owner) {
    s32 flags = func_8007768C(owner);
    /* The match depends on frame, read into a local before entityInit. */
    u16 *frame = D_8006A700;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    func_800C2CCC(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A504;
    this->owner = owner;
    func_8007A4A4(owner, &pos, 4, &D_8006A46C[1]);
    func_800C3078(&this->base, &pos, &D_8006A46C[0], 0x80, 0x808080);
    this->base.sprite.size = 0x33;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F598);

/* Entry 0x14 of vtable D_8006A504: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (func_80079E10), one OT slot further, with buildPolyFT4. */
void func_8009F62C(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = func_80079E10(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.pos;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    func_800C3100(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F76C);

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A4DC, which func_8009F76C installs. */
void func_8009F8E0(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A4DC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009F974);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_8009FC80);

/* The class func_800A0054 constructs (0xD8 bytes, virtual table D_8006A4B4): parts for an owner at pos,
 * moving at vel, with two random words. */
typedef struct Unk800A0054 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *owner;
    /* 0x88 */ Unk8006ED20 *obj; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x8C */ Vec3 pos;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 start;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ Vec3 vel;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0; /* random, 0 to 3 */
    /* 0xC4 */ s32 unkC4; /* random, 0 to 0x7F */
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ s16 vramY; /* the owner's 0x440 (func_8007768C) */
} Unk800A0054;

/* Constructs Unk800A0054 for owner at pos, moving at vel, with arg5, and adds its part to one of obj's two lists
 * (func_80077574 picks which). */
Unk800A0054 *func_800A0054(Unk800A0054 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, Vec3 *vel,
                           s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    this->parts.entity.vtable = &D_8006A4B4;
    func_8007BF20(&this->parts.unk18, owner, 0x6B07000, vel, 3);
    func_80075EE4(&this->parts.part, pos, 0x40, &this->parts.unk18);
    v.vx = func_80079E10(owner) * 1820;
    v.vy = 0;
    v.vz = 0;
    func_8007BDD4(&this->parts.unk18.base, &v);
    this->parts.part.unk14 = &this->parts.entity;
    this->owner = owner;
    this->obj = obj;
    this->parts.part.flags |= 1;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkBC = arg5;
    this->unkC0 = mersenneTwisterGenerate(&obj->random) & 3;
    this->unkC4 = mersenneTwisterGenerate(&obj->random) & 0x7F;
    this->unkC8 = 300;
    this->unkCC = 0;
    this->unkD0 = 0;
    this->vramY = func_8007768C(owner);
    if (func_80077574(owner) == 0) {
        func_8006F044(obj, &this->parts.part.node);
    } else {
        func_8006F024(obj, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A4B4, which func_800A0054 installs. */
void func_800A0214(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A4B4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A02A8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A04BC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A0680);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A0774);

/* The class func_800A0830 constructs (0xD4 bytes, virtual table D_8006A48C): parts for an owner at pos,
 * moving at vel, with the owner's facing. */
typedef struct Unk800A0830 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *owner;
    /* 0x88 */ Unk8006ED20 *obj; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x8C */ Vec3 pos;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 start;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ Vec3 vel;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 facing; /* the owner's (func_80079E10) */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s16 vramY; /* the owner's 0x440 (func_8007768C) */
} Unk800A0830;

/* Constructs Unk800A0830 for owner at pos, moving at vel, with arg5 (non-zero also sets bit 0 of the part's
 * flags), and adds its part to one of obj's two lists (func_80077574 picks which). */
Unk800A0830 *func_800A0830(Unk800A0830 *this, Unk80076F34 *owner, Unk8006ED20 *obj, Vec3 *pos, Vec3 *vel,
                           s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    this->parts.entity.vtable = &D_8006A48C;
    func_8007BF20(&this->parts.unk18, owner, 0x45B07000, (Vec3 *)&D_80010814, 3);
    func_80075EE4(&this->parts.part, pos, 0x200, &this->parts.unk18);
    v.vx = func_80079E10(owner) * 0x5550;
    v.vy = -0x5550;
    v.vz = 0;
    func_8007BDD4(&this->parts.unk18.base, &v);
    this->parts.part.unk14 = &this->parts.entity;
    if (arg5 != 0) {
        this->parts.part.flags |= 1;
    }
    this->owner = owner;
    this->obj = obj;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkBC = arg5;
    this->facing = func_80079E10(owner);
    this->vramY = func_8007768C(owner);
    this->unkC4 = 0;
    this->unkC8 = 0x280;
    this->unkCC = 0;
    if (func_80077574(owner) == 0) {
        func_8006F044(obj, &this->parts.part.node);
    } else {
        func_8006F024(obj, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A48C, which func_800A0830 installs. */
void func_800A09F0(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A48C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A0A84);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A0BD0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_10", func_800A0D94);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800A0E58(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0EF8(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0F4C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800A0FA0(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A48C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A4B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A4DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A504);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A52C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A554);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A57C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A59C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A5C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A6EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_10", D_8006A700);
