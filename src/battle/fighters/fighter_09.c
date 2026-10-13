/* One of the 24 fighters: its constructor, func_80099A7C, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

void func_80099D18(Unk80076F34 *this);

/* the parts of entity updates that the functions below call in a row */
void func_8009AFC4(Entity *this, s32 arg);
void func_8009B370(Entity *this, s32 arg);
void func_8009B664(Entity *this, s32 arg);
struct Unk8009AB40 *func_8009AB40(struct Unk8009AB40 *this, Unk80076F34 *chara, s32 unk48);
extern EntityVtable D_8006A15C;
extern VtableEntry D_8006A1CC[];
extern EntityVtable D_8006A1EC;
extern EntityVtable D_8006A2AC;

typedef struct Unk80099A7C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ Unk80076F34 *unk4A8;
    /* 0x4AC */ MatrixWords unk4AC; /* func_80099DC4 sets it to the identity */
    /* 0x4CC */ u32 unk4CC; /* a sound's handle (stopSound) */
} Unk80099A7C;

/* constructs the entity of virtual table D_8006A25C for a character */
Entity *func_8009B834(void *this, Unk80076F34 *chara, s32 arg2, s32 arg3);

/* The entity class of virtual table D_8006A15C, which func_8009A814 makes for its object. */
typedef struct Unk8009D20C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ void *unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} Unk8009D20C;

/* Constructs an entity of the class of virtual table D_8006A2AC, derived from func_80076F34's. */
Unk80099A7C *func_80099A7C(Unk80099A7C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006A2AC;
    this->unk4A8 = 0;
    this->unk4CC = 0;
    return this;
}

/* Stops the sound at 0x4CC. */
void func_80099AB8(Unk80099A7C *this) {
    stopSound(this->unk4CC);
    this->unk4CC = 0;
}

/* The override of Unk80076F34's method at 0xA8 for virtual table D_8006A2AC: sets bit 6 of 0x37C; arg 0
 * starts clip 6, arg 5 runs func_80077698's entry 1 from the start (func_8007B470), and
 * from 6 on runs it at arg. */
void func_80099AE8(Unk80076F34 *this, s32 arg) {
    Unk8007B470Entry *record;

    func_80079AE4(this);
    record = &func_80077698(this)->entries[1];
    switch (arg) {
    case 0:
        func_80079830(this, 6, 0, 0, -1);
        break;
    case 5:
        func_8007B470(this, 0, record, 1);
        break;
    }
    if (arg > 5) {
        func_8007B470(this, arg, record, 1);
    }
}

/* The override of Unk80076F34's method at 0xC0 for virtual table D_8006A2AC: sets bit 6 of 0x37C; arg 0
 * starts clip 8, arg 6 runs func_80077698's entry 3 from the start with 2 (func_8007B470), and
 * from 7 on runs it at arg. */
void func_80099B98(Unk80076F34 *this, s32 arg) {
    Unk8007B470Entry *record;

    func_80079AE4(this);
    record = &func_80077698(this)->entries[3];
    switch (arg) {
    case 0:
        func_80079830(this, 8, 2, 0, -1);
        break;
    case 6:
        func_8007B470(this, 0, record, 2);
        break;
    }
    if (arg > 6) {
        func_8007B470(this, arg, record, 1);
    }
}

/* Overrides func_8007BB64 (entry 0x10C of Unk80076F34's vtable) in D_8006A2AC: calls it, then
 * func_80099D18 when arg is 2; from 3 on, sets bits 0xD0000 of unk448 when it is not zero. */
void func_80099C48(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_80099D18(this);
    }
    if (arg >= 3 && this->unk448 != 0) {
        this->unk448 |= 0xD0000;
    }
}

/* Overrides func_8007BA98 (entry 0xFC of Unk80076F34's vtable) in D_8006A2AC: calls it, then
 * func_80099D18 when arg is 2; from 3 on, sets bits 0xD0000 of unk448 when it is not zero. */
void func_80099CB0(Unk80076F34 *this, s32 arg) {
    func_8007BA98(this, arg);
    if (arg == 2) {
        func_80099D18(this);
    }
    if (arg >= 3 && this->unk448 != 0) {
        this->unk448 |= 0xD0000;
    }
}

/* three tables of 4 entries for func_80099D18's entities */
extern Unk800C5B18Entry D_80069D3C[3][4];

/* Creates three entities of func_800C5680's class (0x690 bytes), one per table of D_80069D3C, and hands
 * them to func_8007AD4C. */
