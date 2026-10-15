#ifndef DTBE_BATTLE_GAME_H
#define DTBE_BATTLE_GAME_H

/* What game.bin, the battle overlay, defines for itself and for the stage overlays (st00-st09) that run on it. */

#include "common.h"
#include "vtable.h"
#include "engine/game/character.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/lib/list.h"
#include "engine/math/random.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"

EXTERN_C_BEGIN

/* One fight: the rounds, both sides, the camera, the map and the pause menu. It runs its rounds in a thread
 * of its own (Fight_run) and keeps itself in CURRENT_FIGHT. The stage overlays derive their classes from it
 * (their constructors call Fight_init and then install their own virtual table at 0xD34). Its size is not
 * known; 0xD38 is the end of what is known. */
typedef struct Fight {
    /* 0x000 */ Thread thread;
    /* 0x00C */ u8 unkC[0x10 - 0xC];
    /* 0x010 */ MeshScene *meshScene; /* the map, map/mapNN.dmm */
    /* 0x014 */ Chara *charas[4]; /* sides 0 and 1, then their evolved forms when EVOLUTION_LOADED */
    /* 0x024 */ struct InputHistory *inputs[2]; /* by side */
    /* 0x02C */ struct BattleSide *sides[2]; /* by side */
    /* 0x034 */ struct Controller *controllers[2]; /* the computer players (slot 0x18), NULL for a pad */
    /* 0x03C */ EntityGroup stageEntities; /* the map nodes' entities */
    /* 0x04C */ EntityGroup entities; /* the fighters, their effects and the battle messages */
    /* 0x05C */ EntityGroup unk5C; /* while it holds an entity, the fight waits (the evolution scene) */
    /* 0x06C */ Group fighterGroup; /* the fighters' models */
    /* 0x0CC */ Group unkCC;
    /* 0x12C */ LookAt lookAt;
    /* 0x180 */ u8 unk180[0x1B0 - 0x180];
    /* 0x1B0 */ Camera camera;
    /* 0x1EC */ Camera camera2; /* cameraInitShared on camera: the same view, its own ordering table */
    /* 0x228 */ ListNode side1Shapes; /* the collision shapes of side 1's fighter */
    /* 0x230 */ ListNode side0Shapes;
    /* 0x238 */ ListNode stageShapes; /* the stage's and the items' */
    /* 0x240 */ s32 wins[4]; /* the rounds won, by side; the constructor clears the four */
    /* 0x250 */ s32 winsNeeded;
    /* 0x254 */ s32 roundsLeft;
    /* 0x258 */ s32 unk258; /* Fight_setUpAngle's argument */
    /* 0x25C */ u8 unk25C[0x2A4 - 0x25C];
    /* 0x2A4 */ VECTOR unk2A4; /* (0, -0x1000, 0) through setTerrainUp (func_8006F1C4) */
    /* 0x2B4 */ u8 unk2B4[0x2B8 - 0x2B4];
    /* 0x2B8 */ s32 wipe; /* the screen wipe: from 0x20 down to -6 (Fight_wipeIn), from 0 up to 0x26 (Fight_wipeOut) */
    /* 0x2BC */ s32 pauseCursor; /* 0 continue, 1 quit */
    /* 0x2C0 */ s32 pauseSide; /* the side that paused */
    /* 0x2C4 */ s32 pauseButtons; /* its pressed buttons */
    /* 0x2C8 */ u32 flags; /* FIGHT_FLAG_* */
    /* 0x2CC */ u8 unk2CC[0x314 - 0x2CC];
    /* 0x314 */ s32 arena; /* GAME_STATE.arena */
    /* 0x318 */ u8 unk318[0x31C - 0x318];
    /* 0x31C */ Vec3 eye; /* Fight_updateCamera copies it into lookAt.eye */
    /* 0x328 */ u8 unk328[0x32C - 0x328];
    /* 0x32C */ s32 unk32C; /* st01 sways it (st01_tickFallEvent) */
    /* 0x330 */ u8 unk330[0x33C - 0x330];
    /* 0x33C */ s32 boundLeft; /* the arena's bounds (Fight_setBounds), the stages' constants */
    /* 0x340 */ s32 boundTop;
    /* 0x344 */ u8 unk344[0x34C - 0x344];
    /* 0x34C */ s32 boundRight;
    /* 0x350 */ s32 boundBottom; /* the floor: a fighter more than 0x1000 below it has fallen (Fight_isFallen) */
    /* 0x354 */ s32 boundDepth;
    /* 0x358 */ u8 unk358[0x35C - 0x358];
    /* 0x35C */ s32 unk35C; /* half the smaller span of the bounds */
    /* 0x360 */ MersenneTwister random;
    /* 0xD24 */ s32 steps; /* the round's frames, Fight_finishStep counts one per call */
    /* 0xD28 */ s32 timeLeft; /* the round's frames left: 3000 at its start, 1 with no time limit; 1 sends st08's
                               * stage to its end (st08_checkWinner) */
    /* 0xD2C */ u32 modeFlags; /* FIGHT_MODE_* */
    /* 0xD30 */ s32 cameraFocus; /* 0 both fighters, a side + 1 for its close-up (the evolution scene); st02 keeps
                                  * its camera's state in it (StageTurn_updateCamera) */
    /* 0xD34 */ VtableEntry *vtable;
} Fight;

