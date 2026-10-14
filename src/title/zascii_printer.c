/* The zascii printer (ZasciiPrinter): a Printer that draws two-byte characters as 16 by 16 sprites of
 * /zascii.tim, for the ranking and the name entry. The boundary is unproven: no string marks it, only a cut
 * consistent with the order of .rodata. */

#include "common.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/alloc_sprt_16.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"

extern PrinterVtable ZASCII_PRINTER_VTABLE;

/* The uv of code's cell in /zascii.tim: its index in ZASCII_CODES, 16 cells a row of 16 by 16. */
u32 getZasciiUv(u16 code);

INCLUDE_ASM("title/nonmatchings/zascii_printer", getZasciiUv);

/* The zascii printer's constructor: draws into ot after texture page 0x3B (zascii.tim), in gray (0x808080) and
 * semi-transparent. */
ZasciiPrinter *ZasciiPrinter_init(ZasciiPrinter *this, u_long *ot) {
    this->printer.vtable = &ZASCII_PRINTER_VTABLE;
    this->ot = addNestedOt(ot, allocDrTpage(0x3B));
    this->color = 0x2808080;
    return this;
}

/* Sets the color (0xBBGGRR), keeping it semi-transparent. */
void ZasciiPrinter_setColor(ZasciiPrinter *this, u32 color) {
    this->color = (color & 0xFFFFFF) | 0x2000000;
}

INCLUDE_ASM("title/nonmatchings/zascii_printer", ZasciiPrinter_putChar);

/* Draws character code at (x, y): a 16 by 16 sprite from the texture cell getZasciiUv finds, CLUT 0x742C. */
void ZasciiPrinter_drawChar(ZasciiPrinter *this, s32 x, s32 y, u32 code) {
    u32 uv = getZasciiUv(code);
    Sprt16Words *sprt = (Sprt16Words *)allocSprt16();
    u32 rgbCode = this->color;

    sprt->xy = (x & 0xFFFF) | (y << 16);
    sprt->uvClut = uv | 0x742C0000;
    sprt->rgbCode = rgbCode | (((SPRT_16 *)sprt)->code << 24);
    AddPrim(this->ot, sprt);
}

/* The zascii printer's destructor: Printer's. */
void ZasciiPrinter_destroy(Printer *this, s32 flags) {
    printerDestroy(this, flags);
}

INCLUDE_RODATA("title/nonmatchings/zascii_printer", ZASCII_PRINTER_VTABLE);
INCLUDE_RODATA("title/nonmatchings/zascii_printer", ZASCII_CODES);
