#ifndef DTBE_BATTLE_GAME_CLASSES_H
#define DTBE_BATTLE_GAME_CLASSES_H

/* Classes of game.bin known only by their layout so far, named after their
 * constructors, and the functions several of its source files share. */

#include "common.h"
#include "vtable.h"
#include <libgpu.h>
#include "battle/game.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"
#include "engine/game/loading_screen.h"

EXTERN_C_BEGIN

/* A state of a fighter (FighterState_init, virtual table FIGHTER_STATE_VTABLE): the fighter keeps two, its body
 * state and its attack state (Fighter.bodyState / attackState), and switches one by running another state's
 * constructor on the same memory. Slots: 0x08 tick (the fighter's tick), 0x10 jump, 0x18 drop, 0x20 attack (its
 * handleInput), 0x28 hit (the hit applier, Fighter_takeHit). */
typedef struct FighterState {
    /* 0x0 */ s32 state;              /* FIGHTER_STATE_* */
    /* 0x4 */ s32 frame;              /* handed to the fighter's slot of the state, then counted up */
    /* 0x8 */ struct Fighter *fighter;
    /* 0xC */ VtableEntry *vtable;
} FighterState;

/* A face of a fighter (Fighter_setFaceTable gives the table, Fighter_setFace picks one): the frame buffer
 * copies that draw it (FaceMove, in fighter.c) and their count. */
typedef struct FaceEntry {
    /* 0x0 */ void *moves;
    /* 0x4 */ s32 count;
} FaceEntry;

/* An attack of a fighter's params, as Fighter_runAttack runs it. */
typedef struct Attack {
    /* 0x00 */ s32 parts;            /* the attack bits: the model parts that hit (Fighter_getPartMask) */
    /* 0x04 */ s16 firstActiveFrame; /* the frames the parts hit in */
    /* 0x06 */ s16 lastActiveFrame;
    /* 0x08 */ s32 lungeX;           /* the impulse at its first frame, by the facing */
    /* 0x0C */ s32 lungeY;
    /* 0x10 */ s32 drag;             /* vx * drag >> 12 each frame */
    /* 0x14 */ s32 cancelFrame;      /* the next attack may chain after it */
    /* 0x18 */ s32 knockbackX;
    /* 0x1C */ s32 knockbackY;
    /* 0x20 */ s32 hitDamage;        /* the damage (HIT_DAMAGE_MASK) and the hit's flags */
} Attack;

/* A character's params, its row of FIGHTER_PARAMS (0x180 bytes; the code uses up to attacks[8]). */
typedef struct FighterParams {
    /* 0x00 */ u16 flags;      /* FIGHTER_PARAM_* */
    /* 0x02 */ s16 height;     /* the point above the position; 3/4 of it is the radius */
    /* 0x04 */ s32 mass;       /* 16.16, copied to Fighter.mass */
    /* 0x08 */ s32 walkSpeed;
    /* 0x0C */ s32 dashSpeed;
    /* 0x10 */ s32 jumpSpeed;
    /* 0x14 */ u16 stunLimit;  /* the side's stun count it takes to be dizzy */
    /* 0x16 */ u16 unk16;
    /* 0x18 */ s32 airSideClip; /* the clip of Fighter_airSideAttack */
    /* 0x1C */ u16 gaugeGains[16]; /* by MOVE_*: the evolution gauge a move gives when it hits */
    /* 0x3C */ Attack attacks[9]; /* attack1-3, dash, up, crouch, air up, air side, air attack */
} FighterParams;

/* FighterParams.flags */
#define FIGHTER_PARAM_EVOLVED 0x1 /* does not evolve: clear for exactly the ten characters that do */

/* the params of the characters, by CHARACTER_* */
extern FighterParams FIGHTER_PARAMS[];

/* One side of the fight (BattleSide_init, virtual table BATTLE_SIDE_VTABLE at 0x38; Fight.sides): its starting
 * fighter, that fighter's evolution (itself when it has none), the one fighting, and what both forms share: the
 * side's life, evolution gauge and stun (Fighter.battleSide). */
typedef struct BattleSide {
    /* 0x00 */ struct Fighter *fighter;   /* the starting form */
    /* 0x04 */ struct Fighter *evolution; /* its evolution (CHARACTER_EVOLUTIONS), or fighter itself */
    /* 0x08 */ struct Fighter *current;   /* the one fighting (BattleSide_swapCharacter) */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;                 /* 30 from func_8007F730 */
    /* 0x14 */ s32 swapDelay;             /* frames until the forms swap (BattleSide_tick); func_8007F7F4 sets 16 */
    /* 0x18 */ s32 unk18;                 /* 3 from func_8007F730 */
    /* 0x1C */ s32 lifeMax;
    /* 0x20 */ s32 life;                  /* the fighter is knocked out at 0 */
    /* 0x24 */ s32 gauge;                 /* the evolution gauge, 0 to 0x10000 (full) */
    /* 0x28 */ s32 stun;                  /* hits add to it, it wears off; at FighterParams.stunLimit, dizzy */
    /* 0x2C */ s32 frozen;                /* set by BattleSide_freeze: life, gauge and stun stop changing */
    /* 0x30 */ s32 unk30;                 /* set by func_8007F6FC (an evolution asked for), read by BattleSide_tick */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ VtableEntry *vtable;
} BattleSide;

/* An effect a fighter runs for a time (two of them: the item effect at 0x464 and the one at 0x478):
 * updateItemEffect counts the timer down, clearing the whole when it reaches 0, and switches on the kind. */
typedef struct TimedEffect {
    /* 0x00 */ s32 kind;  /* 1 to 6 (the items' effects) */
    /* 0x04 */ s32 timer; /* frames */
    /* 0x08 */ s32 power; /* the 1/256 damage scale of kinds 1 and 2 */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
} TimedEffect;

/* A collision line of the mesh scene (MeshSceneLink.lines, 0x10 each; initTerrainLine builds one): two
 * endpoints, then the line equation normalX * x + normalY * y + offset. */
typedef struct TerrainLine {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
    /* 0x8 */ s16 normalX;
    /* 0xA */ s16 normalY;
    /* 0xC */ s32 offset;
} TerrainLine;

void addCollisionLink(MeshSceneLink *link, s32 *lines, s32 lineCount);
void initTerrainLine(TerrainLine *this, DVECTOR *from, DVECTOR *to);

/* Where a segment crosses the collision lines of the shown COLLISION_LINKS (TerrainProbe_cast); the fighter
 * keeps one for its floor (Fighter.floorProbe). */
typedef struct TerrainProbe {
    /* 0x00 */ TerrainLine line;  /* the line hit (castTerrainLink copies it) */
    /* 0x10 */ s32 startDistance; /* the line distances of the two ends (getCrossingPoint) */
    /* 0x14 */ s32 endDistance;
    /* 0x18 */ VECTOR localStart; /* the ends in the link's space (vz cleared) */
    /* 0x28 */ VECTOR localEnd;
    /* 0x38 */ VECTOR localHit;   /* the crossing point */
    /* 0x48 */ VECTOR start;      /* the segment, in world space */
    /* 0x58 */ VECTOR end;
    /* 0x68 */ VECTOR normal;     /* the hit line's, rotated by its link's matrix (func_80078254 dots it, vecDot12) */
    /* 0x78 */ MeshSceneLink *link; /* the link of COLLISION_LINKS hit last */
} TerrainProbe;

/* the list of the mesh scene links that have collision lines (COLLISION_LINKS) */
ListNode *getCollisionLinks(void);

/* CollisionShape's flags */
#define COLLISION_SHAPE_DISABLED 0x1 /* collideShapes skips it; the debug draw tints it */
#define COLLISION_SHAPE_TOUCHED 0x2  /* collideShapes ORs in whether it touched another */

/* A collision sphere (virtual table COLLISION_SPHERE_VTABLE): its first 0x34 bytes are the CollisionShape it
 * derives from (the same fields), then a position and a radius; CollisionSphere_overlaps is its test. The hits
 * that touch it (Hit_apply, and st02's St02Hit_apply, st00's StageHit_apply) add into its hit fields; the fighter
 * keeps one as its collider (Fighter.collider, getHitFighter finds the entity of the hit). */
