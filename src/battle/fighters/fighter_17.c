/* One of the 24 fighters: its constructor, func_800B45C0, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

struct Unk800B5920 *func_800B5920(struct Unk800B5920 *this, Unk80076F34 *chara, Unk8006ED20 *obj, VECTOR *position);
Entity *func_800B50EC(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *obj);
extern Unk80079B24Entry D_8006BE1C[];
extern EntityVtable D_8006BE4C;
extern EntityVtable D_8006BE74;
extern EntityVtable D_8006BEE4;

/* The class func_800B50EC constructs (virtual tables D_8006BEBC, and D_8006BE9C for its base at
 * 0x18). */
typedef struct Unk800B50EC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0xC8 - 0x84];
    /* 0x0C8 */ Vec3 unkC8;
    /* 0x0D4 */ u8 unkD4[0x118 - 0xD4];
    /* 0x118 */ Vec3 position;
    /* 0x124 */ u8 unk124[0x128 - 0x124];
    /* 0x128 */ Vec3 velocity;
    /* 0x134 */ u8 unk134[0x158 - 0x134];
    /* 0x158 */ s32 speed; /* the length of velocity */
    /* 0x15C */ u32 unk15C; /* a sound's handle: func_80078540's (sound 0xF2), stopSound */
} Unk800B50EC;
extern VtableEntry D_8006BE9C[];
extern EntityVtable D_8006BEBC;
extern VECTOR D_800D345C; /* {0, -70, 40, 0x1000} */

/* Constructs an entity of the class of vtable D_8006BEE4, which derives from Unk80076F34 and
 * gives it the table D_8006BE1C. */
Unk800B45C0 *func_800B45C0(Unk800B45C0 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006BEE4;
    func_80079B24(&this->base, D_8006BE1C);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006BEE4, which func_800B45C0 installs. */
void func_800B4608(Entity *this, s32 flags) {
    this->vtable = &D_8006BEE4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The method at 0x20 of virtual table D_8006BEE4: clears 0x4A4. */
void func_800B465C(Unk800B45C0 *this) {
    this->unk4A4 = 0;
}

/* Overrides func_8007B884: calls it, then func_8007A6FC(1, 80, 8) when arg is 0. */
void func_800B4664(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 0) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B46AC);

void func_800B4FA4(Unk80076F34 *this);
void func_800B5060(Unk80076F34 *this);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006BEE4: arg 0 starts clip 0x16, creates
 * func_800B5920's entity (func_800B5060) and plays sound 0xF5; arg 5 creates func_800B50EC's entity
 * (func_800B4FA4) and calls func_80079B0C with 0x19. */
void func_800B4898(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_800B5060(this);
        func_800784F4(this, 0xF5);
        break;
    case 5:
        func_800B4FA4(this);
        func_80079B0C(this, 0x19);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B491C);

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006BEE4: arg 0 starts clip 0x17,
 * creates func_800B5920's entity (func_800B5060) and plays sound 0xF5; arg 5 pushes x at 0x3B4 back by 0x3000,
 * creates func_800B50EC's entity (func_800B4FA4) and calls func_80079B0C with 0x19; below 14 the vector at
 * 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_800B4E28(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x17, 2, 0, -1);
        func_800B5060(this);
        func_800784F4(this, 0xF5);
        break;
    case 5:
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
        func_800B4FA4(this);
        func_80079B0C(this, 0x19);
        break;
    }
    if (arg < 14) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

struct Unk800B5F14 *func_800B5F14(struct Unk800B5F14 *this, Unk80076F34 *chara, Vec3 *position);

/* Creates func_800B5F14's entity (0x34 bytes) at the character's position moved 0xF5 back from the side it
 * faces (func_80079E10), and hands it to func_8007AD4C. */
