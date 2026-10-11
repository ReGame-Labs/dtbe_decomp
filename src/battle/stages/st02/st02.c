#include "common.h"
#include "battle/game_classes.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* The entity class of virtual table D_800D4004. */
typedef struct Unk800D4FC8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
} Unk800D4FC8;

extern EntityVtable D_800D4004;

/* The stage's class, derived from func_8006ED20's (virtual table D_800D402C). */
typedef struct Unk800D41E0 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ u8 unkD38[0xD44 - 0xD38];
    /* 0xD44 */ s32 unkD44;
} Unk800D41E0;

extern VtableEntry D_800D3FE4[];
extern VtableEntry D_800D402C[];

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3E84;
extern EntityVtable D_800D3F4C;

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4164);

/* Constructs the stage's object: func_8006ED20's, with virtual table D_800D402C. */
Unk800D41E0 *func_800D41E0(Unk800D41E0 *this) {
    func_8006ED20(&this->base);
    this->base.vtable = D_800D402C;
    this->unkD44 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4218);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D43DC);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D45C0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D49E8);

/* The stage's override of func_80070824 (at 0x58 of D_800D402C): whether a
 * character's position (func_800706A4 passes it) has a y greater than 0x1200. */
s32 func_800D4AB4(Unk800D41E0 *this, Vec3 *position) {
    return position->vy > 0x1200;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4AC8);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4CC0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4CE0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4D30);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4DA0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4DE4);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D4F0C);

/* Constructs an entity of the class of virtual table D_800D4004. */
Unk800D4FC8 *func_800D4FC8(Unk800D4FC8 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D4004;
    this->unk18 = 0;
    return this;
}

/* The gather method of Unk800D4FC8: adds 0x40 to 0x18. */
void func_800D5004(Unk800D4FC8 *this) {
    this->unk18 += 0x40;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5018);

/* The class of virtual table D_800D3FE4 (constructor func_800D82AC), derived from Unk80076308. */
typedef struct Unk800D82AC {
    /* 0x00 */ Unk80076308 base;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ Vec3 unkC;
} Unk800D82AC;

/* Unk800D82AC's first method (game's func_8007BDF0 one field earlier): adds its count (saturating at
 * 0x3FFFF), its 0x8 and its vector into sum, whose flags it merges with its own. */
void func_800D54C0(Unk800D82AC *this, Unk80075EE4 *sum) {
    u32 flags = (this->unk4 | sum->unk18) & ~0x3FFFF;
    s32 count = (this->unk4 & 0x3FFFF) + (sum->unk18 & 0x3FFFF);
    s32 x, y, z;

    if (count > 0x3FFFF) {
        count = 0x3FFFF;
    }
    sum->unk18 = flags | count;
    sum->unk1C += this->unk8;
    /* The match depends on vecAddInPlace's shape: the three words read before any is written */
    x = sum->unk20.vx;
    y = sum->unk20.vy;
    z = sum->unk20.vz;
    x += this->unkC.vx;
    y += this->unkC.vy;
    z += this->unkC.vz;
    sum->unk20.vx = x;
    sum->unk20.vy = y;
    sum->unk20.vz = z;
}

/* The entity class func_800D554C constructs for a stage object (owner, position): an Unk800D82AC part at
 * 0x18, a mesh of the owner's at 0x80 and the mesh's world position at 0x84 (func_800D5C24). */
typedef struct Unk800D554C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D82AC unk18;
    /* 0x30 */ u8 unk30[0x34 - 0x30];
    /* 0x34 */ Unk800761AC unk34;
    /* 0x68 */ u8 unk68[0x7C - 0x68];
    /* 0x7C */ Unk8006ED20 *owner;
    /* 0x80 */ Mesh *mesh;
    /* 0x84 */ Vec3 unk84;
    /* 0x90 */ u8 unk90[0x94 - 0x90];
    /* 0x94 */ Vec3 pos;
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
} Unk800D554C;

/* game.bin's constructor of a burst of count particles around pos */
Entity *func_800C62EC(Entity *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 color);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D554C);

extern VtableEntry D_800D3F9C[];
extern EntityVtable D_800D3FBC;

/* Deletes a scene object through its virtual destructor (engine scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Unk800D554C's destructor: takes its mesh out of the scene and deletes it, then its member's and its base's
 * destructors, inlined, and entityDestroy. */
void func_800D56AC(Unk800D554C *this, s32 flags) {
    ListNode *link;

    this->unk18.base.vtable = D_800D3F9C;
    this->entity.vtable = &D_800D3FBC;
    listRemove(&this->mesh->object.link);
    sceneObjectDelete(&this->mesh->object);
    this->unk34.vtable = D_80065184;
    /* listRemove(&this->unk34.link) written out: the match depends on this access order, as in st04's
     * func_800D8618 */
    link = &this->unk34.link;
    this->unk34.link.next->prev = this->unk34.link.prev;
    link->prev->next = this->unk34.link.next;
    this->unk34.link.next = link;
    this->unk34.link.prev = link;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D57A0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5964);

void func_800D5A64(Unk800D554C *this);