typedef struct CollisionSphere {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *owner;       /* its CollisionOwner: a Hit in game.bin, a StageHit in st00 */
    /* 0x0C */ u32 flags;         /* COLLISION_SHAPE_* */
    /* 0x10 */ s32 unk10;         /* the other's must be 0 for an overlap (a kind of shape?) */
    /* 0x14 */ Entity *entity;    /* the entity it belongs to (CollisionShape_setEntity) */
    /* 0x18 */ u32 hitDamage;     /* the damage of the hits added (HIT_DAMAGE_MASK, saturating) and their flags */
    /* 0x1C */ s32 hitKind;       /* the sum of their kinds */
    /* 0x20 */ Vec3 hitKnockback; /* the sum of their knockbacks */
    /* 0x2C */ s32 hitKnockbackPad;
    /* 0x30 */ VtableEntry *vtable;
    /* 0x34 */ Vec3 position;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 radius;
} CollisionSphere;

/* A move's pay when it hits (Fighter.rewards, by MOVE_*): score points to the side and evolution gauge. */
typedef struct MoveReward {
    /* 0x0 */ u16 score;
    /* 0x2 */ u16 gauge;
} MoveReward;

/* The fighter of a side: the base class of the 24 fighters (virtual table FIGHTER_VTABLE, 0x4A4 bytes; the
 * fighters put their own fields from 0x4A4 on). createFighter builds one by CHARACTER_*; a side that can evolve
 * has two, the selected form and its evolution, which share the side object. */
typedef struct Fighter {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 bodyStateStorage[0x100];   /* the body state object, built in place (bodyState points here) */
    /* 0x118 */ u8 attackStateStorage[0x100]; /* the attack state object, the same way */
    /* 0x218 */ s32 character;                /* CHARACTER_*: its params, its tamer, its sounds */
    /* 0x21C */ s32 side;                     /* 0 or 1: its pad port, its sound group, its VRAM half */
    /* 0x220 */ FighterParams *params;
    /* 0x224 */ FaceEntry *faceTable;
    /* 0x228 */ Fight *fight;
    /* 0x22C */ FighterState *bodyState;
    /* 0x230 */ FighterState *attackState;
    /* 0x234 */ struct InputHistory *input;
    /* 0x238 */ BattleSide *battleSide;      /* the side object, shared by both forms of the side */
    /* 0x23C */ u8 unk23C[0x240 - 0x23C];
    /* 0x240 */ Entity *holder;               /* the entity of holderHandle (Fighter_attachToPart) */
    /* 0x244 */ u32 holderHandle;             /* the fighter the model is attached to (held), or 0 */
    /* 0x248 */ u32 unk248;                   /* the handle of the entity func_80079420 makes */
    /* 0x24C */ s32 clipBlendFrames;        /* the blend frames clip started with */
    /* 0x250 */ s32 clip;                     /* the movement clip (FIGHTER_CLIP_*), or negative */
    /* 0x254 */ s32 actionClip;               /* the one-shot clip played over it, or negative */
    /* 0x258 */ s32 pendingPose;              /* FIGHTER_STATE_END_POSE or FIGHTER_STATE_WIN_POSE */
    /* 0x25C */ s32 voiceHandle;              /* the sound Fighter_playVoice played */
    /* 0x260 */ TerrainProbe floorProbe;       /* the map below the fighter */
    /* 0x2DC */ CollisionSphere collider;         /* what the hits add into */
    /* 0x324 */ s32 bodyBoxes;                /* Box[], the parts' boxes (Fighter_updateBoxes) */
    /* 0x328 */ s32 bodyBoxCount;
    /* 0x32C */ s32 attackBoxes;              /* Box[], the boxes of the parts that hit */
    /* 0x330 */ s32 attackBoxCount;
    /* 0x334 */ s32 unk334;
    /* 0x338 */ s32 unk338;
    /* 0x33C */ s32 unk33C;
    /* 0x340 */ s32 comboHits;
    /* 0x344 */ u8 unk344[0x348 - 0x344];
    /* 0x348 */ MoveReward rewards[13];       /* by MOVE_*, Fighter_setMoveReward */
    /* 0x37C */ u32 flags;                    /* FIGHTER_FLAG_* */
    /* 0x380 */ s32 airMovesUsed;             /* bits of the air moves made since the fighter left the ground */
    /* 0x384 */ Vec3 position;                /* in 1/256ths */
    /* 0x390 */ u8 unk390[4];
    /* 0x394 */ Vec3 lastPosition;            /* before the last move (Fighter_move) */
    /* 0x3A0 */ u8 unk3A0[4];
    /* 0x3A4 */ Vec3 velocity;                /* added to the position every step */
    /* 0x3B0 */ s32 velocityPad;
    /* 0x3B4 */ Vec3 force;                   /* force / mass is added to the velocity every step, then cleared */
    /* 0x3C0 */ s32 forcePad;
    /* 0x3C4 */ s32 mass;                     /* 16.16, the params' */
    /* 0x3C8 */ VECTOR unk3C8;                /* func_800A1160 interpolates it (Patamon) */
    /* 0x3D8 */ Vec3 rotation;                /* Fighter_updateRotation turns it toward targetRotation */
    /* 0x3E4 */ u8 unk3E4[0x3E8 - 0x3E4];
    /* 0x3E8 */ Vec3 targetRotation;          /* its vy is the facing: 0x401 right, -0x401 left */
    /* 0x3F4 */ u8 unk3F4[0x3F8 - 0x3F4];
    /* 0x3F8 */ Chara *chara;                 /* the model's files */
    /* 0x3FC */ Model *model;                 /* Fighter_setUp makes it with modelInit */
    /* 0x400 */ Lights lights;                /* the camera's (Fight_lightFighter) */
    /* 0x440 */ s32 vramY;                    /* 0x100 for side 1: the VRAM half of its textures */
    /* 0x444 */ s32 face;                     /* the FaceEntry to draw, ~it once drawn */
    /* 0x448 */ u32 attackParts;              /* the model parts that hit now (Fighter_updateBoxes) */
    /* 0x44C */ s32 hitDamage;                /* the hit the attack lands (Fighter_hitOpponent), the Attack's */
    /* 0x450 */ s32 hitKind;
    /* 0x454 */ Vec3 hitKnockback;
    /* 0x460 */ s32 hitKnockbackPad;
    /* 0x464 */ TimedEffect itemEffect;       /* updateItemEffect runs it */
    /* 0x478 */ TimedEffect unk478;           /* func_800C99B0 runs it */
    /* 0x48C */ s32 unk48C;
    /* 0x490 */ u8 unk490[0x494 - 0x490];
    /* 0x494 */ s32 unk494;                   /* func_8007A12C sets it to 5 or clears it */
    /* 0x498 */ s32 invulnerableFrames;       /* after getting up or a respawn (Fighter_isInvulnerable) */
    /* 0x49C */ s32 unk49C;
    /* 0x4A0 */ s32 unk4A0;                   /* MetalGarurumon_airSpecial1 sets it to 1 */
} Fighter;

/* the damage bits of a hit word; its flags are above them (bits 27 and 28: the guards that do not block it) */
#define HIT_DAMAGE_MASK 0x3FFFF

/* Fighter.flags (the US build has one more bit above these two) */
#define FIGHTER_FLAG_COMPUTER 0x1  /* GAME_STATE's side is the computer */
#define FIGHTER_FLAG_ON_GROUND 0x4

/* The states of a fighter: its body state and its attack state keep one each (Fighter_getBodyStateId,
 * Fighter_getAttackStateId); each sets it and calls its slot of FIGHTER_VTABLE every frame. */
