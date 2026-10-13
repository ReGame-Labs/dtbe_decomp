/* One of the 24 fighters: its constructor, func_800BF0B4, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/sound/sound_control.h"

void func_800BF0E8(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_800BF238(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3);
s32 func_800BF3B8(Unk80076F34 *this);

/* whether the character's fighter (by func_80077574) has the other colors (altColor) */
s32 func_800BF37C(Unk80076F34 *this);
Entity *func_800C06D0(Entity *this, Unk80076F34 *arg);
Entity *func_800C0CC0(Entity *this, Unk80076F34 *arg, Vec3 *vec);
extern EntityVtable D_8006CDD4;
extern EntityVtable D_8006CDFC;
extern EntityVtable D_8006CE4C;
extern s32 D_8006CF74[];
extern s32 D_8006CF7C[];
void func_800C0084(Unk80076F34 *this);
void func_800C00EC(Unk80076F34 *this);
void func_800C0210(Unk80076F34 *this);

/* Constructs an object of the entity class of vtable D_8006CE4C, which derives from func_80076F34's. */
Unk80076F34 *func_800BF0B4(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006CE4C;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CC7C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CCC4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800BF0E8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800BF238);

/* Whether the character's fighter wears the other colors (both sides look alike). */
s32 func_800BF37C(Unk80076F34 *this) {
    return GAME_STATE.fighters[func_80077574(this)].altColor;
}

/* Returns the first or the second word of D_8006CF74, by whether func_800BF37C gives non-zero. */
s32 func_800BF3B8(Unk80076F34 *this) {
    return D_8006CF74[func_800BF37C(this) != 0];
}

/* Returns the first or the second word of D_8006CF7C, by whether func_800BF37C gives non-zero. */
s32 func_800BF3F0(Unk80076F34 *this) {
    return D_8006CF7C[func_800BF37C(this) != 0];
}

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF238 when arg is 0. */
void func_800BF428(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 0) {
        func_800BF238(this, 2, 4, func_800BF3B8(this));
    }
}

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 0. */
void func_800BF47C(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 0) {
        func_800BF0E8(this, 2, 9, func_800BF3B8(this), 0);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in D_8006CE4C: calls it, then func_800BF0E8 when
 * arg is 2; at 8 sets 0x44C to word 0x20 of func_80077698's entry 2 with bit 31 set. */
void func_800BF4D4(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 4, 0x12, func_800BF3B8(this), 0);
    }
    if (arg == 8) {
        this->unk44C = func_80077698(this)->entries[2].unk20 | 0x80000000;
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF238 when arg is 0. */
void func_800BF554(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800BF238(this, 2, 10, func_800BF3B8(this));
    }
}

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 0. */
void func_800BF5A8(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 0) {
        func_800BF0E8(this, 2, 17, func_800BF3B8(this), 0);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in D_8006CE4C: calls it,
 * then func_800BF0E8 when arg is 2. */
void func_800BF600(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 12, func_800BF3B8(this), 1);
    }
}

/* Overrides func_8007BB64 (entry 0x10C of Unk80076F34's vtable) in D_8006CE4C: calls it, then
 * func_800BF0E8 when arg is 2. */
void func_800BF660(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 16, func_800BF3B8(this), 0);
    }
}

/* Overrides func_8007BA98 (entry 0xFC of Unk80076F34's vtable) in D_8006CE4C: calls it, then
 * func_800BF0E8 when arg is 2. */
