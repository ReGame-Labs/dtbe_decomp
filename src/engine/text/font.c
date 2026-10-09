#include "common.h"
#include "engine/text/font.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/vram_cache.h"
#include "engine/system/memory.h"
#include "engine/text/text_printer.h"
#include "libapi.h"
#include "psyq.h"
#include "vtable.h"

/* the texture page of the characters: 4-bit, at (960, 256), with the 8 by 16
 * font and the FontCache's cells */
#define TEXT_TPAGE getTPage(0, 0, 960, 256)

/* a glyph's cell in VRAM: 16 4-bit pixels (4 VRAM pixels) wide, a row
 * taller than the glyph */
#define FONT_CELL_WIDTH (FONT_GLYPH_WIDTH / 4)
#define FONT_CELL_HEIGHT 16

/* the palettes of the characters: the glyphs' (FONT_PALETTE_RECT), and the 8 by 16
 * font's */
#define FONT_CLUT getClut(960, 511)
#define ASCII_CLUT getClut(960, 320)
/* the 8 by 16 font: the characters from ' ' on, 32 to a row of the page */
#define ASCII_FIRST ' '
#define ASCII_PER_ROW 32

/* the codes the BIOS ROM font has glyphs for (Krom2RawAdd): the non-kanji
 * 0x8140-0x84BE and the level 1 kanji 0x889F-0x9872 */
#define KROM_NONKANJI_FIRST 0x8140
#define KROM_NONKANJI_COUNT 0x37F
#define KROM_KANJI_FIRST 0x889F
#define KROM_KANJI_COUNT 0xFD4

/* The header declares these two without their sizes, so this file addresses
 * them absolutely like the original, though they are small data. */

/* the VRAM areas the FontCache keeps its glyphs in: one, (960, 336) to
 * (1024, 496) */
RECT FONT_CACHE_AREAS[1] = { { 960, 336, 64, 160 } };

/* the palette of the glyphs: transparent, white */
u16 FONT_PALETTE[2] = { 0x0000, 0xFFFF };

/* the FontCache, made the first time it is asked for */
static FontCache *FONT_CACHE = NULL;

/* where the palette goes in VRAM: 16 entries at (960, 511) */
static RECT FONT_PALETTE_RECT = { 960, 511, 16, 1 };

/* where a glyph is loaded: its cell (fontCacheLoadGlyph sets where) */
static RECT FONT_GLYPH_RECT = { 0, 0, FONT_CELL_WIDTH, FONT_CELL_HEIGHT };

/* The FontCache, made the first time. */
FontCache *getFontCache(void) {
    if (FONT_CACHE == NULL) {
        FONT_CACHE = fontCacheInit(operatorNew(sizeof(FontCache)));
    }
    return FONT_CACHE;
}

/* Builds the FontCache in its VRAM, in cells of 16 by 16 pixels, and loads
 * the palette of the glyphs. */
FontCache *fontCacheInit(FontCache *this) {
    cacheInit(&this->cache, FONT_CACHE_AREAS, sizeof(FONT_CACHE_AREAS) / sizeof(RECT), FONT_CELL_WIDTH,
                  FONT_CELL_HEIGHT);
    this->cache.vtable = &FONT_CACHE_VTABLE;
    func_80057F98(&FONT_PALETTE_RECT, (u_long *)FONT_PALETTE);
    this->font = NULL;
    return this;
}

/* Loads the glyph of a character into its cell at (x, y) as a 4-bit image:
 * the font's glyph, else the BIOS ROM's for the codes it has, else an empty
 * box. */
