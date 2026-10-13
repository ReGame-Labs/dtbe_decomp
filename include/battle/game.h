#ifndef DTBE_BATTLE_GAME_H
#define DTBE_BATTLE_GAME_H

/* What game.bin, the battle overlay, defines for itself and for the stage overlays (st00-st09) that run on it. */

#include "common.h"
#include "vtable.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/lib/list.h"
#include "engine/math/random.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"

EXTERN_C_BEGIN

/* The object func_8006ED20 constructs: it runs in a thread of its own,
 * keeps itself in D_800D377C, and sets up entity groups, cameras and sprites.
 * The stage overlays derive their classes from it (their constructors call
 * func_8006ED20 and then install their own virtual table at 0xD34). Its size
 * is not known; 0xD38 is the end of what is known. */
typedef struct Unk8006ED20 {
    /* 0x000 */ Thread thread;
    /* 0x00C */ u8 unkC[0x10 - 0xC];
    /* 0x010 */ MeshScene *meshScene; /* the constructor creates it */
    /* 0x014 */ u8 unk14[0x24 - 0x14];
    /* 0x024 */ struct Unk80075000 *unk24[2]; /* by character index; func_80075000 constructs them */
    /* 0x02C */ void *unk2C[2]; /* by side: their 0x8 is a character (func_80071460) */
    /* 0x034 */ u8 unk34[0x3C - 0x34];
    /* 0x03C */ EntityGroup unk3C;
    /* 0x04C */ EntityGroup unk4C;
    /* 0x05C */ EntityGroup unk5C;
    /* 0x06C */ Group unk6C;
    /* 0x0CC */ Group unkCC;
    /* 0x12C */ LookAt lookAt;
    /* 0x180 */ u8 unk180[0x1B0 - 0x180];
    /* 0x1B0 */ Camera camera;
    /* 0x1EC */ u8 unk1EC[0x228 - 0x1EC];
    /* 0x228 */ ListNode unk228;
    /* 0x230 */ ListNode unk230;
    /* 0x238 */ ListNode unk238;
    /* 0x240 */ s32 unk240[4]; /* the constructor clears the four */
    /* 0x250 */ s32 unk250; /* the wins (0x240 by side) that end the fight (func_80071110) */
    /* 0x254 */ s32 unk254; /* the rounds left, counted down by func_80071110 */
    /* 0x258 */ s32 unk258; /* func_8006F1C4's argument */
    /* 0x25C */ u8 unk25C[0x2A4 - 0x25C];
    /* 0x2A4 */ VECTOR unk2A4; /* (0, -0x1000, 0) through func_800769DC (func_8006F1C4) */
    /* 0x2B4 */ u8 unk2B4[0x2B8 - 0x2B4];
    /* 0x2B8 */ s32 unk2B8; /* counted from 0x20 down to -6 (func_80071E10), or from 0 up to 0x26 (func_80071E60) */
    /* 0x2BC */ u8 unk2BC[0x2C8 - 0x2BC];
    /* 0x2C8 */ u32 unk2C8; /* flags */
    /* 0x2CC */ u8 unk2CC[0x314 - 0x2CC];
    /* 0x314 */ s32 unk314;
    /* 0x318 */ u8 unk318[0x31C - 0x318];
    /* 0x31C */ Vec3 eye; /* func_80072088 copies it into lookAt.eye */
    /* 0x328 */ u8 unk328[0x33C - 0x328];
    /* 0x33C */ s32 unk33C; /* func_800700E4 sets 0x33C to 0x35C; 0x33C and 0x34C, 0x340 and 0x350 are read in pairs */
    /* 0x340 */ s32 unk340;
    /* 0x344 */ u8 unk344[0x34C - 0x344];
    /* 0x34C */ s32 unk34C;
    /* 0x350 */ s32 unk350;
    /* 0x354 */ s32 unk354;
    /* 0x358 */ u8 unk358[0x35C - 0x358];
    /* 0x35C */ s32 unk35C;
    /* 0x360 */ MersenneTwister random;
    /* 0xD24 */ s32 steps; /* func_80070C28 counts one per call */
    /* 0xD28 */ s32 unkD28; /* 1 sends st08's stage to its end (func_800D3DF8) */
    /* 0xD2C */ u32 unkD2C; /* flags: bit 0 makes func_80071014 sum func_80070F40's values 3 and 4 */
    /* 0xD30 */ s32 unkD30; /* a character's side (0x21C) + 1 (func_800793A4); func_800794CC clears it;
                             * st02 keeps its camera's state in it (func_800D4AC8) */
    /* 0xD34 */ VtableEntry *vtable;
} Unk8006ED20;

