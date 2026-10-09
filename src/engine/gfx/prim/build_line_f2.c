#include "common.h"
#include "engine/gfx/prim/build_line_f2.h"

/* Projects a line (two SVECTOR pointers and a color) into a LINE_F2 packet as
 * buildPolyF3 does a triangle: RTPT with V2 cleared, depth (SZ1 + SZ2) >> 3,
 * a DR_TPAGE after it when semi-transparent. Compiled C (GCC frame, filled
 * delay slots). The depth is computed in the game's asm, its sra in the load
 * delay of the cfc2 of FLAG; with game macros the GTE part matches, but GCC
 * gives this and ot, and 0xFFFFFF, 0x808080, the packet length and the color,
 * other registers than the game's, as in buildPolyF3. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_f2", buildLineF2);
