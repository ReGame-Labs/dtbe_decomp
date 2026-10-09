#include "common.h"
#include "engine/gfx/box.h"
#include "engine/gfx/prim/alloc_line_f3.h"
#include "engine/gfx/prim/alloc_sprt.h"

/* the greys of the edges and the faces of the boxes */
#define BOX_WHITE 0xFF
#define BOX_LIGHT 0xDF
#define BOX_FACE 0xC0
#define BOX_DARK 0x80
#define BOX_BLACK 0x00
/* the texture page of darkenRect's shadow: it subtracts its color */
#define TPAGE_SUBTRACT 0x40

/* Draws the edges of a box: the top and left ones in one grey, the bottom
 * and right ones in another. */
void drawBoxEdges(u_long *ot, RECT *rect, s32 light, s32 dark) {
    LINE_F3 *topLeft = allocLineF3();
    LINE_F3 *bottomRight = allocLineF3();
    s16 x = rect->x;
    s16 y = rect->y;
    s16 right = rect->x + rect->w - 1;
    s16 bottom = rect->y + rect->h - 1;

    setRGB0(topLeft, light, light, light);
    setXY3(topLeft, right, y, x, y, x, bottom);
    setRGB0(bottomRight, dark, dark, dark);
    setXY3(bottomRight, x, bottom, right, bottom, right, y);
    AddPrim(ot, bottomRight);
    AddPrim(ot, topLeft);
}

/* Darkens a rectangle. */
void darkenRect(u_long *ot, RECT *rect) {
    /* to set its fields, and read the RECT, a word at a time */
    TileWords *tile = (TileWords *)allocTile();
    RectWords *words = (RectWords *)rect;

    tile->rgbCode = (((TILE *)tile)->code << 24) | 0x02808080;
    tile->xy = words->xy;
    tile->wh = words->wh;
    AddPrim(ot, tile);
    AddPrim(ot, allocDrTpage(TPAGE_SUBTRACT));
}

/* Fills a rectangle with a grey. */
void fillRectGrey(u_long *ot, RECT *rect, s32 grey) {
    TILE *tile = allocTile();

    setRGB0(tile, grey, grey, grey);
    tile->x0 = rect->x;
    tile->y0 = rect->y;
    tile->w = rect->w;
    tile->h = rect->h;
    AddPrim(ot, tile);
}

/* Shrinks a rectangle by a pixel on each side. */
void rectShrink(RECT *rect) {
    rect->x++;
    rect->y++;
    rect->w -= 2;
    rect->h -= 2;
}

/* Grows a rectangle by a pixel on each side. */
void rectGrow(RECT *rect) {
    rect->x--;
    rect->y--;
    rect->w += 2;
    rect->h += 2;
}

/* Draws a raised box over the rectangle. */
void drawRaisedBox(u_long *ot, RECT *rect) {
    drawBoxEdges(ot, rect, BOX_WHITE, BOX_BLACK);
    rectShrink(rect);
    drawBoxEdges(ot, rect, BOX_LIGHT, BOX_DARK);
    fillRectGrey(ot, rect, BOX_FACE);
}

/* Draws a sunken box over the rectangle. */
void drawSunkenBox(u_long *ot, RECT *rect) {
    drawBoxEdges(ot, rect, BOX_BLACK, BOX_WHITE);
    rectShrink(rect);
    drawBoxEdges(ot, rect, BOX_DARK, BOX_LIGHT);
    fillRectGrey(ot, rect, BOX_FACE);
}

/* Draws a frame around the rectangle, and darkens it. */
void drawFramedDarkRect(u_long *ot, RECT *rect) {
    RECT frame = *rect;

    rectGrow(&frame);
    drawBoxEdges(ot, &frame, BOX_BLACK, BOX_LIGHT);
    rectGrow(&frame);
    drawBoxEdges(ot, &frame, BOX_FACE, BOX_FACE);
    rectGrow(&frame);
    drawBoxEdges(ot, &frame, BOX_FACE, BOX_FACE);
    rectGrow(&frame);
    drawBoxEdges(ot, &frame, BOX_WHITE, BOX_DARK);
    rectGrow(&frame);
    drawBoxEdges(ot, &frame, BOX_LIGHT, BOX_BLACK);
    darkenRect(ot, rect);
}
