#include "common.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/animator.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"
#include "libgpu.h"
#include "kernel.h"
#include "psyq.h"

/* ApplyMatrix, which nothing calls through this pointer */
VECTOR *(*APPLY_MATRIX_FUNC)(MATRIX *m, SVECTOR *v, VECTOR *r) = func_80053DF0;

/* Builds a transform with no parent and the identity as its matrix. */
Transform *transformInit(Transform *this) {
    this->local.words[0] = ONE;
    this->local.words[1] = 0;
    this->local.words[2] = ONE;
    this->local.words[3] = 0;
    this->local.words[4] = ONE;
    this->local.words[5] = 0;
    this->local.words[6] = 0;
    this->local.words[7] = 0;
    this->valid = 0;
    this->parent = NULL;
    return this;
}

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformAttach);

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformDetach);

/* Destroys the scene object, taking it out of its group. */
void sceneObjectDestroy(SceneObject *this, s32 flags) {
    this->vtable = &SCENE_OBJECT_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Builds an empty group. */
Group *groupInit(Group *this) {
    this->object.link.next = &this->object.link;
    this->object.link.prev = &this->object.link;
    transformInit(&this->object.transform);
    this->object.vtable = &GROUP_VTABLE;
    this->children.next = &this->children;
    this->children.prev = &this->children;
    this->object.hidden = 0;
    return this;
}

/* Destroys a scene object and frees it, as `delete` does: nothing for NULL. */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Deletes the children, then destroys the group as a scene object. */
void groupDestroy(Group *this, s32 flags) {
    ListNode *head;
    ListNode *node;
    ListNode *list = &this->children;

    this->object.vtable = &GROUP_VTABLE;
    /* link is the first member of a SceneObject */
    for (node = list->next; node != list;) {
        ListNode *next = node->next;
        sceneObjectDelete((SceneObject *)node);
        node = next;
    }

    /*
     * The second line reads children.prev through the pointer to the head,
     * the others through the member, as StepperGroup's destructor does
     * (menu/stepper.cpp). Which helper or form the original wrote is not
     * known.
     */
    head = &this->children;
    this->children.next->prev = this->children.prev;
    head->prev->next = this->children.next;
    this->children.next = head;
    this->children.prev = head;
    sceneObjectDestroy(&this->object, flags);
}

/* Adds child at the end of the children. Returns the group, for chaining. */
Group *groupAddChild(Group *this, SceneObject *child) {
    listInsertAfter(this->children.prev, &child->link);
    return this;
}

/* Marks the world matrices of the group and its children out of date. */
void groupInvalidate(Group *this) {
    ListNode *head = &this->children;
    ListNode *node = head->next;
    ListNode *next;

    this->object.transform.valid = 0;
    for (; node != head; node = next) {
        /* the link is the first field of a SceneObject */
        SceneObject *child = (SceneObject *)node;

        next = node->next;
        child->vtable->invalidate.func((u8 *)child + child->vtable->invalidate.delta);
    }
}

/* Culls the children, marking each one culled when it is hidden or out of
 * view. Returns whether the whole group is out of view: hidden, or with
 * every child out of view. */
s32 groupCull(Group *this, Camera *camera) {
    ListNode *head = &this->children;
    ListNode *node = head->next;
    ListNode *next;
    s32 culled;
    s32 anyVisible = 0; /* kept 0 or 1 */

    for (; node != head; node = next) {
        SceneObject *child = (SceneObject *)node;

        next = node->next;
        culled = child->vtable->cull.func((u8 *)child + child->vtable->cull.delta, camera);
        anyVisible = (anyVisible | (!this->object.hidden && !culled)) != 0;
        child->culled = child->hidden | (u32)(culled != 0);
    }
    /* anyVisible is 0 or 1, so ^ 1 is its negation */
    return this->object.culled = anyVisible ^ 1;
}

/* Draws the scene object unless it is out of view. */
void sceneObjectDrawInView(SceneObject *this, Camera *camera) {
    if (!this->vtable->cull.func((u8 *)this + this->vtable->cull.delta, camera)) {
        this->vtable->draw.func((u8 *)this + this->vtable->draw.delta, camera);
    }
}

/* Draws the children in view, while there is time left in the frame. */
void groupDraw(Group *this, Camera *camera) {
    ListNode *node;
    ListNode *next;

    for (node = this->children.next; node != &this->children && !isPrimBufferNearlyFull(); node = next) {
        SceneObject *child = (SceneObject *)node;

        next = node->next;
        if (!child->culled) {
            child->vtable->draw.func((u8 *)child + child->vtable->draw.delta, camera);
        }
    }
}

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshDraw);

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshCull);

/* Clears the culling counters. */
void resetMeshCullCounts(void) {
    MESH_CULL_COUNT = 0;
    MESH_OUT_OF_VIEW_COUNT = 0;
}

/* How many meshes were culled. */
s32 getMeshCullCount(void) {
    return MESH_CULL_COUNT;
}

/* How many meshes were out of view. */
s32 getMeshOutOfViewCount(void) {
    return MESH_OUT_OF_VIEW_COUNT;
}

