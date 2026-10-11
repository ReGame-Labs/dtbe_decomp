#include "common.h"
#include "engine/gfx/prim/alloc_poly_f4.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a POLY_F4 from the frame's primitive buffer. */
POLY_F4 *allocPolyF4(void) {
    POLY_F4 *poly = (POLY_F4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_F4);
    setPolyF4(poly);
    return poly;
}
