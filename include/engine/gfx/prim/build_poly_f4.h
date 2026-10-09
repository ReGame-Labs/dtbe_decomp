#ifndef DTBE_GFX_PRIM_BUILD_POLY_F4_H
#define DTBE_GFX_PRIM_BUILD_POLY_F4_H

/* Projecting shapes into POLY_F4 packets, or a TILE at their vertex. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildPolyF4(void *shape, void *packet, u_long *ot, s32 shift);
void *buildTile(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_POLY_F4_H */
