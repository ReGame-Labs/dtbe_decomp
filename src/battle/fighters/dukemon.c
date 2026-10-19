/* Dukemon (CHARACTER_DUKEMON): the fighter createFighter builds for it, its moves, and the trails and entities they
 * spawn. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"

Entity *DukemonSpecial1Effect_init(Entity *this, Fighter *arg, Fight *arg2);
Entity *func_80085B7C(Entity *this, Fighter *arg, Vec3 *vec);
struct Unk800862C0 *func_800862C0(struct Unk800862C0 *this, Fighter *fighter, Vec3 *position);
struct DukemonSpecial2Effect *DukemonSpecial2Effect_init(struct DukemonSpecial2Effect *this, Fighter *fighter,
                                                          Vec3 *position, Fight *fight);
struct DukemonSpecial3Hitbox *DukemonSpecial3Hitbox_init(struct DukemonSpecial3Hitbox *this, Fighter *fighter,
                                                         Vec3 *position, VECTOR *velocity, Fight *fight);
extern VtableEntry DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable DUKEMON_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable D_800686E4;
extern EntityVtable D_8006870C;
extern EntityVtable DUKEMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable DUKEMON_SPECIAL1_EFFECT_VTABLE;
extern EntityVtable DUKEMON_VTABLE;

typedef struct Dukemon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ u16 special3Landed; /* special3 clears it, its hitbox sets it on the first fighter it knocks back */
    /* 0x4A6 */ u16 special3Active; /* set every frame of special3, cleared by endMove; its effects end with it */
} Dukemon;
extern VECTOR D_80068654;

/* Builds Dukemon: a Fighter with its own vtable. */
Dukemon *Dukemon_init(Dukemon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &DUKEMON_VTABLE;
    this->special3Landed = 0;
    this->special3Active = 0;
    return this;
}

/* Dukemon's destructor. */
void Dukemon_destroy(Entity *this, s32 flags) {
    this->vtable = &DUKEMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: special3 is over. */
void Dukemon_endMove(Dukemon *this) {
    this->special3Active = 0;
}

/* The dash, with dust. */
void Dukemon_dashLeft(Fighter *this, s32 frame) {
    Fighter_dashLeft(this, frame);
    Fighter_spawnDust(this, -1, 160, 4);
}

/* The dash, with dust. */
void Dukemon_dashRight(Fighter *this, s32 frame) {
    Fighter_dashRight(this, frame);
    Fighter_spawnDust(this, -1, 160, 4);
}

/* The dash attack, with dust every fourth frame before frame 18. */
void Dukemon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame < 18 && (frame & 3) == 0) {
        Fighter_spawnDust(this, -1, 160, 2);
    }
}

void Dukemon_spawnAttack3Trails(Fighter *this);

/* Attack 3, a lunge: at frame 0 its clip, trails and hit (knockback forward and up), the lunge of the params' attack 3
 * and parts 8; parts 4 from frame 5; dust before frame 5; out of the attack's active frames no part hits. */
void Dukemon_attack3(Fighter *this, s32 frame) {
    FighterParams *params;
    Attack *attack;
    s32 push;

    func_80079AE4(this);
    params = Fighter_getParams(this);
    attack = &params->attacks[2];
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_ATTACK3, 0, 0, -1);
        Dukemon_spawnAttack3Trails(this);
        this->hitDamage = 0x41881500;
        this->hitKnockback.vx = Fighter_getFacing(this) * 18200;
        this->hitKnockback.vy = -0x1C70;
        this->hitKnockback.vz = 0;
        push = ((s64)params->attacks[2].lungeX * this->mass) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
        Fighter_setAttackParts(this, 8);
        break;
    case 5:
        this->attackParts = 0;
        Fighter_setAttackParts(this, 4);
        break;
    }
    if (frame < 5) {
        Fighter_spawnDust(this, -1, 160, 4);
    }
    if (frame >= attack->firstActiveFrame && frame <= attack->lastActiveFrame) {
        Fighter_setAttackParts(this, attack->parts);
    } else {
        this->attackParts = 0;
    }
}

