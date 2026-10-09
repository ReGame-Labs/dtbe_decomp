#include "common.h"
#include "engine/math/quaternion.h"
#include "engine/math/divide.h"
#include "engine/math/matrix.h"
#include "engine/math/sin_cos.h"
#include "gte.h"
#include "psyq.h"

/* Sets q to the rotation by angle about axis, a unit vector in 4.12 fixed
 * point of which only the low 16 bits of each component are read. The game
 * takes the products unsigned: only the low 16 bits of each shifted product
 * are kept, where the sign makes no difference. */
Quaternion *quaternionInitRotationAxis(Quaternion *q, VECTOR *axis, s32 angle) {
    u32 x;
    u32 y;
    u32 z;
    s16 w;

    angle /= 2;
    x = (s16)axis->vx * COS_SIN_TABLE[angle & 0xFFF].sin;
    y = (s16)axis->vy * COS_SIN_TABLE[angle & 0xFFF].sin;
    z = (s16)axis->vz * COS_SIN_TABLE[angle & 0xFFF].sin;
    w = COS_SIN_TABLE[angle & 0xFFF].cos;
    q->x = x >> 12;
    q->y = y >> 12;
    q->w = w;
    q->z = z >> 12;
    return q;
}

/* Sets q to the rotation by angle about the x axis. */
Quaternion *quaternionInitRotationX(Quaternion *q, s32 angle) {
    s16 sin;
    s16 cos;

    angle /= 2;
    sin = COS_SIN_TABLE[angle & 0xFFF].sin;
    cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    q->y = 0;
    q->z = 0;
    q->x = sin;
    q->w = cos;
    return q;
}

/* Sets q to the rotation by angle about the y axis. */
Quaternion *quaternionInitRotationY(Quaternion *q, s32 angle) {
    s16 sin;
    s16 cos;

    angle /= 2;
    sin = COS_SIN_TABLE[angle & 0xFFF].sin;
    cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    q->x = 0;
    q->z = 0;
    q->y = sin;
    q->w = cos;
    return q;
}

/* Sets q to the rotation by angle about the z axis. */
Quaternion *quaternionInitRotationZ(Quaternion *q, s32 angle) {
    s16 sin;
    s16 cos;

    angle /= 2;
    sin = COS_SIN_TABLE[angle & 0xFFF].sin;
    cos = COS_SIN_TABLE[angle & 0xFFF].cos;
    q->x = 0;
    q->y = 0;
    q->z = sin;
    q->w = cos;
    return q;
}

/* Sets out to q divided by its length. The squares are summed on the GTE in
 * 4.12, so the square root of the sum is the length in 4.12 too. */
Quaternion *quaternionNormalize(Quaternion *out, Quaternion *q) {
    s32 x = q->x;
    s32 y = q->y;
    s32 z = q->z;
    s32 w = q->w;
    s32 lengthSquared;
    s32 length;

    gte_sumSquares4(x, y, z, w, lengthSquared);
    length = func_80053D50(lengthSquared);
    x = divide12(x, length);
    y = divide12(y, length);
    z = divide12(z, length);
    w = divide12(w, length);
    out->x = x;
    out->y = y;
    out->z = z;
    out->w = w;
    return out;
}

/* Sets dst to the conjugate of q, the inverse of a unit quaternion's
 * rotation. q is read whole first, so dst may be q. */
Quaternion *quaternionConjugate(Quaternion *dst, Quaternion *q) {
    s16 x = q->x;
    s16 y = q->y;
    s16 z = q->z;
    s16 w = q->w;

    dst->x = -x;
    dst->y = -y;
    dst->z = -z;
    dst->w = w;
    return dst;
}

/* quaternionSlerp (out = the spherical interpolation from a to b by t) differs
 * only in the stores to out: the game copies out from $v0, the return
 * value, where this compiler reloads it from the stack. */
INCLUDE_ASM("asm/jp/main/nonmatchings/math/quaternion", quaternionSlerp);

/* Sets q to the rotation of the rotation matrix m. Each element of q comes
 * from the trace or from the largest diagonal element: root is the square
 * root of 1 + that sum, twice the element it gives, and the other three come
 * from sums or differences of the off-diagonal elements over twice root. */
