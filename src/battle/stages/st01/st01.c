#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "psyq.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/look_at.h"
#include "engine/math/divide.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "engine/system/memory.h"

/* the virtual tables the functions below install */
extern EntityVtable D_800D3B1C;
extern EntityVtable D_800D3B44;
extern EntityVtable D_800D3B6C;
extern EntityVtable D_800D3B94;
extern EntityVtable D_800D3BBC;
extern EntityVtable D_800D3BE4;
extern EntityVtable ST01_DEBRIS_VTABLE;
extern EntityVtable D_800D3C34;
extern EntityVtable ST01_RISING_QUADS_VTABLE;
extern EntityVtable ST01_HOVER_QUAD_VTABLE;
extern EntityVtable ST01_PULSE_GRID_VTABLE;
extern EntityVtable D_800D3CD4;
extern VtableEntry STAGE_HIT_VTABLE[];
extern VtableEntry STAGE_NATURE_VTABLE[];

/* The sounds the stage plays. */
enum {
    ST01_SOUND_FALL = 0x58,    /* the fall event's, stopped when it ends */
    ST01_SOUND_AMBIENT = 0x5A, /* every 256 frames of scenery */
};

/* Arena 1's stage, "Nature" in STAGE_NAMES (StageNature_init, virtual table STAGE_NATURE_VTABLE): a Fight whose
 * scenery animates CLUTs and textures, swings a node, and runs a fall event (st01_tickFallEvent); it draws its own
 * group through its own camera (st01_drawGroup) and puts the fighters back where the scenery moved them. */
typedef struct StageNature {
    /* 0x000 */ Fight base;
    /* 0xD38 */ MeshScene *scene;
    /* 0xD3C */ s32 frames; /* the scenery's frames (StageNature_tickScenery) */
    /* 0xD40 */ u8 unkD40[0xD44 - 0xD40];
    /* 0xD44 */ u32 sound;     /* sound handles, stopped by the destructor */
    /* 0xD48 */ u32 fallSound; /* the fall event's (ST01_SOUND_FALL) */
    /* 0xD4C */ Camera camera; /* its table linked after the battle camera's (StageNature_drawWorld) */
    /* 0xD88 */ Group group;   /* the mesh of node ST01_GROUP_NODE (st01_moveGroupNode) */
    /* 0xDE8 */ s16 unkDE8;
    /* 0xDEA */ s16 blinkShown[4];  /* the blinking meshes, by blink: in the scene's group (1) or taken out (0) */
    /* 0xDF2 */ s16 unkDF2[4];      /* blinkShown's copy after each toggle of state 0 (func_800D5274) */
    /* 0xDFA */ s16 blinkFrames[4]; /* of the state */
    /* 0xE02 */ s16 blinkStates[4];
    /* 0xE0A */ s16 blinkWaits[4];  /* state 0's frames, an eighth of them while unkE12 is set */
    /* 0xE12 */ s16 unkE12; /* set to 1 while st01_tickFallEvent's entities fall */
    /* 0xE14 */ Mesh *blinkMeshes[4]; /* the scene's meshes of nodes D_800D3830 */
    /* 0xE24 */ s16 unkE24;
    /* 0xE26 */ s16 unkE26;
    /* 0xE28 */ RECT clutRectA;     /* 16 by 2 pixels at (0x300, 0xF3): two CLUTs */
    /* 0xE30 */ RECT clutRectB;     /* 16 by 5 pixels at (0x300, 0xE5): five CLUTs */
    /* 0xE38 */ u16 clutsA[2][16]; /* those CLUTs read back from VRAM (st01_readAnimatedCluts), animated by */
    /* 0xE78 */ u16 clutsB[5][16]; /* func_800D5A50, which sets some colours and loads them again */
    /* 0xF18 */ RECT clutRect;  /* 16 by 1 pixels at (0x310, 0xFE): a CLUT */
    /* 0xF20 */ u16 clut[16];    /* that CLUT read back from VRAM (st01_readClut) */
    /* 0xF40 */ u16 clutCopy[16]; /* and a second copy */
    /* 0xF60 */ u8 unkF60[0xF7C - 0xF60];
    /* 0xF7C */ s16 fallingCount; /* the St01FallingObjects alive */
    /* 0xF7E */ s16 fallState;    /* st01_tickFallEvent's state */
    /* 0xF80 */ s32 fallTimer;    /* and its frames */
    /* 0xF84 */ s16 unkF84;
    /* 0xF86 */ s16 unkF86;
    /* 0xF88 */ s16 unkF88;
    /* 0xF8A */ s16 unkF8A;
    /* 0xF8C */ u8 unkF8C[0xF94 - 0xF8C];
    /* 0xF94 */ VECTOR fighterPositions[2]; /* by side, saved after each tick (StageNature_tick) */
    /* 0xFB4 */ Vec3 unkFB4; /* and saved by st01_saveFighterPositions */
    /* 0xFC0 */ u8 unkFC0[0xFC4 - 0xFC0];
    /* 0xFC4 */ Vec3 unkFC4;
    /* 0xFD0 */ u8 unkFD0[0xFD4 - 0xFD0];
    /* 0xFD4 */ s16 continueShown; /* Fight_isContinueShown, each frame (StageNature_tickScenery) */
} StageNature;

/* this stage's functions that run each frame (StageNature_tickScenery) */
void st01_keepFightersInside(StageNature *this);
void func_800D4950(StageNature *this);
void st01_spawnFallingObject(StageNature *this);
void func_800D4ECC(StageNature *this);
void st01_scrollTexture(StageNature *this);
void func_800D5274(StageNature *this);
void st01_tickFallEvent(StageNature *this);
void func_800D5A50(StageNature *this);
void func_800D5534(StageNature *this);
void st01_cycleClut(StageNature *this);
void func_800D4C54(StageNature *this);

INCLUDE_ASM("st01/nonmatchings/st01", StageNature_init);

/* The stage's destructor: stops its two sounds, destroys its group and camera, then the battle
 * object's (Fight_destroy). */
void StageNature_destroy(StageNature *this, s32 flags) {
    this->base.vtable = STAGE_NATURE_VTABLE;
    stopSound(this->sound);
    stopSound(this->fallSound);
    groupDestroy(&this->group, DESTROY_BASES);
    cameraDestroy(&this->camera, DESTROY_BASES);
    Fight_destroy(&this->base, flags);
}

INCLUDE_ASM("st01/nonmatchings/st01", StageNature_startRound);

