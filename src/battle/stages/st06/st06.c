#include "common.h"
#include <abs.h>
#include "battle/game.h"
#include "battle/game_classes.h"
#include "psyq.h"
#include "engine/gfx/box.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/color.h"
#include "engine/gfx/display.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"

/* An entity the stage keeps by handle, with the level it last gave it (0 to 0xFF, st06_applyLevel). */
typedef struct St06LevelEntry {
    /* 0x0 */ u32 handle;
    /* 0x4 */ s32 level;
} St06LevelEntry;

/* Arena 6's stage, "Boss" in STAGE_NAMES (StageBoss_init, virtual table STAGE_BOSS_VTABLE): a Fight whose arena
 * moves along x with the fighters on it (StageBoss_tick); it keeps four scenery entities by handle with a level. */
typedef struct StageBoss {
    /* 0x000 */ Fight base;
    /* 0xD38 */ St06LevelEntry levels[4]; /* st06_setLevel */
    /* 0xD58 */ MeshSceneLink collisionLink; /* lines of the arena itself, in COLLISION_LINKS (StageBoss_init) */
    /* 0xD70 */ s32 rounds; /* the rounds started (StageBoss_startRound) */
    /* 0xD74 */ s32 scrollX; /* the x its meshes are put at: the fighters' centre (StageBoss_tick) */
    /* 0xD78 */ s32 frames; /* the scenery ticks of the round (StageBoss_tickScenery) */
    /* 0xD7C */ u32 freezeFrame; /* set: copy the shown buffer once, a frozen frame (StageBoss_drawWorld) */
    /* 0xD80 */ u32 sound; /* a sound's handle (StageBoss_startRound plays it), stopped by the destructor */
} StageBoss;

void st06_applyLevel(St06LevelEntry *entry);
void st06_setLevel(StageBoss *this, s32 index, s32 level);

/* The entity class of virtual table D_800D3A4C (0x28 bytes). */
typedef struct Unk800D5164 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5164;

extern EntityVtable D_800D3A4C;

/* The entity class func_800D5040 constructs (virtual table D_800D3A74). */
typedef struct Unk800D5040 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5040;

extern EntityVtable D_800D3A74;

/* The entity class func_800D557C constructs (virtual table D_800D3A24): a sprite at the stage's scroll that comes
 * nearer, grows and spins (func_800D55D4) and adds a func_800D57C4 effect every 16 frames. */
typedef struct Unk800D557C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* x follows the stage's scrollX, z from 0x4000 down to 0x2000 */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 scale;     /* up to 0x480, 0x18 a frame (func_800D5690's matrixScale) */
    /* 0x2C */ s32 angle;     /* 6 a frame (func_800D5690's matrixSetRotationZ) */
    /* 0x30 */ s32 frames;
} Unk800D557C;

extern EntityVtable D_800D3A24;

/* The entity class of virtual table D_800D3B5C. */
typedef struct Unk800D58D8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
} Unk800D58D8;

extern EntityVtable D_800D3B5C;

/* The entity class of virtual table D_800D3B2C (func_800D4468 constructs it). */
typedef struct Unk800D4468 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 level; /* 0..0xFF (func_800D4520); drawn while above 0 */
    /* 0x1C */ s16 hue;
    /* 0x1E */ u8 saturation;
    /* 0x1F */ u8 value;
    /* 0x20 */ s32 unk20; /* func_800D4468's second argument */
    /* 0x24 */ s32 frames;
} Unk800D4468;

void func_800D45BC(Unk800D4468 *this, s32 arg);
void func_800D46C4(Unk800D4468 *this, s32 arg);

extern VtableEntry STAGE_BOSS_VTABLE[];

/* An arena collision line as the stage defines it: its ends. */
typedef struct St06LineEnds {
    /* 0x0 */ DVECTOR from;
    /* 0x4 */ DVECTOR to;
} St06LineEnds;

extern St06LineEnds ST06_TERRAIN_LINE_ENDS[1];
extern TerrainLine ST06_TERRAIN_LINES[1];

