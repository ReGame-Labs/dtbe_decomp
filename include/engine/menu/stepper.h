#ifndef DTBE_MENU_STEPPER_H
#define DTBE_MENU_STEPPER_H

/* Countdowns, and the steppers: values stepped between bounds, alone or in groups. */

#ifdef __cplusplus
/* The inline functions below are written out once, in stepper.cpp, at the
 * end of its code. */
#pragma interface
#endif

#include "common.h"
#include "engine/lib/list.h"

/* the masks of the buttons a StepperGroup reacts to */
typedef struct {
    /* 0x0 */ u16 prev;     /* make the previous child current */
    /* 0x2 */ u16 next;     /* make the next child current */
    /* 0x4 */ u16 confirm;
    /* 0x6 */ u16 cancel;
    /* 0x8 */ u16 stepUp;   /* step the current child up */
    /* 0xA */ u16 stepDown; /* step it down */
} StepperButtons;

#ifdef __cplusplus

/*
 * Only C++ files (src/engine/menu/stepper.cpp and debug/system_menu.cpp) use
 * these classes. The member functions keep, through asm labels, the names
 * of the symbol files (g++ would mangle them).
 */

/* Frames left until something happens. */
class Countdown {
public:
    /* 0x0 */ s32 frames;

    Countdown(s32 frames) __asm__("countdownInit");
    Countdown *countdownStart(s32 frames) __asm__("countdownStart");
    s32 countdownGetFrames() __asm__("countdownGetFrames");
    s32 countdownUpdate() __asm__("countdownUpdate");
    s32 countdownIsOver() __asm__("countdownIsOver");
};

class StepperGroup;

/* A value stepped up and down between bounds, one of the children of a StepperGroup. */
class Stepper : public ListItem {
public:
    /* 0x08 */ StepperGroup *group;
    /* 0x0C */ s32 index; /* among the children of its group */
    /* 0x10 */ s32 value;
    /* 0x14 */ s32 min;
    /* 0x18 */ s32 max;
    /* 0x1C: the virtual table pointer */

    Stepper(s32 value = 0) __asm__("stepperInit");
    virtual s32 confirm(s32 arg, s32 held, s32 own) __asm__("stepperConfirm");
    virtual s32 cancel(s32 arg, s32 held, s32 own) __asm__("stepperCancel");
    virtual void focus(s32 arg) __asm__("stepperFocus");
    virtual void blur(s32 arg) __asm__("stepperBlur");
    virtual void stepUp(s32 arg, s32 held, u32 buttons) __asm__("stepperStepUp");
    virtual void stepDown(s32 arg, s32 held, u32 buttons) __asm__("stepperStepDown");
    virtual s32 input(s32 arg, s32 held, u32 buttons) __asm__("stepperInput");
    virtual void draw(s32 arg, s32 isCurrent, s32 active) __asm__("stepperDraw");
    virtual void refresh(s32 arg) __asm__("stepperRefresh");
    virtual ~Stepper();
    s32 stepperStepUpClamp() __asm__("stepperStepUpClamp");
    void stepperStepUpWrap() __asm__("stepperStepUpWrap");
    s32 stepperStepDownClamp() __asm__("stepperStepDownClamp");
    void stepperStepDownWrap() __asm__("stepperStepDownWrap");
    StepperGroup *stepperGetGroup() __asm__("stepperGetGroup");
    void stepperSetGroupCurrentOnly(s32 clear) __asm__("stepperSetGroupCurrentOnly");
    void stepperCloseGroup(s32 value) __asm__("stepperCloseGroup");
    s32 stepperGetValue() __asm__("stepperGetValue");
    void stepperSetValue(s32 value) __asm__("stepperSetValue");
    void stepperSetMin(s32 min) __asm__("stepperSetMin");
    void stepperSetMax(s32 max) __asm__("stepperSetMax");
    s32 stepperGetIndex() __asm__("stepperGetIndex");
};

/*
 * A Stepper that holds a list of child steppers, one of them current, and
 * passes buttons to them. Stepper is its virtual base, which g++ 2.95 puts
 * after the members of the most derived class (at 0x24 in a plain
 * StepperGroup) and reaches through a pointer at 0x00.
 */
class StepperGroup : public virtual Stepper {
public:
    /* 0x04 */ ListNode children;
    /* 0x0C */ const StepperButtons *buttons;
    /* 0x10 */ u32 wraps : 1;          /* moving past the last child goes to the first */
    /* 0x10 */ u32 childActive : 1;    /* the current child takes the buttons */
    /* 0x10 */ u32 childWasActive : 1; /* it did when this frame's buttons came */
    /* 0x10 */ u32 currentOnly : 1;    /* draw the active current child alone, until it stops */
    /* 0x10 */ u32 closed : 1;
    /* 0x10 */ u32 keepOpen : 1;       /* a closed group does not count as closed */
    /* 0x14 */ Stepper *current; /* the list head while there are no children */
    /* 0x18 */ s32 pendingIndex; /* the index of a child to make current, or -1 */
    /* 0x1C */ s32 closeValue; /* what stepperCloseGroup closed it with, or -1 */
    /* 0x20: the virtual table pointer */

