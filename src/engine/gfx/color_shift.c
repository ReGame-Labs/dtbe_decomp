#include "common.h"
#include "engine/gfx/color_shift.h"

/* Shifts one opaque 15-bit color. As an inline function its u16 parameter is
 * a variable of its own, reused for each channel, which gives the game's
 * order of the unpacking; the packing is one channel per statement, as the
 * game's order of its loads shows. */
static inline u16 shiftColor(u16 color, HsvColor *shift) {
    CVECTOR rgb;
    HsvColor hsv;
    s32 saturation;
    s32 value;
    u16 packed;

    rgb.r = color << 3;
    color >>= 5;
    rgb.g = color << 3;
    color >>= 5;
    rgb.b = color << 3;
    color >>= 5;
    rgb.cd = color;
    convertRgbToHsv(&hsv, rgb.r, rgb.g, rgb.b);
    saturation = (hsv.saturation * shift->saturation) >> 7;
    value = (hsv.value * shift->value) >> 7;
    hsv.hue += shift->hue;
    hsv.saturation = saturation < 0 ? 0 : saturation > 0xFF ? 0xFF : saturation;
    hsv.value = value < 0 ? 0 : value > 0xFF ? 0xFF : value;
    convertHsvToRgb(&rgb, hsv.hue, hsv.saturation, hsv.value);
    rgb.cd = 1;
    packed = rgb.r >> 3;
    packed |= (rgb.g >> 3) << 5;
    packed |= (rgb.b >> 3) << 10;
    return packed | 0x8000;
}

/* Whether a 15-bit color is the transparent one, 0. As a function, its test
 * is a flag: the game stores the color read where a test of color itself
 * would let the compiler store 0. */
static inline s32 isTransparent(u16 color) {
    return color == 0;
}

/* Shifts the hue, saturation and value of count 15-bit colors from src into
 * dst by shift, copying transparent colors as they are. */
void shiftColors(u16 *src, u16 *dst, u32 count, HsvColor *shift) {
    u16 color;

    while (count--) {
        color = *src++;
        if (isTransparent(color)) {
            *dst = color;
        } else {
            *dst = shiftColor(color, shift);
        }
        dst++;
    }
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/color_shift", D_80019350);
