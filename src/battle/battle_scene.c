/* Fight (virtual table FIGHT_VTABLE), one fight: the sides' fighters and input, the evolution, effect and map
 * files, the rounds, and the continue or quit choice. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/cd/file_load.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/debug/system_menu.h"
#include "engine/gfx/box.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_line_f4.h"
#include "engine/gfx/prim/alloc_poly_f3.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/build_line_f2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/lib/string.h"
#include "engine/math/block_copy.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "engine/system/main.h"
#include "engine/system/memory.h"
#include "stdio.h"

void Fight_run(Fight *object);
void Fight_showContinue(Fight *this);
void Fight_playIntro(Fight *this);
s32 Fight_playRound(Fight *this);
void func_80071894(Fight *this, s32 side);
void Fight_playTimeUpWin(Fight *this, s32 side);
void Fight_playDoubleKnockOut(Fight *this);
void Fight_playDraw(Fight *this);
void Fight_playInterruption(Fight *this);
void Fight_wipeOut(Fight *this);
void Fight_end(Fight *this);
void Fight_resetRound(Fight *this);
s32 Fight_waitEntity(Fight *this, Entity *entity, s32 port);
void Fight_wipeIn(Fight *this);
void Fight_releaseClock(Fight *this);

INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_DIRECTION_0);
INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_DIRECTION_1);
INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_DIRECTION_2);
INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_COLOR_0);
INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_COLOR_1);
INCLUDE_RODATA("game/nonmatchings/battle_scene", LIGHT_COLOR_2);
INCLUDE_RODATA("game/nonmatchings/battle_scene", BACK_COLOR);

extern VECTOR LIGHT_DIRECTION_0;
extern VECTOR LIGHT_DIRECTION_1;
extern VECTOR LIGHT_DIRECTION_2;
extern VECTOR LIGHT_COLOR_0;
extern VECTOR LIGHT_COLOR_1;
extern VECTOR LIGHT_COLOR_2;
extern VECTOR BACK_COLOR;

extern VtableEntry FIGHT_VTABLE[];

void runFightThread(s32 unused, Fight *fight);
void func_8006F1C4(Fight *this, s32 angle);

/* Constructs the fight: its thread (runFightThread, 0x2000 of stack), the entity groups, the groups, the look-at
 * and both cameras, the empty shape lists, the transform at 0x25C, the continue screen's lerps, the background and
 * the random generator; it becomes CURRENT_FIGHT. The camera gets its view and the fight's lights; both input
 * histories and the map's mesh scene are made, no side has a computer controller nor a chara yet, func_8006F1C4
 * with 0, five rounds at most and two wins needed, no wins nor gauges. Seeds the random generator and applies the
 * save's button maps. The match depends on GAME_STATE taken through a local pointer for the button maps. */
Fight *Fight_init(Fight *this) {
    this->vtable = FIGHT_VTABLE;
    threadInit(&this->thread, runFightThread, 0x2000, this, 0);
    entityGroupInit(&this->stageEntities);
    entityGroupInit(&this->entities);
    entityGroupInit(&this->unk5C);
    groupInit(&this->fighterGroup);
    groupInit(&this->unkCC);
    lookAtInit(&this->lookAt);
    cameraInit(&this->camera, 11);
    cameraInitShared(&this->camera2, 11, &this->camera);
    this->side1Shapes.next = &this->side1Shapes;
    this->side1Shapes.prev = &this->side1Shapes;
    this->side0Shapes.next = &this->side0Shapes;
    this->side0Shapes.prev = &this->side0Shapes;
    this->stageShapes.next = &this->stageShapes;
    this->stageShapes.prev = &this->stageShapes;
    transformInit((Transform *)this->unk25C);
    *(s32 *)this->unk2B4 = 0;
    lerpInit(&this->continueDistance, 0);
    lerpInit(&this->continueTilt, 0);
    spriteInit(&this->background);
    mersenneTwisterInit(&this->random);
    CURRENT_FIGHT = this;
    cameraReset(&this->camera);
    cameraSetFov(&this->camera, 0x238);
    cameraSetFar(&this->camera, 0x3200);
    cameraSetLightDirection0(&this->camera, &LIGHT_DIRECTION_0);
    cameraSetLightDirection1(&this->camera, &LIGHT_DIRECTION_1);
    cameraSetLightDirection2(&this->camera, &LIGHT_DIRECTION_2);
    cameraSetLightColor0(&this->camera, &LIGHT_COLOR_0);
    cameraSetLightColor1(&this->camera, &LIGHT_COLOR_1);
    cameraSetLightColor2(&this->camera, &LIGHT_COLOR_2);
    cameraSetBackColor(&this->camera, &BACK_COLOR);
    this->inputs[0] = InputHistory_init(operatorNew(0x218));
    this->inputs[1] = InputHistory_init(operatorNew(0x218));
    this->controllers[0] = NULL;
    this->controllers[1] = NULL;
    this->meshScene = meshSceneInit(operatorNew(0x10C));
    func_8006F1C4(this, 0);
    this->charas[0] = NULL;
    this->charas[1] = NULL;
    this->charas[2] = NULL;
    this->charas[3] = NULL;
    this->roundsLeft = 5;
    this->winsNeeded = 2;
    this->wins[0] = 0;
    this->wins[1] = 0;
    this->gauges[0] = 0;
    this->gauges[1] = 0;
    seedRandom(&this->random);
    {
        GameState *state = &GAME_STATE;

        saveDataApplyButtons(&state->save, &state->maps[0], &state->maps[1]);
    }
    return this;
}

/* Returns the fight's camera. */
Camera *Fight_getCamera(Fight *this) {
    return &this->camera;
}

/* Returns the camera's look-at. */
LookAt *Fight_getLookAt(Fight *this) {
    return &this->lookAt;
}

/* Returns the score of side. */
s32 Fight_getScore(Fight *this, s32 side) {
    return GAME_STATE.fighters[side].score;
}

/* Adds add to the score of side. */
void Fight_addScore(Fight *this, s32 side, s32 add) {
    GAME_STATE.fighters[side].score += add;
}

/* Unlocks the bonus game of arena in the save data, for the main menu's bonus modes. */
void unlockBonusGame(s16 arena) {
    GAME_STATE.save.bonusGamesUnlocked |= ARENA_BIT(arena);
}

/* Returns the rounds side has won. */
s32 Fight_getWins(Fight *this, s32 side) {
    return this->wins[side];
}

/* Returns the vector at 0x2A4. */
VECTOR *func_8006EFFC(Fight *this) {
    return &this->unk2A4;
}

/* Returns a random number from the fight's own generator. */
u32 Fight_random(Fight *this) {
    return mersenneTwisterGenerate(&this->random);
}

/* Adds a collision shape of side 1's fighter. */
void Fight_addSide1Shape(Fight *this, ListNode *node) {
    listAppend(&this->side1Shapes, node);
}

/* Adds a collision shape of side 0's fighter. */
void Fight_addSide0Shape(Fight *this, ListNode *node) {
    listAppend(&this->side0Shapes, node);
}

/* Adds a collision shape of the stage's (or an item's). */
void Fight_addStageShape(Fight *this, ListNode *node) {
    listAppend(&this->stageShapes, node);
}

/* Adds a stage entity, if there is one; returns it. */
Entity *Fight_addStageEntity(Fight *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->stageEntities, entity);
    }
    return entity;
}

/* Adds an entity (a fighter, an effect, a message), if there is one; returns it. */
Entity *Fight_addEntity(Fight *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->entities, entity);
    }
    return entity;
}

/* Adds an entity to the group at 0x5C; returns the group. */
EntityGroup *func_8006F0E4(Fight *this, Entity *entity) {
    return entityGroupAppend(&this->unk5C, entity);
}

/* Whether the group at 0x5C holds any entity. */
s32 func_8006F104(Fight *this) {
    return entityGroupGetCount(&this->unk5C) != 0;
}

/* Sets bit 2 of the flags to flag (set when a round ends). */
void func_8006F124(Fight *this, s32 flag) {
    this->flags = (this->flags & ~4) | ((flag & 1) << 2);
}

/* Whether bit 2 of the flags is set or the group at 0x5C holds any entity (func_8006F104). */
s32 func_8006F144(Fight *this) {
    return ((this->flags >> 2) & 1) || func_8006F104(this);
}

/* Whether the round is over (FIGHT_FLAG_ROUND_OVER). */
s32 Fight_isRoundOver(Fight *this) {
    return (this->flags >> 4) & 1;
}

/* Whether the continue prompt is up (FIGHT_FLAG_CONTINUE). */
s32 Fight_isContinueShown(Fight *this) {
    return (this->flags >> 8) & 1;
}

/* Returns the map's mesh scene. */
MeshScene *Fight_getMeshScene(Fight *this) {
    return this->meshScene;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F1C4);


void Fight_destroySides(Fight *this);

/* Destroys the fight: its sides, both input histories, the charas and the map, the background, the three shape
 * lists, both cameras, the groups, the entity groups and the thread; frees it when flags has bit 0. The match
 * depends on each list's removal written out in a block of its own with its own node pointer. */
void Fight_destroy(Fight *this, s32 flags) {
    this->vtable = FIGHT_VTABLE;
    Fight_destroySides(this);
    operatorDelete(this->inputs[0]);
    operatorDelete(this->inputs[1]);
    if (this->charas[0] != NULL) {
        charaDestroy(this->charas[0], 3);
    }
    if (this->charas[1] != NULL) {
        charaDestroy(this->charas[1], 3);
    }
    if (this->charas[2] != NULL) {
        charaDestroy(this->charas[2], 3);
    }
    if (this->charas[3] != NULL) {
        charaDestroy(this->charas[3], 3);
    }
    if (this->meshScene != NULL) {
        meshSceneDestroy(this->meshScene, 3);
    }
    spriteDestroy(&this->background, 2);
    /* listRemove of the three shape lists, in the access order the game's compiler used */
    {
        ListNode *node = &this->stageShapes;

        this->stageShapes.next->prev = this->stageShapes.prev;
        node->prev->next = this->stageShapes.next;
        this->stageShapes.next = node;
        this->stageShapes.prev = node;
    }
    {
        ListNode *node = &this->side0Shapes;

        this->side0Shapes.next->prev = this->side0Shapes.prev;
        node->prev->next = this->side0Shapes.next;
        this->side0Shapes.next = node;
        this->side0Shapes.prev = node;
    }
    {
        ListNode *node = &this->side1Shapes;

        this->side1Shapes.next->prev = this->side1Shapes.prev;
        node->prev->next = this->side1Shapes.next;
        this->side1Shapes.next = node;
        this->side1Shapes.prev = node;
    }
    cameraDestroy(&this->camera2, 2);
    cameraDestroy(&this->camera, 2);
    groupDestroy(&this->unkCC, 2);
    groupDestroy(&this->fighterGroup, 2);
    entityGroupDestroy(&this->unk5C, 2);
    entityGroupDestroy(&this->entities, 2);
    entityGroupDestroy(&this->stageEntities, 2);
    threadDestroy(&this->thread, 2);
    if (flags & 1) {
        operatorDelete(this);
    }
}

