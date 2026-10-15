/* MetalGarurumon (CHARACTER_METALGARURUMON): the fighter createFighter builds for it, its moves (a throw that carries
 * the opponent), and the entities they spawn: the air attacks' trails, the special move 1's effects, the special
 * move 2's projectile, and the special move 3's hitbox, projectiles and effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

void MetalGarurumon_spawnAirAttackTrails(Fighter *this);

/* the parts of the draw that func_8009AF7C calls in a row */
void func_8009AFC4(Entity *this, s32 arg);
void func_8009B370(Entity *this, s32 arg);
void func_8009B664(Entity *this, s32 arg);
struct Unk8009AB40 *func_8009AB40(struct Unk8009AB40 *this, Fighter *fighter, s32 vramY);
extern EntityVtable D_8006A15C;
extern VtableEntry METALGARURUMON_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable METALGARURUMON_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable METALGARURUMON_VTABLE;

/* MetalGarurumon's class (CHARACTER_METALGARURUMON). */
typedef struct MetalGarurumon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ Fighter *unk4A8;
    /* 0x4AC */ MatrixWords unk4AC; /* MetalGarurumon_throw sets it to the identity */
    /* 0x4CC */ u32 soundHandle; /* the special move 1's sound (stopSound) */
} MetalGarurumon;

/* constructs the entity of virtual table D_8006A25C for a character */
Entity *func_8009B834(void *this, Fighter *fighter, s32 arg2, s32 arg3);

/* The entity class of virtual table D_8006A15C, which MetalGarurumon_special3 makes for its object. */
typedef struct Unk8009D20C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ void *unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
} Unk8009D20C;

/* Builds MetalGarurumon: a Fighter with its own vtable. */
MetalGarurumon *MetalGarurumon_init(MetalGarurumon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &METALGARURUMON_VTABLE;
    this->unk4A8 = 0;
    this->soundHandle = 0;
    return this;
}

/* Ends a move: stops the special move 1's sound. */
void MetalGarurumon_endMove(MetalGarurumon *this) {
    stopSound(this->soundHandle);
    this->soundHandle = 0;
}

/* Attack 2: func_80079AE4; the clip at frame 0, then the attack (Fighter_getParams's entry 1) from frame 5 on
 * (Fighter_runAttack). */
void MetalGarurumon_attack2(Fighter *this, s32 frame) {
    Attack *record;

    func_80079AE4(this);
    record = &Fighter_getParams(this)->attacks[1];
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_ATTACK2, 0, 0, -1);
        break;
    case 5:
        Fighter_runAttack(this, 0, record, 1);
        break;
    }
    if (frame > 5) {
        Fighter_runAttack(this, frame, record, 1);
    }
}

/* Dash attack: func_80079AE4; the clip at frame 0, then the attack (Fighter_getParams's entry 3) from frame 6 on
 * (Fighter_runAttack). */
void MetalGarurumon_dashAttack(Fighter *this, s32 frame) {
    Attack *record;

    func_80079AE4(this);
    record = &Fighter_getParams(this)->attacks[3];
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_DASH_ATTACK, 2, 0, -1);
        break;
    case 6:
        Fighter_runAttack(this, 0, record, 2);
        break;
    }
    if (frame > 6) {
        Fighter_runAttack(this, frame, record, 1);
    }
}

/* Side attack in the air: Fighter's, with the trails at frame 2; from frame 3 the attack parts get bits 0xD0000. */
void MetalGarurumon_airSideAttack(Fighter *this, s32 frame) {
    Fighter_airSideAttack(this, frame);
    if (frame == 2) {
        MetalGarurumon_spawnAirAttackTrails(this);
    }
    if (frame >= 3 && this->attackParts != 0) {
        this->attackParts |= 0xD0000;
    }
}

/* Attack in the air: Fighter's, with the trails at frame 2; from frame 3 the attack parts get bits 0xD0000. */
void MetalGarurumon_airAttack(Fighter *this, s32 frame) {
    Fighter_airAttack(this, frame);
    if (frame == 2) {
        MetalGarurumon_spawnAirAttackTrails(this);
    }
    if (frame >= 3 && this->attackParts != 0) {
        this->attackParts |= 0xD0000;
    }
}