/* The stage's override of Fight_tick: the base tick and st01_keepFightersInside, then saves the positions of the
 * fighters of sides 0 and 1. */
void StageNature_tick(StageNature *this) {
    Fight_tick(&this->base);
    st01_keepFightersInside(this);
    Fighter_getPosition(Fight_getFighter(&this->base, 0), (Vec3 *)&this->fighterPositions[0]);
    Fighter_getPosition(Fight_getFighter(&this->base, 1), (Vec3 *)&this->fighterPositions[1]);
}

#if VERSION_US
/* The stage's override of Fight_isFallen: whether position is more than 0x200 below the arena's bottom (y grows
 * downwards). */
s32 StageNature_isFallen(StageNature *this, Vec3 *position) {
    return this->base.boundBottom + 0x200 < position->vy;
}

/* The camera's eye of Fight, which Fight_updateCamera computes (as in st05). */
#define BASE_EYE(stage) ((stage)->base.eye)

/* The stage's override of Fight_updateCamera: the base update, then keeps the eye at y 0 or above it (lower y),
 * copying it into the LookAt when it moves it. The match depends on the eye read into locals before the copy, as in
 * StageVolcano_updateCamera (st05). */
void StageNature_updateCamera(StageNature *this) {
    LookAt *lookAt;
    s32 x;
    s32 y;
    s32 z;

    Fight_updateCamera(&this->base);
    if (BASE_EYE(this).vy > 0) {
        BASE_EYE(this).vy = 0;
        lookAt = Fight_getLookAt(&this->base);
        lookAt->dirty = 1;
        x = BASE_EYE(this).vx;
        y = BASE_EYE(this).vy;
        z = BASE_EYE(this).vz;
        lookAt->eye.vx = x;
        lookAt->eye.vy = y;
        lookAt->eye.vz = z;
    }
}
#endif

/* The stage's override of Fight_tickScenery: the base step, then, unless the continue prompt is shown or
 * func_8006F144 holds before the round is over, runs the stage's parts, plays ST01_SOUND_AMBIENT every 256 frames
 * and counts the frame. */
void StageNature_tickScenery(StageNature *this) {
    Fight_tickScenery(&this->base);
    if (Fight_isContinueShown(&this->base)) {
        this->continueShown = 1;
        return;
    }
    this->continueShown = 0;
    if (func_8006F144(&this->base) && !Fight_isRoundOver(&this->base)) {
        return;
    }
    func_800D4950(this);
    st01_spawnFallingObject(this);
    func_800D4ECC(this);
    st01_scrollTexture(this);
    func_800D5274(this);
    st01_tickFallEvent(this);
    func_800D5A50(this);
    func_800D5534(this);
    st01_cycleClut(this);
    func_800D4C54(this);
    if ((u8)this->frames == 0) {
        soundControlPlaySound(&SOUND_CONTROL, ST01_SOUND_AMBIENT, 0);
    }
    this->frames++;
}

/* value brought within low..high */
static inline s32 clampBetween(s32 value, s32 low, s32 high) {
    value = value < high ? value : high;
    return value < low ? low : value;
}

/* Keeps both fighters inside the arena, by twice the character's height: no lower than -0xC00, x within 0xEC0 on
 * either side. */
void st01_keepFightersInside(StageNature *this) {
    s32 side;

    for (side = 0; side < 2; side++) {
        s32 size = FIGHTER_PARAMS[Fighter_getCharacter(Fight_getFighter(&this->base, side))].height * 2;
        Vec3 position;

        Fighter_getPosition(Fight_getFighter(&this->base, side), &position);
        if (position.vy - size < -0xC00) {
            position.vy = size - 0xC00;
            Fighter_setPosition(Fight_getFighter(&this->base, side), &position);
        }
        if (position.vx < size - 0xEC0 || 0xEC0 - size < position.vx) {
            position.vx = clampBetween(position.vx, size - 0xEC0, 0xEC0 - size);
        }
        Fighter_setPosition(Fight_getFighter(&this->base, side), &position);
    }
}

void st01_drawGroup(StageNature *this);

/* The stage's override of Fight_drawWorld: st01_drawGroup first, then links the stage's
 * own ordering table after the camera's first-drawn slot. */
void StageNature_drawWorld(StageNature *this) {
    Camera *camera;

    st01_drawGroup(this);
    camera = Fight_getCamera(&this->base);
    Fight_drawWorld(this);
    orderingTableLinkAfter(&this->camera.ot, camera->ot.firstDrawn);
}

/* Clears the four halfwords at 0xF84 and the two at 0xE24; called by StageNature_init. */
void st01_clearState(StageNature *this) {
    this->unkF84 = 0;
    this->unkF86 = 0;
    this->unkF88 = 0;
    this->unkF8A = 0;
    this->unkE24 = 0;
    this->unkE26 = 0;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4950);

/* The entity class func_800D9744 constructs (virtual table D_800D3B44, 0x3C bytes), sixteen of which
 * func_800D4B14 spreads over the stage: each waits, rises on a sine path about baseY in its direction, and flies
 * off when a fighter comes near (its tick, func_800D984C, steps state 0 to 3). */
typedef struct Unk800D9744 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ s16 baseY;     /* the start's vy, which the sine path is about */
    /* 0x2E */ s16 direction; /* 1 or -1 at random; turned around by the tick */
    /* 0x30 */ u16 state;
    /* 0x32 */ u16 frame;     /* the sprite's frame, from timer */
    /* 0x34 */ u16 timer;     /* random at the start */
    /* 0x36 */ s16 speed;     /* + direction a frame while flying off */
    /* 0x38 */ u8 unk38[0x3C - 0x38];
} Unk800D9744;

Unk800D9744 *func_800D9744(Unk800D9744 *this, Fight *owner, Vec3 *start);

/* Adds 16 entities of func_800D9744's class to the stage: eight with a random positive x and y -0x7D0, eight
 * with a random negative x and y -0x800. The match depends on each side's vy being a local set before the random
 * call: its constant then lives across the call and loop hoists it into a saved register, as in the game (a
 * plain `vec.vy = -0x7D0` after vx sets it next to its store and keeps it in the loop). */
void func_800D4B14(StageNature *this) {
    Vec3 vec;
    Entity *entity;
    s32 i;

    for (i = 0; i < 16; i++) {
        if (i < 8) {
            s32 vy = -0x7D0;

            vec.vx = ((Fight_random(&this->base) & 0x1F) << 4) + 0x800;
            vec.vy = vy;
        } else {
            s32 vy = -0x800;

            vec.vx = -(((Fight_random(&this->base) & 0x1F) << 4) + 0x800);
            vec.vy = vy;
        }
        vec.vz = 0;
        entity = NEW_ENTITY(0x3C, func_800D9744, &this->base, &vec);
        if (entity != NULL) {
            Fight_addStageEntity(&this->base, entity);
        }
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4BFC);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4C54);

