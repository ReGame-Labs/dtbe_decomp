/* Beelzebumon (CHARACTER_BEELZEBUMON): the fighter createFighter builds for it, its moves, and the entities they
 * spawn: the throw's effect, the special move 1's projectile (with its sparks and flashes), the special move 2's
 * hitbox and particles, and the special move 3's hitboxes and effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/camera.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "gte.h"

struct Unk800971E0 *func_800971E0(struct Unk800971E0 *this, Fighter *fighter, s32 arg2, Fight *fight);
struct BeelzebumonSpecial2Hitbox *BeelzebumonSpecial2Hitbox_init(struct BeelzebumonSpecial2Hitbox *this,
                                                                 Fighter *fighter, Fight *fight, s32 facing);
struct BeelzebumonSpecial1Projectile *BeelzebumonSpecial1Projectile_init(struct BeelzebumonSpecial1Projectile *this,
                                                                         Fighter *owner, Fight *fight, VECTOR *pos,
                                                                         VECTOR *vel);
struct Unk80096F6C *func_80096F6C(struct Unk80096F6C *this, Fighter *fighter);
Unk80097FA0 *func_80097FA0(Unk80097FA0 *this, Fighter *fighter, Fight *fight);
struct Unk80098720 *func_80098720(struct Unk80098720 *this, Fighter *fighter, Vec3 *position);
struct Unk80098A78 *func_80098A78(struct Unk80098A78 *this, Fighter *fighter, Fight *fight, Vec3 *position,
                                 VECTOR *velocity);
extern VECTOR D_80069964;
extern EntityVtable BEELZEBUMON_SPECIAL3_EFFECT_VTABLE;
extern EntityVtable D_80069A04;
extern EntityVtable D_80069A2C;
extern EntityVtable D_80069A54;
extern EntityVtable D_80069A7C;
extern EntityVtable BEELZEBUMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable D_80069AF4;
extern EntityVtable D_80069B6C;
extern EntityVtable BEELZEBUMON_THROW_EFFECT_VTABLE;
extern EntityVtable BEELZEBUMON_VTABLE;

/* Beelzebumon's class (CHARACTER_BEELZEBUMON). */
typedef struct Beelzebumon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC; /* Beelzebumon_endMove sets it */
} Beelzebumon;

/* Builds Beelzebumon: a Fighter with its own vtable. */
Beelzebumon *Beelzebumon_init(Beelzebumon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &BEELZEBUMON_VTABLE;
    this->unk4A4 = 0;
    return this;
}

/* Beelzebumon's destructor. */
void Beelzebumon_destroy(Entity *this, s32 flags) {
    this->vtable = &BEELZEBUMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Fighter's input step, then unk4A4 counts down; while it runs and the clip is attack 1 or 2 it is set back to 35. */
void Beelzebumon_handleInput(Beelzebumon *this, void *input) {
    s32 clip;

    Fighter_handleInput(this, input);
    if (this->unk4A4 != 0) {
        this->unk4A4--;
        if (this->unk4A4 != 0) {
            clip = func_80079E2C(&this->base);
            /* The match depends on the two tests being nested ifs: && folds them into one range test. */
            if (clip < FIGHTER_CLIP_ATTACK3) {
                if (clip >= FIGHTER_CLIP_ATTACK1) {
                    this->unk4A4 = 35;
                }
            }
        }
    }
}

/* Ends a move: sets unk4AC. */
void Beelzebumon_endMove(Beelzebumon *this) {
    this->unk4AC = 1;
}

/* Up attack: Fighter's, and at frame 4 a push forward by the attack's lungeX (Fighter_getParams) times the mass. */
void Beelzebumon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 4) {
        s32 push = ((s64)Fighter_getParams(this)->attacks[4].lungeX * this->mass) >> 16;

        this->force.vx += push * Fighter_getFacing(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", Beelzebumon_throw);

struct BeelzebumonThrowEffect *BeelzebumonThrowEffect_init(struct BeelzebumonThrowEffect *this, Fighter *fighter,
                                                           Vec3 *position, s32 facing, s32 arg4);

/* Spawns the throw's effect 0x133 above the fighter, with its facing and arg. */
void Beelzebumon_spawnThrowEffect(Fighter *this, s32 arg) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vy -= 0x133;
    child = allocEntityMemory(0x40);
    if (child != NULL) {
        BeelzebumonThrowEffect_init(entityOperatorNewAt(0x40, child), this, &position, Fighter_getFacing(this), arg);
    }
    Fighter_addEntity(this, child);
}

void func_800955D4(Fighter *this, s32 arg);
void Beelzebumon_spawnSpecial1Projectile(Fighter *this);
void func_80095714(Fighter *this);
void func_800957C0(Fighter *this);

/* Special move 1: Fighter's; the voice at frame 0, func_800955D4(16) at frame 4, func_80095714 at frame 7, the
 * projectile and func_800957C0 at frame 12, a recoil back at frame 13 when there is floor behind; from frame 14 the
 * horizontal velocity is braked by a quarter, with dust until frame 16. */
void Beelzebumon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xB3);
    }
    if (frame == 4) {
        func_800955D4(this, 16);
    }
    if (frame == 7) {
        func_80095714(this);
    }
    if (frame == 12) {
        Beelzebumon_spawnSpecial1Projectile(this);
        func_800957C0(this);
    }
    if (frame == 13 && Fighter_hasFloorAhead(this, -0x100) == 1) {
        s32 push = ((s64)this->mass * 0x238C) >> 16;

        this->force.vx -= push * Fighter_getFacing(this);
    }
    if (frame >= 14) {
        this->force.vx = ((s64)this->velocity.vx * -0x400) >> 12;
        if (frame < 17) {
            Fighter_spawnDust(this, 1, 0x60, 2);
        }
    }
}

