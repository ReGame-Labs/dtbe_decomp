/* One of the 24 fighters: its constructor, func_800827EC, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

struct Unk80083700 *func_80083700(struct Unk80083700 *this, Unk80076F34 *chara);
struct Unk800833F8 *func_800833F8(struct Unk800833F8 *this, Vec3 *position, s32 arg2, s32 arg3, s32 vramY);
extern Unk80079B24Entry D_800681DC[];
extern EntityVtable D_8006843C;

/* The class of func_80083108, derived from Unk800C7024 */
typedef struct Unk80083108 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ s32 unkC0;
} Unk80083108;

extern EntityVtable D_800683EC;
extern VtableEntry D_800683CC[];
void func_80082AF8(Unk80076F34 *this);

/* Constructs an entity of the class of vtable D_8006843C, which derives from Unk80076F34 and
 * gives it the table D_800681DC. */
Unk80076F34 *func_800827EC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006843C;
    func_80079B24(this, D_800681DC);
    return this;
}

/* The override in virtual table D_8006843C: func_80079B2C with 0. */
void func_80082830(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

/* A virtual function of Unk80076F34: func_8007B978; before step 5, func_8007835C and copies unk3A4 and unk3B4
 * to the Unk80076F34 of func_8007AEEC; at step 0, func_80079274 and func_800796F0 on it; at step 5, launches it
 * up and toward func_80079E10's side with newLaunch. */
void func_80082850(Unk80076F34 *this, s32 step) {
    Unk80076F34 *other;
    Vec3 vel;
    s32 side;
    s32 x, y, z;

    func_8007B978(this, step);
    other = func_8007AEEC(this);
    if (step < 5) {
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
        func_80079274(other, 5);
        func_800796F0(other, 0, 0, 0);
        break;
    case 5:
        side = func_80079E10(this);
        vel.vx = side * 5460;
        vel.vy = -21840;
        vel.vz = 0;
        newLaunch(this, &vel, side);
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006843C: func_8007B9C8, sets bit 6 of
 * 0x37C, then for arg 0 plays sound 0x77, and for arg 13 plays sound 0x73 and calls func_80082AF8. */
void func_800829A0(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x77);
        func_80079B2C(this, 1);
        break;
    case 13:
        func_80078540(this, 0x73);
        func_80079B2C(this, 0);
        func_80082AF8(this);
        break;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006843C: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 0 plays sound 0x77, arg 7 plays sound 0x73 and calls func_80082AF8; below 8 the vector at
 * 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_80082A30(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0x77);
        func_80079B2C(this, 1);
        break;
    case 7:
        func_80078540(this, 0x73);
        func_80079B2C(this, 0);
        func_80082AF8(this);
        break;
    }
    if (arg < 8) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80082AF8);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_8006818C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_80068194);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_8006819C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800681DC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80082C10);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_80068334);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80082DBC);

/* Creates an entity of func_80083700's class (0x4C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_80082E8C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x4C);

    if (child != NULL) {
        func_80083700(entityOperatorNewAt(0x4C, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80082EE8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80082FF0);

/* Constructs func_800C7024's entity with 0x80, 0x5A81C00, 0xA00, 3, then the virtual tables
 * D_800683EC (and D_800683CC for the base at 0x18); clears 0xC0. */
Unk80083108 *func_80083108(Unk80083108 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C7024(&this->base, chara, position, velocity, 0x80, 0x5A81C00, 0xA00, 3);
    this->base.parts.base.vtable = D_800683CC;
    this->base.parts.entity.vtable = &D_800683EC;
    this->unkC0 = 0;
    return this;
}

/* The tick of D_800683EC's class: while bit 0 of 0xB8 is set, steps 0xC0 through 1 to 4, then runs the
 * base class's tick (func_800C71DC). */
void func_80083170(Unk80083108 *this, s32 arg) {
    if (this->base.unkB8 & 1) {
        if (++this->unkC0 >= 5) {
            this->unkC0 = 1;
        }
    }
    func_800C71DC(&this->base, arg);
}

/* Spawns func_800833F8's entity (0x38 bytes) at the position, as a sibling, then runs the base
 * class's func_800C72D8. */
void func_800831C4(Unk80083108 *this, Unk80075EE4 *sum) {
    Entity *child = allocEntityMemory(0x38);

    if (child != NULL) {
        func_800833F8(entityOperatorNewAt(0x38, child), &this->base.pos, 0x180, 0xC0, this->base.unkBC);
    }
    entityAddSibling(&this->base.parts.entity, child);
    func_800C72D8(&this->base, sum);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80083248);

/* The entity class of virtual table D_800683A4 (constructor func_800833F8). */
typedef struct Unk800833F8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 vramY;
} Unk800833F8;

extern EntityVtable D_800683A4;

/* Constructs the entity of virtual table D_800683A4 at position, with arg3 at 0x2C, arg2 at 0x30 and
 * vramY. The match depends on reading the three words before storing any, as vecCopy does. */
Unk800833F8 *func_800833F8(Unk800833F8 *this, Vec3 *position, s32 arg2, s32 arg3, s32 vramY) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800683A4;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = arg3;
    this->unk30 = arg2;
    this->vramY = vramY;
    return this;
}

/* The update of D_800683A4's entity: 0x28 grows by 16 a frame, and the entity deletes itself once it reaches
 * 0x2C. */
void func_80083488(Unk800833F8 *this) {
    this->unk28 += 16;
    if (this->unk28 >= this->unk2C) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_800834E4);

/* The entity class of virtual table D_8006837C (constructor func_80083700). */
typedef struct Unk80083700 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ VECTOR position; /* the character's node 2 (func_8007A4A4) */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48; /* func_8007768C of the character */
} Unk80083700;

extern EntityVtable D_8006837C;

/* Constructs the entity of virtual table D_8006837C for chara, at its node 2. */
Unk80083700 *func_80083700(Unk80083700 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006837C;
    this->chara = chara;
    func_8007A4A4(chara, &this->position, 2, &D_80010814);
    this->unk2C = 0x80;
    this->unk30 = 0x180;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = 0;
    this->unk48 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80083790);

void func_8008391C(Unk80083700 *this, s32 arg);
void func_80083B90(Unk80083700 *this, s32 arg);

/* The update of D_8006837C's entity: in state 0 (0x40) runs func_8008391C, in state 1 func_8008391C and
 * then func_80083B90. */
void func_800838A8(Unk80083700 *this, s32 arg) {
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_8008391C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_00", func_80083B90);

/* The destructor of the entity class of vtable D_8006843C, which func_800827EC installs. */
void func_80083D4C(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_80068414, which func_80082EE8 installs. */
void func_80083D94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_80083DDC(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_800683A4, which func_800833F8 installs. */
void func_80083E7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006837C, which func_80083700 installs. */
void func_80083EC4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_8006837C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800683A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800683CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_800683EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_80068414);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_00", D_8006843C);