void fontCacheLoadGlyph(FontCache *this, s16 x, s16 y, u32 code) {
    u8 image[FONT_CELL_HEIGHT][FONT_GLYPH_WIDTH / 2];
    u8 *glyph;
    u8 *out;
    s32 row;

    memset(image, 0, sizeof(image));
    FONT_GLYPH_RECT.x = x;
    FONT_GLYPH_RECT.y = y;
    glyph = NULL;
    if (this->font != NULL) {
        glyph = fontFindGlyph(this->font, code);
    }
    if (glyph == NULL) {
        if (code - KROM_NONKANJI_FIRST < KROM_NONKANJI_COUNT ||
            code - KROM_KANJI_FIRST < KROM_KANJI_COUNT) {
            /* libapi.h has it return a long: the glyph's address */
            glyph = (u8 *)Krom2RawAdd(code);
        } else {
            glyph = FONT_NO_GLYPH;
        }
    }
    out = image[0];
    for (row = FONT_GLYPH_HEIGHT - 1; row != -1; row--) {
        /* the row, a big-endian halfword, leftmost pixel first */
        u32 bits = *glyph++;
        u8 *next = out + FONT_GLYPH_WIDTH / 2;
        s32 i;

        bits <<= 8;
        bits |= *glyph++;
        if (bits == 0) {
            out = next;
            continue;
        }
        bits <<= 16;
        /* two pixels a byte, the left one in the low nibble; the rest of
         * the row is already 0 once no bits are left */
        for (i = 8; --i != -1;) {
            u8 pixels = ((bits >> 26) & 0x10) | (bits >> 31);
            bits <<= 2;
            *out++ = pixels;
            if (bits == 0) {
                break;
            }
        }
        out = next;
    }
    func_80057F98(&FONT_GLYPH_RECT, (u_long *)image);
}

/* Makes the font the one the FontCache draws. */
Font *fontInit(Font *this, u16 *data) {
    FontCache *cache = getFontCache();

    this->data = data;
    cache->font = this;
    return this;
}

