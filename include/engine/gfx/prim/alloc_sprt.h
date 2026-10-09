#ifndef DTBE_GFX_PRIM_ALLOC_SPRT_H
#define DTBE_GFX_PRIM_ALLOC_SPRT_H

/* Taking a SPRT, a TILE or a DR_TPAGE from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

SPRT *allocSprt(void);
TILE *allocTile(void);
DR_TPAGE *allocDrTpage(s32 tpage);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_SPRT_H */
