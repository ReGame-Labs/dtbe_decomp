#include "common.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a POLY_FT4 from the frame's primitive buffer. */
POLY_FT4 *allocPolyFT4(void) {
    POLY_FT4 *poly = (POLY_FT4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_FT4);
    setPolyFT4(poly);
    return poly;
}
