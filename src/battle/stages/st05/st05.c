#include "common.h"
#include "strings.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "gte.h"
#include "battle/game_classes.h"
#include "engine/gfx/display.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/prim/alloc_dr_load.h"
#include "engine/gfx/prim/alloc_dr_mode.h"
#include "engine/gfx/prim/alloc_poly_f3.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim/build_poly_gt3.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* Arena 5's stage, "Volcano" in STAGE_NAMES (StageVolcano_init, virtual table STAGE_VOLCANO_VTABLE): a Fight that
 * lights its fighters (StageVolcano_lightFighter), keeps them within its x range and throws shots; embers rise. */
/* A sprite of the stage's backdrop, drawn at its offset from the scenery's screen height (st05_drawBackdropSprites). */
typedef struct St05OffsetSprite {
    /* 0x00 */ Sprite sprite;
    /* 0x18 */ VECTOR offset; /* D_800D38A4's at construction */
} St05OffsetSprite; /* size 0x28 */

/* An St05OffsetSprite with two more vectors (StageVolcano_init: unk28 = offset = D_800D3924's, unk38 D_800D3954's). */
typedef struct St05OffsetSpriteB {
    /* 0x00 */ St05OffsetSprite base;
    /* 0x28 */ VECTOR unk28;
    /* 0x38 */ VECTOR unk38;
} St05OffsetSpriteB; /* size 0x48 */

typedef struct StageVolcano {
    /* 0x000 */ Fight base;
    /* 0xD38 */ MeshSceneLink collisionLink; /* lines of the arena itself, in COLLISION_LINKS (StageVolcano_init) */
    /* 0xD50 */ VECTOR eruptionPoints[2]; /* where the eruption's two func_800D5D54 hits appear (D_800D39B4) */
    /* 0xD70 */ s32 erupting;      /* set at random with sound 0x67 and the red flash; cleared at eruptionTime 0x1000 */
    /* 0xD74 */ s32 eruptionTime;  /* 8 more a frame: the hits at 0x400 and 0xA00 */
    /* 0xD78 */ s32 frames; /* the scenery's frames: 0 at each round, one more per tick (StageVolcano_tickScenery) */
    /* 0xD7C */ s32 riseBoost; /* adds 4 per unit to the embers' rise (St05Ember_tick) */
    /* 0xD80 */ u32 sound; /* sound 0x67's handle (st05_startSound, st05_stopSound) */
    /* 0xD84 */ Sprite unkD84; /* drawn at the scenery's screen height (func_800D4D3C) */
    /* 0xD9C */ Sprite unkD9C; /* drawn 0x1F below unkD84, tinted by glowRed and glowGreen */
    /* 0xDB4 */ u8 glowRed;   /* unkD9C's red and green: they fade (by 4 and 2 a frame) after a random flash */
    /* 0xDB5 */ u8 glowGreen; /* (StageVolcano_tickScenery) */
    /* 0xDB6 */ u8 unkDB6;
    /* 0xDB7 */ u8 unkDB7;
    /* 0xDB8 */ St05OffsetSprite unkDB8[5];
    /* 0xE80 */ St05OffsetSpriteB unkE80[3];
} StageVolcano; /* size 0xF58 (game_main.c) */

void st05_drawScenery(StageVolcano *this);

/* the virtual tables the functions below install */
extern VtableEntry ST05_HIT_VTABLE[];
extern VtableEntry STAGE_VOLCANO_VTABLE[];

void st05_stopSound(StageVolcano *this);

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_init);

/* The stage's destructor: stops its sound, destroys its sprites (each array from the last), unlinks its collision
 * link from COLLISION_LINKS, then Fight's destructor. */
void StageVolcano_destroy(StageVolcano *this, s32 flags) {
    ListNode *node;

    this->base.vtable = STAGE_VOLCANO_VTABLE;
    st05_stopSound(this);
    /* g++'s destructor of a member array, written out: the match depends on its test of the array's address and on
     * the walk from the end, compared with the array each step. */
    if (this->unkE80 != NULL) {
        St05OffsetSpriteB *sprite = this->unkE80 + 3;

        while (this->unkE80 != sprite) {
            sprite--;
            spriteDestroy(&sprite->base.sprite, DESTROY_BASES);
        }
    }
    if (this->unkDB8 != NULL) {
        St05OffsetSprite *sprite = this->unkDB8 + 5;

        while (this->unkDB8 != sprite) {
            sprite--;
            spriteDestroy(&sprite->sprite, DESTROY_BASES);
        }
    }
    spriteDestroy(&this->unkD9C, DESTROY_BASES);
    spriteDestroy(&this->unkD84, DESTROY_BASES);
    /* listRemove(&this->collisionLink.link), in the access order of StageShrine_destroy's */
    node = &this->collisionLink.link;
    this->collisionLink.link.next->prev = this->collisionLink.link.prev;
    node->prev->next = this->collisionLink.link.next;
    this->collisionLink.link.next = node;
    this->collisionLink.link.prev = node;
    Fight_destroy(&this->base, flags);
}

