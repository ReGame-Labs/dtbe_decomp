/* One of the 24 fighters: its constructor, func_800ADA44, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

Unk800AED0C *func_800AED0C(Unk800AED0C *this, Unk80076F34 *arg);
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
Unk800B050C *func_800B050C(Unk800B050C *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
extern EntityVtable D_8006B5FC;
extern EntityVtable D_8006B624;
extern EntityVtable D_8006B64C;
extern VtableEntry D_8006B674[];
extern EntityVtable D_8006B694;
extern EntityVtable D_8006B6BC;
extern EntityVtable D_8006B6E4;
extern VtableEntry D_8006B70C[];
extern EntityVtable D_8006B72C;
extern EntityVtable D_8006B754;

typedef struct Unk800ADA44 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u8 unk4A4[4];
    /* 0x4A8 */ s32 unk4A8;
} Unk800ADA44;
void func_800AE16C(Unk80076F34 *this);

/* Constructs an entity of the class of virtual table D_8006B754, derived from func_80076F34's. */
Unk800ADA44 *func_800ADA44(Unk800ADA44 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B754;
    this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B754, which func_800ADA44 installs. */
void func_800ADA7C(Entity *this, s32 flags) {
    this->vtable = &D_8006B754;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_800ADAD0(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800ADAF8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800ADCE8);

void func_800ADFD4(Unk80076F34 *this);
void func_800ADE90(Unk80076F34 *this, s32 arg, s32 index);

/* The override of Unk80076F34's method at 0x118 for virtual table D_8006B754: func_8007BC1C and
 * func_80079AE4; arg 1 calls func_800ADFD4 and plays sound 0xE8; arg 10 calls func_800ADE90 for 0 to 6,
 * pushes x at 0x3B4 back by 0x3000 and plays sound 0xE2; up to 10 the vector at 0x3B4 slows by the one at
 * 0x3A4 (a sixteenth of x, twice a positive y). */
void func_800ADDA0(Unk80076F34 *this, s32 arg) {
    s32 i;

    func_8007BC1C(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800ADFD4(this);
        func_800784F4(this, 0xE8);
    }
    if (arg == 10) {
        for (i = 0; i < 7; i++) {
            func_800ADE90(this, 0x200, i);
        }
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
        func_80078540(this, 0xE2);
    }
    if (arg < 11) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy * 2;
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800ADE90);

/* Creates an entity of func_800AED0C's class (0x44 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800ADFD4(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_800AED0C(entityOperatorNewAt(0x44, child), this);
    }
    func_8007AD4C(this, child);
}

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006B754: func_8007B9C8, sets bit 6
 * of 0x37C; arg 1 calls func_800AE16C and plays sound 0xE7, arg 20 plays sound 0xE3, and below 21 the x
 * at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_800AE030(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800AE16C(this);
        func_800784F4(this, 0xE7);
    }
    if (arg == 20) {
        func_80078540(this, 0xE3);
    }
    if (arg < 21) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006B754: func_8007BBE8 and
 * func_80079AE4; arg 1 calls func_800AE16C and plays sound 0xE7, arg 20 plays sound 0xE3, and below 21 the
 * position at 0x3B4 moves back by a sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_800AE0C4(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800AE16C(this);
        func_800784F4(this, 0xE7);
    }
    if (arg == 20) {
        func_80078540(this, 0xE3);
    }
    if (arg < 21) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B5EC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AE16C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AE244);

/* Creates an entity of func_800AFD78's class (0x60 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE438(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x60);

    if (child != NULL) {
        func_800AFD78(entityOperatorNewAt(0x60, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B01AC's class (0x40 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE4A8(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B01AC(entityOperatorNewAt(0x40, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800B050C's class (0x40 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_800AE518(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800B050C(entityOperatorNewAt(0x40, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* The class func_800AE588 constructs (virtual tables D_8006B72C, and D_8006B70C for its base at 0x18): like
 * Unk800AD050, but it keeps its start position at 0x98 and the velocity's length at 0xC0. */
typedef struct Unk800AE588 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ Unk8006ED20 *list; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 start;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ Vec3 vel;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 speed; /* vecGetLength of vel */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s16 unkCC; /* func_8007768C of the owner */
    /* 0xCE */ s16 unkCE;
} Unk800AE588;

/* Constructs Unk800AE588 for chara at position, moving by velocity (its base gets velocity's x and z times 8
 * and y less 0xC00), and adds the part at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AE588 *func_800AE588(Unk800AE588 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list, s32 arg5) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, chara, 0x7AC0800, (Vec3 *)velocity, 1);
    this->parts.base.vtable = D_8006B70C;
    this->parts.entity.vtable = &D_8006B72C;
    func_80075EE4(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 3;
    v.vy -= 0xC00;
    v.vz <<= 3;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    sx = position->vx;
    sy = position->vy;
    sz = position->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkB8 = 0;
    this->unkBC = 0;
    this->speed = vecGetLength(velocity);
    this->unkC4 = 0;
    this->unkC8 = 0;
    this->unkCE = arg5;
    this->unkCC = func_8007768C(chara);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B72C and D_8006B70C, which
 * func_800AE588 installs. */
void func_800AE734(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B70C;
    this->entity.vtable = &D_8006B72C;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AE7D4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AE890);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AEAF0);

