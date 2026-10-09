#ifndef DTBE_LIB_STRING_H
#define DTBE_LIB_STRING_H

/* Shift JIS strings: a character type table, case and full-width copies, and paths. */

#include "common.h"

EXTERN_C_BEGIN

/* the classes of CTYPE_TABLE, a character type table for Shift-JIS text */
#define CTYPE_UPPER 0x01
#define CTYPE_LOWER 0x02
#define CTYPE_SJIS_LEAD 0x04
#define CTYPE_SEPARATOR 0x08 /* '/' and '\\' */
#define CTYPE_CONTROL 0x10
#define CTYPE_KANA 0x20 /* half-width katakana */
#define CTYPE_DIGIT 0x40
#define CTYPE_HEX 0x80

/* whether c is the first byte of a two-byte Shift-JIS character
 * (0x81-0x9F or 0xE0-0xFC) */
#define IS_SJIS_LEAD(c) ((u8)(((c) ^ 0x20) - 0xA1) < 0x3C)

/* the full-width Shift-JIS character of each half-width one, two bytes each */
extern const u8 FULL_WIDTH_CHARS[];
extern const u8 CTYPE_TABLE[256];

u8 *copyLowerCase(u8 *dst, u8 *src);
u8 *copyUpperCase(u8 *dst, u8 *src);
void copyChangingCase(u8 *dst, u8 *src, s32 ctype, s32 delta);
u8 *copyFullWidth(u8 *dst, u8 *src);
s32 getPathElementLength(u8 *path);
u8 *copyBackslashPath(u8 *dst, u8 *src);
u8 *copySlashPath(u8 *dst, u8 *src);
void copyPathWithSeparator(u8 *dst, u8 *src, u8 separator);

EXTERN_C_END

#endif /* DTBE_LIB_STRING_H */
