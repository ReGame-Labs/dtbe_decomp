#include "common.h"
#include "engine/math/shuffle.h"
#include "engine/math/random.h"
#include "engine/system/memory.h"
#include "memory.h"
#include "vtable.h"

/* Sets the flag when value is 1, and clears it otherwise. */
void func_8002D3E0(Unk8002D3E0 *this, s32 value) {
    this->unk10 = value == 1;
}

/* Draws count distinct numbers below count from random, mapped through map
 * when it is not NULL. */
Shuffle *shuffleInit(Shuffle *this, MersenneTwister *random, u32 count, u8 *map) {
    u32 drawn[8]; /* one bit per number already drawn */
    u8 *out;
    u32 left;
    u32 i;

    memset(drawn, 0, sizeof(drawn));
    out = this->order = (u8 *)operatorVecNew(count);
    left = count;
    while (left--) {
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

/* Destroys the shuffle, freeing its order. */
void shuffleDestroy(Shuffle *this, s32 flags) {
    if (this->order != NULL) {
        operatorVecDelete(this->order);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The number drawn index-th. */
u8 shuffleGetNumber(Shuffle *this, s32 index) {
    return this->order[index];
}
