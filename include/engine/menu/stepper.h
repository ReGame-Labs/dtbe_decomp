#ifndef DTBE_MENU_STEPPER_H
#define DTBE_MENU_STEPPER_H

/* Countdowns, and the steppers: values stepped between bounds, alone or in groups. */

#include "common.h"
#include "vtable.h"
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
 * The classes as the C++ files (src/engine/menu/stepper.cpp and
 * debug/system_menu.cpp) declare them. The C files see the same objects through the structs in the #else part:
 * they embed a StepperGroup without its virtual base, which a C++ class can't
 * describe. The member functions keep the func_X names the C files call them
 * by (asm labels; g++ would mangle the names).
 */

/*
 * A list node that starts as a list of its own and leaves its list when it
 * is destroyed. Stepper derives from it: its constructor links the node
 * before it sets the virtual table pointer, which g++ 2.95 does for a base
 * class and not for a member, and StepperGroup's destructor unlinks it after
 * resetting that pointer to Stepper's. The name is ours.
 */
class ListItem : public ListNode {
public:
    ListItem() {
        next = this;
        prev = this;
    }

    ~ListItem() {
        listRemove(this);
    }
};

/* Frames left until something happens. */
class Countdown {
public:
    s32 frames;

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
    virtual s32 confirm(s32, s32, s32 own) __asm__("stepperConfirm");
    virtual s32 cancel(s32, s32, s32 own) __asm__("stepperCancel");
    virtual void focus(s32 arg) __asm__("stepperFocus");
    virtual void blur(s32 arg) __asm__("stepperBlur");
    virtual void stepUp(s32 arg, s32 held, u32 pressed) __asm__("stepperStepUp");
    virtual void stepDown(s32 arg, s32 held, u32 pressed) __asm__("stepperStepDown");
    virtual s32 input(s32, s32, u32 buttons) __asm__("stepperInput");
    virtual void draw(s32 arg, s32 isCurrent, s32 active) __asm__("stepperDraw");
    virtual void refresh(s32 arg) __asm__("stepperRefresh");
    /* inline: StepperGroup's destructor has it inlined */
    virtual ~Stepper() {
    }
    s32 stepperStepUpClamp() __asm__("stepperStepUpClamp");
    void stepperStepUpWrap() __asm__("stepperStepUpWrap");
    s32 stepperStepDownClamp() __asm__("stepperStepDownClamp");
    void stepperStepDownWrap() __asm__("stepperStepDownWrap");
    StepperGroup *stepperGetGroup() __asm__("stepperGetGroup");
    void stepperSetGroupCurrentOnly(s32 clear) __asm__("stepperSetGroupCurrentOnly");
    void stepperCloseGroup(s32 arg1) __asm__("stepperCloseGroup");
    /* defined in menu/stepper_group.c */
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
    /* 0x1C */ s32 unk1C;
    /* 0x20: the virtual table pointer */

    StepperGroup(const StepperButtons *buttons) __asm__("stepperGroupInit");
    virtual void unk8(s32 arg) __asm__("func_8002D390");
    virtual void unk10() __asm__("func_8002D398");
    s32 cancel(s32, s32, s32 own) __asm__("stepperGroupCancel");
    s32 input(s32, s32, u32 buttons) __asm__("stepperGroupInput");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("stepperGroupDraw");
    void refresh(s32 arg) __asm__("stepperGroupRefresh");
    ~StepperGroup();
    /* defined in menu/stepper_group.c */
    void stepperGroupOpen(s32 arg) __asm__("stepperGroupOpen");
    s32 stepperGroupInputIfOpen(s32 arg1, s32 arg2, u32 buttons) __asm__("stepperGroupInputIfOpen");
    void stepperGroupDrawIfOpen(s32 arg) __asm__("stepperGroupDrawIfOpen");
    StepperGroup *stepperGroupAddChild(Stepper *child) __asm__("stepperGroupAddChild");
    void stepperGroupRemoveChildren() __asm__("stepperGroupRemoveChildren");
    void stepperGroupInputCurrent(s32 arg1, s32 arg2, u32 buttons) __asm__("stepperGroupInputCurrent");
    s32 stepperGroupPressCancel(s32 arg1, s32 arg2) __asm__("stepperGroupPressCancel");
    void stepperGroupPressConfirm(s32 arg1, s32 arg2) __asm__("stepperGroupPressConfirm");
    void stepperGroupMoveCurrent(s32 arg, s32 forward) __asm__("stepperGroupMoveCurrent");
    s32 stepperGroupRemoveChild(Stepper *child) __asm__("stepperGroupRemoveChild");
    s32 stepperGroupCountChildren() __asm__("stepperGroupCountChildren");
    void stepperGroupNumberChildren() __asm__("stepperGroupNumberChildren");
    Stepper *stepperGroupGetChild(s32 index) __asm__("stepperGroupGetChild");
    void stepperGroupSetPendingIndex(s32 index) __asm__("stepperGroupSetPendingIndex");
    void stepperGroupSetKeepOpen(s32 arg) __asm__("stepperGroupSetKeepOpen");
    s32 stepperGroupIsChildActive() __asm__("stepperGroupIsChildActive");
    s32 stepperGroupIsClosed() __asm__("stepperGroupIsClosed");
    s32 func_8002D2C4() __asm__("func_8002D2C4");
    StepperGroup *func_8002D3A0(Stepper *child) __asm__("func_8002D3A0");
    void func_8002D3C0(Stepper *child) __asm__("func_8002D3C0");
};