/* Throw: at frame 0 the opponent flinches and is held for 39 frames at point 20, its model 0x120 up; at 39 it is hit
 * forward and down and let go, at 50 it is hit up again; dust on both sides of it at each hit. */
void Dukemon_throw(Fighter *this, s32 frame) {
    Fighter *opponent;
    Vec3 vel;
    Model *model;
    s16 side;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_setClip(opponent, FIGHTER_CLIP_FLINCH, 0, 0);
        Fighter_hold(opponent, 39);
        this->velocity.vx = 0;
        Fighter_attachTo(opponent, this, 20);
        model = opponent->model;
        model->group.object.transform.valid = 0;
        model->group.object.transform.local.m.t[0] = 0;
        model->group.object.transform.local.m.t[1] = 0x120;
        model->group.object.transform.local.m.t[2] = 0;
        break;
    case 39:
        /* side is a halfword: the game sign-extends the result (sll, sra) before the multiply */
        side = Fighter_getFacing(this);
        vel.vx = side * 18200;
        vel.vy = 0x2AA8;
        vel.vz = 0;
        Fighter_takeHit(opponent, 0x998C1A00, &vel, 1);
        Fighter_detach(opponent);
        Fighter_spawnDust(opponent, 1, 160, 3);
        Fighter_spawnDust(opponent, -1, 160, 3);
        break;
    case 50:
        vel.vx = 0;
        vel.vy = -0x238C;
        vel.vz = 0;
        Fighter_takeHit(opponent, 0x998C1A00, &vel, 1);
        Fighter_spawnDust(opponent, 1, 160, 3);
        Fighter_spawnDust(opponent, -1, 160, 3);
        break;
    }
}

void Dukemon_spawnTrail(Fighter *this, s32 length);
void Dukemon_spawnSpecial1Effect(Fighter *this);

/* Special move 1, a charge: voice at frame 0, a trail at 5, hits at 7 and 17 (the second launches); it pushes forward
 * from frame 5 to 16 with dust, and spawns its effect every frame from 6 to 13. */
void Dukemon_special1(Fighter *this, s32 frame) {
    s32 even;

    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        this->velocity.vx = 0;
        Fighter_playVoice(this, 0x7F);
        break;
    case 5:
        Dukemon_spawnTrail(this, 15);
        break;
    case 7:
        Fighter_setAttackParts(this, 8);
        this->hitDamage = 0x5A82190;
        this->hitKind = 2;
        break;
    case 17:
        this->attackParts = 0;
        Fighter_setAttackParts(this, 4);
        this->hitDamage = 0x45A82190;
        Fighter_spawnDust(this, -1, 160, 4);
        this->hitKnockback.vx = Fighter_getFacing(this) * 14560;
        this->hitKnockback.vy = -0x38E0;
        this->hitKnockback.vz = 0;
        this->hitKind = 2;
        this->velocity.vx = 0;
        break;
    case 29:
        this->attackParts = 0;
        break;
    }
    if (frame >= 5 && frame <= 16) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 25480);
    }
    /* The match depends on the parity test computed as a value in a local (xori/andi, do_store_flag). */
    even = (frame & 1) == 0;
    if (even && frame >= 6 && frame < 14) {
        Fighter_spawnDust(this, -1, 160, 4);
    }
    if (frame >= 6 && frame <= 13) {
        Dukemon_spawnSpecial1Effect(this);
    }
}

void Dukemon_spawnSpecial2Effect(Fighter *this);
void Dukemon_spawnSpecial2Trails(Fighter *this);

/* Special move 2, a rising hit: its effect and two trails, the hit and the voice at frame 0; a lift at frame 5; until
 * frame 22 the force brakes a fall. */
