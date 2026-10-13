/* One of the 24 fighters: its constructor, func_800C1924, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

extern EntityVtable D_8006D01C;
extern EntityVtable D_8006D044;

typedef struct Unk800C1924 {
    /* 0x000 */ Unk800BB034 base;
    /* 0x4C4 */ s32 unk4C4;
} Unk800C1924;

Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4);
extern CVECTOR D_8006CFA8;
void func_800C1CE0(Unk800C1924 *this);

/* Constructs an entity of the class of virtual table D_8006D044, derived from func_800BB034's. */
Unk800C1924 *func_800C1924(Unk800C1924 *this) {
    func_800BB034(&this->base);
    this->base.base.entity.vtable = &D_8006D044;
    this->unk4C4 = 0;
    return this;
}

/* The base step (func_80078910) with arg, then this class's own (func_800C1CE0). */
void func_800C195C(void *this, void *arg) {
    func_80078910(this, arg);
    func_800C1CE0(this);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CF84);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C1988);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C1A8C);

/* the velocity func_800C1CE0 throws the partner with */
extern VECTOR D_8006CF88;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CF88);

/* This class's own step (func_800C195C): when the partner (func_8007AEEC) is over 0x800 above its 0x394, 0x4C4
 * goes back to 0; at 1, once func_800778BC allows it, stops the partner's 0x3A4 and 0x3B4 vectors, throws it with
 * D_8006CF88 (func_80078EF8) and sets 0x4C4 to 2. */
void func_800C1CE0(Unk800C1924 *this) {
    Unk80076F34 *partner = func_8007AEEC(&this->base.base);
    VECTOR velocity;

    if ((partner->position.vy >> 8) - (partner->unk394.vy >> 8) < -0x800) {
        this->unk4C4 = 0;
        return;
    }
    if (this->unk4C4 == 1 && func_800778BC(partner)) {
        velocity = D_8006CF88;
        partner->unk3B4.vx = 0;
        partner->unk3B4.vy = 0;
        partner->unk3B4.vz = 0;
        partner->unk3C0 = 0;
        partner->unk3A4.vx = 0;
        partner->unk3A4.vy = 0;
        partner->unk3A4.vz = 0;
        partner->unk3B0 = 0;
        func_80078EF8(partner, 0x408C1000, (Vec3 *)&velocity, 0);
        this->unk4C4 = 2;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CF98);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C1DBC);

/* Creates an entity of the class of vtable D_8006D01C (0x120 bytes) with g++'s new, and hands it
 * to func_8007AD4C. Its constructor func_800C27E4 is inlined. */
void func_800C1E64(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006D01C;
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C1ED8);

/* Creates an entity of func_800C2264's class (0xD4 bytes) at pos with g++'s new and hands it to func_8007AD4C, then
 * plays sound 0x11B, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800C2004(Unk80076F34 *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800C2264(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x200);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x11B);
    this->unk498 = 10;
    func_80077448(this, -0x10000);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C20A8);

extern VtableEntry D_8006CFCC[];
extern EntityVtable D_8006CFEC;

/* Constructs the class of virtual table D_8006CFEC (and D_8006CFCC for its base at 0x18) on
 * func_800BDA50's, with 0x5B05333 and 0 in its base's 0x8 and 0xC. */
Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    func_800BDA50(this, owner, pos, arg3, arg4);
    this->unk18.vtable = D_8006CFCC;
    this->entity.vtable = &D_8006CFEC;
    func_8007BDC8(&this->unk18, 0x5B05333, 0);
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CFA8);

/* The method at 0x20 of virtual table D_8006CFEC: adds func_800BE39C's and func_800C62EC's entities at 0xA4
 * as siblings, then plays sound 0x11C. */
void func_800C22C0(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB0A666, 0x500, &D_8006CFA8);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0xC0, 0xC0, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x11C, 0);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CFAC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_23", func_800C23B8);

/* The destructor of the entity class of vtable D_8006D044, which func_800C1924 installs. */
void func_800C2754(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D01C, which func_800C1E64 and func_800C27E4 install. */
void func_800C279C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006D01C, which derives from func_800BD490's. */
Entity *func_800C27E4(Entity *this, Unk80076F34 *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006D01C;
    return this;
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800C2818(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CFCC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006CFEC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006D01C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_23", D_8006D044);
