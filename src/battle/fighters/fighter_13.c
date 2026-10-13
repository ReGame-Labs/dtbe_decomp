/* One of the 24 fighters: its constructor, func_800A54C8, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

void func_800A56CC(Unk80076F34 *this, s32 arg);
void func_800A7290(Entity *this, s32 arg);
void func_800A7458(Entity *this, s32 arg);
void func_800A8C24(Entity *this, s32 arg);
void func_800A8F80(Entity *this, s32 arg);
struct Unk800A6220 *func_800A6220(struct Unk800A6220 *this, Unk80076F34 *chara);
extern VtableEntry D_8006AF2C[];
extern EntityVtable D_8006AF4C;
extern VtableEntry D_8006AFC4[];
extern EntityVtable D_8006AFE4;
extern EntityVtable D_8006B00C;
extern EntityVtable D_8006B03C;

/* The class func_800A54C8 constructs (virtual table D_8006B06C), derived
 * from func_80076F34's: func_800A58F4 keeps four results of func_800A5B98 at
 * 0x4A8-0x4B4. */
typedef struct Unk800A54C8 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
} Unk800A54C8;

extern EntityVtable D_8006B06C;

/* The classes of D_8006AF2C and D_8006AE8C (virtual tables of their base at
 * 0x18), whose methods func_800A7F88 and func_800A9944 are. */
typedef struct Unk800A7F88 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x44 - 0x34];
    /* 0x44 */ u32 unk44;
    /* 0x48 */ u8 unk48[0x94 - 0x48];
    /* 0x94 */ Vec3 unk94;
    /* 0xA0 */ u8 unkA0[0xA8 - 0xA0];
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
} Unk800A7F88;

extern VtableEntry D_8006AE8C[];
extern EntityVtable D_8006AEAC;

typedef struct Unk800A8858 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C[11];
    /* 0x48 */ s32 unk48; /* the character's 0x440 (func_8007768C) */
} Unk800A8858;

extern EntityVtable D_8006AEDC;

typedef struct Unk800AA04C {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ Vec3 unk20;
} Unk800AA04C;
extern Unk800C5B18Entry D_8006B1AC[];

/* Constructs an Unk800A54C8: func_80076F34's entity with virtual table
 * D_8006B06C, its seven words 0. The game clears 0x4A8-0x4B4 last to first:
 * the match depends on the chained assignment. */
Unk800A54C8 *func_800A54C8(Unk800A54C8 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B06C;
    this->unk4B8 = 0;
    this->unk4A4 = 0;
    this->unk4A8 = this->unk4AC = this->unk4B0 = this->unk4B4 = 0;
    this->unk4BC = 0;
    return this;
}

/* The method at 0x20 of virtual table D_8006B06C: func_80079B0C with 20 while 0x4A4 is set. */
void func_800A5518(Unk800A54C8 *this) {
    if (this->unk4A4 != 0) {
        func_80079B0C(&this->base, 20);
    }
}

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5548(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5588(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 20) when arg is 0. */
void func_800A55C8(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 20);
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 10) when arg is 0. */
void func_800A5608(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 10);
    }
}

/* Overrides func_8007B884: calls it, then func_800A56CC(8) when arg is 15. */
void func_800A5648(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 15) {
        func_800A56CC(this, 8);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in vtable D_8006B06C:
 * calls it, then func_800A56CC(this, 17) when arg is 0. */
void func_800A568C(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 0) {
        func_800A56CC(this, 17);
    }
}

/* Creates func_800C5680's entity (0x690 bytes) for the character with D_8006B1AC, 3, 2 and arg, and
 * hands it to func_8007AD4C. */
void func_800A56CC(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_8006B1AC, 3, 2, arg);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A574C);
extern VECTOR D_8006AB8C;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AB8C);

/* func_80079E50 with D_8006AB8C, 0x100000 and 0x2AA. */
void func_800A58C8(Unk80076F34 *this) {
    func_80079E50(this, &D_8006AB8C, 0x100000, 0x2AA);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AB9C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ABAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ABBC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ABCC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A58F4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A5AD0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A5B98);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A5D44);

void func_800A5F94(Unk800A54C8 *this);
void func_800A605C(Unk800A54C8 *this, Vec3 *velocity);

/* Overrides func_8007BC1C in D_8006B06C: calls it; arg 0 plays sound 0xD9, arg 4 runs func_800A5F94, arg 16
 * throws 0x4B8's entity (func_800A605C) forward by 128 and down by 128. Then 0x3B4.vx brakes to a quarter
 * of -0x3A4.vx, and a positive 0x3A4.vy is taken off with 0x71C scaled by 0x3C4 (16.16). */