void Dukemon_special2(Fighter *this, s32 frame) {
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Dukemon_spawnSpecial2Effect(this);
        Dukemon_spawnSpecial2Trails(this);
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL2, 0, 0, -1);
        this->velocity.vy = 0;
        Fighter_setAttackParts(this, 12);
        this->hitDamage = 0x45AC2A00;
        this->hitKnockback.vx = Fighter_getFacing(this) * 5460;
        this->hitKnockback.vy = -0x6388;
        this->hitKnockback.vz = 0;
        this->hitKind = 3;
        Fighter_playVoice(this, 0x80);
        break;
    case 5:
        this->force.vy -= ((s64)this->mass * 0x6AA4) >> 16;
        break;
    }
    if (frame < 22 && this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", Dukemon_airUpAttack);

/* Special move 2 in the air: as on the ground, with the lift at frames 0 and 5 and the braking until frame 12. */
void Dukemon_airSpecial2(Fighter *this, s32 frame) {
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Dukemon_spawnSpecial2Effect(this);
        Dukemon_spawnSpecial2Trails(this);
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL2, 0, 0, -1);
        this->velocity.vy = 0;
        this->force.vy -= ((s64)this->mass * 0x38E0) >> 16;
        Fighter_setAttackParts(this, 12);
        this->hitDamage = 0x45AC2A00;
        this->hitKnockback.vx = Fighter_getFacing(this) * 5460;
        this->hitKnockback.vy = -0x6388;
        this->hitKnockback.vz = 0;
        Fighter_playVoice(this, 0x80);
        break;
    case 5:
        this->force.vy -= ((s64)this->mass * 0x38E0) >> 16;
        break;
    }
    if (frame < 12 && this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", Dukemon_special3);

/* The two entries of a table that Dukemon_spawnTrail, Dukemon_spawnSpecial2Trails and Dukemon_spawnAttack3Trails each
 * initialise. cc1 gives identical array initializers one constant per unit, so the three share its data (D_800685DC in
 * the asm). */
#define D_800685DC_ENTRIES \
    { 0x14, { 0, 0x180, 0, 0 }, { 0xFF, 0x80, 0x80, 0 } }, \
    { 0x14, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }

/* Spawns a Trail of two points on the fighter, length frames long. */
void Dukemon_spawnTrail(Fighter *this, s32 length) {
    TrailPoint table[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, table, 2, 8, length);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 1's effect. */
void Dukemon_spawnSpecial1Effect(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        DukemonSpecial1Effect_init(entityOperatorNewAt(0x48, child), this, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 2's effect at the fighter. */
void Dukemon_spawnSpecial2Effect(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Vec3 position;
    Entity *child = allocEntityMemory(0x34);

    if (child != NULL) {
        DukemonSpecial2Effect_init(entityOperatorNewAt(0x34, child), this, Fighter_getPosition(this, &position), fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 2's two trails, 20 and 15 frames long. */
void Dukemon_spawnSpecial2Trails(Fighter *this) {
    TrailPoint first[3] = {
        { 0, { 0, -0x40, 0, 0 }, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0, 0 }, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0, 0 }, { 0x08, 0x08, 0xFF, 0 } },
    };
    TrailPoint second[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, first, 3, 8, 20);
    }
    Fighter_addEntity(this, child);
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, second, 2, 8, 15);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special3 effect of vtable D_8006870C at the fighter. */
void func_80084E58(Fighter *this) {
    Vec3 vec;
    Entity *child;

    Fighter_getPosition(this, &vec);
    child = allocEntityMemory(0xDC);
    if (child != NULL) {
        func_80085B7C(entityOperatorNewAt(0xDC, child), this, &vec);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special3 effect of vtable D_800686E4 at the fighter. */
void func_80084EC0(Fighter *this) {
    Vec3 vec;
    Entity *child;

    Fighter_getPosition(this, &vec);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800862C0(entityOperatorNewAt(0x54, child), this, &vec);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_80068654);

/* Spawns the special move 3's hitbox at the fighter's node 18 (moved by D_80068654 and 256 forward), standing still
 * with arg in its velocity's pad. */
void Dukemon_spawnSpecial3Hitbox(Fighter *this, s16 arg) {
    Fight *fight;
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    fight = Fighter_getScene(this);
    Fighter_getNodePosition(this, (VECTOR *)&position, 18, &D_80068654);
    position.vx += Fighter_getFacing(this) * 256;
    velocity.vx = 0;
    velocity.vy = 0;
    velocity.vz = 0;
    velocity.pad = arg;
    child = allocEntityMemory(0xB8);
    if (child != NULL) {
        DukemonSpecial3Hitbox_init(entityOperatorNewAt(0xB8, child), this, &position, &velocity, fight);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the attack 3's trails (as Dukemon_spawnSpecial2Trails) and plays its sound. */
void Dukemon_spawnAttack3Trails(Fighter *this) {
    TrailPoint first[3] = {
        { 0, { 0, -0x40, 0, 0 }, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0, 0 }, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0, 0 }, { 0x08, 0x08, 0xFF, 0 } },
    };
    TrailPoint second[2] = { D_800685DC_ENTRIES };
    Entity *child;

    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, first, 3, 8, 20);
    }
    Fighter_addEntity(this, child);
    child = allocEntityMemory(0x690);
    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, second, 2, 8, 15);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0x7C);
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial1Effect_init);

/* The special move 1 effect's destructor. */
void DukemonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &DUKEMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial1Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial1Effect_draw);

/* The special move 2's effect (0x34 bytes): it follows the fighter for 21 frames, fading from frame 13. */
typedef struct DukemonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ u16 age;       /* frames */
    /* 0x1E */ u8 brightness; /* 0x80 at first, - 0x10 a frame from frame 13 */
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x22 */ u8 unk22[0x24 - 0x22];
    /* 0x24 */ Vec3 position;
    /* 0x30 */ u8 unk30[0x34 - 0x30];
} DukemonSpecial2Effect;

