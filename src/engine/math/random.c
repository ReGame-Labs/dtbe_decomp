#include "common.h"
#include "engine/math/random.h"

/* the Mersenne Twister's constants, as in Matsumoto and Nishimura's
 * mt19937.c */
#define MT_MATRIX_A 0x9908B0DF   /* the twist matrix's last row */
#define MT_UPPER_MASK 0x80000000 /* the most significant w-r bits */
#define MT_LOWER_MASK 0x7FFFFFFF /* the least significant r bits */
#define MT_DEFAULT_SEED 4357
#define MT_SEED_MULTIPLIER 69069
#define MT_TEMPERING_MASK_B 0x9D2C5680
#define MT_TEMPERING_MASK_C 0xEFC60000

/* Draws the next random number (genrand); the first time, seeds the
 * generator with the default seed (mersenneTwisterSeed inline). */
u32 mersenneTwisterGenerate(MersenneTwister *mt) {
    /* what the twist adds by the low bit of y (D_800643C0) */
    static u32 mag01[2] = { 0, MT_MATRIX_A };
    s32 mti = mt->mti;
    u32 y;

    if (mti >= MT_N) {
        s32 kk;

        if (mti == MT_N + 1) {
            /* not seeded yet: mersenneTwisterSeed(mt, MT_DEFAULT_SEED) */
            s32 i;

            mt->mt[0] = MT_DEFAULT_SEED;
            for (i = 1; i < MT_N; i++) {
                mt->mt[i] = MT_SEED_MULTIPLIER * mt->mt[i - 1];
            }
            mt->mti = i;
        }
        for (kk = 0; kk < MT_N - MT_M; kk++) {
            y = (mt->mt[kk] & MT_UPPER_MASK) | (mt->mt[kk + 1] & MT_LOWER_MASK);
            mt->mt[kk] = mt->mt[kk + MT_M] ^ (y >> 1) ^ mag01[y & 1];
        }
        for (; kk < MT_N - 1; kk++) {
            y = (mt->mt[kk] & MT_UPPER_MASK) | (mt->mt[kk + 1] & MT_LOWER_MASK);
            mt->mt[kk] = mt->mt[kk + (MT_M - MT_N)] ^ (y >> 1) ^ mag01[y & 1];
        }
        y = (mt->mt[MT_N - 1] & MT_UPPER_MASK) | (mt->mt[0] & MT_LOWER_MASK);
        mt->mt[MT_N - 1] = mt->mt[MT_M - 1] ^ (y >> 1) ^ mag01[y & 1];
        mti = 0;
    }
    y = mt->mt[mti++];
    mt->mti = mti;
    y ^= y >> 11;
    y ^= (y << 7) & MT_TEMPERING_MASK_B;
    y ^= (y << 15) & MT_TEMPERING_MASK_C;
    y ^= y >> 18;
    return y;
}

/* Constructs a generator; it seeds itself on first use. */
MersenneTwister *mersenneTwisterInit(MersenneTwister *mt) {
    mt->mti = MT_N + 1;
    return mt;
}

/* Seeds the generator. */
void mersenneTwisterSeed(MersenneTwister *mt, u32 seed) {
    s32 i;

    mt->mt[0] = seed;
    for (i = 1; i < MT_N; i++) {
        mt->mt[i] = MT_SEED_MULTIPLIER * mt->mt[i - 1];
    }
    mt->mti = i;
}