/* The entity class func_800D53E4 constructs (virtual table D_800D3BE4): a 14x10 grid of vertices that the tick
 * waves, the colour tint of each vertex, then the handle of sound 0x66. A tint is a GridVertex too: vx and vy are
 * the vertex's wave offset at a smaller scale (set by the tick), pad its shade; the draw subtracts vx and vy from the
 * red and green and the shade as a grey. */
typedef struct Unk800D53E4 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ GridVertex vertices[14][10];
    /* 0x478 */ GridVertex tints[14][10];
    /* 0x8D8 */ u32 sound; /* soundControlPlaySound's handle */
    /* 0x8DC */ s32 unk8DC;
} Unk800D53E4; /* size 0x8E0 */

/* the background's picture, the stage's lights and item spawn points, in its .rodata */
extern RECT ST05_BACKGROUND_CLUT_RECT; /* the background's CLUT: 256 by 1 at (0, 0x1E0) */
extern RECT ST05_BACKGROUND_IMAGE_RECT; /* the background's picture: 160 by 116 at (0x240, 0) */
extern VECTOR ST05_LIGHT_DIRECTION;
extern VECTOR ST05_LIGHT_COLOR;
extern u8 ST05_ITEM_SPAWN_POINTS[];
extern SVECTOR ST05_SCENERY_POINT; /* projected for the scenery's screen height */

Unk800D53E4 *func_800D53E4(Unk800D53E4 *this);

/* The stage's round start: Fight_startRound, its own state cleared and the sound stopped, the background picture, the
 * lights (directions 1 and 2 from above, colours 1 and 2 orange), Fight_setBounds's bounds, the func_800D53E4 entity
 * and createItemSpawner's entity. */
void StageVolcano_startRound(StageVolcano *this) {
    TIM_IMAGE tim;
    Camera *camera;

    Fight_startRound(&this->base);
    this->eruptionPoints[0].vx = 0;
    this->eruptionPoints[0].vy = 0;
    this->eruptionPoints[0].vz = 0;
    this->eruptionPoints[0].pad = 0;
    this->eruptionPoints[1].vx = 0;
    this->eruptionPoints[1].vy = 0;
    this->eruptionPoints[1].vz = 0;
    this->eruptionPoints[1].pad = 0;
    this->erupting = 0;
    this->eruptionTime = 0;
    this->frames = 0;
    this->riseBoost = 0;
    st05_stopSound(this);
    this->glowRed = 0;
    this->glowGreen = 0;
    this->unkDB6 = 0;
    tim.mode = 9; /* 8-bit, with a CLUT */
    tim.crect = &ST05_BACKGROUND_CLUT_RECT;
    tim.prect = &ST05_BACKGROUND_IMAGE_RECT;
    spriteSetTim(&this->base.background, &tim, 0);
    camera = Fight_getCamera(&this->base);
    cameraSetLightDirection1(camera, &ST05_LIGHT_DIRECTION);
    cameraSetLightDirection2(camera, &ST05_LIGHT_DIRECTION);
    cameraSetLightColor1(camera, &ST05_LIGHT_COLOR);
    cameraSetLightColor2(camera, &ST05_LIGHT_COLOR);
    Fight_setBounds(&this->base, -0xF00, 0xFC0, -0x1B00, 0x200, 0);
    Fight_addStageEntity(&this->base, (Entity *)func_800D53E4(entityOperatorNew(sizeof(Unk800D53E4))));
    createItemSpawner(&this->base, ST05_ITEM_SPAWN_POINTS, 3);
}

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_tickScenery);

/* the stage's steps for one fighter */
void st05_clampFighterX(StageVolcano *this, Fighter *fighter);
void st05_burnFighter(StageVolcano *this, Fighter *fighter);

/* The stage's override of Fight_tick: the base tick, then for each side st05_clampFighterX and, unless
 * func_8006F144 or Fight_isRoundOver holds, st05_burnFighter. */