enum FighterStateId {
    FIGHTER_STATE_INTRO = 0,
    FIGHTER_STATE_IDLE = 1,
    FIGHTER_STATE_CROUCH = 2,
    FIGHTER_STATE_JUMP = 3,
    FIGHTER_STATE_FALL = 4,
    FIGHTER_STATE_WALK_LEFT = 5,
    FIGHTER_STATE_WALK_RIGHT = 6,
    FIGHTER_STATE_DASH_LEFT = 7,
    FIGHTER_STATE_DASH_RIGHT = 8,
    FIGHTER_STATE_GUARD = 9,
    FIGHTER_STATE_CROUCH_GUARD = 0xA,
    FIGHTER_STATE_FLINCH = 0xB,
    FIGHTER_STATE_AIR_FLINCH = 0xC,
    FIGHTER_STATE_KNOCKED_DOWN = 0xD,
    FIGHTER_STATE_KNOCKED_OUT = 0xE,
    FIGHTER_STATE_GET_UP = 0xF,
    FIGHTER_STATE_DIZZY = 0x10,
    FIGHTER_STATE_END_POSE = 0x11,
    FIGHTER_STATE_WIN_POSE = 0x12,
    FIGHTER_STATE_HELD = 0x13,
    FIGHTER_STATE_ATTACK1 = 0x14,
    FIGHTER_STATE_ATTACK2 = 0x15,
    FIGHTER_STATE_ATTACK3 = 0x16,
    FIGHTER_STATE_UP_ATTACK = 0x17,
    FIGHTER_STATE_THROW = 0x18,
    FIGHTER_STATE_CROUCH_ATTACK = 0x19,
    FIGHTER_STATE_DASH_ATTACK = 0x1A,
    FIGHTER_STATE_AIR_UP_ATTACK = 0x1B,
    FIGHTER_STATE_AIR_ATTACK = 0x1C, /* neutral and side */
    FIGHTER_STATE_SPECIAL1 = 0x1D,
    FIGHTER_STATE_AIR_SPECIAL1 = 0x1E,
    FIGHTER_STATE_SPECIAL2 = 0x1F,
    FIGHTER_STATE_AIR_SPECIAL2 = 0x20,
    FIGHTER_STATE_SPECIAL3 = 0x21,
    FIGHTER_STATE_AIR_SPECIAL3 = 0x22
};

/* The clips of a fighter's model, the same for every character (the slots play them). */
enum FighterClip {
    FIGHTER_CLIP_STAND = 0,
    FIGHTER_CLIP_WALK = 1,
    FIGHTER_CLIP_DASH = 2,
    FIGHTER_CLIP_JUMP = 3,
    FIGHTER_CLIP_FALL = 4,
    FIGHTER_CLIP_ATTACK1 = 5,
    FIGHTER_CLIP_ATTACK2 = 6,
    FIGHTER_CLIP_ATTACK3 = 7,
    FIGHTER_CLIP_DASH_ATTACK = 8,
    FIGHTER_CLIP_AIR_ATTACK = 9,
    FIGHTER_CLIP_AIR_UP_ATTACK = 0xA,
    FIGHTER_CLIP_THROW = 0xB,
    FIGHTER_CLIP_GUARD = 0xC,
    FIGHTER_CLIP_FLINCH = 0xD,
    FIGHTER_CLIP_AIR_FLINCH = 0xE,
    FIGHTER_CLIP_KNOCKED_DOWN = 0x10,
    FIGHTER_CLIP_DOWN = 0x11,
    FIGHTER_CLIP_GET_UP = 0x12,
    FIGHTER_CLIP_CROUCH = 0x13,
    FIGHTER_CLIP_CROUCH_ATTACK = 0x14,
    FIGHTER_CLIP_CROUCH_GUARD = 0x15,
    FIGHTER_CLIP_SPECIAL1 = 0x16,
    FIGHTER_CLIP_AIR_SPECIAL1 = 0x17,
    FIGHTER_CLIP_SPECIAL2 = 0x18,
    FIGHTER_CLIP_AIR_SPECIAL2 = 0x19,
    FIGHTER_CLIP_SPECIAL3 = 0x1A,
    FIGHTER_CLIP_AIR_SPECIAL3 = 0x1B,
    FIGHTER_CLIP_WIN_POSE = 0x1C,
    FIGHTER_CLIP_END_POSE = 0x1D,
    FIGHTER_CLIP_INTRO = 0x1E,
    FIGHTER_CLIP_UP_ATTACK = 0x1F
};

/* The moves that pay (Fighter.rewards, FighterParams.gaugeGains); the score each pays is in parentheses. */
enum Move {
    MOVE_ATTACK1 = 0,   /* (100) */
    MOVE_ATTACK2 = 1,   /* (100) */
    MOVE_ATTACK3 = 2,   /* (200) */
    MOVE_THROW = 3,     /* (400) */
    MOVE_UP = 4,        /* (200) */
    MOVE_CROUCH = 5,    /* (100) */
    MOVE_DASH = 6,      /* (300) */
    MOVE_AIR_UP = 7,    /* (400) */
    MOVE_AIR = 8,       /* (400) */
    MOVE_AIR_SIDE = 9,  /* (400) */
    MOVE_SPECIAL1 = 10, /* (100) */
    MOVE_SPECIAL2 = 11, /* (1000) */
    MOVE_SPECIAL3 = 12  /* (3000) */
};

/* adds amount to the side's 16.16 evolution gauge, kept between 0 and 1.0 */
void Fighter_addGauge(Fighter *this, s32 amount);
/* the fighter's CHARACTER_* */
s32 Fighter_getCharacter(Fighter *this);
/* sets the position (kept in 1/256ths) from whole units */
void Fighter_setPosition(Fighter *fighter, Vec3 *position);
/* the position in whole units */
Vec3 *Fighter_getPosition(Fighter *fighter, Vec3 *out);
/* the fighter's params */
FighterParams *Fighter_getParams(Fighter *this);
/* whether the fighter stands on the stage part that moves with matrix */
s32 Fighter_isStandingOn(Fighter *fighter, MATRIX *matrix);
void Fighter_requestEvolution(Fighter *this);
Vec3 *func_8007A650(Fighter *this, Vec3 *out);

/* Entity classes that keep the Fighter they were created for, named
 * after their constructors (the ones with the same layout share a struct). */
typedef struct Unk800AFD78 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *unk1C;
    /* 0x20 */ s32 unk20[4];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C[8];
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 vramY; /* Fighter_getVramY of the character */
} Unk800AFD78;

/* The entity func_800B01AC constructs (virtual table D_8006B624); func_800B050C
 * constructs another class (D_8006B5FC) with the same fields. Their ticks move
 * 0x24 down by 0x120 for 24 (16) calls, then delete them. */
typedef struct Unk800B01AC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 ticks; /* ticks so far */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s16 vramY; /* Fighter_getVramY of the character */
} Unk800B01AC;
typedef Unk800B01AC Unk800B050C;

/* Terriermon's special move 1 effect; Seraphimon's special move 2 effect has the same layout. */
typedef struct TerriermonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* the character's (Fighter_getPosition), followed every tick */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 ticks; /* ticks so far */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s16 vramY; /* Fighter_getVramY of the character */
} TerriermonSpecial1Effect;
typedef TerriermonSpecial1Effect SeraphimonSpecial2Effect;

/* The entity func_800C3CFC constructs at a position (0x54 bytes, virtual table D_8006D40C); func_800CAEF0's class
 * derives from it with no fields of its own, and the class func_8008E9B4 and func_8008EA58 derive from it, which keeps
 * the Fighter it was created for (Imperialdramon_special3Hold spawns them). */
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
    /* 0x54 */ Fighter *unk54;
} Unk8008E9B4;

/* A text drawn glyph by glyph (the battle's messages): hudTextSetText copies the text (at most 0x40 characters,
 * with strlen and strncpy) and keeps its length; every character has an entry of its own, whose size it sets to
 * 0x20, its color to 0x80808080 and the rest to 0; the hudTextSet functions set one field of every character (or
 * of a range), and hudTextDraw draws them. */