/* Constructs the stage: Fight, with virtual table STAGE_BOSS_VTABLE, and its one collision line (the floor, from
 * x -0x7000 to 0x7000), built from ST06_TERRAIN_LINE_ENDS and linked into COLLISION_LINKS. */
StageBoss *StageBoss_init(StageBoss *this) {
    s32 i;

    Fight_init(&this->base);
    this->base.vtable = STAGE_BOSS_VTABLE;
    this->collisionLink.link.next = &this->collisionLink.link;
    this->collisionLink.link.prev = &this->collisionLink.link;
    for (i = 0; i < 1; i++) {
        initTerrainLine(&ST06_TERRAIN_LINES[i], &ST06_TERRAIN_LINE_ENDS[i].from, &ST06_TERRAIN_LINE_ENDS[i].to);
    }
    /* MeshSceneLink.lines is typed by the engine's own lines; the stage's are TerrainLines */
    addCollisionLink(&this->collisionLink, (s32 *)ST06_TERRAIN_LINES, 1);
    this->rounds = 0;
    this->sound = 0;
    return this;
}

/* The stage's destructor: stops its sound, unlinks its collision link from COLLISION_LINKS, then Fight's
 * destructor. */
void StageBoss_destroy(StageBoss *this, s32 flags) {
    ListNode *node;

    this->base.vtable = STAGE_BOSS_VTABLE;
    stopSound(this->sound);
    /* listRemove(&this->collisionLink.link), in the access order of StageShrine_destroy's */
    node = &this->collisionLink.link;
    this->collisionLink.link.next->prev = this->collisionLink.link.prev;
    node->prev->next = this->collisionLink.link.next;
    this->collisionLink.link.next = node;
    this->collisionLink.link.prev = node;
    Fight_destroy(&this->base, flags);
}

/* the spawn points of the stage's item spawner (createItemSpawner), in its .rodata */
extern u8 ST06_ITEM_SPAWN_POINTS[];

/* The stage's sounds when its first round starts. */
#define ST06_SOUND_START 0x6A
#define ST06_SOUND_LOOP 0x6B

Unk800D4468 *func_800D4468(Unk800D4468 *this, s32 arg);
Unk800D557C *func_800D557C(Unk800D557C *this);
Unk800D5040 *func_800D5040(Unk800D5040 *this);

/* The stage's round start: Fight_startRound, the node entities, its bounds and scroll, a func_800D4468 entity kept as
 * levels[0], then by round: the first plays ST06_SOUND_START and ST06_SOUND_LOOP (kept in sound) with every level 0;
 * the second gives levels[1] 0xFF; later ones give levels[0] 0x6E and add a func_800D557C sprite. Then it passes every
 * level on and makes the item spawner. */
void StageBoss_startRound(StageBoss *this) {
    Entity *entity;
    s32 i;

    Fight_startRound(&this->base);
    Fight_createNodeEntities(&this->base);
    this->base.hasBackground = 0;
    Fight_setBounds(&this->base, -0xA00, 0xA00, -0x4000, 0x400, 0);
    this->scrollX = 0;
    this->frames = 0;
    this->freezeFrame = 1;
    entity = &func_800D4468(entityOperatorNew(sizeof(Unk800D4468)), this->rounds)->entity;
    Fight_addStageEntity(&this->base, entity);
    this->levels[0].handle = entityGetHandle(entity);
    switch (++this->rounds) {
    case 1:
        this->levels[0].level = 0;
        this->levels[1].level = 0;
        this->levels[2].level = 0;
        this->levels[3].level = 0;
        soundControlPlaySound(&SOUND_CONTROL, ST06_SOUND_START, 0);
        this->sound = soundControlPlaySound(&SOUND_CONTROL, ST06_SOUND_LOOP, 0);
        break;
    case 2:
        st06_setLevel(this, 0, 0);
        this->levels[1].level = 0xFF;
        this->levels[2].level = 0;
        this->levels[3].level = 0;
        break;
    default:
        st06_setLevel(this, 0, 0x6E);
        this->levels[1].level = 0;
        this->levels[2].level = 0;
        Fight_addStageEntity(&this->base, NEW_ENTITY(sizeof(Unk800D557C), func_800D557C));
        break;
    }
    for (i = 0; i < 4; i++) {
        st06_applyLevel(&this->levels[i]);
    }
    createItemSpawner(&this->base, ST06_ITEM_SPAWN_POINTS, 3);
}

