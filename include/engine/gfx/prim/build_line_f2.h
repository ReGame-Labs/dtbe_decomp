#ifndef DTBE_GFX_PRIM_BUILD_LINE_F2_H
#define DTBE_GFX_PRIM_BUILD_LINE_F2_H

/* Projecting a line into a LINE_F2 packet in the ordering table. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildLineF2(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_F2_H */
