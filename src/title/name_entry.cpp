/* The name entry (/zascii.tim, /title/input.tim and the katakana names), which the ranking and the
 * options open. */

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/display.h"
#include "engine/gfx/prim/alloc_dr_area.h"
#include "engine/gfx/prim/alloc_line_f2.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/lib/md5.h"
#include "engine/math/shuffle.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"
#include "psyq.h"
#include "strings.h"

EXTERN_C_BEGIN
/* the grids' rows of two-byte cells */
extern u8 *NAME_GRID[];
extern u8 *CODE_GRID[];
extern u8 STR_NAME_CURSOR[];
extern u8 STR_EMPTY_NAME[];
/* the names RankingTable_reset gives the records, in a random order */
#if VERSION_US
#define DEFAULT_NAME_COUNT 14
#else
#define DEFAULT_NAME_COUNT 16
#endif
extern u8 *DEFAULT_NAMES[DEFAULT_NAME_COUNT];

NameEntry *NameEntry_init(NameEntry *entry, s32 mode, s32 maxLength);
void NameEntry_reset(NameEntry *entry);
void NameEntry_open(NameEntry *entry);
void NameEntry_close(NameEntry *entry);
void loadZasciiTim(void);
void loadInputTim(void);
void NameEntry_setGrid(NameEntry *entry, u8 **grid);
void NameEntry_update(NameEntry *entry);
void NameEntry_draw(NameEntry *entry);
s32 NameEntry_openForScore(NameEntry *entry, RankingTable *table);
void NameEntry_storeRecord(NameEntry *entry, RankingTable *table);
void NameEntry_setName(NameEntry *entry, u8 *name);
u8 *NameEntry_getName(NameEntry *entry);
void NameEntry_enterChar(NameEntry *entry, s32 code);
void NameEntry_eraseChar(NameEntry *entry);
void NameEntry_readPad(NameEntry *entry);
s32 NameEntry_tryCode(NameEntry *entry);
void NameEntry_showMessage(NameEntry *entry, s32 message, s32 count);
void NameEntry_scrollToCursor(NameEntry *entry);
void NameEntry_snapCursor(NameEntry *entry);
void NameEntry_moveCursorToEnd(NameEntry *entry);
void NameEntry_stepCursorMove(NameEntry *entry);
s32 NameEntry_readCell(NameEntry *entry);
s32 NameEntry_input(NameEntry *entry);
void NameEntry_drawName(NameEntry *entry, u_long *ot);
void NameEntry_drawGrid(NameEntry *entry, u_long *ot);
void NameEntry_drawButton(NameEntry *entry, u_long *ot, s32 x, s32 y, s32 shade, s32 index);
void NameEntry_drawGridBackground(NameEntry *entry, u_long *ot);
void NameEntry_drawBackground(NameEntry *entry, u_long *ot);
void NameEntry_drawTitle(NameEntry *entry, u_long *ot, s32 mode);
void NameEntry_drawMessage(NameEntry *entry, u_long *ot, s32 message);
void NameEntry_drawCharHighlight(void *entry, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade);
void NameEntry_drawButtonHighlight(void *entry, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade);
void NameEntry_drawHighlight(NameEntry *entry, u_long *ot);
void NameEntry_drawLine(NameEntry *entry, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b);
s32 NameEntry_isActive(NameEntry *entry);
s32 NameEntry_isFinished(NameEntry *entry);
s32 NameEntry_getRank(NameEntry *entry);
s32 NameEntry_getMode(NameEntry *entry);
void NameEntry_setMaxLength(NameEntry *entry, s32 maxLength);
s32 RankingTable_fillPending(RankingTable *table, u8 *name, s32 character);
s32 RankingTable_check(RankingTable *table);
void computeRecordDigest(RankingRecord *entry, u8 *digest);
void RankingTable_reset(RankingTable *table);
RankingRecord *RankingTable_sort(RankingTable *table);
s32 compareRankingRecords(RankingRecord *a, RankingRecord *b);
EXTERN_C_END

/* The name entry's constructor: the character grid of mode (NAME_GRID for a name, CODE_GRID for a code),
 * the window and the cursor's bounds, how far the grid scrolls, then a reset (NameEntry_reset). */
