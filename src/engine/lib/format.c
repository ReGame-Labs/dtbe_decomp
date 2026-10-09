#include "common.h"
#include "engine/lib/format.h"
#include "strings.h"

/* whether c is a decimal digit, in one unsigned compare */
#define IS_DIGIT(c) ((u32)((c) - '0') < 10)

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/format", HEX_DIGITS_UPPER);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/format", HEX_DIGITS_LOWER);

/* Formats into buffer like vsprintf, without floating point; returns the
 * length of the output. */
s32 formatterPrint(Formatter *this, u8 *buffer, u8 *format, VaList args) {
    u8 c;
    s32 digit;

    this->out = buffer;
    this->state = FORMAT_TEXT;
    for (c = *format; c != '\0'; c = *format) {
        switch (this->state) {
        case FORMAT_TEXT:
            if (c == '%') {
                format++;
                this->flags = 0;
                this->width = 0;
                this->precision = 0;
                this->size = 0;
                this->conversion = 0;
                this->state = FORMAT_FLAGS;
            } else {
                format++;
                *this->out++ = c;
            }
            break;
        case FORMAT_FLAGS:
            switch (c) {
            case '+':
                format++;
                this->flags |= FORMAT_PLUS;
                break;
            case '-':
                format++;
                this->flags |= FORMAT_LEFT;
                break;
            case ' ':
                format++;
                this->flags |= FORMAT_SPACE;
                break;
            case '0':
                format++;
                this->flags |= FORMAT_ZERO;
                break;
            case '#':
                format++;
                this->flags |= FORMAT_ALT;
                break;
            default:
                this->state = FORMAT_WIDTH;
                break;
            }
            break;
        case FORMAT_DOT:
            if (c == '.') {
                this->state = FORMAT_PRECISION;
                format++;
            } else {
                this->state = FORMAT_SIZE;
            }
            break;
        case FORMAT_SIZE:
            switch (c) {
            case 'h':
                this->size = FORMAT_SIZE_SHORT;
                format++;
                break;
            case 'l':
                this->size = FORMAT_SIZE_LONG;
                format++;
                break;
            case 'L':
                this->size = FORMAT_SIZE_LONG_DOUBLE;
                format++;
                break;
            default:
                this->state = FORMAT_CONVERSION;
                break;
            }
            break;
        case FORMAT_WIDTH:
            if (*format == '*') {
                /* the '*' itself is not skipped */
                this->width = VA_ARG(args, s32);
            } else if (IS_DIGIT(*format)) {
                do {
                    digit = *format++ - '0';
                    this->width = this->width * 10 + digit;
                } while (IS_DIGIT(*format));
            }
            this->state = FORMAT_DOT;
            break;
        case FORMAT_PRECISION:
            if (*format == '*') {
                /* the '*' itself is not skipped */
                this->precision = VA_ARG(args, s32);
            } else if (IS_DIGIT(*format)) {
                do {
                    digit = *format++ - '0';
                    this->precision = this->precision * 10 + digit;
                } while (IS_DIGIT(*format));
            }
            this->state = FORMAT_SIZE;
            break;
        case FORMAT_CONVERSION:
            args = formatterPrintArg(this, c, args);
            format++;
            break;
        case FORMAT_ERROR:
            break;
        }
        if (this->state == FORMAT_ERROR) {
            break;
        }
    }
    *this->out = '\0';
    return this->out - buffer;
}

/* Fetches and prints the argument of a conversion: the integers, %c, %s, %p
 * and %%; any other sets FORMAT_ERROR.
 * Each integer conversion reads an int (no size), a short ('h') or a long
 * ('l'), like a printf. int and long are the same word here, so the int and
 * long arms compile to identical blocks, which jump2 cross-jumps: that is
 * what keeps the 'd' size-0 test and has 'u' jump to it while 'u' keeps the
 * size-4 test (with one shared int/long arm per switch, GCC keeps both tests
 * in 'u'). */