/* Reads the 16-colour CLUT at (0x310, 0xFE) back from VRAM twice, into 0xF20 and 0xF40. */
void st01_readClut(StageNature *this) {
    this->clutRect.x = 0x310;
    this->clutRect.y = 0xFE;
    this->clutRect.w = 16;
    this->clutRect.h = 1;
    StoreImage(&this->clutRect, (u_long *)this->clut);
    StoreImage(&this->clutRect, (u_long *)this->clutCopy);
}

/* Cycles entries 8 to 13 of the CLUT, two steps a frame, from the copy, and loads it back to VRAM. */
void st01_cycleClut(StageNature *this) {
    s32 i;

    for (i = 0; i < 6; i++) {
        this->clut[8 + i] = this->clutCopy[(i + this->frames * 2) % 6 + 8];
    }
    LoadImage(&this->clutRect, (u_long *)this->clut);
}

/* Saves the positions of the characters of sides 0 and 1 at 0xFB4 and 0xFC4. */
void st01_saveFighterPositions(StageNature *this) {
    Fighter_getPosition(Fight_getFighter(&this->base, 0), &this->unkFB4);
    Fighter_getPosition(Fight_getFighter(&this->base, 1), &this->unkFC4);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4ECC);

/* The node of the stage's scene whose mesh st01_moveGroupNode moves into the stage's group: the only code
 * difference of this function between the releases (the scene data behind it was not checked). */
#if VERSION_JP
#define ST01_GROUP_NODE 17
#else
#define ST01_GROUP_NODE 16
#endif

/* Copies the 8 by 64 pixels at (0x338, 0x80) in VRAM to (0x2E0, 0) and to (0x2E0, 0x40), then moves the mesh of
 * node ST01_GROUP_NODE of the stage's scene into the stage's group at 0xD88. */
void st01_moveGroupNode(StageNature *this) {
    RECT rect;
    Mesh *mesh;

    rect.x = 0x338;
    rect.y = 0x80;
    rect.w = 8;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0);
    rect.x = 0x338;
    rect.y = 0x80;
    rect.w = 8;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0x40);
    mesh = this->scene->meshes[ST01_GROUP_NODE];
    listRemove(&mesh->object.link);
    groupAddChild(&this->group, &mesh->object);
}

/* Copies the 8 by 64 pixels at (0x2E0, (63 - frames) % 64) in VRAM to (0x338, 0x80) each tick: a texture that
 * scrolls one line per tick (st04_scrollTexture's, every tick). The match depends on the s32 copy of frames:
 * read in place, the front end narrows the halfword result to an lhu. */
void st01_scrollTexture(StageNature *this) {
    s32 count = this->frames;
    RECT rect;

    rect.x = 0x2E0;
    rect.y = (0x3F - count) & 0x3F;
    rect.w = 8;
    rect.h = 64;
    MoveImage(&rect, 0x338, 0x80);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D512C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5274);

/* Draws the stage's group through its own camera, set to the battle's view. */
void st01_drawGroup(StageNature *this) {
    Camera *camera = &this->camera;
    MATRIX view;

    cameraClearOt(camera);
    lookAtGetView(Fight_getLookAt(&this->base), &view);
    cameraSetView(camera, &view);
    sceneObjectDrawInView(&this->group.object, camera);
    groupInvalidate(&this->group);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5534);

/* Ends the fall event: clears its state and timer and stops its sound, if any. */
void st01_stopFallEvent(StageNature *this) {
    u32 sound = this->fallSound;

    this->fallState = 0;
    this->fallTimer = 0;
    if (sound != 0) {
        stopSound(sound);
        this->fallSound = 0;
    }
}

struct St01FallingObject *St01FallingObject_init(struct St01FallingObject *this, StageNature *stage, MeshScene *scene,
                                                 Vec3 *vec);

/* A new St01FallingObject for the stage, in its scene (st01_tickFallEvent and st01_spawnFallingObject). The match
 * depends on vec going through this helper's parameter: its address is taken into a register before the allocation. */
static inline Entity *newFallingObject(StageNature *stage, Vec3 *vec) {
    return NEW_ENTITY(0xF4, St01FallingObject_init, stage, stage->scene, vec);
}

struct St01Debris *St01Debris_init(struct St01Debris *this, Fight *stage);

/* The fall event, one state at a time: after 0x400 frames, 20 St01Debris and ST01_SOUND_FALL; once no
 * St01FallingObject is alive, one more St01Debris every 8 frames and an St01FallingObject every 0x40 frames from
 * frame 0x5B, while 0x32C sways; it ends at frame 0x11B or when the round is over. */
void st01_tickFallEvent(StageNature *this) {
    s32 i;
    s32 frame;

    switch (this->fallState) {
    case 0:
        if (++this->fallTimer < 0x400) {
            break;
        }
        for (i = 0; i < 20; i++) {
            Entity *entity = NEW_ENTITY(0x4C, St01Debris_init, &this->base);
            if (entity != NULL) {
                Fight_addStageEntity(&this->base, entity);
            }
        }
        this->fallTimer = 0;
        this->fallState++;
        if (this->unkE12 == 0) {
            this->unkE12 = 1;
        }
        if (this->fallSound == 0) {
            this->fallSound = soundControlPlaySound(&SOUND_CONTROL, ST01_SOUND_FALL, 0);
        }
        /* fall through */
    case 1:
        if (this->fallingCount != 0) {
            break;
        }
        this->fallState++;
        /* fall through */
    case 2:
        if ((this->fallTimer & 7) == 0) {
            Entity *entity = NEW_ENTITY(0x4C, St01Debris_init, &this->base);
            if (entity != NULL) {
                Fight_addStageEntity(&this->base, entity);
            }
        }
        if ((this->fallTimer & 0x3F) == 0 && this->fallTimer >= 0x5B) {
            Vec3 vec;
            Entity *entity;

            vec.vx = 0;
            vec.vy = 0;
            vec.vz = 0x2000;
            entity = newFallingObject(this, &vec);
            if (entity != NULL) {
                Fight_addStageEntity(&this->base, entity);
            }
        }
        frame = ++this->fallTimer;
        this->base.target.vx += COS_SIN_TABLE[(frame << 10) & 0xFFF].sin >> 9;
        if (frame >= 0x11B || Fight_isRoundOver(&this->base) != 0) {
            u32 sound;

            if (this->unkE12 != 0) {
                this->unkE12 = 0;
            }
            sound = this->fallSound;

            this->fallTimer = 0;
            this->fallState = 0;
            if (sound != 0) {
                stopSound(sound);
                this->fallSound = 0;
            }
        }
        break;
    }
}

