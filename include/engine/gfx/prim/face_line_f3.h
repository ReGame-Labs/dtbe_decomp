#ifndef DTBE_GFX_PRIM_FACE_LINE_F3_H
#define DTBE_GFX_PRIM_FACE_LINE_F3_H

/* Projecting a face into a LINE_F3, POLY_G4, POLY_FT3 or POLY_G3 in the ordering table. */

#include "common.h"

EXTERN_C_BEGIN

void *buildLineF3(void *arg0, void *arg1, s32 arg2, s32 arg3);
void *buildPolyG4(void *arg0, void *arg1, s32 arg2, s32 arg3);

void *buildPolyFT3(void *arg0, void *arg1, s32 arg2, s32 arg3);
void *buildPolyG3(void *arg0, void *arg1, s32 arg2, s32 arg3);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_FACE_LINE_F3_H */