void func_800A5EA0(Unk800A54C8 *this, s32 arg) {
    Vec3 velocity;

    func_8007BC1C(&this->base, arg);
    switch (arg) {
    case 0:
        func_800784F4(&this->base, 0xD9);
        break;
    case 4:
        func_800A5F94(this);
        break;
    case 16:
        velocity.vx = func_80079E10(&this->base) * 128;
        velocity.vy = 128;
        velocity.vz = 0;
        func_800A605C(this, &velocity);
        break;
    }
    this->base.unk3B4.vx = -this->base.unk3A4.vx >> 2;
    if (this->base.unk3A4.vy > 0) {
        /* The match depends on lift being a local of its own (computed inline, other registers) */
        s32 lift = ((s64)this->base.unk3C4 * 0x71C) >> 16;

        this->base.unk3B4.vy = -this->base.unk3A4.vy - lift;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A5F94);

void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8);

/* Throws the entity of handle 0x4B8, when it still exists (func_800A80B4 with velocity), and plays sound
 * 0xD2. */
void func_800A605C(Unk800A54C8 *this, Vec3 *velocity) {
    Entity *entity = getEntityByHandle(this->unk4B8);

    if (entity != NULL) {
        func_800A80B4((Unk800A7F88 *)entity, (VECTOR *)velocity, 0x47AC1400);
        func_80078540(&this->base, 0xD2);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A60B4);

/* the offset from node 19 where func_800A6194 creates its entity */
extern VECTOR D_8006B198;

Unk800A8858 *func_800A8858(Unk800A8858 *this, Unk80076F34 *chara, VECTOR *position);

/* Creates func_800A8858's entity (0x4C bytes) at node 19 (offset D_8006B198), hands it to func_8007AD4C, then
 * func_80077448(-0x10000) and plays sound 0xD3. */
void func_800A6194(Unk80076F34 *this) {
    VECTOR position;
    Entity *child;

    func_8007A4A4(this, &position, 19, &D_8006B198);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        func_800A8858(entityOperatorNewAt(0x4C, child), this, &position);
    }
    func_8007AD4C(this, child);
    func_80077448(this, -0x10000);
    func_80078540(this, 0xD3);
}

/* The entity class of func_800A6220 (virtual table D_8006B03C), which func_800AA0B4's derives from. */
typedef struct Unk800A6220 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk80075000 *input; /* the character's (func_800776C4) */
    /* 0x20 */ Vec3 position;      /* the character's, at construction */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ s32 unk30; /* 96 the way the character faces */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800A6220;

/* Constructs an entity that follows the character: its input, its position, and 96 the way it faces. */
Unk800A6220 *func_800A6220(Unk800A6220 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B03C;
    this->chara = chara;
    this->input = func_800776C4(chara);
    func_8007A650(chara, &this->position);
    this->unk30 = func_80079E10(chara) * 96;
    this->unk34 = 0;
    this->unk38 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A62A0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A63D8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6550);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A65F8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6778);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A68FC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6AB0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6BE4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6CF4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6EB8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A6F8C);

/* The update of the entities of D_8006AF9C: func_800A7290, then func_800A7458. */
void func_800A7254(Entity *this, s32 arg) {
    func_800A7290(this, arg);
    func_800A7458(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7290);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7458);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7694);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7768);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A79D0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7C90);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7DD4);

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44 when func_80078878 finds an entity in arg and 0xAC is 0. */
void func_800A7F88(Unk800A7F88 *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL && this->unkAC == 0) {
        this->unk44 |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A7FEC);

/* Keeps vec at 0x94, clears 0xA8 and 0xB0, counts the call in 0xAC, and sets the second base's vector to
 * vec's unit direction with x doubled, y quadrupled and z cleared, and its word at 0x8 to unk8. */
void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8) {
    VECTOR dir;
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unkA8 = 0;
    this->unkB0 = 0;
    this->unk94.vx = x;
    this->unk94.vy = y;
    this->unk94.vz = z;
    this->unkAC++;
    vecNormalize(&dir, vec);
    dir.vx <<= 1;
    dir.vy <<= 2;
    dir.vz = 0;
    func_8007BDD4(&this->unk18, &dir);
    func_8007BDC8(&this->unk18, unk8, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A8150);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AC5C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A8274);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A83E0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A84E4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A86A8);