/* Every 0x200 frames, when no St01FallingObject is alive and no fall event runs, adds a new one to the stage.
 * The two halfword tests are one word load: gcc merges tests of adjacent fields. */
void st01_spawnFallingObject(StageNature *this) {
    Entity *entity;

    if (((this->frames + 0x80) & 0x1FF) == 0 && this->fallingCount == 0 && this->fallState == 0
        && Fight_isRoundOver(&this->base) == 0) {
        Vec3 vec;

        vec.vx = 0;
        vec.vy = 0;
        vec.vz = 0x2000;
        entity = newFallingObject(this, &vec);
        if (entity != NULL) {
            Fight_addStageEntity(&this->base, entity);
        }
    }
}

/* Takes the mesh of node 9 out of the stage's scene's group and hides that node. */
void func_800D5974(StageNature *this) {
    listRemove(&this->scene->meshes[9]->object.link);
    meshSceneHideNode(this->scene, 9);
}

/* Reads the two CLUTs at (0x300, 0xF3) and the five at (0x300, 0xE5) back from VRAM, for func_800D5A50 to
 * animate. */
void st01_readAnimatedCluts(StageNature *this) {
    this->clutRectA.x = 0x300;
    this->clutRectA.y = 0xF3;
    this->clutRectA.w = 16;
    this->clutRectA.h = 2;
    StoreImage(&this->clutRectA, (u_long *)this->clutsA);
    this->clutRectB.x = 0x300;
    this->clutRectB.y = 0xE5;
    this->clutRectB.w = 16;
    this->clutRectB.h = 5;
    StoreImage(&this->clutRectB, (u_long *)this->clutsB);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5A50);

/* Constructs a StageHit with its damage. */
StageHit *StageHit_init(StageHit *this, s32 damage) {
    this->vtable = STAGE_HIT_VTABLE;
    this->damage = damage;
    return this;
}

/* StageHit's slot 0x08: this stage's hit adds nothing into the sphere it touches. */
void StageHit_apply(StageHit *this, CollisionSphere *sphere) {
}

/* An object that falls on the arena (St01FallingObject_init, virtual table ST01_FALLING_OBJECT_VTABLE): a mesh of
 * the stage's scene with a StageHit and a CollisionSphere. */
typedef struct St01FallingObject {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageNature *stage; /* counts them at 0xF7C */
    /* 0x1C */ MeshScene *scene;
    /* 0x20 */ Mesh *mesh; /* node 9 of the scene */
    /* 0x24 */ StageHit hit; /* damage 0x830002D8 */
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ CollisionSphere sphere;
    /* 0x84 */ s32 frames; /* counts every tick (St01FallingObject_tick) */
    /* 0x88 */ u8 unk88[0x8E - 0x88];
    /* 0x8E */ s16 unk8E; /* cleared at construction */
    /* 0x90 */ u8 unk90[0x9C - 0x90];
    /* 0x9C */ Vec3 force; /* cleared after each step (St01FallingObject_fall) */
    /* 0xA8 */ s32 forcePad;
    /* 0xAC */ Vec3 velocity;
    /* 0xB8 */ s32 velocityPad;
    /* 0xBC */ u8 unkBC[0xC8 - 0xBC];
    /* 0xC8 */ Vec3 position;
    /* 0xD4 */ u8 unkD4[0xE8 - 0xD4];
    /* 0xE8 */ u16 kind; /* 0..2: indexes ST01_FLOOR_OFFSETS, and picks what a touch does (func_800D6994) */
    /* 0xEA */ s16 touched[2]; /* per side: set when the object touches that fighter (func_800D6994) */
    /* 0xEE */ s16 unkEE;
    /* 0xF0 */ s16 holdFrames[2]; /* per side: 90 after the object hits that fighter (func_800D6994) */
    /* 0xF4 */
} St01FallingObject;

extern EntityVtable ST01_FALLING_OBJECT_VTABLE;

void func_800D5E84(St01FallingObject *this);

/* Constructs a falling object of the stage at position: a copy of the scene's mesh node 9 added to the scene, a
 * StageHit with a CollisionSphere of radius 0x280 added to the fight's shapes; the stage counts one more. */
St01FallingObject *St01FallingObject_init(St01FallingObject *this, StageNature *stage, MeshScene *scene,
                                          Vec3 *position) {
    entityInit(&this->entity, ENTITY_KIND_STAGE_OBJECT);
    this->entity.vtable = &ST01_FALLING_OBJECT_VTABLE;
    StageHit_init(&this->hit, 0x830002D8);
    CollisionSphere_init(&this->sphere, position, 0x280, &this->hit);
    this->scene = scene;
    this->stage = stage;
    stage->fallingCount++;
    this->mesh = meshInit(operatorNew(sizeof(Mesh)), meshSceneRelocateNodeTmd(this->scene, 9), 0);
    groupAddChild(&this->scene->group, &this->mesh->object);
    this->frames = 0;
    this->unk8E = 0;
    func_800D5E84(this);
    Fight_addStageShape(&this->stage->base, &this->sphere.link);
    transformSetParentKeepLocalInline(&this->mesh->object.transform, NULL);
    return this;
}


/* Deletes a scene object through its virtual destructor (engine scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* St01FallingObject's destructor: one entity fewer for the stage, the mesh out of the scene and deleted, then its
 * member's and its base's destructors, inlined, and entityDestroy. */
void St01FallingObject_destroy(St01FallingObject *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &ST01_FALLING_OBJECT_VTABLE;
    this->stage->fallingCount--;
    listRemove(&this->mesh->object.link);
    sceneObjectDelete(&this->mesh->object);
    this->sphere.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->sphere.node) written out: the match depends on this access order, as in st04's
     * func_800D8618 */
    link = &this->sphere.link;
    this->sphere.link.next->prev = this->sphere.link.prev;
    link->prev->next = this->sphere.link.next;
    this->sphere.link.next = link;
    this->sphere.link.prev = link;
    this->hit.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D607C(St01FallingObject *this);
void func_800D67F8(St01FallingObject *this);

