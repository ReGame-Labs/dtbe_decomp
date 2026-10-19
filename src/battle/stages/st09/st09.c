#include "common.h"
#include <abs.h>
#include "engine/task/entity.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "engine/sound/sound_control.h"

/* The stage's computer player (StageJewelHunt_createController), derived from Controller. */
typedef struct JewelHuntController {
    /* 0x00 */ Controller base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
} JewelHuntController;

/* the constructor st09_createThrowers calls, and what that constructor calls last */
BonusThrower *BonusThrower_init(BonusThrower *this, Fight *owner, s32 side);
void BonusThrower_place(BonusThrower *this);
void BonusThrower_throw(BonusThrower *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable BONUS_LANDING_MARK_VTABLE;
extern EntityVtable BONUS_THROWER_VTABLE;
extern EntityVtable ST09_THROW_VTABLE;
extern EntityVtable D_800D39C4;

extern VtableEntry JEWEL_HUNT_CONTROLLER_VTABLE[];

/* Constructs the stage's computer player for side (StageJewelHunt_createController): Controller's, its own fields 0. */
JewelHuntController *JewelHuntController_init(JewelHuntController *this, BattleSide *side) {
    Controller_init(&this->base, side);
    this->base.vtable = JEWEL_HUNT_CONTROLLER_VTABLE;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    this->unk14 = 0;
    this->unk18 = 0;
    this->unk1C = 0;
    return this;
}

/* Bonus game 2's stage (StageJewelHunt_init, virtual table STAGE_JEWEL_HUNT_VTABLE), derived from BonusFight: each
 * side holds its button to charge a throw at the mesh objects that appear; it owns a camera of its own. */
typedef struct StageJewelHunt {
    /* 0x0000 */ BonusFight base;
    /* 0x0F70 */ Camera cameras[3];               /* one per group (StageJewelHunt_init) */
    /* 0x1024 */ MeshSceneLink collisionLink;     /* lines of the arena itself, in COLLISION_LINKS (StageJewelHunt_init) */
    /* 0x103C */ s16 frames; /* the scenery's frames (StageJewelHunt_tickScenery adds 1) */
    /* 0x103E */ u8 unk103E[0x1040 - 0x103E];
    /* 0x1040 */ MeshScene *scene; /* Fight_getMeshScene's (StageJewelHunt_startRound) */
    /* 0x1044 */ VECTOR throwPositions[2][30]; /* by side and slot: its St09Throw's position (St09Throw_init) */
    /* 0x1404 */ u16 nextThrowSlot[2]; /* by side: the next of 30 slots BonusThrower_throw gives an St09Throw */
    /* 0x1408 */ u16 unk1408[2][30]; /* by side and slot: set to 1 by func_800D6110 */
    /* 0x1480 */ Group groups[3]; /* each drawn in the view of its own camera (StageJewelHunt_drawWorld) */
    /* 0x15A0 */ s16 unk15A0; /* the func_800D5BA0 entities func_800D49A0 has made, up to 16 */
    /* 0x15A2 */ u16 holdFrames[2]; /* by side: frames button 0x80 has been held, up to BONUS_CHARGE_FRAMES */
    /* 0x15A6 */ u16 heldCharge[2]; /* by side: how long it was held, when let go (st09_updateCharge) */
    /* 0x15AA */ u8 jewelCounts[10][4]; /* the jewels in each cell: column (x + 0x480) >> 8, row (y + 0x480) >> 8 */
    /* 0x15D2 */ s16 unk15D2; /* cleared by StageJewelHunt_startRound */
    /* 0x15D4 */ s16 unk15D4;
    /* 0x15D6 */ u8 unk15D6[0x15D8 - 0x15D6];
} StageJewelHunt; /* size 0x15D8 */

/* the stage's own steps, which its slots call */
void st09_clearFrames(StageJewelHunt *this);
void st09_resetCharge(StageJewelHunt *this);
void st09_createThrowers(Fight *this);
void func_800D476C(StageJewelHunt *this);
void st09_keepFightersInside(StageJewelHunt *this);
void st09_updateCharge(StageJewelHunt *this);
void func_800D49A0(StageJewelHunt *this);
void st09_endOnTimeUp(StageJewelHunt *this);

INCLUDE_ASM("st09/nonmatchings/st09", JewelHuntController_getButtons);

extern VtableEntry STAGE_JEWEL_HUNT_VTABLE[];

/* the cameras' lights: the same directions and colours for the three, a back colour each */
extern VECTOR ST09_LIGHT_DIRECTION0;
extern VECTOR ST09_LIGHT_DIRECTION1;
extern VECTOR ST09_LIGHT_DIRECTION2;
extern VECTOR ST09_LIGHT_COLOR0;
extern VECTOR ST09_LIGHT_COLOR1;
extern VECTOR ST09_LIGHT_COLOR2;
extern VECTOR ST09_BACK_COLOR0;
extern VECTOR ST09_BACK_COLOR1;
extern VECTOR ST09_BACK_COLOR2;

typedef struct St09LineEnds {
    /* 0x0 */ DVECTOR from;
    /* 0x4 */ DVECTOR to;
} St09LineEnds;

extern St09LineEnds ST09_TERRAIN_LINE_ENDS[2];
extern TerrainLine ST09_TERRAIN_LINES[2];

/* Sets a group's camera up like the fight's: its field of view (kept an int until cameraSetFov narrows it), far
 * 0x3C00, the stage's lights and a back colour. */
static inline void setUpCamera(Camera *camera, s32 fov, VECTOR *backColor) {
    cameraReset(camera);
    cameraSetFov(camera, fov);
    cameraSetFar(camera, 0x3C00);
    cameraSetLightDirection0(camera, &ST09_LIGHT_DIRECTION0);
    cameraSetLightDirection1(camera, &ST09_LIGHT_DIRECTION1);
    cameraSetLightDirection2(camera, &ST09_LIGHT_DIRECTION2);
    cameraSetLightColor0(camera, &ST09_LIGHT_COLOR0);
    cameraSetLightColor1(camera, &ST09_LIGHT_COLOR1);
    cameraSetLightColor2(camera, &ST09_LIGHT_COLOR2);
    cameraSetBackColor(camera, backColor);
}

/* Constructs the stage: BonusFight, with virtual table STAGE_JEWEL_HUNT_VTABLE, three cameras and their groups, the
 * nodes, and two collision lines. */
StageJewelHunt *StageJewelHunt_init(StageJewelHunt *this) {
    Group *group;
    u32 count;
    s32 fov;
    s32 i;

    BonusFight_init(&this->base);
    this->base.base.vtable = STAGE_JEWEL_HUNT_VTABLE;
    cameraInit(&this->cameras[0], 1);
    cameraInit(&this->cameras[1], 1);
    cameraInit(&this->cameras[2], 1);
    this->collisionLink.link.next = &this->collisionLink.link;
    this->collisionLink.link.prev = &this->collisionLink.link;
    group = this->groups;
    count = 3;
    while (count--) {
        groupInit(group);
        group++;
    }
    fov = cameraGetFov(Fight_getCamera(&this->base.base));
    setUpCamera(&this->cameras[0], fov, &ST09_BACK_COLOR0);
    setUpCamera(&this->cameras[1], fov, &ST09_BACK_COLOR1);
    setUpCamera(&this->cameras[2], fov, &ST09_BACK_COLOR2);
    this->base.nodes[0] = 1;
    this->base.nodes[1] = 2;
    this->base.nodes[2] = 6;
    this->base.nodes[3] = 3;
    this->base.nodes[4] = 4;
    this->base.nodes[5] = 0;
    this->base.nodes[6] = 7;
    this->base.nodes[7] = 8;
    for (i = 0; i < 2; i++) {
        initTerrainLine(&ST09_TERRAIN_LINES[i], &ST09_TERRAIN_LINE_ENDS[i].from, &ST09_TERRAIN_LINE_ENDS[i].to);
    }
    /* MeshSceneLink.lines is typed by the engine's own lines; the stage's are TerrainLines */
    addCollisionLink(&this->collisionLink, (s32 *)ST09_TERRAIN_LINES, 2);
    return this;
}

/* The stage's override of Fight_createController: a JewelHuntController for the side. */
void *StageJewelHunt_createController(void *this, s32 unk, BattleSide *battleSide) {
    return JewelHuntController_init(operatorNew(0x20), battleSide);
}

/* The stage's destructor: the groups last first (through their virtual destructors, as g++ destroys an array of
 * objects), the collision link unlinked, the cameras last first, then BonusFight's. */
void StageJewelHunt_destroy(StageJewelHunt *this, s32 flags) {
    Group *group;
    ListNode *node;

    this->base.base.vtable = STAGE_JEWEL_HUNT_VTABLE;
    if (this->groups != NULL) {
        for (group = &this->groups[3]; this->groups != group;) {
            group--;
            group->object.vtable->destroy.func((u8 *)group + group->object.vtable->destroy.delta, 0);
        }
    }
    /* listRemove(&this->collisionLink.link), in the access order the game's compiler used */
    node = &this->collisionLink.link;
    this->collisionLink.link.next->prev = this->collisionLink.link.prev;
    node->prev->next = this->collisionLink.link.next;
    this->collisionLink.link.next = node;
    this->collisionLink.link.prev = node;
    cameraDestroy(&this->cameras[2], DESTROY_BASES);
    cameraDestroy(&this->cameras[1], DESTROY_BASES);
    cameraDestroy(&this->cameras[0], DESTROY_BASES);
    BonusFight_destroy(&this->base, flags);
}

/* The x of the column with the most jewels (the first of the fullest, column 0 when there are none), where
 * JewelHuntController_getButtons aims. */
s32 st09_getFullestColumnX(StageJewelHunt *this) {
    s32 column;
    s32 best = 0;
    s32 bestCount = 0;

    for (column = 0; column < 10; column++) {
        s32 count = 0;
        s32 row;

        for (row = 0; row < 4; row++) {
            count += this->jewelCounts[column][row];
        }
        if (bestCount < count) {
            bestCount = count;
            best = column;
        }
    }
    return (best << 8) - 0x480;
}

/* The stage's override of Fight_startRound: BonusFight's, then the frames cleared, the arena's bounds, the scene, the
 * hold counts and the HUD reset, the throwers and func_800D476C; the result waits 90 frames. */
void StageJewelHunt_startRound(StageJewelHunt *this) {
    BonusFight_startRound(&this->base);
    st09_clearFrames(this);
    Fight_setBounds(&this->base.base, -0x800, 0x800, -0xC80, 0x400, 0);
    this->scene = Fight_getMeshScene(&this->base.base);
    this->unk15D2 = 0;
    this->unk15D4 = 0;
    this->base.hudHidden = 0;
    st09_resetCharge(this);
    st09_createThrowers(&this->base.base);
    func_800D476C(this);
    this->base.resultDelay = 90;
}

/* Clears the stage's frames. */
void st09_clearFrames(StageJewelHunt *this) {
    this->frames = 0;
}

/* The stage's override of Fight_tickScenery: both fighters kept invulnerable, BonusFight's, then its own four
 * steps, and counts the frame. */
void StageJewelHunt_tickScenery(StageJewelHunt *this) {
    Fight_getFighter(&this->base.base, 0)->invulnerableFrames = 10;
    Fight_getFighter(&this->base.base, 1)->invulnerableFrames = 10;
    BonusFight_tickScenery(&this->base);
    st09_keepFightersInside(this);
    st09_updateCharge(this);
    func_800D49A0(this);
    st09_endOnTimeUp(this);
    this->frames++;
}

/* Keeps both fighters inside the arena: x within 0x500 less the character's height on either side. */
void st09_keepFightersInside(StageJewelHunt *this) {
    s32 side;

    for (side = 0; side < 2; side++) {
        s32 height = FIGHTER_PARAMS[Fighter_getCharacter(Fight_getFighter(&this->base.base, side))].height;
        Vec3 position;

        Fighter_getPosition(Fight_getFighter(&this->base.base, side), &position);
        if (abs(position.vx) >= 0x500 - height) {
            if (position.vx > 0) {
                position.vx = 0x500 - height;
            } else {
                position.vx = height - 0x500;
            }
        }
        Fighter_setPosition(Fight_getFighter(&this->base.base, side), &position);
    }
}

/* where the stage's view looks at, and from */
extern Vec3 ST09_VIEW_TARGET;
extern Vec3 ST09_VIEW_EYE;

/* look_at.c's lookAtSetTarget and lookAtSetEye, which the game inlined here (the second dirty store is merged
 * into the first) */
static inline void setTarget(LookAt *this, Vec3 *target) {
    s32 x, y, z;

    this->dirty = 1;
    x = target->vx;
    y = target->vy;
    z = target->vz;
    this->target.vx = x;
    this->target.vy = y;
    this->target.vz = z;
}

static inline void setEye(LookAt *this, Vec3 *eye) {
    s32 x, y, z;

    this->dirty = 1;
    x = eye->vx;
    y = eye->vy;
    z = eye->vz;
    this->eye.vx = x;
    this->eye.vy = y;
    this->eye.vz = z;
}

/* The stage's override of Fight_tick: BonusFight's, then, while the HUD is shown, the fixed view from
 * ST09_VIEW_EYE at ST09_VIEW_TARGET. */
void StageJewelHunt_tick(StageJewelHunt *this) {
    LookAt *lookAt;

    BonusFight_tick(&this->base);
    if (this->base.hudHidden == 0) {
        lookAt = Fight_getLookAt(&this->base.base);
        setTarget(lookAt, &ST09_VIEW_TARGET);
        setEye(lookAt, &ST09_VIEW_EYE);
        Fight_applyView(&this->base.base);
    }
}

/* Draws group i in the battle's view through camera i, its world then invalidated, and links that camera's table after
 * the main camera's last-drawn slot. */
static inline void st09_drawGroup(StageJewelHunt *this, s32 i, LookAt *lookAt, Camera *mainCamera) {
    MATRIX view;

    lookAtGetView(lookAt, &view);
    cameraSetView(&this->cameras[i], &view);
    cameraApplyLights(&this->cameras[i]);
    sceneObjectDrawInView(&this->groups[i].object, &this->cameras[i]);
    this->groups[i].object.vtable->invalidate.func((u8 *)&this->groups[i]
                                                   + this->groups[i].object.vtable->invalidate.delta);
    orderingTableLinkAfter(&this->cameras[i].ot, mainCamera->ot.lastDrawn);
}

/* The stage's override of Fight_drawWorld: its three cameras' tables cleared, the base's draw with the main camera's
 * lights, then each group through its own camera. */
void StageJewelHunt_drawWorld(StageJewelHunt *this) {
    Camera *camera;
    LookAt *lookAt;

    camera = Fight_getCamera(&this->base.base);
    cameraClearOt(&this->cameras[0]);
    cameraClearOt(&this->cameras[1]);
    cameraClearOt(&this->cameras[2]);
    cameraApplyLights(camera);
    Fight_drawWorld(this);
    lookAt = Fight_getLookAt(&this->base.base);
    st09_drawGroup(this, 0, lookAt, camera);
    st09_drawGroup(this, 1, lookAt, camera);
    st09_drawGroup(this, 2, lookAt, camera);
}

/* At the end of the time (timeLeft below 2), once: the higher score wins, equal scores end in a draw; then the
 * bonus HUD is hidden. */
void st09_endOnTimeUp(StageJewelHunt *this) {
    if (this->base.base.timeLeft < 2 && this->base.hudHidden == 0) {
        if (this->base.scores[0] == this->base.scores[1]) {
            BonusFight_endInDraw(&this->base);
        } else if (this->base.scores[0] > this->base.scores[1]) {
            BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 0),
                                     Fight_getFighter(&this->base.base, 1));
        } else {
            BonusFight_endWithWinner(&this->base, Fight_getFighter(&this->base.base, 1),
                                     Fight_getFighter(&this->base.base, 0));
        }
        this->base.hudHidden = 1;
    }
}