/* Builds the special move 2's effect at the position, and plays its sound. */
DukemonSpecial2Effect *DukemonSpecial2Effect_init(DukemonSpecial2Effect *this, Fighter *fighter, Vec3 *position,
                                                  Fight *fight) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &DUKEMON_SPECIAL2_EFFECT_VTABLE;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = fighter;
    this->age = 0;
    this->brightness = 0x80;
    /* The match depends on reading the position whole before storing it (a struct copy interleaves). */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    Fighter_playSound(fighter, 0x7A);
    return this;
}

/* The special move 2 effect's destructor. */
void DukemonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &DUKEMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Follows the fighter, fades from frame 13 and deletes itself after frame 20. */
void DukemonSpecial2Effect_tick(DukemonSpecial2Effect *this) {
    Fighter_getPosition(this->fighter, &this->position);
    if (this->age >= 13) {
        this->brightness -= 0x10;
    }
    if (this->age >= 21) {
        entityDelete(&this->entity);
    } else {
        this->age++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial2Effect_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_80068664);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", func_80085B7C);

/* The destructor of the special3 effect of vtable D_8006870C. */
void func_80085C90(Entity *this, s32 flags) {
    this->vtable = &D_8006870C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class of vtable D_8006870C (0xDC bytes, func_80085B7C constructs it for the character): a point of
 * the owner that rises and fades out in steps. */
typedef struct Unk80085B7C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Dukemon *owner;
    /* 0x1C */ u8 unk1C;    /* 0x80 from func_80085B7C, eased to 0 */
    /* 0x1D */ u8 unk1D;    /* 0 from func_80085B7C, eased to 0x84 */
    /* 0x1E */ s16 side;    /* Fighter_getFacing of the owner */
    /* 0x20 */ u16 timer;   /* counts the frames */
    /* 0x22 */ u16 state;
    /* 0x24 */ u16 counter; /* frames in state 2 */
    /* 0x26 */ s16 vramY;   /* Fighter_getVramY of the owner */
    /* 0x28 */ s16 unk28;
    /* 0x2A */ u8 unk2A[0x34 - 0x2A];
    /* 0x34 */ VECTOR position; /* the owner's point 18, by Fighter_getNodePosition */
    /* 0x44 */ VECTOR unk44;    /* position, 128 toward side */
    /* 0x54 */ s32 unk54;       /* 0x1000 from func_80085B7C, eased to 0x3400 */
    /* 0x58 */ s32 unk58;       /* 0x1800 from func_80085B7C, a quarter less every frame of state 3 */
    /* 0x5C */ SVECTOR points[16]; /* 192 apart toward side, from func_80085B7C */
} Unk80085B7C; /* size 0xDC */

extern VECTOR D_80068674;

INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_80068674);

