/* One of the 24 fighters: its constructor, func_800A0FF4, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

extern Unk80079B24Entry D_8006A764[];
extern EntityVtable D_8006A784;
extern EntityVtable D_8006A7AC;
extern EntityVtable D_8006A7FC;
Entity *func_800A1E70(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);

/* Constructs an entity of the class of vtable D_8006A7FC, which derives from Unk80076F34 and
 * gives it the table D_8006A764. */
Unk800A0FF4 *func_800A0FF4(Unk800A0FF4 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006A7FC;
    func_80079B24(&this->base, D_8006A764);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006A7FC, which func_800A0FF4 installs. */
void func_800A103C(Entity *this, s32 flags) {
    this->vtable = &D_8006A7FC;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800A1090(void) {
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A1098);

/* Sets out's x and y to the point at t (4.12) of the way from from to to. */
void func_800A1160(Unk800A0FF4 *this, VECTOR *from, VECTOR *to, s32 t, VECTOR *out) {
    out->vx = from->vx + (((to->vx - from->vx) * t) >> 12);
    out->vy = from->vy + (((to->vy - from->vy) * t) >> 12);
}

/* Clamps 0x4B8 to 0..0x1000 and puts the character's 0x3C8 that far from 0x4CC to 0x4DC (func_800A1160). */
void func_800A11B0(Unk800A0FF4 *this) {
    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &this->base.unk3C8);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A1210);

/* Clamps 0x4B8 to 0..0x1000 and sets chara moving (0x3B4, in 1/256ths) from its position to the point
 * that far from 0x4CC to 0x4DC (func_800A1160), clearing 0x3A4..0x3B0 and 0x3BC. */
void func_800A1760(Unk800A0FF4 *this, Unk80076F34 *chara) {
    Vec3 position;
    VECTOR target;

    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    func_800775D8(chara, &position);
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &target);
    chara->unk3B4.vz = 0;
    chara->unk3A4.vx = 0;
    chara->unk3A4.vy = 0;
    chara->unk3A4.vz = 0;
    chara->unk3B0 = 0;
    chara->unk3B4.vx = (target.vx - position.vx) << 8;
    chara->unk3B4.vy = (target.vy - position.vy) << 8;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A1818);

void func_800A1968(Unk80076F34 *this);

/* A method of func_800A0FF4's class: arg 0 runs func_80079320 when 0x4A4 is set, else plays sound 0xC9 and
 * starts clip 0x17; arg 3 runs func_800A1968; below 17, 0x3B4.vx loses a 16th of 0x3A4.vx and a positive
 * 0x3A4.vy is taken off 0x3B4.vy. */
void func_800A18A4(Unk800A0FF4 *this, s32 arg) {
    if (arg == 0) {
        if (this->unk4A4 != 0) {
            func_80079320(&this->base);
            return;
        }
        func_800784F4(&this->base, 0xC9);
        func_80079830(&this->base, 0x17, 2, 0, -1);
    }
    if (arg == 3) {
        func_800A1968(&this->base);
    }
    if (arg < 17) {
        this->base.unk3B4.vx -= this->base.unk3A4.vx >> 4;
        if (this->base.unk3A4.vy > 0) {
            this->base.unk3B4.vy -= this->base.unk3A4.vy;
        }
    }
}

/* Spawns the entity of func_800A1E70 64 ahead of and 0x60 above the character, with velocity (96
 * forward, 0, 0), then plays sound 0xC6 and sets 0x334 to 40. */
void func_800A1968(Unk80076F34 *this) {
    s32 facing = func_80079E10(this);
    Unk8006ED20 *unk228;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = facing * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 64;
    position.vy -= 0x60;
    unk228 = func_80077580(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_800A1E70(entityOperatorNewAt(0xC8, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xC6);
    func_80079B0C(this, 40);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A1A44);

/* an axis next to AXIS_Y and AXIS_Z in the executable's data */
extern VECTOR D_80010874;

/* Whether the character func_8007AEEC gives is in reach: when func_80079D94 holds for it, what
 * func_80079E50 finds within both func_80079DD8 sizes plus 0xA0 (squared) around AXIS_Z, else around
 * D_80010874; NULL otherwise. */
Unk80076F34 *func_800A1CDC(Unk80076F34 *this) {
    Unk80076F34 *other = func_8007AEEC(this);
    s32 reach;
    s32 range;

    if (!func_80079D94(other)) {
        return NULL;
    }
    reach = func_80079DD8(this) + func_80079DD8(other) + 0xA0;
    range = reach * reach;
    /* The match depends on reusing other for the character found (a local of its own stays in $v0) */
    other = func_80079E50(this, &AXIS_Z, range, 0x200);
    if (other != NULL) {
        return other;
    }
    return func_80079E50(this, &D_80010874, range, 0x200);
}

/* A value by func_80077568 of the character func_8007AEEC gives: 0x3FFC for 11, 0x4718 for 12, 15 and 20,
 * 0x4E34 for any other. */
s32 func_800A1D84(Unk80076F34 *this) {
    s32 value;

    switch (func_80077568(func_8007AEEC(this))) {
    case 11:
        value = 0x3FFC;
        break;
    case 12:
    case 15:
    case 20:
        value = 0x4718;
        break;
    default:
        value = 0x4E34;
        break;
    }
    return value;
}

/* Creates an entity of the class of vtable D_8006A784 (0x120 bytes) with g++'s new, and hands it
 * to func_8007AD4C. Its constructor func_800A2F64 is inlined. */
void func_800A1DFC(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006A784;
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A1E70);

/* The entity func_800A1E70 constructs (0xC8 bytes, virtual table D_8006A7D4) for the character of
 * func_800A1968 (in the code of func_800A0FF4's class): it sets that character's 0x4A4. */
typedef struct Unk800A1E70 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk800A0FF4 *chara;
} Unk800A1E70;

extern EntityVtable D_8006A7D4;

/* The destructor of the entity class of vtable D_8006A7D4: clears the character's 0x4A4 and calls its
 * func_80079B0C with 1. */
void func_800A1FF4(Unk800A1E70 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006A7D4;
    this->chara->unk4A4 = 0;
    func_80079B0C(&this->chara->base, 1);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.unk18.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A209C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A2244);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A2700);

/* The destructor of the entity class of vtable D_8006A7AC, which func_800A2700 installs. */
void func_800A2798(Entity *this, s32 flags) {
    this->vtable = &D_8006A7AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A27EC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A2870);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_11", func_800A2C70);

/* The destructor of the entity class of vtable D_8006A784, which func_800A1DFC and func_800A2F64 install. */
void func_800A2F1C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006A784, which derives from func_800BD490's. */
Entity *func_800A2F64(Entity *this, Unk80076F34 *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006A784;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A714);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A71C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A724);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A72C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A734);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A73C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A744);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A754);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A764);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A774);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A784);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A7AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A7D4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_11", D_8006A7FC);