NameEntry *NameEntry_init(NameEntry *entry, s32 mode, s32 maxLength) {
    entry->port = 0;
    entry->mode = mode;
    entry->maxLength = maxLength;
    entry->moveToY = 0;
    entry->moveToX = 0;
    if (mode == NAME_ENTRY_MODE_NAME) {
        NameEntry_setGrid(entry, NAME_GRID);
    } else {
        NameEntry_setGrid(entry, CODE_GRID);
    }
    entry->windowX = 0x20;
    entry->windowY = 0x50;
    entry->windowW = 0x100;
    entry->windowH = 0x73;
    entry->boundsX = 0x25;
    entry->boundsY = 0x55;
    entry->boundsW = 0xF6;
    entry->boundsH = 0x69;
    entry->maxScrollX = entry->columns * 20 - 0xFC;
    if (entry->maxScrollX < 0) {
        entry->maxScrollX = 0;
    }
    entry->maxScrollY = entry->rows * 20 - entry->windowH + 4;
    if (entry->maxScrollY < 0) {
        entry->maxScrollY = 0;
    }
    NameEntry_reset(entry);
    return entry;
}

/* NameEntry's destructor: frees it when asked, nothing else. */
void NameEntry_destroy(NameEntry *entry, s32 flags) {
    if (flags & DESTROY_FREE) {
        operatorDelete(entry);
    }
}

/* Resets the entry: an empty name, the cursor in the middle of the window, the grid unscrolled. */
void NameEntry_reset(NameEntry *entry) {
    s32 halfW = entry->windowW >> 1;

    entry->padDelay = 10;
    entry->speed = 0x400;
    entry->active = 0;
    entry->finished = 0;
    entry->position = 0;
    entry->name[0] = 0;
    entry->closeDelay = 0;
    entry->messageTime = 0;
    entry->message = 0;
    entry->closingFrames = 0;
    entry->frame = 0;
    entry->rank = 0;
    entry->character = 0;
    entry->score = 0;
    entry->port = 0;
    entry->repeated = 0;
    entry->held = 0;
    entry->scrollY = 0;
    entry->scrollX = 0;
    entry->row = 0;
    entry->column = 0;
    entry->moveLastY = 0;
    entry->moveLastX = 0;
    entry->moveFromY = 0;
    entry->moveFromX = 0;
    entry->moveTime = 0;
    entry->cursorX = entry->windowX + halfW;
    entry->cursorY = entry->windowY + (entry->windowH >> 1);
    NameEntry_scrollToCursor(entry);
    entry->lastCell = NameEntry_readCell(entry);
}

/* Opens the entry: resets it (NameEntry_reset) and makes it active. */
void NameEntry_open(NameEntry *entry) {
    NameEntry_reset(entry);
    entry->active = 1;
}

/* Closes the entry: it is no longer updated nor drawn. */
void NameEntry_close(NameEntry *entry) {
    entry->active = 0;
}

/* Loads "/zascii.tim", uploads its TIMs to VRAM and frees it. */
void loadZasciiTim(void) {
    u32 *file = (u32 *)loadFile("/zascii.tim");

    uploadTims(file);
    mainHeapFree(file);
}

/* Loads "/title/input.tim", uploads its TIMs to VRAM and frees it. */
void loadInputTim(void) {
    u32 *file = (u32 *)loadFile("/title/input.tim");

    uploadTims(file);
    mainHeapFree(file);
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_setGrid);

/* The entry's update: fades in on its first frame, waits padDelay frames before reading the pad, closes
 * 11 frames after NameEntry_input confirms the name (fading out, then in again). */
void NameEntry_update(NameEntry *entry) {
    if (entry->active == 0) {
        return;
    }
    if (entry->frame == 0) {
        fadeControlFadeOut(&FADE_CONTROL, 0);
        fadeControlFadeIn(&FADE_CONTROL, 10);
    }
    if (entry->padDelay == 0) {
        NameEntry_readPad(entry);
    } else {
        entry->padDelay--;
        entry->repeated = 0;
        entry->held = 0;
    }
    if (entry->finished == 0) {
        if (NameEntry_input(entry)) {
            entry->closeDelay = 1;
            entry->padDelay = 99;
        }
    }
    if (entry->closeDelay != 0) {
        if (--entry->closeDelay == 0) {
            entry->finished = 1;
            entry->closingFrames = 0;
            fadeControlFadeOut(&FADE_CONTROL, 10);
        }
    }
    if (entry->finished != 0) {
        if (++entry->closingFrames >= 11) {
            entry->active = 0;
            fadeControlFadeIn(&FADE_CONTROL, 10);
        }
    }
    if (entry->messageTime != 0) {
        entry->messageTime--;
    }
    entry->frame++;
}

/* The entry's draw: the grid's parts (NameEntry_drawMessage while messageTime runs), the name, a green line across
 * the window at the cursor's height and one down it at the cursor's x. */
