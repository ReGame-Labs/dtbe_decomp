/* One of the 24 fighters: its constructor, func_800B0854, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

/* the functions the overrides of entry 0xDC call */
void func_800B1518(Unk80076F34 *this, s32 arg);
extern VtableEntry D_8006BAE4[];
extern EntityVtable D_8006BB04;
extern EntityVtable D_8006BB2C;
extern EntityVtable D_8006BBE4;
extern EntityVtable D_8006BC0C;
extern VtableEntry D_8006BC34[];
extern EntityVtable D_8006BC54;
extern EntityVtable D_8006BCA4;

typedef struct Unk800B0854 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
} Unk800B0854;

/* The entity func_800B2E68 constructs (virtual table D_8006BB74, D_8006BB54
 * for its Unk8007BD8C at 0x18): sixteen points around a centre. */
typedef struct Unk800B2E68 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 part;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ u8 unk84[0x8A - 0x84];
    /* 0x08A */ u16 unk8A; /* while set, the destructor calls func_80079274 on func_8007AEEC's character */
    /* 0x08C */ u8 unk8C[0xAC - 0x8C];
    /* 0x0AC */ Vec3 unkAC;
    /* 0x0B8 */ u8 unkB8[4];
    /* 0x0BC */ Vec3 unkBC; /* func_800B2E68's third argument */
    /* 0x0C8 */ u8 unkC8[4];
    /* 0x0CC */ VECTOR unkCC[16];
} Unk800B2E68;
void func_800B19CC(Unk800B0854 *this);

/* Constructs an entity of the class of virtual table D_8006BCA4, derived from func_80076F34's. */
Unk800B0854 *func_800B0854(Unk800B0854 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006BCA4;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006BCA4, which func_800B0854 installs. */
void func_800B088C(Entity *this, s32 flags) {
    this->vtable = &D_8006BCA4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The method at 0x20 of virtual table D_8006BCA4: clears 0x4A4. */
void func_800B08E0(Unk800B0854 *this) {
    this->unk4A4 = 0;
}

/* func_800B08E8's local table of four entries (see Unk800685DC). */
typedef struct Unk8006B87C {
    Unk800C5B18Entry entries[4];
} Unk8006B87C;

extern Unk8006B87C D_8006B87C;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006B87C);

/* Overrides func_8007B884: calls it, and at step 8 creates func_800C5680's entity (0x690 bytes) with a copy of
 * D_8006B87C, 4, 5 and 5, and hands it to func_8007AD4C. */
void func_800B08E8(Unk80076F34 *this, s32 arg) {
    Unk8006B87C table;
    Entity *child;

    table = D_8006B87C;
    func_8007B884(this, arg);
    if (arg == 8) {
        child = allocEntityMemory(0x690);
        if (child != NULL) {
            func_800C5680(entityOperatorNewAt(0x690, child), this, table.entries, 4, 5, 5);
        }
        func_8007AD4C(this, child);
    }
}

/* Unk800B0854's override of Unk80076F34's method at 0xB0: func_8007B784, then
 * func_800B19CC when arg is 5. */
void func_800B09B0(Unk800B0854 *this, s32 arg) {
    func_8007B784(&this->base, arg);
    if (arg == 5) {
        func_800B19CC(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B09F4);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_8006BCA4: on 0, func_80079830 and
 * sound 0xEF; on 10, func_800B1518. */
void func_800B0B08(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_800784F4(this, 0xEF);
        break;
    case 10:
        func_800B1518(this, 0);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B0B78);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B0CBC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B0DD8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B0F48);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1084);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1220);

extern EntityVtable D_8006BC7C;

/* Constructs an Unk8008E9B4 with vtable D_8006BC7C (see func_8008E9B4), with 12 in unk4C and 24 in unk50. */
Unk8008E9B4 *func_800B146C(Unk8008E9B4 *this, Unk80076F34 *owner, s32 arg2, Vec3 *pos, s32 arg4) {
    func_800C3CFC(&this->base, pos, arg4, 0xFFC080);
    this->base.entity.vtable = &D_8006BC7C;
    this->unk54 = owner;
    this->base.unk4C = 12;
    this->base.unk50 = 24;
    return this;
}