/* The entity class func_800D4958 constructs (virtual table D_800D3AFC): 18 rings of 12 points each, x from -0x2000 by
 * 0x3C3, z from -0x5D1 by 0x5D1, whose colors the tick sets from a hue that turns. */
typedef struct St06RingPoint {
    /* 0x0 */ Vec3 position;
    /* 0xC */ u32 color; /* a CVECTOR as a word */
} St06RingPoint;

typedef struct Unk800D4958 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 level; /* func_800D5914 sets it; the tick's value is half of it */
    /* 0x1C */ HsvColor hsv;
    /* 0x20 */ s32 scrollX; /* the stage's, each tick */
    /* 0x24 */ St06RingPoint points[18][12];
} Unk800D4958;

/* The four St06Spinners of the meshes of ST06_SPIN_NODES, by handle (St06SpinGroup_init, virtual table
 * ST06_SPIN_GROUP_VTABLE, 0x2C bytes). */
typedef struct St06SpinGroup {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 level; /* St06SpinGroup_setLevel's */
    /* 0x1C */ u32 children[4];
} St06SpinGroup;

INCLUDE_ASM("st06/nonmatchings/st06", StageBoss_createNodeEntity);

/* x kept within low to high. StageBoss_tick's match depends on this single expression on a local (an if/else chain,
 * or a clamp of the position field itself, allocates otherwise). */
#define CLAMP(x, low, high) ((x) < (low) ? (low) : (x) > (high) ? (high) : (x))

/* The stage's scenery tick: Fight_tickScenery, a frozen frame each frame and, while the group at 0x5C is empty, the
 * levels by round and frame: the first round raises levels 2 and 1 and gives level 0 a falling cosine; the second
 * fades level 1 out over 0x100 frames, then sets level 0 to 0xA0 - frames clamped to 0x78..0xA0 (always 0x78 by
 * then), adds a func_800D5040 entity at frame 0x100 and a func_800D557C sprite at 0x180. */
void StageBoss_tickScenery(StageBoss *this) {
    Fight_tickScenery(&this->base);
    this->freezeFrame = 1;
    if (func_8006F104(&this->base)) {
        return;
    }
    switch (this->rounds) {
    case 1:
        st06_setLevel(this, 2, this->frames * 4);
        st06_setLevel(this, 1, this->frames);
        st06_setLevel(this, 0, (0x1000 - COS_SIN_TABLE[(this->frames * 4) & 0xFFF].sin) >> 4);
        break;
    case 2:
        if (this->frames >= 0x100) {
            st06_setLevel(this, 0, CLAMP(0xA0 - this->frames, 0x78, 0xA0));
        } else {
            st06_setLevel(this, 1, 0xFF - this->frames);
        }
        if (this->frames == 0x100) {
            Fight_addStageEntity(&this->base, NEW_ENTITY(sizeof(Unk800D5040), func_800D5040));
        }
        if (this->frames == 0x180) {
            Fight_addStageEntity(&this->base, NEW_ENTITY(sizeof(Unk800D557C), func_800D557C));
        }
        break;
    }
    this->frames++;
}

/* The stage's tick: after Fight_tick, unless the continue screen is up, keeps both fighters within x +-0x6800,
 * centres the arena's bounds (and scrollX) between them and, when they stand more than 0x1000 apart, puts them
 * 0x800 each side of that centre. */
