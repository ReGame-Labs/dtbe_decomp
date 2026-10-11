#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/prim/alloc_dr_load.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/sprite.h"
#include "engine/math/matrix.h"
#include "engine/math/vector.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* The stage's class, which func_800D3E18 constructs on func_8006ED20's. */
typedef struct Unk800D3E18 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ ListNode unkD38; /* a list node the destructor unlinks */
    /* 0xD40 */ u8 unkD40[0xD50 - 0xD40];
    /* 0xD50 */ s32 unkD50; /* func_800D878C reads it; Unk800D4EE8's method moves the offset by it */
    /* 0xD54 */ u8 unkD54[0xD58 - 0xD54];
    /* 0xD58 */ u8 unkD58; /* func_800D6A38 copies it */
    /* 0xD59 */ u8 unkD59[0xD60 - 0xD59];
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
    /* 0x20 */ s32 unk20;
    /* 0x24 */ VECTOR pos;    /* only x, y and z set */
    /* 0x34 */ VECTOR target; /* x, y and z of D_800D39CC[point] */
    /* 0x44 */ s32 point;     /* an index into D_800D39CC */
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
} Unk800D59C8;

extern EntityVtable D_800D3D14;

void func_800D4640(void *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3C4C;

INCLUDE_ASM("st03/nonmatchings/st03", func_800D3E18);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D3F1C);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4110);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4428);

/* The base's func_80072748, then the stage's own part (func_800D4640). */
void func_800D4614(void *this) {
    func_80072748(this);
    func_800D4640(this);
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4640);

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

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4928);

/* One VRAM upload of func_800D4CAC: the rectangle, the first halfword of the entity's colours (0x18) and
 * how many words to copy from there. */
typedef struct Unk800D3964 {
    /* 0x0 */ RECT *rect;
    /* 0x4 */ s32 start;
    /* 0x8 */ s32 words;
} Unk800D3964;

extern Unk800D3964 D_800D3964[6];

/* Unk800D48D4's update: one DR_LOAD per rectangle of D_800D3964, filled with its run of the colours at 0x18
 * and added to the camera's first-drawn slot. */
void func_800D4CAC(Unk800D48D4 *this, Camera *camera) {
    u_long *ot = camera->ot.firstDrawn;
    u32 colors = (u32)this->unk18;
    Unk800D3964 *load = D_800D3964;

    do {
        DR_LOAD *prim = allocDrLoad(load->rect);
        u_long *dst = prim->p;
        /* The match depends on adding the halfword offset to the colours' address as integers, in this order
         * (pointer arithmetic adds them the other way round). */
        u_long *src = (u_long *)(load->start * 2 + colors);
        u_long *end = src + load->words;

        do {
            *dst++ = *src++;
        } while (src != end);
        AddPrim(ot, prim);
        load++;
    } while (load != D_800D3964 + 6);
}

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

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4DE0);

/* The virtual table of Unk800D4EE8 (D_800D3CC4): Entity's four entries, then the class's own method, which
 * moves the offset (func_800D500C; Unk800D5060's func_800D514C). */
typedef struct Unk800D4EE8Vtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry unk20; /* (this, stage) */
} Unk800D4EE8Vtable;

/* The entity func_800D4EE8 constructs (made by func_800D4110 and derived by Unk800D5060): one node of a mesh
 * scene, put at pos moved by offset every time its group gathers it. */
typedef struct Unk800D4EE8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *meshScene;
    /* 0x1C */ VECTOR pos;    /* only x, y and z set */
    /* 0x2C */ VECTOR offset; /* the translation added to pos */
    /* 0x3C */ s32 node;
} Unk800D4EE8; /* size 0x40 */

extern Unk800D4EE8Vtable D_800D3CC4;

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

/* The class func_800D5060 constructs on func_800D4EE8's entity (virtual table D_800D3C94, and D_800D3C74
 * for its Unk800D885C at 0x40), with a part at 0x60. */
typedef struct Unk800D5060 {
    /* 0x00 */ Unk800D4EE8 base;
    /* 0x40 */ Unk800D885C unk40;
    /* 0x5C */ u8 unk5C[0x60 - 0x5C];
    /* 0x60 */ Unk80075EE4 part;
} Unk800D5060;

extern VtableEntry D_800D3C74[];
extern Unk800D4EE8Vtable D_800D3C94;

