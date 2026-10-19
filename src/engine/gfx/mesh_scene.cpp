/* writes out the inline functions of its header */
#pragma implementation
#include "common.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"
#include "overlay.h"

/* Builds an empty scene: no file and no meshes. */
MeshScene *meshSceneInit(MeshScene *meshScene) {
    s32 i;

    groupInit(&meshScene->group);
    meshScene->file = NULL;
    for (i = MESH_SCENE_MESHES - 1; i >= 0; i--) {
        meshScene->meshes[i] = NULL;
    }
    return meshScene;
}

/* Deletes the meshes and the links, unlinking each from its list, frees the
 * file, then destroys the group. */
void meshSceneDestroy(MeshScene *meshScene, s32 flags) {
    Mesh **meshes = meshScene->meshes;
    MeshSceneLink *link;
    s32 i;

    for (i = MESH_SCENE_MESHES - 1; i >= 0; i--) {
        if (*meshes != NULL) {
            SceneObject *object = &(*meshes)->object;
            object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
        }
        meshes++;
    }
    if (meshScene->links != NULL) {
        /* new[] keeps the element count two words before the array */
        link = meshScene->links + ((s32 *)meshScene->links)[-2];
        while (meshScene->links != link) {
            link--;
            link->link.next->prev = link->link.prev;
            link->link.prev->next = link->link.next;
            link->link.next = &link->link;
            link->link.prev = &link->link;
        }
        operatorVecDelete((s32 *)meshScene->links - 2);
    }
    mainHeapFree(meshScene->file);
    groupDestroy(&meshScene->group, DESTROY_BASES);
    if (flags & DESTROY_FREE) {
        operatorDelete(meshScene);
    }
}

/* Loads the scene file at path: uploads its TIMs, relocates its TMD, builds
 * a Mesh per object in the scene's group and a link per node (appended to
 * COLLISION_LINKS, which it gets from getCollisionLinks, in an overlay, when the node
 * has collision lines), then sets up the nodes (meshSceneInitNodes). Left in asm:
 * the inlined transformGetWorld copies a MATRIX as a block move (see "Block
 * moves" in TODO.md). */
INCLUDE_ASM("main/nonmatchings/gfx/mesh_scene", meshSceneLoad);

/* Sets each node's local matrix from its MeshScenePose (the rotation, then
 * the translation). It copies a MATRIX as a block move (four loads then four
 * stores), which GCC 2.95.2 does not emit for 32 bytes (see "Block moves" in
 * TODO.md). */
INCLUDE_ASM("main/nonmatchings/gfx/mesh_scene", meshSceneInitNodes);

/* Makes `parent` the parent transform of the nodes that have none. */
void meshSceneSetParent(MeshScene *meshScene, Transform *parent) {
    s16 *nodes = meshScene->file->nodes;
    s32 count = *nodes++;
    s16 *parents = nodes + count * MESH_SCENE_NODE_PARENT;
    s32 i;

    for (i = 0; i < count; i++) {
        if (parents[i] < 0) {
            transformSetParentKeepLocalInline(&meshScene->meshes[i]->object.transform, parent);
        }
    }
}

/* Sets the local matrix of a node's mesh, if it has one: its world matrix
 * goes out of date. It copies the MATRIX as a block move, as
 * meshSceneInitNodes does. */
INCLUDE_ASM("main/nonmatchings/gfx/mesh_scene", meshSceneSetLocalMatrix);

/* Prepares the TMD of a node again (tmdHeaderRelocate) and returns its first
 * object. */
TmdObject *meshSceneRelocateNodeTmd(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *tmdIndex = file->nodes + 1;
    TmdHeader *tmd = file->tmds[tmdIndex[node]];

    tmdHeaderRelocate(tmd);
    return tmdHeaderGetObject(tmd, 0);
}

/* The number of nodes. */
s32 meshSceneGetNodeCount(MeshScene *meshScene) {
    return meshScene->nodeCount;
}

/* The number of collision lines of a node, or 0 if it has none or is hidden. */
s32 meshSceneGetCollisionCount(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *nodes = file->nodes;
    s32 count = *nodes++;
    s16 *hidden = nodes + count * MESH_SCENE_NODE_HIDDEN;
    MeshSceneCollision *collision = file->collisions[nodes[node]];

    if (collision == NULL || hidden[node] != 0) {
        return 0;
    }
    return collision->lineCount;
}

/* The collision lines of a node, or NULL if it has none or is hidden. */
s32 *meshSceneGetCollisionLines(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *nodes = file->nodes;
    s32 count = *nodes++;
    s16 *hidden = nodes + count * MESH_SCENE_NODE_HIDDEN;
    MeshSceneCollision *collision = file->collisions[nodes[node]];

    if (collision == NULL || hidden[node] != 0) {
        return NULL;
    }
    return &collision->lines;
}

/* The mesh of a node. */
Mesh *meshSceneGetMesh(MeshScene *meshScene, s32 node) {
    return meshScene->meshes[node];
}

/* The matrix of a node relative to its parent. */
MATRIX *meshSceneGetLocalMatrix(MeshScene *meshScene, s32 node) {
    return &meshScene->meshes[node]->object.transform.local.m;
}

/* The world matrix of a node's mesh, worked out again if it is out of date:
 * transformGetWorld inlined, whose matrix copy is a block move (see "Block
 * moves" in TODO.md). */
INCLUDE_ASM("main/nonmatchings/gfx/mesh_scene", meshSceneGetWorldMatrix);

/* Hides a node, moving its link, if it is in COLLISION_LINKS, to the back. */
void meshSceneHideNode(MeshScene *meshScene, s32 node) {
    s16 *nodes;
    s32 count;

    meshScene->links[node].shown = 0;
    if (!listIsAlone(&meshScene->links[node].link)) {
        moveCollisionLinkToBack(&meshScene->links[node].link);
    }
    nodes = meshScene->file->nodes;
    count = *nodes++;
    nodes += count * MESH_SCENE_NODE_HIDDEN;
    nodes[node] = 1;
}

/* Shows a node that meshSceneHideNode hid. */
void meshSceneShowNode(MeshScene *meshScene, s32 node) {
    s16 *nodes;
    s32 count;

    meshScene->links[node].shown = 1;
    nodes = meshScene->file->nodes;
    count = *nodes++;
    nodes += count * MESH_SCENE_NODE_HIDDEN;
    nodes[node] = 0;
}

/* the links of the nodes that have collision lines (mesh_scene.h) */
List COLLISION_LINKS;
