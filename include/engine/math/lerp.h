#ifndef DTBE_MATH_LERP_H
#define DTBE_MATH_LERP_H

/* A value that moves from where it is to a target over a number of steps. */

#include "common.h"

EXTERN_C_BEGIN

/* A value that moves from start to target over duration steps. */
typedef struct {
    /* 0x00 */ s32 duration;
    /* 0x04 */ s32 time;
    /* 0x08 */ s32 value;
    /* 0x0C */ s32 start;
    /* 0x10 */ s32 target;
} Lerp;

Lerp *lerpInit(Lerp *lerp, s32 value);
s32 lerpUpdate(Lerp *lerp);
void lerpStart(Lerp *lerp, s32 target, s32 duration);
s32 lerpIsDone(Lerp *lerp);
s32 lerpGetValue(Lerp *lerp);

EXTERN_C_END

#endif /* DTBE_MATH_LERP_H */
