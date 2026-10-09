#ifndef DTBE_GFX_TMD_BOUNDS_H
#define DTBE_GFX_TMD_BOUNDS_H

/* Bounding boxes, grown by the vertices of each type of TMD primitive. */

#include "common.h"
#include <libgte.h>
#include <libgs.h>

EXTERN_C_BEGIN

/* an axis-aligned bounding box */
typedef struct {
    /* 0x0 */ s16 min[3];
    /* 0x6 */ s16 max[3];
} Bounds;

/* The functions of TMD_GROW_BOUNDS_FUNCS, one per type of TMD primitive. */
void growBoundsF3(TMD_P_F3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsG3(TMD_P_G3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNF3(TMD_P_NF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNG3(TMD_P_NG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsF4(TMD_P_F4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsG4(TMD_P_G4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNF4(TMD_P_NF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNG4(TMD_P_NG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTF3(TMD_P_TF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTG3(TMD_P_TG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNF3(TMD_P_TNF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNG3(TMD_P_TNG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTF4(TMD_P_TF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTG4(TMD_P_TG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNF4(TMD_P_TNF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNG4(TMD_P_TNG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void boundsGrowByVertex(Bounds *bounds, SVECTOR *vertex);

EXTERN_C_END

#endif /* DTBE_GFX_TMD_BOUNDS_H */
