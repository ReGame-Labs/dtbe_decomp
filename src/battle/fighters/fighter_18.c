/* One of the 24 fighters: its constructor, func_800B631C, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"

void func_800B6D7C(Unk80076F34 *this);
struct Unk800B6DD8 *func_800B6DD8(struct Unk800B6DD8 *this, Unk800B631C *owner, VECTOR *pos, Unk8006ED20 *list,
                                   u32 color);
Unk800B73E8 *func_800B73E8(Unk800B73E8 *this, Unk80076F34 *arg);
Unk800C7024 *func_800B719C(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity,
                          Unk8006ED20 *obj);
Entity *func_800B7820(Entity *this, Unk80076F34 *arg);
extern Unk80079B24Entry D_8006C05C[];
extern EntityVtable D_8006C07C;
extern EntityVtable D_8006C0A4;
extern VtableEntry D_8006C114[];
extern EntityVtable D_8006C134;
extern EntityVtable D_8006C15C;
extern EntityVtable D_8006C0EC;
extern VtableEntry D_8006C0CC[];
void func_800B6868(Unk80076F34 *this);
void func_800B6910(Unk80076F34 *this);
/* offset's initializer in func_800B64E0: GCC keeps it as an anonymous constant (.rodata stays in asm) */
extern VECTOR D_8006C06C;

/* Constructs an entity of the class of vtable D_8006C15C, which derives from Unk80076F34 and
 * gives it the table D_8006C05C. */
