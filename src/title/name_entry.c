/* The name entry (/zascii.tim, /title/input.tim and the katakana names), which the ranking and the
 * options open. */

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_line_f2.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/lib/md5.h"
#include "engine/pad/pad.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"
#include "psyq.h"
#include "strings.h"

/* the grids' rows of two-byte cells */
extern u8 *NAME_GRID[];
extern u8 *CODE_GRID[];
extern u8 STR_NAME_CURSOR[];
extern u8 STR_EMPTY_NAME[];

void NameEntry_reset(NameEntry *this);
void NameEntry_setGrid(NameEntry *this, u8 **grid);
u8 *NameEntry_getName(NameEntry *this);
void NameEntry_readPad(NameEntry *this);
void NameEntry_scrollToCursor(NameEntry *this);
s32 NameEntry_readCell(NameEntry *this);
s32 NameEntry_input(NameEntry *this);
void NameEntry_drawName(NameEntry *this, u_long *ot);
void NameEntry_drawGrid(NameEntry *this, u_long *ot);
void NameEntry_drawBackground(NameEntry *this, u_long *ot);
void NameEntry_drawTitle(NameEntry *this, u_long *ot, s32 mode);
void NameEntry_drawMessage(NameEntry *this, u_long *ot, s32 message);
void NameEntry_drawHighlight(NameEntry *this, u_long *ot);
void NameEntry_drawLine(NameEntry *this, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b);
s32 RankingTable_fillPending(RankingTable *this, u8 *name, s32 character);
s32 RankingTable_check(RankingTable *this);
void computeRecordDigest(RankingRecord *entry, u8 *digest);
void RankingTable_reset(RankingTable *this);
RankingRecord *RankingTable_sort(RankingTable *this);
RankingRecord *RankingTable_first(RankingTable *this);
RankingRecord *RankingTable_next(RankingTable *this);
s32 compareRankingRecords(RankingRecord *a, RankingRecord *b);

/* The name entry's constructor: the character grid of mode (NAME_GRID for a name, CODE_GRID for a code),
 * the window and the cursor's bounds, how far the grid scrolls, then a reset (NameEntry_reset). */
NameEntry *NameEntry_init(NameEntry *this, s32 mode, s32 maxLength) {
    this->port = 0;
    this->mode = mode;
    this->maxLength = maxLength;
    this->moveToY = 0;
    this->moveToX = 0;
    if (mode == NAME_ENTRY_MODE_NAME) {
        NameEntry_setGrid(this, NAME_GRID);
    } else {
        NameEntry_setGrid(this, CODE_GRID);
    }
    this->windowX = 0x20;
    this->windowY = 0x50;
    this->windowW = 0x100;
    this->windowH = 0x73;
    this->boundsX = 0x25;
    this->boundsY = 0x55;
    this->boundsW = 0xF6;
    this->boundsH = 0x69;
    this->maxScrollX = this->columns * 20 - 0xFC;
    if (this->maxScrollX < 0) {
        this->maxScrollX = 0;
    }
    this->maxScrollY = this->rows * 20 - this->windowH + 4;
    if (this->maxScrollY < 0) {
        this->maxScrollY = 0;
    }
    NameEntry_reset(this);
    return this;
}