typedef struct HudTextChar {
    /* 0x00 */ s16 dx; /* added to the character's place, scaled by the text's scale */
    /* 0x02 */ s16 dy;
    /* 0x04 */ s16 w; /* the glyph's size, 0x20 at first, scaled */
    /* 0x06 */ s16 h;
    /* 0x08 */ CVECTOR color; /* 0x80808080 at first; a cd of 0 draws nothing */
    /* 0x0C */ s32 palette;   /* clut row 0x1E9 + palette */
    /* 0x10 */ s32 unk10;     /* only func_800CD534 sets it */
} HudTextChar;

#define HUD_TEXT_MAX 0x40

typedef struct HudText {
    /* 0x000 */ HudTextChar chars[HUD_TEXT_MAX];
    /* 0x500 */ char text[HUD_TEXT_MAX];
    /* 0x540 */ s32 length;
    /* 0x544 */ s32 step;  /* between two characters (BattleMessage_drawCentered centres the text by it) */
    /* 0x548 */ s32 scale; /* 4.12 fixed point (0x1000 is 1): the offsets, sizes and step are multiplied by it */
} HudText;

/* An entity of game.bin that keeps copies of two 16-color palettes of VRAM (func_800D1F8C, func_800D23C4) */
typedef struct Unk800D1F8C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ RECT clutRects[2];
    /* 0x28 */ u16 cluts[2][16];
    /* 0x68 */ s16 unk68;
    /* 0x6A */ s16 unk6A;
    /* 0x6C */ Fight *owner;
} Unk800D1F8C; /* size 0x70 */

/* A particle emitter (func_800C6088, ParticleBurst_init, ParticleImplosion_init): spawns count particles of
 * createParticle's kind around position, up to spread away (count > 0: one a frame; < 0: all at once). */
typedef struct ParticleEmitter {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 kind;   /* createParticle's */
    /* 0x1C */ VECTOR position;
    /* 0x2C */ s32 spread; /* the random speed (burst) or shell radius (implosion) */
    /* 0x30 */ s32 size;
    /* 0x34 */ s32 count;
    /* 0x38 */ s32 color;
} ParticleEmitter; /* size 0x3C */

/* the constructors of the emitters: count particles of kind around pos, up to spread away, with size and color */
ParticleEmitter *func_800C6088(ParticleEmitter *this, s32 kind, Vec3 *pos, s32 spread, s32 size, s32 count, s32 color);
ParticleEmitter *ParticleBurst_init(ParticleEmitter *this, s32 kind, Vec3 *pos, s32 spread, s32 size, s32 count,
                                    s32 color);

/* A particle's sprite animation (Particle 0x18): spriteAnimInit keeps its row of PARTICLE_ANIMS and the 0x100 bit
 * of the flags, spriteAnimRewind sets the first cell, spriteAnimStep the next one along the strip, and
 * spriteAnimFillShape makes a PolyFT4Shape of it. */
typedef struct SpriteAnim {
    /* 0x00 */ u16 *anim;  /* u16[10]: x, y, cell w, cell h, strip width, cell count, step mask, 0, clut, tpage */
    /* 0x04 */ s16 flags;
    /* 0x06 */ s16 angle;
    /* 0x08 */ s32 size;   /* half the side of the square func_8009F120 draws */
    /* 0x0C */ u32 color;
    /* 0x10 */ VECTOR position;
    /* 0x20 */ s16 left, top, right, bottom;
} SpriteAnim; /* size 0x28 */

/* The shape buildPolyFT4 projects (engine/gfx/prim/build_line_g2.h takes it as void *): four corners, a color
 * and the uv, clut and tpage words of the POLY_FT4; spriteAnimFillShape fills it from an SpriteAnim */
typedef struct PolyFT4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 color;
    /* 0x18 */ u16 uv0, clut;
    /* 0x1C */ u16 uv1, tpage;
    /* 0x20 */ u16 uv2, pad22;
    /* 0x24 */ u16 uv3, pad26;
} PolyFT4Shape; /* size 0x28 */

/* A sprite particle (Particle_init, virtual table PARTICLE_VTABLE): steps its animation, drifts by its velocity
 * with 1/16 drag, and deletes itself after the animation's cells. */
typedef struct Particle {
    /* 0x00 */ Entity entity;
    /* 0x18 */ SpriteAnim sprite;
    /* 0x40 */ s16 cell; /* the cells stepped (Particle_stepAnim) */
    /* 0x42 */ s16 unk42;
    /* 0x44 */ Vec3 velocity;
    /* 0x50 */ s32 velocityPad;
    /* 0x54 */ s32 age;
} Particle; /* size 0x58 */

/* A class derived from Particle that keeps its owner (func_8009F248, func_8009F4D4) */
typedef struct Unk8009F248 {
    /* 0x00 */ Particle base;
    /* 0x58 */ Fighter *owner;
} Unk8009F248; /* size 0x5C */

/* the fighters with fields of their own past Fighter's (src/battle/fighters/) */
typedef struct Gabumon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
} Gabumon;

/* Patamon's class (CHARACTER_PATAMON). */
typedef struct Patamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 special1Active; /* set while the special move 1's hitbox lives (its constructor, destructor) */
    /* 0x4A8 */ u8 unk4A8[0x4B8 - 0x4A8];
    /* 0x4B8 */ s32 unk4B8; /* 0 to 0x1000: how far base.unk3C8 is from unk4CC to unk4DC */
    /* 0x4BC */ u8 unk4BC[0x4CC - 0x4BC];
    /* 0x4CC */ VECTOR unk4CC;
    /* 0x4DC */ VECTOR unk4DC;
} Patamon;

/* Tailmon's class (CHARACTER_TAILMON). */
typedef struct Tailmon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s16 unk4A4;
} Tailmon;

/* Terriermon's class (CHARACTER_TERRIERMON): special2 counts 0x4A4 down between its hitboxes. */
typedef struct Terriermon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
} Terriermon;

/* Wormmon's class (CHARACTER_WORMMON). */
typedef struct Wormmon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s16 guardEffectActive; /* set by guard and crouchGuard with the effect, cleared by its destructor */
} Wormmon;

/* The parts that many entity classes share: a base class or member that
 * Hit starts, and a member CollisionSphere. */

/* The hit an attack part carries (virtual table HIT_VTABLE, Hit_init): its owner, its damage, the reaction it causes
 * and a knockback; when its part touches a target, Hit_apply (slot 0x08) adds it into the target's collider. It derives
 * from the class of COLLISION_OWNER_VTABLE (CollisionOwner, CollisionOwner_init), which has nothing but the virtual
 * table. Entities that derive from it keep it at 0x18: the virtual tables of that base move `this` back by 0x18 to
 * reach them. */
typedef struct Hit {
    /* 0x00 */ VtableEntry *vtable;
    /* 0x04 */ struct Fighter *owner; /* the spawners pass the Fighter that owns them */
    /* 0x08 */ s32 damage;            /* HIT_DAMAGE_MASK, with flags above it */
    /* 0x0C */ s32 kind;              /* the reaction (0-7) */
    /* 0x10 */ Vec3 knockback;
} Hit;

/* the virtual table of Hit */
extern VtableEntry HIT_VTABLE[];

/* A Hit that records what it struck (virtual table TRACKED_HIT_VTABLE, TrackedHit_init clears the flags):
 * flags bit 0 a fighter (ENTITY_KIND_FIGHTER), bit 1 a fighter's object (ENTITY_KIND_FIGHTER_OBJECT). */
typedef struct TrackedHit {
    /* 0x00 */ Hit base;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ u32 flags;
} TrackedHit;

/* A shape of the fight's collision lists (virtual table COLLISION_SHAPE_VTABLE at 0x30, CollisionShape_init):
 * collideShapes calls slot 0x08 (overlaps) of each pair, shapeListDraw slot 0x18 (draw). Its one subclass,
 * CollisionSphere, spells the same fields out. */
typedef struct CollisionShape {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *owner;    /* its CollisionOwner: collideShapes calls its slot 0x08 with the other shape */
    /* 0x0C */ u32 flags;      /* COLLISION_SHAPE_*: the constructors clear both */
    /* 0x10 */ s32 unk10;
    /* 0x14 */ void *entity;   /* the entity it belongs to (CollisionShape_setEntity) */
    /* 0x18 */ u8 unk18[0x30 - 0x18]; /* CollisionSphere's hit fields */
    /* 0x30 */ VtableEntry *vtable;
} CollisionShape;

