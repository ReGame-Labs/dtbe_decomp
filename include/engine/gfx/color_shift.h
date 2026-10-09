#ifndef DTBE_GFX_COLOR_SHIFT_H
#define DTBE_GFX_COLOR_SHIFT_H

/* Shifting the hue, saturation and value of 15-bit colors. */

#include "common.h"
#include "engine/gfx/color.h"

EXTERN_C_BEGIN

void shiftColors(u16 *src, u16 *dst, u32 count, HsvColor *shift);

EXTERN_C_END

#endif /* DTBE_GFX_COLOR_SHIFT_H */
