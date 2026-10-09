#include "common.h"
#include "engine/text/screen_print.h"
#include "engine/lib/format.h"
#include "engine/lib/string.h"
#include "libgpu.h"

/* the buffers screenVprint formats its text in, then turns it full-width in */
#define SCREEN_FORMAT_BUFFER ((u8 *)0x801FE000)
#define SCREEN_TEXT_BUFFER ((u8 *)0x801FA000)
/* the size of a character screenVprint draws, and the rows of the BIOS's glyphs */
#define SCREEN_GLYPH_SIZE 16
#define SCREEN_GLYPH_ROWS 15
/* where screenVprint starts its lines, and the column it wraps them at */
#define SCREEN_TEXT_LEFT 32
#define SCREEN_TEXT_TOP 16
#define SCREEN_TEXT_RIGHT 608

/* Writes formatted text on the screen, as printf does. */
void screenPrint(const char *fmt, ...) {
    VaList args;

    VA_START(args, fmt);
    screenVprint(fmt, args);
}

/* where screenVprint writes the next character on the screen */
static s32 SCREEN_TEXT_X = SCREEN_TEXT_LEFT;
static s32 SCREEN_TEXT_Y = SCREEN_TEXT_TOP;

/*
 * Writes formatted text on the screen, as vprintf does, in full-width
 * characters drawn straight into VRAM; a line wraps at SCREEN_TEXT_RIGHT.
 */
void screenVprint(const char *fmt, VaList args) {
    u8 *text = SCREEN_TEXT_BUFFER;
    s32 x;
    s32 y;
    u32 c;

    vsprintf(SCREEN_FORMAT_BUFFER, (u8 *)fmt, args);
    copyFullWidth(SCREEN_TEXT_BUFFER, SCREEN_FORMAT_BUFFER);
    x = SCREEN_TEXT_X;
    y = SCREEN_TEXT_Y;
    while ((c = *text++) != '\0') {
        if (c == '\n') {
            x = SCREEN_TEXT_LEFT;
            y += SCREEN_GLYPH_SIZE;
        } else if (c >= ' ') {
            screenPutChar(x, y, (c << 8) | *text++);
            x += SCREEN_GLYPH_SIZE;
            if (x >= SCREEN_TEXT_RIGHT) {
                x = SCREEN_TEXT_LEFT;
                y += SCREEN_GLYPH_SIZE;
            }
        }
    }
    SCREEN_TEXT_Y = y;
    SCREEN_TEXT_X = x;
}

/*
 * The area of VRAM screenPutChar (still asm) draws a character in: it moves
 * the corner and keeps the size.
 */
RECT SCREEN_CHAR_RECT = { 0, 0, SCREEN_GLYPH_SIZE, SCREEN_GLYPH_SIZE };

/*
 * Draws a character in bold into VRAM at (x, y): reads the area, ORs the
 * glyph's rows (each widened by a pixel) into it and writes it back. The C
 * (a row loop around a pixel loop that stops early) matches but for the
 * allocation: the original has x/y in s0/s1, the RECT in s2 and the glyph in
 * s3, and sets the RECT argument up before the row loop (39 lines differ).
 * That argument is a pseudo of its own in a0 across the loop, which shortens
 * the RECT's life so it gets s2 first; GCC sets a0 from the symbol, and GCSE
 * folds an explicit copy. Const parameters and C++ change nothing.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/text/screen_print", screenPutChar);
