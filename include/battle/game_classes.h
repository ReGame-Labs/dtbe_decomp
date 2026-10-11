#ifndef DTBE_BATTLE_GAME_CLASSES_H
#define DTBE_BATTLE_GAME_CLASSES_H

/* Classes of game.bin known only by their layout so far, named after their
 * constructors. */

#include "common.h"
#include "vtable.h"
#include <libgpu.h>
#include "battle/game.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"

EXTERN_C_BEGIN

/* A class that func_8007F920 constructs: a value, a zeroed word, and a
 * virtual table at 0xC. Its methods (0x8007F0F4-0x8007FB9C) work on the
 * Unk80076F34 at 8; those keep two of them, at 0x22C and 0x238. */
typedef struct Unk8007F920 {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ struct Unk80076F34 *unk8;
    /* 0xC */ VtableEntry *vtable;
} Unk8007F920;

/* An entry of the tables that func_80079B24 gives an Unk80076F34: something
 * and a count. */
typedef struct Unk80079B24Entry {
    /* 0x0 */ void *unk0;
    /* 0x4 */ s32 count;
} Unk80079B24Entry;

/* An entry of the table an Unk80076F34 points to at 0x220, as func_8007B470
 * reads it. */
typedef struct Unk8007B470Entry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s16 unk4; /* 0x4 and 0x6: a range of steps (func_800B63BC, lh in func_8007B470) */
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s32 unk8[6];
    /* 0x20 */ s32 unk20;
} Unk8007B470Entry;

/* The table an Unk80076F34 points to at 0x220 (func_80077698): a few words,
 * then entries 0x24 bytes apart (the code uses up to entries[8]). The callers
 * use entries 0 to 5 (0x3C to 0xF0) and func_8007B470 reads an entry up to
 * 0x20 only, so the entries are 0x24 bytes, not records of 0x48 at 0x60 and
 * 0xA8 (as func_80099AE8 and func_80092C2C alone would suggest). */
typedef struct Unk80077698 {
    /* 0x00 */ u16 unk0;                 /* bits, read with lhu */
    /* 0x02 */ s16 unk2;                 /* a size; some callers add it to a y */
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u16 unk14; /* func_8007F184 reads it with lhu */
    /* 0x16 */ u16 unk16;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ u16 unk1C[16];               /* func_800788B4 reads one by index */
    /* 0x3C */ Unk8007B470Entry entries[9];
} Unk80077698;

/* The objects of func_8007F184 and func_8007F6FC: classes derived from
 * Unk8007F920, not identified yet. The character keeps an Unk8007F184 at
 * 0x238: unless 0x2C is set, func_8007F068 takes an amount off 0x20 (down
 * to 0) and, in some of the character's states, adds half of it to 0x24. */
typedef struct Unk8007F184 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14; /* func_8007F7F4 acts only when it is not above 0, and sets it to 16 */
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C; /* the most 0x20 can hold */
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24; /* 0 to 0x10000 */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C; /* set by func_8007F59C: the amounts stop changing */
} Unk8007F184;

/* Something timed that an Unk80076F34 runs (two of them, at 0x464 and 0x478):
 * func_800C9790 counts unk4 down, clearing the whole when it reaches 0, and
 * switches on unk0 (1 to 6). */
typedef struct Unk800774C0 {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
} Unk800774C0;

/* A node of the list whose head is D_8006437C, mesh_scene's global list
 * (func_80076A00 gives it; func_80076A0C moves a node to its front). func_80076474 rotates with the
 * matrix at 0xC of the node it picks. */
typedef struct Unk80076A0C {
    /* 0x00 */ ListNode link;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ MATRIX *unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14; /* func_80076AAC sets 1 */
} Unk80076A0C;

