#ifndef DTBE_GFX_PRIM_FACE_POLY_F4_H
#define DTBE_GFX_PRIM_FACE_POLY_F4_H

/* Projecting a face into a POLY_F4, or a TILE at its vertex, in the ordering table. */

#include "common.h"

EXTERN_C_BEGIN

void *buildPolyF4(void *arg0, void *arg1, s32 arg2, s32 arg3);
void *buildTile(void *arg0, void *arg1, s32 arg2, s32 arg3);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_FACE_POLY_F4_H */
