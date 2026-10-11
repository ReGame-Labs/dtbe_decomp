#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "psyq.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"

/* An entity the stage keeps by handle, with the level it last gave it (0 to 0xFF, func_800D43A4). */
typedef struct Unk800D43A4 {
    /* 0x0 */ u32 handle;
    /* 0x4 */ s32 level;
} Unk800D43A4;

/* The stage's class, derived from func_8006ED20's; func_800D3C10 constructs it. */
typedef struct Unk800D3C10 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ Unk800D43A4 unkD38[4];
    /* 0xD58 */ u8 unkD58[0xD74 - 0xD58];
    /* 0xD74 */ s32 unkD74;
    /* 0xD78 */ s32 unkD78;
    /* 0xD7C */ u32 unkD7C; /* set: copy the shown buffer once (func_800D433C) */
    /* 0xD80 */ s32 unkD80;
} Unk800D3C10;

/* game.bin's: the camera of func_8006ED20's object (at 0x1B0) */
Camera *func_8006EF5C(Unk8006ED20 *object);
void func_800D43A4(Unk800D43A4 *entry);

/* The entity class of virtual table D_800D3A4C (0x28 bytes). */
typedef struct Unk800D5164 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5164;

extern EntityVtable D_800D3A4C;

/* The entity class func_800D5040 constructs (virtual table D_800D3A74). */
typedef struct Unk800D5040 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5040;

extern EntityVtable D_800D3A74;

/* The entity class func_800D557C constructs (virtual table D_800D3A24). */
typedef struct Unk800D557C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ Vec3 unk28;
} Unk800D557C;

extern EntityVtable D_800D3A24;

/* The entity class of virtual table D_800D3B5C. */
typedef struct Unk800D58D8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
} Unk800D58D8;

extern EntityVtable D_800D3B5C;

/* The entity class of virtual table D_800D3B2C (func_800D4468 constructs it). */
typedef struct Unk800D4468 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s32 unk20; /* func_800D4468's second argument */
    /* 0x24 */ s32 unk24;
} Unk800D4468;

void func_800D45BC(Unk800D4468 *this, s32 arg);
void func_800D46C4(Unk800D4468 *this, s32 arg);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D3C10);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D3CD0);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D3D40);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D3ED8);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4008);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D41BC);

/* The stage's override of func_80072748: after it, when 0xD7C is set, clears it and queues a copy of the
 * shown buffer to x 0x200 in the camera's first-drawn slot. */
void func_800D433C(Unk800D3C10 *this) {
    u_long *ot;

    func_80072748(this);
    ot = func_8006EF5C(&this->base)->ot.firstDrawn;
    if (this->unkD7C != 0) {
        this->unkD7C = 0;
        AddPrim(ot, allocDrMove(getShownBufferRect(), 0x200, 0));
    }
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D43A4);

/* Sets the level of the stage's entity number index (a negative level keeps the last one) and passes it on. */
void func_800D4434(Unk800D3C10 *this, s32 index, s32 level) {
    Unk800D43A4 *entry = &this->unkD38[index];

    if (level >= 0) {
        entry->level = level;
    }
    func_800D43A4(entry);
}

extern EntityVtable D_800D3B2C;

/* Constructs the entity of virtual table D_800D3B2C with arg at 0x20, the rest cleared. */
Unk800D4468 *func_800D4468(Unk800D4468 *this, s32 arg) {
    entityInit(&this->entity, 0);
    /* The match depends on clearing 0x18 before installing the virtual table. */
    this->unk18 = 0;
    this->entity.vtable = &D_800D3B2C;
    this->unk1C = 0;
    this->unk1E = 0;
    this->unk1F = 0;
    this->unk20 = arg;
    this->unk24 = 0;
    return this;
}

/* engine getCos, inlined.
 * The match depends on this s16 inline: a table read in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* The gather of Unk800D4468: 0x1C climbs by 8 when 0x20 is 0, else follows a cosine of the 0x24 count. */
void func_800D44C4(Unk800D4468 *this) {
    if (this->unk20 == 0) {
        this->unk1C += 8;
    } else {
        this->unk1C = -0x1C0 - (getCosInline(this->unk24 << 3) >> 5);
    }
    this->unk24++;
}

/* Unk800D4468's own method (0x20 of its virtual table): clamps the level to 0..0xFF and keeps it at 0x18;
 * when 0x20 is 0, 0x1E gets half of it and 0x1F the complement of three quarters, else both get it. */
void func_800D4520(Unk800D4468 *this, s32 level) {
    if (level >= 0) {
        if (level >= 0x100) {
            level = 0xFF;
        }
    } else {
        level = 0;
    }
    this->unk18 = level;
    if (this->unk20 == 0) {
        this->unk1E = level >> 1;
        this->unk1F = ~((level * 3 << 6) >> 8);
    } else {
        this->unk1E = level;
        this->unk1F = level;
    }
}

