#ifndef DTBE_TASK_ENTITY_H
#define DTBE_TASK_ENTITY_H

/* The entities: objects from a heap of their own, known by handles, updated in groups. */

#include "common.h"
#include "vtable.h"
#include "engine/lib/heap.h"
#include "engine/lib/list.h"
#include "engine/system/handle_table.h"

EXTERN_C_BEGIN

struct EntityGroup;

/* the virtual table of Entity, as the C files see it */
typedef struct EntityVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry gather;  /* (Entity *, s32 arg), when its group gathers it */
    /* 0x10 */ VtableEntry update;  /* (Entity *, s32 arg) */
    /* 0x18 */ VtableEntry destroy; /* (Entity *, s32 flags) */
} EntityVtable;

/* Something kept in an EntityGroup, allocated from its own heap (ENTITY_HEAP)
 * and known by a handle in ENTITY_HANDLES. */
typedef struct Entity {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u32 handle;
    /* 0x0C */ struct EntityGroup *group;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ EntityVtable *vtable;
} Entity;

/* Two lists of entities: the ones updated every frame, and the ones added
 * since the group last gathered them into the first list. */
typedef struct EntityGroup {
    /* 0x00 */ ListNode entities;
    /* 0x08 */ ListNode pending;
} EntityGroup;

/* the heap allocEntityMemory takes from */
extern Heap ENTITY_HEAP;

/* the handles of the entities */
extern HandleTable *ENTITY_HANDLES;

/* the virtual table of Entity */
extern EntityVtable ENTITY_VTABLE;

void initEntityHeap(s32 size, void *base, HandleTable *handles);
Entity *entityInit(Entity *entity, s32 unk10);
void entityDestroy(Entity *entity, s32 flags);
s32 func_8002BD68(Entity *entity);
u32 entityGetHandle(Entity *entity);
void *entityOperatorNewAt(s32 size, void *place);
void *entityOperatorNew(s32 size);
void entityOperatorDelete(void *ptr);
Entity *getEntityByHandle(u32 handle);
EntityGroup *func_8002BDF4(Entity *entity);
void *allocEntityMemory(s32 size);
void freeEntityMemory(void *ptr);
void destroyEntityByHandle(u32 handle);
EntityGroup *entityGetGroup(Entity *entity);
void entityRenewHandle(Entity *entity);
void entityAddSibling(Entity *entity, Entity *other);
EntityGroup *entityGroupInit(EntityGroup *entityGroup);
void entityGroupDestroy(EntityGroup *entityGroup, s32 flags);
void entityGroupGather(EntityGroup *entityGroup, s32 arg);
void entityGroupUpdate(EntityGroup *entityGroup, s32 arg);
EntityGroup *entityGroupAppend(EntityGroup *entityGroup, Entity *entity);
void entityGroupDestroyAll(EntityGroup *entityGroup);
s32 entityGroupGetCount(EntityGroup *entityGroup);
void entityGather(void);
void entityUpdate(void);
void entityGroupAdd(EntityGroup *entityGroup, Entity *entity);

EXTERN_C_END

#endif /* DTBE_TASK_ENTITY_H */
