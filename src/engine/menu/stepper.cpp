/* writes out the inline functions of its header */
#pragma implementation
#include "common.h"
#include "engine/menu/stepper.h"
#include "engine/lib/list.h"

/* Constructs a countdown of frames. */
Countdown::Countdown(s32 frames) {
    this->frames = frames;
}

/* Restarts a countdown at frames. */
Countdown *Countdown::countdownStart(s32 frames) {
    this->frames = frames;
    return this;
}

/* Returns the frames left. */
s32 Countdown::countdownGetFrames() {
    return this->frames;
}

/* Counts one frame down; returns whether that ended the countdown. */
s32 Countdown::countdownUpdate() {
    s32 ended = 0;

    if (this->frames > 0) {
        this->frames--;
        ended = this->frames == 0;
    }
    return ended;
}

/* Returns whether the countdown is over. */
s32 Countdown::countdownIsOver() {
    return this->frames < 1;
}

/* Constructs a value stepping between bounds, the bounds both 0, in a list of its own. */
Stepper::Stepper(s32 value) {
    this->value = value;
    this->min = 0;
    this->max = 0;
    this->group = NULL;
}

/* Steps up, wrapping to the minimum. */
void Stepper::stepUp(s32 arg, s32 held, u32 buttons) {
    this->stepperStepUpWrap();
}

/* Steps up; returns -1 if that hit the maximum, where the value stays. */
s32 Stepper::stepperStepUpClamp() {
    s32 max = this->max;

    this->value++;
    if (max < this->value) {
        this->value = max;
        return -1;
    }
    return 0;
}

/* Steps up, wrapping past the maximum to the minimum. */
void Stepper::stepperStepUpWrap() {
    this->value++;
    if (this->max < this->value) {
        this->value = this->min;
    }
}

/* Steps down, wrapping to the maximum. */
void Stepper::stepDown(s32 arg, s32 held, u32 buttons) {
    this->stepperStepDownWrap();
}

/* Steps down; returns -1 if that hit the minimum, where the value stays. */
s32 Stepper::stepperStepDownClamp() {
    s32 min = this->min;

    this->value--;
    if (this->value < min) {
        this->value = min;
        return -1;
    }
    return 0;
}

/* Steps down, wrapping past the minimum to the maximum. */
void Stepper::stepperStepDownWrap() {
    this->value--;
    if (this->value < this->min) {
        this->value = this->max;
    }
}

/* Returns the group of a stepper. */
StepperGroup *Stepper::stepperGetGroup() {
    return this->group;
}

/* Makes the stepper's group draw its active current child alone, or not if clear. */
void Stepper::stepperSetGroupCurrentOnly(s32 clear) {
    StepperGroup *group = this->group;

    if (group != NULL) {
        group->currentOnly = !clear;
    }
}

/* Closes the stepper's group with a value (stepperGroupGetCloseValue). */
void Stepper::stepperCloseGroup(s32 value) {
    this->group->closed = 1;
    this->group->closeValue = value;
}

/*
 * Constructs a group with no children, reacting to buttons. The rest is
 * g++'s: it builds the Stepper base only for a whole StepperGroup; for one
 * that is the base of another class it copies the virtual table to the
 * stack and fixes up the offsets of the functions StepperGroup overrides.
 */
StepperGroup::StepperGroup(const StepperButtons *buttons) : Stepper(0) {
    this->children.next = &this->children;
    this->children.prev = &this->children;
    this->buttons = buttons;
    this->current = (Stepper *)&this->children;
    this->pendingIndex = -1;
    this->closeValue = -1;
    this->wraps = 0;
    this->childActive = 0;
    this->childWasActive = 0;
    this->currentOnly = 0;
    this->closed = 0;
    this->keepOpen = 0;
}

/* Destroys the children, then takes the empty list out of any list it is in. */
StepperGroup::~StepperGroup() {
    this->stepperGroupDestroyChildren();

    /*
     * The second line reads children.prev through the pointer to the head,
     * the others through the member: the compiled code does that. Which
     * helper or form the original wrote is not known.
     */
    ListNode *head = &this->children;
    this->children.next->prev = this->children.prev;
    head->prev->next = this->children.next;
    this->children.next = head;
    this->children.prev = head;
}

/*
 * The game tests the group's flags through inline accessors that return a
 * byte. g++ turns a test of the bit-field itself into one mask test (andi),
 * and merges two such tests into one; the game shifts each flag down and
 * masks it (srl, andi). Through an accessor returning s32 it does that too,
 * but in the accessor's return register, which moves the store before a
 * test (stepperGroupInput); the conversion to a byte computes it apart.
 */
