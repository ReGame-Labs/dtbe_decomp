#ifndef DTBE_MATH_LERP_H
#define DTBE_MATH_LERP_H

/* A value that moves from where it is to a target over a number of steps. */

#include "common.h"

EXTERN_C_BEGIN

/* A value that moves from start to target over duration steps. */
typedef struct {
    s32 duration;
    s32 time;
    s32 value;
    s32 start;
    s32 target;
} Lerp;

void lerpStart(Lerp *lerp, s32 target, s32 duration);
s32 lerpIsDone(Lerp *lerp);
s32 lerpGetValue(Lerp *lerp);

EXTERN_C_END

#endif /* DTBE_MATH_LERP_H */
