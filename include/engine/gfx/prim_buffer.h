#ifndef DTBE_GFX_PRIM_BUFFER_H
#define DTBE_GFX_PRIM_BUFFER_H

/* The two primitive buffers the frames fill in turn, and the dither and dfe settings. */

#include "common.h"

EXTERN_C_BEGIN

/* the free end of the frame's primitive buffer */
extern u8 *PRIM_BUFFER_FREE;
/* whether drawing may touch the displayed area (DRAWENV.dfe) and dithers
 * (DRAWENV.dtd) */
extern s32 DRAW_ON_DISPLAY;
extern s32 DRAW_DITHER;

void initPrimBuffers(u8 *base, u32 size);
s32 swapPrimBuffers(void);
s32 checkPrimBufferOverflow(void);
s32 isPrimBufferNearlyFull(void);
u8 *getPrimBufferBase(void);
u8 *allocPrimBytes(s32 size);
void setDrawOnDisplay(s32 on);
void setDrawDither(s32 on);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_BUFFER_H */
