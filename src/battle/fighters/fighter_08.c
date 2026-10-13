/* One of the 24 fighters: its constructor, func_80094F10, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

struct Unk800971E0 *func_800971E0(struct Unk800971E0 *this, Unk80076F34 *chara, s32 arg2, Unk8006ED20 *obj);
Entity *func_8009764C(Entity *this, Unk80076F34 *chara, Unk8006ED20 *obj, s32 side);
struct Unk80096984 *func_800962B8(struct Unk80096984 *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos,
                                  VECTOR *vel);
struct Unk80096F6C *func_80096F6C(struct Unk80096F6C *this, Unk80076F34 *chara);
Entity *func_80097FA0(Entity *this, Unk80076F34 *arg, Unk8006ED20 *arg2);
struct Unk80098720 *func_80098720(struct Unk80098720 *this, Unk80076F34 *chara, Vec3 *position);
extern VECTOR D_80069964;
extern EntityVtable D_800699DC;
extern EntityVtable D_80069A04;
extern EntityVtable D_80069A2C;
extern EntityVtable D_80069A54;
extern EntityVtable D_80069A7C;
extern EntityVtable D_80069ACC;
extern EntityVtable D_80069AF4;
extern EntityVtable D_80069B6C;
extern EntityVtable D_80069BDC;
extern EntityVtable D_80069C04;

typedef struct Unk80094F10 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC; /* func_80094FFC sets it */
} Unk80094F10;
Entity *func_80098A78(Entity *this, Unk80076F34 *chara, Unk8006ED20 *unk228, Vec3 *position, Vec3 *velocity);

