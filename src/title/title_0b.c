/* A cache class with a heap block (virtual table D_80066B68) and small functions nothing calls. The
 * boundary is unproven: the unit has no strings, only that virtual table marks it. */

#include "common.h"
#include "engine/system/memory.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

/* A Cache with a heap block at 0x18 (func_8007A46C constructs it, virtual
 * table D_80066B68). */
typedef struct Unk8007A46C {
    /* 0x00 */ Cache cache;
    /* 0x18 */ void *unk18; /* freed with mainHeapFree */
    /* 0x1C */ s32 codeCount; /* of codes */
    /* 0x20 */ u16 *codes;    /* the character codes the glyphs are for */
    /* 0x24 */ u8 *glyphs;    /* 192 bytes each, in the order of codes */
    /* 0x28 */ u8 *glyph;     /* the one func_8007A6A0 picked */
    /* 0x2C */ RECT unk2C;
    /* 0x34 */ RECT unk34;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ RECT unk40; /* 0, 0, 24, 24 at first (func_8007A46C) */
    /* 0x48 */ u8 unk48[4]; /* 0x80 each at first; func_8007B008 sets three and 0x80 */
} Unk8007A46C;

extern CacheVtable D_80066B68;

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A46C);

/* Unk8007A46C's destructor: frees the block at 0x18, then destroys the Cache. */
void func_8007A534(Unk8007A46C *this, s32 flags) {
    this->cache.vtable = &D_80066B68;
    mainHeapFree(this->unk18);
    cacheDestroy(&this->cache, flags);
}

INCLUDE_RODATA("title/nonmatchings/title_0b", D_80066B60);

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A580);

/* Picks the glyph of character code (0x28), from its index in codes. */
void func_8007A6A0(Unk8007A46C *this, u16 code) {
    this->glyph = this->glyphs + findCodeIndex(this->codes, this->codeCount, code) * 192;
}

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A6EC);

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A82C);

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A8C4);

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007A9C8);

INCLUDE_ASM("title/nonmatchings/title_0b", func_8007ABB0);

/* Copies the rectangle at 0x34 to out. */
void func_8007AF88(Unk8007A46C *this, RECT *out) {
    *out = this->unk34;
}

/* Sets x and y of the rectangle at 0x40. */
void func_8007AFB0(Unk8007A46C *this, s32 x, s32 y) {
    this->unk40.x = x;
    this->unk40.y = y;
}

/* Sets the width and height of the rectangle at 0x40 to w - 1 and h - 1. */
void func_8007AFBC(Unk8007A46C *this, s16 w, s16 h) {
    this->unk40.w = w - 1;
    this->unk40.h = h - 1;
}

/* Sets the rectangle at 0x40 (24 by 24 at first) from rect. */
void func_8007AFE0(Unk8007A46C *this, RECT *rect) {
    this->unk40 = *rect;
}

/* Sets the color at 0x48 to r, g, b with 0x80 in the fourth byte. */
void func_8007B008(Unk8007A46C *this, u8 r, u8 g, u8 b) {
    this->unk48[0] = r;
    this->unk48[1] = g;
    this->unk48[2] = b;
    this->unk48[3] = 0x80;
}

/* Sets the four bytes of the color at 0x48. */
void func_8007B020(Unk8007A46C *this, u8 r, u8 g, u8 b, u8 a) {
    this->unk48[0] = r;
    this->unk48[1] = g;
    this->unk48[2] = b;
    this->unk48[3] = a;
}

/* Sets the fourth byte of the color at 0x48. */
void func_8007B038(Unk8007A46C *this, u8 a) {
    this->unk48[3] = a;
}

/* Sets 0x3C. */
void func_8007B040(Unk8007A46C *this, s32 value) {
    this->unk3C = value;
}

INCLUDE_RODATA("title/nonmatchings/title_0b", D_80066B68);
