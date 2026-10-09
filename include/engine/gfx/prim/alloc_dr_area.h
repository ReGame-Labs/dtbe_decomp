#ifndef DTBE_GFX_PRIM_ALLOC_DR_AREA_H
#define DTBE_GFX_PRIM_ALLOC_DR_AREA_H

/* Taking a DR_AREA, POLY_F4, DR_MODE or LINE_F2 from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

LINE_F2 *allocLineF2(void);

DR_AREA *allocDrArea(RECT *area);
POLY_F4 *allocPolyF4(void);
DR_MODE *allocDrMode(s32 tpage, RECT *tw);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_DR_AREA_H */
