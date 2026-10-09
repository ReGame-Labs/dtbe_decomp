#include "common.h"
#include "engine/gfx/sprite.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/system/memory.h"
#include "psyq.h"
#include "vtable.h"

/* Builds a sprite with no texture, drawn with its texture's own colors. */
Sprite *spriteInit(Sprite *this) {
    this->pieces = NULL;
    this->x = 0;
    this->y = 0;
    this->color = SPRITE_COLOR_NEUTRAL;
    this->flags = 0;
    return this;
}

/* Builds a sprite of the texture of a TIM, loading it into VRAM first if
 * asked. */
Sprite *spriteInitFromTim(Sprite *this, TIM_IMAGE *tim, s32 load) {
    this->x = 0;
    this->y = 0;
    this->color = SPRITE_COLOR_NEUTRAL;
    this->flags = 0;
    this->pieces = NULL;
    spriteSetTim(this, tim, load);
    return this;
}

/* Takes the texture of a TIM, loading it into VRAM first if asked. */
void spriteSetTim(Sprite *this, TIM_IMAGE *tim, s32 load) {
    RECT pieces[SPRITE_PIECE_MAX];
    RECT *copy;
    s32 count;
    s32 i;

    spriteFreePieces(this);
    if (load) {
        if (tim->caddr != NULL) {
            func_80055B28(tim->crect, tim->caddr);
        }
        if (tim->paddr != NULL) {
            func_80055B28(tim->prect, tim->paddr);
        }
    }
    if (tim->crect != NULL) {
        this->clut = getClut(tim->crect->x, tim->crect->y);
    } else {
        this->clut = 0;
    }
    count = rectCountCells(tim->prect, TPAGE_WSHIFT, TPAGE_HSHIFT);
    this->count = count;
    rectSplitCells(pieces, tim->prect, TPAGE_WSHIFT, TPAGE_HSHIFT);
    this->mode = tim->mode & 3;
    copy = (RECT *)operatorVecNew(count * sizeof(RECT));
    this->pieces = copy;
    for (i = 0; i < count; i++) {
        copy[i] = pieces[i];
    }
}

/* Frees the pieces: the sprite has no texture any more. */
void spriteFreePieces(Sprite *this) {
    if (this->pieces != NULL) {
        operatorVecDelete(this->pieces);
    }
    this->pieces = NULL;
}

