#include "common.h"
#include "engine/task/entity.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/lib/heap.h"
#include "engine/lib/list.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* Destroys an entity and frees it, as `delete` does: nothing for NULL. */
static inline void entityDelete(Entity *entity) {
    if (entity != NULL) {
        entity->vtable->destroy.func((u8 *)entity + entity->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Sets up the heap the entities are allocated from and the table of their handles. */
void initEntityHeap(s32 size, void *base, HandleTable *handles) {
    heapInit(&ENTITY_HEAP, base, size);
    ENTITY_HANDLES = handles;
}

/* Constructs an entity, alone in its list, with a handle. */
Entity *entityInit(Entity *this, s32 unk10) {
    this->vtable = &ENTITY_VTABLE;
    this->unk10 = unk10;
    this->link.next = &this->link;
    this->link.prev = &this->link;
    this->handle = handleTableAdd(ENTITY_HANDLES, this);
    return this;
}

/* Destroys an entity: frees its handle and takes it out of its list. */
void entityDestroy(Entity *this, s32 flags) {
    this->vtable = &ENTITY_VTABLE;
    handleTableRemove(ENTITY_HANDLES, this->handle);
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Returns the value an entity was constructed with. */
s32 func_8002BD68(Entity *this) {
    return this->unk10;
}

/* Returns the handle of an entity. */
u32 entityGetHandle(Entity *this) {
    return this->handle;
}

/* Placement new for an entity: constructs it where it is told to. */
void *entityOperatorNewAt(s32 size, void *place) {
    return place;
}

/* Allocates memory for an entity. */
void *entityOperatorNew(s32 size) {
    return allocEntityMemory(size);
}

/* Frees the memory of an entity. */
void entityOperatorDelete(void *ptr) {
    freeEntityMemory(ptr);
}

/* Returns the entity of a handle, or NULL if it is gone. */
Entity *getEntityByHandle(u32 handle) {
    return handleTableGet(ENTITY_HANDLES, handle);
}

/* The same as entityGetGroup, which it calls. */
EntityGroup *func_8002BDF4(Entity *this) {
    return entityGetGroup(this);
}

/* Allocates from the entities' heap. */
void *allocEntityMemory(s32 size) {
    return heapAllocNext(&ENTITY_HEAP, size);
}

/* Frees into the entities' heap. */
void freeEntityMemory(void *ptr) {
    heapFree(&ENTITY_HEAP, ptr);
}

/* Deletes the entity of a handle, if it is still there. */
void destroyEntityByHandle(u32 handle) {
    entityDelete(getEntityByHandle(handle));
}

/* Returns the group of an entity, or NULL if it is in none. */
EntityGroup *entityGetGroup(Entity *this) {
    if (listIsAlone(&this->link)) {
        return NULL;
    }
    return this->group;
}

/* Gives an entity a new handle, so that the old one no longer finds it. */
void entityRenewHandle(Entity *this) {
    handleTableRemove(ENTITY_HANDLES, this->handle);
    this->handle = handleTableAdd(ENTITY_HANDLES, this);
}

/* Adds another entity to the pending list of this entity's group. */
void entityAddSibling(Entity *this, Entity *other) {
    if (other != NULL) {
        entityGroupAppend(func_8002BDF4(this), other);
    }
}

/* Constructs an empty group. */
EntityGroup *entityGroupInit(EntityGroup *this) {
    this->entities.next = &this->entities;
    this->entities.prev = &this->entities;
    this->pending.next = &this->pending;
    this->pending.prev = &this->pending;
    return this;
}

/* Destroys a group and deletes its entities. */
void entityGroupDestroy(EntityGroup *this, s32 flags) {
    ListNode *pending;

    entityGroupDestroyAll(this);
    /* listRemove(&this->pending), in the access order the game's compiler used */
    pending = &this->pending;
    this->pending.next->prev = this->pending.prev;
    pending->prev->next = this->pending.next;
    this->pending.next = pending;
    this->pending.prev = pending;
    listRemove(&this->entities);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Moves both lists into a local list, then each entity back to the end of
 * the entities, calling its gather (virtual). The game's compiler read most fields of
 * the lists at their offsets from the frame or from this, but each
 * `x->prev->next = ...` through a register holding the list's address, as
 * in entityGroupDestroy: gathered's fields are read directly here, list is
 * the address the loop keeps in a register, and head the one the closing
 * listRemove(&gathered) works through. */
void entityGroupGather(EntityGroup *this, s32 arg) {
    ListNode gathered;
    ListNode *list = &gathered;
    ListNode *head;
    ListNode *node;
    s32 alone;

    gathered.next = &gathered;
    gathered.prev = &gathered;
    /* splice the entities, then the pending ones, onto the end of gathered */
    if (!listIsAlone(&this->entities)) {
        ListNode *first = this->entities.next;
        ListNode *last = this->entities.prev;

        this->entities.next = &this->entities;
        this->entities.prev = &this->entities;
        first->prev = gathered.prev;
        last->next = &gathered;
        list->prev->next = first;
        gathered.prev = last;
    }
    /* listIsAlone(&this->pending), reading at offsets from this */
    alone = this->pending.next == &this->pending && &this->pending == this->pending.prev;
    if (!alone) {
        ListNode *first = this->pending.next;
        ListNode *last = this->pending.prev;

        this->pending.next = &this->pending;
        this->pending.prev = &this->pending;
        first->prev = gathered.prev;
        last->next = &gathered;
        list->prev->next = first;
        gathered.prev = last;
    }
    for (node = list->next; node != list; node = list->next) {
        /* link is the first member of an Entity */
        Entity *entity = (Entity *)node;

        /* back to the end of the entities */
        listRemove(node);
        node->next = &this->entities;
        node->prev = this->entities.prev;
        this->entities.prev->next = node;
        this->entities.prev = node;
        entity->vtable->gather.func((u8 *)entity + entity->vtable->gather.delta, arg);
    }
    /* listRemove(&gathered), in the access order the game's compiler used */
    head = &gathered;
    gathered.next->prev = gathered.prev;
    head->prev->next = gathered.next;
    gathered.next = head;
    gathered.prev = head;
}

/* Updates the group's entities, stopping when the frame's primitive buffer is nearly full. */
void entityGroupUpdate(EntityGroup *this, s32 arg) {
    Entity *entity;
    Entity *next;

    /* link is the first member of an Entity */
    for (entity = (Entity *)this->entities.next; &entity->link != &this->entities && !isPrimBufferNearlyFull();
         entity = next) {
        next = (Entity *)entity->link.next;
        entity->vtable->update.func((u8 *)entity + entity->vtable->update.delta, arg);
    }
}

/* Adds an entity to the end of the group's pending list. */
EntityGroup *entityGroupAppend(EntityGroup *this, Entity *entity) {
    listInsertAfter(this->pending.prev, &entity->link);
    entity->group = this;
    return this;
}

/* Deletes every entity of the group. */
void entityGroupDestroyAll(EntityGroup *this) {
    ListNode *node;
    ListNode *list = &this->entities;

    /* link is the first member of an Entity */
    for (node = list->next; node != list;) {
        ListNode *next = node->next;
        entityDelete((Entity *)node);
        node = next;
    }
    list = &this->pending;
    for (node = list->next; node != list;) {
        ListNode *next = node->next;
        entityDelete((Entity *)node);
        node = next;
    }
}

/* Returns the number of entities of the group, pending ones included. */
s32 entityGroupGetCount(EntityGroup *this) {
    ListNode *node;
    s32 count = 0;
    ListNode *list = &this->entities;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    list = &this->pending;
    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}

/* Does nothing (virtual gather of Entity). */
void entityGather(void) {
}

/* Does nothing (virtual update of Entity). */
void entityUpdate(void) {
}

/* Adds an entity to the end of the group's pending list. */
void entityGroupAdd(EntityGroup *this, Entity *entity) {
    entityGroupAppend(this, entity);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/task/entity", ENTITY_VTABLE);
