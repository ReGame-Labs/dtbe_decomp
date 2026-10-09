#ifndef DTBE_LIB_LIST_H
#define DTBE_LIB_LIST_H

/* Doubly linked lists: ones that end in NULL, sorted by merge sort, and circular ones. */

#include "common.h"

EXTERN_C_BEGIN

/* A node of a doubly linked list that ends in NULL both ways. */
typedef struct LinkNode {
    /* 0x0 */ struct LinkNode *next;
    /* 0x4 */ struct LinkNode *prev;
} LinkNode;

typedef struct {
    /* 0x0 */ LinkNode *head;
    /* 0x4 */ LinkNode *tail;
    /* 0x8 */ s32 count;
} LinkList;

/* orders two nodes: above 0 when a goes after b */
typedef s32 (*LinkCompare)(LinkNode *a, LinkNode *b);

EXTERN_C_END

/* A node of a circular doubly linked list. A list is a node of its own, its
 * head: an empty list points at itself. */
typedef struct ListNode {
    /* 0x0 */ struct ListNode *next;
    /* 0x4 */ struct ListNode *prev;
} ListNode;

EXTERN_C_BEGIN

LinkNode *linkNodeSort(LinkNode *nodes, LinkCompare compare);
void linkListSort(LinkList *list, LinkCompare compare);
LinkNode *mergeSortLinkNodes(LinkCompare compare, LinkNode *nodes);
LinkNode *mergeLinkNodes(LinkCompare compare, LinkNode *a, LinkNode *b);
void linkListInsertBefore(LinkList *list, LinkNode *before, LinkNode *node);

void linkListRemove(LinkList *list, LinkNode *node);

EXTERN_C_END

/* Takes a node out of its list, leaving it alone in a list of its own. */
static inline void listRemove(ListNode *node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
    node->next = node;
    node->prev = node;
}

/* Whether a node is alone in a list of its own: in no list, or an empty list's head. */
static inline s32 listIsAlone(ListNode *node) {
    return node->next == node && node == node->prev;
}

/* Puts a node into a list right after another node of it. */
static inline void listInsertAfter(ListNode *pos, ListNode *node) {
    node->prev = pos;
    node->next = pos->next;
    pos->next->prev = node;
    pos->next = node;
}

#ifdef __cplusplus
/* A circular list's head in g++ code, empty from its construction on. Its
 * destructor takes it out of its circle, as listRemove does; for a global
 * one, these run in g++'s static constructor and destructor. (Declared ahead
 * of listRemove, it changes how g++ inlines ~ListItem in debug/system_menu.) */
class List : public ListNode {
public:
    List() {
        next = this;
        prev = next;
    }
    ~List() {
        next->prev = prev;
        prev->next = next;
        next = this;
        prev = next;
    }
};
#endif

#endif /* DTBE_LIB_LIST_H */
