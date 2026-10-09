#include "common.h"
#include "engine/gfx/prim/alloc_poly_f3.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a POLY_F3 from the frame's primitive buffer. */
POLY_F3 *allocPolyF3(void) {
    POLY_F3 *poly = (POLY_F3 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_F3);
    setPolyF3(poly);
    return poly;
}

/* Takes a POLY_G4 from the frame's primitive buffer. */
POLY_G4 *allocPolyG4(void) {
    POLY_G4 *poly = (POLY_G4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_G4);
    setPolyG4(poly);
    return poly;
}

/* Takes a POLY_GT4 from the frame's primitive buffer. */
POLY_GT4 *allocPolyGT4(void) {
    POLY_GT4 *poly = (POLY_GT4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_GT4);
    setPolyGT4(poly);
    return poly;
}
