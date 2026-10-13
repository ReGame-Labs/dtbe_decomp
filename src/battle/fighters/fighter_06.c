/* One of the 24 fighters: its constructor, func_80091A44, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

Entity *func_800925AC(Entity *this, Unk80076F34 *chara, Vec3 *position);
extern Unk800C5B18Entry D_800694B4[];
extern EntityVtable D_800694F4;
extern VtableEntry D_8006951C[];
extern EntityVtable D_8006953C;
extern EntityVtable D_80069564;
Entity *func_80091E60(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);

/* Constructs an object of the entity class of vtable D_80069564, which derives from func_8008D21C's. */
Unk8008D21C *func_80091A44(Unk8008D21C *this) {
    func_8008D21C(this);
    this->base.entity.vtable = &D_80069564;
    return this;
}

/* The destructor of the entity class of vtable D_80069564, which func_80091A44 installs. */
void func_80091A78(Entity *this, s32 flags) {
    this->vtable = &D_80069564;
    func_8008D27C(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* For arg 1, creates func_800925AC's entity (0x74 bytes) at the character's position and hands it to
 * func_8007AD4C. */
void func_80091ACC(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Entity *child;

    if (arg == 1) {
        func_800775D8(this, &position);
        child = allocEntityMemory(0x74);
        if (child != NULL) {
            func_800925AC(entityOperatorNewAt(0x74, child), this, &position);
        }
        func_8007AD4C(this, child);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_80091B3C);

/* Spawns the entity of func_80091E60 at the character (func_800775D8), with
 * velocity (64 forward, 0, 0). */
void func_80091D4C(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = func_80079E10(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xF0);
    if (child != NULL) {
        func_80091E60(entityOperatorNewAt(0xF0, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_800694B4);

/* Creates an entity of func_800C5680's class (0x690 bytes) with g++'s new and the table
 * D_800694B4, and hands it to func_8007AD4C. */
void func_80091DE8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_800694B4, 2, 8, 15);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_80091E60);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006953C and D_8006951C, which
 * func_80091E60 installs. */
void func_8009201C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006951C;
    this->entity.vtable = &D_8006953C;
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_800694E4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_800920BC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_800921B4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_8009230C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_800925AC);

/* The destructor of the entity class of vtable D_800694F4, which func_800925AC installs. */
void func_800926A4(Entity *this, s32 flags) {
    this->vtable = &D_800694F4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_800926F8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_06", func_800927C8);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_800694F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_8006951C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_8006953C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_06", D_80069564);