void NameEntry_draw(NameEntry *entry) {
    u_long *ot;
    s32 x, y, w, h;
    s32 right, bottom;

    if (entry->active == 0) {
        return;
    }
    ot = FRAME_OT.firstDrawn + 5;
    if (entry->messageTime != 0) {
        NameEntry_drawMessage(entry, ot, entry->message);
    }
    NameEntry_drawHighlight(entry, ot);
    NameEntry_drawName(entry, ot);
    NameEntry_drawTitle(entry, ot, entry->mode);
    NameEntry_drawBackground(entry, ot);
    x = entry->windowX;
    y = entry->windowY;
    w = entry->windowW;
    h = entry->windowH;
    right = x + w;
    bottom = y + h;
    NameEntry_drawLine(entry, ot, x, entry->cursorY, right, entry->cursorY, 0x20, 0xA0, 0x20);
    NameEntry_drawLine(entry, ot, entry->cursorX, y, entry->cursorX, bottom, 0x20, 0xA0, 0x20);
    NameEntry_drawGrid(entry, ot);
}

/* Opens the entry for the player's side when its score ranks 1 to 7 in table (RankingTable_insertScore), keeping
 * the score, character and rank; returns whether it opened. */
s32 NameEntry_openForScore(NameEntry *entry, RankingTable *table) {
    s32 side = gameStateGetPlayerSide(&GAME_STATE);

    entry->port = side;
    if (side < 0 || side >= FIGHTER_COUNT) {
        entry->port = 0;
        entry->active = 0;
        return 0;
    }
    entry->score = GAME_STATE.fighters[side].score;
    entry->character = GAME_STATE.fighters[side].character;
    entry->rank = RankingTable_insertScore(table, entry->score);
    if (entry->rank >= 1 && entry->rank <= 7) {
        entry->active = 1;
    } else {
        entry->active = 0;
        entry->rank = 0;
    }
    return entry->active;
}

/* Stores the name (STR_EMPTY_NAME when it is empty) and the character in the table's pending record
 * (RankingTable_fillPending). */
void NameEntry_storeRecord(NameEntry *entry, RankingTable *table) {
    u8 *name = NameEntry_getName(entry);

    if (*name == 0) {
        name = STR_EMPTY_NAME;
    }
    RankingTable_fillPending(table, name, entry->character);
}

/* Sets the name to a string of two-byte characters, the position after its last one. */
void NameEntry_setName(NameEntry *entry, u8 *name) {
    u8 *dst;

    entry->position = 0;
    dst = entry->name;
    /* The match depends on a test before a do/while: the loop reloads the first byte, which the
     * duplicated exit test of a while reuses. */
    if (*name != 0) {
        do {
            *dst++ = *name++;
            *dst++ = *name++;
            entry->position++;
        } while (*name != 0);
    }
    *dst = 0;
}

/* Returns the name. */
u8 *NameEntry_getName(NameEntry *entry) {
    return entry->name;
}

/* Writes the two-byte character code at the position, which moves on unless it is the last. */
void NameEntry_enterChar(NameEntry *entry, s32 code) {
    u8 *p = entry->name;

    /* The match depends on stepping p from the name: &name[position * 2] adds the base last. */
    p += entry->position * 2;
    *p++ = code >> 8;
    *p++ = code;
    *p = 0;
    if (entry->position < entry->maxLength - 1) {
        entry->position++;
    }
}

/* Erases a character: the one before the position, or the last one when it is filled. */
void NameEntry_eraseChar(NameEntry *entry) {
    s32 position = entry->position;
    u8 *p = entry->name;

    /* The match depends on stepping p from the name: &name[position * 2] adds the base last. */
    p += position * 2;
    if (position < entry->maxLength - 1 || p[0] == 0) {
        if (position > 0) {
            entry->position = position - 1;
            p -= 2;
        }
    }
    *p = 0;
}

/* Reads the entry's pad: the buttons held and repeated. */
void NameEntry_readPad(NameEntry *entry) {
    PadManager *padManager = PAD_MANAGER_INSTANCE;

    entry->held = padManagerGetHeld(padManager, entry->port);
    entry->repeated = padManagerGetRepeated(padManager, entry->port);
}

/* Tries the code entered: a wrong one plays the refused sound and returns -1. A right one unlocks its
 * character (gameStateUnlockByCode) and shows message 0 for 300 frames when that character was already pickable,
 * else message code + 1 with the pad held off for 10 frames. Returns the code's number. */
s32 NameEntry_tryCode(NameEntry *entry) {
    /* game_state.cpp's CODE_CHARACTERS, the character of each code */
    u8 characters[CODE_COUNT] = {
        CHARACTER_OMEGAMON, CHARACTER_IMPERIALDRAMON_PALADIN_MODE, CHARACTER_WARGREYMON, CHARACTER_IMPERIALDRAMON,
        CHARACTER_DUKEMON, CHARACTER_SAKUYAMON, CHARACTER_SAINTGARGOMON,
    };
    u32 pickable;
    s32 code;

    pickable = gameStateGetPickableCharacters(&GAME_STATE);
    code = gameStateUnlockByCode(&GAME_STATE, (char *)entry->name);
    if (code < 0) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_REFUSE, 0);
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CODE, 0);
    if (code < CODE_COUNT) {
        if (pickable & CHARACTER_BIT(characters[code])) {
            NameEntry_showMessage(entry, 0, 300);
            return code;
        }
        NameEntry_showMessage(entry, code + 1, 300);
        entry->padDelay = 10;
    }
    return code;
}

