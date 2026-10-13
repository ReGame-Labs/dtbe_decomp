#include "common.h"
#include "engine/task/entity.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/math/vector.h"
#include "engine/system/memory.h"

/* the constructor func_800D3EB0 calls, and what that constructor calls last */
UnkStageQuad *func_800D5080(UnkStageQuad *this, Unk8006ED20 *owner, s32 index);
void func_800D53AC(UnkStageQuad *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3874;
extern EntityVtable D_800D389C;
extern EntityVtable D_800D38C4;
extern EntityVtable D_800D38EC;
extern EntityVtable D_800D3914;

/* The stage's object of func_800D3C08, derived from func_8007C084's class. */
typedef struct Unk800D39B4 {
    /* 0x00 */ Unk8007C084 base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} Unk800D39B4;

/* The stage's class, derived from func_800D02C8's (virtual table D_800D393C). */
typedef struct Unk800D3B14 {
    /* 0x000 */ Unk800D02C8 base;
    /* 0xDA8 */ u8 unkDA8[0xF2C - 0xDA8];
    /* 0xF2C */ s16 unkF2C[2]; /* unkF84 again, each frame (func_800D4044) */
    /* 0xF30 */ u8 unkF30[0xF34 - 0xF30];
    /* 0xF34 */ s16 unkF34;
    /* 0xF36 */ u8 unkF36[0xF68 - 0xF36];
    /* 0xF68 */ s16 unkF68; /* 90 after the init */
    /* 0xF6A */ s16 unkF6A;
    /* 0xF6C */ u8 unkF6C[0xF70 - 0xF6C];
    /* 0xF70 */ s16 unkF70; /* counts the stage's updates (func_800D3D10) */
    /* 0xF72 */ u8 unkF72[0xF78 - 0xF72];
    /* 0xF78 */ MeshScene *unkF78; /* what func_8006F1B8 returns, at the init */
    /* 0xF7C */ s16 unkF7C;
    /* 0xF7E */ s16 unkF7E;
    /* 0xF80 */ s16 unkF80;
    /* 0xF82 */ s16 unkF82;
    /* 0xF84 */ u16 unkF84[2]; /* by character: frames button 0x80 has been held, up to 30 (func_800D4044) */
    /* 0xF88 */ s16 unkF88[2]; /* by character: how long it was held, when let go */
} Unk800D3B14;

extern VtableEntry D_800D3854[];
extern VtableEntry D_800D393C[];

Unk800D39B4 *func_800D39B4(Unk800D39B4 *this, s32 arg);

/* Constructs the stage's object of func_800D3C08 (virtual table D_800D3854). */
Unk800D39B4 *func_800D39B4(Unk800D39B4 *this, s32 arg) {
    func_8007C084(&this->base, arg);
    this->base.vtable = D_800D3854;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    this->unk14 = 0;
    return this;
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D39F8);

/* The eight words at 0xD38 of func_800D02C8's class, which the stages' constructors fill (game.h's
 * Unk800D02C8 leaves 0xD38-0xDA4 as a gap). */
#define BASE_TABLE(stage) ((s32 *)(stage)->base.unkD38)

/* Constructs the stage's object: func_800D02C8's, with virtual table D_800D393C and the eight words at 0xD38. */
Unk800D3B14 *func_800D3B14(Unk800D3B14 *this) {
    func_800D02C8(&this->base);
    this->base.base.vtable = D_800D393C;
    BASE_TABLE(this)[0] = 9;
    BASE_TABLE(this)[1] = 10;
    BASE_TABLE(this)[2] = 11;
    BASE_TABLE(this)[3] = 13;
    BASE_TABLE(this)[4] = 14;
    BASE_TABLE(this)[5] = 15;
    BASE_TABLE(this)[6] = 16;
    BASE_TABLE(this)[7] = 12;
    return this;
}

/* Destroys the stage's object: virtual table D_800D393C, then func_800D02C8's destructor. */
void func_800D3B88(Unk800D3B14 *this, s32 flags) {
    this->base.base.vtable = D_800D393C;
    func_800D02FC(&this->base, flags);
}

/* what the stage's own init and update call */
void func_800D3C3C(Unk800D3B14 *this);
void func_800D3E94(Unk800D3B14 *this);
void func_800D3EB0(Unk8006ED20 *this);
void func_800D3F30(Unk800D3B14 *this);
void func_800D3D58(Unk800D3B14 *this);
void func_800D4044(Unk800D3B14 *this);
void func_800D4140(Unk800D3B14 *this);

/* The stage's init: func_800D02C8's, then its own state and objects; 0xF68 ends at 90. */
void func_800D3BB0(Unk800D3B14 *this) {
    func_800D0364(&this->base);
    func_800D3C3C(this);
    this->unkF78 = func_8006F1B8(&this->base.base);
    func_800D3E94(this);
    func_800D3EB0(&this->base.base);
    func_800D3F30(this);
    this->unkF68 = 90;
}

/* The stage's override of func_800702E0: creates its own 0x18-byte object for arg. */
Unk800D39B4 *func_800D3C08(void *this, s32 unk, s32 arg) {
    return func_800D39B4(operatorNew(0x18), arg);
}

/* Clears six of the stage's halfwords; its init (func_800D3BB0) calls it first. */
void func_800D3C3C(Unk800D3B14 *this) {
    this->unkF70 = 0;
    this->unkF6A = 0;
    this->unkF7E = 0;
    this->unkF7C = 0;
    this->unkF82 = 0;
    this->unkF80 = 0;
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D3C58);

/* The stage's update: func_800D02C8's, then its own three steps, and counts the update at 0xF70. */
void func_800D3D10(Unk800D3B14 *this) {
    func_800D06D0(&this->base);
    func_800D4044(this);
    func_800D4140(this);
    func_800D3D58(this);
    this->unkF70++;
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D3D58);

/* Clears the four halfwords at 0xF84 and sets 0xF34 to 30; called by the stage's init. */
void func_800D3E94(Unk800D3B14 *this) {
    this->unkF84[0] = 0;
    this->unkF84[1] = 0;
    this->unkF88[1] = 0;
    this->unkF88[0] = 0;
    this->unkF34 = 30;
}

/* g++'s `new` of func_800D5080's class (0x54 bytes): NULL when the allocation fails. The match depends
 * on this helper: written out in the loop, the second NULL check folds into the first. */
static inline Entity *newStageQuad(Unk8006ED20 *owner, s32 index) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_800D5080(entityOperatorNewAt(0x54, child), owner, index);
    }
    return child;
}