/* the virtual table of CollisionShape, which the destructors that inline its
 * destructor install last; the one of Hit's base is COLLISION_OWNER_VTABLE
 * (battle/game.h) */
extern VtableEntry COLLISION_SHAPE_VTABLE[];

/* Constructs a sphere of owner (a CollisionOwner) at position with radius. */
CollisionSphere *CollisionSphere_init(CollisionSphere *this, Vec3 *position, s32 radius, void *owner);

/* An entity that derives from Hit too and holds a CollisionSphere. */
typedef struct UnkPartsEntity38 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit base;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ CollisionSphere part;
} UnkPartsEntity38;

/* A projectile (Projectile_init, virtual tables PROJECTILE_VTABLE and PROJECTILE_HIT_VTABLE): an entity of kind
 * 0x30000 with an attack and a hit part (UnkPartsEntity38) that flies by its velocity until it has gone its range.
 * Nine fighter classes and the item projectile derive from it (AgumonSpecial1Projectile_init's adds 0xC0). */
typedef struct Projectile {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Vec3 position;      /* copied to the hit part every frame */
    /* 0x8C */ s32 positionPad;
    /* 0x90 */ Vec3 velocity;
    /* 0x9C */ s32 velocityPad;
    /* 0xA0 */ s32 hitLevel;  /* bits 24-25 of the attack word: the burst it spawns (Projectile_getBurstKind) */
    /* 0xA4 */ s32 radius;    /* copied to the hit part */
    /* 0xA8 */ s32 maxRadius;
    /* 0xAC */ s32 speed;     /* the velocity's length */
    /* 0xB0 */ s32 distance;  /* gone so far (+= speed) */
    /* 0xB4 */ s32 range;
    /* 0xB8 */ s32 age;
    /* 0xBC */ s32 unkBC; /* Fighter_getVramY of the owner */
} Projectile; /* size 0xC0 */

/* MetalGarurumon's special move 2 projectile (a Projectile); its special move 3's func_8009CE8C derives from it. */
typedef struct MetalGarurumonSpecial2Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
} MetalGarurumonSpecial2Projectile;

/* An entity that holds a TrackedHit and a CollisionSphere. */
typedef struct UnkPartsEntity3C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ TrackedHit unk18;
    /* 0x3C */ CollisionSphere part;
} UnkPartsEntity3C;

/* The class that func_80097FA0 constructs (vtable D_80069A7C): an
 * UnkPartsEntity3C with a Fighter and a vector it hands to what it creates. */
typedef struct Unk80097FA0 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Fighter *unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ Vec3 unk8C;
} Unk80097FA0;

/* The classes func_800ACA60 and func_800AC470 construct (0xB0 bytes): parts that move from position by velocity
 * every frame and, after 12 frames, spawn two entities and delete themselves. */
typedef struct Unk800ACA60 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *owner;
    /* 0x84 */ Fight *unk84;
    /* 0x88 */ Vec3 position;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 velocity;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 frames;
    /* 0xAC */ s16 vramY; /* Fighter_getVramY of the owner */
    /* 0xAE */ s16 unkAE;
} Unk800ACA60;
typedef Unk800ACA60 Unk800AC470;

/* The class func_800AD050 constructs (virtual tables D_8006B3C4, and D_8006B3A4 for its base at 0x18),
 * like Unk800ACA60 with more: the owner's position when made and some counters. */
typedef struct Unk800AD050 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *owner;
    /* 0x84 */ Fight *list; /* the part goes into one of its two lists (Fighter_getSide picks which) */
    /* 0x88 */ Vec3 position;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 fighterPosition; /* the owner's when made (Fighter_getPosition) */
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ Vec3 velocity;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ Vec3 acceleration; /* (0, 10, 0): it falls */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 frames;
    /* 0xCC */ s16 vramY; /* Fighter_getVramY of the owner */
} Unk800AD050;

/* Gokumon's special move 1 hitbox (0xB0 bytes): it plays sound 0x111 and keeps the handle. */
typedef struct GokumonSpecial1Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *owner;
    /* 0x84 */ VECTOR unk84; /* Fighter_getNodePosition of the owner's point 5 */
    /* 0x94 */ s32 unk94;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ s32 unk9C;
    /* 0xA0 */ s32 unkA0;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ u32 soundHandle;
} GokumonSpecial1Hitbox;

/* Gokumon's special move 3 hitbox (0x2AC bytes): it plays a sound and keeps the handle last. */
typedef struct GokumonSpecial3Hitbox {
    /* 0x000 */ UnkPartsEntity38 parts;
    /* 0x080 */ u8 unk80[0x280 - 0x80];
    /* 0x280 */ Fighter *owner;
    /* 0x284 */ u8 unk284[0x2A8 - 0x284];
    /* 0x2A8 */ u32 soundHandle;
} GokumonSpecial3Hitbox;

/* the destructor of an entity class that others derive from */
void Fighter_destroy(Entity *this, s32 flags);

/* ends the item's effect */
void Fighter_clearItemEffect(Fighter *this);

/* the fight the fighter is in: the one whose entities Fighter_addEntity adds to */
Fight *Fighter_getScene(Fighter *this);

/* the VRAM row of the fighter's textures */
s32 Fighter_getVramY(Fighter *this);

/* gives a fighter its faces */
void Fighter_setFaceTable(Fighter *this, FaceEntry *table);

/* virtual functions of Fighter, and functions of its that they call */
s32 Fighter_canEvolve(Fighter *this);
void Fighter_accelerateTo(Fighter *this, s32 speed);
void Fighter_faceLeft(Fighter *this, s32 now);
void Fighter_faceRight(Fighter *this, s32 now);
/* Plays a clip of the fighter's model over the movement clip (a negative clip stops the model):
 * plays < 0 takes the count from the character's .loop file (charaGetClipPlays). */
void Fighter_playActionClip(Fighter *this, s32 clip, s32 blendFrames, s32 frame, s32 plays);
/* Sets the movement clip (FIGHTER_CLIP_*), with the play count of the .loop file; it keeps clip and
 * blendFrames, and starts the clip only when Fighter_isActionClipDone allows it. */
void Fighter_setClip(Fighter *this, s32 clip, s32 blendFrames, s32 frame);
void func_80079AE4(Fighter *this);
void Fighter_spawnDust(Fighter *this, s32 dir, s32 arg2, s32 count);
void Fighter_dashLeft(Fighter *this, s32 frame);
void Fighter_dashRight(Fighter *this, s32 frame);
void Fighter_runAttack(Fighter *this, s32 frame, Attack *attack, s32 kind);
void Fighter_attack1(Fighter *this, s32 frame);
void Fighter_attack2(Fighter *this, s32 frame);
void Fighter_attack3(Fighter *this, s32 frame);
void Fighter_upAttack(Fighter *this, s32 frame);
void Fighter_dashAttack(Fighter *this, s32 frame);
void Fighter_airUpAttack(Fighter *this, s32 frame);
void Fighter_airSideAttack(Fighter *this, s32 frame);
void Fighter_airAttack(Fighter *this, s32 frame);
s32 Fighter_getFacing(Fighter *this); /* the way the character faces: 1 or -1 */
void Fighter_throw(Fighter *this, s32 frame);
void Fighter_special2(Fighter *this, s32 frame);

/* the fighter's opponent (Fight_getOpponent) */
Fighter *Fighter_getOpponent(Fighter *this);

/* gives the world position (in out) of node `node` of the character's model, moved by offset: it hands
 * both to matrixTransformVec */
VECTOR *Fighter_getNodePosition(Fighter *this, VECTOR *out, s32 node, VECTOR *offset);

/* play a sound of a fighter and return its handle (the first also keeps it as voiceHandle): they call
 * soundControlPlaySound and leave $v0 alone after the call, and Gokumon_special3 keeps its next constant out
 * of $v0 (a call_value) */
