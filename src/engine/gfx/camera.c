#include "common.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/ordering_table.h"
#include "engine/math/divide.h"
#include "engine/math/matrix.h"
#include "engine/math/sin_cos.h"
#include "engine/system/memory.h"
#include "inline_c.h"
#include "stdio.h"
#include "gte.h"
#include "vtable.h"

/* the field of view a new camera starts with: about 30 degrees, ONE being a
 * full turn */
#define CAMERA_DEFAULT_FOV 0x155

/* Builds a camera with an ordering table of 1 << length slots, and lights and
 * a projection of its own. */
Camera *cameraInit(Camera *this, s32 length) {
    orderingTableInit(&this->ot, -(1 << length));
    this->zShift = CAMERA_DEPTH_SHIFT - length;
    this->owner = 1;
    this->view = operatorNew(sizeof(CameraView));
    this->lights = operatorNew(sizeof(Lights));
    this->gsot.length = length;
    this->view->fov = CAMERA_DEFAULT_FOV;
    return this;
}

/* Builds a camera with an ordering table of 1 << length slots that shares the
 * lights and the projection of other. */
Camera *cameraInitShared(Camera *this, s32 length, Camera *other) {
    orderingTableInit(&this->ot, -(1 << length));
    this->zShift = CAMERA_DEPTH_SHIFT - length;
    this->owner = 0;
    this->view = other->view;
    this->lights = other->lights;
    this->gsot.length = length;
    return this;
}