/* St01FallingObject's tick: its two steps, then each fighter it hit is held while its hold frames count down. */
void St01FallingObject_tick(St01FallingObject *this) {
    s32 side;

    func_800D607C(this);
    func_800D67F8(this);
    for (side = 0; side < 2; side++) {
        if (this->touched[side] != 0 && this->holdFrames[side] != 0) {
            Fighter *fighter = Fight_getFighter(&this->stage->base, side);
            Vec3 position;

            Fighter_getPosition(fighter, &position);
            if (position.vz != 0) {
                Fighter_hold(fighter, 1);
            }
            this->holdFrames[side]--;
        }
    }
    this->frames++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5E84);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D607C);

/* A point of the floor's profile under the falling objects: the floor's height at a depth. */
typedef struct St01FloorPoint {
    /* 0x0 */ s32 y;
    /* 0x4 */ s32 z;
} St01FloorPoint;

extern St01FloorPoint ST01_FLOOR_PROFILE[4]; /* by decreasing depth */
extern s32 ST01_FLOOR_OFFSETS[3];            /* by kind: how far above the floor the object stops */

/* The height the object stops at for a depth: the floor's profile, interpolated between the first point nearer
 * than z and the one before it, less the kind's offset (func_800D607C keeps the object above it). The match
 * depends on the offset from the point computed first (the game addresses that point's z before the rest). */
s32 St01FallingObject_getFloorY(St01FallingObject *this, s32 z) {
    s32 found;
    s32 i;
    s32 offset;
    s32 slope;

    found = 0;
    for (i = 0; i < 4; i++) {
        if (ST01_FLOOR_PROFILE[i].z < z) {
            found = i;
            i = 0xFF;
        }
    }
    offset = (z << 8) - ST01_FLOOR_PROFILE[found].z;
    slope = (ST01_FLOOR_PROFILE[found - 1].y - ST01_FLOOR_PROFILE[found].y) /
            (ST01_FLOOR_PROFILE[found - 1].z - ST01_FLOOR_PROFILE[found].z);
    return ((ST01_FLOOR_PROFILE[found].y - offset * slope) >> 8) - ST01_FLOOR_OFFSETS[this->kind];
}

/* Falls one step: gravity into the force, the force over a mass of 5 into the vertical velocity, and 1/8 of that
 * velocity (in 1/256 units) into the height. The match depends on the step local (the operands' pseudo order
 * decides the schedule). */
void St01FallingObject_fall(St01FallingObject *this) {
    s32 step;

    this->force.vy += 0x238C;
    this->velocity.vy += divide16(this->force.vy, 0x50000);
    this->force.vy = 0;
    step = this->velocity.vy >> 3;
    this->position.vy = ((this->position.vy << 8) + step) >> 8;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D652C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D67F8);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6994);

/* Returns the local matrix of the mesh (transformGetLocal, inlined). */
MATRIX *func_800D6C94(St01FallingObject *this) {
    return &this->mesh->object.transform.local.m;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6CA0);

/* Where StageNature_startRound puts one Unk800D6D08 (a 16-byte entry of its table, passed by value). */
typedef struct Unk800D6D08Place {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 size; /* the grid's side */
    /* 0x08 */ s32 z;
    /* 0x0C */ s32 shade; /* the grey of its colour */
} Unk800D6D08Place;

/* The class func_800D6D08 constructs (virtual table D_800D3CD4): a 2 by 2 grid of quads with one grey colour. */
typedef struct Unk800D6D08 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 color;
    /* 0x1C */ u16 angle;
    /* 0x20 */ GridVertex grid[3][3]; /* the quads' corners: (x + column * size / 2, 0x80, z + row * size / 2) */
    /* 0x68 */ Vec3 position;
    /* 0x74 */ s32 shade;
} Unk800D6D08; /* size 0x78 (StageNature_startRound's allocation) */

/* Unk800D6D08's constructor: the grid's corners at the place (the owner is not kept), the grey colour. The match
 * depends on `step * row` in that operand order (the game's mult). */
Unk800D6D08 *func_800D6D08(Unk800D6D08 *this, Fight *owner, Unk800D6D08Place place) {
    s32 row;
    s32 column;
    s16 step;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3CD4;
    this->position.vx = place.x;
    this->position.vy = 0;
    this->position.vz = place.z;
    this->shade = place.shade;
    step = place.size >> 1;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 3; column++) {
            this->grid[row][column].vx = this->position.vx + column * step;
            this->grid[row][column].vy = 0x80;
            this->grid[row][column].vz = this->position.vz + step * row;
        }
    }
    this->color = (u8)place.shade | ((u8)place.shade << 8) | ((u8)place.shade << 16);
    this->angle = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3CD4, which func_800D6D08 installs. */
void func_800D6DF4(Entity *this, s32 flags) {
    this->vtable = &D_800D3CD4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6E48);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6EA4);

/* A 3 by 9 grid of points around a position, with a pulsing level (St01PulseGrid_init, virtual table
 * ST01_PULSE_GRID_VTABLE). */
typedef struct St01PulseGrid {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 phase;
    /* 0x1A */ u16 level; /* 0x80 at first */
    /* 0x1C */ GridVertex points[3][9]; /* position + (column * 0x100, row * 0xC0, 0) */
    /* 0xF4 */ Vec3 position;
} St01PulseGrid;

/* St01PulseGrid's constructor: the grid's points from the VECTOR's position (the owner is not kept). */
St01PulseGrid *St01PulseGrid_init(St01PulseGrid *this, Fight *owner, VECTOR position) {
    s32 row;
    s32 column;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST01_PULSE_GRID_VTABLE;
    this->phase = 0;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    for (row = 0; row < 3; row++) {
        for (column = 0; column < 9; column++) {
            this->points[row][column].vx = this->position.vx + column * 0x100;
            this->points[row][column].vy = this->position.vy + row * 0xC0;
            this->points[row][column].vz = this->position.vz;
        }
    }
    this->level = 0x80;
    return this;
}