/* Shows input.tim's message for count frames. */
void NameEntry_showMessage(NameEntry *entry, s32 message, s32 count) {
    entry->message = message;
    entry->messageTime = count;
}

/* Keeps the cursor inside the bounds: within 0x50 (x) or 0x28 (y) of an edge the grid scrolls instead,
 * as far as it goes; then clamps the scroll and the cursor. */
void NameEntry_scrollToCursor(NameEntry *entry) {
    s32 edge;

    edge = entry->boundsX + 0x50;
    if (entry->cursorX < edge) {
        if (entry->scrollX > 0) {
            entry->scrollX -= edge - entry->cursorX;
            entry->cursorX = edge;
        }
    }
    edge = entry->boundsX + entry->boundsW - 0x50;
    if (edge < entry->cursorX) {
        if (entry->scrollX < entry->maxScrollX) {
            entry->scrollX += entry->cursorX - edge;
            entry->cursorX = edge;
        }
    }
    edge = entry->boundsY + 0x28;
    if (entry->cursorY < edge) {
        if (entry->scrollY > 0) {
            entry->scrollY -= edge - entry->cursorY;
            entry->cursorY = edge;
        }
    }
    edge = entry->boundsY + entry->boundsH - 0x28;
    if (edge < entry->cursorY) {
        if (entry->scrollY < entry->maxScrollY) {
            entry->scrollY += entry->cursorY - edge;
            entry->cursorY = edge;
        }
    }
    if (entry->scrollX < 0) {
        entry->scrollX = 0;
    }
    if (entry->scrollY < 0) {
        entry->scrollY = 0;
    }
    if (entry->scrollX > entry->maxScrollX) {
        entry->scrollX = entry->maxScrollX;
    }
    if (entry->scrollY > entry->maxScrollY) {
        entry->scrollY = entry->maxScrollY;
    }
    if (entry->cursorX < entry->boundsX) {
        entry->cursorX = entry->boundsX;
    }
    if (entry->cursorY < entry->boundsY) {
        entry->cursorY = entry->boundsY;
    }
    if (entry->boundsX + entry->boundsW < entry->cursorX) {
        entry->cursorX = entry->boundsX + entry->boundsW;
    }
    if (entry->boundsY + entry->boundsH < entry->cursorY) {
        entry->cursorY = entry->boundsY + entry->boundsH;
    }
}

/* While no direction is held and the cursor is on a character or a button, moves the cursor halfway to the
 * center of its cell on screen (20-pixel cells, scrolled, in the window). */
void NameEntry_snapCursor(NameEntry *entry) {
    /* The match depends on one test per direction: or-ed in one if, GCC merges them into one mask. */
    if (entry->held & PADLleft) {
        return;
    }
    if (entry->held & PADLright) {
        return;
    }
    if (entry->held & PADLup) {
        return;
    }
    if (entry->held & PADLdown) {
        return;
    }
    if (entry->cellCode != 0 || entry->cellButton != 0) {
        entry->cursorX += (entry->column * 20 + 10 - entry->scrollX + entry->windowX - entry->cursorX) >> 1;
        entry->cursorY += (entry->row * 20 + 10 - entry->scrollY + entry->windowY - entry->cursorY) >> 1;
    }
}

/* Starts a move of the cursor from the middle of its cell (20 pixels a cell) and holds the pad 22 frames. */
void NameEntry_moveCursorToEnd(NameEntry *entry) {
    s32 column = entry->column;
    s32 row;

    entry->padDelay = 22;
    entry->moveTime = 0x1000;
    entry->moveFromX = column * 20 + 10;
    entry->moveLastX = column * 20 + 10;
    row = entry->row;
    entry->moveFromY = row * 20 + 10;
    entry->moveLastY = row * 20 + 10;
}

/* Steps the cursor's move: moveTime falls by 0x155 a frame, and the cursor goes by what the point between
 * moveTo and moveFrom moved since the last frame. */
