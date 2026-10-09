#ifndef DTBE_GFX_LOOK_AT_H
#define DTBE_GFX_LOOK_AT_H

/* A view looking from an eye at a target: moving it, turning it, setting it,
 * its view matrix, and moving it along a path between two eyes and rotations. */

#include "common.h"
#include <libgte.h>
#include "engine/math/quaternion.h"
#include "engine/math/vector.h"

EXTERN_C_BEGIN

/* A view looking from eye at target, with its matrix built on demand. */
typedef struct {
    /* 0x00 */ VECTOR eye;
    /* 0x10 */ VECTOR target;
    /* 0x20 */ VECTOR up;
    /* 0x30 */ MATRIX view;
    /* 0x50 */ s32 dirty; /* view is out of date */
} LookAt;

/* A LookAt that moves between two eyes and turns between two rotations. */
typedef struct {
    /* 0x00 */ LookAt look;
    /* 0x54 */ Quaternion fromRotation;
    /* 0x5C */ Quaternion toRotation;
    /* 0x64 */ VECTOR fromEye;
    /* 0x74 */ VECTOR toEye;
} LookAtPath;

LookAt *lookAtInit(LookAt *lookAt);
void lookAtResetUp(LookAt *lookAt);
void lookAtMoveLocal(LookAt *lookAt, VECTOR *move);
void lookAtMove(LookAt *lookAt, VECTOR *move);
void lookAtTurn(LookAt *lookAt, s32 yaw, s32 pitch);
void lookAtGetView(LookAt *lookAt, MATRIX *view);
void lookAtGetInverseView(LookAt *lookAt, MATRIX *view);
void func_8002F038(void);
void func_8002F040(void);
void func_8002F048(void);
void lookAtPathMove(LookAtPath *lookAtPath, s32 t);
void lookAtSetEye(LookAt *lookAt, Vec3 *eye);
void lookAtSetTarget(LookAt *lookAt, Vec3 *target);
void lookAtSetUp(LookAt *lookAt, Vec3 *up);

EXTERN_C_END

#endif /* DTBE_GFX_LOOK_AT_H */
