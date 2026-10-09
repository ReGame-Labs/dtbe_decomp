#ifndef DTBE_GFX_FADE_H
#define DTBE_GFX_FADE_H

/* The task that fades the screen to and from black. */

#include "common.h"
#include "engine/math/lerp.h"
#include "engine/task/task.h"

EXTERN_C_BEGIN

/* the task that fades the screen to and from black */
typedef struct FadeControl {
    /* 0x00 */ Task task;
    /* 0x20 */ Lerp level; /* how black, 0 to 255 */
} FadeControl;

/* the level of a fully black screen */
#define FADE_BLACK 255

/* the priority of the fade task */
#define FADE_CONTROL_PRIORITY 0xF000

extern struct TaskVtable FADE_CONTROL_VTABLE; /* the virtual table of FadeControl */

/* the task that fades the screen */
extern struct FadeControl FADE_CONTROL;

FadeControl *fadeControlInit(FadeControl *fadeControl);
void fadeControlFadeOut(FadeControl *fadeControl, s32 duration);
void fadeControlFadeIn(FadeControl *fadeControl, s32 duration);
void fadeControlFadeTo(FadeControl *fadeControl, s32 duration, s32 level);
s32 fadeControlIsDone(FadeControl *fadeControl);
void fadeControlDraw(FadeControl *fadeControl);
void fadeControlDestroy(FadeControl *fadeControl, s32 flags);

EXTERN_C_END

#endif /* DTBE_GFX_FADE_H */
