#ifndef DTBE_BATTLE_GAME_CLASSES_H
#define DTBE_BATTLE_GAME_CLASSES_H

/* Classes of game.bin known only by their layout so far, named after their
 * constructors, and the functions several of its source files share. */

#include "common.h"
#include "vtable.h"
#include <libgpu.h>
#include "battle/game.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"
#include "engine/game/loading_screen.h"

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

/* The tables of the characters, by func_80077568's index: the character's 0x220 (func_80077698) points to one. */
extern Unk80077698 D_800656D4[];

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
    /* 0x2DC */ Unk80075EE4 unk2DC;        /* func_80079300 returns it */
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
    /* 0x49C */ s32 unk49C;
    /* 0x4A0 */ s32 unk4A0;                /* func_8009A35C sets it to 1 */
} Unk80076F34;

/* adds delta to the 16.16 level of the object at 0x238, kept between 0 and 1.0 */
void func_80077448(Unk80076F34 *this, s32 delta);
/* the index at 0x218 */
s32 func_80077568(Unk80076F34 *this);
/* sets the position (kept in 1/256ths) from whole units */
void func_800775B0(Unk80076F34 *chara, Vec3 *position);
/* writes the vector at 0x384, each component shifted down by 8 bits */
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out);
/* the character's table (at 0x220) */
Unk80077698 *func_80077698(Unk80076F34 *this);
/* whether the character stands on the given matrix */
s32 func_8007814C(Unk80076F34 *chara, MATRIX *matrix);
void func_80079350(Unk80076F34 *this);
Vec3 *func_8007A650(Unk80076F34 *chara, Vec3 *out);

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

Unk800C3CFC *func_800C3CFC(Unk800C3CFC *this, Vec3 *pos, s32 size, s32 color);
/* its gather */
void func_800C3E64(Unk800C3CFC *this);

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

/* the constructors of Unk800C6088's two classes: count particles around pos; arg6 is a color (0x808080) */
Unk800C6088 *func_800C6088(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6);
Unk800C6088 *func_800C62EC(Unk800C6088 *this, s32 arg1, Vec3 *pos, s32 radius, s32 arg4, s32 count, s32 arg6);

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

/* the destructor of an entity class that others derive from */
void func_800776D0(Entity *this, s32 flags);

/* clears the timed thing at 0x464 */
void func_800774A8(Unk80076F34 *this);

/* returns the object at 0x228 of `this`: the one whose entity group func_8007AD4C uses */
Unk8006ED20 *func_80077580(Unk80076F34 *this);

/* returns the word at 0x440 of an Unk80076F34 */
s32 func_8007768C(Unk80076F34 *this);

/* gives an Unk80076F34 a table (at 0x224) */
void func_80079B24(Unk80076F34 *this, Unk80079B24Entry *table);

/* virtual functions of Unk80076F34, and functions of its that they call */
s32 func_80077794(Unk80076F34 *this);
void func_8007835C(Unk80076F34 *this, s32 arg);
void func_80078804(Unk80076F34 *this, s32 arg);
void func_8007881C(Unk80076F34 *this, s32 arg);
/* Starts a clip of the model at 0x3FC (modelStartClip; a negative clip stops it):
 * plays < 0 takes the count from the character's .loop file (charaGetClipPlays). */
void func_80079830(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame, s32 plays);
/* The same, with the play count of the .loop file; it keeps clip and
 * blendFrames at 0x250/0x24C, and starts the clip only when func_80079A0C allows it. */
