#ifndef DTBE_MATH_TRIG_H
#define DTBE_MATH_TRIG_H

/* The tangent and the arc cosine, in 4096ths of a turn and 4.12 fixed point. */

#include "common.h"

EXTERN_C_BEGIN

/* the arc cosine of (i - 4096) / 4096 by i, in 4096ths of a turn */
extern s16 ARC_COS_TABLE[2 * 4096 + 1];

s32 getTan(s32 angle);
s32 getArcCos(s32 cos);

EXTERN_C_END

#endif /* DTBE_MATH_TRIG_H */
