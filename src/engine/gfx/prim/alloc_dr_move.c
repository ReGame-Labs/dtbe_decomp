#include "common.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a DR_MOVE from the frame's primitive buffer. */
DR_MOVE *allocDrMove(RECT *rect, s32 x, s32 y) {
    DR_MOVE *prim = (DR_MOVE *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(DR_MOVE);
    SetDrawMove(prim, rect, x, y);
    return prim;
}
