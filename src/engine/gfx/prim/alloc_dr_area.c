#include "common.h"
#include "engine/gfx/prim/alloc_dr_area.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a DR_AREA from the frame's primitive buffer. */
DR_AREA *allocDrArea(RECT *area) {
    DR_AREA *prim = (DR_AREA *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(DR_AREA);
    SetDrawArea(prim, area);
    return prim;
}
