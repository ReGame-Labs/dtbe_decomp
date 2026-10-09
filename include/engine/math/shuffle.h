#ifndef DTBE_MATH_SHUFFLE_H
#define DTBE_MATH_SHUFFLE_H

/* Random permutations of the numbers below a count, and stepping a Lerp. */

#include "common.h"
#include "engine/math/lerp.h"

EXTERN_C_BEGIN

/* The object of func_8002D3E0, which nothing in the executable calls. */
typedef struct Unk8002D3E0 {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u32 unk10 : 1;
} Unk8002D3E0;

/* A random permutation of the numbers below a count, optionally mapped
 * through a table. */
typedef struct {
    u8 *order;
} Shuffle;

void func_8002D3E0(Unk8002D3E0 *unk8002D3E0, s32 value);
Shuffle *shuffleInit(Shuffle *shuffle, void *random, u32 count, u8 *map);
void shuffleDestroy(Shuffle *shuffle, s32 flags);
u8 shuffleGetNumber(Shuffle *shuffle, s32 index);
Lerp *lerpInit(Lerp *lerp, s32 value);
s32 lerpUpdate(Lerp *lerp);

EXTERN_C_END

#endif /* DTBE_MATH_SHUFFLE_H */
