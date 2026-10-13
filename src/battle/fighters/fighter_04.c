/* One of the 24 fighters: its constructor, func_80089FB8, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

struct Unk8008A9BC *func_8008A9BC(struct Unk8008A9BC *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                                  Unk8006ED20 *list);
struct Unk8008B124 *func_8008B124(struct Unk8008B124 *this, Unk80076F34 *chara);
extern EntityVtable D_80068D84;
extern EntityVtable D_80068DAC;
extern EntityVtable D_80068DD4;
extern EntityVtable D_80068DFC;
extern VtableEntry D_80068E24[];
extern EntityVtable D_80068E44;
extern EntityVtable D_80068E6C;
extern VtableEntry D_80068E94[];
extern EntityVtable D_80068EB4;
extern EntityVtable D_80068EDC;

typedef struct Unk80089FB8 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ s16 unk4A6;
    /* 0x4A8 */ s16 unk4A8;
} Unk80089FB8;
void func_8008A384(struct Unk80089FB8 *this);
void func_8008A44C(Unk80076F34 *this);
/* an offset from a node of the character's model */
extern VECTOR D_80010824; /* a zero offset in the executable's .rodata */

/* Constructs an entity of the class of virtual table D_80068EDC, derived from func_80076F34's. */
Unk80089FB8 *func_80089FB8(Unk80089FB8 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80068EDC;
    this->unk4A4 = 0;
    this->unk4A6 = 0;
    this->unk4A8 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068EDC, which func_80089FB8 installs. */
void func_80089FF8(Entity *this, s32 flags) {
    this->vtable = &D_80068EDC;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008A04C);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_80068EDC: func_8007B9C8; arg 1 plays
 * sound 0x92 and calls func_8008A44C, arg 6 calls func_8008A384 and plays sound 0x8D, and below 7 the x
 * at 0x3B4 moves by a sixteenth of the x at 0x3A4, backwards. */
void func_8008A218(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800784F4(this, 0x92);
        func_8008A44C(this);
    }
    if (arg == 6) {
        func_8008A384((Unk80089FB8 *)this);
        func_80078540(this, 0x8D);
    }
    if (arg < 7) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_80068EDC: func_8007BBE8, sets bit 6 of
 * 0x37C; arg 1 plays sound 0x92 and calls func_8008A44C; arg 6 calls func_8008A384, plays sound 0x8D and pushes vx at 0x3B4 back by
 * 0x3000; below 7 the vector at 0x3B4 slows by the one at 0x3A4 (a sixteenth of x, all of a positive y). */
void func_8008A2B4(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 1) {
        func_800784F4(this, 0x92);
        func_8008A44C(this);
    }
    if (arg == 6) {
        func_8008A384((Unk80089FB8 *)this);
        func_80078540(this, 0x8D);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 7) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_8008A9BC's entity (0x2CC bytes) at the character's point 12, flying 96 a frame toward
 * func_80079E10's side (kept at 0x4A4), hands it to func_8007AD4C and calls func_80079B0C(30). */
void func_8008A384(Unk80089FB8 *this) {
    Vec3 position;
    VECTOR velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(&this->base, (VECTOR *)&position, 12, &D_80010824);
    this->unk4A4 = func_80079E10(&this->base);
    velocity.vx = this->unk4A4 * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0x2CC);
    if (child != NULL) {
        func_8008A9BC(entityOperatorNewAt(0x2CC, child), &this->base, &position, &velocity, obj);
    }
    func_8007AD4C(&this->base, child);
    func_80079B0C(&this->base, 30);
}

/* Creates an entity of func_8008B124's class (0x48 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_8008A44C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_8008B124(entityOperatorNewAt(0x48, child), this);
    }
    func_8007AD4C(this, child);
}
void func_8008A628(Unk80089FB8 *this, s32 angle);

/* A virtual function of Unk80089FB8: func_8007B9FC; at step 0 clears 0x4A8, func_80079B14(60) and sounds 0x93
 * and 0x8E; at steps 3 to 25 func_8008A628 turned to the facing (0 or 0x800) and the position at 0x3B4 moves
 * back by a sixteenth of the x at 0x3A4. */
void func_8008A4A8(Unk80089FB8 *this, s32 step) {
    func_8007B9FC(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        func_800784F4(&this->base, 0x93);
        func_80078540(&this->base, 0x8E);
    }
    if (step >= 3 && step < 26) {
        if (func_80079E10(&this->base) > 0) {
            func_8008A628(this, 0);
        } else {
            func_8008A628(this, 0x800);
        }
        this->base.unk3B4.vx += -this->base.unk3A4.vx >> 4;
    }
}

