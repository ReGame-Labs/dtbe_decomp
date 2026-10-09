#ifndef DTBE_GFX_COLOR_H
#define DTBE_GFX_COLOR_H

/* Colors as hue, saturation and value, and converting them to and from RGB. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A colour as hue, saturation and value. As a tint, sat and val scale by
 * n / 128 and hue turns. */
typedef struct {
    s16 hue;
    u8 sat;
    u8 val;
} HsvColor;

#define HUE_TURN 0x600     /* a full turn of hue; red is at 0 */
#define HUE_SEXTANT (HUE_TURN / 6)
#define HUE_GREEN (HUE_TURN / 3)
#define HUE_BLUE (HUE_TURN * 2 / 3)
#define HUE_GREY -1        /* the hue of a colour without saturation */

void convertRgbToHsv(HsvColor *hsv, s32 r, s32 g, s32 b);
void convertHsvToRgb(CVECTOR *rgb, s32 hue, u8 sat, u8 val);

EXTERN_C_END

#endif /* DTBE_GFX_COLOR_H */
