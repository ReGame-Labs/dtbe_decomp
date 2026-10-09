#ifndef DTBE_MATH_RANDOM_H
#define DTBE_MATH_RANDOM_H

/* The Mersenne Twister (MT19937) random number generator, and the game's own. */

#include "common.h"

EXTERN_C_BEGIN

/* the state size of MT19937 in words, and its middle offset */
#define MT_N 624
#define MT_M 397

/* Mersenne Twister (MT19937) random number generator state */
typedef struct {
    /* 0x000 */ u32 mt[MT_N];
    /* 0x9C0 */ s32 mti; /* MT_N + 1: not seeded yet */
} MersenneTwister;

/* the random numbers of the game */
extern MersenneTwister RANDOM;

MersenneTwister *mersenneTwisterInit(MersenneTwister *mersenneTwister);
void mersenneTwisterSeed(MersenneTwister *mersenneTwister, u32 seed);

u32 mersenneTwisterGenerate(MersenneTwister *mersenneTwister);

EXTERN_C_END

#endif /* DTBE_MATH_RANDOM_H */
