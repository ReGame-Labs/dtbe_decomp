#include "common.h"
#include "engine/math/lerp.h"
#include "libgte.h"

/* Sets up a lerp standing at value; returns it. */
Lerp *lerpInit(Lerp *this, s32 value) {
    lerpStart(this, value, 0);
    return this;
}

/* Advances one step and returns the new value. */
s32 lerpUpdate(Lerp *this) {
    s32 value = this->value;
    s32 t;

    if (!lerpIsDone(this)) {
        this->time++;
        t = (this->time << 12) / (this->duration + 1);
        value = (this->target * t + this->start * (ONE - t)) >> 12;
        this->value = value;
    }
    return value;
}

/* Starts moving towards target; with no duration the value jumps there. */
void lerpStart(Lerp *this, s32 target, s32 duration) {
    this->duration = duration;
    this->time = 0;
    if (duration == 0) {
        this->value = target;
        this->start = target;
        this->target = target;
        return;
    }
    this->target = target;
    this->start = this->value;
}

/* Whether the value has reached its target. */
s32 lerpIsDone(Lerp *this) {
    return this->time > this->duration;
}

/* The current value. */
s32 lerpGetValue(Lerp *this) {
    return this->value;
}
