#ifndef DTBE_GFX_LOOK_AT_H
#define DTBE_GFX_LOOK_AT_H

/* A view looking from an eye at a target: moving it, turning it and its view matrix. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

/* A view looking from eye at target, with its matrix built on demand. */
typedef struct {
    /* 0x00 */ VECTOR eye;
    /* 0x10 */ VECTOR target;
    /* 0x20 */ VECTOR up;
    /* 0x30 */ MATRIX view;
    /* 0x50 */ s32 dirty; /* view is out of date */
} LookAt;

LookAt *lookAtInit(LookAt *lookAt);
void lookAtResetUp(LookAt *lookAt);
void lookAtMoveLocal(LookAt *lookAt, VECTOR *move);
void lookAtMove(LookAt *lookAt, VECTOR *move);
void lookAtTurn(LookAt *lookAt, s32 yaw, s32 pitch);
void lookAtGetView(LookAt *lookAt, MATRIX *view);
void lookAtGetInverseView(LookAt *lookAt, MATRIX *view);

EXTERN_C_END

#endif /* DTBE_GFX_LOOK_AT_H */
