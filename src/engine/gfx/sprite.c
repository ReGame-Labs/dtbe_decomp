#include "common.h"
#include "engine/gfx/sprite.h"
#include "engine/system/memory.h"
#include "psyq.h"

Sprite *spriteInit(Sprite *this) {
    this->pieces = NULL;
    this->x = 0;
    this->y = 0;
    this->color = 0x808080;
    this->flags = 0;
    return this;
}

Sprite *spriteInitFromTim(Sprite *this, TIM_IMAGE *tim, s32 load) {
    this->x = 0;
    this->y = 0;
    this->color = 0x808080;
    this->flags = 0;
    this->pieces = NULL;
    spriteSetTim(this, tim, load);
    return this;
}

/* Takes the texture of a TIM, loading it into VRAM first if asked. */
void spriteSetTim(Sprite *this, TIM_IMAGE *tim, s32 load) {
    RECT pieces[32];
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
    count = rectCountCells(tim->prect, 6, 8);
    this->count = count;
    rectSplitCells(pieces, tim->prect, 6, 8);
    this->mode = tim->mode & 3;
    copy = (RECT *)operatorVecNew(count * sizeof(RECT));
    this->pieces = copy;
    for (i = 0; i < count; i++) {
        copy[i] = pieces[i];
    }
}

void spriteFreePieces(Sprite *this) {
    if (this->pieces != NULL) {
        operatorVecDelete(this->pieces);
    }
    this->pieces = NULL;
}

void spriteDestroy(Sprite *this, s32 flags) {
    spriteFreePieces(this);
    if (flags & 1) {
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
        /* the color and code all the SPRTs share: textured (0x64), raw
         * (+1) when asked or when the color would not change the texture,
         * semi-transparent (+2) */
        SpritePrim *sprt = sprts;
        u32 n = this->count;

        color = this->color & 0xFFFFFF;
        if (color == 0x808080) {
            color |= (0x65 | ((this->flags >> 2) & 2)) << 24;
        } else {
            color |= (0x64 | ((this->flags >> 2) & 3)) << 24;
        }
        while (n--) {
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
        u32 n = this->count;
        RECT *piece = this->pieces;
        s32 shift = 2 - this->mode;
        s16 x0 = piece->x;
        s16 y0 = piece->y;

        while (n--) {
            sprt->xy = ((u16)(this->y + (piece->y - y0)) << 16)
                     | (u16)(this->x + ((piece->x - x0) << shift));
            sprt->uv = ((u8)piece->y << 8) | (u8)((piece->x & 0x3F) << shift);
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
        u32 n = this->count;
        RECT *piece = this->pieces;
        s32 tpage;
        u_long *code;
        u_long mode;

        prims += n * sizeof(DR_TPAGE);
        while (n--) {
            tpage = getTPage(this->mode, this->flags, piece->x & ~0x3F, piece->y);
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

/* Number of (1 << wshift) x (1 << hshift) aligned cells that rect touches. */
s32 rectCountCells(RECT *rect, s32 wshift, s32 hshift) {
    s32 cellW = 1 << wshift;
    s32 cellH = 1 << hshift;
    s32 cols = ((-cellW & (rect->x + rect->w + cellW - 1)) - (-cellW & rect->x)) >> wshift;
    s32 rows = ((-cellH & (rect->y + rect->h + cellH - 1)) - (-cellH & rect->y)) >> hshift;

    return cols * rows;
}

/* Splits rect at the (1 << wshift) x (1 << hshift) aligned cells it touches,
 * column by column, into out; returns the end of what it wrote. A piece
 * reaches the next cell boundary, or the rect's own edge in the last column
 * and row. */
RECT *rectSplitCells(RECT *out, RECT *rect, s32 wshift, s32 hshift) {
    s32 cellW = 1 << wshift;
    s32 cellH = 1 << hshift;
    s32 cols = ((-cellW & (rect->x + rect->w + cellW - 1)) - (-cellW & rect->x)) >> wshift;
    s32 rows = ((-cellH & (rect->y + rect->h + cellH - 1)) - (-cellH & rect->y)) >> hshift;
    s32 x = rect->x;
    s32 nextX;
    s32 col;

    for (col = cols - 1; col >= 0; col--, x = nextX) {
        s32 w;
        s32 y;
        s32 row;

        if (col == 0) {
            w = rect->x + rect->w - x;
        } else {
            w = (-cellW & (x + cellW)) - x;
        }
        y = rect->y;
        nextX = x + w;
        for (row = rows - 1; row >= 0; row--) {
            s32 h;

            if (row == 0) {
                h = rect->y + rect->h - y;
            } else {
                h = (-cellH & (y + cellH)) - y;
            }
            setRECT(out, x, y, w, h);
            y += h;
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
    this->color = 0x808080;
}

void spriteSetColor(Sprite *this, u8 r, u8 g, u8 b) {
    this->color = r | (g << 8) | (b << 16);
}

void spriteSetGray(Sprite *this, u8 gray) {
    this->color = gray | (gray << 8) | (gray << 16);
}

void spriteSetFlags(Sprite *this, s32 flags) {
    this->flags |= flags;
}

void spriteClearFlags(Sprite *this, s32 flags) {
    this->flags &= ~flags;
}