void func_800796F0(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame);
void func_80079AE4(Unk80076F34 *this);
void func_8007A6FC(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3);
void func_8007B034(Unk80076F34 *this, s32 arg);
void func_8007B0E0(Unk80076F34 *this, s32 arg);
void func_8007B470(Unk80076F34 *this, s32 arg, Unk8007B470Entry *entry, s32 arg3);
void func_8007B6A0(Unk80076F34 *this, s32 arg);
void func_8007B70C(Unk80076F34 *this, s32 arg); /* the method at 0xA8 */
void func_8007B784(Unk80076F34 *this, s32 arg); /* the method at 0xB0 */
void func_8007B80C(Unk80076F34 *this, s32 arg); /* the method at 0xB8 */
void func_8007B884(Unk80076F34 *this, s32 arg);
void func_8007BAFC(Unk80076F34 *this, s32 arg);
void func_8007BB64(Unk80076F34 *this, s32 arg);
void func_8007BA98(Unk80076F34 *this, s32 arg);
s32 func_80079E10(Unk80076F34 *this); /* the way the character faces: 1 or -1 */
void func_8007B978(Unk80076F34 *this, s32 arg);
void func_8007B9FC(Unk80076F34 *this, s32 step);

/* returns the Unk80076F34 that func_80073218 gives for the object at 0x228 of `this` */
Unk80076F34 *func_8007AEEC(Unk80076F34 *this);

/* gives the world position (in out) of node `node` of the character's model, moved by offset: it hands
 * both to matrixTransformVec */
VECTOR *func_8007A4A4(Unk80076F34 *this, VECTOR *out, s32 node, VECTOR *offset);

/* play a sound of an Unk80076F34 and return its handle (the first also keeps it at 0x25C): they call
 * soundControlPlaySound and leave $v0 alone after the call, and func_800BFF28 keeps its next constant out
 * of $v0 (a call_value) */
s32 func_800784F4(Unk80076F34 *this, s32 sound);
s32 func_80078540(Unk80076F34 *this, s32 sound);

/* the constructor of the sprite member of Unk800C2F14, and the setup of its motion */
Unk800C2CCC *func_800C2CCC(Unk800C2CCC *this, u16 *frame, s32 flags);
void func_800C3078(Unk800C2F14 *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color);
void func_800C3100(Unk800C2F14 *this, PolyFT4Shape *shape, SVECTOR *corners);

/* the parts classes: their constructors, and the vector setter of Unk8007BD8C */
Unk8007BD8C *func_8007BD8C(Unk8007BD8C *this, Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC);
void func_8007BDD4(Unk8007BD8C *this, VECTOR *vec);

/* the side of the character (0x21C): func_800BF37C picks GAME_STATE's fighter with it */
s32 func_80077574(Unk80076F34 *this);
void func_800A9998(Entity *this, s32 arg);

/* the constructors the functions below call */
Unk80076F34 *func_80076F34(Unk80076F34 *this);
Unk8007F920 *func_800808EC(Unk8007F920 *this, s32 arg);
Entity *func_80082EE8(Entity *this, Unk80076F34 *arg, Vec3 *vel, s32 arg3, s32 side);
void func_800C72D8(Unk800C7024 *this, Unk80075EE4 *sum);
struct Unk8008D21C *func_8008D21C(struct Unk8008D21C *this);
Entity *func_800BD490(Entity *this, Unk80076F34 *arg);
struct Unk800BDA50 *func_800BDA50(struct Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4);
Unk800C2F14 *func_800C2FBC(Unk800C2F14 *this, s32 frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags);
Entity *func_800C3654(Entity *this, Vec3 *unk28, Vec3 *unk40, s32 unk20, u32 color, s32 unk18);
/* An entry of the tables func_800C5680 takes (func_800C5B18 copies up to 4 of them, all but unk10). */
typedef struct Unk800C5B18Entry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ Vec3 unk4;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ CVECTOR color;
} Unk800C5B18Entry;

Entity *func_800C5680(Entity *this, Unk80076F34 *arg, Unk800C5B18Entry *entries, s32 count, s32 arg4, s32 arg5);
extern VtableEntry D_8006C9DC[];
extern EntityVtable D_8006C9FC;
extern EntityVtable D_8006D4FC;
extern EntityVtable D_8006D59C;
extern VtableEntry D_8006D740[];
extern EntityVtable D_8006D760;
extern LoadingScreenVtable D_8006DF3C;

