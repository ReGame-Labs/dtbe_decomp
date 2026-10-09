#ifndef DTBE_GFX_PRIM_ALLOC_DR_LOAD_H
#define DTBE_GFX_PRIM_ALLOC_DR_LOAD_H

/* Taking a DR_LOAD, set to load a rectangle of VRAM, from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

DR_LOAD *allocDrLoad(RECT *rect);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_DR_LOAD_H */
