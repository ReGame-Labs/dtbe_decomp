#include "common.h"
#include "engine/math/trig.h"
#include "libgte.h"

/* getTan (returns the tangent of an angle in 4096ths of a turn, in
 * 4.12: sin / cos from COS_SIN_TABLE, through divide12's divide inlined,
 * with gte_leadingZeros) is compiled, but has the same divide as
 * divide12: no divide-by-zero break and the div before the test of the
 * denominator, which the C does not give (see math/divide.c). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/trig", getTan);

/* Returns the arc cosine of cos (4.12 fixed point) as an angle in 4096ths of
 * a turn, clamping cos to -1..1. */
s32 getArcCos(s32 cos) {
    if (cos > ONE) {
        return 0;
    }
    if (cos < -ONE) {
        return ONE / 2; /* half a turn */
    }
    return ARC_COS_TABLE[cos + ONE];
}
