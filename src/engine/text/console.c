#include "common.h"
#include "engine/text/console.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/lib/format.h"
#include "engine/system/memory.h"
#include "vtable.h"

/* a Console's font: 8x16 glyphs from ' ' on, 32 to a row, 4-bit, with its clut below it */
#define CONSOLE_GLYPH_WIDTH 8
#define CONSOLE_GLYPH_HEIGHT 16
#define CONSOLE_FONT_COLUMNS 32
#define CONSOLE_FONT_TPAGE getTPage(0, 0, 960, 256)
#define CONSOLE_FONT_CLUT getClut(960, 320)
/* the color that draws the font as it is */
#define CONSOLE_NEUTRAL 0x80
/* a tab moves to the next multiple of 4 columns */
#define CONSOLE_TAB_COLUMNS 4
/* a line goes on to the next once it is past this x */
#define CONSOLE_WRAP_X 640

/* the console the functions below write on: the first one made, or NULL */
static Console *MAIN_CONSOLE = NULL;

/* Moves where the main console writes next to a column and a row. */
void mainConsoleLocate(s32 x, s32 y) {
    if (MAIN_CONSOLE != NULL) {
        consoleLocate(MAIN_CONSOLE, x, y);
    }
}

/* Sets the color the main console writes in next. */
void mainConsoleSetColor(u8 r, u8 g, u8 b) {
    if (MAIN_CONSOLE != NULL) {
        consoleSetColor(MAIN_CONSOLE, r, g, b);
    }
}

/* Writes formatted text on the main console, as printf does. */
void mainConsolePrint(char *format, ...) {
    VaList args;

    VA_START(args, format);
    if (MAIN_CONSOLE != NULL) {
        consoleVprint(MAIN_CONSOLE, format, args);
    }
}

/* Constructs a console showing its text at (x, y); the first one is the main console. */
Console *consoleInit(Console *this, s32 x, s32 y) {
    this->text = debugHeapAlloc(sizeof(ConsoleText));
    this->text->x = x;
    this->text->y = y;
    this->text->length = 0;
    this->text->text[0] = '\0';
    if (MAIN_CONSOLE == NULL) {
        MAIN_CONSOLE = this;
    }
    return this;
}