/* Clears the hold counts and sets the gauge's maximum to BONUS_CHARGE_FRAMES (as st07_resetCharge). */
void st09_resetCharge(StageJewelHunt *this) {
    this->holdFrames[0] = 0;
    this->holdFrames[1] = 0;
    this->heldCharge[1] = 0;
    this->heldCharge[0] = 0;
    this->base.gaugeMax = BONUS_CHARGE_FRAMES;
}

/* g++'s `new` of a BonusThrower: NULL when the allocation fails. The match depends
 * on this helper: written out in the loop, the second NULL check folds into the first. */
static inline Entity *newThrower(Fight *owner, s32 side) {
    Entity *child = allocEntityMemory(sizeof(BonusThrower));

    if (child != NULL) {
        BonusThrower_init(entityOperatorNewAt(sizeof(BonusThrower), child), owner, side);
    }
    return child;
}

/* Creates the stage's two BonusThrowers (side 0 and 1), each put into the entity group of this. */
void st09_createThrowers(Fight *this) {
    s32 i;

    for (i = 0; i < 2; i++) {
        Entity *child = newThrower(this, i);

        if (child != NULL) {
            Fight_addEntity(this, child);
        }
    }
}

INCLUDE_ASM("st09/nonmatchings/st09", func_800D476C);

