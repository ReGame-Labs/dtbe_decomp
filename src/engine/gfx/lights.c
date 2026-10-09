#include "common.h"
#include "engine/gfx/lights.h"
#include "engine/math/matrix.h"
#include "engine/math/vector.h"
#include "gte.h"

/* Sets the lights to their defaults and hands them to the GTE. */
void lightsReset(Lights *this) {
    /* both matrices start as the identity, set two elements at a time */
    this->light.words[0] = ONE;
    this->light.words[1] = 0;
    this->light.words[2] = ONE;
    this->light.words[3] = 0;
    this->light.words[4] = ONE;
    this->light.words[5] = 0;
    this->light.words[6] = 0;
    this->light.words[7] = 0;
    this->color.words[0] = ONE;
    this->color.words[1] = 0;
    this->color.words[2] = ONE;
    this->color.words[3] = 0;
    this->color.words[4] = ONE;
    this->color.words[5] = 0;
    this->color.words[6] = 0;
    this->color.words[7] = 0;
    lightsSetDirection0(this, &LIGHTS_DEFAULT_DIRECTION0);
    lightsSetDirection1(this, &LIGHTS_DEFAULT_DIRECTION1);
    lightsSetDirection2(this, &LIGHTS_DEFAULT_DIRECTION2);
    lightsSetColor0(this, &LIGHTS_DEFAULT_COLOR0);
    lightsSetColor1(this, &LIGHTS_DEFAULT_COLOR1);
    lightsSetColor2(this, &LIGHTS_DEFAULT_COLOR2);
    lightsSetBackColor(this, &LIGHTS_DEFAULT_BACK_COLOR);
    lightsSetGteColors(this);
}

/* Sets the color of light 0. */
void lightsSetColor0(Lights *this, VECTOR *color) {
    this->color.m.m[0][0] = color->vx;
    this->color.m.m[1][0] = color->vy;
    this->color.m.m[2][0] = color->vz;
}

/* Sets the color of light 1. */
void lightsSetColor1(Lights *this, VECTOR *color) {
    this->color.m.m[0][1] = color->vx;
    this->color.m.m[1][1] = color->vy;
    this->color.m.m[2][1] = color->vz;
}

/* Sets the color of light 2. */
void lightsSetColor2(Lights *this, VECTOR *color) {
    this->color.m.m[0][2] = color->vx;
    this->color.m.m[1][2] = color->vy;
    this->color.m.m[2][2] = color->vz;
}

/* Turns a color component in 4.12 fixed point into 0 to 255. */
s32 convertColorComponent(s32 value) {
    value /= 16;
    if (value > 255) {
        value = 255;
    } else if (value < 0) {
        value = 0;
    }
    return value;
}

/* Sets the back color. */
void lightsSetBackColor(Lights *this, VECTOR *color) {
    this->color.m.t[0] = convertColorComponent(color->vx);
    this->color.m.t[1] = convertColorComponent(color->vy);
    this->color.m.t[2] = convertColorComponent(color->vz);
}

/* Points light 0 in direction: its row of the light matrix is the unit
 * vector the other way, towards the light. */
void lightsSetDirection0(Lights *this, VECTOR *direction) {
    VECTOR unit;

    vecNormalize(&unit, direction);
    this->light.m.m[0][0] = -unit.vx;
    this->light.m.m[0][1] = -unit.vy;
    this->light.m.m[0][2] = -unit.vz;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_DIRECTION0);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_DIRECTION1);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_DIRECTION2);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_COLOR0);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_COLOR1);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_COLOR2);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", LIGHTS_DEFAULT_BACK_COLOR);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", AXIS_Y);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/lights", AXIS_Z);

/* Points light 1 in direction (see lightsSetDirection0). */
void lightsSetDirection1(Lights *this, VECTOR *direction) {
    VECTOR unit;

    vecNormalize(&unit, direction);
    this->light.m.m[1][0] = -unit.vx;
    this->light.m.m[1][1] = -unit.vy;
    this->light.m.m[1][2] = -unit.vz;
}

/* Points light 2 in direction (see lightsSetDirection0). */
void lightsSetDirection2(Lights *this, VECTOR *direction) {
    VECTOR unit;

    vecNormalize(&unit, direction);
    this->light.m.m[2][0] = -unit.vx;
    this->light.m.m[2][1] = -unit.vy;
    this->light.m.m[2][2] = -unit.vz;
}

/* Loads the colors of the lights, the back color and the far color into the
 * GTE. C: the addiu that makes &this->color in $a0 for the macro, rather
 * than offsets from this in the loads, is what GCC emits. */
void lightsSetGteColors(Lights *this) {
    gte_loadLightColors(&this->color.m);
}

/* Sets the GTE's light matrix to light times rotation. Written by hand: no
 * frame, no instruction a compiler chose (gte_mulMatrix's fixed registers and
 * column packing, ending in ctc2s to the light matrix instead of sws), an
 * unfilled jr $ra delay slot. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/lights", setGteLightMatrix);
