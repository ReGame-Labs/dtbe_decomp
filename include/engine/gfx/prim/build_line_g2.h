#ifndef DTBE_GFX_PRIM_BUILD_LINE_G2_H
#define DTBE_GFX_PRIM_BUILD_LINE_G2_H

/* Projecting shapes into LINE_G2, POLY_FT4 (also windowed) and POLY_GT4 packets. */

#include "common.h"

EXTERN_C_BEGIN

void *buildLineG2(void *arg0, void *arg1, s32 arg2, s32 arg3);

void *buildPolyFT4(void *arg0, void *arg1, s32 arg2, s32 arg3);
void *buildPolyFT4Windowed(void *arg0, void *arg1, s32 arg2, s32 arg3, void *arg4);

void *buildPolyGT4(void *arg0, void *arg1, s32 arg2, s32 arg3);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_G2_H */