/* The update of Unk800D4468: func_800D45BC when 0x20 is 0, func_800D46C4 otherwise. */
void func_800D457C(Unk800D4468 *this, s32 arg) {
    if (this->unk20 == 0) {
        func_800D45BC(this, arg);
    } else {
        func_800D46C4(this, arg);
    }
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D45BC);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D46C4);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4958);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4A00);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4B78);

/* The entity class of virtual table D_800D3AA4 (0x38 bytes): one mesh of the stage, scaled and spun about
 * a random axis. */
typedef struct Unk800D4EC0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ VECTOR axis;  /* random, normalized */
    /* 0x2C */ s32 angle;    /* about axis */
    /* 0x30 */ s32 speed;    /* the angle's step, 1 to 8 */
    /* 0x34 */ s32 scale;    /* of the three axes (func_800D4E64) */
} Unk800D4EC0;

/* The entity class of virtual table D_800D3ACC (0x2C bytes): the four Unk800D4EC0 of the meshes of
 * D_800D397C, by handle. */
typedef struct Unk800D4D8C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18; /* func_800D4E64's argument */
    /* 0x1C */ u32 children[4];
} Unk800D4D8C;

extern EntityVtable D_800D3ACC;
extern EntityVtable D_800D3AA4;

/* the nodes of the four meshes func_800D4D8C spins: 0 to 3 */
extern s32 D_800D397C[4];

Unk800D4EC0 *func_800D4EC0(Unk800D4EC0 *this, Mesh *mesh);

/* game.bin's: puts entity (when not NULL) into the entity group at 0x3C of object; returns it */
Entity *func_8006F084(Unk8006ED20 *object, Entity *entity);

/* Constructs the entity of virtual table D_800D3ACC: an Unk800D4EC0 for each of the four meshes of
 * D_800D397C in scene, each put into the stage's entity group. */
Unk800D4D8C *func_800D4D8C(Unk800D4D8C *this, Unk8006ED20 *stage, MeshScene *scene) {
    s32 i;

    entityInit(&this->entity, 0);
    this->unk18 = 0;
    this->entity.vtable = &D_800D3ACC;
    for (i = 0; i < 4; i++) {
        Mesh *mesh = meshSceneGetMesh(scene, D_800D397C[i]);
        Unk800D4EC0 *child = func_800D4EC0(entityOperatorNew(sizeof(Unk800D4EC0)), mesh);

        this->children[i] = entityGetHandle(&child->entity);
        func_8006F084(stage, &child->entity);
    }
    return this;
}

/* Unk800D4D8C's own method (0x20 of its virtual table): keeps arg at 0x18 and scales the four meshes by
 * 40 times it. */
void func_800D4E64(Unk800D4D8C *this, s32 arg) {
    s32 scale = arg * 40;
    s32 i;

    this->unk18 = arg;
    for (i = 0; i < 4; i++) {
        ((Unk800D4EC0 *)getEntityByHandle(this->children[i]))->scale = scale;
    }
}

/* Constructs the entity of virtual table D_800D3AA4 for mesh: a random axis, a random angle and a random
 * step of 1 to 8. */
Unk800D4EC0 *func_800D4EC0(Unk800D4EC0 *this, Mesh *mesh) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3AA4;
    this->mesh = mesh;
    x = mersenneTwisterGenerate(&RANDOM);
    y = mersenneTwisterGenerate(&RANDOM);
    z = mersenneTwisterGenerate(&RANDOM);
    this->axis.vx = x;
    this->axis.vy = y;
    this->axis.vz = z;
    vecNormalize(&this->axis, &this->axis);
    this->angle = mersenneTwisterGenerate(&RANDOM) & 0xFFF;
    this->speed = (s32)mersenneTwisterGenerate(&RANDOM) % 8;
    if (this->speed == 0) {
        this->speed = 8;
    }
    return this;
}

/* Sets the mesh's matrix to no rotation at (x, y, z), out of date. */
static inline void meshSetPosition(Mesh *mesh, s32 x, s32 y, s32 z) {
    mesh->object.transform.valid = 0;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x;
    mesh->object.transform.local.m.t[1] = y;
    mesh->object.transform.local.m.t[2] = z;
}

/* Sets the rotation part of the mesh's matrix to a scale of the three axes (element pairs written as words). */
static inline void meshSetScale(Mesh *mesh, u16 scale) {
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[0] = scale;
    mesh->object.transform.local.words[2] = scale;
    mesh->object.transform.local.words[4] = scale;
}

/* The gather of Unk800D4EC0: steps the angle, then sets the mesh's matrix at (0xD74 of the stage, 0, 0),
 * scaled by 0x34, turned by the angle about the axis and by twice it about y. The match depends on reading
 * the stage's x before the angle step. */
