/* VMon (CHARACTER_V_MON): the fighter createFighter builds for it (a plain Fighter), its moves, and the entities they
 * spawn: the special move 1's effect and the special move 2's effect. */

#include "common.h"
#include "battle/game_classes.h"

struct VMonSpecial2Effect *VMonSpecial2Effect_init(struct VMonSpecial2Effect *this, Fighter *fighter, Vec3 *position,
                                                   Fight *fight);
struct VMonSpecial1Effect *VMonSpecial1Effect_init(struct VMonSpecial1Effect *this, Fighter *fighter, VECTOR *pos,
                                                   Fight *fight);
extern FaceEntry V_MON_FACES[];
extern EntityVtable V_MON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable V_MON_SPECIAL1_EFFECT_VTABLE;
extern EntityVtable V_MON_VTABLE;
extern VECTOR D_800D348C;

/* Builds VMon: a Fighter with its own vtable and faces. */
Fighter *VMon_init(Fighter *this) {
    Fighter_init(this);
    this->entity.vtable = &V_MON_VTABLE;
    Fighter_setFaceTable(this, V_MON_FACES);
    return this;
}

/* VMon's destructor. */
void VMon_destroy(Entity *this, s32 flags) {
    this->vtable = &V_MON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: the z rotation and its target back to 0. */
void VMon_endMove(Fighter *this) {
    this->targetRotation.vz = 0;
    this->rotation.vz = 0;
}

/* Attack 1: Fighter's; frame 3 a push forward of 0x2AA8 times the mass (16.16); from frame 4 the force brakes a quarter
 * of the horizontal velocity. */
void VMon_attack1(Fighter *this, s32 frame) {
    Fighter_attack1(this, frame);
    if (frame == 3) {
        s32 push;

        push = ((s64)this->mass * 0x2AA8) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
    }
    if (frame >= 4) {
        this->force.vx += -this->velocity.vx >> 2;
    }
}

/* Attack 2: Fighter's; frame 4 a push forward of 0x38E0 times the mass (16.16); from frame 5 the force brakes a quarter
 * of the horizontal velocity. */
void VMon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 4) {
        s32 push;

        push = ((s64)this->mass * 0x38E0) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
    }
    if (frame >= 5) {
        this->force.vx += -this->velocity.vx >> 2;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMon_attack3);

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMon_throw);

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMon_special1);

void VMon_spawnSpecial1Effect(Fighter *this, s32 arg);

/* Special move 1 in the air: at frame 0 tilted 0x180 toward the facing, its clip, a push forward and down, hits of kind
 * 3 and the voice; part 2 hits from frame 2 (0x85A81A00), the effect at 4, no part from 17; the force brakes 1/64 of the
 * horizontal velocity after frame 0. */
void VMon_airSpecial1(Fighter *this, s32 frame) {
    s32 push;
    s32 drop;

    func_80079AE4(this);
    switch (frame) {
    case 0:
        if (Fighter_getFacing(this) > 0) {
            this->rotation.vz = this->targetRotation.vz = 0x180;
        } else {
            this->rotation.vz = this->targetRotation.vz = -0x180;
        }
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 0, 0, -1);
        this->velocity.vx = 0;
        push = ((s64)this->mass * 0x4718) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
        /* The match depends on the downward push in its own local, like the forward one. */
        drop = ((s64)this->mass * 0x1C70) >> 16;
        this->force.vy += drop;
        func_80079B0C(this, 28);
        this->hitKind = 3;
        Fighter_playVoice(this, 0xFF);
        break;
    case 2:
        Fighter_setAttackParts(this, 2);
        this->hitDamage = 0x85A81A00;
        break;
    case 4:
        VMon_spawnSpecial1Effect(this, 1);
        break;
    case 17:
        this->attackParts = 0;
        break;
    }
    if (frame > 0) {
        this->force.vx -= this->velocity.vx >> 6;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMon_special2);