/* three tables of 4 entries for MetalGarurumon_spawnAirAttackTrails's entities */
extern TrailPoint D_80069D3C[3][4];

/* Spawns the air attacks' three trails (Trail_init), one per table of D_80069D3C. */
void MetalGarurumon_spawnAirAttackTrails(Fighter *this) {
    TrailPoint (*table)[4] = D_80069D3C;
    TrailPoint (*end)[4] = &D_80069D3C[3];

    /* The match depends on a do/while walking a pointer to an end kept in a local (an index is counted
     * down, a for loop keeps its first test, and an end computed in the test gets another register) */
    do {
        Entity *child = allocEntityMemory(0x690);

        if (child != NULL) {
            Trail_init(entityOperatorNewAt(0x690, child), this, *table, 4, 2, 16);
        }
        Fighter_addEntity(this, child);
        table++;
    } while (table != end);
}

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_80069D2C);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_80069D3C);

void MetalGarurumon_carryOpponent(Fighter *this);
void func_8009A0EC(MetalGarurumon *this);
void func_80099FBC(MetalGarurumon *this, MATRIX *matrix);
void func_8009A128(MetalGarurumon *this);
void MetalGarurumon_throwHit(Fighter *this, u32 unk, Vec3 *velocity);

/* Throw: Fighter_throw and Fighter_accelerateTo(0), then by frame: 0 clears 0x4A4 and 0x4A8, sets the matrix at 0x4AC
 * to the identity and holds the opponent (Fighter_hold(2)), then, like 1, carries it at node 5; 2, 15 and 30 hit it (30
 * with a velocity in the way it faces, after dropping the fighter at 0x4A8); frames 3 to 29 end with func_8009A128. */
void MetalGarurumon_throw(MetalGarurumon *this, s32 frame) {
    Fighter *opponent;
    Vec3 velocity;

    Fighter_throw(&this->base, frame);
    opponent = Fighter_getOpponent(&this->base);
    Fighter_accelerateTo(&this->base, 0);
    switch (frame) {
    case 0:
        this->unk4A4 = 0;
        this->unk4A8 = NULL;
        this->unk4AC.words[0] = 0x1000;
        this->unk4AC.words[1] = 0;
        this->unk4AC.words[2] = 0x1000;
        this->unk4AC.words[3] = 0;
        this->unk4AC.words[4] = 0x1000;
        this->unk4AC.m.t[0] = 0;
        this->unk4AC.m.t[1] = 0;
        this->unk4AC.m.t[2] = 0;
        Fighter_hold(opponent, 2);
        /* fallthrough */
    case 1:
        MetalGarurumon_carryOpponent(&this->base);
        break;
    case 2:
        MetalGarurumon_throwHit(&this->base, 0x8C0C00, (Vec3 *)&ZERO_VECTOR);
        func_80099FBC(this, 0);
        break;
    case 15:
        MetalGarurumon_throwHit(&this->base, 0x8C1400, (Vec3 *)&ZERO_VECTOR);
        break;
    case 30:
        func_8009A0EC(this);
        velocity.vx = Fighter_getFacing(&this->base) * 0x38E0;
        velocity.vy = -0x38E0;
        velocity.vz = 0;
        MetalGarurumon_throwHit(&this->base, 0x408C2000, &velocity);
        break;
    }
    if ((u32)(frame - 3) < 27) {
        func_8009A128(this);
    }
}

/* the offset from node 5 where MetalGarurumon_carryOpponent puts the opponent */
extern VECTOR D_80069D2C;

/* Puts the opponent at node 5 (offset D_80069D2C), raised by its height (Fighter_getParams), and clears its force,
 * velocity and z rotation. */
