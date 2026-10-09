#ifndef DTBE_GFX_CAMERA_H
#define DTBE_GFX_CAMERA_H

/* The cameras: their ordering tables, lights, projection and view frustum. */

#include "common.h"
#include <sys/types.h>
#include <libgte.h>
#include <libgs.h>
#include "engine/gfx/lights.h"
#include "engine/gfx/ordering_table.h"

EXTERN_C_BEGIN

/* A plane of the view frustum: its normal (4.12 fixed point) and its
 * distance from the origin along it, negated. */
typedef struct {
    /* 0x0 */ s32 normal[3];
    /* 0xC */ s32 distance;
} Plane;

/* The planes of the view frustum: first the four sides, which cameraClipLine
 * clips lines against, then the near and the far plane. Their normals point
 * into the frustum. */
#define SIDE_PLANE_COUNT 4
#define PLANE_NEAR 4
#define PLANE_FAR 5
#define PLANE_COUNT 6

/* The projection of a Camera and its view frustum, which copies of the
 * camera share. */
typedef struct {
    /* 0x000 */ s16 width;      /* of the screen */
    /* 0x002 */ s16 height;
    /* 0x004 */ s16 aspectX;    /* the shape of a screen pixel */
    /* 0x006 */ s16 aspectY;
    /* 0x008 */ s16 fov;        /* the horizontal field of view, as an angle */
    /* 0x00A */ s16 projection; /* the distance of the screen from the eye */
    /* 0x00C */ s32 near;
    /* 0x010 */ s32 far;
    /* 0x014 */ Plane localPlanes[PLANE_COUNT]; /* relative to the eye */
    /* 0x074 */ Plane planes[PLANE_COUNT];      /* in the world */
    /* 0x0D4 */ MATRIX view;
    /* 0x0F4 */ MATRIX rotation; /* the inverse of view, without its translation */
} CameraView; /* size 0x114 */

/* A camera: its ordering table, its lights and its projection. */
typedef struct {
    /* 0x00 */ OrderingTable ot;
    /* 0x18 */ GsOT gsot;   /* the same table for libgs */
    /* 0x2C */ s32 zShift;  /* how far to shift a depth to get a slot */
    /* 0x30 */ Lights *lights;
    /* 0x34 */ CameraView *view;
    /* 0x38 */ s32 owner;   /* allocated lights and view, rather than share them */
} Camera; /* size 0x3C */

/* Lights lent to a camera for as long as the LightsOverride lives. */
typedef struct {
    /* 0x0 */ Camera *camera;
    /* 0x4 */ Lights *saved; /* the lights the camera had */
} LightsOverride;

/* Depths go up to 1 << 14: zShift brings them down to the slots of a table of
 * 1 << length. */
#define CAMERA_DEPTH_SHIFT 14

/* The camera's view matrix scaled for the shape of a pixel, which
 * cameraSetView builds in the scratchpad, and the light matrix of its lights,
 * which cameraApplyLights copies there. */
#define SCRATCH_VIEW_MATRIX ((MATRIX *)0x1F800000)
#define SCRATCH_LIGHT_MATRIX ((MATRIX *)0x1F800020)

/* the outcodes of the ends of a line in the last call to cameraClipLine */
typedef struct {
    u_long start;
    u_long end;
} Outcodes;

extern Outcodes CLIP_OUTCODES[SIDE_PLANE_COUNT];

Camera *cameraInit(Camera *camera, s32 length);
Camera *cameraInitShared(Camera *camera, s32 length, Camera *other);
void cameraDestroy(Camera *camera, s32 flags);
void cameraReset(Camera *camera);
void cameraSetLightColor0(Camera *camera, VECTOR *color);
void cameraSetLightColor1(Camera *camera, VECTOR *color);
void cameraSetLightColor2(Camera *camera, VECTOR *color);
void cameraSetBackColor(Camera *camera, VECTOR *color);
void cameraSetLightDirection0(Camera *camera, VECTOR *direction);
void cameraSetLightDirection1(Camera *camera, VECTOR *direction);
void cameraSetLightDirection2(Camera *camera, VECTOR *direction);
void cameraApplyLights(Camera *camera);
Lights *cameraGetLights(Camera *camera);
void cameraSetView(Camera *camera, MATRIX *view);
void planeSetPoint(Plane *plane, VECTOR *point);
void setModelMatrixAndLights(MATRIX *m);
void setModelMatrix(MATRIX *m);
s32 cameraIsSphereVisible(Camera *camera, VECTOR *center, s32 radius);
s32 planeIsSphereInside(Plane *plane, VECTOR *center, s32 radius);
s32 planeGetDistance(Plane *plane, VECTOR *point);
s32 cameraClipLine(Camera *camera, SVECTOR *start, SVECTOR *end);
s32 getFrustumOutcode(Plane *planes, SVECTOR *point);
void clipPointToFrustum(Plane *planes, SVECTOR *point, SVECTOR *other, u32 outcode);
s32 planeGetDistanceFixed(Plane *plane, SVECTOR *point);
void cameraClearOt(Camera *camera);
void cameraSetScreen(Camera *camera, s16 width, s16 height, s16 aspectX, s16 aspectY);
void cameraSetFov(Camera *camera, s16 fov);
void cameraSetNear(Camera *camera, s32 near);
void cameraSetFar(Camera *camera, s32 far);
s16 cameraGetProjection(Camera *camera);
s16 cameraGetFov(Camera *camera);
LightsOverride *lightsOverrideInit(LightsOverride *lightsOverride, Camera *camera, Lights *lights);
void lightsOverrideDestroy(LightsOverride *lightsOverride, s32 flags);

EXTERN_C_END

#endif /* DTBE_GFX_CAMERA_H */