void NameEntry_stepCursorMove(NameEntry *entry) {
    s32 t;
    s32 x, y;
    s32 dx, dy;

    if (entry->moveTime == 0) {
        return;
    }
    entry->moveTime -= 0x155;
    if (entry->moveTime < 0) {
        entry->moveTime = 0;
    }
    dx = entry->moveFromX - entry->moveToX;
    dy = entry->moveFromY - entry->moveToY;
    t = entry->moveTime;
    x = entry->moveToX + ((dx * t) >> 12);
    y = entry->moveToY + ((dy * t) >> 12);
    /* The match depends on finishing x before y: x then lives no longer than y and gets the lower register. */
    entry->cursorX += x - entry->moveLastX;
    entry->moveLastX = x;
    entry->cursorY += y - entry->moveLastY;
    entry->moveLastY = y;
}

INCLUDE_ASM("title/nonmatchings/name_entry", NameEntry_readCell);

/* NameEntry_input's action for confirm on a character: enter it */
#define NAME_ENTRY_ENTER_CHAR (-9)

/* Reads the pad for the entry: while the cursor moves on its own, only that; else the directions move the cursor
 * (speed >> 8 pixels), a new cell under it plays the cursor sound, confirm enters the character or presses the
 * button under it, cancel erases (or, in an empty code, leaves) and Start ends. Returns whether the entry is done:
 * a code is tried instead (NameEntry_tryCode) and the entry stays open. */
s32 NameEntry_input(NameEntry *entry) {
    s32 action = 0;
    s32 done;
    s32 speed;
    s32 cell;
    s32 current;

    if (entry->moveTime != 0) {
        NameEntry_stepCursorMove(entry);
        NameEntry_scrollToCursor(entry);
        NameEntry_readCell(entry);
        return 0;
    }
    done = 0;
    speed = entry->speed >> 8;
    if (entry->held & PADLleft) {
        entry->cursorX -= speed;
    }
    if (entry->held & PADLright) {
        entry->cursorX += speed;
    }
    if (entry->held & PADLup) {
        entry->cursorY -= speed;
    }
    if (entry->held & PADLdown) {
        entry->cursorY += speed;
    }
    NameEntry_scrollToCursor(entry);
    cell = NameEntry_readCell(entry);
    NameEntry_snapCursor(entry);
    current = entry->cellCode;
    if (entry->cellButton != 0) {
        current = entry->cellButton;
    }
    if (current != entry->lastCell) {
        entry->lastCell = current;
        if (current != 0) {
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
        }
    }
    if (entry->repeated & PAD_CONFIRM) {
        action = cell;
        if (cell > 0) {
            action = NAME_ENTRY_ENTER_CHAR;
        }
    }
    if (entry->repeated & PAD_CANCEL) {
        action = NAME_ENTRY_BUTTON_ERASE;
        if (entry->mode == NAME_ENTRY_MODE_CODE && entry->position == 0) {
            action = NAME_ENTRY_BUTTON_EXIT;
        }
    }
    if (entry->repeated & PADstart) {
        done = 1;
    }
    switch (action) {
    case NAME_ENTRY_ENTER_CHAR:
        if (cell > 0) {
            if (entry->position == entry->maxLength - 1) {
                NameEntry_moveCursorToEnd(entry);
            }
            NameEntry_enterChar(entry, cell);
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        }
        break;
    case NAME_ENTRY_BUTTON_END:
        done = 1;
        if (entry->mode == NAME_ENTRY_MODE_NAME) {
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        }
        break;
    case NAME_ENTRY_BUTTON_ERASE:
        if (entry->position != 0) {
            NameEntry_eraseChar(entry);
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        }
        break;
    case NAME_ENTRY_BUTTON_EXIT:
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        return 1;
    }
    if (done) {
        if (entry->mode == NAME_ENTRY_MODE_CODE) {
            NameEntry_tryCode(entry);
            done = 0;
        }
    }
    return done;
}

INCLUDE_RODATA("title/nonmatchings/name_entry", STR_NAME_CURSOR);
INCLUDE_RODATA("title/nonmatchings/name_entry", STR_EMPTY_NAME);

/* Prints the name in grey, centred on x 0xA0 at y 0x2F, and every other 8 frames STR_NAME_CURSOR (the cursor)
 * under the position. */
void NameEntry_drawName(NameEntry *entry, u_long *ot) {
    ZasciiPrinter printer(ot);
    PrinterVtable *vtable;
    s32 left;

    vtable = printer.vtable;
    vtable->setColor.func((u8 *)&printer + vtable->setColor.delta, 0x606060);
    printer.y = 0x2F;
    left = 0xA0 - (entry->maxLength << 4) / 2;
    printer.x = left;
    printerPrint(&printer, entry->name);
    if (entry->frame & 8) {
        printer.y = 0x30;
        printer.x = (entry->position << 4) + left;
        printerPrint(&printer, STR_NAME_CURSOR);
    }
}

