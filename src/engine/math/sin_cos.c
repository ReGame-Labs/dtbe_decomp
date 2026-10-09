#include "common.h"
#include "engine/math/sin_cos.h"

/* Returns the cosine of a 4096-step angle, in 4.12 fixed point. */
s16 getCos(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* Returns the sine of a 4096-step angle, in 4.12 fixed point. */
s16 getSin(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].sin;
}