/* Constructs an entity of D_800D3CC4 for a node of a mesh scene and where it is, with no offset. */
Unk800D4EE8 *func_800D4EE8(Unk800D4EE8 *this, MeshScene *meshScene, s32 node, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3CC4.entity;
    this->meshScene = meshScene;
    this->node = node;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->offset.vx = 0;
    this->offset.vy = 0;
    this->offset.vz = 0;
    this->offset.pad = 0;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
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

/* Unk800D4EE8's gather: runs the class's own method (0x20) with the stage, then puts the node's mesh at pos
 * (no rotation) and translates it by offset. */
void func_800D4F74(Unk800D4EE8 *this, Unk800D3E18 *stage) {
    Unk800D4EE8Vtable *vtable = (Unk800D4EE8Vtable *)this->entity.vtable;
    Mesh *mesh;

    vtable->unk20.func((u8 *)this + vtable->unk20.delta, stage);
    mesh = meshSceneGetMesh(this->meshScene, this->node);
    meshSetPosition(mesh, &this->pos);
    matrixTranslate(&mesh->object.transform.local.m, this->offset.vx, this->offset.vy, this->offset.vz);
}

/* Unk800D4EE8's own method: while the stage's 0xD50 is clear, moves offset y back up to 0 by 8 a call;
 * while it is set, down by 8 to -0xF0. */
void func_800D500C(Unk800D4EE8 *this, Unk800D3E18 *stage) {
    if (stage->unkD50 == 0) {
        this->offset.vy += 8;
        if (this->offset.vy >= 0) {
            this->offset.vy = 0;
        }
    } else {
        this->offset.vy -= 8;
        if (this->offset.vy <= -0xF0) {
            this->offset.vy = -0xF0;
        }
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5060);

void func_800D5270(Unk800D5060 *this, Unk800D3E18 *stage);

/* listIsAlone(&this->part.node), reading the fields through this (the match depends on it) */
static inline s32 partIsAlone(Unk800D5060 *this) {
    ListNode *node = &this->part.node;

    return this->part.node.next == node && node == this->part.node.prev;
}

/* listRemove(&this->part.node), in the access order the game's compiler used (the match depends on it) */
static inline void partRemove(Unk800D5060 *this) {
    ListNode *node = &this->part.node;

    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
}

/* Unk800D5060's own method (0x20): while the stage's 0xD50 is clear, moves offset y down to 0 by 8 a call;
 * while it is set, up by 8 to 0xF0. Between 9 and 0x7F the part follows it (and is put back in the stage's
 * lists by func_800D5270 while 0xD50 is clear); outside, the part leaves its list with sound 0x62. The node
 * shows unless the offset is 0. */
void func_800D514C(Unk800D5060 *this, Unk800D3E18 *stage) {
    s32 flag = stage->unkD50;
    s32 y;

    if (flag == 0) {
        this->base.offset.vy -= 8;
        if (this->base.offset.vy <= 0) {
            this->base.offset.vy = 0;
        }
    } else {
        this->base.offset.vy += 8;
        if (this->base.offset.vy >= 0xF0) {
            this->base.offset.vy = 0xF0;
        }
    }
    y = this->base.offset.vy;
    if (y >= 9 && y < 0x80) {
        this->part.unk44 = y;
        if (flag == 0) {
            func_800D5270(this, stage);
        }
    } else if (!partIsAlone(this)) {
        partRemove(this);
        soundControlPlaySound(&SOUND_CONTROL, 0x62, 0);
    }
    if (this->base.offset.vy == 0) {
        meshSceneHideNode(this->base.meshScene, this->base.node);
    } else {
        meshSceneShowNode(this->base.meshScene, this->base.node);
    }
}

/* game.bin's: whether the character stands on the given matrix */
s32 func_8007814C(Unk80076F34 *chara, MATRIX *matrix);
/* game.bin's: the character of a side (0 or 1) */
Unk80076F34 *func_800700C8(Unk8006ED20 *object, s32 side);
/* game.bin's: add a node at the end of the list at 0x228, 0x230 (func_8006F064, in game.h: 0x238) */
void func_8006F024(Unk8006ED20 *object, ListNode *node);
void func_8006F044(Unk8006ED20 *object, ListNode *node);

/* Puts the part back in the stage's lists by which characters stand on the node's matrix: the list at 0x228
 * when only the first does, 0x230 when only the second does, 0x238 when both do; none when neither does. */
void func_800D5270(Unk800D5060 *this, Unk800D3E18 *stage) {
    MATRIX *matrix = meshSceneGetWorldMatrix(this->base.meshScene, this->base.node);
    s32 first = func_8007814C(func_800700C8(&stage->base, 0), matrix);
    s32 second = func_8007814C(func_800700C8(&stage->base, 1), matrix);

    partRemove(this);
    if (first != 0 && second == 0) {
        func_8006F024(&stage->base, &this->part.node);
    } else if (first == 0 && second != 0) {
        func_8006F044(&stage->base, &this->part.node);
    } else if (first != 0 && second != 0) {
        func_8006F064(&stage->base, &this->part.node);
    }
}

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

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5594);

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

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5880);

