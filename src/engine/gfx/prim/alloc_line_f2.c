#include "common.h"
#include "engine/gfx/prim/alloc_line_f2.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a LINE_F2 from the frame's primitive buffer. */
LINE_F2 *allocLineF2(void) {
    LINE_F2 *line = (LINE_F2 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(LINE_F2);
    setLineF2(line);
    return line;
}