s32 Fighter_playVoice(Fighter *this, s32 sound);
s32 Fighter_playSound(Fighter *this, s32 sound);

/* the constructor of a particle's sprite animation, and the setup of its motion */
SpriteAnim *spriteAnimInit(SpriteAnim *this, u16 *anim, s32 flags);
void Particle_setMotion(Particle *this, VECTOR *pos, VECTOR *vel, s32 size, u32 color);
void Particle_buildShape(Particle *this, PolyFT4Shape *shape, SVECTOR *corners);

/* the parts classes: their constructors, and the vector setter of Hit */
Hit *Hit_init(Hit *this, Fighter *owner, s32 damage, Vec3 *knockback, s32 kind);
void Hit_setKnockback(Hit *this, VECTOR *vec);

/* the fighter's side: Gokumon_hasAltColor picks GAME_STATE's side setup with it */
s32 Fighter_getSide(Fighter *this);
void func_800A9998(Entity *this, s32 arg);

/* the constructors the functions below call */
Fighter *Fighter_init(Fighter *this);
FighterState *FighterStateGround_init(FighterState *this, s32 arg);
Entity *ThrowLaunch_init(Entity *this, Fighter *arg, Vec3 *vel, s32 arg3, s32 side);
void Projectile_hit(Projectile *this, CollisionSphere *sum);
struct Imperialdramon *Imperialdramon_init(struct Imperialdramon *this);
Entity *func_800BD490(Entity *this, Fighter *arg);
struct Unk800BDA50 *func_800BDA50(struct Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4);
Particle *Particle_initById(Particle *this, s32 anim, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags);
Entity *Sparkle_init(Entity *this, Vec3 *unk28, Vec3 *unk40, s32 unk20, u32 color, s32 unk18);
/* A point of the model a trail follows (Trail_init's table; Trail_setPoints copies up to 4, all but the pad): a
 * model node and an offset from it (Fighter_getNodePosition), and its color. */
typedef struct TrailPoint {
    /* 0x00 */ s32 node;
    /* 0x04 */ Vec3 offset;
    /* 0x10 */ s32 offsetPad;
    /* 0x14 */ CVECTOR color;
} TrailPoint;

Entity *Trail_init(Entity *this, Fighter *arg, TrailPoint *entries, s32 count, s32 arg4, s32 arg5);
extern VtableEntry D_8006C9DC[];
extern EntityVtable D_8006C9FC;
extern EntityVtable HIT_SPARK_VTABLE;
extern EntityVtable PARTICLE_VTABLE;
extern VtableEntry PROJECTILE_HIT_VTABLE[];
extern EntityVtable PROJECTILE_VTABLE;
extern LoadingScreenVtable FIGHT_LOADING_SCREEN_VTABLE;


void func_8007F6FC(BattleSide *this);

/* The virtual table of Fighter (FIGHTER_VTABLE): Entity's four entries, then the class's own methods, which the
 * fighters override. From handleInput on, each slot is what its FIGHTER_STATE_* runs (and the clip it plays).
 * The table ends with a zero entry at 0x120; only the classes of fighters 21 and 23 put methods of their own
 * there (0x120 and 0x128), so their vtable types declare them. */
typedef struct FighterVtable {
    /* 0x000 */ EntityVtable entity; /* tick, draw, destroy */
    /* 0x020 */ VtableEntry endMove;
    /* 0x028 */ VtableEntry hitOpponent;
    /* 0x030 */ VtableEntry handleInput;
    /* 0x038 */ VtableEntry intro;
    /* 0x040 */ VtableEntry getUp;
    /* 0x048 */ VtableEntry walkLeft;
    /* 0x050 */ VtableEntry walkRight;
    /* 0x058 */ VtableEntry dashLeft;
    /* 0x060 */ VtableEntry dashRight;
    /* 0x068 */ VtableEntry jump;
    /* 0x070 */ VtableEntry crouch;
    /* 0x078 */ VtableEntry guard;
    /* 0x080 */ VtableEntry crouchGuard;
    /* 0x088 */ VtableEntry dizzy;
    /* 0x090 */ VtableEntry flinch;
    /* 0x098 */ VtableEntry airFlinch;
    /* 0x0A0 */ VtableEntry attack1;
    /* 0x0A8 */ VtableEntry attack2;
    /* 0x0B0 */ VtableEntry attack3;
    /* 0x0B8 */ VtableEntry upAttack;
    /* 0x0C0 */ VtableEntry dashAttack;
    /* 0x0C8 */ VtableEntry crouchAttack;
    /* 0x0D0 */ VtableEntry throw;
    /* 0x0D8 */ VtableEntry special1;
    /* 0x0E0 */ VtableEntry special2;
    /* 0x0E8 */ VtableEntry special3;
    /* 0x0F0 */ VtableEntry airSpecial3;
    /* 0x0F8 */ VtableEntry airAttack;
    /* 0x100 */ VtableEntry airUpAttack;
    /* 0x108 */ VtableEntry airSideAttack;
    /* 0x110 */ VtableEntry airSpecial1;
    /* 0x118 */ VtableEntry airSpecial2;
} FighterVtable;

/* A fighter's virtual table as the base class sees it. */
extern FighterVtable FIGHTER_VTABLE;

#define GET_FIGHTER_VTABLE(object) ((FighterVtable *)(object)->entity.vtable)

/* WarGreymon's class (CHARACTER_WARGREYMON), also the base of BlackWarGreymon's. */
typedef struct WarGreymon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
} WarGreymon;

WarGreymon *WarGreymon_init(WarGreymon *this);

/* The flash of an evolution (TransformFlash_init, virtual table TRANSFORM_FLASH_VTABLE), for a side
 * (func_8007F7A0) at its fighter's position: in three phases its height eases from the fighter's to its other
 * form's (CHARA_OTHER_FORM), a burst follows, then it widens and fades. */
typedef struct TransformFlash {
    /* 0x00 */ Entity entity;
    /* 0x18 */ BattleSide *side;
    /* 0x1C */ Vec3 position; /* the fighter's (Fighter_getPosition) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 height;       /* eases to targetHeight */
    /* 0x30 */ s32 targetHeight; /* the other form's */
    /* 0x34 */ s32 radius;
    /* 0x38 */ s32 maxRadius;
    /* 0x3C */ s32 alpha;
    /* 0x40 */ s32 phase;        /* 0 to 2 */
    /* 0x44 */ s32 frame;
} TransformFlash; /* size 0x48 */

/* The classes func_800AD5BC and GuilmonSpecial2Hitbox_init construct (virtual tables D_8006B314 and
 * GUILMON_SPECIAL2_HITBOX_HIT_VTABLE for their base at 0x18); StingmonSpecial2Hitbox_hit and
 * StingmonAttack3Hitbox_hit see the same part. */
typedef struct Unk800AD5BC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ u8 unk34[0x44 - 0x34];
    /* 0x44 */ u32 unk44;
} Unk800AD5BC;

typedef struct Unk800A9944 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ CollisionSphere unk38;
    /* 0x80 */ u8 unk80[0xB4 - 0x80];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ u8 unkB8[0xD4 - 0xB8];
    /* 0xD4 */ u32 unkD4; /* a sound's handle (stopSound) */
} Unk800A9944;

/* Projectile_init constructs an entity for fighter at position, moving by
 * velocity; the other four arguments are set by the derived classes. */
Projectile *Projectile_init(Projectile *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, s32 unkA8, u32 unk14,
                            s32 unkB4, s32 unk1C);

/* Returns the entity at 0x14 of arg when entityGetKind gives 0x10000 for it, else NULL. */
Entity *getHitFighter(CollisionSphere *sum);

/* The entity class func_800BDA50 constructs for a character (virtual table D_8006C9FC; func_800C2264's
 * class, D_8006CFEC, derives from it): Hit at 0x18, a member of CollisionSphere_init's class at 0x38. */