/* The tick of Unk80085B7C: follows the owner's point 18 moved by D_80068674 and eases unk1C and unk1D; then by
 * state: grows unk54 toward 0x3400 while bobbing up and down, waits 20 frames, shrinks unk58 below 5 and deletes
 * itself. It goes to state 3 early when the owner's special3 is over. */
void func_80085CE4(Unk80085B7C *this) {
    VECTOR offset = D_80068674;

    Fighter_getNodePosition(&this->owner->base, &this->position, 18, &offset);
    this->unk44.vy = this->position.vy;
    this->unk44.vz = this->position.vz;
    this->unk44.vx = this->position.vx + (this->side << 7);
    this->unk1C += (0 - this->unk1C) >> 2;
    this->unk1D += (0x84 - this->unk1D) >> 2;
    if (this->state < 3 && this->owner->special3Active == 0) {
        this->state = 3;
    }
    switch (this->state) {
    case 0:
        this->state++;
        /* fallthrough */
    case 1:
        this->unk44.vy = (COS_SIN_TABLE[(this->timer << 10) & 0xC00].sin >> 7) + this->position.vy;
        this->unk54 += (0x3400 - this->unk54) >> 3;
        if (this->unk54 < 0x3300) {
            break;
        }
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 2:
        this->counter++;
        if (this->counter < 20) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 3:
        this->unk58 -= this->unk58 >> 2;
        if (this->unk58 >= 5) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 4:
        entityDelete(&this->entity);
        return;
    }
    this->timer++;
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", func_80085EF4);

/* The entity class of virtual table D_800686E4 (func_800862C0, 0x54 bytes): eight values with a scale
 * each. */
typedef struct Unk800862C0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Dukemon *fighter;
    /* 0x1C */ u8 shade;        /* 0x80, eased to 0 once every part is done or the move ends */
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u16 frames;
    /* 0x20 */ u16 states[8];   /* per part: 0 starting, 1 shrinking, then done */
    /* 0x30 */ s16 vramY;       /* Fighter_getVramY of the character */
    /* 0x32 */ u16 count;       /* parts out: one more every 4 frames, up to 8 */
    /* 0x34 */ s32 scales[8];   /* per part: from 0x2800 eased to 0x1000 */
} Unk800862C0;

/* Constructs the entity of virtual table D_800686E4 for fighter (the position is not used): 0x80 at 0x1C,
 * and its eight values at 0 with the scale 0x2800. */
Unk800862C0 *func_800862C0(Unk800862C0 *this, Fighter *fighter, Vec3 *position) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800686E4;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = (Dukemon *)fighter;
    this->frames = 0;
    this->count = 0;
    this->shade = 0x80;
    for (i = 0; i < 8; i++) {
        this->scales[i] = 0x2800;
        this->states[i] = 0;
    }
    return this;
}