/* Constructs an Unk800A8858 for the character, its fields 0; its one caller (func_800A6194) also passes a
 * position, which it does not read. */
Unk800A8858 *func_800A8858(Unk800A8858 *this, Unk80076F34 *chara, VECTOR *position) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AEDC;
    this->chara = chara;
    this->unk1C[0] = 0;
    this->unk1C[1] = 0;
    this->unk1C[2] = 0;
    this->unk1C[3] = 0;
    this->unk1C[4] = 0;
    this->unk1C[5] = 0;
    this->unk1C[6] = 0;
    this->unk1C[7] = 0;
    this->unk1C[8] = 0;
    this->unk1C[9] = 0;
    this->unk1C[10] = 0;
    this->unk48 = func_8007768C(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A88D8);

/* The update of the entities of D_8006AEDC: func_800A8C24, then func_800A8F80. */
void func_800A8BE8(Entity *this, s32 arg) {
    func_800A8C24(this, arg);
    func_800A8F80(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AC7C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A8C24);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ACC4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ACCC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A8F80);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006ACD4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A926C);

/* The destructor of func_800A926C's class: stops its sound, takes its member at 0x38 out of its list. */
void func_800A9428(Unk800A9944 *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006AE8C;
    this->entity.vtable = &D_8006AEAC;
    stopSound(this->unkD4);
    this->unk38.vtable = D_80065184;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.node;
    this->unk38.node.next->prev = this->unk38.node.prev;
    link->prev->next = this->unk38.node.next;
    this->unk38.node.next = link;
    this->unk38.node.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0xD4 and keeps its handle at 0xD4 (func_800A926C's class; the destructor stops it). */
void func_800A94D0(Unk800A9944 *this) {
    this->unkD4 = soundControlPlaySound(&SOUND_CONTROL, 0xD4, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A9508);

/* The second base's func_8007BDF0 with arg, then halves 0xB4 when func_80078878 finds an entity in arg. */
void func_800A9944(Unk800A9944 *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    if (func_80078878(arg) != NULL) {
        this->unkB4 >>= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A9998);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A9C4C);

/* The class func_800A9C4C constructs (virtual table D_8006AE64). */
typedef struct Unk800A9C4C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* three mersenneTwisterGenerate values */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38; /* eases towards 0x804 (func_800A9D0C) */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40; /* func_8007768C of the owner */
} Unk800A9C4C;

/* The update of func_800A9C4C's entity: 0x38 eases a quarter of the way to 0x804 every frame, and the
 * entity deletes itself once it reaches 0x800. */
void func_800A9D0C(Unk800A9C4C *this) {
    this->unk38 += (0x804 - this->unk38) >> 2;
    if (this->unk38 >= 0x800) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AE40);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_13", func_800A9D70);

/* The destructor of the entity class of vtable D_8006B06C, which func_800A54C8 installs. */
void func_800A9FBC(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006B03C, which func_800A6220 installs. */
void func_800AA004(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Copies the vector at 0x20 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800AA04C(Unk800AA04C *this, Vec3 *out) {
    s32 x = this->unk20.vx;
    s32 y = this->unk20.vy;
    s32 z = this->unk20.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* The destructor of the entity class of vtable D_8006B00C, which func_800A5AD0 and func_800AA0B4 install. */
void func_800AA06C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006B00C, which derives from func_800A6220's. */
Unk800A6220 *func_800AA0B4(Unk800A6220 *this, Unk80076F34 *chara) {
    func_800A6220(this, chara);
    this->entity.vtable = &D_8006B00C;
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AFE4 and D_8006AFC4, which
 * func_800A65F8 installs. */
void func_800AA0E8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AFC4;
    this->entity.vtable = &D_8006AFE4;
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

/* The destructor of the entity class of vtable D_8006AF9C, which func_800A6EB8 installs. */
void func_800AA188(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF74, which func_800A7694 installs. */
void func_800AA1D0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AF4C and D_8006AF2C, which
 * func_800A7C90 installs. */
void func_800AA218(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AF2C;
    this->entity.vtable = &D_8006AF4C;
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

/* The destructor of the entity class of vtable D_8006AF04, which func_800A83E0 installs. */
void func_800AA2B8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AEDC, which func_800A8858 installs. */
void func_800AA300(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AE64, which func_800A9C4C installs. */
void func_800AA348(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AE64);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AE8C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AEAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AEDC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AF04);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AF2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AF4C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AF74);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AF9C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AFC4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006AFE4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006B00C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006B03C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006B06C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006B198);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_13", D_8006B1AC);