typedef struct Unk800BDA50 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ CollisionSphere unk38;
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
    /* 0xD0 */ s32 vramY; /* Fighter_getVramY of the character */
} Unk800BDA50;
Entity *func_800BE39C(Entity *this, s32 arg1, Vec3 *position, u32 arg3, s32 arg4, CVECTOR *arg5);

TerrainProbe *TerrainProbe_init(TerrainProbe *this);
s32 TerrainProbe_cast(TerrainProbe *this, Vec3 *start, Vec3 *end);

/* A side's pad input, one per side (Fight.inputs): the buttons of the last 256 frames, kept by
 * InputHistory_record every frame, and the back searches the fighter states read commands with. */
typedef struct InputHistory {
    /* 0x000 */ u16 history[256]; /* the buttons held, frame by frame */
    /* 0x200 */ u16 held;
    /* 0x202 */ u16 pressed;  /* held now and not the frame before */
    /* 0x204 */ u16 released; /* held the frame before and not now */
    /* 0x206 */ u8 unk206[2];
    /* 0x208 */ s32 count;    /* the next entry of history, 0 to 255 */
    /* 0x20C */ s32 searchIndex;  /* the entry the next search starts at (InputHistory_startSearch) */
    /* 0x210 */ s32 searchFrames; /* the frames it may still look back */
    /* 0x214 */ u32 mirrorButtons; /* PAD_LEFT | PAD_RIGHT or 0: the buttons InputHistory_mirror swaps */
} InputHistory;

void InputHistory_setMirrored(InputHistory *this, s32 swap);
/* the input history of a side */
InputHistory *Fight_getInput(Fight *this, s32 side);
/* the last entry of history, through InputHistory_mirror */
u32 InputHistory_getHeld(InputHistory *this);
/* the buttons pressed, through InputHistory_mirror */
u32 InputHistory_getPressed(InputHistory *this);
void Hit_apply(Hit *this, CollisionSphere *sum);
struct ComputerController *ComputerController_init(struct ComputerController *this, struct BattleSide *side);
void Fighter_handleInput(void *this, void *input);
void Fight_addScore(Fight *object, s32 index, s32 add);
void Fighter_setFace(Fighter *this, s32 face);
/* this->unk334 = arg */
void func_80079B0C(Fighter *this, s32 arg);
void Fighter_stopActionClip(Fighter *this);
void Fighter_detach(Fighter *this);
s32 BattleSide_canEvolve(BattleSide *this);
BattleSide *func_8007758C(Fighter *this);
void func_8007F1B8(BattleSide *this);
FighterState *FighterAttackIdle_init(FighterState *this);
void updateItemEffect(Fighter *this);
s32 Fighter_lacksClip(Fighter *this, s32 clip);
s32 Fighter_tryUseAirMove(Fighter *this, s32 move);
void Fighter_setMoveReward(Fighter *this, s32 move, s32 score);
void Fighter_mirrorInput(Fighter *this, void *input);
Fighter *Fighter_getOpponentInReach(Fighter *this, s32 index);
void func_800C99B0(Fighter *this);
/* the body state (the attack state: Fighter_getAttackState, in fighter.c) */
FighterState *Fighter_getBodyState(Fighter *this);
InputHistory *Fighter_getInput(Fighter *this);
void Fighter_guard(Fighter *this, s32 frame);
void Fighter_crouchGuard(Fighter *this, s32 frame);
void BattleSide_addLife(BattleSide *this, s32 amount);
void BattleSide_addGauge(BattleSide *this, s32 amount);
s32 FighterState_getClip(FighterState *this);
void *FighterStateHeld_init(FighterState *this, s32 arg);
ListNode *listAppend(ListNode *head, ListNode *node);

void InputHistory_record(InputHistory *this, u32 buttons);
void BattleSide_tickGauges(BattleSide *this);

void func_8007A158(Fighter *fighter, Fighter *other);

void func_80079268(Fighter *fighter);
/* The battle's messages (BattleMessage_init's kind): each indexes BATTLE_MESSAGE_LAYOUTS, _TICKS and _DRAWS. */
enum BattleMessageKind {
    BATTLE_MESSAGE_READY_FIGHT,    /* the round's call, READY? then FIGHT! */
    BATTLE_MESSAGE_NEW_CHALLENGER, /* a second player joins */
    BATTLE_MESSAGE_EVOLUTION,
    BATTLE_MESSAGE_KO,
    BATTLE_MESSAGE_YOU_WIN,
    BATTLE_MESSAGE_YOU_LOSE,
    BATTLE_MESSAGE_P1_WIN, /* P1_WIN + side: keep P2_WIN next to it */
    BATTLE_MESSAGE_P2_WIN,
    BATTLE_MESSAGE_TIME_UP,
    BATTLE_MESSAGE_DRAW_GAME,
    BATTLE_MESSAGE_DOUBLE_KO,
    BATTLE_MESSAGE_GAME_OVER,
    BATTLE_MESSAGE_DEMONSTRATION
};

/* Constructs a battle message (a 0x59C-byte BattleMessage, hud.c) of a BATTLE_MESSAGE_* kind; character picks
 * the EVOLUTION message's palette. */
Entity *BattleMessage_init(void *this, s32 kind, s32 character);

Vec3 *TerrainProbe_getHitPoint(TerrainProbe *this, Vec3 *out);

void Fight_freezeFighters(Fight *this);

void Fight_unfreezeFighters(Fight *this);

void Fight_holdClock(Fight *this);

MATRIX *TerrainProbe_getMatrix(TerrainProbe *this);

void Fighter_addScore(Fighter *this, s32 points);

void Fighter_addLife(Fighter *this, s32 amount);

void Fighter_evolveIfReady(Fighter *this);

void Fighter_startItemEffect(Fighter *this, s32 kind, s32 timer, s32 power);

void func_80077508(Fighter *this);

s32 Fighter_isComputer(Fighter *this);

s32 func_800777BC(Fighter *this);

/* Bits of Fighter.flags, named after the functions that set them. The USA release has one more flag
 * below them, so each is one bit higher there. */
#if VERSION_JP
#define FIGHTER_FLAG_FROZEN_BIT 4
#define FIGHTER_FLAG_80079AF8_BIT 5
#define FIGHTER_FLAG_80079AE4_BIT 6
#else
#define FIGHTER_FLAG_FROZEN_BIT 5
#define FIGHTER_FLAG_80079AF8_BIT 6
#define FIGHTER_FLAG_80079AE4_BIT 7
#endif

s32 Fighter_isOnGround(Fighter *this);

s32 Fighter_isAttackIdle(Fighter *this);

s32 Fighter_isGuarding(Fighter *this);

void *Fighter_hold(Fighter *this, s32 arg);

s32 Fighter_getBodyStateId(Fighter *this);

s32 Fighter_getAttackStateId(Fighter *this);

s32 Fighter_isActionClipDone(Fighter *this);

s32 Fighter_isClipDone(Fighter *this);

Fighter *Fight_getOpponent(Fight *this, Fighter *fighter);
s32 Fighter_isThrowable(Fighter *this);
s32 func_80079DD8(Fighter *this);
Fighter *func_80079E50(Fighter *this, VECTOR *arg1, s32 arg2, s32 arg3);

s32 func_80079E2C(Fighter *this);

void Fighter_setAttackParts(Fighter *this, s32 parts);

void func_8007A718(Fighter *this);

Vec3 *Fighter_getOpponentPosition(Fighter *this, Vec3 *out);

Vec3 *func_8007A6CC(Fighter *this, Vec3 *out);

void Fighter_addEntity(Fighter *this, Entity *entity);
struct ParticleSpray;
struct ParticleSpray *ParticleSpray_init(struct ParticleSpray *this, s32 kind, Vec3 *pos, Vec3 *vel, s32 size,
                                         s32 count, s32 color);
struct HitSpark *HitSpark_init(struct HitSpark *this, Vec3 *position, VECTOR *dir, s32 unk3, u32 color);

extern EntityVtable D_8006D4D4;

void Fighter_special1(Fighter *this, s32 arg);

void Fighter_special3(Fighter *this, s32 arg);

void Fighter_airSpecial3(Fighter *this, s32 arg);

