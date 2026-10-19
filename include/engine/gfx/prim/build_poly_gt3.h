#ifndef DTBE_GFX_PRIM_BUILD_POLY_GT3_H
#define DTBE_GFX_PRIM_BUILD_POLY_GT3_H

/* Projecting a textured, Gouraud-shaded triangle into a POLY_GT3 packet. */

#include "common.h"
#include <sys/types.h>
#include <libgte.h>

EXTERN_C_BEGIN

/* The triangle buildPolyGT3 projects: flags (as LineG2Shape's), its three corners, their colors, and the uv, clut
 * and tpage words of the POLY_GT3 (the layout of the packet's uv0/clut, uv1/tpage and uv2 words). */
typedef struct PolyGT3Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[3];
    /* 0x10 */ u32 colors[3];
    /* 0x1C */ u16 uv0, clut;
    /* 0x20 */ u16 uv1, tpage;
    /* 0x24 */ u16 uv2, pad26;
} PolyGT3Shape; /* size 0x28 */

void *buildPolyGT3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_POLY_GT3_H */
