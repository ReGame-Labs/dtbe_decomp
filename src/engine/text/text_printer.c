#include "common.h"
#include "engine/text/text_printer.h"
#include "engine/lib/format.h"

/* Prints formatted text at the cursor, the two bytes of a Shift JIS
 * character as one code; returns the length of the text. */
s32 printerPrint(Printer *this, u8 *format, ...) {
    VaList args;
    u8 *text;
    s32 length;
    u32 code;

    VA_START(args, format);
    length = vsprintf(PRINTER_FORMAT_BUFFER, format, args);
    text = PRINTER_FORMAT_BUFFER;
    for (code = *text++; code != 0; code = *text++) {
        /* a lead byte: 0x81 to 0x9F or 0xE0 to 0xFC */
        if ((u8)((code ^ 0x20) + 0x5F) < 0x3C) {
            code = (code << 8) | *text++;
        }
        this->vtable->putChar.func((u8 *)this + this->vtable->putChar.delta, this->x, this->y, code);
    }
    return length;
}

/* Does nothing. */
void printerIgnoreColor(Printer *this, u32 color) {
}

/* Does nothing: the function at unk18 of both PRINTER_VTABLE and
 * TEXT_PRINTER_VTABLE. */
void func_80032878(Printer *this, s32 value) {
}

/* Moves the cursor. */
void printerSetCursor(Printer *this, s32 x, s32 y) {
    this->x = x;
    this->y = y;
}

/* Prints a character at the cursor. */
void printerPrintChar(Printer *this, u32 code) {
    this->vtable->putChar.func((u8 *)this + this->vtable->putChar.delta, this->x, this->y, code);
}

/* Sets the color of the text. */
void printerSetColor(Printer *this, u32 color) {
    this->vtable->setColor.func((u8 *)this + this->vtable->setColor.delta, color);
}

/* Calls the printer's function at unk18 of its vtable with value, whose
 * meaning is unknown. */
void func_80032900(Printer *this, s32 value) {
    this->vtable->unk18.func((u8 *)this + this->vtable->unk18.delta, value);
}
