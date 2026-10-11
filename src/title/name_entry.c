/* The name entry (/zascii.tim, /title/input.tim and the katakana names), which the ranking and the
 * options open. */

#include "common.h"
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

extern char D_800663D0[];
extern char D_800663DC[];

extern u8 D_8007B250[];
extern u8 D_8007B280[];
extern u8 D_800663F8[];
extern u8 D_800663FC[];

void func_80074248(Unk80074220 *this);
void func_800743C4(Unk80074220 *this, u8 *grid);
u8 *func_800748A0(Unk80074220 *this);
void func_80074938(Unk80074220 *this);
void func_80074AA8(Unk80074220 *this);
s32 func_80074E28(Unk80074220 *this);
s32 func_80074F68(Unk80074220 *this);
void func_80075238(Unk80074220 *this, u_long *ot);
void func_80075304(Unk80074220 *this, u_long *ot);
void func_8007588C(Unk80074220 *this, u_long *ot);
void func_80075A54(Unk80074220 *this, u_long *ot, s32 grid);
void func_80075B14(Unk80074220 *this, u_long *ot, s32 arg);
void func_80075DF8(Unk80074220 *this, u_long *ot);
void func_80075EC8(Unk80074220 *this, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b);
s32 func_80076220(Unk800764BC *this, u8 *name, s32 arg);
s32 func_80076298(Unk800764BC *this);
void func_80076310(Unk800764F4 *entry, u8 *digest);
void func_80076380(Unk800764BC *this);
Unk800764F4 *func_800764BC(Unk800764BC *this);
s32 func_800764F4(Unk800764F4 *a, Unk800764F4 *b);

/* The name entry's constructor: the character grid (D_8007B250 when unk8 is 0, else D_8007B280), the
 * window and the cursor's bounds, how far the grid scrolls, then a reset (func_80074248). */
