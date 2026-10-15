/* The fighter factory, createFighter, which calls one constructor per fighter, and the base class of the
 * 24 fighters (virtual table FIGHTER_VTABLE). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/game/character.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "gte.h"

extern VtableEntry TRACKED_HIT_VTABLE[];
s32 Fighter_getPartMask(Fighter *this, s32 parts);
FighterState *Fighter_getAttackState(Fighter *this);

/* the constructors of the 24 fighters (src/battle/fighters/fighter_NN.c) */
Fighter *Agumon_init(Fighter *this);
struct Dukemon *Dukemon_init(struct Dukemon *this);
Gabumon *Gabumon_init(Gabumon *this);
Fighter *Guilmon_init(Fighter *this);
struct Holydramon *Holydramon_init(struct Holydramon *this);
Imperialdramon *ImperialdramonPaladinMode_init(Imperialdramon *this);
struct Impmon *Impmon_init(struct Impmon *this);
struct Beelzebumon *Beelzebumon_init(struct Beelzebumon *this);
struct MetalGarurumon *MetalGarurumon_init(struct MetalGarurumon *this);
Fighter *Omegamon_init(Fighter *this);
Patamon *Patamon_init(Patamon *this);
struct Renamon *Renamon_init(struct Renamon *this);
struct Sakuyamon *Sakuyamon_init(struct Sakuyamon *this);
struct SaintGargomon *SaintGargomon_init(struct SaintGargomon *this);
struct Seraphimon *Seraphimon_init(struct Seraphimon *this);
struct Stingmon *Stingmon_init(struct Stingmon *this);
Tailmon *Tailmon_init(Tailmon *this);
Terriermon *Terriermon_init(Terriermon *this);
Fighter *VMon_init(Fighter *this);
Wormmon *Wormmon_init(Wormmon *this);
Fighter *Gokumon_init(Fighter *this);
struct BlackWarGreymon *BlackWarGreymon_init(struct BlackWarGreymon *this);

/* The fighter factory: constructs the fighter of character (CHARACTER_*, 0 to 23) with g++'s new and its
 * constructor; NULL for any other character. */
Fighter *createFighter(s32 character) {
    void *fighter = NULL; /* each constructor returns its own fighter type, all of them on Fighter */

    switch (character) {
    case 0:
        fighter = Agumon_init(entityOperatorNew(0x4A4));
        break;
    case 1:
        fighter = Dukemon_init(entityOperatorNew(0x4A8));
        break;
    case 2:
        fighter = Gabumon_init(entityOperatorNew(0x4A8));
        break;
    case 3:
        fighter = Guilmon_init(entityOperatorNew(0x4A4));
        break;
    case 4:
        fighter = Holydramon_init(entityOperatorNew(0x4AC));
        break;
    case 5:
        fighter = Imperialdramon_init(entityOperatorNew(0x4F4));
        break;
    case 6:
        fighter = ImperialdramonPaladinMode_init(entityOperatorNew(0x4F4));
        break;
    case 7:
        fighter = Impmon_init(entityOperatorNew(0x4C0));
        break;
    case 8:
        fighter = Beelzebumon_init(entityOperatorNew(0x4B4));
        break;
    case 9:
        fighter = MetalGarurumon_init(entityOperatorNew(0x4D0));
        break;
    case 10:
        fighter = Omegamon_init(entityOperatorNew(0x4B0));
        break;
    case 11:
        fighter = Patamon_init(entityOperatorNew(0x4EC));
        break;
    case 12:
        fighter = Renamon_init(entityOperatorNew(0x4D4));
        break;
    case 13:
        fighter = Sakuyamon_init(entityOperatorNew(0x4C0));
        break;
    case 14:
        fighter = SaintGargomon_init(entityOperatorNew(0x4AC));
        break;
    case 15:
        fighter = Seraphimon_init(entityOperatorNew(0x4AC));
        break;
    case 16:
        fighter = Stingmon_init(entityOperatorNew(0x4A8));
        break;
    case 17:
        fighter = Tailmon_init(entityOperatorNew(0x4A8));
        break;
    case 18:
        fighter = Terriermon_init(entityOperatorNew(0x4AC));
        break;
    case 19:
        fighter = VMon_init(entityOperatorNew(0x4A4));
        break;
    case 20:
        fighter = Wormmon_init(entityOperatorNew(0x4A8));
        break;
    case 21:
        fighter = WarGreymon_init(entityOperatorNew(0x4C4));
        break;
    case 22:
        fighter = Gokumon_init(entityOperatorNew(0x4A4));
        break;
    case 23:
        fighter = BlackWarGreymon_init(entityOperatorNew(0x4C8));
        break;
    }
    return fighter;
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_init);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_setUp);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_prepare);

/* Adds points to the fighter's side's score (Fight_addScore). */
void Fighter_addScore(Fighter *this, s32 points) {
    Fight_addScore(this->fight, this->side, points);
}

/* Adds amount to the side's life (BattleSide_addLife on the side object). */
void Fighter_addLife(Fighter *this, s32 amount) {
    BattleSide_addLife(this->battleSide, amount);
}

/* Adds amount to the side's evolution gauge (BattleSide_addGauge on the side object). */
void Fighter_addGauge(Fighter *this, s32 amount) {
    BattleSide_addGauge(this->battleSide, amount);
}

/* Asks to evolve (Fighter_requestEvolution) once the side object says the gauge is full (BattleSide_canEvolve). */
void Fighter_evolveIfReady(Fighter *this) {
    if (BattleSide_canEvolve(this->battleSide)) {
        Fighter_requestEvolution(this);
    }
}

/* Ends the item's effect. */
void Fighter_clearItemEffect(Fighter *this) {
    this->itemEffect.kind = 0;
    this->itemEffect.timer = 0;
    this->itemEffect.power = 0;
    this->itemEffect.unkC = 0;
    this->itemEffect.unk10 = 0;
}

/* Starts an item's effect: its kind, for `timer` frames, with its power. */
void Fighter_startItemEffect(Fighter *this, s32 kind, s32 timer, s32 power) {
    this->itemEffect.kind = kind;
    this->itemEffect.timer = timer;
    this->itemEffect.power = power;
    this->itemEffect.unkC = 0;
    this->itemEffect.unk10 = 0;
}

/* Runs the item's effect (updateItemEffect) while there is one. */
void Fighter_tickItemEffect(Fighter *this) {
    if (this->itemEffect.kind != 0) {
        updateItemEffect(this);
    }
}