void StageBoss_tick(StageBoss *this) {
    Fighter *fighter0;
    Fighter *fighter1;
    Vec3 position0;
    Vec3 position1;
    s32 x0;
    s32 x1;
    s32 center;

    Fight_tick(&this->base);
    if (Fight_isContinueShown(&this->base)) {
        return;
    }
    fighter0 = Fight_getFighter(&this->base, 0);
    fighter1 = Fight_getFighter(&this->base, 1);
    Fighter_getPosition(fighter0, &position0);
    Fighter_getPosition(fighter1, &position1);
    x0 = position0.vx;
    x1 = position1.vx;
    x0 = CLAMP(x0, -0x6800, 0x6800);
    x1 = CLAMP(x1, -0x6800, 0x6800);
    center = (x0 + x1) >> 1;
    Fight_setBounds(&this->base, center - 0xA00, center + 0xA00, -0x4000, 0x400, 0);
    this->scrollX = center;
    if (abs(x1 - x0) > 0x1000) { /* abs, not ABS: the match takes the builtin's bgez */
        if (x0 < x1) {
            x0 = center - 0x800;
            x1 = center + 0x800;
        } else {
            x0 = center + 0x800;
            x1 = center - 0x800;
        }
    }
    position0.vx = x0;
    position1.vx = x1;
    Fighter_setPosition(fighter0, &position0);
    Fighter_setPosition(fighter1, &position1);
}

/* The stage's override of Fight_drawWorld: after it, when freezeFrame is set, clears it and queues a copy of the
 * shown buffer to x 0x200 in the camera's first-drawn slot. */
void StageBoss_drawWorld(StageBoss *this) {
    u_long *ot;

    Fight_drawWorld(this);
    ot = Fight_getCamera(&this->base)->ot.firstDrawn;
    if (this->freezeFrame != 0) {
        this->freezeFrame = 0;
        AddPrim(ot, allocDrMove(getShownBufferRect(), 0x200, 0));
    }
}

/* The virtual table of the entities in StageBoss.levels: an entity's, then the slot that takes the level. */
typedef struct St06LevelVtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry setLevel; /* (Entity *, s32 level) */
} St06LevelVtable;

/* Clamps *level to 0..0xFF. The match depends on the pointer (the caller reads the level again) and on the
 * value read again inside the first branch. */
static inline void clampLevel(s32 *level) {
    s32 value;

    if (*level >= 0) {
        value = *level;
        if (value > 0xFF) {
            value = 0xFF;
        }
    } else {
        value = 0;
    }
    *level = value;
}

/* Passes the entry's level, clamped to 0..0xFF, to its entity, or forgets the handle once the entity is gone. */
void st06_applyLevel(St06LevelEntry *entry) {
    Entity *entity = getEntityByHandle(entry->handle);
    St06LevelVtable *vtable;

    if (entity != NULL) {
        clampLevel(&entry->level);
        vtable = (St06LevelVtable *)entity->vtable;
        vtable->setLevel.func((u8 *)entity + vtable->setLevel.delta, entry->level);
    } else {
        entry->handle = 0;
    }
}

/* Sets the level of the stage's entity number index (a negative level keeps the last one) and passes it on. */
void st06_setLevel(StageBoss *this, s32 index, s32 level) {
    St06LevelEntry *entry = &this->levels[index];

    if (level >= 0) {
        entry->level = level;
    }
    st06_applyLevel(entry);
}

extern EntityVtable D_800D3B2C;

/* Constructs the entity of virtual table D_800D3B2C with arg at 0x20, the rest cleared. */
Unk800D4468 *func_800D4468(Unk800D4468 *this, s32 arg) {
    entityInit(&this->entity, 0);
    /* The match depends on clearing 0x18 before installing the virtual table. */
    this->level = 0;
    this->entity.vtable = &D_800D3B2C;
    this->hue = 0;
    this->saturation = 0;
    this->value = 0;
    this->unk20 = arg;
    this->frames = 0;
    return this;
}