/* A class derived from Unk8007F920 (func_8007F6FC), not identified yet. */
typedef struct Unk8007F6FC {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ u8 unk10[0x30 - 0x10];
    /* 0x30 */ s32 unk30;
} Unk8007F6FC;

void func_8007F6FC(Unk8007F6FC *this);

/* The virtual table of Unk80076F34 (D_8006530C): Entity's four entries, then
 * the class's own methods, which its derived classes override. */
typedef struct Unk80076F34Vtable {
    /* 0x000 */ EntityVtable entity;
    /* 0x020 */ VtableEntry unk20;
    /* 0x028 */ VtableEntry unk28;
    /* 0x030 */ VtableEntry unk30;
    /* 0x038 */ VtableEntry unk38;
    /* 0x040 */ VtableEntry unk40;
    /* 0x048 */ VtableEntry unk48;
    /* 0x050 */ VtableEntry unk50;
    /* 0x058 */ VtableEntry unk58;
    /* 0x060 */ VtableEntry unk60;
    /* 0x068 */ VtableEntry unk68;
    /* 0x070 */ VtableEntry unk70;
    /* 0x078 */ VtableEntry unk78;
    /* 0x080 */ VtableEntry unk80;
    /* 0x088 */ VtableEntry unk88;
    /* 0x090 */ VtableEntry unk90;
    /* 0x098 */ VtableEntry unk98;
    /* 0x0A0 */ VtableEntry unkA0;
    /* 0x0A8 */ VtableEntry unkA8;
    /* 0x0B0 */ VtableEntry unkB0;
    /* 0x0B8 */ VtableEntry unkB8;
    /* 0x0C0 */ VtableEntry unkC0;
    /* 0x0C8 */ VtableEntry unkC8;
    /* 0x0D0 */ VtableEntry unkD0;
    /* 0x0D8 */ VtableEntry unkD8;
    /* 0x0E0 */ VtableEntry unkE0;
    /* 0x0E8 */ VtableEntry unkE8;
    /* 0x0F0 */ VtableEntry unkF0;
    /* 0x0F8 */ VtableEntry unkF8;
    /* 0x100 */ VtableEntry unk100;
    /* 0x108 */ VtableEntry unk108;
    /* 0x110 */ VtableEntry unk110;
    /* 0x118 */ VtableEntry unk118;
    /* 0x120 */ VtableEntry unk120;
    /* 0x128 */ VtableEntry unk128;
    /* 0x130 */ VtableEntry unk130;
    /* 0x138 */ VtableEntry unk138;
} Unk80076F34Vtable;

/* An Unk80076F34's virtual table as its own class sees it. */
#define UNK80076F34_VTABLE(object) ((Unk80076F34Vtable *)(object)->entity.vtable)

/* derived from func_80076F34's class, and the base of func_800C1924's */
typedef struct Unk800BB034 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
} Unk800BB034;

Unk800BB034 *func_800BB034(Unk800BB034 *this);

/* The entity func_800C49E4 constructs (virtual table D_8006D394) for a
 * character's state object, at the character's position. */
typedef struct Unk800C49E4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007F920 *unk18;
    /* 0x1C */ Vec3 position; /* the character's (func_800775D8) */
} Unk800C49E4;

/* The classes func_800AD5BC and func_800893BC construct (virtual tables
 * D_8006B314 and D_80068BAC for their base at 0x18); the methods of
 * D_8006BC34 and D_8006BAE4 (func_800B1FD4, func_800B427C) see the same part. */
typedef struct Unk800AD5BC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x44 - 0x34];
    /* 0x44 */ u32 unk44;
} Unk800AD5BC;

typedef struct Unk800A9944 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ u8 unk80[0xB4 - 0x80];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ u8 unkB8[0xD4 - 0xB8];
    /* 0xD4 */ u32 unkD4; /* a sound's handle (stopSound) */
} Unk800A9944;