/* St01PulseGrid's destructor. */
void St01PulseGrid_destroy(Entity *this, s32 flags) {
    this->vtable = &ST01_PULSE_GRID_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St01PulseGrid's tick: the level follows a sine of the phase (64 to 96), and the phase steps. */
void St01PulseGrid_tick(St01PulseGrid *this) {
    this->level = ((COS_SIN_TABLE[(this->phase << 4) & 0xFFF].sin >> 7) + 160) >> 1;
    this->phase++;
}

INCLUDE_ASM("st01/nonmatchings/st01", St01PulseGrid_draw);

/* A grey quad that hovers in a figure eight around a position and flickers (St01HoverQuad_init, virtual table
 * ST01_HOVER_QUAD_VTABLE). */
typedef struct St01HoverQuad {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 phase;     /* a random start; the tick steps it, the draw turns it into the offsets */
    /* 0x1A */ s16 amplitude; /* the offsets' scale */
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 pad1E[2];
    /* 0x20 */ u32 color; /* 0x808080 at first; the tick sets a grey level from the phase's sine */
    /* 0x24 */ Vec3 position;
    /* 0x30 */ u8 unk30[4];
} St01HoverQuad;

/* St01HoverQuad's constructor: the VECTOR's three components are the position and its pad the amplitude. */
St01HoverQuad *St01HoverQuad_init(St01HoverQuad *this, Fight *owner, VECTOR position) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST01_HOVER_QUAD_VTABLE;
    this->unk1C = 0;
    this->phase = Fight_random(owner) & 0xFFF;
    this->color = 0x808080;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    this->amplitude = position.pad;
    return this;
}

/* St01HoverQuad's destructor. */
void St01HoverQuad_destroy(Entity *this, s32 flags) {
    this->vtable = &ST01_HOVER_QUAD_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", St01HoverQuad_tick);

INCLUDE_ASM("st01/nonmatchings/st01", St01HoverQuad_draw);

/* Eight grey quads that rise over a position and fade (St01RisingQuads_init, virtual table
 * ST01_RISING_QUADS_VTABLE; StageNature_startRound makes one). */
typedef struct St01RisingQuads {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ u16 frames;
    /* 0x1E */ u8 pad1E[2];
    /* 0x20 */ Vec3 position;
    /* 0x2C */ u8 unk2C[4];
    /* 0x30 */ u8 shades[8]; /* by quad: its grey level, 0x80 when it starts */
    /* 0x38 */ GridVertex quads[8]; /* by quad: vx a random x offset from the position, vy the height above it (rises
                                      * from 0 towards 0xF9, starts again at 0xC8), vz the position's z */
} St01RisingQuads; /* size 0x78 */

/* St01RisingQuads's constructor: the VECTOR's position, and each quad at a random x and height. */
St01RisingQuads *St01RisingQuads_init(St01RisingQuads *this, Fight *owner, VECTOR position) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST01_RISING_QUADS_VTABLE;
    this->owner = owner;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    for (i = 0; i < 8; i++) {
        this->shades[i] = 0x80;
        this->quads[i].vx = (Fight_random(owner) & 0xFF) - 0x7F;
        this->quads[i].vy = Fight_random(owner) & 0xFF;
        this->quads[i].vz = this->position.vz;
    }
    this->frames = 0;
    return this;
}

/* St01RisingQuads's destructor. */
void St01RisingQuads_destroy(Entity *this, s32 flags) {
    this->vtable = &ST01_RISING_QUADS_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St01RisingQuads's tick: each quad rises an eighth of the way to 0xF9 and fades as it goes; at 0xC8 it starts again
 * from height 0 at a new random x. */
void St01RisingQuads_tick(St01RisingQuads *this) {
    u8 *shade = this->shades;
    GridVertex *quad = this->quads;
    s32 i;

    for (i = 0; i < 8; i++) {
        quad->vy += (0xF9 - quad->vy) >> 3;
        if (quad->vy < 0xFF) {
            *shade = (0xFF - quad->vy) >> 1;
        } else {
            *shade = 0;
        }
        if (quad->vy >= 0xC8) {
            quad->vx = (Fight_random(this->owner) & 0xFF) - 0x7F;
            quad->vy = 0;
            *shade = 0x80;
        }
        shade++;
        quad++;
    }
    this->frames++;
}

INCLUDE_ASM("st01/nonmatchings/st01", St01RisingQuads_draw);

/* The class func_800D7A94 constructs (virtual table D_800D3C34): four quads around a side's fighter, at random
 * offsets. */
typedef struct Unk800D7A94 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageNature *stage;
    /* 0x1C */ u16 side;
    /* 0x1E */ u16 frames;
    /* 0x20 */ Vec3 position;     /* the side's fighter's, followed every tick */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ u8 shades[4];      /* 0x80 at first */
    /* 0x34 */ GridVertex quads[4]; /* offsets: x -0x40 to 0x3F, y 0 to 0x1F, z 0 to 0xFF */
} Unk800D7A94;

/* Copies a position into a Unk800D7A94, the three words read before any store (as vecCopy). */
static inline void unk800D7A94SetPosition(Unk800D7A94 *this, VECTOR *position) {
    s32 x, y, z;

    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
}

/* Unk800D7A94's constructor: at the side's fighter, four quads at random offsets. */
Unk800D7A94 *func_800D7A94(Unk800D7A94 *this, StageNature *stage, s32 side) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3C34;
    this->side = side;
    this->stage = stage;
    unk800D7A94SetPosition(this, &stage->fighterPositions[this->side]);
    for (i = 0; i < 4; i++) {
        this->shades[i] = 0x80;
        this->quads[i].vx = (Fight_random(&stage->base) & 0x7F) - 0x40;
        this->quads[i].vy = Fight_random(&stage->base) & 0x1F;
        this->quads[i].vz = Fight_random(&stage->base) & 0xFF;
    }
    this->frames = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3C34, which func_800D7A94 installs. */
