#include "common.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/display.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/lerp.h"
#include "engine/math/shuffle.h"
#include "engine/task/task.h"
#include "gte.h"
#include "libgpu.h"

/* Builds the fade task, with the screen not faded. */
FadeControl *fadeControlInit(FadeControl *this) {
    taskInit(&this->task, 0xF000, "Fade Control");
    this->task.vtable = &FADE_CONTROL_VTABLE;
    lerpInit(&this->level, 0);
    return this;
}

/* Fades the screen to black over duration frames. */
void fadeControlFadeOut(FadeControl *this, s32 duration) {
    fadeControlFadeTo(this, duration, FADE_BLACK);
}

/* Fades the screen back in over duration frames. */
void fadeControlFadeIn(FadeControl *this, s32 duration) {
    fadeControlFadeTo(this, duration, 0);
}

/* Fades the screen to level over duration frames. */
void fadeControlFadeTo(FadeControl *this, s32 duration, s32 level) {
    lerpStart(&this->level, level, duration);
}

/* Whether the fade has reached its level. */
s32 fadeControlIsDone(FadeControl *this) {
    return lerpIsDone(&this->level);
}

/* Draws the fade: a black-to-white tile over the screen, subtracted. */
void fadeControlDraw(FadeControl *this) {
    s32 level = lerpUpdate(&this->level);

    if (level != 0) {
        TILE *tile;
        Display *display = CURRENT_DISPLAY;
        u_long *ot = FRAME_OT.tail;

        tile = allocTile();
        tile->r0 = level;
        tile->g0 = level;
        tile->b0 = level;
        *(u32 *)&tile->x0 = 0; /* x and y at once */
        displayGetSize(display, &tile->w);
        SetSemiTrans(tile, 1);
        AddPrim(ot, tile);
        AddPrim(ot, allocDrTpage(0x40));
    }
}

/* Destroys the fade task. */
void fadeControlDestroy(FadeControl *this, s32 flags) {
    taskDestroy(&this->task, flags);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/fade", FADE_CONTROL_VTABLE);