/* Destroys the camera. */
void cameraDestroy(Camera *this, s32 flags) {
    if (this->owner) {
        operatorDelete(this->view);
        operatorDelete(this->lights);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* cameraReset, cameraSetLightColor2, cameraSetLightDirection0,
 * cameraSetLightDirection1, cameraSetLightDirection2 and cameraApplyLights
 * copy a MATRIX as a block move (four loads then four stores), which GCC
 * 2.95.2 does not emit for 32 bytes (see "Block moves" in TODO.md). */

/* Puts the camera back to its defaults: a 320x240 screen in 4:3, the
 * nearest near plane, the far plane at 0x3200, a view from the eye at
 * z = -projection and the default lights, whose light matrix it copies to
 * the scratchpad. Also zeroes D_8011A2D0 to D_8011A2F8. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraReset);

/* Sets the color of light 0. */
void cameraSetLightColor0(Camera *this, VECTOR *color) {
    lightsSetColor0(this->lights, color);
}

/* Sets the color of light 1. */
void cameraSetLightColor1(Camera *this, VECTOR *color) {
    lightsSetColor1(this->lights, color);
}

/* Sets the color of light 2, then copies the light matrix to the scratchpad
 * as the direction setters do (light 0 and 1's color setters don't). */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetLightColor2);

/* Sets the back color. */
void cameraSetBackColor(Camera *this, VECTOR *color) {
    lightsSetBackColor(this->lights, color);
}

/* Points light 0 along direction and copies the light matrix to the
 * scratchpad. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetLightDirection0);

/* Points light 1 along direction and copies the light matrix to the
 * scratchpad. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetLightDirection1);

/* Points light 2 along direction and copies the light matrix to the
 * scratchpad. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetLightDirection2);

/* Hands the light colors to the GTE and copies the light matrix to the
 * scratchpad. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraApplyLights);

/* The lights of the camera. */
Lights *cameraGetLights(Camera *this) {
    return this->lights;
}

/* Sets the view matrix: keeps it, builds in the scratchpad the view scaled
 * for the shape of a pixel (SCRATCH_VIEW_MATRIX), keeps its inverse's
 * rotation, and puts the frustum's planes in the world: turned by that
 * rotation, the sides through the eye, the near and the far plane through
 * the points near and far ahead of it. It copies MATRIXes with four loads
 * then four stores, as cameraReset does. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetView);

/* Moves the plane to pass through point. */
void planeSetPoint(Plane *this, VECTOR *point) {
    this->distance = -(this->normal[0] * point->vx + this->normal[1] * point->vy +
                       this->normal[2] * point->vz);
}

/* Loads the model matrix m, combined with the view, into the GTE's rotation
 * and translation, and the camera's lights turned by m into its light
 * matrix. */
void setModelMatrixAndLights(MATRIX *m) {
    MATRIX local;

    setGteLightMatrix(SCRATCH_LIGHT_MATRIX, m);
    matrixMul(&local, SCRATCH_VIEW_MATRIX, m);
    gte_loadRotTrans(&local);
}

/* Loads the model matrix m, combined with the view, into the GTE's rotation
 * and translation. */
void setModelMatrix(MATRIX *m) {
    MATRIX local;

    matrixMul(&local, SCRATCH_VIEW_MATRIX, m);
    gte_loadRotTrans(&local);
}

/* Whether a sphere is at least partly inside the view frustum. */
s32 cameraIsSphereVisible(Camera *this, VECTOR *center, s32 radius) {
    Plane *planes = this->view->planes;

    if (!(planeIsSphereInside(&planes[PLANE_LEFT], center, radius) &&
          planeIsSphereInside(&planes[PLANE_RIGHT], center, radius) &&
          planeIsSphereInside(&planes[PLANE_TOP], center, radius) &&
          planeIsSphereInside(&planes[PLANE_BOTTOM], center, radius) &&
          planeIsSphereInside(&planes[PLANE_NEAR], center, radius) &&
          planeIsSphereInside(&planes[PLANE_FAR], center, radius))) {
        return 0;
    }
    return 1;
}

/* Whether a sphere is at least partly on the inner side of the plane. */
s32 planeIsSphereInside(Plane *this, VECTOR *center, s32 radius) {
    return planeGetDistance(this, center) > -radius;
}

/* How far point is from the plane, on its inner side. */
s32 planeGetDistance(Plane *this, VECTOR *point) {
    return (this->normal[0] * point->vx + this->normal[1] * point->vy +
            this->normal[2] * point->vz + this->distance) >> 12;
}

/* Clips the line from start to end to the sides of the view frustum. Returns
 * whether it is outside them. */
s32 cameraClipLine(Camera *this, SVECTOR *start, SVECTOR *end) {
    Plane *planes = this->view->planes;
    Outcodes *outcodes;
    s32 startCode;
    s32 endCode;
    s32 i;
    s32 j;

    for (i = 0, outcodes = CLIP_OUTCODES; i < SIDE_PLANE_COUNT; outcodes++, i++) {
        startCode = getFrustumOutcode(planes, start);
        endCode = getFrustumOutcode(planes, end);
        outcodes->start = startCode;
        outcodes->end = endCode;
        if ((startCode | endCode) == 0) {
            break;
        }
        if (startCode & endCode) {
            return 1;
        }
        if (startCode) {
            clipPointToFrustum(planes, start, end, startCode);
        }
        if (endCode) {
            clipPointToFrustum(planes, end, start, endCode);
        }
    }
    if (i > SIDE_PLANE_COUNT) {
        for (j = 0; j < i; j++) {
            printf("%d:%04lx %04lx\n", j, CLIP_OUTCODES[j].start, CLIP_OUTCODES[j].end);
        }
    }
    return 0;
}

/* The sides of the view frustum point is outside of, a bit for each. */
s32 getFrustumOutcode(Plane *planes, SVECTOR *point) {
    s32 outcode = 0;
    s32 bit = 1;
    Plane *plane = planes;
    s32 i;

    for (i = 0; i < SIDE_PLANE_COUNT; i++) {
        if (planeGetDistanceFixed(plane, point) < 0) {
            outcode |= bit;
        }
        plane++;
        bit *= 2;
    }
    return outcode;
}

/* Moves point toward other onto the first plane of outcode it is outside of. */
void clipPointToFrustum(Plane *planes, SVECTOR *point, SVECTOR *other, u32 outcode) {
    s32 distance;
    s32 span;

    while (!(outcode & 1)) {
        outcode >>= 1;
        planes++;
    }
    distance = planeGetDistanceFixed(planes, point) >> 12;
    span = (planeGetDistanceFixed(planes, other) >> 12) - distance;
    if (span != 0) {
        point->vx += (other->vx - point->vx) * -distance / span;
        point->vy += (other->vy - point->vy) * -distance / span;
        point->vz += (other->vz - point->vz) * -distance / span;
    }
}

/* How far point is from the plane, on its inner side, in 4.12 fixed point. */
s32 planeGetDistanceFixed(Plane *this, SVECTOR *point) {
    return this->normal[0] * point->vx + this->normal[1] * point->vy +
           this->normal[2] * point->vz + this->distance;
}

/* Clears the ordering table for a new frame. */
void cameraClearOt(Camera *this) {
    orderingTableClear(&this->ot);
    this->gsot.point = 0;
    this->gsot.offset = 0;
    this->gsot.org = (GsOT_TAG *)this->ot.slots;
    this->gsot.tag = (GsOT_TAG *)this->ot.slots;
}

/* Sets the size of the screen and the shape of its pixels, and the GTE's
 * screen offset to its center. In C (ctc2 $24/$25 of (size / 2) << 16) it
 * matches but for the load of aspectY from the stack: GCC 2.95.2 converts
 * that parameter at the entry and schedules its lh before the load of
 * this->view, the game has it after. The two tie in both scheduling passes
 * and the one later in the insn stream wins, which the entry conversion
 * never is; const parameters and C++ change nothing. */
INCLUDE_ASM("asm/jp/main/nonmatchings/gfx/camera", cameraSetScreen);

/* Sets the field of view: the distance of the screen from the eye (half the
 * width over the tangent of half the angle), no nearer near plane than half of
 * it, and the frustum's planes relative to the eye. */
void cameraSetFov(Camera *this, s16 fov) {
    CameraView *view = this->view;
    s32 angle;
    s32 sin;
    s32 projection;
    s32 vertical;
    s32 cosX;
    s32 sinX;
    s32 cosY;
    s32 sinY;

    angle = fov;
    view->fov = angle;
    angle /= 2;
    sin = COS_SIN_TABLE[angle & 0xFFF].sin;
    projection = mul12(divide12(view->width / 2, sin), COS_SIN_TABLE[angle & 0xFFF].cos);
    view->projection = projection;
    gte_SetGeomScreen(projection);
    if (view->near < projection / 2) {
        view->near = projection / 2;
    }

    /* the left and the right side */
    cosX = COS_SIN_TABLE[angle & 0xFFF].cos;
    sinX = COS_SIN_TABLE[angle & 0xFFF].sin;
    view->localPlanes[PLANE_LEFT].normal[0] = cosX;
    view->localPlanes[PLANE_LEFT].normal[1] = 0;
    view->localPlanes[PLANE_LEFT].normal[2] = sinX;
    view->localPlanes[PLANE_LEFT].distance = 0;
    view->localPlanes[PLANE_RIGHT].normal[0] = -cosX;
    view->localPlanes[PLANE_RIGHT].normal[1] = 0;
    view->localPlanes[PLANE_RIGHT].normal[2] = sinX;
    view->localPlanes[PLANE_RIGHT].distance = 0;

    /* the top and the bottom, half the vertical field of view away */
    vertical = angle * view->aspectY / view->aspectX;
    cosY = COS_SIN_TABLE[vertical & 0xFFF].cos;
    sinY = COS_SIN_TABLE[vertical & 0xFFF].sin;
    view->localPlanes[PLANE_TOP].normal[0] = 0;
    view->localPlanes[PLANE_TOP].normal[1] = cosY;
    view->localPlanes[PLANE_TOP].normal[2] = sinY;
    view->localPlanes[PLANE_TOP].distance = 0;
    view->localPlanes[PLANE_BOTTOM].normal[0] = 0;
    view->localPlanes[PLANE_BOTTOM].normal[1] = -cosY;
    view->localPlanes[PLANE_BOTTOM].normal[2] = sinY;
    view->localPlanes[PLANE_BOTTOM].distance = 0;

    view->localPlanes[PLANE_NEAR].normal[0] = 0;
    view->localPlanes[PLANE_NEAR].normal[1] = 0;
    view->localPlanes[PLANE_NEAR].normal[2] = ONE;
    view->localPlanes[PLANE_NEAR].distance = 0;
    view->localPlanes[PLANE_FAR].normal[0] = 0;
    view->localPlanes[PLANE_FAR].normal[1] = 0;
    view->localPlanes[PLANE_FAR].normal[2] = -ONE;
    view->localPlanes[PLANE_FAR].distance = 0;
}

/* Sets the near plane, no nearer than half the distance of the screen. */
void cameraSetNear(Camera *this, s32 near) {
    CameraView *view = this->view;
    s32 nearest = view->projection / 2;

    if (near < nearest) {
        near = nearest;
    }
    view->near = near;
}

/* Sets the far plane. */
void cameraSetFar(Camera *this, s32 far) {
    this->view->far = far;
}

/* The distance of the screen from the eye. */
s16 cameraGetProjection(Camera *this) {
    return this->view->projection;
}

/* The field of view. */
s16 cameraGetFov(Camera *this) {
    return this->view->fov;
}

/* Gives camera the lights until this is destroyed. */
LightsOverride *lightsOverrideInit(LightsOverride *this, Camera *camera, Lights *lights) {
    this->camera = camera;
    this->saved = camera->lights;
    camera->lights = lights;
    cameraApplyLights(camera);
    return this;
}

/* Gives the camera its lights back. */
void lightsOverrideDestroy(LightsOverride *this, s32 flags) {
    this->camera->lights = this->saved;
    cameraApplyLights(this->camera);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}