/* Clears the second timed effect, at 0x478. */
void func_80077508(Fighter *this) {
    this->unk478.kind = 0;
    this->unk478.timer = 0;
    this->unk478.power = 0;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Starts the second timed effect, at 0x478: its kind, for `timer` frames, with its power. */
void func_80077520(Fighter *this, s32 kind, s32 timer, s32 power) {
    this->unk478.kind = kind;
    this->unk478.timer = timer;
    this->unk478.power = power;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Runs the second timed effect (func_800C99B0) while there is one. */
void func_80077538(Fighter *this) {
    if (this->unk478.kind != 0) {
        func_800C99B0(this);
    }
}

/* The fighter's character (CHARACTER_*). */
s32 Fighter_getCharacter(Fighter *this) {
    return this->character;
}

/* The fighter's side, 0 or 1. */
s32 Fighter_getSide(Fighter *this) {
    return this->side;
}

/* The fight the fighter is in. */
Fight *Fighter_getScene(Fighter *this) {
    return this->fight;
}

/* The fighter's side object, which both forms of the side share. */
BattleSide *func_8007758C(Fighter *this) {
    return this->battleSide;
}

/* Whether the computer plays the fighter. */
s32 Fighter_isComputer(Fighter *this) {
    return this->flags & FIGHTER_FLAG_COMPUTER;
}

/* The fighter's model. */
Model *Fighter_getModel(Fighter *this) {
    return this->model;
}

/* Puts the fighter at a position in whole units (kept in 1/256ths; Fighter_getPosition reads it back).
 * The match depends on reading the three words before storing any, as vecCopy does. */
void Fighter_setPosition(Fighter *fighter, Vec3 *in) {
    s32 x = in->vx << 8;
    s32 y = in->vy << 8;
    s32 z = in->vz << 8;

    fighter->position.vx = x;
    fighter->position.vy = y;
    fighter->position.vz = z;
}

/* The fighter's position in whole units (kept in 1/256ths); returns out. */
Vec3 *Fighter_getPosition(Fighter *fighter, Vec3 *out) {
    out->vx = fighter->position.vx >> 8;
    out->vy = fighter->position.vy >> 8;
    out->vz = fighter->position.vz >> 8;
    return out;
}

/* The fighter's position before its last move, in whole units; returns out. */
Vec3 *Fighter_getLastPosition(Fighter *fighter, Vec3 *out) {
    out->vx = fighter->lastPosition.vx >> 8;
    out->vy = fighter->lastPosition.vy >> 8;
    out->vz = fighter->lastPosition.vz >> 8;
    return out;
}

/* Where the fighter's velocity takes it next step, in whole units; returns out. */
Vec3 *Fighter_getNextPosition(Fighter *fighter, Vec3 *out) {
    out->vx = (fighter->position.vx + fighter->velocity.vx) >> 8;
    out->vy = (fighter->position.vy + fighter->velocity.vy) >> 8;
    out->vz = (fighter->position.vz + fighter->velocity.vz) >> 8;
    return out;
}

/* The VRAM row of the fighter's textures: 0x100 for side 1. */
s32 Fighter_getVramY(Fighter *this) {
    return this->vramY;
}

/* The fighter's params (FIGHTER_PARAMS' row of its character). */
FighterParams *Fighter_getParams(Fighter *this) {
    return this->params;
}

extern s8 D_80067AD4[][10];

/* The character's row of D_80067AD4: which model part each attack bit is (Fighter_getPartMask reads it). */
s8 *Fighter_getPartMap(Fighter *this) {
    /* The match depends on the table taken into a local first (the game computes its address before the load). */
    s8 (*rows)[10] = D_80067AD4;

    return rows[this->character];
}

/* The fighter's input history. */
InputHistory *Fighter_getInput(Fighter *this) {
    return this->input;
}

/* Destroys a scene object and frees it, as `delete` does: nothing for NULL (scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Fighter's destructor: frees the body boxes and deletes the model, takes the collider out of its list (its
 * destructor inlined), then destroys the entity. */
void Fighter_destroy(Entity *entity, s32 flags) {
    Fighter *this = (Fighter *)entity; /* the fighters' destructors pass their Entity */
    ListNode *node;

    this->entity.vtable = &FIGHTER_VTABLE.entity;
    if (this->bodyBoxes != 0) {
        operatorVecDelete((void *)this->bodyBoxes); /* a Box[], typed s32 as its getter returns it */
    }
    sceneObjectDelete(&this->model->group.object);
    this->collider.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->collider.link), in the access order the game's compiler used */
    node = &this->collider.link;
    this->collider.link.next->prev = this->collider.link.prev;
    node->prev->next = this->collider.link.next;
    this->collider.link.next = node;
    this->collider.link.prev = node;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Whether the fighter's character can evolve (FIGHTER_PARAM_EVOLVED clear). */
s32 Fighter_canEvolve(Fighter *this) {
    return !(Fighter_getParams(this)->flags & FIGHTER_PARAM_EVOLVED);
}

/* Returns bit 1 of the fighter's flags. */
s32 func_800777BC(Fighter *this) {
    return (this->flags >> 1) & 1;
}

/* Adds the fighter's model to group. */
void Fighter_addModelTo(Fighter *this, Group *group) {
    /* The match depends on reading the model into a local first (the game loads it before moving group). */
    Model *model = this->model;

    groupAddChild(group, &model->group.object);
}

/* Takes the fighter's model out of the group it is a child of. */
void Fighter_removeModel(Fighter *this) {
    listRemove(&this->model->group.object.link);
}

/* Returns the flag bit func_80079AE4 sets, and clears it. */
s32 func_8007782C(Fighter *this) {
    u32 flags = this->flags;
    u32 shifted = flags >> FIGHTER_FLAG_80079AE4_BIT;

    /* The match depends on flags masked in place (set twice): a set-once value lets sched1 move the shift. */
    flags &= ~(1 << FIGHTER_FLAG_80079AE4_BIT);
    this->flags = flags;
    return shifted & 1;
}

/* Probes the map (TerrainProbe_cast) on the segment from 0x26 to 0x1E26 below the fighter's position (y grows
 * down); returns what TerrainProbe_cast returns. The USA release first checks bit 3 of the fighter's flags. */
#if VERSION_JP
s32 Fighter_probeFloorBelow(Fighter *this) {
    Vec3 from;
    Vec3 to;
    TerrainProbe probe;

    Fighter_getPosition(this, &from);
    from.vy += 0x26;
    Fighter_getPosition(this, &to);
    to.vy += 0x1E26;
    TerrainProbe_init(&probe);
    return TerrainProbe_cast(&probe, &from, &to);
}
#else
s32 Fighter_probeFloorBelow(Fighter *this) {
    Vec3 from;
    Vec3 to;
    TerrainProbe probe;

    if (!((this->flags >> 3) & 1)) {
        return 0;
    }
    Fighter_getPosition(this, &from);
    from.vy += 0x26;
    Fighter_getPosition(this, &to);
    to.vy += 0x1E26;
    TerrainProbe_init(&probe);
    return TerrainProbe_cast(&probe, &from, &to);
}
#endif

/* Whether the fighter stands on the ground (FIGHTER_FLAG_ON_GROUND). */
s32 Fighter_isOnGround(Fighter *this) {
    return (this->flags >> 2) & 1;
}

/* Marks air move `move` used and returns 1, or returns 0 when it was used since the fighter left the ground. */
s32 Fighter_tryUseAirMove(Fighter *this, s32 move) {
    s32 bit = 1 << move;

    if (this->airMovesUsed & bit) {
        return 0;
    }
    this->airMovesUsed |= bit;
    return 1;
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_tick);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_updateModel);

/* Out of an attack, shows face 1 (hurt) while the body flinches, is knocked down or out, or is dizzy, else
 * face 0. */
void Fighter_updateFace(Fighter *this) {
    if (Fighter_isAttackIdle(this)) {
        s32 state = Fighter_getBodyState(this)->state;

        Fighter_setFace(this, (state >= FIGHTER_STATE_FLINCH && state <= FIGHTER_STATE_KNOCKED_OUT)
                                  || state == FIGHTER_STATE_DIZZY);
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_applyPose);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_updatePhysics);

INCLUDE_ASM("game/nonmatchings/fighter", func_80077F80);

/* Whether the fighter stands on the ground (Fighter_isOnGround) and the map node its floor probe found last
 * moves with matrix (TerrainProbe_getMatrix): it stands on that moving part of the stage. */
s32 Fighter_isStandingOn(Fighter *this, MATRIX *matrix) {
    if (!Fighter_isOnGround(this)) {
        return 0;
    }
    return matrix == TerrainProbe_getMatrix(&this->floorProbe);
}

/* How far above the last position Fighter_checkGround's second segment starts: 0x80, 0x40 in the USA release. */
#if VERSION_JP
#define GROUND_CHECK_RISE 0x80
#else
#define GROUND_CHECK_RISE 0x40
#endif

/* Whether the floor probe finds the map (TerrainProbe_cast) on the segment from 0x80 above to 8 below the fighter's
 * position, or else on the one from above its last position to its position (y grows down). */
s32 Fighter_checkGround(Fighter *this) {
    TerrainProbe *probe = &this->floorProbe;
    Vec3 from;
    Vec3 to;

    Fighter_getPosition(this, &from);
    Fighter_getPosition(this, &to);
    from.vy -= 0x80;
    to.vy += 8;
    if (TerrainProbe_cast(probe, &from, &to)) {
        return 1;
    }
    Fighter_getLastPosition(this, &from);
    from.vy -= GROUND_CHECK_RISE;
    Fighter_getPosition(this, &to);
    return TerrainProbe_cast(probe, &from, &to) != 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078254);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_accelerateTo);

extern s32 ARENA_FRICTION;

/* The arena's friction on the fighter: -velocity.vx scaled by the mass, then by ARENA_FRICTION (both 16.16),
 * along x only; returns out. */