/* func_800C7024 constructs an entity for chara at position, moving by
 * velocity; the other four arguments are set by the derived classes. */
Unk800C7024 *func_800C7024(Unk800C7024 *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, s32 unkA8, u32 unk14,
                                s32 unkB4, s32 unk1C);

/* Returns the entity at 0x14 of arg when func_8002BD68 gives 0x10000 for it, else NULL. */
Entity *func_80078878(Unk80075EE4 *sum);

/* The entity class func_800BDA50 constructs for a character (virtual table D_8006C9FC; func_800C2264's
 * class, D_8006CFEC, derives from it): Unk8007BD8C at 0x18, a member of func_80075EE4's class at 0x38. */
typedef struct Unk800BDA50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ s32 unk80; /* the character's entityGetHandle */
    /* 0x84 */ s32 unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ s32 unk90;
    /* 0x94 */ Vec3 unk94; /* func_800BDA50's third argument */
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ Vec3 unkA4; /* 0 from func_800BDA50 */
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8; /* func_800BDA50's fifth argument */
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8; /* func_800BDA50's fourth argument */
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s32 vramY; /* func_8007768C of the character */
} Unk800BDA50;
Entity *func_800BE39C(Entity *this, s32 arg1, Vec3 *position, u32 arg3, s32 arg4, CVECTOR *arg5);

Unk80076444 *func_80076444(Unk80076444 *this);
s32 func_80076474(Unk80076444 *this, Vec3 *a, Vec3 *b);

/* What func_80075000 constructs, one per character (Unk8006ED20.unk24): the
 * character's pad input, kept by func_8007504C every frame. */
typedef struct Unk80075000 {
    /* 0x000 */ u16 history[256]; /* the buttons held, frame by frame */
    /* 0x200 */ u16 held;
    /* 0x202 */ u16 pressed;  /* held now and not the frame before */
    /* 0x204 */ u16 released; /* held the frame before and not now */
    /* 0x206 */ u8 unk206[2];
    /* 0x208 */ s32 count;    /* the next entry of history, 0 to 255 */
    /* 0x20C */ s32 unk20C;
    /* 0x210 */ s32 unk210;
    /* 0x214 */ u32 unk214;   /* PAD_LEFT | PAD_RIGHT or 0: the buttons func_80075574 swaps */
} Unk80075000;

void func_80075034(Unk80075000 *this, s32 swap);
/* the input of a character of the battle */
Unk80075000 *func_80073260(Unk8006ED20 *this, s32 index);
/* the last entry of history, through func_80075574 */
u32 func_800750C4(Unk80075000 *this);
/* the buttons pressed (0x202), through func_80075574 */
u32 func_80075100(Unk80075000 *this);
void func_8007BDF0(Unk8007BD8C *this, Unk80075EE4 *sum);
struct Unk8007C17C *func_8007C17C(struct Unk8007C17C *this, s32 arg);
void func_80078910(void *this, void *arg);
void func_8006EF90(Unk8006ED20 *object, s32 index, s32 add);
void func_80079B2C(Unk80076F34 *this, s32 arg);
/* this->unk334 = arg */
void func_80079B0C(Unk80076F34 *this, s32 arg);
void func_80079320(Unk80076F34 *this);
void func_8007AE34(Unk80076F34 *this);
s32 func_8007F5A8(Unk8007F184 *this);
Unk8007F184 *func_8007758C(Unk80076F34 *this);
void func_8007F1B8(Unk8007F184 *this);
Unk8007F920 *func_800819E0(Unk8007F920 *this);
void func_800C9790(Unk80076F34 *this);
s32 func_800798F4(Unk80076F34 *this, s32 arg);
s32 func_800778D0(Unk80076F34 *this, s32 arg);
void func_800788B4(Unk80076F34 *this, s32 arg1, s32 arg2);
void func_80078848(Unk80076F34 *this, void *arg);
Unk80076F34 *func_80079CF4(Unk80076F34 *this, s32 index);
void func_800C99B0(Unk80076F34 *this);
/* this->unk22C and this->unk230 */
Unk8007F920 *func_800792A0(Unk80076F34 *this);
Unk80075000 *func_800776C4(Unk80076F34 *this); /* returns 0x234 */
void func_8007B2C0(Unk80076F34 *this, s32 arg);
void func_8007B2EC(Unk80076F34 *this, s32 arg);
void func_8007F0F4(Unk8007F184 *this, s32 amount);
void func_8007F1FC(Unk8007F184 *this, s32 amount);
s32 func_8007FB9C(Unk8007F920 *this);
void *func_800807F4(Unk8007F920 *this, s32 arg);
ListNode *func_80075C64(ListNode *head, ListNode *node);

