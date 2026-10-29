#include <libgte.h>
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/matrix.h"
/* BlackWarGreymon (CHARACTER_BLACKWARGREYMON): the fighter createFighter builds for it, derived from WarGreymon (whose
 * methods it keeps but five), and the entities its own slots spawn: the slot 0x120 effect and the slot 0x128 hitbox. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

extern EntityVtable D_8006D01C;
extern EntityVtable BLACKWARGREYMON_VTABLE;

/* BlackWarGreymon's class (CHARACTER_BLACKWARGREYMON), derived from WarGreymon's. */
typedef struct BlackWarGreymon {
    /* 0x000 */ WarGreymon base;
    /* 0x4C4 */ s32 unk4C4;
} BlackWarGreymon;


extern s32 D_8006C844[];
void WarGreymon_holdOpponent(Fighter *this);
s32 func_800C1DBC(Fighter *this);

typedef struct {
    s32 words[8];
} AlignedSV4;

extern AlignedSV4 D_8006CFAC;

static inline void setTranslation(long *t, Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    t[0] = x;
    t[1] = y;
    t[2] = z;
}

typedef struct Unk800BD490 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ MATRIX matrix;          /* func_800BD8DC's model matrix */
    /* 0x3C */ SVECTOR band[8][2];     /* the band's points, two per step */
    /* 0xBC */ u8 unkBC[0x114 - 0xBC];
    /* 0x114 */ s32 shade;             /* red, and a quarter of it green */
    /* 0x118 */ s32 unk118;
    /* 0x11C */ s32 vramY;             /* the VRAM half of the clut and tpage */
} Unk800BD490;

Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4);
extern CVECTOR D_8006CFA8;
extern CVECTOR D_8006CF84; /* the intro hitbox's colour */
extern VECTOR D_8006CF98;
void func_800C1CE0(BlackWarGreymon *this);

/* Builds BlackWarGreymon: a WarGreymon with its own vtable, 0x4C4 0. */
BlackWarGreymon *BlackWarGreymon_init(BlackWarGreymon *this) {
    WarGreymon_init(&this->base);
    this->base.base.entity.vtable = &BLACKWARGREYMON_VTABLE;
    this->unk4C4 = 0;
    return this;
}

/* Input: Fighter's (Fighter_handleInput), then this class's own step (func_800C1CE0). */
void BlackWarGreymon_handleInput(void *this, void *arg) {
    Fighter_handleInput(this, arg);
    func_800C1CE0(this);
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF84);

/* Intro: at frame 24 the intro's hitbox (func_800BE39C) at the fighter's point (func_8007A650) and sound 0x11C; at
 * frame 80 it faces the opponent's half (right on side 0). */
void BlackWarGreymon_intro(BlackWarGreymon *this, s32 frame) {
    if (frame == 24) {
        CVECTOR color = D_8006CF84;
        Vec3 position;
        Entity *child;

        func_8007A650(&this->base.base, &position);
        child = allocEntityMemory(0xE0);
        if (child != NULL) {
            /* arg1 is the fighter here; the hitbox methods that also spawn one pass their s32 arg */
            func_800BE39C(entityOperatorNewAt(0xE0, child), (s32)this, &position, 0, 0x300, &color);
        }
        Fighter_addEntity(&this->base.base, child);
        Fighter_playSound(&this->base.base, 0x11C);
    }
    if (frame == 80) {
        if (Fighter_getSide(&this->base.base) == 0) {
            Fighter_faceRight(&this->base.base, 0);
        } else {
            Fighter_faceLeft(&this->base.base, 0);
        }
    }
}

/* Throw: Fighter's (Fighter_throw); frames 0 to 6 accelerates to 0 and copies velocity and force to the opponent;
 * at frame 0 holds the opponent 7 frames; at frame 7 takes a 0-damage hit, holds 35 frames and sets character-dependent
 * clip; at frame 35 sets clip 0xE and adds 0x8E30 to y force; frames 7 to 34 holds opponent with WarGreymon_holdOpponent;
 * at frame 59 stops if out of reach; at frame 60 hits with forward push and sparks. */