/* by character: the tint of its other colors (FighterSetup.altColor) */
extern HsvColor D_80064F44[];
/* the tint that leaves the colors as they are */
extern HsvColor D_80064F40;

static inline HsvColor *getTint(HsvColor *table, s32 character) {
    return &table[character];
}

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D38);

/* the CLUT rectangle of the fighters' palettes in VRAM (side 0's row, then the alternatives) */
extern RECT D_80064D38;
void shiftColors(u16 *src, u16 *dst, u32 count, HsvColor *shift);

/* The other colors of the character and of its evolution (CHARACTER_EVOLUTIONS): reads the palette row at
 * D_80064D38 from VRAM and loads it, tinted by each one's D_80064F44, into the next two rows. With FIGHTER_PARAMS
 * flag bit 0 the evolution's tint goes first (none when it has none), else the character's. */
void func_8006F414(s32 character) {
    u16 *clut = mainHeapAllocBest(0x200);
    u16 *tinted = mainHeapAllocBest(0x200);
    RECT rect = D_80064D38;
    s32 other;

    StoreImage2(&rect, (u_long *)clut);
    if (FIGHTER_PARAMS[character].flags & 1) {
        other = CHARACTER_EVOLUTIONS[character];
        if (other == character) {
            other = -1;
        }
    } else {
        other = character;
        character = CHARACTER_EVOLUTIONS[other];
    }
    rect.y++;
    if (other >= 0) {
        /* The match depends on the table passed through getTint. */
        shiftColors(clut, tinted, 0x100, getTint(D_80064F44, other));
        LoadImage2(&rect, (u_long *)tinted);
    }
    rect.y++;
    shiftColors(clut, tinted, 0x100, getTint(D_80064F44, character));
    LoadImage2(&rect, (u_long *)tinted);
    mainHeapFree(tinted);
    mainHeapFree(clut);
}

/* the tamer of each character (the name in its evolution TIMs' paths), NULL for none */
extern char *CHARACTER_TAMERS[];

/* Puts the evolution TIMs of the character's tamer for player in VRAM: "evolution/<tamer>_<player + 1>p.tim", or
 * "..._sc.tim" with sc; the two forms of two partners take one of their tamers at random, characters with no tamer
 * nothing. The match depends on the system's random generator read before the switch. */
void Fight_loadTamerTim(s32 character, s32 player, s32 sc) {
    char path[0x80];
    char *tamer;
    u32 *file;
    MersenneTwister *random = SYSTEM_CONTEXT.random;

    switch (character) {
    case CHARACTER_IMPERIALDRAMON_PALADIN_MODE:
        if (mersenneTwisterGenerate(random) & 1) {
            tamer = CHARACTER_TAMERS[CHARACTER_IMPERIALDRAMON];
        } else {
            tamer = CHARACTER_TAMERS[CHARACTER_STINGMON];
        }
        break;
    case CHARACTER_OMEGAMON:
        if (mersenneTwisterGenerate(random) & 1) {
            tamer = CHARACTER_TAMERS[CHARACTER_WARGREYMON];
        } else {
            tamer = CHARACTER_TAMERS[CHARACTER_METALGARURUMON];
        }
        break;
    default:
        tamer = CHARACTER_TAMERS[character];
        break;
    }
    if (tamer != NULL) {
        sprintf(path, "evolution/%s_%dp%s.tim", tamer, player + 1, sc ? "_sc" : "");
        file = loadFile(path);
        uploadTims(file);
        mainHeapFree(file);
    }
}

/* Puts the character's effect TIMs (effect/mon_efcNN.tim) in VRAM, 256 lines lower for side 1. */
void Fight_loadCharacterEffectTim(s32 character, s32 side) {
    char path[0x80];
    TIM_IMAGE tim;
    u_long *file;
    s32 offset;

    sprintf(path, "effect/mon_efc%02d.tim", character);
    file = loadFile(path);
    OpenTIM(file);
    offset = (side != 0) << 8;
    while (ReadTIM(&tim) != NULL) {
        tim.crect->y += offset;
        tim.prect->y += offset;
        if (tim.caddr != NULL) {
            LoadImage(tim.crect, tim.caddr);
        }
        if (tim.paddr != NULL) {
            LoadImage(tim.prect, tim.paddr);
        }
        DrawSync(0);
    }
    mainHeapFree(file);
}

/* the background's CLUT and image rectangles in VRAM */
extern RECT D_80064DDC;
extern RECT D_80064DE4;
void func_8006F414(s32 character);
void Fight_createSides(Fight *this);

/* Loads the fight: each side's alternative colors (func_8006F414), the effect TIMs, the map (map/mapNN.dmm), and
 * unless the arena is the basketball or target bonus game, the tamers' evolution TIMs; then the characters' effect TIMs
 * and charas, the evolved forms' too, the background sprite, and the sides (Fight_createSides). */
void Fight_load(Fight *this) {
    GameState *state = &GAME_STATE;
    s32 character0 = state->fighters[0].character;
    s32 character1 = state->fighters[1].character;
    s32 alt1 = state->fighters[1].altColor;
    s32 evolved0 = CHARACTER_EVOLUTIONS[character0];
    s32 evolved1 = CHARACTER_EVOLUTIONS[character1];
    s32 alt0 = state->fighters[0].altColor;
    HsvColor *tint0 = alt0 ? getTint(D_80064F44, character0) : &D_80064F40;
    HsvColor *evolvedTint0 = alt0 ? getTint(D_80064F44, evolved0) : &D_80064F40;
    HsvColor *tint1 = alt1 ? getTint(D_80064F44, character1) : &D_80064F40;
    HsvColor *evolvedTint1 = alt1 ? getTint(D_80064F44, evolved1) : &D_80064F40;
    u32 *file;
    union {
        char path[0x40];
        TIM_IMAGE tim;
    } buf;

    if (alt0) {
        func_8006F414(character0);
    }
    if (alt1) {
        func_8006F414(character1);
    }
    file = loadFile("effect/effect.tim");
    uploadTims(file);
    mainHeapFree(file);
    sprintf(buf.path, "map/map%02d.dmm", state->arena);
    meshSceneLoad(this->meshScene, buf.path);
    EVOLUTION_LOADED =
        state->arena != ARENA_BONUS + BONUS_GAME_BASKETBALL && state->arena != ARENA_BONUS + BONUS_GAME_TARGET;
    if (EVOLUTION_LOADED) {
        Fight_loadTamerTim(character0, 0, state->fighters[0].tamer);
        Fight_loadTamerTim(character1, 1, state->fighters[1].tamer);
    }
    /* The match depends on the do { } while (0) with the break before the evolutions' charas, the table in getTint
     * and the side 1 tests written character1 != evolved1. */
    do {
        Fight_loadCharacterEffectTim(character0, 0);
        Fight_loadCharacterEffectTim(character1, 1);
        if (EVOLUTION_LOADED) {
            if (evolved0 != character0) {
                Fight_loadCharacterEffectTim(evolved0, 0);
            }
            if (character1 != evolved1) {
                Fight_loadCharacterEffectTim(evolved1, 1);
            }
        }
        this->charas[0] = charaInit(operatorNew(0x30), character0);
        this->charas[1] = charaInit(operatorNew(0x30), character1);
        charaSetPlayerTint(this->charas[0], 0, tint0);
        charaSetPlayerTint(this->charas[1], 1, tint1);
        charaLoad(this->charas[0]);
        charaLoad(this->charas[1]);
        if (!EVOLUTION_LOADED) {
            break;
        }
        if (evolved0 != character0) {
            this->charas[2] = charaInit(operatorNew(0x30), evolved0);
            charaSetPlayerTint(this->charas[2], 0, evolvedTint0);
            charaLoad(this->charas[2]);
        }
        if (character1 != evolved1) {
            this->charas[3] = charaInit(operatorNew(0x30), evolved1);
            charaSetPlayerTint(this->charas[3], 1, evolvedTint1);
            charaLoad(this->charas[3]);
        }
    } while (0);
    buf.tim.mode = 9;
    buf.tim.crect = &D_80064DDC;
    buf.tim.prect = &D_80064DE4;
    spriteSetTim(&this->background, &buf.tim, 0);
    this->hasBackground = 1;
    this->background.flags |= SPRITE_DITHER;
    Fight_createSides(this);
    this->flags &= ~FIGHT_FLAG_OVER;
    this->flags &= ~FIGHT_FLAG_STOP_ROUND;
}

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064DDC);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064DE4);

INCLUDE_ASM("game/nonmatchings/battle_scene", Fight_createSides);

/* Ends a round's sides: deletes the computer players, takes the fighters out of the entity lists, destroys the
 * stage's, the fight's and the waiting entities, keeps each side's evolution gauge for the next round, then deletes
 * both sides. */