/* NameEntry's destructor: frees it when asked, nothing else. */
void NameEntry_destroy(NameEntry *this, s32 flags) {
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Resets the entry: an empty name, the cursor in the middle of the window, the grid unscrolled. */
void NameEntry_reset(NameEntry *this) {
    s32 halfW = this->windowW >> 1;

    this->padDelay = 10;
    this->speed = 0x400;
    this->active = 0;
    this->finished = 0;
    this->position = 0;
    this->name[0] = 0;
    this->closeDelay = 0;
    this->messageTime = 0;
    this->message = 0;
    this->closingFrames = 0;
    this->frame = 0;
    this->rank = 0;
    this->character = 0;
    this->score = 0;
    this->port = 0;
    this->repeated = 0;
    this->held = 0;
    this->scrollY = 0;
    this->scrollX = 0;
    this->row = 0;
    this->column = 0;
    this->moveLastY = 0;
    this->moveLastX = 0;
    this->moveFromY = 0;
    this->moveFromX = 0;
    this->moveTime = 0;
    this->cursorX = this->windowX + halfW;
    this->cursorY = this->windowY + (this->windowH >> 1);
    NameEntry_scrollToCursor(this);
    this->lastCell = NameEntry_readCell(this);
}

/* Opens the entry: resets it (NameEntry_reset) and makes it active. */
void NameEntry_open(NameEntry *this) {
    NameEntry_reset(this);
    this->active = 1;
}

/* Closes the entry: it is no longer updated nor drawn. */
void NameEntry_close(NameEntry *this) {
    this->active = 0;
}

/* Loads "/zascii.tim", uploads its TIMs to VRAM and frees it. */
void loadZasciiTim(void) {
    u32 *file = loadFile("/zascii.tim");

    uploadTims(file);
    mainHeapFree(file);
}

/* Loads "/title/input.tim", uploads its TIMs to VRAM and frees it. */
void loadInputTim(void) {
    u32 *file = loadFile("/title/input.tim");

    uploadTims(file);
    mainHeapFree(file);
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_setGrid);

/* The entry's update: fades in on its first frame, waits padDelay frames before reading the pad, closes
 * 11 frames after NameEntry_input confirms the name (fading out, then in again). */
void NameEntry_update(NameEntry *this) {
    if (this->active == 0) {
        return;
    }
    if (this->frame == 0) {
        fadeControlFadeOut(&FADE_CONTROL, 0);
        fadeControlFadeIn(&FADE_CONTROL, 10);
    }
    if (this->padDelay == 0) {
        NameEntry_readPad(this);
    } else {
        this->padDelay--;
        this->repeated = 0;
        this->held = 0;
    }
    if (this->finished == 0) {
        if (NameEntry_input(this)) {
            this->closeDelay = 1;
            this->padDelay = 99;
        }
    }
    if (this->closeDelay != 0) {
        if (--this->closeDelay == 0) {
            this->finished = 1;
            this->closingFrames = 0;
            fadeControlFadeOut(&FADE_CONTROL, 10);
        }
    }
    if (this->finished != 0) {
        if (++this->closingFrames >= 11) {
            this->active = 0;
            fadeControlFadeIn(&FADE_CONTROL, 10);
        }
    }
    if (this->messageTime != 0) {
        this->messageTime--;
    }
    this->frame++;
}

/* The entry's draw: the grid's parts (NameEntry_drawMessage while messageTime runs), the name, a green line across
 * the window at the cursor's height and one down it at the cursor's x. */
void NameEntry_draw(NameEntry *this) {
    u_long *ot;
    s32 x, y, w, h;
    s32 right, bottom;

    if (this->active == 0) {
        return;
    }
    ot = FRAME_OT.firstDrawn + 5;
    if (this->messageTime != 0) {
        NameEntry_drawMessage(this, ot, this->message);
    }
    NameEntry_drawHighlight(this, ot);
    NameEntry_drawName(this, ot);
    NameEntry_drawTitle(this, ot, this->mode);
    NameEntry_drawBackground(this, ot);
    x = this->windowX;
    y = this->windowY;
    w = this->windowW;
    h = this->windowH;
    right = x + w;
    bottom = y + h;
    NameEntry_drawLine(this, ot, x, this->cursorY, right, this->cursorY, 0x20, 0xA0, 0x20);
    NameEntry_drawLine(this, ot, this->cursorX, y, this->cursorX, bottom, 0x20, 0xA0, 0x20);
    NameEntry_drawGrid(this, ot);
}

/* Opens the entry for the player's side when its score ranks 1 to 7 in table (RankingTable_insertScore), keeping
 * the score, character and rank; returns whether it opened. */
s32 NameEntry_openForScore(NameEntry *this, RankingTable *table) {
    s32 side = gameStateGetPlayerSide(&GAME_STATE);

    this->port = side;
    if (side < 0 || side >= FIGHTER_COUNT) {
        this->port = 0;
        this->active = 0;
        return 0;
    }
    this->score = GAME_STATE.fighters[side].score;
    this->character = GAME_STATE.fighters[side].character;
    this->rank = RankingTable_insertScore(table, this->score);
    if (this->rank >= 1 && this->rank <= 7) {
        this->active = 1;
    } else {
        this->active = 0;
        this->rank = 0;
    }
    return this->active;
}

/* Stores the name (STR_EMPTY_NAME when it is empty) and the character in the table's pending record
 * (RankingTable_fillPending). */
void NameEntry_storeRecord(NameEntry *this, RankingTable *table) {
    u8 *name = NameEntry_getName(this);

    if (*name == 0) {
        name = STR_EMPTY_NAME;
    }
    RankingTable_fillPending(table, name, this->character);
}

/* Sets the name to a string of two-byte characters, the position after its last one. */
void NameEntry_setName(NameEntry *this, u8 *name) {
    u8 *dst;

    this->position = 0;
    dst = this->name;
    /* The match depends on a test before a do/while: the loop reloads the first byte, which the
     * duplicated exit test of a while reuses. */
    if (*name != 0) {
        do {
            *dst++ = *name++;
            *dst++ = *name++;
            this->position++;
        } while (*name != 0);
    }
    *dst = 0;
}

/* Returns the name. */
u8 *NameEntry_getName(NameEntry *this) {
    return this->name;
}

/* Writes the two-byte character code at the position, which moves on unless it is the last. */
void NameEntry_enterChar(NameEntry *this, s32 code) {
    u8 *p = this->name;

    /* The match depends on stepping p from the name: &name[position * 2] adds the base last. */
    p += this->position * 2;
    *p++ = code >> 8;
    *p++ = code;
    *p = 0;
    if (this->position < this->maxLength - 1) {
        this->position++;
    }
}

/* Erases a character: the one before the position, or the last one when it is filled. */
void NameEntry_eraseChar(NameEntry *this) {
    s32 position = this->position;
    u8 *p = this->name;

    /* The match depends on stepping p from the name: &name[position * 2] adds the base last. */
    p += position * 2;
    if (position < this->maxLength - 1 || p[0] == 0) {
        if (position > 0) {
            this->position = position - 1;
            p -= 2;
        }
    }
    *p = 0;
}

/* Reads the entry's pad: the buttons held and repeated. */
void NameEntry_readPad(NameEntry *this) {
    PadManager *padManager = PAD_MANAGER_INSTANCE;

    this->held = padManagerGetHeld(padManager, this->port);
    this->repeated = padManagerGetRepeated(padManager, this->port);
}

INCLUDE_RODATA("title/nonmatchings/name_entry", NAME_ENTRY_CODE_CHARACTERS);

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_tryCode);