Vec3 *Fighter_getFriction(Fighter *this, Vec3 *out) {
    s32 scaled = ((s64)-this->velocity.vx * this->mass) >> 16;

    out->vy = 0;
    out->vz = 0;
    out->vx = ((s64)scaled * ARENA_FRICTION) >> 16;
    return out;
}

/* Whether there is floor `distance` ahead of the fighter: castMapRay on the vertical segment from 0x100 below to
 * 0x4C above its position (y grows down), moved along x by distance times its facing (Fighter_getFacing). */
s32 Fighter_hasFloorAhead(Fighter *this, s32 distance) {
    Vec3 from;
    Vec3 to;
    s32 delta;

    Fighter_getPosition(this, &from);
    delta = distance * Fighter_getFacing(this);
    to.vz = from.vz;
    to.vy = from.vy - 0x4C;
    from.vy += 0x100;
    from.vx += delta;
    to.vx = from.vx;
    return castMapRay(&from, &to);
}

/* Plays a voice: sound in the fighter's group (side, character) + 0x40; keeps its handle. */
s32 Fighter_playVoice(Fighter *this, s32 sound) {
    s32 handle = soundControlPlaySound(&SOUND_CONTROL, sound, (this->side << 7) + this->character + 0x40);

    this->voiceHandle = handle;
    return handle;
}

/* Plays sound in the fighter's group (side, character). */
s32 Fighter_playSound(Fighter *this, s32 sound) {
    /* The match depends on the u8 local: the game reads both fields as bytes (lbu). */
    u8 group = this->character + (this->side << 7);

    return soundControlPlaySound(&SOUND_CONTROL, sound, group);
}

/* Steps an angle by diff: all of it when it is within 3, else a quarter of it.
 * The match depends on reading the angle first and on a new result in each arm. */
static inline void stepAngle(s32 *angle, s32 diff) {
    s32 value = *angle;
    s32 result;

    if (diff < -3 || diff > 3) {
        result = value + diff / 4;
    } else {
        result = value + diff;
    }
    *angle = result;
}

/* Turns the fighter's rotation toward its target rotation, the short way round (12-bit angles). */
void Fighter_updateRotation(Fighter *this) {
    s32 dx = ((this->targetRotation.vx - this->rotation.vx - 0x800) & 0xFFF) - 0x800;
    s32 dy = ((this->targetRotation.vy - this->rotation.vy - 0x800) & 0xFFF) - 0x800;
    s32 dz = ((this->targetRotation.vz - this->rotation.vz - 0x800) & 0xFFF) - 0x800;

    stepAngle(&this->rotation.vx, dx);
    stepAngle(&this->rotation.vy, dy);
    stepAngle(&this->rotation.vz, dz);
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_updateTransform);

/* Turns the fighter to face left (facing -1); at once when `now` is set. */
void Fighter_faceLeft(Fighter *this, s32 now) {
    this->targetRotation.vy = -0x401;
    if (now) {
        this->rotation.vy = -0x401;
    }
}

/* Turns the fighter to face right (facing 1); at once when `now` is set. */
void Fighter_faceRight(Fighter *this, s32 now) {
    this->targetRotation.vy = 0x401;
    if (now) {
        this->rotation.vy = 0x401;
    }
}

/* Turns the fighter to face the camera's way (0x800). */
void func_80078834(Fighter *this) {
    this->targetRotation.vy = 0x800;
}

/* Turns the fighter to face the camera (0). */
void func_80078840(Fighter *this) {
    this->targetRotation.vy = 0;
}

/* Mirrors the input's left and right while the fighter faces left (InputHistory_setMirrored). */
void Fighter_mirrorInput(Fighter *this, void *input) {
    InputHistory_setMirrored(input, this->targetRotation.vy == -0x401);
}

/* The entity a collider's hit came from when it is a fighter (ENTITY_KIND_FIGHTER), else NULL. */
Entity *getHitFighter(CollisionSphere *collider) {
    Entity *entity = collider->entity;

    if (entityGetKind(entity) != ENTITY_KIND_FIGHTER) {
        return NULL;
    }
    return entity;
}

void MoveReward_set(MoveReward *this, u16 score, u16 gauge);

/* Sets what move `move` (MOVE_*) pays when it hits: `score` points and the params' evolution gauge for it. */
void Fighter_setMoveReward(Fighter *this, s32 move, s32 score) {
    MoveReward_set(&this->rewards[move], score, Fighter_getParams(this)->gaugeGains[move]);
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_handleInput);

/* Keeps the fighter's input and hands it to the handleInput slot (Fighter_tick calls it with what
 * Fight_getInput returns). */
void Fighter_setInput(Fighter *this, struct InputHistory *input) {
    FighterVtable *vtable;

    this->input = input;
    vtable = GET_FIGHTER_VTABLE(this);
    vtable->handleInput.func((u8 *)this + vtable->handleInput.delta, input);
}

/* Starts a vibration of the fighter's pad (its side's port), unless the computer plays it, a demo plays or
 * the vibration option is off. */
void Fighter_vibratePad(Fighter *this, s32 mode, u8 power0, u8 power1, s32 duration, s32 priority) {
    PadManager *padManager;

    if (this->flags & FIGHTER_FLAG_COMPUTER) {
        return;
    }
    if (GAME_STATE.demo != 0) {
        return;
    }
    if (GAME_STATE.save.vibration == 0) {
        return;
    }
    /* The match depends on reading the pad manager in its own statement: the game loads it before the
     * stack arguments. */
    padManager = PAD_MANAGER_INSTANCE;
    padManagerStartVibration(padManager, this->side, mode, power0, power1, duration, priority);
}

/* The fighter's momentum: its velocity scaled by its mass (16.16); returns out. */
Vec3 *func_80078CE0(Fighter *this, Vec3 *out) {
    s32 scale = this->mass;

    out->vx = ((s64)this->velocity.vx * scale) >> 16;
    out->vy = ((s64)this->velocity.vy * scale) >> 16;
    out->vz = ((s64)this->velocity.vz * scale) >> 16;
    return out;
}

/* Whether hits miss the fighter: knocked down or out, getting up, throwing, or in its invulnerable frames. */
s32 Fighter_isInvulnerable(Fighter *this) {
    s32 state = Fighter_getBodyState(this)->state;
    s32 attack = Fighter_getAttackState(this)->state;
    s32 result = 0;

    if (state == FIGHTER_STATE_GET_UP || state == FIGHTER_STATE_KNOCKED_DOWN || state == FIGHTER_STATE_KNOCKED_OUT
        || attack == FIGHTER_STATE_THROW || this->invulnerableFrames > 0) {
        result = 1;
    }
    return result;
}

/* Whether the fighter guards, standing or crouching. */
s32 Fighter_isGuarding(Fighter *this) {
    s32 state = Fighter_getBodyState(this)->state;

    return state >= FIGHTER_STATE_GUARD && state <= FIGHTER_STATE_CROUCH_GUARD;
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_blocks);

/* While an item's effect of kind 1 or 2 runs, scales the damage of hit by its power (in 1/256ths), keeping
 * the hit's flags. */
u32 Fighter_scaleDamage(Fighter *this, u32 hit) {
    if (this->itemEffect.kind == 1 || this->itemEffect.kind == 2) {
        /* The match depends on one variable for the count's steps (separate expressions take other
         * registers). */
        s32 damage = hit & HIT_DAMAGE_MASK;

        damage = (damage * this->itemEffect.power) >> 8;
        hit = damage | (hit & ~HIT_DAMAGE_MASK);
    }
    return hit;
}

/* percentages (-5, 0 or 5) by two indexes of 0 to 3 */
extern s8 D_8006520C[4][4];

INCLUDE_RODATA("game/nonmatchings/fighter", D_8006520C);

/* value with the percentage D_8006520C[row][column] added. */
s32 func_80078EB0(s32 row, s32 column, s32 value) {
    return value * (D_8006520C[row][column] + 100) / 100;
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_takeHit);

/* Asks for the win pose once the round ends. */
void Fighter_requestWinPose(Fighter *this) {
    this->pendingPose = FIGHTER_STATE_WIN_POSE;
}

/* Asks for the end pose once the round ends. */
void func_80079268(Fighter *this) {
    this->pendingPose = FIGHTER_STATE_END_POSE;
}