/* Spawns the special move 1's effect at the fighter's point 2 moved by D_800D348C, with arg in the position's pad. */
void VMon_spawnSpecial1Effect(Fighter *this, s32 arg) {
    Fight *fight = Fighter_getScene(this);
    VECTOR pos;
    Entity *child;

    Fighter_getNodePosition(this, &pos, 2, &D_800D348C);
    pos.pad = arg;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        VMonSpecial1Effect_init(entityOperatorNewAt(0x58, child), this, &pos, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 2's effect at the fighter's position. */
void VMon_spawnSpecial2Effect(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    child = allocEntityMemory(0x60);
    if (child != NULL) {
        VMonSpecial2Effect_init(entityOperatorNewAt(0x60, child), this, &position, fight);
    }
    Fighter_addEntity(this, child);
}

/* The special move 1's effect: a square of side 0x100 facing the character's way, with the pad of its position
 * vector. */
typedef struct VMonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *fight;
    /* 0x20 */ s16 unk20; /* the pad of the position (VMon_spawnSpecial1Effect's arg) */
    /* 0x22 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x24 */ s16 unk24; /* minus the character's facing */
    /* 0x26 */ u16 frames;
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ VECTOR position; /* the character's point 2 (Fighter_getNodePosition) */
} VMonSpecial1Effect;

/* Builds the special move 1's effect for fighter: a square from (-0x80, -0x80) to (0x80, 0x80) at z -0x80. */
VMonSpecial1Effect *VMonSpecial1Effect_init(VMonSpecial1Effect *this, Fighter *fighter, VECTOR *pos, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &V_MON_SPECIAL1_EFFECT_VTABLE;
    this->fighter = fighter;
    this->fight = fight;
    this->frames = 0;
    this->vramY = Fighter_getVramY(this->fighter);
    this->unk24 = -Fighter_getFacing(this->fighter);
    this->unk20 = pos->pad;
    /* The match depends on these chained assignments, as in st07's BonusThrower_init. */
    this->corners[0].vx = this->corners[2].vx = -0x80;
    this->corners[1].vx = this->corners[3].vx = 0x80;
    this->corners[0].vy = this->corners[1].vy = -0x80;
    this->corners[2].vy = this->corners[3].vy = 0x80;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x80;
    return this;
}

/* The special move 1 effect's destructor. */
void VMonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &V_MON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

extern VECTOR D_800D349C;

/* The tick of VMonSpecial1Effect_init's entity: puts it at the character's point 2 moved by D_800D349C, and deletes
 * it after 11 frames. The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x26. */
void VMonSpecial1Effect_tick(VMonSpecial1Effect *this) {
    Fighter_getNodePosition(this->fighter, &this->position, 2, &D_800D349C);
    if (this->frames > 10) {
        entityDelete(&this->entity);
    } else {
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMonSpecial1Effect_draw);

/* The special move 2's effect: a flat square of side 0x140 facing the character's way, at a position. */
typedef struct VMonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *fight;
    /* 0x20 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x22 */ s16 unk22; /* minus the character's facing */
    /* 0x24 */ u16 frames; /* counts the frames of VMonSpecial2Effect_tick */
    /* 0x26 */ u8 unk26[0x28 - 0x26];
    /* 0x28 */ SVECTOR corners[4];
    /* 0x48 */ u8 unk48; /* eases up to 0x80, then fades out (VMonSpecial2Effect_tick) */
    /* 0x4A */ u16 phase; /* VMonSpecial2Effect_tick's step: fade in, hold, fade out, delete */
    /* 0x4C */ u16 holdFrames; /* the frames of the hold step */
    /* 0x50 */ Vec3 position;
} VMonSpecial2Effect;

/* Builds the special move 2's effect for fighter: a square from (-0xA0, -0xA0) to (0xA0, 0xA0) at z 0, at position
 * raised by 0x80. */
VMonSpecial2Effect *VMonSpecial2Effect_init(VMonSpecial2Effect *this, Fighter *fighter, Vec3 *position, Fight *fight) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &V_MON_SPECIAL2_EFFECT_VTABLE;
    this->fighter = fighter;
    this->fight = fight;
    this->frames = 0;
    this->vramY = Fighter_getVramY(this->fighter);
    this->unk22 = -Fighter_getFacing(this->fighter);
    x = position->vx;
    y = position->vy - 0x80;
    z = position->vz;
    this->unk48 = 0;
    this->phase = 0;
    this->holdFrames = 0;
    /* The match depends on these chained assignments, as in st07's BonusThrower_init. */
    this->corners[0].vx = this->corners[2].vx = -0xA0;
    this->corners[1].vx = this->corners[3].vx = 0xA0;
    this->corners[0].vy = this->corners[1].vy = -0xA0;
    this->corners[2].vy = this->corners[3].vy = 0xA0;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The special move 2 effect's destructor. */
void VMonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &V_MON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of VMonSpecial2Effect_init's entity: follows the character (Fighter_getPosition) 0x80 higher, fades in to
 * 0x80, holds 22 frames, fades out and deletes itself; it also goes when the character's state ends. */
void VMonSpecial2Effect_tick(VMonSpecial2Effect *this) {
    s32 state;

    Fighter_getPosition(this->fighter, &this->position);
    this->position.vy -= 0x80;
    switch (this->phase) {
    case 0:
        this->unk48 += (0x84 - this->unk48) >> 2;
        if (this->unk48 < 0x80) {
            break;
        }
        this->holdFrames = 0;
        this->phase++;
    case 1:
        if (++this->holdFrames < 22) {
            break;
        }
        this->phase++;
    case 2:
        this->unk48 -= this->unk48 >> 2;
        if (this->unk48 >= 5) {
            break;
        }
        this->phase++;
    case 3:
        entityDelete(&this->entity);
        return;
    }
    state = Fighter_getBodyState(this->fighter)->state;
    if (Fighter_isAttackIdle(this->fighter) || state == 0x13) {
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/v_mon", VMonSpecial2Effect_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C284);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C28C);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C294);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C29C);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2A4);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2AC);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2B4);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2BC);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2C4);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2CC);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2D4);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2DC);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C2E4);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", D_8006C304);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", V_MON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", V_MON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", V_MON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/v_mon", V_MON_VTABLE);
