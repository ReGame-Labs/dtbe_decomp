#include "common.h"
#include "engine/game/game_state.h"
#include "engine/game/save_data.h"
#include "engine/lib/list.h"
#include "engine/lib/md5.h"
#include "engine/math/random.h"
#include "engine/pad/button_map.h"
#include "psyq.h"
#include "memory.h"
#include "strings.h"

/* Builds the game state, with the default buttons. */
GameState *gameStateInit(GameState *this) {
    buttonMapInit(&this->maps[0]);
    buttonMapInit(&this->maps[1]);
    saveDataResetButtons(&this->save);
    return this;
}

void func_8001D1C8(GameState *this) {
    this->unk318 = 0x13;
    this->unk314 = 0;
    this->unk31C = 0;
    this->unk320 = 7;
}

/* the character each code unlocks, by code (CODE_DIGESTS) */
static u8 CODE_CHARACTERS[CODE_COUNT] = { 10, 6, 21, 5, 1, 13, 14 };

/* Unlocks the character of a code if code is one: the MD5 of each code is
 * kept, not the code. Returns the number of the code, or -1. */
s32 gameStateUnlockByCode(GameState *this, char *code) {
    u8 digest[MD5_DIGEST_SIZE];
    Md5 md5;
    u8 *expected;
    s32 i;

    func_800383B4(&md5);
    MD5Update(&md5, (u8 *)code, strlen(code));
    MD5Final(&md5, digest);
    i = 0;
    expected = CODE_DIGESTS[0];
    do {
        if (func_8003DF40(expected, digest, MD5_DIGEST_SIZE) == 0) {
            this->save.unlocked |= CHARACTER_BIT(CODE_CHARACTERS[i]);
            return i;
        }
        i++;
        expected += MD5_DIGEST_SIZE;
    } while (i < CODE_COUNT);
    return -1;
}

/* The characters that can be picked: those from the start, those unlocked,
 * and those won by clearing runs with others. */
u32 gameStateGetPickableCharacters(GameState *this) {
    u32 characters = 0;
    u32 cleared = this->save.cleared;
    s32 i;

    for (i = 0; i < 9; i++) {
        characters |= CHARACTER_BIT(STARTING_CHARACTERS[i]);
    }
    if (cleared) {
        characters |= CHARACTER_BIT(22);
    }
    if ((cleared & (CHARACTER_BIT(0) | CHARACTER_BIT(2))) == (CHARACTER_BIT(0) | CHARACTER_BIT(2))) {
        characters |= CHARACTER_BIT(10);
    }
    if ((cleared & (CHARACTER_BIT(19) | CHARACTER_BIT(20))) == (CHARACTER_BIT(19) | CHARACTER_BIT(20))) {
        characters |= CHARACTER_BIT(6);
    }
    if ((cleared & (CHARACTER_BIT(3) | CHARACTER_BIT(12) | CHARACTER_BIT(18))) ==
        (CHARACTER_BIT(3) | CHARACTER_BIT(12) | CHARACTER_BIT(18))) {
        characters |= CHARACTER_BIT(7);
    }
    for (i = 0; i < 10; i++) {
        if (cleared & CHARACTER_BIT(CLEAR_UNLOCKS[i][0])) {
            characters |= CHARACTER_BIT(CLEAR_UNLOCKS[i][1]);
        }
    }
    return characters | this->save.unlocked;
}

/* Records the run won with the player's character; one without continues
 * unlocks the secret character. */
void gameStateRecordClear(GameState *this) {
    this->save.cleared |= CHARACTER_BIT(this->fighters[this->unk2D0].character);
    if (!this->unk2C0 && !this->unk2C8) {
        this->save.unlocked |= CHARACTER_BIT(CHARACTER_SECRET);
    }
}

void gameStateReset(GameState *this) {
    this->unk2AC = 0;
    this->unk2B0 = 0;
    this->unk2B4 = 0;
    this->unk2B8 = 0;
    this->unk2BC = 0;
    this->unk2C0 = 0;
    this->unk2C8 = 0;
    this->unk2A8 = this->save.unk10;
    bzero((u8 *)this->fighters, sizeof(this->fighters));
    this->fighters[0].unk14 = 7;
    this->fighters[1].unk14 = 7;
}

void gameStateSetUpVsComputer(GameState *this) {
    gameStateReset(this);
    this->unk2A4 = 1;
    this->fighters[1].computer = 1;
}

