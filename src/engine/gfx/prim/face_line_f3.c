#include "common.h"
#include "engine/gfx/prim/face_line_f3.h"

/* Projects a face's three vertices and adds its LINE_F3, and a DR_TPAGE when
 * the face is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_line_f3", buildLineF3);

/* Projects a face's four vertices and adds its POLY_G4, and a DR_TPAGE when
 * the face is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_line_f3", buildPolyG4);

/* Projects a face's three vertices and adds its POLY_FT3 to the ordering
 * table. Compiled C (GCC frame and registers): a C draft over GTE macros
 * matches except the length store, whose 7 the original loads early into a1,
 * as GCC does only when that register is set more than once; no plausible
 * source for that was found. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_line_f3", buildPolyFT3);

/* Projects a face's three vertices and adds its POLY_G3, and a DR_TPAGE when
 * the face is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/face_line_f3", buildPolyG3);
