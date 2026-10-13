/* The battle's loading screen (system/load%02d.tim, system/bonus%02d.tim), a class derived from the
 * executable's loading screen. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/alloc_sprt.h"

extern u_long *D_8005F844; /* an ordering table: func_800D0204 draws in the slot before it */

INCLUDE_ASM("game/nonmatchings/battle_loading", func_800D00C8);

/* The loading screen of vtable D_8006DF3C, with one more sprite. */
typedef struct Unk800D0204 {
    /* 0x00 */ LoadingScreen base;
    /* 0x14 */ s32 unk14; /* draws the sprite of func_800D0204 when not 0 */
} Unk800D0204;

/* A member function (D_8006DF3C's draw): draws a 56x32 sprite at (8, 16) in front of the loading screen when
 * unk14 is set, then the loading screen. */
void func_800D0204(Unk800D0204 *this) {
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

/* Constructs an object of the class of vtable D_8006DF3C, which derives from loadingScreenInit's. */
LoadingScreen *func_800D0294(LoadingScreen *this) {
    loadingScreenInit(this);
    this->vtable = &D_8006DF3C;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/battle_loading", D_8006DF3C);