/* The member at 0x260 of Unk80076F34, which func_80076444 constructs. */
typedef struct Unk80076444 {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s32 unk10; /* func_800765B4 hands both to func_800765F0 */
    /* 0x14 */ s32 unk14;
    /* 0x18 */ u8 unk18[0x48 - 0x18];
    /* 0x48 */ VECTOR unk48; /* func_80076474's a */
    /* 0x58 */ VECTOR unk58; /* func_80076474's b */
    /* 0x68 */ VECTOR unk68; /* func_80078254 takes vecDot12 of it */
    /* 0x78 */ Unk80076A0C *unk78; /* the node of D_8006437C func_80076474 last picked */
} Unk80076444;

/* The entity class that func_80076F34 constructs (virtual table D_8006530C),
 * which many classes derive from: 0x4A4 bytes, as the derived constructors'
 * first fields show; they put their own fields from 0x4A4 on. The fields are
 * the ones the C code and the small accessors use. */
typedef struct Unk80076F34 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x218 - 0x18];
    /* 0x218 */ s32 unk218;                /* func_80077568; picks the row of D_80067AD4 (func_800776A4) */
    /* 0x21C */ s32 unk21C;                /* func_80077574 */
    /* 0x220 */ Unk80077698 *unk220;       /* func_80077698 */
    /* 0x224 */ Unk80079B24Entry *unk224;  /* func_80079B24 */
    /* 0x228 */ Unk8006ED20 *unk228;       /* func_80077580 */
    /* 0x22C */ Unk8007F920 *unk22C;
    /* 0x230 */ s32 unk230;
    /* 0x234 */ struct Unk80075000 *unk234; /* its input (func_80078C28, func_800776C4) */
    /* 0x238 */ Unk8007F184 *unk238;
    /* 0x23C */ u8 unk23C[0x240 - 0x23C];
    /* 0x240 */ Entity *unk240;            /* the entity of unk244 (func_8007ADCC) */
    /* 0x244 */ u32 unk244;                /* the handle of the character the model is attached to, or 0 */
    /* 0x248 */ u32 unk248;                /* the handle of the entity func_80079420 makes */
    /* 0x24C */ u8 unk24C[0x250 - 0x24C];
    /* 0x250 */ s32 unk250;                /* a clip of the model, or negative */
    /* 0x254 */ s32 unk254;                /* another clip, or negative */
    /* 0x258 */ s32 unk258;                /* 0x11 or 0x12 (func_80079268, func_8007925C) */
    /* 0x25C */ s32 unk25C;                /* the handle of the sound func_800784F4 played */
    /* 0x260 */ Unk80076444 unk260;
    /* 0x2DC */ u8 unk2DC[0x48];           /* an Unk80075EE4 (defined below; func_80079300 returns it) */
    /* 0x324 */ s32 unk324;
    /* 0x328 */ s32 unk328;
    /* 0x32C */ s32 unk32C;
    /* 0x330 */ s32 unk330;
    /* 0x334 */ s32 unk334;
    /* 0x338 */ s32 unk338;
    /* 0x33C */ s32 unk33C;
    /* 0x340 */ s32 unk340;
    /* 0x344 */ u8 unk344[0x348 - 0x344];
    /* 0x348 */ u16 unk348[13][2];         /* Unk8007BF74 counter pairs that func_800788B4 sets */
    /* 0x37C */ u32 flags;
    /* 0x380 */ s32 unk380;
    /* 0x384 */ Vec3 position;             /* in 1/256ths (func_800775D8) */
    /* 0x390 */ u8 unk390[4];
    /* 0x394 */ Vec3 unk394;               /* in 1/256ths too (func_8007760C) */
    /* 0x3A0 */ u8 unk3A0[4];
    /* 0x3A4 */ Vec3 unk3A4;
    /* 0x3B0 */ s32 unk3B0;
    /* 0x3B4 */ Vec3 unk3B4;               /* vx grows by the facing (func_80095008) */
    /* 0x3C0 */ s32 unk3C0;
    /* 0x3C4 */ s32 unk3C4;                /* a 16.16 scale (func_80095008) */
    /* 0x3C8 */ VECTOR unk3C8;             /* func_800A1160 interpolates it (Unk800A0FF4) */
    /* 0x3D8 */ s32 unk3D8;                /* with 0x3DC, 0x3E0: angles func_80078578 turns toward 0x3E8.. */
    /* 0x3DC */ s32 unk3DC;
    /* 0x3E0 */ s32 unk3E0;                /* an angle (func_800BBF00 turns toward it) */
    /* 0x3E4 */ u8 unk3E4[0x3E8 - 0x3E4];
    /* 0x3E8 */ s32 unk3E8;
    /* 0x3EC */ s32 unk3EC; /* 0x401 or -0x401: func_80079E10 makes it the facing, 1 or -1 */
    /* 0x3F0 */ s32 unk3F0;
    /* 0x3F4 */ u8 unk3F4[0x3FC - 0x3F4];
    /* 0x3FC */ Model *model;              /* func_800770AC makes it with modelInit */
    /* 0x400 */ u8 unk400[0x440 - 0x400];
    /* 0x440 */ s32 unk440;                /* func_8007768C */
    /* 0x444 */ u8 unk444[0x448 - 0x444];
    /* 0x448 */ u32 unk448;                /* bits: set with |= */
    /* 0x44C */ s32 unk44C;                /* set with 0x450..0x45C by func_80084558, func_80084838 */
    /* 0x450 */ s32 unk450;
    /* 0x454 */ s32 unk454;
    /* 0x458 */ s32 unk458;
    /* 0x45C */ s32 unk45C;
    /* 0x460 */ s32 unk460;
    /* 0x464 */ Unk800774C0 unk464;        /* func_800C9790 runs it */
    /* 0x478 */ Unk800774C0 unk478;        /* func_800C99B0 runs it */
    /* 0x48C */ s32 unk48C;
    /* 0x490 */ u8 unk490[0x494 - 0x490];
    /* 0x494 */ s32 unk494;                /* func_8007A12C sets it to 5 or clears it */
    /* 0x498 */ s32 unk498;
    /* 0x49C */ u8 unk49C[0x4A0 - 0x49C];
    /* 0x4A0 */ s32 unk4A0;                /* func_8009A35C sets it to 1 */
} Unk80076F34;

