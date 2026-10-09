#include "common.h"
#include "engine/gfx/prim/build_line_g2.h"

/* buildLineF2 for a Gouraud line: a LINE_G2 packet, the second color copied
 * from the shape. Compiled C; the same register difference as buildLineF2,
 * and the game loads the second color before AddPrim and stores it after,
 * across the call, which no plain C form gave. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_g2", buildLineG2);

/* Projects a textured quad (four SVECTOR pointers, a color and four uv/clut/
 * tpage words) into a POLY_FT4 packet: RTPT for three corners, RTPS for the
 * fourth, AVSZ3 for its depth; returns the next packet. Compiled C (GCC frame,
 * filled delay slots, shape in $t4 past the asm's $t0-$t3). With game macros
 * the GTE part matches, but GCC gives the color block other registers, as in
 * buildPolyF3, and the game loads the four uv words before storing any,
 * where GCC alternates loads and stores (shape and packet may alias). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_g2", buildPolyFT4);

/* buildPolyFT4 between two DR_TWINs: the default texture window
 * (D_8006434C), the POLY_FT4, then the window tw (an empty command for NULL).
 * Compiled C, with the same differences as buildPolyFT4. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_g2", buildPolyFT4Windowed);

/* buildPolyFT4 for a POLY_GT4 (four colors). Compiled C, with the same
 * register difference; the game loads the last three words (uv2, color 3,
 * uv3) before AddPrim and stores them after it, across the call, as
 * buildPolyGT3 (build_poly_gt3.c) does its last one: no plain C form gave
 * that. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_line_g2", buildPolyGT4);
