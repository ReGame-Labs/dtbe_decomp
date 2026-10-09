#include "common.h"
#include "engine/menu/stepper_group.h"
#include "engine/lib/list.h"
#include "engine/menu/stepper.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* Destroys a stepper and frees it, as `delete` does: nothing for NULL. */
static inline void stepperDelete(Stepper *stepper) {
    if (stepper != NULL) {
        stepper->vtable->destroy.func((u8 *)stepper + stepper->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/*
 * The game read the group's flags through inline accessors, which keeps the
 * compiler from merging two flag tests into one.
 */
static inline s32 isChildActive(StepperGroup *group) {
    return group->childActive;
}

static inline s32 wasChildActive(StepperGroup *group) {
    return group->childWasActive;
}

static inline s32 isCurrentOnly(StepperGroup *group) {
    return group->currentOnly;
}

static inline s32 stepperGroupIsClosedInline(StepperGroup *group) {
    return group->closed;
}

static inline s32 isKeptOpen(StepperGroup *group) {
    return group->keepOpen;
}

/* Opens the group again and refreshes it (virtual refresh). */
void stepperGroupOpen(StepperGroup *this, s32 arg) {
    Stepper *base;

    this->closed = 0;
    base = this->base;
    base->vtable->refresh.func((u8 *)base + base->vtable->refresh.delta, arg);
}

/* Passes the buttons to the group's input (virtual), unless it is closed. */
s32 stepperGroupInputIfOpen(StepperGroup *this, s32 arg1, s32 arg2, u32 buttons) {
    Stepper *base;

    if (stepperGroupIsClosed(this)) {
        return 0;
    }
    base = this->base;
    return base->vtable->input.func((u8 *)base + base->vtable->input.delta, arg1, arg2, buttons);
}

/* Draws the group (virtual) as the current one, unless it is closed. */
void stepperGroupDrawIfOpen(StepperGroup *this, s32 arg) {
    Stepper *base;

    if (!stepperGroupIsClosed(this)) {
        base = this->base;
        base->vtable->draw.func((u8 *)base + base->vtable->draw.delta, arg, 1, !this->childActive);
    }
}

/* Adds a child at the end of the group; the first child becomes the current one. */
StepperGroup *stepperGroupAddChild(StepperGroup *this, Stepper *child) {
    listInsertAfter(this->children.prev, &child->link);
    if (&this->children == &this->current->link) {
        this->current = child;
    }
    child->group = this;
    stepperGroupNumberChildren(this);
    return this;
}

/* Deletes every child of the group. */
void stepperGroupRemoveChildren(StepperGroup *this) {
    ListNode *node;
    ListNode *list = &this->children;

    /* link is the first member of a Stepper */
    for (node = list->next; node != list;) {
        ListNode *next = node->next;
        stepperDelete((Stepper *)node);
        node = next;
    }
    /* the list head stands for no current child */
    this->current = (Stepper *)&this->children;
}

/* Returns -1 while a child takes the buttons or did so this frame, else 0 to close the group (virtual cancel). */
s32 stepperGroupCancel(StepperGroup *this) {
    if (isChildActive(this) || wasChildActive(this)) {
        return -1;
    }
    return 0;
}

/*
 * Reacts to the buttons pressed: passes them to the current child while it is
 * active, else closes, confirms, moves between the children or steps the
 * current one (virtual input). Returns 1, or what cancelling returns.
 */
s32 stepperGroupInput(StepperGroup *this, s32 arg1, s32 arg2, u32 buttons) {
    Stepper *current = this->current;
    const StepperButtons *map;

    if (&this->children == &current->link) {
        return 1;
    }
    this->childWasActive = this->childActive;
    if (this->childActive) {
        stepperGroupInputCurrent(this, arg1, arg2, buttons);
        return 1;
    }
    map = this->buttons;
    if (buttons & map->cancel) {
        return stepperGroupPressCancel(this, arg1, arg2);
    }
    if ((buttons & map->confirm) || this->pendingIndex >= 0) {
        stepperGroupPressConfirm(this, arg1, arg2);
        return 1;
    }
    if (buttons & map->prev) {
        stepperGroupMoveCurrent(this, arg1, 0);
        return 1;
    }
    if (buttons & map->next) {
        stepperGroupMoveCurrent(this, arg1, 1);
        return 1;
    }
    if (buttons & map->stepUp) {
        current->vtable->stepUp.func((u8 *)current + current->vtable->stepUp.delta, arg1, arg2, buttons);
        return 1;
    }
    if (buttons & map->stepDown) {
        current->vtable->stepDown.func((u8 *)current + current->vtable->stepDown.delta, arg1, arg2, buttons);
        return 1;
    }
    /* no button it reacts to */
    return 1;
}

/* Passes the buttons to the current child, which keeps them while its input returns 1. */
void stepperGroupInputCurrent(StepperGroup *this, s32 arg1, s32 arg2, u32 buttons) {
    Stepper *current = this->current;

    this->childActive = current->vtable->input.func((u8 *)current + current->vtable->input.delta, arg1, arg2, buttons) == 1;
    if (this->childActive) {
        if (buttons & this->buttons->cancel) {
            current = this->current;
            this->childActive = current->vtable->cancel.func((u8 *)current + current->vtable->cancel.delta, arg1, arg2, 1) != 0;
        } else if (buttons & this->buttons->confirm) {
            current = this->current;
            current->vtable->confirm.func((u8 *)current + current->vtable->confirm.delta, arg1, arg2, 1);
        }
    }
    if (this->currentOnly) {
        this->currentOnly = this->childActive;
    }
}

/* The cancel button: tells the current child, then the group; closes the group if it returns 0. */
s32 stepperGroupPressCancel(StepperGroup *this, s32 arg1, s32 arg2) {
    Stepper *current = this->current;
    Stepper *base;

    current->vtable->cancel.func((u8 *)current + current->vtable->cancel.delta, arg1, arg2, 0);
    base = this->base;
    if (base->vtable->cancel.func((u8 *)base + base->vtable->cancel.delta, arg1, arg2, 1) == 0) {
        this->closed = 1;
        return 0;
    }
    return 1;
}

/*
 * The confirm button: first makes the pending child current, if one was set;
 * then confirms the current child, which takes the buttons if that returns 0;
 * else runs the group's unk10 (virtual).
 */
void stepperGroupPressConfirm(StepperGroup *this, s32 arg1, s32 arg2) {
    Stepper *child;
    Stepper *current;

    if (this->pendingIndex >= 0) {
        child = stepperGroupGetChild(this, this->pendingIndex);
        if (child != NULL) {
            this->current = child;
        }
        this->pendingIndex = -1;
    }
    current = this->current;
    if (current->vtable->confirm.func((u8 *)current + current->vtable->confirm.delta, arg1, arg2, 0) == 0) {
        current = this->current;
        this->childActive = current->vtable->input.func((u8 *)current + current->vtable->input.delta, arg1, arg2, 0) == 1;
    } else {
        this->vtable->unk10.func((u8 *)this + this->vtable->unk10.delta);
    }
}

/* Makes the next child current, or the previous one; past the end runs the group's unk8 (virtual) instead. */
void stepperGroupMoveCurrent(StepperGroup *this, s32 arg, s32 forward) {
    ListNode *node = &this->current->link;
    ListNode *list = &this->children;
    Stepper *current;
    Stepper *next;

    node = forward ? node->next : node->prev;
    /* past the end, a group that wraps steps on over the list head */
    if (node == list && (!this->wraps || (node = forward ? node->next : node->prev) == list)) {
        this->vtable->unk8.func((u8 *)this + this->vtable->unk8.delta, arg);
    } else {
        current = this->current;
        current->vtable->blur.func((u8 *)current + current->vtable->blur.delta, arg);
        /* link is the first member of a Stepper */
        next = (Stepper *)node;
        next->vtable->focus.func((u8 *)next + next->vtable->focus.delta, arg);
        this->current = next;
    }
}

/* Draws the children, or the active current one alone if currentOnly (virtual draw). */
void stepperGroupDraw(StepperGroup *this, s32 arg, s32 isCurrent, s32 active) {
    Stepper *child;
    ListNode *list;
    ListNode *node;
    s32 childIsCurrent;

    if (isChildActive(this) && isCurrentOnly(this)) {
        child = this->current;
        child->vtable->draw.func((u8 *)child + child->vtable->draw.delta, arg, 1, 1);
        return;
    }
    list = &this->children;
    for (node = list->next; node != list; node = node->next) {
        /* link is the first member of a Stepper */
        child = (Stepper *)node;
        childIsCurrent = child == this->current;
        child->vtable->draw.func((u8 *)child + child->vtable->draw.delta, arg, childIsCurrent,
                                  childIsCurrent && isChildActive(this));
    }
}

/* Refreshes every child (virtual refresh). */
void stepperGroupRefresh(StepperGroup *this, s32 arg) {
    ListNode *list = &this->children;
    ListNode *node;
    Stepper *child;

    for (node = list->next; node != list; node = node->next) {
        /* link is the first member of a Stepper */
        child = (Stepper *)node;
        child->vtable->refresh.func((u8 *)child + child->vtable->refresh.delta, arg);
    }
}

/* Deletes a child of the group; returns -1 if it is not one. */
s32 stepperGroupRemoveChild(StepperGroup *this, Stepper *child) {
    ListNode *list = &this->children;
    ListNode *node;

    if (this->current == child) {
        /* the next child becomes current, or the previous one after the last */
        node = child->link.next;
        if (node == list) {
            node = child->link.prev;
        }
        /* link is the first member of a Stepper */
        this->current = (Stepper *)node;
        stepperDelete(child);
        stepperGroupNumberChildren(this);
        return 0;
    }
    for (node = list->next; node != list; node = node->next) {
        if (node == &child->link) {
            stepperDelete((Stepper *)node);
            stepperGroupNumberChildren(this);
            return 0;
        }
    }
    return -1;
}

/* Returns the number of children of the group. */
s32 stepperGroupCountChildren(StepperGroup *this) {
    ListNode *node;
    s32 count = 0;

    for (node = this->children.next; node != &this->children; node = node->next) {
        count++;
    }
    return count;
}

/* Numbers the children of the group in order, from 0. */
void stepperGroupNumberChildren(StepperGroup *this) {
    ListNode *node;
    s32 index = 0;

    for (node = this->children.next; node != &this->children; node = node->next) {
        /* link is the first member of a Stepper */
        ((Stepper *)node)->index = index;
        index++;
    }
}

/* Returns the child of an index, or NULL if there is none. */
Stepper *stepperGroupGetChild(StepperGroup *this, s32 index) {
    ListNode *node;

    for (node = this->children.next; node != &this->children; node = node->next) {
        /* index counts down to the child */
        if (--index == -1) {
            /* link is the first member of a Stepper */
            return (Stepper *)node;
        }
    }
    return NULL;
}

/* Sets the index of the child that the confirm button makes current. */
void stepperGroupSetPendingIndex(StepperGroup *this, s32 index) {
    this->pendingIndex = index;
}

/* Keeps the group from counting as closed if arg is 1, else stops keeping it. */
void stepperGroupSetKeepOpen(StepperGroup *this, s32 arg) {
    this->keepOpen = arg == 1;
}

/* Returns whether the current child takes the buttons. */
s32 stepperGroupIsChildActive(StepperGroup *this) {
    return this->childActive;
}

/* Returns whether the group is closed, unless it is kept open. */
s32 stepperGroupIsClosed(StepperGroup *this) {
    return stepperGroupIsClosedInline(this) && !isKeptOpen(this);
}

/* Returns the group's unk1C. */
s32 func_8002D2C4(StepperGroup *this) {
    return this->unk1C;
}

/* Returns -1: a plain stepper does not take the buttons (virtual confirm of Stepper). */
s32 stepperConfirm(void) {
    return -1;
}

/* Returns 0: cancel closes the group (virtual cancel of Stepper). */
s32 stepperCancel(void) {
    return 0;
}

/* Does nothing (virtual focus of Stepper). */
void stepperFocus(void) {
}

/* Does nothing (virtual blur of Stepper). */
void stepperBlur(void) {
}

/* Returns 1: a plain stepper stays active (virtual input of Stepper). */
s32 stepperInput(void) {
    return 1;
}

/* Draws nothing (virtual draw of Stepper). */
void stepperDraw(void) {
}

/* Does nothing (virtual refresh of Stepper). */
void stepperRefresh(void) {
}

/* Destroys a stepper: takes it out of its list. */
void stepperDestroy(Stepper *this, s32 flags) {
    this->vtable = &stepperVtable;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Returns the value of a stepper. */
s32 stepperGetValue(Stepper *this) {
    return this->value;
}

/* Sets the value of a stepper. */
void stepperSetValue(Stepper *this, s32 value) {
    this->value = value;
}

/* Sets the minimum of a stepper. */
void stepperSetMin(Stepper *this, s32 min) {
    this->min = min;
}

/* Sets the maximum of a stepper. */
void stepperSetMax(Stepper *this, s32 max) {
    this->max = max;
}

/* Returns the index of a stepper in its group. */
s32 stepperGetIndex(Stepper *this) {
    return this->index;
}

/* Does nothing (virtual unk8 of StepperGroup). */
void func_8002D390(void) {
}

/* Does nothing (virtual unk10 of StepperGroup). */
void func_8002D398(void) {
}

/* Adds a child at the end of the group. */
StepperGroup *func_8002D3A0(StepperGroup *this, Stepper *child) {
    return stepperGroupAddChild(this, child);
}

/* Deletes a child of the group. */
void func_8002D3C0(StepperGroup *this, Stepper *child) {
    stepperGroupRemoveChild(this, child);
}
