#ifndef DTBE_MATH_VECTOR_H
#define DTBE_MATH_VECTOR_H

/* Vectors: normalizing, lengths, dot products and interpolation. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A VECTOR without its padding word, so that it copies in three words. */
typedef struct Vec3 {
    s32 vx;
    s32 vy;
    s32 vz;
} Vec3;

VECTOR *vecNormalize(VECTOR *unit, VECTOR *v);
s32 vecGetLength(VECTOR *v);
s32 vecGetLength12(VECTOR *v);
s32 vecGetLengthSquared16(VECTOR *v);

VECTOR *vecLerp(VECTOR *out, VECTOR *a, VECTOR *b, s32 t);
s32 vecDot(VECTOR *a, VECTOR *b);
s32 vecDot12(VECTOR *a, VECTOR *b);

EXTERN_C_END

#endif /* DTBE_MATH_VECTOR_H */
