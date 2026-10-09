#include "common.h"
#include "engine/gfx/prim/build_poly_gt3.h"

/* Projects a textured, Gouraud-shaded triangle (three SVECTOR pointers,
 * colors, uv/clut/tpage words) into a POLY_GT3 packet, its semi-transparency
 * and raw-texture bits from its flags, raw texture forced for the neutral
 * color 0x808080; adds it to the ordering table at its average depth >> shift
 * and returns the next packet (the same one when RTPT flags an error).
 * Compiled C (GCC frame, filled delay slots; the F3 twin is buildPolyF3): in
 * C with game macros for the lw/lwc2 of the three vertices, RTPT and AVSZ3,
 * the GTE part matches, but GCC gives 9, 0xFFFFFF, 0x808080 and the color
 * other registers than the game's $a1, $v1, $v0 and $a0, and the game loads
 * four words of the triangle, stores them to the packet, then loads the fifth
 * (uv2) before AddPrim, an order no plain C form gave. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_poly_gt3", buildPolyGT3);