/* Sets a side's gauge: BonusFight's setter, out of line in game.bin as func_800D3284. The match
 * depends on its s16 value parameter: promoted to int, it loads the u16 count with lh before the store. */
static inline void setGauge(StageJewelHunt *this, s32 index, s16 value) {
    this->base.gauges[index] = value;
}

/* Each frame, for both sides: counts the frames their button 0x80 is held (up to BONUS_CHARGE_FRAMES), keeps the
 * count in heldCharge when it is let go, and copies the count to the side's gauge (as st07_updateCharge). */
void st09_updateCharge(StageJewelHunt *this) {
    struct InputHistory *pads[2];
    u32 buttons[2];
    s32 i;

    for (i = 0; i < 2; i++) {
        pads[i] = Fight_getInput(&this->base.base, i);
        buttons[i] = InputHistory_getHeld(pads[i]);
        if (buttons[i] & 0x80) {
            if (++this->holdFrames[i] > BONUS_CHARGE_FRAMES) {
                this->holdFrames[i] = BONUS_CHARGE_FRAMES;
            }
            this->heldCharge[i] = 0;
        } else {
            this->heldCharge[i] = this->holdFrames[i];
            this->holdFrames[i] = 0;
        }
        setGauge(this, i, this->holdFrames[i]);
    }
}

