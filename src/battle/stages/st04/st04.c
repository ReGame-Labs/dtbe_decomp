#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "psyq.h"
#include "engine/math/sin_cos.h"
#include "engine/task/entity.h"
#include "engine/system/memory.h"

/* the virtual tables the functions below install, and the data they read */
extern VtableEntry D_800D3914[];
extern EntityVtable D_800D3934;
extern EntityVtable D_800D395C;
extern EntityVtable D_800D3984;
extern EntityVtable D_800D39AC;
extern EntityVtable D_800D39D4;
extern EntityVtable D_800D39FC;
extern EntityVtable D_800D3A24;

/* The stage's class, derived from func_8006ED20's (virtual table D_800D3A4C). */
typedef struct Unk800D3AC4 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ s32 unkD38; /* counts the stage's updates (func_800D3CB8) */
    /* 0xD3C */ s32 unkD3C;
    /* 0xD40 */ u8 unkD40[0x1D68 - 0xD40];
    /* 0x1D68 */ RECT unk1D68;     /* 16 by 1 pixels at (0x330, 0xE6): a CLUT */
    /* 0x1D70 */ u8 unk1D70[0x1D90 - 0x1D70];
    /* 0x1D90 */ u16 unk1D90[16]; /* that CLUT, read back from VRAM by func_800D40BC */
    /* 0x1DB0 */ u8 unk1DB0[0x1DCC - 0x1DB0];
    /* 0x1DCC */ VECTOR unk1DCC[2]; /* by side: the character's position, each update (func_800D3CB8) */
    /* 0x1DEC */ s32 unk1DEC[2]; /* by side, as the next two (func_800D49D0) */
    /* 0x1DF4 */ s32 unk1DF4[2];
    /* 0x1DFC */ VECTOR unk1DFC[2];
    /* 0x1E1C */ s32 unk1E1C;
    /* 0x1E20 */ s32 unk1E20;
    /* 0x1E24 */ u8 unk1E24[0x1E3C - 0x1E24];
    /* the per-side state func_800D4DEC resets */
    /* 0x1E3C */ s32 unk1E3C[2];
    /* 0x1E44 */ u8 unk1E44[0x1E54 - 0x1E44];
    /* 0x1E54 */ s16 unk1E54[2];
    /* 0x1E58 */ u8 unk1E58[0x1E60 - 0x1E58];
    /* 0x1E60 */ s16 unk1E60[2];
    /* 0x1E64 */ u8 unk1E64[0x1E6C - 0x1E64];
    /* 0x1E6C */ VECTOR unk1E6C[2]; /* reset to the next one */
    /* 0x1E8C */ u8 unk1E8C[0x1ECC - 0x1E8C];
    /* 0x1ECC */ VECTOR unk1ECC[2];
    /* 0x1EEC */ u8 unk1EEC[0x1F2C - 0x1EEC];
    /* 0x1F2C */ VECTOR unk1F2C[2];
    /* 0x1F4C */ u8 unk1F4C[0x1F8C - 0x1F4C];
    /* 0x1F8C */ VECTOR unk1F8C[2];
    /* 0x1FAC */ u8 unk1FAC[0x1FF4 - 0x1FAC];
    /* the per-side state func_800D56E4 resets */
    /* 0x1FF4 */ s32 unk1FF4[2];
    /* 0x1FFC */ s16 unk1FFC[2];
    /* 0x2000 */ s16 unk2000[2];
    /* 0x2004 */ s16 unk2004[2];
    /* 0x2008 */ VECTOR unk2008[2]; /* reset to the next one */
    /* 0x2028 */ VECTOR unk2028[2];
    /* 0x2048 */ VECTOR unk2048[2];
    /* 0x2068 */ VECTOR unk2068[2];
    /* 0x2088 */ u16 unk2088; /* set while func_8006F1A4 holds the stage's update */
} Unk800D3AC4;

extern VtableEntry D_800D3A4C[];

/* Constructs the stage's object: func_8006ED20's, with virtual table D_800D3A4C. */
Unk800D3AC4 *func_800D3AC4(Unk800D3AC4 *this) {
    func_8006ED20(&this->base);
    this->base.vtable = D_800D3A4C;
    this->unkD3C = 0;
    return this;
}

/* Destroys the stage's object: virtual table D_800D3A4C, then func_8006ED20's destructor. */
void func_800D3AFC(Unk800D3AC4 *this, s32 flags) {
    this->base.vtable = D_800D3A4C;
    func_8006F254(&this->base, flags);
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D3B24);

