#ifndef DTBE_GFX_SPRITE_H
#define DTBE_GFX_SPRITE_H

/* Sprites: the textures of TIM images, drawn in 64x256 pieces of one SPRT each. */

#include "common.h"
#include <libgpu.h>

EXTERN_C_BEGIN

/* A texture split into 64x256 pieces drawn as one SPRT each. */
typedef struct {
    s32 count; /* number of pieces */
    s32 unk4;
    RECT *pieces;
    u32 color;
    s8 mode; /* TIM pixel mode */
    u8 flags;
    s16 x;
    s16 y;
    u16 clut;
} Sprite;

/* A SPRT primitive written a word at a time. */
typedef struct {
    u32 tag;
    u32 color; /* r, g, b, code */
    u32 xy;
    u16 uv;
    u16 clut;
    u32 wh;
} SpritePrim;

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
s32 rectCountCells(RECT *rect, s32 wshift, s32 hshift);
RECT *rectSplitCells(RECT *out, RECT *rect, s32 wshift, s32 hshift);

void spriteSetSemiTrans(Sprite *sprite, s32 abr);
void spriteResetColor(Sprite *sprite);
void spriteSetColor(Sprite *sprite, u8 r, u8 g, u8 b);
void spriteSetGray(Sprite *sprite, u8 gray);
void spriteSetFlags(Sprite *sprite, s32 flags);
void spriteClearFlags(Sprite *sprite, s32 flags);

EXTERN_C_END

#endif /* DTBE_GFX_SPRITE_H */
