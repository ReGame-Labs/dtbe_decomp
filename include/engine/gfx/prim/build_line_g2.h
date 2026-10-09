#ifndef DTBE_GFX_PRIM_BUILD_LINE_G2_H
#define DTBE_GFX_PRIM_BUILD_LINE_G2_H

/* Projecting shapes into LINE_G2, POLY_FT4 (also windowed) and POLY_GT4 packets. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

void *buildLineG2(void *shape, void *packet, u_long *ot, s32 shift);

void *buildPolyFT4(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyFT4Windowed(void *shape, void *packet, u_long *ot, s32 shift, RECT *tw);

void *buildPolyGT4(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_G2_H */
