#ifndef DTBE_LIB_FORMAT_H
#define DTBE_LIB_FORMAT_H

/* The library's vsprintf, without floating point: the formatter, its flags and sizes. */

#include "common.h"

EXTERN_C_BEGIN

/* The arguments of a printf-style call, one 4-byte slot each except for
 * doubles, which take 8. */
typedef u8 *VaList;
/* Points args at the arguments after last, as PsyQ's va_start does. */
#define VA_START(args, last) ((args) = (VaList)&(last) + sizeof(last))
/* the next argument of args, as a type */
#define VA_ARG(args, type) (((type *)((args) += (sizeof(type) < 4 ? 4 : sizeof(type))))[-1])

/* where a formatter is in the format string */
typedef enum {
    FORMAT_TEXT,
    FORMAT_FLAGS,
    FORMAT_WIDTH,
    FORMAT_DOT,
    FORMAT_PRECISION,
    FORMAT_SIZE,
    FORMAT_CONVERSION,
    FORMAT_ERROR /* an unknown conversion: the output stops there */
} FormatState;

/* the flags of a conversion */
#define FORMAT_PLUS 0x01  /* '+' */
#define FORMAT_LEFT 0x02  /* '-' */
#define FORMAT_SPACE 0x04 /* ' ' */
#define FORMAT_ZERO 0x08  /* '0' */
#define FORMAT_ALT 0x10   /* '#' */

/* the argument sizes of a conversion, in bytes; 0 is the default */
#define FORMAT_SIZE_SHORT 2       /* 'h' */
#define FORMAT_SIZE_LONG 4        /* 'l' */
#define FORMAT_SIZE_LONG_DOUBLE 8 /* 'L' */

/* the state of a vsprintf */
typedef struct {
    /* 0x00 */ FormatState state;
    /* 0x04 */ u8 *out;
    /* 0x08 */ s32 flags;
    /* 0x0C */ s32 width;
    /* 0x10 */ s32 precision;
    /* 0x14 */ s32 size;
    /* 0x18 */ s32 conversion;
} Formatter;

extern const u8 HEX_DIGITS_UPPER[]; /* "0123456789ABCDEF" */
extern const u8 HEX_DIGITS_LOWER[]; /* "0123456789abcdef" */

s32 formatterPrint(Formatter *formatter, u8 *buffer, u8 *format, VaList args);
VaList formatterPrintArg(Formatter *formatter, s32 conversion, VaList args);
void formatterPadNumber(Formatter *formatter, u8 *end, s32 sign);
s32 vsprintf(u8 *buffer, u8 *format, VaList args);
void formatterPrintChar(Formatter *formatter, s32 c);
void formatterPrintString(Formatter *formatter, u8 *s);
void formatterPrintPointer(Formatter *formatter, u32 value);
void formatterPrintDecimal(Formatter *formatter, s32 value);
void formatterPrintOctal(Formatter *formatter, u32 value);
void formatterPrintHex(Formatter *formatter, u32 value);
void formatterReverse(Formatter *formatter, u8 *end);

EXTERN_C_END

#endif /* DTBE_LIB_FORMAT_H */