extern EntityVtable D_800D3BD4;

/* the eight points of func_800D59C8's entities */
extern VECTOR D_800D39CC[8];

/* Constructs an entity of D_800D3BD4 for a node of a mesh scene (hidden) and where it is: nodes 5 and 6 start
 * at points 2 and 4 of D_800D39CC (0x58 1 and 2), the others at point 0. */
Unk800D59C8 *func_800D59C8(Unk800D59C8 *this, MeshScene *meshScene, s32 node, s32 unk20, Vec3 *pos) {
    s32 x, y, z;
    s32 tx, ty, tz;
    VECTOR *point;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BD4;
    this->meshScene = meshScene;
    this->node = node;
    this->unk20 = unk20;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    switch (node) {
    case 0:
    default:
        this->point = 0;
        this->unk58 = 0;
        break;
    case 5:
        this->point = 2;
        this->unk58 = 1;
        break;
    case 6:
        this->point = 4;
        this->unk58 = 2;
        break;
    }
    point = &D_800D39CC[this->point];
    tx = point->vx;
    ty = point->vy;
    tz = point->vz;
    this->unk48 = 0;
    this->unk4C = 0;
    this->unk50 = 0;
    this->unk54 = 0;
    this->target.vx = tx;
    this->target.vy = ty;
    this->target.vz = tz;
    meshSceneHideNode(meshScene, this->node);
    return this;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5ADC);

/* game.bin's character calls: its position, and its record */
Vec3 *func_8007A650(Unk80076F34 *chara, Vec3 *out);
Unk80077698 *func_80077698(Unk80076F34 *this);

/* vecGetLength through an inline. The match depends on it: the inline's parameter is the one pseudo for &offset
 * that the loop keeps in a saved register (hypothesis: an inline length method in the original). */
static inline s32 vecLengthInline(VECTOR *v) {
    return vecGetLength(v);
}

/* Shows the node once no character is within reach of pos (0xC0 plus the high half of the first word of the
 * character's record); does nothing while one is. */
void func_800D5E48(Unk800D59C8 *this) {
    Unk8006ED20 *stage = D_800D377C;
    VECTOR offset;
    s32 x, y, z;
    s32 side;

    for (side = 0; side < 2; side++) {
        Unk80076F34 *chara = func_800700C8(stage, side);
        /* game_classes.h's Unk80077698 has 0x0 as one word; the game reads its high half (lh 2). */
        s32 reach = ((s16 *)&func_80077698(chara)->unk0)[1] + 0xC0;

        func_8007A650(chara, (Vec3 *)&offset);
        /* The match depends on reading the whole offset before subtracting (as vecSubInPlace). */
        x = offset.vx;
        y = offset.vy;
        z = offset.vz;
        x -= this->pos.vx;
        y -= this->pos.vy;
        z -= this->pos.vz;
        offset.vx = x;
        offset.vy = y;
        offset.vz = z;
        if (vecLengthInline(&offset) < reach) {
            return;
        }
    }
    meshSceneShowNode(this->meshScene, this->node);
}

/* Hides the entity's node of its mesh scene. */
void func_800D5F2C(Unk800D59C8 *this) {
    meshSceneHideNode(this->meshScene, this->node);
}

/* The entity func_800D5F58 constructs for a node of a mesh scene (made by func_800D4110): an Unk800D885C at
 * 0x18 and an Unk80075EE4 part at 0x38 for it, positions in 256ths, and a state (0xCC) its gather steps. */
typedef struct Unk800D5F58 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D885C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 part;
    /* 0x80 */ Mesh *mesh;
    /* 0x84 */ MeshScene *meshScene;
    /* 0x88 */ s32 node;
    /* 0x8C */ VECTOR pos;  /* in 256ths */
    /* 0x9C */ VECTOR home; /* the position at construction, in 256ths */
    /* 0xAC */ Unk80076F34 *riders[2]; /* their weights add to the entity's */
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;  /* the speed func_800D65D4 keeps */
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;   /* a countdown of state 2 */
    /* 0xC8 */ s32 unkC8;   /* counts the gathers from a random start */
    /* 0xCC */ s32 state;
} Unk800D5F58; /* size 0xD0 */