void BlackWarGreymon_throw(BlackWarGreymon *this, s32 frame) {
    Fighter *opponent;
    s32 x, y, z;

    Fighter_throw(&this->base.base, frame);
    opponent = Fighter_getOpponent(&this->base.base);
    if (frame < 7) {
        Fighter_accelerateTo(&this->base.base, 0);
        x = this->base.base.velocity.vx;
        y = this->base.base.velocity.vy;
        z = this->base.base.velocity.vz;
        opponent->velocity.vx = x;
        opponent->velocity.vy = y;
        opponent->velocity.vz = z;
        x = this->base.base.force.vx;
        y = this->base.base.force.vy;
        z = this->base.base.force.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
    }
    switch (frame) {
    case 0:
        this->unk4C4 = 0;
        Fighter_hold(opponent, 7);
        break;
    case 7: {
        s32 clip;
        Fighter_takeHit(opponent, 0x800800, (Vec3 *)&ZERO_VECTOR, 0);
        Fighter_hold(opponent, 35);
        clip = D_8006C844[Fighter_getCharacter(opponent)];
        Fighter_setClip(opponent, clip, 0, 0);
        break;
    }
    case 35: {
        Vec3 vec;
        vec.vx = 0;
        vec.vy = 0x8E30;
        vec.vz = 0;
        Fighter_setClip(opponent, 0xE, 0, 0);
        {
            s32 fx = opponent->force.vx;
            s32 fy = opponent->force.vy;
            s32 fz = opponent->force.vz;

            fx += vec.vx;
            fy += vec.vy;
            fz += vec.vz;
            opponent->force.vx = fx;
            opponent->force.vy = fy;
            opponent->force.vz = fz;
        }
        this->unk4C4 = 1;
        break;
    }
    case 59:
        if (!func_800C1DBC(&this->base.base)) {
            Fighter_stopActionClip(&this->base.base);
            this->unk4C4 = 0;
            return;
        }
        break;
    case 60: {
        Vec3 push;
        s32 facing = Fighter_getFacing(&this->base.base);
        push.vx = facing * 29120;
        push.vy = -0x5550;
        push.vz = 0;
        opponent->force.vx = 0;
        opponent->force.vy = 0;
        opponent->force.vz = 0;
        opponent->forcePad = 0;
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        Fighter_takeHit(opponent, 0x408C4000, &push, 0);
        Fighter_spawnHitSpark(&this->base.base, 0x100);
        this->unk4C4 = 0;
        break;
    }
    }

    if ((u32)(frame - 7) < 28) {
        WarGreymon_holdOpponent(&this->base.base);
    }
}

/* the velocity func_800C1CE0 throws the opponent with */
extern VECTOR D_8006CF88;

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF88);

/* This class's own step (BlackWarGreymon_handleInput): when the opponent (Fighter_getOpponent) is over 0x800 above its
 * last position, 0x4C4 goes back to 0; at 1, once the opponent is on the ground, stops its force and velocity, throws
 * it with D_8006CF88 (Fighter_takeHit) and sets 0x4C4 to 2. */
void func_800C1CE0(BlackWarGreymon *this) {
    Fighter *opponent = Fighter_getOpponent(&this->base.base);
    VECTOR velocity;

    if ((opponent->position.vy >> 8) - (opponent->lastPosition.vy >> 8) < -0x800) {
        this->unk4C4 = 0;
        return;
    }
    if (this->unk4C4 == 1 && Fighter_isOnGround(opponent)) {
        velocity = D_8006CF88;
        opponent->force.vx = 0;
        opponent->force.vy = 0;
        opponent->force.vz = 0;
        opponent->forcePad = 0;
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        Fighter_takeHit(opponent, 0x408C1000, (Vec3 *)&velocity, 0);
        this->unk4C4 = 2;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF98);

/* Whether point 10 moved by D_8006CF98 is nearer the opponent's top (func_8007A6CC) than its height plus 512. */
s32 func_800C1DBC(Fighter *this) {
    s32 reach;
    VECTOR node;
    VECTOR way;
    s32 x, y, z;

    reach = Fighter_getParams(Fighter_getOpponent(this))->height + 512;
    Fighter_getNodePosition(this, &node, 10, &D_8006CF98);
    func_8007A6CC(this, (Vec3 *)&way);
    /* The match depends on the vecSubInPlace shape: read way whole, subtract, then store. */
    x = way.vx;
    y = way.vy;
    z = way.vz;
    x -= node.vx;
    y -= node.vy;
    z -= node.vz;
    way.vx = x;
    way.vy = y;
    way.vz = z;
    return vecGetLength(&way) < reach;
}

/* Creates an entity of the class of vtable D_8006D01C (0x120 bytes) with g++'s new, and hands it
 * to Fighter_addEntity. Its constructor func_800C27E4 is inlined. */
void func_800C1E64(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006D01C;
    }
    Fighter_addEntity(this, child);
}

