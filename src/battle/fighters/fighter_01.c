/* One of the 24 fighters: its constructor, func_80083F0C, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"

Entity *func_8008514C(Entity *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Entity *func_80085B7C(Entity *this, Unk80076F34 *arg, Vec3 *vec);
struct Unk800862C0 *func_800862C0(struct Unk800862C0 *this, Unk80076F34 *chara, Vec3 *position);
Entity *func_800856D0(Entity *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj);
Entity *func_800868EC(Entity *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity, Unk8006ED20 *obj);
extern VtableEntry D_8006869C[];
extern EntityVtable D_800686BC;
extern EntityVtable D_800686E4;
extern EntityVtable D_8006870C;
extern EntityVtable D_80068734;
extern EntityVtable D_8006875C;
extern EntityVtable D_80068784;

typedef struct Unk80083F0C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ u16 unk4A6;
} Unk80083F0C;
extern VECTOR D_80068654;

/* Constructs an entity of the class of virtual table D_80068784, derived from func_80076F34's. */
Unk80083F0C *func_80083F0C(Unk80083F0C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80068784;
    this->unk4A4 = 0;
    this->unk4A6 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068784, which func_80083F0C installs. */
void func_80083F48(Entity *this, s32 flags) {
    this->vtable = &D_80068784;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Overrides entry 0x20 of Unk80076F34's vtable in D_80068784: clears 0x4A6. */
void func_80083F9C(Unk80083F0C *this) {
    this->unk4A6 = 0;
}

/* Overrides func_8007B034 (entry 0x58 of Unk80076F34's vtable) in D_80068784: calls it,
 * then func_8007A6FC. */
void func_80083FA4(Unk80076F34 *this, s32 arg) {
    func_8007B034(this, arg);
    func_8007A6FC(this, -1, 160, 4);
}

/* Overrides func_8007B0E0 (entry 0x60 of Unk80076F34's vtable) in D_80068784: calls it,
 * then func_8007A6FC. */
void func_80083FDC(Unk80076F34 *this, s32 arg) {
    func_8007B0E0(this, arg);
    func_8007A6FC(this, -1, 160, 4);
}

/* Overrides func_8007B884: calls it, then func_8007A6FC(-1, 160, 2) every fourth step below 18. */
void func_80084014(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg < 18 && (arg & 3) == 0) {
        func_8007A6FC(this, -1, 160, 2);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80084068);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80084200);

void func_80084B4C(Unk80076F34 *this, s32 arg);
void func_80084C04(Unk80076F34 *this);

/* A step method of D_80068784: func_80079AE4; at step 0 clip 22, sound 0x7F; at 5 the entity of func_80084B4C;
 * at 7 and 17 func_8007A0E4 and the words at 0x44C; at 29 clears unk448. Steps 5-16 push the character forward
 * (func_8007835C); even steps 6-12 call func_8007A6FC, and steps 6-13 make func_80084C04's entity. */
void func_8008439C(Unk80076F34 *this, s32 step) {
    s32 even;

    func_80079AE4(this);
    switch (step) {
    case 0:
        func_80079830(this, 22, 0, 0, -1);
        this->unk3A4.vx = 0;
        func_800784F4(this, 0x7F);
        break;
    case 5:
        func_80084B4C(this, 15);
        break;
    case 7:
        func_8007A0E4(this, 8);
        this->unk44C = 0x5A82190;
        this->unk450 = 2;
        break;
    case 17:
        this->unk448 = 0;
        func_8007A0E4(this, 4);
        this->unk44C = 0x45A82190;
        func_8007A6FC(this, -1, 160, 4);
        this->unk454 = func_80079E10(this) * 14560;
        this->unk458 = -0x38E0;
        this->unk45C = 0;
        this->unk450 = 2;
        this->unk3A4.vx = 0;
        break;
    case 29:
        this->unk448 = 0;
        break;
    }
    if (step >= 5 && step <= 16) {
        func_8007835C(this, func_80079E10(this) * 25480);
    }
    /* The match depends on the parity test computed as a value in a local (xori/andi, do_store_flag). */
    even = (step & 1) == 0;
    if (even && step >= 6 && step < 14) {
        func_8007A6FC(this, -1, 160, 4);
    }
    if (step >= 6 && step <= 13) {
        func_80084C04(this);
    }
}

void func_80084C74(Unk80076F34 *this);
void func_80084D00(Unk80076F34 *this);

/* Entry 0xE0 of D_80068784 (func_8007B9FC's slot): func_80079AE4; at step 0 the entities of func_80084C74 and
 * func_80084D00, clip 24, func_8007A0E4 with 12, the words at 0x44C and sound 0x80; at step 5 unk3B4.vy
 * falls by unk3C4 * 0x6AA4 (16.16); below step 22 a rising unk3A4 slows unk3B4. */
void func_80084558(Unk80076F34 *this, s32 step) {
    func_80079AE4(this);
    switch (step) {
    case 0:
        func_80084C74(this);
        func_80084D00(this);
        func_80079830(this, 24, 0, 0, -1);
        this->unk3A4.vy = 0;
        func_8007A0E4(this, 12);
        this->unk44C = 0x45AC2A00;
        this->unk454 = func_80079E10(this) * 5460;
        this->unk458 = -0x6388;
        this->unk45C = 0;
        this->unk450 = 3;
        func_800784F4(this, 0x80);
        break;
    case 5:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x6AA4) >> 16;
        break;
    }
    if (step < 22 && this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80084694);

/* Entry 0x118 of D_80068784 (func_8007BC1C's slot): func_80079AE4; at step 0 the entities of func_80084C74 and func_80084D00, clip 25,
 * unk3B4.vy falls by unk3C4 * 0x38E0 (16.16), func_8007A0E4 with 12, the words at 0x44C and sound 0x80; at step 5
 * the same fall; below step 12 a rising unk3A4 slows unk3B4. */
void func_80084838(Unk80076F34 *this, s32 step) {
    func_80079AE4(this);
    switch (step) {
    case 0:
        func_80084C74(this);
        func_80084D00(this);
        func_80079830(this, 25, 0, 0, -1);
        this->unk3A4.vy = 0;
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x38E0) >> 16;
        func_8007A0E4(this, 12);
        this->unk44C = 0x45AC2A00;
        this->unk454 = func_80079E10(this) * 5460;
        this->unk458 = -0x6388;
        this->unk45C = 0;
        func_800784F4(this, 0x80);
        break;
    case 5:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x38E0) >> 16;
        break;
    }
    if (step < 12 && this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_8008499C);