/* The override of its Unk800D82AC part's first method: adds that part into sum, and the first time
 * also spawns func_800D5A64's particles. */
void func_800D5A1C(Unk800D554C *this, Unk80075EE4 *sum) {
    func_800D54C0(&this->unk18, sum);
    if (this->unkAC == 0) {
        func_800D5A64(this);
    }
    this->unkAC = 1;
}

/* Spawns the entity of func_800C62EC at 0x84 (radius 0x30, colour 0x808080), as a sibling, and plays
 * sound 0x4D. */
void func_800D5A64(Unk800D554C *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unk84, 0x30, 0x80, -0x18, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x4D, 0);
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5AF8);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5B30);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5BC4);

/* Detaches its mesh's transform (transformDetach). */
void func_800D5C00(Unk800D554C *this) {
    transformDetach(&this->mesh->object.transform);
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5C24);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5D04);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5DF0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D5E58);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6044);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6100);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6298);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D63A4);

/* Constructs an entity of D_800D3F4C for a mesh of the stage and where it is (func_800D4110). */
UnkStageMeshPos *func_800D680C(UnkStageMeshPos *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3F4C;
    this->mesh = mesh;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk2C = 0;
    this->unk30 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3F4C, which func_800D680C installs. */
void func_800D6880(Entity *this, s32 flags) {
    this->vtable = &D_800D3F4C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D68D4);

/* The entity class func_800D6924 constructs (virtual table D_800D3F24). */
typedef struct Unk800D6924 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* bit 0 of an argument */
} Unk800D6924;

extern EntityVtable D_800D3F24;

/* Constructs an Unk800D6924 at position, with unk2C and bit 0 of unk30.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D6924 *func_800D6924(Unk800D6924 *this, Vec3 *position, s32 unk2C, s32 unk30) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3F24;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = unk2C;
    this->unk30 = unk30 & 1;
    return this;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D69A8);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D69C8);

extern EntityVtable D_800D3EFC;

/* The stage's entity of func_800D6B7C (vtable D_800D3EFC, 0x1F8 bytes), made for the stage mesh 15: two tables
 * of bytes that func_800D6DEC (0x12B down to 0x2C) and func_800D6E10 (12 rows of 16 at 0x138) clear. */
typedef struct Unk800D6B7C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} Unk800D6B7C;

void func_800D6DEC(Unk800D6B7C *this);
void func_800D6E10(Unk800D6B7C *this);

/* Constructs the stage's entity for mesh, then clears its two tables. */
Unk800D6B7C *func_800D6B7C(Unk800D6B7C *this, Mesh *mesh) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3EFC;
    this->mesh = mesh;
    this->unk1C = 0;
    this->unk20 = 0;
    func_800D6DEC(this);
    func_800D6E10(this);
    return this;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6BDC);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6C8C);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6DEC);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6E10);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6E5C);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6E84);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6F34);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D6F88);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7024);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D73E8);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D746C);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7574);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7800);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7888);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D78B0);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7918);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7BAC);

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D7F38);

/* Constructs an entity of D_800D3E84 for a mesh and the Unk8006ED20 whose entity group will hold it
 * (func_800D43DC). */
UnkSt02MeshEntity *func_800D7FC0(UnkSt02MeshEntity *this, Mesh *mesh, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3E84;
    this->mesh = mesh;
    this->owner = owner;
    return this;
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D8018);

/* The stage's destructor: func_8006ED20's class's. */
void func_800D8210(Unk800D41E0 *this, s32 flags) {
    func_8006F254(&this->base, flags);
}

/* The destructor of the entity class of vtable D_800D4004, which func_800D4FC8 installs. */
void func_800D8230(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the class of virtual table D_800D3FE4: Unk80076308's, inlined. */
void func_800D8278(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an Unk800D82AC with its two words and a copy of position (game's func_8007BD8C). */
Unk800D82AC *func_800D82AC(Unk800D82AC *this, s32 unk4, Vec3 *position, s32 unk8) {
    s32 x, y, z;

    this->base.vtable = D_800D3FE4;
    this->unk4 = unk4;
    this->unk8 = unk8;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unkC.vx = x;
    this->unkC.vy = y;
    this->unkC.vz = z;
    return this;
}

/* Sets the object's vector at 0xC from in (game's func_80076414 for 0x34).
 * The match depends on reading the three words before storing any, as vecCopy does. */
void func_800D82E0(Unk800D82AC *this, Vec3 *in) {
    s32 x = in->vx;
    s32 y = in->vy;
    s32 z = in->vz;

    this->unkC.vx = x;
    this->unkC.vy = y;
    this->unkC.vz = z;
}

/* The destructor of the entity class of vtable D_800D3F74, which func_800D6044 installs. */
void func_800D82FC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3F24, which func_800D6924 installs. */
void func_800D8344(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3EFC, which func_800D6B7C installs. */
void func_800D838C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st02/nonmatchings/st02", func_800D83D4);

/* The destructor of the entity class of vtable D_800D3ED4, which func_800D73E8 installs. */
void func_800D83E0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3EAC, which func_800D7800 installs. */
void func_800D8428(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3E84, which func_800D7FC0 installs. */
void func_800D8470(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
