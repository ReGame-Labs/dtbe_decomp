#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/sprite.h"
#include "engine/math/sin_cos.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* The stage's class, which func_800D3E18 constructs on func_8006ED20's. */
typedef struct Unk800D3E18 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ ListNode unkD38; /* a list node the destructor unlinks */
    /* 0xD40 */ u8 unkD40[0xD60 - 0xD40];
    /* 0xD60 */ Sprite unkD60;
    /* 0xD78 */ Group unkD78;
} Unk800D3E18;

/* The entity class of virtual table D_800D3D14: it draws the stage's group at
 * 0xD78 (func_800D4D98, its update). */
typedef struct Unk800D4D50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D3E18 *stage;
} Unk800D4D50;

/* The stage's entity func_800D59C8 constructs: it shows and hides one node of
 * a mesh scene (meshSceneShowNode/meshSceneHideNode). */
typedef struct Unk800D59C8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *meshScene;
    /* 0x1C */ s32 node;
} Unk800D59C8;

extern EntityVtable D_800D3D14;

void func_800D4640(void *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3C4C;

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D3E18);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D3F1C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4110);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4428);

/* The base's func_80072748, then the stage's own part (func_800D4640). */
void func_800D4614(void *this) {
    func_80072748(this);
    func_800D4640(this);
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4640);

/* Draws the group at 0xD78 as camera sees it, then marks it to be worked out again. */
void func_800D48A4(Unk800D3E18 *this, Camera *camera) {
    sceneObjectDrawInView(&this->unkD78.object, camera);
    groupInvalidate(&this->unkD78);
}

/* The entity class of virtual table D_800D3D3C (constructor func_800D48D4, made by func_800D3F1C):
 * twenty words and one more, all cleared. */
typedef struct Unk800D48D4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18[20];
    /* 0x68 */ s32 unk68;
} Unk800D48D4;

extern EntityVtable D_800D3D3C;

/* Constructs the entity of virtual table D_800D3D3C with everything cleared, the twenty words last first. */
Unk800D48D4 *func_800D48D4(Unk800D48D4 *this) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3D3C;
    for (i = 19; i >= 0; i--) {
        this->unk18[i] = 0;
    }
    this->unk68 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4928);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4CAC);

/* Constructs an entity of the class of virtual table D_800D3D14 for the stage. */
Unk800D4D50 *func_800D4D50(Unk800D4D50 *this, Unk800D3E18 *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3D14;
    this->stage = stage;
    return this;
}

/* game.bin's: bit 8 of the flags at 0x2C8 of func_8006ED20's object */
s32 func_8006F1A4(Unk8006ED20 *object);

/* Unk800D4D50's update: draws the stage's group (func_800D48A4) unless bit 8 of its flags is set. */
void func_800D4D98(Unk800D4D50 *this, Camera *camera) {
    if (!func_8006F1A4(&this->stage->base)) {
        func_800D48A4(this->stage, camera);
    }
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4DE0);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4EE8);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D4F74);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D500C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5060);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D514C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5270);

/* Constructs an entity of D_800D3C4C for a mesh of the stage and where it is (func_800D4110). */
UnkStageMeshPos *func_800D5358(UnkStageMeshPos *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3C4C;
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

/* Sets the mesh's matrix to no rotation at pos, out of date. The match depends on reading pos before writing
 * the matrix. */
static inline void meshSetPosition(Mesh *mesh, VECTOR *pos) {
    s32 x, y, z;

    mesh->object.transform.valid = 0;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x;
    mesh->object.transform.local.m.t[1] = y;
    mesh->object.transform.local.m.t[2] = z;
}

/* getSin (engine/math/sin_cos.c), inline. */
static inline s16 getSinInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].sin;
}

/* The update of func_800D5358's entity: puts the mesh at pos raised by 0x2C (no rotation), then moves 0x2C
 * along a sine (128 steps of 0x30). */
