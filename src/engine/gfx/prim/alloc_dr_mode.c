#include "common.h"
#include "engine/gfx/prim/alloc_dr_mode.h"
#include "engine/gfx/prim_buffer.h"

/* Takes a DR_MODE from the frame's primitive buffer, with the frame's dither
 * and display area settings. */
DR_MODE *allocDrMode(s32 tpage, RECT *tw) {
    DR_MODE *prim = (DR_MODE *)PRIM_BUFFER_FREE;
    u_long *mode;

    PRIM_BUFFER_FREE += sizeof(DR_MODE);
    setlen(prim, 2);
    /* the address is taken before the value is built */
    mode = &prim->code[0];
    *mode = _get_mode(DRAW_ON_DISPLAY, DRAW_DITHER, tpage);
    prim->code[1] = _get_tw(tw);
    return prim;
}
