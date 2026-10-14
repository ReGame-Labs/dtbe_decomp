#ifndef DTBE_TASK_ENTITY_H
#define DTBE_TASK_ENTITY_H

/* The entities: objects from a heap of their own, known by handles, ticked and drawn in groups. */

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
    /* 0x08 */ VtableEntry tick;    /* (Entity *, s32 arg): a frame of its logic */
    /* 0x10 */ VtableEntry draw;    /* (Entity *, s32 arg): adds its primitives */
    /* 0x18 */ VtableEntry destroy; /* (Entity *, s32 flags) */
} EntityVtable;

/* Something kept in an EntityGroup, allocated from its own heap (ENTITY_HEAP)
 * and known by a handle in ENTITY_HANDLES. */
typedef struct Entity {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u32 handle;
    /* 0x0C */ struct EntityGroup *group;
    /* 0x10 */ s32 kind; /* ENTITY_KIND_*, or 0 */
    /* 0x14 */ EntityVtable *vtable;
} Entity;

/* An entity's kind, given to entityInit by the class family that constructs
 * it: 0 for the rest. */
#define ENTITY_KIND_FIGHTER 0x10000        /* the base fighter constructor's */
#define ENTITY_KIND_ITEM 0x20000           /* the battle's pickup items (Item_init) */
#define ENTITY_KIND_FIGHTER_OBJECT 0x30000 /* the objects of the fighters' own code */
#define ENTITY_KIND_STAGE_OBJECT 0x40000   /* the objects of the stages' own code */

/* Two lists of entities: the ones ticked and drawn every frame, and the ones
 * added since the group last ticked, which the next tick moves into the first. */
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
Entity *entityInit(Entity *entity, s32 kind);
void entityDestroy(Entity *entity, s32 flags);
s32 entityGetKind(Entity *entity);
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
void entityGroupTick(EntityGroup *entityGroup, s32 arg);
void entityGroupDraw(EntityGroup *entityGroup, s32 arg);
EntityGroup *entityGroupAppend(EntityGroup *entityGroup, Entity *entity);
void entityGroupDestroyAll(EntityGroup *entityGroup);
s32 entityGroupGetCount(EntityGroup *entityGroup);
void entityTick(void);
void entityDraw(void);
void entityGroupAdd(EntityGroup *entityGroup, Entity *entity);

/* Destroys an entity and frees it, as `delete` does: nothing for NULL. */
static inline void entityDelete(Entity *entity) {
    if (entity != NULL) {
        entity->vtable->destroy.func((u8 *)entity + entity->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* g++'s `new T(args)` for an entity class of `size` bytes: allocEntityMemory's memory, once the constructor call
 * `ctor(entityOperatorNewAt(size, memory), args)` has run on it when it is not NULL. The match depends on the value
 * going through the statement expression's temporary, as g++'s does through the new-expression's: a caller that
 * tests the value tests the memory again after the constructor (with an assignment to the caller's variable or two
 * ifs on memory, jump threading or cse drop that second test). */
#define NEW_ENTITY(size, ctor, ...) ({ \
    Entity *newMemory = allocEntityMemory(size); \
    newMemory != NULL ? (ctor(entityOperatorNewAt(size, newMemory), __VA_ARGS__), newMemory) : newMemory; \
})

EXTERN_C_END

#endif /* DTBE_TASK_ENTITY_H */
