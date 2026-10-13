/* One of the 24 fighters: its constructor, func_800B7EDC, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

struct Unk800B8FAC *func_800B8FAC(struct Unk800B8FAC *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj);
struct Unk800B8BBC *func_800B8BBC(struct Unk800B8BBC *this, Unk80076F34 *chara, VECTOR *pos, Unk8006ED20 *obj);
extern Unk80079B24Entry D_8006C324[];
extern EntityVtable D_8006C334;
extern EntityVtable D_8006C35C;
extern EntityVtable D_8006C384;
extern VECTOR D_800D348C;

/* Constructs an entity of the class of vtable D_8006C384, which derives from Unk80076F34 and
 * gives it the table D_8006C324. */
Unk80076F34 *func_800B7EDC(Unk80076F34 *this) {
    func_80076F34(this);
    this->entity.vtable = &D_8006C384;
    func_80079B24(this, D_8006C324);
    return this;
}

/* The destructor of the entity class of vtable D_8006C384, which func_800B7EDC installs. */
void func_800B7F20(Entity *this, s32 flags) {
    this->vtable = &D_8006C384;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The method at 0x20 of virtual table D_8006C384: clears 0x3F0 and the angle at 0x3E0. */
void func_800B7F74(Unk80076F34 *this) {
    this->unk3F0 = 0;
    this->unk3E0 = 0;
}

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in D_8006C384: calls it; when arg is 3, moves unk3B4 by unk3C4 * 0x2AA8 (16.16)
 * toward func_80079E10's side; from 4 on, moves unk3B4 back by a quarter of unk3A4. */
void func_800B7F80(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    if (arg == 3) {
        s32 step;

        step = ((s64)this->unk3C4 * 0x2AA8) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 4) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

/* Overrides func_8007B70C (entry 0xAC of Unk80076F34's vtable) in D_8006C384: calls it; when arg is 4,
 * moves unk3B4 by unk3C4 * 0x38E0 (16.16) toward func_80079E10's side; from 5 on, moves unk3B4 back
 * by a quarter of unk3A4. */
void func_800B8030(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 4) {
        s32 step;

        step = ((s64)this->unk3C4 * 0x38E0) >> 16;
        this->unk3B4.vx += step * func_80079E10(this);
    }
    if (arg >= 5) {
        this->unk3B4.vx += -this->unk3A4.vx >> 2;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B80E0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B82B8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B8480);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B8670);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B8814);

/* Creates an entity of func_800B8BBC's class (0x58 bytes) at the owner's point 2 moved by D_800D348C,
 * with arg in the vector's pad, and hands it to func_8007AD4C. */
void func_800B8AA8(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR pos;
    Entity *child;

    func_8007A4A4(this, &pos, 2, &D_800D348C);
    pos.pad = arg;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_800B8BBC(entityOperatorNewAt(0x58, child), this, &pos, obj);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800B8FAC's entity (0x60 bytes) at the character's position, and hands it to
 * func_8007AD4C. */
void func_800B8B3C(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    child = allocEntityMemory(0x60);
    if (child != NULL) {
        func_800B8FAC(entityOperatorNewAt(0x60, child), this, &position, obj);
    }
    func_8007AD4C(this, child);
}

/* The entity func_800B8AA8 makes (vtable D_8006C35C, 0x58 bytes): a square of side 0x100 facing the
 * character's way, with the pad of its position vector. */
typedef struct Unk800B8BBC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *obj;
    /* 0x20 */ s16 unk20; /* the pad of the position (func_800B8AA8's arg) */
    /* 0x22 */ s16 unk22; /* func_8007768C of the character */
    /* 0x24 */ s16 unk24; /* minus the character's facing */
    /* 0x26 */ u16 frames;
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ VECTOR position; /* the character's point 2 (func_8007A4A4) */
} Unk800B8BBC;

