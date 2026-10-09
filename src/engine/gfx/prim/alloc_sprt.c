#include "common.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim_buffer.h"
#include "libgpu.h"

/* Takes a sprite primitive from the frame's primitive buffer. */
SPRT *allocSprt(void) {
    SPRT *sprt = (SPRT *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(SPRT);
    setSprt(sprt);
    return sprt;
}

/* Takes a tile primitive from the frame's primitive buffer. */
TILE *allocTile(void) {
    TILE *tile = (TILE *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(TILE);
    setTile(tile);
    return tile;
}

/* Takes a texture page primitive from the frame's primitive buffer, with the drawing flags. */
DR_TPAGE *allocDrTpage(s32 tpage) {
    DR_TPAGE *prim = (DR_TPAGE *)PRIM_BUFFER_FREE;
    u_long *code;

    PRIM_BUFFER_FREE += sizeof(DR_TPAGE);
    setlen(prim, 1);
    code = prim->code;
    *code = _get_mode(DRAW_ON_DISPLAY, DRAW_DITHER, tpage);
    return prim;
}