void func_80099D18(Unk80076F34 *this) {
    Unk800C5B18Entry (*table)[4] = D_80069D3C;
    Unk800C5B18Entry (*end)[4] = &D_80069D3C[3];

    /* The match depends on a do/while walking a pointer to an end kept in a local (an index is counted
     * down, a for loop keeps its first test, and an end computed in the test gets another register) */
    do {
        Entity *child = allocEntityMemory(0x690);

        if (child != NULL) {
            func_800C5680(entityOperatorNewAt(0x690, child), this, *table, 4, 2, 16);
        }
        func_8007AD4C(this, child);
        table++;
    } while (table != end);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_80069D2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_80069D3C);

void func_80099F30(Unk80076F34 *this);
void func_8009A0EC(Unk80099A7C *this);
void func_80099FBC(Unk80099A7C *this, MATRIX *matrix);
void func_8009A128(Unk80099A7C *this);
void func_8009A1B0(Unk80076F34 *this, u32 unk, Vec3 *velocity);

/* A virtual function of Unk80099A7C: func_8007B978 and func_8007835C(0), then by step: 0 clears 0x4A4 and
 * 0x4A8, sets the matrix at 0x4AC to the identity and calls func_80079274(2) on the character func_8007AEEC
 * gives, then, like 1, puts that character at node 5 (func_80099F30); 2, 15 and 30 call func_8009A1B0 (30
 * with a velocity in the way it faces, after dropping the character at 0x4A8); steps 3 to 29 end with
 * func_8009A128. */
void func_80099DC4(Unk80099A7C *this, s32 step) {
    Unk80076F34 *other;
    Vec3 velocity;

    func_8007B978(&this->base, step);
    other = func_8007AEEC(&this->base);
    func_8007835C(&this->base, 0);
    switch (step) {
    case 0:
        this->unk4A4 = 0;
        this->unk4A8 = NULL;
        this->unk4AC.words[0] = 0x1000;
        this->unk4AC.words[1] = 0;
        this->unk4AC.words[2] = 0x1000;
        this->unk4AC.words[3] = 0;
        this->unk4AC.words[4] = 0x1000;
        this->unk4AC.m.t[0] = 0;
        this->unk4AC.m.t[1] = 0;
        this->unk4AC.m.t[2] = 0;
        func_80079274(other, 2);
        /* fallthrough */
    case 1:
        func_80099F30(&this->base);
        break;
    case 2:
        func_8009A1B0(&this->base, 0x8C0C00, (Vec3 *)&D_80010814);
        func_80099FBC(this, 0);
        break;
    case 15:
        func_8009A1B0(&this->base, 0x8C1400, (Vec3 *)&D_80010814);
        break;
    case 30:
        func_8009A0EC(this);
        velocity.vx = func_80079E10(&this->base) * 0x38E0;
        velocity.vy = -0x38E0;
        velocity.vz = 0;
        func_8009A1B0(&this->base, 0x408C2000, &velocity);
        break;
    }
    if ((u32)(step - 3) < 27) {
        func_8009A128(this);
    }
}

/* the offset from node 5 where func_80099F30 puts the other character */
extern VECTOR D_80069D2C;

/* Puts the character func_8007AEEC gives at node 5 of this one (offset D_80069D2C), raised by the half word
 * at 2 of its table (func_80077698), and clears its 0x3A4..0x3C0, 0x3E0 and 0x3F0. */
void func_80099F30(Unk80076F34 *this) {
    Unk80076F34 *other = func_8007AEEC(this);
    VECTOR position;

    func_8007A4A4(this, &position, 5, &D_80069D2C);
    position.vy += func_80077698(other)->unk2;
    func_800775B0(other, (Vec3 *)&position);
    other->unk3B4.vx = 0;
    other->unk3B4.vy = 0;
    other->unk3B4.vz = 0;
    other->unk3C0 = 0;
    other->unk3A4.vx = 0;
    other->unk3A4.vy = 0;
    other->unk3A4.vz = 0;
    other->unk3B0 = 0;
    other->unk3E0 = 0;
    other->unk3F0 = 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_80099FBC);

/* Calls func_8007AE34 on the character at 0x4A8, if there is one, and clears it. */
void func_8009A0EC(Unk80099A7C *this) {
    if (this->unk4A8 != NULL) {
        func_8007AE34(this->unk4A8);
    }
    this->unk4A8 = NULL;
}

