#include "common.h"
#include "engine/math/quaternion_ops.h"
#include "engine/math/quaternion.h"

/* Copies src to dst. */
QuaternionWords *quaternionCopy(QuaternionWords *dst, QuaternionWords *src) {
    u32 xy = src->words[0];
    u32 zw = src->words[1];

    dst->words[0] = xy;
    dst->words[1] = zw;
    return dst;
}

/* Sets q to the product q b. */
Quaternion *quaternionMulInPlace(Quaternion *q, Quaternion *b) {
    return quaternionMul(q, q, b);
}

/* Sets out to the product a b, through a copy, so out may be a or b. */
QuaternionWords *func_80026954(QuaternionWords *out, Quaternion *a, Quaternion *b) {
    QuaternionWords product;
    QuaternionWords *result = (QuaternionWords *)quaternionMul(&product.q, a, b);

    *out = *result;
    return out;
}

/* Sets q to the identity rotation (0, 0, 0, ONE). */
QuaternionWords *quaternionInitIdentity(QuaternionWords *q) {
    q->words[0] = 0;
    q->words[1] = ONE << 16; /* z 0, w ONE */
    return q;
}

/* Normalizes q. */
Quaternion *quaternionNormalizeInPlace(Quaternion *q) {
    return quaternionNormalize(q, q);
}

/* Sets q to its conjugate. */
Quaternion *quaternionConjugateInPlace(Quaternion *q) {
    return quaternionConjugate(q, q);
}

/* Turns q towards to by t (4.12), in place. */
void quaternionSlerpInPlace(Quaternion *q, Quaternion *to, s32 t) {
    quaternionSlerp(q, q, to, t);
}
