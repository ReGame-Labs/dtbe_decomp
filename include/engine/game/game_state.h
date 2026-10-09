#ifndef DTBE_GAME_GAME_STATE_H
#define DTBE_GAME_GAME_STATE_H

/* The game state: a run's arenas and opponents, the two sides, unlocks, who joined. */

#include "common.h"
#include "engine/game/save_data.h"
#include "engine/lib/md5.h"
#include "engine/pad/button_map.h"

EXTERN_C_BEGIN

/* One side of a fight. */
typedef struct Fighter {
    /* 0x00 */ s32 character;
    /* 0x04 */ s32 computer; /* played by the computer, not with a pad */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 altColor; /* the other colors, when both sides look alike */
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} Fighter; /* size 0x18 */

#define FIGHTER_COUNT 2

#define CHARACTER_COUNT 24
/* the character a run without continues unlocks */
#define CHARACTER_SECRET 23
/* the bit of a character in the masks of characters (SaveData.unlocked) */
#define CHARACTER_BIT(character) (1 << (character))
/* the arenas picked at random */
#define ARENA_RANDOM_COUNT 7
/* the most fights one run of the game goes through */
#define RUN_STEP_COUNT 9

/* The state of the game: the arenas and opponents of the run, the options
 * and progress (SaveData), the button maps and the two sides. */
typedef struct GameState {
    /* 0x000 */ s32 arena; /* of the next fight */
    /* 0x004 */ s32 step;  /* the fight of the run */
    /* 0x008 */ s32 arenas[RUN_STEP_COUNT];    /* by step */
    /* 0x02C */ s32 opponents[RUN_STEP_COUNT]; /* by step */
    /* 0x050 */ s32 unk50;
    /* 0x054 */ s32 unk54;
    /* 0x058 */ SaveData save;
    /* 0x2A4 */ s32 unk2A4;
    /* 0x2A8 */ s32 unk2A8;
    /* 0x2AC */ s32 unk2AC;
    /* 0x2B0 */ s32 unk2B0;
    /* 0x2B4 */ s32 unk2B4;
    /* 0x2B8 */ s32 unk2B8;
    /* 0x2BC */ s32 unk2BC;
    /* 0x2C0 */ s32 unk2C0;
    /* 0x2C4 */ s32 unk2C4;
    /* 0x2C8 */ s32 unk2C8;
    /* 0x2CC */ s32 unk2CC;
    /* 0x2D0 */ s32 unk2D0; /* a side, or < 0 */
    /* 0x2D4 */ ButtonMap maps[2]; /* by player */
    /* 0x314 */ s32 unk314;
    /* 0x318 */ s32 unk318;
    /* 0x31C */ s32 unk31C;
    /* 0x320 */ s32 unk320;
    /* 0x324 */ Fighter fighters[FIGHTER_COUNT];
    /* 0x354 */ Fighter saved[FIGHTER_COUNT]; /* the sides of the last fight */
    /* 0x384 */ s16 unk384[FIGHTER_COUNT];
    /* 0x388 */ s16 unk388[CHARACTER_COUNT];
} GameState; /* size 0x3B8 */

/* A node of the lists sortNodeInitList builds, to be sorted by linkNodeSort. */
typedef struct SortNode {
    /* 0x0 */ struct SortNode *next;
    /* 0x4 */ s32 value;
    /* 0x8 */ s32 index;
    /* 0xC */ s32 key; /* random: sorting by it shuffles the list */
} SortNode;

/* orders two nodes: above 0 when a goes after b */
typedef s32 (*SortCompare)(SortNode *a, SortNode *b);

/* the characters a run picks its opponents from */
#define RUN_OPPONENTS 0x1E180D
/* the opponents of a run picked at random; a fixed one follows them */
#define RUN_RANDOM_OPPONENTS 7

/* Whether players joined by pressing Start, as joinStateUpdate updates it. */
typedef struct JoinState {
    /* 0x0 */ u32 joined0 : 1;   /* player 1 is in */
    /* 0x0 */ u32 joined1 : 1;   /* player 2 is in */
    /* 0x0 */ u32 wasJoined : 1; /* joined, in the last update */
    /* 0x0 */ u32 joined : 1;    /* either player is in */
    /* 0x0 */ u32 entered : 1;   /* joined in this update */
    /* 0x0 */ u32 left : 1;      /* left in this update */
    /* 0x4 */ s32 side;          /* the side of the computer, or -1 */
} JoinState;