/* While 0x4A4 is 0, waits for the character at 0x4A8 to be out of every list, then drops it
 * (func_8009A0EC) and counts 0x4A4 up; otherwise calls func_80099FBC with the matrix at 0x4AC and clears
 * 0x4A4. */
void func_8009A128(Unk80099A7C *this) {
    if (this->unk4A4 == 0) {
        if (listIsAlone(&this->unk4A8->entity.link)) {
            func_8009A0EC(this);
            this->unk4A4++;
        }
    } else {
        func_80099FBC(this, &this->unk4AC.m);
        this->unk4A4 = 0;
    }
}

/* Throws the character func_8007AEEC gives (func_80078EF8 with unk and velocity), then creates
 * func_800C5DF4's entity (0x48 bytes) at node 5 (offset D_80069D2C), moving back and up, and hands it to
 * func_8007AD4C. */
void func_8009A1B0(Unk80076F34 *this, u32 unk, Vec3 *velocity) {
    VECTOR position;
    Vec3 drift;
    Entity *child;

    func_80078EF8(func_8007AEEC(this), unk, velocity, 1);
    func_8007A4A4(this, &position, 5, &D_80069D2C);
    drift.vx = -(func_80079E10(this) << 5);
    drift.vy = -16;
    drift.vz = 0;
    child = allocEntityMemory(0x48);
    if (child != NULL) {
        func_800C5DF4(entityOperatorNewAt(0x48, child), 6, (Vec3 *)&position, &drift, 0x80, 8, 0xFFE0C0);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009A290);

void func_8009A4D8(Unk80076F34 *this);
void func_8009A44C(Unk80099A7C *this);

/* Overrides func_8007BBE8: calls it and func_80079AE4; arg 0 plays sound 0xBB and spawns func_8009A4D8's
 * entity, arg 13 runs func_8009A44C and pushes 0x3B4.vx back by 0x1C70 scaled by 0x3C4 (16.16); below 20,
 * a positive 0x3A4.vy takes half of it off 0x3B4.vy. Always sets 0x4A0. */
void func_8009A35C(Unk80099A7C *this, s32 arg) {
    func_8007BBE8(&this->base, arg);
    func_80079AE4(&this->base);
    switch (arg) {
    case 0:
        func_800784F4(&this->base, 0xBB);
        func_8009A4D8(&this->base);
        break;
    case 13:
        func_8009A44C(this);
        this->base.unk3B4.vx -= ((s64)(func_80079E10(&this->base) * 0x1C70) * this->base.unk3C4) >> 16;
        break;
    }
    if (arg < 20 && this->base.unk3A4.vy > 0) {
        this->base.unk3B4.vy -= this->base.unk3A4.vy >> 1;
    }
    this->base.unk4A0 = 1;
}

/* Plays sound 0xB6 (its handle kept at 0x4CC), creates func_8009AB40's entity (0x4C bytes) for the
 * character's 0x440, hands it to func_8007AD4C and calls func_80079B0C(50). */
void func_8009A44C(Unk80099A7C *this) {
    Entity *child;

    this->unk4CC = func_80078540(&this->base, 0xB6);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        func_8009AB40(entityOperatorNewAt(0x4C, child), &this->base, func_8007768C(&this->base));
    }
    func_8007AD4C(&this->base, child);
    func_80079B0C(&this->base, 50);
}

/* Spawns the entity of func_8009B834 for the character (func_8007AD4C). */
void func_8009A4D8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x50);

    if (child != NULL) {
        func_8009B834(entityOperatorNewAt(0x50, child), this, 0x200, 0xFF8080);
    }
    func_8007AD4C(this, child);
}

void func_8009A764(Unk80076F34 *this);

/* Overrides func_8007B9FC: calls it and func_80079AE4; arg 0 plays sound 0xBC, arg 19 runs func_8009A764
 * and pushes 0x3B4.vx back by 0x4718 scaled by 0x3C4 (16.16), and arg 40, when func_800778BC holds, clears
 * 0x3A4.vx and pushes it back by 0x2AA8. */
void func_8009A540(Unk80076F34 *this, s32 arg) {
    func_8007B9FC(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0xBC);
        break;
    case 19:
        func_8009A764(this);
        this->unk3B4.vx -= ((s64)(func_80079E10(this) * 0x4718) * this->unk3C4) >> 16;
        break;
    case 40:
        if (func_800778BC(this)) {
            this->unk3A4.vx = 0;
            this->unk3B4.vx -= ((s64)(func_80079E10(this) * 0x2AA8) * this->unk3C4) >> 16;
        }
        break;
    }
}

