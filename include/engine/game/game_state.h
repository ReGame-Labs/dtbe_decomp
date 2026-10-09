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

/* the sides of a fight */
#define FIGHTER_COUNT 2

/* how many characters there are */
#define CHARACTER_COUNT 24
/* the characters, in the order of the debug menu's CHARACTER_NAMES (the
 * rookies evolve into the others of CHARACTER_EVOLUTIONS) */
#define CHARACTER_AGUMON 0
#define CHARACTER_DUKEMON 1
#define CHARACTER_GABUMON 2
#define CHARACTER_GUILMON 3
#define CHARACTER_HOLYDRAMON 4
#define CHARACTER_IMPERIALDRAMON 5
#define CHARACTER_IMPERIALDRAMON_PALADIN_MODE 6
#define CHARACTER_IMPMON 7
#define CHARACTER_BEELZEBUMON 8
#define CHARACTER_METALGARURUMON 9
#define CHARACTER_OMEGAMON 10
#define CHARACTER_PATAMON 11
#define CHARACTER_RENAMON 12
#define CHARACTER_SAKUYAMON 13
#define CHARACTER_SAINTGARGOMON 14
#define CHARACTER_SERAPHIMON 15
#define CHARACTER_STINGMON 16
#define CHARACTER_TAILMON 17
#define CHARACTER_TERRIERMON 18
#define CHARACTER_V_MON 19
#define CHARACTER_WORMMON 20
#define CHARACTER_WARGREYMON 21
#define CHARACTER_GOKUMON 22
#define CHARACTER_BLACKWARGREYMON 23
/* the last opponent of a run; clearing any run unlocks him */
#define CHARACTER_BOSS CHARACTER_GOKUMON
/* the character a run without continues unlocks */
#define CHARACTER_SECRET CHARACTER_BLACKWARGREYMON
/* the bit of a character in the masks of characters (SaveData.unlocked) */
#define CHARACTER_BIT(character) (1 << (character))
/* the fights' arenas the debug menu's STAGE_NAMES lists ahead of the bonus
 * games, the boss's last; a run picks from the first ARENA_PICK_COUNT. The
 * characters' homes (CHARACTER_HOMES) are the boss's or past the bonus games'. */
#define ARENA_FIGHT_COUNT 7
/* the boss's arena ("Boss" in the debug menu's STAGE_NAMES) */
#define ARENA_BOSS 6
/* the arena of the first bonus game: the bonus games' follow the fights' */
#define ARENA_BONUS ARENA_FIGHT_COUNT
/* the bonus games (the debug menu's "Bonus #0" to "Bonus #2") */
#define BONUS_GAME_COUNT 3
/* the most fights one run of the game goes through */
#define RUN_STEP_COUNT 9
/* the step of a run that is a bonus game */
#define RUN_STEP_BONUS 3
/* the step the secret character can come into */
#define RUN_STEP_SECRET 6
/* the last step of a run: the boss, in his arena */
#define RUN_STEP_LAST 7
/* the phase of the last step: the bonus step has none */
#define RUN_PHASE_LAST (RUN_STEP_LAST - 1)

/* GameState.level: the hardest (LEVEL_NAMES: "EASY", "NORMAL", "HARD") */
#define LEVEL_HARD 2
/* the levels */
#define LEVEL_COUNT 3

/* GameState.selectFlags: what the select scene picks */
#define SELECT_SIDE_0 0x1     /* side 1's character */
#define SELECT_SIDE_1 0x2     /* side 2's character */
#define SELECT_VERSUS 0x4     /* the arena, and each side's unk14 */
#define SELECT_BONUS 0x8      /* the bonus game */
#define SELECT_COMPUTER_1 0x20 /* side 2's character for the computer */

/* GameState.roundResult: the secret character came in; the fight starts
 * again against it */
#define ROUND_SECRET_OPPONENT 6

/* The state of the game: the arenas and opponents of the run, the options
 * and progress (SaveData), the button maps and the two sides. */
