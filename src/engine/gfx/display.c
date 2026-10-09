#include "common.h"
#include "engine/gfx/display.h"
#include "engine/gfx/prim_buffer.h"
#include "libetc.h"
#include "libgpu.h"
#include "kernel.h"
#include "psyq.h"

/* the display, set when it is built */
Display *CURRENT_DISPLAY = NULL;
/* how many times the drawing ended (countDrawEnd) */
static s32 DRAW_END_COUNT = 0;

/* Builds the display, of width by height, waiting vsyncMode vertical blanks
 * a frame. */
Display *displayInit(Display *this, s32 width, s32 height, s32 vsyncMode) {
    CURRENT_DISPLAY = this;
    this->buffer = 0;
    this->dispOf[0] = &this->disp[0];
    this->dispOf[1] = &this->disp[1];
    this->drawOf[0] = &this->draw[0];
    this->drawOf[1] = &this->draw[1];
    displaySetSize(this, width, height);
    displaySetVsyncMode(this, vsyncMode);
    this->skipFrames = vsyncMode;
    displaySetTvPosition(this, 0, 0);
    displaySetTvSize(this, 256, 240);
    displaySetClearColor(this, 0, 0, 0);
    displayDisable(this, DISPLAY_CLEAR);
    displayDisable(this, DISPLAY_INTERLACE);
    displayDisable(this, DISPLAY_RGB24);
    displayEnable(this, DISPLAY_DITHER);
    displayEnable(this, DISPLAY_UNK4);
    this->ot = NULL;
    this->override = NULL;
    return this;
}

/* Starts the GPU with a black screen and installs the interrupt callbacks. */
void displayStart(Display *this) {
    ResetGraph(0);
    VSync(2);
    SetDispMask(0);
    ClearImage2(&VRAM_CLEAR_RECTS[0], 0, 0, 0);
    ClearImage2(&VRAM_CLEAR_RECTS[1], 0, 0, 0);
    DrawSync(0);
    VSync(2);
    SetDispMask(1);
    VSync(0);
    this->field = GetODE() == 0;
    VSyncCallback(flipDisplay);
    DrawSyncCallback(countDrawEnd);
    this->redraw = 1;
}

/* Sets how many vertical blanks a frame waits for; 0 below 2. */
void displaySetVsyncMode(Display *this, s32 vsyncMode) {
    if (vsyncMode < 2) {
        vsyncMode = 0;
    }
    this->vsyncMode = vsyncMode;
}

/* Sets the color the buffers are cleared to. */
void displaySetClearColor(Display *this, u8 r, u8 g, u8 b) {
    this->draw[0].r0 = r;
    this->draw[0].g0 = g;
    this->draw[0].b0 = b;
    this->draw[1].r0 = r;
    this->draw[1].g0 = g;
    this->draw[1].b0 = b;
}

/* Has ot drawn at the next vertical blank. */
void displaySetOt(Display *this, u_long *ot) {
    this->ot = ot;
}

/* The vertical blank callback: shows the buffer drawn and draws the next
 * ordering table into the other one. */
void flipDisplay(void) {
    Display *display = CURRENT_DISPLAY;
    s32 buffer;
    u_long *ot;
    s32 unsynced;
    DISPENV *disp;
    DRAWENV *draw;
    s32 field;
    s32 tries;
    s32 ode;

    if (display->override != NULL) {
        PutDispEnv(display->override);
        return;
    }
    buffer = display->buffer;
    ot = display->ot;
    /* displayWaitFrame does not wait for the drawing then */
    unsynced = 0;
    if (display->interlaced || display->unk10C) {
        unsynced = 1;
    }
    disp = display->dispOf[buffer];
    draw = display->drawOf[buffer];
    if (ot != NULL && unsynced) {
        ResetGraph(1);
    }
    if (!display->interlaced) {
        display->field ^= 1;
        if (display->skipFrames) {
            display->skipFrames--;
            return;
        }
    } else {
        /* waits for the field to change */
        field = display->field;
        tries = 0x1000;
        while (--tries != -1) {
            ode = GetODE();
            if (field != ode) {
                display->field = ode;
                break;
            }
        }
    }
    if (ot != NULL && (unsynced || display->redraw)) {
        DrawOTagEnv(ot, draw);
        PutDispEnv(disp);
        display->redraw = 0;
        display->buffer = buffer ^ 1;
        display->ot = NULL;
    }
}