void StageVolcano_tick(StageVolcano *this) {
    s32 held;
    s32 side;
    Fighter *fighter;

    Fight_tick(&this->base);
    held = 0;
    if (func_8006F144(&this->base)) {
        held = 1;
    } else if (Fight_isRoundOver(&this->base)) {
        held = 1;
    }
    for (side = 0; side < 2; side++) {
        fighter = Fight_getFighter(&this->base, side);
        st05_clampFighterX(this, fighter);
        if (!held) {
            st05_burnFighter(this, fighter);
        }
    }
}

/* Keeps a fighter's x within -0xE00..0xF00 (gcc folds the in-range test into one unsigned compare).
 * The match depends on the clamp testing the low bound first, with the high one inside it. */
void st05_clampFighterX(StageVolcano *this, Fighter *fighter) {
    Vec3 position;
    s32 x;

    Fighter_getPosition(fighter, &position);
    x = position.vx;
    if (x >= -0xE00 && x <= 0xF00) {
        return;
    }
    if (x >= -0xE00) {
        if (x > 0xF00) {
            x = 0xF00;
        }
    } else {
        x = -0xE00;
    }
    position.vx = x;
    Fighter_setPosition(fighter, &position);
}

/* A quad whose size closes toward its target while its alpha falls (St05Fade_init, virtual table
 * ST05_FADE_VTABLE). */
typedef struct St05Fade {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 halfWidth; /* the quad spans -halfWidth to halfWidth in x */
    /* 0x2C */ s32 height;    /* and -height to height / 4 in y */
    /* 0x30 */ s32 targetHalfWidth;
    /* 0x34 */ s32 targetHeight;
    /* 0x38 */ s32 brightness; /* 0xFF at construction, 0x18 less each tick */
} St05Fade;

St05Fade *St05Fade_init(St05Fade *this, Vec3 *position, s32 targetHalfWidth, s32 targetHeight);
s32 func_800D4768(Fighter *fighter);

/* The lava: a fighter below y 0x80 is burnt. Every fourth frame an St05Fade rises at his x (z -0x80), sized by his
 * height; his velocity is pushed back (an eighth in x, half in y), his side takes func_800D4768's damage, he is held
 * in clip 0xD and, unless his last y was below 0x8000, takes a hit. */
void st05_burnFighter(StageVolcano *this, Fighter *fighter) {
    Vec3 position;
    Vec3 push;
    s32 height;

    Fighter_getPosition(fighter, &position);
    if (position.vy <= 0x80) {
        return;
    }
    if ((this->frames & 3) == 0) {
        height = Fighter_getParams(fighter)->height;
        position.vz = -0x80;
        Fight_addEntity(&this->base, NEW_ENTITY(sizeof(St05Fade), St05Fade_init, &position, height * 3, height * 5));
    }
    push.vx = -fighter->velocity.vx >> 3;
    push.vy = -fighter->velocity.vy >> 1;
    push.vz = 0;
    BattleSide_takeDamage(func_8007758C(fighter), func_800D4768(fighter));
    /* The match depends on adding the push as vecAddInPlace does: the three force words read first. */
    {
        s32 x = fighter->force.vx;
        s32 y = fighter->force.vy;
        s32 z = fighter->force.vz;

        x += push.vx;
        y += push.vy;
        z += push.vz;
        fighter->force.vx = x;
        fighter->force.vy = y;
        fighter->force.vz = z;
    }
    Fighter_hold(fighter, 1);
    Fighter_setClip(fighter, 0xD, 0, 0);
    if ((fighter->lastPosition.vy >> 8) <= 0x80) {
        Fighter_takeHit(fighter, 0x1000000, (Vec3 *)&ZERO_VECTOR, 0);
    }
}

/* A value by bits 8-9 of 0x0 of the character's table (Fighter_getParams): 0xC0, 0x40, 0x80 or 0x100. */
s32 func_800D4768(Fighter *fighter) {
    switch (Fighter_getParams(fighter)->flags & 0x300) {
    case 0x000:
        return 0xC0;
    case 0x100:
        return 0x40;
    case 0x200:
        return 0x80;
    case 0x300:
        return 0x100;
    }
    return 0xC0;
}

/* The stage's override of Fight_isFallen: whether a
 * fighter's position (Fight_catchFalls passes it) has a y greater than 0x400. */
s32 StageVolcano_isFallen(StageVolcano *this, Vec3 *position) {
    return position->vy > 0x400;
}