void MetalGarurumon_carryOpponent(Fighter *this) {
    Fighter *opponent = Fighter_getOpponent(this);
    VECTOR position;

    Fighter_getNodePosition(this, &position, 5, &D_80069D2C);
    position.vy += Fighter_getParams(opponent)->height;
    Fighter_setPosition(opponent, (Vec3 *)&position);
    opponent->force.vx = 0;
    opponent->force.vy = 0;
    opponent->force.vz = 0;
    opponent->forcePad = 0;
    opponent->velocity.vx = 0;
    opponent->velocity.vy = 0;
    opponent->velocity.vz = 0;
    opponent->velocityPad = 0;
    opponent->rotation.vz = 0;
    opponent->targetRotation.vz = 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_80099FBC);

/* Calls Fighter_detach on the character at 0x4A8, if there is one, and clears it. */
void func_8009A0EC(MetalGarurumon *this) {
    if (this->unk4A8 != NULL) {
        Fighter_detach(this->unk4A8);
    }
    this->unk4A8 = NULL;
}

/* While 0x4A4 is 0, waits for the character at 0x4A8 to be out of every list, then drops it
 * (func_8009A0EC) and counts 0x4A4 up; otherwise calls func_80099FBC with the matrix at 0x4AC and clears
 * 0x4A4. */
void func_8009A128(MetalGarurumon *this) {
    if (this->unk4A4 == 0) {
        if (listIsAlone(&this->unk4A8->entity.link)) {
            func_8009A0EC(this);
            this->unk4A4++;
        }
    } else {
        func_80099FBC(this, &this->unk4AC.m);
        this->unk4A4 = 0;
    }
}

/* Hits the opponent (Fighter_takeHit with hit and velocity), then sprays particles (ParticleSpray_init) at node 5,
 * drifting back and up. */
void MetalGarurumon_throwHit(Fighter *this, u32 hit, Vec3 *velocity) {
    VECTOR position;
    Vec3 drift;
    Entity *child;

    Fighter_takeHit(Fighter_getOpponent(this), hit, velocity, 1);
    Fighter_getNodePosition(this, &position, 5, &D_80069D2C);
    drift.vx = -(Fighter_getFacing(this) << 5);
    drift.vy = -16;
    drift.vz = 0;
    child = allocEntityMemory(0x48);
    if (child != NULL) {
        ParticleSpray_init(entityOperatorNewAt(0x48, child), 6, (Vec3 *)&position, &drift, 0x80, 8, 0xFFE0C0);
    }
    Fighter_addEntity(this, child);
}

void func_8009A4D8(Fighter *this);
void func_8009A44C(MetalGarurumon *this);

/* Special move 1: Fighter's and func_80079AE4; at frame 0 the voice and the effect of func_8009A4D8, at frame 13
 * func_8009A44C and a push back by 0x2AA8 times the mass (16.16). Always sets 0x4A0. */
void MetalGarurumon_special1(MetalGarurumon *this, s32 frame) {
    Fighter_special1(&this->base, frame);
    func_80079AE4(&this->base);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xBB);
        func_8009A4D8(&this->base);
        break;
    case 13:
        func_8009A44C(this);
        this->base.force.vx -= ((s64)(Fighter_getFacing(&this->base) * 0x2AA8) * this->base.mass) >> 16;
        break;
    }
    this->base.unk4A0 = 1;
}

/* Special move 1 in the air: Fighter's and func_80079AE4; at frame 0 the voice and the effect of func_8009A4D8, at
 * frame 13 func_8009A44C and a push back by 0x1C70 times the mass (16.16); before frame 20 the force brakes half a
 * fall. Always sets 0x4A0. */
void MetalGarurumon_airSpecial1(MetalGarurumon *this, s32 frame) {
    Fighter_airSpecial1(&this->base, frame);
    func_80079AE4(&this->base);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xBB);
        func_8009A4D8(&this->base);
        break;
    case 13:
        func_8009A44C(this);
        this->base.force.vx -= ((s64)(Fighter_getFacing(&this->base) * 0x1C70) * this->base.mass) >> 16;
        break;
    }
    if (frame < 20 && this->base.velocity.vy > 0) {
        this->base.force.vy -= this->base.velocity.vy >> 1;
    }
    this->base.unk4A0 = 1;
}

/* Plays the special move 1's sound (its handle kept), spawns func_8009AB40's effect with the fighter's vramY and calls
 * func_80079B0C(50). */
