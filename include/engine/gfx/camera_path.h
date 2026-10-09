#ifndef DTBE_GFX_CAMERA_PATH_H
#define DTBE_GFX_CAMERA_PATH_H

/* A view that moves between two eyes and rotations; setting a view's eye, target, up. */

#include "common.h"
#include <libgte.h>
#include "engine/gfx/look_at.h"
#include "engine/math/quaternion.h"
#include "engine/math/vector.h"

EXTERN_C_BEGIN

/* A LookAt that moves between two eyes and turns between two rotations. */
typedef struct {
    /* 0x00 */ LookAt look;
    /* 0x54 */ Quaternion fromRot;
    /* 0x5C */ Quaternion toRot;
    /* 0x64 */ VECTOR fromEye;
    /* 0x74 */ VECTOR toEye;
} LookAtPath;

void func_8002F038(void);
void func_8002F040(void);
void func_8002F048(void);
void lookAtPathMove(LookAtPath *lookAtPath, s32 t);
void lookAtSetEye(LookAt *lookAt, Vec3 *eye);
void lookAtSetTarget(LookAt *lookAt, Vec3 *target);
void lookAtSetUp(LookAt *lookAt, Vec3 *up);

EXTERN_C_END

#endif /* DTBE_GFX_CAMERA_PATH_H */