/* The stage's override of Fight_drawWorld: the base's, then the stage's own part (st05_drawScenery). */
void StageVolcano_drawWorld(void *this) {
    Fight_drawWorld(this);
    st05_drawScenery(this);
}

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_lightFighter);

/* The stage's override of Fight_updateCamera: the base update, then keeps the eye at y -0x300 or above it (lower
 * y), copying it into the LookAt when it moves it. The match depends on the eye read into locals before the copy
 * (the LookAt's stores could alias the stage, so a direct copy interleaves the loads). */
void StageVolcano_updateCamera(StageVolcano *this) {
    LookAt *lookAt;
    s32 x;
    s32 y;
    s32 z;

    Fight_updateCamera(&this->base);
    if (this->base.eye.vy > -0x300) {
        this->base.eye.vy = -0x300;
        lookAt = Fight_getLookAt(&this->base);
        lookAt->dirty = 1;
        x = this->base.eye.vx;
        y = this->base.eye.vy;
        z = this->base.eye.vz;
        lookAt->eye.vx = x;
        lookAt->eye.vy = y;
        lookAt->eye.vz = z;
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4A74);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4B10);

void func_800D4D3C(StageVolcano *this, u_long *ot, s32 y);
void func_800D4E04(StageVolcano *this, u_long *ot, s32 y);
void st05_loadPulsingCluts(StageVolcano *this);
void st05_drawBackdropSprites(StageVolcano *this, u_long *ot, s32 y);
void st05_drawRedGlow(StageVolcano *this, u_long *ot, s32 y);

/* The stage's own part of the world's draw: the scenery's screen height is ST05_SCENERY_POINT's projection, and 0xA0
 * above it func_800D4D3C's backdrop, func_800D4E04, the pulsing CLUTs and the backdrop sprites are drawn into the
 * camera's first-drawn slot, over a dark orange band from y 0x74 to the screen's bottom; then the red glow, at the
 * scenery's height, into its last-drawn slot. */
