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

/* A hit of a stage (st00, st01, st04): derived from CollisionOwner, like game.bin's Hit but without its owner and
 * kind; its slot 0x08 (StageHit_apply) adds its damage and knockback into the sphere it touches, as Hit_apply does
 * (st01's adds nothing). st05's class has this layout too (St05Hit). */
typedef struct StageHit {
    /* 0x0 */ VtableEntry *vtable;
    /* 0x4 */ s32 damage; /* HIT_DAMAGE_MASK, with flags above it */
    /* 0x8 */ Vec3 knockback;
} StageHit;

/* st02's entity that draws one mesh node of the scenery (St02MeshDrawer_init), kept with the Fight whose entity
 * group holds it. */
typedef struct St02MeshDrawer {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ Fight *owner;
} St02MeshDrawer;

/* st00's entity that draws the stage's Group (St00GroupDrawer_init): the Fight whose entity group holds it, and
 * the MeshScene the stage keeps at 0xEC4. */
typedef struct St00GroupDrawer {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ MeshScene *meshScene;
} St00GroupDrawer;

/* The mesh movers of st02 (St02MeshMover_init) and st03 (St03MeshMover_init): a mesh of the stage and where it is
 * (the translation of the mesh's world matrix, which the stage's createNodeEntity passes); the tick moves the mesh
 * by motion. */
typedef struct StageMeshMover {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ Vec3 pos;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 motion; /* st02: the angle about z, 0xC0 more each tick; st03: the height added to pos */
    /* 0x30 */ s32 frame;  /* its ticks */
} StageMeshMover; /* size 0x34 */

/* The thrower of a side in st07 (BonusThrower_init) and st09: each stage makes two (side 0 and 1); a 128 by 128
 * square drawn at position, scaled while the side holds its button. Its tick puts it at its fighter
 * (BonusThrower_place) and throws when the button is let go (BonusThrower_throw). */
typedef struct BonusThrower {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ u16 side;  /* unsigned: st09's BonusThrower_throw indexes with lhu */
    /* 0x1E */ s16 frame; /* its ticks */
    /* 0x20 */ s32 scale; /* the draw's matrixScale: eases to 0x1000 while held (st07), the hold * 0xCC (st09) */
    /* 0x24 */ SVECTOR corners[4];
    /* 0x44 */ Vec3 position; /* where it is drawn, and where st09's BonusThrower_throw throws from */
    /* 0x50 */ u8 unk50[0x54 - 0x50];
} BonusThrower; /* size 0x54 */

/* The mark a thrown object leaves where it lands (BonusLandingMark_init, st07 and st09): a 512 by 512 square at
 * pos. Its tick counts frame and deletes it after BONUS_LANDING_MARK_FRAMES. */
typedef struct BonusLandingMark {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ u16 frame; /* unsigned: the ticks compare it with andi 0xFFFF and sltiu */
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ SVECTOR corners[4];
    /* 0x40 */ Vec3 pos;
    /* 0x4C */ s32 unk4C;
} BonusLandingMark; /* size 0x50 */

/* The bonus games' hold: the frames a side's button charges up to (st07 and st09; BonusFight.gaugeMax there). */
#define BONUS_CHARGE_FRAMES 30

/* A BonusLandingMark's ticks: it is deleted on the one after them. */
#define BONUS_LANDING_MARK_FRAMES 16

EXTERN_C_END

#endif /* DTBE_BATTLE_STAGE_OBJECTS_H */
