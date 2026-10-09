#ifndef DTBE_TEXT_TEXT_PRINTER_H
#define DTBE_TEXT_TEXT_PRINTER_H

/* The printer: text formatted and printed a character at a time at a cursor. */

#include "common.h"
#include "vtable.h"

EXTERN_C_BEGIN

/* the virtual table of Printer */
typedef struct PrinterVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry putChar;  /* (Printer *, s32 x, s32 y, u32 code) */
    /* 0x10 */ VtableEntry setColor; /* (Printer *, u32 color) */
    /* 0x18 */ VtableEntry unk18;    /* (Printer *, s32) */
    /* 0x20 */ VtableEntry destroy;  /* (Printer *, s32 flags) */
} PrinterVtable;

/* Prints text a character at a time at its cursor. */
typedef struct Printer {
    /* 0x0 */ s32 x; /* the cursor */
    /* 0x4 */ s32 y;
    /* 0x8 */ PrinterVtable *vtable;
} Printer;

extern u8 PRINTER_FORMAT_BUFFER[0x400]; /* where a Printer formats its text */

s32 printerPrint(Printer *printer, u8 *format, ...);
void printerIgnoreColor(Printer *printer, u32 color);
void func_80032878(Printer *printer, s32 value);
void printerSetCursor(Printer *printer, s32 x, s32 y);
void printerPrintChar(Printer *printer, u32 code);
void printerSetColor(Printer *printer, u32 color);
void func_80032900(Printer *printer, s32 value);

EXTERN_C_END

#endif /* DTBE_TEXT_TEXT_PRINTER_H */
