#include "common.h"
#include "engine/gfx/color_shift.h"

/* Shifts the hue, saturation and value of 15-bit colors. In C the unpacking
 * of a color is scheduled differently and the transparent color is stored as
 * 0 rather than from the register it was read into. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/color_shift", shiftColors);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/color_shift", D_80019350);
