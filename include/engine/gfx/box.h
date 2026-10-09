#ifndef DTBE_GFX_BOX_H
#define DTBE_GFX_BOX_H

/* Grey boxes on the screen: raised, sunken and framed ones, and darkened rectangles. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

/* A TILE as words, to set its color and code, its position and its size a
 * word at a time. */
typedef struct TileWords {
    /* 0x0 */ u_long tag;
    /* 0x4 */ u32 rgbCode; /* the color, then the primitive's code */
    /* 0x8 */ u32 xy;
    /* 0xC */ u32 wh;
} TileWords;

/* A RECT as words, to copy its position or its size at once. */
typedef struct RectWords {
    /* 0x0 */ u32 xy;
    /* 0x4 */ u32 wh;
} RectWords;

void drawBoxEdges(u_long *ot, RECT *rect, s32 light, s32 dark);
void darkenRect(u_long *ot, RECT *rect);
void fillRectGrey(u_long *ot, RECT *rect, s32 grey);
void rectShrink(RECT *rect);
void rectGrow(RECT *rect);
void drawRaisedBox(u_long *ot, RECT *rect);
void drawSunkenBox(u_long *ot, RECT *rect);
void drawFramedDarkRect(u_long *ot, RECT *rect);

EXTERN_C_END

#endif /* DTBE_GFX_BOX_H */