/* Entity classes that keep the Unk80076F34 they were created for, named
 * after their constructors (the ones with the same layout share a struct). */
typedef struct Unk800AFD78 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *unk18;
    /* 0x1C */ Unk8006ED20 *unk1C;
    /* 0x20 */ s32 unk20[4];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C[8];
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
} Unk800AFD78;

/* The entity func_800B01AC constructs (virtual table D_8006B624); func_800B050C
 * constructs another class (D_8006B5FC) with the same fields. Their gathers move
 * 0x24 down by 0x120 for 24 (16) calls, then delete them. */
typedef struct Unk800B01AC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *unk18; /* the character (func_8007768C gives 0x3C) */
    /* 0x1C */ Unk8006ED20 *unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* gathers so far */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s16 unk3C;
} Unk800B01AC;
typedef Unk800B01AC Unk800B050C;

typedef struct Unk800B73E8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *unk18;
    /* 0x1C */ Vec3 unk1C;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s16 unk40;
} Unk800B73E8;
typedef Unk800B73E8 Unk800AED0C;

/* The entity func_800C3CFC constructs at a position (0x54 bytes, virtual table D_8006D40C);
 * func_800CAEF0's class derives from it with no fields of its own, and the class func_8008E9B4 and
 * func_8008EA58 derive from it, which keeps the Unk80076F34 it was created for (func_8008DE84 spawns them). */
typedef struct Unk800C3CFC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR unk18; /* a random direction, normalized */
    /* 0x28 */ VECTOR unk28; /* the position it is constructed with */
    /* 0x38 */ s32 unk38; /* the size it is constructed with */
    /* 0x3C */ s32 unk3C; /* a random angle, turned by unk40 a frame (func_800C3E64) */
    /* 0x40 */ s32 unk40; /* 0 to 47 at random */
    /* 0x44 */ s32 unk44; /* size / 32 + 1 to twice that, at random */
    /* 0x48 */ s32 unk48; /* the color it is constructed with */
    /* 0x4C */ s32 unk4C; /* frames lived */
    /* 0x50 */ s32 unk50;
} Unk800C3CFC;

