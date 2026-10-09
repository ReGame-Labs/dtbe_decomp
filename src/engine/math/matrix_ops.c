#include "common.h"
#include "engine/math/matrix_ops.h"
#include "engine/gfx/lights.h"
#include "engine/math/matrix.h"
#include "gte.h"

/* Returns m unchanged. */
MATRIX *func_80026588(MATRIX *m) {
    return m;
}

/* Returns m unchanged. */
MATRIX *func_80026590(MATRIX *m) {
    return m;
}

/* Returns the translation of m. */
long *matrixGetTranslation(MATRIX *m) {
    return m->t;
}

/* Copies a MATRIX as a block move (four loads then four stores), which GCC
 * 2.95.2 does not emit for 32 bytes (see "Block moves" in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix_ops", matrixCopy);

/* Multiplies m by n (the rotation, and n's translation through m) and
 * returns m. */
MATRIX *matrixMulInPlace(MATRIX *m, MATRIX *n) {
    matrixMul(m, m, n);
    return m;
}

/* The matrices below are written a word, two elements, at a time, as the
 * game did: the halfword after each diagonal element is a zero. */

/* Sets m to the identity, without translation. */
MATRIX *matrixInitIdentity(MATRIX *m) {
    MatrixWords *words = (MatrixWords *)m;

    words->words[0] = ONE;
    words->words[1] = 0;
    words->words[2] = ONE;
    words->words[3] = 0;
    words->words[4] = ONE;
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Sets m to a scale by sx, sy and sz (4.12 fixed point), without
 * translation. */
MATRIX *matrixInitScale(MATRIX *m, s32 sx, s32 sy, s32 sz) {
    MatrixWords *words = (MatrixWords *)m;

    words->words[0] = (u16)sx;
    words->words[1] = 0;
    words->words[2] = (u16)sy;
    words->words[3] = 0;
    words->words[4] = (u16)sz;
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Sets m to a translation by (x, y, z). */
MATRIX *matrixInitTranslation(MATRIX *m, s32 x, s32 y, s32 z) {
    MatrixWords *words = (MatrixWords *)m;

    words->words[0] = ONE;
    words->words[1] = 0;
    words->words[2] = ONE;
    words->words[3] = 0;
    words->words[4] = ONE;
    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Sets m to a translation by v. */
MATRIX *matrixInitTranslationVec(MATRIX *m, VECTOR *v) {
    MatrixWords *words = (MatrixWords *)m;
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    words->words[0] = ONE;
    words->words[1] = 0;
    words->words[2] = ONE;
    words->words[3] = 0;
    words->words[4] = ONE;
    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Sets the 3x3 part of m to a scale by sx, sy and sz. */
MATRIX *matrixSetScale(MATRIX *m, s32 sx, s32 sy, s32 sz) {
    MatrixWords *words = (MatrixWords *)m;

    words->words[0] = (u16)sx;
    words->words[1] = 0;
    words->words[2] = (u16)sy;
    words->words[3] = 0;
    words->words[4] = (u16)sz;
    return m;
}

/* Sets the translation of m to (x, y, z). */
MATRIX *matrixSetTranslation(MATRIX *m, s32 x, s32 y, s32 z) {
    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Sets the 3x3 part of m to a scale by the components of scale. */
MATRIX *matrixSetScaleVec(MATRIX *m, VECTOR *scale) {
    MatrixWords *words = (MatrixWords *)m;
    s32 sy = scale->vy;
    s32 sz = scale->vz;
    u16 sx = scale->vx;

    words->words[1] = 0;
    words->words[3] = 0;
    words->words[0] = sx;
    words->words[2] = (u16)sy;
    words->words[4] = (u16)sz;
    return m;
}

/* Sets the 3x3 part of m to a scale by the components of scale. */
MATRIX *matrixSetScaleSvec(MATRIX *m, SVECTOR *scale) {
    MatrixWords *words = (MatrixWords *)m;
    s32 sy = scale->vy;
    s32 sz = scale->vz;
    u16 sx = scale->vx;

    words->words[1] = 0;
    words->words[3] = 0;
    words->words[0] = sx;
    words->words[2] = (u16)sy;
    words->words[4] = (u16)sz;
    return m;
}

/* Sets the translation of m to v. */
MATRIX *matrixSetTranslationVec(MATRIX *m, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Sets the translation of m to v. */
MATRIX *matrixSetTranslationSvec(MATRIX *m, SVECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Clears the translation of m. */
MATRIX *matrixClearTranslation(MATRIX *m) {
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Scales the 3x3 part of m by the components of scale and returns m. */
MATRIX *matrixScaleVec(MATRIX *m, VECTOR *scale) {
    return matrixScale(m, scale->vx, scale->vy, scale->vz);
}

/* Scales the 3x3 part of m by the components of scale and returns m. */
MATRIX *matrixScaleSvec(MATRIX *m, SVECTOR *scale) {
    return matrixScale(m, scale->vx, scale->vy, scale->vz);
}

/* Moves m by v, given in m's own axes, and returns m. */
MATRIX *matrixTranslateVec(MATRIX *m, VECTOR *v) {
    return matrixTranslate(m, v->vx, v->vy, v->vz);
}

/* Moves m by v, given in m's own axes, and returns m. */
MATRIX *matrixTranslateSvec(MATRIX *m, SVECTOR *v) {
    return matrixTranslate(m, v->vx, v->vy, v->vz);
}

/* Inverts m, a rotation and a translation, in place. */
MATRIX *matrixInvertRigidInPlace(MATRIX *m) {
    return matrixInvertRigid(m, m);
}

/* Inverts m in place. */
MATRIX *matrixInvertInPlace(MATRIX *m) {
    return matrixInvert(m, m);
}

/* Loads m, rotation and translation, into the GTE. */
void setGteMatrix(MATRIX *m) {
    gte_setMatrix(m);
}
