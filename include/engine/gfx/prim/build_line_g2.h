#ifndef DTBE_GFX_PRIM_BUILD_LINE_G2_H
#define DTBE_GFX_PRIM_BUILD_LINE_G2_H

/* Projecting shapes into LINE_G2, POLY_FT4 (also windowed) and POLY_GT4 packets. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

/* The line buildLineG2 projects: flags (bits 0-1 the semi-transparency rate, bit 3 semi-transparent: the
 * SPRITE_ABR_MASK and SPRITE_SEMI_TRANS bits of sprite.h), its two ends and their colors (0x808080 for none). */
typedef struct LineG2Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *ends[2];
    /* 0x0C */ u32 colors[2];
} LineG2Shape; /* size 0x14 */

void *buildLineG2(void *shape, void *packet, u_long *ot, s32 shift);

void *buildPolyFT4(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyFT4Windowed(void *shape, void *packet, u_long *ot, s32 shift, RECT *tw);

/* The shape buildPolyGT4 projects: flags (as LineG2Shape's), four corners, their colors and the uv, clut and tpage
 * words of the POLY_GT4. */
typedef struct PolyGT4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 colors[4];
    /* 0x24 */ u8 u0, v0;
    /* 0x26 */ u16 clut;
    /* 0x28 */ u8 u1, v1;
    /* 0x2A */ u16 tpage;
    /* 0x2C */ u8 u2, v2;
    /* 0x2E */ u16 pad2E;
    /* 0x30 */ u8 u3, v3;
    /* 0x32 */ u16 pad32;
} PolyGT4Shape; /* size 0x34 */

void *buildPolyGT4(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_G2_H */