u32 func_8006F004(Unk8006ED20 *this);

void func_8006F024(Unk8006ED20 *this, ListNode *node);

void func_8006F044(Unk8006ED20 *this, ListNode *node);

Entity *func_8006F084(Unk8006ED20 *this, Entity *entity);

Entity *func_8006F0B4(Unk8006ED20 *this, Entity *entity);

EntityGroup *func_8006F0E4(Unk8006ED20 *this, Entity *entity);

s32 func_8006F104(Unk8006ED20 *this);

s32 func_8006F190(Unk8006ED20 *this);

Unk80076F34 *func_800700C8(Unk8006ED20 *this, s32 side);

void func_8007504C(Unk80075000 *this, u32 buttons);
void func_8007F22C(Unk8007F184 *this);

void func_8007A158(Unk80076F34 *chara, Unk80076F34 *other);

void func_80079268(Unk80076F34 *chara);
/* the constructor of the 0x59C-byte entity class Unk800CB7B0 (declared below) */
Entity *func_800CB7B0(void *this, s32 arg1, s32 arg2);

Vec3 *func_800765B4(Unk80076444 *this, Vec3 *out);

s32 func_80072F80(Vec3 *a, Vec3 *b);

void func_80073288(Unk8006ED20 *this);

void func_800732B0(Unk8006ED20 *this);

MATRIX *func_80076598(Unk80076444 *this);

void func_800773F4(Unk80076F34 *this, s32 arg);

void func_80077424(Unk80076F34 *this, s32 arg);

void func_80077448(Unk80076F34 *this, s32 arg);

void func_8007746C(Unk80076F34 *this);

void func_800774C0(Unk80076F34 *this, s32 unk0, s32 unk4, s32 unk8);

void func_80077508(Unk80076F34 *this);

s32 func_80077598(Unk80076F34 *this);

void func_800775B0(Unk80076F34 *chara, Vec3 *in);

s32 func_800777BC(Unk80076F34 *this);

/* Bits of the flags at 0x37C, named after the functions that set them. The USA release has one more flag
 * below them, so each is one bit higher there. */
#if VERSION_JP
#define UNK80076F34_FLAG_A78_BIT 4
#define UNK80076F34_FLAG_AF8_BIT 5
#define UNK80076F34_FLAG_AE4_BIT 6
#else
#define UNK80076F34_FLAG_A78_BIT 5
#define UNK80076F34_FLAG_AF8_BIT 6
#define UNK80076F34_FLAG_AE4_BIT 7
#endif

s32 func_800778BC(Unk80076F34 *this);

s32 func_8007AED0(Unk80076F34 *this);

s32 func_80078DE0(Unk80076F34 *this);

void *func_80079274(Unk80076F34 *this, s32 arg);

s32 func_800792B8(Unk80076F34 *this);

s32 func_800792DC(Unk80076F34 *this);

s32 func_80079A0C(Unk80076F34 *this);

s32 func_800799A0(Unk80076F34 *this);

Unk80076F34 *func_80073218(Unk8006ED20 *this, Unk80076F34 *chara);
s32 func_80079D94(Unk80076F34 *this);
s32 func_80079DD8(Unk80076F34 *this);
Unk80076F34 *func_80079E50(Unk80076F34 *this, VECTOR *arg1, s32 arg2, s32 arg3);