void st05_drawScenery(StageVolcano *this) {
    Camera *camera;
    u_long *ot;
    long screen;
    long depth;
    long flag;
    s16 sceneryY;
    s16 backdropY;
    POLY_G4 *band;

    camera = Fight_getCamera(&this->base);
    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.firstDrawn;
    RotTransPers(&ST05_SCENERY_POINT, &screen, &depth, &flag);
    /* The match depends on the projection kept as RotTransPers' long and read through a DVECTOR (lhu, then the sign
     * extension). */
    sceneryY = ((DVECTOR *)&screen)->vy;
    backdropY = sceneryY - 0xA0;
    func_800D4D3C(this, ot, backdropY);
    func_800D4E04(this, ot, backdropY);
    st05_loadPulsingCluts(this);
    st05_drawBackdropSprites(this, ot, backdropY);
    band = allocPolyG4();
    setRGB0(band, 0x40, 0x28, 0x20);
    setRGB1(band, 0x40, 0x28, 0x28);
    setRGB2(band, 0, 0, 0);
    setRGB3(band, 0, 0, 0);
    setXY4(band, 0, 0x74, SCREEN_WIDTH, 0x74, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
    AddPrim(ot, band);
    st05_drawRedGlow(this, camera->ot.lastDrawn, sceneryY);
}

/* Draws the backdrop at the scenery's screen height y: unkD9C (tinted by the glow) 0x1F below unkD84, then a dark red
 * tile from 0x7B below y down to the bottom of the screen. */
void func_800D4D3C(StageVolcano *this, u_long *ot, s32 y) {
    TILE *tile;
    s32 top;

    this->unkD9C.y = y + 0x1F;
    this->unkD9C.color = this->glowRed | (this->glowGreen << 8);
    this->unkD84.y = y;
    PRIM_BUFFER_FREE = spriteDraw(&this->unkD84, spriteDraw(&this->unkD9C, PRIM_BUFFER_FREE, ot), ot);
    top = y + 0x7B;
    tile = allocTile();
    setRGB0(tile, 8, 0, 0);
    setXY0(tile, 0, top);
    setWH(tile, 0x140, 0x100 - top);
    AddPrim(ot, tile);
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4E04);

void func_800D4FFC(void) {
}

/* the CLUT rectangles st05_loadPulsingCluts loads: each takes the last `w` of its three pulsing colours */
extern RECT ST05_PULSING_CLUT_RECTS[5];

/* Loads three pulsing colours into the CLUT rectangles ST05_PULSING_CLUT_RECTS: levels 1 to 31 of the sine of
 * frames << 6 at thirds of a turn apart, as cyan, yellow and red, each rectangle taking the last `w` of them. The
 * match depends on the end pointer read into a local, as in St03PaletteLoad_draw. */
void st05_loadPulsingCluts(StageVolcano *this) {
    u_long *ot = Fight_getCamera(&this->base)->ot.firstDrawn;
    u16 colors[3];
    s32 levels[3];
    s32 angle;
    s32 i;
    RECT *rect;
    RECT *end;

    memset(colors, 0, sizeof(colors));
    angle = this->frames << 6;
    for (i = 0; i < 3; i++) {
        levels[i] = ((COS_SIN_TABLE[angle & 0xFFF].sin * 15) >> 12) + 16;
        angle -= 0x555;
    }
    colors[0] = (levels[0] << 5) | (levels[0] << 10);
    colors[1] = levels[1] | (levels[1] << 5);
    colors[2] = levels[2];
    rect = ST05_PULSING_CLUT_RECTS;
    end = rect + 5;
    do {
        DR_LOAD *prim = allocDrLoad(rect);
        u16 *dst = (u16 *)prim->p;

        for (i = 3 - rect->w; i < 3; i++) {
            *dst++ = colors[i];
        }
        AddPrim(ot, prim);
        rect++;
    } while (rect != end);
}

/* Draws the backdrop's sprites at the scenery's screen height y: each one at its offset, unkE80's then unkDB8's. */
void st05_drawBackdropSprites(StageVolcano *this, u_long *ot, s32 y) {
    u8 *prims = PRIM_BUFFER_FREE;
    St05OffsetSprite *sprite;
    VECTOR offset;
    s32 i;

    for (i = 0; i < 3; i++) {
        sprite = &this->unkE80[i].base;
        offset = sprite->offset;
        sprite->sprite.x = offset.vx;
        sprite->sprite.y = offset.vy + y;
        prims = spriteDraw(&sprite->sprite, prims, ot);
    }
    for (i = 0; i < 5; i++) {
        sprite = &this->unkDB8[i];
        offset = sprite->offset;
        sprite->sprite.x = offset.vx;
        sprite->sprite.y = offset.vy + y;
        prims = spriteDraw(&sprite->sprite, prims, ot);
    }
    PRIM_BUFFER_FREE = prims;
}

/* Adds a red glow over the bottom of the screen: a quad, black at its top and red at its bottom, added to the screen
 * (semi-transparent, abr 1), its top 0x28 above the scenery's screen height y, bobbing with the scenery's frames,
 * down to the screen's bottom and at least 0xB4 high. */
void st05_drawRedGlow(StageVolcano *this, u_long *ot, s32 y) {
    DR_MODE *mode;
    POLY_G4 *quad;
    s32 top;
    s32 height;

    top = y - (COS_SIN_TABLE[(this->frames << 6) & 0xFFF].sin >> 8) - 0x28;
    height = SCREEN_HEIGHT - top;
    if (height < 0xB4) {
        height = 0xB4;
    }
    mode = allocDrMode(getTPage(0, 1, 512, 0), NULL);
    quad = allocPolyG4();
    setSemiTrans(quad, 1);
    setRGB0(quad, 0, 0, 0);
    setRGB1(quad, 0, 0, 0);
    setRGB2(quad, 0x80, 0, 0);
    setRGB3(quad, 0x80, 0, 0);
    setXY4(quad, 0, top, SCREEN_WIDTH, top, 0, top + height, SCREEN_WIDTH, top + height);
    AddPrim(ot, quad);
    AddPrim(ot, mode);
}

/* Starts sound 0x67 into the stage's sound, unless one is playing there. */
void st05_startSound(StageVolcano *this) {
    if (this->sound == 0) {
        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x67, 0);
    }
}

/* Stops the stage's sound, if there is one. */
void st05_stopSound(StageVolcano *this) {
    if (this->sound != 0) {
        stopSound(this->sound);
        this->sound = 0;
    }
}

extern EntityVtable D_800D3BE4;

/* Constructs an Unk800D53E4: the inner rows shade from 200 at column 0 down to 0 at column 9, the first and last
 * rows unshaded; then starts sound 0x66. */
