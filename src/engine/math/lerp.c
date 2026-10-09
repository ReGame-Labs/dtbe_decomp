#include "common.h"
#include "engine/math/lerp.h"

/* Starts moving towards target; with no duration the value jumps there. */
void lerpStart(Lerp *lerp, s32 target, s32 duration) {
    lerp->duration = duration;
    lerp->time = 0;
    if (duration == 0) {
        lerp->value = target;
        lerp->start = target;
        lerp->target = target;
        return;
    }
    lerp->target = target;
    lerp->start = lerp->value;
}

/* Whether the value has reached its target. */
s32 lerpIsDone(Lerp *lerp) {
    return lerp->time > lerp->duration;
}

s32 lerpGetValue(Lerp *lerp) {
    return lerp->value;
}