/* Shows input.tim's message for count frames. */
void NameEntry_showMessage(NameEntry *this, s32 message, s32 count) {
    this->message = message;
    this->messageTime = count;
}

/* Keeps the cursor inside the bounds: within 0x50 (x) or 0x28 (y) of an edge the grid scrolls instead,
 * as far as it goes; then clamps the scroll and the cursor. */
void NameEntry_scrollToCursor(NameEntry *this) {
    s32 edge;

    edge = this->boundsX + 0x50;
    if (this->cursorX < edge) {
        if (this->scrollX > 0) {
            this->scrollX -= edge - this->cursorX;
            this->cursorX = edge;
        }
    }
    edge = this->boundsX + this->boundsW - 0x50;
    if (edge < this->cursorX) {
        if (this->scrollX < this->maxScrollX) {
            this->scrollX += this->cursorX - edge;
            this->cursorX = edge;
        }
    }
    edge = this->boundsY + 0x28;
    if (this->cursorY < edge) {
        if (this->scrollY > 0) {
            this->scrollY -= edge - this->cursorY;
            this->cursorY = edge;
        }
    }
    edge = this->boundsY + this->boundsH - 0x28;
    if (edge < this->cursorY) {
        if (this->scrollY < this->maxScrollY) {
            this->scrollY += this->cursorY - edge;
            this->cursorY = edge;
        }
    }
    if (this->scrollX < 0) {
        this->scrollX = 0;
    }
    if (this->scrollY < 0) {
        this->scrollY = 0;
    }
    if (this->scrollX > this->maxScrollX) {
        this->scrollX = this->maxScrollX;
    }
    if (this->scrollY > this->maxScrollY) {
        this->scrollY = this->maxScrollY;
    }
    if (this->cursorX < this->boundsX) {
        this->cursorX = this->boundsX;
    }
    if (this->cursorY < this->boundsY) {
        this->cursorY = this->boundsY;
    }
    if (this->boundsX + this->boundsW < this->cursorX) {
        this->cursorX = this->boundsX + this->boundsW;
    }
    if (this->boundsY + this->boundsH < this->cursorY) {
        this->cursorY = this->boundsY + this->boundsH;
    }
}

