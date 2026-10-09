#include "common.h"
#include "engine/game/game_state.h"
#include "engine/game/save_data.h"
#include "engine/lib/list.h"
#include "engine/lib/md5.h"
#include "engine/math/random.h"
#include "engine/math/shuffle.h"
#include "engine/pad/button_map.h"
#include "engine/pad/pad.h"
#include "libetc.h"
#include "memory.h"
#include "strings.h"
#include "psyq.h"

/* Builds the game state, with the default buttons. */
GameState *gameStateInit(GameState *gameState) {
    buttonMapInit(&gameState->maps[0]);
    buttonMapInit(&gameState->maps[1]);
    saveDataResetButtons(&gameState->save);
    return gameState;
}

/* Starts the select scene's picks over: Agumon against V-mon, the first arena
 * and the first bonus game. */
void gameStateResetLastPicks(GameState *gameState) {
    gameState->lastCharacters[1] = CHARACTER_V_MON;
    gameState->lastCharacters[0] = CHARACTER_AGUMON;
    gameState->lastVersusArena = 0;
    gameState->lastBonusArena = ARENA_BONUS;
}

/* the character each code unlocks, by code (CODE_DIGESTS) */
static u8 CODE_CHARACTERS[CODE_COUNT] = {
    CHARACTER_OMEGAMON, CHARACTER_IMPERIALDRAMON_PALADIN_MODE, CHARACTER_WARGREYMON, CHARACTER_IMPERIALDRAMON,
    CHARACTER_DUKEMON, CHARACTER_SAKUYAMON, CHARACTER_SAINTGARGOMON,
};

/* Unlocks the character of a code if code is one: the MD5 of each code is
 * kept, not the code. Returns the number of the code, or -1. */
s32 gameStateUnlockByCode(GameState *gameState, char *code) {
    u8 digest[MD5_DIGEST_SIZE];
    Md5 md5;
    u8 *expected;
    s32 i;

    md5Init(&md5);
    MD5Update(&md5, (u8 *)code, strlen(code));
    MD5Final(&md5, digest);
    i = 0;
    expected = CODE_DIGESTS[0];
    do {
        if (func_8003DF40(expected, digest, MD5_DIGEST_SIZE) == 0) {
            gameState->save.unlocked |= CHARACTER_BIT(CODE_CHARACTERS[i]);
            return i;
        }
        i++;
        expected += MD5_DIGEST_SIZE;
    } while (i < CODE_COUNT);
    return -1;
}

/* The characters that can be picked: those from the start, those unlocked,
 * and those won by clearing runs with others. */
u32 gameStateGetPickableCharacters(GameState *gameState) {
    u32 characters = 0;
    u32 cleared = gameState->save.cleared;
    s32 i;

    for (i = 0; i < STARTING_CHARACTER_COUNT; i++) {
        characters |= CHARACTER_BIT(STARTING_CHARACTERS[i]);
    }
    if (cleared) {
        characters |= CHARACTER_BIT(CHARACTER_BOSS);
    }
    if ((cleared & (CHARACTER_BIT(CHARACTER_AGUMON) | CHARACTER_BIT(CHARACTER_GABUMON))) ==
        (CHARACTER_BIT(CHARACTER_AGUMON) | CHARACTER_BIT(CHARACTER_GABUMON))) {
        characters |= CHARACTER_BIT(CHARACTER_OMEGAMON);
    }
    if ((cleared & (CHARACTER_BIT(CHARACTER_V_MON) | CHARACTER_BIT(CHARACTER_WORMMON))) ==
        (CHARACTER_BIT(CHARACTER_V_MON) | CHARACTER_BIT(CHARACTER_WORMMON))) {
        characters |= CHARACTER_BIT(CHARACTER_IMPERIALDRAMON_PALADIN_MODE);
    }
    if ((cleared & (CHARACTER_BIT(CHARACTER_GUILMON) | CHARACTER_BIT(CHARACTER_RENAMON) |
                    CHARACTER_BIT(CHARACTER_TERRIERMON))) ==
        (CHARACTER_BIT(CHARACTER_GUILMON) | CHARACTER_BIT(CHARACTER_RENAMON) |
         CHARACTER_BIT(CHARACTER_TERRIERMON))) {
        characters |= CHARACTER_BIT(CHARACTER_IMPMON);
    }
    for (i = 0; i < CLEAR_UNLOCK_COUNT; i++) {
        if (cleared & CHARACTER_BIT(CLEAR_UNLOCKS[i][0])) {
            characters |= CHARACTER_BIT(CLEAR_UNLOCKS[i][1]);
        }
    }
    return characters | gameState->save.unlocked;
}