/* Constructs an entity of the class of virtual table D_80069C04, derived from func_80076F34's. */
Unk80094F10 *func_80094F10(Unk80094F10 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_80069C04;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069C04, which func_80094F10 installs. */
void func_80094F48(Entity *this, s32 flags) {
    this->vtable = &D_80069C04;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A method of the class of virtual table D_80069C04: the base step (func_80078910), then unk4A4 counts down;
 * while it runs and func_80079E2C is 5 or 6 it is set back to 35. */
void func_80094F9C(Unk80094F10 *this, void *arg) {
    s32 state;

    func_80078910(this, arg);
    if (this->unk4A4 != 0) {
        this->unk4A4--;
        if (this->unk4A4 != 0) {
            state = func_80079E2C(&this->base);
            /* The match depends on the two tests being nested ifs: && folds them into one range test. */
            if (state < 7) {
                if (state >= 5) {
                    this->unk4A4 = 35;
                }
            }
        }
    }
}

/* The method at 0x24 for virtual table D_80069C04 (func_80092B90's slot): sets unk4AC. */
void func_80094FFC(Unk80094F10 *this) {
    this->unk4AC = 1;
}

/* The override of Unk80076F34's method at 0xB8 for virtual table D_80069C04: func_8007B80C,
 * then for arg 4 pushes vx at 0x3B4 by field 0x8 of func_80077698's entry 4 times the scale at 0x3C4,
 * along the facing. */
void func_80095008(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 4) {
        s32 push = ((s64)func_80077698(this)->entries[4].unk8[0] * this->unk3C4) >> 16;

        this->unk3B4.vx += push * func_80079E10(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095098);

struct Unk80095F98 *func_80095F98(struct Unk80095F98 *this, Unk80076F34 *chara, Vec3 *position, s32 facing,
                                  s32 arg4);

/* Spawns func_80095F98's entity (0x40 bytes) 0x133 above the character, with its facing and arg. */
void func_80095294(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x133;
    child = allocEntityMemory(0x40);
    if (child != NULL) {
        func_80095F98(entityOperatorNewAt(0x40, child), this, &position, func_80079E10(this), arg);
    }
    func_8007AD4C(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095330);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095480);

/* Creates func_800971E0's entity (0x30 bytes) for the character with arg, and hands it to
 * func_8007AD4C. */
void func_800955D4(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        func_800971E0(entityOperatorNewAt(0x30, child), this, arg, obj);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069964);

/* Creates an entity of func_800962B8's class (0x15C bytes) at the owner's point 6 moved by D_80069964, moving
 * 192 a frame toward func_80079E10's side, hands it to func_8007AD4C and plays sound 0xAE. */
void func_80095654(Unk80076F34 *this) {
    VECTOR pos;
    VECTOR vel;
    Unk8006ED20 *obj;
    Entity *child;

    func_8007A4A4(this, &pos, 6, &D_80069964);
    vel.vx = func_80079E10(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0x15C);
    if (child != NULL) {
        func_800962B8(entityOperatorNewAt(0x15C, child), this, obj, &pos, &vel);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xAE);
}

extern VECTOR D_80069974;
Entity *func_80096CFC(void *this, Unk80076F34 *chara, VECTOR *pos, s32 angle);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069974);

/* Spawns six entities of func_80096CFC (0x58 bytes) at the owner's point 6 moved by D_80069974, with angles
 * (4096ths: func_80096CFC masks it with 0xFFF) 0x100 apart from 0x100, or from 0xA00 when the character faces left. */
void func_80095714(Unk80076F34 *this) {
    VECTOR pos;
    Entity *child;
    s32 angle;
    s32 i;

    func_8007A4A4(this, &pos, 6, &D_80069974);
    angle = func_80079E10(this) > 0 ? 0x100 : 0xA00;
    for (i = 5; i >= 0; i--) {
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_80096CFC(entityOperatorNewAt(0x58, child), this, &pos, angle);
        }
        func_8007AD4C(this, child);
        angle += 0x100;
    }
}

/* Creates an entity of func_80096F6C's class (0x5C bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800957C0(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_80096F6C(entityOperatorNewAt(0x5C, child), this);
    }
    func_8007AD4C(this, child);
}

void func_80095894(Unk80076F34 *this);
void func_80095928(Unk80076F34 *this);

/* A virtual function of Unk80076F34: func_8007B9FC; at step 0 sound 0xB4, at step 7 func_80095894, at step 14
 * func_80095928 20 times. */
void func_8009581C(Unk80076F34 *this, s32 step) {
    s32 i;

    func_8007B9FC(this, step);
    if (step == 0) {
        func_800784F4(this, 0xB4);
    }
    if (step == 7) {
        func_80095894(this);
    }
    if (step == 14) {
        for (i = 0; i < 20; i++) {
            func_80095928(this);
        }
    }
}

/* Creates func_8009764C's entity (0xA4 bytes) facing func_80079E10's side, hands it to func_8007AD4C
 * and plays sound 0xAF. */
void func_80095894(Unk80076F34 *this) {
    Unk8006ED20 *obj = func_80077580(this);
    Entity *child = allocEntityMemory(0xA4);

    if (child != NULL) {
        func_8009764C(entityOperatorNewAt(0xA4, child), this, obj, func_80079E10(this));
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xAF);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095928);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095A70);

/* Creates an entity of func_80097FA0's class (0xB4 bytes) with g++'s new, for this one and its
 * func_80077580 object, and hands it to func_8007AD4C. */
void func_80095C44(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Entity *child = allocEntityMemory(0xB4);

    if (child != NULL) {
        func_80097FA0(entityOperatorNewAt(0xB4, child), this, unk228);
    }
    func_8007AD4C(this, child);
}

/* Spawns the entity of func_80098A78 384 ahead of and 0x140 above the character, with velocity (192
 * forward, 0, 0), then plays sound 0xB0. */
void func_80095CB4(Unk80076F34 *this) {
    s32 facing = func_80079E10(this);
    Unk8006ED20 *unk228;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = facing * 192;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 384;
    position.vy -= 0x140;
    unk228 = func_80077580(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_80098A78(entityOperatorNewAt(0xC8, child), this, unk228, &position, &velocity);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xB0);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80095D88);

/* Pushes the character func_80079E50 finds (AXIS_Z, 0x640000, 0x238) along x by 0x1000 (0x400 when its
 * func_80079E2C is not 0): toward this one when it is 0x221..0x9FF ahead, away from it when 1..0x1E0 ahead,
 * both within 0x57F up or down. */