/* func_8008A4A8 on func_8007BC1C, below step 26, and the position at 0x3B4 also moves back by the y at 0x3A8
 * when that is positive. */
void func_8008A560(Unk80089FB8 *this, s32 step) {
    func_8007BC1C(&this->base, step);
    func_80079AE4(&this->base);
    if (step == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        func_800784F4(&this->base, 0x93);
        func_80078540(&this->base, 0x8E);
    }
    if (step < 26) {
        if (func_80079E10(&this->base) > 0) {
            func_8008A628(this, 0);
        } else {
            func_8008A628(this, 0x800);
        }
        this->base.unk3B4.vx -= this->base.unk3A4.vx >> 4;
        if (this->base.unk3A4.vy > 0) {
            this->base.unk3B4.vy -= this->base.unk3A4.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068D64);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008A628);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008A730);

struct Unk8008B62C *func_8008B62C(struct Unk8008B62C *this, Unk80076F34 *chara, Vec3 *position);

/* Spawns func_8008B62C's entity (0x38 bytes) at the position of func_8007AEEC's character, moved by a
 * random -0x20 to 0x1F times 9 in x (the generator of the object at 0x228), and hands it to
 * func_8007AD4C. */
void func_8008A91C(Unk80089FB8 *this) {
    Unk8006ED20 *obj = func_80077580(&this->base);
    Vec3 position;
    Entity *child;

    func_800775D8(func_8007AEEC(&this->base), &position);
    position.vx += ((s32)(mersenneTwisterGenerate(&obj->random) & 0x3F) - 0x20) * 9;
    child = allocEntityMemory(0x38);
    if (child != NULL) {
        func_8008B62C(entityOperatorNewAt(0x38, child), &this->base, &position);
    }
    func_8007AD4C(&this->base, child);
}

/* The entity func_8008A384 spawns (virtual tables D_80068EB4, and D_80068E94 for its base at 0x18). */
typedef struct Unk8008A9BC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk8007BD8C unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ Unk80075EE4 unk38;
    /* 0x080 */ Unk80076F34 *chara;
    /* 0x084 */ Vec3 position;
    /* 0x090 */ u8 unk90[0x2A4 - 0x90];
    /* 0x2A4 */ Vec3 velocity;
    /* 0x2B0 */ u8 unk2B0[0x2B4 - 0x2B0];
    /* 0x2B4 */ s32 unk2B4;
    /* 0x2B8 */ s32 unk2B8;
    /* 0x2BC */ s32 speed; /* the velocity's length */
    /* 0x2C0 */ s32 facing; /* the character's (func_80079E10) */
    /* 0x2C4 */ s32 unk2C4;
    /* 0x2C8 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008A9BC;

/* Constructs func_8008A384's entity for the character at position, flying at velocity, and adds its member
 * at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk8008A9BC *func_8008A9BC(Unk8008A9BC *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x6A81800, (Vec3 *)velocity, 2);
    this->unk18.vtable = D_80068E94;
    this->entity.vtable = &D_80068EB4;
    func_80075EE4(&this->unk38, position, 0xC0, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
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
    this->unk2B4 = 0;
    this->unk2B8 = 0;
    this->speed = vecGetLength(velocity);
    this->facing = func_80079E10(this->chara);
    this->unk2C4 = 0;
    this->vramY = func_8007768C(chara);
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_80068EB4 and D_80068E94, which
 * func_8008A9BC installs. */
void func_8008AB44(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068E94;
    this->entity.vtable = &D_80068EB4;
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
static inline Entity *func_8008ABE4_newFlash(Unk8008A9BC *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of func_800C62EC's burst (0x3C bytes, kind 4, five particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *func_8008ABE4_newBurst(Unk8008A9BC *owner) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 4, &owner->position, 0x20, 0x100, 5, 0x808080);
    }
    return child;
}

/* The base at 0x18's first method for func_8008A9BC's class: func_8007BDF0 with sum; when sum holds an entity
 * of kind 0x10000 (func_8002BD68), sets bit 0 of the member at 0x38 and adds a flash at the position when
 * func_80078DE0 holds for it, else a burst of 5. */