void gameStateStartRun(GameState *this) {
    this->step = -1;
    this->unk2AC = 0;
    gameStatePickArenas(this);
    gameStatePickOpponents(this);
    gameStateStartNextFight(this);
}

void gameStateSetUpVersus(GameState *this, s32 arg1) {
    gameStateReset(this);
    this->unk2A4 = 5;
    if (arg1) {
        this->unk2A4 |= 0x20;
    } else {
        this->unk2A4 |= 0x2;
    }
    this->fighters[1].computer = arg1;
    this->unk2B0 = 1;
}

void gameStateSetUpMinigameVsComputer(GameState *this) {
    gameStateReset(this);
    this->unk2A4 = 0x29;
    this->fighters[1].computer = 1;
    this->unk2B0 = 1;
    this->unk2A8 = this->save.unk10;
}

void gameStateSetUpMinigameVersus(GameState *this) {
    gameStateReset(this);
    this->unk2A4 = 0xB;
    this->unk2B0 = 1;
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
void gameStateSetUpRandomFight(GameState *this) {
    gameStateReset(this);
    this->fighters[0].computer = 1;
    this->fighters[1].computer = 1;
    this->fighters[0].character = getRandomExcept(this->fighters[0].character, CHARACTER_COUNT);
    this->fighters[1].character = getRandomExcept(this->fighters[1].character, CHARACTER_COUNT);
    this->arena = getRandomExcept(this->fighters[1].character, ARENA_RANDOM_COUNT);
    this->unk2B0 = 1;
    this->unk2A8 = 2;
    this->unk2AC = 6;
}

/* The arena of the next fight: the one picked, or the home of a character. */
s32 gameStateGetArena(GameState *this) {
    s32 side;
    s32 character;
    s32 otherCharacter;
    s32 home;
    s32 otherHome;
    s32 unk8;
    s32 otherUnk8;

    if (this->arena >= ARENA_RANDOM_COUNT) {
        return this->arena;
    }
    /* the player's side, or side 0 */
    side = 0;
    if (this->fighters[0].computer) {
        side = !this->fighters[1].computer;
    }
    character = this->fighters[side].character;
    otherCharacter = this->fighters[side ^ 1].character;
    home = CHARACTER_HOMES[character];
    otherHome = CHARACTER_HOMES[otherCharacter];
    unk8 = this->fighters[side].unk8;
    otherUnk8 = this->fighters[side ^ 1].unk8;
    if (home == 10 && !unk8) {
        home = 12;
    }
    if (otherHome == 10 && !otherUnk8) {
        otherHome = 12;
    }
    if (this->unk2B0) {
        if (home != 6 && home == otherHome) {
            home++;
        }
        return home;
    }
    return this->arena < 6 ? this->arena : home;
}

/* A random number in the range D_8005FCBC gives for unk2CC. */
s32 func_8001D750(GameState *this) {
    s32 index = this->unk2CC;
    s32 low = D_8005FCBC[index];
    s32 span = D_8005FCBC[index + 1] - low;

    return low + (u8)mersenneTwisterGenerate(&RANDOM) % span;
}

/* Picks a random number for each character, in its range of D_8005FCD8. */
void func_8001D7BC(GameState *this) {
    MersenneTwister *random = &RANDOM;
    s32 i;

    for (i = 0; i < CHARACTER_COUNT; i++) {
        s32 value = D_8005FCD8[i];
        s32 span = D_8005FCD8[i + 1] - value;

        value += (u8)mersenneTwisterGenerate(random) % span;
        this->unk388[i] = value;
    }
}

s16 func_8001D874(GameState *this, s32 index) {
    return this->unk388[index];
}

/* Picks unk384 of each side: its character's value, plus a random 0 to 4. */
void func_8001D888(GameState *this) {
    s32 side;

    for (side = 0; side < FIGHTER_COUNT; side++) {
        s16 *table = this->fighters[side].unk8 ? D_800102D4 : D_80010304;
        s32 value = table[this->fighters[side].character];

        if (value) {
            value += mersenneTwisterGenerate(&RANDOM) % 5;
        }
        this->unk384[side] = value;
    }
}

/* unk384 of a side, 18 when it is 0. */
s16 func_8001D97C(GameState *this, s32 side) {
    s16 value = this->unk384[side];

    if (!value) {
        value = 18;
    }
    return value;
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
 * sorted by the preference of the player's group for their kinds, with one
 * of the three arenas after the random ones fourth and arena 6 eighth. */
void gameStatePickArenas(GameState *this) {
    SortNode nodes[ARENA_PICK_COUNT];
    SortNode *node;
    s32 player; /* the player's side, then their character */
    s32 i;
    s32 j;
    u8 extra;

    player = gameStateGetPlayerSide(this);
    if (player < 0) {
        player = 0;
    }
    player = this->fighters[player].character;
    PREFERENCE_GROUP = CHARACTER_GROUPS[player];
    sortNodeInitList(nodes, ARENA_KINDS, ARENA_PICK_COUNT);
    node = sortNodeSortList(sortNodeSortList(nodes, sortNodeCompareKeys), sortNodeComparePreference);
    for (i = 0; i < 3; i++) {
        this->arenas[i] = node->index;
        node = node->next;
    }
    extra = (u8)mersenneTwisterGenerate(&RANDOM) % 3;
    this->unk50 = extra;
    this->arenas[3] = ARENA_RANDOM_COUNT + extra;
    for (j = 4; j < 7; j++) {
        this->arenas[j] = node->index;
        node = node->next;
    }
    this->arenas[7] = 6;
    this->arena = this->arenas[0];
}

/* Picks the opponents of a run: seven characters of RUN_OPPONENTS other than
 * the player's, the ones its group likes best first, then character 22. */
void gameStatePickOpponents(GameState *this) {
    SortNode nodes[CHARACTER_COUNT];
    SortNode *node;
    s32 side;
    s32 character;
    u32 candidates;
    s32 *opponents;
    s32 index;
    s32 i;

    candidates = RUN_OPPONENTS;
    side = gameStateGetPlayerSide(this);
    if (side < 0) {
        side = 0;
    }
    character = this->fighters[side].character;
    PREFERENCE_GROUP = CHARACTER_GROUPS[character];
    candidates &= ~CHARACTER_BIT(character);
    sortNodeInitList(nodes, CHARACTER_GROUPS, CHARACTER_COUNT);
    node = sortNodeSortList(sortNodeSortList(nodes, sortNodeCompareKeys), sortNodeComparePreference);
    opponents = this->opponents;
    for (i = 0; i < RUN_RANDOM_OPPONENTS;) {
        index = node->index;
        if (!(candidates & CHARACTER_BIT(index))) {
            node = node->next;
            continue;
        }
        node = node->next;
        opponents[i++] = index;
    }
    opponents[RUN_RANDOM_OPPONENTS] = 22;
}

/* Counts a continue. Returns unk2BC. */
s32 gameStateAddContinue(GameState *this) {
    if (!this->unk2BC) {
        return 0;
    }
    if (!this->fighters[0].computer) {
        this->fighters[0].unk10 = 0;
    } else if (!this->fighters[1].computer) {
        this->fighters[1].unk10 = 0;
    }
    this->unk2C0++;
    return this->unk2BC;
}

/* Goes to the next fight of the run. */
void gameStateStartNextFight(GameState *this) {
    s32 step = this->step + 1;

    this->step = step;
    if (step >= 4) {
        this->unk2AC = step - 1;
    } else {
        this->unk2AC = step;
    }
    this->unk2BC = 0;
    this->arena = this->arenas[step];
    gameStateSetOpponent(this, this->opponents[step]);
    this->saved[0] = this->fighters[0];
    this->saved[1] = this->fighters[1];
}

/* Puts the secret character in the seventh fight, when the run went well
 * enough and it is still locked. Returns whether it did. */
s32 gameStateSetSecretOpponent(GameState *this) {
    s32 step;

    if (this->unk2B4) {
        return 0;
    }
    step = this->step;
    if (step != 6) {
        return 0;
    }
    if (this->unk2B8) {
        return 0;
    }
    if (this->unk2C0) {
        return 0;
    }
    if (this->unk2C8) {
        return 0;
    }
    if (this->fighters[0].character == CHARACTER_SECRET) {
        return 0;
    }
    if (this->fighters[1].character == CHARACTER_SECRET) {
        return 0;
    }
    if (this->save.unlocked & CHARACTER_BIT(CHARACTER_SECRET)) {
        return 0;
    }
    gameStateSetOpponent(this, CHARACTER_SECRET);
    this->opponents[6] = CHARACTER_SECRET;
    this->unk54 = step;
    return 1;
}

s32 func_8001DECC(GameState *this) {
    return this->save.unk8;
}

/* Whether the progress changed since it was saved. */
s32 gameStateHasUnsavedProgress(GameState *this) {
    s32 changed = 0;

    if (this->save.unk4) {
        changed = saveDataIsValid(&this->save) == 0;
    }
    return changed;
}

s32 func_8001DF0C(GameState *this) {
    return this->step == 3;
}

s32 func_8001DF20(GameState *this) {
    return this->step == 7;
}

s32 func_8001DF34(GameState *this) {
    s32 result = 0;

    if (this->unk2B0 || (!this->fighters[0].computer && !this->fighters[1].computer)) {
        result = 1;
    }
    return result;
}

void func_8001DF70(GameState *this) {
    s32 side = gameStateGetPlayerSide(this);
    s32 other = side ^ 1;

    this->unk2C4 = other;
    gameStateStoreFighters(this);
    if (this->unk2B0) {
        this->unk2A4 = 7;
    } else if (!side) {
        this->unk2A4 = 2;
    } else {
        this->unk2A4 = 1;
    }
    this->fighters[0].computer = 0;
    this->fighters[1].computer = 0;
    this->fighters[other].unk10 = 0;
    this->unk2B8 = 1;
}

/* Keeps the sides of the fight, with side unk2C4 played by the computer. When
 * unk2B0 is set, the arena and that side's character become the first step
 * of the run. */
void gameStateStoreFighters(GameState *this) {
    s32 side = this->unk2C4;
    Fighter *fighters = this->fighters;

    this->saved[0] = this->fighters[0];
    this->saved[1] = this->fighters[1];
    this->saved[side].computer = 1;
    this->saved[side].unk10 = 0;
    if (this->unk2B0) {
        Fighter *opponent = &fighters[side];

        this->arenas[0] = this->arena;
        this->opponents[0] = opponent->character;
    }
}

/* Puts the computer's side of the last fight back, opposite side unk2D0 (or
 * where it was when unk2D0 < 0), and takes the arena of the step. Only the
 * operand order of two additions differs, even after a permuter run. */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", gameStateRestoreFighters);

/* Gives a side the other colors when both sides look alike. Register
 * allocation differs, even after a permuter run. */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", gameStateSetAltColor);

/* Gives the computer's side character. */
void gameStateSetOpponent(GameState *this, s32 character) {
    s32 side;

    for (side = 0; side < FIGHTER_COUNT; side++) {
        if (this->fighters[side].computer) {
            this->fighters[side].character = character;
            this->fighters[side].unk10 = 0;
            gameStateSetAltColor(this, side);
            break;
        }
    }
}

/* Sets up a fight of two different random characters, both played by the
 * computer. The address of the Shuffle stays in a register in C, where the
 * game computes it again for each call. */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", func_8001E264);

/* The side of the player: side 0 when both sides are players, -1 when both
 * are the computer. */
s32 gameStateGetPlayerSide(GameState *this) {
    if (this->fighters[0].computer && this->fighters[1].computer) {
        return -1;
    }
    return this->fighters[0].computer != 0;
}

/* Whether the side unk2D0 is a player. */
s32 func_8001E3B4(GameState *this) {
    if (this->unk2D0 < 0) {
        return 0;
    }
    /* computer is 0 or 1 */
    return this->fighters[this->unk2D0].computer ^ 1;
}

/* unk10 of the player's side, 0 when there is no player. */
s32 func_8001E3E4(GameState *this) {
    s32 side = gameStateGetPlayerSide(this);

    if (side < 0) {
        return 0;
    }
    return this->fighters[side].unk10;
}

/* Has no player joined. */
void joinStateReset(JoinState *this) {
    this->joined0 = 0;
    this->joined1 = 0;
    this->wasJoined = 0;
    this->joined = 0;
    this->entered = 0;
    this->left = 0;
}

/* Has no player joined, as if one just left. */
void joinStateSetLeft(JoinState *this) {
    joinStateReset(this);
    this->left = 1;
}

/* Updates who joined: Start joins a player, or makes a joined one leave;
 * player 2 only joins when player 1 does not. The bit field updates do not
 * come out in the game's order (about 100 lines differ). */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/game_state", joinStateUpdate);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_800101B8);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", STARTING_CHARACTERS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CODE_DIGESTS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CLEAR_UNLOCKS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_800102A8);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CHARACTER_HOMES);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_800102D4);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", D_80010304);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", CHARACTER_GROUPS);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/game_state", ARENA_PREFERENCES);
