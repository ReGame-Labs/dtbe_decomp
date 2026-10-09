#ifndef DTBE_TEXT_FONT_H
#define DTBE_TEXT_FONT_H

/* The Shift JIS font, its glyph cache in VRAM, and the printer that draws with them. */

#include "common.h"
#include <libgpu.h>
#include "engine/gfx/vram_cache.h"
#include "engine/text/text_printer.h"

EXTERN_C_BEGIN

/* A glyph of a Font: 15 rows of 16 one-bit pixels, each row a big-endian
 * halfword. */
#define FONT_GLYPH_WIDTH 16
#define FONT_GLYPH_HEIGHT 15
#define FONT_GLYPH_SIZE (FONT_GLYPH_HEIGHT * 2)

/* A font of glyphs for the two-byte (Shift JIS) characters. data points to
 * the glyph count n, then to the n character codes in increasing order, then
 * to the n glyphs. */
typedef struct Font {
    /* 0x0 */ u16 *data;
} Font;

/* The cache of the glyphs of the font in use, in VRAM (getFontCache). */
typedef struct FontCache {
    /* 0x00 */ Cache cache;
    /* 0x18 */ Font *font;
} FontCache;

/* Prints into an ordering table: the two-byte characters in the glyphs of
 * the FontCache, the others in the 8 by 16 font in VRAM. */
typedef struct TextPrinter {
    /* 0x00 */ Printer printer;
    /* 0x0C */ u_long *ot; /* where the characters go */
    /* 0x10 */ FontCache *cache;
    /* 0x14 */ u32 color;       /* 0xBBGGRR */
    /* 0x18 */ u32 shadowColor; /* 0xBBGGRR */
    /* 0x1C */ s32 left; /* where a new line starts */
    /* 0x20 */ s32 top;
    /* 0x24 */ u32 shadow : 1; /* draw a shadow one pixel down and right */
} TextPrinter;

/* A SPRT_16 as words, to set it a word at a time. */
typedef struct Sprt16Words {
    /* 0x0 */ u_long tag;
    /* 0x4 */ u32 rgbCode; /* the color, then the primitive's code */
    /* 0x8 */ u32 xy;      /* y in the high half */
    /* 0xC */ u32 uvClut;  /* v in the second byte, the clut in the high half */
} Sprt16Words;

/* A SPRT as words, its color and code in one word (unlike console.h's
 * SprtWords), to set it a word at a time. */
typedef struct SprtColorWords {
    /* 0x00 */ u_long tag;
    /* 0x04 */ u32 rgbCode; /* the color, then the primitive's code */
    /* 0x08 */ u32 xy;      /* y in the high half */
    /* 0x0C */ u32 uvClut;  /* v in the second byte, the clut in the high half */
    /* 0x10 */ u32 wh;      /* h in the high half */
} SprtColorWords;

/* a TextPrinter's colors until they are set: grey, and a darker grey */
#define TEXT_PRINTER_COLOR 0x808080
#define TEXT_PRINTER_SHADOW_COLOR 0x404040

/* the sizes of the characters */
#define TEXT_ASCII_WIDTH 8
#define TEXT_KANJI_WIDTH 16
#define TEXT_LINE_HEIGHT 16

/* the Shift JIS full-width space, and the first two-byte code */
#define SJIS_SPACE 0x8140
#define SJIS_FIRST 0x100

extern struct PrinterVtable TEXT_PRINTER_VTABLE; /* of TextPrinter */
extern struct PrinterVtable PRINTER_VTABLE; /* of Printer */

/* the VRAM areas the FontCache keeps its glyphs in, and the palette of the
 * glyphs: declared without their sizes, which makes text/font.c address
 * them absolutely, as the original does */
extern RECT FONT_CACHE_AREAS[];
extern u16 FONT_PALETTE[];
/* the vtable of FontCache */
extern CacheVtable FONT_CACHE_VTABLE;
/* the glyph of the characters with none: a filled box with a diagonal cross
 * cut out of it */
extern u8 FONT_NO_GLYPH[];

FontCache *getFontCache(void);
FontCache *fontCacheInit(FontCache *fontCache);
void fontCacheLoadGlyph(FontCache *fontCache, s16 x, s16 y, u32 code);
Font *fontInit(Font *font, u16 *data);
void fontDestroy(Font *font, s32 flags);
u8 *fontFindGlyph(Font *font, u16 code);
TextPrinter *textPrinterInit(TextPrinter *textPrinter, u_long *ot);
void textPrinterSetColor(TextPrinter *textPrinter, u32 color);
void textPrinterSetShadow(TextPrinter *textPrinter, s32 shadow);
void textPrinterSetShadowColor(TextPrinter *textPrinter, u32 color);
void textPrinterPutChar(TextPrinter *textPrinter, s32 x, s32 y, u32 code);
void textPrinterDrawKanji(TextPrinter *textPrinter, s32 x, s32 y, u32 code);
void textPrinterDrawAscii(TextPrinter *textPrinter, s32 x, s32 y, u32 code);
void textPrinterSetOrigin(TextPrinter *textPrinter, s32 left, s32 top);
void fontCacheDestroy(FontCache *fontCache, s32 flags);
void textPrinterDestroy(TextPrinter *textPrinter, s32 flags);
u_long *addNestedOt(u_long *ot, void *prim);
s32 findCodeIndex(u16 *codes, s32 count, u16 code);
void printerDestroy(Printer *printer, s32 flags);

EXTERN_C_END

#endif /* DTBE_TEXT_FONT_H */
