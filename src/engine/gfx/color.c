#include "common.h"
#include "engine/gfx/color.h"

/* Converts r, g, b (0 to 255) to hsv. */
void convertRgbToHsv(HsvColor *hsv, s32 r, s32 g, s32 b) {
    s32 max;
    s32 maxChannel;
    s32 delta;
    s32 saturation;
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
    /* delta is the smallest channel first, then how far the largest is from
     * it */
    delta = b;
    hsv->value = max;
    if (r < delta) {
        delta = r;
    }
    if (g < delta) {
        delta = g;
    }
    delta = max - delta;
    saturation = 0;
    if (max != 0) {
        saturation = delta * 255 / max;
    }
    hsv->saturation = saturation;
    if (saturation == 0) {
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

/* Converts hsv to rgb. The hue may be outside one turn, either way; a
 * color without saturation is grey. p, q and t are value scaled by
 * 1 - saturation, 1 - saturation * f and 1 - saturation * (1 - f), rounded,
 * where f is how far the hue is into its sextant.
 *
 * saturation is masked to a byte only where it is tested: the
 * multiplications use the register as it came, so it is an int here, not a
 * u8 like value. One variable holds how far into the sextant, then the
 * sextant: they share a register that lives into the switch's blocks. */
void convertHsvToRgb(CVECTOR *rgb, s32 hue, s32 saturation, u8 value) {
    s32 angle;
    s32 p, q, t;
    s32 i;

    if ((u8)saturation == 0) {
        rgb->r = value;
        rgb->g = value;
        rgb->b = value;
        return;
    }
    if (hue < 0) {
        angle = HUE_TURN - -hue % HUE_TURN;
    } else {
        angle = hue % HUE_TURN;
    }
    i = angle & (HUE_SEXTANT - 1);
    p = (value * (0xFF00 - saturation * HUE_SEXTANT) + 0x7F80) / 0xFF00;
    q = (value * (0xFF00 - saturation * i) + 0x7F80) / 0xFF00;
    t = (value * (0xFF00 - saturation * (HUE_SEXTANT - i)) + 0x7F80) / 0xFF00;
    i = angle >> HUE_SEXTANT_SHIFT;
    switch (i) {
    case 0:
    case 6:
        rgb->r = value;
        rgb->g = t;
        rgb->b = p;
        break;
    case 1:
        rgb->r = q;
        rgb->g = value;
        rgb->b = p;
        break;
    case 2:
        rgb->r = p;
        rgb->g = value;
        rgb->b = t;
        break;
    case 3:
        rgb->r = p;
        rgb->g = q;
        rgb->b = value;
        break;
    case 4:
        rgb->r = t;
        rgb->g = p;
        rgb->b = value;
        break;
    case 5:
        rgb->r = value;
        rgb->g = p;
        rgb->b = q;
        break;
    }
}

/* 12 zero bytes after convertHsvToRgb's jump table: what they are is not
 * known */
INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/color", D_80019344);
