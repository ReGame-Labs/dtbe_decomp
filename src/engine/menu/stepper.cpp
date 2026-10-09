#include "common.h"
#include "engine/menu/stepper.h"
#include "engine/lib/list.h"
#include "engine/menu/stepper_group.h"

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
void Stepper::stepUp(s32 arg, s32 held, u32 pressed) {
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
void Stepper::stepDown(s32 arg, s32 held, u32 pressed) {
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

/* Closes the stepper's group, setting its unk1C. */
void Stepper::stepperCloseGroup(s32 arg1) {
    this->group->closed = 1;
    this->group->unk1C = arg1;
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
    this->unk1C = -1;
    this->wraps = 0;
    this->childActive = 0;
    this->childWasActive = 0;
    this->currentOnly = 0;
    this->closed = 0;
    this->keepOpen = 0;
}

/* Deletes the children, then takes the empty list out of any list it is in. */
StepperGroup::~StepperGroup() {
    this->stepperGroupRemoveChildren();

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

INCLUDE_RODATA("asm/jp/main/nonmatchings/menu/stepper", _vt.12StepperGroup);

INCLUDE_RODATA("asm/jp/main/nonmatchings/menu/stepper", _vt.12StepperGroup.7Stepper);

INCLUDE_RODATA("asm/jp/main/nonmatchings/menu/stepper", _vt.7Stepper);
