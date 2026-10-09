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

/* Takes a POLY_F4 from the frame's primitive buffer. */
POLY_F4 *allocPolyF4(void) {
    POLY_F4 *poly = (POLY_F4 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(POLY_F4);
    setPolyF4(poly);
    return poly;
}

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

/* Takes a LINE_F2 from the frame's primitive buffer. */
LINE_F2 *allocLineF2(void) {
    LINE_F2 *line = (LINE_F2 *)PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE += sizeof(LINE_F2);
    setLineF2(line);
    return line;
}