void func_8009A44C(MetalGarurumon *this) {
    Entity *child;

    this->soundHandle = Fighter_playSound(&this->base, 0xB6);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        func_8009AB40(entityOperatorNewAt(0x4C, child), &this->base, Fighter_getVramY(&this->base));
    }
    Fighter_addEntity(&this->base, child);
    func_80079B0C(&this->base, 50);
}

/* Spawns the entity of func_8009B834 for the character (Fighter_addEntity). */
void func_8009A4D8(Fighter *this) {
    Entity *child = allocEntityMemory(0x50);

    if (child != NULL) {
        func_8009B834(entityOperatorNewAt(0x50, child), this, 0x200, 0xFF8080);
    }
    Fighter_addEntity(this, child);
}

void MetalGarurumon_spawnSpecial2Projectile(Fighter *this);

/* Special move 2: Fighter's and func_80079AE4; the voice at frame 0, the projectile and a push back by 0x4718 times the
 * mass (16.16) at frame 19, and at frame 40, on the ground, a stop and a push back by 0x2AA8. */
void MetalGarurumon_special2(Fighter *this, s32 frame) {
    Fighter_special2(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0xBC);
        break;
    case 19:
        MetalGarurumon_spawnSpecial2Projectile(this);
        this->force.vx -= ((s64)(Fighter_getFacing(this) * 0x4718) * this->mass) >> 16;
        break;
    case 40:
        if (Fighter_isOnGround(this)) {
            this->velocity.vx = 0;
            this->force.vx -= ((s64)(Fighter_getFacing(this) * 0x2AA8) * this->mass) >> 16;
        }
        break;
    }
}

/* Special move 2 in the air: Fighter's and func_80079AE4; the voice at frame 0; at frame 19 the projectile, a push back
 * and up by 0x2AA8 times the mass (16.16) plus the vertical velocity; before frame 19 the force brakes half a fall. */
void MetalGarurumon_airSpecial2(Fighter *this, s32 frame) {
    s32 lift;

    Fighter_airSpecial2(this, frame);
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playVoice(this, 0xBC);
        break;
    case 19:
        MetalGarurumon_spawnSpecial2Projectile(this);
        this->force.vx -= ((s64)(Fighter_getFacing(this) * 0x2AA8) * this->mass) >> 16;
        lift = ((s64)this->mass * 0x2AA8) >> 16;
        lift += this->velocity.vy;
        this->force.vy -= lift;
        break;
    }
    if (frame < 19 && this->velocity.vy > 0) {
        this->force.vy -= this->velocity.vy >> 1;
    }
}

/* the offset from node 0 where MetalGarurumon_spawnSpecial2Projectile fires the projectile */
extern VECTOR D_80069EE0;

MetalGarurumonSpecial2Projectile *MetalGarurumonSpecial2Projectile_init(MetalGarurumonSpecial2Projectile *this,
                                                                        Fighter *fighter, Vec3 *position,
                                                                        Vec3 *velocity);

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_80069EE0);

/* Fires the special move 2's projectile from node 0 (offset D_80069EE0), 24 a frame forward, then calls
 * func_80079B14(60) and plays its sound. */
void MetalGarurumon_spawnSpecial2Projectile(Fighter *this) {
    VECTOR position;
    Vec3 velocity;
    Entity *child;

    Fighter_getNodePosition(this, &position, 0, &D_80069EE0);
    velocity.vx = Fighter_getFacing(this) * 24;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        MetalGarurumonSpecial2Projectile_init(entityOperatorNewAt(0xC8, child), this, (Vec3 *)&position, &velocity);
    }
    Fighter_addEntity(this, child);
    func_80079B14(this, 60);
    Fighter_playSound(this, 0xB7);
}

Unk8009D20C *func_8009D20C(Unk8009D20C *this, void *arg);
Entity *func_8009D4EC(void *this, Fighter *fighter);
void func_8009A920(Fighter *this);

