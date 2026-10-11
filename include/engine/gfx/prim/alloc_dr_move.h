#ifndef DTBE_GFX_PRIM_ALLOC_DR_MOVE_H
#define DTBE_GFX_PRIM_ALLOC_DR_MOVE_H

/* Taking a DR_MOVE from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

DR_MOVE *allocDrMove(RECT *rect, s32 x, s32 y);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_DR_MOVE_H */
