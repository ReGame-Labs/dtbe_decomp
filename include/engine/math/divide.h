#ifndef DTBE_MATH_DIVIDE_H
#define DTBE_MATH_DIVIDE_H

/* Division into 4.12 and 16.16 fixed point, and the 4.12 product. */

#include "common.h"

EXTERN_C_BEGIN

s32 divide12(s32 numerator, s32 denominator); /* in 4.12 fixed point */
s32 divide16(s32 numerator, s32 denominator); /* in 16.16 fixed point */

/* The product of a and b in 4.12 fixed point, taken in 64 bits. */
static inline s32 mul12(s32 a, s32 b) {
    return ((s64)a * b) >> 12;
}

EXTERN_C_END

#endif /* DTBE_MATH_DIVIDE_H */