typedef struct Unk8008E9B4 {
    /* 0x00 */ Unk800C3CFC base;
    /* 0x54 */ Unk80076F34 *unk54;
} Unk8008E9B4;

/* A text shown character by character: func_800CD140 copies the text (at most 0x40 characters, with
 * strlen and strncpy) and keeps its length; every character has an entry of its own, whose 0x4 and 0x6 it
 * sets to 0x20, its color to 0x80808080 and the rest to 0; the functions after it set one field of every
 * character. */
typedef struct Unk800CD140Char {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s16 unk4;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ CVECTOR color; /* 0x80808080 at first */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
} Unk800CD140Char;

typedef struct Unk800CD140 {
    /* 0x000 */ Unk800CD140Char chars[0x40];
    /* 0x500 */ char text[0x40];
    /* 0x540 */ s32 length;
    /* 0x544 */ s32 unk544; /* the step between two characters (func_800CBB88 centres the text by it) */
    /* 0x548 */ s32 unk548;
} Unk800CD140;

/* An entity of game.bin that keeps copies of two 16-color palettes of VRAM (func_800D1F8C, func_800D23C4) */
typedef struct Unk800D1F8C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ RECT clutRects[2];
    /* 0x28 */ u16 cluts[2][16];
    /* 0x68 */ s16 unk68;
    /* 0x6A */ s16 unk6A;
    /* 0x6C */ Unk8006ED20 *owner;
} Unk800D1F8C; /* size 0x70 */

/* An entity of game.bin that spawns count particles of func_800C28B8 around pos, at random up to radius away
 * (func_800C6088, func_800C62EC, func_800C652C) */
typedef struct Unk800C6088 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ VECTOR pos;
    /* 0x2C */ s32 radius;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 count;
    /* 0x38 */ s32 unk38;
} Unk800C6088; /* size 0x3C */

/* The member at 0x18 of Unk800C2F14: func_800C2CCC keeps the frame table (its first 4 halfwords are x, y, w, h)
 * and the 0x100 bit of the flags, then func_800C2D04 its corners; func_800C2E84 makes a PolyFT4Shape of it */
typedef struct Unk800C2CCC {
    /* 0x00 */ u16 *frame;
    /* 0x04 */ s16 flags;
    /* 0x06 */ s16 unk6;
    /* 0x08 */ s32 size;   /* half the side of the square func_8009F120 draws */
    /* 0x0C */ u32 color;
    /* 0x10 */ VECTOR pos;
    /* 0x20 */ s16 left, top, right, bottom;
} Unk800C2CCC; /* size 0x28 */

/* The shape buildPolyFT4 projects (engine/gfx/prim/build_line_g2.h takes it as void *): four corners, a color
 * and the uv, clut and tpage words of the POLY_FT4; func_800C2E84 fills it from an Unk800C2CCC */
typedef struct PolyFT4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 color;
    /* 0x18 */ u16 uv0, clut;
    /* 0x1C */ u16 uv1, tpage;
    /* 0x20 */ u16 uv2, pad22;
    /* 0x24 */ u16 uv3, pad26;
} PolyFT4Shape; /* size 0x28 */

/* The entity class of vtable D_8006D59C (func_800C2F14): a frame of a sprite moved by func_800C3078's vectors */
typedef struct Unk800C2F14 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800C2CCC sprite;
    /* 0x40 */ s16 unk40;
    /* 0x42 */ s16 unk42;
    /* 0x44 */ Vec3 unk44;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54;
} Unk800C2F14; /* size 0x58 */

/* A class derived from Unk800C2F14 that keeps its owner (func_8009F248, func_8009F4D4) */
typedef struct Unk8009F248 {
    /* 0x00 */ Unk800C2F14 base;
    /* 0x58 */ Unk80076F34 *owner;
} Unk8009F248; /* size 0x5C */

