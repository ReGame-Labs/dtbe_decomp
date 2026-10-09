#include "common.h"
#include "engine/gfx/prim/build_line_g3.h"

/* Projects a shape's three vertices and adds its LINE_G3, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_g3", buildLineG3);
