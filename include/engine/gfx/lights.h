#ifndef DTBE_GFX_LIGHTS_H
#define DTBE_GFX_LIGHTS_H

/* The three lights of a scene, as the GTE takes them: their directions and colors. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* The three lights of a scene, as the GTE takes them. */
typedef struct {
    /* 0x00 */ MATRIX light; /* the direction of light i in row i */
    /* 0x20 */ MATRIX color; /* the color of light i in column i, the back
                              * color (0 to 255) in t */
} Lights;

/* A matrix as eight words, to clear or set two of its elements at once. */
typedef union {
    MATRIX m;
    u32 words[8];
} MatrixWords;

/* the defaults lightsReset gives the lights: the directions of lights 0,
 * 1 and 2, their colors, then the back color */
extern VECTOR LIGHTS_DEFAULT_DIRECTION0;
extern VECTOR LIGHTS_DEFAULT_DIRECTION1;
extern VECTOR LIGHTS_DEFAULT_DIRECTION2;
extern VECTOR LIGHTS_DEFAULT_COLOR0;
extern VECTOR LIGHTS_DEFAULT_COLOR1;
extern VECTOR LIGHTS_DEFAULT_COLOR2;
extern VECTOR LIGHTS_DEFAULT_BACK_COLOR;
/* the unit y and z axes, in 4.12 fixed point */
extern VECTOR AXIS_Y; /* the y axis */
extern VECTOR AXIS_Z; /* the z axis */

void lightsReset(Lights *lights);
void lightsSetColor0(Lights *lights, VECTOR *color);
void lightsSetColor1(Lights *lights, VECTOR *color);
void lightsSetColor2(Lights *lights, VECTOR *color);
s32 convertColorComponent(s32 value);
void lightsSetBackColor(Lights *lights, VECTOR *color);
void lightsSetDirection0(Lights *lights, VECTOR *direction);

void lightsSetDirection1(Lights *lights, VECTOR *direction);
void lightsSetDirection2(Lights *lights, VECTOR *direction);
void lightsSetGteColors(Lights *lights);
void setGteLightMatrix(MATRIX *light, MATRIX *rotation);

EXTERN_C_END

#endif /* DTBE_GFX_LIGHTS_H */