/* The two entries of a table that func_80084B4C, func_80084D00 and func_80084FE8 each initialise. cc1 gives
 * identical array initializers one constant per unit, so the three share its data (D_800685DC in the asm). */
#define D_800685DC_ENTRIES \
    { 0x14, { 0, 0x180, 0 }, 0, { 0xFF, 0x80, 0x80, 0 } }, \
    { 0x14, { 0, 0, 0 }, 0, { 0, 0, 0, 0 } }

/* Creates func_800C5680's entity (0x690 bytes) for the character with a table of two entries, 2, 8 and arg, and
 * hands it to func_8007AD4C. */
void func_80084B4C(Unk80076F34 *this, s32 arg) {
    Unk800C5B18Entry table[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, table, 2, 8, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_8008514C's class (0x48 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_80084C04(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_8008514C(entityOperatorNewAt(0x48, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800856D0's entity (0x34 bytes) at the character's position, and hands it to
 * func_8007AD4C. */
void func_80084C74(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Vec3 position;
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        func_800856D0(entityOperatorNewAt(0x34, child), this, func_800775D8(this, &position), obj);
    }
    func_8007AD4C(this, child);
}

/* Creates two of func_800C5680's entities (0x690 bytes) for the character, one with a table of three entries,
 * 3, 8 and 20, one with the table of two entries, 2, 8 and 15, and hands each to func_8007AD4C. */
void func_80084D00(Unk80076F34 *this) {
    Unk800C5B18Entry first[3] = {
        { 0, { 0, -0x40, 0 }, 0, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0 }, 0, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0 }, 0, { 0x08, 0x08, 0xFF, 0 } },
    };
    Unk800C5B18Entry second[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, first, 3, 8, 20);
    }
    func_8007AD4C(this, child);
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, second, 2, 8, 15);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_80085B7C's class (0xDC bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_80084E58(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0xDC);
    if (child != NULL) {
        func_80085B7C(entityOperatorNewAt(0xDC, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800862C0's class (0x54 bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_80084EC0(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800862C0(entityOperatorNewAt(0x54, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_80068654);

/* Creates func_800868EC's entity (0xB8 bytes) at the character's point 18 moved by D_80068654 and 256
 * toward func_80079E10's side, standing still with arg in the velocity's pad; hands it to func_8007AD4C. */
void func_80084F28(Unk80076F34 *this, s16 arg) {
    Unk8006ED20 *obj;
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    obj = func_80077580(this);
    func_8007A4A4(this, (VECTOR *)&position, 18, &D_80068654);
    position.vx += func_80079E10(this) * 256;
    velocity.vx = 0;
    velocity.vy = 0;
    velocity.vz = 0;
    velocity.pad = arg;
    child = allocEntityMemory(0xB8);
    if (child != NULL) {
        func_800868EC(entityOperatorNewAt(0xB8, child), this, &position, &velocity, obj);
    }
    func_8007AD4C(this, child);
}

/* func_80084D00 followed by the sound 0x7C (func_80078540). */
void func_80084FE8(Unk80076F34 *this) {
    Unk800C5B18Entry first[3] = {
        { 0, { 0, -0x40, 0 }, 0, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0 }, 0, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0 }, 0, { 0x08, 0x08, 0xFF, 0 } },
    };
    Unk800C5B18Entry second[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, first, 3, 8, 20);
    }
    func_8007AD4C(this, child);
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, second, 2, 8, 15);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x7C);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_8008514C);

