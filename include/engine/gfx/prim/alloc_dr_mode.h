#ifndef DTBE_GFX_PRIM_ALLOC_DR_MODE_H
#define DTBE_GFX_PRIM_ALLOC_DR_MODE_H

/* Taking a DR_MODE from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

DR_MODE *allocDrMode(s32 tpage, RECT *tw);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_DR_MODE_H */
