#ifndef DTBE_GFX_PRIM_BUILD_POLY_F3_H
#define DTBE_GFX_PRIM_BUILD_POLY_F3_H

/* Projecting a flat triangle into a POLY_F3 packet in the ordering table. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildPolyF3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_POLY_F3_H */
