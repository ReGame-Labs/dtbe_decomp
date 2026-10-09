#ifndef DTBE_MATH_MATRIX_H
#define DTBE_MATH_MATRIX_H

/* Matrices: transforming vectors, rotations, scaling, products, inverses and look-at. */

#include "common.h"
#include <libgte.h>
#include "engine/math/quaternion.h"

EXTERN_C_BEGIN

/* A matrix as eight words, to clear or set two of its elements at once. */
typedef union {
    /* 0x00 */ MATRIX m;
    /* 0x00 */ u32 words[8];
} MatrixWords;

void swapFourWords(u32 *a, u32 *b);
void swapTwoWords(u32 *a, u32 *b);
void matrixTransformVec(MATRIX *m, VECTOR *out, VECTOR *in);
/* transforms count vectors from in into out by m, translation included
 * (handwritten GTE) */
void matrixTransformVecs(MATRIX *m, VECTOR *out, VECTOR *in, s32 count);
void matrixTransformSvec(MATRIX *m, VECTOR *out, SVECTOR *in);
/* transforms count short vectors from in into out by m, translation
 * included (handwritten GTE) */
void matrixTransformSvecs(MATRIX *m, VECTOR *out, SVECTOR *in, s32 count);
void matrixRotateVec(MATRIX *m, VECTOR *out, VECTOR *in);
/* rotates count vectors from in into out by the 3x3 part of m (handwritten
 * GTE) */
void matrixRotateVecs(MATRIX *m, VECTOR *out, VECTOR *in, s32 count);
MATRIX *matrixRotateAxis(MATRIX *m, VECTOR *axis, s32 angle);
MATRIX *matrixRotateX(MATRIX *m, s32 angle);
MATRIX *matrixRotateY(MATRIX *m, s32 angle);
MATRIX *matrixRotateZ(MATRIX *m, s32 angle);
MATRIX *matrixScale(MATRIX *m, s32 sx, s32 sy, s32 sz);
MATRIX *matrixTranslate(MATRIX *m, s32 x, s32 y, s32 z);
MATRIX *matrixMul(MATRIX *out, MATRIX *a, MATRIX *b);
MATRIX *matrixInitInverseRotationAxis(MATRIX *m, VECTOR *axis, s32 angle);
MATRIX *matrixInitRotationX(MATRIX *m, s32 angle);
MATRIX *matrixInitRotationY(MATRIX *m, s32 angle);
MATRIX *matrixInitRotationZ(MATRIX *m, s32 angle);
MATRIX *matrixInitLookAt(MATRIX *m, VECTOR *eye, VECTOR *target, VECTOR *up);
MATRIX *matrixInvertRigid(MATRIX *out, MATRIX *m);
MATRIX *matrixInvert(MATRIX *out, MATRIX *m);
MATRIX *matrixInitFromQuaternion(MATRIX *m, Quaternion *q);
MATRIX *matrixSetRotationAxis(MATRIX *m, VECTOR *axis, s32 angle);
MATRIX *matrixSetRotationX(MATRIX *m, s32 angle);
MATRIX *matrixSetRotationY(MATRIX *m, s32 angle);
MATRIX *matrixSetRotationZ(MATRIX *m, s32 angle);

EXTERN_C_END

#endif /* DTBE_MATH_MATRIX_H */
