/* A Printer class (virtual table D_80066B88) and the table after it, which the ranking and the name
 * entry use. The boundary is unproven: no string marks it, only a cut consistent with the order of
 * .rodata. */

#include "common.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_sprt_16.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"

extern PrinterVtable D_80066B88;

u32 func_8007B048(u16 code);

INCLUDE_ASM("title/nonmatchings/title_0c", func_8007B048);

/* The constructor: draws into ot after a texture page 0x3B, in gray (0x808080) and semi-transparent. */
Unk8007B0A0 *func_8007B0A0(Unk8007B0A0 *this, u_long *ot) {
    this->printer.vtable = &D_80066B88;
    this->ot = addNestedOt(ot, allocDrTpage(0x3B));
    this->color = 0x2808080;
    return this;
}

/* Sets the color (0xBBGGRR), keeping it semi-transparent. */
void func_8007B100(Unk8007B0A0 *this, u32 color) {
    this->color = (color & 0xFFFFFF) | 0x2000000;
}

INCLUDE_ASM("title/nonmatchings/title_0c", func_8007B11C);

/* Draws character code at (x, y): a 16 by 16 sprite from the texture cell func_8007B048 finds, CLUT 0x742C. */
void func_8007B184(Unk8007B0A0 *this, s32 x, s32 y, u32 code) {
    u32 uv = func_8007B048(code);
    Sprt16Words *sprt = (Sprt16Words *)allocSprt16();
    u32 rgbCode = this->color;

    sprt->xy = (x & 0xFFFF) | (y << 16);
    sprt->uvClut = uv | 0x742C0000;
    sprt->rgbCode = rgbCode | (((SPRT_16 *)sprt)->code << 24);
    AddPrim(this->ot, sprt);
}

/* The destructor of the Printer class of virtual table D_80066B88: Printer's. */
void func_8007B210(Printer *this, s32 flags) {
    printerDestroy(this, flags);
}

INCLUDE_RODATA("title/nonmatchings/title_0c", D_80066B88);
INCLUDE_RODATA("title/nonmatchings/title_0c", D_80066BB8);