/* Special move 1 in the air: Fighter's; the voice at frame 0, func_800955D4(16) at frame 5, func_80095714 at frame 9,
 * the projectile and func_800957C0 at frame 14; until then the horizontal velocity loses a sixteenth a frame and a
 * fall is braked; a recoil back at frame 15, then the horizontal velocity is braked by a fifth. */
void Beelzebumon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xB3);
    }
    if (frame == 5) {
        func_800955D4(this, 16);
    }
    if (frame == 9) {
        func_80095714(this);
    }
    if (frame == 14) {
        Beelzebumon_spawnSpecial1Projectile(this);
        func_800957C0(this);
    }
    if (frame < 15) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
    if (frame == 15) {
        s32 push = ((s64)this->mass * 0x38E0) >> 16;

        this->force.vx -= push * Fighter_getFacing(this);
    }
    if (frame >= 16) {
        this->force.vx = ((s64)this->velocity.vx * -0x333) >> 12;
    }
}

/* Creates func_800971E0's entity (0x30 bytes) for the character with arg, and hands it to
 * Fighter_addEntity. */
void func_800955D4(Fighter *this, s32 arg) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        func_800971E0(entityOperatorNewAt(0x30, child), this, arg, fight);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069964);

/* Fires the special move 1's projectile from the fighter's point 6 moved by D_80069964, 192 a frame forward, with its
 * sound. */
void Beelzebumon_spawnSpecial1Projectile(Fighter *this) {
    VECTOR pos;
    VECTOR vel;
    Fight *fight;
    Entity *child;

    Fighter_getNodePosition(this, &pos, 6, &D_80069964);
    vel.vx = Fighter_getFacing(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0x15C);
    if (child != NULL) {
        BeelzebumonSpecial1Projectile_init(entityOperatorNewAt(0x15C, child), this, fight, &pos, &vel);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xAE);
}

extern VECTOR D_80069974;
Entity *func_80096CFC(void *this, Fighter *fighter, VECTOR *pos, s32 angle);

INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069974);

/* Spawns six entities of func_80096CFC (0x58 bytes) at the owner's point 6 moved by D_80069974, with angles
 * (4096ths: func_80096CFC masks it with 0xFFF) 0x100 apart from 0x100, or from 0xA00 when the character faces left. */
void func_80095714(Fighter *this) {
    VECTOR pos;
    Entity *child;
    s32 angle;
    s32 i;

    Fighter_getNodePosition(this, &pos, 6, &D_80069974);
    angle = Fighter_getFacing(this) > 0 ? 0x100 : 0xA00;
    for (i = 5; i >= 0; i--) {
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_80096CFC(entityOperatorNewAt(0x58, child), this, &pos, angle);
        }
        Fighter_addEntity(this, child);
        angle += 0x100;
    }
}

/* Creates an entity of func_80096F6C's class (0x5C bytes) with g++'s new, and hands it to Fighter_addEntity. */
void func_800957C0(Fighter *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_80096F6C(entityOperatorNewAt(0x5C, child), this);
    }
    Fighter_addEntity(this, child);
}

void Beelzebumon_spawnSpecial2Hitbox(Fighter *this);
void Beelzebumon_spawnSpecial2Particle(Fighter *this);
struct BeelzebumonSpecial2Particle *BeelzebumonSpecial2Particle_init(struct BeelzebumonSpecial2Particle *this,
                                                                     Fighter *fighter, VECTOR *pos, VECTOR *vel,
                                                                     s32 angle);

/* Special move 2: Fighter's, with the voice at frame 0, the hitbox at frame 7 and 20 particles at frame 14. */
void Beelzebumon_special2(Fighter *this, s32 frame) {
    s32 i;

    Fighter_special2(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xB4);
    }
    if (frame == 7) {
        Beelzebumon_spawnSpecial2Hitbox(this);
    }
    if (frame == 14) {
        for (i = 0; i < 20; i++) {
            Beelzebumon_spawnSpecial2Particle(this);
        }
    }
}

/* Spawns the special move 2's hitbox toward the fighter's facing, with its sound. */
void Beelzebumon_spawnSpecial2Hitbox(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0xA4);

    if (child != NULL) {
        BeelzebumonSpecial2Hitbox_init(entityOperatorNewAt(0xA4, child), this, fight, Fighter_getFacing(this));
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xAF);
}

/* Spawns one of the special move 2's particles 0x180 to 0x27F from the fighter (0x140 lower, at z 0), at a random
 * angle within a quarter turn from 0x380 (from 0x880 when it faces left), moving outward at 1/256 of the unit. */
