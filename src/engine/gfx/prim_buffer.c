#include "common.h"
#include "engine/gfx/prim_buffer.h"
#include "libpad.h"
#include "memory.h"

/* whether drawing may touch the displayed area (DRAWENV.dfe) and dithers
 * (DRAWENV.dtd) */
s32 DRAW_ON_DISPLAY = FALSE;
s32 DRAW_DITHER = FALSE;
/* the free end of the frame's primitive buffer */
u8 *PRIM_BUFFER_FREE = NULL;
/* which of the two primitive buffers the frame fills */
static s32 PRIM_BUFFER_INDEX = 0;
/* the end of the frame's primitive buffer */
static u8 *PRIM_BUFFER_END = NULL;
/* where each primitive buffer starts */
static u8 *PRIM_BUFFER_STARTS[2] = { NULL, NULL };
/* where each primitive buffer ends */
static u8 *PRIM_BUFFER_ENDS[2] = { NULL, NULL };

/* Splits memory into the two primitive buffers and starts filling the first. */
void initPrimBuffers(u8 *base, u32 size) {
    u32 half = (size & ~3) / 2;

    PRIM_BUFFER_STARTS[0] = base;
    PRIM_BUFFER_FREE = base;
    base += half;
    PRIM_BUFFER_STARTS[1] = base;
    PRIM_BUFFER_ENDS[0] = base;
    PRIM_BUFFER_END = base;
    base += half;
    PRIM_BUFFER_INDEX = 0;
    PRIM_BUFFER_ENDS[1] = base;
}

/*
 * Starts filling the other primitive buffer. Returns how much of the one just
 * filled was used, in hundredths of a percent.
 */
s32 swapPrimBuffers(void) {
    s32 index = PRIM_BUFFER_INDEX;
    u8 *start = PRIM_BUFFER_STARTS[index];
    s32 used = PRIM_BUFFER_FREE - start;
    s32 size = PRIM_BUFFER_END - start;

    index ^= 1;
    PRIM_BUFFER_INDEX = index;
    PRIM_BUFFER_FREE = PRIM_BUFFER_STARTS[index];
    PRIM_BUFFER_END = PRIM_BUFFER_ENDS[index];
    return used * 10000 / size;
}

/* Returns -1 if the frame's primitive buffer overflowed, else 0. */
s32 checkPrimBufferOverflow(void) {
    return PRIM_BUFFER_FREE > PRIM_BUFFER_END ? -1 : 0;
}

/* Returns whether less than 4 KiB of the frame's primitive buffer is free. */
s32 isPrimBufferNearlyFull(void) {
    return PRIM_BUFFER_END - PRIM_BUFFER_FREE < 0x1000;
}

/* Returns where the first primitive buffer starts. */
u8 *getPrimBufferBase(void) {
    return PRIM_BUFFER_STARTS[0];
}

/* Takes size bytes from the frame's primitive buffer; returns where they start. */
u8 *allocPrimBytes(s32 size) {
    u8 *start = PRIM_BUFFER_FREE;

    PRIM_BUFFER_FREE = start + size;
    return start;
}

/* Sets whether drawing may touch the displayed area. */
void setDrawOnDisplay(s32 on) {
    DRAW_ON_DISPLAY = on == TRUE;
}

/* Sets whether drawing dithers. */
void setDrawDither(s32 on) {
    DRAW_DITHER = on == TRUE;
}