void Fight_destroySides(Fight *this) {
    if (this->controllers[0] != NULL) {
        this->controllers[0]->vtable[2].func((u8 *)this->controllers[0] + this->controllers[0]->vtable[2].delta,
                                             DESTROY_DELETE);
    }
    if (this->controllers[1] != NULL) {
        this->controllers[1]->vtable[2].func((u8 *)this->controllers[1] + this->controllers[1]->vtable[2].delta,
                                             DESTROY_DELETE);
    }
    BattleSide_unlinkCharacters(this->sides[0]);
    BattleSide_unlinkCharacters(this->sides[1]);
    entityGroupDestroyAll(&this->stageEntities);
    this->gauges[0] = this->sides[0]->gauge;
    this->gauges[1] = this->sides[1]->gauge;
    entityGroupDestroyAll(&this->entities);
    entityGroupDestroyAll(&this->unk5C);
    if (this->sides[0] != NULL) {
        this->sides[0]->vtable[1].func((u8 *)this->sides[0] + this->sides[0]->vtable[1].delta, DESTROY_DELETE);
    }
    if (this->sides[1] != NULL) {
        this->sides[1]->vtable[1].func((u8 *)this->sides[1] + this->sides[1]->vtable[1].delta, DESTROY_DELETE);
    }
    this->sides[0] = NULL;
    this->sides[1] = NULL;
}

/* Fight_createNodeEntity, the method at 0x50 of Fight's virtual table, which the stages override:
 * (this, index, meshScene) gives the entity of node index of the map, or NULL. */
typedef Entity *(*FightNodeEntityFunc)();

/* Creates the entities of the map's nodes (Fight_createNodeEntity) and adds them to the stage entities. */
void Fight_createNodeEntities(Fight *this) {
    s32 count = meshSceneGetNodeCount(this->meshScene);
    s32 i;
    Entity *entity;

    for (i = 0; i < count; i++) {
        entity = ((FightNodeEntityFunc)this->vtable[10].func)((u8 *)this + this->vtable[10].delta, i,
                                                                 this->meshScene);
        if (entity != NULL) {
            entityGroupAppend(&this->stageEntities, entity);
        }
    }
}

/* Creates the entity of the map's node index (Fight_createNodeEntity); NULL for an index out of range. */
Entity *Fight_createNodeEntityAt(Fight *this, s32 index) {
    if (index < meshSceneGetNodeCount(this->meshScene) && index >= 0) {
        return ((FightNodeEntityFunc)this->vtable[10].func)((u8 *)this + this->vtable[10].delta, index,
                                                               this->meshScene);
    }
    return NULL;
}

/* The map nodes' entities: none in the base class. */
s32 Fight_createNodeEntity(void) {
    return 0;
}

/* Returns the fighter of side. */
Fighter *Fight_getFighter(Fight *this, s32 side) {
    return this->sides[side]->current;
}

/* Sets the arena's bounds (the stages pass their own), and 0x35C to half the smaller of the width and 4/3 of
 * the height. */
void Fight_setBounds(Fight *this, s32 left, s32 right, s32 top, s32 bottom, s32 depth) {
    s32 size = ((bottom - top) << 2) / 3;

    this->boundLeft = left;
    this->boundTop = top;
    this->boundRight = right;
    this->boundBottom = bottom;
    this->boundDepth = depth;
    if (right - left < size) {
        size = right - left;
    }
    this->unk35C = size >> 1;
}

#if VERSION_JP
INCLUDE_ASM("game/nonmatchings/battle_scene", Fight_startRound);
#else
/* Starts a round: clears the display, adds both sides' fighters to the fighters' group and the entities, puts them
 * at the arena's start positions and runs func_8007F730 on both sides; the camera's eye goes to (0, 0, -0x6400)
 * and its target to 0. Clears the frame count and sets the time left (3000 frames, or 1 with no time limit),
 * clears the bonus mode and bit 0 of unk318 and bit 2 of the flags, freezes the fighters, stops the clock, starts
 * the wipe at 0x20 and resets the camera focus and the pause menu, then clears FIGHT_FLAG_ROUND_OVER and
 * FIGHT_FLAG_SLOW. The match depends on the position passed through a pointer of its own block, the time left set
 * in an if/else and the two flags cleared one by one. (US only: JP's pause menu has no pausePage.) */
void Fight_startRound(Fight *this) {
    Fighter *fighter0;
    Fighter *fighter1;
    DVECTOR *start;
    Vec3 position;

    displayClear(CURRENT_DISPLAY);
    fighter0 = this->sides[0]->current;
    fighter1 = this->sides[1]->current;
    Fighter_addModelTo(fighter0, &this->fighterGroup);
    Fighter_addModelTo(fighter1, &this->fighterGroup);
    entityGroupAppend(&this->entities, &fighter0->entity);
    entityGroupAppend(&this->entities, &fighter1->entity);
    start = ARENA_START_POSITIONS[this->arena];
    {
        Vec3 *p = &position;

        position.vx = start[0].vx;
        position.vy = start[0].vy;
        position.vz = 0;
        Fighter_setPosition(fighter0, p);
        position.vx = start[1].vx;
        position.vy = start[1].vy;
        position.vz = 0;
        Fighter_setPosition(fighter1, p);
    }
    func_8007F730(this->sides[0]);
    func_8007F730(this->sides[1]);
    this->eye.vx = 0;
    this->eye.vy = 0;
    this->eye.vz = -0x6400;
    this->target.vx = 0;
    this->target.vy = 0;
    this->target.vz = 0;
    this->steps = 0;
    if (func_8001DECC(&GAME_STATE)) {
        this->timeLeft = 1;
    } else {
        this->timeLeft = 3000;
    }
    this->modeFlags &= ~FIGHT_MODE_BONUS;
    this->unk318 &= ~1;
    func_8006F124(this, 0);
    Fight_freezeFighters(this);
    Fight_holdClock(this);
    this->wipe = 0x20;
    this->cameraFocus = 0;
    this->pauseCursor = 0;
    this->pausePage = 0;
    this->pauseSide = 0;
    this->pauseButtons = 0;
    this->flags &= ~FIGHT_FLAG_ROUND_OVER;
    this->flags &= ~FIGHT_FLAG_SLOW;
}
#endif

/* Creates the computer player of a side without a pad, for its side object; the bonus stages make their own. */
void *Fight_createController(void *this, s32 side, BattleSide *battleSide) {
    return ComputerController_init(operatorNew(0x6C), battleSide);
}
void Fight_finishStep(Fight *this);
void Fight_updateSides(Fight *this);

/* A frame of the fight: switches to its thread for one frame, then ticks the group at 0x5C (with no input for
 * either side) while it holds any entity, else updates the sides and ticks the stage entities. */
void Fight_tick(Fight *this) {
    threadSwitchTo(&this->thread, 0);
    if (func_8006F104(this)) {
        entityGroupTick(&this->unk5C, (s32)this);
        InputHistory_record(this->inputs[0], 0);
        InputHistory_record(this->inputs[1], 0);
        Fight_finishStep(this);
    } else {
        Fight_updateSides(this);
        entityGroupTick(&this->stageEntities, (s32)this);
        Fight_finishStep(this);
    }
}

/* The entry of the fight's thread. */
void runFightThread(s32 unused, Fight *fight) {
    Fight_run(fight);
}

void Fight_readInputs(Fight *this);
void func_800708C4(Fight *this);
void func_800708F4(Fight *this);
void Fight_catchFalls(Fight *this);

/* Reads the inputs and ticks the entities; unless the round is over or the fighters are frozen
 * (FIGHT_FLAG_FREEZE), ticks both sides before and pushes and hits after; catches falls unless the continue
 * prompt is up. */
void Fight_updateSides(Fight *this) {
    s32 skipSides;

    Fight_readInputs(this);
    skipSides = Fight_isRoundOver(this) || ((this->flags >> 5) & 1);
    if (!skipSides) {
        BattleSide_tickGauges(this->sides[0]);
        BattleSide_tickGauges(this->sides[1]);
    }
    entityGroupTick(&this->entities, (s32)this);
    if (!skipSides) {
        func_800708C4(this);
        func_800708F4(this);
    }
    if (!Fight_isContinueShown(this)) {
        Fight_catchFalls(this);
    }
}

s32 Controller_getButtons(Controller *this);
u32 func_800C9938(Fighter *fighter, u32 buttons);


/* Reads both pads into the sides' input histories. The buttons still held from the pause are taken out of the
 * pausing side's pad until released; with FIGHT_FLAG_FREEZE both record nothing. The buttons go through the
 * players' button maps; the bonus mode keeps only some of them (arena 9 keeps others). A computer side's buttons
 * come from its controller, and outside the bonus mode item effect 3 changes them (func_800C9938). The match
 * depends on the bonus mask set in an if/else on arena != 9. */
void Fight_readInputs(Fight *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    Fighter *fighter0 = this->sides[0]->current;
    Fighter *fighter1 = this->sides[1]->current;
    u32 buttons0 = padManagerGetHeld(pads, 0);
    u32 buttons1 = padManagerGetHeld(pads, 1);
    u32 held = this->pauseButtons;

    if (held) {
        switch (this->pauseSide) {
        case 0:
            held &= buttons0;
            buttons0 ^= held;
            break;
        case 1:
            held &= buttons1;
            buttons1 ^= held;
            break;
        }
        this->pauseButtons = held;
    }
    if ((this->flags >> 5) & 1) {
        InputHistory_record(this->inputs[0], 0);
        InputHistory_record(this->inputs[1], 0);
        return;
    }
    buttons0 = buttonMapApply(&GAME_STATE.maps[0], buttons0);
    buttons1 = buttonMapApply(&GAME_STATE.maps[1], buttons1);
    if (this->modeFlags & FIGHT_MODE_BONUS) {
        u32 mask;

        if (GAME_STATE.arena != 9) {
            mask = 0xFFFF0F83;
        } else {
            mask = -0x507D;
        }
        buttons0 &= mask;
        buttons1 &= mask;
        if (fighter0->flags.bits.computer) {
            buttons0 = Controller_getButtons(this->controllers[0]);
        }
        if (fighter1->flags.bits.computer) {
            buttons1 = Controller_getButtons(this->controllers[1]);
        }
    } else {
        if (fighter0->flags.bits.computer) {
            buttons0 = Controller_getButtons(this->controllers[0]);
        }
        if (fighter1->flags.bits.computer) {
            buttons1 = Controller_getButtons(this->controllers[1]);
        }
        if (fighter0->itemEffect.kind == 3) {
            buttons0 = func_800C9938(fighter0, buttons0);
        }
        if (fighter1->itemEffect.kind == 3) {
            buttons1 = func_800C9938(fighter1, buttons1);
        }
    }
    InputHistory_record(this->inputs[0], buttons0);
    InputHistory_record(this->inputs[1], buttons1);
}