void Fighter_airSpecial1(Fighter *this, s32 arg);

void Fighter_airSpecial2(Fighter *this, s32 arg);

void Fighter_takeHit(Fighter *this, u32 unk, Vec3 *velocity, s32 unk3);

void Hit_set(Hit *this, s32 damage, s32 kind);

TrackedHit *TrackedHit_init(TrackedHit *this, Fighter *owner, s32 damage, Vec3 *knockback, s32 kind);

CollisionSphere *shapeListFindNearest(ListNode *list, Vec3 *point, Vec3 *nearest);

TransformFlash *TransformFlash_init(TransformFlash *this, BattleSide *owner);

/* The entity class func_800C4F74 constructs for a side (0x4A8 bytes, virtual table D_8006D36C; func_8007F7F4
 * makes one when an evolved form's gauge runs out). */
typedef struct Unk800C4F74 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x488 - 0x18];
    /* 0x488 */ s32 unk488;
    /* 0x48C */ u8 unk48C[0x4A8 - 0x48C];
} Unk800C4F74;

Unk800C4F74 *func_800C4F74(Unk800C4F74 *this, BattleSide *owner);
void func_80079B14(Fighter *this, s32 arg);
void func_80079B1C(Fighter *this, s32 arg);

void Fighter_takeOverFrom(Fighter *this, Fighter *previous);

void InputHistory_startSearch(InputHistory *this, s32 frames);
s32 InputHistory_findHeld(InputHistory *this, u32 buttons, s32 any);
s32 InputHistory_findPresses(InputHistory *this, u32 buttons, s32 count, s32 any);
void func_800797B8(Fighter *this, s32 clip);
s32 func_8007F1C0(BattleSide *this);

s32 Fighter_probeFloorBelow(Fighter *this);
void func_8007A12C(Fighter *this, s32 arg);

/* Creates an entity of ThrowLaunch_init's class (0x34 bytes) with g++'s new, and hands it to Fighter_addEntity: it
 * launches the Fighter of Fighter_getOpponent with vel. The match depends on this helper: its vel parameter keeps
 * &vel in a saved register across allocEntityMemory. */
static inline void newLaunch(Fighter *this, Vec3 *vel, s32 side) {
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        ThrowLaunch_init(entityOperatorNewAt(0x34, child), this, vel, 0x808C2000, side);
    }
    Fighter_addEntity(this, child);
}

void Projectile_tick(Projectile *this, s32 arg);

Entity *createHitSpark(Entity *target, Vec3 *arg1, s32 arg2);
void createItemSpawner(Fight *this, void *arg1, s32 arg2);

extern EntityVtable SPIN_PARTICLE_VTABLE;

/* The constructor of the grey particle class of vtable SPIN_PARTICLE_VTABLE, which the game inlines (createParticle,
 * func_800C811C; out-of-line copy SpinParticle_initGrey): Particle_initById on frame, color 0x808080 and no flags. */
static inline void func_8006D574_init(Entity *this, VECTOR *pos, VECTOR *vel, s32 size, s32 frame) {
    Particle_initById((Particle *)this, frame, pos, vel, size, 0x808080, 0);
    this->vtable = &SPIN_PARTICLE_VTABLE;
}

/* The constructor of the entity class of vtable D_8006D4D4, which the game inlines: Particle_initById with frame 8,
 * no velocity (ZERO_VECTOR), size 0x40, color 0xA0A0A0 and no flags. */
static inline void func_8006D4D4_init(Entity *this, VECTOR *pos) {
    Particle_initById((Particle *)this, 8, pos, &ZERO_VECTOR, 0x40, 0xA0A0A0, 0);
    this->vtable = &D_8006D4D4;
}

/* Imperialdramon's class (CHARACTER_IMPERIALDRAMON); ImperialdramonPaladinMode's is the same class with its own
 * vtable. The fields from 0x4A6 are the special move 3's hold (Imperialdramon_special3Hold). */
typedef struct Imperialdramon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ u16 unk4A4;          /* frame 0 of the hold clears it; while set the hold does not carry; set when a
                                     * special 3 hitbox hits an unguarded fighter (ImperialdramonSpecial3Hitbox_hit) */
    /* 0x4A6 */ u16 special3Parts;   /* the attack parts of the hold's current hit (Fighter_setAttackParts) */
    /* 0x4A8 */ u16 holding;         /* set when the hold grabs the opponent (Fighter_hold) */
    /* 0x4AA */ u16 hitPending;      /* set by the hold's hits, cleared once their sparks are spawned */
    /* 0x4AC */ u16 special3Active;  /* set every frame of the hold; endMove lets the opponent go while it is set */
    /* 0x4AE */ s16 unk4AE;
    /* 0x4B0 */ s32 unk4B0;
    /* 0x4B4 */ s32 unk4B4;
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
    /* 0x4C0 */ s32 unk4C0;
    /* 0x4C4 */ s32 unk4C4;
    /* 0x4C8 */ s32 unk4C8;
    /* 0x4CC */ s32 unk4CC;
    /* 0x4D0 */ Vec3 carryForce;     /* func_8008DD50 hands it to func_80079C1C as the force */
    /* 0x4DC */ s32 unk4DC;
    /* 0x4E0 */ Vec3 carryVelocity;  /* and this as the velocity */
    /* 0x4EC */ s32 unk4EC;
    /* 0x4F0 */ s32 opponentStartX;  /* the opponent's x at the hold's frame 0 */
} Imperialdramon;

void Imperialdramon_destroy(Entity *this, s32 flags);
Particle *Particle_init(Particle *this, u16 *anim, VECTOR *pos, VECTOR *vel, s32 size, u32 color, s32 flags);

void Particle_tick(Particle *this, s32 arg);

void Projectile_spawnBurst(Projectile *this, s32 arg);

extern VECTOR AXIS_Z;

void Projectile_spawnHitSpark(Projectile *this, Entity *target);

void func_800A9428(Unk800A9944 *this, s32 flags);

/* A sample of a trail: where one of its points was, and its color then. */
typedef struct TrailSample {
    /* 0x0 */ s16 x, y, z, pad;
    /* 0x8 */ CVECTOR color;
} TrailSample; /* size 0xC */

/* A fighter's motion trail (Trail_init, virtual table TRAIL_VTABLE, 0x690 bytes): while recordTimer runs it
 * records up to 4 model points a frame into rings of 32 samples, their colors fading along it; then it shrinks
 * and dies. */
typedef struct Trail {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Fighter *fighter;
    /* 0x01C */ s32 head;        /* the ring index last recorded; -1 from Trail_init */
    /* 0x020 */ s32 length;      /* the samples drawn, at most 32; counted down once recordTimer runs out */
    /* 0x024 */ s32 recordTimer; /* counted down every frame (Trail_tick) */
    /* 0x028 */ s32 validCount;  /* the samples recorded since the last jump (Trail_record) */
    /* 0x02C */ s32 pointCount;  /* the points in use */
    /* 0x030 */ TrailPoint points[4];
    /* 0x090 */ TrailSample samples[4][32];
} Trail; /* size 0x690 */

/* The virtual table of Trail (TRAIL_VTABLE): Entity's four entries, then the strips' draw. */
typedef struct TrailVtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry drawStrips; /* (Trail *, Camera *): the POLY_G4 strips */
} TrailVtable;
void Trail_tick(Trail *this, CollisionSphere *sum);

/* The entity HitSpark_init constructs (virtual table HIT_SPARK_VTABLE): a sprite with frame 7 of PARTICLE_ANIMS, turned
 * the way of its direction. */
typedef struct HitSpark {
    /* 0x00 */ Entity entity;
    /* 0x18 */ SpriteAnim sprite;
    /* 0x40 */ s32 targetSize; /* the size eases to it */
    /* 0x44 */ s32 age;        /* deleted at 5 */
} HitSpark; /* size 0x48 */

Entity *func_800C391C(Entity *this, Vec3 *arg1, Vec3 *arg2, s32 arg3, s32 arg4);

s32 Projectile_getBurstKind(Projectile *this);

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_CLASSES_H */
