#ifndef DTBE_GFX_PRIM_BUILD_LINE_F3_H
#define DTBE_GFX_PRIM_BUILD_LINE_F3_H

/* Projecting shapes into LINE_F3, POLY_G4, POLY_FT3 and POLY_G3 packets. */

#include "common.h"
#include <sys/types.h>
#include <libgte.h>

EXTERN_C_BEGIN

/* The shape buildPolyG4 projects: four corners and a color for each. */
typedef struct PolyG4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 colors[4];
} PolyG4Shape; /* size 0x24 */

void *buildLineF3(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyG4(void *shape, void *packet, u_long *ot, s32 shift);

void *buildPolyFT3(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyG3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_F3_H */
