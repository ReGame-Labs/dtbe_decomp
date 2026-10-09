#ifndef DTBE_MATH_RANDOM_H
#define DTBE_MATH_RANDOM_H

/* The Mersenne Twister (MT19937) random number generator, and the game's own. */

#include "common.h"

EXTERN_C_BEGIN

/* Mersenne Twister (MT19937) random number generator state */
#define MT_N 624
#define MT_M 397

typedef struct {
    /* 0x000 */ u32 mt[MT_N];
    /* 0x9C0 */ s32 mti; /* MT_N + 1: not seeded yet */
} MersenneTwister;

/* the random numbers of the game */
extern MersenneTwister RANDOM;

MersenneTwister *mersenneTwisterInit(MersenneTwister *mt);
void mersenneTwisterSeed(MersenneTwister *mt, u32 seed);

u32 mersenneTwisterGenerate(MersenneTwister *mt);

EXTERN_C_END

#endif /* DTBE_MATH_RANDOM_H */