/* While no direction is held and the cursor is on a character or a button, moves the cursor halfway to the
 * center of its cell on screen (20-pixel cells, scrolled, in the window). */
void NameEntry_snapCursor(NameEntry *this) {
    /* The match depends on one test per direction: or-ed in one if, GCC merges them into one mask. */
    if (this->held & PADLleft) {
        return;
    }
    if (this->held & PADLright) {
        return;
    }
    if (this->held & PADLup) {
        return;
    }
    if (this->held & PADLdown) {
        return;
    }
    if (this->cellCode != 0 || this->cellButton != 0) {
        this->cursorX += (this->column * 20 + 10 - this->scrollX + this->windowX - this->cursorX) >> 1;
        this->cursorY += (this->row * 20 + 10 - this->scrollY + this->windowY - this->cursorY) >> 1;
    }
}

/* Starts a move of the cursor from the middle of its cell (20 pixels a cell) and holds the pad 22 frames. */
void NameEntry_moveCursorToEnd(NameEntry *this) {
    s32 column = this->column;
    s32 row;

    this->padDelay = 22;
    this->moveTime = 0x1000;
    this->moveFromX = column * 20 + 10;
    this->moveLastX = column * 20 + 10;
    row = this->row;
    this->moveFromY = row * 20 + 10;
    this->moveLastY = row * 20 + 10;
}

/* Steps the cursor's move: moveTime falls by 0x155 a frame, and the cursor goes by what the point between
 * moveTo and moveFrom moved since the last frame. */
void NameEntry_stepCursorMove(NameEntry *this) {
    s32 t;
    s32 x, y;
    s32 dx, dy;

    if (this->moveTime == 0) {
        return;
    }
    this->moveTime -= 0x155;
    if (this->moveTime < 0) {
        this->moveTime = 0;
    }
    dx = this->moveFromX - this->moveToX;
    dy = this->moveFromY - this->moveToY;
    t = this->moveTime;
    x = this->moveToX + ((dx * t) >> 12);
    y = this->moveToY + ((dy * t) >> 12);
    /* The match depends on finishing x before y: x then lives no longer than y and gets the lower register. */
    this->cursorX += x - this->moveLastX;
    this->moveLastX = x;
    this->cursorY += y - this->moveLastY;
    this->moveLastY = y;
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_readCell);

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_input);

INCLUDE_RODATA("title/nonmatchings/name_entry", STR_NAME_CURSOR);
INCLUDE_RODATA("title/nonmatchings/name_entry", STR_EMPTY_NAME);

/* Prints the name in grey, centred on x 0xA0 at y 0x2F, and every other 8 frames STR_NAME_CURSOR (the cursor)
 * under the position. */
void NameEntry_drawName(NameEntry *this, u_long *ot) {
    ZasciiPrinter printer;
    PrinterVtable *vtable;
    s32 left;

    ZasciiPrinter_init(&printer, ot);
    vtable = printer.printer.vtable;
    vtable->setColor.func((u8 *)&printer + vtable->setColor.delta, 0x606060);
    printer.printer.y = 0x2F;
    left = 0xA0 - (this->maxLength << 4) / 2;
    printer.printer.x = left;
    printerPrint(&printer.printer, this->name);
    if (this->frame & 8) {
        printer.printer.y = 0x30;
        printer.printer.x = (this->position << 4) + left;
        printerPrint(&printer.printer, STR_NAME_CURSOR);
    }
    printerDestroy(&printer.printer, 2);
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_drawGrid);

/* Draws in ot button index (0 for A, 1 for B, 2 for C) of the grid as a 64 by 21 POLY_FT4 at (x, y): the
 * texels (0x80, 0x60 + index * 21) to (0xBF, 0x74 + index * 21) of page 12, clut 0x3CE8, in the gray shade. */
