#include "common.h"
#include "engine/gfx/prim/alloc_line_g2.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a LINE_G2 from the frame's primitive buffer. */
LINE_G2 *allocLineG2(void) {
    LINE_G2 *line = (LINE_G2 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(LINE_G2);
    setLineG2(line);
    return line;
}

/* Takes a POLY_FT3 from the frame's primitive buffer. */
POLY_FT3 *allocPolyFT3(void) {
    POLY_FT3 *poly = (POLY_FT3 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_FT3);
    setPolyFT3(poly);
    return poly;
}