/* Special move 3: Fighter's; at frame 0 func_8009D20C's effect, 12 invulnerable frames and Fighter_addGauge(-0x8000);
 * on the ground every 4th frame below 45 func_8009D4EC's effect; the voice at frame 20 and func_8009A920 at 45. */
void MetalGarurumon_special3(Fighter *this, s32 frame) {
    Entity *child;

    Fighter_special3(this, frame);
    if (frame == 0) {
        child = allocEntityMemory(0x24);
        if (child != NULL) {
            func_8009D20C(entityOperatorNewAt(0x24, child), this);
        }
        Fighter_addEntity(this, child);
        this->invulnerableFrames = 12;
        Fighter_addGauge(this, -0x8000);
    }
    if (Fighter_isOnGround(this) && frame < 45 && (frame & 3) == 0) {
        child = allocEntityMemory(0x14C);
        if (child != NULL) {
            func_8009D4EC(entityOperatorNewAt(0x14C, child), this);
        }
        Fighter_addEntity(this, child);
    }
    if (frame == 20) {
        Fighter_playVoice(this, 0xBD);
    }
    if (frame == 45) {
        func_8009A920(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_80069EF0);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009A920);

extern EntityVtable D_8006A284;

/* The entity class of virtual table D_8006A284 (constructor func_8009AB40, size 0x4C). */
typedef struct Unk8009AB40 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ VECTOR position; /* node 5 of the character, offset D_80069D2C */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 facing; /* Fighter_getFacing: 1 or -1 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 vramY; /* Fighter_getVramY of the character */
} Unk8009AB40;

/* Constructs the entity of virtual table D_8006A284 for fighter, at its node 5 and facing its way. */
Unk8009AB40 *func_8009AB40(Unk8009AB40 *this, Fighter *fighter, s32 vramY) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A284;
    this->fighter = fighter;
    Fighter_getNodePosition(fighter, &this->position, 5, &D_80069D2C);
    this->unk2C = 4;
    this->facing = Fighter_getFacing(fighter);
    this->vramY = vramY;
    this->unk38 = 0;
    this->unk34 = 0x2000;
    this->unk3C = 0x40;
    this->unk40 = 0;
    this->unk44 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009ABE0);

/* The draw of the entities of D_8006A284: func_8009AFC4, func_8009B370, then func_8009B664. */
void func_8009AF7C(Entity *this, s32 arg) {
    func_8009AFC4(this, arg);
    func_8009B370(this, arg);
    func_8009B664(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009AFC4);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009B370);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009B664);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009B834);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009BA18);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009BC24);

extern EntityVtable METALGARURUMON_SPECIAL2_PROJECTILE_VTABLE;
extern VtableEntry METALGARURUMON_SPECIAL2_PROJECTILE_HIT_VTABLE[];

/* Builds the special move 2's projectile: Projectile_init (size 0x80, 0x1000, 3), its vtables and Hit_set(0x84AC4000,
 * 7). */
MetalGarurumonSpecial2Projectile *MetalGarurumonSpecial2Projectile_init(MetalGarurumonSpecial2Projectile *this,
                                                                        Fighter *fighter, Vec3 *position,
                                                                        Vec3 *velocity) {
    Projectile_init(&this->base, fighter, position, velocity, 0x80, 0, 0x1000, 3);
    this->base.parts.base.vtable = METALGARURUMON_SPECIAL2_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &METALGARURUMON_SPECIAL2_PROJECTILE_VTABLE;
    Hit_set(&this->base.parts.base, 0x84AC4000, 7);
    this->unkC0 = 0;
    this->unkC4 = 0;
    return this;
}

/* Speeds up along x by 1/16, eases 0xC0 toward 0x100, cycles 0xC4 from 0x70 to 0xA0 by 16, sheds a burst
 * (ParticleBurst_init) at its position as a sibling, then Projectile_tick. func_8009CE8C's class shares it. */
