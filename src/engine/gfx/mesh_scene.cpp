/* writes out the inline functions of its header */
#pragma implementation
#include "common.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/camera.h"
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
    groupDestroy(&meshScene->group, 2);
    if (flags & DESTROY_FREE) {
        operatorDelete(meshScene);
    }
}

/* func_80076A00, in an overlay, is not declared yet, and the inlined
 * transformGetWorld copies a MATRIX as a block move (see "Block moves" in
 * TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneLoad);

/* Sets each node's local matrix from its MeshScenePose (the rotation, then the
 * position). It copies a MATRIX as a block move (four loads then four stores),
 * which GCC 2.95.2 does not emit for 32 bytes (see "Block moves" in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneInitNodes);

/* Makes `parent` the parent transform of the nodes that have none. */
void meshSceneSetParent(MeshScene *meshScene, Transform *parent) {
    s16 *nodes = meshScene->file->nodes;
    s32 count = *nodes++;
    s16 *parents = nodes + count * MESH_SCENE_NODE_PARENT;
    s32 i;

    for (i = 0; i < count; i++) {
        if (parents[i] < 0) {
            Mesh *mesh = meshScene->meshes[i];

            mesh->object.transform.valid = 0;
            mesh->object.transform.parent = parent;
        }
    }
}

/* Copies a MATRIX as a block move, as meshSceneInitNodes does. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneSetLocalMatrix);

/* Prepares the TMD of a node again (tmdHeaderRelocate), and its first object. */
void meshSceneRelocateNodeTmd(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *tmdIndex = file->nodes + 1;
    TmdHeader *tmd = file->tmds[tmdIndex[node]];

    tmdHeaderRelocate(tmd);
    tmdHeaderGetObject(tmd, 0);
}

/* The number of nodes. */
s32 meshSceneGetNodeCount(MeshScene *meshScene) {
    return meshScene->nodeCount;
}

/* unk0 of the entry of a node, or 0 if it has none or is hidden. */
s32 func_80031B4C(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *nodes = file->nodes;
    s32 count = *nodes++;
    s16 *hidden = nodes + count * MESH_SCENE_NODE_HIDDEN;
    MeshSceneEntry *entry = file->entries[nodes[node]];

    if (entry == NULL || hidden[node] != 0) {
        return 0;
    }
    return entry->unk0;
}

/* &unk4 of the entry of a node, or NULL if it has none or is hidden. */
s32 *func_80031BB8(MeshScene *meshScene, s32 node) {
    MeshSceneFile *file = meshScene->file;
    s16 *nodes = file->nodes;
    s32 count = *nodes++;
    s16 *hidden = nodes + count * MESH_SCENE_NODE_HIDDEN;
    MeshSceneEntry *entry = file->entries[nodes[node]];

    if (entry == NULL || hidden[node] != 0) {
        return NULL;
    }
    return &entry->unk4;
}

/* The mesh of a node. */
Mesh *meshSceneGetMesh(MeshScene *meshScene, s32 node) {
    return meshScene->meshes[node];
}

/* The matrix of a node relative to its parent. */
MATRIX *meshSceneGetLocalMatrix(MeshScene *meshScene, s32 node) {
    return &meshScene->meshes[node]->object.transform.local.m;
}

/* transformGetWorld inlined, whose matrix copy is a block move (see "Block moves"
 * in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneGetWorldMatrix);

/* Hides a node, taking its link out of the list it is in. */
void meshSceneHideNode(MeshScene *meshScene, s32 node) {
    s16 *nodes;
    s32 count;

    meshScene->links[node].shown = 0;
    if (!listIsAlone(&meshScene->links[node].link)) {
        func_80076A5C(&meshScene->links[node].link);
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

/* a list that, in the executable, only this file's static constructor and
 * destructor touch */
List D_8006437C;