/* Stops the FontCache drawing the font, and empties it. */
void fontDestroy(Font *this, s32 flags) {
    FontCache *cache = getFontCache();

    cache->font = NULL;
    cacheReset(&cache->cache);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The glyph of a character, or NULL if the font has none. */
u8 *fontFindGlyph(Font *this, u16 code) {
    u16 *data = this->data;
    s32 count = *data;
    u8 *glyphs = (u8 *)data + (count * 2 + 2);
    s32 i = findCodeIndex(data + 1, count, code);

    if (i < 0) {
        return NULL;
    }
    return glyphs + i * FONT_GLYPH_SIZE;
}

/* Builds a printer into the ordering table, in grey with a dark grey
 * shadow, the shadow off. */
TextPrinter *textPrinterInit(TextPrinter *this, u_long *ot) {
    this->printer.vtable = &TEXT_PRINTER_VTABLE;
    this->ot = addNestedOt(ot, allocDrTpage(TEXT_TPAGE));
    this->color = TEXT_PRINTER_COLOR;
    this->shadowColor = TEXT_PRINTER_SHADOW_COLOR;
    this->cache = getFontCache();
    textPrinterSetShadow(this, 0);
    return this;
}

/* Sets the color of the characters. */
void textPrinterSetColor(TextPrinter *this, u32 color) {
    this->color = color & 0xFFFFFF;
}

/* Turns the shadow on or off. */
void textPrinterSetShadow(TextPrinter *this, s32 shadow) {
    this->shadow = shadow;
}

/* Sets the color of the shadow. */
void textPrinterSetShadowColor(TextPrinter *this, u32 color) {
    this->shadowColor = color & 0xFFFFFF;
}

/* Prints a character at (x, y) and moves the cursor past it; a newline
 * moves it to the start of the next line. */
void textPrinterPutChar(TextPrinter *this, s32 x, s32 y, u32 code) {
    s32 width = TEXT_KANJI_WIDTH;

    if (code >= SJIS_FIRST) {
        if (code != SJIS_SPACE) {
            textPrinterDrawKanji(this, x, y, code);
        }
    } else {
        width = TEXT_ASCII_WIDTH;
        if (code != ' ') {
            if (code == '\n') {
                this->printer.x = this->left;
                this->printer.y += TEXT_LINE_HEIGHT;
                return;
            }
            textPrinterDrawAscii(this, x, y, code);
        }
    }
    this->printer.x = x + width;
}

/* Draws a two-byte character from its glyph in the FontCache, and its
 * shadow. */
void textPrinterDrawKanji(TextPrinter *this, s32 x, s32 y, u32 code) {
    CacheSlot *slot = cacheGetSlot(&this->cache->cache, code);
    /* the texture coordinates of the cell in the 4-bit page: 4 pixels to a
     * VRAM pixel, the page's coordinates in a byte */
    u8 u = (u8)slot->x * 4;
    u8 v = slot->y;
    Sprt16Words *sprt = (Sprt16Words *)allocSprt16();
    u32 rgbCode = this->color | (((SPRT_16 *)sprt)->code << 24);
    u32 uvClut;

    sprt->xy = x | (y << 16);
    uvClut = (FONT_CLUT << 16) | (v << 8) | u;
    sprt->uvClut = uvClut;
    sprt->rgbCode = rgbCode;
    AddPrim(this->ot, sprt);
    if (this->shadow) {
        x++;
        y++;
        sprt = (Sprt16Words *)allocSprt16();
        rgbCode = this->shadowColor | (((SPRT_16 *)sprt)->code << 24);
        sprt->xy = x | (y << 16);
        sprt->uvClut = uvClut;
        sprt->rgbCode = rgbCode;
        AddPrim(this->ot, sprt);
    }
}

/* Draws a one-byte character from the 8 by 16 font, and its shadow. */
void textPrinterDrawAscii(TextPrinter *this, s32 x, s32 y, u32 code) {
    u32 cell = code - ASCII_FIRST;
    u8 u = (cell % ASCII_PER_ROW) * TEXT_ASCII_WIDTH;
    u8 v = (cell / ASCII_PER_ROW) * TEXT_LINE_HEIGHT;
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();
    u32 rgbCode = this->color | (((SPRT *)sprt)->code << 24);
    u32 uvClut;

    sprt->xy = (y << 16) | x;
    uvClut = (ASCII_CLUT << 16) | (v << 8) | u;
    sprt->uvClut = uvClut;
    sprt->wh = (TEXT_LINE_HEIGHT << 16) | TEXT_ASCII_WIDTH;
    sprt->rgbCode = rgbCode;
    AddPrim(this->ot, sprt);
    if (this->shadow) {
        x++;
        y++;
        sprt = (SprtColorWords *)allocSprt();
        rgbCode = this->shadowColor | (((SPRT *)sprt)->code << 24);
        sprt->xy = (y << 16) | x;
        sprt->uvClut = uvClut;
        sprt->wh = (TEXT_LINE_HEIGHT << 16) | TEXT_ASCII_WIDTH;
        sprt->rgbCode = rgbCode;
        AddPrim(this->ot, sprt);
    }
}

/* Moves the cursor, and where new lines start, to (left, top). */
void textPrinterSetOrigin(TextPrinter *this, s32 left, s32 top) {
    this->left = left;
    this->top = top;
    this->printer.x = left;
    this->printer.y = top;
}

/* Destroys the FontCache. */
void fontCacheDestroy(FontCache *this, s32 flags) {
    cacheDestroy(&this->cache, flags);
}

/* Destroys the printer. */
void textPrinterDestroy(TextPrinter *this, s32 flags) {
    printerDestroy(&this->printer, flags);
}

/* Adds to the ordering table a three-entry one that draws the primitive
 * first, and returns its middle entry, for what goes over it. */
u_long *addNestedOt(u_long *ot, void *prim) {
    u_long *nested = (u_long *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += 3 * sizeof(u_long);
    ClearOTagR(nested, 3);
    AddPrim(&nested[2], prim);
    setaddr(nested, getaddr(ot));
    setaddr(ot, &nested[2]);
    return &nested[1];
}

/* The index of a code in the count codes in increasing order, or -1. */
s32 findCodeIndex(u16 *codes, s32 count, u16 code) {
    s32 high = count - 1;
    s32 low = 0;
    s32 mid = high / 2;

    while (codes[mid] != code) {
        if (low == high) {
            return -1;
        }
        if (code < codes[mid]) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
        if (high < low) {
            return -1;
        }
        mid = (low + high) / 2;
    }
    return mid;
}

/* Destroys the printer. */
void printerDestroy(Printer *this, s32 flags) {
    this->vtable = &PRINTER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/text/font", TEXT_PRINTER_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/text/font", FONT_CACHE_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/text/font", FONT_NO_GLYPH);

INCLUDE_RODATA("asm/jp/main/nonmatchings/text/font", PRINTER_VTABLE);
