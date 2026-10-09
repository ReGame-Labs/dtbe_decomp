#ifndef DTBE_MATH_SHUFFLE_H
#define DTBE_MATH_SHUFFLE_H

/* Random permutations of the numbers below a count. */

#include "common.h"
#include "engine/math/random.h"

#ifdef __cplusplus

/*
 * A random permutation of the numbers below a count, optionally mapped
 * through a table. The C files see it as the struct in the #else part.
 */
class Shuffle {
public:
    /* 0x00 */ u8 *order;

    Shuffle(MersenneTwister *random, u32 count, u8 *map) __asm__("shuffleInit");
    ~Shuffle();
    u8 getNumber(s32 index) __asm__("shuffleGetNumber");
};

#else

typedef struct {
    /* 0x00 */ u8 *order;
} Shuffle;

#endif /* __cplusplus */

EXTERN_C_BEGIN

/* The object of func_8002D3E0, which nothing in the executable calls. */
typedef struct Unk8002D3E0 {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u32 unk10 : 1;
} Unk8002D3E0;

void func_8002D3E0(Unk8002D3E0 *unk8002D3E0, s32 value);
#ifndef __cplusplus
Shuffle *shuffleInit(Shuffle *shuffle, MersenneTwister *random, u32 count, u8 *map);
/* Shuffle's destructor, by g++'s name for it */
void shuffleDestroy(Shuffle *shuffle, s32 flags) __asm__("_._7Shuffle");
u8 shuffleGetNumber(Shuffle *shuffle, s32 index);
#endif

EXTERN_C_END

#endif /* DTBE_MATH_SHUFFLE_H */