/* the codes gameStateUnlockByCode knows */
#define CODE_COUNT 7

/* the arenas gameStatePickArenas orders for a run */
#define ARENA_PICK_COUNT 6

/* the ranges func_8001D750 picks from, by GameState.unk2CC */
extern u8 D_8005FCBC[];
/* the bounds of the ranges func_8001D7BC picks from, CHARACTER_COUNT + 1 */
extern u16 D_8005FCD8[];

/* the state of the game */
extern GameState GAME_STATE;

/* by character */
extern s32 D_800101B8[];
extern u8 CHARACTER_HOMES[];
extern s16 D_800102D4[];
extern s16 D_80010304[];
extern s8 CHARACTER_GROUPS[];
/* the characters that can be picked from the start */
extern u8 STARTING_CHARACTERS[9];
/* pairs of characters: a run won with the first unlocks the second */
extern s8 CLEAR_UNLOCKS[10][2];
/* the characters func_8001E264 picks from */
extern u8 D_800102A8[18];

/* the MD5 of each code */
extern u8 CODE_DIGESTS[CODE_COUNT][MD5_DIGEST_SIZE];
/* the preference of each group of characters for each kind of arena */
extern s8 ARENA_PREFERENCES[4][4];

/* the kind of each of those arenas */
extern s8 ARENA_KINDS[ARENA_PICK_COUNT];
/* the group of the player's character, for sortNodeComparePreference */
extern s32 PREFERENCE_GROUP;

GameState *gameStateInit(GameState *gameState);
void func_8001D1C8(GameState *gameState);
s32 gameStateUnlockByCode(GameState *gameState, char *code);
u32 gameStateGetPickableCharacters(GameState *gameState);
void gameStateRecordClear(GameState *gameState);
void gameStateReset(GameState *gameState);
void gameStateSetUpVsComputer(GameState *gameState);
void gameStateStartRun(GameState *gameState);
void gameStateSetUpVersus(GameState *gameState, s32 arg1);
void gameStateSetUpMinigameVsComputer(GameState *gameState);
void gameStateSetUpMinigameVersus(GameState *gameState);
u32 getRandomExcept(u32 previous, u32 count);
void gameStateSetUpRandomFight(GameState *gameState);
s32 gameStateGetArena(GameState *gameState);
s32 func_8001D750(GameState *gameState);
void func_8001D7BC(GameState *gameState);
s16 func_8001D874(GameState *gameState, s32 index);
void func_8001D888(GameState *gameState);
s16 func_8001D97C(GameState *gameState, s32 side);
void sortNodeInitList(SortNode *nodes, s8 *values, s32 count);
SortNode *sortNodeSortList(SortNode *nodes, SortCompare compare);
s32 sortNodeCompareKeys(SortNode *a, SortNode *b);
s32 sortNodeComparePreference(SortNode *a, SortNode *b);
void gameStatePickArenas(GameState *gameState);
void gameStatePickOpponents(GameState *gameState);
s32 gameStateAddContinue(GameState *gameState);
void gameStateStartNextFight(GameState *gameState);
s32 gameStateSetSecretOpponent(GameState *gameState);
s32 func_8001DECC(GameState *gameState);
s32 gameStateHasUnsavedProgress(GameState *gameState);
s32 func_8001DF0C(GameState *gameState);
s32 func_8001DF20(GameState *gameState);
s32 func_8001DF34(GameState *gameState);
void func_8001DF70(GameState *gameState);
void gameStateStoreFighters(GameState *gameState);
void gameStateRestoreFighters(GameState *gameState);
void gameStateSetAltColor(GameState *gameState, s32 side);
void gameStateSetOpponent(GameState *gameState, s32 character);
void func_8001E264(GameState *gameState);
s32 gameStateGetPlayerSide(GameState *gameState);
s32 func_8001E3B4(GameState *gameState);
s32 func_8001E3E4(GameState *gameState);
void joinStateReset(JoinState *joinState);
void joinStateSetLeft(JoinState *joinState);
void joinStateUpdate(JoinState *joinState);

EXTERN_C_END

#endif /* DTBE_GAME_GAME_STATE_H */
