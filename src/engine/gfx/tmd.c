#include "common.h"
#include "engine/gfx/tmd.h"
#include "engine/gfx/camera.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "gte.h"

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/tmd", TMD_PRIM_SIZES);

/* Loads the view matrix alone into the GTE's rotation and translation, for
 * what is drawn in world coordinates. */
void setGteViewMatrix(void) {
    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
}

/* Returns the address 0xF4 bytes into the buffer at 0x34 of an object
 * nothing else in this file uses. */
s32 func_80021C58(Unk80021C58 *this) {
    return this->unk34 + 0xF4;
}

/* Adds the primitives of a TMD object to an ordering table, a run of
 * primitives of the same type at a time, and returns the packet after the
 * last one it used. */
PACKET *tmdObjectDraw(TmdObject *this, GsOT *ot, PACKET *packet, s32 shift) {
    TmdPrimHeader *prim = this->primTop;
    s32 left = this->primCount;
    SVECTOR *vertTop = this->vertTop;
    SVECTOR *normalTop = this->normalTop;
    u32 header;
    u16 run;
    s32 type;

    while (left != 0) {
        header = prim->word;
        run = prim->run;
        /* the semi-transparency bit of the mode gives way to the
         * double-sided bit of the flags */
        type = (((header >> 24) & ~2) | ((header >> 16) & 2)) - 0x20;
        if (!(type & 1)) {
            packet = TMD_DRAW_FUNCS[type](prim, vertTop, normalTop, packet, run, shift, ot);
        } else {
            /* brightness calculation off: no normals */
            packet = TMD_DRAW_FUNCS[type](prim, vertTop, packet, run, shift, ot);
        }
        left -= run;
        prim = (TmdPrimHeader *)((u8 *)prim + TMD_PRIM_SIZES[type] * run);
    }
    return packet;
}

/* Turns the object offsets of a TMD model into addresses, once: returns 1
 * when it is not a TMD model, 0 otherwise. The 0 is the original's status
 * variable: returning 0 directly, GCC gives this $s2 and i $s1, the game has
 * them the other way round. */
s32 tmdHeaderRelocate(TmdHeader *this) {
    TmdObject *objects = this->objects;
    s32 status;
    u32 i;

    if (this->id != TMD_ID) {
        return 1;
    }
    status = 0;
    if (this->flags & TMD_FIXP) {
        return status;
    }
    this->flags |= TMD_FIXP;
    for (i = 0; i < this->objectCount; i++) {
        /* the offsets in an object count from the start of the table */
        tmdObjectRelocate(&objects[i], (u32)objects);
    }
    return status;
}

/* Turns the offsets of a TMD object into addresses by adding base, sorts its
 * primitives, then stores in the first primitive of every run of primitives
 * with the same header word the length of the run. */
void tmdObjectRelocate(TmdObject *this, u32 base) {
    s32 left = this->primCount;
    TmdPrimHeader *prim;
    TmdPrimHeader *first;
    u32 header;
    u16 run;

    this->vertTop = (SVECTOR *)((u32)this->vertTop + base);
    this->normalTop = (SVECTOR *)((u32)this->normalTop + base);
    this->primTop = (TmdPrimHeader *)((u32)this->primTop + base);
    prim = this->primTop;
    sortTmdPrims(left, prim);
    while (left != 0) {
        run = 1;
        first = prim;
        left--;
        header = first->word;
        /* the primitives of a run have the size of the first */
        prim = (TmdPrimHeader *)((u8 *)first + TMD_PRIM_SIZE(first));
        while (left != 0 && prim->word == header) {
            run++;
            prim = (TmdPrimHeader *)((u8 *)prim + TMD_PRIM_SIZE(first));
            left--;
        }
        first->run = run;
    }
}

/* Sorts the count primitives at prims by their header word: copies them a
 * word at a time into a chain of nodes, sorts the chain, then copies them
 * back in order. */