Unk800D53E4 *func_800D53E4(Unk800D53E4 *this) {
    s32 row;
    s32 column;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BE4;
    for (row = 1; row < 13; row++) {
        for (column = 0; column < 10; column++) {
            this->tints[row][column].pad = (0x708 - column * 0xC8) / 9; /* the shade */
        }
    }
    for (column = 0; column < 10; column++) {
        this->tints[0][column].pad = 0;
        this->tints[13][column].pad = 0;
    }
    this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x66, 0);
    this->unk8DC = 0;
    return this;
}

/* The destructor of func_800D53E4's class: stops its sound. */
void func_800D54B8(Unk800D53E4 *this, s32 flags) {
    this->entity.vtable = &D_800D3BE4;
    stopSound(this->sound);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5518);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5690);

/* An ember: a rising particle that keeps its last four positions, drawn as a line strip (St05Ember_init, virtual
 * table ST05_EMBER_VTABLE). */
typedef struct St05Ember {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR positions[4]; /* a ring; current indexes the newest */
    /* 0x58 */ VECTOR velocity;
    /* 0x68 */ s32 current;
    /* 0x6C */ s32 life; /* 128 to 255 at construction, 2 less per tick */
} St05Ember;

extern EntityVtable ST05_EMBER_VTABLE;

/* Constructs an ember at a random point of the floor (x 0x80..0x117F, z 0x480..0x127F, y 0), every one of its
 * positions there, at rest, with a life of 128 to 255. The match depends on each random value taken whole and
 * then reduced in a statement of its own. */
St05Ember *St05Ember_init(St05Ember *this) {
    MersenneTwister *random;
    Vec3 start;
    s32 x;
    s32 z;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST05_EMBER_VTABLE;
    random = &RANDOM;
    x = mersenneTwisterGenerate(random);
    x %= 0x1100;
    z = mersenneTwisterGenerate(random);
    z %= 0xE00;
    start.vx = x + 0x80;
    start.vy = 0;
    start.vz = z + 0x480;
    for (i = 0; i < 4; i++) {
        copyVector(&this->positions[i], &start);
    }
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocity.pad = 0;
    this->current = 0;
    this->life = (mersenneTwisterGenerate(random) & 0x7F) + 0x80;
    return this;
}

/* St05Ember's tick: the next position is the newest plus the velocity, then the velocity
 * drifts by -3..3 in x and z and its y becomes -(16 + 4 * the stage's riseBoost) minus 0..7; the entity
 * deletes itself when its life runs out. The match depends on the newest position read before the
 * velocity is added, on riseBoost read before the random calls, and on the y's two parts in locals of their
 * own (an inline "- (extra + 0x10)" folds the 0x10 elsewhere). */
void St05Ember_tick(St05Ember *this, StageVolcano *stage) {
    VECTOR *newest;
    VECTOR *position;
    s32 next;
    s32 x;
    s32 y;
    s32 z;
    s32 extra;
    s32 jitter;
    s32 rise;

    newest = &this->positions[this->current];
    next = (this->current + 1) & 3;
    x = newest->vx;
    y = newest->vy;
    z = newest->vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    position = &this->positions[next];
    position->vx = x;
    position->vy = y;
    position->vz = z;
    this->current = next;
    extra = stage->riseBoost * 4;
    this->velocity.vx += (s32)mersenneTwisterGenerate(&RANDOM) % 4;
    jitter = -(mersenneTwisterGenerate(&RANDOM) & 7);
    rise = extra + 0x10;
    this->velocity.vy = jitter - rise;
    this->velocity.vz += (s32)mersenneTwisterGenerate(&RANDOM) % 4;
    this->life -= 2;
    if (this->life <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", St05Ember_draw);

/* The hit of the stage's shots (St05Hit_init): StageHit's layout, but its slot 0x08 (St05Hit_apply) pushes the
 * touched sphere away from center (its knockback gets 32 times its offset from center in x, 16 times in y) and adds
 * the damage. */
typedef struct St05Hit {
    /* 0x0 */ VtableEntry *vtable;
    /* 0x4 */ s32 damage;
    /* 0x8 */ Vec3 center;
} St05Hit;

/* The entity class func_800D5D54 constructs (virtual table D_800D3B94; StageVolcano_tickScenery makes one at 0xD50 and
 * one at 0xD60 of the stage): an St05Hit at a position with its CollisionSphere, which grows and fades. */
typedef struct Unk800D5D54 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ St05Hit hit; /* centred on the position */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ CollisionSphere sphere; /* in the stage's shapes (Fight_addStageShape) */
    /* 0x78 */ Vec3 position;
    /* 0x84 */ u8 unk84[0x88 - 0x84];
    /* 0x88 */ s32 size;   /* grows towards 0x800, then follows a cosine; the sphere leaves its list under 0x200 */
    /* 0x8C */ s32 state;  /* 0 growing, 1 waiting 120 frames, 2 shrinking */
    /* 0x90 */ s32 frames; /* in the state */
} Unk800D5D54; /* size 0x94 */

extern EntityVtable D_800D3B94;

/* Unk800D5D54's constructor: an St05Hit of damage 0x41001000 centred on the position, its sphere added to the stage's
 * shapes, and sound 0x69. */
Unk800D5D54 *func_800D5D54(Unk800D5D54 *this, Fight *stage, Vec3 *position) {
    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3B94;
    this->hit.vtable = ST05_HIT_VTABLE;
    this->hit.damage = 0x41001000;
    CollisionSphere_init(&this->sphere, position, 0, &this->hit);
    Fight_addStageShape(stage, &this->sphere.link);
    /* The match depends on reading each copy's three words before storing any (as vecCopy does), each copy in its
     * own block. */
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->hit.center.vx = x;
        this->hit.center.vy = y;
        this->hit.center.vz = z;
    }
    this->sphere.entity = &this->entity;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    this->size = 0;
    this->state = 0;
    this->frames = 0;
    soundControlPlaySound(&SOUND_CONTROL, 0x69, 0);
    return this;
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5E3C);