struct Unk800D5BA0 *func_800D5BA0(struct Unk800D5BA0 *this, StageJewelHunt *stage, MeshScene *scene, Vec3 *vec,
                                  u16 kind);

/* Every 16 frames, until 16 have been made: a new func_800D5BA0 entity of a random kind (0 to 2), whose x
 * depends on the kind and y is random. The random words are read as signed (s32) for % and abs. */
void func_800D49A0(StageJewelHunt *this) {
    Vec3 vec;
    Entity *entity;
    u16 kind;

    if (this->unk15A0 < 16 && (this->frames & 0xF) == 0) {
        kind = abs((s32)Fight_random(&this->base.base) % 3);
        switch (kind) {
        case 0:
            vec.vx = ((((s32)Fight_random(&this->base.base) % 5) | 1) << 8) - 0x80;
            break;
        case 1:
            if (Fight_random(&this->base.base) & 1) {
                vec.vx = -0x500;
            } else {
                vec.vx = 0x500;
            }
            break;
        case 2:
            vec.vx = ((((s32)Fight_random(&this->base.base) % 5) & ~1) << 8) - 0x80;
            break;
        }
        vec.vy = -((Fight_random(&this->base.base) & 3) << 8) - 0x180;
        vec.vz = 0;
        entity = NEW_ENTITY(0x4C, func_800D5BA0, this, this->scene, &vec, kind);
        if (entity != NULL) {
            Fight_addStageEntity(&this->base.base, entity);
            this->unk15A0++;
        }
    }
}