void NameEntry_drawButton(NameEntry *this, u_long *ot, s32 x, s32 y, s32 shade, s32 index) {
    POLY_FT4 *poly = allocPolyFT4();

    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    poly->x0 = x;
    poly->y0 = y;
    poly->x1 = x + 64;
    poly->y1 = y;
    poly->x2 = x;
    poly->y2 = y + 21;
    poly->x3 = x + 64;
    poly->y3 = y + 21;
    poly->u0 = 0x80;
    poly->v0 = index * 21 + 0x60;
    poly->u1 = 0xBF;
    poly->v1 = index * 21 + 0x60;
    poly->u2 = 0x80;
    poly->v2 = index * 21 + 0x74;
    poly->u3 = 0xBF;
    poly->v3 = index * 21 + 0x74;
    poly->tpage = 0xC;
    poly->clut = 0x3CE8;
    AddPrim(ot, poly);
}

/* Tiles the window with the grid's background, 64 pixels a tile (texels (0x80, 0x20) to (0xBF, 0x5F) of
 * page 12, clut 0x3C28), offset by half the scroll. */
void NameEntry_drawGridBackground(NameEntry *this, u_long *ot) {
    s32 windowX = this->windowX;
    s32 halfScrollX = this->scrollX >> 1;
    s32 left = windowX - halfScrollX % 64;
    s32 windowY = this->windowY;
    s32 halfScrollY = this->scrollY >> 1;
    s32 top = windowY - halfScrollY % 64;
    s32 right = windowX + this->windowW;
    s32 bottom = windowY + this->windowH;
    s32 x, y;

    for (y = top; y < bottom; y += 64) {
        for (x = left; x < right; x += 64) {
            POLY_FT4 *poly = allocPolyFT4();

            poly->r0 = 0x20;
            poly->g0 = 0x20;
            poly->b0 = 0x20;
            poly->x0 = x;
            poly->y0 = y;
            poly->x1 = x + 64;
            poly->y1 = y;
            poly->x2 = x;
            poly->y2 = y + 64;
            poly->x3 = x + 64;
            poly->y3 = y + 64;
            poly->u0 = 0x80;
            poly->v0 = 0x20;
            poly->u1 = 0xBF;
            poly->v1 = 0x20;
            poly->u2 = 0x80;
            poly->v2 = 0x5F;
            poly->u3 = 0xBF;
            poly->v3 = 0x5F;
            poly->tpage = 0xC;
            poly->clut = 0x3C28;
            AddPrim(ot, poly);
        }
    }
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_drawBackground);

/* Draws in ot the entry's title for mode: the 104 by 16 texel row mode * 16 at u 0x80 of page 12, clut row
 * 252 + mode (x 640), at (108, 24) in neutral gray. */
void NameEntry_drawTitle(NameEntry *this, u_long *ot, s32 mode) {
    POLY_FT4 *poly = allocPolyFT4();

    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    poly->x0 = 108;
    poly->y0 = 24;
    poly->x1 = 212;
    poly->y1 = 24;
    poly->x2 = 108;
    poly->y2 = 40;
    poly->x3 = 212;
    poly->y3 = 40;
    poly->u0 = 0x80;
    poly->v0 = mode * 16;
    poly->u1 = 0xE7;
    poly->v1 = mode * 16;
    poly->u2 = 0x80;
    poly->v2 = mode * 16 + 15;
    poly->u3 = 0xE7;
    poly->v3 = mode * 16 + 15;
    poly->tpage = 0xC;
    poly->clut = ((252 + mode) << 6) | (640 >> 4);
    AddPrim(ot, poly);
}


INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_drawMessage);

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x35) to (0xCF, 0x44) of page 12, clut 0x3C68, w and h to each
 * side of (x, y), all its corners of the gray shade (NameEntry_drawHighlight passes its object as this,
 * unused). */