void func_80095E8C(Unk80076F34 *this) {
    Unk80076F34 *target;
    Vec3 position;
    Vec3 targetPosition;
    s32 dx, dy;
    s32 ahead;
    s32 push;

    target = func_80079E50(this, &AXIS_Z, 0x640000, 0x238);
    if (target != NULL) {
        func_800775D8(this, &position);
        func_800775D8(target, &targetPosition);
        dx = targetPosition.vx - position.vx;
        dy = targetPosition.vy - position.vy;
        ahead = dx * func_80079E10(this);
        if (func_80079E2C(target) == 0) {
            push = 0x1000;
        } else {
            push = 0x400;
        }
        if (dy >= -0x57F && dy <= 0x57F) {
            if (ahead >= 0x221 && ahead <= 0x9FF) {
                if (dx > 0) {
                    target->unk3B4.vx -= push;
                } else {
                    target->unk3B4.vx += push;
                }
            } else if (ahead >= 1 && ahead <= 0x1E0) {
                if (dx > 0) {
                    target->unk3B4.vx += push;
                } else {
                    target->unk3B4.vx -= push;
                }
            }
        }
    }
}

/* The entity of D_80069BDC (func_80095294 makes it, 0x40 bytes): a character's effect at a position,
 * moved 15 down at each of its three gathers. */
typedef struct Unk80095F98 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *unk18; /* the character */
    /* 0x1C */ Vec3 unk1C;         /* a position */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C; /* the character's facing (func_80079E10) */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* 60, less 15 at each gather */
    /* 0x38 */ s32 unk38; /* gathers so far */
    /* 0x3C */ s16 unk3C; /* func_8007768C of the character */
} Unk80095F98;