static inline u8 isChildActive(StepperGroup *group) {
    return group->childActive;
}

static inline u8 wasChildActive(StepperGroup *group) {
    return group->childWasActive;
}

static inline u8 isCurrentOnly(StepperGroup *group) {
    return group->currentOnly;
}

static inline u8 isClosed(StepperGroup *group) {
    return group->closed;
}

static inline u8 isKeptOpen(StepperGroup *group) {
    return group->keepOpen;
}

/* Opens the group again and refreshes it. */
void StepperGroup::stepperGroupOpen(s32 arg) {
    this->closed = 0;
    this->refresh(arg);
}

/* Passes the buttons to the group's input, unless it is closed. */
s32 StepperGroup::stepperGroupInputIfOpen(s32 arg, s32 held, u32 buttons) {
    if (this->stepperGroupIsClosed()) {
        return 0;
    }
    return this->input(arg, held, buttons);
}

/* Draws the group as the current one, unless it is closed. */
void StepperGroup::stepperGroupDrawIfOpen(s32 arg) {
    if (!this->stepperGroupIsClosed()) {
        this->draw(arg, 1, !isChildActive(this));
    }
}

/* Adds a child at the end of the group; the first child becomes the current one. */
StepperGroup *StepperGroup::stepperGroupAddChild(Stepper *child) {
    listInsertAfter(this->children.prev, child);
    if (&this->children == this->current) {
        this->current = child;
    }
    child->group = this;
    this->stepperGroupNumberChildren();
    return this;
}

/* Destroys and frees every child of the group (delete), leaving it no current child. */
void StepperGroup::stepperGroupDestroyChildren() {
    ListNode *node;
    ListNode *list = &this->children;

    for (node = list->next; node != list;) {
        ListNode *next = node->next;
        delete (Stepper *)node;
        node = next;
    }
    /* the list head stands for no current child */
    this->current = (Stepper *)&this->children;
}

/* Returns -1 while a child takes the buttons or did so this frame, else 0 to close the group. */
s32 StepperGroup::cancel(s32 arg, s32 held, s32 own) {
    if (isChildActive(this) || wasChildActive(this)) {
        return -1;
    }
    return 0;
}

/*
 * Reacts to the buttons pressed: passes them to the current child while it is
 * active, else closes, confirms, moves between the children or steps the
 * current one. Returns 1, or what cancelling returns.
 */
s32 StepperGroup::input(s32 arg, s32 held, u32 buttons) {
    Stepper *current = this->current;
    const StepperButtons *map;

    if (&this->children == current) {
        return 1;
    }
    this->childWasActive = this->childActive;
    if (isChildActive(this)) {
        this->stepperGroupInputCurrent(arg, held, buttons);
        return 1;
    }
    map = this->buttons;
    if (buttons & map->cancel) {
        return this->stepperGroupPressCancel(arg, held);
    }
    if ((buttons & map->confirm) || this->pendingIndex >= 0) {
        this->stepperGroupPressConfirm(arg, held);
        return 1;
    }
    if (buttons & map->prev) {
        this->stepperGroupMoveCurrent(arg, 0);
        return 1;
    }
    if (buttons & map->next) {
        this->stepperGroupMoveCurrent(arg, 1);
        return 1;
    }
    if (buttons & map->stepUp) {
        current->stepUp(arg, held, buttons);
        return 1;
    }
    if (buttons & map->stepDown) {
        current->stepDown(arg, held, buttons);
        return 1;
    }
    /* no button it reacts to */
    return 1;
}

/* Passes the buttons to the current child, which keeps them while its input returns 1. */
void StepperGroup::stepperGroupInputCurrent(s32 arg, s32 held, u32 buttons) {
    this->childActive = this->current->input(arg, held, buttons) == 1;
    if (isChildActive(this)) {
        if (buttons & this->buttons->cancel) {
            this->childActive = this->current->cancel(arg, held, 1) != 0;
        } else if (buttons & this->buttons->confirm) {
            this->current->confirm(arg, held, 1);
        }
    }
    if (isCurrentOnly(this)) {
        this->currentOnly = this->childActive;
    }
}

/* The cancel button: tells the current child, then the group; closes the group if it returns 0. */
s32 StepperGroup::stepperGroupPressCancel(s32 arg, s32 held) {
    this->current->cancel(arg, held, 0);
    if (this->cancel(arg, held, 1) == 0) {
        this->closed = 1;
        return 0;
    }
    return 1;
}

/*
 * The confirm button: first makes the pending child current, if one was set;
 * then confirms the current child, which takes the buttons if that returns 0;
 * else runs the group's confirmRefused.
 */
