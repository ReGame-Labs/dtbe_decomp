#include "common.h"
#include "engine/math/matrix.h"
#include "engine/math/quaternion.h"
#include "engine/math/sin_cos.h"
#include "inline_c.h"
#include "gte.h"

/* Swaps the two words at a with the two at b, by xor, without a third
 * buffer. */
#define SWAP_WORD_PAIRS(a, b)       \
    {                               \
        u32 a0 = (a)[0];            \
        u32 b0 = (b)[0];            \
        u32 a1 = (a)[1];            \
        u32 b1 = (b)[1];            \
                                    \
        b0 ^= a0;                   \
        b1 ^= a1;                   \
        a0 ^= b0;                   \
        a1 ^= b1;                   \
        b0 ^= a0;                   \
        b1 ^= a1;                   \
        (a)[0] = a0;                \
        (b)[0] = b0;                \
        (a)[1] = a1;                \
        (b)[1] = b1;                \
    }

/* Swaps four words at a with four at b. */
void swapFourWords(u32 *a, u32 *b) {
    SWAP_WORD_PAIRS(a, b);
    SWAP_WORD_PAIRS(a + 2, b + 2);
}

/* Swaps two words at a with two at b. */
void swapTwoWords(u32 *a, u32 *b) {
    SWAP_WORD_PAIRS(a, b);
}

/* Sets out to m in, translation included, in is a VECTOR. Handwritten: m goes
 * in through fixed $8-$11 and in by the high/low split in $8-$13, the same
 * code as one turn of matrixTransformVecs's handwritten loop; nothing in it is
 * GCC's. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixTransformVec);

/* Handwritten: the count is tested by addiu $a3, -1; li $v0, -1; beq, not
 * as GCC tests a count-down loop (against $zero, the decrement in the
 * branch's delay slot). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixTransformVecs);

/* Meant to set out to m in, translation included; gte_setMatrixUnsafe
 * overwrites m with its first word, so the GTE gets the words at that
 * address instead. Compiled: the bug is GCC's register allocation. */
void matrixTransformSvec(MATRIX *m, VECTOR *out, SVECTOR *in) {
    gte_setMatrixUnsafe(m);
    gte_ldv0(in);
    gte_rotTransV0();
    gte_stlvnl(out);
}

/* Handwritten: the same count test as matrixTransformVecs, and the loop
 * software-pipelined (the next vector loaded and the pointers moved before
 * the stores, which use offsets of -16). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixTransformSvecs);

/* Sets out to the 3x3 part of m times in, in is a VECTOR. Handwritten: fixed
 * $8-$13 throughout, the same code as one turn of matrixRotateVecs's handwritten
 * loop; nothing in it is GCC's. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixRotateVec);

/* Handwritten: the same count test as matrixTransformVecs. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixRotateVecs);

/* Rotates m by angle about axis: m = m R, translation kept. */
MATRIX *matrixRotateAxis(MATRIX *m, VECTOR *axis, s32 angle) {
    MATRIX rotation;

    gte_mulMatrix(m, matrixSetRotationAxis(&rotation, axis, angle));
    return m;
}

/* Rotates m by angle about its x axis: m = m R, translation kept. */
MATRIX *matrixRotateX(MATRIX *m, s32 angle) {
    MATRIX rotation;

    gte_mulMatrix(m, matrixSetRotationX(&rotation, angle));
    return m;
}

/* Rotates m by angle about its y axis: m = m R, translation kept. */
MATRIX *matrixRotateY(MATRIX *m, s32 angle) {
    MATRIX rotation;

    gte_mulMatrix(m, matrixSetRotationY(&rotation, angle));
    return m;
}

/* Rotates m by angle about its z axis: m = m R, translation kept. */
MATRIX *matrixRotateZ(MATRIX *m, s32 angle) {
    MATRIX rotation;

    gte_mulMatrix(m, matrixSetRotationZ(&rotation, angle));
    return m;
}

