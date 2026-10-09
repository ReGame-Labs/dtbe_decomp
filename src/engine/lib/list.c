#include "common.h"
#include "engine/lib/list.h"

/* Sorts a chain of nodes by their next links; the prev links are left as
 * they were. Returns the new first node. */
LinkNode *linkNodeSort(LinkNode *nodes, LinkCompare compare) {
    return mergeSortLinkNodes(compare, nodes);
}

/* Sorts a list, then rebuilds its prev links and its tail. */
void linkListSort(LinkList *this, LinkCompare compare) {
    LinkNode *node;
    LinkNode *prev;

    node = mergeSortLinkNodes(compare, this->head);
    prev = NULL;
    this->head = node;
    while (node != NULL) {
        node->prev = prev;
        prev = node;
        node = node->next;
    }
    this->tail = prev;
}

/* Merge sort of a chain of nodes by their next links: returns the new
 * first node. */
LinkNode *mergeSortLinkNodes(LinkCompare compare, LinkNode *nodes) {
    LinkNode *middle;
    LinkNode *second;

    if (nodes == NULL) {
        return NULL;
    }
    if (nodes->next == NULL) {
        return nodes;
    }
    /* second runs two nodes for each one of middle's, which ends up at the
     * end of the first half; then second becomes the second half */
    middle = nodes;
    second = nodes->next->next;
    while (second != NULL) {
        second = second->next;
        middle = middle->next;
        if (second != NULL) {
            second = second->next;
        }
    }
    second = middle->next;
    middle->next = NULL;
    return mergeLinkNodes(compare, mergeSortLinkNodes(compare, nodes), mergeSortLinkNodes(compare, second));
}

/* Merges two sorted chains into one; on a tie the node of a goes first. */
LinkNode *mergeLinkNodes(LinkCompare compare, LinkNode *a, LinkNode *b) {
    LinkNode head;
    LinkNode *tail = &head;

    while (a != NULL && b != NULL) {
        if (compare(a, b) <= 0) {
            tail->next = a;
            tail = a;
            a = a->next;
        } else {
            tail->next = b;
            tail = b;
            b = b->next;
        }
    }
    if (a == NULL) {
        tail->next = b;
    } else {
        tail->next = a;
    }
    return head.next;
}

/* Inserts node into the list before the node before; into an empty list, as
 * its only node. */
void linkListInsertBefore(LinkList *this, LinkNode *before, LinkNode *node) {
    LinkNode *prev;

    this->count++;
    if (this->head == NULL) {
        node->next = NULL;
        node->prev = NULL;
        this->head = node;
        this->tail = node;
        return;
    }
    node->next = before;
    node->prev = before->prev;
    prev = before->prev;
    if (prev == NULL) {
        this->head = node;
    } else {
        prev->next = node;
    }
    before->prev = node;
}

/* Takes node out of the list. */
void linkListRemove(LinkList *this, LinkNode *node) {
    LinkNode *prev = node->prev;
    LinkNode *next = node->next;

    if (--this->count < 0) {
        /* crash on purpose: the list lost count */
        *(s32 *)1 = 1;
    }
    if (prev == NULL) {
        this->head = next;
        if (next == NULL) {
            this->tail = NULL;
            return;
        }
        next->prev = NULL;
        return;
    }
    if (next == NULL) {
        this->tail = prev;
        prev->next = NULL;
        return;
    }
    next->prev = prev;
    prev->next = next;
}