    StepperGroup(const StepperButtons *buttons) __asm__("stepperGroupInit");
    virtual void moveOffEnd(s32 arg) __asm__("stepperGroupMoveOffEnd");
    virtual void confirmRefused() __asm__("stepperGroupConfirmRefused");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("stepperGroupCancel");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("stepperGroupInput");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("stepperGroupDraw");
    void refresh(s32 arg) __asm__("stepperGroupRefresh");
    ~StepperGroup();
    void stepperGroupOpen(s32 arg) __asm__("stepperGroupOpen");
    s32 stepperGroupInputIfOpen(s32 arg, s32 held, u32 buttons) __asm__("stepperGroupInputIfOpen");
    void stepperGroupDrawIfOpen(s32 arg) __asm__("stepperGroupDrawIfOpen");
    StepperGroup *stepperGroupAddChild(Stepper *child) __asm__("stepperGroupAddChild");
    void stepperGroupDestroyChildren() __asm__("stepperGroupDestroyChildren");
    void stepperGroupInputCurrent(s32 arg, s32 held, u32 buttons) __asm__("stepperGroupInputCurrent");
    s32 stepperGroupPressCancel(s32 arg, s32 held) __asm__("stepperGroupPressCancel");
    void stepperGroupPressConfirm(s32 arg, s32 held) __asm__("stepperGroupPressConfirm");
    void stepperGroupMoveCurrent(s32 arg, s32 forward) __asm__("stepperGroupMoveCurrent");
    s32 stepperGroupDestroyChild(Stepper *child) __asm__("stepperGroupDestroyChild");
    s32 stepperGroupCountChildren() __asm__("stepperGroupCountChildren");
    void stepperGroupNumberChildren() __asm__("stepperGroupNumberChildren");
    Stepper *stepperGroupGetChild(s32 index) __asm__("stepperGroupGetChild");
    void stepperGroupSetPendingIndex(s32 index) __asm__("stepperGroupSetPendingIndex");
    void stepperGroupSetKeepOpen(s32 keepOpen) __asm__("stepperGroupSetKeepOpen");
    s32 stepperGroupIsChildActive() __asm__("stepperGroupIsChildActive");
    s32 stepperGroupIsClosed() __asm__("stepperGroupIsClosed");
    s32 stepperGroupGetCloseValue() __asm__("stepperGroupGetCloseValue");
    StepperGroup *func_8002D3A0(Stepper *child) __asm__("func_8002D3A0");
    void func_8002D3C0(Stepper *child) __asm__("func_8002D3C0");
};

/*
 * The inline functions, defined out of the classes because g++ takes no asm
 * label on a definition. g++ writes them out at the end of stepper.cpp in
 * this order, which is the game's.
 */

/* Returns -1: a plain stepper does not take the buttons. */
inline s32 Stepper::confirm(s32 arg, s32 held, s32 own) {
    return -1;
}

/* Returns 0: cancel closes the group. */
inline s32 Stepper::cancel(s32 arg, s32 held, s32 own) {
    return 0;
}

/* Does nothing. */
inline void Stepper::focus(s32 arg) {
}

/* Does nothing. */
inline void Stepper::blur(s32 arg) {
}

/* Returns 1: a plain stepper stays active. */
inline s32 Stepper::input(s32 arg, s32 held, u32 buttons) {
    return 1;
}

/* Draws nothing. */
inline void Stepper::draw(s32 arg, s32 isCurrent, s32 active) {
}

/* Does nothing. */
inline void Stepper::refresh(s32 arg) {
}

/* Takes the stepper out of its list (~ListItem). */
inline Stepper::~Stepper() {
}

/* Returns the value of a stepper. */
inline s32 Stepper::stepperGetValue() {
    return this->value;
}

/* Sets the value of a stepper. */
inline void Stepper::stepperSetValue(s32 value) {
    this->value = value;
}

/* Sets the minimum of a stepper. */
inline void Stepper::stepperSetMin(s32 min) {
    this->min = min;
}

/* Sets the maximum of a stepper. */
inline void Stepper::stepperSetMax(s32 max) {
    this->max = max;
}

/* Returns the index of a stepper in its group. */
inline s32 Stepper::stepperGetIndex() {
    return this->index;
}

/* Runs when moving past the first or the last child finds no child to make
 * current; does nothing. */
inline void StepperGroup::moveOffEnd(s32 arg) {
}

/* Runs when the current child's confirm does not take the buttons; does
 * nothing. */
inline void StepperGroup::confirmRefused() {
}

/* Adds a child at the end of the group. */
inline StepperGroup *StepperGroup::func_8002D3A0(Stepper *child) {
    return stepperGroupAddChild(child);
}

/* The same as stepperGroupDestroyChild, without its result. */
inline void StepperGroup::func_8002D3C0(Stepper *child) {
    stepperGroupDestroyChild(child);
}

#endif /* __cplusplus */

#endif /* DTBE_MENU_STEPPER_H */