void func_800D53CC(UnkStageMeshPos *this) {
    /* stage_objects.h has pos as a Vec3 and 0x28 apart, but the game copies the four words as a VECTOR */
    VECTOR pos = *(VECTOR *)&this->pos;

    pos.vy += this->unk2C;
    meshSetPosition(this->mesh, &pos);
    /* The match depends on the sine read through getSin's inline (a sine read in the expression is narrowed to
     * an s16 shift, loaded with lhu). */
    this->unk2C = getSinInline(this->unk30 << 5) >> 7;
    this->unk30++;
}

/* The entity func_800D5464 makes for a mesh of the stage (func_800D4110), at pos: 0x2C0, 0x40 and 0x80
 * at 0x1C-0x1F. */
typedef struct Unk800D5464 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ VECTOR pos;  /* its update copies the four words, pad included */
    /* 0x30 */ s32 bob;     /* added to pos.vy: the sine of 0x34, by 64 */
    /* 0x34 */ s32 counter;
} Unk800D5464; /* size 0x38 */

extern EntityVtable D_800D3C24;

/* Constructs an entity of D_800D3C24 for a mesh of the stage and where it is (func_800D4110). */
Unk800D5464 *func_800D5464(Unk800D5464 *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3C24;
    this->mesh = mesh;
    this->unk1C = 0x2C0;
    this->unk1E = 0x40;
    this->unk1F = 0x80;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->bob = 0;
    this->counter = 0;
    return this;
}

/* The update of func_800D5464's entity: puts the mesh at pos raised by bob (no rotation), then moves bob along a
 * sine (128 steps) and 0x1C by 8. */
void func_800D54F0(Unk800D5464 *this) {
    VECTOR pos = this->pos;

    pos.vy += this->bob;
    meshSetPosition(this->mesh, &pos);
    this->unk1C += 8;
    /* The match depends on getSin's inline, as in func_800D53CC. */
    this->bob = getSinInline(this->counter << 5) >> 6;
    this->counter++;
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5594);

/* The entity func_800D5804 makes for a mesh of the stage (func_800D4110), at pos: UnkStageMeshPos's
 * fields, and 0x10 at 0x34. */
typedef struct Unk800D5804 {
    /* 0x00 */ UnkStageMeshPos base;
    /* 0x34 */ s32 unk34;
} Unk800D5804; /* size 0x38 */

extern EntityVtable D_800D3BFC;

/* Constructs an entity of D_800D3BFC for a mesh of the stage and where it is (func_800D4110). */
Unk800D5804 *func_800D5804(Unk800D5804 *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->base.entity, 0);
    this->base.entity.vtable = &D_800D3BFC;
    this->base.mesh = mesh;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->base.pos.vx = x;
    this->base.pos.vy = y;
    this->base.pos.vz = z;
    this->base.unk2C = 0;
    this->base.unk30 = 0;
    this->unk34 = 0x10;
    return this;
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5880);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D59C8);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5ADC);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5E48);

/* Hides the entity's node of its mesh scene. */
void func_800D5F2C(Unk800D59C8 *this) {
    meshSceneHideNode(this->meshScene, this->node);
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D5F58);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6080);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6150);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D62B4);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6440);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6550);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D65D4);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D66B4);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6738);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6874);

/* The entity class func_800D69C4 constructs at a position (virtual table D_800D3B84); func_800D8B4C's
 * derives from it. 0x2C is 4 plus a 256th of how far the position is in front of z 0x1000. */
typedef struct Unk800D69C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 pos;
    /* 0x24 */ u8 unk24[0x2C - 0x24];
    /* 0x2C */ s32 unk2C;
} Unk800D69C4;

extern EntityVtable D_800D3B5C;

Unk800D69C4 *func_800D69C4(Unk800D69C4 *this, Vec3 *pos);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D69C4);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6A38);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6A6C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6C94);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6D0C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6E38);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D6F30);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D701C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D70BC);

/* The object of func_800D8B8C and func_800D8BBC: points at 0x70, a color per point at 0x120 (func_800D7418
 * clears them) and the index of the current point. */
