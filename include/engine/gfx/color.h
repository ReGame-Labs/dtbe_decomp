#ifndef DTBE_GFX_COLOR_H
#define DTBE_GFX_COLOR_H

/* Colors as hue, saturation and value, and converting them to and from RGB. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A color as hue, saturation and value. As a tint, saturation and value
 * scale by n / 128 and hue turns. */
typedef struct {
    /* 0x0 */ s16 hue;
    /* 0x2 */ u8 saturation;
    /* 0x3 */ u8 value;
} HsvColor;

#define HUE_TURN 0x600              /* a full turn of hue; red is at 0 */
#define HUE_SEXTANT (HUE_TURN / 6)  /* a sixth of a turn */
#define HUE_SEXTANT_SHIFT 8         /* HUE_SEXTANT is 1 << 8 */
#define HUE_GREEN (HUE_TURN / 3)    /* the hue of green */
#define HUE_BLUE (HUE_TURN * 2 / 3) /* the hue of blue */
#define HUE_GREY (-1)               /* the hue of a color without saturation */

void convertRgbToHsv(HsvColor *hsv, s32 r, s32 g, s32 b);
void convertHsvToRgb(CVECTOR *rgb, s32 hue, s32 saturation, u8 value);

EXTERN_C_END

#endif /* DTBE_GFX_COLOR_H */