s32 func_80079E2C(Unk80076F34 *this);

void func_8007A0E4(Unk80076F34 *this, s32 arg);

void func_8007A718(Unk80076F34 *this);

Vec3 *func_8007A650(Unk80076F34 *this, Vec3 *out);

Vec3 *func_8007A69C(Unk80076F34 *this, Vec3 *out);

Vec3 *func_8007A6CC(Unk80076F34 *this, Vec3 *out);

void func_8007AD4C(Unk80076F34 *this, Entity *entity);
struct Unk800C5DF4;
struct Unk800C5DF4 *func_800C5DF4(struct Unk800C5DF4 *this, s32 arg1, Vec3 *vec1, Vec3 *vec2, s32 arg4, s32 arg5,
                                  s32 arg6);
struct Unk800C32E0 *func_800C32E0(struct Unk800C32E0 *this, Vec3 *position, VECTOR *dir, s32 unk3, u32 color);

extern EntityVtable D_8006D4D4;

void func_8007B9C8(Unk80076F34 *this, s32 arg);

void func_8007BA30(Unk80076F34 *this, s32 arg);

void func_8007BA64(Unk80076F34 *this, s32 arg);

void func_8007BBE8(Unk80076F34 *this, s32 arg);

void func_8007BC1C(Unk80076F34 *this, s32 arg);

void func_80078EF8(Unk80076F34 *this, u32 unk, Vec3 *velocity, s32 unk3);

void func_8007BDC8(Unk8007BD8C *this, s32 unk8, s32 unkC);

Unk8007BF20 *func_8007BF20(Unk8007BF20 *this, Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC);

Unk8007C084 *func_8007C084(Unk8007C084 *object, s32 unk0);

void func_8007C09C(Unk8007C084 *this, s32 flags);

Unk80075EE4 *func_80075D44(ListNode *list, Vec3 *point, Vec3 *nearest);

Unk800C49E4 *func_800C49E4(Unk800C49E4 *this, Unk8007F920 *owner);

/* The entity class func_800C4F74 constructs for an object of a character (0x4A8 bytes, virtual table
 * D_8006D36C). */
typedef struct Unk800C4F74 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x488 - 0x18];
    /* 0x488 */ s32 unk488;
    /* 0x48C */ u8 unk48C[0x4A8 - 0x48C];
} Unk800C4F74;

Unk800C4F74 *func_800C4F74(Unk800C4F74 *this, Unk8007F920 *owner);
void func_80079B14(Unk80076F34 *this, s32 arg);
void func_80079B1C(Unk80076F34 *this, s32 arg);

void func_80079534(Unk80076F34 *this, Unk80076F34 *previous);

void func_8007516C(Unk80075000 *this, s32 frames);
s32 func_800751A8(Unk80075000 *this, u32 buttons, s32 any);
s32 func_80075454(Unk80075000 *this, u32 buttons, s32 count, s32 any);
void func_800797B8(Unk80076F34 *this, s32 clip);
s32 func_8007F1C0(Unk8007F184 *this);

s32 func_80077848(Unk80076F34 *this);
void func_8007A12C(Unk80076F34 *this, s32 arg);

/* Creates an entity of func_80082EE8's class (0x34 bytes) with g++'s new, and hands it to func_8007AD4C: it
 * launches the Unk80076F34 of func_8007AEEC with vel. The match depends on this helper: its vel parameter keeps
 * &vel in a saved register across allocEntityMemory. */
static inline void newLaunch(Unk80076F34 *this, Vec3 *vel, s32 side) {
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        func_80082EE8(entityOperatorNewAt(0x34, child), this, vel, 0x808C2000, side);
    }
    func_8007AD4C(this, child);
}

void func_800C71DC(Unk800C7024 *this, s32 arg);