extern EntityVtable D_800D3BAC;

/* main's zero Vec3 */
extern Vec3 D_80010814;

/* Constructs an entity of D_800D3BAC for a node of a mesh scene, its mesh and where it is (the node's world
 * translation), in state 0. */
Unk800D5F58 *func_800D5F58(Unk800D5F58 *this, MeshScene *meshScene, s32 node, Mesh *mesh, Vec3 *pos) {
    s32 vx, vy, vz;
    s32 x, y;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3BAC;
    /* func_800D885C(&this->unk18, 0x80001000, &D_80010814, 6, 0), inline (the vector read before any store) */
    this->unk18.base.vtable = D_800D3CF4;
    this->unk18.unk4 = 0x80001000;
    this->unk18.unk8 = 6;
    this->unk18.unkC = 0;
    vx = D_80010814.vx;
    vy = D_80010814.vy;
    vz = D_80010814.vz;
    this->unk18.unk10.vx = vx;
    this->unk18.unk10.vy = vy;
    this->unk18.unk10.vz = vz;
    func_80075EE4(&this->part, pos, 0x70, &this->unk18);
    this->part.unk14 = &this->entity;
    this->mesh = mesh;
    this->meshScene = meshScene;
    this->node = node;
    x = pos->vx;
    y = pos->vy;
    this->pos.vz = 0;
    this->home.vz = 0;
    this->riders[0] = NULL;
    this->riders[1] = NULL;
    this->unkB4 = 0;
    this->unkB8 = 0;
    this->unkBC = 0;
    this->unkC0 = 0;
    this->unkC4 = 0;
    this->pos.vx = x << 8;
    this->pos.vy = y << 8;
    this->home.vx = x << 8;
    this->home.vy = y << 8;
    this->unkC8 = mersenneTwisterGenerate(&RANDOM);
    this->state = 0;
    return this;
}

/* listRemove(&this->part.node), in the access order the game's compiler used (as partRemove) */
static inline void unk800D5F58PartRemove(Unk800D5F58 *this) {
    ListNode *node = &this->part.node;

    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
}

/* Sets the mesh's matrix to no rotation at x and y of pos, given in 256ths (z 0), out of date: meshSetPosition's
 * order (pos read after the flag store). */
static inline void meshSetPosition256(Mesh *mesh, VECTOR *pos) {
    s32 x, y;

    mesh->object.transform.valid = 0;
    x = pos->vx;
    y = pos->vy;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x >> 8;
    mesh->object.transform.local.m.t[1] = y >> 8;
    mesh->object.transform.local.m.t[2] = 0;
}

/* The destructor of Unk800D5F58: puts the mesh back at home (no rotation), then destroys the part, the
 * Unk800D885C and the entity. */
