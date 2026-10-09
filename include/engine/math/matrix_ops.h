#ifndef DTBE_MATH_MATRIX_OPS_H
#define DTBE_MATH_MATRIX_OPS_H

/* The simple matrix operations: copies, identity, scales, translations, in-place forms. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

MATRIX *func_80026588(MATRIX *m);
MATRIX *func_80026590(MATRIX *m);
long *matrixGetTranslation(MATRIX *m);
MATRIX *matrixCopy(MATRIX *dst, MATRIX *src);

MATRIX *matrixMulInPlace(MATRIX *m, MATRIX *n);
MATRIX *matrixInitIdentity(MATRIX *m);
MATRIX *matrixInitScale(MATRIX *m, s32 sx, s32 sy, s32 sz);
MATRIX *matrixInitTranslation(MATRIX *m, s32 x, s32 y, s32 z);
MATRIX *matrixInitTranslationVec(MATRIX *m, VECTOR *v);
MATRIX *matrixSetScale(MATRIX *m, s32 sx, s32 sy, s32 sz);
MATRIX *matrixSetTranslation(MATRIX *m, s32 x, s32 y, s32 z);
MATRIX *matrixSetScaleVec(MATRIX *m, VECTOR *scale);
MATRIX *matrixSetScaleSvec(MATRIX *m, SVECTOR *scale);
MATRIX *matrixSetTranslationVec(MATRIX *m, VECTOR *v);
MATRIX *matrixSetTranslationSvec(MATRIX *m, SVECTOR *v);
MATRIX *matrixClearTranslation(MATRIX *m);
MATRIX *matrixScaleVec(MATRIX *m, VECTOR *scale);

MATRIX *matrixScaleSvec(MATRIX *m, SVECTOR *scale);
MATRIX *matrixTranslateVec(MATRIX *m, VECTOR *v);
MATRIX *matrixTranslateSvec(MATRIX *m, SVECTOR *v);
MATRIX *matrixInvertRigidInPlace(MATRIX *m);
MATRIX *matrixInvertInPlace(MATRIX *m);
void setGteMatrix(MATRIX *m);

EXTERN_C_END

#endif /* DTBE_MATH_MATRIX_OPS_H */