typedef s32 (*FightIsFallenFunc)(void *this, Vec3 *position);

/* Catches each side's fighter that fell off the arena (the stage's Fight_isFallen): unless the round is over, it
 * loses 0x1000 life and takes a launching hit; then, if it still has life, it is put back at its respawn
 * position facing the middle, with a burst of particles. */
void Fight_catchFalls(Fight *this) {
    s32 side;
    Fighter *fighter;
    Vec3 position;

    for (side = 0; side < 2; side++) {
        fighter = this->sides[side]->current;
        Fighter_getPosition(fighter, &position);
        if (((FightIsFallenFunc)this->vtable[11].func)((u8 *)this + this->vtable[11].delta, &position)) {
            if (!Fight_isRoundOver(this)) {
                BattleSide_takeDamage(this->sides[side], 0x1000);
                Fighter_takeHit(fighter, HIT_FLAG_LAUNCH, (Vec3 *)&ZERO_VECTOR, 0);
            }
            if (this->sides[side]->life > 0) {
                Fight_respawnFighter(this, side);
                if (side & 1) {
                    Fighter_faceLeft(fighter, 0);
                } else {
                    Fighter_faceRight(fighter, 0);
                }
                func_8007A650(fighter, &position);
                Fight_addEntity(this, NEW_ENTITY(0x3C, ParticleBurst_init, 9, &position, 0x40, 0x20, -30, 0x80C0FF));
            }
        }
    }
}

/* Whether position has fallen more than 0x1000 below the floor (y grows downwards). */
s32 Fight_isFallen(Fight *this, Vec3 *position) {
    return this->boundBottom + 0x1000 < position->vy;
}

/* Puts side's fighter back at the arena's respawn position (ARENA_RESPAWN_POSITIONS), at rest, invulnerable for 3
 * frames. */
void Fight_respawnFighter(Fight *this, s32 side) {
    DVECTOR *spawn = &ARENA_RESPAWN_POSITIONS[this->arena][side];
    Fighter *fighter = this->sides[side]->current;
    Vec3 position;

    fighter->velocity.vx = 0;
    fighter->velocity.vy = 0;
    fighter->velocity.vz = 0;
    fighter->velocityPad = 0;
    fighter->force.vx = 0;
    fighter->force.vy = 0;
    fighter->force.vz = 0;
    fighter->forcePad = 0;
    position.vx = spawn->vx;
    position.vy = spawn->vy;
    position.vz = 0;
    Fighter_setPosition(fighter, &position);
    fighter->invulnerableFrames = 3;
}

/* Runs func_8007A158 on both sides' fighters. */
void func_800708C4(Fight *this) {
    func_8007A158(this->sides[0]->current, this->sides[1]->current);
}

s32 Fighter_getBodyBoxCount(Fighter *this);
s32 Fighter_getAttackBoxCount(Fighter *this);
Box *Fighter_getBodyBoxes(Fighter *this);
Box *Fighter_getAttackBoxes(Fighter *this);
s32 Fighter_isInvulnerable(Fighter *this);
Box *boxesFindOverlap(Box *boxes, s32 count, Box *others, s32 othersCount);
CollisionSphere *Fighter_resetCollider(Fighter *this);
void shapeListCollide(ListNode *list, CollisionSphere *other);
void shapeListsCollide(ListNode *list, ListNode *others);

/* The hits between the sides' fighters: each one's attack boxes against the other's body boxes (boxesFindOverlap,
 * unless it is idle or the other invulnerable) through the hitOpponent slot, with a copy of the matrix of the box
 * met; then each fighter's collider against the stage's shapes and the other side's (Fighter_takeHit of what it
 * gathered), and the three shape lists against each other. */
void func_800708F4(Fight *this) {
    Fighter *fighter0;
    Fighter *fighter1;
    s32 bodyCount0;
    s32 bodyCount1;
    s32 attackCount0;
    s32 attackCount1;
    Box *body0;
    Box *body1;
    Box *attack0;
    Box *attack1;
    Box *hit0;
    Box *hit1;
    MATRIX hit;

    /* The match depends on the two NULLs assigned first, hit1 before hit0. */
    hit1 = NULL;
    hit0 = NULL;
    fighter0 = this->sides[0]->current;
    fighter1 = this->sides[1]->current;
    bodyCount0 = Fighter_getBodyBoxCount(fighter0);
    bodyCount1 = Fighter_getBodyBoxCount(fighter1);
    attackCount0 = Fighter_getAttackBoxCount(fighter0);
    attackCount1 = Fighter_getAttackBoxCount(fighter1);
    body0 = Fighter_getBodyBoxes(fighter0);
    body1 = Fighter_getBodyBoxes(fighter1);
    attack0 = Fighter_getAttackBoxes(fighter0);
    attack1 = Fighter_getAttackBoxes(fighter1);


    if (!Fighter_isAttackIdle(fighter0) && !Fighter_isInvulnerable(fighter1)) {
        hit0 = boxesFindOverlap(attack0, attackCount0, body1, bodyCount1);
    }
    if (!Fighter_isAttackIdle(fighter1) && !Fighter_isInvulnerable(fighter0)) {
        hit1 = boxesFindOverlap(attack1, attackCount1, body0, bodyCount0);
    }
    if (hit0 != NULL) {
        hit = *hit0->matrix;
        GET_FIGHTER_VTABLE(fighter0)->hitOpponent.func(
            (u8 *)fighter0 + GET_FIGHTER_VTABLE(fighter0)->hitOpponent.delta, fighter1, &hit);
    }
    if (hit1 != NULL) {
        hit = *hit1->matrix;
        GET_FIGHTER_VTABLE(fighter1)->hitOpponent.func(
            (u8 *)fighter1 + GET_FIGHTER_VTABLE(fighter1)->hitOpponent.delta, fighter0, &hit);
    }
    if (!Fighter_isInvulnerable(fighter0)) {
        CollisionSphere *collider = Fighter_resetCollider(fighter0);

        shapeListCollide(&this->stageShapes, collider);
        shapeListCollide(&this->side1Shapes, collider);
        if (collider->hitDamage != 0) {
            Fighter_takeHit(fighter0, collider->hitDamage, &collider->hitKnockback, collider->hitKind);
        }
    }
    if (!Fighter_isInvulnerable(fighter1)) {
        CollisionSphere *collider = Fighter_resetCollider(fighter1);

        shapeListCollide(&this->stageShapes, collider);
        shapeListCollide(&this->side0Shapes, collider);
        if (collider->hitDamage != 0) {
            Fighter_takeHit(fighter1, collider->hitDamage, &collider->hitKnockback, collider->hitKind);
        }
    }
    shapeListsCollide(&this->side1Shapes, &this->side0Shapes);
    shapeListsCollide(&this->side1Shapes, &this->stageShapes);
    shapeListsCollide(&this->side0Shapes, &this->stageShapes);
}

/* Counts the step, updates the camera (Fight_updateCamera), then applies its view. */
void Fight_finishStep(Fight *this) {
    this->steps++;
    this->vtable[12].func((u8 *)this + this->vtable[12].delta);
    Fight_applyView(this);
}

/* The round's result so far: a ROUND_* once a side's life runs out or the time is up, ROUND_CHALLENGER when a
 * player presses Start on a computer side, else 0 (the round goes on). While the clock is held the round goes on
 * and the frame is given back, unless save.unk8. */
s32 Fight_getRoundResult(Fight *this) {
    s32 life0;
    s32 life1;

    if ((this->flags >> 6) & 1) { /* FIGHT_FLAG_HOLD_CLOCK */
        if (!func_8001DECC(&GAME_STATE)) {
            this->timeLeft++;
        }
        return 0;
    }
    life0 = this->sides[0]->life;
    life1 = this->sides[1]->life;
    if (life0 <= 0) {
        if (life1 <= 0) {
            return ROUND_DOUBLE_KO;
        }
    } else if (life1 <= 0) {
        return ROUND_WON_SIDE_0;
    }
    if (life0 <= 0) {
        return ROUND_WON_SIDE_1;
    }
    if (this->timeLeft > 0) {
        joinStateUpdate(&PAUSE_JOIN);
        if (PAUSE_JOIN.entered && GAME_STATE.fighters[PAUSE_JOIN.side].computer &&
            !(this->modeFlags & FIGHT_MODE_BONUS)) {
            joinStateReset(&PAUSE_JOIN);
            return ROUND_CHALLENGER;
        }
        return 0;
    }
    if (life0 == life1) {
        return ROUND_DRAW;
    }
    if (life0 > life1) {
        return ROUND_WON_SIDE_0;
    }
    return ROUND_WON_SIDE_1;
}

/* Shows a round's winner, as bonusShowWinner with a cry first (0x41, or 0x40 for YOU LOSE): YOU WIN to a human
 * against the computer, YOU LOSE when the computer beat a human, else P1/P2 WIN; after the last round a human winner
 * also gets the score tally. Returns the tally, or the message. */
Entity *Fight_showWinner(Fight *this, s32 side, s32 lastRound) {
    s32 otherComputer = GAME_STATE.fighters[side ^ 1].computer;
    s32 computer = GAME_STATE.fighters[side].computer;
    s32 kind;
    Entity *entity;

    if (!computer && otherComputer) {
        kind = BATTLE_MESSAGE_YOU_WIN;
        soundControlPlaySound(&SOUND_CONTROL, 0x41, 0);
        soundControlPlaySound(&SOUND_CONTROL, 0x8008, 0);
    } else if (computer && !otherComputer) {
        kind = BATTLE_MESSAGE_YOU_LOSE;
        soundControlPlaySound(&SOUND_CONTROL, 0x40, 0);
        soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    } else {
        kind = BATTLE_MESSAGE_P1_WIN + side;
        soundControlPlaySound(&SOUND_CONTROL, 0x41, 0);
        soundControlPlaySound(&SOUND_CONTROL, 0x8008, 0);
    }
    entity = BattleMessage_init(entityOperatorNew(0x59C), kind, 0);
    Fight_addEntity(this, entity);
    if (lastRound && !computer) {
        entity = ScoreTally_init(entityOperatorNew(0x64), this, side);
        Fight_addEntity(this, entity);
    }
    return entity;
}

