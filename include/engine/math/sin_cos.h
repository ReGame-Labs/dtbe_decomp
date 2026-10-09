#ifndef DTBE_MATH_SIN_COS_H
#define DTBE_MATH_SIN_COS_H

/* The cosine and sine of 4096-step angles, from a table, in 4.12 fixed point. */

#include "common.h"

EXTERN_C_BEGIN

/* One step of the 4096-step cosine and sine table, in 4.12 fixed point. */
typedef struct {
    s16 cos;
    s16 sin;
} CosSin;

extern CosSin COS_SIN_TABLE[4096];

s16 getCos(s32 angle);
s16 getSin(s32 angle);

EXTERN_C_END

#endif /* DTBE_MATH_SIN_COS_H */
