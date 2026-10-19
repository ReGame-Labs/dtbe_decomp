/* Tailmon (CHARACTER_TAILMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's hitbox and effect, and the special move 2's effect. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

struct TailmonSpecial1Effect *TailmonSpecial1Effect_init(struct TailmonSpecial1Effect *this, Fighter *fighter,
                                                         Fight *fight, VECTOR *position);
Entity *TailmonSpecial1Hitbox_init(Entity *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, Fight *fight);
extern FaceEntry TAILMON_FACES[];
extern EntityVtable TAILMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable TAILMON_SPECIAL1_EFFECT_VTABLE;
extern EntityVtable TAILMON_VTABLE;

/* The special move 1's hitbox: it flies forward with a sound, which its destructor stops. */
typedef struct TailmonSpecial1Hitbox {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Hit unk18;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ CollisionSphere unk38;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ u8 unk84[0xC8 - 0x84];
    /* 0x0C8 */ Vec3 unkC8;
    /* 0x0D4 */ u8 unkD4[0x118 - 0xD4];
    /* 0x118 */ Vec3 position;
    /* 0x124 */ u8 unk124[0x128 - 0x124];
    /* 0x128 */ Vec3 velocity;
    /* 0x134 */ u8 unk134[0x158 - 0x134];
    /* 0x158 */ s32 speed; /* the length of velocity */
    /* 0x15C */ u32 soundHandle; /* Fighter_playSound's (sound 0xF2), which the destructor stops */
} TailmonSpecial1Hitbox;
extern VtableEntry TAILMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable TAILMON_SPECIAL1_HITBOX_VTABLE;
extern VECTOR D_800D345C; /* {0, -70, 40, 0x1000} */

/* Builds Tailmon: a Fighter with its own vtable and faces, 0x4A4 0. */
Tailmon *Tailmon_init(Tailmon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &TAILMON_VTABLE;
    Fighter_setFaceTable(&this->base, TAILMON_FACES);
    this->unk4A4 = 0;
    return this;
}

/* Tailmon's destructor. */
void Tailmon_destroy(Entity *this, s32 flags) {
    this->vtable = &TAILMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: clears 0x4A4. */
void Tailmon_endMove(Tailmon *this) {
    this->unk4A4 = 0;
}

/* Dash attack: Fighter's, with dust (Fighter_spawnDust(1, 80, 8)) at frame 0. */
void Tailmon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 0) {
        Fighter_spawnDust(this, 1, 80, 8);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", Tailmon_throw);

void Tailmon_spawnSpecial1Hitbox(Fighter *this);
void Tailmon_spawnSpecial1Effect(Fighter *this);

/* Special move 1: frame 0 its clip, the effect and the voice; frame 5 the hitbox and func_80079B0C(0x19). */
void Tailmon_special1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        Tailmon_spawnSpecial1Effect(this);
        Fighter_playVoice(this, 0xF5);
        break;
    case 5:
        Tailmon_spawnSpecial1Hitbox(this);
        func_80079B0C(this, 0x19);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", Tailmon_special2);

/* Special move 1 in the air: frame 0 its clip, the effect and the voice; frame 5 a push back by 0x3000, the hitbox and
 * func_80079B0C(0x19); until frame 14 the force brakes a sixteenth of the horizontal velocity and all of a fall. */
void Tailmon_airSpecial1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 2, 0, -1);
        Tailmon_spawnSpecial1Effect(this);
        Fighter_playVoice(this, 0xF5);
        break;
    case 5:
        this->force.vx -= Fighter_getFacing(this) * 0x3000;
        Tailmon_spawnSpecial1Hitbox(this);
        func_80079B0C(this, 0x19);
        break;
    }
    if (frame < 14) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

struct TailmonSpecial2Effect *TailmonSpecial2Effect_init(struct TailmonSpecial2Effect *this, Fighter *fighter,
                                                         Vec3 *position);