void func_800BF6BC(Unk80076F34 *this, s32 arg) {
    func_8007BA98(this, arg);
    if (arg == 2) {
        func_800BF0E8(this, 2, 8, func_800BF3B8(this), 0);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD0C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800BF718);

void func_800BFB6C(Unk80076F34 *this);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006CE4C: func_8007B9C8 and func_8007835C(0);
 * arg 0 resets 0x44C..0x460, sets 0x498 to 4 and plays sound 0x118; arg 4 spawns func_800BFB6C's entity and calls
 * func_80079B0C(0x3C); arg 14 calls func_800BF238; then sets 0x448 to 0x20. */
void func_800BF9D0(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_8007835C(this, 0);
    if (arg == 0) {
        this->unk44C = 0x800000;
        this->unk454 = 0;
        this->unk458 = 0;
        this->unk45C = 0;
        this->unk460 = 0;
        this->unk450 = 1;
        this->unk498 = 4;
        func_800784F4(this, 0x118);
    }
    if (arg == 4) {
        func_800BFB6C(this);
        func_80079B0C(this, 0x3C);
    }
    if (arg == 14) {
        func_800BF238(this, 8, 0x10, func_800BF3F0(this));
    }
    this->unk448 = 0x20;
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006CE4C: func_8007BBE8; arg 0 resets
 * 0x44C..0x460, sets 0x498 to 4 and plays sound 0x118; arg 4 spawns func_800BFB6C's entity; from 4 on clears
 * 0x3A4's x and y and 0x3B4's x and lowers 0x3B4's y by 0x71C; arg 14 calls func_800BF238; then sets 0x448 to
 * 0x20. */
void func_800BFA98(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    if (arg == 0) {
        this->unk44C = 0x800000;
        this->unk454 = 0;
        this->unk458 = 0;
        this->unk45C = 0;
        this->unk460 = 0;
        this->unk450 = 1;
        this->unk498 = 4;
        func_800784F4(this, 0x118);
    }
    if (arg == 4) {
        func_800BFB6C(this);
    }
    if (arg >= 4) {
        this->unk3A4.vx = 0;
        this->unk3A4.vy = 0;
        this->unk3B4.vx = 0;
        this->unk3B4.vy -= 0x71C;
    }
    if (arg == 14) {
        func_800BF238(this, 8, 0x13, func_800BF3F0(this));
    }
    this->unk448 = 0x20;
}

/* Creates an entity of func_800C06D0's class (0xB0 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800BFB6C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0xB0);

    if (child != NULL) {
        func_800C06D0(entityOperatorNewAt(0xB0, child), this);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800BFBC8);

/* The override of Unk80076F34's method at 0xE8 for virtual table D_8006CE4C: func_8007BA30; arg 0 plays
 * sound 0x116, sets 0x498 to 10 and func_80077448(-0x10000); arg 4 calls func_800C0084, 5 and up
 * func_800C00EC; then spawns func_800C0210's entity and (JP only) calls func_80079320 when bit 4 of 0x2C8 of
 * 0x228 is set. */
void func_800BFF28(Unk80076F34 *this, s32 arg) {
    func_8007BA30(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0x116);
        this->unk498 = 10;
        func_80077448(this, -0x10000);
    }
    if (arg == 4) {
        func_800C0084(this);
    }
    if (arg >= 5) {
        func_800C00EC(this);
    }
    func_800C0210(this);
#if VERSION_JP
    if (func_8006F190(func_80077580(this))) {
        func_80079320(this);
    }
#endif
}

/* The override of Unk80076F34's method at 0xF0 for virtual table D_8006CE4C: func_8007BA64; arg 0 plays
 * sound 0x116, sets 0x498 to 10 and func_80077448(-0x10000); arg 4 calls func_800C0084, 5 and up
 * func_800C00EC, lowers vy at 0x3B4 by 0x71C; then spawns func_800C0210's entity and (JP only) calls
 * func_80079320 when bit 4 of 0x2C8 of 0x228 is set. */
void func_800BFFD0(Unk80076F34 *this, s32 arg) {
    func_8007BA64(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0x116);
        this->unk498 = 10;
        func_80077448(this, -0x10000);
    }
    if (arg == 4) {
        func_800C0084(this);
    }
    if (arg >= 5) {
        func_800C00EC(this);
    }
    this->unk3B4.vy -= 0x71C;
    func_800C0210(this);
#if VERSION_JP
    if (func_8006F190(func_80077580(this))) {
        func_80079320(this);
    }
#endif
}

/* Creates an entity of func_800C0CC0's class (0x2AC bytes) with g++'s new and the
 * vector func_800775D8 gives, and hands it to func_8007AD4C. */
void func_800C0084(Unk80076F34 *this) {
    Vec3 vec;
    Entity *child;

    func_800775D8(this, &vec);
    child = allocEntityMemory(0x2AC);
    if (child != NULL) {
        func_800C0CC0(entityOperatorNewAt(0x2AC, child), this, &vec);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C00EC);

/* Spawns the entity of func_800C62EC at the character (func_800775D8). */
void func_800C0210(Unk80076F34 *this) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &position, 0x50, 0x60, 1, 0x808080);
    }
    func_8007AD4C(this, child);
}

