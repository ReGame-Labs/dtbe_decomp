#ifndef DTBE_MATH_VECTOR_OPS_H
#define DTBE_MATH_VECTOR_OPS_H

/* Vector and short vector operations: copies, sums, differences and cross products. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A short vector as two words, to copy two of its elements at once. */
typedef union {
    /* 0x0 */ SVECTOR v;
    /* 0x0 */ u32 words[2];
} SVectorWords;

SVECTOR *func_800261A8(SVECTOR *v);
SVECTOR *func_800261B0(SVECTOR *v);
SVectorWords *svecCopy(SVectorWords *dst, SVectorWords *src);
SVECTOR *svecSet(SVECTOR *v, s16 x, s16 y, s16 z);
void svecScale12(SVECTOR *out, SVECTOR *in, s16 t);
void svecScale(SVECTOR *out, SVECTOR *in, s16 t);
VECTOR *func_80026284(VECTOR *v);
VECTOR *func_8002628C(VECTOR *v);
VECTOR *vecClear(VECTOR *v);
VECTOR *vecCopy(VECTOR *dst, VECTOR *src);
VECTOR *vecSet(VECTOR *v, s32 x, s32 y, s32 z);
VECTOR *vecSetWithPad(VECTOR *v, s32 x, s32 y, s32 z, s32 pad);
VECTOR *vecAddInPlace(VECTOR *a, VECTOR *b);
VECTOR *vecAdd(VECTOR *out, VECTOR *a, VECTOR *b);
VECTOR *vecSubInPlace(VECTOR *a, VECTOR *b);
VECTOR *vecSub(VECTOR *out, VECTOR *a, VECTOR *b);

VECTOR *vecNormalizeInPlace(VECTOR *v);
s32 vecGetLengthSquared(VECTOR *v);
u32 vecGetLengthSquared12(VECTOR *v);
VECTOR *func_80026478(VECTOR *out, VECTOR *a, VECTOR *b);
VECTOR *func_800264A0(VECTOR *out, VECTOR *a, VECTOR *b);
VECTOR *vecCross(VECTOR *out, VECTOR *a, VECTOR *b);
VECTOR *vecCross12(VECTOR *out, VECTOR *a, VECTOR *b);

EXTERN_C_END

#endif /* DTBE_MATH_VECTOR_OPS_H */