/* engine getCos, inlined.
 * The match depends on this s16 inline: a table read in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* Unk800D4468's tick: the hue climbs by 8 when 0x20 is 0, else follows a cosine of the frame count. */
void func_800D44C4(Unk800D4468 *this) {
    if (this->unk20 == 0) {
        this->hue += 8;
    } else {
        this->hue = -0x1C0 - (getCosInline(this->frames << 3) >> 5);
    }
    this->frames++;
}

/* Unk800D4468's own method (0x20 of its virtual table): clamps the level to 0..0xFF and keeps it; when 0x20 is 0,
 * the saturation gets half of it and the value the complement of three quarters, else both get it. */
void func_800D4520(Unk800D4468 *this, s32 level) {
    if (level >= 0) {
        if (level >= 0x100) {
            level = 0xFF;
        }
    } else {
        level = 0;
    }
    this->level = level;
    if (this->unk20 == 0) {
        this->saturation = level >> 1;
        this->value = ~((level * 3 << 6) >> 8);
    } else {
        this->saturation = level;
        this->value = level;
    }
}

/* Unk800D4468's draw: func_800D45BC when 0x20 is 0, func_800D46C4 otherwise. */
void func_800D457C(Unk800D4468 *this, s32 arg) {
    if (this->unk20 == 0) {
        func_800D45BC(this, arg);
    } else {
        func_800D46C4(this, arg);
    }
}

/* The frozen frame at x 0x200, from its second line (StageBoss_drawWorld copies the shown buffer there). */
extern const RECT ST06_FROZEN_FRAME_RECT;

/* Unk800D4468's draw when 0x20 is 0: while it has a level, copies the frozen frame one line up into the buffer being
 * drawn, then subtracts its color from the whole screen. */
void func_800D45BC(Unk800D4468 *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    CVECTOR rgb;
    RECT *shown;
    RECT frozen;
    u_long *ot;
    s32 y;
    DR_MOVE *move;
    DR_TPAGE *tpage;
    TileWords *tile;

    if (this->level > 0) {
        ot = camera->ot.firstDrawn;
        convertHsvToRgb(&rgb, this->hue, this->saturation, this->value);
        shown = getShownBufferRect();
        frozen = ST06_FROZEN_FRAME_RECT;
        y = 0;
        if (shown->y == 0) {
            y = 0xF0;
        }
        move = allocDrMove(&frozen, 0, y);
        tpage = allocDrTpage(getTPage(0, 2, 0x140, 0));
        tile = (TileWords *)allocTile();
        rgb.cd = ((TILE *)tile)->code | PRIM_SEMI_TRANS;
        tile->rgbCode = *(u32 *)&rgb; /* the color and code at once: a word on the stack */
        tile->xy = 0;
        tile->wh = 0xF00140;
        AddPrim(ot, tile);
        AddPrim(ot, tpage);
        AddPrim(ot, move);
    }
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D46C4);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4958);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4A00);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D4B78);

/* One mesh of the stage, scaled and spun about a random axis (St06Spinner_init, virtual table ST06_SPINNER_VTABLE,
 * 0x38 bytes). */
typedef struct St06Spinner {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ VECTOR axis;  /* random, normalized */
    /* 0x2C */ s32 angle;    /* about axis */
    /* 0x30 */ s32 speed;    /* the angle's step, 1 to 8 */
    /* 0x34 */ s32 scale;    /* of the three axes (St06SpinGroup_setLevel) */
} St06Spinner;

extern EntityVtable ST06_SPIN_GROUP_VTABLE;
extern EntityVtable ST06_SPINNER_VTABLE;

/* the nodes of the four meshes St06SpinGroup_init spins: 0 to 3 */
extern s32 ST06_SPIN_NODES[4];

St06Spinner *St06Spinner_init(St06Spinner *this, Mesh *mesh);

/* Constructs an St06SpinGroup: an St06Spinner for each of the four meshes of
 * ST06_SPIN_NODES in scene, each put into the stage's entity group. */