/* Destroys a console, and frees its text. */
void consoleDestroy(Console *this, s32 flags) {
    if (MAIN_CONSOLE == this) {
        MAIN_CONSOLE = NULL;
    }
    debugHeapFree(this->text);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Moves where the console writes next to a column and a row. */
void consoleLocate(Console *this, s32 x, s32 y) {
    u8 *end;

    if (this->text->length < CONSOLE_LIMIT) {
        end = &this->text->text[this->text->length];
        end[0] = CONSOLE_LOCATE;
        end[1] = x;
        end[2] = y;
        end[3] = '\0';
        this->text->length += 3;
    }
}

/* Sets the color the console writes in next. */
void consoleSetColor(Console *this, u8 r, u8 g, u8 b) {
    u8 *end;

    if (this->text->length < CONSOLE_LIMIT) {
        end = &this->text->text[this->text->length];
        end[0] = CONSOLE_COLOR;
        end[1] = r;
        end[2] = g;
        end[3] = b;
        end[4] = '\0';
        this->text->length += 4;
    }
}

/* Writes formatted text on the console, as printf does. */
void consolePrint(Console *this, char *format, ...) {
    VaList args;

    VA_START(args, format);
    consoleVprint(this, format, args);
}

/*
 * Writes formatted text on the console, as vprintf does; a color escape in it
 * is followed by the color in hexadecimal, as "RRGGBB". str is the format,
 * then the text it gives.
 */
void consoleVprint(Console *this, u8 *str, VaList args) {
    u8 *out;
    u8 c;

    if (this->text->length < CONSOLE_LIMIT) {
        out = &this->text->text[this->text->length];
        vsprintf(CONSOLE_FORMAT_BUFFER, str, args);
        str = CONSOLE_FORMAT_BUFFER;
        for (c = *str++; c != '\0'; c = *str++) {
            if (c == CONSOLE_COLOR) {
                *out++ = c;
                str = readHexColor(out, str);
                out += 3;
            } else {
                *out++ = c;
            }
        }
        this->text->length = out - this->text->text;
        *out = '\0';
    }
}

/*
 * Draws the console's text in ot, each character as a sprite over a black
 * shadow 1 pixel down and right, then empties the text.
 */
void consoleDraw(Console *this, u_long *ot) {
    s32 r;
    s32 g;
    s32 b;
    s32 left = this->text->x;
    s32 top = this->text->y;
    s32 x;
    s32 y;
    u8 *s;
    u8 c;
    SprtWords *sprt;
    s32 screenY;
    u32 uvClut;
    u32 u;
    u32 v;

    if (this->text->text[0] != '\0') {
        r = CONSOLE_NEUTRAL;
        g = CONSOLE_NEUTRAL;
        b = CONSOLE_NEUTRAL;
        x = 0;
        y = 0;
        s = this->text->text;
        for (c = *s++; c != '\0'; c = *s++) {
            if (x > CONSOLE_WRAP_X) {
                x = 0;
                y += CONSOLE_GLYPH_HEIGHT;
            }
            if (c == ' ') {
                x += CONSOLE_GLYPH_WIDTH;
            } else if (c == '\t') {
                x /= CONSOLE_GLYPH_WIDTH;
                x = (x + CONSOLE_TAB_COLUMNS) & ~(CONSOLE_TAB_COLUMNS - 1);
                x *= CONSOLE_GLYPH_WIDTH;
            } else if (c == '\n') {
                x = 0;
                y += CONSOLE_GLYPH_HEIGHT;
            } else if (c == '<') {
                s = consoleTextReadLocate(this->text, s);
                if (this->text->locate[0] >= 0) {
                    x = this->text->locate[0] * CONSOLE_GLYPH_WIDTH;
                }
                if (this->text->locate[1] >= 0) {
                    y = this->text->locate[1] * CONSOLE_GLYPH_HEIGHT;
                }
                if (*s == '>') {
                    s++;
                }
            } else if (c == CONSOLE_LOCATE) {
                x = *s++ * CONSOLE_GLYPH_WIDTH;
                y = *s++ * CONSOLE_GLYPH_HEIGHT;
            } else if (c == CONSOLE_COLOR) {
                r = *s++;
                g = *s++;
                b = *s++;
            } else {
                /* the glyph's column and row in the font */
                c -= ' ';
                u = (c % CONSOLE_FONT_COLUMNS) * CONSOLE_GLYPH_WIDTH;
                v = c / CONSOLE_FONT_COLUMNS;
                /* allocSprt's SPRT, filled a word at a time */
                sprt = (SprtWords *)allocSprt();
                sprt->r0 = r;
                sprt->g0 = g;
                sprt->b0 = b;
                u = u | (CONSOLE_FONT_CLUT << 16);
                screenY = top + y;
                sprt->xy = (screenY << 16) | (left + x);
                sprt->wh = (CONSOLE_GLYPH_HEIGHT << 16) | CONSOLE_GLYPH_WIDTH;
                uvClut = ((v * CONSOLE_GLYPH_HEIGHT) << 8) | u;
                sprt->uvClut = uvClut;
                AddPrim(ot, sprt);
                sprt = (SprtWords *)allocSprt();
                sprt->r0 = 0;
                sprt->g0 = 0;
                sprt->b0 = 0;
                sprt->xy = ((screenY + 1) << 16) | (left + x + 1);
                sprt->uvClut = uvClut;
                sprt->wh = (CONSOLE_GLYPH_HEIGHT << 16) | CONSOLE_GLYPH_WIDTH;
                AddPrim(ot, sprt);
                x += CONSOLE_GLYPH_WIDTH;
            }
        }
        AddPrim(ot, allocDrTpageMode(0, 0, CONSOLE_FONT_TPAGE));
        consoleClear(this);
    }
}

/* Clears the console. */
void consoleClear(Console *this) {
    this->text->length = 0;
    this->text->text[0] = '\0';
}

/*
 * Reads the hexadecimal "RRGGBB" of a color escape at in into 3 bytes at
 * out; returns where it ends.
 */
u8 *readHexColor(u8 *out, u8 *in) {
    s32 i;
    s32 byte;

    for (i = 0; i < 3; i++) {
        byte = hexDigitValue(*in++);
        if (byte >= 0) {
            byte = (byte << 4) + hexDigitValue(*in++);
            *out++ = byte;
        }
    }
    return in;
}

/* Returns the value of a hexadecimal digit, or -1 if c is not one. */
s32 hexDigitValue(u8 c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/*
 * Reads the "column,row" of a locate escape ("<column,row>") at s into the
 * text's locate, a missing number staying -1; returns the character that ends it.
 */
u8 *consoleTextReadLocate(ConsoleText *this, u8 *s) {
    s32 value = 0;
    s32 noDigit = -1; /* -1 until a digit of the current number is read, then 0 */
    s32 i = 0;
    u8 c;

    this->locate[0] = -1;
    this->locate[1] = -1;
    for (;;) {
        c = *s++;
        if (c == ' ') {
            continue;
        }
        if (c == ',') {
            if (noDigit == -1) {
                i++;
                continue;
            }
            this->locate[i++] = value;
            value = 0;
            noDigit = -1;
        } else if ((u32)(c - '0') < 10) { /* unsigned: one compare checks both ends */
            value = value * 10 + (c - '0');
            noDigit = 0;
        } else {
            if (noDigit == 0) {
                this->locate[i] = value;
            }
            return s - 1;
        }
    }
}

/* Takes a texture page primitive from the frame's primitive buffer, with the
 * given drawing flags. As in allocDrTpage, the mode word is worked out after
 * the address of the code word and then stored through it. */
DR_TPAGE *allocDrTpageMode(s32 dfe, s32 dtd, s32 tpage) {
    DR_TPAGE *prim = (DR_TPAGE *)PRIM_BUFFER_FREE;
    u_long *code;
    u_long mode;

    PRIM_BUFFER_FREE += sizeof(DR_TPAGE);
    setlen(prim, 1);
    code = prim->code;
    mode = _get_mode(dfe, dtd, tpage);
    *code = mode;
    return prim;
}