void NameEntry_drawCharHighlight(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
    POLY_FT4 *poly = allocPolyFT4();
    s32 left, top;

    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    /* The match depends on doubling w and h in place: left + w * 2 folds to x + w. */
    left = x - w;
    top = y - h;
    w *= 2;
    h *= 2;
    poly->x0 = left;
    poly->y0 = top;
    poly->x1 = left + w;
    poly->y1 = top;
    poly->x2 = left;
    poly->y2 = top + h;
    poly->x3 = left + w;
    poly->y3 = top + h;
    poly->u0 = 0xC0;
    poly->v0 = 0x35;
    poly->u1 = 0xCF;
    poly->v1 = 0x35;
    poly->u2 = 0xC0;
    poly->v2 = 0x44;
    poly->u3 = 0xCF;
    poly->v3 = 0x44;
    /* The match depends on setting tpage before clut. */
    poly->tpage = 0xC;
    poly->clut = 0x3C68;
    AddPrim(ot, poly);
}

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x20) to (0xFF, 0x34) of page 12, clut 0x3CA8, w and h to each
 * side of (x, y), all its corners of the gray shade (NameEntry_drawHighlight passes its object as this,
 * unused). */
void NameEntry_drawButtonHighlight(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
    POLY_FT4 *poly = allocPolyFT4();
    s32 left, top;

    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    /* The match depends on doubling w and h in place: left + w * 2 folds to x + w. */
    left = x - w;
    top = y - h;
    w *= 2;
    h *= 2;
    poly->x0 = left;
    poly->y0 = top;
    poly->x1 = left + w;
    poly->y1 = top;
    poly->x2 = left;
    poly->y2 = top + h;
    poly->x3 = left + w;
    poly->y3 = top + h;
    poly->u0 = 0xC0;
    poly->v0 = 0x20;
    poly->u1 = 0xFF;
    poly->v1 = 0x20;
    poly->u2 = 0xC0;
    poly->v2 = 0x34;
    poly->u3 = 0xFF;
    poly->v3 = 0x34;
    /* The match depends on setting tpage before clut. */
    poly->tpage = 0xC;
    poly->clut = 0x3CA8;
    AddPrim(ot, poly);
}

/* Highlights the cell under the cursor in ot, in gray 0x60, around its center on screen: a button's
 * 39 by 17 frame (2 pixels to the right), or a character's 14 by 14 one; a blank cell gets none. */
void NameEntry_drawHighlight(NameEntry *this, u_long *ot) {
    s32 x = this->column * 20 + 10 - this->scrollX + this->windowX;
    s32 y = this->row * 20 + 10 - this->scrollY + this->windowY;

    /* The match depends on testing for no button first: the character case comes first in the code. */
    if (this->cellButton == 0) {
        if (this->cellCode > 0) {
            NameEntry_drawCharHighlight(this, ot, x, y, 14, 14, 0x60);
        }
    } else {
        NameEntry_drawButtonHighlight(this, ot, x + 2, y, 39, 17, 0x60);
    }
}

/* Draws in ot a semi-transparent line from (x0, y0) to (x1, y1) of the colour (r, g, b). */
void NameEntry_drawLine(NameEntry *this, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b) {
    LINE_F2 *line = allocLineF2();

    line->x0 = x0;
    line->y0 = y0;
    line->x1 = x1;
    line->y1 = y1;
    line->r0 = r;
    line->g0 = g;
    line->b0 = b;
    setSemiTrans(line, 1);
    AddPrim(ot, line);
}

/* Returns whether the entry is active (no caller in the game). */
s32 NameEntry_isActive(NameEntry *this) {
    return this->active;
}

/* Returns whether the name was confirmed (no caller in the game). */
s32 NameEntry_isFinished(NameEntry *this) {
    return this->finished;
}

/* Returns the rank the score took (no caller in the game). */
s32 NameEntry_getRank(NameEntry *this) {
    return this->rank;
}

/* Returns the entry's mode, NAME_ENTRY_MODE_* (no caller in the game). */
s32 NameEntry_getMode(NameEntry *this) {
    return this->mode;
}

/* Sets the name's maximum length (no caller in the game). */
void NameEntry_setMaxLength(NameEntry *this, s32 maxLength) {
    this->maxLength = maxLength;
}

/* The table's constructor over count records of 0x40 bytes: resets them (RankingTable_reset) when a digest
 * does not match (RankingTable_check), then links them in their order. */
RankingTable *RankingTable_init(RankingTable *this, RankingRecord *entries, u32 count) {
    this->entries = entries;
    this->head = entries;
    this->count = count;
    this->pending = NULL;
    if (RankingTable_check(this) == -1) {
        RankingTable_reset(this);
    }
    /* The match depends on the u32 count: its countdown tests against -1. */
    while (count--) {
        entries->next = entries + 1;
        entries++;
    }
    entries[-1].next = NULL;
    return this;
}