/* Records the run won with the winner's character; one without continues
 * and with every round won unlocks the secret character. */
void gameStateRecordClear(GameState *gameState) {
    gameState->save.cleared |= CHARACTER_BIT(gameState->fighters[gameState->winner].character);
    if (!gameState->continues && !gameState->roundsNotWon) {
        gameState->save.unlocked |= CHARACTER_BIT(CHARACTER_SECRET);
    }
}

/* Clears the run and the sides, at the level of the options. */
void gameStateReset(GameState *gameState) {
    gameState->phase = 0;
    gameState->singleFight = 0;
    gameState->demo = 0;
    gameState->challenge = 0;
    gameState->continued = 0;
    gameState->continues = 0;
    gameState->roundsNotWon = 0;
    gameState->level = gameState->save.level;
    bzero((u8 *)gameState->fighters, sizeof(gameState->fighters));
    gameState->fighters[0].unk14 = 7;
    gameState->fighters[1].unk14 = 7;
}

/* Sets up a run against the computer: the player picks side 1's character. */
void gameStateSetUpVsComputer(GameState *gameState) {
    gameStateReset(gameState);
    gameState->selectFlags = SELECT_SIDE_0;
    gameState->fighters[1].computer = 1;
}

/* Starts a run: picks its arenas and opponents, and goes to its first fight. */
void gameStateStartRun(GameState *gameState) {
    gameState->step = -1;
    gameState->phase = 0;
    gameStatePickArenas(gameState);
    gameStatePickOpponents(gameState);
    gameStateStartNextFight(gameState);
}

/* Sets up a versus fight, side 2 played by the computer or by player 2. */
void gameStateSetUpVersus(GameState *gameState, s32 computer) {
    gameStateReset(gameState);
    gameState->selectFlags = SELECT_SIDE_0 | SELECT_VERSUS;
    if (computer) {
        gameState->selectFlags |= SELECT_COMPUTER_1;
    } else {
        gameState->selectFlags |= SELECT_SIDE_1;
    }
    gameState->fighters[1].computer = computer;
    gameState->singleFight = 1;
}

/* Sets up a bonus game against the computer. */
void gameStateSetUpBonusVsComputer(GameState *gameState) {
    gameStateReset(gameState);
    gameState->selectFlags = SELECT_SIDE_0 | SELECT_BONUS | SELECT_COMPUTER_1;
    gameState->fighters[1].computer = 1;
    gameState->singleFight = 1;
    gameState->level = gameState->save.level;
}

/* Sets up a bonus game of two players. */
void gameStateSetUpBonusVersus(GameState *gameState) {
    gameStateReset(gameState);
    gameState->selectFlags = SELECT_SIDE_0 | SELECT_SIDE_1 | SELECT_BONUS;
    gameState->singleFight = 1;
}

/* A random number below count, other than previous. */
u32 getRandomExcept(u32 previous, u32 count) {
    MersenneTwister *random = &RANDOM;
    u32 value;

    do {
        value = mersenneTwisterGenerate(random) % count;
    } while (value == previous);
    return value;
}

/* Sets up a fight of two random characters, both played by the computer, in
 * a random arena. */
void gameStateSetUpRandomFight(GameState *gameState) {
    gameStateReset(gameState);
    gameState->fighters[0].computer = 1;
    gameState->fighters[1].computer = 1;
    gameState->fighters[0].character = getRandomExcept(gameState->fighters[0].character, CHARACTER_COUNT);
    gameState->fighters[1].character = getRandomExcept(gameState->fighters[1].character, CHARACTER_COUNT);
    /* the game's own quirk: the arena to avoid is side 2's character, not the
     * last arena */
    gameState->arena = getRandomExcept(gameState->fighters[1].character, ARENA_FIGHT_COUNT);
    gameState->singleFight = 1;
    gameState->level = LEVEL_HARD;
    gameState->phase = RUN_PHASE_LAST;
}