/* Puts the body in the held state (FighterStateHeld_init), with arg. */
void *Fighter_hold(Fighter *this, s32 arg) {
    return FighterStateHeld_init(this->bodyState, arg);
}

void func_80079298(void) {
}

/* The fighter's body state. */
FighterState *Fighter_getBodyState(Fighter *this) {
    return this->bodyState;
}

/* The fighter's attack state. */
FighterState *Fighter_getAttackState(Fighter *this) {
    return this->attackState;
}

/* The FIGHTER_STATE_* of the fighter's body (Fighter_getBodyState). */
s32 Fighter_getBodyStateId(Fighter *this) {
    return Fighter_getBodyState(this)->state;
}

/* The FIGHTER_STATE_* of the fighter's attack (Fighter_getAttackState). */
s32 Fighter_getAttackStateId(Fighter *this) {
    return Fighter_getAttackState(this)->state;
}

/* Clears what the hits added into the fighter's collider and returns it (func_800708F4 hands it to
 * shapeListCollide). */
CollisionSphere *Fighter_resetCollider(Fighter *this) {
    this->collider.hitDamage = 0;
    this->collider.hitKind = 0;
    this->collider.hitKnockback.vx = 0;
    this->collider.hitKnockback.vy = 0;
    this->collider.hitKnockback.vz = 0;
    this->collider.hitKnockbackPad = 0;
    return &this->collider;
}

/* Stops the model's clip (Fighter_playActionClip with -1). */
void Fighter_stopActionClip(Fighter *this) {
    Fighter_playActionClip(this, -1, 0, 0, -1);
}

/* While the side has life left: ends both timed effects and asks the side to evolve (func_8007F6FC, which sets
 * its 0x30). */
void Fighter_requestEvolution(Fighter *this) {
    if (this->battleSide->life > 0) {
        Fighter_clearItemEffect(this);
        func_80077508(this);
        func_8007F6FC(this->battleSide);
    }
}

void func_80079420(Fighter *this);

/* Starts the evolution scene: moves the fighter to the fight's group that holds the fight while it is not
 * empty (func_8006F0E4), makes the scene's entity (func_80079420) and points the camera at its side
 * (cameraFocus side + 1); returns whether that entity exists. */
s32 Fighter_beginEvolutionScene(Fighter *this) {
    listRemove(&this->entity.link);
    func_8006F0E4(this->fight, &this->entity);
    func_80079420(this);
    this->fight->cameraFocus = this->side + 1;
    return getEntityByHandle(this->unk248) != NULL;
}

/* Constructs, in place, the entity of class CUTIN_VTABLE for side (game_0g). */
Entity *CutIn_init(void *place, s32 side);

/* For characters 0, 2, 3, 11, 12 and 17 to 20: makes the CutIn_init entity of its side, keeps its handle
 * at 0x248 and plays the side's fighter sound. */
void func_80079420(Fighter *this) {
    Entity *entity;

    switch (this->character) {
    case 0:
    case 2:
    case 3:
    case 11:
    case 12:
    case 17:
    case 18:
    case 19:
    case 20:
        entity = CutIn_init(entityOperatorNew(0x40), this->side);
        this->unk248 = entityGetHandle(entity);
        entityAddSibling(&this->entity, entity);
        soundControlPlaySound(&SOUND_CONTROL, gameStateGetFighterSound(&GAME_STATE, this->side), 0);
        break;
    }
}

/* Ends the evolution scene: destroys the entity of 0x248, moves the fighter back to the fight's entities
 * (Fight_addEntity) and gives the camera back to both sides (cameraFocus 0). */
void Fighter_endEvolutionScene(Fighter *this) {
    destroyEntityByHandle(this->unk248);
    this->unk248 = 0;
    listRemove(&this->entity.link);
    Fight_addEntity(this->fight, &this->entity);
    this->fight->cameraFocus = 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_takeOverFrom);

/* Starts clip (FIGHTER_CLIP_*) with the play count of the character's .loop file, keeping clip and blendFrames,
 * when Fighter_isActionClipDone allows it; a negative clip stops the model. */
void Fighter_setClip(Fighter *this, s32 clip, s32 blendFrames, s32 frame) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->clip = clip;
    this->clipBlendFrames = blendFrames;
    if (Fighter_isActionClipDone(this)) {
        modelStartClip(model, clip, blendFrames, charaGetClipPlays(this->chara, clip), frame);
        modelSetLoopFrame(model, charaGetLoopFrame(this->chara, clip));
    }
}

/* Starts clip and runs the model's animation twice, then stops the model. */
void func_800797B8(Fighter *this, s32 clip) {
    Model *model = this->model;
    s32 i;

    /* The match depends on counting down to 0 (an upward count is reversed into a bgez loop). */
    for (i = 2; i != 0; i--) {
        modelStartClip(model, clip, 0, 0, 0);
        modelUpdateAnimation(model);
    }
    modelStartClip(model, -1, 0, 0, 0);
}

/* Plays clip over the movement clip (a negative one stops the model); plays < 0 takes the count from the
 * character's .loop file. The match depends on the ternary: assigning plays loads it at the entry. */
void Fighter_playActionClip(Fighter *this, s32 clip, s32 blendFrames, s32 frame, s32 plays) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->actionClip = clip;
    modelStartClip(model, clip, blendFrames, plays < 0 ? charaGetClipPlays(this->chara, clip) : plays, frame);
    modelSetLoopFrame(model, charaGetLoopFrame(this->chara, clip));
}

/* Whether the model has no clip of that number, or the character's .loop file plays it 0x1000 times or
 * more. */
s32 Fighter_lacksClip(Fighter *this, s32 clip) {
    s32 plays;

    if (animatorIsClipEmpty(&this->model->animator, clip)) {
        return 1;
    }
    /* The match depends on the s32 local: comparing the u16 itself gives an unsigned sltiu. */
    plays = charaGetClipPlays(this->chara, clip);
    return plays >= 0x1000;
}

/* Whether the model is stopped; once it is, the action clip is forgotten (-1). */
s32 Fighter_isModelStopped(Fighter *this) {
    s32 stopped = modelIsStopped(this->model);

    if (stopped && this->actionClip >= 0) {
        this->actionClip = -1;
    }
    return stopped;
}

/* Whether the model is done with the movement clip: stopped, no clip, or playing another. */
s32 Fighter_isClipDone(Fighter *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->clip < 0) {
        return 1;
    }
    return modelGetClip(model) != this->clip;
}

/* Whether the model is done with the action clip: stopped, no clip, or playing another. */
s32 Fighter_isActionClipDone(Fighter *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->actionClip < 0) {
        return 1;
    }
    return modelGetClip(model) != this->actionClip;
}

/* Freezes the fighter: sets a flag, adds 1 to its life, stops it (velocity and force), and resets its body state
 * (FighterStateGround_init with 1) and its attack state (FighterAttackIdle_init). */
void Fighter_freeze(Fighter *this) {
    this->flags |= 1 << FIGHTER_FLAG_FROZEN_BIT;
    Fighter_addLife(this, 1);
    this->force.vx = 0;
    this->force.vy = 0;
    this->force.vz = 0;
    this->forcePad = 0;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocityPad = 0;
    FighterStateGround_init(this->bodyState, 1);
    FighterAttackIdle_init(this->attackState);
}

/* Sets a flag of the fighter (bit 6, bit 7 in the USA release). */
void func_80079AE4(Fighter *this) {
    this->flags |= 1 << FIGHTER_FLAG_80079AE4_BIT;
}

/* Sets a flag of the fighter (bit 5, bit 6 in the USA release). */
void func_80079AF8(Fighter *this) {
    this->flags |= 1 << FIGHTER_FLAG_80079AF8_BIT;
}

/* Sets 0x334 to arg. */
void func_80079B0C(Fighter *this, s32 arg) {
    this->unk334 = arg;
}

/* Sets 0x338 to arg. */
void func_80079B14(Fighter *this, s32 arg) {
    this->unk338 = arg;
}

/* Sets 0x33C to arg. */
void func_80079B1C(Fighter *this, s32 arg) {
    this->unk33C = arg;
}