/* Constructs func_800B8AA8's entity for chara: a square from (-0x80, -0x80) to (0x80, 0x80) at z -0x80. */
Unk800B8BBC *func_800B8BBC(Unk800B8BBC *this, Unk80076F34 *chara, VECTOR *pos, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C35C;
    this->chara = chara;
    this->obj = obj;
    this->frames = 0;
    this->unk22 = func_8007768C(this->chara);
    this->unk24 = -func_80079E10(this->chara);
    this->unk20 = pos->pad;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0x80;
    this->corners[1].vx = this->corners[3].vx = 0x80;
    this->corners[0].vy = this->corners[1].vy = -0x80;
    this->corners[2].vy = this->corners[3].vy = 0x80;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x80;
    return this;
}

/* The destructor of the entity class of vtable D_8006C35C, which func_800B8BBC installs. */
void func_800B8C80(Entity *this, s32 flags) {
    this->vtable = &D_8006C35C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

extern VECTOR D_800D349C;

/* The update of func_800B8BBC's entity: puts it at the character's point 2 moved by D_800D349C, and deletes it
 * after 11 frames.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x26. */
void func_800B8CD4(Unk800B8BBC *this) {
    func_8007A4A4(this->chara, &this->position, 2, &D_800D349C);
    if (this->frames > 10) {
        entityDelete(&this->entity);
    } else {
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B8D5C);

/* The entity func_800B8B3C makes (vtable D_8006C334, 0x60 bytes): a flat square of side 0x140 facing the
 * character's way, at a position. */
typedef struct Unk800B8FAC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *obj;
    /* 0x20 */ s16 unk20; /* func_8007768C of the character */
    /* 0x22 */ s16 unk22; /* minus the character's facing */
    /* 0x24 */ u16 frames; /* counts the frames of func_800B90E4 */
    /* 0x26 */ u8 unk26[0x28 - 0x26];
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ u8 unk48; /* eases up to 0x80, then fades out (func_800B90E4) */
    /* 0x4A */ u16 phase; /* func_800B90E4's step: fade in, hold, fade out, delete */
    /* 0x4C */ u16 holdFrames; /* the frames of the hold step */
    /* 0x50 */ Vec3 position;
} Unk800B8FAC;

/* Constructs func_800B8B3C's entity for chara: a square from (-0xA0, -0xA0) to (0xA0, 0xA0) at z 0, at
 * position raised by 0x80. */
Unk800B8FAC *func_800B8FAC(Unk800B8FAC *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006C334;
    this->chara = chara;
    this->obj = obj;
    this->frames = 0;
    this->unk20 = func_8007768C(this->chara);
    this->unk22 = -func_80079E10(this->chara);
    x = position->vx;
    y = position->vy - 0x80;
    z = position->vz;
    this->unk48 = 0;
    this->phase = 0;
    this->holdFrames = 0;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0xA0;
    this->corners[1].vx = this->corners[3].vx = 0xA0;
    this->corners[0].vy = this->corners[1].vy = -0xA0;
    this->corners[2].vy = this->corners[3].vy = 0xA0;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The destructor of the entity class of vtable D_8006C334, which func_800B8FAC installs. */
void func_800B9090(Entity *this, s32 flags) {
    this->vtable = &D_8006C334;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of func_800B8FAC's entity: follows the character (func_800775D8) 0x80 higher, fades in to 0x80,
 * holds 22 frames, fades out and deletes itself; it also goes when the character's state ends. */
void func_800B90E4(Unk800B8FAC *this) {
    s32 state;

    func_800775D8(this->chara, &this->position);
    this->position.vy -= 0x80;
    switch (this->phase) {
    case 0:
        this->unk48 += (0x84 - this->unk48) >> 2;
        if (this->unk48 < 0x80) {
            break;
        }
        this->holdFrames = 0;
        this->phase++;
    case 1:
        if (++this->holdFrames < 22) {
            break;
        }
        this->phase++;
    case 2:
        this->unk48 -= this->unk48 >> 2;
        if (this->unk48 >= 5) {
            break;
        }
        this->phase++;
    case 3:
        entityDelete(&this->entity);
        return;
    }
    state = func_800792A0(this->chara)->unk0;
    if (func_8007AED0(this->chara) || state == 0x13) {
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_19", func_800B926C);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C284);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C28C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C294);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C29C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2D4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C2E4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C304);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C324);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C334);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C35C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_19", D_8006C384);
