/* The digifont cache (DigifontCache): a font of 24 by 24 glyphs cached in VRAM, which nothing in the title
 * builds. The boundary is unproven: the unit has no strings, only its virtual table marks it. */

#include "common.h"
#include "engine/system/memory.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

/* A Cache of glyphs (DigifontCache_init, DIGIFONT_CACHE_VTABLE) loaded from a font file: a u16 count, count
 * two-byte character codes, then a 192-byte glyph for each, the layout of the assets' title/digifont.bin. The
 * cache's load slot expands the glyph picked into its VRAM cell. */
typedef struct DigifontCache {
    /* 0x00 */ Cache cache;
    /* 0x18 */ void *file;    /* the font file DigifontCache_loadFile loaded, freed with mainHeapFree */
    /* 0x1C */ s32 codeCount; /* of codes */
    /* 0x20 */ u16 *codes;    /* the character codes the glyphs are for */
    /* 0x24 */ u8 *glyphs;    /* 192 bytes each, in the order of codes */
    /* 0x28 */ u8 *glyph;     /* the one DigifontCache_selectGlyph picked */
    /* 0x2C */ RECT area;     /* the cache's VRAM area */
    /* 0x34 */ RECT clutRect; /* where DIGIFONT_CLUT goes in VRAM */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ RECT charRect; /* where and how big a character is drawn: 0, 0, 24, 24 at first */
    /* 0x48 */ u8 color[4];   /* r, g, b and alpha, 0x80 each at first; an alpha of 0 hides the print */
} DigifontCache;

extern CacheVtable DIGIFONT_CACHE_VTABLE;

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_init);

/* The digifont cache's destructor: frees the font file, then destroys the Cache. */
void DigifontCache_destroy(DigifontCache *this, s32 flags) {
    this->cache.vtable = &DIGIFONT_CACHE_VTABLE;
    mainHeapFree(this->file);
    cacheDestroy(&this->cache, flags);
}

INCLUDE_RODATA("title/nonmatchings/digifont", DIGIFONT_CLUT);

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_loadFile);

/* Picks the glyph of character code (0x28), from its index in codes. */
void DigifontCache_selectGlyph(DigifontCache *this, u16 code) {
    this->glyph = this->glyphs + findCodeIndex(this->codes, this->codeCount, code) * 192;
}

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_load);

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_getSlot);

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_print);

INCLUDE_ASM("title/nonmatchings/digifont", func_8007A9C8);

INCLUDE_ASM("title/nonmatchings/digifont", DigifontCache_drawChar);

/* Copies the CLUT's VRAM rectangle to out. */
void DigifontCache_getClutRect(DigifontCache *this, RECT *out) {
    *out = this->clutRect;
}

/* Sets where a character is drawn. */
void func_8007AFB0(DigifontCache *this, s32 x, s32 y) {
    this->charRect.x = x;
    this->charRect.y = y;
}

/* Sets the size a character is drawn at: charRect's w and h to w - 1 and h - 1. */
void func_8007AFBC(DigifontCache *this, s16 w, s16 h) {
    this->charRect.w = w - 1;
    this->charRect.h = h - 1;
}

/* Sets charRect (24 by 24 at first) from rect. */
void func_8007AFE0(DigifontCache *this, RECT *rect) {
    this->charRect = *rect;
}

/* Sets the color to r, g, b with an alpha of 0x80. */
void DigifontCache_setColor(DigifontCache *this, u8 r, u8 g, u8 b) {
    this->color[0] = r;
    this->color[1] = g;
    this->color[2] = b;
    this->color[3] = 0x80;
}

/* Sets the color to r, g, b and alpha a. */
void DigifontCache_setColorAlpha(DigifontCache *this, u8 r, u8 g, u8 b, u8 a) {
    this->color[0] = r;
    this->color[1] = g;
    this->color[2] = b;
    this->color[3] = a;
}

/* Sets the color's alpha. */
void DigifontCache_setAlpha(DigifontCache *this, u8 a) {
    this->color[3] = a;
}

/* Sets 0x3C. */
void func_8007B040(DigifontCache *this, s32 value) {
    this->unk3C = value;
}

INCLUDE_RODATA("title/nonmatchings/digifont", DIGIFONT_CACHE_VTABLE);