Unk800B631C *func_800B631C(Unk800B631C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006C15C;
    func_80079B24(&this->base, D_8006C05C);
    this->unk4A4 = this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006C15C, which func_800B631C installs. */
void func_800B6368(Entity *this, s32 flags) {
    this->vtable = &D_8006C15C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The override of Unk80076F34's method at 0xC8 for virtual table D_8006C15C, with func_80077698's entry 5: arg 0
 * starts clip 20, sets 0x44C to 0x454 from the entry and calls func_8007A6FC(-1, 0x60, 3); below 17 it calls
 * func_8007835C with (16 - arg) * 0x600 toward the side it faces; from the entry's 0x4 to its 0x6 it calls
 * func_8007A0E4 with the entry's first word, else clears 0x448. */
void func_800B63BC(Unk80076F34 *this, s32 arg) {
    Unk80077698 *table;
    s32 vx;

    func_80079AE4(this);
    table = func_80077698(this);
    if (arg == 0) {
        func_80079830(this, 20, 0, 0, -1);
        this->unk44C = table->entries[5].unk20;
        /* The match depends on the product computed before the stores of 0x458 and 0x45C and stored last */
        vx = table->entries[5].unk8[4] * func_80079E10(this);
        this->unk458 = -table->entries[5].unk8[5];
        this->unk45C = 0;
        this->unk454 = vx;
        func_8007A6FC(this, -1, 0x60, 3);
    }
    if (arg < 17) {
        func_8007835C(this, func_80079E10(this) * (16 - arg) * 0x600);
    }
    if (arg >= table->entries[5].unk4 && table->entries[5].unk6 >= arg) {
        func_8007A0E4(this, table->entries[5].unk0);
    } else {
        this->unk448 = 0;
    }
}

/* Unk80076F34's method at 0xD4 for virtual table D_8006C15C: func_8007B978; under arg 10 func_8007835C(0) and the
 * partner (func_8007AEEC) takes the velocities at 0x3A4 and 0x3B4; arg 0 starts the partner's clip 13; arg 10, when
 * the partner is within 0x180 by 0x80, throws it (func_80078EF8) and spawns func_800C32E0's entity at node 12,
 * else stops it (func_80079320). */
void func_800B64E0(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Vec3 partnerPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Unk80076F34 *partner;
    Entity *child;
    s32 dx, dy;

    func_8007B978(this, arg);
    partner = func_8007AEEC(this);
    if (arg < 10) {
        s32 x, y, z;

        func_8007835C(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        partner->unk3A4.vx = x;
        partner->unk3A4.vy = y;
        partner->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        partner->unk3B4.vx = x;
        partner->unk3B4.vy = y;
        partner->unk3B4.vz = z;
    }
    switch (arg) {
    case 0:
        func_80079274(partner, 10);
        func_800796F0(partner, 13, 0, 0);
        break;
    case 10:
        func_800775D8(this, &position);
        func_8007A69C(this, &partnerPosition);
        dx = ABS(position.vx - partnerPosition.vx);
        dy = ABS(position.vy - partnerPosition.vy);
        if (dx < 0x180 && dy < 0x80) {
            velocity.vx = (s16)func_80079E10(this) * 14560;
            velocity.vy = -0x2AA8;
            velocity.vz = 0;
            func_80078EF8(partner, 0xC38C1B58, &velocity, 0);
            offset = D_8006C06C;
            func_8007A4A4(this, (VECTOR *)&node, 12, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                func_800C32E0(entityOperatorNewAt(0x48, child), &node, &D_80010814, 0xC0, 0x808080);
            }
            func_8007AD4C(this, child);
        } else {
            func_80079274(partner, 0);
            func_80079320(this);
        }
        break;
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006C15C: func_8007B9C8, sets bit 6
 * of 0x37C; arg 1 calls func_800B6910 and plays sound 0xFB, arg 13 calls func_800B6868 and plays sound
 * 0xF7, and below 14 the x at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_800B66FC(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800B6910(this);
        func_800784F4(this, 0xFB);
    }
    if (arg == 13) {
        func_800B6868(this);
        func_80078540(this, 0xF7);
    }
    if (arg < 14) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006C15C: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 1 calls func_800B6910 and plays sound 0xFB; arg 13 calls func_800B6868, plays sound 0xF7 and pushes vx at 0x3B4 back by
 * 0x3000; below 14 the vector at 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_800B6798(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800B6910(this);
        func_800784F4(this, 0xFB);
    }
    if (arg == 13) {
        func_800B6868(this);
        func_80078540(this, 0xF7);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 14) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_800B719C's entity (0xC0 bytes) 0x80 above the character, moving 128 a frame toward
 * func_80079E10's side, and hands it to func_8007AD4C. */
void func_800B6868(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x80;
    velocity.vx = func_80079E10(this) * 128;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        func_800B719C(entityOperatorNewAt(0xC0, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B73E8's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800B6910(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800B73E8(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

/* A step function of the class of func_800B631C: at step 0, sounds 0xFC and 0xF8 and clears unk4A4 and unk4A8;
 * counts unk4A4 down to 0; on odd steps before 16, func_800B6D7C; on steps 4 to 29 while unk4A4 is 0, creates a
 * func_800B6DD8 entity (0x94 bytes, color by unk4A8) at point 5 (odd steps) or 12 of the body; before step 31,
 * func_8007835C toward func_80079E10's side and unk3A4.vy = -step * 256, else func_8007835C with 0; before step 3,
 * func_8007A6FC. */
void func_800B696C(Unk800B631C *this, s32 step) {
    Unk8006ED20 *unk228;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 color;
    s32 odd;

    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        func_800784F4(&this->base, 0xFC);
        func_80078540(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = step & 1;
    if (odd && step < 16) {
        func_800B6D7C(&this->base);
    }
    if (step < 30 && this->unk4A4 == 0 && step >= 4) {
        unk228 = func_80077580(&this->base);
        memset(&offset, 0, sizeof(offset));
        color = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            color = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before color. */
        if (odd) {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        } else {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        }
    }
    if (step < 31) {
        func_8007835C(&this->base, func_80079E10(&this->base) << 14);
        this->base.unk3A4.vy = -(step << 8);
    } else {
        func_8007835C(&this->base, 0);
    }
    if (step < 3) {
        func_8007A6FC(&this->base, -1, 0x60, 5);
    }
}

/* A step function of the class of func_800B631C: at step 0, sounds 0xFC and 0xF8 and clears unk4A4 and unk4A8;
 * counts unk4A4 down to 0; on odd steps before 16, func_800B6D7C; on steps 0 to 29 while unk4A4 is 0, creates a
 * func_800B6DD8 entity (0x94 bytes, color by unk4A8) at point 5 (odd steps) or 12 of the body; before step 31,
 * func_8007835C toward func_80079E10's side and unk3A4.vy = -step * 256, else func_8007835C with 0. */
void func_800B6B88(Unk800B631C *this, s32 step) {
    Unk8006ED20 *unk228;
    /* The match depends on pos declared before offset: their stack slots. */
    VECTOR pos;
    VECTOR offset;
    u32 color;
    s32 odd;

    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        func_800784F4(&this->base, 0xFC);
        func_80078540(&this->base, 0xF8);
        this->unk4A4 = 0;
        this->unk4A8 = 0;
    }
    this->unk4A4--;
    if (this->unk4A4 < 0) {
        this->unk4A4 = 0;
    }
    odd = step & 1;
    if (odd && step < 16) {
        func_800B6D7C(&this->base);
    }
    if (step < 30 && this->unk4A4 == 0) {
        unk228 = func_80077580(&this->base);
        memset(&offset, 0, sizeof(offset));
        color = 0x7AC0800;
        if (this->unk4A8 >= 2) {
            color = 0xC7AC1000;
        }
        /* The match depends on a child per branch: one shared variable takes $s0 before color. */
        if (odd) {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 5, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        } else {
            Entity *child;

            func_8007A4A4(&this->base, &pos, 12, &offset);
            child = allocEntityMemory(0x94);
            if (child != NULL) {
                func_800B6DD8(entityOperatorNewAt(0x94, child), this, &pos, unk228, color);
            }
            func_8007AD4C(&this->base, child);
        }
    }
    if (step < 31) {
        func_8007835C(&this->base, func_80079E10(&this->base) << 14);
        this->base.unk3A4.vy = -(step << 8);
    } else {
        func_8007835C(&this->base, 0);
    }
}

/* Creates an entity of func_800B7820's class (0x120 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800B6D7C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        func_800B7820(entityOperatorNewAt(0x120, child), this);
    }
    func_8007AD4C(this, child);
}

/* The class func_800B6DD8 constructs (0x94 bytes; virtual tables D_8006C134, and D_8006C114 for its base at
 * 0x18). */
typedef struct Unk800B6DD8 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk800B631C *owner;
    /* 0x84 */ Vec3 pos;
    /* 0x90 */ s32 unk90;
} Unk800B6DD8;

/* Constructs Unk800B6DD8 for owner at pos with color, pushed 0x2AA8 a frame toward the side of the owner's
 * partner (func_8007A69C), and adds its part to one of list's two lists (func_80077574 picks which). */
Unk800B6DD8 *func_800B6DD8(Unk800B6DD8 *this, Unk800B631C *owner, VECTOR *pos, Unk8006ED20 *list, u32 color) {
    Vec3 ownerPosition;
    Vec3 partnerPosition;
    VECTOR v;
    s32 push;
    s32 x, y, z;
    ListNode *node;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, &owner->base, color, (Vec3 *)&D_80010814, 1);
    this->parts.base.vtable = D_8006C114;
    this->parts.entity.vtable = &D_8006C134;
    func_80075EE4(&this->parts.part, (Vec3 *)pos, 0x80, &this->parts.base);
    func_800775D8(&owner->base, &ownerPosition);
    func_8007A69C(&owner->base, &partnerPosition);
    /* The match depends on one variable for the difference and the push */
    push = partnerPosition.vx - ownerPosition.vx;
    if (push > 0) {
        push = 0x2AA8;
    } else {
        push = -0x2AA8;
    }
    v.vx = push;
    v.vy = 0;
    v.vz = 0;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = owner;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    /* The match depends on the node's address taken here, kept in a saved register for the list calls */
    node = &this->parts.part.node;
    this->parts.part.unk14 = &this->parts.entity;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    if (func_80077574(&owner->base) == 0) {
        func_8006F044(list, node);
    } else {
        func_8006F024(list, node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006C134 and D_8006C114, which
 * func_800B6DD8 installs. */
void func_800B6F24(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C114;
    this->entity.vtable = &D_8006C134;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B6FC4);

/* The gather of func_800B6DD8's entity: deletes it. */
void func_800B7158(Unk800B6DD8 *this, s32 arg) {
    entityDelete(&this->parts.entity);
}

/* Constructs func_800C7024's entity with 0x80, 0x7A81800, 0x2000, 3, then the virtual tables
 * D_8006C0EC (and D_8006C0CC for the base at 0x18). */
Unk800C7024 *func_800B719C(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity,
                          Unk8006ED20 *obj) {
    func_800C7024(this, chara, position, velocity, 0x80, 0x7A81800, 0x2000, 3);
    this->parts.base.vtable = D_8006C0CC;
    this->parts.entity.vtable = &D_8006C0EC;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B7200);

/* Constructs an entity of the class of vtable D_8006C0A4 for an Unk80076F34, with the vector
 * func_800775D8 gives. */
Unk800B73E8 *func_800B73E8(Unk800B73E8 *this, Unk80076F34 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C0A4;
    this->unk18 = arg;
    func_800775D8(arg, &this->unk1C);
    this->unk2C = 0x40;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006C0A4, which func_800B73E8 installs. */
void func_800B7464(Entity *this, s32 flags) {
    this->vtable = &D_8006C0A4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006C0A4: follows the character's position, 0x60 above it, 13 times, then
 * deletes itself. */
void func_800B74B8(Unk800B73E8 *this, s32 arg) {
    if (this->unk38 >= 13) {
        entityDelete(&this->entity);
    } else {
        this->unk38++;
        func_800775D8(this->unk18, &this->unk1C);
        this->unk1C.vy -= 0x60;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B7538);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B7820);

/* The destructor of the entity class of vtable D_8006C07C, which func_800B7820 installs. */
void func_800B79D4(Entity *this, s32 flags) {
    this->vtable = &D_8006C07C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B7A28);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_18", func_800B7BB0);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800B7E3C(UnkPartsEntity38 *this, s32 flags) {
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C00C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C014);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C01C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C024);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C02C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C034);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C03C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C04C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C05C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C06C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C07C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C0A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C0CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C0EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C114);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C134);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_18", D_8006C15C);
