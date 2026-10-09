#include "common.h"
#include "engine/lib/format.h"
#include "strings.h"

#define IS_DIGIT(c) ((u32)((c) - '0') < 10)

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/format", HEX_DIGITS_UPPER);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/format", HEX_DIGITS_LOWER);

/* Formats into buf like vsprintf, without floating point; returns the
 * length of the output. */
s32 formatterPrint(Formatter *f, u8 *buf, u8 *format, VaList args) {
    u8 c;
    s32 digit;

    f->out = buf;
    f->state = FORMAT_TEXT;
    for (c = *format; c != '\0'; c = *format) {
        switch (f->state) {
        case FORMAT_TEXT:
            if (c == '%') {
                format++;
                f->flags = 0;
                f->width = 0;
                f->precision = 0;
                f->size = 0;
                f->conversion = 0;
                f->state = FORMAT_FLAGS;
            } else {
                format++;
                *f->out++ = c;
            }
            break;
        case FORMAT_FLAGS:
            switch (c) {
            case '+':
                format++;
                f->flags |= FORMAT_PLUS;
                break;
            case '-':
                format++;
                f->flags |= FORMAT_LEFT;
                break;
            case ' ':
                format++;
                f->flags |= FORMAT_SPACE;
                break;
            case '0':
                format++;
                f->flags |= FORMAT_ZERO;
                break;
            case '#':
                format++;
                f->flags |= FORMAT_ALT;
                break;
            default:
                f->state = FORMAT_WIDTH;
                break;
            }
            break;
        case FORMAT_DOT:
            if (c == '.') {
                f->state = FORMAT_PRECISION;
                format++;
            } else {
                f->state = FORMAT_SIZE;
            }
            break;
        case FORMAT_SIZE:
            switch (c) {
            case 'h':
                f->size = FORMAT_SIZE_SHORT;
                format++;
                break;
            case 'l':
                f->size = FORMAT_SIZE_LONG;
                format++;
                break;
            case 'L':
                f->size = FORMAT_SIZE_DOUBLE;
                format++;
                break;
            default:
                f->state = FORMAT_CONVERSION;
                break;
            }
            break;
        case FORMAT_WIDTH:
            if (*format == '*') {
                /* the '*' itself is not skipped */
                f->width = VA_ARG(args, s32);
            } else if (IS_DIGIT(*format)) {
                do {
                    digit = *format++ - '0';
                    f->width = f->width * 10 + digit;
                } while (IS_DIGIT(*format));
            }
            f->state = FORMAT_DOT;
            break;
        case FORMAT_PRECISION:
            if (*format == '*') {
                /* the '*' itself is not skipped */
                f->precision = VA_ARG(args, s32);
            } else if (IS_DIGIT(*format)) {
                do {
                    digit = *format++ - '0';
                    f->precision = f->precision * 10 + digit;
                } while (IS_DIGIT(*format));
            }
            f->state = FORMAT_SIZE;
            break;
        case FORMAT_CONVERSION:
            args = formatterPrintArg(f, c, args);
            format++;
            break;
        case FORMAT_ERROR:
            break;
        }
        if (f->state == FORMAT_ERROR) {
            break;
        }
    }
    *f->out = '\0';
    return f->out - buf;
}

/* Fetches and prints the argument of a conversion: the integers, %c, %s, %p
 * and %%; any other sets FORMAT_ERROR. Of the identical size-0 blocks, the original keeps the 'd' one and jumps to it from 'u'; GCC
 * keeps the 'u' one, and the only form found that flips it changes behaviour. */
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/format", formatterPrintArg);

/* Finishes a number whose digits are in f->out, last digit first: adds the
 * sign ('-', or '+' or ' ' with FORMAT_PLUS or FORMAT_SPACE), pads it to the
 * width with '0's (FORMAT_ZERO, not FORMAT_LEFT) or spaces on the side
 * FORMAT_LEFT says, and reverses it in place.
 * The body matches, but the frame is 8 bytes larger (0x28 against 0x20): only an
 * unused 8-byte local array gives it, so the original had a dead local. */
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/format", formatterPadNumber);

/* vsprintf: formats into buf and returns the length of the output. */
s32 vsprintf(u8 *buf, u8 *format, VaList args) {
    Formatter f;

    return formatterPrint(&f, buf, format, args);
}

/* Outputs a character. */
void formatterPrintChar(Formatter *f, s32 c) {
    *f->out++ = c;
}

/* Outputs a string, padded to the width. */
void formatterPrintString(Formatter *f, u8 *s) {
    u8 *out = f->out;
    s32 pad = 0;

    if (f->width > 0) {
        pad = f->width - strlen(s);
    }
    if (f->flags & FORMAT_LEFT) {
        while (*s != '\0') {
            *out++ = *s++;
        }
        while (pad > 0) {
            *out++ = ' ';
            pad--;
        }
    } else {
        while (pad > 0) {
            *out++ = ' ';
            pad--;
        }
        while (*s != '\0') {
            *out++ = *s++;
        }
    }
    f->out = out;
}

/* Outputs a pointer: 8 hexadecimal digits. */
void formatterPrintPointer(Formatter *f, u32 value) {
    u8 *out = f->out;

    f->out += 8;
    out[0] = HEX_DIGITS_UPPER[value >> 28];
    out[1] = HEX_DIGITS_UPPER[(value >> 24) & 0xF];
    out[2] = HEX_DIGITS_UPPER[(value >> 20) & 0xF];
    out[3] = HEX_DIGITS_UPPER[(value >> 16) & 0xF];
    out[4] = HEX_DIGITS_UPPER[(value >> 12) & 0xF];
    out[5] = HEX_DIGITS_UPPER[(value >> 8) & 0xF];
    out[6] = HEX_DIGITS_UPPER[(value >> 4) & 0xF];
    out[7] = HEX_DIGITS_UPPER[value & 0xF];
}

/* Outputs a signed decimal number. */
void formatterPrintDecimal(Formatter *f, s32 value) {
    u8 *out = f->out;
    s32 sign = 0;
    s32 digit;

    if (value < 0) {
        value = -value;
        sign = -1;
    }
    do {
        digit = value % 10;
        value /= 10;
        *out++ = HEX_DIGITS_UPPER[digit];
    } while (value > 0);
    formatterPadNumber(f, out, sign);
}

/* Outputs an octal number. */
void formatterPrintOctal(Formatter *f, u32 value) {
    u8 *out = f->out;

    do {
        *out++ = HEX_DIGITS_UPPER[value & 7];
        value >>= 3;
    } while (value != 0);
    formatterPadNumber(f, out, 0);
}

/* Outputs a hexadecimal number, in lower case for 'x'. */
void formatterPrintHex(Formatter *f, u32 value) {
    u8 *out = f->out;
    const u8 *digits;

    switch (f->conversion) {
    case 'X':
    default:
        digits = HEX_DIGITS_UPPER;
        break;
    case 'x':
        digits = HEX_DIGITS_LOWER;
        break;
    }
    do {
        *out++ = digits[value & 0xF];
        value >>= 4;
    } while (value != 0);
    formatterPadNumber(f, out, 0);
}

/* Reverses the output from f->out to end. */
void formatterReverse(Formatter *f, u8 *end) {
    u8 *start = f->out;
    u8 c;

    while (start < end) {
        end--;
        c = *start;
        *start++ = *end;
        *end = c;
    }
}