/* Overrides func_8007BC1C: calls it and func_80079AE4; arg 0 plays sound 0xBC, arg 19 runs func_8009A764,
 * pushes 0x3B4.vx back by 0x2AA8 and lifts 0x3B4.vy by 0x2AA8 plus 0x3A4.vy (both scaled by 0x3C4, 16.16);
 * below 19, a positive 0x3A4.vy takes half of it off 0x3B4.vy. */
void func_8009A644(Unk80076F34 *this, s32 arg) {
    s32 lift;

    func_8007BC1C(this, arg);
    func_80079AE4(this);
    switch (arg) {
    case 0:
        func_800784F4(this, 0xBC);
        break;
    case 19:
        func_8009A764(this);
        this->unk3B4.vx -= ((s64)(func_80079E10(this) * 0x2AA8) * this->unk3C4) >> 16;
        lift = ((s64)this->unk3C4 * 0x2AA8) >> 16;
        lift += this->unk3A4.vy;
        this->unk3B4.vy -= lift;
        break;
    }
    if (arg < 19 && this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy >> 1;
    }
}

/* the offset from node 0 where func_8009A764 creates its entity */
extern VECTOR D_80069EE0;

Unk8009BE08 *func_8009BE08(Unk8009BE08 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_80069EE0);

/* Creates func_8009BE08's entity (0xC8 bytes) at node 0 (offset D_80069EE0), moving forward by 24, hands it
 * to func_8007AD4C, then calls func_80079B14(60) and plays sound 0xB7. */
void func_8009A764(Unk80076F34 *this) {
    VECTOR position;
    Vec3 velocity;
    Entity *child;

    func_8007A4A4(this, &position, 0, &D_80069EE0);
    velocity.vx = func_80079E10(this) * 24;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_8009BE08(entityOperatorNewAt(0xC8, child), this, (Vec3 *)&position, &velocity);
    }
    func_8007AD4C(this, child);
    func_80079B14(this, 60);
    func_80078540(this, 0xB7);
}

Unk8009D20C *func_8009D20C(Unk8009D20C *this, void *arg);
Entity *func_8009D4EC(void *this, Unk80076F34 *chara);
void func_8009A920(Unk80076F34 *this);

/* Overrides func_8007BA30: calls it; arg 0 spawns func_8009D20C's entity (0x24 bytes), sets 0x498 to 12 and
 * func_80077448(-0x8000); while bit 2 of the flags is set (func_800778BC), every 4th arg below 45 spawns
 * func_8009D4EC's entity (0x14C bytes); arg 20 plays sound 0xBD and arg 45 runs func_8009A920. */
void func_8009A814(Unk80076F34 *this, s32 arg) {
    Entity *child;

    func_8007BA30(this, arg);
    if (arg == 0) {
        child = allocEntityMemory(0x24);
        if (child != NULL) {
            func_8009D20C(entityOperatorNewAt(0x24, child), this);
        }
        func_8007AD4C(this, child);
        this->unk498 = 12;
        func_80077448(this, -0x8000);
    }
    if (func_800778BC(this) && arg < 45 && (arg & 3) == 0) {
        child = allocEntityMemory(0x14C);
        if (child != NULL) {
            func_8009D4EC(entityOperatorNewAt(0x14C, child), this);
        }
        func_8007AD4C(this, child);
    }
    if (arg == 20) {
        func_800784F4(this, 0xBD);
    }
    if (arg == 45) {
        func_8009A920(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_80069EF0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009A920);

extern EntityVtable D_8006A284;

/* The entity class of virtual table D_8006A284 (constructor func_8009AB40, size 0x4C). */
typedef struct Unk8009AB40 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ VECTOR position; /* node 5 of the character, offset D_80069D2C */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 facing; /* func_80079E10: 1 or -1 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48; /* the character's 0x440 (func_8007768C) */
} Unk8009AB40;

/* Constructs the entity of virtual table D_8006A284 for chara, at its node 5 and facing its way. */
Unk8009AB40 *func_8009AB40(Unk8009AB40 *this, Unk80076F34 *chara, s32 unk48) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A284;
    this->chara = chara;
    func_8007A4A4(chara, &this->position, 5, &D_80069D2C);
    this->unk2C = 4;
    this->facing = func_80079E10(chara);
    this->unk48 = unk48;
    this->unk38 = 0;
    this->unk34 = 0x2000;
    this->unk3C = 0x40;
    this->unk40 = 0;
    this->unk44 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009ABE0);

