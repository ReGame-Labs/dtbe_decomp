#ifndef DTBE_GFX_PRIM_ALLOC_DR_AREA_H
#define DTBE_GFX_PRIM_ALLOC_DR_AREA_H

/* Taking a DR_AREA from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

DR_AREA *allocDrArea(RECT *area);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_DR_AREA_H */