void StepperGroup::stepperGroupPressConfirm(s32 arg, s32 held) {
    Stepper *child;

    if (this->pendingIndex >= 0) {
        child = this->stepperGroupGetChild(this->pendingIndex);
        if (child != NULL) {
            this->current = child;
        }
        this->pendingIndex = -1;
    }
    if (this->current->confirm(arg, held, 0) == 0) {
        this->childActive = this->current->input(arg, held, 0) == 1;
    } else {
        this->confirmRefused();
    }
}

/*
 * Makes the next child current, or the previous one. Moving past the first or
 * the last child, a group that wraps goes on at the other end; one that does
 * not, or that has nothing there, runs its moveOffEnd instead.
 */
void StepperGroup::stepperGroupMoveCurrent(s32 arg, s32 forward) {
    ListNode *node = this->current;
    ListNode *list = &this->children;
    Stepper *next;

    node = forward ? node->next : node->prev;
    /* past the end, a group that wraps steps on over the list head */
    if (node == list && (!this->wraps || (node = forward ? node->next : node->prev) == list)) {
        this->moveOffEnd(arg);
    } else {
        this->current->blur(arg);
        next = (Stepper *)node;
        next->focus(arg);
        this->current = next;
    }
}

/* Draws the children, or the active current one alone if currentOnly. */
void StepperGroup::draw(s32 arg, s32 isCurrent, s32 active) {
    Stepper *child;
    ListNode *list;
    ListNode *node;
    s32 childIsCurrent;

    if (isChildActive(this) && isCurrentOnly(this)) {
        this->current->draw(arg, 1, 1);
        return;
    }
    list = &this->children;
    for (node = list->next; node != list; node = node->next) {
        child = (Stepper *)node;
        childIsCurrent = child == this->current;
        child->draw(arg, childIsCurrent, childIsCurrent && isChildActive(this));
    }
}

/* Refreshes every child. */
void StepperGroup::refresh(s32 arg) {
    ListNode *list = &this->children;
    ListNode *node;

    for (node = list->next; node != list; node = node->next) {
        ((Stepper *)node)->refresh(arg);
    }
}

/* Destroys and frees a child of the group (delete); returns -1 if it is not one. */
s32 StepperGroup::stepperGroupDestroyChild(Stepper *child) {
    ListNode *list = &this->children;
    ListNode *node;

    if (this->current == child) {
        /* the next child becomes current, or the previous one after the last */
        node = child->next;
        if (node == list) {
            node = child->prev;
        }
        this->current = (Stepper *)node;
        delete child;
        this->stepperGroupNumberChildren();
        return 0;
    }
    for (node = list->next; node != list; node = node->next) {
        if (node == child) {
            delete (Stepper *)node;
            this->stepperGroupNumberChildren();
            return 0;
        }
    }
    return -1;
}

/* Returns the number of children of the group. */
s32 StepperGroup::stepperGroupCountChildren() {
    ListNode *node;
    s32 count = 0;

    for (node = this->children.next; node != &this->children; node = node->next) {
        count++;
    }
    return count;
}

/* Numbers the children of the group in order, from 0. */
void StepperGroup::stepperGroupNumberChildren() {
    ListNode *node;
    s32 index = 0;

    for (node = this->children.next; node != &this->children; node = node->next) {
        ((Stepper *)node)->index = index;
        index++;
    }
}

/* Returns the child of an index, or NULL if there is none. */
Stepper *StepperGroup::stepperGroupGetChild(s32 index) {
    ListNode *node;

    for (node = this->children.next; node != &this->children; node = node->next) {
        /* index counts down to the child */
        if (--index == -1) {
            return (Stepper *)node;
        }
    }
    return NULL;
}

/* Sets the index of the child that the confirm button makes current. */
void StepperGroup::stepperGroupSetPendingIndex(s32 index) {
    this->pendingIndex = index;
}

/* Keeps the group from counting as closed if keepOpen is 1, else stops keeping it. */
void StepperGroup::stepperGroupSetKeepOpen(s32 keepOpen) {
    this->keepOpen = keepOpen == 1;
}

/* Returns whether the current child takes the buttons. */
s32 StepperGroup::stepperGroupIsChildActive() {
    return this->childActive;
}

/* Returns whether the group is closed, unless it is kept open. */
s32 StepperGroup::stepperGroupIsClosed() {
    return isClosed(this) && !isKeptOpen(this);
}

/* Returns the value the group was closed with (stepperCloseGroup), or -1. */
s32 StepperGroup::stepperGroupGetCloseValue() {
    return this->closeValue;
}