/* The bonus of side by kind (BONUS_*) after a won round, on a 0-10000 scale. */
s32 Fight_getBonus(Fight *this, s32 side, s32 kind) {
    BattleSide *battleSide = this->sides[side];
    s32 life = battleSide->life;
    s32 lifeMax = battleSide->lifeMax;

    switch (kind) {
    case BONUS_PERFECT:
        return (life == lifeMax) ? 10000 : 0;
    case BONUS_LIFE:
        return life * 10000 / lifeMax;
    case BONUS_TIME:
    case BONUS_GAME_TIME:
        return this->timeLeft / 30 * 100;
    case BONUS_FLAT:
        return 5000;
    }
    return 0;
}

/* The bonus of side after a won round: perfect, life and time, or in a bonus game flat and time. */
s32 Fight_computeBonus(Fight *this, s32 side) {
    s32 sum;

    if (!(this->modeFlags & FIGHT_MODE_BONUS)) {
        sum = Fight_getBonus(this, side, BONUS_PERFECT);
        sum += Fight_getBonus(this, side, BONUS_LIFE);
        sum += Fight_getBonus(this, side, BONUS_TIME);
    } else {
        sum = Fight_getBonus(this, side, BONUS_FLAT);
        sum += Fight_getBonus(this, side, BONUS_GAME_TIME);
    }
    return sum;
}

/* Waits frames frames (once at least) in the fight's thread. */
void Fight_waitFrames(Fight *this, s32 frames) {
    do {
        threadSwitchBack(&this->thread, 0);
    } while (--frames > 0);
}

/* Ends the round with ROUND_STOPPED (the stages call it). */
void Fight_stopRound(Fight *this) {
    this->flags |= FIGHT_FLAG_STOP_ROUND;
}

/* The fight's thread: plays rounds (Fight_playIntro, then Fight_playRound gives how the round ended) until a side
 * has the wins needed or no round is left. A round won by a side counts in its wins, sets GAME_STATE's winner,
 * adds the computer's win to its rounds not won and plays the other side's sound. Then Fight_showContinue unless
 * the run is over, and Fight_end. */
void Fight_run(Fight *this) {
    s32 side;
    s32 result;
    s32 over;
    Fighter *other;

    for (;;) {
        side = -1;
        GAME_STATE.winner = side;
        Fight_playIntro(this);
        result = Fight_playRound(this);
        if (GAME_STATE.demo) {
            Fight_end(this);
        }
        this->vtable[6].func((u8 *)this + this->vtable[6].delta); /* Fight_endRound */
        if (this->modeFlags & FIGHT_MODE_BONUS) {
            Fight_end(this);
        }
        soundControlFadeOut(&SOUND_CONTROL, 90);
        switch (result) {
        case ROUND_STOPPED:
            Fight_end(this);
            break;
        case ROUND_WON_SIDE_0:
            side = 0;
            GAME_STATE.winner = side;
            this->wins[side]++;
            break;
        case ROUND_WON_SIDE_1:
            side = 1;
            GAME_STATE.winner = side;
            this->wins[side]++;
            break;
        case ROUND_DRAW:
            GAME_STATE.roundsNotWon++;
            Fight_playDraw(this);
            break;
        case ROUND_DOUBLE_KO:
            GAME_STATE.roundsNotWon++;
            Fight_playDoubleKnockOut(this);
            break;
        case ROUND_CHALLENGER:
        case ROUND_SECRET_OPPONENT:
            Fight_playInterruption(this);
            break;
        case ROUND_QUIT:
            Fight_end(this);
            break;
        }
        over = --this->roundsLeft == 0 || this->wins[0] >= this->winsNeeded || this->wins[1] >= this->winsNeeded;
        this->flags = (this->flags & ~FIGHT_FLAG_LAST_ROUND) | (over << 7);
        if (side >= 0) {
            GAME_STATE.winnerCharacter = Fighter_getCharacter(this->sides[side]->current);
            GAME_STATE.roundsNotWon += GAME_STATE.fighters[side].computer;
            other = this->sides[side ^ 1]->current;
            if (this->timeLeft > 0) {
                soundControlPlayEcho(&SOUND_CONTROL, gameStateGetSound(&GAME_STATE, Fighter_getCharacter(other)));
                func_80071894(this, side);
            } else {
                soundControlPlaySound(&SOUND_CONTROL, gameStateGetSound(&GAME_STATE, Fighter_getCharacter(other)), 0);
                Fight_playTimeUpWin(this, side);
            }
            Fight_addScore(this, side, Fight_computeBonus(this, side));
        }
        if ((this->flags >> 7) & 1) {
            break;
        }
        this->vtable[7].func((u8 *)this + this->vtable[7].delta); /* Fight_prepareNextRound */
        Fight_wipeOut(this);
        Fight_resetRound(this);
    }
    if (!gameStateIsOutsideRun(&GAME_STATE) && !gameStateHasPlayerWon(&GAME_STATE)) {
        Fight_showContinue(this);
    }
    Fight_end(this);
}

/* The continue screen after a lost run: wipes out, sets FIGHT_FLAG_CONTINUE, takes the other side's fighter out
 * and puts the player's frozen fighter at the origin, swings the eye in from far (0x1800) to 0x200 while it tilts
 * down, waits 30 frames, starts the slow pull back and waits for the CONTINUE? screen (ContinuePrompt). */
void Fight_showContinue(Fight *this) {
    s32 side;
    Fighter *fighter;

    Fight_wipeOut(this);
    this->continueFrames = 0;
    this->flags |= FIGHT_FLAG_CONTINUE;
    side = gameStateGetPlayerSide(&GAME_STATE);
    if (side < 0) {
        side = 0;
    }
    Fighter_removeModel(this->sides[side ^ 1]->current);
    fighter = this->sides[side]->current;
    Fighter_setPosition(fighter, (Vec3 *)&D_80010824);
    Fighter_freeze(fighter);
    func_80079268(fighter);
    displayDisable(CURRENT_DISPLAY, 0);
    fadeControlFadeTo(&FADE_CONTROL, 0, 0x20);
    lerpStart(&this->continueDistance, 0x1800, 0);
    lerpStart(&this->continueTilt, 0x355, 0);
    Fight_wipeIn(this);
    lerpStart(&this->continueDistance, 0x200, 30);
    lerpStart(&this->continueTilt, 0x18E, 60);
    Fight_waitFrames(this, 30);
    lerpStart(&this->continueDistance, 0x1800, 600);
    Fight_waitEntity(
        this,
        Fight_addEntity(this, &ContinuePrompt_init(entityOperatorNew(sizeof(ContinuePrompt)),
                                                   gameStateGetPlayerSide(&GAME_STATE))->entity),
        -1);
}

/* Starts a round: outside the demo, the arena's music from the start, faded in over 30 frames; then clears the
 * continue and quit flags, waits 4 frames, wipes in, shows READY? FIGHT! (with sound 0x3F) until it ends, adds the
 * DEMONSTRATION message in the demo, and lets the fighters and the clock go with the pause's join state cleared. */
void Fight_playIntro(Fight *this) {
    Entity *message;

    if (!GAME_STATE.demo) {
        seekMusicTrack(gameStateGetArena(&GAME_STATE), 0);
        soundControlResumeMusic(&SOUND_CONTROL);
        soundControlFadeOut(&SOUND_CONTROL, 0);
        soundControlFadeIn(&SOUND_CONTROL, 30);
    }
    this->flags &= ~FIGHT_FLAG_CONTINUE;
    this->flags &= ~FIGHT_FLAG_QUIT;
    Fight_waitFrames(this, 4);
    Fight_wipeIn(this);
    message = Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_READY_FIGHT, 0));
    soundControlPlaySound(&SOUND_CONTROL, 0x3F, 0);
    Fight_waitEntity(this, message, -1);
    if (GAME_STATE.demo) {
        Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_DEMONSTRATION, 0));
    }
    Fight_unfreezeFighters(this);
    Fight_releaseClock(this);
    joinStateReset(&PAUSE_JOIN);
}

/* Plays a round until it has a result (Fight_getRoundResult), a stage stops it (ROUND_STOPPED) or the pause menu
 * quits (ROUND_QUIT); the secret opponent's coming in ends it at once with the result GAME_STATE already holds.
 * The clock runs while no entity of the group at 0x5C plays, unless save.unk8 holds it outside a bonus game. Then
 * the fighters freeze, the entities finish, and the sides freeze; the result goes to GAME_STATE.roundResult. */
s32 Fight_playRound(Fight *this) {
    s32 holdClock = func_8001DECC(&GAME_STATE);
    s32 result;

    if (gameStateSetSecretOpponent(&GAME_STATE)) {
        Fight_waitFrames(this, 30);
        result = GAME_STATE.roundResult;
    } else {
        while ((result = Fight_getRoundResult(this)) == 0 && !((this->flags >> 3) & 1)) { /* FIGHT_FLAG_STOP_ROUND */
            if ((this->flags >> 9) & 1) { /* FIGHT_FLAG_QUIT */
                result = ROUND_QUIT;
                break;
            }
            if (!holdClock || (this->modeFlags & FIGHT_MODE_BONUS)) {
                if (!func_8006F104(this)) {
                    this->timeLeft--;
                }
            }
            Fight_waitFrames(this, 0);
        }
    }
    Fight_freezeFighters(this);
    func_8006F124(this, 1);
    while (func_8006F104(this)) {
        Fight_waitFrames(this, 0);
    }
    this->flags |= FIGHT_FLAG_ROUND_OVER;
    BattleSide_freeze(this->sides[0]);
    BattleSide_freeze(this->sides[1]);
    GAME_STATE.roundResult = result;
    return result;
}

s32 Fight_waitHandle(Fight *this, u32 handle, s32 port);

/* Counts timer down while above 0; whether it has just reached 0 (as BattleSide_tick's). */
static inline s32 countDown(s32 *timer) {
    s32 done = 0;

    if (*timer > 0) {
        done = --*timer == 0;
    }
    return done;
}