/* Constructs an entity of the class of vtable D_8006B6E4 for an Unk80076F34, with the vector
 * func_800775D8 gives. */
Unk800AED0C *func_800AED0C(Unk800AED0C *this, Unk80076F34 *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B6E4;
    this->unk18 = arg;
    func_800775D8(arg, &this->unk1C);
    this->unk2C = -0x180;
    this->unk30 = 0x80;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B6E4, which func_800AED0C installs. */
void func_800AED88(Entity *this, s32 flags) {
    this->vtable = &D_8006B6E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B6E4: follows the character's position 16 times, then deletes itself. */
void func_800AEDDC(Unk800AED0C *this, s32 arg) {
    if (this->unk38 >= 16) {
        entityDelete(&this->entity);
    } else {
        this->unk38++;
        func_800775D8(this->unk18, &this->unk1C);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AEE48);

/* The entity class of virtual table D_8006B6BC (constructor func_800AF0A0, spawned by func_800AE890). */
typedef struct Unk800AF0A0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 unk2C;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
} Unk800AF0A0;

/* Constructs the entity of virtual table D_8006B6BC with arg1 and copies of the two vectors. The match
 * depends on reading each vector's three words before storing any, as vecCopy does. */
Unk800AF0A0 *func_800AF0A0(Unk800AF0A0 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2) {
    s32 x, y, z;
    s32 x2, y2, z2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B6BC;
    this->unk18 = arg1;
    x = vec1->vx;
    y = vec1->vy;
    z = vec1->vz;
    this->unk1C.vx = x;
    this->unk1C.vy = y;
    this->unk1C.vz = z;
    x2 = vec2->vx;
    y2 = vec2->vy;
    z2 = vec2->vz;
    this->unk2C.vx = x2;
    this->unk2C.vy = y2;
    this->unk2C.vz = z2;
    this->unk3C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B6BC, which func_800AF0A0 installs. */
void func_800AF134(Entity *this, s32 flags) {
    this->vtable = &D_8006B6BC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of func_800AF0A0's entity: for 8 frames the vector at 0x1C moves by the one at 0x2C, then the
 * entity deletes itself. */
void func_800AF188(Unk800AF0A0 *this) {
    s32 x, y, z;

    if (this->unk3C < 8) {
        this->unk3C++;
        /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
        x = this->unk1C.vx;
        y = this->unk1C.vy;
        z = this->unk1C.vz;
        x += this->unk2C.vx;
        y += this->unk2C.vy;
        z += this->unk2C.vz;
        this->unk1C.vx = x;
        this->unk1C.vy = y;
        this->unk1C.vz = z;
    } else {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AF214);

/* The entity func_800AE16C spawns (virtual tables D_8006B694, and D_8006B674 for its base at 0x18). */
typedef struct Unk800AF364 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ Unk8006ED20 *list;
    /* 0x088 */ Vec3 position;
    /* 0x094 */ u8 unk94[0x98 - 0x94];
    /* 0x098 */ Vec3 velocity;
    /* 0x0A4 */ u8 unkA4[0x138 - 0xA4];
    /* 0x138 */ s32 unk138;
    /* 0x13C */ s32 unk13C;
    /* 0x140 */ s32 facing; /* the character's (func_80079E10) */
    /* 0x144 */ s32 speed; /* the velocity's length */
    /* 0x148 */ s32 unk148;
    /* 0x14C */ s32 unk14C;
    /* 0x150 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
    /* 0x152 */ u8 unk152[0x154 - 0x152];
    /* 0x154 */ s32 unk154;
} Unk800AF364;

/* Constructs func_800AE16C's entity for the character at position, flying at velocity, and adds its member
 * at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AF364 *func_800AF364(Unk800AF364 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x7A83000, (Vec3 *)velocity, 3);
    this->unk18.vtable = D_8006B674;
    this->entity.vtable = &D_8006B694;
    func_80075EE4(&this->unk38, position, 0x100, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->facing = func_80079E10(chara);
    this->unk138 = 0;
    this->unk13C = 0;
    this->speed = vecGetLength(velocity);
    this->unk148 = 0;
    this->unk14C = 0;
    this->vramY = func_8007768C(chara);
    this->unk154 = 0;
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    this->unk38.flags |= 1;
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B694 and D_8006B674, which
 * func_800AF364 installs. */
void func_800AF504(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B674;
    this->entity.vtable = &D_8006B694;
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

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes) at owner's position: NULL when the allocation
 * fails. */
static inline Entity *func_800AF5A4_newFlash(Unk800AF364 *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of func_800C62EC's burst (0x3C bytes, kind 4, eight particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *func_800AF5A4_newBurst(Unk800AF364 *owner, s32 radius) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 4, &owner->position, radius, 0xA0, 8, 0x808080);
    }
    return child;
}

/* The base at 0x18's first method for func_800AE16C's entity: func_8007BDF0 with sum; when sum holds an entity
 * of kind 0x10000 (func_8002BD68), sets 0x154 and bit 0 of the member at 0x38 and adds a flash at the position
 * when func_80078DE0 holds for the character's partner (func_8007AEEC), else a burst of 8 of radius
 * velocity.vx / 4. */
void func_800AF5A4(Unk800AF364 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->unk18, sum);
    if (sum->unk14 != NULL && func_8002BD68(sum->unk14) == 0x10000) {
        this->unk154 = 1;
        this->unk38.flags |= 1;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, func_80078DE0(func_8007AEEC(this->chara))
                                            ? func_800AF5A4_newFlash(this)
                                            : func_800AF5A4_newBurst(this, this->velocity.vx >> 2));
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AF6F0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AF970);

/* Constructs an entity of the class of vtable D_8006B64C for an Unk80076F34 and its
 * func_80077580 object. */
Unk800AFD78 *func_800AFD78(Unk800AFD78 *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B64C;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk5E = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B64C, which func_800AFD78 installs. */
void func_800AFDE8(Entity *this, s32 flags) {
    this->vtable = &D_8006B64C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AFE3C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800AFF40);

/* Constructs an entity of the class of vtable D_8006B624 for an Unk80076F34 and its
 * func_80077580 object. */
Unk800B01AC *func_800B01AC(Unk800B01AC *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B624;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B624, which func_800B01AC installs. */
void func_800B021C(Entity *this, s32 flags) {
    this->vtable = &D_8006B624;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B624: moves 0x24 down by 0x120, 24 times, then deletes it. */
void func_800B0270(Unk800B01AC *this, s32 arg) {
    if (this->unk34 >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk34++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800B02E0);

/* Constructs an entity of the class of vtable D_8006B5FC for an Unk80076F34 and its
 * func_80077580 object. */
Unk800B050C *func_800B050C(Unk800B050C *this, Unk80076F34 *arg, Unk8006ED20 *arg2) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B5FC;
    this->unk18 = arg;
    this->unk1C = arg2;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = func_8007768C(arg);
    return this;
}

/* The destructor of the entity class of vtable D_8006B5FC, which func_800B050C installs. */
void func_800B057C(Entity *this, s32 flags) {
    this->vtable = &D_8006B5FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of the entity of D_8006B5FC: moves 0x24 down by 0x120, 16 times, then deletes it. */
void func_800B05D0(Unk800B01AC *this, s32 arg) {
    if (this->unk34 >= 16) {
        entityDelete(&this->entity);
    } else {
        this->unk34++;
        this->unk24 -= 0x120;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_15", func_800B0640);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B5FC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B624);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B64C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B674);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B694);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B6BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B6E4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B70C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B72C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_15", D_8006B754);