/* Fight.flags */
#define FIGHT_FLAG_OVER 0x1         /* Fight_end: the main loop stops */
#define FIGHT_FLAG_SLOW 0x2         /* slow motion after a knock-out: odd frames are skipped */
#define FIGHT_FLAG_STOP_ROUND 0x8   /* Fight_stopRound: the round ends with ROUND_STOPPED */
#define FIGHT_FLAG_ROUND_OVER 0x10  /* set after the round's messages */
#define FIGHT_FLAG_FREEZE 0x20      /* the sides are not updated */
#define FIGHT_FLAG_HOLD_CLOCK 0x40  /* the round's clock does not run */
#define FIGHT_FLAG_LAST_ROUND 0x80  /* a side has the wins, or no round is left */
#define FIGHT_FLAG_CONTINUE 0x100   /* Fight_showContinue */
#define FIGHT_FLAG_QUIT 0x200       /* the pause menu's quit */

/* Fight.modeFlags */
#define FIGHT_MODE_BONUS 0x1 /* a bonus game's fight: BONUS_FLAT and BONUS_GAME_TIME, the clock always runs */

/* Fight_getBonus's kinds, on a 0-10000 scale */
#define BONUS_PERFECT 0   /* 10000 with full life */
#define BONUS_LIFE 1      /* the life left */
#define BONUS_TIME 2      /* the seconds left * 100 */
#define BONUS_FLAT 3      /* 5000 */
#define BONUS_GAME_TIME 4 /* BONUS_TIME's value, the bonus games' pair with BONUS_FLAT */

/* the fight Fight_init constructed last */
extern Fight *CURRENT_FIGHT;

/* where each side's fighter stands, by arena and side: at the start of a round (Fight_startRound) and when it is
 * put back (Fight_respawnFighter) */
extern DVECTOR ARENA_START_POSITIONS[10][2];
extern DVECTOR ARENA_RESPAWN_POSITIONS[10][2];

/* What makes the pad word of a side without a pad: its slot 0x08 gives the buttons for the side's fighter
 * (Controller_getButtons). The computer player derives from it, and the bonus stages derive their own. */
typedef struct Controller {
    /* 0x0 */ struct BattleSide *side; /* the side it plays */
    /* 0x4 */ VtableEntry *vtable;
} Controller;

/* The owner of a collision shape, told of each touch: only a virtual table, COLLISION_OWNER_VTABLE, whose slot
 * 0x08 (pure: an empty method of the executable's) collideShapes calls with the other shape, and the destructor
 * CollisionOwner_destroy. Hit and many classes of the stages derive from it. */
typedef struct CollisionOwner {
    /* 0x0 */ VtableEntry *vtable;
} CollisionOwner;

/* the virtual tables of CollisionOwner and Controller */
extern VtableEntry COLLISION_OWNER_VTABLE[];
extern VtableEntry CONTROLLER_VTABLE[];

/* BonusFight's result (BonusFight.result). */
enum BonusResult {
    BONUS_RESULT_PLAYING,      /* the game goes on */
    BONUS_RESULT_WON,          /* BonusFight_endWithWinner */
    BONUS_RESULT_DRAW,         /* BonusFight_endInDraw */
    BONUS_RESULT_ENDING = 0xFE, /* the message is gone: endWait frames to go */
    BONUS_RESULT_OVER = 0xFF
};

/* The bonus games' fight (BonusFight_init, virtual table BONUS_FIGHT_VTABLE): arenas 7 to 9, whose stage classes
 * (st07, st08, st09) derive from it. The mode is the arena - 7; the stages fill nodes and the gauges' maxima, and
 * end the game with BonusFight_endWithWinner or BonusFight_endInDraw. */
