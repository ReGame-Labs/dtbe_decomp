#ifndef DTBE_GFX_PRIM_ALLOC_LINE_F4_H
#define DTBE_GFX_PRIM_ALLOC_LINE_F4_H

/* Taking a LINE_F4 or a POLY_G3 from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

LINE_F4 *allocLineF4(void);
POLY_G3 *allocPolyG3(void);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_LINE_F4_H */
