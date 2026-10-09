#ifndef DTBE_GFX_SPRITE_H
#define DTBE_GFX_SPRITE_H

/* Sprites: the textures of TIM images, drawn in 64x256 pieces of one SPRT each. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

/* A texture split into 64x256 pieces drawn as one SPRT each. */
typedef struct {
    /* 0x00 */ s32 count; /* number of pieces */
    /* 0x04 */ s32 unk4;
    /* 0x08 */ RECT *pieces;
    /* 0x0C */ u32 color;
    /* 0x10 */ s8 mode; /* TIM pixel mode */
    /* 0x11 */ u8 flags;
    /* 0x12 */ s16 x;
    /* 0x14 */ s16 y;
    /* 0x16 */ u16 clut;
} Sprite; /* size 0x18 */

/* A SPRT primitive written a word at a time. */
typedef struct {
    /* 0x00 */ u32 tag;
    /* 0x04 */ u32 color; /* r, g, b, code */
    /* 0x08 */ u32 xy;
    /* 0x0C */ u16 uv;
    /* 0x0E */ u16 clut;
    /* 0x10 */ u32 wh;
} SpritePrim; /* size 0x14 */

/* the color that draws a texture with its own colors (128 is 1.0) */
#define SPRITE_COLOR_NEUTRAL 0x808080

/* the most pieces a texture has: VRAM is 16 by 2 texture pages */
#define SPRITE_PIECE_MAX 32

/* flags */
#define SPRITE_ABR_MASK 3   /* semi-transparency rate */
#define SPRITE_RAW 4        /* no color modulation */
#define SPRITE_SEMI_TRANS 8
#define SPRITE_DRAW_DISPLAY 0x10 /* the dfe bit of the draw mode */
#define SPRITE_DITHER 0x20       /* the dtd bit of the draw mode */

Sprite *spriteInit(Sprite *sprite);
Sprite *spriteInitFromTim(Sprite *sprite, TIM_IMAGE *tim, s32 load);
void spriteSetTim(Sprite *sprite, TIM_IMAGE *tim, s32 load);
void spriteFreePieces(Sprite *sprite);
void spriteDestroy(Sprite *sprite, s32 flags);
void *spriteDraw(Sprite *sprite, u8 *prims, u_long *ot);
s32 rectCountCells(RECT *rect, s32 widthShift, s32 heightShift);
RECT *rectSplitCells(RECT *out, RECT *rect, s32 widthShift, s32 heightShift);

void spriteSetSemiTrans(Sprite *sprite, s32 abr);
void spriteResetColor(Sprite *sprite);
void spriteSetColor(Sprite *sprite, u8 r, u8 g, u8 b);
void spriteSetGrey(Sprite *sprite, u8 grey);
void spriteSetFlags(Sprite *sprite, s32 flags);
void spriteClearFlags(Sprite *sprite, s32 flags);

EXTERN_C_END

#endif /* DTBE_GFX_SPRITE_H */
