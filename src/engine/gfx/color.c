#include "common.h"
#include "engine/gfx/color.h"

/* Converts r, g, b (0 to 255) to hsv. */
void convertRgbToHsv(HsvColor *hsv, s32 r, s32 g, s32 b) {
    s32 max;
    s32 maxChannel;
    s32 delta;
    s32 sat;
    s32 hue;

    max = b;
    maxChannel = 0;
    if (max < r) {
        max = r;
        maxChannel = 1;
    }
    if (max < g) {
        max = g;
        maxChannel = 2;
    }
    /* delta is the smallest channel first, then how far the largest is from it */
    delta = b;
    hsv->val = max;
    if (r < delta) {
        delta = r;
    }
    if (g < delta) {
        delta = g;
    }
    delta = max - delta;
    sat = 0;
    if (max != 0) {
        sat = delta * 255 / max;
    }
    hsv->sat = sat;
    if (sat == 0) {
        hsv->hue = HUE_GREY;
        return;
    }
    switch (maxChannel) {
    case 0:
    default:
        hue = (r - g) * HUE_SEXTANT / delta + HUE_BLUE;
        break;
    case 1:
        hue = (g - b) * HUE_SEXTANT / delta;
        break;
    case 2:
        hue = (b - r) * HUE_SEXTANT / delta + HUE_GREEN;
        break;
    }
    if (hue < 0) {
        hue += HUE_TURN;
    }
    hsv->hue = hue;
}

/* HSV to RGB: in C the registers of rgb, sat, val and the hue come out in
 * another order; and its jump table is followed by 12 zero bytes of
 * .rodata (the start of the table split off as D_80019350) that C would not
 * emit. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/color", convertHsvToRgb);
