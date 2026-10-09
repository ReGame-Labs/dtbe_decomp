#ifndef DTBE_GFX_DISPLAY_H
#define DTBE_GFX_DISPLAY_H

/* The double buffered display: both buffers' environments and the vertical blank flip. */

#include "common.h"
#include <sys/types.h>
#include <libgpu.h>

EXTERN_C_BEGIN

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

/* The double buffered display: the display and drawing environments of both
 * buffers and what the vertical blank interrupt is to show next. */
typedef struct Display {
    /* 0x000 */ DISPENV disp[2];
    /* 0x028 */ DRAWENV draw[2];
    /* 0x0E0 */ DISPENV *dispOf[2]; /* the display environment of each buffer */
    /* 0x0E8 */ DRAWENV *drawOf[2];
    /* 0x0F0 */ DISPENV *override; /* shown instead of the buffers when set */
    /* 0x0F4 */ s32 buffer;
    /* 0x0F8 */ s32 vsyncMode;
    /* 0x0FC */ volatile s32 skipFrames; /* shared with the vertical blank callback */
    /* 0x100 */ s32 vsyncTime;
    /* 0x104 */ s32 field; /* the interlaced field being shown */
    /* 0x108 */ s32 interlaced;
    /* 0x10C */ s32 unk10C;
    /* 0x110 */ s32 unk110;
    /* 0x114 */ s32 redraw;
    /* 0x118 */ u_long *ot; /* the ordering table to draw at the next vertical blank */
} Display;

/* the settings of a Display that displayEnable turns on and displayDisable
 * turns off */
#define DISPLAY_CLEAR 0     /* clear the buffer before drawing (isbg) */
#define DISPLAY_INTERLACE 1 /* interlace even below 256 lines */
#define DISPLAY_RGB24 2     /* 24 bit color */
#define DISPLAY_DITHER 3
#define DISPLAY_UNK4 4      /* sets unk10C */

/* the display, built by initOrDestroyMainGlobals */
extern struct Display DISPLAY;

/* the areas of VRAM cleared at boot: both buffers */
extern RECT VRAM_CLEAR_RECTS[2];
/* the display, set when it is built */
extern Display *CURRENT_DISPLAY;

Display *displayInit(Display *display, s32 width, s32 height, s32 vsyncMode);
void displayStart(Display *display);
void displaySetVsyncMode(Display *display, s32 vsyncMode);
void displaySetClearColor(Display *display, u8 r, u8 g, u8 b);
void displaySetOt(Display *display, u_long *ot);
void flipDisplay(void);
void countDrawEnd(void);
s32 displayGetDrawEndCount(Display *display);
s32 displayWaitFrame(Display *display);
void displaySetSize(Display *display, s32 width, s32 height);
void displayGetSize(Display *display, s16 *size);
void displaySetTvPosition(Display *display, s32 x, s32 y);
void displaySetTvSize(Display *display, s32 width, s32 height);
void displaySetDispEnv(Display *display, DISPENV *env);
void displaySetOverride(Display *display, DISPENV *env);
void displayEnable(Display *display, s32 setting);
void displayDisable(Display *display, s32 setting);
void displayClear(Display *display);
void getDrawBufferSubRect(RECT *rect, s32 x, s32 y, s32 width, s32 height);
void getDrawBufferRect(RECT *rect);
RECT *getShownBufferRect(void);
void displayCopyShownBuffer(Display *display, RECT *rect, u_long *dest);
Display *getDisplay(void);

EXTERN_C_END

#endif /* DTBE_GFX_DISPLAY_H */