typedef struct GameState {
    /* 0x000 */ s32 arena; /* of the next fight */
    /* 0x004 */ s32 step;  /* the fight of the run */
    /* 0x008 */ s32 arenas[RUN_STEP_COUNT];    /* by step */
    /* 0x02C */ s32 opponents[RUN_STEP_COUNT]; /* by step */
    /* 0x050 */ s32 bonusGame;   /* of the run, or the one picked: its arena less ARENA_BONUS */
    /* 0x054 */ s32 roundResult; /* how the last round ended, as the fight scene's round returns it */
    /* 0x058 */ SaveData save;
    /* 0x2A4 */ s32 selectFlags; /* what the select scene picks: SELECT_* */
    /* 0x2A8 */ s32 level;       /* easy, normal or hard (the debug menu's LEVEL_NAMES) */
    /* 0x2AC */ s32 phase;       /* the fight of the run, the bonus game left out */
    /* 0x2B0 */ s32 singleFight; /* one fight, not a run: versus, the bonus games, random fights */
    /* 0x2B4 */ s32 demo;        /* the fight the title shows when left alone; a button ends it */
    /* 0x2B8 */ s32 challenge;   /* a player joined in (gameStateSetUpChallenge) */
    /* 0x2BC */ s32 continued;   /* Start was pressed on the continue screen */
    /* 0x2C0 */ s32 continues;   /* the continues taken in the run */
    /* 0x2C4 */ s32 challengerSide; /* the computer's side in the run */
    /* 0x2C8 */ s32 roundsNotWon;   /* the rounds the computer won, and those nobody won */
    /* 0x2CC */ s32 winnerCharacter; /* the character of the last round's winner */
    /* 0x2D0 */ s32 winner;          /* the side that won the last round, or -1 */
    /* 0x2D4 */ ButtonMap maps[2]; /* by player */
    /* 0x314 */ s32 lastCharacters[FIGHTER_COUNT]; /* by side: picked for the last single fight */
    /* 0x31C */ s32 lastVersusArena; /* picked for the last versus fight */
    /* 0x320 */ s32 lastBonusArena;  /* picked for the last bonus game */
    /* 0x324 */ Fighter fighters[FIGHTER_COUNT];
    /* 0x354 */ Fighter saved[FIGHTER_COUNT]; /* the sides of the last fight */
    /* 0x384 */ s16 fighterSounds[FIGHTER_COUNT]; /* by side: a sound of its character, or 0 */
    /* 0x388 */ s16 sounds[CHARACTER_COUNT]; /* by character: one of its CHARACTER_SOUNDS */
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

#ifdef __cplusplus
/*
 * JoinState and its functions are C++ only, as only game_state.cpp uses them:
 * its flags are bool bit fields, which g++ tests one at a time as the game
 * does (joinStateUpdate's `joined0 || joined1`), where the tests of u32 bit
 * fields merge into one mask test.
 */

/* Whether players joined by pressing Start, as joinStateUpdate updates it. */
typedef struct JoinState {
    /* 0x0 */ bool joined0 : 1;   /* player 1 is in */
    /* 0x0 */ bool joined1 : 1;   /* player 2 is in */
    /* 0x0 */ bool wasJoined : 1; /* joined, in the last update */
    /* 0x0 */ bool joined : 1;    /* either player is in */
    /* 0x0 */ bool entered : 1;   /* joined in this update */
    /* 0x0 */ bool left : 1;      /* left in this update */
    /* 0x4 */ s32 side;           /* of the player who joined, player 1's if both did, or -1 */
} JoinState;
#endif

/* the codes gameStateUnlockByCode knows */
#define CODE_COUNT 7
/* the characters that can be picked from the start (STARTING_CHARACTERS) */
#define STARTING_CHARACTER_COUNT 9
/* the pairs of CLEAR_UNLOCKS */
#define CLEAR_UNLOCK_COUNT 10
/* the characters of DEMO_CHARACTERS */
#define DEMO_CHARACTER_COUNT 18
/* the sounds of a character a side picks one of (gameStatePickFighterSounds) */
#define FIGHTER_SOUND_COUNT 5

/* the arenas gameStatePickArenas orders for a run */
#define ARENA_PICK_COUNT 6

/* the XA tracks of each character, played when it wins: character c's run
 * from CHARACTER_TRACKS[c] to CHARACTER_TRACKS[c + 1] */
extern u8 CHARACTER_TRACKS[CHARACTER_COUNT + 1];
/* the sounds of each character, as CHARACTER_TRACKS */
extern u16 CHARACTER_SOUNDS[CHARACTER_COUNT + 1];

/* the state of the game */
extern GameState GAME_STATE;

/* by character: the character it evolves to, itself for those that do not
 * (Agumon's is WarGreymon) */
extern s32 CHARACTER_EVOLUTIONS[CHARACTER_COUNT];
/* by character: the arena of its home (gameStateGetArena), the boss's or one
 * past the bonus games'; 10 is 12 for a side whose unk8 is 0 */
extern u8 CHARACTER_HOMES[CHARACTER_COUNT];
/* by character: the first of the FIGHTER_SOUND_COUNT sounds of a side whose
 * Fighter.unk8 is set, 0 for none */
extern s16 D_800102D4[CHARACTER_COUNT];
/* the same, for a side whose unk8 is 0 */
extern s16 D_80010304[CHARACTER_COUNT];
/* by character: its group (0 to 3), by whose preferences (ARENA_PREFERENCES)
 * a run orders its arenas and opponents */
extern s8 CHARACTER_GROUPS[CHARACTER_COUNT];
/* the characters that can be picked from the start */
extern u8 STARTING_CHARACTERS[STARTING_CHARACTER_COUNT];
/* pairs of characters: a run won with the first unlocks the second */
extern s8 CLEAR_UNLOCKS[CLEAR_UNLOCK_COUNT][2];
/* the characters gameStateSetUpDemo picks from */
extern u8 DEMO_CHARACTERS[DEMO_CHARACTER_COUNT];

/* the MD5 of each code */
extern u8 CODE_DIGESTS[CODE_COUNT][MD5_DIGEST_SIZE];
/* the preference of each group of characters for each kind of arena */
extern s8 ARENA_PREFERENCES[4][4];

/* the kind of each of those arenas */
extern s8 ARENA_KINDS[ARENA_PICK_COUNT];
/* the group of the player's character, for sortNodeComparePreference */
extern s32 PREFERENCE_GROUP;

GameState *gameStateInit(GameState *gameState);
void gameStateResetLastPicks(GameState *gameState);
s32 gameStateUnlockByCode(GameState *gameState, char *code);
u32 gameStateGetPickableCharacters(GameState *gameState);
void gameStateRecordClear(GameState *gameState);
void gameStateReset(GameState *gameState);
void gameStateSetUpVsComputer(GameState *gameState);
void gameStateStartRun(GameState *gameState);
void gameStateSetUpVersus(GameState *gameState, s32 computer);
void gameStateSetUpBonusVsComputer(GameState *gameState);
void gameStateSetUpBonusVersus(GameState *gameState);
u32 getRandomExcept(u32 previous, u32 count);
void gameStateSetUpRandomFight(GameState *gameState);
s32 gameStateGetArena(GameState *gameState);
s32 gameStatePickWinnerTrack(GameState *gameState);
void gameStatePickSounds(GameState *gameState);
s16 gameStateGetSound(GameState *gameState, s32 character);
void gameStatePickFighterSounds(GameState *gameState);
s16 gameStateGetFighterSound(GameState *gameState, s32 side);
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
s32 gameStateIsBonusStep(GameState *gameState);
s32 gameStateIsLastStep(GameState *gameState);
s32 gameStateIsOutsideRun(GameState *gameState);
void gameStateSetUpChallenge(GameState *gameState);
void gameStateStoreFighters(GameState *gameState);
void gameStateRestoreFighters(GameState *gameState);
void gameStateSetAltColor(GameState *gameState, s32 side);
void gameStateSetOpponent(GameState *gameState, s32 character);
void gameStateSetUpDemo(GameState *gameState);
s32 gameStateGetPlayerSide(GameState *gameState);
s32 gameStateHasPlayerWon(GameState *gameState);
s32 func_8001E3E4(GameState *gameState);
#ifdef __cplusplus
void joinStateReset(JoinState *joinState);
void joinStateSetLeft(JoinState *joinState);
void joinStateUpdate(JoinState *joinState);
#endif

EXTERN_C_END

#endif /* DTBE_GAME_GAME_STATE_H */