/* game.bin functions missing from game.h */
void func_80071F6C(void);
Unk80076F34 *func_800700C8(Unk8006ED20 *this, s32 side);
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out);
s32 func_8006F144(Unk8006ED20 *object);
s32 func_8006F190(Unk8006ED20 *object);
s32 func_8006F1A4(Unk8006ED20 *object);

/* the stage's own update steps */
void func_800D3EA4(Unk800D3AC4 *this);
void func_800D3FC8(Unk800D3AC4 *this);
void func_800D4104(Unk800D3AC4 *this);
void func_800D41A4(Unk800D3AC4 *this);
void func_800D44D4(Unk800D3AC4 *this);
void func_800D4700(Unk800D3AC4 *this);
void func_800D49D0(Unk800D3AC4 *this);
void func_800D4FE0(Unk800D3AC4 *this);
void func_800D58C0(Unk800D3AC4 *this);

/* The stage's update: func_80071F6C, both characters' positions into 0x1DCC; 0x2088 flags func_8006F1A4;
 * otherwise, unless func_8006F144 holds without func_8006F190, its nine steps, counted at 0xD38
 * (st00's func_800D4A70). */
void func_800D3CB8(Unk800D3AC4 *this) {
    func_80071F6C();
    func_800775D8(func_800700C8(&this->base, 0), (Vec3 *)&this->unk1DCC[0]);
    func_800775D8(func_800700C8(&this->base, 1), (Vec3 *)&this->unk1DCC[1]);
    if (func_8006F1A4(&this->base)) {
        this->unk2088 = 1;
    } else {
        this->unk2088 = 0;
        if (!func_8006F144(&this->base) || func_8006F190(&this->base)) {
            func_800D4FE0(this);
            func_800D58C0(this);
            func_800D3EA4(this);
            func_800D3FC8(this);
            func_800D4104(this);
            func_800D49D0(this);
            func_800D41A4(this);
            func_800D4700(this);
            func_800D44D4(this);
            this->unkD38++;
        }
    }
}

/* Copies the 16 by 32 pixels at (0x2E0, 0x80) and at (0x2E0, 0xC0) in VRAM to (0x2E0, 0xA0) and (0x2E0, 0xE0);
 * the stage's init calls it. */
void func_800D3DA0(Unk800D3AC4 *this) {
    RECT rect;

    rect.x = 0x2E0;
    rect.y = 0x80;
    rect.w = 16;
    rect.h = 32;
    MoveImage(&rect, 0x2E0, 0xA0);
    rect.x = 0x2E0;
    rect.y = 0xC0;
    rect.w = 16;
    rect.h = 32;
    MoveImage(&rect, 0x2E0, 0xE0);
}

/* Copies the 16 by 64 pixels at (0x358, 0x30) in VRAM to (0x2E0, 0) and to (0x2E0, 0x40); the stage's init
 * calls it first. */
void func_800D3E20(Unk800D3AC4 *this) {
    RECT rect;

    rect.x = 0x358;
    rect.y = 0x30;
    rect.w = 16;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0);
    rect.x = 0x358;
    rect.y = 0x30;
    rect.w = 16;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0x40);
}

/* On even updates, copies the 16 by 64 pixels at (0x2E0, 63 - update % 64) in VRAM to (0x358, 0x30): a
 * texture that scrolls one line per two updates. The even test is written "(count ^ 1) & 1" (xori, andi). */
void func_800D3EA4(Unk800D3AC4 *this) {
    s32 count = this->unkD38;
    RECT rect;

    if ((count ^ 1) & 1) {
        rect.x = 0x2E0;
        rect.y = ~count & 0x3F;
        rect.w = 16;
        rect.h = 64;
        MoveImage(&rect, 0x358, 0x30);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D3F04);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D3FC8);

/* Reads the 16-colour CLUT at (0x330, 0xE6) back from VRAM into 0x1D90. */
void func_800D40BC(Unk800D3AC4 *this) {
    this->unk1D68.x = 0x330;
    this->unk1D68.y = 0xE6;
    this->unk1D68.w = 16;
    this->unk1D68.h = 1;
    func_80055B88(&this->unk1D68, (u_long *)this->unk1D90);
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4104);

/* Clears the stage's words at 0x1E20 and 0x1E1C. */
void func_800D4198(Unk800D3AC4 *this) {
    this->unk1E20 = 0;
    this->unk1E1C = 0;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D41A4);

