/* The digifont cache (DigifontCache): a font of 24 by 24 glyphs cached in VRAM, which nothing in the title
 * builds. The boundary is unproven: the unit has no strings, only its virtual table marks it. */

#include "common.h"
#include "engine/lib/format.h"
#include "engine/lib/string.h"
#include "engine/system/memory.h"
#include "engine/text/font.h"
#include "title/title_classes.h"
#include "strings.h"

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
    /* 0x3C */ s32 alternate; /* passed to DigifontCache_getSlot: 1 draws the glyphs 0x300 codes higher */
    /* 0x40 */ RECT charRect; /* where and how big a character is drawn: 0, 0, 24, 24 at first */
    /* 0x48 */ u8 color[4];   /* r, g, b and alpha, 0x80 each at first; an alpha of 0 hides the print */
} DigifontCache;

/* a glyph's width and height, in pixels */
#define DIGIFONT_CHAR_SIZE 24

extern CacheVtable DIGIFONT_CACHE_VTABLE;

void DigifontCache_loadFile(DigifontCache *this, char *path);

/* The digifont cache's constructor: a Cache of one VRAM area of 6 by 24 cells (a 24-pixel glyph at 4 bits),
 * the area's x aligned down to a 64-halfword texture page and its y to 256, characters drawn 24 by 24 in
 * neutral gray, and the font file at path loaded. */
DigifontCache *DigifontCache_init(DigifontCache *this, RECT *area, char *path) {
    cacheInit(&this->cache, area, 1, 6, DIGIFONT_CHAR_SIZE);
    this->cache.vtable = &DIGIFONT_CACHE_VTABLE;
    this->area = *area;
    this->area.x &= ~0x3F;
    this->area.y &= ~0xFF;
    this->alternate = 0;
    this->charRect.x = 0;
    this->charRect.y = 0;
    this->charRect.w = DIGIFONT_CHAR_SIZE;
    this->charRect.h = DIGIFONT_CHAR_SIZE;
    *(u32 *)this->color = 0x80808080; /* the four bytes at once, in one word store */
    DigifontCache_loadFile(this, path);
    return this;
}

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

/* Finds code's cell in the cache, loading its glyph if needed, and puts its VRAM place (x in pixels, 23 by 23)
 * in rect; alternate 1 takes the glyph 0x300 codes higher. Returns 0, or -1 with no glyph (no font loaded). */
s32 DigifontCache_getSlot(DigifontCache *this, RECT *rect, u32 code, s32 alternate) {
    CacheSlot *slot;

    if (alternate == 1) {
        code += 0x300;
    }
    DigifontCache_selectGlyph(this, code);
    if (this->glyph == NULL) {
        return -1;
    }
    slot = cacheGetSlot(&this->cache, code);
    rect->x = slot->x * 4;
    rect->y = slot->y;
    rect->w = 23;
    rect->h = 23;
    return 0;
}

void DigifontCache_drawChar(DigifontCache *this, u_long *ot, RECT *rect, u8 *color, u32 code, s32 alternate);

/* Prints formatted text as printf does, in full-width glyphs from charRect's place, one character width
 * (charRect.w) apart; nothing when the color's alpha is 0. */
void DigifontCache_print(DigifontCache *this, u_long *ot, u8 *format, ...) {
    VaList args;
    u8 text[0x200];
    u8 wide[0x400];
    RECT rect;

    if (this->color[3] == 0) {
        return;
    }
    VA_START(args, format);
    vsprintf(text, format, args);
    copyFullWidth(wide, text);
    /* The match depends on format walking the full-width text: the game keeps the pointer in format's stack
     * slot (its address is taken by VA_START). */
    format = wide;
    rect = this->charRect;
    rect.w--;
    while (*format != '\0') {
        DigifontCache_drawChar(this, ot, &rect, this->color, (format[0] << 8) | format[1], this->alternate);
        rect.x += this->charRect.w;
        format += 2;
    }
}

/* Prints formatted text as printf does, in full-width glyphs centred in box: a glyph taller than the box takes
 * its height, and a line wider than it is squeezed to box->w / count per glyph. Nothing when the color's alpha
 * is 0. */
void DigifontCache_printInBox(DigifontCache *this, u_long *ot, RECT *box, u8 *format, ...) {
    VaList args;
    u8 text[0x200];
    u8 wide[0x400];
    RECT rect;
    s32 count;
    s32 squeezed;
    s32 i;

    if (this->color[3] == 0) {
        return;
    }
    VA_START(args, format);
    vsprintf(text, format, args);
    copyFullWidth(wide, text);
    format = wide;
    if (this->charRect.h > box->h) {
        rect.h = box->h;
        rect.y = box->y;
    } else {
        rect.h = this->charRect.h;
        rect.y = box->y + ((box->h - this->charRect.h) >> 1);
    }
    count = strlen(format) / 2;
    squeezed = count * this->charRect.w > box->w;
    if (squeezed) {
        rect.x = box->x;
        rect.w = box->w / count;
    } else {
        rect.w = this->charRect.w - 1;
        rect.x = box->x + ((box->w - count * this->charRect.w) >> 1);
    }
    for (i = 1; i <= count; i++) {
        DigifontCache_drawChar(this, ot, &rect, this->color, (format[0] << 8) | format[1], this->alternate);
        if (squeezed) {
            rect.x = box->x + box->w * i / count;
        } else {
            rect.x += this->charRect.w;
        }
        format += 2;
    }
}

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

/* Sets alternate: 1 draws the glyphs 0x300 codes higher. */
void DigifontCache_setAlternate(DigifontCache *this, s32 value) {
    this->alternate = value;
}

INCLUDE_RODATA("title/nonmatchings/digifont", DIGIFONT_CACHE_VTABLE);
