#ifndef DTBE_MATH_QUATERNION_H
#define DTBE_MATH_QUATERNION_H

/* Rotations as unit quaternions: from angles and matrices, products and slerp. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* a rotation as a unit quaternion, in 4.12 fixed point */
typedef struct {
    /* 0x0 */ s16 x;
    /* 0x2 */ s16 y;
    /* 0x4 */ s16 z;
    /* 0x6 */ s16 w;
} Quaternion;

Quaternion *quaternionInitRotationAxis(Quaternion *q, VECTOR *axis, s32 angle);
Quaternion *quaternionInitRotationX(Quaternion *q, s32 angle);
Quaternion *quaternionInitRotationY(Quaternion *q, s32 angle);
Quaternion *quaternionInitRotationZ(Quaternion *q, s32 angle);
Quaternion *quaternionNormalize(Quaternion *out, Quaternion *q);

Quaternion *quaternionConjugate(Quaternion *dst, Quaternion *q);
Quaternion *quaternionSlerp(Quaternion *out, Quaternion *a, Quaternion *b, s32 t);
Quaternion *quaternionInitFromMatrix(Quaternion *q, MATRIX *m);
void quaternionGetMatrix(Quaternion *q, MATRIX *m);
Quaternion *quaternionMul(Quaternion *out, Quaternion *a, Quaternion *b);
void quaternionRotateVec(Quaternion *q, VECTOR *out, VECTOR *in);

EXTERN_C_END

#endif /* DTBE_MATH_QUATERNION_H */
