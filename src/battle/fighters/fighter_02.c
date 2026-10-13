/* One of the 24 fighters: its constructor, func_800870F0, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

extern Unk80079B24Entry D_8006894C[];
extern EntityVtable D_800689F4;

/* The entity class func_8008787C constructs (virtual table D_80068984) for a
 * character. */
typedef struct Unk8008787C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
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
    /* 0x50 */ s32 unk50; /* the character's 0x440 (func_8007768C) */
} Unk8008787C;

Unk8008787C *func_8008787C(Unk8008787C *this, Unk80076F34 *chara);

extern EntityVtable D_80068984;
extern EntityVtable D_800689CC;
extern VtableEntry D_800689AC[];
void func_80087400(Unk80076F34 *this);
Unk800C7024 *func_80087690(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);

/* Constructs an entity of the class of vtable D_800689F4, which derives from Unk80076F34 and
 * gives it the table D_8006894C. */
Unk800870F0 *func_800870F0(Unk800870F0 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_800689F4;
    func_80079B24(&this->base, D_8006894C);
    this->unk4A4 = 0;
    return this;
}

/* The override in virtual table D_800689F4: func_80079B2C with 0. */
void func_80087138(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

/* A virtual function of Unk80076F34: func_8007B978; before step 16, func_8007835C and copies unk3A4 and unk3B4
 * to the Unk80076F34 of func_8007AEEC; at step 0, func_80079274 and func_800796F0 on it; at step 16, launches it
 * up and away from func_80079E10's side with newLaunch. */
void func_80087158(Unk80076F34 *this, s32 step) {
    Unk80076F34 *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    func_8007B978(this, step);
    other = func_8007AEEC(this);
    if (step < 16) {
        func_8007835C(this, 0);
        /* The match depends on each vector read into x, y and z before it is stored. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        other->unk3A4.vx = x;
        other->unk3A4.vy = y;
        other->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        other->unk3B4.vx = x;
        other->unk3B4.vy = y;
        other->unk3B4.vz = z;
    }
    switch (step) {
    case 0:
        func_80079274(other, 16);
        func_800796F0(other, 0, 0, 0);
        break;
    case 16:
        side = -func_80079E10(this);
        vel.vx = side * 10920;
        vel.vy = -18200;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_800689F4: func_8007B9C8, sets bit 6 of
 * 0x37C, then for arg 0 plays sound 0x85, and for arg 14 plays sound 0x82 and calls func_80087400. */
void func_800872A8(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x85);
        func_80079B2C(this, 1);
        break;
    case 14:
        func_80078540(this, 0x82);
        func_80079B2C(this, 0);
        func_80087400(this);
        break;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_800689F4: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 0 plays sound 0x85, arg 14 plays sound 0x82 and calls func_80087400; below 15 the vector at
 * 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_80087338(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x85);
        func_80079B2C(this, 1);
        break;
    case 14:
        func_80078540(this, 0x82);
        func_80079B2C(this, 0);
        func_80087400(this);
        break;
    }
    if (arg < 15) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Spawns the entity of func_80087690 0xC0 above the character (func_800775D8), with velocity
 * (96 forward, 0, 0). */
void func_80087400(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0xC0;
    velocity.vx = func_80079E10(this) * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_80087690(entityOperatorNewAt(0xC0, child), this, &position, &velocity);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_80087498);

/* Creates an entity of func_8008787C's class (0x54 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_80087634(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_8008787C(entityOperatorNewAt(0x54, child), this);
    }
    func_8007AD4C(this, child);
}

/* Constructs func_800C7024's entity with 0x60, 0x6A81E00, 0x900, 3, then the virtual tables
 * D_800689CC (and D_800689AC for the base at 0x18). */
Unk800C7024 *func_80087690(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C7024(this, chara, position, velocity, 0x60, 0x6A81E00, 0x900, 3);
    this->parts.base.vtable = D_800689AC;
    this->parts.entity.vtable = &D_800689CC;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_800876F4);

/* Constructs an Unk8008787C for the character: 0x1C, 0x24 and 0x2C are 0x1000, 0x44 is 0x80. */
Unk8008787C *func_8008787C(Unk8008787C *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068984;
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
    this->unk50 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_8008790C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_80087C5C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_80087E14);

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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_02", func_80087F3C);

/* The destructor of the entity class of vtable D_800689F4, which func_800870F0 installs. */
void func_80088094(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800880DC(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_80068984, which func_8008787C installs. */
void func_8008817C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006895C, which func_80087E14 installs. */
void func_800881C4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688D4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688E4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800688FC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_80068904);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_8006890C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_8006892C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_8006894C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_8006895C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_80068984);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800689AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800689CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_02", D_800689F4);