/* The object a BonusThrower throws (St09Throw_init, virtual table ST09_THROW_VTABLE, 0x78 bytes). */
typedef struct St09Throw {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageJewelHunt *stage;
    /* 0x1C */ u16 side; /* indexes the stage's throwPositions by side (lhu) */
    /* 0x1E */ u16 slot; /* and by slot (BonusThrower_throw's nextThrowSlot) */
    /* 0x20 */ s16 direction; /* -1 for a throw toward +x, else 1 */
    /* 0x22 */ u16 ticks;     /* counted up by St09Throw_tick */
    /* 0x24 */ SVECTOR corners[4]; /* a 128 by 128 square */
    /* 0x44 */ Vec3 force;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ Vec3 velocity;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ Vec3 position;
    /* 0x70 */ u8 unk70[0x74 - 0x70];
    /* 0x74 */ s32 scale; /* the thrower's charge times 0xCC, the draw's scale */
} St09Throw;

/* Copies a position into one of the stage's throwPositions. */
static inline void copyThrowPosition(VECTOR *dst, Vec3 *position) {
    s32 x = position->vx;
    s32 y = position->vy;
    s32 z = position->vz;

    dst->vx = x;
    dst->vy = y;
    dst->vz = z;
}

/* Constructs a thrown object of the stage at position: launch's vx and vy are its force, vz its scale, pad its side;
 * records the position in the stage's throwPositions[side][slot], makes a 128 by 128 square and loads sound 0x800A.
 * The match depends on each copy reading its words into its own block's locals first, and on the if/else for
 * direction (as in St07Ball_init). */