void func_8008ABE4(Unk8008A9BC *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->unk18, sum);
    target = sum->unk14;
    if (target != NULL && func_8002BD68(target) == 0x10000) {
        this->unk38.flags |= 1;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, func_80078DE0((Unk80076F34 *)target) ? func_8008ABE4_newFlash(this)
                                                                             : func_8008ABE4_newBurst(this));
    }
}

/* The update of func_8008A384's entity: flies by its velocity, its member at 0x38 following, for 65 frames,
 * then deletes itself.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x2C4. */
void func_8008AD20(Unk8008A9BC *this) {
    if (this->unk2C4 > 64) {
        entityDelete(&this->entity);
    } else {
        s32 x = this->position.vx;
        s32 y = this->position.vy;
        s32 z = this->position.vz;

        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
        this->unk38.unk34.vx = x;
        this->unk38.unk34.vy = y;
        this->unk38.unk34.vz = z;
        this->unk2C4++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008ADC4);

/* The entity class of virtual table D_80068E6C (constructor func_8008B124). */
typedef struct Unk8008B124 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position; /* the character's (func_800775D8) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 unk44; /* func_8007768C of the character */
} Unk8008B124;

/* Constructs the entity of virtual table D_80068E6C for chara, at its position. */
Unk8008B124 *func_8008B124(Unk8008B124 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068E6C;
    this->chara = chara;
    func_800775D8(chara, &this->position);
    this->unk2C = 0x40;
    this->unk30 = 0x40;
    this->unk34 = 0x80;
    this->unk38 = 0;
    this->unk3C = 0;
    this->unk40 = 0;
    this->unk44 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068E6C, which func_8008B124 installs. */
void func_8008B1A4(Entity *this, s32 flags) {
    this->vtable = &D_80068E6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of D_80068E6C's entity: counts 0x3C up to 10, then deletes itself. The match depends on taking
 * the Entity the virtual call passes and its class in a local: a pointer param's REG_EQUIV doubles its live
 * length (-dl: 20 insns, 10 for the local), so global-alloc would give the count $v1 before `this`. */
void func_8008B1F8(Entity *entity) {
    Unk8008B124 *this = (Unk8008B124 *)entity;

    if (this->unk3C < 10) {
        this->unk3C++;
    } else {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068D74);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008B25C);

/* The entity class of virtual table D_80068DAC (constructor func_8008B62C, made by func_8008A91C). */
typedef struct Unk8008B62C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x30 - 0x28];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s16 unk34; /* func_8007768C of the character */
} Unk8008B62C;

/* Constructs the entity of virtual table D_80068DAC for chara at position. The match depends on reading
 * the three words before storing any, as vecCopy does, and on storing chara between the reads and the
 * writes. */
