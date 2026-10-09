#include "common.h"
#include "engine/system/handle_table.h"
#include "engine/system/memory.h"

/* Constructs a handle table with room for capacity - 1 handles: every node
 * but the two list heads starts out free. */
HandleTable *handleTableInit(HandleTable *table, s32 capacity) {
    HandleNode *usedList;
    s32 i;

    if (capacity > HANDLE_TABLE_MAX) {
        capacity = HANDLE_TABLE_MAX;
    }
    table->capacity = capacity;
    /* psyq.h declares operator new[] as returning s32 */
    table->nodes = (HandleNode *)operatorVecNew((capacity + 1) * sizeof(HandleNode));
    usedList = table->nodes + capacity;
    table->usedList = usedList;
    usedList->next = usedList;
    usedList->prev = usedList;
    for (i = 1; i < capacity; i++) {
        table->nodes[i].next = &table->nodes[i] + 1;
        table->nodes[i].prev = &table->nodes[i] - 1;
        table->nodes[i].owner = NULL;
        table->nodes[i].handle.f.index = i;
        table->nodes[i].handle.f.serial = 0;
    }
    table->nodes[1].prev = table->nodes;
    table->nodes[capacity - 1].next = table->nodes;
    table->nodes[0].next = &table->nodes[1];
    table->nodes[0].prev = table->nodes + capacity - 1;
    return table;
}

/* Destroys a handle table; flags bit 0 also frees the table itself. */
void handleTableDestroy(HandleTable *table, s32 flags) {
    HandleNode *nodes = table->nodes;

    if (nodes != NULL) {
        operatorVecDelete(nodes);
    }
    if (flags & 1) {
        operatorDelete(table);
    }
}

/* Moves a free node to the used list and returns a new handle for owner,
 * or 0 when the table is full. */
u32 handleTableAdd(HandleTable *table, void *owner) {
    HandleNode *freeList = table->nodes;
    HandleNode *node = freeList->next;

    if (node == freeList) {
        return 0;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = table->usedList->next;
    node->prev = table->usedList;
    table->usedList->next->prev = node;
    table->usedList->next = node;
    node->owner = owner;
    node->handle.f.serial++;
    return node->handle.word;
}

/* Gives back a handle: its node goes to the end of the free list. Returns
 * what the handle stood for, or NULL when it is not a live handle. */
void *handleTableRemove(HandleTable *table, u32 handle) {
    HandleNode *node;
    void *owner;
    u32 index;

    if (handle == 0) {
        return NULL;
    }
    index = handle & (HANDLE_TABLE_MAX - 1);
    if (index >= table->capacity) {
        return NULL;
    }
    node = &table->nodes[index];
    if (node->handle.word != handle) {
        return NULL;
    }
    owner = node->owner;
    node->owner = NULL;
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = table->nodes;
    node->prev = table->nodes->prev;
    table->nodes->prev->next = node;
    table->nodes->prev = node;
    return owner;
}

/* Returns what a handle stands for, or NULL when it is not a live handle. */
void *handleTableGet(HandleTable *table, u32 handle) {
    HandleNode *node;
    u32 index = handle & (HANDLE_TABLE_MAX - 1);

    if (handle == 0 || index >= table->capacity) {
        return NULL;
    }
    node = &table->nodes[index];
    if (node->handle.word != handle) {
        return NULL;
    }
    return node->owner;
}

/* Counts the nodes of one of the table's lists, without its head. */
s32 handleTableCountList(HandleTable *table, HandleNode *list) {
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}

/* Counts the handles in use. */
s32 handleTableCountUsed(HandleTable *table) {
    HandleNode *list = table->usedList;
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}

/* Counts the free handles. */
s32 handleTableCountFree(HandleTable *table) {
    HandleNode *list = table->nodes;
    HandleNode *node;
    s32 count = 0;

    for (node = list->next; node != list; node = node->next) {
        count++;
    }
    return count;
}
