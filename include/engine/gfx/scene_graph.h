#ifndef DTBE_GFX_SCENE_GRAPH_H
#define DTBE_GFX_SCENE_GRAPH_H

/* The scene graph: transforms, groups, culled TMD meshes and animated TMD models. */

#include "common.h"
#include <libgte.h>
#include "vtable.h"
#include "engine/gfx/animator.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"

EXTERN_C_BEGIN

/* A transform in a hierarchy: a matrix relative to the parent's, and the
 * world matrix they give, worked out when asked for (transformGetWorld). */
typedef struct Transform {
    /* 0x00 */ s32 valid; /* world is up to date */
    /* 0x04 */ MATRIX world;
    /* 0x24 */ MatrixWords local;
    /* 0x44 */ struct Transform *parent;
} Transform; /* size 0x48 */

struct SceneObject;

typedef struct SceneObjectVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry invalidate; /* (SceneObject *): its world moved */
    /* 0x10 */ struct {
        s16 delta;
        s16 index;
        s32 (*func)(); /* (SceneObject *, Camera *): whether it is out of view */
    } cull;
    /* 0x18 */ VtableEntry draw;    /* (SceneObject *, Camera *) */
    /* 0x20 */ VtableEntry destroy; /* (SceneObject *, s32 flags) */
} SceneObjectVtable;

/* Something placed in the scene: the base class of Group and Mesh. */
typedef struct SceneObject {
    /* 0x00 */ ListNode link; /* in the children of its Group */
    /* 0x08 */ Transform transform;
    /* 0x50 */ u32 hidden : 1;
    /* 0x50 */ u32 culled : 1; /* out of view or hidden, the last time it was culled */
    /* 0x54 */ SceneObjectVtable *vtable;
} SceneObject; /* size 0x58 */

/* A scene object made of others. */
typedef struct Group {
    /* 0x00 */ SceneObject object;
    /* 0x58 */ ListNode children; /* SceneObjects, by their link */
} Group; /* size 0x60 */

/* Mesh.debug: draw the bounding box */
#define MESH_SHOW_BOUNDS 1

/* An object of a TMD model, culled by its bounding sphere. */
typedef struct Mesh {
    /* 0x00 */ SceneObject object;
    /* 0x58 */ TmdObject *tmd;
    /* 0x5C */ VECTOR center; /* of its bounding box */
    /* 0x6C */ s32 radius;    /* of the sphere around it */
    /* 0x70 */ SVECTOR corners[8]; /* of its bounding box */
    /* 0xB0 */ u32 debug;
    /* 0xB4 */ s32 unkB4;
} Mesh; /* size 0xB8 */

/* A mesh moved by a node of the Animator of its Model. */
typedef struct MeshPart {
    /* 0x00 */ Mesh mesh;
    /* 0xB8 */ AnimNode node;
} MeshPart; /* size 0xF8 */

/* An animated TMD model: a group of a MeshPart per animated node. */
typedef struct Model {
    /* 0x00 */ Group group;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ MeshPart **parts; /* by the object of the TMD they draw */
    /* 0x68 */ Animator animator;
    /* 0x94 */ Lights *lights; /* used while it is drawn, unless NULL */
} Model;

extern struct SceneObjectVtable MESH_PART_VTABLE; /* of MeshPart */
extern struct AnimNodeVtable MESH_PART_ANIM_NODE_VTABLE; /* of MeshPart, as an AnimNode */
extern struct SceneObjectVtable MODEL_VTABLE; /* of Model */
extern struct SceneObjectVtable MESH_VTABLE; /* of Mesh */
extern struct SceneObjectVtable GROUP_VTABLE; /* of Group */

extern struct SceneObjectVtable SCENE_OBJECT_VTABLE; /* of SceneObject */

extern VECTOR *(*APPLY_MATRIX_FUNC)(MATRIX *m, SVECTOR *v, VECTOR *r);

/* How many meshes were culled, and how many of them were out of view, since
 * resetMeshCullCounts. */
extern s32 MESH_CULL_COUNT;
extern s32 MESH_OUT_OF_VIEW_COUNT;

Transform *transformInit(Transform *transform);
void transformAttach(Transform *transform, Transform *parent);
void transformDetach(Transform *transform);
void sceneObjectDestroy(SceneObject *sceneObject, s32 flags);
Group *groupInit(Group *group);
void groupDestroy(Group *group, s32 flags);
Group *groupAddChild(Group *group, SceneObject *child);
void groupInvalidate(Group *group);
s32 groupCull(Group *group, Camera *camera);
void sceneObjectDrawInView(SceneObject *sceneObject, Camera *camera);
void groupDraw(Group *group, Camera *camera);
void meshDraw(Mesh *mesh, Camera *camera);
s32 meshCull(Mesh *mesh, Camera *camera);
void resetMeshCullCounts(void);
s32 getMeshCullCount(void);
s32 getMeshOutOfViewCount(void);
Mesh *meshInit(Mesh *mesh, TmdObject *tmd, s32 unused);
SVECTOR *meshGetCorners(Mesh *mesh);
void meshSetTmd(Mesh *mesh, TmdObject *tmd);
void meshPartUpdate(MeshPart *meshPart);
Model *modelInit(Model *model, TmdHeader *tmd, AnimData *anim);
void modelDestroy(Model *model, s32 flags);
s32 modelGetFrameCount(Model *model);
s32 modelGetFrame(Model *model);
s32 modelGetClip(Model *model);
s32 modelIsStopped(Model *model);
void modelStepAnimation(Model *model);
void modelStartClip(Model *model, s32 clip, s32 blendFrames, s32 plays, s32 frame);
void modelSetLoopFrame(Model *model, s32 frame);
void modelDraw(Model *model, Camera *camera);
void modelSetLights(Model *model, Lights *lights);
MATRIX *transformGetWorld(Transform *transform);
void transformSetDirty(Transform *transform, s32 dirty);
MATRIX *transformEditLocal(Transform *transform);
MATRIX *transformGetLocal(Transform *transform);
void transformSetLocal(Transform *transform, MATRIX *local);
void func_8002099C(Transform *transform, MATRIX *local);
void transformSetParentKeepLocal(Transform *transform, Transform *parent);
void sceneObjectInvalidate(SceneObject *sceneObject);
SceneObject *sceneObjectInit(SceneObject *sceneObject);
void sceneObjectSetHidden(SceneObject *sceneObject, s32 hidden);
ListNode *groupGetChildren(Group *group);
void func_80020A74(Group *group, SceneObject *child);
void meshDestroy(Mesh *mesh, s32 flags);
MeshPart *modelGetPart(Model *model, s32 object);
Animator *modelGetAnimator(Model *model);
void meshPartDestroy(MeshPart *meshPart, s32 flags);

/* Sets the parent of a transform, keeping its matrix relative to the parent:
 * its world matrix goes out of date. */
static inline void transformSetParentKeepLocalInline(Transform *transform, Transform *parent) {
    transform->valid = 0;
    transform->parent = parent;
}

EXTERN_C_END

#endif /* DTBE_GFX_SCENE_GRAPH_H */
