#include "common.h"
#include "engine/lib/string.h"

/* Copies src to dst in lower case; returns dst. */
u8 *copyLowerCase(u8 *dst, u8 *src) {
    copyChangingCase(dst, src, CTYPE_UPPER, 'a' - 'A');
    return dst;
}

/* Copies src to dst in upper case; returns dst. */
u8 *copyUpperCase(u8 *dst, u8 *src) {
    copyChangingCase(dst, src, CTYPE_LOWER, 'A' - 'a');
    return dst;
}

/* Copies src to dst, adding delta to the single-byte characters of a class
 * of ctype (CTYPE_*); two-byte characters are copied as they are. */
void copyChangingCase(u8 *dst, u8 *src, s32 ctype, s32 delta) {
    u8 c;

    while (*src != '\0') {
        c = *src++;
        if (IS_SJIS_LEAD(c)) {
            *dst++ = c;
            *dst = *src++;
        } else {
            if (CTYPE_TABLE[c] & ctype) {
                c += delta;
            }
            *dst = c;
        }
        dst++;
    }
    *dst = '\0';
}

/* Copies src to dst with every printable half-width character turned into
 * its full-width one; returns dst. */
u8 *copyFullWidth(u8 *dst, u8 *src) {
    u8 *out = dst;
    u8 c;

    while (*src != '\0') {
        c = *src++;
        if (IS_SJIS_LEAD(c)) {
            *out++ = c;
            *out = *src++;
        } else if (c < ' ') {
            *out = c;
        } else {
            /* the table has the characters 0x20-0x7F, then 0xA0-0xDF */
            if (c >= 0xE0) {
                c = 0;
            } else {
                c = (c >= 0xA0) ? c - 0x40 : c - 0x20;
            }
            c *= 2;
            *out++ = FULL_WIDTH_CHARS[c];
            *out = FULL_WIDTH_CHARS[c + 1];
        }
        out++;
    }
    *out = '\0';
    return dst;
}

/* Returns the length in bytes of the first element of a path. */
s32 getPathElementLength(u8 *path) {
    s32 length = 0;
    u8 c;

    while (*path != '\0') {
        c = *path++;
        if (CTYPE_TABLE[c] & CTYPE_SEPARATOR) {
            break;
        }
        if (IS_SJIS_LEAD(c)) {
            path++;
            length++;
        }
        length++;
    }
    return length;
}

/* Copies a path with '\\' as its separators; returns dst. */
u8 *copyBackslashPath(u8 *dst, u8 *src) {
    copyPathWithSeparator(dst, src, '\\');
    return dst;
}

/* Copies a path with '/' as its separators; returns dst. */
u8 *copySlashPath(u8 *dst, u8 *src) {
    copyPathWithSeparator(dst, src, '/');
    return dst;
}

/* Copies a path, turning every run of separators into one separator. */
void copyPathWithSeparator(u8 *dst, u8 *src, u8 separator) {
    u8 c;

    while (*src != '\0') {
        c = *src++;
        if (IS_SJIS_LEAD(c)) {
            *dst++ = c;
            *dst = *src++;
        } else if (CTYPE_TABLE[c] & CTYPE_SEPARATOR) {
            *dst++ = separator;
            for (;;) {
                c = *src;
                if (c != '\0' && (CTYPE_TABLE[c] & CTYPE_SEPARATOR)) {
                    src++;
                } else {
                    break;
                }
            }
            continue;
        } else {
            *dst = c;
        }
        dst++;
    }
    *dst = '\0';
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/string", FULL_WIDTH_CHARS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/string", CTYPE_TABLE);