INCLUDE_ASM("st05/nonmatchings/st05", St05Hit_apply);

extern EntityVtable ST05_FADE_VTABLE;

/* Constructs an St05Fade at position, growing toward targetHalfWidth and targetHeight; brightness 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
St05Fade *St05Fade_init(St05Fade *this, Vec3 *position, s32 targetHalfWidth, s32 targetHeight) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST05_FADE_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->halfWidth = 0;
    this->height = 0;
    this->targetHalfWidth = targetHalfWidth;
    this->targetHeight = targetHeight;
    this->brightness = 0xFF;
    return this;
}

/* St05Fade's tick: halfWidth and height close a quarter of their distance to their targets, and brightness falls
 * by 0x18; the entity deletes itself when it runs out. */
void St05Fade_tick(St05Fade *this) {
    this->halfWidth += (this->targetHalfWidth - this->halfWidth) >> 2;
    this->height += (this->targetHeight - this->height) >> 2;
    this->brightness -= 0x18;
    if (this->brightness <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", St05Fade_draw);

/* A Gouraud triangle that grows while it fades (St05Shard_init, virtual table ST05_SHARD_VTABLE): its top edge at
 * height above position, turned about z by angle. */
typedef struct St05Shard {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 height;
    /* 0x2C */ s32 growth;     /* height's step, losing an eighth of itself each tick */
    /* 0x30 */ s32 angle;      /* about z */
    /* 0x34 */ s32 brightness; /* of the bottom corner: 0xFF, 8 less each tick; deleted at 0 */
} St05Shard;

extern EntityVtable ST05_SHARD_VTABLE;

/* Constructs an St05Shard at position, turned by angle, growing by growth; brightness 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
St05Shard *St05Shard_init(St05Shard *this, Vec3 *position, s32 angle, s32 growth) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST05_SHARD_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->height = 0;
    this->growth = growth;
    this->angle = angle;
    this->brightness = 0xFF;
    return this;
}

/* St05Shard's tick: height grows by growth, which loses an eighth of itself, and brightness falls by 8; the
 * entity deletes itself when brightness runs out. */
void St05Shard_tick(St05Shard *this) {
    this->height += this->growth;
    this->growth += -this->growth >> 3;
    this->brightness -= 8;
    if (this->brightness <= 0) {
        entityDelete(&this->entity);
    }
}

/* St05Shard's draw: turned about z by angle at position, a triangle with its top edge (x -0x40..0x40) at height and
 * its bottom point a quarter of the way up, black at the top and grey by brightness at the bottom, additive, with the
 * picture (0xD0, 0)-(0xDF, 0x1F) of the 4-bit page at x 512 and the CLUT at (512, 254). */
void St05Shard_draw(St05Shard *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    u_long *ot = camera->ot.slots;
    s32 zShift = camera->zShift;
    MATRIX m;
    SVECTOR corners[3];
    PolyGT3Shape shape;
    Vec3 *position;
    s32 x, y, z;
    s16 height;
    s16 top;
    u32 shade;

    matrixInitRotationZ(&m, this->angle);
    position = &this->position;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    m.t[0] = x;
    m.t[1] = y;
    m.t[2] = z;
    setModelMatrix(&m);
    /* The match depends on height read into an s16 (the game's lh) and top kept in one (its sll/sra), and below on
     * the chained zero stores (colors[1] and uv1's v stored first). */
    height = this->height;
    top = -height;
    corners[0].vx = -0x40;
    corners[0].vy = top;
    corners[0].vz = 0;
    corners[1].vx = 0x40;
    corners[1].vy = top;
    corners[1].vz = 0;
    corners[2].vx = 0;
    corners[2].vy = -height >> 2;
    corners[2].vz = 0;
    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.colors[0] = shape.colors[1] = 0;
    UV_BYTES(shape.uv0)[0] = 0xD0;
    UV_BYTES(shape.uv1)[0] = 0xDF;
    UV_BYTES(shape.uv2)[0] = 0xD7;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0;
    UV_BYTES(shape.uv2)[1] = 0x1F;
    shape.clut = getClut(512, 254);
    shape.tpage = getTPage(0, 1, 512, 0);
    shade = this->brightness;
    shape.colors[2] = shade | (shade << 8) | (shade << 16);
    shape.flags = SPRITE_SEMI_TRANS | 1;
    PRIM_BUFFER_FREE = buildPolyGT3(&shape, PRIM_BUFFER_FREE, ot, zShift);
}

/* The entity class of virtual table D_800D3B1C (func_800D6740, 0x48 bytes): a sprite of one of two kinds at random,
 * turned and scaled (func_800D6814, func_800D68A4). */
typedef struct Unk800D6740 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ Vec3 unk28;
    /* 0x34 */ u8 unk34[4];
    /* 0x38 */ s32 kind; /* 0 or 1 at random: the row of D_800D3B0C */
    /* 0x3C */ s16 size; /* four times the row's third byte, at least 0x80 */
    /* 0x3E */ u8 pad3E[2];
    /* 0x40 */ s32 unk40; /* random */
    /* 0x44 */ s32 unk44;
} Unk800D6740;

extern EntityVtable D_800D3B1C;
extern u8 D_800D3B0C[2][3];

/* Constructs an Unk800D6740 at position with unk28 (each read whole before it is stored, as vecCopy does), a random
 * kind and its size, and a random unk40. */
Unk800D6740 *func_800D6740(Unk800D6740 *this, Vec3 *position, Vec3 *unk28) {
    s32 kind;
    s32 size;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B1C;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = unk28->vx;
        s32 y = unk28->vy;
        s32 z = unk28->vz;

        this->unk28.vx = x;
        this->unk28.vy = y;
        this->unk28.vz = z;
    }
    kind = mersenneTwisterGenerate(&RANDOM) & 1;
    size = D_800D3B0C[kind][2] * 4;
    this->kind = kind;
    if (size < 0x80) {
        size = 0x80;
    }
    this->size = size;
    this->unk40 = mersenneTwisterGenerate(&RANDOM);
    this->unk44 = 0;
    return this;
}