Unk80074220 *func_80074130(Unk80074220 *this, s32 grid, s32 maxLength) {
    this->port = 0;
    this->unk8 = grid;
    this->maxLength = maxLength;
    this->moveToY = 0;
    this->moveToX = 0;
    if (grid == 0) {
        func_800743C4(this, D_8007B250);
    } else {
        func_800743C4(this, D_8007B280);
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
    func_80074248(this);
    return this;
}

/* Unk80074220's destructor: frees it when asked, nothing else. */
void func_80074220(Unk80074220 *this, s32 flags) {
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Resets the entry: an empty name, the cursor in the middle of the window, the grid unscrolled. */
void func_80074248(Unk80074220 *this) {
    s32 halfW = this->windowW >> 1;

    this->unk3C = 10;
    this->unk90 = 0x400;
    this->active = 0;
    this->finished = 0;
    this->position = 0;
    this->name[0] = 0;
    this->unk38 = 0;
    this->unk4C = 0;
    this->unk48 = 0;
    this->unk44 = 0;
    this->frame = 0;
    this->unk68 = 0;
    this->unk64 = 0;
    this->unk60 = 0;
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
    func_80074AA8(this);
    this->unk88 = func_80074E28(this);
}

/* Opens the entry: resets it (func_80074248) and makes it active. */
void func_80074318(Unk80074220 *this) {
    func_80074248(this);
    this->active = 1;
}

/* Closes the entry: it is no longer updated nor drawn. */
void func_80074344(Unk80074220 *this) {
    this->active = 0;
}

/* Loads "/zascii.tim" (D_800663D0), uploads its TIMs to VRAM and frees it. */
void func_8007434C(void) {
    u32 *file = loadFile(D_800663D0);

    uploadTims(file);
    mainHeapFree(file);
}

/* Loads "/title/input.tim" (D_800663DC), uploads its TIMs to VRAM and frees it. */
void func_80074388(void) {
    u32 *file = loadFile(D_800663DC);

    uploadTims(file);
    mainHeapFree(file);
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_800743C4);

/* The entry's update: fades in on its first frame, waits unk3C frames before reading the pad, closes
 * 11 frames after func_80074F68 confirms the name (fading out, then in again). */
void func_800744BC(Unk80074220 *this) {
    if (this->active == 0) {
        return;
    }
    if (this->frame == 0) {
        fadeControlFadeOut(&FADE_CONTROL, 0);
        fadeControlFadeIn(&FADE_CONTROL, 10);
    }
    if (this->unk3C == 0) {
        func_80074938(this);
    } else {
        this->unk3C--;
        this->repeated = 0;
        this->held = 0;
    }
    if (this->finished == 0) {
        if (func_80074F68(this)) {
            this->unk38 = 1;
            this->unk3C = 99;
        }
    }
    if (this->unk38 != 0) {
        if (--this->unk38 == 0) {
            this->finished = 1;
            this->unk44 = 0;
            fadeControlFadeOut(&FADE_CONTROL, 10);
        }
    }
    if (this->finished != 0) {
        if (++this->unk44 >= 11) {
            this->active = 0;
            fadeControlFadeIn(&FADE_CONTROL, 10);
        }
    }
    if (this->unk4C != 0) {
        this->unk4C--;
    }
    this->frame++;
}

/* The entry's draw: the grid's parts (func_80075B14 while unk4C runs), the name, a green line across
 * the window at the cursor's height and one down it at the cursor's x. */
void func_8007460C(Unk80074220 *this) {
    u_long *ot;
    s32 x, y, w, h;
    s32 right, bottom;

    if (this->active == 0) {
        return;
    }
    ot = FRAME_OT.firstDrawn + 5;
    if (this->unk4C != 0) {
        func_80075B14(this, ot, this->unk48);
    }
    func_80075DF8(this, ot);
    func_80075238(this, ot);
    func_80075A54(this, ot, this->unk8);
    func_8007588C(this, ot);
    x = this->windowX;
    y = this->windowY;
    w = this->windowW;
    h = this->windowH;
    right = x + w;
    bottom = y + h;
    func_80075EC8(this, ot, x, this->cursorY, right, this->cursorY, 0x20, 0xA0, 0x20);
    func_80075EC8(this, ot, this->cursorX, y, this->cursorX, bottom, 0x20, 0xA0, 0x20);
    func_80075304(this, ot);
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_80074740);

/* Stores the name (D_800663FC when it is empty) and unk64 in the table's pending record (func_80076220). */
void func_800747F8(Unk80074220 *this, Unk800764BC *table) {
    u8 *name = func_800748A0(this);

    if (*name == 0) {
        name = D_800663FC;
    }
    func_80076220(table, name, this->unk64);
}

/* Sets the name to a string of two-byte characters, the position after its last one. */
void func_8007484C(Unk80074220 *this, u8 *name) {
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
u8 *func_800748A0(Unk80074220 *this) {
    return this->name;
}

/* Writes the two-byte character code at the position, which moves on unless it is the last. */
void func_800748A8(Unk80074220 *this, s32 code) {
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
void func_800748F0(Unk80074220 *this) {
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
void func_80074938(Unk80074220 *this) {
    PadManager *padManager = PAD_MANAGER_INSTANCE;

    this->held = padManagerGetHeld(padManager, this->port);
    this->repeated = padManagerGetRepeated(padManager, this->port);
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_80074988);

/* Sets unk48 and the countdown unk4C. */
void func_80074A9C(Unk80074220 *this, s32 arg, s32 count) {
    this->unk48 = arg;
    this->unk4C = count;
}

/* Keeps the cursor inside the bounds: within 0x50 (x) or 0x28 (y) of an edge the grid scrolls instead,
 * as far as it goes; then clamps the scroll and the cursor. */
void func_80074AA8(Unk80074220 *this) {
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

INCLUDE_ASM("title/nonmatchings/name_entry", func_80074C7C);

/* Starts a move of the cursor from the middle of its cell (20 pixels a cell) and holds the pad 22 frames. */
void func_80074D44(Unk80074220 *this) {
    s32 column = this->column;
    s32 row;

    this->unk3C = 22;
    this->moveTime = 0x1000;
    this->moveFromX = column * 20 + 10;
    this->moveLastX = column * 20 + 10;
    row = this->row;
    this->moveFromY = row * 20 + 10;
    this->moveLastY = row * 20 + 10;
}

/* Steps the cursor's move: moveTime falls by 0x155 a frame, and the cursor goes by what the point between
 * moveTo and moveFrom moved since the last frame. */
void func_80074D90(Unk80074220 *this) {
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

INCLUDE_ASM("title/nonmatchings/name_entry", func_80074E28);

INCLUDE_ASM("title/nonmatchings/name_entry", func_80074F68);

/* Prints the name in grey, centred on x 0xA0 at y 0x2F, and every other 8 frames D_800663F8 (the cursor)
 * under the position. */
void func_80075238(Unk80074220 *this, u_long *ot) {
    Unk8007B0A0 printer;
    PrinterVtable *vtable;
    s32 left;

    func_8007B0A0(&printer, ot);
    vtable = printer.printer.vtable;
    vtable->setColor.func((u8 *)&printer + vtable->setColor.delta, 0x606060);
    printer.printer.y = 0x2F;
    left = 0xA0 - (this->maxLength << 4) / 2;
    printer.printer.x = left;
    printerPrint(&printer.printer, this->name);
    if (this->frame & 8) {
        printer.printer.y = 0x30;
        printer.printer.x = (this->position << 4) + left;
        printerPrint(&printer.printer, D_800663F8);
    }
    printerDestroy(&printer.printer, 2);
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_80075304);

INCLUDE_ASM("title/nonmatchings/name_entry", func_8007560C);

/* Tiles the window with the grid's background, 64 pixels a tile (texels (0x80, 0x20) to (0xBF, 0x5F) of
 * page 12, clut 0x3C28), offset by half the scroll. */
void func_800756F0(Unk80074220 *this, u_long *ot) {
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

INCLUDE_ASM("title/nonmatchings/name_entry", func_8007588C);

INCLUDE_ASM("title/nonmatchings/name_entry", func_80075A54);

INCLUDE_ASM("title/nonmatchings/name_entry", func_80075B14);

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x35) to (0xCF, 0x44) of page 12, clut 0x3C68, w and h to each
 * side of (x, y), all its corners of the gray shade (func_80075DF8 passes its object as this, unused). */
void func_80075C18(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
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
 * side of (x, y), all its corners of the gray shade (func_80075DF8 passes its object as this, unused). */
void func_80075D08(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
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

INCLUDE_ASM("title/nonmatchings/name_entry", func_80075DF8);

/* Draws in ot a semi-transparent line from (x0, y0) to (x1, y1) of the colour (r, g, b). */
void func_80075EC8(Unk80074220 *this, u_long *ot, s32 x0, s32 y0, s32 x1, s32 y1, s32 r, s32 g, s32 b) {
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
s32 func_80075F74(Unk80074220 *this) {
    return this->active;
}

/* Returns whether the name was confirmed (no caller in the game). */
s32 func_80075F80(Unk80074220 *this) {
    return this->finished;
}

/* Returns unk68 (no caller in the game). */
s32 func_80075F8C(Unk80074220 *this) {
    return this->unk68;
}

/* Returns which character grid the entry shows (no caller in the game). */
s32 func_80075F98(Unk80074220 *this) {
    return this->unk8;
}

/* Sets the name's maximum length (no caller in the game). */
void func_80075FA4(Unk80074220 *this, s32 maxLength) {
    this->maxLength = maxLength;
}

/* The table's constructor over count records of 0x40 bytes: resets them (func_80076380) when a digest
 * does not match (func_80076298), then links them in their order. */
Unk800764BC *func_80075FAC(Unk800764BC *this, Unk800764F4 *entries, u32 count) {
    this->entries = entries;
    this->head = entries;
    this->count = count;
    this->pending = NULL;
    if (func_80076298(this) == -1) {
        func_80076380(this);
    }
    /* The match depends on the u32 count: its countdown tests against -1. */
    while (count--) {
        entries->next = entries + 1;
        entries++;
    }
    entries[-1].next = NULL;
    return this;
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_80076040);

/* Sorts the records (func_800764BC) and starts the cursor at the first. */
void func_80076138(Unk800764BC *this) {
    this->cursor = func_800764BC(this);
}

/* Steps the cursor to the next record and returns it (NULL at the end). */
Unk800764F4 *func_80076160(Unk800764BC *this) {
    Unk800764F4 *entry = this->cursor;

    if (entry == NULL) {
        return NULL;
    }
    entry = entry->next;
    this->cursor = entry;
    return entry;
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_8007618C);

/* Fills the pending record with a name (when it is shorter than 0x24 bytes) and unk8, and returns 0;
 * -1 when no record is pending or the name is too long. */
s32 func_80076220(Unk800764BC *this, u8 *name, s32 arg) {
    Unk800764F4 *entry = this->pending;

    if (entry != NULL) {
        /* The match depends on taking the name's address before the stores. */
        u8 *dst = entry->name;

        entry->unk8 = arg;
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
s32 func_80076298(Unk800764BC *this) {
    u32 count = this->count;
    Unk800764F4 *entry = this->entries;
    u8 digest[0x10];

    /* The match depends on the u32 count: its countdown tests against -1. */
    while (count--) {
        func_80076310(entry, digest);
        if (func_8003DF40(digest, entry->digest, 0x10) != 0) {
            return -1;
        }
        entry++;
    }
    return 0;
}

/* Computes the MD5 digest of a record's unk4, unk8 and name. */
void func_80076310(Unk800764F4 *entry, u8 *digest) {
    Md5 md5;

    md5Init(&md5);
    MD5Update(&md5, (u8 *)&entry->unk4, 4);
    MD5Update(&md5, (u8 *)&entry->unk8, 4);
    MD5Update(&md5, entry->name, 0x24);
    MD5Final(&md5, digest);
}

INCLUDE_ASM("title/nonmatchings/name_entry", func_80076380);

/* Sorts the records, largest unk4 first (func_800764F4), and returns the first. */
Unk800764F4 *func_800764BC(Unk800764BC *this) {
    return this->head = (Unk800764F4 *)linkNodeSort((LinkNode *)this->head, (LinkCompare)func_800764F4);
}

/* Orders the nodes by their word at 4, largest first. */
s32 func_800764F4(Unk800764F4 *a, Unk800764F4 *b) {
    return b->unk4 - a->unk4;
}
