#ifndef DTBE_BATTLE_GAME_H
#define DTBE_BATTLE_GAME_H

/* What game.bin, the battle overlay, defines for itself and for the stage overlays (st00-st09) that run on it. */

#include "common.h"
#include "vtable.h"
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
    /* 0x00C */ u8 unkC[0x24 - 0xC];
    /* 0x024 */ struct Unk80075000 *unk24[2]; /* by character index; func_80075000 constructs them */
    /* 0x02C */ void *unk2C[2]; /* by side: their 0x8 is a character (func_80071460) */
    /* 0x034 */ u8 unk34[0x3C - 0x34];
    /* 0x03C */ EntityGroup unk3C;
    /* 0x04C */ EntityGroup unk4C;
    /* 0x05C */ EntityGroup unk5C;
    /* 0x06C */ u8 unk6C[0x228 - 0x6C];
    /* 0x228 */ ListNode unk228;
    /* 0x230 */ ListNode unk230;
    /* 0x238 */ ListNode unk238;
    /* 0x240 */ s32 unk240[4]; /* the constructor clears the four */
    /* 0x250 */ u8 unk250[0x2C8 - 0x250];
    /* 0x2C8 */ u32 unk2C8; /* flags */
    /* 0x2CC */ u8 unk2CC[0x360 - 0x2CC];
    /* 0x360 */ MersenneTwister random;
    /* 0xD24 */ u8 unkD24[0xD34 - 0xD24];
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
    /* 0xD38 */ u8 unkD38[0xDA4 - 0xD38];
    /* 0xDA4 */ u32 unkDA4; /* a sound's handle (stopSound) */
} Unk800D02C8;

Unk8006ED20 *func_8006ED20(Unk8006ED20 *object);
void func_8006F254(Unk8006ED20 *object, s32 flags);

/* adds node at the end of the list an Unk8006ED20 keeps at 0x238 */
void func_8006F064(Unk8006ED20 *object, ListNode *node);

/* puts an entity (when not NULL) into the entity group an Unk8006ED20 keeps at 0x4C; returns it */
Entity *func_8006F0B4(Unk8006ED20 *object, Entity *entity);

void func_800D02FC(Unk800D02C8 *object, s32 flags);
Unk8007C084 *func_8007C084(Unk8007C084 *object, s32 unk0);
void func_8007C09C(Unk8007C084 *object, s32 flags);

/* a method of func_8006ED20's class (it sits in the stages' virtual tables
 * too): their overrides call it before their own part */
void func_80072748(void *object);

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_H */