INCLUDE_ASM("st05/nonmatchings/st05", func_800D6814);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D68A4);

/* The stage's riseBoost. */
s32 st05_getRiseBoost(StageVolcano *this) {
    return this->riseBoost;
}

/* The destructor of the class of virtual table ST05_HIT_VTABLE: CollisionOwner's, inlined. */
void St05Hit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs a St05Hit with its damage (virtual table ST05_HIT_VTABLE, derived from CollisionOwner). */
St05Hit *St05Hit_init(St05Hit *this, s32 damage) {
    this->vtable = ST05_HIT_VTABLE;
    this->damage = damage;
    return this;
}

/* Sets the hit's center, reading all three components before writing any (as vecCopy does). */
void St05Hit_setCenter(St05Hit *this, Vec3 *center) {
    s32 x, y, z;

    x = center->vx;
    y = center->vy;
    z = center->vz;
    this->center.vx = x;
    this->center.vy = y;
    this->center.vz = z;
}

/* St05Ember's destructor. */
void St05Ember_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800D5D54's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D6AC4(Unk800D5D54 *this, s32 flags) {
    ListNode *link;

    this->sphere.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->sphere.link) written out: the match depends on this
     * access order (soundControlDestroy's), as in st04's func_800D8618 */
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

/* St05Fade's destructor. */
void St05Fade_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St05Shard's destructor. */
void St05Shard_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B1C, which func_800D6740 installs. */
void func_800D6BDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