St09Throw *St09Throw_init(St09Throw *this, StageJewelHunt *stage, Vec3 *position, VECTOR *launch, s32 slot) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST09_THROW_VTABLE;
    this->stage = stage;
    this->scale = launch->vz;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = launch->vx;
        s32 y = launch->vy;

        this->force.vx = x;
        this->force.vy = y;
    }
    this->force.vz = 0;
    this->unk50 = 0;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->unk60 = 0;
    if (this->force.vx > 0) {
        this->direction = -1;
    } else {
        this->direction = 1;
    }
    this->side = launch->pad;
    this->slot = slot;
    copyThrowPosition(&this->stage->throwPositions[this->side][slot], position);
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->ticks = 0;
    soundControlLoadSound(&SOUND_CONTROL, 0x800A, 1);
    return this;
}

/* St09Throw's destructor. */
void St09Throw_destroy(Entity *this, s32 flags) {
    this->vtable = &ST09_THROW_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st09/nonmatchings/st09", St09Throw_tick);

INCLUDE_ASM("st09/nonmatchings/st09", St09Throw_draw);

/* Constructs the BonusThrower of side for owner: the corners of a 128 by 128 square, then BonusThrower_place. */
BonusThrower *BonusThrower_init(BonusThrower *this, Fight *owner, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &BONUS_THROWER_VTABLE;
    this->owner = owner;
    this->side = side;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x40;
    this->corners[1].vx = this->corners[3].vx = 0x40;
    this->corners[0].vy = this->corners[1].vy = -0x40;
    this->corners[2].vy = this->corners[3].vy = 0x40;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->frame = 0;
    BonusThrower_place(this);
    return this;
}

/* BonusThrower's destructor. */
void BonusThrower_destroy(Entity *this, s32 flags) {
    this->vtable = &BONUS_THROWER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The stage that owns a thrower. */
static inline StageJewelHunt *throwerStage(BonusThrower *thrower) {
    return (StageJewelHunt *)thrower->owner;
}

/* BonusThrower's tick: BonusThrower_place, BonusThrower_throw, then counts its frames. */
void BonusThrower_tick(BonusThrower *this, s32 arg) {
    BonusThrower_place(this);
    BonusThrower_throw(this);
    this->frame++;
}

/* Each frame for its side: scale follows the hold count; when the button is let go after a hold (heldCharge) and the
 * stage allows it, throws an St09Throw from position, harder the longer the hold, in the slot after the last,
 * plays sound 10 and holds the side's fighter (BonusFight_holdFighter). */
void BonusThrower_throw(BonusThrower *this) {
    VECTOR vec;
    Entity *entity;
    s32 power;
    s32 x;
    s32 y;

    this->scale = throwerStage(this)->holdFrames[this->side] * 0xCC;
    if (throwerStage(this)->base.holding[this->side] == 0 && throwerStage(this)->base.hudHidden == 0
        && throwerStage(this)->heldCharge[this->side] != 0) {
        /* The match depends on vec's address taken here: the game keeps it in a saved register from the start
         * of the block. */
        VECTOR *launch = &vec;

        vec.vx = 0;
        vec.vy = 0;
        vec.vz = 0;
        vec.pad = 0;
        power = ((throwerStage(this)->heldCharge[this->side] + 8) >> 1) * 0x71C;
        x = ((s64)power * 0x4000) >> 16;
        vec.vx += x * Fighter_getFacing(Fight_getFighter(&throwerStage(this)->base.base, this->side));
        y = ((s64)power * 0x10000) >> 16;
        vec.vy -= y;
        vec.vz = throwerStage(this)->heldCharge[this->side] * 0xCC;
        vec.pad = this->side;
        this->position.vz = 0;
        entity = NEW_ENTITY(0x78, St09Throw_init, throwerStage(this), &this->position, launch,
                            throwerStage(this)->nextThrowSlot[this->side]);
        if (entity != NULL) {
            entityAddSibling(&this->entity, entity);
            throwerStage(this)->nextThrowSlot[this->side]++;
            throwerStage(this)->nextThrowSlot[this->side] %= 30;
        }
        soundControlPlaySound(&SOUND_CONTROL, 10, 0);
        BonusFight_holdFighter(&throwerStage(this)->base, this->side);
    }
}


INCLUDE_ASM("st09/nonmatchings/st09", BonusThrower_place);

INCLUDE_ASM("st09/nonmatchings/st09", BonusThrower_draw);

/* The entity func_800D572C constructs (virtual table D_800D39C4): 24 items that slide up from y -0x4C4
 * (even ones) or down from y -0x40 (odd ones), fading as they go. */
typedef struct Unk800D572C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 frame;
    /* 0x1A */ u16 count; /* items moving: 2 more every 4 frames, up to 24 */
    /* 0x1C */ u16 speed[24];
    /* 0x4C */ u8 brightness[24];
    /* 0x64 */ u16 unk64[24]; /* the item stays still when set */
    /* 0x94 */ VECTOR position[24];
} Unk800D572C;

/* PsyQ libc (stdlib.h); not in psyq.h yet. */

/* Whether item i is an even one.
 * The match depends on this inline: its store-flag gives the game's xori/andi test. */
static inline s32 isEven(s32 i) {
    return (i & 1) == 0;
}

/* Constructs the stage's sliding items: even ones at y -0x4C4, odd ones at -0x40, at full brightness, still. */
Unk800D572C *func_800D572C(Unk800D572C *this) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D39C4;
    for (i = 0; i < 24; i++) {
        /* The match depends on a whole setVector in each arm (the stores before the join). */
        if (isEven(i)) {
            setVector(&this->position[i], 0, -0x4C4, 0);
        } else {
            setVector(&this->position[i], 0, -0x40, 0);
        }
        this->brightness[i] = 0x80;
        this->speed[i] = 0;
        this->unk64[i] = 0;
    }
    this->frame = 0;
    this->count = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D39C4, which func_800D572C installs. */
void func_800D57D4(Entity *this, s32 flags) {
    this->vtable = &D_800D39C4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}


/* Unk800D572C's tick: every 4 frames two more items move; each speeds up, its brightness falling
 * with the distance from its start, and starts again from there at the end of its way. */
void func_800D5828(Unk800D572C *this, s32 arg) {
    s32 i;

    if ((this->frame & 3) == 0) {
        this->count += 2;
        if (this->count > 24) {
            this->count = 24;
        }
    }
    for (i = 0; i < this->count; i++) {
        if (this->unk64[i] != 0) {
            continue;
        }
        if (isEven(i)) {
            /* The match depends on step and y being local to each arm (step before the position, abs()):
             * y then stays in one block and shares the position load's register. */
            s32 step = (s16)++this->speed[i];
            s32 y = this->position[i].vy + step;

            this->position[i].vy = y;
            this->brightness[i] = 0x80 - ((0x4C4 - abs(y)) >> 2);
            if (this->position[i].vy >= -0x2C3) {
                this->position[i].vy = -0x4C4;
                this->brightness[i] = 0x80;
                this->speed[i] = 0;
            }
        } else {
            s32 step = (s16)++this->speed[i];
            s32 y = this->position[i].vy - step;

            this->position[i].vy = y;
            this->brightness[i] = 0x80 - ((abs(y) - 0x40) >> 2);
            if (this->position[i].vy < -0x240) {
                this->position[i].vy = -0x40;
                this->brightness[i] = 0x80;
                this->speed[i] = 0;
            }
        }
    }
    this->frame++;
}

INCLUDE_ASM("st09/nonmatchings/st09", func_800D5998);

INCLUDE_ASM("st09/nonmatchings/st09", func_800D5BA0);

/* The entity func_800D49A0 adds every 16 frames (func_800D5BA0, virtual table D_800D399C): a mesh of the scene, of a
 * kind 0 to 2; kinds other than 1 count in the stage's jewelCounts cell under them. */
typedef struct Unk800D5BA0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *scene;
    /* 0x1C */ StageJewelHunt *stage;
    /* 0x20 */ Mesh *mesh; /* of the kind's node of the scene (D_800D3824), in the stage's group of the kind */
    /* 0x24 */ Vec3 position;
    /* 0x30 */ u8 unk30[0x34 - 0x30];
    /* 0x34 */ u16 column; /* its jewelCounts cell: (x + 0x480) >> 8 */
    /* 0x36 */ u16 row;    /* and (y + 0x480) >> 8 */
    /* 0x38 */ u16 kind;
    /* 0x3A */ s16 unk3A;
    /* 0x3C */ s16 unk3C;
    /* 0x3E */ u8 unk3E[0x40 - 0x3E];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 unk44;
    /* 0x46 */ s16 unk46;
    /* 0x48 */ u8 unk48[0x4C - 0x48];
} Unk800D5BA0;

