#include "common.h"
#include "engine/game/loading_screen.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/box.h"
#include "engine/gfx/display.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/system/memory.h"
#include "libgpu.h"
#include "psyq.h"

/* Builds the loading screen. */
LoadingScreen *loadingScreenInit(LoadingScreen *this) {
    this->vtable = &LOADING_SCREEN_VTABLE;
    this->first = 1;
    return this;
}

/* Draws the loading screen, loading its picture the first time. */
void loadingScreenUpdate(LoadingScreen *this) {
    if (this->first) {
        this->first = 0;
        this->vtable->load.func((u8 *)this + this->vtable->load.delta);
    }
    this->vtable->draw.func((u8 *)this + this->vtable->draw.delta);
}

/* Loads the picture of the loading screen. */
void loadingScreenLoad(LoadingScreen *this) {
    loadingScreenLoadTim(this, "system/now_load.tim");
}

/* Loads the TIM at path into VRAM and centers its picture on the screen. */
void loadingScreenLoadTim(LoadingScreen *this, char *path) {
    TIM_IMAGE image;
    u32 *tim = loadFile(path);
    s32 mode;

    uploadTims(tim);
    func_800585B0(tim);
    ReadTIM(&image);
    mode = image.mode & 3;
    this->tpage = getTPage(mode, 0, 640, 256);
    this->rect = *image.prect;
    /* from VRAM halfwords to pixels: 4 of them in a halfword at 4 bits */
    this->rect.w <<= 2 - mode;
    this->rect.x = (SCREEN_WIDTH - this->rect.w) >> 1;
    this->rect.y = (SCREEN_HEIGHT - this->rect.h) >> 1;
    mainHeapFree(tim);
}

/* Draws the loading screen: a caption from a fixed place in VRAM, added
 * then subtracted, and the picture. The code bits set with the neutral
 * color are 2 for semi-transparent and 1 for no color modulation. */
void loadingScreenDraw(LoadingScreen *this) {
    u_long *ot = FRAME_OT.lastDrawn - 1;
    SpriteWords *sprite = (SpriteWords *)allocSprt();
    SpriteWords *copy;
    DR_TPAGE *tpage = allocDrTpage(getTPage(0, 1, 896, 0));
    /* to copy the position and the size a word at a time */
    RectWords *rect = (RectWords *)&this->rect;

    sprite->rgbCode = (((SPRT *)sprite)->code << 24) | 0x02808080;
    sprite->xy = PACK16(208, 200);
    sprite->uvClut = PACK16(PACK8(160, 0), getClut(944, 123));
    sprite->wh = PACK16(96, 32);
    AddPrim(ot, sprite);
    AddPrim(ot, tpage);

    copy = (SpriteWords *)allocSprt();
    tpage = allocDrTpage(getTPage(0, 2, 896, 0));
    copy->rgbCode = (((SPRT *)copy)->code << 24) | 0x02808080;
    copy->xy = sprite->xy;
    copy->uvClut = PACK16(PACK8(160, 32), getClut(944, 124));
    copy->wh = sprite->wh;
    AddPrim(ot, copy);
    AddPrim(ot, tpage);

    sprite = (SpriteWords *)allocSprt();
    tpage = allocDrTpage(this->tpage);
    sprite->rgbCode = (((SPRT *)sprite)->code << 24) | 0x01808080;
    sprite->xy = rect->xy;
    sprite->uvClut = PACK16(PACK8(0, 0), getClut(0, 511));
    sprite->wh = rect->wh;
    AddPrim(ot, sprite);
    AddPrim(ot, tpage);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/loading_screen", D_800103CC);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/loading_screen", LOADING_SCREEN_VTABLE);