/* Spawns the special move 2's effect 0xF5 behind the fighter. */
void Tailmon_spawnSpecial2Effect(Fighter *this) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    if (Fighter_getFacing(this) > 0) {
        position.vx -= 0xF5;
    } else {
        position.vx += 0xF5;
    }
    child = allocEntityMemory(0x34);
    if (child != NULL) {
        TailmonSpecial2Effect_init(entityOperatorNewAt(0x34, child), this, &position);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the special move 1's hitbox 0xB0 above the fighter, flying 48 a frame forward, then func_80079B0C(40). */
void Tailmon_spawnSpecial1Hitbox(Fighter *this) {
    Fight *fight;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    fight = Fighter_getScene(this);
    Fighter_getPosition(this, &position);
    position.vy -= 0xB0;
    velocity.vx = Fighter_getFacing(this) * 48;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        TailmonSpecial1Hitbox_init(entityOperatorNewAt(0x160, child), this, &position, &velocity, fight);
    }
    Fighter_addEntity(this, child);
    func_80079B0C(this, 40);
}

/* Spawns the special move 1's effect at the fighter's point 9, moved by D_800D345C. */
void Tailmon_spawnSpecial1Effect(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    VECTOR position;
    Entity *child;

    Fighter_getNodePosition(this, &position, 9, &D_800D345C);
    child = allocEntityMemory(0x74);
    if (child != NULL) {
        TailmonSpecial1Effect_init(entityOperatorNewAt(0x74, child), this, fight, &position);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial1Hitbox_init);

/* The special move 1 hitbox's destructor: first stops its sound if it plays. */
void TailmonSpecial1Hitbox_destroy(TailmonSpecial1Hitbox *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = TAILMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &TAILMON_SPECIAL1_HITBOX_VTABLE;
    if (this->soundHandle != 0) {
        stopSound(this->soundHandle);
    }
    this->unk38.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.link;
    this->unk38.link.next->prev = this->unk38.link.prev;
    link->prev->next = this->unk38.link.next;
    this->unk38.link.next = link;
    this->unk38.link.prev = link;
    this->unk18.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial1Hitbox_tick);

/* Fighter_isGuarding of a target, after taking its position (Fighter_getPosition), which is not used. */
static inline s32 func_800B55EC_targetTest(Fighter *target) {
    Vec3 position;

    Fighter_getPosition(target, &position);
    return Fighter_isGuarding(target);
}

/* On a hit: Hit_apply with sum; a hit spark on the fighter it hit when that one guards (Fighter_isGuarding), else a
 * burst of 6 (ParticleBurst_init) at 0xC8; then the hitbox deletes itself. */
void TailmonSpecial1Hitbox_hit(TailmonSpecial1Hitbox *this, CollisionSphere *sum) {
    Entity *target;
    Entity *child;

    Hit_apply(&this->unk18, sum);
    target = getHitFighter(sum);
    /* The match depends on the target test as an inline beside the null test (an if/else, one delete). */
    if (target != NULL && func_800B55EC_targetTest((Fighter *)target)) {
        entityAddSibling(&this->entity, createHitSpark(target, &this->velocity, 0x60));
    } else {
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->unkC8, this->speed >> 1, 0x60, 6, 0x808080);
        }
        entityAddSibling(&this->entity, child);
    }
    entityDelete(&this->entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial1Hitbox_draw);

/* The special move 1's effect, made for a character at a position. */
typedef struct TailmonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *fight;
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 vramY;  /* the character's 0x440 (Fighter_getVramY) */
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 facing; /* Fighter_getFacing of the character: 1 or -1 */
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ VECTOR position;
    /* 0x44 */ VECTOR start; /* the position it was made at */
    /* 0x54 */ SVECTOR corners[4];
} TailmonSpecial1Effect;

/* Builds the special move 1's effect for fighter and fight at position, with the corners of a 180 by 180 square 16 in
 * front. */
