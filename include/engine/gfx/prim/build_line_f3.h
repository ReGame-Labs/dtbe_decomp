#ifndef DTBE_GFX_PRIM_BUILD_LINE_F3_H
#define DTBE_GFX_PRIM_BUILD_LINE_F3_H

/* Projecting shapes into LINE_F3, POLY_G4, POLY_FT3 and POLY_G3 packets. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildLineF3(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyG4(void *shape, void *packet, u_long *ot, s32 shift);

void *buildPolyFT3(void *shape, void *packet, u_long *ot, s32 shift);
void *buildPolyG3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_F3_H */