/* The arena of the next fight: the one picked, or the home of a character. */
s32 gameStateGetArena(GameState *gameState) {
    s32 side;
    s32 character;
    s32 otherCharacter;
    s32 home;
    s32 otherHome;
    s32 unk8;
    s32 otherUnk8;

    if (gameState->arena >= ARENA_FIGHT_COUNT) {
        return gameState->arena;
    }
    /* the player's side, or side 0 */
    side = 0;
    if (gameState->fighters[0].computer) {
        side = !gameState->fighters[1].computer;
    }
    character = gameState->fighters[side].character;
    otherCharacter = gameState->fighters[side ^ 1].character;
    home = CHARACTER_HOMES[character];
    otherHome = CHARACTER_HOMES[otherCharacter];
    unk8 = gameState->fighters[side].unk8;
    otherUnk8 = gameState->fighters[side ^ 1].unk8;
    if (home == 10 && !unk8) {
        home = 12;
    }
    if (otherHome == 10 && !otherUnk8) {
        otherHome = 12;
    }
    if (gameState->singleFight) {
        if (home != ARENA_BOSS && home == otherHome) {
            home++;
        }
        return home;
    }
    return gameState->arena < ARENA_BOSS ? gameState->arena : home;
}

/* One of the XA tracks of the last round's winner, at random. */
s32 gameStatePickWinnerTrack(GameState *gameState) {
    s32 character = gameState->winnerCharacter;
    s32 low = CHARACTER_TRACKS[character];
    s32 span = CHARACTER_TRACKS[character + 1] - low;

    return low + (u8)mersenneTwisterGenerate(&RANDOM) % span;
}

/* Picks one of the sounds of each character (CHARACTER_SOUNDS), at random. */
void gameStatePickSounds(GameState *gameState) {
    MersenneTwister *random = &RANDOM;
    s32 i;

    for (i = 0; i < CHARACTER_COUNT; i++) {
        s32 sound = CHARACTER_SOUNDS[i];
        s32 span = CHARACTER_SOUNDS[i + 1] - sound;

        sound += (u8)mersenneTwisterGenerate(random) % span;
        gameState->sounds[i] = sound;
    }
}

/* The sound picked for a character. */
s16 gameStateGetSound(GameState *gameState, s32 character) {
    return gameState->sounds[character];
}

/* Picks a sound of each side's character: the first of its five (0 for
 * none, by the side's unk8), plus a random 0 to 4. */
void gameStatePickFighterSounds(GameState *gameState) {
    s32 side;

    for (side = 0; side < FIGHTER_COUNT; side++) {
        s16 *table = gameState->fighters[side].unk8 ? D_800102D4 : D_80010304;
        s32 sound = table[gameState->fighters[side].character];

        if (sound) {
            sound += mersenneTwisterGenerate(&RANDOM) % FIGHTER_SOUND_COUNT;
        }
        gameState->fighterSounds[side] = sound;
    }
}

/* The sound picked for a side, sound 18 when it has none. */
s16 gameStateGetFighterSound(GameState *gameState, s32 side) {
    s16 sound = gameState->fighterSounds[side];

    if (!sound) {
        sound = 18;
    }
    return sound;
}

/* Builds a list of count nodes with the values, in a random order of keys. */
void sortNodeInitList(SortNode *nodes, s8 *values, s32 count) {
    MersenneTwister *random = &RANDOM;
    s32 index = 0;

    for (count--; count != -1; count--) {
        nodes->next = nodes + 1;
        nodes->value = *values++;
        nodes->index = index++;
        nodes->key = mersenneTwisterGenerate(random);
        nodes++;
    }
    nodes[-1].next = NULL;
}

/* Sorts a list of SortNodes. Returns its new first node. */
SortNode *sortNodeSortList(SortNode *nodes, SortCompare compare) {
    /* a SortNode starts with its next link, all the sort uses of a LinkNode */
    return (SortNode *)linkNodeSort((LinkNode *)nodes, (LinkCompare)compare);
}

/* Orders nodes by their keys. */
s32 sortNodeCompareKeys(SortNode *a, SortNode *b) {
    return a->key - b->key;
}

/* the kind of each arena a run picks from, by arena */
s8 ARENA_KINDS[ARENA_PICK_COUNT] = { 3, 2, 3, 3, 0, 1 };
/* the group of the player's character (CHARACTER_GROUPS), by whose preference
 * sortNodeComparePreference sorts */
s32 PREFERENCE_GROUP = 0;

/* Orders nodes by the preference of the player's group for their kinds. */
s32 sortNodeComparePreference(SortNode *a, SortNode *b) {
    s8 *preference = ARENA_PREFERENCES[PREFERENCE_GROUP];

    return preference[b->value] - preference[a->value];
}

