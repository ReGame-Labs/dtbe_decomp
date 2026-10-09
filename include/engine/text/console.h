#ifndef DTBE_TEXT_CONSOLE_H
#define DTBE_TEXT_CONSOLE_H

/* The debug console: text with locate and color escapes, drawn over the frame. */

#include "common.h"
#include <libgpu.h>
#include "engine/lib/format.h"

EXTERN_C_BEGIN

/* the escapes in a Console's text */
#define CONSOLE_LOCATE 0x1F /* followed by the column and the row to write at */
#define CONSOLE_COLOR 0x1E  /* followed by the red, green and blue to write in */
/* the length from which a Console drops what it is given, keeping room for a line */
#define CONSOLE_LIMIT 0xF9D

/* The text a Console shows: characters and escapes. */
typedef struct {
    /* 0x00 */ u8 unk0[4];
    /* 0x04 */ s32 x; /* where the text starts on the screen */
    /* 0x08 */ s32 y;
    /* 0x0C */ s32 length;
    /* 0x10 */ s32 locate[2]; /* the column and row of the last "<column,row>", or -1 */
    /* 0x18 */ u8 unk18[0x18];
    /* 0x30 */ u8 text[0x1000];
} ConsoleText;

/* Text printed over the frame, as the debug menus do. */
typedef struct {
    /* 0x00 */ ConsoleText *text;
} Console;

/* A SPRT as a Console fills it, a word at a time. */
typedef struct {
    /* 0x00 */ u_long tag;
    /* 0x04 */ u8 r0;
    /* 0x05 */ u8 g0;
    /* 0x06 */ u8 b0;
    /* 0x07 */ u8 code;
    /* 0x08 */ u32 xy;     /* y in the high half */
    /* 0x0C */ u32 uvClut; /* v in the second byte, the clut in the high half */
    /* 0x10 */ u32 wh;     /* h in the high half */
} SprtWords;

/* where a Console formats its text */
extern u8 CONSOLE_FORMAT_BUFFER[0x400];

void mainConsoleLocate(s32 x, s32 y);
void mainConsoleSetColor(u8 r, u8 g, u8 b);
void mainConsolePrint(char *format, ...);
Console *consoleInit(Console *console, s32 x, s32 y);
void consoleDestroy(Console *console, s32 flags);
void consoleLocate(Console *console, s32 x, s32 y);
void consoleSetColor(Console *console, u8 r, u8 g, u8 b);
void consolePrint(Console *console, char *format, ...);
void consoleVprint(Console *console, u8 *str, VaList args);
void consoleDraw(Console *console, u_long *ot);
void consoleClear(Console *console);
u8 *readHexColor(u8 *out, u8 *in);
s32 hexDigitValue(u8 c);
u8 *consoleTextReadLocate(ConsoleText *consoleText, u8 *s);
DR_TPAGE *allocDrTpageMode(s32 dfe, s32 dtd, s32 tpage);

EXTERN_C_END

#endif /* DTBE_TEXT_CONSOLE_H */