/* Draws the grid's cells seen in the window, clipped to it: each character, white under the cursor and dark grey
 * elsewhere, and for a letter followed by '1' the button of that letter (NameEntry_drawButton) from the cell
 * before, bright under the cursor; then the background behind them. Cells are 20 pixels; a row ends at a 0.
 * The match depends on letter read back from cell after the stores (the game zero-extends the stored byte) and
 * on y computed before row. */
void NameEntry_drawGrid(NameEntry *entry, u_long *ot) {
    RECT area;
    u8 cell[3];
    s32 left;
    s32 firstColumn;
    s32 right;
    s32 bottom;
    s32 x;
    s32 y;
    s32 column;
    s32 row;

    getDrawBufferRect(&area);
    AddPrim(ot, allocDrArea(&area));
    ZasciiPrinter printer(ot);

    cell[2] = 0;
    y = entry->windowY - entry->scrollY % 20;
    row = entry->scrollY / 20;
    left = entry->windowX - entry->scrollX % 20 - 20;
    firstColumn = entry->scrollX / 20 - 1;
    right = entry->windowX + entry->windowW + 20;
    bottom = entry->windowY + entry->windowH;
    for (; y < bottom; y += 20, row++) {
        if (row < 0) {
            continue;
        }
        if (row >= entry->rows) {
            break;
        }
        for (x = left, column = firstColumn; x < right; x += 20, column++) {
            u8 *code;
            s32 letter;

            if (column < 0) {
                continue;
            }
            if (column >= entry->columns) {
                break;
            }
            code = &entry->grid[row][column * 2];
            if (code[0] == 0) {
                break;
            }
            cell[0] = code[0];
            cell[1] = code[1];
            letter = cell[0];
            if (letter >= 'A' && letter <= 'Z') {
                if (cell[1] == '1') {
                    s32 shade = 0x30;

                    if (column == entry->column && row == entry->row) {
                        shade = 0x80;
                    }
                    NameEntry_drawButton(entry, ot, x - 20, y, shade, letter - 'A');
                }
            } else {
                if (column == entry->column && row == entry->row) {
                    TEXT_SET_COLOR(printer, 0x808080);
                } else {
                    TEXT_SET_COLOR(printer, 0x303030);
                }
                printer.x = x + 2;
                printer.y = y + 2;
                printerPrint(&printer, cell);
            }
        }
    }
    NameEntry_drawGridBackground(entry, ot);
    getDrawBufferSubRect(&area, entry->windowX, entry->windowY, entry->windowW, entry->windowH);
    AddPrim(ot, allocDrArea(&area));
}

/* Draws in ot button index (0 for A, 1 for B, 2 for C) of the grid as a 64 by 21 POLY_FT4 at (x, y): the
 * texels (0x80, 0x60 + index * 21) to (0xBF, 0x74 + index * 21) of page 12, clut 0x3CE8, in the gray shade. */