/* Orders the arenas of a run: the arenas of ARENA_KINDS shuffled, then
 * sorted by the preference of the player's group for their kinds, with a
 * bonus game at random for the bonus step and the boss's arena last. */
void gameStatePickArenas(GameState *gameState) {
    SortNode nodes[ARENA_PICK_COUNT];
    SortNode *node;
    s32 player; /* the player's side, then their character */
    s32 i;
    s32 j;
    u8 bonusGame;

    player = gameStateGetPlayerSide(gameState);
    if (player < 0) {
        player = 0;
    }
    player = gameState->fighters[player].character;
    PREFERENCE_GROUP = CHARACTER_GROUPS[player];
    sortNodeInitList(nodes, ARENA_KINDS, ARENA_PICK_COUNT);
    node = sortNodeSortList(sortNodeSortList(nodes, sortNodeCompareKeys), sortNodeComparePreference);
    for (i = 0; i < RUN_STEP_BONUS; i++) {
        gameState->arenas[i] = node->index;
        node = node->next;
    }
    bonusGame = (u8)mersenneTwisterGenerate(&RANDOM) % BONUS_GAME_COUNT;
    gameState->bonusGame = bonusGame;
    gameState->arenas[RUN_STEP_BONUS] = ARENA_BONUS + bonusGame;
    for (j = RUN_STEP_BONUS + 1; j < RUN_STEP_LAST; j++) {
        gameState->arenas[j] = node->index;
        node = node->next;
    }
    gameState->arenas[RUN_STEP_LAST] = ARENA_BOSS;
    gameState->arena = gameState->arenas[0];
}

/* Picks the opponents of a run: seven characters of RUN_OPPONENTS other than
 * the player's, the ones its group likes best first, then the boss. */
void gameStatePickOpponents(GameState *gameState) {
    SortNode nodes[CHARACTER_COUNT];
    SortNode *node;
    s32 side;
    s32 character;
    u32 candidates;
    s32 *opponents;
    s32 index;
    s32 i;

    candidates = RUN_OPPONENTS;
    side = gameStateGetPlayerSide(gameState);
    if (side < 0) {
        side = 0;
    }
    character = gameState->fighters[side].character;
    PREFERENCE_GROUP = CHARACTER_GROUPS[character];
    candidates &= ~CHARACTER_BIT(character);
    sortNodeInitList(nodes, CHARACTER_GROUPS, CHARACTER_COUNT);
    node = sortNodeSortList(sortNodeSortList(nodes, sortNodeCompareKeys), sortNodeComparePreference);
    opponents = gameState->opponents;
    for (i = 0; i < RUN_RANDOM_OPPONENTS;) {
        index = node->index;
        if (!(candidates & CHARACTER_BIT(index))) {
            node = node->next;
            continue;
        }
        node = node->next;
        opponents[i++] = index;
    }
    opponents[RUN_RANDOM_OPPONENTS] = CHARACTER_BOSS;
}

/* Takes a continue if the player chose to on the continue screen, clearing
 * their side's unk10. Returns whether they did. */
s32 gameStateAddContinue(GameState *gameState) {
    if (!gameState->continued) {
        return 0;
    }
    if (!gameState->fighters[0].computer) {
        gameState->fighters[0].unk10 = 0;
    } else if (!gameState->fighters[1].computer) {
        gameState->fighters[1].unk10 = 0;
    }
    gameState->continues++;
    return gameState->continued;
}

/* Goes to the next fight of the run. */
void gameStateStartNextFight(GameState *gameState) {
    s32 step = gameState->step + 1;

    gameState->step = step;
    if (step > RUN_STEP_BONUS) {
        gameState->phase = step - 1;
    } else {
        gameState->phase = step;
    }
    gameState->continued = 0;
    gameState->arena = gameState->arenas[step];
    gameStateSetOpponent(gameState, gameState->opponents[step]);
    gameState->saved[0] = gameState->fighters[0];
    gameState->saved[1] = gameState->fighters[1];
}

/* Brings the secret character into the fight of RUN_STEP_SECRET, when it is
 * still locked and the run went without continues, lost rounds or
 * challengers. Returns whether it did. */