void func_800D4F9C(Unk800D4EC0 *this, Unk800D3C10 *stage) {
    s32 x = stage->unkD74;
    Mesh *mesh;

    this->angle += this->speed;
    mesh = this->mesh;
    meshSetPosition(mesh, x, 0, 0);
    meshSetScale(mesh, this->scale);
    matrixRotateAxis(&mesh->object.transform.local.m, &this->axis, this->angle);
    matrixRotateY(&mesh->object.transform.local.m, this->angle << 1);
}

/* Constructs an entity of the class of virtual table D_800D3A74, 0x24 set to 0x100. */
Unk800D5040 *func_800D5040(Unk800D5040 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A74;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20 = 0;
    this->unk24 = 0x100;
    return this;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D508C);

void func_800D515C(void) {
}

/* Constructs an entity of the class of virtual table D_800D3A4C; func_800D508C
 * makes them, passing 0 and 1 in turn as arg. */
Unk800D5164 *func_800D5164(Unk800D5164 *this, s32 arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A4C;
    this->unk18 = 0;
    this->unk1C = 0x10;
    this->unk20 = arg;
    this->unk24 = 0;
    return this;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D51BC);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D5254);

/* Constructs an entity of the class of virtual table D_800D3A24: 0x1C -0x200, 0x20 0x4000, the vector at
 * 0x28 zero. */
Unk800D557C *func_800D557C(Unk800D557C *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A24;
    this->unk18 = 0;
    this->unk1C = -0x200;
    this->unk20 = 0x4000;
    this->unk28.vx = 0;
    this->unk28.vy = 0;
    this->unk28.vz = 0;
    return this;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D55D4);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D5690);

extern EntityVtable D_800D39FC;

/* the zero vector of the main executable */
extern Vec3 D_80010814;

/* game.bin's: constructs an Unk800C3CFC at pos (0x38 from unk38, color at 0x48) */
Unk800C3CFC *func_800C3CFC(Unk800C3CFC *this, Vec3 *pos, s32 unk38, s32 color);

/* game.bin's: the gather of the class func_800C3CFC constructs */
void func_800C3E64(Unk800C3CFC *this);

/* Constructs the entity of virtual table D_800D39FC, derived from func_800C3CFC's at the zero vector, with
 * 0x50 a random 0x60 to 0x7F. */
Unk800C3CFC *func_800D57C4(Unk800C3CFC *this, s32 unk38, s32 color) {
    func_800C3CFC(this, &D_80010814, unk38, color);
    this->entity.vtable = &D_800D39FC;
    this->unk50 = (mersenneTwisterGenerate(&RANDOM) & 0x1F) + 0x60;
    return this;
}

/* The gather of func_800D57C4's entity: func_800C3E64, then puts it at (0xD74 of the stage, -0x200,
 * 0x4000). */
void func_800D5824(Unk800C3CFC *this, Unk800D3C10 *stage) {
    func_800C3E64(this);
    this->unk28.vx = stage->unkD74;
    this->unk28.vy = -0x200;
    this->unk28.vz = 0x4000;
}

/* The camera's eye of func_8006ED20's class, which func_80072088 computes (game.h leaves 0x2CC-0x360
 * as a gap); st05 has the same macro. */
#define BASE_EYE(stage) (*(Vec3 *)&(stage)->base.unk2CC[0x31C - 0x2CC])

/* Returns the stage's camera eye. */
Vec3 *func_800D586C(Unk800D3C10 *this) {
    return &BASE_EYE(this);
}

/* Returns the stage's 0xD74 (the x its meshes are put at). */
s32 func_800D5874(Unk800D3C10 *this) {
    return this->unkD74;
}

/* The destructor of the entity class of vtable D_800D3B5C, which func_800D58D8 installs. */
void func_800D5880(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D58C8(void) {
}

void func_800D58D0(void) {
}

/* Constructs an entity of the class of virtual table D_800D3B5C. */
Unk800D58D8 *func_800D58D8(Unk800D58D8 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B5C;
    this->unk18 = 0;
    return this;
}

/* The method at 0x20 of the virtual tables D_800D3A74, D_800D3AFC and D_800D3B5C: keeps arg at 0x18. */
void func_800D5914(Unk800D58D8 *this, s32 arg) {
    this->unk18 = arg;
}

/* The destructor of the entity class of vtable D_800D3B2C, which func_800D4468 installs. */
void func_800D591C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AFC, which func_800D4958 installs. */
void func_800D5964(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3ACC, which func_800D4D8C installs. */
void func_800D59AC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AA4, which func_800D4EC0 installs. */
void func_800D59F4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A74, which func_800D5040 installs. */
void func_800D5A3C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A4C, which func_800D5164 installs. */
void func_800D5A84(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A24, which func_800D557C installs. */
void func_800D5ACC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D39FC, which func_800D57C4 installs. */
void func_800D5B14(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
