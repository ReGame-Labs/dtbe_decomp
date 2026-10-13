/* One of the 24 fighters: its constructor, func_800B9410, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

void func_800B9F24(Unk80076F34 *this, s32 arg);
extern Unk80079B24Entry D_8006C4FC[];
extern VtableEntry D_8006C6AC[];
extern EntityVtable D_8006C6CC;
extern EntityVtable D_8006C71C;
Entity *func_800B9FF0(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
Entity *func_800BAAA4(void *this, Unk80076F34 *chara, s32 arg);

/* Constructs an entity of the class of vtable D_8006C71C, which derives from Unk80076F34 and
 * gives it the table D_8006C4FC. */
Unk800B9410 *func_800B9410(Unk800B9410 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006C71C;
    func_80079B24(&this->base, D_8006C4FC);
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006C71C, which func_800B9410 installs. */
void func_800B9458(Entity *this, s32 flags) {
    this->vtable = &D_8006C71C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800B9410's override of the method at 0x78: func_8007B2C0, then, the first
 * time, spawns the entity of func_800BAAA4 (with 0) for the character. */
void func_800B94AC(Unk800B9410 *this, s32 arg) {
    func_8007B2C0(&this->base, arg);
    if (this->unk4A4 == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            func_800BAAA4(entityOperatorNewAt(0x4C, child), &this->base, 0);
        }
        func_8007AD4C(&this->base, child);
        this->unk4A4 = 1;
    }
}

/* Unk800B9410's override of the method at 0x80: func_8007B2EC, then, the first
 * time, spawns the entity of func_800BAAA4 (with 1) for the character. */
void func_800B9528(Unk800B9410 *this, s32 arg) {
    func_8007B2EC(&this->base, arg);
    if (this->unk4A4 == 0) {
        Entity *child = allocEntityMemory(0x4C);

        if (child != NULL) {
            func_800BAAA4(entityOperatorNewAt(0x4C, child), &this->base, 1);
        }
        func_8007AD4C(&this->base, child);
        this->unk4A4 = 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800B95A4);

/* Overrides func_8007B9C8 (entry 0xDC of Unk80076F34's vtable) in D_8006C71C: on 0, func_80079830 and
 * sound 0x105; on 13, func_800B9F24. */
void func_800B972C(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_800784F4(this, 0x105);
        break;
    case 13:
        func_800B9F24(this, 0);
        break;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4D4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C4FC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800B979C);

/* A step method: func_8007BAFC; at step 0 clip 10 and unk3A4.vy cleared; at step 5 unk3B4.vy falls by
 * unk3C4 * 0x4718 (16.16); a rising unk3A4 slows unk3B4. */
void func_800B99A8(Unk80076F34 *this, s32 step) {
    func_8007BAFC(this, step);
    switch (step) {
    case 0:
        func_80079830(this, 10, 2, 0, -1);
        this->unk3A4.vy = 0;
        break;
    case 5:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x4718) >> 16;
        break;
    }
    if (this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

/* Overrides func_8007B80C (entry 0xBC of Unk80076F34's vtable) in D_8006C71C: calls it; when arg is 4,
 * clears unk3A4 and moves unk3B4 by unk3C4 * 0x38E0 (16.16) toward func_80079E10's side; from 5 on,
 * moves unk3B4 back by a quarter of unk3A4. */
void func_800B9A64(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 4) {
        s32 step;

        this->unk3A4.vx = 0;
        step = ((s64)this->unk3C4 * 0x38E0) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 5) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800B9B18);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800B9C48);

/* func_800B9E58's local table of three entries (see Unk800685DC). */
typedef struct Unk8006C5FC {
    Unk800C5B18Entry entries[3];
} Unk8006C5FC;

extern Unk8006C5FC D_8006C5FC;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C5FC);

/* Creates func_800C5680's entity (0x690 bytes) for the character with a copy of D_8006C5FC, 3, 8 and 23, hands
 * it to func_8007AD4C, and plays sound 0x102. */
void func_800B9E58(Unk80076F34 *this) {
    Unk8006C5FC table;
    Entity *child;

    table = D_8006C5FC;
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, table.entries, 3, 8, 23);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x102);
}

/* Spawns the entity of func_800B9FF0 0x70 above the character (0x60 when arg is 0), with velocity
 * (48 forward, 0, 0). */
void func_800B9F24(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    if (arg == 0) {
        position.vy -= 0x60;
    } else {
        position.vy -= 0x70;
    }
    velocity.vx = func_80079E10(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x184);
    if (child != NULL) {
        func_800B9FF0(entityOperatorNewAt(0x184, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800B9FF0);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006C6CC and D_8006C6AC, which
 * func_800B9FF0 installs. */
void func_800BA1F8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C6AC;
    this->entity.vtable = &D_8006C6CC;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BA298);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BA49C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BA598);

/* The entity class func_800BA734 constructs (virtual table D_8006C6F4): an Unk800C4F74 with 0x488 at 5/6. */
typedef struct Unk800BA734 {
    /* 0x000 */ Unk800C4F74 base;
    /* 0x4A8 */ s16 vramY; /* func_8007768C of the character */
} Unk800BA734;
extern EntityVtable D_8006C6F4;

/* Constructs an Unk800BA734 for chara and owner: func_800C4F74's entity, with 0x488 cut to 5/6. */
Unk800BA734 *func_800BA734(Unk800BA734 *this, Unk80076F34 *chara, Unk8007F920 *owner) {
    func_800C4F74(&this->base, owner);
    this->base.entity.vtable = &D_8006C6F4;
    this->vramY = func_8007768C(chara);
    this->base.unk488 = this->base.unk488 * 5 / 6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BA7AC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BA8AC);

/* The entity class func_800BAAA4 constructs (0x4C bytes, virtual table D_8006C684) for an Unk800B9410,
 * which makes one while its 0x4A4 is 0. */
typedef struct Unk800BAAA4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800B9410 *chara;
    /* 0x1C */ u32 handle; /* the character's (entityGetHandle) */
    /* 0x20 */ s16 unk20;
    /* 0x22 */ s16 facing; /* func_80079E10 */
    /* 0x24 */ s16 vramY;  /* func_8007768C */
    /* 0x26 */ s16 unk26;
    /* 0x28 */ u8 unk28[0x4C - 0x28];
} Unk800BAAA4;

extern EntityVtable D_8006C684;

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BAAA4);

/* The destructor of func_800BAAA4's class: clears the character's 0x4A4, so that it can make another. */
void func_800BAC08(Unk800BAAA4 *this, s32 flags) {
    this->entity.vtable = &D_8006C684;
    this->chara->unk4A4 = 0;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BAC64);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C644);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C664);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_20", func_800BAE98);

/* The destructor of the entity class of vtable D_8006C6F4, which func_800BA734 installs. */
void func_800BAFEC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C684);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C6AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C6CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C6F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_20", D_8006C71C);
