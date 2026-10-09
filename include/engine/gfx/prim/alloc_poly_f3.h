#ifndef DTBE_GFX_PRIM_ALLOC_POLY_F3_H
#define DTBE_GFX_PRIM_ALLOC_POLY_F3_H

/* Taking a POLY_F3, a POLY_G4 or a POLY_GT4 from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

POLY_F3 *allocPolyF3(void);
POLY_G4 *allocPolyG4(void);
POLY_GT4 *allocPolyGT4(void);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_POLY_F3_H */
