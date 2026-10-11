#include "common.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "battle/game_classes.h"
#include "engine/gfx/look_at.h"
#include "engine/math/random.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* The stage's class, which func_800D3CEC constructs on func_8006ED20's. */
typedef struct Unk800D3CEC {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ u8 unkD38[0xD7C - 0xD38];
    /* 0xD7C */ s32 unkD7C; /* adds 4 per unit to the rise of func_800D5968's entities (func_800D5A90) */
    /* 0xD80 */ u32 unkD80; /* a sound's handle (stopSound) */
} Unk800D3CEC;

void func_800D4BB4(void *this);

/* the virtual tables the functions below install */
extern VtableEntry D_800D3C0C[];

INCLUDE_ASM("st05/nonmatchings/st05", func_800D3CEC);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D3FC0);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D40CC);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4210);

/* game.bin functions missing from game.h */
void func_80070314(Unk8006ED20 *object);
s32 func_8006F144(Unk8006ED20 *object);
s32 func_8006F190(Unk8006ED20 *object);
Unk80076F34 *func_800700C8(Unk8006ED20 *object, s32 side);
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out);
void func_800775B0(Unk80076F34 *chara, Vec3 *position);
Unk80077698 *func_80077698(Unk80076F34 *this);

/* the stage's steps for one character */
void func_800D4570(Unk800D3CEC *this, Unk80076F34 *chara);
void func_800D45E0(Unk800D3CEC *this, Unk80076F34 *chara);

/* The stage's step for the characters: func_80070314, then for each side func_800D4570 and, unless
 * func_8006F144 or func_8006F190 holds, func_800D45E0. */
void func_800D44CC(Unk800D3CEC *this) {
    s32 held;
    s32 side;
    Unk80076F34 *chara;

    func_80070314(&this->base);
    held = 0;
    if (func_8006F144(&this->base)) {
        held = 1;
    } else if (func_8006F190(&this->base)) {
        held = 1;
    }
    for (side = 0; side < 2; side++) {
        chara = func_800700C8(&this->base, side);
        func_800D4570(this, chara);
        if (!held) {
            func_800D45E0(this, chara);
        }
    }
}

/* Keeps a character's x within -0xE00..0xF00 (gcc folds the in-range test into one unsigned compare).
 * The match depends on the clamp testing the low bound first, with the high one inside it. */
void func_800D4570(Unk800D3CEC *this, Unk80076F34 *chara) {
    Vec3 position;
    s32 x;

    func_800775D8(chara, &position);
    x = position.vx;
    if (x >= -0xE00 && x <= 0xF00) {
        return;
    }
    if (x >= -0xE00) {
        if (x > 0xF00) {
            x = 0xF00;
        }
    } else {
        x = -0xE00;
    }
    position.vx = x;
    func_800775B0(chara, &position);
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D45E0);

/* A value by bits 8-9 of the low halfword at 0 of the character's table (func_80077698): 0xC0, 0x40,
 * 0x80 or 0x100. game_classes.h's Unk80077698 has 0x0 as one word; the game reads its low half (lhu 0). */
s32 func_800D4768(Unk80076F34 *chara) {
    switch (*(u16 *)&func_80077698(chara)->unk0 & 0x300) {
    case 0x000:
        return 0xC0;
    case 0x100:
        return 0x40;
    case 0x200:
        return 0x80;
    case 0x300:
        return 0x100;
    }
    return 0xC0;
}

/* The stage's override of func_80070824 (at 0x58 of D_800D3C2C): whether a
 * character's position (func_800706A4 passes it) has a y greater than 0x400. */
s32 func_800D47D4(Unk800D3CEC *this, Vec3 *position) {
    return position->vy > 0x400;
}

/* The base's func_80072748, then the stage's own part (func_800D4BB4). */
void func_800D47E8(void *this) {
    func_80072748(this);
    func_800D4BB4(this);
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4814);

/* the camera's update of func_8006ED20's class, and its LookAt (at 0x12C) */
void func_80072088(Unk8006ED20 *object);
LookAt *func_8006EF64(Unk8006ED20 *object);

/* The camera's eye of func_8006ED20's class, which func_80072088 computes (game.h leaves 0x2CC-0x360
 * as a gap). */