/* The ranking table's destructor: writes the records back in sorted order, each with a fresh digest
 * (computeRecordDigest), and frees the table with DESTROY_FREE. */
void RankingTable_destroy(RankingTable *this, s32 flags) {
    RankingRecord *sorted = operatorVecNew(this->count * sizeof(RankingRecord));
    RankingRecord *out = sorted;
    RankingRecord *entry;

    for (entry = RankingTable_first(this); entry != NULL; entry = RankingTable_next(this)) {
        *out = *entry;
        computeRecordDigest(out, out->digest);
        out++;
    }
    memcpy(this->entries, sorted, this->count * sizeof(RankingRecord));
    if (sorted != NULL) {
        operatorVecDelete(sorted);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sorts the records (RankingTable_sort), starts the cursor at the first and returns it. */
RankingRecord *RankingTable_first(RankingTable *this) {
    return this->cursor = RankingTable_sort(this);
}

/* Steps the cursor to the next record and returns it (NULL at the end). */
RankingRecord *RankingTable_next(RankingTable *this) {
    RankingRecord *entry = this->cursor;

    if (entry == NULL) {
        return NULL;
    }
    entry = entry->next;
    this->cursor = entry;
    return entry;
}

/* Ranks score among the sorted records: when it beats one, the last record takes the score and becomes the
 * pending one (RankingTable_fillPending), and the 1-based rank is returned; 0 when it beats none. */
s32 RankingTable_insertScore(RankingTable *this, s32 score) {
    s32 rank = 1;
    RankingRecord *entry = RankingTable_first(this);

    /* The match depends on the goto: the game places the found code after the loop, whose end returns 0 (an
     * if inside the loop keeps it inline; a break needs a second NULL test). */
    while (entry != NULL) {
        if (entry->score < score) {
            goto found;
        }
        entry = entry->next;
        rank++;
    }
    return 0;

found:
    while (entry->next != NULL) {
        entry = entry->next;
    }
    entry->score = score;
    this->pending = entry;
    return rank;
}

/* Fills the pending record with a name (when it is shorter than 0x24 bytes) and character, and returns 0;
 * -1 when no record is pending or the name is too long. */
s32 RankingTable_fillPending(RankingTable *this, u8 *name, s32 character) {
    RankingRecord *entry = this->pending;

    if (entry != NULL) {
        /* The match depends on taking the name's address before the stores. */
        u8 *dst = entry->name;

        entry->character = character;
        entry->name[0] = 0;
        if (strlen(name) < 0x24) {
            strcpy(dst, name);
            this->pending = NULL;
            return 0;
        }
    }
    return -1;
}

/* Checks every record against its digest: 0 when all match, else -1. */
s32 RankingTable_check(RankingTable *this) {
    u32 count = this->count;
    RankingRecord *entry = this->entries;
    u8 digest[0x10];

    /* The match depends on the u32 count: its countdown tests against -1. */
    while (count--) {
        computeRecordDigest(entry, digest);
        if (memcmp(digest, entry->digest, 0x10) != 0) {
            return -1;
        }
        entry++;
    }
    return 0;
}

/* Computes the MD5 digest of a record's score, character and name. */
void computeRecordDigest(RankingRecord *entry, u8 *digest) {
    Md5 md5;

    md5Init(&md5);
    MD5Update(&md5, (u8 *)&entry->score, 4);
    MD5Update(&md5, (u8 *)&entry->character, 4);
    MD5Update(&md5, entry->name, 0x24);
    MD5Final(&md5, digest);
}

INCLUDE_ASM("title/nonmatchings/name_entry", RankingTable_reset);

/* Sorts the records, largest score first (compareRankingRecords), and returns the first. */
RankingRecord *RankingTable_sort(RankingTable *this) {
    return this->head = (RankingRecord *)linkNodeSort((LinkNode *)this->head, (LinkCompare)compareRankingRecords);
}

/* Orders the records by score, largest first. */
s32 compareRankingRecords(RankingRecord *a, RankingRecord *b) {
    return b->score - a->score;
}