s32 gameStateSetSecretOpponent(GameState *gameState) {
    s32 step;

    if (gameState->demo) {
        return 0;
    }
    step = gameState->step;
    if (step != RUN_STEP_SECRET) {
        return 0;
    }
    if (gameState->challenge) {
        return 0;
    }
    if (gameState->continues) {
        return 0;
    }
    if (gameState->roundsNotWon) {
        return 0;
    }
    if (gameState->fighters[0].character == CHARACTER_SECRET) {
        return 0;
    }
    if (gameState->fighters[1].character == CHARACTER_SECRET) {
        return 0;
    }
    if (gameState->save.unlocked & CHARACTER_BIT(CHARACTER_SECRET)) {
        return 0;
    }
    gameStateSetOpponent(gameState, CHARACTER_SECRET);
    gameState->opponents[RUN_STEP_SECRET] = CHARACTER_SECRET;
    gameState->roundResult = ROUND_SECRET_OPPONENT;
    return 1;
}

/* save.unk8, whose meaning is unknown. */
s32 func_8001DECC(GameState *gameState) {
    return gameState->save.unk8;
}

/* Whether the progress changed since it was saved. */
s32 gameStateHasUnsavedProgress(GameState *gameState) {
    s32 changed = 0;

    if (gameState->save.unk4) {
        changed = saveDataIsValid(&gameState->save) == 0;
    }
    return changed;
}

/* Whether the run is at its bonus game. */
s32 gameStateIsBonusStep(GameState *gameState) {
    return gameState->step == RUN_STEP_BONUS;
}

/* Whether the run is at its last fight. */
s32 gameStateIsLastStep(GameState *gameState) {
    return gameState->step == RUN_STEP_LAST;
}

/* Whether the fight is not one of a run against the computer: a single fight,
 * or one of two players. */
s32 gameStateIsOutsideRun(GameState *gameState) {
    s32 result = 0;

    if (gameState->singleFight || (!gameState->fighters[0].computer && !gameState->fighters[1].computer)) {
        result = 1;
    }
    return result;
}

/* Sets up the fight of a player who joined in against the player: the run is
 * kept, the challenger picks a character on the computer's side (both sides
 * pick again in a single fight). */
void gameStateSetUpChallenge(GameState *gameState) {
    s32 side = gameStateGetPlayerSide(gameState);
    s32 other = side ^ 1;

    gameState->challengerSide = other;
    gameStateStoreFighters(gameState);
    if (gameState->singleFight) {
        gameState->selectFlags = SELECT_SIDE_0 | SELECT_SIDE_1 | SELECT_VERSUS;
    } else if (!side) {
        gameState->selectFlags = SELECT_SIDE_1;
    } else {
        gameState->selectFlags = SELECT_SIDE_0;
    }
    gameState->fighters[0].computer = 0;
    gameState->fighters[1].computer = 0;
    gameState->fighters[other].unk10 = 0;
    gameState->challenge = 1;
}

/* Keeps the sides of the fight, with the challenger's side played by the
 * computer. In a single fight, the arena and that side's character become
 * the first step. */
void gameStateStoreFighters(GameState *gameState) {
    s32 side = gameState->challengerSide;
    Fighter *fighters = gameState->fighters;

    gameState->saved[0] = gameState->fighters[0];
    gameState->saved[1] = gameState->fighters[1];
    gameState->saved[side].computer = 1;
    gameState->saved[side].unk10 = 0;
    if (gameState->singleFight) {
        Fighter *opponent = &fighters[side];

        gameState->arenas[0] = gameState->arena;
        gameState->opponents[0] = opponent->character;
    }
}

/* After a challenge, puts the computer back on the side that lost it (the
 * challenger's when nobody won), and takes the arena of the step. As C,
 * `gameState->fighters[side] = gameState->saved[computer]` (side reused for
 * the step) differs only in the operand order of the two index additions
 * (base first).
 * g++ copies the struct through its operator=, as pointer sums: the index
 * comes first as in the game, but our 2.95.2 then keeps both addresses in
 * registers (only the first word's address is folded by CSE), where the game
 * folds 0x324/0x354 into every offset (36 lines differ): CSE's find_best_addr
 * folds the first word only, the later ones tie on cost. The array form folds
 * them all but puts the base first (get_inner_reference). */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", gameStateRestoreFighters);

/* Gives a side the other colors when both sides look alike. Register
 * allocation of the look comparison differs (14 lines, C and C++ alike):
 * the game loads each look into the register of its address, which
 * local-alloc only does by tying a load to its dying address register, and
 * it never ties through a MEM operand. Global pseudos for the looks would give
 * it (global-alloc's set_preference looks through the MEM); no source found
 * that makes them global. */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", gameStateSetAltColor);

