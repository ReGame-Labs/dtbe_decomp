#include "common.h"
#include "engine/gfx/prim/alloc_line_f4.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a LINE_F4 from the frame's primitive buffer. */
LINE_F4 *allocLineF4(void) {
    LINE_F4 *line = (LINE_F4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(LINE_F4);
    setLineF4(line);
    return line;
}

/* Takes a POLY_G3 from the frame's primitive buffer. */
POLY_G3 *allocPolyG3(void) {
    POLY_G3 *poly = (POLY_G3 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_G3);
    setPolyG3(poly);
    return poly;
}