/* A point of Unk800C0298's trail. */
typedef struct Unk800C0298Point {
    /* 0x0 */ Vec3 pos;
    /* 0xC */ s32 unkC;
} Unk800C0298Point;

/* The entity func_800C0298 constructs for a character (virtual table D_8006CE24): the character's last eight
 * positions, kept in a ring while it records and dropped one by one after. */
typedef struct Unk800C0298 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *owner;
    /* 0x1C */ Unk800C0298Point trail[8];
    /* 0x9C */ s32 fading; /* 0 while it records */
    /* 0xA0 */ s32 newest; /* the trail's newest point */
    /* 0xA4 */ s32 count;  /* the points in use, up to 8 */
    /* 0xA8 */ s32 unkA8;  /* whether the character faced left (func_80079E10) */
    /* 0xAC */ s32 frames; /* frames recorded */
    /* 0xB0 */ s32 unkB0;  /* the character's 0x440 (func_8007768C) */
} Unk800C0298; /* size 0xB4 */

extern EntityVtable D_8006CE24;

/* Constructs an Unk800C0298 for owner: the whole trail at the owner's position. The match depends on the pointer
 * walk over the trail (an indexed this->trail[i] folds 0x1C into the stores). */
Unk800C0298 *func_800C0298(Unk800C0298 *this, Unk80076F34 *owner) {
    Vec3 position;
    Unk800C0298Point *point;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006CE24;
    this->owner = owner;
    this->fading = 0;
    this->newest = 0;
    this->count = 0;
    this->unkA8 = func_80079E10(owner) < 1;
    this->frames = 0;
    this->unkB0 = func_8007768C(owner);
    func_800775D8(owner, &position);
    point = this->trail;
    for (i = 0; i < 8; i++) {
        point->pos = position;
        point++;
    }
    return this;
}

/* The first method of Unk800C0298 (virtual table D_8006CE24): for 9 frames writes the owner's position to the next
 * point of the ring, growing the count to 8; then drops a point a frame and deletes itself at none. */
void func_800C0344(Unk800C0298 *this) {
    if (this->fading == 0) {
        if (++this->newest >= 8) {
            this->newest = 0;
        }
        if (++this->count >= 8) {
            this->count = 8;
        }
        func_800775D8(this->owner, &this->trail[this->newest].pos);
        if (++this->frames >= 9) {
            this->fading++;
        }
    } else if (--this->count <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD5C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C0434);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD60);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C06D0);

/* The destructor of Unk800C06D0 (vtable D_8006CDFC): stops its sound, takes the part out of its list,
 * then destroys the entity. */
void func_800C0830(Unk800C06D0 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006CDFC;
    stopSound(this->sound);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C08CC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C0A60);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C0CC0);

/* The destructor of Unk800C0CC0 (vtable D_8006CDD4): the same as func_800C0830. */
void func_800C0F10(Unk800C0CC0 *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &D_8006CDD4;
    stopSound(this->sound);
    this->parts.part.vtable = D_80065184;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.node;
    this->parts.part.node.next->prev = this->parts.part.node.prev;
    node->prev->next = this->parts.part.node.next;
    this->parts.part.node.next = node;
    this->parts.part.node.prev = node;
    this->parts.base.vtable = D_80065164;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C0FAC);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD70);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C1180);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C12A4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C14F0);

/* the frame of func_800C16F8's sprite */
extern u16 D_8006CD94[];

/* the virtual table of func_800C16F8's sprite class */
extern EntityVtable D_8006CDAC;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CD94);

/* Constructs a sprite entity of the class of vtable D_8006CDAC: func_800C2F14 with the frame D_8006CD94, size 0x40
 * and grey. */
Unk800C2F14 *func_800C16F8(Unk800C2F14 *this, VECTOR *pos, VECTOR *vel, s32 flags) {
    func_800C2F14(this, D_8006CD94, pos, vel, 0x40, 0x808080, flags);
    this->entity.vtable = &D_8006CDAC;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_22", func_800C1758);

/* The destructor of the entity class of vtable D_8006CE4C, which func_800BF0B4 installs. */
void func_800C1840(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CE24, which func_800C0298 installs. */
void func_800C1888(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800C18D0(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CDAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CDD4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CDFC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CE24);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CE4C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CF74);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_22", D_8006CF7C);