Unk8008B62C *func_8008B62C(Unk8008B62C *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DAC;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->chara = chara;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk30 = 0;
    this->unk34 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068DAC, which func_8008B62C installs. */
void func_8008B6A8(Entity *this, s32 flags) {
    this->vtable = &D_80068DAC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008B6FC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008B860);

/* The entity of func_8008BE04 (vtable D_80068D84, 0x34 bytes): a character's, with its 0x440 (func_8007768C). */
typedef struct Unk8008BE04 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position; /* the character's (func_800775D8) */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008BE04; /* size 0x34 */

/* Constructs the character's entity of vtable D_80068D84. */
Unk8008BE04 *func_8008BE04(Unk8008BE04 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068D84;
    this->chara = chara;
    this->unk2C = 0;
    this->vramY = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068D84, which func_8008BE04 installs. */
void func_8008BE5C(Entity *this, s32 flags) {
    this->vtable = &D_80068D84;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of D_80068D84's entity: follows the character's position, and deletes itself after 24
 * frames (count at 0x2C).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x2C. */
void func_8008BEB0(Unk8008BE04 *this) {
    func_800775D8(this->chara, &this->position);
    if (this->unk2C >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk2C++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008BF2C);

/* The class func_8008C20C constructs (virtual tables D_80068E44, and D_80068E24 for its base at 0x18) for an
 * Unk80089FB8 character: like Unk800ACA60, with a flag at 0xBA. */
typedef struct Unk8008C20C {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80089FB8 *owner;
    /* 0x84 */ Unk8006ED20 *list; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 vel;
    /* 0xA4 */ u8 unkA4[0xB4 - 0xA4];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s16 unkB8; /* func_8007768C of the owner */
    /* 0xBA */ u16 unkBA; /* set, with the part's flag 1, when made while the owner's 0x4A8 is set */
} Unk8008C20C;

/* Constructs Unk8008C20C for chara at position, moving by velocity (its base gets it times 4), adds the part
 * at 0x38 to one of list's two lists (func_80077574 picks which), and flags it if chara's 0x4A8 is set. */
Unk8008C20C *func_8008C20C(Unk8008C20C *this, Unk80089FB8 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, &chara->base, 0x2AC0D00, (Vec3 *)velocity, 0);
    this->parts.base.vtable = D_80068E24;
    this->parts.entity.vtable = &D_80068E44;
    func_80075EE4(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 2;
    v.vy <<= 2;
    v.vz <<= 2;
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
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->unkB4 = 0;
    this->unkBA = 0;
    this->unkB8 = func_8007768C(&chara->base);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(&this->owner->base) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    if (this->owner->unk4A8 != 0) {
        this->unkBA = 1;
        this->parts.part.flags |= 1;
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_80068E44 and D_80068E24, which
 * func_8008C20C installs. */
void func_8008C3A0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80068E24;
    this->entity.vtable = &D_80068E44;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008C440);

/* The update of D_80068E44's part: flagged (0xBA and the part's flag 1) while the owner's 0x4A8 is set,
 * then it slows by a quarter a frame; moves by its velocity with the part following, flags itself after
 * 32 frames and deletes itself after 40.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0xB4. */
void func_8008C620(Unk8008C20C *this) {
    s32 x, y, z;

    if (this->owner->unk4A8 != 0) {
        this->unkBA = 1;
        this->parts.part.flags |= 1;
    }
    if (this->unkBA) {
        this->vel.vx -= this->vel.vx >> 2;
        this->vel.vy -= this->vel.vy >> 2;
        this->vel.vz -= this->vel.vz >> 2;
    }
    x = this->pos.vx;
    y = this->pos.vy;
    z = this->pos.vz;
    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    if (++this->unkB4 > 40) {
        entityDelete(&this->parts.entity);
    } else if (this->unkB4 > 32) {
        this->unkBA = 1;
        this->parts.part.flags |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008C748);

/* The entity class of virtual table D_80068DFC (func_8008C94C, made by func_8008C440): a frame count at
 * 0x28. */
typedef struct Unk8008C94C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* func_8007A6CC of the character */
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008C94C;

/* Constructs the entity of virtual table D_80068DFC for chara, at func_8007A6CC's position. */
Unk8008C94C *func_8008C94C(Unk8008C94C *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DFC;
    func_8007A6CC(chara, &this->position);
    this->vramY = func_8007768C(chara);
    this->unk28 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068DFC, which func_8008C94C installs. */
void func_8008C9AC(Entity *this, s32 flags) {
    this->vtable = &D_80068DFC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of D_80068DFC's entity: deletes itself after 24 frames (count at 0x28).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x28. */
void func_8008CA00(Unk8008C94C *this) {
    if (this->unk28 >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk28++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008CA74);

/* The entity class of virtual table D_80068DD4 (func_8008CE68, 0x40 bytes, made by func_8008C440). */
typedef struct Unk8008CE68 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* func_8007A6CC of the character */
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s16 vramY; /* the character's 0x440 (func_8007768C) */
} Unk8008CE68;

/* Constructs the entity of virtual table D_80068DD4 for chara, at func_8007A6CC's position, with 0x28 and
 * 0x2C random in -0x40 to 0x3E (list's generator). It reads the character's 0x440 twice. */
Unk8008CE68 *func_8008CE68(Unk8008CE68 *this, Unk80076F34 *chara, Unk8006ED20 *list) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DD4;
    func_8007A6CC(chara, &this->position);
    this->vramY = func_8007768C(chara);
    this->unk28 = ((s32)(mersenneTwisterGenerate(&list->random) & 0x3F) - 0x20) * 2;
    this->unk2C = ((s32)(mersenneTwisterGenerate(&list->random) & 0x3F) - 0x20) * 2;
    this->unk30 = 0;
    this->unk38 = 0;
    this->vramY = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80068DD4, which func_8008CE68 installs. */
void func_8008CF18(Entity *this, s32 flags) {
    this->vtable = &D_80068DD4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008CF6C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_04", func_8008D028);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068D84);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068DAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068DD4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068DFC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068E24);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068E44);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068E6C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068E94);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068EB4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_04", D_80068EDC);
