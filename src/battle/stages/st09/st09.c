#include "common.h"
#include <abs.h>
#include "engine/task/entity.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"

/* The class func_800D3A64 constructs (virtual table D_800D3904), derived from Unk8007C084. */
typedef struct Unk800D3A64 {
    /* 0x00 */ Unk8007C084 base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
} Unk800D3A64;

/* the constructor func_800D46EC calls, and what that constructor calls last */
UnkStageQuad *func_800D5068(UnkStageQuad *this, Unk8006ED20 *owner, s32 index);
void func_800D5410(UnkStageQuad *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3924;
extern EntityVtable D_800D394C;
extern EntityVtable D_800D3974;
extern EntityVtable D_800D39C4;

extern VtableEntry D_800D3904[];

/* Constructs an Unk800D3A64 with unk0 (passed to Unk8007C084's constructor); its own fields 0. */
Unk800D3A64 *func_800D3A64(Unk800D3A64 *this, s32 unk0) {
    func_8007C084(&this->base, unk0);
    this->base.vtable = D_800D3904;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    this->unk14 = 0;
    this->unk18 = 0;
    this->unk1C = 0;
    return this;
}

/* The stage's class, derived from func_800D02C8's; func_800D3D58 constructs it. */
typedef struct Unk800D3D58 {
    /* 0x0000 */ Unk800D02C8 base;
    /* 0x0DA8 */ u8 unkDA8[0xF34 - 0xDA8];
    /* 0x0F34 */ s16 unkF34;
    /* 0x0F36 */ u8 unkF36[0x15A2 - 0xF36];
    /* 0x15A2 */ s16 unk15A2;
    /* 0x15A4 */ s16 unk15A4;
    /* 0x15A6 */ s16 unk15A6;
    /* 0x15A8 */ s16 unk15A8;
} Unk800D3D58;

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D3AB0);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D3D58);

/* The stage's override of func_800702E0: creates its own 0x20-byte object for arg. */
void *func_800D4070(void *this, s32 unk, s32 arg) {
    return func_800D3A64(operatorNew(0x20), arg);
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D40A4);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D417C);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D41E4);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4268);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4270);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D42EC);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D43CC);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4458);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D45F8);

/* Clears the four halfwords at 0x15A2 and sets 0xF34 to 30 (st07's func_800D3E94). */
void func_800D46D0(Unk800D3D58 *this) {
    this->unk15A2 = 0;
    this->unk15A4 = 0;
    this->unk15A8 = 0;
    this->unk15A6 = 0;
    this->unkF34 = 30;
}

/* g++'s `new` of func_800D5068's class (0x54 bytes): NULL when the allocation fails. The match depends
 * on this helper: written out in the loop, the second NULL check folds into the first. */
static inline Entity *newStageQuad(Unk8006ED20 *owner, s32 index) {
    Entity *child = allocEntityMemory(0x54);

    if (child != NULL) {
        func_800D5068(entityOperatorNewAt(0x54, child), owner, index);
    }
    return child;
}

/* Creates the stage's two entities of func_800D5068's class (index 0 and 1), each put into the entity
 * group of this. */
void func_800D46EC(Unk8006ED20 *this) {
    s32 i;

    for (i = 0; i < 2; i++) {
        Entity *child = newStageQuad(this, i);

        if (child != NULL) {
            func_8006F0B4(this, child);
        }
    }
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D476C);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D48A4);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D49A0);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4B78);

/* The destructor of the entity class of vtable D_800D3974, which func_800D4B78 installs. */
void func_800D4CDC(Entity *this, s32 flags) {
    this->vtable = &D_800D3974;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4D30);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D4EC8);

/* Constructs an UnkStageQuad (vtable D_800D394C) of owner: the corners of a 128 by 128 square, then
 * func_800D5410. */
UnkStageQuad *func_800D5068(UnkStageQuad *this, Unk8006ED20 *owner, s32 index) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D394C;
    this->owner = owner;
    this->index = index;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->unk1E = 0;
    func_800D5410(this);
    return this;
}

/* The destructor of the entity class of vtable D_800D394C, which func_800D5068 installs. */
void func_800D5104(Entity *this, s32 flags) {
    this->vtable = &D_800D394C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5158);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5190);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5410);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5550);

