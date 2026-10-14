/* Dukemon (CHARACTER_DUKEMON): the fighter createFighter builds for it, its moves, and the trails and entities they
 * spawn. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"

Entity *DukemonSpecial1Effect_init(Entity *this, Fighter *arg, Fight *arg2);
Entity *func_80085B7C(Entity *this, Fighter *arg, Vec3 *vec);
struct Unk800862C0 *func_800862C0(struct Unk800862C0 *this, Fighter *fighter, Vec3 *position);
struct DukemonSpecial2Effect *DukemonSpecial2Effect_init(struct DukemonSpecial2Effect *this, Fighter *fighter,
                                                          Vec3 *position, Fight *fight);
Entity *DukemonSpecial3Hitbox_init(Entity *this, Fighter *fighter, Vec3 *position, VECTOR *velocity, Fight *obj);
extern VtableEntry DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable DUKEMON_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable D_800686E4;
extern EntityVtable D_8006870C;
extern EntityVtable DUKEMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable DUKEMON_SPECIAL1_EFFECT_VTABLE;
extern EntityVtable DUKEMON_VTABLE;

typedef struct Dukemon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ u16 special3Active; /* set every frame of special3, cleared by endMove; its effects end with it */
} Dukemon;
extern VECTOR D_80068654;

/* Builds Dukemon: a Fighter with its own vtable. */
Dukemon *Dukemon_init(Dukemon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &DUKEMON_VTABLE;
    this->unk4A4 = 0;
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

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", Dukemon_attack3);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", Dukemon_throw);

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
    { 0x14, { 0, 0x180, 0 }, 0, { 0xFF, 0x80, 0x80, 0 } }, \
    { 0x14, { 0, 0, 0 }, 0, { 0, 0, 0, 0 } }

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
        { 0, { 0, -0x40, 0 }, 0, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0 }, 0, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0 }, 0, { 0x08, 0x08, 0xFF, 0 } },
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
        { 0, { 0, -0x40, 0 }, 0, { 0xFF, 0x80, 0x20, 0 } },
        { 3, { 0, 0, 0 }, 0, { 0x80, 0x40, 0xFF, 0 } },
        { 1, { 0, 0, 0 }, 0, { 0x08, 0x08, 0xFF, 0 } },
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
    /* 0x5C */ s16 unk5C[16][4];
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
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ s16 unk1E;
    /* 0x20 */ s16 unk20[8];
    /* 0x30 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x32 */ s16 unk32;
    /* 0x34 */ s32 unk34[8];
} Unk800862C0;

/* Constructs the entity of virtual table D_800686E4 for fighter (the position is not used): 0x80 at 0x1C,
 * and its eight values at 0 with the scale 0x2800. */
Unk800862C0 *func_800862C0(Unk800862C0 *this, Fighter *fighter, Vec3 *position) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800686E4;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = fighter;
    this->unk1E = 0;
    this->unk32 = 0;
    this->unk1C = 0x80;
    for (i = 0; i < 8; i++) {
        this->unk34[i] = 0x2800;
        this->unk20[i] = 0;
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

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", func_800863A0);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", func_80086530);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial3Hitbox_init);

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

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial3Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial3Hitbox_hit);

INCLUDE_ASM("game/nonmatchings/fighters/dukemon", DukemonSpecial3Hitbox_draw);

INCLUDE_RODATA_ALIGN8("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_800686E4);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", D_8006870C);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/dukemon", DUKEMON_VTABLE);
