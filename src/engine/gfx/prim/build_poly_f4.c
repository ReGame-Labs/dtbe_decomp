#include "common.h"
#include "engine/gfx/prim/build_poly_f4.h"

/* Projects a shape's four vertices and adds its POLY_F4, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_poly_f4", buildPolyF4);

/* Projects a shape's vertex and adds a TILE there, and a DR_TPAGE when the
 * shape is semi-transparent, to the ordering table. Compiled C, the template
 * of buildPolyFT3 with the same early length load into a1: left asm for the
 * same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_poly_f4", buildTile);