/* Gives the computer's side character. */
void gameStateSetOpponent(GameState *gameState, s32 character) {
    s32 side;

    for (side = 0; side < FIGHTER_COUNT; side++) {
        if (gameState->fighters[side].computer) {
            gameState->fighters[side].character = character;
            gameState->fighters[side].unk10 = 0;
            gameStateSetAltColor(gameState, side);
            break;
        }
    }
}

/* Sets up the demo: a fight of two different characters of DEMO_CHARACTERS at
 * random, both played by the computer. The Shuffle is a C++ object: g++ computes its address again for
 * each call and destroys it at the end of the function. */
void gameStateSetUpDemo(GameState *gameState) {
    s32 side;

    gameStateReset(gameState);
    Shuffle shuffle(&RANDOM, sizeof(DEMO_CHARACTERS), DEMO_CHARACTERS);
    gameState->level = LEVEL_HARD;
    gameState->phase = RUN_PHASE_LAST;
    gameState->demo = 1;
    gameState->arena = mersenneTwisterGenerate(&RANDOM) % ARENA_PICK_COUNT;
    for (side = 0; side < FIGHTER_COUNT; side++) {
        gameState->fighters[side].character = shuffle.getNumber(side);
        gameState->fighters[side].computer = 1;
        gameState->fighters[side].unk8 = 0;
        gameState->fighters[side].unk14 = 7;
        gameState->fighters[side].unk10 = 0;
    }
}

/* The side of the player: side 0 when both sides are players, -1 when both
 * are the computer. */
s32 gameStateGetPlayerSide(GameState *gameState) {
    if (gameState->fighters[0].computer && gameState->fighters[1].computer) {
        return -1;
    }
    return gameState->fighters[0].computer != 0;
}

/* Whether a player won the last round. */
s32 gameStateHasPlayerWon(GameState *gameState) {
    if (gameState->winner < 0) {
        return 0;
    }
    /* computer is 0 or 1 */
    return gameState->fighters[gameState->winner].computer ^ 1;
}

/* unk10 of the player's side, 0 when there is no player. */
s32 func_8001E3E4(GameState *gameState) {
    s32 side = gameStateGetPlayerSide(gameState);

    if (side < 0) {
        return 0;
    }
    return gameState->fighters[side].unk10;
}

/* Has no player joined. */
void joinStateReset(JoinState *joinState) {
    joinState->joined0 = 0;
    joinState->joined1 = 0;
    joinState->wasJoined = 0;
    joinState->joined = 0;
    joinState->entered = 0;
    joinState->left = 0;
}

/* Has no player joined, as if one just left. */
void joinStateSetLeft(JoinState *joinState) {
    joinStateReset(joinState);
    joinState->left = 1;
}

/* Updates who joined: Start joins a player, or makes a joined one leave;
 * player 2 only joins when player 1 does not. The game keeps the address of
 * side in a register for its one store (the reference), and tests joined as
 * the condition of `wasJoined ^ joined` (operands in that order). */
void joinStateUpdate(JoinState *joinState) {
    PadManager *padManager = PAD_MANAGER_INSTANCE;
    u32 pressed0 = padManagerGetPressed(padManager, 0);
    u32 pressed1 = padManagerGetPressed(padManager, 1);

    if (joinState->joined0 || joinState->joined1) {
        joinState->joined0 ^= joinState->joined0 && (pressed0 & PADstart);
        joinState->joined1 ^= joinState->joined1 && (pressed1 & PADstart);
    } else {
        joinState->joined0 = joinState->joined0 || (pressed0 & PADstart);
        joinState->joined1 = !joinState->joined0 && (pressed1 & PADstart);
    }
    joinState->joined = joinState->joined0 | joinState->joined1;
    s32 &side = joinState->side;
    side = joinState->joined ? (joinState->joined0 ? 0 : 1) : -1;
    /* changed since the last update, to joined or to not joined */
    joinState->entered = joinState->joined & (joinState->wasJoined ^ joinState->joined);
    joinState->left = !joinState->joined & (joinState->wasJoined ^ joinState->joined);
    joinState->wasJoined = joinState->joined;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CHARACTER_EVOLUTIONS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", STARTING_CHARACTERS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CODE_DIGESTS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CLEAR_UNLOCKS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", DEMO_CHARACTERS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CHARACTER_HOMES);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_800102D4);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_80010304);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CHARACTER_GROUPS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", ARENA_PREFERENCES);