/* The destructor of the special3 effect of vtable D_800686E4. */
void func_8008634C(Entity *this, s32 flags) {
    this->vtable = &D_800686E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of func_800862C0's entity: one more of its 8 parts every 4 frames; each part's scale eases a quarter of
 * the way from 0x2800 to 0x1000 and starts again until frame 50, then the part is done; once none is left
 * shrinking, or the move ends, the shade eases to 0 and the entity is deleted under 4. The match depends on case 0
 * setting active as well: the game stores its 1 from active's register. */
void func_800863A0(Unk800862C0 *this) {
    s32 active = 0;
    s32 i;

    if ((this->frames & 3) == 0) {
        this->count++;
        if (this->count > 8) {
            this->count = 8;
        }
    }
    for (i = 0; i < this->count; i++) {
        switch (this->states[i]) {
        case 0:
            active = 1;
            this->states[i] = 1;
            /* fall through */
        case 1:
            this->scales[i] += (0x1000 - this->scales[i]) >> 2;
            active = 1;
            if (this->scales[i] <= 0x1000) {
                if (this->frames < 50) {
                    this->scales[i] = 0x2800;
                    this->states[i] = 0;
                } else {
                    this->states[i]++;
                }
            }
            break;
        }
    }
    if (!active || !this->fighter->special3Active) {
        this->shade += (0 - this->shade) >> 2;
        if (this->shade < 4) {
            entityDelete(&this->entity);
            return;
        }
    }
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", func_80086530);

/* The special move 3's hitbox (DukemonSpecial3Hitbox_init, 0xB8 bytes): flies from the fighter's node 18 by
 * velocity, fades once it has gone 0x2800 or the move ends. */
typedef struct DukemonSpecial3Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Dukemon *fighter;
    /* 0x84 */ VECTOR position;  /* the fighter's node 18 moved by D_80068674 at first, then by velocity */
    /* 0x94 */ Vec3 velocity;    /* (facing << 7, 0, 0) */
    /* 0xA0 */ s32 unkA0;
    /* 0xA4 */ u8 shade;         /* loses a quarter a tick once it fades; deleted under 5 */
    /* 0xA6 */ s16 facing;
    /* 0xA8 */ u16 unkA8;        /* the spawner's argument (velocity's pad) */
    /* 0xAA */ u16 frames;
    /* 0xAC */ u16 state;        /* DUKEMON_SPECIAL3_HITBOX_* */
    /* 0xAE */ u16 vramY;
    /* 0xB0 */ s32 distance;     /* gone so far (+= |speed|) */
    /* 0xB4 */ s32 speed;        /* the velocity's length */
} DukemonSpecial3Hitbox;

/* DukemonSpecial3Hitbox's states. */
enum {
    DUKEMON_SPECIAL3_HITBOX_FLY = 0,
    DUKEMON_SPECIAL3_HITBOX_FADE = 1,
    DUKEMON_SPECIAL3_HITBOX_DONE = 2
};

/* Builds the special move 3's hitbox of fighter: hit 0x5000300 with velocity's knockback, a 0xFF sphere at position
 * in its side's list of fight, placed at the fighter's node 18 moved by D_80068674, flying forward at 0x80 a tick;
 * velocity's pad is kept in unkA8. */
DukemonSpecial3Hitbox *DukemonSpecial3Hitbox_init(DukemonSpecial3Hitbox *this, Fighter *fighter, Vec3 *position,
                                                  VECTOR *velocity, Fight *fight) {
    VECTOR offset;
    VECTOR flight;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, 0x5000300, (Vec3 *)velocity, 0);
    this->parts.base.vtable = DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &DUKEMON_SPECIAL3_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0xFF, &this->parts.base);
    offset = D_80068674;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = (Dukemon *)fighter;
    Fighter_getNodePosition(fighter, &this->position, 18, &offset);
    this->unkA8 = velocity->pad;
    this->unkA0 = 0x1000;
    flight.vx = Fighter_getFacing(fighter) << 7;
    flight.vy = 0;
    flight.vz = 0;
    this->velocity.vx = flight.vx;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->distance = 0;
    this->frames = 0;
    this->shade = 0x80;
    this->state = DUKEMON_SPECIAL3_HITBOX_FLY;
    this->speed = vecGetLength(&flight);
    this->facing = Fighter_getFacing(fighter);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(&this->fighter->base) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The special move 3 hitbox's destructor. */
