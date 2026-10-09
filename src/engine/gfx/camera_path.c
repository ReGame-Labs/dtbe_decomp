#include "common.h"
#include "engine/gfx/camera_path.h"

/* Does nothing. */
void func_8002F038(void) {
}

/* Does nothing. */
void func_8002F040(void) {
}

/* Does nothing. */
void func_8002F048(void) {
}

/* Moves the view along the path by t. The original keeps &look.target and
 * &forward in saved registers; C either does not or reads target through
 * the register. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera_path", lookAtPathMove);

/* Sets the eye the view looks from. The setters read all three components
 * before writing any, as vecCopy does; the view is built again when asked for. */
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
