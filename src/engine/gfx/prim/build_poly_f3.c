#include "common.h"
#include "engine/gfx/prim/build_poly_f3.h"

/* Projects a flat triangle (three SVECTOR pointers and a color) into a POLY_F3
 * packet, semi-transparent and raw by its flags (raw forced for the neutral
 * color 0x808080), adds it to the ordering table at its average depth >> shift,
 * then a DR_TPAGE with its blend rate when semi-transparent, and returns the
 * next packet (the same one when RTPT flags an error). Compiled C, not written
 * by hand: GCC frame, filled delay slots, GCC's registers. In C with game
 * macros for the lw/lwc2 of the vertices, RTPT, AVSZ3 and the FLAG and OTZ
 * reads, all matches but the registers: GCC gives this and ot $s2 and $s1 (the
 * game's $s1 and $s2), and 0xFFFFFF, 0x808080, the packet length and the color
 * other registers than the game's $v1, $v0, $a1 and $a0, as in buildPolyGT3
 * (build_poly_gt3.c) and the other packet builders (buildLineF2, buildLineG2,
 * buildPolyFT4, buildPolyFT4Windowed, buildPolyGT4). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/prim/build_poly_f3", buildPolyF3);
