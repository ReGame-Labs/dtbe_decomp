#ifndef DTBE_GFX_PRIM_BUILD_LINE_G3_H
#define DTBE_GFX_PRIM_BUILD_LINE_G3_H

/* Projecting a shape's three vertices into a LINE_G3 packet. */

#include "common.h"
#include <sys/types.h>

EXTERN_C_BEGIN

void *buildLineG3(void *shape, void *packet, u_long *ot, s32 shift);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUILD_LINE_G3_H */