#if VERSION_JP
/* The Japanese version has other code here and stays in asm for now. */
INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071894);
#else
/* A round won by knock-out: slow motion while KO shows (the winner's port can skip it, ending the slow motion);
 * then, until the winner is idle, waits (the slow motion ends after 60 frames), waits 30 more, wipes out, puts the
 * loser back, focuses the camera on the winner, wipes in and plays the winner's music; the winner takes its win
 * pose, the fight waits up to 120 frames for the voice, then shows the win message and waits for it. The match
 * depends on the countdown inline and on a second counter for the voice's wait, set right before its loop. */
void func_80071894(Fight *this, s32 side) {
    Fighter *winner;
    Vec3 position;
    s32 slow;
    s32 wait;

    this->flags |= FIGHT_FLAG_SLOW;
    if (Fight_waitHandle(this,
                         entityGetHandle(Fight_addEntity(
                             this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_KO, 0))),
                         side)) {
        this->flags &= ~FIGHT_FLAG_SLOW;
        return;
    }
    winner = this->sides[side]->current;
    Fighter_getPosition(winner, &position);
    slow = 60;
    while (Fighter_getBodyState(winner)->state != FIGHTER_STATE_IDLE) {
        Fight_waitFrames(this, 0);
        if (countDown(&slow)) {
            this->flags &= ~FIGHT_FLAG_SLOW;
        }
    }
    Fight_waitFrames(this, 30);
    this->flags &= ~FIGHT_FLAG_SLOW;
    Fight_wipeOut(this);
    Fight_respawnFighter(this, side);
    this->cameraFocus = side + 1;
    Fight_waitFrames(this, 2);
    Fight_wipeIn(this);
    seekMusicTrack(gameStatePickWinnerTrack(&GAME_STATE), 1);
    soundControlResumeMusicFull(&SOUND_CONTROL);
    Fight_waitFrames(this, 5);
    Fighter_requestWinPose(winner);
    wait = 120;
    while (isXaPlaying() && --wait >= 0) {
        Fight_waitFrames(this, 0);
    }
    Fight_waitFrames(this, 5);
    Fight_waitEntity(this, Fight_showWinner(this, side, (this->flags >> 7) & 1 /* FIGHT_FLAG_LAST_ROUND */), -1);
}
#endif

/* A round won on time: shows TIME UP, sets the winner's win pose (func_80079268 on the loser) and waits for the
 * message (the winner's port can skip it); unless skipped, shows the win message (Fight_showWinner), waits for it and
 * 90 frames more. */
void Fight_playTimeUpWin(Fight *this, s32 side) {
    u32 handle;
    Fighter *winner;
    Fighter *loser;

    handle = entityGetHandle(
        Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_TIME_UP, 0)));
    winner = this->sides[side]->current;
    loser = this->sides[side ^ 1]->current;
    Fighter_requestWinPose(winner);
    func_80079268(loser);
    if (!Fight_waitHandle(this, handle, side)) {
        Fight_waitEntity(this, Fight_showWinner(this, side, (this->flags >> 7) & 1 /* FIGHT_FLAG_LAST_ROUND */), -1);
        Fight_waitFrames(this, 90);
    }
}


/* A double KO: plays sounds 0x8009 and 0x42, then shows the DOUBLE KO message and waits for it (Fight_waitEntity). */
void Fight_playDoubleKnockOut(Fight *this) {
    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    Fight_waitEntity(
        this, Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_DOUBLE_KO, 0)), -1);
}

/* A draw: as Fight_playDoubleKnockOut but with the DRAW GAME message, after func_80079268 on both sides' fighters. */
void Fight_playDraw(Fight *this) {
    Fighter *fighter0;
    Fighter *fighter1;

    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    /* The match depends on reading both fighters before the calls. */
    fighter0 = this->sides[0]->current;
    fighter1 = this->sides[1]->current;
    func_80079268(fighter0);
    func_80079268(fighter1);
    Fight_waitEntity(
        this, Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_DRAW_GAME, 0)), -1);
}

/* A challenger or the secret opponent came in: stops the music, plays sound 0x8007 and the NEW CHALLENGER message
 * as Fight_playDraw does, and ends the fight. */
void Fight_playInterruption(Fight *this) {
    Fighter *fighter0;
    Fighter *fighter1;

    stopMusic();
    soundControlPlaySound(&SOUND_CONTROL, 0x8007, 0);
    fighter0 = this->sides[0]->current;
    fighter1 = this->sides[1]->current;
    func_80079268(fighter0);
    func_80079268(fighter1);
    Fight_waitEntity(
        this,
        Fight_addEntity(this, BattleMessage_init(entityOperatorNew(0x59C), BATTLE_MESSAGE_NEW_CHALLENGER, 0)),
        -1);
    Fight_end(this);
}

/* Fight_waitHandle for the entity's handle: waits until it is gone or the port presses Start (0x800). */
s32 Fight_waitEntity(Fight *this, Entity *entity, s32 port) {
    return Fight_waitHandle(this, entityGetHandle(entity), port);
}

/* Waits, a frame at a time, until the entity of handle is gone: returns 0 then. With a port (>= 0), a press of
 * Start (0x800) on it skips: destroys the entity and returns 1. */
s32 Fight_waitHandle(Fight *this, u32 handle, s32 port) {
    while (getEntityByHandle(handle) != NULL) {
        Fight_waitFrames(this, 0);
        if (port >= 0 && (padManagerGetPressed(PAD_MANAGER_INSTANCE, port) & 0x800)) {
            destroyEntityByHandle(handle);
            return 1;
        }
    }
    return 0;
}

/* Wipes the screen in: the wipe from 0x20 down by 2 a frame, until it is under -4. */
void Fight_wipeIn(Fight *this) {
    this->wipe = 0x20;
    do {
        Fight_waitFrames(this, 0);
        this->wipe -= 2;
    } while (this->wipe >= -4);
}

/* Wipes the screen out: the wipe from 0 up by 2 a frame, until it is over 0x24, then fades the screen to level 0
 * at once. */
void Fight_wipeOut(Fight *this) {
    this->wipe = 0;
    do {
        Fight_waitFrames(this, 0);
        this->wipe += 2;
    } while (this->wipe < 0x25);
    fadeControlFadeTo(&FADE_CONTROL, 0, 0);
}

/* Ends the fight: wipes out, tells the main loop (FIGHT_FLAG_OVER) and only waits frames from then on: the thread
 * never returns. */
void Fight_end(Fight *this) {
    Fight_wipeOut(this);
    this->flags |= FIGHT_FLAG_OVER;
    for (;;) {
        Fight_waitFrames(this, 0);
    }
}

void Fight_createSides(Fight *this);

/* Sets up the next round: builds the sides again and starts the round (Fight_startRound), waits for the CD, then
 * plays the arena's music from where it was. */
void Fight_resetRound(Fight *this) {
    Fight_destroySides(this);
    Fight_createSides(this);
    this->vtable[1].func((u8 *)this + this->vtable[1].delta);
    syncCdfs(0);
    seekMusicTrack(gameStateGetArena(&GAME_STATE), 0);
    soundControlResumeMusic(&SOUND_CONTROL);
}

/* The stage's own part of a frame: nothing in the base class. */
void Fight_tickScenery(Fight *this) {
}

/* Sets FIGHT_FLAG_QUIT to quit. */
static inline void Fight_setQuit(Fight *this, s32 quit) {
    this->flags = (this->flags & ~FIGHT_FLAG_QUIT) | ((quit & 1) << 9);
}

#if VERSION_JP
/* While paused, the pausing side's pad: up and down move the cursor (continue / quit) with a click; cancel resumes
 * with the cursor back on continue; Start or confirm resume, quitting (FIGHT_FLAG_QUIT) when the cursor is on quit.
 * Either way the pause's join state is left; else it is updated. The US version asks again before quitting (below).
 * The match depends on the side read before the buttons are stored and on the quit flag set through Fight_setQuit
 * (a plain expression branches). */
void Fight_updatePauseMenu(Fight *this) {
    u32 pressed = padManagerGetPressed(PAD_MANAGER_INSTANCE, PAUSE_JOIN.side);
    s32 cursor;

    this->pauseSide = PAUSE_JOIN.side;
    this->pauseButtons = pressed & (PADstart | PAD_CONFIRM | PAD_CANCEL);
    cursor = this->pauseCursor;
    if (pressed & PAD_UP) {
        this->pauseCursor = 0;
    }
    if (pressed & PAD_DOWN) {
        this->pauseCursor = 1;
    }
    if (this->pauseCursor != cursor) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
    }
    if (pressed & PAD_CANCEL) {
        this->pauseCursor = 0;
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        joinStateSetLeft(&PAUSE_JOIN);
    } else if (pressed & (PADstart | PAD_CONFIRM)) {
        Fight_setQuit(this, this->pauseCursor != 0);
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        joinStateSetLeft(&PAUSE_JOIN);
    } else {
        joinStateUpdate(&PAUSE_JOIN);
    }
}
#else
/* While paused, the pausing side's pad: up and down move the cursor with a click. Cancel goes back a page, or
 * resumes from the first; Start or confirm resumes on continue, asks again on quit (pausePage 1, cursor on no),
 * and on that page quits (FIGHT_FLAG_QUIT) or resumes by the cursor. Leaving resets the menu and leaves the
 * pause's join state; else that state is updated. The match depends on the one leave block the cases jump to. */
void Fight_updatePauseMenu(Fight *this) {
    u32 pressed = padManagerGetPressed(PAD_MANAGER_INSTANCE, PAUSE_JOIN.side);
    s32 cursor;

    this->pauseSide = PAUSE_JOIN.side;
    this->pauseButtons = pressed & (PADstart | PAD_CONFIRM | PAD_CANCEL);
    cursor = this->pauseCursor;
    if (pressed & PAD_UP) {
        this->pauseCursor = 0;
    }
    if (pressed & PAD_DOWN) {
        this->pauseCursor = 1;
    }
    if (this->pauseCursor != cursor) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
    }
    if (pressed & PAD_CANCEL) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        switch (this->pausePage) {
        case 0:
            goto leave;
        case 1:
            this->pausePage--;
            return;
        }
    }
    if (pressed & (PADstart | PAD_CONFIRM)) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        switch (this->pausePage) {
        case 0:
            if (this->pauseCursor == 0) {
                goto leave;
            }
            this->pauseCursor = 0;
            this->pausePage = 1;
            return;
        case 1:
            Fight_setQuit(this, this->pauseCursor != 0);
            goto leave;
        }
    }
    joinStateUpdate(&PAUSE_JOIN);
    return;