TailmonSpecial1Effect *TailmonSpecial1Effect_init(TailmonSpecial1Effect *this, Fighter *fighter, Fight *fight,
                                                  VECTOR *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &TAILMON_SPECIAL1_EFFECT_VTABLE;
    this->fighter = fighter;
    this->fight = fight;
    this->facing = Fighter_getFacing(fighter);
    this->unk24 = 0;
    this->unk28 = 0;
    this->unk2A = 0;
    this->unk30 = 0;
    this->unk2C = 0;
    this->unk20 = 0x80;
    this->unk21 = 0x80;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->start.vx = x;
    this->start.vy = y;
    this->start.vz = z;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->vramY = Fighter_getVramY(fighter);
    /* The match depends on these chains: they store from right to left */
    this->corners[0].vy = this->corners[1].vy = this->corners[0].vx = this->corners[2].vx = -90;
    this->corners[2].vy = this->corners[3].vy = this->corners[1].vx = this->corners[3].vx = 90;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -16;
    return this;
}

/* The special move 1 effect's destructor. */
void TailmonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &TAILMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial1Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial1Effect_draw);

/* The special move 2's effect, made for a character at a position. */
typedef struct TailmonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 age;    /* + 2 a frame; the tick deletes the effect past 0x20 */
    /* 0x1A */ s16 vramY;  /* the character's 0x440 (Fighter_getVramY) */
    /* 0x1C */ s16 facing; /* Fighter_getFacing of the character: 1 or -1 */
    /* 0x1E */ u8 r;       /* the draw's color, 0x80 each at first */
    /* 0x1F */ u8 g;
    /* 0x20 */ u8 b;
    /* 0x21 */ u8 unk21[3];
    /* 0x24 */ Vec3 position;
    /* 0x30 */ s32 unk30;
} TailmonSpecial2Effect;

/* Builds the special move 2's effect for fighter at position. */
TailmonSpecial2Effect *TailmonSpecial2Effect_init(TailmonSpecial2Effect *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &TAILMON_SPECIAL2_EFFECT_VTABLE;
    this->vramY = Fighter_getVramY(fighter);
    this->facing = Fighter_getFacing(fighter);
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->age = 0;
    /* The match depends on this chain: it stores from right to left */
    this->r = this->g = this->b = 0x80;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The special move 2 effect's destructor. */
void TailmonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &TAILMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Darkens the effect's color as it ages by 2 a frame, then deletes it once its age passes 0x20. */
void TailmonSpecial2Effect_tick(TailmonSpecial2Effect *this) {
    s32 level;
    s32 fade;
    s32 r, g, b;

    level = this->age + 8;
    this->r = 0x80 - level * 2;
    this->g = 0x80 - level * 4;
    this->b = 0x80 - level * 8;
    fade = 0x80 - this->age * 4;
    r = (this->r * fade) >> 7;
    g = (this->g * fade) >> 7;
    b = (this->b * fade) >> 7;
    /* The match depends on the clamps testing >= 0 first (the game branches to the 0 arm placed last) */
    if (r >= 0) {
        if (r > 0xFF) {
            r = 0xFF;
        }
    } else {
        r = 0;
    }
    if (g >= 0) {
        if (g > 0xFF) {
            g = 0xFF;
        }
    } else {
        g = 0;
    }
    if (b >= 0) {
        if (b > 0xFF) {
            b = 0xFF;
        }
    } else {
        b = 0;
    }
    this->r = r;
    this->g = g;
    this->b = b;
    if (this->age > 0x20) {
        entityDelete(&this->entity);
        return;
    }
    this->age += 2;
}

INCLUDE_ASM("game/nonmatchings/fighters/tailmon", TailmonSpecial2Effect_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDCC);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDD4);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDDC);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDE4);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDEC);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDF4);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BDFC);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BE0C);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", D_8006BE2C);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/tailmon", TAILMON_VTABLE);
