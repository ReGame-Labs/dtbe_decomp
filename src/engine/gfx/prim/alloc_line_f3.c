#include "common.h"
#include "engine/gfx/prim/alloc_line_f3.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a LINE_F3 from the frame's primitive buffer. */
LINE_F3 *allocLineF3(void) {
    LINE_F3 *line = (LINE_F3 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(LINE_F3);
    setLineF3(line);
    return line;
}
