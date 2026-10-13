/* One of the 24 fighters: its constructor, func_800A2F98, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"

struct Unk800A47F4 *func_800A47F4(struct Unk800A47F4 *this, Unk80076F34 *arg);
extern VtableEntry D_8006AA1C[];
extern EntityVtable D_8006AA3C;

/* The class of func_800A4550 and func_800A45C8, which spawn entities at its
 * two positions. */
typedef struct Unk800A4550 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x84 - 0x18];
    /* 0x84 */ Vec3 unk84;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ Vec3 unkA4;
} Unk800A4550;

/* The character class of virtual table D_8006AA64 (constructor func_800A2F98). */
typedef struct Unk800A2F98 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
} Unk800A2F98;

extern Unk80079B24Entry D_8006A974[];
extern EntityVtable D_8006AA64;

/* Constructs a character of the class of vtable D_8006AA64 (which derives from Unk80076F34) with the
 * table D_8006A974. */
Unk800A2F98 *func_800A2F98(Unk800A2F98 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006AA64;
    this->unk4AC = 0;
    func_80079B24(&this->base, D_8006A974);
    this->unk4A4 = 0;
    this->unk4A8 = 0;
    this->unk4B0 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A2FEC);

/* The override in virtual table D_8006AA64: func_80079B2C with 0. */
void func_800A313C(Unk80076F34 *this) {
    func_80079B2C(this, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A315C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A3304);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A34BC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A35C8);

s32 func_800A3754(Unk80076F34 *this);
void func_800A37F4(Unk80076F34 *this);

/* func_800A3754 when bit 0 of the character's flags at 0x37C is clear, else func_800A37F4. */
void func_800A3710(Unk80076F34 *this) {
    if (func_80077598(this) == 0) {
        func_800A3754(this);
    } else {
        func_800A37F4(this);
    }
}

/* A value from the character's input (func_800750C4): up gives 0, or -0x200 unless func_800792DC is 30; down
 * gives 0x200; holding the way it faces (func_80079E10) gives 0, anything else -1. */
s32 func_800A3754(Unk80076F34 *this) {
    u32 input = func_800750C4(func_800776C4(this));
    s32 facing;

    if (input & PAD_UP) {
        return func_800792DC(this) == 30 ? 0 : -0x200;
    }
    if (input & PAD_DOWN) {
        return 0x200;
    }
    facing = func_80079E10(this);
    if (facing > 0 && (input & PAD_RIGHT)) {
        return 0;
    }
    if (facing >= 0) {
        return -1;
    }
    if (input & PAD_LEFT) {
        return 0;
    }
    return -1;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A37F4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A38D8);

/* Creates an entity of func_800A47F4's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800A3A40(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800A47F4(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A3A9C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A3C98);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A3E30);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A3FB0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A444C);

/* Spawns the entity of func_800C3654 between 0x84 and 0xA4, as a sibling. */
void func_800A4550(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_800C3654(entityOperatorNewAt(0x5C, child), &this->unk84, &this->unkA4, 0x10, 0x408080, 9);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of func_800C62EC at 0x84, as a sibling. */
void func_800A45C8(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 9, &this->unk84, 0x20, 0x10, 8, 0x408080);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4648);

extern EntityVtable D_8006A9F4;

/* The entity class of virtual table D_8006A9F4 (constructor func_800A47F4, size 0x44). */
typedef struct Unk800A47F4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position; /* the character's at construction (func_800775D8) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 facing; /* func_80079E10: 1 or -1 */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40; /* the character's 0x440 (func_8007768C) */
} Unk800A47F4;

/* Constructs the entity of virtual table D_8006A9F4 for chara, at its position and facing its way. */
Unk800A47F4 *func_800A47F4(Unk800A47F4 *this, Unk80076F34 *chara) {
    Vec3 position;
    s32 facing;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A9F4;
    facing = func_80079E10(chara);
    func_800775D8(chara, &position);
    this->chara = chara;
    this->unk2C = 0;
    this->unk30 = 0x80;
    this->facing = facing;
    this->unk38 = 0;
    this->unk3C = 0;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    this->unk40 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4898);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4A08);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4C04);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4D10);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4E1C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A4F64);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A5064);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_12", func_800A51DC);

/* The destructor of the entity class of vtable D_8006AA64, which func_800A2F98 installs. */
void func_800A5308(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AA3C and D_8006AA1C, which
 * func_800A3E30 installs. */
void func_800A5350(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AA1C;
    this->entity.vtable = &D_8006AA3C;
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

/* The destructor of the entity class of vtable D_8006A9F4, which func_800A47F4 installs. */
void func_800A53F0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9CC, which func_800A4C04 installs. */
void func_800A5438(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9A4, which func_800A4F64 installs. */
void func_800A5480(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A924);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A92C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A934);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A93C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A944);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A94C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A954);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A964);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A974);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A984);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A9A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A9CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006A9F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006AA1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006AA3C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_12", D_8006AA64);
