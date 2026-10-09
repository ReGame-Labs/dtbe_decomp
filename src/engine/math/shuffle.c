#include "common.h"
#include "engine/math/shuffle.h"
#include "engine/math/lerp.h"
#include "engine/math/random.h"
#include "engine/system/memory.h"
#include "memory.h"

/* Sets the flag when value is 1, and clears it otherwise. */
void func_8002D3E0(Unk8002D3E0 *this, s32 value) {
    this->unk10 = value == 1;
}

/* Draws count distinct numbers below count from random (mersenneTwisterGenerate). */
Shuffle *shuffleInit(Shuffle *this, void *random, u32 count, u8 *map) {
    u32 drawn[8]; /* one bit per number already drawn */
    u8 *out;
    u32 n;
    u32 i;

    memset(drawn, 0, sizeof(drawn));
    out = this->order = (u8 *)operatorVecNew(count);
    n = count;
    while (n--) {
        do {
            i = mersenneTwisterGenerate(random) % count;
        } while (drawn[i >> 5] & (1 << (i & 31)));
        drawn[i >> 5] |= 1 << (i & 31);
        if (map != NULL) {
            i = map[i];
        }
        *out++ = i;
    }
    return this;
}

void shuffleDestroy(Shuffle *this, s32 flags) {
    if (this->order != NULL) {
        operatorVecDelete(this->order);
    }
    if (flags & 1) {
        operatorDelete(this);
    }
}

u8 shuffleGetNumber(Shuffle *this, s32 index) {
    return this->order[index];
}

Lerp *lerpInit(Lerp *this, s32 value) {
    lerpStart(this, value, 0);
    return this;
}

/* Advances one step and returns the new value. */
s32 lerpUpdate(Lerp *lerp) {
    s32 value = lerp->value;
    s32 t;

    if (!lerpIsDone(lerp)) {
        lerp->time++;
        t = (lerp->time << 12) / (lerp->duration + 1);
        value = (lerp->target * t + lerp->start * (0x1000 - t)) >> 12;
        lerp->value = value;
    }
    return value;
}