void sortTmdPrims(s32 count, TmdPrimHeader *prims) {
    TmdPrimNode *nodes;
    TmdPrimNode *node;
    TmdPrimHeader *prim;
    u32 *src;
    u32 *dst;
    u32 *srcEnd;
    u32 *from;
    u32 *fromEnd;
    u32 *to;
    u32 *next;
    s32 i;
    s32 left;

    i = count;
    prim = prims;
    do {
        prim = (TmdPrimHeader *)((u8 *)prim + TMD_PRIM_SIZE(prim));
        i--;
    } while (i != 0);
    /* the primitives and a link word for each */
    nodes = (TmdPrimNode *)operatorVecNew((prim - prims + count) * sizeof(u32));
    dst = (u32 *)nodes;
    left = count;
    src = (u32 *)prims;
    do {
        node = (TmdPrimNode *)dst;
        dst = (u32 *)&node->prim;
        srcEnd = (u32 *)((u8 *)src + TMD_PRIM_SIZE((TmdPrimHeader *)src));
        left--;
        do {
            *dst++ = *src++;
        } while (src != srcEnd);
        node->next = (TmdPrimNode *)dst;
    } while (left != 0);
    node->next = NULL;
    /* the sort only follows and sets the next links, the first word of a
     * node as of a LinkNode */
    from = (u32 *)linkNodeSort((LinkNode *)nodes, (LinkCompare)tmdPrimNodeCompare);
    to = (u32 *)prims;
    while (from != NULL) {
        next = (u32 *)*from++;
        fromEnd = (u32 *)((u8 *)from + TMD_PRIM_SIZE((TmdPrimHeader *)from));
        do {
            *to++ = *from++;
        } while (from != fromEnd);
        from = next;
    }
    if (nodes != NULL) {
        operatorVecDelete(nodes);
    }
}

/* Orders the nodes of sortTmdPrims by the header word of their primitive. */
s32 tmdPrimNodeCompare(TmdPrimNode *a, TmdPrimNode *b) {
    return a->prim.word - b->prim.word;
}

/* Returns how many objects a TMD model has, 0 when it is not one. */
s32 tmdHeaderGetObjectCount(TmdHeader *this) {
    if (this->id != TMD_ID) {
        return 0;
    }
    return this->objectCount;
}

/* Returns object index of a TMD model, NULL when there is none. */
TmdObject *tmdHeaderGetObject(TmdHeader *this, u32 index) {
    TmdObject *objects = this->objects;

    if (this->id != TMD_ID) {
        return NULL;
    }
    if (index >= this->objectCount) {
        return NULL;
    }
    return &objects[index];
}

/* Relocates a TMD model and moves the textures of all its primitives to the
 * lower half of VRAM, a run of primitives of the same type at a time. */
void tmdHeaderMoveTexturesDown(TmdHeader *this) {
    s32 objectCount;
    s32 i;
    TmdObject *tmdObject;
    TmdPrimHeader *prim;
    s32 left;
    s32 type;
    s32 run;

    tmdHeaderRelocate(this);
    objectCount = tmdHeaderGetObjectCount(this);
    for (i = 0; i < objectCount; i++) {
        tmdObject = tmdHeaderGetObject(this, i);
        prim = tmdObject->primTop;
        for (left = tmdObject->primCount; left != 0; left -= run) {
            type = TMD_PRIM_TYPE(prim->f.mode);
            run = prim->run;
            if (TMD_MOVE_TEXTURES_DOWN_FUNCS[type] != NULL) {
                TMD_MOVE_TEXTURES_DOWN_FUNCS[type](prim, run);
            }
            prim = (TmdPrimHeader *)((u8 *)prim + TMD_PRIM_SIZES[type] * run);
        }
    }
}

/* the bits that put a CLUT 256 lines down and a texture page in the second
 * row: in the lower half of VRAM */
#define CLUT_LOWER_HALF 0x4000
#define TPAGE_LOWER_HALF 0x10

