#include "common.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/lights.h"
#include "engine/math/matrix.h"
#include "gte.h"

/* Puts the eye on the z axis at -H (the GTE's distance to the projection
 * plane), looking at the origin, y up. Returns this. */
LookAt *lookAtInit(LookAt *this) {
    s32 h;

    gte_getGeomScreen(h);
    this->eye.vx = 0;
    this->eye.vy = 0;
    this->eye.vz = -h;
    this->target.vx = 0;
    this->target.vy = 0;
    this->target.vz = 0;
    this->up.vx = 0;
    this->up.vy = 0x1000;
    this->up.vz = 0;
    this->dirty = 1;
    return this;
}

/* Makes y the up direction. */
void lookAtResetUp(LookAt *this) {
    this->up.vy = 0x1000;
    this->up.vx = 0;
    this->up.vz = 0;
    this->dirty = 1;
}

/* Moves by move, given in the camera's own axes. */
void lookAtMoveLocal(LookAt *this, VECTOR *move) {
    VECTOR worldMove;
    MATRIX view;

    lookAtGetInverseView(this, &view);
    view.t[0] = 0;
    view.t[1] = 0;
    view.t[2] = 0;
    matrixTransformVec(&view, &worldMove, move);
    lookAtMove(this, &worldMove);
}

/* Moves the eye and the target by move, keeping the view direction. */
void lookAtMove(LookAt *this, VECTOR *move) {
    s32 eyeX = this->eye.vx;
    s32 eyeY = this->eye.vy;
    s32 eyeZ = this->eye.vz;
    s32 targetX = this->target.vx;
    s32 targetY = this->target.vy;
    s32 targetZ = this->target.vz;

    eyeX += move->vx;
    eyeY += move->vy;
    eyeZ += move->vz;
    this->eye.vx = eyeX;
    this->eye.vy = eyeY;
    this->eye.vz = eyeZ;
    targetX += move->vx;
    targetY += move->vy;
    targetZ += move->vz;
    this->dirty = 1;
    this->target.vx = targetX;
    this->target.vy = targetY;
    this->target.vz = targetZ;
}

/* Turns the view by yaw then pitch: the target becomes the z axis turned
 * by the turned view matrix. */
void lookAtTurn(LookAt *this, s32 yaw, s32 pitch) {
    MATRIX view;

    if ((yaw | pitch) != 0) {
        lookAtGetInverseView(this, &view);
        matrixRotateY(&view, -yaw);
        matrixRotateX(&view, -pitch);
        matrixTransformVec(&view, &this->target, &AXIS_Z);
        this->dirty = 1;
    }
}

/* Gives the view matrix, rebuilding it first if the camera moved. It copies
 * it as a block move (four loads then four stores), which GCC 2.95.2 does not
 * emit for 32 bytes (see "Block moves" in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/look_at", lookAtGetView);

/* Gives the inverse of the view matrix, the camera's own placement in the
 * world, rebuilding the view first if the camera moved. */
void lookAtGetInverseView(LookAt *this, MATRIX *view) {
    if (this->dirty) {
        matrixInitLookAt(&this->view, &this->eye, &this->target, &this->up);
        this->dirty = 0;
    }
    matrixInvertRigid(view, &this->view);
}