#else

/* Frames left until something happens. */
typedef struct {
    s32 frames;
} Countdown;

struct Stepper;
struct StepperGroup;

/* An entry of a g++ 2.95 virtual table whose function returns a value. */
typedef struct {
    /* 0x0 */ s16 delta;
    /* 0x2 */ s16 index;
    /* 0x4 */ s32 (*func)();
} VtableEntryS32;

typedef struct StepperVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntryS32 confirm; /* (Stepper *, s32, s32, s32 own): 0 to take the buttons */
    /* 0x10 */ VtableEntryS32 cancel;  /* (Stepper *, s32, s32, s32 own): 0 to close its group */
    /* 0x18 */ VtableEntry focus;    /* (Stepper *, s32 arg): it became the current one */
    /* 0x20 */ VtableEntry blur;     /* (Stepper *, s32 arg): it stopped being the current one */
    /* 0x28 */ VtableEntry stepUp;   /* (Stepper *, s32 arg, s32 held, u32 pressed) */
    /* 0x30 */ VtableEntry stepDown; /* (Stepper *, s32 arg, s32 held, u32 pressed) */
    /* 0x38 */ VtableEntryS32 input; /* (Stepper *, s32, s32, u32 buttons): 1 while it stays active */
    /* 0x40 */ VtableEntry draw;     /* (Stepper *, s32 arg, s32 isCurrent, s32 active) */
    /* 0x48 */ VtableEntry refresh;   /* (Stepper *, s32 arg) */
    /* 0x50 */ VtableEntry destroy;  /* (Stepper *, s32 flags) */
} StepperVtable;

/* A value stepped up and down between bounds, one of the children of a StepperGroup. */
typedef struct Stepper {
    /* 0x00 */ ListNode link;
    /* 0x08 */ struct StepperGroup *group;
    /* 0x0C */ s32 index; /* among the children of its group */
    /* 0x10 */ s32 value;
    /* 0x14 */ s32 min;
    /* 0x18 */ s32 max;
    /* 0x1C */ StepperVtable *vtable;
} Stepper;

typedef struct StepperGroupVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry unk8;  /* (StepperGroup *, s32 arg), when moving past its first or last child */
    /* 0x10 */ VtableEntry unk10; /* (StepperGroup *) */
} StepperGroupVtable;

/*
 * A Stepper that holds a list of child steppers, one of them current, and
 * passes buttons to them. Stepper is its virtual base: g++ 2.95 keeps a
 * pointer to it, and puts it after the members of the most derived class
 * (at 0x24 in a plain StepperGroup).
 */
typedef struct StepperGroup {
    /* 0x00 */ Stepper *base;
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
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ StepperGroupVtable *vtable;
} StepperGroup;

/* the virtual table of Stepper, by g++'s name for it */
extern StepperVtable stepperVtable __asm__("_vt.7Stepper");

Countdown *countdownInit(Countdown *countdown, s32 frames);
Countdown *countdownStart(Countdown *countdown, s32 frames);
s32 countdownGetFrames(Countdown *countdown);
s32 countdownUpdate(Countdown *countdown);
s32 countdownIsOver(Countdown *countdown);
Stepper *stepperInit(Stepper *stepper, s32 value);
void stepperStepUp(Stepper *stepper);
s32 stepperStepUpClamp(Stepper *stepper);
void stepperStepUpWrap(Stepper *stepper);
void stepperStepDown(Stepper *stepper);
s32 stepperStepDownClamp(Stepper *stepper);
void stepperStepDownWrap(Stepper *stepper);
StepperGroup *stepperGetGroup(Stepper *stepper);
void stepperSetGroupCurrentOnly(Stepper *stepper, s32 clear);
void stepperCloseGroup(Stepper *stepper, s32 arg1);

#endif /* __cplusplus */

#endif /* DTBE_MENU_STEPPER_H */
