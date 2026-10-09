#ifndef DTBE_GFX_PRIM_BUILD_POLY_GT3_H
#define DTBE_GFX_PRIM_BUILD_POLY_GT3_H

/* Projecting a textured, Gouraud-shaded triangle into a POLY_GT3 packet. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildPolyGT3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_POLY_GT3_H */
