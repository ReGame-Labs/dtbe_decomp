#include "common.h"
#include "engine/math/vector_ops.h"
#include "engine/math/vector.h"
#include "gte.h"

/* Returns v unchanged. */
SVECTOR *func_800261A8(SVECTOR *v) {
    return v;
}

/* Returns v unchanged. */
SVECTOR *func_800261B0(SVECTOR *v) {
    return v;
}

/* Copies src to dst. */
SVectorWords *svecCopy(SVectorWords *dst, SVectorWords *src) {
    u32 xy = src->words[0];
    u32 zpad = src->words[1];

    dst->words[0] = xy;
    dst->words[1] = zpad;
    return dst;
}

/* Sets v to (x, y, z). */
SVECTOR *svecSet(SVECTOR *v, s16 x, s16 y, s16 z) {
    v->vx = x;
    v->vy = y;
    v->vz = z;
    return v;
}

/* Meant to set out to in scaled by t (4.12) on the GTE; the macro that
 * reads the result back has its mfc2s' operands swapped, so out gets in. */
void svecScale12(SVECTOR *out, SVECTOR *in, s16 t) {
    s32 x, y, z;

    gte_ldIr0SVector(t, in, x, y, z);
    gte_scaleByIr0_12();
    gte_stIrSVectorSwapped(out, x, y, z);
}

/* Meant to set out to in scaled by t, as an integer on the GTE; the macro that
 * reads the result back has its mfc2s' operands swapped, so out gets in. */
void svecScale(SVECTOR *out, SVECTOR *in, s16 t) {
    s32 x, y, z;

    gte_ldIr0SVector(t, in, x, y, z);
    gte_scaleByIr0();
    gte_stIrSVectorSwapped(out, x, y, z);
}

/* Returns v unchanged. */
VECTOR *func_80026284(VECTOR *v) {
    return v;
}

/* Returns v unchanged. */
VECTOR *func_8002628C(VECTOR *v) {
    return v;
}

/* Clears v, pad included. */
VECTOR *vecClear(VECTOR *v) {
    v->vx = 0;
    v->vy = 0;
    v->vz = 0;
    v->pad = 0;
    return v;
}

/* Copies the components of src to dst. */
VECTOR *vecCopy(VECTOR *dst, VECTOR *src) {
    s32 x = src->vx;
    s32 y = src->vy;
    s32 z = src->vz;

    dst->vx = x;
    dst->vy = y;
    dst->vz = z;
    return dst;
}

/* Sets v to (x, y, z). */
VECTOR *vecSet(VECTOR *v, s32 x, s32 y, s32 z) {
    v->vx = x;
    v->vy = y;
    v->vz = z;
    return v;
}

/* Sets v to (x, y, z) and its pad to pad. */
VECTOR *vecSetWithPad(VECTOR *v, s32 x, s32 y, s32 z, s32 pad) {
    v->vx = x;
    v->vy = y;
    v->vz = z;
    v->pad = pad;
    return v;
}

/* Adds b to a. */
VECTOR *vecAddInPlace(VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x += b->vx;
    y += b->vy;
    z += b->vz;
    a->vx = x;
    a->vy = y;
    a->vz = z;
    return a;
}

/* Sets out to a + b. */
VECTOR *vecAdd(VECTOR *out, VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x += b->vx;
    y += b->vy;
    z += b->vz;
    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* Subtracts b from a. */
VECTOR *vecSubInPlace(VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x -= b->vx;
    y -= b->vy;
    z -= b->vz;
    a->vx = x;
    a->vy = y;
    a->vz = z;
    return a;
}

/* Sets out to a - b. */
VECTOR *vecSub(VECTOR *out, VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x -= b->vx;
    y -= b->vy;
    z -= b->vz;
    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* Normalizes v in place. */
VECTOR *vecNormalizeInPlace(VECTOR *v) {
    return vecNormalize(v, v);
}

/* Returns the squared length of v. */
s32 vecGetLengthSquared(VECTOR *v) {
    return v->vx * v->vx + v->vy * v->vy + v->vz * v->vz;
}

/* Returns the squared length of v in 4.12 fixed point. */
u32 vecGetLengthSquared12(VECTOR *v) {
    u32 squared = v->vx * v->vx + v->vy * v->vy + v->vz * v->vz;

    return squared >> 12;
}

/* Sets out to the cross product a x b, in integers. */
VECTOR *func_80026478(VECTOR *out, VECTOR *a, VECTOR *b) {
    vecCross(out, a, b);
    return out;
}

/* Sets out to the cross product a x b, in 4.12 fixed point. */
VECTOR *func_800264A0(VECTOR *out, VECTOR *a, VECTOR *b) {
    vecCross12(out, a, b);
    return out;
}

/* Sets out to the cross product a x b, in integers. */
VECTOR *vecCross(VECTOR *out, VECTOR *a, VECTOR *b) {
    gte_outerProduct0(a, b, out);
    return out;
}

/* Sets out to the cross product a x b, in 4.12 fixed point. */
VECTOR *vecCross12(VECTOR *out, VECTOR *a, VECTOR *b) {
    gte_outerProduct12(a, b, out);
    return out;
}
