#include "common.h"
#include "engine/gfx/prim/alloc_sprt_16.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a SPRT_16 from the frame's primitive buffer. */
SPRT_16 *allocSprt16(void) {
    SPRT_16 *sprt = (SPRT_16 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(SPRT_16);
    setSprt16(sprt);
    return sprt;
}