/* The drawing end callback: counts the drawings. */
void countDrawEnd(void) {
    DRAW_END_COUNT++;
}

/* How many times the drawing ended. */
s32 displayGetDrawEndCount(Display *this) {
    return DRAW_END_COUNT;
}

/* Waits for the end of the frame. Returns VSync's time. */
s32 displayWaitFrame(Display *this) {
    if (!this->interlaced && !this->unk10C) {
        DrawSync(0);
        this->redraw = 1;
    }
    this->vsyncTime = VSync(this->vsyncMode);
    this->skipFrames = this->vsyncMode > 0 ? this->vsyncMode - 1 : 0;
    return this->vsyncTime;
}

/* Sets the size of the screen; 256 lines or more is interlaced. */
void displaySetSize(Display *this, s32 width, s32 height) {
    s32 dfe;

    if (height >= 256) {
        dfe = 0;
        setRECT(&this->disp[0].disp, 0, 0, width, height);
        setRECT(&this->disp[1].disp, 0, 0, width, height);
        this->interlaced = 1;
        this->disp[0].isinter = 1;
        this->disp[1].isinter = 1;
    } else {
        setRECT(&this->disp[0].disp, 0, 0, width, height);
        setRECT(&this->disp[1].disp, 0, height, width, height);
        this->interlaced = 0;
        if (this->unk110) {
            this->disp[0].isinter = 1;
            this->disp[1].isinter = 1;
        } else {
            this->disp[0].isinter = 0;
            this->disp[1].isinter = 0;
        }
        dfe = 1;
    }
    DRAW_ON_DISPLAY = dfe;
    this->draw[0].clip = this->disp[1].disp;
    this->draw[0].dfe = dfe;
    setRECT(&this->draw[0].tw, 0, 0, 256, 256);
    this->draw[0].tpage = 0;
    this->draw[0].ofs[0] = this->disp[1].disp.x;
    this->draw[0].ofs[1] = this->disp[1].disp.y;
    this->draw[1].clip = this->disp[0].disp;
    this->draw[1].dfe = dfe;
    setRECT(&this->draw[1].tw, 0, 0, 256, 256);
    this->draw[1].tpage = 0;
    this->draw[1].ofs[0] = this->disp[0].disp.x;
    this->draw[1].ofs[1] = this->disp[0].disp.y;
}

/* Gives the size of the screen. */
void displayGetSize(Display *this, s16 *size) {
    size[0] = this->disp[0].disp.w;
    size[1] = this->disp[0].disp.h;
}

/* Sets where the screen shows on the TV. */
void displaySetTvPosition(Display *this, s32 x, s32 y) {
    this->disp[0].screen.x = x;
    this->disp[0].screen.y = y;
    this->disp[1].screen.x = x;
    this->disp[1].screen.y = y;
}