VaList formatterPrintArg(Formatter *this, s32 conversion, VaList args) {
    this->conversion = conversion;
    switch (conversion) {
    case 'd':
    case 'i':
        switch (this->size) {
        case 0:
            formatterPrintDecimal(this, VA_ARG(args, int));
            break;
        case FORMAT_SIZE_SHORT:
            formatterPrintDecimal(this, VA_ARG(args, short));
            break;
        case FORMAT_SIZE_LONG:
            formatterPrintDecimal(this, VA_ARG(args, long));
            break;
        }
        break;
    case 'o':
        switch (this->size) {
        case 0:
            formatterPrintOctal(this, VA_ARG(args, unsigned int));
            break;
        case FORMAT_SIZE_SHORT:
            formatterPrintOctal(this, VA_ARG(args, unsigned short));
            break;
        case FORMAT_SIZE_LONG:
            formatterPrintOctal(this, VA_ARG(args, unsigned long));
            break;
        }
        break;
    case 'x':
    case 'X':
        switch (this->size) {
        case 0:
            formatterPrintHex(this, VA_ARG(args, unsigned int));
            break;
        case FORMAT_SIZE_SHORT:
            formatterPrintHex(this, VA_ARG(args, unsigned short));
            break;
        case FORMAT_SIZE_LONG:
            formatterPrintHex(this, VA_ARG(args, unsigned long));
            break;
        }
        break;
    case 'u':
        switch (this->size) {
        case 0:
            formatterPrintDecimal(this, VA_ARG(args, unsigned int));
            break;
        case FORMAT_SIZE_SHORT:
            formatterPrintDecimal(this, VA_ARG(args, unsigned short));
            break;
        case FORMAT_SIZE_LONG:
            formatterPrintDecimal(this, VA_ARG(args, unsigned long));
            break;
        }
        break;
    case 'c':
        formatterPrintChar(this, VA_ARG(args, s32));
        break;
    case 's':
        formatterPrintString(this, VA_ARG(args, u8 *));
        break;
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
        /* no floating point: the double is skipped */
        args += sizeof(double);
        break;
    case 'p':
        formatterPrintPointer(this, VA_ARG(args, u32));
        break;
    case 'n':
        break;
    case '%':
        *this->out++ = '%';
        break;
    default:
        this->state = FORMAT_ERROR;
        return args;
    }
    this->state = FORMAT_TEXT;
    return args;
}

/* Finishes a number whose digits are in this->out, last digit first: adds the
 * sign ('-', or '+' or ' ' with FORMAT_PLUS or FORMAT_SPACE), pads it to the
 * width with '0's (FORMAT_ZERO, not FORMAT_LEFT) or spaces on the side
 * FORMAT_LEFT says, and reverses it in place.
 * The body matches, but the frame is 8 bytes larger (0x28 against 0x20): only an
 * unused 8-byte local array gives it, so the original had a dead local. */
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/format", formatterPadNumber);

/* vsprintf: formats into buffer and returns the length of the output. */
s32 vsprintf(u8 *buffer, u8 *format, VaList args) {
    Formatter formatter;

    return formatterPrint(&formatter, buffer, format, args);
}

/* Outputs a character. */
void formatterPrintChar(Formatter *this, s32 c) {
    *this->out++ = c;
}

/* Outputs a string, padded to the width. */
void formatterPrintString(Formatter *this, u8 *s) {
    u8 *out = this->out;
    s32 pad = 0;

    if (this->width > 0) {
        pad = this->width - strlen(s);
    }
    if (this->flags & FORMAT_LEFT) {
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
    this->out = out;
}

/* Outputs a pointer: 8 hexadecimal digits. */
void formatterPrintPointer(Formatter *this, u32 value) {
    u8 *out = this->out;

    this->out += 8;
    out[0] = HEX_DIGITS_UPPER[value >> 28];
    out[1] = HEX_DIGITS_UPPER[(value >> 24) & 0xF];
    out[2] = HEX_DIGITS_UPPER[(value >> 20) & 0xF];
    out[3] = HEX_DIGITS_UPPER[(value >> 16) & 0xF];
    out[4] = HEX_DIGITS_UPPER[(value >> 12) & 0xF];
    out[5] = HEX_DIGITS_UPPER[(value >> 8) & 0xF];
    out[6] = HEX_DIGITS_UPPER[(value >> 4) & 0xF];
    out[7] = HEX_DIGITS_UPPER[value & 0xF];
}

/* Outputs a signed decimal number. %u goes through it too, as an s32, so an
 * unsigned value from 2^31 up prints negative. */
void formatterPrintDecimal(Formatter *this, s32 value) {
    u8 *out = this->out;
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
    formatterPadNumber(this, out, sign);
}

/* Outputs an octal number. */
void formatterPrintOctal(Formatter *this, u32 value) {
    u8 *out = this->out;

    do {
        *out++ = HEX_DIGITS_UPPER[value & 7];
        value >>= 3;
    } while (value != 0);
    formatterPadNumber(this, out, 0);
}

/* Outputs a hexadecimal number, in lower case for 'x'. */
void formatterPrintHex(Formatter *this, u32 value) {
    u8 *out = this->out;
    const u8 *digits;

    switch (this->conversion) {
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
    formatterPadNumber(this, out, 0);
}

/* Reverses the output from this->out to end. */
void formatterReverse(Formatter *this, u8 *end) {
    u8 *start = this->out;
    u8 c;

    while (start < end) {
        end--;
        c = *start;
        *start++ = *end;
        *end = c;
    }
}
