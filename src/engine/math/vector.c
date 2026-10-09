#include "common.h"
#include "engine/math/vector.h"
#include "engine/math/divide.h"
#include "gte.h"
#include "psyq.h"

/* vecNormalize shifts the components down until the largest has 18 leading
 * zeros (sign bits), small enough for the GTE's 16-bit IR registers. */
#define UNIT_LEADING_ZEROS 18

/* Writes v scaled to a length of 1.0 (4.12 fixed point) to unit, and returns
 * unit. The GTE squares the components, shifted down to UNIT_LEADING_ZEROS
 * leading zeros at least. shift holds the fewest leading zeros before it
 * becomes the shift: the game keeps both in one register. */
VECTOR *vecNormalize(VECTOR *unit, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;
    s32 zeros;
    s32 shift;
    s32 squareLength;
    s32 length;

    gte_leadingZeros(x, shift);
    gte_leadingZeros(y, zeros);
    if (zeros < shift) {
        shift = zeros;
    }
    gte_leadingZeros(z, zeros);
    if (zeros < shift) {
        shift = zeros;
    }
    shift = UNIT_LEADING_ZEROS - shift;
    if (shift > 0) {
        x >>= shift;
        y >>= shift;
        z >>= shift;
    }
    gte_loadIR(x, y, z);
    gte_square12();
    gte_sumMac(squareLength);
    length = func_80053D50(squareLength);
    x = divide12(x, length);
    y = divide12(y, length);
    z = divide12(z, length);
    unit->vx = x;
    unit->vy = y;
    unit->vz = z;
    return unit;
}

/* Returns the length of v. */
s32 vecGetLength(VECTOR *v) {
    return SquareRoot0(v->vx * v->vx + v->vy * v->vy + v->vz * v->vz);
}

/* Returns the length of v, its components being in 4.12 fixed point. */
s32 vecGetLength12(VECTOR *v) {
    return func_80053D50((v->vx * v->vx + v->vy * v->vy + v->vz * v->vz) >> 12);
}

/* Returns the square of the length of v, its components and the result in
 * 16.16 fixed point. */
s32 vecGetLengthSquared16(VECTOR *v) {
    s32 xx = ((s64)v->vx * v->vx) >> 16;
    s32 yy = ((s64)v->vy * v->vy) >> 16;
    s32 zz = ((s64)v->vz * v->vz) >> 16;

    return xx + yy + zz;
}

/*
 * Sets out to the point t (4.12 fixed point) of the way from a to b, from
 * 64-bit products. Compiled C, not handwritten: GCC's split 64-bit shifts are
 * there, dead high words included. The C computing the six products in
 * locals before storing comes out in other registers: the original moves t
 * from a3 to v0 first and needs only s0. Why is not known.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/vector", vecLerp);

/* Returns the dot product of a and b. */
s32 vecDot(VECTOR *a, VECTOR *b) {
    return a->vx * b->vx + a->vy * b->vy + a->vz * b->vz;
}

/* Returns the dot product of a and b in 4.12 fixed point. */
s32 vecDot12(VECTOR *a, VECTOR *b) {
    return (a->vx * b->vx + a->vy * b->vy + a->vz * b->vz) >> 12;
}