/* Gives the fighter its faces. */
void Fighter_setFaceTable(Fighter *this, FaceEntry *table) {
    this->faceTable = table;
}

/* Shows face `face` of the face table, unless it is the one already drawn (kept complemented once drawn). */
void Fighter_setFace(Fighter *this, s32 face) {
    s32 *current = &this->face;

    if (~*current != face) {
        *current = face;
    }
}

/* A step of the fighter's motion: keeps the position as the last one, adds force / mass to the velocity
 * (func_80079C1C), clears the force and moves the position by what func_80079C1C returns. */
void Fighter_move(Fighter *this) {
    Vec3 step;
    s32 x;
    s32 y;
    s32 z;

    this->lastPosition = this->position;
    func_80079C1C(&step, &this->velocity, &this->force, this->mass);
    this->force.vx = 0;
    this->force.vy = 0;
    this->force.vz = 0;
    this->forcePad = 0;
    /* The match depends on the position read into locals before the step is added (all loads first). */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += step.vx;
    y += step.vy;
    z += step.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
}

/* Adds force / mass to the velocity (func_80079C1C), then clears the force. */
void Fighter_applyImpulse(Fighter *this) {
    Vec3 mid;

    func_80079C1C(&mid, &this->velocity, &this->force, this->mass);
    this->force.vx = 0;
    this->force.vy = 0;
    this->force.vz = 0;
    this->forcePad = 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079C1C);
extern s16 D_80065270[]; /* distances added by Fighter_getOpponentInReach, by its index */

INCLUDE_RODATA("game/nonmatchings/fighter", D_80065270);

/* NULL unless the other character (Fight_getOpponent) passes Fighter_isThrowable; else func_80079E50 with AXIS_Z, the
 * square of both characters' func_80079DD8 plus D_80065270[index], and 0x155. */
Fighter *Fighter_getOpponentInReach(Fighter *this, s32 index) {
    Fighter *other = Fight_getOpponent(this->fight, this);
    s32 distance;

    if (!Fighter_isThrowable(other)) {
        return 0;
    }
    distance = func_80079DD8(this) + func_80079DD8(other) + D_80065270[index];
    return func_80079E50(this, &AXIS_Z, distance * distance, 0x155);
}

/* Whether the fighter can be thrown: bit 3 of its flags, and not Fighter_isInvulnerable. */
s32 Fighter_isThrowable(Fighter *this) {
    if (Fighter_isInvulnerable(this) != 0) {
        return 0;
    }
    return (this->flags >> 3) & 1;
}

/* The fighter's radius: three quarters of its params' height. */
s32 func_80079DD8(Fighter *this) {
    s32 size = Fighter_getParams(this)->height;

    return size - size / 4;
}

/* The way the fighter faces: 1 (right) when its target rotation is 0x401, else -1. */
s32 Fighter_getFacing(Fighter *this) {
    s32 side = -1;

    if (this->targetRotation.vy == 0x401) {
        side = 1;
    }
    return side;
}

/* FighterState_getClip on the fighter's body state. */
s32 func_80079E2C(Fighter *this) {
    return FighterState_getClip(this->bodyState);
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079E50);

/* The model part of the highest of attack bits `parts`, or -1 when there is none. */
s32 Fighter_getPart(Fighter *this, s32 parts) {
    s8 *map = Fighter_getPartMap(this);
    s32 zeros;

    if (parts == 0) {
        return -1;
    }
    gte_leadingZeros(parts, zeros);
    return map[31 - zeros];
}

/* The model part bits of attack bits `parts`: one bit per part, through the part map. */
s32 Fighter_getPartMask(Fighter *this, s32 parts) {
    s8 *map = Fighter_getPartMap(this);
    s32 mask = 0;
    s32 zeros;
    s32 bit;

    while (parts != 0) {
        gte_leadingZeros(parts, zeros);
        bit = 31 - zeros;
        mask |= 1 << map[bit];
        parts ^= 1 << bit;
    }
    return mask;
}

/* Adds the model parts of attack bits `parts` (Fighter_getPartMask) to the parts that hit. */
void Fighter_addAttackParts(Fighter *this, s32 parts) {
    this->attackParts |= Fighter_getPartMask(this, parts);
}

/* Makes the model parts of attack bits `parts` (Fighter_getPartMask) the parts that hit. */
void Fighter_setAttackParts(Fighter *this, s32 parts) {
    this->attackParts = Fighter_getPartMask(this, parts);
}

/* The hits of the fighter's combo (Fighter_countComboHit counts them). */
s32 Fighter_getComboHits(Fighter *this) {
    return this->comboHits;
}

/* Counts a hit of the fighter's combo. */
void Fighter_countComboHit(Fighter *this) {
    this->comboHits++;
}

/* With on, sets 0x494 to 5 unless it is above 0 already; else clears it. */
void func_8007A12C(Fighter *this, s32 on) {
    if (on) {
        if (this->unk494 <= 0) {
            this->unk494 = 5;
        }
    } else {
        this->unk494 = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_8007A158);

/* Tests castMapRay toward point (raised by dy) from both sides, a third more than width away: -1 for the left
 * one only, 1 for the right one only, else 0. */
s32 func_8007A3EC(Vec3 *point, s32 dy, s32 width) {
    Vec3 left;
    Vec3 center;
    Vec3 right;
    s32 reach = width + width / 3;
    s32 side = 0;

    center.vx = point->vx;
    center.vz = point->vz;
    center.vy = point->vy + dy;
    left.vx = center.vx - reach;
    left.vy = center.vy;
    left.vz = center.vz;
    right.vx = center.vx + reach;
    right.vy = center.vy;
    right.vz = center.vz;
    if (castMapRay(&left, &center)) {
        side = -1;
    }
    if (castMapRay(&right, &center)) {
        side++;
    }
    return side;
}

#if VERSION_US
/* Sets 0x48C to 8 (the JP build stores it in FighterStateGround_drop itself). */
void func_8007A718(Fighter *this) {
    this->unk48C = 8;
}
#endif

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_getNodePosition);

/* Writes the point halfway between the model's nodes node0 and node1 (each moved by its offset,
 * Fighter_getNodePosition) to out and returns it. */
VECTOR *Fighter_getNodesMidpoint(Fighter *this, VECTOR *out, s32 node0, VECTOR *offset0, s32 node1, VECTOR *offset1) {
    VECTOR first;

    Fighter_getNodePosition(this, &first, node0, offset0);
    Fighter_getNodePosition(this, out, node1, offset1);
    out->vx = (out->vx + first.vx) / 2;
    out->vy = (out->vy + first.vy) / 2;
    out->vz = (out->vz + first.vz) / 2;
    return out;
}

/* Writes the top of the fighter (its position raised by its params' height) to out and returns it. */
Vec3 *func_8007A650(Fighter *this, Vec3 *out) {
    Fighter_getPosition(this, out);
    out->vy -= Fighter_getParams(this)->height;
    return out;
}

/* Writes the opponent's position, in whole units, to out and returns it. */
Vec3 *Fighter_getOpponentPosition(Fighter *this, Vec3 *out) {
    return Fighter_getPosition(Fighter_getOpponent(this), out);
}

/* As Fighter_getOpponentPosition, through func_8007A650. */
Vec3 *func_8007A6CC(Fighter *this, Vec3 *out) {
    return func_8007A650(Fighter_getOpponent(this), out);
}

/* With a count above 0 (at most 8), spawns a dust cloud (ParticleSpray_init, kind 6, color 0xFFFFFF) at the
 * fighter's position, moving 14 * dir the way it faces and 6 up, and hands it to Fighter_addEntity. */
void Fighter_spawnDust(Fighter *this, s32 dir, s32 arg2, s32 count) {
    if (count > 0) {
        Vec3 position;
        Vec3 velocity;
        s32 speed;
        Entity *child;

        if (count > 8) {
            count = 8;
        }
        /* The match depends on speed: scaling dir itself reorders the parameter copies. */
        speed = dir * Fighter_getFacing(this);
        Fighter_getPosition(this, &position);
        velocity.vx = speed * 14;
        velocity.vy = -6;
        velocity.vz = 0;
        child = allocEntityMemory(0x48);
        if (child != NULL) {
            ParticleSpray_init(entityOperatorNewAt(0x48, child), 6, &position, &velocity, arg2, count, 0xFFFFFF);
        }
        Fighter_addEntity(this, child);
    }
}