void func_800B4F10(Unk80076F34 *this) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    if (func_80079E10(this) > 0) {
        position.vx -= 0xF5;
    } else {
        position.vx += 0xF5;
    }
    child = allocEntityMemory(0x34);
    if (child != NULL) {
        func_800B5F14(entityOperatorNewAt(0x34, child), this, &position);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800B50EC's entity (0x160 bytes) 0xB0 above the character, moving 48 a frame toward
 * func_80079E10's side; hands it to func_8007AD4C and calls func_80079B0C(40). */
void func_800B4FA4(Unk80076F34 *this) {
    Unk8006ED20 *obj;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    obj = func_80077580(this);
    func_800775D8(this, &position);
    position.vy -= 0xB0;
    velocity.vx = func_80079E10(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        func_800B50EC(entityOperatorNewAt(0x160, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
    func_80079B0C(this, 40);
}

/* Creates func_800B5920's entity (0x74 bytes) at the character's point 9 moved by D_800D345C, and
 * hands it to func_8007AD4C. */
void func_800B5060(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR position;
    Entity *child;

    func_8007A4A4(this, &position, 9, &D_800D345C);
    child = allocEntityMemory(0x74);
    if (child != NULL) {
        func_800B5920(entityOperatorNewAt(0x74, child), this, obj, &position);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B50EC);

/* The destructor of func_800B50EC's class: stops its sound if it plays, takes its member at 0x38 out of
 * its list. */
void func_800B5320(Unk800B50EC *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006BE9C;
    this->entity.vtable = &D_8006BEBC;
    if (this->unk15C != 0) {
        stopSound(this->unk15C);
    }
    this->unk38.vtable = D_80065184;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.node;
    this->unk38.node.next->prev = this->unk38.node.prev;
    link->prev->next = this->unk38.node.next;
    this->unk38.node.next = link;
    this->unk38.node.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B53D0);

/* func_80078DE0 of a target, after taking its position (func_800775D8), which is not used. */
static inline s32 func_800B55EC_targetTest(Unk80076F34 *target) {
    Vec3 position;

    func_800775D8(target, &position);
    return func_80078DE0(target);
}

/* The base at 0x18's first method for func_800B50EC's class: func_8007BDF0 with sum; when func_80078878 finds a
 * target in sum for which func_80078DE0 holds, func_800C75FC's entity on it, else a burst of 6 (func_800C62EC) at
 * 0xC8; then the entity deletes itself. */
void func_800B55EC(Unk800B50EC *this, Unk80075EE4 *sum) {
    Entity *target;
    Entity *child;

    func_8007BDF0(&this->unk18, sum);
    target = func_80078878(sum);
    /* The match depends on the target test as an inline beside the null test (an if/else, one delete). */
    if (target != NULL && func_800B55EC_targetTest((Unk80076F34 *)target)) {
        entityAddSibling(&this->entity, func_800C75FC(target, &this->velocity, 0x60));
    } else {
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unkC8, this->speed >> 1, 0x60, 6, 0x808080);
        }
        entityAddSibling(&this->entity, child);
    }
    entityDelete(&this->entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B56EC);

/* The entity class of vtable D_8006BE74 (0x74 bytes), made for a character at a position. */
typedef struct Unk800B5920 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *obj;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 vramY;  /* the character's 0x440 (func_8007768C) */
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ VECTOR position;
    /* 0x44 */ VECTOR start; /* the position it was made at */
    /* 0x54 */ SVECTOR corners[4];
} Unk800B5920;

/* Constructs the entity of vtable D_8006BE74 for a character and obj at a position, with the corners of a
 * 180 by 180 square 16 in front. */
Unk800B5920 *func_800B5920(Unk800B5920 *this, Unk80076F34 *chara, Unk8006ED20 *obj, VECTOR *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BE74;
    this->chara = chara;
    this->obj = obj;
    this->facing = func_80079E10(chara);
    this->unk24 = 0;
    this->unk28 = 0;
    this->unk2A = 0;
    this->unk30 = 0;
    this->unk2C = 0;
    this->unk20 = 0x80;
    this->unk21 = 0x80;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->start.vx = x;
    this->start.vy = y;
    this->start.vz = z;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->vramY = func_8007768C(chara);
    /* The match depends on these chains: they store from right to left */
    this->corners[0].vy = this->corners[1].vy = this->corners[0].vx = this->corners[2].vx = -90;
    this->corners[2].vy = this->corners[3].vy = this->corners[1].vx = this->corners[3].vx = 90;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -16;
    return this;
}

/* The destructor of the entity class of vtable D_8006BE74, which func_800B5920 installs. */
void func_800B5A1C(Entity *this, s32 flags) {
    this->vtable = &D_8006BE74;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B5A70);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B5C90);

/* The entity class of vtable D_8006BE4C (0x34 bytes), made for a character at a position. */
typedef struct Unk800B5F14 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s16 unk18;
    /* 0x1A */ s16 vramY;  /* the character's 0x440 (func_8007768C) */
    /* 0x1C */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x1E */ u8 unk1E;   /* 0x1E-0x20: 0x80 each, maybe a color */
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21[3];
    /* 0x24 */ Vec3 position;
    /* 0x30 */ s32 unk30;
} Unk800B5F14;

/* Constructs the entity of vtable D_8006BE4C for a character at a position. */
Unk800B5F14 *func_800B5F14(Unk800B5F14 *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BE4C;
    this->vramY = func_8007768C(chara);
    this->facing = func_80079E10(chara);
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unk18 = 0;
    /* The match depends on this chain: it stores from right to left */
    this->unk1E = this->unk1F = this->unk20 = 0x80;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BE4C, which func_800B5F14 installs. */
void func_800B5FA8(Entity *this, s32 flags) {
    this->vtable = &D_8006BE4C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B5FFC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_17", func_800B6130);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDCC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDD4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDDC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDE4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDEC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDF4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BDFC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE0C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE4C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE74);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BE9C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BEBC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_17", D_8006BEE4);