Quaternion *quaternionInitFromMatrix(Quaternion *q, MATRIX *m) {
    s32 m00 = m->m[0][0];
    s32 m11 = m->m[1][1];
    s32 m22 = m->m[2][2];
    s32 trace = m00 + m11 + m22;
    s32 i;
    u32 root;
    s16 scale;
    s32 x;
    s32 y;
    s32 z;
    s32 w;

    if (trace != 0) {
        root = func_80053D50(trace + ONE);
        w = root >> 1;
        scale = ONE * ONE / 2 / root;
        x = (s16)(((s16)(m->m[2][1] - m->m[1][2]) * scale) >> 12);
        y = (s16)(((s16)(m->m[0][2] - m->m[2][0]) * scale) >> 12);
        z = (s16)(((s16)(m->m[1][0] - m->m[0][1]) * scale) >> 12);
    } else {
        /* the index of the largest diagonal element */
        i = 0;
        if (m00 < m11) {
            i = 1;
        }
        if (m->m[i][i] < m22) {
            i = 2;
        }
        switch (i) {
        case 0:
        default:
            root = func_80053D50(m00 - (m11 + m22) + ONE);
            x = root >> 1;
            if (root != 0) {
                root = ONE * ONE / 2 / root;
            }
            y = (s16)(((s16)(m->m[1][0] + m->m[0][1]) * (s16)root) >> 12);
            z = (s16)(((s16)(m->m[2][0] + m->m[0][2]) * (s16)root) >> 12);
            w = (s16)(((s16)(m->m[2][1] - m->m[1][2]) * (s16)root) >> 12);
            break;
        case 1:
            root = func_80053D50(m11 - (m22 + m00) + ONE);
            y = root >> 1;
            if (root != 0) {
                root = ONE * ONE / 2 / root;
            }
            x = (s16)(((s16)(m->m[0][1] + m->m[1][0]) * (s16)root) >> 12);
            z = (s16)(((s16)(m->m[2][1] + m->m[1][2]) * (s16)root) >> 12);
            w = (s16)(((s16)(m->m[0][2] - m->m[2][0]) * (s16)root) >> 12);
            break;
        case 2:
            root = func_80053D50(m22 - (m00 + m11) + ONE);
            z = root >> 1;
            if (root != 0) {
                root = ONE * ONE / 2 / root;
            }
            x = (s16)(((s16)(m->m[0][2] + m->m[2][0]) * (s16)root) >> 12);
            y = (s16)(((s16)(m->m[1][2] + m->m[2][1]) * (s16)root) >> 12);
            w = (s16)(((s16)(m->m[1][0] - m->m[0][1]) * (s16)root) >> 12);
            break;
        }
    }
    q->x = x;
    q->y = y;
    q->z = z;
    q->w = w;
    return q;
}

/* Sets m to the rotation of q, without translation. */
void quaternionGetMatrix(Quaternion *q, MATRIX *m) {
    matrixInitFromQuaternion(m, q);
}

/* Returns the product of a and b, in 4.12 fixed point, taken in 64 bits. */
static inline s32 mul12(s32 a, s32 b) {
    return ((s64)a * b) >> 12;
}

/* Sets out to the product of the quaternions a and b: the rotation by b, then
 * by a. out may be a or b. */
Quaternion *quaternionMul(Quaternion *out, Quaternion *a, Quaternion *b) {
    s32 ax = a->x;
    s32 ay = a->y;
    s32 az = a->z;
    s32 aw = a->w;
    s32 bx = b->x;
    s32 by = b->y;
    s32 bz = b->z;
    s32 bw = b->w;

    out->x = mul12(aw, bx) + mul12(bw, ax) + mul12(ay, bz) - mul12(az, by);
    out->y = mul12(aw, by) + mul12(bw, ay) + mul12(az, bx) - mul12(ax, bz);
    out->z = mul12(aw, bz) + mul12(bw, az) + mul12(ax, by) - mul12(ay, bx);
    out->w = mul12(aw, bw) - mul12(ax, bx) - mul12(ay, by) - mul12(az, bz);
    return out;
}

/* Rotates in by q into out. */
void quaternionRotateVec(Quaternion *q, VECTOR *out, VECTOR *in) {
    MATRIX m;

    matrixInitFromQuaternion(&m, q);
    matrixTransformVec(&m, out, in);
}