void MetalGarurumonSpecial2Projectile_tick(MetalGarurumonSpecial2Projectile *this, s32 arg) {
    Entity *child;

    this->base.velocity.vx = (this->base.velocity.vx * 17) >> 4;
    this->base.speed = vecGetLength((VECTOR *)&this->base.velocity);
    this->unkC0 += (0x100 - this->unkC0) >> 2;
    this->unkC4 += 16;
    if (this->unkC4 > 0xA0) {
        this->unkC4 = 0x70;
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 6, &this->base.position, 0x30, 0xC0, 1, 0x808080);
    }
    entityAddSibling(&this->base.parts.entity, child);
    Projectile_tick(&this->base, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", MetalGarurumonSpecial2Projectile_hit);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", MetalGarurumonSpecial2Projectile_draw);

/* A point of MetalGarurumonSpecial3Hitbox's trail: where it was, and an age that grows by 3 a tick. */
typedef struct MetalGarurumonSpecial3HitboxTrail {
    /* 0x0 */ Vec3 position;
    /* 0xC */ s32 age;
} MetalGarurumonSpecial3HitboxTrail;

/* The special move 3's hitbox, with a trail of its last 16 positions. */
typedef struct MetalGarurumonSpecial3Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ u8 unk80[0x84 - 0x80];
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0xB4 - 0x90];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ u8 unkB8[0xC0 - 0xB8];
    /* 0xC0 */ s32 ticks; /* ticks so far */
    /* 0xC4 */ u8 unkC4[0xCC - 0xC4];
    /* 0xCC */ s32 trailIndex; /* the newest point of trail */
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ MetalGarurumonSpecial3HitboxTrail trail[16];
} MetalGarurumonSpecial3Hitbox;

void func_8009C914(MetalGarurumonSpecial3Hitbox *this);
void func_8009CB58(MetalGarurumonSpecial3Hitbox *this, s32 arg);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", MetalGarurumonSpecial3Hitbox_init);

void func_8009C678(MetalGarurumonSpecial3Hitbox *this);
void func_8009C898(MetalGarurumonSpecial3Hitbox *this);

/* While 0xB4 is set, counts 0xD0 down and deletes itself at 0; otherwise runs func_8009C678 and counts 0xD0 up to 16.
 * Then adds a trail point. */
void MetalGarurumonSpecial3Hitbox_tick(MetalGarurumonSpecial3Hitbox *this) {
    if (this->unkB4 != 0) {
        if (--this->unkD0 <= 0) {
            entityDelete(&this->parts.entity);
            return;
        }
    } else {
        func_8009C678(this);
        if (++this->unkD0 > 16) {
            this->unkD0 = 16;
        }
    }
    func_8009C898(this);
    this->ticks++;
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", MetalGarurumonSpecial3Hitbox_hit);

/* Draws the hitbox (func_8009C914) until 0xB4 is set, then its trail (func_8009CB58). */
void MetalGarurumonSpecial3Hitbox_draw(MetalGarurumonSpecial3Hitbox *this, s32 arg) {
    if (this->unkB4 == 0) {
        func_8009C914(this);
    }
    func_8009CB58(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009C678);

/* Adds the position as the newest point of the trail (age 16) and ages every point by 3. */
void func_8009C898(MetalGarurumonSpecial3Hitbox *this) {
    MetalGarurumonSpecial3HitboxTrail *point;
    s32 i;

    if (++this->trailIndex >= 16) {
        this->trailIndex = 0;
    }
    /* The match depends on reading the position before the point's address is taken */
    {
        s32 x = this->position.vx;
        s32 y = this->position.vy;
        s32 z = this->position.vz;

        point = &this->trail[this->trailIndex];
        point->position.vx = x;
        point->position.vy = y;
        point->position.vz = z;
        point->age = 16;
    }
    for (i = 0; i < 16; i++) {
        this->trail[i].age += 3;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A0D0);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009C914);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009CB58);

extern EntityVtable D_8006A184;
extern VtableEntry D_8006A1AC[];

/* Constructs the class of virtual table D_8006A184 (and D_8006A1AC for its base at 0x18) on
 * MetalGarurumonSpecial2Projectile_init's for fighter at position, moving by velocity, with 0x6B08000 and 7 in its
 * base's 0x8 and 0xC. */
MetalGarurumonSpecial2Projectile *func_8009CE8C(MetalGarurumonSpecial2Projectile *this, Fighter *fighter,
                                                Vec3 *position, Vec3 *velocity) {
    MetalGarurumonSpecial2Projectile_init(this, fighter, position, velocity);
    this->base.parts.base.vtable = D_8006A1AC;
    this->base.parts.entity.vtable = &D_8006A184;
    Hit_set(&this->base.parts.base, 0x6B08000, 7);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009CEE0);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009CFBC);

