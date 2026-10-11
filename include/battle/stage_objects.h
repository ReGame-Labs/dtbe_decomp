#ifndef DTBE_BATTLE_STAGE_OBJECTS_H
#define DTBE_BATTLE_STAGE_OBJECTS_H

/* Small polymorphic objects of the stages, known only by their layout. */

#include "common.h"
#include "vtable.h"
#include "engine/math/vector.h"
#include "engine/gfx/scene_graph.h"
#include "engine/task/entity.h"
#include "battle/game.h"

EXTERN_C_BEGIN

/* An object whose class derives from the class of game.bin's D_80065164
 * (func_800762F4: nothing but the virtual table): a word and a vector. Each
 * stage has classes of its own with this layout. */
typedef struct UnkStageObject {
    /* 0x0 */ VtableEntry *vtable;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ Vec3 unk8;
} UnkStageObject;

/* An entity of st02 (func_800D7FC0) that keeps a mesh of the stage and the Unk8006ED20 whose entity
 * group holds it. */
typedef struct UnkSt02MeshEntity {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ Unk8006ED20 *owner;
} UnkSt02MeshEntity;

/* An entity of st00 (func_800D7978) with the same layout: the Unk8006ED20 whose entity group holds
 * it, and the word that object keeps at 0xEC4. */
typedef struct UnkSt00Entity {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *owner;
    /* 0x1C */ s32 unk1C;
} UnkSt00Entity;

/* An entity of st02 (func_800D680C) and st03 (func_800D5358): a mesh of the stage and where it is (the
 * translation of the mesh's world matrix, which the stage's func_800D4110 passes). */
typedef struct UnkStageMeshPos {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ Vec3 pos;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
} UnkStageMeshPos; /* size 0x34 */

/* An entity of st07 (func_800D5080) and st09 (func_800D5068), made twice by each stage (index 0 and 1):
 * the four corners of a 128 by 128 square around the origin. */
typedef struct UnkStageQuad {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *owner;
    /* 0x1C */ s16 index;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ SVECTOR corners[4];
    /* 0x44 */ u8 unk44[0x54 - 0x44];
} UnkStageQuad; /* size 0x54 */

/* A 256 by 256 square of the stage at pos: index (0x1C) picks its entry in the owner's tables. Its gather
 * (st07's func_800D572C, st09's func_800D64D0) adds 1 to index and deletes it on the 17th. */
typedef struct UnkStageQuadPos {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *owner;
    /* 0x1C */ u16 index; /* unsigned: the gathers compare it with andi 0xFFFF and sltiu */
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ SVECTOR corners[4];
    /* 0x40 */ Vec3 pos;
    /* 0x4C */ s32 unk4C;
} UnkStageQuadPos; /* size 0x50 */

EXTERN_C_END

#endif /* DTBE_BATTLE_STAGE_OBJECTS_H */