/* Builds a mesh that draws tmd. */
Mesh *meshInit(Mesh *this, TmdObject *tmd, s32 unused) {
    this->object.link.next = &this->object.link;
    this->object.link.prev = &this->object.link;
    transformInit(&this->object.transform);
    this->object.vtable = &MESH_VTABLE;
    this->object.hidden = 0;
    meshSetTmd(this, tmd);
    this->debug = 0;
    return this;
}

/* The corners of the bounding box. */
SVECTOR *meshGetCorners(Mesh *this) {
    return this->corners;
}

/* Has the mesh draw tmd, and works out its bounding box and sphere. The
 * register allocation differs; a permuter run found only junk forms. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshSetTmd);

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshPartUpdate);

/* Builds a model of the objects of tmd, a MeshPart for each node anim
 * moves. C with the part's AnimNode taken into a variable before the
 * null test of its upcast (part != NULL ? node : NULL) has every
 * instruction, but the variable gets v0 where the game has s0; the game's
 * choice leaves tmd no saved register, so it keeps tmd in its argument
 * slot and reloads it each pass. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", modelInit);

/* Destroys the model. */
void modelDestroy(Model *this, s32 flags) {
    this->group.object.vtable = &MODEL_VTABLE;
    if (this->parts != NULL) {
        operatorVecDelete(this->parts);
    }
    animatorDestroy(&this->animator, 2);
    groupDestroy(&this->group, flags);
}

s32 modelGetFrameCount(Model *this) {
    return animatorGetFrameCount(&this->animator);
}

s32 modelGetFrame(Model *this) {
    return animatorGetFrame(&this->animator);
}

s32 modelGetClip(Model *this) {
    return animatorGetClip(&this->animator);
}

s32 modelIsStopped(Model *this) {
    return animatorIsStopped(&this->animator);
}

void modelStepAnimation(Model *this) {
    animatorUpdate(&this->animator);
}

/* Plays clip, blending into it over blendFrames frames. */
void modelStartClip(Model *this, s32 clip, s32 blendFrames, s32 plays, s32 frame) {
    animatorStartClip(&this->animator, clip, blendFrames, plays, frame);
}

/* Makes the animation's loops start again at frame. */
void modelSetLoopFrame(Model *this, s32 frame) {
    animatorSetLoopFrame(&this->animator, frame);
}

/* Draws the parts in view, with the model's lights when it has some. */
void modelDraw(Model *this, Camera *camera) {
    LightsOverride lights;

    if (this->lights != NULL) {
        lightsOverrideInit(&lights, camera, this->lights);
        groupDraw(&this->group, camera);
        lightsOverrideDestroy(&lights, 2);
    } else {
        groupDraw(&this->group, camera);
    }
}

/* Sets the lights the model is drawn with; NULL keeps the camera's. */
void modelSetLights(Model *this, Lights *lights) {
    this->lights = lights;
}

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformGetWorld);

/* Marks the world matrix out of date, or up to date. */
void transformSetDirty(Transform *this, s32 dirty) {
    this->valid = dirty ^ 1;
}

/* The matrix relative to the parent, to change: the world matrix goes out of
 * date. */
MATRIX *transformEditLocal(Transform *this) {
    this->valid = 0;
    return &this->local.m;
}

/* The matrix relative to the parent. */
MATRIX *transformGetLocal(Transform *this) {
    return &this->local.m;
}

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformSetLocal);

INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", func_8002099C);

/* Sets the parent, keeping the matrix relative to it. */
void transformSetParentKeepLocal(Transform *this, Transform *parent) {
    transformSetParentKeepLocalInline(this, parent);
}

/* Marks the world matrix out of date. */
void sceneObjectInvalidate(SceneObject *this) {
    this->transform.valid = 0;
}

/* Builds a scene object in no group. */
SceneObject *sceneObjectInit(SceneObject *this) {
    this->link.next = &this->link;
    this->link.prev = &this->link;
    transformInit(&this->transform);
    this->vtable = &SCENE_OBJECT_VTABLE;
    this->hidden = 0;
    return this;
}

/* Hides or shows the scene object. */
void sceneObjectSetHidden(SceneObject *this, s32 hidden) {
    this->hidden = hidden;
}

/* The children of the group. */
ListNode *groupGetChildren(Group *this) {
    return &this->children;
}

/* Adds child at the end of the children. */
void func_80020A74(Group *this, SceneObject *child) {
    groupAddChild(this, child);
}

/* Destroys the mesh. */
void meshDestroy(Mesh *this, s32 flags) {
    this->object.vtable = &MESH_VTABLE;
    sceneObjectDestroy(&this->object, flags);
}

/* The part that draws an object of the TMD. */
MeshPart *modelGetPart(Model *this, s32 object) {
    return this->parts[object];
}

/* What animates the model. */
Animator *modelGetAnimator(Model *this) {
    return &this->animator;
}

/* Destroys the part. */
void meshPartDestroy(MeshPart *this, s32 flags) {
    this->node.vtable = &MESH_PART_ANIM_NODE_VTABLE;
    this->mesh.object.vtable = &MESH_VTABLE;
    sceneObjectDestroy(&this->mesh.object, flags);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", MESH_BOUNDS_EDGES);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", MESH_PART_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", MESH_PART_ANIM_NODE_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", MODEL_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", MESH_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", GROUP_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/scene_graph", SCENE_OBJECT_VTABLE);
