#ifndef DTBE_GFX_PRIM_ALLOC_LINE_G2_H
#define DTBE_GFX_PRIM_ALLOC_LINE_G2_H

/* Taking a LINE_G2 or a POLY_FT3 from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

LINE_G2 *allocLineG2(void);
POLY_FT3 *allocPolyFT3(void);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_LINE_G2_H */