/* Creates the stage's two entities of func_800D5080's class (index 0 and 1), each put into the entity
 * group of this. */
void func_800D3EB0(Unk8006ED20 *this) {
    s32 i;

    for (i = 0; i < 2; i++) {
        Entity *child = newStageQuad(this, i);

        if (child != NULL) {
            func_8006F0B4(this, child);
        }
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D3F30);


/* Sets entry index of 0xF2C: the stage base's setter, out of line in game.bin as func_800D3284. The match
 * depends on its s16 value parameter: promoted to int, it loads the u16 count with lh before the store. */
static inline void setUnkF2C(Unk800D3B14 *this, s32 index, s16 value) {
    this->unkF2C[index] = value;
}

/* Each frame, for both characters: counts the frames their button 0x80 is held (up to 30), keeps the count
 * at 0xF88 when it is let go, and copies the count to 0xF2C. */
void func_800D4044(Unk800D3B14 *this) {
    struct Unk80075000 *pads[2];
    u32 buttons[2];
    s32 i;

    for (i = 0; i < 2; i++) {
        pads[i] = func_80073260(&this->base.base, i);
        buttons[i] = func_800750C4(pads[i]);
        if (buttons[i] & 0x80) {
            if (++this->unkF84[i] > 30) {
                this->unkF84[i] = 30;
            }
            this->unkF88[i] = 0;
        } else {
            this->unkF88[i] = this->unkF84[i];
            this->unkF84[i] = 0;
        }
        setUnkF2C(this, i, this->unkF84[i]);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4140);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D449C);

/* The destructor of the entity class of vtable D_800D3914, which func_800D449C installs. */
void func_800D458C(Entity *this, s32 flags) {
    this->vtable = &D_800D3914;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D45E0);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4718);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D49B0);

/* The destructor of the entity class of vtable D_800D38EC, which func_800D49B0 installs. */
void func_800D4AB8(Entity *this, s32 flags) {
    this->vtable = &D_800D38EC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4B0C);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4C64);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4DE4);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D4F68);

/* Constructs an UnkStageQuad (vtable D_800D38C4) of owner: the corners of a 128 by 128 square, then
 * func_800D53AC. */
UnkStageQuad *func_800D5080(UnkStageQuad *this, Unk8006ED20 *owner, s32 index) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D38C4;
    this->owner = owner;
    this->index = index;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->unk1E = 0;
    this->unk20 = 0;
    func_800D53AC(this);
    return this;
}

/* The destructor of the entity class of vtable D_800D38C4, which func_800D5080 installs. */
void func_800D5120(Entity *this, s32 flags) {
    this->vtable = &D_800D38C4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5174);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D51F0);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D53AC);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D54EC);

/* Constructs an UnkStageQuadPos (vtable D_800D389C) of owner at pos: the corners of a 512 by 512 square, index 0. */
UnkStageQuadPos *func_800D5630(UnkStageQuadPos *this, Unk8006ED20 *owner, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D389C;
    this->owner = owner;
    /* The match depends on reading pos into x, y and z here and storing them last. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x100;
    this->corners[1].vx = this->corners[3].vx = 0x100;
    this->corners[0].vy = this->corners[1].vy = -0x100;
    this->corners[2].vy = this->corners[3].vy = 0x100;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->index = 0;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    return this;
}

/* The destructor of the entity class of vtable D_800D389C, which func_800D5630 installs. */
void func_800D56D8(Entity *this, s32 flags) {
    this->vtable = &D_800D389C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather method of UnkStageQuadPos: counts it in index, and deletes the entity on the 17th. */
void func_800D572C(UnkStageQuadPos *this, s32 arg) {
    if (++this->index > 16) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5794);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5910);

/* The destructor of the entity class of vtable D_800D3874, which func_800D5910 installs. */
void func_800D59B4(Entity *this, s32 flags) {
    this->vtable = &D_800D3874;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5A08);

INCLUDE_ASM("st07/nonmatchings/st07", func_800D5AFC);

/* Unk800D39B4's destructor: Unk8007C084's. */
void func_800D5E04(Unk800D39B4 *this, s32 flags) {
    func_8007C09C(&this->base, flags);
}