/* The destructor of the entity class of vtable D_8006875C, which func_8008514C installs. */
void func_800852C0(Entity *this, s32 flags) {
    this->vtable = &D_8006875C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80085314);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_800854B4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_800856D0);

/* The destructor of the entity class of vtable D_80068734, which func_800856D0 installs. */
void func_80085760(Entity *this, s32 flags) {
    this->vtable = &D_80068734;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class of virtual table D_80068734 (func_800856D0, 0x34 bytes). */
typedef struct Unk800856D0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ u16 unk1C; /* frames */
    /* 0x1E */ u8 unk1E;  /* 0x80 at first */
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s16 unk20; /* func_8007768C of the character */
    /* 0x22 */ u8 unk22[0x24 - 0x22];
    /* 0x24 */ Vec3 position;
    /* 0x30 */ u8 unk30[0x34 - 0x30];
} Unk800856D0;

/* The update of D_80068734's entity: follows the character's position, takes 0x10 off 0x1E a frame from
 * frame 13 on, and deletes itself after frame 20. */
void func_800857B4(Unk800856D0 *this) {
    func_800775D8(this->chara, &this->position);
    if (this->unk1C >= 13) {
        this->unk1E -= 0x10;
    }
    if (this->unk1C >= 21) {
        entityDelete(&this->entity);
    } else {
        this->unk1C++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80085848);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_80068664);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80085B7C);