typedef struct Unk800D8B8C {
    /* 0x000 */ u8 unk0[0x70];
    /* 0x070 */ VECTOR unk70[11];
    /* 0x120 */ u32 unk120[8]; /* r, g, b bytes; word-aligned (lw), so not a CVECTOR */
    /* 0x140 */ s32 unk140; /* the current point */
} Unk800D8B8C;

void func_800D72C8(Unk800D8B8C *this, VECTOR *from, VECTOR *to);
void func_800D7458(Unk800D8B8C *this, u32 from, u32 to);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D72C8);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7418);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7458);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7534);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7764);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7AC4);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7B58);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7D88);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D7F20);

extern EntityVtable D_800D3AE4;

/* the character calls of game.c the entity below makes */
Vec3 *func_8007A650(Unk80076F34 *chara, Vec3 *out);
Unk80077698 *func_80077698(Unk80076F34 *this);

/* The stage's entity of func_800D80DC (virtual table D_800D3AE4), which keeps a character's handle. */
typedef struct Unk800D80DC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 chara;     /* the character's handle */
    /* 0x1C */ Vec3 position; /* the character's, at construction */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ s32 unk2C;     /* the high half of the word at 0 of func_80077698's record */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38; /* 0xFF at construction */
} Unk800D80DC;

/* Constructs the stage's entity for a character: its handle, its position and a value of its record. */
Unk800D80DC *func_800D80DC(Unk800D80DC *this, Unk80076F34 *chara) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3AE4;
    this->chara = entityGetHandle(&chara->entity);
    func_8007A650(chara, &this->position);
    /* game_classes.h's Unk80077698 has 0x0 as one word; the game reads its high half (lh 2). */
    this->unk2C = ((s16 *)&func_80077698(chara)->unk0)[1];
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0xFF;
    return this;
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8158);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8224);

/* The entity class func_800D8374 constructs (virtual table D_800D3ABC). */
typedef struct Unk800D8374 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x114 - 0x18];
    /* 0x114 */ s32 unk114;
    /* 0x118 */ Vec3 unk118;
} Unk800D8374;

extern EntityVtable D_800D3ABC;

/* Constructs an entity of the class of virtual table D_800D3ABC: 0x114 0x200, the vector at 0x118 zero. */
Unk800D8374 *func_800D8374(Unk800D8374 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3ABC;
    this->unk114 = 0x200;
    this->unk118.vx = 0;
    this->unk118.vy = 0;
    this->unk118.vz = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D83C0);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D849C);

/* The stage's destructor: the group at 0xD78, the sprite at 0xD60, unlinks the node at 0xD38, then
 * func_8006ED20's class's destructor. */
void func_800D8718(Unk800D3E18 *this, s32 flags) {
    ListNode *node;

    groupDestroy(&this->unkD78, DESTROY_BASES);
    spriteDestroy(&this->unkD60, DESTROY_BASES);
    /* listRemove(&this->unkD38), in the access order the game's compiler used */
    node = &this->unkD38;
    this->unkD38.next->prev = this->unkD38.prev;
    node->prev->next = this->unkD38.next;
    this->unkD38.next = node;
    this->unkD38.prev = node;
    func_8006F254(&this->base, flags);
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D878C);

