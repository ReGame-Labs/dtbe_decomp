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
    this->up.vy = ONE;
    this->up.vz = 0;
    this->dirty = 1;
    return this;
}

/* Makes y the up direction. */
void lookAtResetUp(LookAt *this) {
    this->up.vy = ONE;
    this->up.vx = 0;
    this->up.vz = 0;
    this->dirty = 1;
}

/* Moves by move, given in the view's own axes. */
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

/* Gives the view matrix, rebuilding it first if the view moved. It copies
 * it as a block move (four loads then four stores), which GCC 2.95.2 does not
 * emit for 32 bytes (see "Block moves" in TODO.md). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/look_at", lookAtGetView);

/* Gives the inverse of the view matrix, the view's own placement in the
 * world, rebuilding the view first if it moved. */
void lookAtGetInverseView(LookAt *this, MATRIX *view) {
    if (this->dirty) {
        matrixInitLookAt(&this->view, &this->eye, &this->target, &this->up);
        this->dirty = 0;
    }
    matrixInvertRigid(view, &this->view);
}

/* Does nothing. */
void func_8002F038(void) {
}

/* Does nothing. */
void func_8002F040(void) {
}

/* Does nothing. */
void func_8002F048(void) {
}

/* Moves the view along the path by t: the eye goes from fromEye to toEye, and
 * the rotation from fromRotation to toRotation turns the up direction (the y
 * axis) and the target. The target becomes (0, 0, distance) turned, a point
 * distance away from the origin, distance being the eye's distance to the old
 * target (ONE when it was under 16). The match depends on target and in,
 * copies of &look.target and &forward: the original computes them into saved
 * registers ($s3, $s1) before vecGetLength12 and quaternionSlerp and passes
 * them after, while it reads target's components at their offsets from this
 * ($s2); without the two pointers GCC computes both addresses after the
 * calls, and with target->vx it reads them through $s3.
 * direction copies the target before subtracting the eye, as the order of its
 * loads shows. */
void lookAtPathMove(LookAtPath *this, s32 t) {
    VECTOR forward;
    Quaternion rotation;
    VECTOR direction;
    s32 distance;
    VECTOR *target = &this->look.target;
    VECTOR *in;

    direction.vx = this->look.target.vx;
    direction.vy = this->look.target.vy;
    direction.vz = this->look.target.vz;
    direction.vx -= this->look.eye.vx;
    direction.vy -= this->look.eye.vy;
    direction.vz -= this->look.eye.vz;
    distance = vecGetLength12(&direction);
    if (distance < 16) {
        distance = ONE;
    }
    in = &forward;
    forward.vx = 0;
    forward.vy = 0;
    forward.vz = distance;
    quaternionSlerp(&rotation, &this->fromRotation, &this->toRotation, t);
    quaternionRotateVec(&rotation, target, in);
    quaternionRotateVec(&rotation, &this->look.up, &AXIS_Y);
    vecLerp(&this->look.eye, &this->fromEye, &this->toEye, t);
}

/* Sets the eye the view looks from. The setters read all three components
 * before writing any, as vecCopy does; the view is built again when asked
 * for. */
void lookAtSetEye(LookAt *this, Vec3 *eye) {
    s32 x, y, z;

    this->dirty = 1;
    x = eye->vx;
    y = eye->vy;
    z = eye->vz;
    this->eye.vx = x;
    this->eye.vy = y;
    this->eye.vz = z;
}

/* Sets the point the view looks at. */
void lookAtSetTarget(LookAt *this, Vec3 *target) {
    s32 x, y, z;

    this->dirty = 1;
    x = target->vx;
    y = target->vy;
    z = target->vz;
    this->target.vx = x;
    this->target.vy = y;
    this->target.vz = z;
}

/* Sets the view's up direction. */
void lookAtSetUp(LookAt *this, Vec3 *up) {
    s32 x, y, z;

    this->dirty = 1;
    x = up->vx;
    y = up->vy;
    z = up->vz;
    this->up.vx = x;
    this->up.vy = y;
    this->up.vz = z;
}
