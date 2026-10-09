#ifndef DTBE_GAME_LOADING_SCREEN_H
#define DTBE_GAME_LOADING_SCREEN_H

/* The loading screen: the picture shown while a scene loads, centered on the screen. */

#include "common.h"
#include <sys/types.h>
#include <libgpu.h>
#include "vtable.h"

EXTERN_C_BEGIN

/* Two 16-bit and two 8-bit fields packed low first */
#define PACK16(low, high) (((u16)(high) << 16) | (u16)(low))
#define PACK8(low, high) (((u8)(high) << 8) | (u8)(low))

/* the virtual table of LoadingScreen */
typedef struct LoadingScreenVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry load; /* (LoadingScreen *) */
    /* 0x10 */ VtableEntry draw; /* (LoadingScreen *) */
} LoadingScreenVtable;

/* A SPRT written a word at a time, its color and code too. */
typedef struct SpriteWords {
    /* 0x00 */ u_long tag;
    /* 0x04 */ u32 rgbCode; /* the color, then the primitive's code */
    /* 0x08 */ u32 xy;      /* y in the high half */
    /* 0x0C */ u32 uvClut;  /* v in the second byte, the clut in the high half */
    /* 0x10 */ u32 wh;      /* h in the high half */
} SpriteWords;

/* the picture shown while a scene loads */
typedef struct LoadingScreen {
    /* 0x00 */ RECT rect; /* where it is drawn, and its size */
    /* 0x08 */ u32 tpage;
    /* 0x0C */ s32 first; /* not drawn yet */
    /* 0x10 */ LoadingScreenVtable *vtable;
} LoadingScreen;

extern struct LoadingScreenVtable LOADING_SCREEN_VTABLE; /* of LoadingScreen */

LoadingScreen *loadingScreenInit(LoadingScreen *loadingScreen);
void loadingScreenUpdate(LoadingScreen *loadingScreen);
void loadingScreenLoad(LoadingScreen *loadingScreen);
void loadingScreenLoadTim(LoadingScreen *loadingScreen, char *path);
void loadingScreenDraw(LoadingScreen *loadingScreen);

EXTERN_C_END

#endif /* DTBE_GAME_LOADING_SCREEN_H */