/* The gather of the Unk8008E9B4 of D_8006BC7C: func_800C3E64, then puts the world position of the
 * owner's point 3 (func_8007A4A4) at 0x28. */
void func_800B14D0(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    func_8007A4A4(this->unk54, &this->base.unk28, 3, &origin);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006B92C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1518);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1618);

struct Unk800B2240 *func_800B2240(struct Unk800B2240 *this, Unk80076F34 *chara, Vec3 *position, s32 arg3);

/* Creates func_800B2240's entity (0x3C bytes) for the character at its point 20, moved 128 ahead (func_80079E10),
 * 0x40 up or, with arg, 0x1B down and with z cleared, and hands it to func_8007AD4C. */
void func_800B16F0(Unk80076F34 *this, s32 arg) {
    VECTOR position;
    VECTOR origin;
    Entity *child;
    s32 y;

    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    func_8007A4A4(this, &position, 20, &origin);
    position.vx += func_80079E10(this) * 128;
    y = position.vy;
    position.vz = 0;
    /* The match depends on storing the default height first and overwriting it with arg */
    position.vy = y - 0x40;
    if (arg != 0) {
        position.vy = y + 0x1B;
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800B2240(entityOperatorNewAt(0x3C, child), this, (Vec3 *)&position, arg);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006B93C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B17B4);

Entity *func_800B3990(Entity *this, Unk80076F34 *chara, Vec3 *position);

/* Creates func_800B3990's entity (0x1F4 bytes) at the character's position moved 640 toward func_80079E10's
 * side and 0xF0 up, and hands it to func_8007AD4C. */
void func_800B1938(Unk80076F34 *this) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    position.vx += func_80079E10(this) * 640;
    position.vy -= 0xF0;
    child = allocEntityMemory(0x1F4);
    if (child != NULL) {
        func_800B3990(entityOperatorNewAt(0x1F4, child), this, &position);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B19CC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1AC0);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006BC54 and D_8006BC34, which
 * func_800B1AC0 installs. */
void func_800B1C84(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006BC34;
    this->entity.vtable = &D_8006BC54;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B1D24);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg. */
void func_800B1FD4(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2028);

/* The entity class of vtable D_8006BC0C (0x3C bytes), made for a character at a position. */
typedef struct Unk800B2240 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u16 frames; /* counts the frames of func_800B2340 */
    /* 0x20 */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x22 */ u8 unk22;
    /* 0x23 */ u8 unk23;
    /* 0x24 */ u16 phase; /* func_800B2340's */
    /* 0x26 */ s16 unk26; /* the character's 0x440 (func_8007768C) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} Unk800B2240;

/* Constructs the entity of vtable D_8006BC0C for a character at a position, with arg3 at 0x1C. */
Unk800B2240 *func_800B2240(Unk800B2240 *this, Unk80076F34 *chara, Vec3 *position, s32 arg3) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BC0C;
    this->unk26 = func_8007768C(chara);
    this->facing = func_80079E10(chara);
    this->chara = chara;
    this->phase = 0;
    this->unk22 = 0x80;
    this->frames = 0;
    this->unk28 = 0x1000;
    this->unk1C = arg3;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BC0C, which func_800B2240 installs. */
void func_800B22EC(Entity *this, s32 flags) {
    this->vtable = &D_8006BC0C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of func_800B2240's entity: in phase 0, 0x28 grows by frames * 128 up to 0x2000 while 0x22 fades
 * by an eighth; once 0x22 is down to 8 the entity deletes itself. */
void func_800B2340(Unk800B2240 *this) {
    switch (this->phase) {
    case 0:
        this->unk28 += this->frames << 7;
        if (this->unk28 > 0x2000) {
            this->unk28 = 0x2000;
        }
        this->unk22 += -this->unk22 >> 3;
        if (this->unk22 > 8) {
            break;
        }
        this->phase++;
        /* fallthrough */
    case 1:
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BA10);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2414);

/* The entity class of vtable D_8006BBE4 (0x3C bytes), made for a character at a position. */
typedef struct Unk800B25CC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ u16 frames; /* counts the frames of func_800B26B8 */
    /* 0x1E */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 unk22;
    /* 0x24 */ s16 unk24; /* the character's 0x440 (func_8007768C) */
    /* 0x26 */ u8 unk26[2];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[4];
} Unk800B25CC;

/* Constructs the entity of vtable D_8006BBE4 for a character at a position. */
Unk800B25CC *func_800B25CC(Unk800B25CC *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006BBE4;
    this->unk24 = func_8007768C(chara);
    this->facing = func_80079E10(chara);
    this->chara = chara;
    this->unk22 = 0;
    this->unk20 = 0x80;
    this->frames = 0;
    this->unk28 = 0;
    /* The match depends on reading the three words before storing any, as vecCopy does. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_8006BBE4, which func_800B25CC installs. */
void func_800B2664(Entity *this, s32 flags) {
    this->vtable = &D_8006BBE4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of func_800B25CC's entity: 0x28 eases halfway to 0x1020 (at most 0x1000) every frame while 0x20
 * fades by an eighth; once 0x20 is down to 8 the entity deletes itself. */
void func_800B26B8(Unk800B25CC *this) {
    this->unk28 += (0x1020 - this->unk28) >> 1;
    if (this->unk28 > 0x1000) {
        this->unk28 = 0x1000;
    }
    if (this->unk20 < 9) {
        entityDelete(&this->entity);
    } else {
        this->unk20 += -this->unk20 >> 3;
        this->frames++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BA30);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B275C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B29C4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2A7C);

void func_800B2BA8(Unk800C7024 *this, s32 arg);
void func_800B2D10(Unk800C7024 *this, s32 arg);

/* The update of func_800B29C4's entity (an Unk800C7024): func_800B2BA8 while its part is in a list, then
 * func_800B2D10. */
void func_800B2B40(Unk800C7024 *this, s32 arg) {
    s32 alone;

    /* listIsAlone(&this->parts.part.node), reading at offsets from this (as entity.c's gather) */
    alone = this->parts.part.node.next == &this->parts.part.node
            && &this->parts.part.node == this->parts.part.node.prev;
    if (!alone) {
        func_800B2BA8(this, arg);
    }
    func_800B2D10(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BA50);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2BA8);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BA70);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2D10);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B2E68);

