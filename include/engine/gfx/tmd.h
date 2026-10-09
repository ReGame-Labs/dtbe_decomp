#ifndef DTBE_GFX_TMD_H
#define DTBE_GFX_TMD_H

/* TMD models: relocating and sorting their primitives, drawing them, moving
 * their textures, and their bounding boxes. */

#include "common.h"
#include <libgte.h>
#include <libgs.h>

EXTERN_C_BEGIN

/* an axis-aligned bounding box */
typedef struct {
    /* 0x0 */ s16 min[3];
    /* 0x6 */ s16 max[3];
} Bounds;

/* the id word of a TMD model file */
#define TMD_ID 0x41
/* TmdHeader.flags: the object pointers are addresses, not offsets */
#define TMD_FIXP 1

/* The header word of a TMD primitive. The game reuses its out/in halfword:
 * once tmdObjectRelocate has sorted an object's primitives, the first one of
 * every run of primitives with the same header word holds the length of the
 * run there. */
typedef union {
    struct {
        /* 0x0 */ u8 out; /* words of packet the primitive draws */
        /* 0x1 */ u8 in;  /* words of data after this header */
        /* 0x2 */ u8 flag;
        /* 0x3 */ u8 mode;
    } f;
    /* 0x0 */ u16 run;
    /* 0x0 */ u32 word;
} TmdPrimHeader;

/* the size in bytes of a primitive, header included */
#define TMD_PRIM_SIZE(header) ((header)->f.in * 4 + 4)

/* The index into the tables by primitive type (TMD_PRIM_SIZES,
 * TMD_DRAW_FUNCS, TMD_MOVE_TEXTURES_DOWN_FUNCS, TMD_GROW_BOUNDS_FUNCS) of a
 * primitive's mode byte, without its semi-transparency bit: the polygon modes
 * start at 0x20. */
#define TMD_PRIM_TYPE(mode) (((mode) & 0xFD) - 0x20)

/* An object of a TMD model: its vertices, normals and primitives. The TMD
 * format calls the fields vert_top, n_vert, normal_top, n_normal,
 * primitive_top, n_primitive and scale. */
typedef struct {
    /* 0x00 */ SVECTOR *vertTop;
    /* 0x04 */ u32 vertCount;
    /* 0x08 */ SVECTOR *normalTop;
    /* 0x0C */ u32 normalCount;
    /* 0x10 */ TmdPrimHeader *primTop;
    /* 0x14 */ u32 primCount;
    /* 0x18 */ s32 scale;
} TmdObject; /* size 0x1C */

/* The header of a TMD model, followed by its objects. The TMD format calls
 * objectCount nobj. */
typedef struct {
    /* 0x0 */ u32 id; /* TMD_ID */
    /* 0x4 */ u32 flags;
    /* 0x8 */ u32 objectCount;
    /* 0xC */ TmdObject objects[1]; /* objectCount of them */
} TmdHeader;

/* A copy of a primitive in the list sortTmdPrims sorts. */
typedef struct TmdPrimNode {
    /* 0x0 */ struct TmdPrimNode *next;
    /* 0x4 */ TmdPrimHeader prim; /* followed by the rest of the primitive */
} TmdPrimNode;

/* The object of func_80021C58, which nothing in the executable calls. */
typedef struct {
    /* 0x00 */ u8 unk0[0x34];
    /* 0x34 */ s32 unk34;
} Unk80021C58;

/* the size of a TMD primitive, by type (TMD_PRIM_TYPE) */
extern s32 TMD_PRIM_SIZES[];

/* The low-level GsTMDfast* functions that add count primitives of one type
 * to an ordering table, by primitive type. libgs declares them without a
 * prototype: the lit types take the normals as well. */
extern PACKET *(*TMD_DRAW_FUNCS[])();
/* The functions that move the textures of count primitives of one type to
 * the lower half of VRAM, by primitive type (NULL for untextured types). */
extern void (*TMD_MOVE_TEXTURES_DOWN_FUNCS[])(void *prim, s32 count);
/* The functions that grow a bounding box by the vertices of count primitives
 * of one type, by primitive type. */
extern void (*TMD_GROW_BOUNDS_FUNCS[])(void *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);

void setGteViewMatrix(void);
s32 func_80021C58(Unk80021C58 *unk80021C58);
PACKET *tmdObjectDraw(TmdObject *tmdObject, GsOT *ot, PACKET *packet, s32 shift);
s32 tmdHeaderRelocate(TmdHeader *tmdHeader);
void tmdObjectRelocate(TmdObject *tmdObject, u32 base);
void sortTmdPrims(s32 count, TmdPrimHeader *prims);
s32 tmdPrimNodeCompare(TmdPrimNode *a, TmdPrimNode *b);
s32 tmdHeaderGetObjectCount(TmdHeader *tmdHeader);
TmdObject *tmdHeaderGetObject(TmdHeader *tmdHeader, u32 index);
void tmdHeaderMoveTexturesDown(TmdHeader *tmdHeader);
void moveTexturesDownTF3(TMD_P_TF3 *prim, s32 count);
void moveTexturesDownTNF3(TMD_P_TNF3 *prim, s32 count);
void moveTexturesDownTF4(TMD_P_TF4 *prim, s32 count);
void moveTexturesDownTNF4(TMD_P_TNF4 *prim, s32 count);
void moveTexturesDownTG3(TMD_P_TG3 *prim, s32 count);
void moveTexturesDownTNG3(TMD_P_TNG3 *prim, s32 count);
void moveTexturesDownTG4(TMD_P_TG4 *prim, s32 count);
void moveTexturesDownTNG4(TMD_P_TNG4 *prim, s32 count);
void tmdObjectGetBounds(TmdObject *tmdObject, Bounds *bounds);
/* The functions of TMD_GROW_BOUNDS_FUNCS, one per type of TMD primitive. */
void growBoundsF3(TMD_P_F3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsG3(TMD_P_G3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNF3(TMD_P_NF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNG3(TMD_P_NG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsF4(TMD_P_F4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsG4(TMD_P_G4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNF4(TMD_P_NF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsNG4(TMD_P_NG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTF3(TMD_P_TF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTG3(TMD_P_TG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNF3(TMD_P_TNF3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNG3(TMD_P_TNG3 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTF4(TMD_P_TF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTG4(TMD_P_TG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNF4(TMD_P_TNF4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void growBoundsTNG4(TMD_P_TNG4 *prim, s32 count, SVECTOR *vertTop, Bounds *bounds);
void boundsGrowByVertex(Bounds *bounds, SVECTOR *vertex);

EXTERN_C_END

#endif /* DTBE_GFX_TMD_H */
