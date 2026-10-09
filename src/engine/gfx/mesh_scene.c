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
MeshScene *meshSceneInit(MeshScene *this) {
    s32 i;

    groupInit(&this->group);
    this->file = NULL;
    for (i = MESH_SCENE_MESHES - 1; i >= 0; i--) {
        this->meshes[i] = NULL;
    }
    return this;
}

/* Deletes the meshes and the links, unlinking each from its list, frees the
 * file, then destroys the group. */
void meshSceneDestroy(MeshScene *this, s32 flags) {
    Mesh **meshes = this->meshes;
    MeshSceneLink *link;
    s32 i;

    for (i = MESH_SCENE_MESHES - 1; i >= 0; i--) {
        if (*meshes != NULL) {
            SceneObject *object = &(*meshes)->object;
            object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
        }
        meshes++;
    }
    if (this->links != NULL) {
        /* new[] keeps the element count two words before the array */
        link = this->links + ((s32 *)this->links)[-2];
        while (this->links != link) {
            link--;
            link->link.next->prev = link->link.prev;
            link->link.prev->next = link->link.next;
            link->link.next = &link->link;
            link->link.prev = &link->link;
        }
        operatorVecDelete((s32 *)this->links - 2);
    }
    mainHeapFree(this->file);
    groupDestroy(&this->group, 2);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
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
void meshSceneSetParent(MeshScene *this, Transform *parent) {
    s16 *nodes = this->file->nodes;
    s32 count = *nodes++;
    s16 *parents = nodes + count * MESH_SCENE_NODE_PARENT;
    s32 i;

    for (i = 0; i < count; i++) {
        if (parents[i] < 0) {
            Mesh *mesh = this->meshes[i];

            mesh->object.transform.valid = 0;
            mesh->object.transform.parent = parent;
        }
    }
}

/* Copies a MATRIX as a block move, as meshSceneInitNodes does. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneSetLocalMatrix);

/* Prepares the TMD of a node again (tmdHeaderRelocate), and its first object. */
void meshSceneRelocateNodeTmd(MeshScene *this, s32 node) {
    MeshSceneFile *file = this->file;
    s16 *tmdIndex = file->nodes + 1;
    TmdHeader *tmd = file->tmds[tmdIndex[node]];

    tmdHeaderRelocate(tmd);
    tmdHeaderGetObject(tmd, 0);
}

/* The number of nodes. */
s32 meshSceneGetNodeCount(MeshScene *this) {
    return this->nodeCount;
}

/* unk0 of the entry of a node, or 0 if it has none or is hidden. */
s32 func_80031B4C(MeshScene *this, s32 node) {
    MeshSceneFile *file = this->file;
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
s32 *func_80031BB8(MeshScene *this, s32 node) {
    MeshSceneFile *file = this->file;
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
Mesh *meshSceneGetMesh(MeshScene *this, s32 node) {
    return this->meshes[node];
}

/* The matrix of a node relative to its parent. */
MATRIX *meshSceneGetLocalMatrix(MeshScene *this, s32 node) {
    return &this->meshes[node]->object.transform.local.m;
}

/* transformGetWorld inlined, whose matrix copy is a block move (see "Block moves"
 * in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", meshSceneGetWorldMatrix);

/* Hides a node, taking its link out of the list it is in. */
void meshSceneHideNode(MeshScene *this, s32 node) {
    s16 *nodes;
    s32 count;

    this->links[node].shown = 0;
    if (!listIsAlone(&this->links[node].link)) {
        func_80076A5C(&this->links[node].link);
    }
    nodes = this->file->nodes;
    count = *nodes++;
    nodes += count * MESH_SCENE_NODE_HIDDEN;
    nodes[node] = 1;
}

/* Shows a node that meshSceneHideNode hid. */
void meshSceneShowNode(MeshScene *this, s32 node) {
    s16 *nodes;
    s32 count;

    this->links[node].shown = 1;
    nodes = this->file->nodes;
    count = *nodes++;
    nodes += count * MESH_SCENE_NODE_HIDDEN;
    nodes[node] = 0;
}

/* a list that, in the executable, only this file's static constructor and
 * destructor touch */
ListNode D_8006437C = { NULL, NULL };

/*
 * g++'s constructor and destructor of the list D_8006437C
 * (__static_initialization_and_destruction_0). The 16-byte frame comes from
 * the inlined deleting destructor of the list's owner, whose dead call to
 * operator delete C has no faithful equivalent for.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/mesh_scene", func_80031E04);

/* Whether the node of the link is hidden. */
s32 meshSceneLinkIsHidden(MeshSceneLink *this) {
    return this->shown ^ 1;
}

s32 *func_80031E68(MeshSceneLink *this) {
    return this->unk8;
}

/* The world matrix of the node of the link. */
MATRIX *meshSceneLinkGetWorldMatrix(MeshSceneLink *this) {
    return this->world;
}

s32 func_80031E80(MeshSceneLink *this) {
    return this->unk10;
}

/* Draws the meshes that are in view. */
void meshSceneDraw(MeshScene *this, Camera *camera) {
    sceneObjectDrawInView(&this->group.object, camera);
}

/* The same as meshSceneDraw. */
void func_80031EAC(MeshScene *this, Camera *camera) {
    sceneObjectDrawInView(&this->group.object, camera);
}

/* Marks the world matrices of the meshes out of date. */
void meshSceneInvalidate(MeshScene *this) {
    groupInvalidate(&this->group);
}

/* The links of the nodes, by node. */
MeshSceneLink *meshSceneGetLinks(MeshScene *this) {
    return this->links;
}

/* Adds a scene object to the group of the meshes. */
void meshSceneAddChild(MeshScene *this, SceneObject *child) {
    groupAddChild(&this->group, child);
}

/* Takes a node out of its list. */
void meshSceneRemoveFromList(MeshScene *this, ListNode *node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = node;
    node->prev = node;
}

/* The same as meshSceneGetMesh. */
Mesh *func_80031F44(MeshScene *this, s32 node) {
    return this->meshes[node];
}

/* Builds the list of func_80031E04. */
void func_80031F58(void) {
    func_80031E04(1, 0xFFFF);
}

/* Takes down the list of func_80031E04. */
void func_80031F7C(void) {
    func_80031E04(0, 0xFFFF);
}