extern VtableEntry D_8006BB54[];
extern EntityVtable D_8006BB74;

/* The destructor of Unk800B2E68 (virtual tables D_8006BB74 and D_8006BB54): while 0x8A is set,
 * func_80079274 on func_8007AEEC's character first. */
void func_800B304C(Unk800B2E68 *this, s32 flags) {
    ListNode *node;

    this->unk18.vtable = D_8006BB54;
    this->entity.vtable = &D_8006BB74;
    if (this->unk8A != 0) {
        func_80079274(func_8007AEEC(this->chara), 0);
    }
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B310C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B33DC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B34D0);

/* The direction (ratan2) from the centre at 0xAC to point index, on x and y. */
s16 func_800B36B4(Unk800B2E68 *this, s32 index) {
    return ratan2(this->unkCC[index].vx - this->unkAC.vx, this->unkCC[index].vy - this->unkAC.vy);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BA90);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BAB0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B36F8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B3990);

/* The destructor of the entity class of vtable D_8006BB2C, which func_800B3990 installs. */
void func_800B3AC0(Entity *this, s32 flags) {
    this->vtable = &D_8006BB2C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B3B14);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B3CCC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B3F38);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006BB04 and D_8006BAE4, which
 * func_800B3F38 installs. */
void func_800B408C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006BAE4;
    this->entity.vtable = &D_8006BB04;
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BAD0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B412C);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg. */
void func_800B427C(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_16", func_800B42D0);

/* The destructor of the entity class of vtable D_8006BC7C, which func_800B146C installs. */
void func_800B44D8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800B4520(UnkPartsEntity38 *this, s32 flags) {
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BAE4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BB04);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BB2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BB54);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BB74);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BB9C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BBBC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BBE4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BC0C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BC34);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BC54);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BC7C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_16", D_8006BCA4);