/* Constructs the entity of D_80069BDC for a character at a position. */
Unk80095F98 *func_80095F98(Unk80095F98 *this, Unk80076F34 *chara, Vec3 *position, s32 facing, s32 arg4) {
    s32 vx, vy, vz;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069BDC;
    this->unk18 = chara;
    vx = position->vx;
    vy = position->vy;
    vz = position->vz;
    this->unk2C = facing;
    this->unk34 = 60;
    this->unk38 = 0;
    this->unk1C.vx = vx;
    this->unk1C.vy = vy;
    this->unk1C.vz = vz;
    this->unk30 = arg4;
    this->unk3C = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80069BDC, which func_80095F98 installs. */
void func_8009603C(Entity *this, s32 flags) {
    this->vtable = &D_80069BDC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of D_80069BDC's entity (Unk80095F98): takes 15 off 0x34 at each of its 3 gathers, then
 * deletes itself.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x38. */
void func_80096090(Unk80095F98 *this, s32 arg) {
    if (this->unk38 >= 3) {
        entityDelete(&this->entity);
    } else {
        this->unk34 -= 15;
        this->unk38++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80096104);

/* One of the four slots at 0xCC of Unk80096984 (func_80096360 moves them, func_800965D8 draws them). */
typedef struct Unk80096984Slot {
    /* 0x00 */ s16 unk0; /* nonzero while the slot is in use */
    /* 0x02 */ u16 unk2;
    /* 0x04 */ Vec3 unk4;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ Vec3 unk14;
    /* 0x20 */ s32 unk20;
} Unk80096984Slot;

/* The entity class func_800962B8 constructs (0x15C bytes, virtual tables D_80069BB4, and D_80069B94 for the
 * base at 0x18; func_80095654 spawns it): an Unk800C7024 with the owner, its obj and four slots. */
typedef struct Unk80096984 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ Unk80076F34 *unkC0; /* the owner */
    /* 0xC4 */ Unk8006ED20 *obj;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ Unk80096984Slot slots[4];
} Unk80096984;

extern EntityVtable D_80069BB4;
extern VtableEntry D_80069B94[];

/* Constructs func_800C7024's entity with 0x80, 0x4A82800, 0x1000, 3, then the virtual tables D_80069BB4 (and
 * D_80069B94 for the base at 0x18), the owner and obj, and frees the four slots. */
Unk80096984 *func_800962B8(Unk80096984 *this, Unk80076F34 *owner, Unk8006ED20 *obj, VECTOR *pos, VECTOR *vel) {
    s32 i;

    func_800C7024(&this->base, owner, (Vec3 *)pos, (Vec3 *)vel, 0x80, 0x4A82800, 0x1000, 3);
    this->base.parts.base.vtable = D_80069B94;
    this->base.parts.entity.vtable = &D_80069BB4;
    this->unkC0 = owner;
    this->obj = obj;
    this->unkC8 = 0;
    for (i = 3; i >= 0; i--) {
        this->slots[i].unk0 = 0;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80096360);
void func_80096984(Unk80096984 *this);

/* The second base's first method of Unk80096984: func_8007BDF0 with sum; when func_80078878 finds an entity in
 * sum, func_80096984's spawn unless func_80078DE0 holds for it, and func_800C7474 with it; else func_800C73A8
 * with 4; then deletes itself. */
void func_80096538(Unk80096984 *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->base.parts.base, sum);
    target = func_80078878(sum);
    if (target != NULL) {
        if (func_80078DE0((Unk80076F34 *)target) == 0) {
            func_80096984(this);
        }
        func_800C7474(&this->base, target);
    } else {
        func_800C73A8(&this->base, 4);
    }
    entityDelete(&this->base.parts.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800965D8);

struct Unk800969E0 *func_800969E0(struct Unk800969E0 *this, Unk80076F34 *chara);

/* Spawns the entity of func_800969E0 with the value at 0xC0, as a sibling. */
void func_80096984(Unk80096984 *this) {
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800969E0(entityOperatorNewAt(0x40, child), this->unkC0);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* The entity class of vtable D_80069B6C (0x40 bytes, func_80096984 spawns it): it follows the position
 * func_8007A650 gives for the partner of a character for 11 frames, cycling unk38 through 0..2. */
typedef struct Unk800969E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position; /* func_8007A650 of func_8007AEEC's character */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ u32 color;     /* 0xC0C0C0 */
    /* 0x30 */ s32 unk30;     /* 0xB3 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ u16 unk38;     /* cycles 0, 1, 2 */
    /* 0x3A */ s16 unk3A;     /* func_8007768C of the character */
    /* 0x3C */ s32 frames;
} Unk800969E0;

/* Constructs the entity of vtable D_80069B6C for a character. */
Unk800969E0 *func_800969E0(Unk800969E0 *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069B6C;
    this->chara = chara;
    this->color = 0xC0C0C0;
    this->unk30 = 0xB3;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3A = func_8007768C(chara);
    this->frames = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069B6C, which func_800969E0 installs. */
void func_80096A54(Entity *this, s32 flags) {
    this->vtable = &D_80069B6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800969E0's update: follows the partner's position (func_8007A650 of func_8007AEEC's character); after
 * 11 frames it deletes itself, before that unk38 cycles 0..2. */
void func_80096AA8(Unk800969E0 *this) {
    func_8007A650(func_8007AEEC(this->chara), &this->position);
    if (this->frames > 10) {
        entityDelete(&this->entity);
    } else {
        if (++this->unk38 >= 3) {
            this->unk38 = 0;
        }
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80096B54);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80096CFC);

/* The update of func_80096CFC's sprite entity: moves by the vector at 0x44 in color 0xA02828 for 5 frames,
 * then deletes itself (Unk800C2F14).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x54,
 * and on the position read before any of it is stored. */
void func_80096DC4(Unk800C2F14 *this) {
    if (this->unk54 > 4) {
        entityDelete(&this->entity);
    } else {
        s32 x = this->sprite.pos.vx;
        s32 y = this->sprite.pos.vy;
        s32 z = this->sprite.pos.vz;

        this->sprite.color = 0xA02828;
        x += this->unk44.vx;
        y += this->unk44.vy;
        z += this->unk44.vz;
        this->sprite.pos.vx = x;
        this->sprite.pos.vy = y;
        this->sprite.pos.vz = z;
        this->unk54++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80096E64);

/* The sprite entity of virtual table D_80069B1C (0x5C bytes, func_800957C0 spawns it): an Unk800C2F14 at the
 * character's point 6 moved by D_80069998, with the character's facing. */
typedef struct Unk80096F6C {
    /* 0x00 */ Unk800C2F14 base;
    /* 0x58 */ s32 facing; /* func_80079E10 of the character */
} Unk80096F6C;

extern u16 D_80069984[];
extern EntityVtable D_80069B1C;
extern VECTOR D_80069998;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069984);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069998);

/* Constructs the sprite of D_80069B1C for a character: func_800C2F14 with frame D_80069984, no motion, size
 * 0x33, color 0x646464 and the character's 0x440 (func_8007768C) as flags. */
Unk80096F6C *func_80096F6C(Unk80096F6C *this, Unk80076F34 *chara) {
    func_800C2F14(&this->base, D_80069984, &D_80010814, &D_80010814, 0x33, 0x646464,
                  func_8007768C(chara));
    this->base.entity.vtable = &D_80069B1C;
    func_8007A4A4(chara, &this->base.sprite.pos, 6, &D_80069998);
    this->facing = func_80079E10(chara);
    return this;
}

/* Unk80096F6C's update (over func_800C3134): after 5 steps it deletes itself, before that its color darkens
 * with unk54 and the sprite grows by 0x33, then the base update.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x54. */
void func_80097008(Unk80096F6C *this, s32 arg) {
    s32 level;
    s32 low;

    if (this->base.unk54 >= 5) {
        entityDelete(&this->base.entity);
    } else {
        level = 100 - this->base.unk54 * 20;
        low = level >> 2;
        this->base.sprite.color = low | (low << 8) | (level << 16);
        this->base.sprite.size += 0x33;
        func_800C3134(&this->base, arg);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800970A8);

/* The entity class of virtual table D_80069AF4 (constructor func_800971E0). */
typedef struct Unk800971E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 unk2C; /* func_8007768C of the character */
} Unk800971E0;

/* Constructs the entity of virtual table D_80069AF4 for chara with arg2 (obj, which its spawner passes, is
 * not read). */
Unk800971E0 *func_800971E0(Unk800971E0 *this, Unk80076F34 *chara, s32 arg2, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069AF4;
    this->chara = chara;
    this->unk1C = 0;
    this->unk20 = 0;
    /* The match depends on storing arg2 after 0x28 (it fills the call's delay slot). */
    this->unk28 = 0;
    this->unk24 = arg2;
    this->unk2C = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80069AF4, which func_800971E0 installs. */
void func_80097250(Entity *this, s32 flags) {
    this->vtable = &D_80069AF4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800971E0's update: once unk28 counts past unk24 it deletes itself; before that unk1C cycles 0..2,
 * unk20 falls by 0x180 and unk28 counts up. */
void func_800972A4(Unk800971E0 *this) {
    if (this->unk28 > this->unk24) {
        entityDelete(&this->entity);
    } else {
        if (++this->unk1C >= 3) {
            this->unk1C = 0;
        }
        this->unk20 -= 0x180;
        this->unk28++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_800699A8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_8009733C);

/* A helper of Unk800971E0's draw (func_8009733C), this not read: out is the point t/4096 of the way from
 * from to to. */
void func_800975D0(Unk800971E0 *this, SVECTOR *out, VECTOR *from, VECTOR *to, s32 t) {
    out->vx = from->vx + (((to->vx - from->vx) * t) >> 12);
    out->vy = from->vy + (((to->vy - from->vy) * t) >> 12);
    out->vz = from->vz + (((to->vz - from->vz) * t) >> 12);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_8009764C);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069ACC, which func_8009764C installs. */
void func_80097790(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069ACC;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80097824);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80097A30);

/* The sprite entity of virtual table D_80069AA4 (func_80095A70 spawns it): an Unk800C2F14 with an angle. */
typedef struct Unk80097D50 {
    /* 0x00 */ Unk800C2F14 base;
    /* 0x58 */ s32 angle; /* in 4096ths (masked with 0xFFF) */
} Unk80097D50;

extern EntityVtable D_80069AA4;

/* Constructs the sprite of D_80069AA4: func_800C2FBC with frame 9 at pos moving by vel, size 2, then the angle
 * (arg1 is not read). */
Unk80097D50 *func_80097D50(Unk80097D50 *this, s32 arg1, VECTOR *pos, VECTOR *vel, s32 angle) {
    func_800C2FBC(&this->base, 9, pos, vel, 2, 0, 0);
    this->base.entity.vtable = &D_80069AA4;
    this->angle = angle & 0xFFF;
    return this;
}

/* Unk80097D50's update: after 11 steps it deletes itself; before that the sprite grows by 7 up to 25, moves
 * by unk44 and its color darkens with unk54. */
void func_80097DAC(Unk80097D50 *this) {
    s32 x, y, z;
    s32 level;
    s32 low;

    if (this->base.unk54 > 10) {
        entityDelete(&this->base.entity);
    } else {
        this->base.sprite.size += 7;
        if (this->base.sprite.size > 25) {
            this->base.sprite.size = 25;
        }
        x = this->base.sprite.pos.vx;
        y = this->base.sprite.pos.vy;
        z = this->base.sprite.pos.vz;
        x += this->base.unk44.vx;
        y += this->base.unk44.vy;
        z += this->base.unk44.vz;
        this->base.sprite.pos.vx = x;
        this->base.sprite.pos.vy = y;
        this->base.sprite.pos.vz = z;
        level = 120 - this->base.unk54 * 10;
        low = level >> 2;
        this->base.sprite.color = low | (low << 8) | (level << 16);
        this->base.unk54++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80097E90);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80097FA0);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A7C, which func_80097FA0 installs. */
void func_80098128(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A7C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800981BC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800983FC);

/* Creates an entity of func_80098720's class (0x44 bytes) with g++'s new, and hands it to entityAddSibling. */
void func_800986C0(Unk80097FA0 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_80098720(entityOperatorNewAt(0x44, child), this->unk84, &this->unk8C);
    }
    entityAddSibling(&this->parts.entity, child);
}

/* The entity class of vtable D_80069A54 (0x44 bytes, func_800986C0 spawns it): a character's effect at a
 * position. */
typedef struct Unk80098720 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C; /* 0x320 */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* 0x5A00 */
    /* 0x38 */ s32 unk38; /* 20 */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s16 unk40; /* func_8007768C of the character */
} Unk80098720;

/* Constructs the entity of vtable D_80069A54 for a character at a position. */
Unk80098720 *func_80098720(Unk80098720 *this, Unk80076F34 *chara, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069A54;
    this->chara = chara;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unk2C = 0x320;
    this->unk30 = 0;
    this->unk34 = 0x5A00;
    this->unk38 = 20;
    this->unk3C = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk40 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_80069A54, which func_80098720 installs. */
void func_800987B8(Entity *this, s32 flags) {
    this->vtable = &D_80069A54;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_8009880C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800988A4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80098A78);

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A2C, which func_80098A78 installs. */
void func_80098C04(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A2C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80098C98);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80098ECC);

/* The UnkPartsEntity3C class func_80098A78 constructs (0xC8 bytes, virtual table D_80069A2C). */
typedef struct Unk80098A78 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *chara;
    /* 0x88 */ Unk8006ED20 *unk228;
    /* 0x8C */ Vec3 pos;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 vel;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 direction; /* 1, or -1 when vel.vx is negative */
    /* 0xB4 */ s16 unkB4;     /* func_8007768C of the character */
    /* 0xB6 */ s16 unkB6;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
} Unk80098A78;

/* The entity class of vtable D_80069A04 (0x54 bytes, func_8009918C spawns it, constructor func_8009921C). */
typedef struct Unk8009921C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *unk228;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 direction;
    /* 0x34 */ Vec3 velocity; /* vx: 12 along direction */
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 unk44; /* func_8007768C of the character */
    /* 0x46 */ u16 unk46; /* toggled every gather */
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C; /* 0x20 */
    /* 0x50 */ s32 unk50; /* gathers so far */
} Unk8009921C;

Unk8009921C *func_8009921C(Unk8009921C *this, Unk80076F34 *chara, Unk8006ED20 *unk228, Vec3 *position,
                           s32 direction, s32 unk48);

/* Spawns func_8009921C's entity (0x54 bytes) at pos as a sibling, with the direction and unkBC (at least
 * 300). */
void func_8009918C(Unk80098A78 *this) {
    s32 unk48 = this->unkBC;
    Entity *child;

    if (unk48 < 300) {
        unk48 = 300;
    }
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_8009921C(entityOperatorNewAt(0x54, child), this->chara, this->unk228, &this->pos, this->direction,
                      unk48);
    }
    entityAddSibling(&this->parts.entity, child);
}

/* Constructs the entity of vtable D_80069A04 for a character at a position, its step 12 along direction. */
Unk8009921C *func_8009921C(Unk8009921C *this, Unk80076F34 *chara, Unk8006ED20 *unk228, Vec3 *position,
                           s32 direction, s32 unk48) {
    s32 x, y, z;
    s32 step;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069A04;
    this->chara = chara;
    this->unk228 = unk228;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    step = -12;
    /* The game clears the velocity with the next word and sets vx again below. */
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->unk40 = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->direction = direction;
    this->unk48 = unk48;
    if (direction > 0) {
        step = 12;
    }
    this->velocity.vx = step;
    this->unk44 = func_8007768C(chara);
    this->unk46 = 0;
    this->unk4C = 0x20;
    this->unk50 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069A04, which func_8009921C installs. */
void func_800992F4(Entity *this, s32 flags) {
    this->vtable = &D_80069A04;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of D_80069A04's entity: moves by its velocity, grows 0x4C by 16 for its first 5 gathers and then
 * shrinks it by 6, and deletes itself once 0x4C is no longer positive. */
void func_80099348(Unk8009921C *this) {
    /* The match depends on reading position and velocity before adding (as vecAddInPlace) */
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;
    s32 dx = this->velocity.vx;
    s32 dy = this->velocity.vy;
    s32 dz = this->velocity.vz;

    x += dx;
    y += dy;
    z += dz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (this->unk50 < 5) {
        this->unk4C += 16;
    } else {
        this->unk4C -= 6;
    }
    if (this->unk4C <= 0) {
        entityDelete(&this->entity);
    } else {
        this->unk46 ^= 1;
        this->unk50++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_80099410);

/* The entity class of virtual table D_800699DC (constructor func_800995BC, size 0x58): a line
 * (buildLineG2) whose two ends start at one position and move by the velocity and by twice it. */
typedef struct Unk800995BC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ Unk8006ED20 *unk1C;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ Vec3 unk30; /* the other end, moving twice as fast */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ Vec3 velocity;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54; /* gathers so far */
} Unk800995BC;

/* Constructs the line entity of virtual table D_800699DC for chara, with both ends at position. */
Unk800995BC *func_800995BC(Unk800995BC *this, Unk80076F34 *chara, Unk8006ED20 *unk1C, Vec3 *position,
                           Vec3 *velocity) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800699DC;
    this->chara = chara;
    this->unk1C = unk1C;
    /* The match depends on each copy having locals of its own (three words read before any is stored) */
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->unk30.vx = x;
        this->unk30.vy = y;
        this->unk30.vz = z;
    }
    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;

        this->unk50 = 200;
        this->unk54 = 0;
        this->velocity.vx = x;
        this->velocity.vy = y;
        this->velocity.vz = z;
    }
    return this;
}

/* The destructor of the entity class of vtable D_800699DC, which func_800995BC installs. */
void func_80099680(Entity *this, s32 flags) {
    this->vtable = &D_800699DC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800996D4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_08", func_800997D0);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800998E0(UnkPartsEntity38 *this, s32 flags) {
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

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80099980(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800999D4(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80099A28(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_800699DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069A04);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069A2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069A54);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069A7C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069AA4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069ACC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069AF4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069B1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069B44);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069B6C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069B94);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069BB4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069BDC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_08", D_80069C04);