/* The update of the entities of D_8006A284: func_8009AFC4, func_8009B370, then func_8009B664. */
void func_8009AF7C(Entity *this, s32 arg) {
    func_8009AFC4(this, arg);
    func_8009B370(this, arg);
    func_8009B664(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009AFC4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009B370);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009B664);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009B834);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009BA18);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009BC24);

extern EntityVtable D_8006A234;
extern VtableEntry D_8006A214[];

/* Constructs the Unk800C7024 class of virtual table D_8006A234 (and D_8006A214 for its base at 0x18) for
 * chara at position, moving by velocity, with 0x84AC4000 and 7 in its base's 0x8 and 0xC. */
Unk8009BE08 *func_8009BE08(Unk8009BE08 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C7024(&this->base, chara, position, velocity, 0x80, 0, 0x1000, 3);
    this->base.parts.base.vtable = D_8006A214;
    this->base.parts.entity.vtable = &D_8006A234;
    func_8007BDC8(&this->base.parts.base, 0x84AC4000, 7);
    this->unkC0 = 0;
    this->unkC4 = 0;
    return this;
}

/* The gather of func_8009BE08's and func_8009CE8C's classes: speeds up along x by 1/16, eases 0xC0 toward
 * 0x100, cycles 0xC4 from 0x70 to 0xA0 by 16, spawns func_800C62EC's particles (0x3C bytes) at its position
 * as a sibling, then runs its base's func_800C71DC. */
void func_8009BE80(Unk8009BE08 *this, s32 arg) {
    Entity *child;

    this->base.vel.vx = (this->base.vel.vx * 17) >> 4;
    this->base.speed = vecGetLength((VECTOR *)&this->base.vel);
    this->unkC0 += (0x100 - this->unkC0) >> 2;
    this->unkC4 += 16;
    if (this->unkC4 > 0xA0) {
        this->unkC4 = 0x70;
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 6, &this->base.pos, 0x30, 0xC0, 1, 0x808080);
    }
    entityAddSibling(&this->base.parts.entity, child);
    func_800C71DC(&this->base, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009BF6C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009C048);

/* A point of Unk8009C294's trail: where it was, and an age that grows by 3 a gather. */
typedef struct Unk8009C294Trail {
    /* 0x0 */ Vec3 position;
    /* 0xC */ s32 age;
} Unk8009C294Trail;

/* The UnkPartsEntity38 class func_8009C294 constructs (virtual tables D_8006A1EC and D_8006A1CC). */
typedef struct Unk8009C294 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ u8 unk80[0x84 - 0x80];
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0xB4 - 0x90];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ u8 unkB8[0xC0 - 0xB8];
    /* 0xC0 */ s32 unkC0; /* gathers so far */
    /* 0xC4 */ u8 unkC4[0xCC - 0xC4];
    /* 0xCC */ s32 trailIndex; /* the newest point of trail */
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ Unk8009C294Trail trail[16];
} Unk8009C294;

void func_8009C914(Unk8009C294 *this);
void func_8009CB58(Unk8009C294 *this, s32 arg);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009C294);

void func_8009C678(Unk8009C294 *this);
void func_8009C898(Unk8009C294 *this);

/* The gather of func_8009C294's class: while 0xB4 is set, counts 0xD0 down and deletes itself at 0;
 * otherwise runs func_8009C678 and counts 0xD0 up to 16. Then adds a trail point (func_8009C898). */
void func_8009C43C(Unk8009C294 *this) {
    if (this->unkB4 != 0) {
        if (--this->unkD0 <= 0) {
            entityDelete(&this->parts.entity);
            return;
        }
    } else {
        func_8009C678(this);
        if (++this->unkD0 > 16) {
            this->unkD0 = 16;
        }
    }
    func_8009C898(this);
    this->unkC0++;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009C4EC);