void NameEntry_drawButton(NameEntry *entry, u_long *ot, s32 x, s32 y, s32 shade, s32 index) {
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
void NameEntry_drawGridBackground(NameEntry *entry, u_long *ot) {
    s32 windowX = entry->windowX;
    s32 halfScrollX = entry->scrollX >> 1;
    s32 left = windowX - halfScrollX % 64;
    s32 windowY = entry->windowY;
    s32 halfScrollY = entry->scrollY >> 1;
    s32 top = windowY - halfScrollY % 64;
    s32 right = windowX + entry->windowW;
    s32 bottom = windowY + entry->windowH;
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

/* Draws in ot the entry's 320 by 240 background for its mode: the 8-bit pages 0x8A, 0x8B and 0x8C (128, 128
 * and 64 texels of each) side by side, clut row 254 + mode (x 640), in neutral gray. */
void NameEntry_drawBackground(NameEntry *entry, u_long *ot) {
    s32 clutY = 254 + entry->mode;
    POLY_FT4 *poly;

    poly = allocPolyFT4();
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    poly->x0 = 0;
    poly->y0 = 0;
    poly->x1 = 128;
    poly->y1 = 0;
    poly->x2 = 0;
    poly->y2 = 240;
    poly->x3 = 128;
    poly->y3 = 240;
    poly->u0 = 0;
    poly->v0 = 0;
    poly->u1 = 0x7F;
    poly->v1 = 0;
    poly->u2 = 0;
    poly->v2 = 0xEF;
    poly->u3 = 0x7F;
    poly->v3 = 0xEF;
    poly->tpage = 0x8A;
    poly->clut = getClut(640, clutY);
    AddPrim(ot, poly);

    poly = allocPolyFT4();
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    poly->x0 = 128;
    poly->y0 = 0;
    poly->x1 = 256;
    poly->y1 = 0;
    poly->x2 = 128;
    poly->y2 = 240;
    poly->x3 = 256;
    poly->y3 = 240;
    poly->u0 = 0;
    poly->v0 = 0;
    poly->u1 = 0x7F;
    poly->v1 = 0;
    poly->u2 = 0;
    poly->v2 = 0xEF;
    poly->u3 = 0x7F;
    poly->v3 = 0xEF;
    poly->tpage = 0x8B;
    poly->clut = getClut(640, clutY);
    AddPrim(ot, poly);

    poly = allocPolyFT4();
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    poly->x0 = 256;
    poly->y0 = 0;
    poly->x1 = 320;
    poly->y1 = 0;
    poly->x2 = 256;
    poly->y2 = 240;
    poly->x3 = 320;
    poly->y3 = 240;
    poly->u0 = 0;
    poly->v0 = 0;
    poly->u1 = 0x3F;
    poly->v1 = 0;
    poly->u2 = 0;
    poly->v2 = 0xEF;
    poly->u3 = 0x3F;
    poly->v3 = 0xEF;
    poly->tpage = 0x8C;
    poly->clut = getClut(640, clutY);
    AddPrim(ot, poly);
}

/* Draws in ot the entry's title for mode: the 104 by 16 texel row mode * 16 at u 0x80 of page 12, clut row
 * 252 + mode (x 640), at (108, 24) in neutral gray. */
void NameEntry_drawTitle(NameEntry *entry, u_long *ot, s32 mode) {
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


/* Draws in ot the message of NameEntry_showMessage: one of input.tim's eight 192 by 48 images (0 to 4 on page
 * 0x19, 5 to 7 on page 0x1A), clut row 0x1F8 + message (x 576), at (64, 96) in neutral gray. */
void NameEntry_drawMessage(NameEntry *entry, u_long *ot, s32 message) {
    POLY_FT4 *poly = allocPolyFT4();

    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    poly->x0 = 64;
    poly->y0 = 96;
    poly->x1 = 256;
    poly->y1 = 96;
    poly->x2 = 64;
    poly->y2 = 144;
    poly->x3 = 256;
    poly->y3 = 144;
    if (message < 5) {
        poly->u0 = 0;
        poly->v0 = message * 48;
        poly->u1 = 0xBF;
        poly->v1 = message * 48;
        poly->u2 = 0;
        poly->v2 = message * 48 + 47;
        poly->u3 = 0xBF;
        poly->v3 = message * 48 + 47;
        poly->tpage = 0x19;
    } else {
        poly->u0 = 0;
        poly->v0 = (message - 5) * 48;
        poly->u1 = 0xBF;
        poly->v1 = (message - 5) * 48;
        poly->u2 = 0;
        poly->v2 = (message - 5) * 48 + 47;
        poly->u3 = 0xBF;
        poly->v3 = (message - 5) * 48 + 47;
        poly->tpage = 0x1A;
    }
    poly->clut = ((0x1F8 + message) << 6) | (576 >> 4);
    AddPrim(ot, poly);
}

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x35) to (0xCF, 0x44) of page 12, clut 0x3C68, w and h to each
 * side of (x, y), all its corners of the gray shade (NameEntry_drawHighlight passes its object as entry,
 * unused). */
void NameEntry_drawCharHighlight(void *entry, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
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
 * side of (x, y), all its corners of the gray shade (NameEntry_drawHighlight passes its object as entry,
 * unused). */
void NameEntry_drawButtonHighlight(void *entry, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
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
void NameEntry_drawHighlight(NameEntry *entry, u_long *ot) {
    s32 x = entry->column * 20 + 10 - entry->scrollX + entry->windowX;
    s32 y = entry->row * 20 + 10 - entry->scrollY + entry->windowY;

    /* The match depends on testing for no button first: the character case comes first in the code. */
    if (entry->cellButton == 0) {
        if (entry->cellCode > 0) {
            NameEntry_drawCharHighlight(entry, ot, x, y, 14, 14, 0x60);
        }
    } else {
        NameEntry_drawButtonHighlight(entry, ot, x + 2, y, 39, 17, 0x60);
    }
}

/* Draws in ot a semi-transparent line from (x0, y0) to (x1, y1) of the colour (r, g, b). */
void NameEntry_drawLine(NameEntry *entry, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b) {
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
s32 NameEntry_isActive(NameEntry *entry) {
    return entry->active;
}

/* Returns whether the name was confirmed (no caller in the game). */
s32 NameEntry_isFinished(NameEntry *entry) {
    return entry->finished;
}

/* Returns the rank the score took (no caller in the game). */
s32 NameEntry_getRank(NameEntry *entry) {
    return entry->rank;
}

/* Returns the entry's mode, NAME_ENTRY_MODE_* (no caller in the game). */
s32 NameEntry_getMode(NameEntry *entry) {
    return entry->mode;
}

/* Sets the name's maximum length (no caller in the game). */
void NameEntry_setMaxLength(NameEntry *entry, s32 maxLength) {
    entry->maxLength = maxLength;
}

/* The table's constructor over count records of 0x40 bytes: resets them (RankingTable_reset) when a digest
 * does not match (RankingTable_check), then links them in their order. */
RankingTable *RankingTable_init(RankingTable *table, RankingRecord *entries, u32 count) {
    table->entries = entries;
    table->head = entries;
    table->count = count;
    table->pending = NULL;
    if (RankingTable_check(table) == -1) {
        RankingTable_reset(table);
    }
    /* The match depends on the u32 count: its countdown tests against -1. */
    while (count--) {
        entries->next = entries + 1;
        entries++;
    }
    entries[-1].next = NULL;
    return table;
}

/* The ranking table's destructor: writes the records back in sorted order, each with a fresh digest
 * (computeRecordDigest), and frees the table with DESTROY_FREE. */
void RankingTable_destroy(RankingTable *table, s32 flags) {
    RankingRecord *sorted = new RankingRecord[table->count];
    RankingRecord *out = sorted;
    RankingRecord *entry;

    for (entry = RankingTable_first(table); entry != NULL; entry = RankingTable_next(table)) {
        *out = *entry;
        computeRecordDigest(out, out->digest);
        out++;
    }
    memcpy(table->entries, sorted, table->count * sizeof(RankingRecord));
    delete[] sorted;
    if (flags & DESTROY_FREE) {
        operatorDelete(table);
    }
}

/* Sorts the records (RankingTable_sort), starts the cursor at the first and returns it. */
RankingRecord *RankingTable_first(RankingTable *table) {
    return table->cursor = RankingTable_sort(table);
}

/* Steps the cursor to the next record and returns it (NULL at the end). */
RankingRecord *RankingTable_next(RankingTable *table) {
    RankingRecord *entry = table->cursor;

    if (entry == NULL) {
        return NULL;
    }
    entry = entry->next;
    table->cursor = entry;
    return entry;
}

/* Ranks score among the sorted records: when it beats one, the last record takes the score and becomes the
 * pending one (RankingTable_fillPending), and the 1-based rank is returned; 0 when it beats none. */
s32 RankingTable_insertScore(RankingTable *table, s32 score) {
    s32 rank = 1;
    RankingRecord *entry = RankingTable_first(table);

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
    table->pending = entry;
    return rank;
}

/* Fills the pending record with a name (when it is shorter than 0x24 bytes) and character, and returns 0;
 * -1 when no record is pending or the name is too long. */
s32 RankingTable_fillPending(RankingTable *table, u8 *name, s32 character) {
    RankingRecord *entry = table->pending;

    if (entry != NULL) {
        /* The match depends on taking the name's address before the stores. */
        u8 *dst = entry->name;

        entry->character = character;
        entry->name[0] = 0;
        if (strlen((char *)name) < 0x24) { /* the SDK's string functions take char */
            strcpy((char *)dst, (char *)name);
            table->pending = NULL;
            return 0;
        }
    }
    return -1;
}

/* Checks every record against its digest: 0 when all match, else -1. */
s32 RankingTable_check(RankingTable *table) {
    u32 count = table->count;
    RankingRecord *entry = table->entries;
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

/* Fills the records with defaults: scores 10000 to count * 10000 in steps of 10000, each with a name of
 * DEFAULT_NAMES and a character of STARTING_CHARACTERS, both in a random order. */
void RankingTable_reset(RankingTable *table) {
    RankingRecord *entries = table->entries;
    s32 count = table->count;
    s32 i;

    bzero((u8 *)entries, count * sizeof(RankingRecord));
    Shuffle names(&RANDOM, DEFAULT_NAME_COUNT, NULL);
    Shuffle characters(&RANDOM, STARTING_CHARACTER_COUNT, NULL);

    for (i = 0; i < count; i++) {
        strcpy((char *)entries[i].name, (char *)DEFAULT_NAMES[names.getNumber(i)]);
        entries[i].score = (i + 1) * 10000;
        entries[i].character = STARTING_CHARACTERS[characters.getNumber(i)];
    }
}

/* Sorts the records, largest score first (compareRankingRecords), and returns the first. */
RankingRecord *RankingTable_sort(RankingTable *table) {
    return table->head = (RankingRecord *)linkNodeSort((LinkNode *)table->head, (LinkCompare)compareRankingRecords);
}

/* Orders the records by score, largest first. */
s32 compareRankingRecords(RankingRecord *a, RankingRecord *b) {
    return b->score - a->score;
}