#define BASE_EYE(stage) (*(Vec3 *)&(stage)->base.unk2CC[0x31C - 0x2CC])

/* The stage's camera step: func_80072088, then keeps the eye at y -0x300 or above it (lower y), copying
 * it into the LookAt when it moves it. The match depends on the eye read into locals before the copy
 * (the LookAt's stores could alias the stage, so a direct copy interleaves the loads). */
void func_800D4A10(Unk800D3CEC *this) {
    LookAt *lookAt;
    s32 x;
    s32 y;
    s32 z;

    func_80072088(&this->base);
    if (BASE_EYE(this).vy > -0x300) {
        BASE_EYE(this).vy = -0x300;
        lookAt = func_8006EF64(&this->base);
        lookAt->dirty = 1;
        x = BASE_EYE(this).vx;
        y = BASE_EYE(this).vy;
        z = BASE_EYE(this).vz;
        lookAt->eye.vx = x;
        lookAt->eye.vy = y;
        lookAt->eye.vz = z;
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4A74);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4B10);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4BB4);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4D3C);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4E04);

void func_800D4FFC(void) {
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5004);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5144);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5254);

/* Starts sound 0x67 into 0xD80, unless one is playing there. */
void func_800D5364(Unk800D3CEC *this) {
    if (this->unkD80 == 0) {
        this->unkD80 = soundControlPlaySound(&SOUND_CONTROL, 0x67, 0);
    }
}

/* Stops the sound at 0xD80, if there is one. */
void func_800D53A8(Unk800D3CEC *this) {
    if (this->unkD80 != 0) {
        stopSound(this->unkD80);
        this->unkD80 = 0;
    }
}

/* The entity class func_800D53E4 constructs (virtual table D_800D3BE4): tables of halfwords from 0x47E,
 * then the handle of sound 0x66. */
typedef struct Unk800D53E4 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x8D8 - 0x18];
    /* 0x8D8 */ u32 sound; /* soundControlPlaySound's handle */
    /* 0x8DC */ s32 unk8DC;
} Unk800D53E4;

extern EntityVtable D_800D3BE4;

INCLUDE_ASM("st05/nonmatchings/st05", func_800D53E4);

/* The destructor of func_800D53E4's class: stops its sound. */
void func_800D54B8(Unk800D53E4 *this, s32 flags) {
    this->entity.vtable = &D_800D3BE4;
    stopSound(this->sound);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5518);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5690);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5968);

/* The entity class func_800D5968 constructs (virtual table D_800D3BBC): a rising particle that keeps its
 * last four positions. */
typedef struct Unk800D5968 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR positions[4]; /* a ring; current indexes the newest */
    /* 0x58 */ VECTOR velocity;
    /* 0x68 */ s32 current;
    /* 0x6C */ s32 life; /* 128 to 255 at construction, 2 less per update */
} Unk800D5968;

/* func_800D5968's class's update: the next position is the newest plus the velocity, then the velocity
 * drifts by -3..3 in x and z and its y becomes -(16 + 4 * the stage's 0xD7C) minus 0..7; the entity
 * deletes itself when its life runs out. The match depends on the newest position read before the
 * velocity is added, on 0xD7C read before the random calls, and on the y's two parts in locals of their
 * own (an inline "- (extra + 0x10)" folds the 0x10 elsewhere). */
void func_800D5A90(Unk800D5968 *this, Unk800D3CEC *stage) {
    VECTOR *newest;
    VECTOR *position;
    s32 next;
    s32 x;
    s32 y;
    s32 z;
    s32 extra;
    s32 jitter;
    s32 rise;

    newest = &this->positions[this->current];
    next = (this->current + 1) & 3;
    x = newest->vx;
    y = newest->vy;
    z = newest->vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    position = &this->positions[next];
    position->vx = x;
    position->vy = y;
    position->vz = z;
    this->current = next;
    extra = stage->unkD7C * 4;
    this->velocity.vx += (s32)mersenneTwisterGenerate(&RANDOM) % 4;
    jitter = -(mersenneTwisterGenerate(&RANDOM) & 7);
    rise = extra + 0x10;
    this->velocity.vy = jitter - rise;
    this->velocity.vz += (s32)mersenneTwisterGenerate(&RANDOM) % 4;
    this->life -= 2;
    if (this->life <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5BD4);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5D54);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5E3C);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D621C);