/* classes that derive from Unk80076F34, named after their constructors */
typedef struct Unk800870F0 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
} Unk800870F0;

typedef struct Unk800A0FF4 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ u8 unk4A8[0x4B8 - 0x4A8];
    /* 0x4B8 */ s32 unk4B8; /* 0 to 0x1000: how far base.unk3C8 is from unk4CC to unk4DC */
    /* 0x4BC */ u8 unk4BC[0x4CC - 0x4BC];
    /* 0x4CC */ VECTOR unk4CC;
    /* 0x4DC */ VECTOR unk4DC;
} Unk800A0FF4;

typedef struct Unk800B45C0 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
} Unk800B45C0;

typedef struct Unk800B631C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
} Unk800B631C;

/* The class func_800B9410 constructs (virtual table D_8006C71C): 0x4A4 is set once its entity of
 * func_800BAAA4 is out. */
typedef struct Unk800B9410 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s16 unk4A4;
} Unk800B9410;

/* A class that func_800CB53C constructs: it derives from the class of
 * D_80065164 and keeps a word. */
typedef struct Unk800CB53C {
    /* 0x0 */ VtableEntry *vtable;
    /* 0x4 */ s32 unk4;
} Unk800CB53C;

/* The parts that many entity classes share: a base class or member that
 * Unk8007BD8C starts, and a member Unk80075EE4. */

/* A class that func_8007BD8C constructs (virtual table D_800652EC): a
 * virtual table, three arguments and a copied vector; func_8007BDF0 is its
 * first method. It derives from the class of D_80065164 (Unk80076308,
 * func_800762F4), which has nothing but the virtual table. Entities that
 * derive from it keep it at 0x18: the virtual tables of that base move
 * `this` back by 0x18 to reach them. */
typedef struct Unk8007BD8C {
    /* 0x00 */ VtableEntry *vtable;
    /* 0x04 */ struct Unk80076F34 *chara; /* the spawners pass the Unk80076F34 that owns them */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ Vec3 unk10;
} Unk8007BD8C;

/* the virtual table of Unk8007BD8C */
extern VtableEntry D_800652EC[];

/* A class that func_8007BF20 constructs (virtual table D_800652CC) on
 * func_8007BD8C's: an Unk8007BD8C with flags; it clears their bits 0 and 1. */
typedef struct Unk8007BF20 {
    /* 0x00 */ Unk8007BD8C base;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ u32 flags;
} Unk8007BF20;

/* A class kept in a list, with its virtual table at 0x30 (D_80065184), which
 * func_800761AC constructs; Unk80075EE4 (D_8006513C) derives from it. */
typedef struct Unk800761AC {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *unk8; /* func_80075EE4's fourth argument: the owner's second base */
    /* 0x0C */ u32 unkC;   /* flags: the constructors clear bits 0 and 1 */
    /* 0x10 */ s32 unk10;
    /* 0x14 */ void *unk14; /* the owning entity (its constructor sets it) */
    /* 0x18 */ u8 unk18[0x30 - 0x18];
    /* 0x30 */ VtableEntry *vtable;
} Unk800761AC;

/* the virtual table of Unk800761AC, which the destructors that inline its
 * destructor install last; the one of Unk8007BD8C's base is D_80065164
 * (battle/game.h) */
extern VtableEntry D_80065184[];

/* A class that func_80075EE4 constructs: a node of a circular list, the
 * object it belongs to, flags and a virtual table after them, which make the
 * class of D_80065184 (Unk800761AC) it derives from, then a vector and two
 * words of its own. func_8007BDF0 (and st02's func_800D54C0, st00's
 * func_800D99AC) add their owner's count, word and vector into the one
 * linked to theirs (func_80075B48); func_80078878 finds an entity at 0x14. */