leave:
    this->pauseCursor = 0;
    this->pausePage = 0;
    joinStateSetLeft(&PAUSE_JOIN);
}
#endif

INCLUDE_ASM("game/nonmatchings/battle_scene", Fight_updateCamera);

void Fight_drawContinueFloor(Fight *this);
void Fight_drawWipe(Fight *this);

/* Draws a frame of the fight: clears both cameras' tables; on the continue screen draws Fight_drawContinueFloor, the
 * fighters and the entities into camera2, else the world (slot 0x40, Fight_drawWorld); then links both tables into
 * the frame's and draws the wipe. */
void Fight_draw(Fight *this) {
    cameraClearOt(&this->camera);
    cameraClearOt(&this->camera2);
    if ((this->flags >> 8) & 1) { /* FIGHT_FLAG_CONTINUE */
        Fight_drawContinueFloor(this);
        sceneObjectDrawInView(&this->fighterGroup.object, &this->camera2);
        entityGroupDraw(&this->entities, (s32)&this->camera2);
    } else {
        this->vtable[8].func((u8 *)this + this->vtable[8].delta); /* Fight_drawWorld */
    }
    orderingTableLinkAfter(&this->camera.ot, FRAME_OT.firstDrawn + 2);
    orderingTableLinkAfter(&this->camera2.ot, FRAME_OT.firstDrawn + 3);
    Fight_drawWipe(this);
}

/* Draws the world (FIGHT_VTABLE 0x40): the background sprite when there is one; the fighters in camera2, over the
 * world, or in camera with it while unk318 bit 0 is set and the round is not over; the second group and the map;
 * the waiting and the fight's entities where the fighters went; the stage's entities. */
void Fight_drawWorld(void *arg) {
    Fight *this = arg;

    if (this->hasBackground) {
        u_long *ot = FRAME_OT.firstDrawn + 1;

        PRIM_BUFFER_FREE = spriteDraw(&this->background, PRIM_BUFFER_FREE, ot);
    }
    if (!(this->unk318 & 1) || Fight_isRoundOver(this)) {
        sceneObjectDrawInView(&this->fighterGroup.object, &this->camera2);
    } else {
        sceneObjectDrawInView(&this->fighterGroup.object, &this->camera);
    }
    sceneObjectDrawInView(&this->unkCC.object, &this->camera);
    sceneObjectDrawInView(&this->meshScene->group.object, &this->camera);
    if (!(this->unk318 & 1) || Fight_isRoundOver(this)) {
        entityGroupDraw(&this->unk5C, (s32)&this->camera2);
        entityGroupDraw(&this->entities, (s32)&this->camera2);
    } else {
        entityGroupDraw(&this->unk5C, (s32)&this->camera);
        entityGroupDraw(&this->entities, (s32)&this->camera);
    }
    entityGroupDraw(&this->stageEntities, (s32)&this->camera);
}

INCLUDE_RODATA("game/nonmatchings/battle_scene", VIGNETTE_RING);

/* A POLY_G4 as words, to set each corner's color and position a word at a time. */
typedef struct PolyG4Words {
    /* 0x00 */ u_long tag;
    /* 0x04 */ u32 rgbCode0; /* the first color, then the primitive's code */
    /* 0x08 */ u32 xy0;      /* y in the high half */
    /* 0x0C */ u32 rgb1;
    /* 0x10 */ u32 xy1;
    /* 0x14 */ u32 rgb2;
    /* 0x18 */ u32 xy2;
    /* 0x1C */ u32 rgb3;
    /* 0x20 */ u32 xy3;
} PolyG4Words;

/* One spoke of the vignette ring: a screen point on its outer edge and one on its inner edge, as xy words. */
typedef struct VignetteSpoke {
    /* 0x0 */ u32 outer;
    /* 0x4 */ u32 inner;
} VignetteSpoke;

#define VIGNETTE_QUADS 16
#define VIGNETTE_EDGE_COLOR 0x404040

/* The ring around the screen's center (160, 120): 16 sides, inner radius 101, outer radius 202, its first spoke
 * repeated at the end. */
extern VignetteSpoke VIGNETTE_RING[VIGNETTE_QUADS + 1];

/* Darkens the screen's edges: a ring of quads, black inside and grey outside, subtracted from the screen.
 * The match depends on the next spoke's address taken at the top of the loop (the game computes it into its own
 * register before the loads and copies it at the end; spoke++ at the end gives one addiu) and on the end pointer
 * set before the loop (else it is hoisted after the colors). Reading the corners through next puts the last load
 * on it (7 diffs). */
void drawVignette(u_long *ot) {
    VignetteSpoke *spoke;
    VignetteSpoke *next;
    VignetteSpoke *end;
    PolyG4Words *quad;

    spoke = VIGNETTE_RING;
    end = &VIGNETTE_RING[VIGNETTE_QUADS];
    do {
        next = spoke + 1;
        quad = (PolyG4Words *)allocPolyG4();
        quad->rgbCode0 = (((POLY_G4 *)quad)->code << 24) | (PRIM_SEMI_TRANS << 24) | VIGNETTE_EDGE_COLOR;
        quad->rgb1 = 0;
        quad->rgb2 = VIGNETTE_EDGE_COLOR;
        quad->rgb3 = 0;
        quad->xy0 = spoke[0].outer;
        quad->xy1 = spoke[0].inner;
        quad->xy2 = spoke[1].outer;
        quad->xy3 = spoke[1].inner;
        AddPrim(ot, quad);
        spoke = next;
    } while (spoke != end);
    AddPrim(ot, allocDrTpage(TPAGE_SUBTRACT));
}

/* The line buildLineF2 projects: its flags, its two ends and its color. */
typedef struct LineF2Shape {
    /* 0x0 */ u32 flags;
    /* 0x4 */ SVECTOR *from;
    /* 0x8 */ SVECTOR *to;
    /* 0xC */ u32 color;
} LineF2Shape;

/* The continue screen's floor (Fight_draw): counts its frame (continueFrames), then a grid of 81 by 81 lines
 * 0xC0 apart over -0x1E00..0x1E00 in x and z, color 0x2804000, the lines camera clips away skipped, and the
 * vignette over it. */
void Fight_drawContinueFloor(Fight *this) {
    MatrixWords m;
    LineF2Shape line;
    SVECTOR start;
    SVECTOR end;
    u8 *packet;
    u_long *ot;
    s32 x;
    s32 i;

    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = 0;
    m.m.t[1] = 0;
    m.m.t[2] = 0;
    this->continueFrames++;
    setModelMatrix(&m.m);
    packet = PRIM_BUFFER_FREE;
    ot = FRAME_OT.firstDrawn + 1;
    line.flags = 0;
    line.color = 0x2804000;
    line.from = &start;
    line.to = &end;
    for (i = 0; i <= 80; i++) {
        x = i * 0xC0 - 0x1E00;
        start.vx = x;
        end.vx = x;
        start.vy = 0;
        end.vy = 0;
        start.vz = -0x1E00;
        end.vz = 0x1E00;
        if (!cameraClipLine(&this->camera, &start, &end)) {
            packet = buildLineF2(&line, packet, ot, 0x10);
        }
        start.vx = -0x1E00;
        end.vx = 0x1E00;
        start.vy = 0;
        end.vy = 0;
        start.vz = x;
        end.vz = x;
        if (!cameraClipLine(&this->camera, &start, &end)) {
            packet = buildLineF2(&line, packet, ot, 0x10);
        }
    }
    PRIM_BUFFER_FREE = packet;
    drawVignette(FRAME_OT.firstDrawn + 3);
}

/* Draws rect less a pixel of width and height, subtracting color from the screen (semi-transparent, in a texture
 * page of abr 2). */
void drawDarkRect(u_long *ot, RECT *rect, u32 color) {
    /* to set the color and code, and copy the position, a word at a time */
    TileWords *tile = (TileWords *)allocTile();

    tile->rgbCode = ((((TILE *)tile)->code | PRIM_SEMI_TRANS) << 24) | color;
    tile->xy = ((RectWords *)rect)->xy;
    ((TILE *)tile)->w = rect->w - 1;
    ((TILE *)tile)->h = rect->h - 1;
    AddPrim(ot, tile);
    AddPrim(ot, allocDrTpage(getTPage(0, 2, 960, 256)));
}

/* Draws a frame around rect in color with its corners cut by 2 pixels: two LINE_F4, the top and right edges, then
 * the bottom and left ones. */
void drawRectFrame(u_long *ot, RECT *rect, u32 color) {
    LINE_F4 *topRight = allocLineF4();
    LINE_F4 *bottomLeft = allocLineF4();
    s32 x;
    s32 y;
    s32 right;
    s32 bottom;

    /* the color and the code, a word at a time */
    *(u32 *)&topRight->r0 = (topRight->code << 24) | color;
    *(u32 *)&bottomLeft->r0 = (bottomLeft->code << 24) | color;
    x = rect->x;
    y = rect->y;
    right = x + rect->w;
    bottom = y + rect->h;
    setXY4(topRight, x + 2, y, right, y, right, bottom - 2, right - 2, bottom);
    setXY4(bottomLeft, right - 2, bottom, x, bottom, x, y + 2, x + 2, y);
    AddPrim(ot, topRight);
    AddPrim(ot, bottomLeft);
}

/* Prints text of length characters (8 pixels each) centered on the 320-pixel screen, at y. */
void printCentered(Printer *printer, u8 *text, s32 length, s32 y) {
    printer->x = (0x140 - (length << 3)) >> 1;
    printer->y = y;
    printerPrint(printer, text);
}

/* the ordering table drawn over the fight: the debug lines go into it, the wipe and the pause menu into the slot
 * before it */