/* Scales the columns of m's 3x3 part by sx, sy and sz (4.12): m = m S. */
MATRIX *matrixScale(MATRIX *m, s32 sx, s32 sy, s32 sz) {
    MatrixWords scale;

    scale.words[0] = (u16)sx;
    scale.words[1] = 0;
    scale.words[2] = (u16)sy;
    scale.words[3] = 0;
    scale.words[4] = (u16)sz;
    gte_mulMatrix(m, &scale.m);
    return m;
}

/* Moves m by (x, y, z), given in m's own axes. Left in asm as handwritten: the
 * fixed-register code ($8-$13) of matrixTransformVec, the translation of
 * matrixMul. It matches as C around a macro used once, but nothing in it
 * is only GCC's (the first move to $v0, m used through it, fits both). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixTranslate);

/* Sets out to the composition of a and b (out = a b, out's translation
 * a b->t + a->t), as PsyQ's CompMatrix. Handwritten: the translation and the
 * product are interleaved instruction by instruction in fixed registers;
 * apart from them there is only the return value's move, in the jr's delay
 * slot. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixMul);

/* matrixInitInverseRotationAxis (m = the rotation by -angle about axis,
 * without translation) differs only in register allocation (28 lines): the
 * game's registers need the cosine allocated before the axis's x, which in
 * the C outranks it in local-alloc (priorities about 3600 to 3100). Moving
 * the cosine's first use earlier or its last use later doesn't reorder them,
 * so the original's instructions came in another order before sched2. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixInitInverseRotationAxis);

/* Sets m to a rotation by angle about the x axis, without translation. */
MATRIX *matrixInitRotationX(MATRIX *m, s32 angle) {
    matrixSetRotationX(m, angle);
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Sets m to a rotation by angle about the y axis, without translation. */
MATRIX *matrixInitRotationY(MATRIX *m, s32 angle) {
    matrixSetRotationY(m, angle);
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Sets m to a rotation by angle about the z axis, without translation. */
MATRIX *matrixInitRotationZ(MATRIX *m, s32 angle) {
    matrixSetRotationZ(m, angle);
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* matrixInitLookAt (m = the view matrix of eye looking at target, in 4.12: z
 * the unit target - eye, x the unit up x z, up replaced by (z.vy, 0, 0) when
 * nearly parallel to z, y = z x x, t = -(x, y, z) . eye) is compiled. Its two
 * outer products come out in the game's registers from a macro that loads a's
 * elements in C (gte_outerProduct12 loads them in its asm, as
 * vecCross needs). What is left is where GCC keeps the vectors' stack
 * addresses: the game recomputes &x and &y at each use and holds &z in $s0,
 * then $s1, saving $s0-$s3; the C keeps all three in saved registers and needs
 * $s4. An asm with no outputs or clobbers for the outer products makes CSE
 * forget the addresses (65 lines left), but not the game's &z, built straight
 * into a0/a1 for vecNormalize and again for vecDot12; a Vec class with inline
 * members gives more differences. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixInitLookAt);

/* Sets out to the inverse of the rigid transform m: the transposed rotation
 * and the negated translation turned by it. The match depends on negated, a
 * copy of &translation: the game takes the address before the negation (addiu
 * $s1, $sp, 0x10) and keeps it in $s1 across TransposeMatrix, where
 * &translation written at the call, or the negation written through negated,
 * computes it after. matrixInvert also takes it first (it keeps it at
 * 0x40(sp)). */
MATRIX *matrixInvertRigid(MATRIX *out, MATRIX *m) {
    VECTOR translation;
    VECTOR *negated = &translation;

    translation.vx = -m->t[0];
    translation.vy = -m->t[1];
    translation.vz = -m->t[2];
    TransposeMatrix(m, out);
    /* t laid out as the vx, vy, vz of a VECTOR, all matrixRotateVec stores */
    matrixRotateVec(out, (VECTOR *)out->t, negated);
    return out;
}

/* matrixInvert copies a MATRIX as a block move (see "Block moves" in
 * TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/matrix", matrixInvert);

/* Sets m to the rotation of a unit quaternion, without translation. The
 * products are in 8.24 fixed point: >> 11 takes them back to 4.12 doubled. */
MATRIX *matrixInitFromQuaternion(MATRIX *m, Quaternion *q) {
    s32 yy = q->y * q->y;
    s32 zz = q->z * q->z;
    s32 xy = q->x * q->y;
    s32 wz = q->w * q->z;
    s32 xz = q->x * q->z;
    s32 wy = q->w * q->y;
    s32 xx = q->x * q->x;
    s32 yz = q->y * q->z;
    s32 wx = q->w * q->x;

    m->m[0][0] = ONE - ((yy + zz) >> 11);
    m->m[0][1] = (xy - wz) >> 11;
    m->m[0][2] = (xz + wy) >> 11;
    m->m[1][0] = (xy + wz) >> 11;
    m->m[1][1] = ONE - ((xx + zz) >> 11);
    m->m[1][2] = (yz - wx) >> 11;
    m->m[2][0] = (xz - wy) >> 11;
    m->m[2][1] = (yz + wx) >> 11;
    m->m[2][2] = ONE - ((xx + yy) >> 11);
    m->t[0] = 0;
    m->t[1] = 0;
    m->t[2] = 0;
    return m;
}

/* Sets the 3x3 part of m to the rotation by angle about axis: the rotation
 * of the quaternion quaternionInitRotationAxis makes, as matrixInitFromQuaternion turns it into a
 * matrix. */
MATRIX *matrixSetRotationAxis(MATRIX *m, VECTOR *axis, s32 angle) {
    s16 x, y, z, w;
    s32 yy, zz, xy, wz, xz, wy, xx, yz, wx;

    angle /= 2;
    x = ((s16)axis->vx * COS_SIN_TABLE[angle & 0xFFF].sin) >> 12;
    y = ((s16)axis->vy * COS_SIN_TABLE[angle & 0xFFF].sin) >> 12;
    z = ((s16)axis->vz * COS_SIN_TABLE[angle & 0xFFF].sin) >> 12;
    w = COS_SIN_TABLE[angle & 0xFFF].cos;
    yy = y * y;
    zz = z * z;
    xy = x * y;
    wz = w * z;
    xz = x * z;
    wy = w * y;
    xx = x * x;
    yz = y * z;
    wx = w * x;
    m->m[0][0] = ONE - ((yy + zz) >> 11);
    m->m[0][1] = (xy - wz) >> 11;
    m->m[0][2] = (xz + wy) >> 11;
    m->m[1][0] = (xy + wz) >> 11;
    m->m[1][1] = ONE - ((xx + zz) >> 11);
    m->m[1][2] = (yz - wx) >> 11;
    m->m[2][0] = (xz - wy) >> 11;
    m->m[2][1] = (yz + wx) >> 11;
    m->m[2][2] = ONE - ((xx + yy) >> 11);
    return m;
}

/* Sets the 3x3 part of m to a rotation by angle about the x axis. The
 * words write two elements at once, as the game did. */
MATRIX *matrixSetRotationX(MATRIX *m, s32 angle) {
    MatrixWords *words = (MatrixWords *)m;
    s16 cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    s16 sin = COS_SIN_TABLE[angle & 0xFFF].sin;

    words->words[0] = ONE;
    words->words[1] = 0;
    words->words[2] = (-sin << 16) | (u16)cos;
    words->words[3] = sin << 16;
    words->words[4] = (u16)cos;
    return m;
}

/* Sets the 3x3 part of m to a rotation by angle about the y axis. */
MATRIX *matrixSetRotationY(MATRIX *m, s32 angle) {
    MatrixWords *words = (MatrixWords *)m;
    s16 cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    s16 sin = COS_SIN_TABLE[angle & 0xFFF].sin;

    words->words[0] = (u16)cos;
    words->words[2] = ONE;
    words->words[1] = (u16)sin;
    words->words[3] = (u16)-sin;
    words->words[4] = (u16)cos;
    return m;
}

/* Sets the 3x3 part of m to a rotation by angle about the z axis. */
MATRIX *matrixSetRotationZ(MATRIX *m, s32 angle) {
    MatrixWords *words = (MatrixWords *)m;
    u16 cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    s16 sin = COS_SIN_TABLE[angle & 0xFFF].sin;

    words->words[3] = 0;
    words->words[4] = ONE;
    words->words[0] = (-sin << 16) | cos;
    words->words[1] = sin << 16;
    words->words[2] = cos;
    return m;
}