/* Special move 3: frame 0 the voice, action clip 0x1A and the slot 0x128 hitbox at (144 forward, 48) (arg 0x31);
 * forward pushes at frames 45 and 64, the velocity stopped at frame 50. */
void BlackWarGreymon_special3(BlackWarGreymon *this, s32 frame) {
    Vec3 offset;
    WarGreymonVtable *vtable;

    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base.base, 0x121);
        Fighter_playActionClip(&this->base.base, FIGHTER_CLIP_SPECIAL3, 4, 0, -1);
        offset.vx = Fighter_getFacing(&this->base.base) * 144;
        offset.vy = 48;
        offset.vz = 0;
        vtable = GET_WARGREYMON_VTABLE(&this->base);
        vtable->unk128.func((u8 *)this + vtable->unk128.delta, &offset, 0x31);
        break;
    case 45: {
        s64 push = ((s64)this->base.base.mass * 0x3FFC) >> 16;

        this->base.base.force.vx = (s32)push * Fighter_getFacing(&this->base.base);
        break;
    }
    case 50:
        this->base.base.velocity.vx = 0;
        break;
    case 64: {
        s64 push = ((s64)this->base.base.mass * 0xE38) >> 16;

        this->base.base.force.vx = (s32)push * Fighter_getFacing(&this->base.base);
        break;
    }
    }
}

/* Creates an entity of func_800C2264's class (0xD4 bytes) at pos with g++'s new and hands it to Fighter_addEntity, then
 * plays sound 0x11B, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800C2004(Fighter *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800C2264(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x200);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0x11B);
    this->invulnerableFrames = 10;
    Fighter_addGauge(this, -0x10000);
}

/* The method at 0x14 of virtual table D_8006D01C: draws the entity's 8 band segments, each between its two
 * points and the second point of the step after, with the 32 by 32 picture at (0xE0, 0xC0), black at the start and
 * shade at the end, semi-transparent. In state 6 (loading / result screen) its colors and tpage change. */
void func_800C20A8(Unk800BD490 *this, Camera *camera) {
    PolyGT4Shape shape;
    u_long *ot;
    s32 shift;
    u8 *packet;
    s32 i;
    s32 next;
    s32 after;
    s32 vramY;
    s32 shade;

    setModelMatrix(&this->matrix);
    ot = camera->ot.slots;
    shift = camera->zShift;
    packet = PRIM_BUFFER_FREE;
    shape.u0 = shape.u2 = 0xE0;
    shape.u1 = shape.u3 = 0xFF;
    shape.v0 = shape.v1 = 0xC0;
    shape.v2 = shape.v3 = 0xDF;
    shape.colors[0] = shape.colors[1] = 0;
    shape.flags = 0xA;
    shade = this->shade;
    vramY = this->vramY;
    shape.colors[2] = shape.colors[3] = shade | (shade << 8) | ((shade >> 1) << 16);
    shape.clut = getClut(0x1C0, vramY + 0xFA);
    shape.tpage = ((((vramY & 0x100) >> 4) | 0x47) | ((vramY & 0x200) << 2));
    if (GAME_STATE.arena == 6) {
        shape.colors[2] = shape.colors[3] = (shade >> 2) | ((shade >> 2) << 8) | (shade << 16);
        shape.tpage = ((((vramY & 0x100) >> 4) | 0x27) | ((vramY & 0x200) << 2));
        shape.flags = 9;
    }
    for (i = 0; i < 8; i++) {
        next = i + 1;
        after = i + 2;
        if (next >= 8) {
            next -= 8;
        }
        if (after >= 8) {
            after -= 8;
        }
        shape.corners[0] = &this->band[i][0];
        shape.corners[1] = &this->band[next][0];
        shape.corners[2] = &this->band[next][1];
        shape.corners[3] = &this->band[after][1];
        packet = buildPolyGT4(&shape, packet, ot, shift);
    }
    PRIM_BUFFER_FREE = packet;
}