Vec3 *func_8007A920(Fighter *this, Vec3 *out);

/* Spawns a hit spark (HitSpark_init, color 0x808080) of size between the two fighters (func_8007A920), and
 * hands it to Fighter_addEntity. */
void Fighter_spawnHitSpark(Fighter *this, s32 size) {
    Vec3 position;
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        HitSpark_init(entityOperatorNewAt(0x48, child), func_8007A920(this, &position), &ZERO_VECTOR, size, 0x808080);
    }
    Fighter_addEntity(this, child);
}

/* Spawns a guard flash (vtable D_8006D4D4 on Particle_initById with frame 8, no velocity, the size given, color
 * 0xA0A0A0) between the two fighters (func_8007A920), and hands it to Fighter_addEntity. */
void Fighter_spawnGuardFlash(Fighter *this, s32 size) {
    Vec3 position;
    Entity *entity;
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        entity = entityOperatorNewAt(0x58, child);
        Particle_initById((Particle *)entity, 8, (VECTOR *)func_8007A920(this, &position), &ZERO_VECTOR,
                      size, 0xA0A0A0, 0);
        entity->vtable = &D_8006D4D4;
    }
    Fighter_addEntity(this, child);
}

/* Writes the point a quarter of the way from the opponent's top (func_8007A6CC) toward this fighter's
 * (func_8007A650), in x and y, to out and returns it: where their hit lands. */
Vec3 *func_8007A920(Fighter *this, Vec3 *out) {
    Vec3 own;

    func_8007A650(this, &own);
    func_8007A6CC(this, out);
    out->vx += (own.vx - out->vx) / 4;
    out->vy += (own.vy - out->vy) / 4;
    return out;
}

/* Spawns a guard flash (size 0x60) when the opponent guards, else a hit spark (size 0x100). */
void Fighter_spawnContactEffect(Fighter *this) {
    if (Fighter_isGuarding(Fighter_getOpponent(this))) {
        Fighter_spawnGuardFlash(this, 0x60);
    } else {
        Fighter_spawnHitSpark(this, 0x100);
    }
}

/* A move of a face (FaceEntry.moves, count of them): a rectangle of the frame buffer and where Fighter_draw
 * copies it. */
typedef struct FaceMove {
    /* 0x0 */ DVECTOR *position;
    /* 0x4 */ RECT *rect;
} FaceMove;

/* Draws the fighter's face once it changes (then marks it drawn by complementing it): a frame buffer copy
 * (DR_MOVE) for each of its moves, 0x100 lower in both rectangle and position for side 1's VRAM half. */
void Fighter_draw(Fighter *this, Camera *camera) {
    s32 *face = &this->face;
    FaceEntry *table = this->faceTable;
    FaceEntry *entry;
    FaceMove *item;
    FaceMove *end;
    s32 index;
    s16 dy; /* The match depends on its type: a frame buffer coordinate, as RECT's (s32 swaps the add) */
    u_long *ot;

    if (table == NULL) {
        return;
    }
    index = *face;
    if (index < 0) {
        return;
    }
    entry = &table[index];
    *face = ~index;
    item = entry->moves;
    end = item + entry->count;
    dy = (this->side != 0) << 8;
    ot = camera->ot.firstDrawn;
    while (item != end) {
        RECT rect = *item->rect;
        DVECTOR *position;

        rect.y += dy;
        position = item->position;
        item++;
        AddPrim(ot, allocDrMove(&rect, position->vx, position->vy + dy));
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_updateBoxes);

/* The boxes of the fighter's model parts (Box[], Fighter_updateBoxes). */
s32 Fighter_getBodyBoxes(Fighter *this) {
    return this->bodyBoxes;
}

/* How many body boxes there are. */
s32 Fighter_getBodyBoxCount(Fighter *this) {
    return this->bodyBoxCount;
}

/* The boxes of the model parts that hit now (Box[]). */
s32 Fighter_getAttackBoxes(Fighter *this) {
    return this->attackBoxes;
}

/* How many attack boxes there are. */
s32 Fighter_getAttackBoxCount(Fighter *this) {
    return this->attackBoxCount;
}

/* Adds a collision shape to its side's list of the fight (Fight_addSide0Shape / Fight_addSide1Shape). node is
 * passed on untouched in $a1. */
void func_8007ACF4(Fighter *this, ListNode *node) {
    switch (this->side) {
    case 0:
        Fight_addSide0Shape(this->fight, node);
        break;
    case 1:
        Fight_addSide1Shape(this->fight, node);
        break;
    }
}

/* Adds entity, if there is one, to the fight's entities.
 * It returns nothing: Wormmon_guard puts a constant in $v0 right after calling it,
 * which a call with a value would keep it from. */
void Fighter_addEntity(Fighter *this, Entity *entity) {
    Fight_addEntity(this->fight, entity);
}

void Fighter_attachToPart(Fighter *this, MeshPart *part, u32 handle);

/* Attaches the fighter to part `index` of holder's model (Fighter_attachToPart): holder holds it. */
void Fighter_attachTo(Fighter *this, Fighter *holder, s32 index) {
    MeshPart *part = holder->model->parts[index];

    Fighter_attachToPart(this, part, entityGetHandle(&holder->entity));
}

/* Unless the model is already attached, attaches its transform to part's (to none when part is NULL) and
 * keeps the holder's handle and entity. */
void Fighter_attachToPart(Fighter *this, MeshPart *part, u32 handle) {
    if (this->holderHandle == 0) {
        transformAttach(&this->model->group.object.transform, part != NULL ? &part->mesh.object.transform : NULL);
        this->holderHandle = handle;
        this->holder = getEntityByHandle(handle);
    }
}

void func_80077F80(Fighter *this);

/* Undoes Fighter_attachToPart: detaches the model and takes its local translation (x, y, in 1/256ths, z 0) as the
 * position, then calls func_80077F80. */
void Fighter_detach(Fighter *this) {
    Model *model;
    s32 y;

    if (this->holderHandle != 0) {
        this->holderHandle = 0;
        this->holder = NULL;
        transformDetach(&this->model->group.object.transform);
        model = this->model;
        this->position.vx = model->group.object.transform.local.m.t[0] << 8;
        /* The match depends on y: the game reads it before it clears vz. */
        y = model->group.object.transform.local.m.t[1];
        this->position.vz = 0;
        this->position.vy = y << 8;
        func_80077F80(this);
    }
}

/* Whether the fighter flinches, on the ground or in the air. */
s32 func_8007AEA8(Fighter *this) {
    s32 state = Fighter_getBodyState(this)->state;

    return state >= FIGHTER_STATE_FLINCH && state <= FIGHTER_STATE_AIR_FLINCH;
}

/* Whether the fighter attacks with nothing (attack state FIGHTER_STATE_IDLE, as FighterState_isAttackIdle). */
s32 Fighter_isAttackIdle(Fighter *this) {
    return this->attackState->state == FIGHTER_STATE_IDLE;
}

/* The fighter's opponent (Fight_getOpponent). */
Fighter *Fighter_getOpponent(Fighter *this) {
    return Fight_getOpponent(this->fight, this);
}

/* The getUp slot: on its first frame, plays the get-up clip. */
void Fighter_getUp(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_GET_UP, 2, 0);
    }
}

/* The walkLeft slot: on its first frame, plays the walk clip, faces left and raises dust; then speeds toward
 * the params' walk speed, leftward. */
void Fighter_walkLeft(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_WALK, 4, 0);
        Fighter_faceLeft(this, 0);
        Fighter_spawnDust(this, -1, 0x60, Fighter_canEvolve(this) ? 2 : 3);
    }
    Fighter_accelerateTo(this, -Fighter_getParams(this)->walkSpeed);
}

/* The walkRight slot: on its first frame, plays the walk clip, faces right and raises dust; then speeds toward
 * the params' walk speed, rightward. */
void Fighter_walkRight(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_WALK, 4, 0);
        Fighter_faceRight(this, 0);
        Fighter_spawnDust(this, -1, 0x60, Fighter_canEvolve(this) ? 2 : 3);
    }
    Fighter_accelerateTo(this, Fighter_getParams(this)->walkSpeed);
}

