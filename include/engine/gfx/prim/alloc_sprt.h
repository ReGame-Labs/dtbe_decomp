#ifndef DTBE_GFX_PRIM_ALLOC_SPRT_H
#define DTBE_GFX_PRIM_ALLOC_SPRT_H

/* Taking a SPRT, a TILE or a DR_TPAGE from the frame's primitive buffer. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

/* the code setSprt gives a SPRT: a textured rectangle of any size */
#define SPRT_CODE 0x64

/* the bits of a primitive's code that setShadeTex and setSemiTrans set */
#define PRIM_RAW_TEXTURE 1 /* the texture's own colors, not modulated */
#define PRIM_SEMI_TRANS 2

/* the texture page whose semi-transparent primitives subtract their color
 * from the screen (abr 2) */
#define TPAGE_SUBTRACT getTPage(0, 2, 0, 0)

/* a texture page of VRAM is 1 << 6 by 1 << 8 pixels */
#define TPAGE_WSHIFT 6
#define TPAGE_HSHIFT 8
/* the bits of a VRAM x within its texture page */
#define TPAGE_WMASK ((1 << TPAGE_WSHIFT) - 1)

SPRT *allocSprt(void);
TILE *allocTile(void);
DR_TPAGE *allocDrTpage(s32 tpage);

EXTERN_C_END

#endif /* DTBE_GFX_PRIM_ALLOC_SPRT_H */
