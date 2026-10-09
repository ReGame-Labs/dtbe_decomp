#ifndef DTBE_MENU_STEPPER_GROUP_H
#define DTBE_MENU_STEPPER_GROUP_H

/* The stepper groups as the C files call them: children, buttons, Stepper's defaults. */

#include "common.h"
#include "engine/menu/stepper.h"

/* C++ sees these as members of Stepper and StepperGroup (engine/menu/stepper.h). */
#ifndef __cplusplus

void stepperGroupOpen(StepperGroup *stepperGroup, s32 arg);
s32 stepperGroupInputIfOpen(StepperGroup *stepperGroup, s32 arg1, s32 arg2, u32 buttons);
void stepperGroupDrawIfOpen(StepperGroup *stepperGroup, s32 arg);
StepperGroup *stepperGroupAddChild(StepperGroup *stepperGroup, Stepper *child);
void stepperGroupRemoveChildren(StepperGroup *stepperGroup);
s32 stepperGroupCancel(StepperGroup *stepperGroup);
s32 stepperGroupInput(StepperGroup *stepperGroup, s32 arg1, s32 arg2, u32 buttons);
void stepperGroupInputCurrent(StepperGroup *stepperGroup, s32 arg1, s32 arg2, u32 buttons);
s32 stepperGroupPressCancel(StepperGroup *stepperGroup, s32 arg1, s32 arg2);
void stepperGroupPressConfirm(StepperGroup *stepperGroup, s32 arg1, s32 arg2);
void stepperGroupMoveCurrent(StepperGroup *stepperGroup, s32 arg, s32 forward);
void stepperGroupDraw(StepperGroup *stepperGroup, s32 arg, s32 isCurrent, s32 active);
void stepperGroupRefresh(StepperGroup *stepperGroup, s32 arg);
s32 stepperGroupRemoveChild(StepperGroup *stepperGroup, Stepper *child);
s32 stepperGroupCountChildren(StepperGroup *stepperGroup);
void stepperGroupNumberChildren(StepperGroup *stepperGroup);
Stepper *stepperGroupGetChild(StepperGroup *stepperGroup, s32 index);
void stepperGroupSetPendingIndex(StepperGroup *stepperGroup, s32 index);
void stepperGroupSetKeepOpen(StepperGroup *stepperGroup, s32 arg);
s32 stepperGroupIsChildActive(StepperGroup *stepperGroup);
s32 stepperGroupIsClosed(StepperGroup *stepperGroup);
s32 func_8002D2C4(StepperGroup *stepperGroup);
s32 stepperConfirm(void);
s32 stepperCancel(void);
void stepperFocus(void);
void stepperBlur(void);
s32 stepperInput(void);
void stepperDraw(void);
void stepperRefresh(void);
void stepperDestroy(Stepper *stepper, s32 flags);
s32 stepperGetValue(Stepper *stepper);
void stepperSetValue(Stepper *stepper, s32 value);
void stepperSetMin(Stepper *stepper, s32 min);
void stepperSetMax(Stepper *stepper, s32 max);
s32 stepperGetIndex(Stepper *stepper);
void func_8002D390(void);
void func_8002D398(void);
StepperGroup *func_8002D3A0(StepperGroup *stepperGroup, Stepper *child);
void func_8002D3C0(StepperGroup *stepperGroup, Stepper *child);
#endif /* __cplusplus */

#endif /* DTBE_MENU_STEPPER_GROUP_H */
