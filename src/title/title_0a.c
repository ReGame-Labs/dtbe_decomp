/* A character model class seen by a camera (virtual table D_8006659C) that the character select, the
 * guide and the options use. The boundary is unproven: no string or constructor call marks it, only
 * a cut consistent with the order of .rodata and .data. */

#include "common.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/task/entity.h"
#include "engine/game/character.h"
#include "engine/math/matrix.h"
#include "engine/system/memory.h"
#include "title/title_classes.h"

/* The entity class of vtable D_8006659C (func_80076E24): a character model of the title, seen by camera.
 * func_800774D4 and func_800775AC draw through the camera's ordering table. */
typedef struct Unk80076E24 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Camera *camera;
    /* 0x1C */ Chara *chara;
    /* 0x20 */ Model *model;
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 unk26; /* func_800773C8 sets it */
    /* 0x28 */ s16 unk28; /* func_800773F0 returns it */
    /* 0x2A */ u8 unk2A[0x2C - 0x2A];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s16 unk30;
    /* 0x32 */ s16 unk32;
    /* 0x34 */ s16 unk34;
    /* 0x36 */ s16 unk36;
    /* 0x38 */ s32 unk38; /* func_80077684 sets it to 1 */
    /* 0x3C */ s32 unk3C; /* func_800773E8 sets it */
} Unk80076E24; /* size 0x40 */

extern RECT *D_8007B2DC[][2];
extern RECT *D_8007B2FC[][2];

extern EntityVtable D_8006659C;

void func_80077378(Unk80076E24 *this, s32 clip, s32 blendFrames, s32 plays);

/* The constructor of the title's character model entity (vtable D_8006659C): loads character 0x18, makes its
 * model in clip 0 and places it at position, turned by rotation about x, y and z, to be drawn in camera. */
Unk80076E24 *func_80076E24(Unk80076E24 *this, Camera *camera, Vec3 *position, Vec3 *rotation) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006659C;
    this->camera = camera;
    this->chara = charaInit(operatorNew(sizeof(Chara)), 0x18);
    charaLoad(this->chara);
    this->model = modelInit(operatorNew(sizeof(Model)), charaGetTmd(this->chara), charaGetAnimData(this->chara));
    func_80077378(this, 0, 0, 0);
    matrixTranslate(&this->model->group.object.transform.local.m, position->vx, position->vy, position->vz);
    matrixRotateX(&this->model->group.object.transform.local.m, rotation->vx);
    matrixRotateY(&this->model->group.object.transform.local.m, rotation->vy);
    matrixRotateZ(&this->model->group.object.transform.local.m, rotation->vz);
    this->model->group.object.transform.valid = 0;
    this->unk26 = 0;
    this->unk28 = 0;
    this->unk2C = 0;
    this->unk30 = 0;
    this->unk32 = 0;
    this->unk34 = 0;
    this->unk36 = 0;
    this->unk24 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    return this;
}

/* Destroys a scene object and frees it, as `delete` does: nothing for NULL (scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* The destructor of the title's character model entity (vtable D_8006659C): deletes the model and the
 * character. */
void func_80076F6C(Unk80076E24 *this, s32 flags) {
    this->entity.vtable = &D_8006659C;
    sceneObjectDelete(&this->model->group.object);
    if (this->chara != NULL) {
        charaDestroy(this->chara, DESTROY_DELETE);
    }
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("title/nonmatchings/title_0a", func_80077000);

INCLUDE_ASM("title/nonmatchings/title_0a", func_80077208);

/* The update (entry 0x10) of vtable D_8006659C: animates the model and draws it in the camera at 0x18. */
void func_8007731C(Unk80076E24 *this) {
    Model *model = this->model;

    modelUpdateAnimation(model);
    SCENE_OBJECT_INVALIDATE(&model->group.object);
    sceneObjectDrawInView(&model->group.object, this->camera);
}

/* Starts clip of the model and sets the frame its loops start again from the character's .loop file. */
void func_80077378(Unk80076E24 *this, s32 clip, s32 blendFrames, s32 plays) {
    modelStartClip(this->model, clip, blendFrames, plays, 0);
    modelSetLoopFrame(this->model, charaGetLoopFrame(this->chara, clip));
}

/* Sets 0x26; when 0x38 is 1, also clears 0x28 and 0x3C. */
void func_800773C8(Unk80076E24 *this, s32 value) {
    this->unk26 = value;
    if (this->unk38 == 1) {
        this->unk28 = 0;
        this->unk3C = 0;
    }
}

/* Sets 0x3C. */
void func_800773E8(Unk80076E24 *this, s32 value) {
    this->unk3C = value;
}

/* Returns 0x28. */
s32 func_800773F0(Unk80076E24 *this) {
    return this->unk28;
}

/* Returns whether the model's animation has stopped; nothing calls it. */
s32 func_800773FC(Unk80076E24 *this) {
    return modelIsStopped(this->model);
}

/* Sets the model's own matrix to the identity, as matrixInitIdentity does (inline). */
static inline void modelInitMatrix(Model *model) {
    model->group.object.transform.local.words[0] = ONE;
    model->group.object.transform.local.words[1] = 0;
    model->group.object.transform.local.words[2] = ONE;
    model->group.object.transform.local.words[3] = 0;
    model->group.object.transform.local.words[4] = ONE;
    model->group.object.transform.local.m.t[0] = 0;
    model->group.object.transform.local.m.t[1] = 0;
    model->group.object.transform.local.m.t[2] = 0;
}

/* Sets the translation of the model's own matrix to position (inline). */
static inline void modelSetPosition(Model *model, Vec3 *position) {
    s32 x = position->vx;
    s32 y = position->vy;
    s32 z = position->vz;

    model->group.object.transform.local.m.t[0] = x;
    model->group.object.transform.local.m.t[1] = y;
    model->group.object.transform.local.m.t[2] = z;
}

/* Sets the model's matrix: no rotation, then turned about x, y and z by rotation, at position; out of date.
 * The match depends on the two inlines, which take the model (one load of this->model each, the matrix
 * offsets folded into the stores). */
void func_80077420(Unk80076E24 *this, Vec3 *position, Vec3 *rotation) {
    modelInitMatrix(this->model);
    matrixRotateX(&this->model->group.object.transform.local.m, rotation->vx);
    matrixRotateY(&this->model->group.object.transform.local.m, rotation->vy);
    matrixRotateZ(&this->model->group.object.transform.local.m, rotation->vz);
    modelSetPosition(this->model, position);
    this->model->group.object.transform.valid = 0;
}

/* Copies the two VRAM rects of entry index of D_8007B2DC to (360, 0) and (320, 254) with DR_MOVEs in the camera's
 * ordering table. */
void func_800774D4(Unk80076E24 *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *D_8007B2DC[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x168, 0));
    rect = *D_8007B2DC[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFE));
}

/* Copies the two VRAM rects of entry index of D_8007B2FC to (368, 0) and (320, 253) with DR_MOVEs in the camera's
 * ordering table. */
void func_800775AC(Unk80076E24 *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *D_8007B2FC[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x170, 0));
    rect = *D_8007B2FC[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFD));
}

/* Sets 0x38 to 1 and returns 1. */
s32 func_80077684(Unk80076E24 *this) {
    this->unk38 = 1;
    return 1;
}