extern VtableEntry D_8006CFCC[];
extern EntityVtable D_8006CFEC;

/* Constructs the class of virtual table D_8006CFEC (and D_8006CFCC for its base at 0x18) on
 * func_800BDA50's, with 0x5B05333 and 0 in its base's 0x8 and 0xC. */
Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    func_800BDA50(this, owner, pos, arg3, arg4);
    this->unk18.vtable = D_8006CFCC;
    this->entity.vtable = &D_8006CFEC;
    Hit_set(&this->unk18, 0x5B05333, 0);
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFA8);

/* The method at 0x20 of virtual table D_8006CFEC: adds func_800BE39C's and ParticleBurst_init's entities at 0xA4
 * as siblings, then plays sound 0x11C. */
void func_800C22C0(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB0A666, 0x500, &D_8006CFA8);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0xC0, 0xC0, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x11C, 0);
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFAC);

void func_800C23B8(Unk800BDA50 *this, Camera *camera) {
    MATRIX matrix;
    MATRIX *m = &matrix;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    PolyFT4Shape *s;
    s32 new_var;
    u8 *packet;
    u_long *ot;
    s32 shift;
    s32 vramY;
    s32 fade;

    packet = PRIM_BUFFER_FREE;
    matrix = camera->view->rotation;
    setTranslation(matrix.t, (Vec3 *)&this->unk84);
    ot = camera->ot.slots;
    new_var = ~3;
    shift = camera->zShift;
    matrixRotateZ(m, this->unkBC);
    matrixScale(m, this->unkB4, this->unkB4, this->unkB4);
    setModelMatrix(m);

    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    *(AlignedSV4 *)corners = D_8006CFAC;

    vramY = this->vramY;
    shape.flags = 9;
    shape.tpage = ((((vramY & 0x100) >> 4) | 0x27) | ((vramY & 0x200) << 2));
    s = &shape;

    if (this->unkC0 < 0x80) {
        fade = 0x80 - this->unkC0;
        shape.color = fade | (fade << 8) | (fade << 16);
        UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0xC0;
        UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0xDF;
        UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0xC0;
        UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xDF;
        shape.clut = getClut(0x1C0, vramY + 0xFB);
        packet = buildPolyFT4(s, packet, ot, shift);
    }

    fade = this->unkC0;
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0x3F;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0x80;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xBF;
    shape.clut = getClut(0x1C0, vramY + 0xFC);
    corners[2].vy = corners[3].vy = 0;
    corners[1].vx = corners[3].vx = 0;
    shape.color = fade | (fade << 8) | (fade << 16);
    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vx = corners[2].vx = 0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vy = corners[1].vy = 0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vx = corners[2].vx = -0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    shape.clut = getClut(0x1C0, vramY + 0xFD);
    shape.tpage = ((((vramY & 0x100) >> 4) | 0x47) | ((vramY & 0x200) << 2));
    shape.flags = (shape.flags & new_var) | 0xA;

    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vy = corners[1].vy = -0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vx = corners[2].vx = 0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    corners[0].vy = corners[1].vy = 0x1000;
    packet = buildPolyFT4(s, packet, ot, shift);

    PRIM_BUFFER_FREE = packet;
}

/* BlackWarGreymon's destructor. */
void BlackWarGreymon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D01C, which func_800C1E64 and func_800C27E4 install. */
void func_800C279C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006D01C, which derives from func_800BD490's. */
Entity *func_800C27E4(Entity *this, Fighter *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006D01C;
    return this;
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800C2818(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFCC);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFEC);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006D01C);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", BLACKWARGREYMON_VTABLE);
