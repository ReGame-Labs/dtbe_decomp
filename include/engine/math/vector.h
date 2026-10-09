#ifndef DTBE_MATH_VECTOR_H
#define DTBE_MATH_VECTOR_H

/* Vectors: normalizing, lengths, dot products and interpolation. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A VECTOR without its padding word, so that it copies in three words. */
typedef struct Vec3 {
    /* 0x0 */ s32 vx;
    /* 0x4 */ s32 vy;
    /* 0x8 */ s32 vz;
} Vec3;

/* An SVECTOR without its padding. */
typedef struct Vec3s {
    /* 0x0 */ s16 vx;
    /* 0x2 */ s16 vy;
    /* 0x4 */ s16 vz;
} Vec3s;

VECTOR *vecNormalize(VECTOR *unit, VECTOR *v);
s32 vecGetLength(VECTOR *v);
s32 vecGetLength12(VECTOR *v);
s32 vecGetLengthSquared16(VECTOR *v);

VECTOR *vecLerp(VECTOR *out, VECTOR *a, VECTOR *b, s32 t);
s32 vecDot(VECTOR *a, VECTOR *b);
s32 vecDot12(VECTOR *a, VECTOR *b);

EXTERN_C_END

#endif /* DTBE_MATH_VECTOR_H */