/* The entity class func_800D627C constructs (virtual table D_800D3B6C). */
typedef struct Unk800D627C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800D627C;

extern EntityVtable D_800D3B6C;

/* Constructs an Unk800D627C at position, with unk30 and unk34; 0x38 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D627C *func_800D627C(Unk800D627C *this, Vec3 *position, s32 unk30, s32 unk34) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B6C;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = 0;
    this->unk30 = unk30;
    this->unk34 = unk34;
    this->unk38 = 0xFF;
    return this;
}

/* Unk800D627C's update: 0x28 and 0x2C close a quarter of their distance to 0x30 and 0x34, and 0x38 falls
 * by 0x18; the entity deletes itself when 0x38 runs out. */
void func_800D6308(Unk800D627C *this) {
    this->unk28 += (this->unk30 - this->unk28) >> 2;
    this->unk2C += (this->unk34 - this->unk2C) >> 2;
    this->unk38 -= 0x18;
    if (this->unk38 <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D6388);

/* The entity class func_800D64FC constructs (virtual table D_800D3B44). */
typedef struct Unk800D64FC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
} Unk800D64FC;

extern EntityVtable D_800D3B44;

/* Constructs an Unk800D64FC at position, with unk30 and unk2C; 0x34 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D64FC *func_800D64FC(Unk800D64FC *this, Vec3 *position, s32 unk30, s32 unk2C) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B44;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = unk2C;
    this->unk30 = unk30;
    this->unk34 = 0xFF;
    return this;
}

/* Unk800D64FC's update: 0x28 moves by 0x2C, which loses an eighth of itself, and 0x34 falls by 8; the
 * entity deletes itself when 0x34 runs out. */
void func_800D6584(Unk800D64FC *this) {
    this->unk28 += this->unk2C;
    this->unk2C += -this->unk2C >> 3;
    this->unk34 -= 8;
    if (this->unk34 <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D65F8);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D6740);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D6814);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D68A4);

/* The stage's 0xD7C. */
s32 func_800D6A08(Unk800D3CEC *this) {
    return this->unkD7C;
}

/* The destructor of the class of virtual table D_800D3C0C: Unk80076308's, inlined. */
void func_800D6A14(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an object of the class of vtable D_800D3C0C, which derives from the class of
 * D_80065164, with a word. */
UnkStageObject *func_800D6A48(UnkStageObject *this, s32 arg) {
    this->vtable = D_800D3C0C;
    this->unk4 = arg;
    return this;
}

/* Sets the object's vector, reading all three components before writing any (as vecCopy
 * does). */
void func_800D6A60(UnkStageObject *this, Vec3 *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk8.vx = x;
    this->unk8.vy = y;
    this->unk8.vz = z;
}

/* The destructor of the entity class of vtable D_800D3BBC, which func_800D5968 installs. */
void func_800D6A7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class func_800D5D54 constructs (virtual table D_800D3B94): an
 * Unk80076308 base at 0x18 (virtual table D_800D3C0C) and a member of
 * func_80075EE4's class at 0x30. */
typedef struct Unk800D5D54 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076308 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ Vec3 unk20; /* func_800D5D54's third argument */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ Unk800761AC unk30;
} Unk800D5D54;

/* Unk800D5D54's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D6AC4(Unk800D5D54 *this, s32 flags) {
    ListNode *link;

    this->unk30.vtable = D_80065184;
    /* listRemove(&this->unk30.link) written out: the match depends on this
     * access order (soundControlDestroy's), as in st04's func_800D8618 */
    link = &this->unk30.link;
    this->unk30.link.next->prev = this->unk30.link.prev;
    link->prev->next = this->unk30.link.next;
    this->unk30.link.next = link;
    this->unk30.link.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B6C, which func_800D627C installs. */
void func_800D6B4C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B44, which func_800D64FC installs. */
void func_800D6B94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B1C, which func_800D6740 installs. */
void func_800D6BDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