/* The entity func_800D572C constructs (virtual table D_800D39C4): 24 items that slide up from y -0x4C4
 * (even ones) or down from y -0x40 (odd ones), fading as they go. */
typedef struct Unk800D572C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 frame;
    /* 0x1A */ u16 count; /* items moving: 2 more every 4 frames, up to 24 */
    /* 0x1C */ u16 speed[24];
    /* 0x4C */ u8 brightness[24];
    /* 0x64 */ u16 unk64[24]; /* the item stays still when set */
    /* 0x94 */ VECTOR position[24];
} Unk800D572C;

/* PsyQ libc (stdlib.h); not in psyq.h yet. */

/* Whether item i is an even one.
 * The match depends on this inline: its store-flag gives the game's xori/andi test. */
static inline s32 isEven(s32 i) {
    return (i & 1) == 0;
}

/* Constructs the stage's sliding items: even ones at y -0x4C4, odd ones at -0x40, at full brightness, still. */
Unk800D572C *func_800D572C(Unk800D572C *this) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D39C4;
    for (i = 0; i < 24; i++) {
        /* The match depends on a whole setVector in each arm (the stores before the join). */
        if (isEven(i)) {
            setVector(&this->position[i], 0, -0x4C4, 0);
        } else {
            setVector(&this->position[i], 0, -0x40, 0);
        }
        this->brightness[i] = 0x80;
        this->speed[i] = 0;
        this->unk64[i] = 0;
    }
    this->frame = 0;
    this->count = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D39C4, which func_800D572C installs. */
void func_800D57D4(Entity *this, s32 flags) {
    this->vtable = &D_800D39C4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}


/* Unk800D572C's update: every 4 frames two more items move; each speeds up, its brightness falling
 * with the distance from its start, and starts again from there at the end of its way. */
void func_800D5828(Unk800D572C *this, s32 arg) {
    s32 i;

    if ((this->frame & 3) == 0) {
        this->count += 2;
        if (this->count > 24) {
            this->count = 24;
        }
    }
    for (i = 0; i < this->count; i++) {
        if (this->unk64[i] != 0) {
            continue;
        }
        if (isEven(i)) {
            /* The match depends on step and y being local to each arm (step before the position, abs()):
             * y then stays in one block and shares the position load's register. */
            s32 step = (s16)++this->speed[i];
            s32 y = this->position[i].vy + step;

            this->position[i].vy = y;
            this->brightness[i] = 0x80 - ((0x4C4 - abs(y)) >> 2);
            if (this->position[i].vy >= -0x2C3) {
                this->position[i].vy = -0x4C4;
                this->brightness[i] = 0x80;
                this->speed[i] = 0;
            }
        } else {
            s32 step = (s16)++this->speed[i];
            s32 y = this->position[i].vy - step;

            this->position[i].vy = y;
            this->brightness[i] = 0x80 - ((abs(y) - 0x40) >> 2);
            if (this->position[i].vy < -0x240) {
                this->position[i].vy = -0x40;
                this->brightness[i] = 0x80;
                this->speed[i] = 0;
            }
        }
    }
    this->frame++;
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5998);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5BA0);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5DDC);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D5EB8);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D6110);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D6360);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D636C);

/* Constructs an UnkStageQuadPos (vtable D_800D3924) of owner at pos: the corners of a 512 by 512 square, index 0. */
UnkStageQuadPos *func_800D63D4(UnkStageQuadPos *this, Unk8006ED20 *owner, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3924;
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

/* The destructor of the entity class of vtable D_800D3924, which func_800D63D4 installs. */
void func_800D647C(Entity *this, s32 flags) {
    this->vtable = &D_800D3924;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather method of UnkStageQuadPos: counts it in index, and deletes the entity on the 17th. */
void func_800D64D0(UnkStageQuadPos *this, s32 arg) {
    if (++this->index > 16) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D6538);

INCLUDE_ASM("asm/jp/st09/nonmatchings/st09", func_800D66C0);

/* Takes child out of the group it is in; the object is not used. Nothing calls it (nor
 * func_800D66C0, which adds a child to one of the object's groups at 0x1480). */
void func_800D66F8(void *this, SceneObject *child) {
    listRemove(&child->link);
}

/* The destructor of func_800D3A64's object (virtual table D_800D3904): Unk8007C084's. */
void func_800D6724(Unk8007C084 *this, s32 flags) {
    func_8007C09C(this, flags);
}