void Beelzebumon_spawnSpecial2Particle(Fighter *this) {
    VECTOR position;
    VECTOR velocity;
    Fight *fight;
    Entity *child;
    u32 spread;
    s32 angle;
    s32 distance;
    s16 sin;
    s16 cos;

    fight = Fighter_getScene(this);
    spread = mersenneTwisterGenerate(&fight->random);
    if (Fighter_getFacing(this) > 0) {
        angle = (spread & 0x3FF) + 0x380;
    } else {
        angle = ((spread & 0x3FF) + 0x880) & 0xFFF;
    }
    distance = (mersenneTwisterGenerate(&fight->random) & 0xFF) + 0x180;
    sin = COS_SIN_TABLE[angle].sin;
    cos = COS_SIN_TABLE[angle].cos;
    Fighter_getPosition(this, (Vec3 *)&position);
    position.vx += (distance * sin) >> 12;
    position.vy += ((distance * cos) >> 12) - 0x140;
    position.vz = 0;
    velocity.vx = sin >> 8;
    velocity.vy = cos >> 8;
    velocity.vz = 0;
    child = allocEntityMemory(0x5C);
    if (child != NULL) {
        BeelzebumonSpecial2Particle_init(entityOperatorNewAt(0x5C, child), this, &position, &velocity, angle);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", Beelzebumon_special3);

/* Creates an entity of func_80097FA0's class (0xB4 bytes) with g++'s new, for this one and its
 * Fighter_getScene object, and hands it to Fighter_addEntity. */
void func_80095C44(Fighter *this) {
    Fight *unk228 = Fighter_getScene(this);
    Entity *child = allocEntityMemory(0xB4);

    if (child != NULL) {
        func_80097FA0(entityOperatorNewAt(0xB4, child), this, unk228);
    }
    Fighter_addEntity(this, child);
}

/* Spawns the entity of func_80098A78 384 ahead of and 0x140 above the character, with velocity (192
 * forward, 0, 0), then plays sound 0xB0. */
void func_80095CB4(Fighter *this) {
    s32 facing = Fighter_getFacing(this);
    Fight *unk228;
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    velocity.vx = facing * 192;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 384;
    position.vy -= 0x140;
    unk228 = Fighter_getScene(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        func_80098A78(entityOperatorNewAt(0xC8, child), this, unk228, &position, &velocity);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xB0);
}

struct BeelzebumonSpecial3Effect *BeelzebumonSpecial3Effect_init(struct BeelzebumonSpecial3Effect *this,
                                                                 Fighter *fighter, Fight *unk1C, Vec3 *position,
                                                                 Vec3 *velocity);

/* Spawns a special move 3 effect 0x800 to 0x9FF ahead of the character's top and up to 0x400 above or below it,
 * flying back at 192 a frame and a sixteenth of that height toward the top. */
void Beelzebumon_spawnSpecial3Effect(Fighter *this) {
    Fight *fight = Fighter_getScene(this);
    s32 facing = Fighter_getFacing(this);
    Vec3 position;
    Vec3 velocity;
    s32 offset;
    Entity *child;

    /* The match depends on one offset variable for both random draws */
    func_8007A650(this, &position);
    offset = (mersenneTwisterGenerate(&fight->random) & 0x1FF) + 0x800;
    position.vx += offset * facing;
    offset = (mersenneTwisterGenerate(&fight->random) & 0x7FF) - 0x400;
    velocity.vx = -(facing * 192);
    velocity.vy = -offset >> 4;
    velocity.vz = 0;
    position.vy += offset;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        BeelzebumonSpecial3Effect_init(entityOperatorNewAt(0x58, child), this, fight, &position, &velocity);
    }
    Fighter_addEntity(this, child);
}

/* Pushes the character func_80079E50 finds (AXIS_Z, 0x640000, 0x238) along x by 0x1000 (0x400 when its
 * func_80079E2C is not 0): toward this one when it is 0x221..0x9FF ahead, away from it when 1..0x1E0 ahead,
 * both within 0x57F up or down. */
void func_80095E8C(Fighter *this) {
    Fighter *target;
    Vec3 position;
    Vec3 targetPosition;
    s32 dx, dy;
    s32 ahead;
    s32 push;

    target = func_80079E50(this, &AXIS_Z, 0x640000, 0x238);
    if (target != NULL) {
        Fighter_getPosition(this, &position);
        Fighter_getPosition(target, &targetPosition);
        dx = targetPosition.vx - position.vx;
        dy = targetPosition.vy - position.vy;
        ahead = dx * Fighter_getFacing(this);
        if (func_80079E2C(target) == 0) {
            push = 0x1000;
        } else {
            push = 0x400;
        }
        if (dy >= -0x57F && dy <= 0x57F) {
            if (ahead >= 0x221 && ahead <= 0x9FF) {
                if (dx > 0) {
                    target->force.vx -= push;
                } else {
                    target->force.vx += push;
                }
            } else if (ahead >= 1 && ahead <= 0x1E0) {
                if (dx > 0) {
                    target->force.vx += push;
                } else {
                    target->force.vx -= push;
                }
            }
        }
    }
}

/* The throw's effect (0x40 bytes) at a position: 0x34 drops by 15 at each of its three ticks. */
typedef struct BeelzebumonThrowEffect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 facing; /* Fighter_getFacing of the character */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* 60, less 15 at each tick */
    /* 0x38 */ s32 ticks;  /* ticks so far */
    /* 0x3C */ s16 vramY;  /* Fighter_getVramY of the character */
} BeelzebumonThrowEffect;