typedef struct BonusFight {
    /* 0x000 */ Fight base;
    /* 0xD38 */ s32 nodes[8]; /* mesh scene nodes the stage's constructor picks (BonusFight_startRound's effects) */
    /* 0xD58 */ VECTOR startPos[2]; /* by side: where the round starts */
    /* 0xD78 */ VECTOR holdPos[2];  /* by side: where BonusFight_holdFighter keeps the fighter */
    /* 0xD98 */ s16 holding[2]; /* by side: held until its clip ends (BonusFight_tick); stops st09's BonusThrower_throw */
    /* 0xD9C */ u16 result;        /* BONUS_RESULT_* */
    /* 0xD9E */ u16 resultFrames;  /* counted up to resultDelay before the result runs */
    /* 0xDA0 */ u32 messageHandle; /* the handle of the result's battle message */
    /* 0xDA4 */ u32 jingleHandle;  /* the win jingle (sound 0x6D, 0x6F or 0x71 by arena), stopped by slot 0x38 */
    /* 0xDA8 */ MeshScene *meshScene; /* Fight_getMeshScene at the round's start; nodes are looked up in it */
    /* 0xDAC */ u8 unkDAC[0xF2C - 0xDAC];
    /* 0xF2C */ s16 gauges[2];  /* by side: the bars of modes 0 and 2, over gaugeMax (st07: frames a button is held) */
    /* 0xF30 */ s16 counts[2];  /* by side: mode 1's (st08) count, printed "%03d" and drawn over countMax */
    /* 0xF34 */ s16 gaugeMax;   /* 30 in st07 and st09, 15 in st08 */
    /* 0xF36 */ s16 countMax;   /* 120 in st08 */
    /* 0xF38 */ s16 markCounts[2]; /* by side: how many marks are set (BonusFight_addMark) */
    /* 0xF3C */ s16 marks[2][10];  /* by side: mode 0's (st07) ten marks */
    /* 0xF64 */ u16 scores[2]; /* by side: mode 2's (st09) score, printed "%06d"; the leader's palette pulses */
    /* 0xF68 */ u16 resultDelay; /* frames before the result runs: 90 in st07, 120 in st08 */
    /* 0xF6A */ u16 hudHidden;   /* the bonus HUD draws nothing while set */
    /* 0xF6C */ u16 endWait;     /* 60 frames in BONUS_RESULT_ENDING */
    /* 0xF6E */ u8 unkF6E[0xF70 - 0xF6E];
} BonusFight;

Fight *Fight_init(Fight *this);
void Fight_destroy(Fight *this, s32 flags);

/* the fight's camera and its look-at */
Camera *Fight_getCamera(Fight *this);
LookAt *Fight_getLookAt(Fight *this);
/* sets the camera's view from the look-at (the stages call it after moving the look-at) */
void Fight_applyView(Fight *this);

/* a random number from the fight's own generator */
u32 Fight_random(Fight *this);

/* add a collision shape of side 1's fighter, of side 0's, of the stage (or an item) */
void Fight_addSide1Shape(Fight *this, ListNode *node);
void Fight_addSide0Shape(Fight *this, ListNode *node);
void Fight_addStageShape(Fight *this, ListNode *node);

/* add an entity (when not NULL) to the stage entities, or to the entities; return it */
Entity *Fight_addStageEntity(Fight *this, Entity *entity);
Entity *Fight_addEntity(Fight *this, Entity *entity);

EntityGroup *func_8006F0E4(Fight *this, Entity *entity);
s32 func_8006F104(Fight *this);
s32 func_8006F144(Fight *this);
s32 Fight_isRoundOver(Fight *this);
s32 Fight_isContinueShown(Fight *this);
MeshScene *Fight_getMeshScene(Fight *this);

/* the fighter of a side (0 or 1) */
struct Fighter *Fight_getFighter(Fight *this, s32 side);
void Fight_setBounds(Fight *this, s32 left, s32 right, s32 top, s32 bottom, s32 depth);

/* the virtual methods the stages override and call from theirs */
void Fight_startRound(Fight *this);
void Fight_tick(Fight *this);
void Fight_tickScenery(void);
void Fight_updateCamera(Fight *this);
void Fight_drawWorld(void *this);

/* whether the segment from from to to hits the map, moving to to the hit */
s32 castMapRay(Vec3 *to, Vec3 *from);

/* integrates velocity by force over mass (divide16), setting move to the mean of the old and new velocity */
void func_80079C1C(Vec3 *move, Vec3 *velocity, Vec3 *force, s32 mass);

BonusFight *BonusFight_init(BonusFight *this);
void BonusFight_destroy(BonusFight *this, s32 flags);
/* its virtual slots, which the stages call from theirs */
void BonusFight_startRound(BonusFight *this);
void BonusFight_tick(BonusFight *this);
void BonusFight_tickScenery(BonusFight *this);
void BonusFight_prepareNextRound(BonusFight *this);
/* the ends of a bonus game, and holding a side's fighter in place while its clip plays */
void BonusFight_endWithWinner(BonusFight *this, struct Fighter *winner, struct Fighter *loser);
void BonusFight_endInDraw(BonusFight *this);
void BonusFight_holdFighter(BonusFight *this, u16 side);
void BonusFight_addMark(BonusFight *this, s16 side);
Controller *Controller_init(Controller *this, struct BattleSide *side);
void Controller_destroy(Controller *this, s32 flags);

EXTERN_C_END

#endif /* DTBE_BATTLE_GAME_H */