void func_800D7B8C(Entity *this, s32 flags) {
    this->vtable = &D_800D3C34;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800D7A94's tick: follows the side's fighter; each quad rises a quarter of the way to 0x7D, fading as it goes,
 * and starts again at 0x78; the further the fighter is from y 0 the darker they are (none once it is above -0x80). */
void func_800D7BE0(Unk800D7A94 *this) {
    s32 i;

    unk800D7A94SetPosition(this, &this->stage->fighterPositions[this->side]);
    for (i = 0; i < 4; i++) {
        this->quads[i].vz += (0x7D - this->quads[i].vz) >> 2;
        if (this->quads[i].vz < 0x80) {
            this->shades[i] = 0x80 - this->quads[i].vz;
        } else {
            this->shades[i] = 0;
        }
        if (this->quads[i].vz >= 0x78) {
            this->quads[i].vz = 0;
            this->shades[i] = 0x80;
        }
        if (this->position.vy < -0x80) {
            this->shades[i] = 0;
        } else {
            this->shades[i] >>= (this->position.vy >= 0 ? this->position.vy : -this->position.vy) >> 3;
        }
    }
    this->frames++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7CE8);

/* A gray particle that starts at a random place and falls (St01Debris_init) or starts around a position
 * (St01Debris_initAt); virtual table ST01_DEBRIS_VTABLE, made by st01_tickFallEvent and func_800D5534. */
typedef struct St01Debris {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *stage;
    /* 0x1C */ s16 unk1C; /* 4 */
    /* 0x1E */ s16 unk1E; /* a random bit */
    /* 0x20 */ u32 color;
    /* 0x24 */ s16 hit;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 angle; /* random */
    /* 0x2A */ u8 unk2A[0x2C - 0x2A];
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ Vec3 start;
    /* 0x48 */ s32 unk48; /* random, 6 to 13 */
} St01Debris;

/* Constructs a particle of the stage at a random x (-0x800 to 0x7FF) and z (0 to 0x3FF), at y -0xD00. */
St01Debris *St01Debris_init(St01Debris *this, Fight *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST01_DEBRIS_VTABLE;
    this->stage = stage;
    this->start.vx = this->position.vx = (Fight_random(stage) & 0xFFF) - 0x800;
    this->start.vy = this->position.vy = -0xD00;
    this->start.vz = this->position.vz = Fight_random(stage) & 0x3FF;
    this->unk48 = (Fight_random(stage) & 7) + 6;
    this->unk1E = ((s32)Fight_random(stage) >> 3) & 1;
    this->hit = this->unk26 = 0;
    this->angle = Fight_random(stage) & 0xFFF;
    this->color = 0x808080;
    this->unk1C = 4;
    return this;
}

INCLUDE_ASM("st01/nonmatchings/st01", St01Debris_initAt);

/* St01Debris's destructor. */
void St01Debris_destroy(Entity *this, s32 flags) {
    this->vtable = &ST01_DEBRIS_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", St01Debris_tick);

INCLUDE_ASM("st01/nonmatchings/st01", St01Debris_draw);

extern s32 D_800DA060[]; /* by character: 1 or 0, the peak is 0x80 or 0x100 */

/* The entity class of virtual table D_800D3BE4 (func_800D84B8, 0x54 bytes): a quad near a position that rises by
 * speed until it reaches peak, then spreads and fades (func_800D8650). */
typedef struct Unk800D84B8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ SVECTOR position;
    /* 0x20 */ s16 peak;      /* the rise at which it spreads: 0x100 - D_800DA060[character] * 0x80 */
    /* 0x22 */ u16 frames;
    /* 0x24 */ u16 state;
    /* 0x26 */ u8 brightness; /* 0x80, quartered each frame while it spreads */
    /* 0x28 */ u16 speed;     /* 0x3F..0x42 */
    /* 0x2A */ s16 rise;      /* how far the top corners are from center */
    /* 0x2C */ SVECTOR center; /* a random point within 0x80 in x and z */
    /* 0x34 */ SVECTOR corners[4];
} Unk800D84B8; /* size 0x54 */

/* Constructs the entity at position (y 0x4C) for side's fighter: its peak from the character, a random speed and
 * center, the corners a 0x100-wide line through the center. */
Unk800D84B8 *func_800D84B8(Unk800D84B8 *this, Fight *stage, VECTOR position, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BE4;
    this->peak = D_800DA060[Fighter_getCharacter(Fight_getFighter(stage, side))];
    this->position.vx = position.vx;
    this->position.vy = 0x4C;
    this->position.vz = position.vz;
    this->frames = 0;
    this->state = 0;
    this->speed = (Fight_random(stage) & 3) + 0x3F;
    this->brightness = 0x80;
    this->rise = 0;
    this->center.vx = (Fight_random(stage) & 0xFF) - 0x80;
    this->center.vy = 0;
    this->center.vz = (Fight_random(stage) & 0xFF) - 0x80;
    this->corners[0].vx = this->corners[2].vx = this->center.vx - 0x80;
    this->corners[1].vx = this->corners[3].vx = this->center.vx + 0x80;
    this->corners[0].vy = this->corners[1].vy = this->corners[2].vy = this->corners[3].vy = this->center.vy;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = this->center.vz;
    this->peak = 0x100 - (this->peak << 7);
    return this;
}

/* The destructor of the entity class of vtable D_800D3BE4, which func_800D84B8 installs. */
void func_800D85FC(Entity *this, s32 flags) {
    this->vtable = &D_800D3BE4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8650);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8808);

/* A value per character (-1 for some), which the entity of vtable D_800D3BBC keeps for each side's fighter. */
extern s32 D_800D3A3C[];

/* The entity class of virtual table D_800D3BBC (func_800D8994): a quad that takes the two sides' values in turn
 * (count & 1) and is not drawn while its value is -1; its tick (func_800D8AF4) opens and closes it about
 * y -0x29B. */
typedef struct Unk800D8994 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x28 - 0x18];
    /* 0x28 */ s16 sideValues[2]; /* D_800D3A3C of each side's character */
    /* 0x2C */ s16 value;         /* sideValues[count & 1]; -1: not drawn */
    /* 0x2E */ u16 wait;          /* the frames a state waits, random */
    /* 0x30 */ u16 count;
    /* 0x32 */ u16 frames;
    /* 0x34 */ u16 stateFrames;
    /* 0x36 */ u16 state;
    /* 0x38 */ u8 unk38[0x3A - 0x38];
    /* 0x3A */ s16 opening; /* 0x102 while open; the corners stand a third of it above and below y -0x29B */
    /* 0x3C */ SVECTOR corners[4];
} Unk800D8994; /* size 0x5C */

/* Constructs the entity: each side's value from its character, its corners (a quad at x 0x9DE..0xAA8 slanting from
 * z 0x297 to 0x25D, y -0x29B), the rest cleared and a random first wait below 0x20. */
Unk800D8994 *func_800D8994(Unk800D8994 *this, Fight *stage) {
    s32 side;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BBC;
    for (side = 0; side < 2; side++) {
        this->sideValues[side] = D_800D3A3C[Fighter_getCharacter(Fight_getFighter(stage, side))];
    }
    this->corners[0].vx = this->corners[2].vx = 0x9DE;
    this->corners[1].vx = this->corners[3].vx = 0xAA8;
    this->corners[0].vy = this->corners[1].vy = this->corners[2].vy = this->corners[3].vy = -0x29B;
    this->corners[0].vz = this->corners[2].vz = 0x297;
    this->corners[1].vz = this->corners[3].vz = 0x25D;
    this->frames = 0;
    this->state = 0;
    this->stateFrames = 0;
    this->opening = 0;
    this->count = 0;
    this->value = 0;
    this->wait = Fight_random(stage) & 0x1F;
    return this;
}

