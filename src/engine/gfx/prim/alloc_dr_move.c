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

/* Takes a SPRT_16 from the frame's primitive buffer. */
SPRT_16 *allocSprt16(void) {
    SPRT_16 *sprt = (SPRT_16 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(SPRT_16);
    setSprt16(sprt);
    return sprt;
}