extern u_long *D_8005F844;

#if VERSION_JP
INCLUDE_RODATA("game/nonmatchings/battle_scene", STR_PAUSE_CONTINUE);
INCLUDE_RODATA("game/nonmatchings/battle_scene", STR_PAUSE_QUIT);

extern u8 STR_PAUSE_CONTINUE[]; /* "つづける" */
extern u8 STR_PAUSE_QUIT[];     /* "やめる" */
#else
/* the pause menu's two lines on each page: "Continue" and "Quit", then "ＮＯ" and "ＹＥＳ" */
extern u8 *PAUSE_MENU_TEXTS[2][2];
#endif

/* Draws the pause menu over the fight: its two lines (of the page in the US version), the one under the cursor
 * grey and the other dark, then a white frame and, one pixel in, a dark panel with a grey frame. The match depends
 * on setting the colors after the printer calls, not at their declarations. */
void Fight_drawPauseMenu(Fight *this) {
    TextPrinter printer;
    RECT frame;
    u_long *ot;
    u32 quitColor;
    u32 continueColor;
#if !VERSION_JP
    u8 *continueText;
    u8 *quitText;
#endif

    ot = D_8005F844 - 1;
    textPrinterInit(&printer, ot);
    textPrinterSetShadow(&printer, 1);
    textPrinterSetShadowColor(&printer, 0x800000);
    continueColor = 0x808080;
    quitColor = 0x404040;
#if !VERSION_JP
    continueText = PAUSE_MENU_TEXTS[this->pausePage][0];
    quitText = PAUSE_MENU_TEXTS[this->pausePage][1];
#endif
    if (this->pauseCursor != 0) {
        continueColor = 0x404040;
        quitColor = 0x808080;
    }
    TEXT_SET_COLOR(printer, continueColor);
#if VERSION_JP
    printCentered(&printer.printer, STR_PAUSE_CONTINUE, 8, 104);
#else
    printCentered(&printer.printer, continueText, strlen(continueText), 104);
#endif
    TEXT_SET_COLOR(printer, quitColor);
#if VERSION_JP
    printCentered(&printer.printer, STR_PAUSE_QUIT, 6, 120);
#else
    printCentered(&printer.printer, quitText, strlen(quitText), 120);
#endif
    frame.x = 112;
    frame.y = 100;
    frame.w = 96;
    frame.h = 40;
    drawRectFrame(ot, &frame, 0xFFFFFF);
    frame.x++;
    frame.y++;
    drawDarkRect(ot, &frame, 0x407870);
    drawRectFrame(ot, &frame, 0x808080);
    printerDestroy(&printer.printer, 2);
}

/* Draws the screen wipe: in each 32-pixel column, two black triangles that close on each other from its two edges
 * as the wipe grows (to 32, the column covered). */
void Fight_drawWipe(Fight *this) {
    s32 width = this->wipe;
    s32 x;
    POLY_F3 *left;
    POLY_F3 *right;
    u_long *ot;

    if (width <= 0) {
        return;
    }
    if (width > 32) {
        width = 32;
    }
    ot = D_8005F844 - 1;
    for (x = 0; x < SCREEN_WIDTH; x += 32) {
        left = allocPolyF3();
        right = allocPolyF3();
        setXY3(left, x, 0, x, SCREEN_HEIGHT - 1, x + width, SCREEN_HEIGHT);
        setXY3(right, x + 32 - width, 0, x + 32, 0, x + 32, SCREEN_HEIGHT);
        setRGB0(left, 0, 0, 0);
        setRGB0(right, 0, 0, 0);
        AddPrim(ot, left);
        AddPrim(ot, right);
    }
}

/* Sets the camera's view from the look-at and marks the world matrices of the groups and the mesh scene out of
 * date. */
void Fight_applyView(Fight *this) {
    MATRIX view;

    lookAtGetView(&this->lookAt, &view);
    cameraSetView(&this->camera, &view);
    groupInvalidate(&this->unkCC);
    groupInvalidate(&this->fighterGroup);
    groupInvalidate(&this->meshScene->group);
}

/* Whether the segment from from to to hits the map's collision lines (TerrainProbe_cast with a temporary
 * TerrainProbe); when it does, moves to to the hit (TerrainProbe_getHitPoint) and returns 1. */
s32 castMapRay(Vec3 *to, Vec3 *from) {
    TerrainProbe probe;

    TerrainProbe_init(&probe);
    if (TerrainProbe_cast(&probe, from, to)) {
        TerrainProbe_getHitPoint(&probe, to);
        return 1;
    }
    return 0;
}

/* Whether the segment from start to end hits the collision lines (TerrainProbe_cast with a temporary TerrainProbe,
 * which also moves the link it hits to the front of COLLISION_LINKS). */
s32 func_80072FE0(Vec3 *start, Vec3 *end) {
    TerrainProbe probe;

    TerrainProbe_init(&probe);
    return TerrainProbe_cast(&probe, start, end);
}



/* Sets a debug line's end to (x, y), z 0. */
static inline void setDebugPoint(SVECTOR *point, s16 x, s16 y) {
    point->vx = x;
    point->vy = y;
    point->vz = 0;
}

/* A debug drawing: a cross of random color at point's x and z under model, a line 0x80 up and one 0x80 wide.
 * The match depends on the ordering table and the packet read before Fight_random, the shape's address taken into
 * `shape` once its ends are set (kept in a saved register for both buildLineF2 calls), and the ends set through
 * setDebugPoint's s16 parameters (plain copies load with lh, the sums with lhu in 16 bits; s32 parameters give lh
 * and SImode sums). */
void func_80073024(Fight *this, MATRIX *model, SVECTOR *point) {
    LineF2Shape line;
    SVECTOR start;
    SVECTOR end;
    LineF2Shape *shape;
    u_long *ot;
    u8 *packet;

    line.flags = 0;
    line.from = &start;
    line.to = &end;
    ot = D_8005F844;
    packet = PRIM_BUFFER_FREE;
    shape = &line;
    line.color = Fight_random(this) & 0xFFFFFF;
    setModelMatrix(model);
    setDebugPoint(&start, point->vx, point->vz);
    setDebugPoint(&end, point->vx, point->vz - 0x80);
    packet = buildLineF2(shape, packet, ot, 0x10);
    setDebugPoint(&start, point->vx - 0x40, point->vz);
    setDebugPoint(&end, point->vx + 0x40, point->vz);
    PRIM_BUFFER_FREE = buildLineF2(shape, packet, ot, 0x10);
}

/* A debug drawing with no caller: marks from and to (func_80073024, under model) and joins them with a line of a
 * random color, in their x and z. Built as func_80073024 builds its lines (the same shape pointer, kept in s0
 * across Fight_random). */
void func_80073130(Fight *this, MATRIX *model, SVECTOR *from, SVECTOR *to) {
    LineF2Shape line;
    SVECTOR start;
    SVECTOR end;
    LineF2Shape *shape;
    u_long *ot;
    u8 *packet;

    func_80073024(this, model, from);
    func_80073024(this, model, to);
    ot = D_8005F844;
    packet = PRIM_BUFFER_FREE;
    line.flags = 0;
    line.from = &start;
    line.to = &end;
    shape = &line;
    line.color = Fight_random(this) & 0xFFFFFF;
    setDebugPoint(&start, from->vx, from->vz);
    setDebugPoint(&end, to->vx, to->vz);
    PRIM_BUFFER_FREE = buildLineF2(shape, packet, ot, 0x10);
}

/* Returns the fighter of the other side than fighter's. */
Fighter *Fight_getOpponent(Fight *this, Fighter *fighter) {
    if (Fighter_getSide(fighter) == 0) {
        return this->sides[1]->current;
    }
    return this->sides[0]->current;
}

/* Returns the input history of side. */
struct InputHistory *Fight_getInput(Fight *this, s32 side) {
    return this->inputs[side];
}

/* Returns the side object of side; no caller found. */
void *Fight_getSide(Fight *this, s32 side) {
    return this->sides[side];
}

/* Stops updating the sides. */
void Fight_freezeFighters(Fight *this) {
    this->flags |= FIGHT_FLAG_FREEZE;
}

/* Updates the sides again. */
void Fight_unfreezeFighters(Fight *this) {
    this->flags &= ~FIGHT_FLAG_FREEZE;
}

/* Stops the round's clock. */
void Fight_holdClock(Fight *this) {
    this->flags |= FIGHT_FLAG_HOLD_CLOCK;
}

/* Runs the round's clock again. */
void Fight_releaseClock(Fight *this) {
    this->flags &= ~FIGHT_FLAG_HOLD_CLOCK;
}

/* Gives the fighter the camera's lights (its light and color matrices). The match depends on copy32Bytes (see
 * engine/math/block_copy.h) and on the fighter parameter itself stepped to each matrix (a new pointer takes a
 * register below the copy's words). */
void Fight_lightFighter(Fight *this, Fighter *fighter) {
    MatrixWords *src = (MatrixWords *)cameraGetLights(&this->camera);

    fighter = (Fighter *)&fighter->lights;
    copy32Bytes(fighter, src);
    fighter = (Fighter *)((MatrixWords *)fighter + 1);
    src++;
    copy32Bytes(fighter, src);
}

/* Right after every round: nothing in the base class. */
void Fight_endRound(void) {
}

/* Before the next round's wipe-out, when another round follows: nothing in the base class (the stages stop
 * their looping sounds). */
void Fight_prepareNextRound(void) {
}

/* Returns the arena (no caller). */
s32 Fight_getArena(Fight *this) {
    return this->arena;
}

/* Returns the word at 0x258 (no caller). */
s32 func_800733A8(Fight *this) {
    return this->unk258;
}

/* Returns the fight Fight_init constructed last. */
Fight *getCurrentFight(void) {
    return CURRENT_FIGHT;
}

INCLUDE_RODATA("game/nonmatchings/battle_scene", FIGHT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064F40);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064F44);
INCLUDE_RODATA("game/nonmatchings/battle_scene", ARENA_START_POSITIONS);
INCLUDE_RODATA("game/nonmatchings/battle_scene", ARENA_RESPAWN_POSITIONS);