/* Destroys the sprite. */
void spriteDestroy(Sprite *this, s32 flags) {
    spriteFreePieces(this);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Builds the sprite and texture page primitives of a Sprite into prims, a
 * SPRT for each piece and then a DR_TPAGE for each, and adds them to ot;
 * returns the end of what it wrote. */
void *spriteDraw(Sprite *this, u8 *prims, u_long *ot) {
    SpritePrim *sprts;
    u32 color;

    if (this->pieces == NULL) {
        return prims;
    }
    sprts = (SpritePrim *)prims;
    prims += this->count * sizeof(SpritePrim);
    {
        /* the color and code all the SPRTs share: raw when asked or when the
         * color would not change the texture, semi-transparent when asked;
         * SPRITE_RAW and SPRITE_SEMI_TRANS shifted down are the code's
         * PRIM_RAW_TEXTURE and PRIM_SEMI_TRANS */
        SpritePrim *sprt = sprts;
        u32 left = this->count;

        color = this->color & 0xFFFFFF;
        if (color == SPRITE_COLOR_NEUTRAL) {
            color |= (SPRT_CODE | PRIM_RAW_TEXTURE | ((this->flags >> 2) & PRIM_SEMI_TRANS)) << 24;
        } else {
            color |= (SPRT_CODE | ((this->flags >> 2) & (PRIM_RAW_TEXTURE | PRIM_SEMI_TRANS))) << 24;
        }
        while (left--) {
            setlen(sprt, 4);
            sprt->color = color;
            sprt->clut = this->clut;
            sprt++;
        }
    }
    {
        /* where each piece goes, from the first piece's corner; a VRAM
         * pixel holds 4, 2 or 1 texels by mode, hence the shift */
        SpritePrim *sprt = sprts;
        u32 left = this->count;
        RECT *piece = this->pieces;
        s32 shift = 2 - this->mode;
        s16 x0 = piece->x;
        s16 y0 = piece->y;

        while (left--) {
            sprt->xy = ((u16)(this->y + (piece->y - y0)) << 16)
                     | (u16)(this->x + ((piece->x - x0) << shift));
            sprt->uv = ((u8)piece->y << 8) | (u8)((piece->x & TPAGE_WMASK) << shift);
            sprt->wh = ((u16)piece->h << 16) | (u16)(piece->w << shift);
            piece++;
            sprt++;
        }
    }
    {
        /* each piece's texture page (the 64-pixel column it lies in) ahead
         * of its SPRT; the mode word is worked out after the address of its
         * code word, as in allocDrTpage */
        SpritePrim *sprt = sprts;
        DR_TPAGE *tp = (DR_TPAGE *)prims;
        u32 left = this->count;
        RECT *piece = this->pieces;
        s32 tpage;
        u_long *code;
        u_long mode;

        prims += left * sizeof(DR_TPAGE);
        while (left--) {
            tpage = getTPage(this->mode, this->flags, piece->x & ~TPAGE_WMASK, piece->y);
            piece++;
            AddPrim(ot, sprt++);
            setlen(tp, 1);
            code = tp->code;
            mode = _get_mode(this->flags & SPRITE_DRAW_DISPLAY, this->flags & SPRITE_DITHER, tpage);
            *code = mode;
            AddPrim(ot, tp);
            tp++;
        }
    }
    return prims;
}

/* Number of (1 << widthShift) x (1 << heightShift) aligned cells that rect
 * touches. */
s32 rectCountCells(RECT *rect, s32 widthShift, s32 heightShift) {
    s32 cellWidth = 1 << widthShift;
    s32 cellHeight = 1 << heightShift;
    s32 cols = ((-cellWidth & (rect->x + rect->w + cellWidth - 1)) - (-cellWidth & rect->x)) >> widthShift;
    s32 rows = ((-cellHeight & (rect->y + rect->h + cellHeight - 1)) - (-cellHeight & rect->y)) >> heightShift;

    return cols * rows;
}

/* Splits rect at the (1 << widthShift) x (1 << heightShift) aligned cells it
 * touches, column by column, into out; returns the end of what it wrote. A
 * piece reaches the next cell boundary, or the rect's own edge in the last
 * column and row. */
RECT *rectSplitCells(RECT *out, RECT *rect, s32 widthShift, s32 heightShift) {
    s32 cellWidth = 1 << widthShift;
    s32 cellHeight = 1 << heightShift;
    s32 cols = ((-cellWidth & (rect->x + rect->w + cellWidth - 1)) - (-cellWidth & rect->x)) >> widthShift;
    s32 rows = ((-cellHeight & (rect->y + rect->h + cellHeight - 1)) - (-cellHeight & rect->y)) >> heightShift;
    s32 x = rect->x;
    s32 nextX;
    s32 col;

    for (col = cols - 1; col >= 0; col--, x = nextX) {
        s32 width;
        s32 y;
        s32 row;

        if (col == 0) {
            width = rect->x + rect->w - x;
        } else {
            width = (-cellWidth & (x + cellWidth)) - x;
        }
        y = rect->y;
        nextX = x + width;
        for (row = rows - 1; row >= 0; row--) {
            s32 height;

            if (row == 0) {
                height = rect->y + rect->h - y;
            } else {
                height = (-cellHeight & (y + cellHeight)) - y;
            }
            setRECT(out, x, y, width, height);
            y += height;
            out++;
        }
    }
    return out;
}

/* Turns on semi-transparency with the given rate. */
void spriteSetSemiTrans(Sprite *this, s32 abr) {
    this->flags = (this->flags & ~SPRITE_ABR_MASK) | SPRITE_SEMI_TRANS | abr;
}

/* Draws the texture with its own colors. */
void spriteResetColor(Sprite *this) {
    this->color = SPRITE_COLOR_NEUTRAL;
}

/* Sets the color the texture is drawn with. */
void spriteSetColor(Sprite *this, u8 r, u8 g, u8 b) {
    this->color = r | (g << 8) | (b << 16);
}

/* Sets a grey the texture is drawn with. */
void spriteSetGrey(Sprite *this, u8 grey) {
    this->color = grey | (grey << 8) | (grey << 16);
}

/* Turns flags on. */
void spriteSetFlags(Sprite *this, s32 flags) {
    this->flags |= flags;
}

/* Turns flags off. */
void spriteClearFlags(Sprite *this, s32 flags) {
    this->flags &= ~flags;
}
