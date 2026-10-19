#ifndef DTBE_GFX_MESH_SCENE_H
#define DTBE_GFX_MESH_SCENE_H

/* The meshes of a file's TMD models, placed in a hierarchy of nodes that can be hidden. */

#ifdef __cplusplus
/* The inline functions below are written out once, in mesh_scene.cpp, at
 * the end of its code (after its static constructor). */
#pragma interface
#endif

#include "common.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/math/quaternion.h"
#include "engine/math/vector.h"

EXTERN_C_BEGIN

/* the most TMDs, and collision tables, that a MeshSceneFile holds */
#define MESH_SCENE_FILE_SLOTS 32
/* the most meshes a MeshScene holds */
#define MESH_SCENE_MESHES 40

/* The collision lines of a node: lineCount lines of 0x10 bytes from lines on,
 * which game_0c tests segments against. */
typedef struct MeshSceneCollision {
    /* 0x0 */ s32 lineCount;
    /* 0x4 */ s32 lines; /* the first word of the first line */
} MeshSceneCollision;

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
    /* 0x088 */ MeshSceneCollision *collisions[MESH_SCENE_FILE_SLOTS];
    /* 0x108 */ s16 *nodes;
    /* 0x10C */ u32 *tims;
} MeshSceneFile;

/* the s16 arrays that follow the node count: */
#define MESH_SCENE_NODE_TMD 0    /* the TMD, and collision lines, of each node */
#define MESH_SCENE_NODE_PARENT 1 /* the parent node, or -1 */
#define MESH_SCENE_NODE_UNK 2    /* not read in the executable */
#define MESH_SCENE_NODE_HIDDEN 3 /* hidden by meshSceneHideNode */
#define MESH_SCENE_NODE_ARRAYS 4

/* Where a node is posed relative to its parent. */
typedef struct MeshScenePose {
    /* 0x0 */ Quaternion rotation;
    /* 0x8 */ Vec3s translation;
    /* 0xE */ s16 pad;
} MeshScenePose;

/* What a MeshScene keeps of a node: in COLLISION_LINKS when the node has collision
 * lines. */
typedef struct MeshSceneLink {
    /* 0x00 */ ListNode link;
    /* 0x08 */ s32 *lines;    /* meshSceneGetCollisionLines */
    /* 0x0C */ MATRIX *world; /* of the node's mesh */
    /* 0x10 */ s32 lineCount; /* meshSceneGetCollisionCount */
    /* 0x14 */ s32 shown;     /* not hidden by meshSceneHideNode */
} MeshSceneLink; /* size 0x18 */

/* The meshes of the TMDs of a MeshSceneFile, placed in a hierarchy. */
typedef struct MeshScene {
    /* 0x00 */ MeshSceneFile *file;
    /* 0x04 */ s32 nodeCount;
    /* 0x08 */ Mesh *meshes[MESH_SCENE_MESHES]; /* by node */
    /* 0xA8 */ MeshSceneLink *links;            /* by node, from new[] */
    /* 0xAC */ Group group;                     /* of the meshes */
} MeshScene;

/* the links of the nodes that have collision lines: meshSceneLoad appends them,
 * the hidden ones go to the back, game_0c tests segments against the shown
 * ones; mesh_scene's static constructor and destructor set it up and take it
 * down */
#ifdef __cplusplus
extern List COLLISION_LINKS;
#else
extern ListNode COLLISION_LINKS;
#endif

MeshScene *meshSceneInit(MeshScene *meshScene);
void meshSceneDestroy(MeshScene *meshScene, s32 flags);

void meshSceneLoad(MeshScene *meshScene, char *path);
void meshSceneInitNodes(MeshScene *meshScene);
void meshSceneSetParent(MeshScene *meshScene, Transform *parent);
void meshSceneSetLocalMatrix(MeshScene *meshScene, MATRIX *local, s32 node);
TmdObject *meshSceneRelocateNodeTmd(MeshScene *meshScene, s32 node);
s32 meshSceneGetNodeCount(MeshScene *meshScene);
s32 meshSceneGetCollisionCount(MeshScene *meshScene, s32 node);
s32 *meshSceneGetCollisionLines(MeshScene *meshScene, s32 node);

Mesh *meshSceneGetMesh(MeshScene *meshScene, s32 node);
MATRIX *meshSceneGetLocalMatrix(MeshScene *meshScene, s32 node);
MATRIX *meshSceneGetWorldMatrix(MeshScene *meshScene, s32 node);
void meshSceneHideNode(MeshScene *meshScene, s32 node);
void meshSceneShowNode(MeshScene *meshScene, s32 node);

#ifdef __cplusplus

/* Whether the node of the link is hidden. */
inline s32 meshSceneLinkIsHidden(MeshSceneLink *meshSceneLink) {
    return meshSceneLink->shown ^ 1;
}

/* The collision lines of the node of the link. */
inline s32 *meshSceneLinkGetCollisionLines(MeshSceneLink *meshSceneLink) {
    return meshSceneLink->lines;
}

/* The world matrix of the node of the link. */
inline MATRIX *meshSceneLinkGetWorldMatrix(MeshSceneLink *meshSceneLink) {
    return meshSceneLink->world;
}

/* The number of collision lines of the node of the link. */
inline s32 meshSceneLinkGetCollisionCount(MeshSceneLink *meshSceneLink) {
    return meshSceneLink->lineCount;
}

/* Draws the meshes that are in view. */
inline void meshSceneDraw(MeshScene *meshScene, Camera *camera) {
    sceneObjectDrawInView(&meshScene->group.object, camera);
}

/* The same as meshSceneDraw. */
inline void func_80031EAC(MeshScene *meshScene, Camera *camera) {
    sceneObjectDrawInView(&meshScene->group.object, camera);
}

/* Marks the world matrices of the meshes out of date. */
inline void meshSceneInvalidate(MeshScene *meshScene) {
    groupInvalidate(&meshScene->group);
}

/* The links of the nodes, by node. */
inline MeshSceneLink *meshSceneGetLinks(MeshScene *meshScene) {
    return meshScene->links;
}

/* Adds a scene object to the group of the meshes. */
inline void meshSceneAddChild(MeshScene *meshScene, SceneObject *child) {
    groupAddChild(&meshScene->group, child);
}

/* Takes a node out of its list. */
inline void meshSceneRemoveFromList(MeshScene *meshScene, ListNode *node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = node;
    node->prev = node;
}

/* The same as meshSceneGetMesh. */
inline Mesh *func_80031F44(MeshScene *meshScene, s32 node) {
    return meshScene->meshes[node];
}

#else

s32 meshSceneLinkIsHidden(MeshSceneLink *meshSceneLink);
s32 *meshSceneLinkGetCollisionLines(MeshSceneLink *meshSceneLink);
MATRIX *meshSceneLinkGetWorldMatrix(MeshSceneLink *meshSceneLink);
s32 meshSceneLinkGetCollisionCount(MeshSceneLink *meshSceneLink);
void meshSceneDraw(MeshScene *meshScene, Camera *camera);
void func_80031EAC(MeshScene *meshScene, Camera *camera);
void meshSceneInvalidate(MeshScene *meshScene);
MeshSceneLink *meshSceneGetLinks(MeshScene *meshScene);
void meshSceneAddChild(MeshScene *meshScene, SceneObject *child);
void meshSceneRemoveFromList(MeshScene *meshScene, ListNode *node);
Mesh *func_80031F44(MeshScene *meshScene, s32 node);

#endif

EXTERN_C_END

#endif /* DTBE_GFX_MESH_SCENE_H */
