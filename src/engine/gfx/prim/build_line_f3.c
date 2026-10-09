#include "common.h"
#include "engine/gfx/prim/build_line_f3.h"

/* Projects a shape's three vertices and adds its LINE_F3, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f3", buildLineF3);

/* Projects a shape's four vertices and adds its POLY_G4, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f3", buildPolyG4);

/* Projects a shape's three vertices and adds its POLY_FT3 to the ordering
 * table. Compiled C (GCC frame and registers), the same in cc1 and cc1plus: a
 * C draft over GTE macros has 5 instructions left. The original loads the
 * length 7 first in its block and into a1; sched1 leaves it there only when
 * its pseudo is set more than once (birthing_insn_p), and a `code` variable
 * reused for the code byte does that but moves the code byte's arms to a1 too.
 * Its first uv word is also loaded before `move a0`, which needs that word's
 * pseudo set twice with one set leaving no instruction. No plausible source
 * gives either: most likely the game's GCC variant (see TODO.md, Block moves)
 * counts these sets differently. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f3", buildPolyFT3);

/* Projects a shape's three vertices and adds its POLY_G3, and a DR_TPAGE when
 * the shape is semi-transparent, to the ordering table. Compiled C, the
 * template of buildPolyFT3 with the same early length load into a1: left asm
 * for the same reason. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f3", buildPolyG3);