void func_800D6080(Unk800D5F58 *this, s32 flags) {
    this->entity.vtable = &D_800D3BAC;
    meshSetPosition256(this->mesh, &this->home);
    this->part.vtable = D_80065184;
    unk800D5F58PartRemove(this);
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

s32 func_800D62B4(Unk800D5F58 *this, Unk800D3E18 *stage);
s32 func_800D6440(Unk800D5F58 *this, s32 speed);
void func_800D6550(Unk800D5F58 *this, Unk800D3E18 *stage);
void func_800D66B4(Unk800D5F58 *this);
void func_800D6738(Unk800D5F58 *this, Unk800D3E18 *stage);

/* Unk800D5F58's gather, by state: 0 moves by an eighth of the way from 0x100 above pos to home until 0xC0
 * passes 8; 1 moves (func_800D6738) and at 0x71C (func_800D6440) leaves the part's list, waiting 0x1E; 2
 * counts that down and shows the node again (state 0) once func_800D62B4 said 0. */
void func_800D6150(Unk800D5F58 *this, Unk800D3E18 *stage) {
    s32 busy = func_800D62B4(this, stage);
    s32 top;

    switch (this->state) {
    case 0:
        this->unkBC = 0;
        top = this->pos.vy + 0x100;
        func_800D6440(this, (this->home.vy - top) >> 3);
        if (this->unkC0 >= 9) {
            this->unkBC = 0;
            this->state++;
        }
        break;
    case 1:
        func_800D6738(this, stage);
        if (func_800D6440(this, 0x71C) != 0) {
            unk800D5F58PartRemove(this);
            this->unkB8 = 0;
            this->unkB4 = 0;
            this->unkC4 = 0x1E;
            this->state++;
        }
        break;
    case 2:
        func_800D6550(this, stage);
        this->unkC8 = 0;
        if (--this->unkC4 <= 0 && busy == 0) {
            meshSceneShowNode(this->meshScene, this->node);
            this->state = 0;
        }
        break;
    }
    func_800D66B4(this);
    this->unkC8++;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D62B4);

/* game.bin's: whether the segment from from to to hits something, moving to to the hit */
s32 func_80072F80(VECTOR *to, VECTOR *from);

void func_800D65D4(Unk800D5F58 *this, s32 speed);
void func_800D6874(Unk800D5F58 *this);

/* Moves the entity by speed (func_800D65D4) and stops it: at home (returns whether it was going down), at
 * 0x40000 (hides the node; returns whether it was going up), or where its point (0x40 right, 0x10 down)
 * hits something on the way (func_800D6874, sound 0x62; returns whether it was going up). */
s32 func_800D6440(Unk800D5F58 *this, s32 speed) {
    VECTOR from;
    VECTOR to;

    from.vz = 0;
    from.vx = (this->pos.vx >> 8) + 0x40;
    from.vy = (this->pos.vy >> 8) - 0x10;
    func_800D65D4(this, speed);
    to.vz = 0;
    to.vx = (this->pos.vx >> 8) + 0x40;
    to.vy = (this->pos.vy >> 8) - 0x10;
    if (this->pos.vy < this->home.vy) {
        this->pos.vy = this->home.vy;
        this->unkBC = 0;
        return speed < 0;
    }
    if (this->pos.vy > 0x40000) {
        this->pos.vy = 0x40000;
        this->unkBC = 0;
        if (speed < 0) {
            return 0;
        }
        meshSceneHideNode(this->meshScene, this->node);
        return 1;
    }
    if (func_80072F80(&to, &from) != 0) {
        this->unkBC = 0;
        this->pos.vy = to.vy << 8;
        func_800D6874(this);
        soundControlPlaySound(&SOUND_CONTROL, 0x62, 0);
        return speed >= 0;
    }
    return 0;
}

/* State 2's move: drops the entity by 0xF0 (in whole units) when nothing is in the way of its point, else
 * pushes it with 0x71C (func_800D65D4). */
void func_800D6550(Unk800D5F58 *this, Unk800D3E18 *stage) {
    VECTOR from;
    VECTOR to;
    s32 x = (this->pos.vx >> 8) + 0x40;
    s32 y = this->pos.vy >> 8;

    from.vx = x;
    from.vy = y - 0x10;
    from.vz = 0;
    to.vx = x;
    to.vy = y + 0xF0;
    to.vz = 0;
    if (func_80072F80(&to, &from) != 0) {
        this->unkBC = 0;
        this->pos.vy = to.vy << 8;
    } else {
        func_800D65D4(this, 0x71C);
    }
}

/* game.bin's: integrates velocity by force over mass, setting move to the step (the mean of the speeds) */
void func_80079C1C(VECTOR *move, VECTOR *velocity, VECTOR *force, s32 mass);

/* Pushes the entity up or down by force: its mass is 0x20000 plus the riders' weights (0x4 of their record),
 * its speed kept at 0xBC. */
void func_800D65D4(Unk800D5F58 *this, s32 force) {
    VECTOR move;
    VECTOR velocity;
    VECTOR push;
    s32 mass = 0x20000;
    s32 i;

    for (i = 0; i < 2; i++) {
        if (this->riders[i] != NULL) {
            mass += func_80077698(this->riders[i])->unk4;
        }
    }
    velocity.vx = 0;
    velocity.vy = this->unkBC;
    velocity.vz = 0;
    push.vx = 0;
    push.vy = force;
    push.vz = 0;
    func_80079C1C(&move, &velocity, &push, mass);
    this->unkBC = velocity.vy;
    this->pos.vy += move.vy;
}

/* Puts the mesh at pos (no rotation), then tilts it about x along a sine of the gather count (64 steps of
 * 0x40, a 32nd of the sine). */
void func_800D66B4(Unk800D5F58 *this) {
    Mesh *mesh = this->mesh;

    meshSetPosition256(mesh, &this->pos);
    matrixRotateX(&mesh->object.transform.local.m, COS_SIN_TABLE[(this->unkC8 << 6) & 0xFFF].sin >> 5);
}

/* State 1's move: sets the part's vector at 0x34 (0x70 above the entity) and the Unk800D885C's (a random x
 * below 0x1000, y -0x38E0), counts down 0xB4 and 0xB8 and puts the part back in the stage's lists by which of
 * them were running: 0x238 when neither was, 0x230 when only 0xB4 was, 0x228 when only 0xB8 was; none when
 * both were. */
void func_800D6738(Unk800D5F58 *this, Unk800D3E18 *stage) {
    ListNode *node = &this->part.node;
    VECTOR v;
    s32 wait0, wait1;

    v.vz = 0;
    v.vx = this->pos.vx >> 8;
    v.vy = (this->pos.vy >> 8) + 0x70;
    this->part.unk34.vx = v.vx;
    this->part.unk34.vy = v.vy;
    this->part.unk34.vz = v.vz;
    v.vx = (s32)mersenneTwisterGenerate(&RANDOM) % 0x1000;
    v.vy = -0x38E0;
    v.vz = 0;
    this->unk18.unk10.vx = v.vx;
    this->unk18.unk10.vy = v.vy;
    this->unk18.unk10.vz = v.vz;
    wait0 = this->unkB4 > 0;
    wait1 = this->unkB8 > 0;
    if (this->unkB4 > 0) {
        this->unkB4--;
    }
    if (this->unkB8 > 0) {
        this->unkB8--;
    }
    unk800D5F58PartRemove(this);
    if (!wait0 && !wait1) {
        func_8006F064(&stage->base, node);
    } else if (wait0 && !wait1) {
        func_8006F044(&stage->base, node);
    } else if (!wait0 && wait1) {
        func_8006F024(&stage->base, node);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6874);

/* The entity class func_800D69C4 constructs at a position (virtual table D_800D3B84); func_800D8B4C's
 * derives from it. 0x2C is 4 plus a 256th of how far the position is in front of z 0x1000. */
typedef struct Unk800D69C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 pos;
    /* 0x24 */ u8 unk24[0x2C - 0x24];
    /* 0x2C */ s32 unk2C;     /* how far it moves left each gather */
    /* 0x30 */ u8 unk30;      /* the stage's 0xD58 */
} Unk800D69C4;

extern EntityVtable D_800D3B84;

extern EntityVtable D_800D3B5C;

/* Constructs an entity of D_800D3B84 at pos: it moves 4 plus a 256th of how far pos is in front of z 0x1000. */
Unk800D69C4 *func_800D69C4(Unk800D69C4 *this, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B84;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->unk2C = ((0x1000 - pos->vz) >> 8) + 4;
    return this;
}

/* Unk800D69C4's gather: copies the stage's 0xD58 and moves x left by 0x2C, wrapping from -0x300 to 0x300. */
void func_800D6A38(Unk800D69C4 *this, Unk800D3E18 *stage) {
    s32 x = this->pos.vx - this->unk2C;

    this->unk30 = stage->unkD58;
    this->pos.vx = x;
    if (x <= -0x300) {
        this->pos.vx = x + 0x600;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6A6C);

/* The gather of func_800D8B4C's entity: copies the stage's 0xD58 and moves x left by 0x2C; past -0x1800 it
 * wraps to the right at a new random depth (0x200 to 0x9FF), which sets the speed again. */
void func_800D6C94(Unk800D69C4 *this, Unk800D3E18 *stage) {
    s32 x = this->pos.vx - this->unk2C;
    /* The match depends on the generator's address taken here, before the test. */
    MersenneTwister *random = &RANDOM;

    this->unk30 = stage->unkD58;
    this->pos.vx = x;
    if (x <= -0x1800) {
        s32 z;

        this->pos.vx = x + 0x3000;
        z = (mersenneTwisterGenerate(random) & 0x7FF) + 0x200;
        this->pos.vz = z;
        this->unk2C = ((0x1000 - z) >> 8) + 4;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6D0C);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6E38);

/* A jittering line through up to 8 points (three of them sit at 0x18 of func_800D7AC4's object, 0x14C apart):
 * func_800D72C8 lays the targets out, func_800D6F30 moves each point toward its target plus a random offset,
 * func_800D701C draws the segments between the points. */
typedef struct Unk800D8B8C {
    /* 0x000 */ VECTOR target[8];
    /* 0x080 */ VECTOR point[8];
    /* 0x100 */ s32 unk100[8];
    /* 0x120 */ u32 color[8]; /* r, g, b bytes; word-aligned (lw), so not a CVECTOR */
    /* 0x140 */ s32 count;
    /* 0x144 */ s32 jitter; /* the range of the random offset */
    /* 0x148 */ s32 unk148; /* scales the segments' width (func_800D70BC) */
} Unk800D8B8C; /* size 0x14C */

void func_800D70BC(Unk800D8B8C *this, Camera *camera, VECTOR *point, u32 *color, s32 unk100);
void func_800D72C8(Unk800D8B8C *this, VECTOR *from, VECTOR *to);
void func_800D7458(Unk800D8B8C *this, u32 from, u32 to);

/* Moves each point a sixteenth of the way toward its target plus a random offset in x and y. */
void func_800D6F30(Unk800D8B8C *this) {
    MersenneTwister *random = &RANDOM;
    /* The match depends on taking point before target (point comes from $a0, target keeps this's copy). */
    VECTOR *point = this->point;
    VECTOR *target = this->target;
    s32 jitter = this->jitter;
    s32 count = this->count;
    s32 i;
    s32 dx;
    s32 dy;

    if (jitter <= 0) {
        jitter = 1;
    }
    for (i = 0; i < count; i++) {
        dx = (s32)mersenneTwisterGenerate(random) % jitter;
        dy = (s32)mersenneTwisterGenerate(random) % jitter;
        point->vx += (target->vx + dx - point->vx) >> 4;
        point->vy += (target->vy + dy - point->vy) >> 4;
        target++;
        point++;
    }
}

/* Draws the segments between consecutive points. */
void func_800D701C(Unk800D8B8C *this, Camera *camera) {
    VECTOR *point = this->point;
    s32 *unk100 = this->unk100;
    u32 *color = this->color;
    s32 segments = this->count - 1;
    s32 i = 0;

    /* The match depends on counting the segment before drawing it: the counter steps at the top of the loop. */
    while (i < segments) {
        i++;
        func_800D70BC(this, camera, point, color, *unk100);
        point++;
        unk100++;
        color++;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D70BC);

/* Lays the targets out evenly from from to to, one per 128 units of length (3 to 8 of them); when their count
 * changes, the points jump straight to their targets. */
void func_800D72C8(Unk800D8B8C *this, VECTOR *from, VECTOR *to) {
    VECTOR delta;
    s32 x, y, z;
    s32 count;
    s32 i;

    x = to->vx;
    y = to->vy;
    z = to->vz;
    x -= from->vx;
    y -= from->vy;
    z -= from->vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    count = vecGetLength(&delta) / 128 + 1;
    if (count > 8) {
        count = 8;
    }
    if (count < 3) {
        count = 3;
    }
    for (i = 0; i < count; i++) {
        vecLerp(&this->target[i], from, to, (i << 12) / (count - 1));
    }
    if (this->count != count) {
        for (i = 0; i < count; i++) {
            /* The match depends on a three-word copy (Vec3): the loop then walks the points, not the targets. */
            *(Vec3 *)&this->point[i] = *(Vec3 *)&this->target[i];
        }
    }
    this->count = count;
}

/* Gives the inner points color and blacks out both ends. */
void func_800D7418(Unk800D8B8C *this, u32 color) {
    s32 last = this->count - 1;
    s32 i;

    for (i = 1; i < last; i++) {
        this->color[i] = color;
    }
    this->color[last] = 0;
    this->color[0] = 0;
}

/* Fades the points' colors from from (first point) to to (last point); a color is r, g, b in its low three bytes. */
void func_800D7458(Unk800D8B8C *this, u32 from, u32 to) {
    VECTOR fromRgb;
    VECTOR toRgb;
    VECTOR rgb;
    u8 *fromBytes = (u8 *)&from;
    u8 *toBytes = (u8 *)&to;
    s32 last;
    s32 i;

    fromRgb.vx = fromBytes[0];
    fromRgb.vy = fromBytes[1];
    fromRgb.vz = fromBytes[2];
    toRgb.vx = toBytes[0];
    toRgb.vy = toBytes[1];
    toRgb.vz = toBytes[2];
    last = this->count - 1;
    for (i = 0; i <= last; i++) {
        vecLerp(&rgb, &fromRgb, &toRgb, (i << 12) / last);
        this->color[i] = rgb.vx | (rgb.vy << 8) | (rgb.vz << 16);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7534);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7764);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7AC4);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7B58);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7D88);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7F20);