void DukemonSpecial3Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &DUKEMON_SPECIAL3_HITBOX_VTABLE;
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

/* The special move 3 hitbox's tick: moves by velocity; flies until it has gone 0x2800 or the move ends, then fades
 * by a quarter a tick and is deleted under 5; its sphere follows it, disabled once the fighter's special3Landed is
 * set. */
void DukemonSpecial3Hitbox_tick(DukemonSpecial3Hitbox *this) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (this->state == DUKEMON_SPECIAL3_HITBOX_FLY && !this->fighter->special3Active) {
        this->state = DUKEMON_SPECIAL3_HITBOX_FADE;
    }
    switch (this->state) {
    case DUKEMON_SPECIAL3_HITBOX_FLY:
        this->distance += ABS(this->speed);
        if (this->distance <= 0x2800) {
            break;
        }
        this->state++;
        /* fall through */
    case DUKEMON_SPECIAL3_HITBOX_FADE:
        this->shade -= this->shade >> 2;
        if (this->shade >= 5) {
            break;
        }
        this->state++;
        /* fall through */
    case DUKEMON_SPECIAL3_HITBOX_DONE:
        entityDelete(&this->parts.entity);
        return;
    }
    {
        s32 px, py, pz;

        px = this->position.vx;
        py = this->position.vy;
        pz = this->position.vz;
        this->parts.part.position.vx = px;
        this->parts.part.position.vy = py;
        this->parts.part.position.vz = pz;
    }
    if (this->fighter->special3Landed) {
        this->parts.part.flags.bits.disabled = 1;
        this->parts.part.flags.bits.touched = 0;
    }
    this->frames++;
}

/* The special move 3 hitbox's hit: Hit_apply; the first fighter it hits while the fighter's special3Landed is
 * clear, unless it guards, is knocked forward and up by its mass (0x45B0A900, kind 3) with a burst of 4 (spread
 * velocity.vx >> 1, size 0x120) at its position, and special3Landed is set; the sphere is disabled then. The match
 * depends on push scaled by the facing in place and stored after the lift (its store sinks to the call, as in the
 * game). */
void DukemonSpecial3Hitbox_hit(DukemonSpecial3Hitbox *this, CollisionSphere *sum) {
    Fighter *target;
    Vec3 position;
    Vec3 knockback;
    s32 spread;
    s32 push;
    Entity *child;

    Hit_apply(&this->parts.base, sum);
    target = (Fighter *)getHitFighter(sum);
    if (target == NULL || this->fighter->special3Landed) {
        return;
    }
    if (!Fighter_isGuarding(target)) {
        this->parts.part.flags.bits.disabled = 1;
        spread = this->velocity.vx >> 1;
        Fighter_getPosition(target, &position);
        push = ((s64)target->mass * 0x6AA4) >> 16;
        push *= Fighter_getFacing(&this->fighter->base);
        knockback.vy = -(((s64)target->mass * 0x5550) >> 16);
        knockback.vx = push;
        knockback.vz = 0;
        Fighter_takeHit(target, 0x45B0A900, &knockback, 3);
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            ParticleBurst_init(entityOperatorNewAt(0x3C, child), 4, &position, spread, 0x120, 4, 0x808080);
        }
        entityAddSibling(&this->parts.entity, child);
        this->fighter->special3Landed = 1;
    }
    this->parts.part.flags.bits.touched = 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial3Hitbox_draw);

INCLUDE_RODATA_ALIGN8("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_800686E4);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_8006870C);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_VTABLE);
