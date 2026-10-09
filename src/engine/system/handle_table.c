#include "common.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* Constructs a handle table with room for capacity - 1 handles: every node
 * but the two list heads starts out free. */
HandleTable *handleTableInit(HandleTable *this, s32 capacity) {
    HandleNode *usedList;
    s32 i;

    if (capacity > HANDLE_TABLE_MAX) {
        capacity = HANDLE_TABLE_MAX;
    }
    this->capacity = capacity;
    this->nodes = operatorVecNew((capacity + 1) * sizeof(HandleNode));
    usedList = this->nodes + capacity;
    this->usedList = usedList;
    usedList->next = usedList;
    usedList->prev = usedList;
    for (i = 1; i < capacity; i++) {
        this->nodes[i].next = &this->nodes[i] + 1;
        this->nodes[i].prev = &this->nodes[i] - 1;
        this->nodes[i].owner = NULL;
        this->nodes[i].handle.f.index = i;
        this->nodes[i].handle.f.serial = 0;
    }
    this->nodes[1].prev = this->nodes;
    this->nodes[capacity - 1].next = this->nodes;
    this->nodes[0].next = &this->nodes[1];
    this->nodes[0].prev = this->nodes + capacity - 1;
    return this;
}

/* Destroys a handle table; DESTROY_FREE in flags also frees the table itself. */
void handleTableDestroy(HandleTable *this, s32 flags) {
    HandleNode *nodes = this->nodes;

    if (nodes != NULL) {
        operatorVecDelete(nodes);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Moves a free node to the used list and returns a new handle for owner,
 * or 0 when the table is full. */
u32 handleTableAdd(HandleTable *this, void *owner) {
    HandleNode *freeList = this->nodes;
    HandleNode *node = freeList->next;

    if (node == freeList) {
        return 0;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = this->usedList->next;
    node->prev = this->usedList;
    this->usedList->next->prev = node;
    this->usedList->next = node;
    node->owner = owner;
    node->handle.f.serial++;
    return node->handle.word;
}

/* Gives back a handle: its node goes to the end of the free list. Returns
 * what the handle stood for, or NULL when it is not a live handle. */
void *handleTableRemove(HandleTable *this, u32 handle) {
    HandleNode *node;
    void *owner;
    u32 index;

    if (handle == 0) {
        return NULL;
    }
    index = handle & (HANDLE_TABLE_MAX - 1);
    if (index >= this->capacity) {
        return NULL;
    }
    node = &this->nodes[index];
    if (node->handle.word != handle) {
        return NULL;
    }
    owner = node->owner;
    node->owner = NULL;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = this->nodes;
    node->prev = this->nodes->prev;
    this->nodes->prev->next = node;
    this->nodes->prev = node;
    return owner;
}

/* Returns what a handle stands for, or NULL when it is not a live handle. */
void *handleTableGet(HandleTable *this, u32 handle) {
    HandleNode *node;
    u32 index = handle & (HANDLE_TABLE_MAX - 1);

    if (handle == 0 || index >= this->capacity) {
        return NULL;
    }
    node = &this->nodes[index];
    if (node->handle.word != handle) {
        return NULL;
    }
    return node->owner;
}

/* Counts the nodes of one of the table's lists, without its head. */
s32 handleTableCountList(HandleTable *this, HandleNode *list) {
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}

/* Counts the handles in use. */
s32 handleTableCountUsed(HandleTable *this) {
    HandleNode *list = this->usedList;
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}

/* Counts the free handles. */
s32 handleTableCountFree(HandleTable *this) {
    HandleNode *list = this->nodes;
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}
