#include "common.h"
#include "engine/gfx/prim/face_poly_f4.h"

/* Projects a face's four vertices and adds its POLY_F4, and a DR_TPAGE when
 * the face is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_poly_f4", buildPolyF4);

/* Projects a face's vertex and adds a TILE there, and a DR_TPAGE when the face
 * is semi-transparent, to the ordering table. Compiled C, the template of
 * buildPolyFT3 with the same early length load into a1: left asm for the same
 * reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_poly_f4", buildTile);