/* The destructor of the entity class of vtable D_8006870C, which func_80085B7C installs. */
void func_80085C90(Entity *this, s32 flags) {
    this->vtable = &D_8006870C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class of vtable D_8006870C (0xDC bytes, func_80085B7C constructs it for the character): a point of
 * the owner that rises and fades out in steps. */
typedef struct Unk80085B7C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80083F0C *owner;
    /* 0x1C */ u8 unk1C;    /* 0x80 from func_80085B7C, eased to 0 */
    /* 0x1D */ u8 unk1D;    /* 0 from func_80085B7C, eased to 0x84 */
    /* 0x1E */ s16 side;    /* func_80079E10 of the owner */
    /* 0x20 */ u16 timer;   /* counts the frames */
    /* 0x22 */ u16 state;
    /* 0x24 */ u16 counter; /* frames in state 2 */
    /* 0x26 */ s16 flags;   /* func_8007768C of the owner */
    /* 0x28 */ s16 unk28;
    /* 0x2A */ u8 unk2A[0x34 - 0x2A];
    /* 0x34 */ VECTOR position; /* the owner's point 18, by func_8007A4A4 */
    /* 0x44 */ VECTOR unk44;    /* position, 128 toward side */
    /* 0x54 */ s32 unk54;       /* 0x1000 from func_80085B7C, eased to 0x3400 */
    /* 0x58 */ s32 unk58;       /* 0x1800 from func_80085B7C, a quarter less every frame of state 3 */
    /* 0x5C */ s16 unk5C[16][4];
} Unk80085B7C; /* size 0xDC */

extern VECTOR D_80068674;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_80068674);

/* The update of Unk80085B7C: follows the owner's point 18 moved by D_80068674 and eases unk1C and unk1D; then by
 * state: grows unk54 toward 0x3400 while bobbing up and down, waits 20 frames, shrinks unk58 below 5 and deletes
 * itself. It goes to state 3 early when the owner's unk4A6 is 0. */
void func_80085CE4(Unk80085B7C *this) {
    VECTOR offset = D_80068674;

    func_8007A4A4(&this->owner->base, &this->position, 18, &offset);
    this->unk44.vy = this->position.vy;
    this->unk44.vz = this->position.vz;
    this->unk44.vx = this->position.vx + (this->side << 7);
    this->unk1C += (0 - this->unk1C) >> 2;
    this->unk1D += (0x84 - this->unk1D) >> 2;
    if (this->state < 3 && this->owner->unk4A6 == 0) {
        this->state = 3;
    }
    switch (this->state) {
    case 0:
        this->state++;
        /* fallthrough */
    case 1:
        this->unk44.vy = (COS_SIN_TABLE[(this->timer << 10) & 0xC00].sin >> 7) + this->position.vy;
        this->unk54 += (0x3400 - this->unk54) >> 3;
        if (this->unk54 < 0x3300) {
            break;
        }
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 2:
        this->counter++;
        if (this->counter < 20) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 3:
        this->unk58 -= this->unk58 >> 2;
        if (this->unk58 >= 5) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 4:
        entityDelete(&this->entity);
        return;
    }
    this->timer++;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80085EF4);

/* The entity class of virtual table D_800686E4 (func_800862C0, 0x54 bytes): eight values with a scale
 * each. */
typedef struct Unk800862C0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20[8];
    /* 0x30 */ s16 unk30; /* func_8007768C of the character */
    /* 0x32 */ s16 unk32;
    /* 0x34 */ s32 unk34[8];
} Unk800862C0;

/* Constructs the entity of virtual table D_800686E4 for chara (the position is not used): 0x80 at 0x1C,
 * and its eight values at 0 with the scale 0x2800. */
Unk800862C0 *func_800862C0(Unk800862C0 *this, Unk80076F34 *chara, Vec3 *position) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800686E4;
    this->unk30 = func_8007768C(chara);
    this->chara = chara;
    this->unk1E = 0;
    this->unk32 = 0;
    this->unk1C = 0x80;
    for (i = 0; i < 8; i++) {
        this->unk34[i] = 0x2800;
        this->unk20[i] = 0;
    }
    return this;
}

/* The destructor of the entity class of vtable D_800686E4, which func_800862C0 installs. */
void func_8008634C(Entity *this, s32 flags) {
    this->vtable = &D_800686E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_800863A0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80086530);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_800868EC);

/* The destructor of the UnkPartsEntity38 class of vtables D_800686BC and D_8006869C, which
 * func_800868EC installs. */
void func_80086A80(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006869C;
    this->entity.vtable = &D_800686BC;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80086B20);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80086CB0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_01", func_80086E48);

INCLUDE_RODATA_ALIGN8("game/nonmatchings/fighters/fighter_01", D_8006869C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_800686BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_800686E4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_8006870C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_80068734);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_8006875C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_01", D_80068784);