/* Clears the stage's per-side words at 0x1DEC and 0x1DF4 and its two vectors at 0x1DFC. */
void func_800D4280(Unk800D3AC4 *this) {
    /* The match depends on the chained clears: the game stores [1] before [0]. */
    this->unk1DEC[0] = this->unk1DEC[1] = 0;
    this->unk1DF4[0] = this->unk1DF4[1] = 0;
    this->unk1DFC[0].vx = 0;
    this->unk1DFC[0].vy = 0;
    this->unk1DFC[0].vz = 0;
    this->unk1DFC[0].pad = 0;
    this->unk1DFC[1].vx = 0;
    this->unk1DFC[1].vy = 0;
    this->unk1DFC[1].vz = 0;
    this->unk1DFC[1].pad = 0;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D42B4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D43D4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D44D4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4700);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D49D0);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4BE0);

/* Clears the four words of a VECTOR. The match depends on this helper: it takes the vector's address once,
 * where the fields written out each add the offset to their store. */
static inline void clearVector(VECTOR *v) {
    v->vx = 0;
    v->vy = 0;
    v->vz = 0;
    v->pad = 0;
}

/* Resets one side's state: two vectors and three counters cleared, 0x1E6C set back to 0x1ECC. */
void func_800D4DEC(Unk800D3AC4 *this, s32 side) {
    clearVector(&this->unk1F2C[side]);
    clearVector(&this->unk1F8C[side]);
    this->unk1E60[side] = 0;
    this->unk1E54[side] = 0;
    this->unk1E3C[side] = 0;
    this->unk1E6C[side].vx = this->unk1ECC[side].vx;
    this->unk1E6C[side].vy = this->unk1ECC[side].vy;
    this->unk1E6C[side].vz = this->unk1ECC[side].vz;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4E6C);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D4FE0);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5430);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5598);

/* Resets one side's other state: two vectors and 0x1FF4 cleared, 0x2004 and 0x1FFC set to 0x1000, 0x2008 set back
 * to 0x2028. */
void func_800D56E4(Unk800D3AC4 *this, s32 side) {
    clearVector(&this->unk2048[side]);
    clearVector(&this->unk2068[side]);
    this->unk2004[side] = 0x1000;
    this->unk2000[side] = 0;
    this->unk1FF4[side] = 0;
    this->unk1FFC[side] = 0x1000;
    this->unk2008[side].vx = this->unk2028[side].vx;
    this->unk2008[side].vy = this->unk2028[side].vy;
    this->unk2008[side].vz = this->unk2028[side].vz;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5770);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D58C0);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5EA4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D5F94);

/* The destructor of the entity class of vtable D_800D3A24, which func_800D5F94 installs. */
void func_800D60E8(Entity *this, s32 flags) {
    this->vtable = &D_800D3A24;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D613C);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6384);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6520);

/* The destructor of the entity class of vtable D_800D39FC, which func_800D6520 installs. */
void func_800D667C(Entity *this, s32 flags) {
    this->vtable = &D_800D39FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D66D0);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6938);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6A9C);

/* The destructor of the entity class of vtable D_800D39D4, which func_800D6A9C installs. */
void func_800D6B64(Entity *this, s32 flags) {
    this->vtable = &D_800D39D4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6BB8);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D6DC0);

/* The 0x180-byte records of a table in game.bin, indexed by func_80077568's value (st08 reads 0x0's bit 0). */
typedef struct Unk800656D4 {
    /* 0x000 */ u16 unk0;
    /* 0x002 */ s16 unk2; /* half the size of the square of func_800D6F9C */
    /* 0x004 */ u8 unk4[0x180 - 0x4];
} Unk800656D4;

extern Unk800656D4 D_800656D4[];

/* the character calls of game.c the entity below makes */
Vec3 *func_8007A650(Unk80076F34 *chara, Vec3 *out);
s32 func_80077568(Unk80076F34 *this);

/* The stage's entity of func_800D6F9C (virtual table D_800D39AC): a square around a side's character. */
typedef struct Unk800D6F9C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D3AC4 *stage;
    /* 0x1C */ u16 side;
    /* 0x20 */ u32 unk20; /* 0xF08080 at construction */
    /* 0x24 */ u8 unk24[0x34 - 0x24];
    /* 0x34 */ SVECTOR unk34[4]; /* the square's corners, in the z = 0 plane */
} Unk800D6F9C;

/* Constructs the stage's entity for a side: its corners are +-2 * 0x2 of the character's record.
 * The match depends on the record taken as "table + index" (an index or a record pointer local swaps
 * the address registers) and on the chained corner stores (the game stores the last first). */