/* The destructor of the entity class of vtable D_800D3D3C, which func_800D48D4 installs. */
void func_800D8798(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3D14, which func_800D4D50 installs. */
void func_800D87E0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The class of virtual table D_800D3CF4 (constructor func_800D885C), derived from
 * Unk80076308; game's Unk8007BD8C has the same fields. */
typedef struct Unk800D885C {
    /* 0x00 */ Unk80076308 base;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ Vec3 unk10;
} Unk800D885C;

extern VtableEntry D_800D3CF4[];

/* The destructor of the class of virtual table D_800D3CF4: Unk80076308's, inlined. */
void func_800D8828(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs the class of virtual table D_800D3CF4: the three words, then the vector at 0x10.
 * The match depends on reading the vector's three words before storing any, as vecCopy does. */
Unk800D885C *func_800D885C(Unk800D885C *this, s32 unk4, Vec3 *unk10, s32 unk8, s32 unkC) {
    s32 x, y, z;

    this->base.vtable = D_800D3CF4;
    this->unk4 = unk4;
    this->unk8 = unk8;
    this->unkC = unkC;
    x = unk10->vx;
    y = unk10->vy;
    z = unk10->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
    return this;
}

/* Sets the vector at 0x10, reading all three components before writing any (game.bin's
 * func_8007BDD4 is the same function). */
void func_800D8898(Unk8007BD8C *this, VECTOR *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
}

/* The destructor of the entity class of vtable D_800D3CC4, which func_800D4EE8 installs. */
void func_800D88B4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The class func_800D5060 constructs on func_800D4EE8's entity (virtual table D_800D3C94, and D_800D3C74
 * for its Unk800D885C at 0x40), with a part at 0x60. */
typedef struct Unk800D5060 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x40 - 0x18];
    /* 0x40 */ Unk800D885C unk40;
    /* 0x5C */ u8 unk5C[0x60 - 0x5C];
    /* 0x60 */ Unk80075EE4 part;
} Unk800D5060;

extern VtableEntry D_800D3C74[];
extern EntityVtable D_800D3C94;

/* The destructor of Unk800D5060 (virtual tables D_800D3C94 and D_800D3C74). */
void func_800D88FC(Unk800D5060 *this, s32 flags) {
    ListNode *node;

    this->unk40.base.vtable = D_800D3C74;
    this->entity.vtable = &D_800D3C94;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk40.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3C4C, which func_800D5358 installs. */
void func_800D899C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3C24, which func_800D5464 installs. */
void func_800D89E4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3BFC, which func_800D5804 installs. */
void func_800D8A2C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3BD4, which func_800D59C8 installs. */
void func_800D8A74(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B84, which func_800D69C4 installs. */
void func_800D8ABC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B5C, which func_800D3F1C and func_800D8B4C install. */
void func_800D8B04(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs func_800D69C4's entity at pos with virtual table D_800D3B5C, 0x2C doubled. */
Unk800D69C4 *func_800D8B4C(Unk800D69C4 *this, Vec3 *pos) {
    func_800D69C4(this, pos);
    this->entity.vtable = &D_800D3B5C;
    this->unk2C *= 2;
    return this;
}

/* func_800D72C8 from point to the current point. */
void func_800D8B8C(Unk800D8B8C *this, VECTOR *point) {
    func_800D72C8(this, point, &this->unk70[this->unk140]);
}

/* func_800D72C8 from the current point to point. */
void func_800D8BBC(Unk800D8B8C *this, VECTOR *point) {
    func_800D72C8(this, &this->unk70[this->unk140], point);
}

/* func_800D7458 from color to the previous point's color. */
void func_800D8BEC(Unk800D8B8C *this, u32 color) {
    func_800D7458(this, color, this->unk120[this->unk140 - 1]);
}

/* func_800D7458 from the previous point's color to color. */
void func_800D8C24(Unk800D8B8C *this, u32 color) {
    func_800D7458(this, this->unk120[this->unk140 - 1], color);
}

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8C5C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8C64);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8C6C);

INCLUDE_ASM("asm/jp/st03/nonmatchings/st03", func_800D8C74);

/* The entity class func_800D7B58 constructs (virtual table D_800D3B0C): an
 * Unk800D885C base at 0x18 and a member of func_80075EE4's class at 0x38. */
typedef struct Unk800D7B58 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D885C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk800761AC unk38;
} Unk800D7B58;

/* Unk800D7B58's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D8D24(Unk800D7B58 *this, s32 flags) {
    ListNode *link;

    this->unk38.vtable = D_80065184;
    /* listRemove(&this->unk38.link) written out: the match depends on this
     * access order (soundControlDestroy's), as in st04's func_800D8618 */
    link = &this->unk38.link;
    this->unk38.link.next->prev = this->unk38.link.prev;
    link->prev->next = this->unk38.link.next;
    this->unk38.link.next = link;
    this->unk38.link.prev = link;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AE4, which func_800D80DC installs. */
void func_800D8DAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3ABC, which func_800D8374 installs. */
void func_800D8DF4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