/* Constructs an entity of the class of virtual table D_8006A15C for arg. */
Unk8009D20C *func_8009D20C(Unk8009D20C *this, void *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A15C;
    this->unk18 = arg;
    this->unk1C = 0;
    this->unk20 = 0;
    return this;
}

/* The tick of func_8009D20C's entity: 0x1C rises with 0x20 (by 8) for 16 ticks, holds, and from 45 on
 * (at once when Fighter_isAttackIdle holds for the character) falls by 16 until the entity deletes itself. */
void func_8009D25C(Unk8009D20C *this) {
    s32 time;

    if (Fighter_isAttackIdle(this->unk18)) {
        this->unk20 = 45;
    }
    time = this->unk20;
    if (time <= 16) {
        this->unk1C = time * 8;
    } else if (time >= 45) {
        this->unk1C -= 16;
        if (this->unk1C <= 0) {
            entityDelete(&this->entity);
            return;
        }
    }
    this->unk20++;
}

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A0F0);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009D308);

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009D4EC);

/* The entity class of virtual table D_8006A134 (constructor func_8009D4EC, size 0x14C). */
typedef struct Unk8009D4EC {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Fighter *fighter;
    /* 0x01C */ u8 unk1C[0x12C - 0x1C]; /* 17 points of 0x10 bytes, from COS_SIN_TABLE */
    /* 0x12C */ Vec3 position; /* the character's (Fighter_getPosition) */
    /* 0x138 */ u8 unk138[0x13C - 0x138];
    /* 0x13C */ s32 unk13C; /* 0x100 at construction */
    /* 0x140 */ s32 unk140;
    /* 0x144 */ s32 unk144;
    /* 0x148 */ s32 vramY;  /* Fighter_getVramY of the character */
} Unk8009D4EC;

/* The tick of func_8009D4EC's entity: follows the character's position until Fighter_isAttackIdle holds or bit 2
 * of its flags (Fighter_isOnGround) clears, and deletes itself once 0x13C, growing by 0x15, reaches 0x200. */
void func_8009D5D4(Unk8009D4EC *this) {
    if (this->unk140 == 0) {
        if (Fighter_isAttackIdle(this->fighter) || !Fighter_isOnGround(this->fighter)) {
            this->unk140++;
        } else {
            Fighter_getPosition(this->fighter, &this->position);
        }
    }
    this->unk13C += 0x15;
    if (this->unk13C >= 0x200) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/metalgarurumon", func_8009D68C);

/* MetalGarurumon's destructor. */
void MetalGarurumon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A284, which func_8009AB40 installs. */
void func_8009D8BC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A25C, which func_8009B834 installs. */
void func_8009D904(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void MetalGarurumonSpecial2Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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

/* The special move 3 hitbox's destructor. */
void MetalGarurumonSpecial3Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = METALGARURUMON_SPECIAL3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &METALGARURUMON_SPECIAL3_HITBOX_VTABLE;
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

/* A copy of the destructor of the UnkPartsEntity38 class of vtables PROJECTILE_VTABLE and PROJECTILE_HIT_VTABLE,
 * which Projectile_init installs: game has 11 identical ones. */
void func_8009DA8C(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of the entity class of vtable D_8006A15C, which func_8009D20C installs. */
void func_8009DB2C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A134, which func_8009D4EC installs. */
void func_8009DB74(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A134);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A15C);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A184);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A1AC);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", METALGARURUMON_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", METALGARURUMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", METALGARURUMON_SPECIAL2_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", METALGARURUMON_SPECIAL2_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A25C);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", D_8006A284);
INCLUDE_RODATA("game/nonmatchings/fighters/metalgarurumon", METALGARURUMON_VTABLE);