extern EntityVtable D_800D399C;

/* Deletes a scene object through its virtual destructor (engine scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Unk800D5BA0's destructor: its cell of jewelCounts emptied (kinds other than 1), the mesh out of the scene and
 * deleted, then entityDestroy. */
void func_800D5DDC(Unk800D5BA0 *this, s32 flags) {
    this->entity.vtable = &D_800D399C;
    if (this->kind != 1) {
        this->stage->jewelCounts[this->column][this->row] = 0;
    }
    listRemove(&this->mesh->object.link);
    sceneObjectDelete(&this->mesh->object);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st09/nonmatchings/st09", func_800D5EB8);

INCLUDE_ASM("st09/nonmatchings/st09", func_800D6110);

/* Returns the local matrix of the scene object at 0x20. */
MATRIX *Unk800D5BA0_getLocalMatrix(Unk800D5BA0 *this) {
    return &this->mesh->object.transform.local.m;
}

INCLUDE_ASM("st09/nonmatchings/st09", func_800D636C);

/* Constructs a BonusLandingMark of owner at pos: a 512 by 512 square. */
BonusLandingMark *BonusLandingMark_init(BonusLandingMark *this, Fight *owner, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &BONUS_LANDING_MARK_VTABLE;
    this->owner = owner;
    /* The match depends on reading pos into x, y and z here and storing them last. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    /* The match depends on these chained assignments: the left and right sides, the top and bottom. */
    this->corners[0].vx = this->corners[2].vx = -0x100;
    this->corners[1].vx = this->corners[3].vx = 0x100;
    this->corners[0].vy = this->corners[1].vy = -0x100;
    this->corners[2].vy = this->corners[3].vy = 0x100;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->frame = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* BonusLandingMark's destructor. */
void BonusLandingMark_destroy(Entity *this, s32 flags) {
    this->vtable = &BONUS_LANDING_MARK_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* BonusLandingMark's tick: counts its frames, and deletes it after BONUS_LANDING_MARK_FRAMES. */
void BonusLandingMark_tick(BonusLandingMark *this, s32 arg) {
    if (++this->frame > BONUS_LANDING_MARK_FRAMES) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st09/nonmatchings/st09", BonusLandingMark_draw);

/* Adds child to the stage's group index. Nothing calls it. */
void StageJewelHunt_addToGroup(StageJewelHunt *this, SceneObject *child, s16 index) {
    groupAddChild(&this->groups[index], child);
}

/* Takes child out of the group it is in (StageJewelHunt_addToGroup's undo); the stage is not used. Nothing
 * calls it. */
void StageJewelHunt_removeFromGroup(StageJewelHunt *this, SceneObject *child) {
    listRemove(&child->link);
}

/* JewelHuntController's destructor: Controller's. */
void JewelHuntController_destroy(Controller *this, s32 flags) {
    Controller_destroy(this, flags);
}
