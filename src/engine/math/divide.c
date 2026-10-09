#include "common.h"
#include "engine/math/divide.h"

/* divide12 and divide16 return numerator / denominator in 4.12 and
 * 16.16 fixed point: (numerator << 12 or 16) / denominator, the shift cut to
 * the leading zeros of numerator (gte_leadingZeros) less one and the
 * denominator shifted down by the rest; 0 for a numerator of 0, 0x7FFFFFFF
 * or 0x80000000 by the sign of numerator when the denominator is or becomes 0.
 * Compiled C (one body, inlined with the shift 12 or 16 in $a3), but its div
 * has no divide-by-zero break and is issued before the C test of the
 * denominator, with the mflo after it. Every other division of the game has
 * the break, and these files have no other, so they were likely built with
 * -mno-check-zero-division: with it, the C (the quotient taken, then
 * `if (denominator == 0) return numerator < 0 ? 0x80000000 : 0x7FFFFFFF;`,
 * the leading zeros in their own variable) gives every instruction but one
 * thing: global-alloc puts the quotient in $v0 right after the div, where
 * the game keeps it in LO until the return (so its lui of 0x7FFFFFFF fills
 * the branch's delay slot). The quotient's pseudo has no preference for LO;
 * a div and mflo of their own (inline asm) would explain it. getTan has the
 * same divide inlined. The game also tests numerator from a copy in $v1,
 * which the C does not make. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/divide", divide12);

INCLUDE_ASM("asm/jp/main/nonmatchings/math/divide", divide16);