Entity *func_800C75FC(Entity *target, Vec3 *arg1, s32 arg2);

extern EntityVtable D_8006D574;

/* The constructor of the grey particle class of vtable D_8006D574, which the game inlines (func_800C28B8,
 * func_800C811C; out-of-line copy func_800C691C): func_800C2FBC on frame, color 0x808080 and no flags. */
static inline void func_8006D574_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    func_800C2FBC((Unk800C2F14 *)this, frame, pos, vel, size, 0x808080, 0);
    this->vtable = &D_8006D574;
}

/* The constructor of the entity class of vtable D_8006D4D4, which the game inlines: func_800C2FBC with frame 8,
 * no velocity (D_80010814), size 0x40, color 0xA0A0A0 and no flags. */
static inline void func_8006D4D4_init(Entity *this, VECTOR *pos) {
    func_800C2FBC((Unk800C2F14 *)this, 8, pos, &D_80010814, 0x40, 0xA0A0A0, 0);
    this->vtable = &D_8006D4D4;
}

/* The character class func_8008D21C constructs (virtual table D_8006938C); func_80091A44's derives
 * from it. */
typedef struct Unk8008D21C {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ u16 unk4A4;
    /* 0x4A6 */ u16 unk4A6;
    /* 0x4A8 */ u16 unk4A8;
    /* 0x4AA */ u16 unk4AA;
    /* 0x4AC */ u16 unk4AC; /* halfword stores in func_8008D2D0 and func_8008DE84 */
    /* 0x4AE */ s16 unk4AE;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
    /* 0x4C4 */ s32 unk4C4;
    /* 0x4C8 */ s32 unk4C8;
    /* 0x4CC */ s32 unk4CC;
    /* 0x4D0 */ Vec3 unk4D0; /* func_8008DD50 hands it to func_80079C1C as the force */
    /* 0x4DC */ s32 unk4DC;
    /* 0x4E0 */ Vec3 unk4E0; /* and this as the velocity */
    /* 0x4EC */ s32 unk4EC;
    /* 0x4F0 */ s32 unk4F0; /* the partner's x when func_8008DE84's step 0 runs */
} Unk8008D21C;

void func_8008D27C(Entity *this, s32 flags);
Unk800C2F14 *func_800C2F14(Unk800C2F14 *this, u16 *frame, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags);

void func_800C3134(Unk800C2F14 *this, s32 arg);

void func_800C73A8(Unk800C7024 *this, s32 arg);

extern VECTOR AXIS_Z;

void func_800C7474(Unk800C7024 *this, Entity *target);

void func_800A9428(Unk800A9944 *this, s32 flags);

/* The entity func_800C5680 constructs for a character (virtual table D_8006D33C, 0x690 bytes): up to 4 entries of
 * a table copied at 0x30 (func_800C5B18). */
typedef struct Unk800C5680 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk80076F34 *chara;
    /* 0x01C */ s32 unk1C; /* -1 from func_800C5680 */
    /* 0x020 */ s32 unk20; /* func_800C5680's fifth argument, at most 32; counted down once unk24 runs out */
    /* 0x024 */ s32 unk24; /* func_800C5680's sixth argument, a timer counted down every frame (func_800C5778) */
    /* 0x028 */ s32 unk28;
    /* 0x02C */ s32 count; /* the entries in use */
    /* 0x030 */ Unk800C5B18Entry entries[4];
    /* 0x090 */ u8 unk90[0x690 - 0x90];
} Unk800C5680; /* size 0x690 */
void func_800C5778(Unk800C5680 *this, Unk80075EE4 *sum);

/* The entity func_800C32E0 constructs (virtual table D_8006D4FC): a sprite with frame 7 of D_8006D5C4, turned the
 * way of its direction. */
typedef struct Unk800C32E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800C2CCC sprite;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
} Unk800C32E0; /* size 0x48 */

Entity *func_800C391C(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4);

s32 func_800C7358(Unk800C7024 *this);

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_CLASSES_H */
