#include "common.h"
#include "engine/gfx/prim/alloc_dr_load.h"
#include "engine/gfx/prim_buffer.h"
#include "psyq.h"

/* Takes a VRAM load primitive from the frame's primitive buffer, set to load
 * rect. */
DR_LOAD *allocDrLoad(RECT *rect) {
    /* the buffer holds primitives of every type */
    DR_LOAD *load = (DR_LOAD *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(DR_LOAD);
    func_80058540(load, rect);
    return load;
}
