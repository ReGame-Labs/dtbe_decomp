/* The fight's loading screen (system/load%02d.tim, system/bonus%02d.tim), a class derived from the
 * executable's loading screen. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/alloc_sprt.h"

extern u_long *D_8005F844; /* an ordering table: FightLoadingScreen_draw draws in the slot before it */

INCLUDE_ASM("game/nonmatchings/battle_loading", FightLoadingScreen_load);

/* The fight's loading screen: the executable's, with one more sprite. */
typedef struct FightLoadingScreen {
    /* 0x00 */ LoadingScreen base;
    /* 0x14 */ s32 unk14; /* draws the sprite of FightLoadingScreen_draw when not 0 */
} FightLoadingScreen;

/* Draws a 56x32 sprite at (8, 16) in front of the loading screen when unk14 is set, then the loading screen. */
void FightLoadingScreen_draw(FightLoadingScreen *this) {
    SPRT *sprite;

    if (this->unk14 != 0) {
        sprite = allocSprt();
        /* The match depends on writing each field pair as one word through its address (not through
         * SpriteWords): the scheduler then keeps the code read ahead of the stores. */
        *(u32 *)&sprite->r0 = (sprite->code << 24) | 0x01808080; /* the color and the code */
        *(u32 *)&sprite->x0 = PACK16(8, 16);
        *(u32 *)&sprite->u0 = PACK16(PACK8(160, 64), getClut(944, 125)); /* u, v and the clut */
        *(u32 *)&sprite->w = PACK16(56, 32);
        AddPrim(D_8005F844 - 1, sprite);
    }
    loadingScreenDraw(&this->base);
}

/* Constructs the fight's loading screen on the executable's. */
LoadingScreen *FightLoadingScreen_init(LoadingScreen *this) {
    loadingScreenInit(this);
    this->vtable = &FIGHT_LOADING_SCREEN_VTABLE;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/battle_loading", FIGHT_LOADING_SCREEN_VTABLE);
