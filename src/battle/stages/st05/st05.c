#include "common.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "battle/game_classes.h"
#include "engine/gfx/look_at.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/random.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* Arena 5's stage, "Volcano" in STAGE_NAMES (StageVolcano_init, virtual table STAGE_VOLCANO_VTABLE): a Fight that
 * lights its fighters (StageVolcano_lightFighter), keeps them within its x range and throws shots; embers rise. */
typedef struct StageVolcano {
    /* 0x000 */ Fight base;
    /* 0xD38 */ u8 unkD38[0xD7C - 0xD38];
    /* 0xD7C */ s32 riseBoost; /* adds 4 per unit to the embers' rise (St05Ember_tick) */
    /* 0xD80 */ u32 sound; /* sound 0x67's handle (st05_startSound, st05_stopSound) */
    /* 0xD84 */ Sprite unkD84; /* drawn at the scenery's screen height (func_800D4D3C) */
    /* 0xD9C */ Sprite unkD9C; /* drawn 0x1F below unkD84, tinted by glowRed and glowGreen */
    /* 0xDB4 */ u8 glowRed;   /* unkD9C's red and green: they fade (by 4 and 2 a frame) after a random flash */
    /* 0xDB5 */ u8 glowGreen; /* (StageVolcano_tickScenery) */
    /* 0xDB6 */ u8 unkDB6[0xF58 - 0xDB6];
} StageVolcano; /* size 0xF58 (game_main.c) */

void func_800D4BB4(void *this);

/* the virtual tables the functions below install */
extern VtableEntry ST05_HIT_VTABLE[];

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_init);

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_destroy);

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_startRound);

INCLUDE_ASM("st05/nonmatchings/st05", StageVolcano_tickScenery);

/* the stage's steps for one fighter */
void st05_clampFighterX(StageVolcano *this, Fighter *fighter);
void func_800D45E0(StageVolcano *this, Fighter *fighter);

/* The stage's override of Fight_tick: the base tick, then for each side st05_clampFighterX and, unless
 * func_8006F144 or Fight_isRoundOver holds, func_800D45E0. */
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
            func_800D45E0(this, fighter);
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

INCLUDE_ASM("st05/nonmatchings/st05", func_800D45E0);

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

/* The stage's override of Fight_drawWorld: the base's, then the stage's own part (func_800D4BB4). */
void StageVolcano_drawWorld(void *this) {
    Fight_drawWorld(this);
    func_800D4BB4(this);
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

INCLUDE_ASM("st05/nonmatchings/st05", func_800D4BB4);

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

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5004);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5144);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5254);

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
} Unk800D53E4;

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

INCLUDE_ASM("st05/nonmatchings/st05", St05Ember_init);

/* An ember: a rising particle that keeps its last four positions, drawn as a line strip (St05Ember_init, virtual
 * table ST05_EMBER_VTABLE). */
typedef struct St05Ember {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR positions[4]; /* a ring; current indexes the newest */
    /* 0x58 */ VECTOR velocity;
    /* 0x68 */ s32 current;
    /* 0x6C */ s32 life; /* 128 to 255 at construction, 2 less per tick */
} St05Ember;

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

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5D54);

INCLUDE_ASM("st05/nonmatchings/st05", func_800D5E3C);

INCLUDE_ASM("st05/nonmatchings/st05", St05Hit_apply);

/* A quad whose size closes toward its target while its alpha falls (St05Fade_init, virtual table
 * ST05_FADE_VTABLE). */
typedef struct St05Fade {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} St05Fade;

extern EntityVtable ST05_FADE_VTABLE;

/* Constructs an St05Fade at position, with unk30 and unk34; 0x38 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
St05Fade *St05Fade_init(St05Fade *this, Vec3 *position, s32 unk30, s32 unk34) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST05_FADE_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = 0;
    this->unk30 = unk30;
    this->unk34 = unk34;
    this->unk38 = 0xFF;
    return this;
}

/* St05Fade's tick: 0x28 and 0x2C close a quarter of their distance to 0x30 and 0x34, and 0x38 falls
 * by 0x18; the entity deletes itself when 0x38 runs out. */
void St05Fade_tick(St05Fade *this) {
    this->unk28 += (this->unk30 - this->unk28) >> 2;
    this->unk2C += (this->unk34 - this->unk2C) >> 2;
    this->unk38 -= 0x18;
    if (this->unk38 <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", St05Fade_draw);

/* A Gouraud triangle that slows down while its alpha falls (St05Shard_init, virtual table ST05_SHARD_VTABLE). */
typedef struct St05Shard {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
} St05Shard;

extern EntityVtable ST05_SHARD_VTABLE;

/* Constructs an St05Shard at position, with unk30 and unk2C; 0x34 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
St05Shard *St05Shard_init(St05Shard *this, Vec3 *position, s32 unk30, s32 unk2C) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST05_SHARD_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = unk2C;
    this->unk30 = unk30;
    this->unk34 = 0xFF;
    return this;
}

/* St05Shard's tick: 0x28 moves by 0x2C, which loses an eighth of itself, and 0x34 falls by 8; the
 * entity deletes itself when 0x34 runs out. */
void St05Shard_tick(St05Shard *this) {
    this->unk28 += this->unk2C;
    this->unk2C += -this->unk2C >> 3;
    this->unk34 -= 8;
    if (this->unk34 <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st05/nonmatchings/st05", St05Shard_draw);

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

/* The hit of the stage's shots (St05Hit_init): StageHit's layout, but its slot 0x08 (St05Hit_apply) pushes the
 * touched sphere away from center (its knockback gets 32 times its offset from center in x, 16 times in y) and adds
 * the damage. */
typedef struct St05Hit {
    /* 0x0 */ VtableEntry *vtable;
    /* 0x4 */ s32 damage;
    /* 0x8 */ Vec3 center;
} St05Hit;

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

/* The entity class func_800D5D54 constructs (virtual table D_800D3B94): an
 * CollisionOwner base at 0x18 (virtual table ST05_HIT_VTABLE) and a member of
 * CollisionSphere_init's class at 0x30. */
typedef struct Unk800D5D54 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ CollisionOwner unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ Vec3 unk20; /* func_800D5D54's third argument */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ CollisionShape unk30;
} Unk800D5D54;

/* Unk800D5D54's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D6AC4(Unk800D5D54 *this, s32 flags) {
    ListNode *link;

    this->unk30.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk30.link) written out: the match depends on this
     * access order (soundControlDestroy's), as in st04's func_800D8618 */
    link = &this->unk30.link;
    this->unk30.link.next->prev = this->unk30.link.prev;
    link->prev->next = this->unk30.link.next;
    this->unk30.link.next = link;
    this->unk30.link.prev = link;
    this->unk18.vtable = COLLISION_OWNER_VTABLE;
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
