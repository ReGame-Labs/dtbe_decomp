/* One of the 24 fighters: its constructor, func_8008820C, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

extern Unk80079B24Entry D_80068B6C[];
extern VtableEntry D_80068BAC[];
extern EntityVtable D_80068BCC;
extern VtableEntry D_80068BF4[];
extern EntityVtable D_80068C14;
extern EntityVtable D_80068C3C;

/* Constructs an entity of the class of vtable D_80068C3C, which derives from Unk80076F34 and
 * gives it the table D_80068B6C. */
Unk80076F34 *func_8008820C(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_80068C3C;
    func_80079B24(this, D_80068B6C);
    return this;
}

/* The destructor of the entity class of vtable D_80068C3C, which func_8008820C installs. */
void func_80088250(Entity *this, s32 flags) {
    this->vtable = &D_80068C3C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The override of Unk80076F34's method at 0xA8 for virtual table D_80068C3C:
 * func_8007B70C, then for arg 1 to 4 func_8007835C with 9100 times the facing. */
void func_800882A4(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg >= 1 && arg <= 4) {
        func_8007835C(this, func_80079E10(this) * 9100);
    }
}

/* The override of Unk80076F34's method at 0xB0 for virtual table D_80068C3C:
 * func_8007B784, then for arg 16 to 19 func_8007835C with 18200 times the facing. */
void func_80088310(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg >= 16 && arg <= 19) {
        func_8007835C(this, func_80079E10(this) * 18200);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_8008837C);

void func_80088A64(Unk80076F34 *this, s32 arg);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_80068C3C: on 0, func_80079830,
 * func_80079B0C(27) and sound 0x8B; on 14, func_80088A64. */
void func_800884FC(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_80079B0C(this, 27);
        func_800784F4(this, 0x8B);
        break;
    case 14:
        func_80088A64(this, 0);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088578);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_8008876C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088898);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088A64);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088B1C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088C24);

/* The destructor of the UnkPartsEntity38 class of vtables D_80068C14 and D_80068BF4, which
 * func_80088C24 installs. */
void func_80088E48(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068BF4;
    this->entity.vtable = &D_80068C14;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80088EE8);

/* The UnkPartsEntity38 class func_80088C24 constructs (virtual tables D_80068C14 and D_80068BF4), as far as
 * func_800890A4 needs it. */
typedef struct Unk80088C24 {
    /* 0x000 */ UnkPartsEntity38 base;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0x9C - 0x84];
    /* 0x09C */ Vec3 unk9C; /* the first of a run of positions */
    /* 0x0A8 */ u8 unkA8[0x100 - 0xA8];
    /* 0x100 */ Vec3 unk100; /* a constructor argument */
    /* 0x10C */ u8 unk10C[0x110 - 0x10C];
    /* 0x110 */ SVECTOR corners[4];
    /* 0x130 */ s32 unk130; /* the length of unk100 */
} Unk80088C24;

/* The second base's first method for virtual table D_80068BF4: func_8007BDF0 with sum; when func_80078878
 * finds a target in sum, func_800C75FC's entity on it; then a burst of 6 (func_800C62EC) at 0x9C, and the
 * entity deletes itself. */
void func_800890A4(Unk80088C24 *this, Unk80075EE4 *sum) {
    Entity *target;
    Entity *child;

    func_8007BDF0(&this->base.base, sum);
    target = func_80078878(sum);
    if (target != NULL) {
        entityAddSibling(&this->base.entity, func_800C75FC(target, &this->unk100, 0x80));
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unk9C, this->unk130 >> 2, 0x60, 6, 0x808080);
    }
    entityAddSibling(&this->base.entity, child);
    entityDelete(&this->base.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80089180);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_800893BC);

/* The destructor of the UnkPartsEntity38 class of vtables D_80068BCC and D_80068BAC, which
 * func_800893BC installs. */
void func_800895D0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068BAC;
    this->entity.vtable = &D_80068BCC;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_80089670);

/* Sets bit 0 of 0x44. */
void func_800899A0(Unk800AD5BC *this) {
    this->unk44 |= 1;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_03", func_800899B4);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B24);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B34);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B3C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B44);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B4C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B5C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B6C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B7C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068B8C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068BAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068BCC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068BF4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068C14);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_03", D_80068C3C);