/* The flag of the fighter Fighter_dashLeft and Fighter_dashRight set: 0x80, 0x100 in the USA release. */
#if VERSION_JP
#define DASH_FLAG 0x80
#else
#define DASH_FLAG 0x100
#endif

/* Which side of the fighter func_8007A3EC finds blocked (-1 left, 1 right, 0 none); the JP release
 * does not test it, so there it is always 0. */
static inline s32 dashBlockedSide(Fighter *this) {
#if VERSION_JP
    return 0;
#else
    Vec3 position;

    Fighter_getPosition(this, &position);
    return func_8007A3EC(&position, -0x33, Fighter_getParams(this)->height);
#endif
}

/* The dashLeft slot: on its first frame, plays the dash clip, pushes left by the params' dash speed (times the
 * mass, 16.16) unless the left side is blocked, faces left and raises dust; every frame sets DASH_FLAG. */
void Fighter_dashLeft(Fighter *this, s32 frame) {
    if (frame == 0) {
        s32 delta;

        Fighter_setClip(this, FIGHTER_CLIP_DASH, 1, 0);
        if (dashBlockedSide(this) >= 0) {
            delta = ((s64)Fighter_getParams(this)->dashSpeed * this->mass) >> 16;
            this->force.vx -= delta;
        }
        Fighter_faceLeft(this, 0);
        Fighter_spawnDust(this, -1, 0x60, Fighter_canEvolve(this) ? 3 : 4);
    }
    this->flags |= DASH_FLAG;
}

/* The dashRight slot: on its first frame, plays the dash clip, pushes right by the params' dash speed (times
 * the mass, 16.16) unless the right side is blocked, faces right and raises dust; every frame sets DASH_FLAG. */
void Fighter_dashRight(Fighter *this, s32 frame) {
    if (frame == 0) {
        s32 delta;

        Fighter_setClip(this, FIGHTER_CLIP_DASH, 1, 0);
        if (dashBlockedSide(this) <= 0) {
            /* The match depends on delta: as one expression, the sum takes unk3B4's register. */
            delta = ((s64)Fighter_getParams(this)->dashSpeed * this->mass) >> 16;
            this->force.vx += delta;
        }
        Fighter_faceRight(this, 0);
        Fighter_spawnDust(this, -1, 0x60, Fighter_canEvolve(this) ? 3 : 4);
    }
    this->flags |= DASH_FLAG;
}

/* The jump slot: pushes up by the params' jump speed (times the mass, 16.16) from a standstill, plays the jump
 * clip, raises dust on both sides when on the ground, and keeps the x speed within 1.5 walk speeds. */
void Fighter_jump(Fighter *this) {
    s32 value = Fighter_getParams(this)->jumpSpeed;
    s32 scale = this->mass;

    this->force.vy = ((s64)-value * scale) >> 16;
    this->velocity.vy = 0;
    Fighter_setClip(this, FIGHTER_CLIP_JUMP, 1, 0);
    if (Fighter_isOnGround(this)) {
        s32 count = Fighter_canEvolve(this) ? 2 : 3;

        Fighter_spawnDust(this, 1, 0x60, count);
        Fighter_spawnDust(this, -1, 0x60, count);
    }
    value = Fighter_getParams(this)->walkSpeed;
    value += Fighter_getParams(this)->walkSpeed / 2;
    if (value < this->velocity.vx) {
        this->velocity.vx = value;
    } else if (this->velocity.vx < -value) {
        this->velocity.vx = -value;
    }
}

/* The crouch slot: plays the crouch clip. */
void Fighter_crouch(Fighter *this) {
    Fighter_setClip(this, FIGHTER_CLIP_CROUCH, 2, 0);
}

/* The guard slot: on its first frame, plays the guard clip. */
void Fighter_guard(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_GUARD, 3, 0);
    }
}

/* The crouchGuard slot: on its first frame, plays the crouching guard clip. */
void Fighter_crouchGuard(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_CROUCH_GUARD, 3, 0);
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_dizzy);

/* The flinch slot: on its first frame, plays the flinch clip. */
void Fighter_flinch(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_setClip(this, FIGHTER_CLIP_FLINCH, 1, 0);
    }
}

/* The airFlinch slot: plays the air flinch clip. */
void Fighter_airFlinch(Fighter *this) {
    Fighter_setClip(this, FIGHTER_CLIP_AIR_FLINCH, 1, 0);
}

INCLUDE_RODATA("game/nonmatchings/fighter", FIGHTER_ATTACK_VOICES);

INCLUDE_ASM("game/nonmatchings/fighter", Fighter_runAttack);

/* The attack1 slot: plays the attack 1 clip on its first frame, then runs the params' attack 1. */
void Fighter_attack1(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_ATTACK1, 0, 0, -1);
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[0], 1);
}

/* The attack2 slot: on its first frame, plays the attack 2 clip and stops; then runs the params' attack 2. */
void Fighter_attack2(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_ATTACK2, 0, 0, -1);
        Fighter_accelerateTo(this, 0);
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[1], 1);
}

/* The attack3 slot: on its first frame, plays the attack 3 clip and halves the x speed; then runs the params'
 * attack 3. */
void Fighter_attack3(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_ATTACK3, 0, 0, -1);
        this->velocity.vx -= this->velocity.vx / 2;
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[2], 2);
}

/* The upAttack slot: on its first frame, plays the up attack clip and stops; then runs the params' up
 * attack. */
void Fighter_upAttack(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_UP_ATTACK, 0, 0, -1);
        Fighter_accelerateTo(this, 0);
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[4], 1);
}

/* The dashAttack slot: on its first frame, stops the movement clip, plays the dash attack clip and stops along
 * x; then runs the params' dash attack. */
void Fighter_dashAttack(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_setClip(this, -1, 0, 0);
        Fighter_playActionClip(this, FIGHTER_CLIP_DASH_ATTACK, 2, 0, -1);
        this->velocity.vx = 0;
        this->force.vx = 0;
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[3], 2);
}

/* The crouchAttack slot: plays the crouch attack clip on its first frame, then runs the params' crouch
 * attack. */
void Fighter_crouchAttack(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_CROUCH_ATTACK, 0, 0, -1);
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[5], 1);
}

/* The throw slot: plays the throw clip on its first frame. */
void Fighter_throw(Fighter *this, s32 frame) {
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_THROW, 0, 0, -1);
    }
}

/* The special1 slot: plays the special 1 clip on its first frame (the fighters override it). */
void Fighter_special1(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
    }
}

/* The special2 slot: plays the special 2 clip on its first frame (the fighters override it). */
void Fighter_special2(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL2, 0, 0, -1);
    }
}

/* The special3 slot: plays the special 3 clip on its first frame (the fighters override it). */
void Fighter_special3(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL3, 0, 0, -1);
    }
}

/* The airSpecial3 slot: plays the air special 3 clip on its first frame (the fighters override it). */
void Fighter_airSpecial3(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL3, 2, 0, -1);
    }
}

/* The airAttack slot: plays the air attack clip on its first frame, then runs the params' air attack. */
void Fighter_airAttack(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_ATTACK, 2, 0, -1);
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[8], 1);
}

/* The airUpAttack slot: on its first frame, plays the air up attack clip and stops falling; then runs the
 * params' air up attack. */
void Fighter_airUpAttack(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_UP_ATTACK, 2, 0, -1);
        this->velocity.vy = 0;
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[6], 1);
}

/* The airSideAttack slot: on its first frame, plays the params' air side clip and stops; then runs the params'
 * air side attack. */
void Fighter_airSideAttack(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, Fighter_getParams(this)->airSideClip, 2, 0, -1);
        this->force.vx = 0;
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->velocity.vz = 0;
        this->velocityPad = 0;
    }
    Fighter_runAttack(this, frame, &Fighter_getParams(this)->attacks[7], 1);
}

/* The airSpecial1 slot: plays the air special 1 clip on its first frame (the fighters override it). */
void Fighter_airSpecial1(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 2, 0, -1);
    }
}

/* The airSpecial2 slot: plays the air special 2 clip on its first frame (the fighters override it). */
void Fighter_airSpecial2(Fighter *this, s32 frame) {
    if (frame == 0) {
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL2, 2, 0, -1);
    }
}

