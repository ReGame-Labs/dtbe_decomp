#include "common.h"
#include "engine/gfx/prim/build_line_f4.h"

/* Projects a shape's four vertices and adds its LINE_F4, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f4", buildLineF4);