/* Builds the throw's effect for fighter at position, with its facing and arg4. */
BeelzebumonThrowEffect *BeelzebumonThrowEffect_init(BeelzebumonThrowEffect *this, Fighter *fighter, Vec3 *position,
                                                    s32 facing, s32 arg4) {
    s32 vx, vy, vz;

    entityInit(&this->entity, 0);
    this->entity.vtable = &BEELZEBUMON_THROW_EFFECT_VTABLE;
    this->fighter = fighter;
    vx = position->vx;
    vy = position->vy;
    vz = position->vz;
    this->facing = facing;
    this->unk34 = 60;
    this->ticks = 0;
    this->position.vx = vx;
    this->position.vy = vy;
    this->position.vz = vz;
    this->unk30 = arg4;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The throw effect's destructor. */
void BeelzebumonThrowEffect_destroy(Entity *this, s32 flags) {
    this->vtable = &BEELZEBUMON_THROW_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Takes 15 off 0x34 at each of its 3 ticks, then deletes itself. The match depends on the unit's
 * -fno-cse-follow-jumps (Makefile): the else block reloads 0x38. */
void BeelzebumonThrowEffect_tick(BeelzebumonThrowEffect *this, s32 arg) {
    if (this->ticks >= 3) {
        entityDelete(&this->entity);
    } else {
        this->unk34 -= 15;
        this->ticks++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonThrowEffect_draw);

/* One of the four slots at 0xCC of BeelzebumonSpecial1Projectile (BeelzebumonSpecial1Projectile_tick moves them,
 * BeelzebumonSpecial1Projectile_draw draws them). */
typedef struct BeelzebumonSpecial1ProjectileSlot {
    /* 0x00 */ s16 unk0; /* nonzero while the slot is in use */
    /* 0x02 */ u16 unk2;
    /* 0x04 */ Vec3 unk4;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ Vec3 unk14;
    /* 0x20 */ s32 unk20;
} BeelzebumonSpecial1ProjectileSlot;

/* The special move 1's projectile (0x15C bytes, a Projectile) with its owner, the fight and four slots. */
typedef struct BeelzebumonSpecial1Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ Fighter *owner;
    /* 0xC4 */ Fight *fight;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ BeelzebumonSpecial1ProjectileSlot slots[4];
} BeelzebumonSpecial1Projectile;

extern EntityVtable BEELZEBUMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry BEELZEBUMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];

/* Builds the special move 1's projectile: Projectile_init (size 0x80, hit 0x4A82800, 0x1000, 3), its vtables, the owner
 * and the fight, and frees the four slots. */
BeelzebumonSpecial1Projectile *BeelzebumonSpecial1Projectile_init(BeelzebumonSpecial1Projectile *this, Fighter *owner,
                                                                  Fight *fight, VECTOR *pos, VECTOR *vel) {
    s32 i;

    Projectile_init(&this->base, owner, (Vec3 *)pos, (Vec3 *)vel, 0x80, 0x4A82800, 0x1000, 3);
    this->base.parts.base.vtable = BEELZEBUMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &BEELZEBUMON_SPECIAL1_PROJECTILE_VTABLE;
    this->owner = owner;
    this->fight = fight;
    this->unkC8 = 0;
    for (i = 3; i >= 0; i--) {
        this->slots[i].unk0 = 0;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonSpecial1Projectile_tick);
void func_80096984(BeelzebumonSpecial1Projectile *this);

/* On a hit: Hit_apply; when it struck a fighter, func_80096984's flash unless the fighter guards, and the projectile's
 * hit spark; else its burst (4); then deletes itself. */
void BeelzebumonSpecial1Projectile_hit(BeelzebumonSpecial1Projectile *this, CollisionSphere *sum) {
    Entity *target;

    Hit_apply(&this->base.parts.base, sum);
    target = getHitFighter(sum);
    if (target != NULL) {
        if (Fighter_isGuarding((Fighter *)target) == 0) {
            func_80096984(this);
        }
        Projectile_spawnHitSpark(&this->base, target);
    } else {
        Projectile_spawnBurst(&this->base, 4);
    }
    entityDelete(&this->base.parts.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonSpecial1Projectile_draw);

struct Unk800969E0 *func_800969E0(struct Unk800969E0 *this, Fighter *fighter);

/* Spawns the flash of vtable D_80069B6C for the owner, as a sibling. */
void func_80096984(BeelzebumonSpecial1Projectile *this) {
    Entity *child = allocEntityMemory(0x40);

    if (child != NULL) {
        func_800969E0(entityOperatorNewAt(0x40, child), this->owner);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* The entity class of vtable D_80069B6C (0x40 bytes, func_80096984 spawns it): it follows the position
 * func_8007A650 gives for the partner of a character for 11 frames, cycling unk38 through 0..2. */
typedef struct Unk800969E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* func_8007A650 of Fighter_getOpponent's character */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ u32 color;     /* 0xC0C0C0 */
    /* 0x30 */ s32 unk30;     /* 0xB3 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ u16 unk38;     /* cycles 0, 1, 2 */
    /* 0x3A */ s16 vramY;     /* Fighter_getVramY of the character */
    /* 0x3C */ s32 frames;
} Unk800969E0;

/* Constructs the entity of vtable D_80069B6C for a character. */
Unk800969E0 *func_800969E0(Unk800969E0 *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069B6C;
    this->fighter = fighter;
    this->color = 0xC0C0C0;
    this->unk30 = 0xB3;
    this->unk34 = 0;
    this->unk38 = 0;
    this->vramY = Fighter_getVramY(fighter);
    this->frames = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069B6C, which func_800969E0 installs. */
void func_80096A54(Entity *this, s32 flags) {
    this->vtable = &D_80069B6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800969E0's tick: follows the partner's position (func_8007A650 of Fighter_getOpponent's character); after
 * 11 frames it deletes itself, before that unk38 cycles 0..2. */
void func_80096AA8(Unk800969E0 *this) {
    func_8007A650(Fighter_getOpponent(this->fighter), &this->position);
    if (this->frames > 10) {
        entityDelete(&this->entity);
    } else {
        if (++this->unk38 >= 3) {
            this->unk38 = 0;
        }
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_80096B54);

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_80096CFC);

/* The tick of func_80096CFC's sprite entity: moves by the vector at 0x44 in color 0xA02828 for 5 frames,
 * then deletes itself (Particle).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x54,
 * and on the position read before any of it is stored. */
void func_80096DC4(Particle *this) {
    if (this->age > 4) {
        entityDelete(&this->entity);
    } else {
        s32 x = this->sprite.position.vx;
        s32 y = this->sprite.position.vy;
        s32 z = this->sprite.position.vz;

        this->sprite.color = 0xA02828;
        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->sprite.position.vx = x;
        this->sprite.position.vy = y;
        this->sprite.position.vz = z;
        this->age++;
    }
}

/* The draw of func_80096CFC's sprite entity: the sprite in the camera as a square of side 2 * size around its
 * position, with buildPolyFT4.
 * The match depends on taking the camera as the s32 the vtable passes and casting it in a local (func_800970A8). */
void func_80096E64(Particle *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    shape.flags = 0;
    size = this->sprite.size;
    corners[0].vx = corners[2].vx = this->sprite.position.vx - size;
    corners[1].vx = corners[3].vx = this->sprite.position.vx + size;
    corners[0].vy = corners[1].vy = this->sprite.position.vy - size;
    corners[2].vy = corners[3].vy = this->sprite.position.vy + size;
    corners[0].vz = corners[2].vz = corners[1].vz = corners[3].vz = this->sprite.position.vz;
    Particle_buildShape(this, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift);
}

/* The sprite entity of virtual table D_80069B1C (0x5C bytes, func_800957C0 spawns it): an Particle at the
 * character's point 6 moved by D_80069998, with the character's facing. */
typedef struct Unk80096F6C {
    /* 0x00 */ Particle base;
    /* 0x58 */ s32 facing; /* Fighter_getFacing of the character */
} Unk80096F6C;

extern SpriteAnimRow D_80069984;
extern EntityVtable D_80069B1C;
extern VECTOR D_80069998;

INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069984);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069998);

/* Constructs the sprite of D_80069B1C for a character: Particle_init with frame D_80069984, no motion, size
 * 0x33, color 0x646464 and the character's 0x440 (Fighter_getVramY) as flags. */
Unk80096F6C *func_80096F6C(Unk80096F6C *this, Fighter *fighter) {
    Particle_init(&this->base, &D_80069984, &ZERO_VECTOR, &ZERO_VECTOR, 0x33, 0x646464,
                  Fighter_getVramY(fighter));
    this->base.entity.vtable = &D_80069B1C;
    Fighter_getNodePosition(fighter, &this->base.sprite.position, 6, &D_80069998);
    this->facing = Fighter_getFacing(fighter);
    return this;
}

/* Unk80096F6C's tick (over Particle_tick): after 5 steps it deletes itself, before that its color darkens
 * with unk54 and the sprite grows by 0x33, then the base tick.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x54. */
void func_80097008(Unk80096F6C *this, s32 arg) {
    s32 level;
    s32 low;

    if (this->base.age >= 5) {
        entityDelete(&this->base.entity);
    } else {
        level = 100 - this->base.age * 20;
        low = level >> 2;
        this->base.sprite.color = low | (low << 8) | (level << 16);
        this->base.sprite.size += 0x33;
        Particle_tick(&this->base, arg);
    }
}

/* Unk80096F6C's draw: the sprite in the camera as a rectangle of half width size / 2 and half height size around
 * its position, mirrored by the character's facing, with buildPolyFT4.
 * The match depends on taking the camera as the s32 the vtable passes and casting it in a local: with a Camera *
 * param, the ot load sinks to the call and the camera and ot registers swap. */
void func_800970A8(Unk80096F6C *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 halfWidth;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    pos = this->base.sprite.position;
    size = this->base.sprite.size;
    halfWidth = (size >> 1) * this->facing;
    corners[0].vx = corners[2].vx = pos.vx - halfWidth;
    corners[1].vx = corners[3].vx = pos.vx + halfWidth;
    corners[0].vy = corners[1].vy = pos.vy - size;
    corners[2].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[2].vz = corners[1].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    Particle_buildShape(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift);
}

/* The entity class of virtual table D_80069AF4 (constructor func_800971E0). */
typedef struct Unk800971E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 vramY; /* Fighter_getVramY of the character */
} Unk800971E0;

/* Constructs the entity of virtual table D_80069AF4 for fighter with arg2 (fight, which its spawner passes, is
 * not read). */
Unk800971E0 *func_800971E0(Unk800971E0 *this, Fighter *fighter, s32 arg2, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069AF4;
    this->fighter = fighter;
    this->unk1C = 0;
    this->unk20 = 0;
    /* The match depends on storing arg2 after 0x28 (it fills the call's delay slot). */
    this->unk28 = 0;
    this->unk24 = arg2;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_80069AF4, which func_800971E0 installs. */
void func_80097250(Entity *this, s32 flags) {
    this->vtable = &D_80069AF4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800971E0's tick: once unk28 counts past unk24 it deletes itself; before that unk1C cycles 0..2,
 * unk20 falls by 0x180 and unk28 counts up. */
void func_800972A4(Unk800971E0 *this) {
    if (this->unk28 > this->unk24) {
        entityDelete(&this->entity);
    } else {
        if (++this->unk1C >= 3) {
            this->unk1C = 0;
        }
        this->unk20 -= 0x180;
        this->unk28++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_800699A8);

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_8009733C);

/* A helper of Unk800971E0's draw (func_8009733C), this not read: out is the point t/4096 of the way from
 * from to to. */
void func_800975D0(Unk800971E0 *this, SVECTOR *out, VECTOR *from, VECTOR *to, s32 t) {
    out->vx = from->vx + (((to->vx - from->vx) * t) >> 12);
    out->vy = from->vy + (((to->vy - from->vy) * t) >> 12);
    out->vz = from->vz + (((to->vz - from->vz) * t) >> 12);
}

/* The special move 2's hitbox (Beelzebumon_spawnSpecial2Hitbox). */
typedef struct BeelzebumonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Fighter *fighter;
    /* 0x88 */ Fight *fight;
    /* 0x8C */ s32 facing; /* Fighter_getFacing of the fighter when spawned */
    /* 0x90 */ s32 unk90;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ s32 unk9C;
    /* 0xA0 */ s16 vramY;  /* Fighter_getVramY of the fighter */
} BeelzebumonSpecial2Hitbox; /* size 0xA4 */

/* Builds the special move 2's hitbox for fighter: a disabled 0x140 sphere (damage 0x44AC2800, knockback facing * 2
 * and up by 1) on fighter's side of fight. */
BeelzebumonSpecial2Hitbox *BeelzebumonSpecial2Hitbox_init(BeelzebumonSpecial2Hitbox *this, Fighter *fighter,
                                                          Fight *fight, s32 facing) {
    VECTOR knockback;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &BEELZEBUMON_SPECIAL2_HITBOX_VTABLE;
    /* ZERO_VECTOR is a VECTOR; only its first three words are read here */
    TrackedHit_init(&this->parts.unk18, fighter, 0x44AC2800, (Vec3 *)&ZERO_VECTOR, 2);
    CollisionSphere_init(&this->parts.part, (Vec3 *)&ZERO_VECTOR, 0x140, &this->parts.unk18);
    knockback.vx = facing * 2;
    knockback.vy = -1;
    knockback.vz = 0;
    Hit_setKnockback(&this->parts.unk18.base, &knockback);
    this->parts.part.flags.bits.disabled = 1;
    this->parts.part.entity = &this->parts.entity;
    this->fighter = fighter;
    this->fight = fight;
    this->facing = facing;
    this->unk90 = 0xFA;
    this->unk94 = 0x533;
    this->unk98 = 0x100;
    this->unk9C = 0;
    this->vramY = Fighter_getVramY(fighter);
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The special move 2 hitbox's destructor. */
void BeelzebumonSpecial2Hitbox_destroy(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &BEELZEBUMON_SPECIAL2_HITBOX_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonSpecial2Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonSpecial2Hitbox_draw);

/* The special move 2's particle (Beelzebumon_spawnSpecial2Particle spawns 20): a Particle with an angle. */
typedef struct BeelzebumonSpecial2Particle {
    /* 0x00 */ Particle base;
    /* 0x58 */ s32 angle; /* in 4096ths (masked with 0xFFF) */
} BeelzebumonSpecial2Particle;

extern EntityVtable BEELZEBUMON_SPECIAL2_PARTICLE_VTABLE;

/* Builds a special move 2 particle: Particle_initById with cell 9 at pos moving by vel, size 2, then the angle (the
 * fighter is not read). */
BeelzebumonSpecial2Particle *BeelzebumonSpecial2Particle_init(BeelzebumonSpecial2Particle *this, Fighter *fighter,
                                                              VECTOR *pos, VECTOR *vel, s32 angle) {
    Particle_initById(&this->base, 9, pos, vel, 2, 0, 0);
    this->base.entity.vtable = &BEELZEBUMON_SPECIAL2_PARTICLE_VTABLE;
    this->angle = angle & 0xFFF;
    return this;
}

/* Deletes itself after 11 frames; before that the sprite grows by 7 up to 25, moves by its velocity and its red darkens
 * with its age. */
void BeelzebumonSpecial2Particle_tick(BeelzebumonSpecial2Particle *this) {
    s32 x, y, z;
    s32 level;
    s32 low;

    if (this->base.age > 10) {
        entityDelete(&this->base.entity);
    } else {
        this->base.sprite.size += 7;
        if (this->base.sprite.size > 25) {
            this->base.sprite.size = 25;
        }
        x = this->base.sprite.position.vx;
        y = this->base.sprite.position.vy;
        z = this->base.sprite.position.vz;
        x += this->base.velocity.vx;
        y += this->base.velocity.vy;
        z += this->base.velocity.vz;
        this->base.sprite.position.vx = x;
        this->base.sprite.position.vy = y;
        this->base.sprite.position.vz = z;
        level = 120 - this->base.age * 10;
        low = level >> 2;
        this->base.sprite.color = low | (low << 8) | (level << 16);
        this->base.age++;
    }
}

extern MATRIX D_800108B4;

/* Draws the particle: a square of side 2 * size around the origin, placed at the sprite's position by a copy of the
 * matrix D_800108B4 translated there, with buildPolyFT4. */
void BeelzebumonSpecial2Particle_draw(BeelzebumonSpecial2Particle *this, Camera *camera) {
    SVECTOR corners[4];
    PolyFT4Shape shape;
    MATRIX m;
    u_long *ot;
    s32 size;

    size = this->base.sprite.size;
    corners[0].vx = corners[2].vx = -size;
    corners[1].vx = corners[3].vx = size;
    corners[0].vy = corners[1].vy = -size;
    corners[2].vy = corners[3].vy = size;
    corners[0].vz = corners[2].vz = corners[1].vz = corners[3].vz = 0;
    shape.flags = 0;
    ot = camera->ot.slots;
    Particle_buildShape(&this->base, &shape, corners);
    m = D_800108B4;
    matrixTranslate(&m, this->base.sprite.position.vx, this->base.sprite.position.vy, this->base.sprite.position.vz);
    setModelMatrix(&m);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift);
}

/* Constructs func_80097FA0's class for fighter: a 0x180 sphere (0x8008A0, knocking back 0x1000 the fighter's way,
 * starting disabled) in its side's list of fight, at 0x200 in front of the fighter and 0x140 up. The match depends
 * on the knockback as an initialized local in its own block: gcc fills a zeroed temporary and copies it whole. */
Unk80097FA0 *func_80097FA0(Unk80097FA0 *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &D_80069A7C;
    /* ZERO_VECTOR is a VECTOR; only its first three words are read here */
    TrackedHit_init(&this->parts.unk18, fighter, 0x8008A0, (Vec3 *)&ZERO_VECTOR, 0);
    CollisionSphere_init(&this->parts.part, (Vec3 *)&ZERO_VECTOR, 0x180, &this->parts.unk18);
    {
        VECTOR knockback = {Fighter_getFacing(fighter) << 12};

        Hit_setKnockback(&this->parts.unk18.base, &knockback);
    }
    this->parts.part.entity = &this->parts.entity;
    this->parts.part.flags.bits.disabled = 1;
    Fighter_getPosition(fighter, &this->position);
    this->position.vx += Fighter_getFacing(fighter) << 9;
    this->position.vy -= 0x140;
    this->fighter = fighter;
    this->fight = fight;
    this->unk9C = 0x133;
    this->unkA0 = 0;
    this->unkA4 = 0;
    this->unkA8 = 1;
    this->unkAC = 0;
    this->vramY = Fighter_getVramY(fighter);
    if (Fighter_getSide(fighter) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A7C, which func_80097FA0 installs. */
void func_80098128(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A7C;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_800981BC);

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_800983FC);

/* Creates an entity of func_80098720's class (0x44 bytes) with g++'s new, and hands it to entityAddSibling. */
void func_800986C0(Unk80097FA0 *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        func_80098720(entityOperatorNewAt(0x44, child), this->fighter, &this->position);
    }
    entityAddSibling(&this->parts.entity, child);
}

/* The entity class of vtable D_80069A54 (0x44 bytes, func_800986C0 spawns it): a character's effect at a
 * position. */
typedef struct Unk80098720 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C; /* 0x320 */
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* 0x5A00 */
    /* 0x38 */ s32 unk38; /* 20 */
    /* 0x3C */ s32 frames;
    /* 0x40 */ s16 vramY; /* Fighter_getVramY of the character */
} Unk80098720;

/* Constructs the entity of vtable D_80069A54 for a character at a position. */
Unk80098720 *func_80098720(Unk80098720 *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069A54;
    this->fighter = fighter;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unk2C = 0x320;
    this->unk30 = 0;
    this->unk34 = 0x5A00;
    this->unk38 = 20;
    this->frames = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_80069A54, which func_80098720 installs. */
void func_800987B8(Entity *this, s32 flags) {
    this->vtable = &D_80069A54;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of the entity of vtable D_80069A54: for 11 frames unk2C falls by 45, unk30 grows by 48, unk34 falls by
 * 1500 and unk38 grows by 5; then it deletes itself. */
void func_8009880C(Unk80098720 *this) {
    if (this->frames > 10) {
        entityDelete(&this->entity);
    } else {
        this->unk2C -= 45;
        this->unk38 += 5;
        this->unk34 -= 1500;
        this->unk30 += 48;
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_800988A4);

/* The UnkPartsEntity3C class func_80098A78 constructs (0xC8 bytes, virtual table D_80069A2C). */
typedef struct Unk80098A78 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Fighter *fighter;
    /* 0x88 */ Fight *fight;
    /* 0x8C */ Vec3 position;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 velocity;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 direction; /* 1, or -1 when velocity.vx is negative */
    /* 0xB4 */ s16 vramY;     /* Fighter_getVramY of the character */
    /* 0xB6 */ s16 unkB6;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;
} Unk80098A78;

/* Builds func_80095CB4's entity for fighter at position, moving by velocity: a 0x180 sphere in its side's list of
 * fight, hitting 0x84B09C00 with a knockback of 32 times velocity's x and as much up; direction is velocity's
 * side. The match depends on direction set to 1 and flipped by an if with no else: CSE then carries &parts.part
 * across the calls in a saved register, as the game does (a ternary breaks the block). */
Unk80098A78 *func_80098A78(Unk80098A78 *this, Fighter *fighter, Fight *fight, Vec3 *position, VECTOR *velocity) {
    VECTOR knockback;
    s32 x, y, z;
    s32 vx, vy, vz;
    s32 direction;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &D_80069A2C;
    TrackedHit_init(&this->parts.unk18, fighter, 0x84B09C00, (Vec3 *)velocity, 3);
    CollisionSphere_init(&this->parts.part, position, 0x180, &this->parts.unk18);
    knockback = *velocity;
    knockback.vx <<= 5;
    knockback.vy = -abs(knockback.vx);
    knockback.vz = 0;
    Hit_setKnockback(&this->parts.unk18.base, &knockback);
    this->parts.part.entity = &this->parts.entity;
    this->fighter = fighter;
    this->fight = fight;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->unkAC = 0;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    direction = 1;
    if (vx < 0) {
        direction = -1;
    }
    this->direction = direction;
    this->vramY = Fighter_getVramY(fighter);
    this->unkB6 = 0;
    this->unkB8 = 0;
    this->unkBC = 0x40;
    this->unkC0 = 0x20;
    this->unkC4 = 0;
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_80069A2C, which func_80098A78 installs. */
void func_80098C04(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_80069A2C;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_80098C98);

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_80098ECC);

/* The entity class of vtable D_80069A04 (0x54 bytes, func_8009918C spawns it, constructor func_8009921C). */
typedef struct Unk8009921C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *unk228;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 direction;
    /* 0x34 */ Vec3 velocity; /* vx: 12 along direction */
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 vramY; /* Fighter_getVramY of the character */
    /* 0x46 */ u16 unk46; /* toggled every tick */
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C; /* 0x20 */
    /* 0x50 */ s32 ticks; /* ticks so far */
} Unk8009921C;

Unk8009921C *func_8009921C(Unk8009921C *this, Fighter *fighter, Fight *unk228, Vec3 *position,
                           s32 direction, s32 unk48);

/* Spawns func_8009921C's entity (0x54 bytes) at position as a sibling, with the direction and unkBC (at least
 * 300). */
void func_8009918C(Unk80098A78 *this) {
    s32 unk48 = this->unkBC;
    Entity *child;

    if (unk48 < 300) {
        unk48 = 300;
    }
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_8009921C(entityOperatorNewAt(0x54, child), this->fighter, this->fight, &this->position, this->direction,
                      unk48);
    }
    entityAddSibling(&this->parts.entity, child);
}

/* Constructs the entity of vtable D_80069A04 for a character at a position, its step 12 along direction. */
Unk8009921C *func_8009921C(Unk8009921C *this, Fighter *fighter, Fight *unk228, Vec3 *position,
                           s32 direction, s32 unk48) {
    s32 x, y, z;
    s32 step;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80069A04;
    this->fighter = fighter;
    this->unk228 = unk228;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    step = -12;
    /* The game clears the velocity with the next word and sets vx again below. */
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->unk40 = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->direction = direction;
    this->unk48 = unk48;
    if (direction > 0) {
        step = 12;
    }
    this->velocity.vx = step;
    this->vramY = Fighter_getVramY(fighter);
    this->unk46 = 0;
    this->unk4C = 0x20;
    this->ticks = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80069A04, which func_8009921C installs. */
void func_800992F4(Entity *this, s32 flags) {
    this->vtable = &D_80069A04;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The gather of D_80069A04's entity: moves by its velocity, grows 0x4C by 16 for its first 5 gathers and then
 * shrinks it by 6, and deletes itself once 0x4C is no longer positive. */
void func_80099348(Unk8009921C *this) {
    /* The match depends on reading position and velocity before adding (as vecAddInPlace) */
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;
    s32 dx = this->velocity.vx;
    s32 dy = this->velocity.vy;
    s32 dz = this->velocity.vz;

    x += dx;
    y += dy;
    z += dz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (this->ticks < 5) {
        this->unk4C += 16;
    } else {
        this->unk4C -= 6;
    }
    if (this->unk4C <= 0) {
        entityDelete(&this->entity);
    } else {
        this->unk46 ^= 1;
        this->ticks++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", func_80099410);

/* The entity class of virtual table BEELZEBUMON_SPECIAL3_EFFECT_VTABLE (constructor BeelzebumonSpecial3Effect_init,
 * size 0x58): a line (buildLineG2) whose two ends start at one position and move by the velocity and by half of it.
 */
typedef struct BeelzebumonSpecial3Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Fight *unk1C;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ Vec3 unk30; /* the other end, moving at half the velocity */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ Vec3 velocity;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 brightness; /* the near end's gray level (the draw's color) */
    /* 0x54 */ s32 ticks; /* ticks so far */
} BeelzebumonSpecial3Effect;

/* Constructs the line entity of virtual table BEELZEBUMON_SPECIAL3_EFFECT_VTABLE for fighter, with both ends at
 * position. */
BeelzebumonSpecial3Effect *BeelzebumonSpecial3Effect_init(BeelzebumonSpecial3Effect *this, Fighter *fighter,
                                                          Fight *unk1C, Vec3 *position, Vec3 *velocity) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &BEELZEBUMON_SPECIAL3_EFFECT_VTABLE;
    this->fighter = fighter;
    this->unk1C = unk1C;
    /* The match depends on each copy having locals of its own (three words read before any is stored) */
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->unk30.vx = x;
        this->unk30.vy = y;
        this->unk30.vz = z;
    }
    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;

        this->brightness = 200;
        this->ticks = 0;
        this->velocity.vx = x;
        this->velocity.vy = y;
        this->velocity.vz = z;
    }
    return this;
}

/* The special move 3 effect's destructor. */
void BeelzebumonSpecial3Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &BEELZEBUMON_SPECIAL3_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 3 effect's tick: for 8 ticks brightens by 0x20 up to 0x80, then dims by 0x30 and deletes itself
 * once dark; its near end moves by the velocity, its far end by half of it. */
void BeelzebumonSpecial3Effect_tick(BeelzebumonSpecial3Effect *this) {
    s32 x, y, z;

    if (this->ticks >= 8) {
        this->brightness -= 0x30;
        if (this->brightness <= 0) {
            entityDelete(&this->entity);
            return;
        }
    } else {
        this->brightness += 0x20;
        if (this->brightness >= 0x80) {
            this->brightness = 0x80;
        }
    }
    /* The match depends on reading the position whole before adding (as vecAddInPlace) */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk30.vx += this->velocity.vx >> 1;
    this->unk30.vy += this->velocity.vy >> 1;
    this->unk30.vz += this->velocity.vz >> 1;
    this->ticks++;
}

INCLUDE_ASM("game/nonmatchings/fighters/beelzebumon", BeelzebumonSpecial3Effect_draw);

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void BeelzebumonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = PROJECTILE_HIT_VTABLE;
    this->entity.vtable = &PROJECTILE_VTABLE;
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

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_80099980(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800999D4(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 2 particle's destructor, a copy of Particle's (game has 15 identical ones). */
void BeelzebumonSpecial2Particle_destroy(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_SPECIAL3_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069A04);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069A2C);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069A54);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069A7C);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_SPECIAL2_PARTICLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069AF4);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069B1C);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069B44);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", D_80069B6C);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_THROW_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/beelzebumon", BEELZEBUMON_VTABLE);