St06SpinGroup *St06SpinGroup_init(St06SpinGroup *this, Fight *stage, MeshScene *scene) {
    s32 i;

    entityInit(&this->entity, 0);
    this->level = 0;
    this->entity.vtable = &ST06_SPIN_GROUP_VTABLE;
    for (i = 0; i < 4; i++) {
        Mesh *mesh = meshSceneGetMesh(scene, ST06_SPIN_NODES[i]);
        St06Spinner *child = St06Spinner_init(entityOperatorNew(sizeof(St06Spinner)), mesh);

        this->children[i] = entityGetHandle(&child->entity);
        Fight_addStageEntity(stage, &child->entity);
    }
    return this;
}

/* St06SpinGroup's setLevel (0x20 of its virtual table): keeps the level and scales the four meshes by 40 times
 * it. */
void St06SpinGroup_setLevel(St06SpinGroup *this, s32 level) {
    s32 scale = level * 40;
    s32 i;

    this->level = level;
    for (i = 0; i < 4; i++) {
        ((St06Spinner *)getEntityByHandle(this->children[i]))->scale = scale;
    }
}

/* Constructs an St06Spinner for mesh: a random axis, a random angle and a random
 * step of 1 to 8. */
St06Spinner *St06Spinner_init(St06Spinner *this, Mesh *mesh) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST06_SPINNER_VTABLE;
    this->mesh = mesh;
    x = mersenneTwisterGenerate(&RANDOM);
    y = mersenneTwisterGenerate(&RANDOM);
    z = mersenneTwisterGenerate(&RANDOM);
    this->axis.vx = x;
    this->axis.vy = y;
    this->axis.vz = z;
    vecNormalize(&this->axis, &this->axis);
    this->angle = mersenneTwisterGenerate(&RANDOM) & 0xFFF;
    this->speed = (s32)mersenneTwisterGenerate(&RANDOM) % 8;
    if (this->speed == 0) {
        this->speed = 8;
    }
    return this;
}

/* Sets the mesh's matrix to no rotation at (x, y, z), out of date. */
static inline void meshSetPosition(Mesh *mesh, s32 x, s32 y, s32 z) {
    mesh->object.transform.valid = 0;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x;
    mesh->object.transform.local.m.t[1] = y;
    mesh->object.transform.local.m.t[2] = z;
}

/* Sets the rotation part of the mesh's matrix to a scale of the three axes (element pairs written as words). */
static inline void meshSetScale(Mesh *mesh, u16 scale) {
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[0] = scale;
    mesh->object.transform.local.words[2] = scale;
    mesh->object.transform.local.words[4] = scale;
}

/* St06Spinner's tick: steps the angle, then sets the mesh's matrix at (the stage's scrollX, 0, 0),
 * scaled by 0x34, turned by the angle about the axis and by twice it about y. The match depends on reading
 * the stage's x before the angle step. */
void St06Spinner_tick(St06Spinner *this, StageBoss *stage) {
    s32 x = stage->scrollX;
    Mesh *mesh;

    this->angle += this->speed;
    mesh = this->mesh;
    meshSetPosition(mesh, x, 0, 0);
    meshSetScale(mesh, this->scale);
    matrixRotateAxis(&mesh->object.transform.local.m, &this->axis, this->angle);
    matrixRotateY(&mesh->object.transform.local.m, this->angle << 1);
}

/* Constructs an entity of the class of virtual table D_800D3A74, 0x24 set to 0x100. */
Unk800D5040 *func_800D5040(Unk800D5040 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A74;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20 = 0;
    this->unk24 = 0x100;
    return this;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D508C);

void func_800D515C(void) {
}

/* Constructs an entity of the class of virtual table D_800D3A4C; func_800D508C
 * makes them, passing 0 and 1 in turn as arg. */
Unk800D5164 *func_800D5164(Unk800D5164 *this, s32 arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A4C;
    this->unk18 = 0;
    this->unk1C = 0x10;
    this->unk20 = arg;
    this->unk24 = 0;
    return this;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D51BC);