/* The destructor of the entity class of vtable D_800D3BBC, which func_800D8994 installs. */
void func_800D8AA0(Entity *this, s32 flags) {
    this->vtable = &D_800D3BBC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick: waits a random time, opens the quad (opening 0x102), waits again, then closes it, takes the other side's
 * value and starts over; the corners follow it. */
void func_800D8AF4(Unk800D8994 *this, Fight *stage) {
    switch (this->state) {
    case 0:
        if (++this->stateFrames < this->wait) {
            break;
        }
        this->wait = Fight_random(stage) & 0xF;
        this->stateFrames = 0;
        this->state++;
    case 1:
        this->opening = 0x102;
        this->state++;
    case 2:
        if (++this->stateFrames < this->wait) {
            break;
        }
        this->stateFrames = 0;
        this->state++;
    case 3:
        this->opening = 0;
        this->wait = Fight_random(stage) & 0x1F;
        this->count++;
        this->value = this->sideValues[this->count & 1];
        this->opening = 0; /* cleared again after the call, as the game does */
        this->state = 0;
    }
    this->frames++;
    this->corners[0].vy = this->corners[1].vy = -0x29B - this->opening / 3;
    this->corners[2].vy = this->corners[3].vy = this->opening / 3 - 0x29B;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8C70);

/* The entity class of virtual table D_800D3B94 (func_800D8E1C constructs two of them, 0x44 bytes): it sways around
 * center by sines of its phase (func_800D8F08). */
typedef struct Unk800D8E1C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR position; /* center plus the sway */
    /* 0x28 */ VECTOR center;
    /* 0x38 */ s16 direction; /* -1 for the first, 1 for the second: the sign of the x and z sway */
    /* 0x3A */ u16 phase;     /* a random start, stepped each tick */
    /* 0x3C */ u8 sphere[4];
    /* 0x40 */ s16 tilts[2]; /* the draw's z angles (func_800D8FD4): the sine of phase * 256, / 4, and its negative */
} Unk800D8E1C;

/* Unk800D8E1C's constructor: both at (-0x900, -0x580, -0x200), the second (index 1) swaying the other way. */
Unk800D8E1C *func_800D8E1C(Unk800D8E1C *this, Fight *owner, s32 index) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B94;
    this->position.vx = this->center.vx = -0x900;
    this->position.vy = this->center.vy = -0x580;
    this->position.vz = this->center.vz = -0x200;
    this->direction = (index == 0) ? -1 : 1;
    this->tilts[1] = 0;
    this->tilts[0] = 0;
    this->phase = Fight_random(owner);
    return this;
}

/* Unk800D8E1C's destructor. */
void func_800D8EB4(Entity *this, s32 flags) {
    this->vtable = &D_800D3B94;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* getSin (engine/math/sin_cos.c), inline. */
static inline s16 getSinInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].sin;
}

/* Unk800D8E1C's tick: position sways around center by sines of the phase (x and z by direction, cut to 16 bits),
 * tilts follow a faster sine, and the phase steps. The match depends on the x and z sways reading the table directly
 * (lhu, then shifts) while the others go through getSin (lh). */
void func_800D8F08(Unk800D8E1C *this) {
    s32 swayX;
    s32 swayZ;
    s32 swayY;
    s32 sine;

    sine = getSinInline(this->phase * 256) >> 2;
    swayY = getSinInline(this->phase * 32) >> 5;
    swayX = (COS_SIN_TABLE[(this->phase * 8) & 0xFFF].sin >> 2) * this->direction;
    swayZ = (COS_SIN_TABLE[(this->phase * 16) & 0xFFF].sin >> 2) * this->direction;
    this->tilts[0] = sine;
    this->tilts[1] = -sine;
    this->phase++;
    this->position.vy = this->center.vy + swayY;
    this->position.vx = this->center.vx + (s16)swayX;
    this->position.vz = this->center.vz + (s16)swayZ;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8FD4);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9158);

/* The destructor of the entity class of vtable D_800D3B6C, which func_800D9158 installs. */
void func_800D927C(Entity *this, s32 flags) {
    this->vtable = &D_800D3B6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D92D0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9458);

/* Constructs one of them at start, in a random direction and with a random timer. */
Unk800D9744 *func_800D9744(Unk800D9744 *this, Fight *owner, Vec3 *start) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B44;
    this->owner = owner;
    /* The match depends on a store in each arm (a ternary puts the value in another register). */
    if ((Fight_random(owner) & 1) == 0) {
        this->direction = -1;
    } else {
        this->direction = 1;
    }
    this->position.vx = start->vx;
    this->position.vy = this->baseY = start->vy;
    this->position.vz = start->vz;
    this->timer = Fight_random(owner);
    this->state = 0;
    this->frame = 0;
    this->speed = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3B44, which func_800D9744 installs. */
void func_800D97F8(Entity *this, s32 flags) {
    this->vtable = &D_800D3B44;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D984C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9A98);

/* The entity class func_800D9BF4 constructs (virtual table D_800D3B1C, 0x34 bytes); its tick counts at 0x1C, which
 * the constructor clears. */
typedef struct Unk800D9BF4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ s16 unk1E; /* StageNature_init passes whether its counter has reached 6 */
    /* 0x20 */ u16 unk20;
    /* 0x22 */ u8 pad22[2];
    /* 0x24 */ Vec3 position;
    /* 0x30 */ s32 unk30; /* 0x1000 at first */
} Unk800D9BF4;

/* Unk800D9BF4's constructor: the owner, a copy of the position and of *arg3 (StageNature_init passes a table
 * entry's position and the u16 at its 0xC), and arg4. */
Unk800D9BF4 *func_800D9BF4(Unk800D9BF4 *this, Fight *owner, Vec3 *position, u16 *arg3, s16 arg4) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B1C;
    this->owner = owner;
    this->unk1E = arg4;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk20 = *arg3;
    this->unk30 = 0x1000;
    this->unk1C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3B1C, which func_800D9BF4 installs. */
void func_800D9C90(Entity *this, s32 flags) {
    this->vtable = &D_800D3B1C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of D_800D3B1C: adds 1 to 0x1C. */
void func_800D9CE4(Unk800D9BF4 *this) {
    this->unk1C++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9CF8);

/* StageHit's destructor: CollisionOwner's, inlined. */
void StageHit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the hit's knockback, reading all three components before writing any (as vecCopy does). */
void StageHit_setKnockback(StageHit *this, Vec3 *knockback) {
    s32 x, y, z;

    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}