/* The update of func_8009C294's class: func_8009C914 until 0xB4 is set, then func_8009CB58. */
void func_8009C62C(Unk8009C294 *this, s32 arg) {
    if (this->unkB4 == 0) {
        func_8009C914(this);
    }
    func_8009CB58(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009C678);

/* Adds the position as the newest point of the trail (age 16) and ages every point by 3. */
void func_8009C898(Unk8009C294 *this) {
    Unk8009C294Trail *point;
    s32 i;

    if (++this->trailIndex >= 16) {
        this->trailIndex = 0;
    }
    /* The match depends on reading the position before the point's address is taken */
    {
        s32 x = this->position.vx;
        s32 y = this->position.vy;
        s32 z = this->position.vz;

        point = &this->trail[this->trailIndex];
        point->position.vx = x;
        point->position.vy = y;
        point->position.vz = z;
        point->age = 16;
    }
    for (i = 0; i < 16; i++) {
        this->trail[i].age += 3;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A0D0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009C914);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009CB58);

extern EntityVtable D_8006A184;
extern VtableEntry D_8006A1AC[];

/* Constructs the class of virtual table D_8006A184 (and D_8006A1AC for its base at 0x18) on
 * func_8009BE08's for chara at position, moving by velocity, with 0x6B08000 and 7 in its base's 0x8 and 0xC. */
Unk8009BE08 *func_8009CE8C(Unk8009BE08 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_8009BE08(this, chara, position, velocity);
    this->base.parts.base.vtable = D_8006A1AC;
    this->base.parts.entity.vtable = &D_8006A184;
    func_8007BDC8(&this->base.parts.base, 0x6B08000, 7);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009CEE0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009CFBC);

/* Constructs an entity of the class of virtual table D_8006A15C for arg. */
Unk8009D20C *func_8009D20C(Unk8009D20C *this, void *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A15C;
    this->unk18 = arg;
    this->unk1C = 0;
    this->unk20 = 0;
    return this;
}

/* The gather of func_8009D20C's entity: 0x1C rises with 0x20 (by 8) for 16 gathers, holds, and from 45 on
 * (at once when func_8007AED0 holds for the character) falls by 16 until the entity deletes itself. */
void func_8009D25C(Unk8009D20C *this) {
    s32 time;

    if (func_8007AED0(this->unk18)) {
        this->unk20 = 45;
    }
    time = this->unk20;
    if (time <= 16) {
        this->unk1C = time * 8;
    } else if (time >= 45) {
        this->unk1C -= 16;
        if (this->unk1C <= 0) {
            entityDelete(&this->entity);
            return;
        }
    }
    this->unk20++;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A0F0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009D308);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009D4EC);

/* The entity class of virtual table D_8006A134 (constructor func_8009D4EC, size 0x14C). */
typedef struct Unk8009D4EC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk80076F34 *chara;
    /* 0x01C */ u8 unk1C[0x12C - 0x1C]; /* 17 points of 0x10 bytes, from COS_SIN_TABLE */
    /* 0x12C */ Vec3 position; /* the character's (func_800775D8) */
    /* 0x138 */ u8 unk138[0x13C - 0x138];
    /* 0x13C */ s32 unk13C; /* 0x100 at construction */
    /* 0x140 */ s32 unk140;
    /* 0x144 */ s32 unk144;
    /* 0x148 */ s32 unk148; /* the character's 0x440 (func_8007768C) */
} Unk8009D4EC;

/* The gather of func_8009D4EC's entity: follows the character's position until func_8007AED0 holds or bit 2
 * of its flags (func_800778BC) clears, and deletes itself once 0x13C, growing by 0x15, reaches 0x200. */
void func_8009D5D4(Unk8009D4EC *this) {
    if (this->unk140 == 0) {
        if (func_8007AED0(this->chara) || !func_800778BC(this->chara)) {
            this->unk140++;
        } else {
            func_800775D8(this->chara, &this->position);
        }
    }
    this->unk13C += 0x15;
    if (this->unk13C >= 0x200) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_09", func_8009D68C);

/* The destructor of the entity class of vtable D_8006A2AC, which func_80099A7C installs. */
void func_8009D874(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A284, which func_8009AB40 installs. */
void func_8009D8BC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A25C, which func_8009B834 installs. */
void func_8009D904(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_8009D94C(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of the UnkPartsEntity38 class of vtables D_8006A1EC and D_8006A1CC, which
 * func_8009C294 installs. */
void func_8009D9EC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006A1CC;
    this->entity.vtable = &D_8006A1EC;
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

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_8009DA8C(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_8006A15C, which func_8009D20C installs. */
void func_8009DB2C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A134, which func_8009D4EC installs. */
void func_8009DB74(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A134);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A15C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A184);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A1AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A1CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A1EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A214);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A234);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A25C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A284);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_09", D_8006A2AC);