/* The 0x20 bytes that func_800708F4 copies from the object at 0x20 of boxesFindOverlap's result and passes to
 * the hitOpponent slot (Fighter_hitOpponent): where the boxes met, at 0x14. */
typedef struct Unk800708F4 {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Vec3 position;
} Unk800708F4;
s32 Fighter_blocks(Fighter *this, u32 hit);

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes: Particle_initById with frame 8, no velocity,
 * size 0x60, color 0xA0A0A0) at hit's position: NULL when the allocation fails. */
static inline Entity *func_8007BC50_newFlash(Unk800708F4 *hit) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        Particle *flash = entityOperatorNewAt(0x58, child);

        Particle_initById(flash, 8, (VECTOR *)&hit->position, &ZERO_VECTOR, 0x60, 0xA0A0A0, 0);
        flash->entity.vtable = &D_8006D4D4;
    }
    return child;
}

/* g++'s new of a hit spark (HitSpark_init, 0x48 bytes) at hit's position with velocity: NULL when the
 * allocation fails. */
static inline Entity *func_8007BC50_newC32E0(Unk800708F4 *hit, Vec3 *velocity) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        HitSpark_init(entityOperatorNewAt(0x48, child), &hit->position, (VECTOR *)velocity, 0xC0, 0x808080);
    }
    return child;
}

/* The hitOpponent slot (the fight calls it when this fighter's attack box meets other): lands the pending hit
 * of the attack on other (Fighter_takeHit, with its knockback and kind), spawns at hit's position a guard flash
 * when other blocks it (Fighter_blocks), else a hit spark, and clears the pending hit. */
void Fighter_hitOpponent(Fighter *this, Fighter *other, Unk800708F4 *hit) {
    if (this->hitDamage != 0) {
        Vec3 *velocity = &this->hitKnockback;

        Fighter_takeHit(other, this->hitDamage, velocity, this->hitKind);
        /* The match depends on the two news as inlines (a ternary of their results). */
        Fighter_addEntity(this, Fighter_blocks(other, this->hitDamage) ? func_8007BC50_newFlash(hit)
                                                              : func_8007BC50_newC32E0(hit, velocity));
        this->hitKnockback.vx = 0;
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        this->hitKnockbackPad = 0;
        this->hitDamage = 0;
        this->hitKind = 0;
    }
}

/* Constructs a hit of owner's: its damage, its kind, then its knockback.
 * The match depends on reading the vector's three words before storing any, as vecCopy does. */
Hit *Hit_init(Hit *this, struct Fighter *owner, s32 damage, Vec3 *knockback, s32 kind) {
    s32 x, y, z;

    this->vtable = HIT_VTABLE;
    this->owner = owner;
    this->damage = damage;
    this->kind = kind;
    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
    return this;
}

/* Sets the hit's damage and kind. */
void Hit_set(Hit *this, s32 damage, s32 kind) {
    this->damage = damage;
    this->kind = kind;
}

/* Sets the hit's knockback, reading all three components before writing any. */
void Hit_setKnockback(Hit *this, VECTOR *knockback) {
    s32 x, y, z;

    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}

/* Hit's first method (its part touched sum's): adds its damage (saturating at HIT_DAMAGE_MASK), its kind and
 * its knockback into the target's collider sum, whose flags it replaces with its own. */
void Hit_apply(Hit *this, CollisionSphere *sum) {
    u32 flags = this->damage & ~HIT_DAMAGE_MASK;
    /* The match depends on starting count with this object's part, then adding
     * sum's: count then lives in $a3 */
    s32 count = this->damage & HIT_DAMAGE_MASK;
    s32 x, y, z;

    count += sum->hitDamage & HIT_DAMAGE_MASK;
    if (count > HIT_DAMAGE_MASK) {
        count = HIT_DAMAGE_MASK;
    }
    sum->hitDamage = flags | count;
    sum->hitKind += this->kind;
    /* The match depends on vecAddInPlace's shape: the three words read before any is written */
    x = sum->hitKnockback.vx;
    y = sum->hitKnockback.vy;
    z = sum->hitKnockback.vz;
    x += this->knockback.vx;
    y += this->knockback.vy;
    z += this->knockback.vz;
    sum->hitKnockback.vx = x;
    sum->hitKnockback.vy = y;
    sum->hitKnockback.vz = z;
}

/* TrackedHit's first method: Hit_apply with sum, then sets bit 0 of the flags when sum's entity is a fighter
 * and bit 1 when it is a fighter's object (entityGetKind). */
void TrackedHit_apply(TrackedHit *this, CollisionSphere *sum) {
    Entity *target;

    Hit_apply(&this->base, sum);
    target = sum->entity;
    if (target != NULL) {
        if (entityGetKind(target) == ENTITY_KIND_FIGHTER) {
            this->flags |= 1;
        }
        if (entityGetKind(target) == ENTITY_KIND_FIGHTER_OBJECT) {
            this->flags |= 2;
        }
    }
}

/* Whether the hit struck a fighter. */
s32 TrackedHit_hitFighter(TrackedHit *this) {
    return this->flags & 1;
}

/* Whether the hit struck a fighter's object; no caller found. */
s32 func_8007BF0C(TrackedHit *this) {
    return (this->flags >> 1) & 1;
}

/* Constructs a TrackedHit on Hit_init's (the same arguments) with nothing struck yet. */
TrackedHit *TrackedHit_init(TrackedHit *this, Fighter *owner, s32 damage, Vec3 *knockback, s32 kind) {
    Hit_init(&this->base, owner, damage, knockback, kind);
    this->base.vtable = TRACKED_HIT_VTABLE;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

/* Constructs a move's pay with nothing to pay. */
MoveReward *MoveReward_init(MoveReward *this) {
    this->score = 0;
    this->gauge = 0;
    return this;
}

/* Sets the score and the evolution gauge the move pays. */
void MoveReward_set(MoveReward *this, u16 score, u16 gauge) {
    this->score = score;
    this->gauge = gauge;
}

/* Multiplies the score by scale and grows the gauge by (scale - 1) percent of itself (the combo bonus). */
MoveReward *MoveReward_scale(MoveReward *this, s32 scale) {
    this->score *= scale;
    this->gauge += this->gauge * (scale - 1) / 100;
    return this;
}

void MoveReward_giveGauge(MoveReward *this, Fighter *fighter);

/* Pays the score to fighter (Fighter_addScore) and the gauge with MoveReward_giveGauge, then clears the score. */
void MoveReward_give(MoveReward *this, Fighter *fighter) {
    Fighter_addScore(fighter, this->score);
    MoveReward_giveGauge(this, fighter);
    this->score = 0;
}

/* Pays the gauge to fighter (Fighter_addGauge) when it can evolve (Fighter_canEvolve or func_800777BC), then
 * clears it. */
void MoveReward_giveGauge(MoveReward *this, Fighter *fighter) {
    if (Fighter_canEvolve(fighter) || func_800777BC(fighter)) {
        Fighter_addGauge(fighter, this->gauge);
    }
    this->gauge = 0;
}

INCLUDE_RODATA("game/nonmatchings/fighter", CONTROLLER_VTABLE);

/* Constructs the controller of side (its side object). */
Controller *Controller_init(Controller *this, BattleSide *side) {
    this->vtable = CONTROLLER_VTABLE;
    this->side = side;
    return this;
}

/* Controller's destructor. */
void Controller_destroy(Controller *this, s32 flags) {
    this->vtable = CONTROLLER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The buttons the controller presses this frame: its virtual method at 8 (pure in Controller) for the side's
 * current fighter. */
s32 Controller_getButtons(Controller *this) {
    VtableEntryS32 *entry = (VtableEntryS32 *)&this->vtable[1];

    return entry->func((u8 *)this + entry->delta, this->side->current);
}

void Fighter_endMove(void) {
}

void Fighter_intro(void) {
}

/* The destructor of the class of virtual table HIT_VTABLE: CollisionOwner's, inlined. */
void Hit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The destructor of the class of virtual table TRACKED_HIT_VTABLE: CollisionOwner's, inlined. */
void TrackedHit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighter", TRACKED_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighter", HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighter", FIGHTER_VTABLE);