/* Moves the textures of count TF3 primitives to the lower half of VRAM. */
void moveTexturesDownTF3(TMD_P_TF3 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TNF3 primitives to the lower half of VRAM. */
void moveTexturesDownTNF3(TMD_P_TNF3 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TF4 primitives to the lower half of VRAM. */
void moveTexturesDownTF4(TMD_P_TF4 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TNF4 primitives to the lower half of VRAM. */
void moveTexturesDownTNF4(TMD_P_TNF4 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TG3 primitives to the lower half of VRAM. */
void moveTexturesDownTG3(TMD_P_TG3 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TNG3 primitives to the lower half of VRAM. */
void moveTexturesDownTNG3(TMD_P_TNG3 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TG4 primitives to the lower half of VRAM. */
void moveTexturesDownTG4(TMD_P_TG4 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Moves the textures of count TNG4 primitives to the lower half of VRAM. */
void moveTexturesDownTNG4(TMD_P_TNG4 *prim, s32 count) {
    while (count != 0) {
        count--;
        prim->clut |= CLUT_LOWER_HALF;
        prim->tpage |= TPAGE_LOWER_HALF;
        prim++;
    }
}

/* Sets bounds to the bounding box of the vertices of a TMD object's
 * primitives. */
void tmdObjectGetBounds(TmdObject *this, Bounds *bounds) {
    TmdPrimHeader *prim;
    s32 left;
    SVECTOR *vertTop;
    u16 run;
    s32 type;

    bounds->min[0] = 0x7FFF;
    bounds->min[1] = 0x7FFF;
    bounds->min[2] = 0x7FFF;
    bounds->max[0] = -0x8000;
    bounds->max[1] = -0x8000;
    bounds->max[2] = -0x8000;
    prim = this->primTop;
    left = this->primCount;
    vertTop = this->vertTop;
    while (left != 0) {
        run = prim->run;
        type = TMD_PRIM_TYPE(prim->f.mode);
        TMD_GROW_BOUNDS_FUNCS[type](prim, run, vertTop, bounds);
        left -= run;
        prim = (TmdPrimHeader *)((u8 *)prim + TMD_PRIM_SIZES[type] * run);
    }
}

/* Grows bounds by the vertices of count flat triangles. */
void growBoundsF3(TMD_P_F3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count Gouraud-shaded triangles. */
void growBoundsG3(TMD_P_G3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit flat triangles. */
void growBoundsNF3(TMD_P_NF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit Gouraud-shaded triangles. */
void growBoundsNG3(TMD_P_NG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count flat quads. */
void growBoundsF4(TMD_P_F4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count Gouraud-shaded quads. */
void growBoundsG4(TMD_P_G4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit flat quads. */
void growBoundsNF4(TMD_P_NF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit Gouraud-shaded quads. */
void growBoundsNG4(TMD_P_NG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured flat triangles. */
void growBoundsTF3(TMD_P_TF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured Gouraud-shaded triangles. */
void growBoundsTG3(TMD_P_TG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured flat triangles. */
void growBoundsTNF3(TMD_P_TNF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured Gouraud-shaded
 * triangles. */
void growBoundsTNG3(TMD_P_TNG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured flat quads. */
void growBoundsTF4(TMD_P_TF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count textured Gouraud-shaded quads. */
void growBoundsTG4(TMD_P_TG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured flat quads. */
void growBoundsTNF4(TMD_P_TNF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds by the vertices of count unlit textured Gouraud-shaded
 * quads. */
void growBoundsTNG4(TMD_P_TNG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds) {
    SVECTOR *v0;
    SVECTOR *v1;
    SVECTOR *v2;
    SVECTOR *v3;

    while (count != 0) {
        v0 = &vertTop[prim->v0];
        v1 = &vertTop[prim->v1];
        v2 = &vertTop[prim->v2];
        v3 = &vertTop[prim->v3];
        boundsGrowByVertex(bounds, v0);
        boundsGrowByVertex(bounds, v1);
        boundsGrowByVertex(bounds, v2);
        boundsGrowByVertex(bounds, v3);
        count--;
        prim++;
    }
}

/* Grows bounds to take in vertex. */
void boundsGrowByVertex(Bounds *this, SVECTOR *vertex) {
    s16 coord;

    coord = vertex->vx;
    if (coord < this->min[0]) {
        this->min[0] = coord;
    }
    if (this->max[0] < coord) {
        this->max[0] = coord;
    }
    coord = vertex->vy;
    if (coord < this->min[1]) {
        this->min[1] = coord;
    }
    if (this->max[1] < coord) {
        this->max[1] = coord;
    }
    coord = vertex->vz;
    if (coord < this->min[2]) {
        this->min[2] = coord;
    }
    if (this->max[2] < coord) {
        this->max[2] = coord;
    }
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/tmd", TMD_DRAW_FUNCS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/tmd", TMD_MOVE_TEXTURES_DOWN_FUNCS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/gfx/tmd", TMD_GROW_BOUNDS_FUNCS);