Unk800D6F9C *func_800D6F9C(Unk800D6F9C *this, Unk800D3AC4 *stage, u16 side) {
    Vec3 position;
    s32 size;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D39AC;
    this->stage = stage;
    this->side = side;
    this->unk20 = 0xF08080;
    func_8007A650(func_800700C8(&stage->base, side), &position);
    size = (D_800656D4 + func_80077568(func_800700C8(&this->stage->base, this->side)))->unk2 * 2;
    this->unk34[0].vz = this->unk34[1].vz = this->unk34[2].vz = this->unk34[3].vz = 0;
    this->unk34[0].vx = this->unk34[2].vx = -size;
    this->unk34[1].vx = this->unk34[3].vx = size;
    this->unk34[0].vy = this->unk34[1].vy = -size;
    this->unk34[2].vy = this->unk34[3].vy = size;
    return this;
}

/* The destructor of the entity class of vtable D_800D39AC, which func_800D6F9C installs. */
void func_800D7084(Entity *this, s32 flags) {
    this->vtable = &D_800D39AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D70D8);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D71D0);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D72D8);

/* The destructor of the entity class of vtable D_800D3984, which func_800D72D8 installs. */
void func_800D7458(Entity *this, s32 flags) {
    this->vtable = &D_800D3984;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D74AC);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D770C);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7900);

/* The destructor of the entity class of vtable D_800D395C, which func_800D7900 installs. */
void func_800D7A64(Entity *this, s32 flags) {
    this->vtable = &D_800D395C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7AB8);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7CE4);

INCLUDE_ASM("st04/nonmatchings/st04", func_800D7F70);

/* The destructor of the entity class of vtable D_800D3934, which func_800D7F70 installs. */
void func_800D80C8(Entity *this, s32 flags) {
    this->vtable = &D_800D3934;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity of vtable D_800D3934 (func_800D7F70), with a pulsing level. */
typedef struct Unk800D7F70 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 phase;
    /* 0x1A */ u8 level;
} Unk800D7F70;

/* The update of D_800D3934's entity: the level follows a sine of the phase (0x6F +- 16), and the phase steps. */
void func_800D811C(Unk800D7F70 *this) {
    this->level = (COS_SIN_TABLE[(this->phase << 4) & 0xFFF].sin >> 8) + 0x6F;
    this->phase++;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D8158);

/* Constructs an object of D_800D3914 for arg, with the vector (0x60, 0, 0). */
UnkStageObject *func_800D848C(UnkStageObject *this, s32 arg) {
    Vec3 vec;

    /* The match depends on vec, a whole vector copied in, and on filling it after the vtable store. */
    this->vtable = D_800D3914;
    vec.vx = 0x60;
    vec.vy = 0;
    vec.vz = 0;
    this->unk4 = arg;
    this->unk8 = vec;
    return this;
}

/* Entry 0xC of D_800D3914: adds the word of the object to the part's unk18, and its vector to the part's
 * unk20 (func_80075B48 calls it for the part linked to the one the object owns). */
void func_800D84C8(UnkStageObject *this, Unk80075EE4 *part) {
    s32 x, y, z;

    part->unk18 += this->unk4;
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = part->unk20.vx;
    y = part->unk20.vy;
    z = part->unk20.vz;
    x += this->unk8.vx;
    y += this->unk8.vy;
    z += this->unk8.vz;
    part->unk20.vx = x;
    part->unk20.vy = y;
    part->unk20.vz = z;
}

INCLUDE_ASM("st04/nonmatchings/st04", func_800D850C);

/* The entity class func_800D850C constructs (virtual table D_800D38EC): an
 * Unk80076308 base at 0x18 and an Unk800761AC member at 0x30. */
typedef struct Unk800D850C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076308 unk18;
    /* 0x1C */ u8 unk1C[0x30 - 0x1C];
    /* 0x30 */ Unk800761AC unk30;
} Unk800D850C;

extern EntityVtable D_800D38EC;

/* Unk800D850C's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D8618(Unk800D850C *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &D_800D38EC;
    this->unk30.vtable = D_80065184;
    /* listRemove(&this->unk30.link) written out: the match depends on this
     * access order (soundControlDestroy's); the inline gives 12 diffs */
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

INCLUDE_ASM("st04/nonmatchings/st04", func_800D86AC);

/* The destructor of the class of virtual table D_800D3914: Unk80076308's, inlined. */
void func_800D8938(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the object's vector, reading all three components before writing any (as vecCopy
 * does). */
void func_800D896C(UnkStageObject *this, Vec3 *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk8.vx = x;
    this->unk8.vy = y;
    this->unk8.vz = z;
}