/* Sets the size of the screen on the TV. */
void displaySetTvSize(Display *this, s32 width, s32 height) {
    this->disp[0].screen.w = width;
    this->disp[0].screen.h = height;
    this->disp[1].screen.w = width;
    this->disp[1].screen.h = height;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/display", VRAM_CLEAR_RECTS);

/* Shows env from both buffers, or each buffer its own environment when env is
 * NULL. */
void displaySetDispEnv(Display *this, DISPENV *env) {
    if (env == NULL) {
        this->dispOf[0] = &this->disp[0];
        this->dispOf[1] = &this->disp[1];
        return;
    }
    this->dispOf[0] = env;
    this->dispOf[1] = env;
}

/* Shows env instead of the buffers, at the same place on the TV; NULL shows
 * the buffers again. */
void displaySetOverride(Display *this, DISPENV *env) {
    this->override = env;
    if (env != NULL) {
        env->screen = this->disp[0].screen;
    }
}

/* Turns a setting (DISPLAY_*) of both buffers on. */
void displayEnable(Display *this, s32 setting) {
    switch (setting) {
    case DISPLAY_CLEAR:
        this->draw[0].isbg = 1;
        this->draw[1].isbg = 1;
        break;
    case DISPLAY_INTERLACE:
        this->unk110 = 1;
        this->disp[0].isinter = 1;
        this->disp[1].isinter = 1;
        break;
    case DISPLAY_RGB24:
        this->disp[0].isrgb24 = 1;
        this->disp[1].isrgb24 = 1;
        break;
    case DISPLAY_DITHER:
        this->draw[0].dtd = 1;
        this->draw[1].dtd = 1;
        DRAW_DITHER = 1;
        break;
    case DISPLAY_UNK4:
        this->unk10C = 1;
        break;
    }
}

/* Turns a setting (DISPLAY_*) of both buffers off; interlacing goes back to
 * what interlaced says. */
void displayDisable(Display *this, s32 setting) {
    switch (setting) {
    case DISPLAY_CLEAR:
        this->draw[0].isbg = 0;
        this->draw[1].isbg = 0;
        break;
    case DISPLAY_INTERLACE:
        this->unk110 = 0;
        if (this->interlaced) {
            this->disp[0].isinter = 1;
            this->disp[1].isinter = 1;
        } else {
            this->disp[0].isinter = 0;
            this->disp[1].isinter = 0;
        }
        break;
    case DISPLAY_RGB24:
        this->disp[0].isrgb24 = 0;
        this->disp[1].isrgb24 = 0;
        break;
    case DISPLAY_DITHER:
        this->draw[0].dtd = 0;
        this->draw[1].dtd = 0;
        DRAW_DITHER = 0;
        break;
    case DISPLAY_UNK4:
        this->unk10C = 0;
        break;
    }
}

/* Clears both buffers to black with the display turned off. */
void displayClear(Display *this) {
    VSync(0);
    SetDispMask(0);
    ClearImage2(&this->disp[0].disp, 0, 0, 0);
    ClearImage2(&this->disp[1].disp, 0, 0, 0);
    VSync(0);
    SetDispMask(1);
}

/* Gives rect the part of the drawing buffer at x, y of the given size, cut to
 * fit inside it. */
void getDrawBufferSubRect(RECT *rect, s32 x, s32 y, s32 width, s32 height) {
    s32 bufferWidth;
    s32 bufferHeight;

    getDrawBufferRect(rect);
    bufferWidth = rect->w;
    bufferHeight = rect->h;
    if (x < 0) {
        x = 0;
    } else if (x > bufferWidth - 1) {
        x = bufferWidth - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y > bufferHeight - 1) {
        y = bufferHeight - 1;
    }
    if (x + width > bufferWidth) {
        width = bufferWidth - x;
    }
    if (y + height > bufferHeight) {
        height = bufferHeight - y;
    }
    rect->w = width;
    rect->h = height;
    rect->x += x;
    rect->y += y;
}

/* Gives rect the area of the buffer being drawn, the one not shown. */
void getDrawBufferRect(RECT *rect) {
    *rect = (CURRENT_DISPLAY->disp + (CURRENT_DISPLAY->buffer ^ 1))->disp;
}

/*
 * These functions left in asm copy a MATRIX as a block move (four loads then
 * four stores), which GCC 2.95.2 does not emit for 32 bytes (see "Block
 * moves" in TODO.md). They are
 * transformAttach, transformDetach, meshDraw, meshCull, meshPartUpdate,
 * transformGetWorld, transformSetLocal and func_8002099C.
 */

/* The area of the buffer shown. */
RECT *getShownBufferRect(void) {
    return &CURRENT_DISPLAY->disp[CURRENT_DISPLAY->buffer].disp;
}

/* Copies the buffer shown to dest; rect gets its area. */
void displayCopyShownBuffer(Display *this, RECT *rect, u_long *dest) {
    *rect = (this->disp + this->buffer)->disp;
    func_80058084(rect, dest);
}

/* The display. */
Display *getDisplay(void) {
    return CURRENT_DISPLAY;
}