/* the object func_8006ED20 constructed, set by its constructor */
extern Unk8006ED20 *D_800D377C;

/* The base of small objects with a virtual table at 4, which func_8007C084
 * constructs; the stage overlays derive some from it. */
typedef struct Unk8007C084 {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ VtableEntry *vtable;
} Unk8007C084;

/* A class with only a virtual table: D_80065164, an empty method of the
 * executable's and the destructor func_80076308. Many classes, in game.bin and
 * in the stages, derive from it. */
typedef struct Unk80076308 {
    /* 0x0 */ VtableEntry *vtable;
} Unk80076308;

/* the virtual tables of Unk80076308 and Unk8007C084 */
extern VtableEntry D_80065164[];
extern VtableEntry D_800652AC[];

/* The class func_800D02C8 constructs on func_8006ED20's (virtual table
 * D_8006E154); the stage classes of st07, st08 and st09 derive from it. */
typedef struct Unk800D02C8 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ u8 unkD38[0xD9C - 0xD38];
    /* 0xD9C */ s16 unkD9C; /* func_800D0B80 sets 2 */
    /* 0xDA0 */ u32 unkDA0; /* the handle of func_800D0B80's text entity (class 9) */
    /* 0xDA4 */ u32 unkDA4; /* a sound's handle (stopSound) */
} Unk800D02C8;

Unk8006ED20 *func_8006ED20(Unk8006ED20 *object);
void func_8006F254(Unk8006ED20 *object, s32 flags);

/* adds node at the end of the list an Unk8006ED20 keeps at 0x238 */
void func_8006F064(Unk8006ED20 *object, ListNode *node);

/* puts an entity (when not NULL) into the entity group an Unk8006ED20 keeps at 0x4C; returns it */
Entity *func_8006F0B4(Unk8006ED20 *object, Entity *entity);

/* the camera of an Unk8006ED20 (at 0x1B0) and its LookAt (at 0x12C) */
Camera *func_8006EF5C(Unk8006ED20 *object);
LookAt *func_8006EF64(Unk8006ED20 *object);

/* the stage's random number generator */
u32 func_8006F004(Unk8006ED20 *object);

/* add node at the end of the list at 0x228 and at 0x230 (func_8006F064: 0x238) */
void func_8006F024(Unk8006ED20 *object, ListNode *node);
void func_8006F044(Unk8006ED20 *object, ListNode *node);

/* puts an entity (when not NULL) into the entity group at 0x3C; returns it */
Entity *func_8006F084(Unk8006ED20 *object, Entity *entity);

s32 func_8006F144(Unk8006ED20 *object);
s32 func_8006F190(Unk8006ED20 *object);
/* bit 8 of the flags at 0x2C8 */
s32 func_8006F1A4(Unk8006ED20 *object);
MeshScene *func_8006F1B8(Unk8006ED20 *object);

/* the character of a side (0 or 1) */
struct Unk80076F34 *func_800700C8(Unk8006ED20 *object, s32 side);
void func_800700E4(Unk8006ED20 *object, s32 unk33C, s32 unk34C, s32 unk340, s32 unk350, s32 unk354);
void func_80070314(Unk8006ED20 *object);
void func_80071F6C(void);
/* the camera's update */
void func_80072088(Unk8006ED20 *object);

/* whether the segment from from to to hits something, moving to to the hit */
s32 func_80072F80(Vec3 *to, Vec3 *from);

/* integrates velocity by force over mass (divide16), setting move to the mean of the old and new velocity */
void func_80079C1C(Vec3 *move, Vec3 *velocity, Vec3 *force, s32 mass);

Unk800D02C8 *func_800D02C8(Unk800D02C8 *object);
void func_800D02FC(Unk800D02C8 *object, s32 flags);
/* its init and update (the stages call them from theirs) */
void func_800D0364(Unk800D02C8 *object);
void func_800D06D0(Unk800D02C8 *object);
void func_800D0B80(Unk800D02C8 *object);
Unk8007C084 *func_8007C084(Unk8007C084 *object, s32 unk0);
void func_8007C09C(Unk8007C084 *object, s32 flags);

/* a method of func_8006ED20's class (it sits in the stages' virtual tables
 * too): their overrides call it before their own part */
void func_80072748(void *object);

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_H */