extern EntityVtable D_800D3AE4;

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

/* Follows the character: deletes itself once the character is gone or unk38 has faded out (4 a frame);
 * meanwhile takes the character's position, a new random unk34 and steps unk30 through 0, 1, 2. */
void func_800D8158(Unk800D80DC *this) {
    Unk80076F34 *chara = (Unk80076F34 *)getEntityByHandle(this->chara);

    if (chara == NULL) {
        entityDelete(&this->entity);
        return;
    }
    func_8007A650(chara, &this->position);
    this->unk34 = mersenneTwisterGenerate(&RANDOM);
    this->unk30++;
    if (this->unk30 >= 3) {
        this->unk30 = 0;
    }
    this->unk38 -= 4;
    if (this->unk38 <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D8224);

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

INCLUDE_ASM("st03/nonmatchings/st03", func_800D83C0);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D849C);

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

/* Returns the stage's unkD50. */
s32 func_800D878C(Unk800D3E18 *this) {
    return this->unkD50;
}

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

/* The destructor of Unk800D5060 (virtual tables D_800D3C94 and D_800D3C74). */
void func_800D88FC(Unk800D5060 *this, s32 flags) {
    this->unk40.base.vtable = D_800D3C74;
    this->base.entity.vtable = &D_800D3C94.entity;
    this->part.vtable = D_80065184;
    partRemove(this);
    this->unk40.base.vtable = D_80065164;
    entityDestroy(&this->base.entity, 0);
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

/* func_800D72C8 from point to the last point. */
void func_800D8B8C(Unk800D8B8C *this, VECTOR *point) {
    func_800D72C8(this, point, &this->point[this->count - 1]);
}

/* func_800D72C8 from the last point to point. */
void func_800D8BBC(Unk800D8B8C *this, VECTOR *point) {
    func_800D72C8(this, &this->point[this->count - 1], point);
}

/* func_800D7458 from color to the last point's color. */
void func_800D8BEC(Unk800D8B8C *this, u32 color) {
    func_800D7458(this, color, this->color[this->count - 1]);
}

/* func_800D7458 from the last point's color to color. */
void func_800D8C24(Unk800D8B8C *this, u32 color) {
    func_800D7458(this, this->color[this->count - 1], color);
}

/* Sets the range of the points' random offset. */
void func_800D8C5C(Unk800D8B8C *this, s32 jitter) {
    this->jitter = jitter;
}

/* Sets unk148. */
void func_800D8C64(Unk800D8B8C *this, s32 unk148) {
    this->unk148 = unk148;
}

/* Returns this: an empty constructor of Unk800D8B8C (hypothesis: it follows its methods; nothing calls it). */
Unk800D8B8C *func_800D8C6C(Unk800D8B8C *this) {
    return this;
}

/* The entity func_800D7534 constructs (virtual table D_800D3B34: func_800D7764, func_800D7AC4 and
 * func_800D8C74): four jittering lines (func_800D7AC4 draws all four), an Unk800D885C (virtual table
 * D_800D3CF4) and two parts in its list. */
typedef struct Unk800D7534 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk800D8B8C lines[4];
    /* 0x548 */ Unk800D885C unk548;
    /* 0x564 */ u8 unk564[4];
    /* 0x568 */ Unk80075EE4 unk568;
    /* 0x5B0 */ Unk80075EE4 unk5B0;
} Unk800D7534; /* size 0x5F8 */

/* The destructor of Unk800D7534: its parts' (last first) and its Unk800D885C's, inlined. */
void func_800D8C74(Unk800D7534 *this, s32 flags) {
    /* Each listRemove written out in the access order the game's compiler used (as partRemove), with a node
     * of its own (one node assigned twice gives other registers). */
    this->unk5B0.vtable = D_80065184;
    {
        ListNode *node = &this->unk5B0.node;

        this->unk5B0.node.next->prev = this->unk5B0.node.prev;
        node->prev->next = this->unk5B0.node.next;
        this->unk5B0.node.next = node;
        this->unk5B0.node.prev = node;
    }
    this->unk568.vtable = D_80065184;
    {
        ListNode *node = &this->unk568.node;

        this->unk568.node.next->prev = this->unk568.node.prev;
        node->prev->next = this->unk568.node.next;
        this->unk568.node.next = node;
        this->unk568.node.prev = node;
    }
    this->unk548.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

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
