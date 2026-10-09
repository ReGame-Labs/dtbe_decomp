#ifndef DTBE_MATH_QUATERNION_OPS_H
#define DTBE_MATH_QUATERNION_OPS_H

/* The simple quaternion operations: copies, the identity and in-place forms. */

#include "common.h"
#include "engine/math/quaternion.h"

EXTERN_C_BEGIN

/* A quaternion as two words, to copy or set two of its elements at once. */
typedef union {
    /* 0x0 */ Quaternion q;
    /* 0x0 */ u32 words[2];
} QuaternionWords;

QuaternionWords *quaternionCopy(QuaternionWords *dst, QuaternionWords *src);
Quaternion *quaternionMulInPlace(Quaternion *q, Quaternion *b);
QuaternionWords *func_80026954(QuaternionWords *out, Quaternion *a, Quaternion *b);
QuaternionWords *quaternionInitIdentity(QuaternionWords *q);
Quaternion *quaternionNormalizeInPlace(Quaternion *q);
Quaternion *quaternionConjugateInPlace(Quaternion *q);
void quaternionSlerpInPlace(Quaternion *q, Quaternion *to, s32 t);

EXTERN_C_END

#endif /* DTBE_MATH_QUATERNION_OPS_H */
