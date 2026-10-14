/* The title's character model (TitleChara) that the character select and the guide show. The file's
 * boundary is unproven: no string or constructor call marks it, only a cut consistent with the order of
 * .rodata and .data. */

#include "common.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/task/entity.h"
#include "engine/game/character.h"
#include "engine/math/matrix.h"
#include "engine/system/memory.h"
#include "title/title_classes.h"

extern RECT *TITLE_CHARA_EYES[][2];
extern RECT *TITLE_CHARA_MOUTHS[][2];

extern EntityVtable TITLE_CHARA_VTABLE;

void TitleChara_startClip(TitleChara *this, s32 clip, s32 blendFrames, s32 plays);

/* The title character's constructor: loads character 0x18, makes its model in clip 0 and places it at
 * position, turned by rotation about x, y and z, to be drawn in camera. */
TitleChara *TitleChara_init(TitleChara *this, Camera *camera, Vec3 *position, Vec3 *rotation) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &TITLE_CHARA_VTABLE;
    this->camera = camera;
    this->chara = charaInit(operatorNew(sizeof(Chara)), 0x18);
    charaLoad(this->chara);
    this->model = modelInit(operatorNew(sizeof(Model)), charaGetTmd(this->chara), charaGetAnimData(this->chara));
    TitleChara_startClip(this, 0, 0, 0);
    matrixTranslate(&this->model->group.object.transform.local.m, position->vx, position->vy, position->vz);
    matrixRotateX(&this->model->group.object.transform.local.m, rotation->vx);
    matrixRotateY(&this->model->group.object.transform.local.m, rotation->vy);
    matrixRotateZ(&this->model->group.object.transform.local.m, rotation->vz);
    this->model->group.object.transform.valid = 0;
    this->nextAction = 0;
    this->action = 0;
    this->unk2C = 0;
    this->eyes = 0;
    this->blinkState = 0;
    this->blinkWait = 0;
    this->blinkTimer = 0;
    this->frame = 0;
    this->interruptible = 0;
    this->voice = 0;
    return this;
}

/* Destroys a scene object and frees it, as `delete` does: nothing for NULL (scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* The title character's destructor: deletes the model and the character. */
void TitleChara_destroy(TitleChara *this, s32 flags) {
    this->entity.vtable = &TITLE_CHARA_VTABLE;
    sceneObjectDelete(&this->model->group.object);
    if (this->chara != NULL) {
        charaDestroy(this->chara, DESTROY_DELETE);
    }
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("title/nonmatchings/title_chara", TitleChara_tick);

INCLUDE_ASM("title/nonmatchings/title_chara", TitleChara_tickBlink);

/* The title character's draw (slot 0x10): animates the model and draws it in its camera. */
void TitleChara_draw(TitleChara *this) {
    Model *model = this->model;

    modelUpdateAnimation(model);
    SCENE_OBJECT_INVALIDATE(&model->group.object);
    sceneObjectDrawInView(&model->group.object, this->camera);
}

/* Starts clip of the model and sets the frame its loops start again from the character's .loop file. */
void TitleChara_startClip(TitleChara *this, s32 clip, s32 blendFrames, s32 plays) {
    modelStartClip(this->model, clip, blendFrames, plays, 0);
    modelSetLoopFrame(this->model, charaGetLoopFrame(this->chara, clip));
}

/* Asks for action (TITLE_CHARA_*); when interruptible, also ends the action playing and forgets its voice. */
void TitleChara_setAction(TitleChara *this, s32 action) {
    this->nextAction = action;
    if (this->interruptible == 1) {
        this->action = 0;
        this->voice = 0;
    }
}

/* Sets the handle of the voice line playing: the mouth moves until it ends. */
void TitleChara_setVoice(TitleChara *this, s32 voice) {
    this->voice = voice;
}

/* Returns the action playing (TITLE_CHARA_IDLE once it ended). */
s32 TitleChara_getAction(TitleChara *this) {
    return this->action;
}

/* Returns whether the model's animation has stopped; nothing calls it. */
s32 TitleChara_isStopped(TitleChara *this) {
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
void TitleChara_setTransform(TitleChara *this, Vec3 *position, Vec3 *rotation) {
    modelInitMatrix(this->model);
    matrixRotateX(&this->model->group.object.transform.local.m, rotation->vx);
    matrixRotateY(&this->model->group.object.transform.local.m, rotation->vy);
    matrixRotateZ(&this->model->group.object.transform.local.m, rotation->vz);
    modelSetPosition(this->model, position);
    this->model->group.object.transform.valid = 0;
}

/* Shows eye picture index: copies its two VRAM rects of TITLE_CHARA_EYES to (360, 0) and (320, 254) with
 * DR_MOVEs in the camera's ordering table. */
void TitleChara_setEyes(TitleChara *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *TITLE_CHARA_EYES[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x168, 0));
    rect = *TITLE_CHARA_EYES[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFE));
}

/* Shows mouth picture index: copies its two VRAM rects of TITLE_CHARA_MOUTHS to (368, 0) and (320, 253) with
 * DR_MOVEs in the camera's ordering table. */
void TitleChara_setMouth(TitleChara *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *TITLE_CHARA_MOUTHS[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x170, 0));
    rect = *TITLE_CHARA_MOUTHS[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFD));
}

/* Lets a new action cut the one playing; returns 1. Nothing calls it. */
s32 TitleChara_setInterruptible(TitleChara *this) {
    this->interruptible = 1;
    return 1;
}

INCLUDE_RODATA("title/nonmatchings/title_chara", TITLE_CHARA_VTABLE);