INCLUDE_ASM("st06/nonmatchings/st06", func_800D5254);

/* Constructs the sprite at (0, -0x200, 0x4000), unscaled and unturned. */
Unk800D557C *func_800D557C(Unk800D557C *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A24;
    this->position.vx = 0;
    this->position.vy = -0x200;
    this->position.vz = 0x4000;
    this->scale = 0;
    this->angle = 0;
    this->frames = 0;
    return this;
}

Unk800C3CFC *func_800D57C4(Unk800C3CFC *this, s32 unk38, s32 color);

/* The sprite's tick: follows the stage's scrollX, comes nearer (z - 3 down to 0x2000), grows (up to 0x480) and
 * turns, and every 16 frames adds a func_800D57C4 effect beside it. */
void func_800D55D4(Unk800D557C *this, StageBoss *stage) {
    this->position.vx = stage->scrollX;
    if (this->position.vz > 0x2000) {
        this->position.vz -= 3;
    }
    if (this->scale < 0x480) {
        this->scale += 0x18;
    }
    this->angle += 6;
    if ((this->frames & 0xF) == 0) {
        entityAddSibling(&this->entity, NEW_ENTITY(0x54, func_800D57C4, 0x2000, 0x801008));
    }
    this->frames++;
}

INCLUDE_ASM("st06/nonmatchings/st06", func_800D5690);

extern EntityVtable D_800D39FC;

/* Constructs the entity of virtual table D_800D39FC, derived from func_800C3CFC's at the zero vector, with
 * 0x50 a random 0x60 to 0x7F. */
Unk800C3CFC *func_800D57C4(Unk800C3CFC *this, s32 unk38, s32 color) {
    func_800C3CFC(this, (Vec3 *)&ZERO_VECTOR, unk38, color);
    this->entity.vtable = &D_800D39FC;
    this->unk50 = (mersenneTwisterGenerate(&RANDOM) & 0x1F) + 0x60;
    return this;
}

/* The tick of func_800D57C4's entity: func_800C3E64, then puts it at (the stage's scrollX, -0x200,
 * 0x4000). */
void func_800D5824(Unk800C3CFC *this, StageBoss *stage) {
    func_800C3E64(this);
    this->unk28.vx = stage->scrollX;
    this->unk28.vy = -0x200;
    this->unk28.vz = 0x4000;
}

/* Returns the stage's camera eye. */
Vec3 *st06_getEye(StageBoss *this) {
    return &this->base.eye;
}

/* Returns the stage's scrollX (the x its meshes are put at). */
s32 st06_getScrollX(StageBoss *this) {
    return this->scrollX;
}

/* The destructor of the entity class of vtable D_800D3B5C, which func_800D58D8 installs. */
void func_800D5880(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St06SpinGroup's tick: nothing. */
void St06SpinGroup_tick(void) {
}

/* St06SpinGroup's draw: nothing. */
void St06SpinGroup_draw(void) {
}

/* Constructs an entity of the class of virtual table D_800D3B5C. */
Unk800D58D8 *func_800D58D8(Unk800D58D8 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B5C;
    this->unk18 = 0;
    return this;
}

/* The method at 0x20 of the virtual tables D_800D3A74, D_800D3AFC and D_800D3B5C: keeps arg at 0x18. */
void func_800D5914(Unk800D58D8 *this, s32 arg) {
    this->unk18 = arg;
}

/* The destructor of the entity class of vtable D_800D3B2C, which func_800D4468 installs. */
void func_800D591C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AFC, which func_800D4958 installs. */
void func_800D5964(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St06SpinGroup's destructor. */
void St06SpinGroup_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St06Spinner's destructor. */
void St06Spinner_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A74, which func_800D5040 installs. */
void func_800D5A3C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A4C, which func_800D5164 installs. */
void func_800D5A84(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A24, which func_800D557C installs. */
void func_800D5ACC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D39FC, which func_800D57C4 installs. */
void func_800D5B14(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