typedef struct Unk80075EE4 {
    /* 0x00 */ ListNode node;
    /* 0x08 */ void *owner; /* its second base: an Unk8007BD8C in game.bin, an UnkStageObject in st00 */
    /* 0x0C */ u32 flags;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ Entity *unk14; /* func_800ACA60 puts its entity here */
    /* 0x18 */ u32 unk18; /* flags above bit 18, a count saturating at 0x3FFFF below */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ Vec3 unk20;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ VtableEntry *vtable;
    /* 0x34 */ Vec3 unk34;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Unk80075EE4;

/* Constructs an Unk80075EE4 at position (three words) in the list of owner, an object's second base. */
Unk80075EE4 *func_80075EE4(Unk80075EE4 *this, Vec3 *position, s32 unk44, void *owner);

/* An entity that derives from Unk8007BD8C too and holds an Unk80075EE4. */
typedef struct UnkPartsEntity38 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C base;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ Unk80075EE4 part;
} UnkPartsEntity38;

/* The UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740 (func_800C7024): it keeps the position and
 * the velocity it was made with, and the velocity's length at 0xAC. The classes of func_80087690 and
 * func_800B719C derive from it with no fields of their own known; func_80083108's adds 0xC0. */
typedef struct Unk800C7024 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Vec3 pos;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ Vec3 vel;
    /* 0x9C */ s32 unk9C;
    /* 0xA0 */ s32 unkA0;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 speed;
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC; /* func_8007768C of the owner */
} Unk800C7024; /* size 0xC0 */

/* A class derived from Unk800C7024 (func_8009BE08, func_8009CE8C) */
typedef struct Unk8009BE08 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
} Unk8009BE08;

/* An entity that holds an Unk8007BF20 and an Unk80075EE4. */
typedef struct UnkPartsEntity3C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BF20 unk18;
    /* 0x3C */ Unk80075EE4 part;
} UnkPartsEntity3C;

/* The class that func_80097FA0 constructs (vtable D_80069A7C): an
 * UnkPartsEntity3C with an Unk80076F34 and a vector it hands to what it creates. */
typedef struct Unk80097FA0 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Unk80076F34 *unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ Vec3 unk8C;
} Unk80097FA0;

/* The classes func_800ACA60 and func_800AC470 construct (0xB0 bytes): parts that move from pos by vel
 * every frame and, after 12 frames, spawn two entities and delete themselves. */
typedef struct Unk800ACA60 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ Unk8006ED20 *unk84;
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 vel;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 frames;
    /* 0xAC */ s16 unkAC; /* func_8007768C of the owner */
    /* 0xAE */ s16 unkAE;
} Unk800ACA60;
typedef Unk800ACA60 Unk800AC470;

/* The class func_800AD050 constructs (virtual tables D_8006B3C4, and D_8006B3A4 for its base at 0x18),
 * like Unk800ACA60 with more: the owner's position when made and some counters. */
typedef struct Unk800AD050 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ Unk8006ED20 *list; /* the part goes into one of its two lists (func_80077574 picks which) */
    /* 0x88 */ Vec3 pos;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 charaPosition; /* the owner's when made (func_800775D8) */
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ Vec3 vel;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s16 unkCC; /* func_8007768C of the owner */
} Unk800AD050;

/* The class func_800C06D0 constructs (0xB0 bytes): it plays sound 0x111 and keeps the handle. */
typedef struct Unk800C06D0 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ VECTOR unk84; /* func_8007A4A4 of the owner's point 5 */
    /* 0x94 */ s32 unk94;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ s32 unk9C;
    /* 0xA0 */ s32 unkA0;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ u32 sound;
} Unk800C06D0;

/* The class func_800C0CC0 constructs (0x2AC bytes): it plays a sound and keeps the handle last. */
typedef struct Unk800C0CC0 {
    /* 0x000 */ UnkPartsEntity38 parts;
    /* 0x080 */ u8 unk80[0x280 - 0x80];
    /* 0x280 */ Unk80076F34 *owner;
    /* 0x284 */ u8 unk284[0x2A8 - 0x284];
    /* 0x2A8 */ u32 sound;
} Unk800C0CC0;

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_CLASSES_H */
