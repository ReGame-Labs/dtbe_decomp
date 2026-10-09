#ifndef DTBE_GFX_MESH_SCENE_H
#define DTBE_GFX_MESH_SCENE_H

/* The meshes of a file's TMD models, placed in a hierarchy of nodes that can be hidden. */

#include "common.h"
#include "engine/gfx/animator.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/math/quaternion.h"

EXTERN_C_BEGIN

/* the most TMDs, and entries, that a MeshSceneFile holds */
#define MESH_SCENE_FILE_SLOTS 32
/* the most meshes a MeshScene holds */
#define MESH_SCENE_MESHES 40

/* An entry of the second table of a MeshSceneFile. */
typedef struct MeshSceneEntry {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
} MeshSceneEntry;

/*
 * The file a MeshScene is loaded from. Its pointers are offsets from the start
 * of the file until meshSceneLoad relocates them.
 *
 * nodes points to the node count n, then to n-entry s16 arrays, in the order
 * of MESH_SCENE_NODE_*, then to n MeshScenePoses.
 */
typedef struct MeshSceneFile {
    /* 0x000 */ s32 unk0;
    /* 0x004 */ s32 unk4;
    /* 0x008 */ TmdHeader *tmds[MESH_SCENE_FILE_SLOTS];
    /* 0x088 */ MeshSceneEntry *entries[MESH_SCENE_FILE_SLOTS];
    /* 0x108 */ s16 *nodes;
    /* 0x10C */ u32 *tims;
} MeshSceneFile;

/* the s16 arrays that follow the node count: */
#define MESH_SCENE_NODE_TMD 0    /* the TMD, and entry, of each node */
#define MESH_SCENE_NODE_PARENT 1 /* the parent node, or -1 */
#define MESH_SCENE_NODE_UNK 2    /* not read in the executable */
#define MESH_SCENE_NODE_HIDDEN 3 /* hidden by meshSceneHideNode */
#define MESH_SCENE_NODE_ARRAYS 4

/* Where a node is posed relative to its parent. */
typedef struct MeshScenePose {
    /* 0x0 */ Quaternion rot;
    /* 0x8 */ Vec3s pos;
    /* 0xE */ s16 pad;
} MeshScenePose;

/* What a MeshScene keeps of a node, in a list of func_80076A00's. */
typedef struct MeshSceneLink {
    /* 0x00 */ ListNode link;
    /* 0x08 */ s32 *unk8;    /* &entry->unk4 (func_80031BB8) */
    /* 0x0C */ MATRIX *world; /* of the node's mesh */
    /* 0x10 */ s32 unk10;    /* entry->unk0 (func_80031B4C) */
    /* 0x14 */ s32 shown; /* not hidden by meshSceneHideNode */
} MeshSceneLink; /* size 0x18 */

/* The meshes of the TMDs of a MeshSceneFile, placed in a hierarchy. */
typedef struct MeshScene {
    /* 0x00 */ MeshSceneFile *file;
    /* 0x04 */ s32 nodeCount;
    /* 0x08 */ Mesh *meshes[MESH_SCENE_MESHES]; /* by node */
    /* 0xA8 */ MeshSceneLink *links;            /* by node, from new[] */
    /* 0xAC */ Group group;                     /* of the meshes */
} MeshScene;

/* a list built and taken down by func_80031E04 */
extern ListNode D_8006437C;

MeshScene *meshSceneInit(MeshScene *meshScene);
void meshSceneDestroy(MeshScene *meshScene, s32 flags);

void meshSceneLoad(MeshScene *meshScene, char *path);
void meshSceneInitNodes(MeshScene *meshScene);
void meshSceneSetParent(MeshScene *meshScene, Transform *parent);
void meshSceneSetLocalMatrix(MeshScene *meshScene, MATRIX *local, s32 node);
void meshSceneRelocateNodeTmd(MeshScene *meshScene, s32 node);
s32 meshSceneGetNodeCount(MeshScene *meshScene);
s32 func_80031B4C(MeshScene *meshScene, s32 node);
s32 *func_80031BB8(MeshScene *meshScene, s32 node);

Mesh *meshSceneGetMesh(MeshScene *meshScene, s32 node);
MATRIX *meshSceneGetLocalMatrix(MeshScene *meshScene, s32 node);
MATRIX *meshSceneGetWorldMatrix(MeshScene *meshScene, s32 node);
void meshSceneHideNode(MeshScene *meshScene, s32 node);
void meshSceneShowNode(MeshScene *meshScene, s32 node);
void func_80031E04(s32 initialize, s32 priority);
s32 meshSceneLinkIsHidden(MeshSceneLink *meshSceneLink);
s32 *func_80031E68(MeshSceneLink *meshSceneLink);
MATRIX *meshSceneLinkGetWorldMatrix(MeshSceneLink *meshSceneLink);
s32 func_80031E80(MeshSceneLink *meshSceneLink);
void meshSceneDraw(MeshScene *meshScene, Camera *camera);
void func_80031EAC(MeshScene *meshScene, Camera *camera);
void meshSceneInvalidate(MeshScene *meshScene);
MeshSceneLink *meshSceneGetLinks(MeshScene *meshScene);
void meshSceneAddChild(MeshScene *meshScene, SceneObject *child);
void meshSceneRemoveFromList(MeshScene *meshScene, ListNode *node);
Mesh *func_80031F44(MeshScene *meshScene, s32 node);
void func_80031F58(void);
void func_80031F7C(void);

EXTERN_C_END

#endif /* DTBE_GFX_MESH_SCENE_H */
