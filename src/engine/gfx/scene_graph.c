#include "common.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/animator.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "psyq.h"
#include "vtable.h"

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

/*
 * The functions of this file left in asm but meshSetTmd copy a MATRIX as a
 * block move (four loads then four stores), which GCC 2.95.2 does not emit
 * for 32 bytes (see "Block moves" in TODO.md): transformAttach,
 * transformDetach, meshDraw, meshCull, meshPartUpdate, transformGetWorld,
 * transformSetLocal and func_8002099C.
 */

/* Makes parent the parent of the transform, keeping where it is in the
 * world: the local matrix becomes the inverse of the parent's world matrix
 * times it. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformAttach);

/* Takes the transform from its parent, keeping where it is in the world:
 * the local matrix becomes the parent's world matrix times it. */
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

/* Draws the TMD object of the mesh in camera with its world matrix, and,
 * with MESH_SHOW_BOUNDS in debug, its bounding box as 12 white lines. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshDraw);

/* Whether the mesh is out of view of camera: 1 with no TMD object, else
 * whether the sphere around it misses the view. Counts the tests in
 * MESH_CULL_COUNT and the misses in MESH_OUT_OF_VIEW_COUNT. */
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

/*
 * Has the mesh draw tmd, and works out its bounding box and sphere. C that
 * reads the bounds into s16 locals has the game's 81 instructions (its six
 * lh and three lhu reloads of max), but not their order: the game reloads
 * after all 24 corner stores and stores the center after the reloads, as if
 * stores through this could overlap the stack copy of the bounds. Stock GCC
 * 2.95.2 rules that out (alias.c, base_alias_check), in C and C++ alike: it
 * only allows it when the base of this is unknown, as for a u32 parameter or a
 * helper's pointer argument, which then keep an addiu per corner.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshSetTmd);

/* Builds the local matrix of the part from pose, the pose of its node: the
 * rotation, then the scale, then the translation. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", meshPartUpdate);

/* the most nodes modelInit links up: the size of its table of parts */
#define MODEL_NODE_MAX 64

/*
 * Builds a model of the objects of tmd, a MeshPart for each node anim moves,
 * then makes each part's transform a child of its parent node's (of the
 * model's for the roots).
 *
 * Each part is built as g++ builds a `new MeshPart` (Mesh's constructor, then
 * the vtables), and its AnimNode is taken right away, before the calls: the
 * scheduler then moves it down to its use, but it is still allocated as a
 * value that lives across calls, which is why the game keeps it in s0 and,
 * out of saved registers, keeps tmd in its argument slot, reloaded each pass.
 * The null tests are those of g++'s conversion of a pointer to a base class
 * that is not the first one.
 */
Model *modelInit(Model *this, TmdHeader *tmd, AnimData *anim) {
    MeshPart *parts[MODEL_NODE_MAX]; /* by node */
    s32 count;
    s32 i;

    groupInit(&this->group);
    this->group.object.vtable = &MODEL_VTABLE;
    animatorInit(&this->animator, anim);
    count = animatorGetNodeCount(&this->animator);
    this->parts = operatorVecNew(count * sizeof(MeshPart *));
    for (i = 0; i < count; i++) {
        MeshPart *part = operatorNew(sizeof(MeshPart));
        AnimNode *node = &part->node;
        s32 object;

        meshInit(&part->mesh, NULL, 0);
        part->node.vtable = &MESH_PART_ANIM_NODE_VTABLE;
        part->mesh.object.vtable = &MESH_PART_VTABLE;
        object = animatorGetNodeObject(&this->animator, i);
        meshSetTmd(&part->mesh, tmdHeaderGetObject(tmd, object));
        animatorSetNode(&this->animator, i, part != NULL ? node : NULL);
        groupAddChild(&this->group, &part->mesh.object);
        this->parts[object] = part;
        parts[i] = part;
    }
    for (i = 0; i < count; i++) {
        s32 parent = animatorGetNodeParent(&this->animator, i);

        if (parent < 0) {
            transformSetParentKeepLocalInline(&parts[i]->mesh.object.transform,
                                              &this->group.object.transform);
        } else {
            transformSetParentKeepLocalInline(&parts[i]->mesh.object.transform,
                                              parts[parent] != NULL ? &parts[parent]->mesh.object.transform : NULL);
        }
    }
    this->lights = NULL;
    return this;
}

/* Destroys the model. */
void modelDestroy(Model *this, s32 flags) {
    this->group.object.vtable = &MODEL_VTABLE;
    if (this->parts != NULL) {
        operatorVecDelete(this->parts);
    }
    animatorDestroy(&this->animator, DESTROY_BASES);
    groupDestroy(&this->group, flags);
}

/* How many frames the playing clip has. */
s32 modelGetFrameCount(Model *this) {
    return animatorGetFrameCount(&this->animator);
}

/* The frame the animation is at. */
s32 modelGetFrame(Model *this) {
    return animatorGetFrame(&this->animator);
}

/* The clip playing. */
s32 modelGetClip(Model *this) {
    return animatorGetClip(&this->animator);
}

/* Whether the animation has stopped. */
s32 modelIsStopped(Model *this) {
    return animatorIsStopped(&this->animator);
}

/* Moves the animation on a frame. */
void modelUpdateAnimation(Model *this) {
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
        lightsOverrideDestroy(&lights, DESTROY_BASES);
    } else {
        groupDraw(&this->group, camera);
    }
}

/* Sets the lights the model is drawn with; NULL keeps the camera's. */
void modelSetLights(Model *this, Lights *lights) {
    this->lights = lights;
}

/* The world matrix: the parent's world matrix times the local one, worked
 * out again when the transform changed; the local matrix at the root. */
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

/* Sets the matrix relative to the parent, so the world matrix is worked
 * out again. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/scene_graph", transformSetLocal);

/* The same code as transformSetLocal. */
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
