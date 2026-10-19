/* Omegamon (CHARACTER_OMEGAMON): the fighter createFighter builds for it, its moves (a throw that carries the
 * opponent on a node), and the entities they spawn: the special move 1's projectile and flashes, the special move
 * 2's hitbox, and the special move 3's two hitboxes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "gte.h"

struct OmegamonSpecial1Projectile *OmegamonSpecial1Projectile_init(struct OmegamonSpecial1Projectile *this,
                                                                   Fighter *owner, Fight *fight, VECTOR *pos,
                                                                   VECTOR *vel);
Entity *OmegamonSpecial2Hitbox_init(Entity *this, Fighter *owner, Fight *fight, Vec3 *pos, s32 side, s32 arg5);
struct Unk800A0054 *func_800A0054(struct Unk800A0054 *this, Fighter *owner, Fight *fight, Vec3 *pos,
                                  Vec3 *vel, s32 index);
struct Unk800A0830 *func_800A0830(struct Unk800A0830 *this, Fighter *owner, Fight *fight, Vec3 *pos,
                                  Vec3 *vel, s32 index);
Unk8009F248 *func_8009F248(Unk8009F248 *this, Fighter *owner);
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Fighter *owner);
extern VECTOR D_8006A43C;
extern VECTOR D_8006A44C[2];
extern VECTOR D_8006A46C[2];
extern EntityVtable D_8006A48C;
extern EntityVtable D_8006A4B4;
extern EntityVtable OMEGAMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable D_8006A504;
extern EntityVtable D_8006A52C;
extern EntityVtable D_8006A554;
extern EntityVtable OMEGAMON_VTABLE;
extern SpriteAnimRow D_8006A6EC;
extern SpriteAnimRow D_8006A700;

/* Omegamon's class (CHARACTER_OMEGAMON): func_8009E744 and func_8009E814 clear 0x4A8 and 0x4AC before they launch
 * the special move 3's hitboxes. */
typedef struct Omegamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ Fighter *heldOpponent; /* the opponent, carried by the throw from frame 7 */
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
} Omegamon;

/* Builds Omegamon: a Fighter with its own vtable. */
Fighter *Omegamon_init(Fighter *this) {
    Fighter_init(this);
    this->entity.vtable = &OMEGAMON_VTABLE;
    return this;
}

/* Omegamon's destructor. */
void Omegamon_destroy(Entity *this, s32 flags) {
    this->vtable = &OMEGAMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: detaches the opponent (Fighter_detach). */
void Omegamon_endMove(Fighter *this) {
    Fighter_detach(Fighter_getOpponent(this));
}

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", Omegamon_attack2);

INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A3D4);

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", Omegamon_attack3);


/* Throw: Fighter's; at frame 0 the opponent is held (Fighter_hold(8)); at frame 7, unless no opponent is in reach (then
 * Fighter_stopActionClip), it is held (14), takes a clip by its character, is attached to node 13 and its model moved
 * by its height; at frame 20, unless it was let go, it is hit forward and detached. */
void Omegamon_throw(Omegamon *this, s32 frame) {
    Fighter *opponent;
    Model *model;
    s32 clip;
    s16 side;
    s32 alone;
    Vec3 vel;

    Fighter_throw(&this->base, frame);
    opponent = Fighter_getOpponent(&this->base);
    if (frame == 0) {
        Fighter_hold(opponent, 8);
    }
    if (frame == 7) {
        if (Fighter_getOpponentInReach(&this->base, 0) != NULL) {
            switch (Fighter_getCharacter(opponent)) {
            case CHARACTER_METALGARURUMON:
            case CHARACTER_TERRIERMON:
                clip = 3;
                break;
            case CHARACTER_WORMMON:
                clip = 32;
                break;
            case CHARACTER_STINGMON:
            case CHARACTER_GOKUMON:
                clip = 13;
                break;
            default:
                clip = 4;
                break;
            }
            Fighter_hold(opponent, 14);
            Fighter_setClip(opponent, clip, 2, 0);
            Fighter_attachTo(opponent, &this->base, 13);
            this->heldOpponent = opponent;
            model = opponent->model;
            model->group.object.transform.valid = 0;
            if (Fighter_getParams(opponent)->height < 0x80) {
                model->group.object.transform.local.m.t[0] = 0x20;
                model->group.object.transform.local.m.t[1] = 0xC0;
                model->group.object.transform.local.m.t[2] = -0x80;
            } else {
                model->group.object.transform.local.m.t[0] = 0x20;
                model->group.object.transform.local.m.t[1] = 0xC0;
                model->group.object.transform.local.m.t[2] = -0xE0;
            }
        } else {
            Fighter_stopActionClip(&this->base);
            return;
        }
    }
    if (frame == 20) {
        /* The match depends on listIsAlone(&this->heldOpponent->entity.link) computed as a value (xor, sltiu). */
        alone = this->heldOpponent->entity.link.next == &this->heldOpponent->entity.link
                && &this->heldOpponent->entity.link == this->heldOpponent->entity.link.prev;
        if (!alone) {
            /* side is a halfword: the game sign-extends the result (sll, sra) before the multiply */
            side = Fighter_getFacing(&this->base);
            vel.vx = side * 21840;
            vel.vy = -0x38E0;
            vel.vz = 0;
            Fighter_takeHit(opponent, 0x5A8C3500, &vel, 0);
            Fighter_detach(opponent);
        }
    }
}

void Omegamon_spawnSpecial1Projectile(Fighter *this);
void func_8009E424(Fighter *this);
void func_8009E480(Fighter *this);

/* Special move 1: Fighter's; the voice at frame 0, the flash at frame 8, the second flash and the projectile at frame
 * 9. */
void Omegamon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xC3);
    }
    if (frame == 8) {
        func_8009E424(this);
    }
    if (frame == 9) {
        func_8009E480(this);
        Omegamon_spawnSpecial1Projectile(this);
    }
}

/* Special move 1 in the air: as on the ground; before frame 10 the force brakes a sixteenth of the horizontal velocity
 * and any fall. */
void Omegamon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xC3);
    }
    if (frame == 8) {
        func_8009E424(this);
    }
    if (frame == 9) {
        func_8009E480(this);
        Omegamon_spawnSpecial1Projectile(this);
    }
    if (frame < 10) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A43C);

/* Fires the special move 1's projectile from node 4 moved by D_8006A43C, 192 a frame forward, with its sound. */
void Omegamon_spawnSpecial1Projectile(Fighter *this) {
    VECTOR pos;
    VECTOR vel;
    Fight *fight;
    Entity *child;

    Fighter_getNodePosition(this, &pos, 4, &D_8006A43C);
    vel.vx = Fighter_getFacing(this) * 192;
    vel.vy = 0;
    vel.vz = 0;
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0xE4);
    if (child != NULL) {
        OmegamonSpecial1Projectile_init(entityOperatorNewAt(0xE4, child), this, fight, &pos, &vel);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xBE);
}

/* Creates an entity of func_8009F248's class (0x5C bytes) with g++'s new, and hands it to Fighter_addEntity. */
void func_8009E424(Fighter *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F248(entityOperatorNewAt(0x5C, child), this);
    }
    Fighter_addEntity(this, child);
}

/* Creates an entity of func_8009F4D4's class (0x5C bytes) with g++'s new, and hands it to Fighter_addEntity. */
void func_8009E480(Fighter *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        func_8009F4D4(entityOperatorNewAt(0x5C, child), this);
    }
    Fighter_addEntity(this, child);
}

void Omegamon_spawnSpecial2Hitbox(Fighter *this, s32 arg);

/* Special move 2: Fighter's; frame 0 the voice and func_80079B14(0x2A), the hitboxes at frames 7 and 17, a push
 * forward at frame 22, and from frame 23 a force of half the velocity against it. */
void Omegamon_special2(Omegamon *this, s32 frame) {
    s64 push;

    Fighter_special2(&this->base, frame);
    if (frame == 0) {
        Fighter_playVoice(&this->base, 0xC4);
        func_80079B14(&this->base, 0x2A);
    }
    if (frame == 7) {
        Omegamon_spawnSpecial2Hitbox(&this->base, 0);
    }
    if (frame == 17) {
        Omegamon_spawnSpecial2Hitbox(&this->base, 1);
    }
    if (frame == 22) {
        push = ((s64)this->base.mass * 0x4718) >> 16;
        this->base.force.vx += (s32)push * Fighter_getFacing(&this->base);
    }
    if (frame >= 23) {
        this->base.force.vx = ((s64)this->base.velocity.vx * -0x800) >> 12;
    }
}

/* Spawns the special move 2's hitbox 0x180 in front of the fighter and 0x133 up, toward its facing, with arg and its
 * sound. */
void Omegamon_spawnSpecial2Hitbox(Fighter *this, s32 arg) {
    s32 side;
    Vec3 pos;
    Fight *fight;
    Entity *child;

    side = Fighter_getFacing(this);
    Fighter_getPosition(this, &pos);
    pos.vx += side * 0x180;
    pos.vy -= 0x133;
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0xC0);
    if (child != NULL) {
        OmegamonSpecial2Hitbox_init(entityOperatorNewAt(0xC0, child), this, fight, &pos, side, arg);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xBF);
}

void func_8009E744(Omegamon *this);
void func_8009E814(Omegamon *this);

/* Special move 3: Fighter's; at frame 0 12 invulnerable frames, Fighter_addGauge(-0x10000) and the voice; the first
 * hitbox at frame 18, the second at frame 38. */
void Omegamon_special3(Omegamon *this, s32 frame) {
    Fighter_special3(&this->base, frame);
    if (frame == 0) {
        this->base.invulnerableFrames = 12;
        Fighter_addGauge(&this->base, -0x10000);
        Fighter_playVoice(&this->base, 0xC5);
    }
    if (frame == 18) {
        func_8009E744(this);
    }
    if (frame == 38) {
        func_8009E814(this);
    }
}

/* Clears 0x4A8 and creates an entity of func_800A0054's class (0xD8 bytes) 0x120 in front of the
 * character, moving 128 a frame toward Fighter_getFacing's side; hands it to Fighter_addEntity and plays
 * sound 0xC0. */
void func_8009E744(Omegamon *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Fight *fight;
    Entity *child;

    this->unk4A8 = 0;
    side = Fighter_getFacing(&this->base);
    Fighter_getPosition(&this->base, &pos);
    vel.vx = side * 128;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x120;
    fight = Fighter_getScene(&this->base);
    child = allocEntityMemory(0xD8);
    if (child != NULL) {
        func_800A0054(entityOperatorNewAt(0xD8, child), &this->base, fight, &pos, &vel, 0);
    }
    Fighter_addEntity(&this->base, child);
    Fighter_playSound(&this->base, 0xC0);
}

/* As func_8009E744 with 0x4AC, func_800A0830's class (0xD4 bytes), 0x180 in front and 0x180 up,
 * moving 160 a frame. */
void func_8009E814(Omegamon *this) {
    s32 side;
    Vec3 pos;
    Vec3 vel;
    Fight *fight;
    Entity *child;

    this->unk4AC = 0;
    side = Fighter_getFacing(&this->base);
    Fighter_getPosition(&this->base, &pos);
    vel.vx = side * 160;
    vel.vy = 0;
    vel.vz = 0;
    pos.vx += side * 0x180;
    pos.vy -= 0x180;
    fight = Fighter_getScene(&this->base);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        func_800A0830(entityOperatorNewAt(0xD4, child), &this->base, fight, &pos, &vel, 0);
    }
    Fighter_addEntity(&this->base, child);
    Fighter_playSound(&this->base, 0xC0);
}

extern EntityVtable OMEGAMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry OMEGAMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];

/* The special move 1's projectile (0xE4 bytes, a Projectile): it keeps its owner, the fight and a tail, drawn as a
 * trail from the tail to its position. */
typedef struct OmegamonSpecial1Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ Fighter *owner;
    /* 0xC4 */ Fight *fight;
    /* 0xC8 */ Vec3 tail; /* where it started; its x follows at an eighth of the speed */
    /* 0xD4 */ s32 tailPad;
    /* 0xD8 */ s32 animFrame; /* 0, 1, 2 over and over (the draw's texture column) */
    /* 0xDC */ s32 ending;    /* set by OmegamonSpecial1Projectile_hit or once past its range */
    /* 0xE0 */ s32 life;      /* 0x80, 20 less a frame once ending; deleted at 0 */
} OmegamonSpecial1Projectile;

/* Builds the special move 1's projectile for owner at pos, moving by vel: Projectile_init (size 0x80, hit 0x6A82400,
 * 0x1000, 3), its vtables, the owner, the fight and its tail where it starts. */
OmegamonSpecial1Projectile *OmegamonSpecial1Projectile_init(OmegamonSpecial1Projectile *this, Fighter *owner,
                                                            Fight *fight, VECTOR *pos, VECTOR *vel) {
    s32 x, y, z;

    Projectile_init(&this->base, owner, (Vec3 *)pos, (Vec3 *)vel, 0x80, 0x6A82400, 0x1000, 3);
    this->base.parts.base.vtable = OMEGAMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &OMEGAMON_SPECIAL1_PROJECTILE_VTABLE;
    this->owner = owner;
    this->fight = fight;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->life = 0x80;
    this->animFrame = 0;
    this->ending = 0;
    this->tail.vx = x;
    this->tail.vy = y;
    this->tail.vz = z;
    return this;
}

void func_8009EF64(OmegamonSpecial1Projectile *this);

/* The special move 1 projectile's frame: puffs a trail particle, moves by its velocity (the tail's x an eighth as
 * fast) and carries its hit part along, 0x80 wider than its radius; once it has gone its range or hit, it slows to
 * half speed, shrinks by an eighth of maxRadius and fades by 20 a frame, and deletes itself when either is gone. */
void OmegamonSpecial1Projectile_tick(OmegamonSpecial1Projectile *this) {
    s32 x, y, z;

    func_8009EF64(this);
    if (++this->animFrame >= 3) {
        this->animFrame = 0;
    }
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = this->base.position.vx;
    y = this->base.position.vy;
    z = this->base.position.vz;
    x += this->base.velocity.vx;
    y += this->base.velocity.vy;
    z += this->base.velocity.vz;
    this->base.position.vx = x;
    this->base.position.vy = y;
    this->base.position.vz = z;
    /* The match depends on the tail moving before the hit part takes the position */
    this->tail.vx += this->base.velocity.vx >> 3;
    this->base.parts.part.position = this->base.position;
    this->base.parts.part.radius = this->base.radius + 0x80;
    this->base.distance += this->base.speed;
    if (this->base.distance > this->base.range) {
        this->ending = 1;
    }
    if (this->ending == 0) {
        this->base.radius += (this->base.maxRadius - this->base.radius) >> 2;
    } else {
        this->base.velocity.vx >>= 1;
        this->base.radius -= this->base.maxRadius >> 3;
        this->life -= 20;
        if (this->base.radius <= 0 || this->life <= 0) {
            entityDelete(&this->base.parts.entity);
            return;
        }
    }
    this->base.age++;
}

/* On a hit: Hit_apply, then starts ending, disables its sphere and spawns its burst (4). */
void OmegamonSpecial1Projectile_hit(OmegamonSpecial1Projectile *this, CollisionSphere *sum) {
    Hit_apply(&this->base.parts.base, sum);
    this->ending = 1;
    this->base.parts.part.flags.bits.disabled = 1;
    Projectile_spawnBurst(&this->base, 4);
}

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", OmegamonSpecial1Projectile_draw);

Entity *func_8009F014(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel);

/* Spawns func_8009F014's entity (0x58 bytes) at the projectile's position as a sibling, moving back at a 16th
 * of its x speed and up by 13 to 20 (a random of the owner's Fight). */
void func_8009EF64(OmegamonSpecial1Projectile *this) {
    VECTOR velocity;
    Entity *child;

    /* the game copies all four words (0x9C too) before it overwrites three of them */
    velocity.vx = this->base.velocity.vx;
    velocity.vy = this->base.velocity.vy;
    velocity.vz = this->base.velocity.vz;
    velocity.pad = this->base.velocityPad;
    velocity.vy = (mersenneTwisterGenerate(&this->fight->random) & 7) - 20;
    velocity.vz = 0;
    velocity.vx = -this->base.velocity.vx >> 4;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_8009F014(entityOperatorNewAt(0x58, child), (s32)this->owner, (VECTOR *)&this->base.position, &velocity);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* Constructs an entity of D_8006A554: Particle_initById with entry 9 of PARTICLE_ANIMS, 0x19 and 0x802020. */
Entity *func_8009F014(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    Particle_initById((Particle *)this, 9, pos, vel, 0x19, 0x802020, 0);
    this->vtable = &D_8006A554;
    return this;
}

/* The tick of D_8006A554's entity: eleven times moves it by its velocity and fades its blue (200 - 16 a step, red
 * and green a quarter of it), then deletes it.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads unk54. */
void func_8009F064(Particle *this, s32 arg) {
    if (this->age >= 11) {
        entityDelete(&this->entity);
    } else {
        /* The match depends on reading the three words before storing any, as vecAddInPlace does, and on counting
         * the step last. */
        s32 x = this->sprite.position.vx;
        s32 y = this->sprite.position.vy;
        s32 z = this->sprite.position.vz;
        s32 blue;

        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->sprite.position.vx = x;
        this->sprite.position.vy = y;
        this->sprite.position.vz = z;
        blue = 200 - this->age * 16;
        this->sprite.color = (blue >> 2) | ((blue >> 2) << 8) | (blue << 16);
        this->age++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", func_8009F120);

INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A44C);

/* Constructs an Unk8009F248 (vtable D_8006A52C) of owner with the sprite frame D_8006A6EC, at the owner's point 4
 * moved by D_8006A44C[1] and moving by D_8006A44C[0]. */
Unk8009F248 *func_8009F248(Unk8009F248 *this, Fighter *owner) {
    s32 flags = Fighter_getVramY(owner);
    /* The match depends on frame, read into a local before entityInit. */
    SpriteAnimRow *frame = &D_8006A6EC;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    spriteAnimInit(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A52C;
    this->owner = owner;
    Fighter_getNodePosition(owner, &pos, 4, &D_8006A44C[1]);
    Particle_setMotion(&this->base, &pos, &D_8006A44C[0], 0x133, 0x808080);
    this->base.sprite.size = 0x133;
    return this;
}

/* The tick of D_8006A52C's entity: five times fades its grey by 32 from 150, then deletes it.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads unk54. */
void func_8009F30C(Unk8009F248 *this, s32 arg) {
    if (this->base.age >= 5) {
        entityDelete(&this->base.entity);
    } else {
        s32 grey = 150 - this->base.age * 32;

        this->base.sprite.color = grey | (grey << 8) | (grey << 16);
        this->base.age++;
    }
}

/* Entry 0x14 of vtable D_8006A52C: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (Fighter_getFacing), one OT slot further, with buildPolyFT4. */
void func_8009F394(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = Fighter_getFacing(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.position;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    Particle_buildShape(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A46C);

/* Constructs an Unk8009F248 (vtable D_8006A504) of owner with the sprite frame D_8006A700, at the owner's point 4
 * moved by D_8006A46C[1] and moving by D_8006A46C[0]. */
Unk8009F248 *func_8009F4D4(Unk8009F248 *this, Fighter *owner) {
    s32 flags = Fighter_getVramY(owner);
    /* The match depends on frame, read into a local before entityInit. */
    SpriteAnimRow *frame = &D_8006A700;
    VECTOR pos;

    entityInit(&this->base.entity, 0);
    spriteAnimInit(&this->base.sprite, frame, flags);
    this->base.entity.vtable = &D_8006A504;
    this->owner = owner;
    Fighter_getNodePosition(owner, &pos, 4, &D_8006A46C[1]);
    Particle_setMotion(&this->base, &pos, &D_8006A46C[0], 0x80, 0x808080);
    this->base.sprite.size = 0x33;
    return this;
}

/* The tick of D_8006A504's entity: five times fades its grey by 32 from 180 and grows it by 25, then deletes
 * it.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads unk54.
 * It also depends on growing the size before counting the step. */
void func_8009F598(Unk8009F248 *this, s32 arg) {
    if (this->base.age >= 5) {
        entityDelete(&this->base.entity);
    } else {
        s32 grey = 180 - this->base.age * 32;

        this->base.sprite.color = grey | (grey << 8) | (grey << 16);
        this->base.sprite.size += 25;
        this->base.age++;
    }
}

/* Entry 0x14 of vtable D_8006A504: draws the sprite in the camera as a square of side 2 * size around pos, mirrored by
 * the owner's side (Fighter_getFacing), one OT slot further, with buildPolyFT4. */
void func_8009F62C(Unk8009F248 *this, Camera *camera) {
    VECTOR pos;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 size;
    s32 side;

    gte_loadRotTrans(SCRATCH_VIEW_MATRIX);
    ot = camera->ot.slots;
    side = Fighter_getFacing(this->owner);
    /* The match depends on size read before the pos copy. */
    size = this->base.sprite.size;
    pos = this->base.sprite.position;
    corners[0].vx = corners[1].vx = pos.vx - size * side;
    corners[2].vx = corners[3].vx = pos.vx + size * side;
    corners[0].vy = corners[2].vy = pos.vy - size;
    corners[1].vy = corners[3].vy = pos.vy + size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = pos.vz;
    shape.flags = 0;
    Particle_buildShape(&this->base, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, camera->zShift + 1);
}

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", OmegamonSpecial2Hitbox_init);

/* The special move 2 hitbox's destructor. */
void OmegamonSpecial2Hitbox_destroy(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &OMEGAMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", OmegamonSpecial2Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", OmegamonSpecial2Hitbox_draw);

/* The class func_800A0054 constructs (0xD8 bytes, virtual table D_8006A4B4): parts for an owner at pos,
 * moving at vel, with two random words. */
typedef struct Unk800A0054 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Fighter *owner;
    /* 0x88 */ Fight *fight; /* the part goes into one of its two lists (Fighter_getSide picks which) */
    /* 0x8C */ Vec3 position;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ VECTOR start; /* the next link copies its four words */
    /* 0xAC */ Vec3 velocity;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 index; /* its place in the chain (func_800A0680 spawns the next one below 15) */
    /* 0xC0 */ s32 unkC0; /* random, 0 to 3 */
    /* 0xC4 */ s32 unkC4; /* random, 0 to 0x7F */
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 rise; /* how far the tick has moved position.vy up */
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ s16 vramY; /* the owner's 0x440 (Fighter_getVramY) */
} Unk800A0054;

/* Constructs link index of the Unk800A0054 chain for owner at pos, moving at vel, and adds its part to one of fight's
 * two lists (Fighter_getSide picks which). */
Unk800A0054 *func_800A0054(Unk800A0054 *this, Fighter *owner, Fight *fight, Vec3 *pos, Vec3 *vel,
                           s32 index) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &D_8006A4B4;
    TrackedHit_init(&this->parts.unk18, owner, 0x6B07000, vel, 3);
    CollisionSphere_init(&this->parts.part, pos, 0x40, &this->parts.unk18);
    v.vx = Fighter_getFacing(owner) * 1820;
    v.vy = 0;
    v.vz = 0;
    Hit_setKnockback(&this->parts.unk18.base, &v);
    this->parts.part.entity = &this->parts.entity;
    this->owner = owner;
    this->fight = fight;
    this->parts.part.flags.bits.disabled = 1;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->index = index;
    this->unkC0 = mersenneTwisterGenerate(&fight->random) & 3;
    this->unkC4 = mersenneTwisterGenerate(&fight->random) & 0x7F;
    this->unkC8 = 300;
    this->rise = 0;
    this->unkD0 = 0;
    this->vramY = Fighter_getVramY(owner);
    if (Fighter_getSide(owner) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A4B4, which func_800A0054 installs. */
void func_800A0214(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A4B4;
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

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", func_800A02A8);

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", func_800A04BC);

/* Hands parent the next link of its chain (func_800A0054) at position, moved up to 31 forward at random, moving by
 * velocity. The match depends on the random step being in the inline: the copies of both pointers are then made
 * before its calls (spawnNextLink's shape). */
static inline void spawnNextParticleLink(Unk800A0054 *parent, VECTOR *position, Vec3 *velocity) {
    s32 facing = Fighter_getFacing(parent->owner);
    Entity *child;

    position->vx += (mersenneTwisterGenerate(&parent->fight->random) & 0x1F) * facing;
    child = allocEntityMemory(0xD8);
    if (child != NULL) {
        func_800A0054(entityOperatorNewAt(0xD8, child), parent->owner, parent->fight, (Vec3 *)position, velocity,
                      parent->index + 1);
    }
    entityAddSibling(&parent->parts.entity, child);
}

/* Spawns the next link of the chain one velocity step ahead of start. */
void func_800A0680(Unk800A0054 *this) {
    VECTOR position;

    position = this->start;
    position.vx += this->velocity.vx;
    position.vy += this->velocity.vy;
    position.vz += this->velocity.vz;
    spawnNextParticleLink(this, &position, &this->velocity);
}

/* Hands parent a new ParticleSpray (0x48 bytes) of one pink particle (kind 6, size 0x60) at position, moving by
 * velocity. The match depends on this being an inline: its copy of position (the frame's first local) stays in a
 * saved register across the allocation. */
static inline void spawnOneParticle(Entity *parent, Vec3 *position, Vec3 *velocity) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        ParticleSpray_init(entityOperatorNewAt(0x48, child), 6, position, velocity, 0x60, 1, 0xFFE0E0);
    }
    entityAddSibling(parent, child);
}

/* Sprays one particle where it started rising (position.vy + rise), drifting up by 20. */
void func_800A0774(Unk800A0054 *this) {
    Vec3 position;
    Vec3 velocity;

    velocity.vx = 0;
    velocity.vy = -20;
    velocity.vz = 0;
    position = this->position;
    position.vy += this->rise;
    spawnOneParticle(&this->parts.entity, &position, &velocity);
}

/* The class func_800A0830 constructs (0xD4 bytes, virtual table D_8006A48C): parts for an owner at pos,
 * moving at vel, with the owner's facing. */
typedef struct Unk800A0830 {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Fighter *owner;
    /* 0x88 */ Fight *fight; /* the part goes into one of its two lists (Fighter_getSide picks which) */
    /* 0x8C */ VECTOR position; /* the next link copies its four words */
    /* 0x9C */ Vec3 start;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ Vec3 velocity;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 index; /* its place in the chain (func_800A0D94); only link 0 can hit */
    /* 0xC0 */ s32 facing; /* the owner's (Fighter_getFacing) */
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ s32 unkC8;
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s16 vramY; /* the owner's 0x440 (Fighter_getVramY) */
} Unk800A0830;

/* Constructs link index of the Unk800A0830 chain for owner at pos, moving at vel (links past 0 disable the part), and
 * adds its part to one of fight's two lists (Fighter_getSide picks which). */
Unk800A0830 *func_800A0830(Unk800A0830 *this, Fighter *owner, Fight *fight, Vec3 *pos, Vec3 *vel,
                           s32 index) {
    VECTOR v;
    s32 x, y, z;
    s32 sx, sy, sz;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &D_8006A48C;
    TrackedHit_init(&this->parts.unk18, owner, 0x45B07000, (Vec3 *)&ZERO_VECTOR, 3);
    CollisionSphere_init(&this->parts.part, pos, 0x200, &this->parts.unk18);
    v.vx = Fighter_getFacing(owner) * 0x5550;
    v.vy = -0x5550;
    v.vz = 0;
    Hit_setKnockback(&this->parts.unk18.base, &v);
    this->parts.part.entity = &this->parts.entity;
    if (index != 0) {
        this->parts.part.flags.bits.disabled = 1;
    }
    this->owner = owner;
    this->fight = fight;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each copy */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    sx = pos->vx;
    sy = pos->vy;
    sz = pos->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->index = index;
    this->facing = Fighter_getFacing(owner);
    this->vramY = Fighter_getVramY(owner);
    this->unkC4 = 0;
    this->unkC8 = 0x280;
    this->unkCC = 0;
    if (Fighter_getSide(owner) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity3C class of vtable D_8006A48C, which func_800A0830 installs. */
void func_800A09F0(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006A48C;
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

void func_800A0D94(Unk800A0830 *this);

/* A chain link's tick: on frame 1 the links below 10 spawn the next one; links past 0 last 10 ticks, link 0 27 while
 * its start (and part) moves a third of the velocity a tick. Once its part touched, it sets the owner's 0x4AC and
 * disables the part when TrackedHit_hitFighter finds a fighter struck. */
void func_800A0A84(Unk800A0830 *this) {
    if (this->unkCC != 0) {
        if (this->unkCC == 1 && this->index < 10) {
            func_800A0D94(this);
        }
        this->unkC4++;
        if (this->index != 0) {
            if (this->unkC4 >= 10) {
                entityDelete(&this->parts.entity);
                return;
            }
        } else {
            if (this->unkC4 >= 27) {
                entityDelete(&this->parts.entity);
                return;
            }
            this->start.vx += this->velocity.vx / 3;
            this->parts.part.position.vx = this->start.vx;
            this->parts.part.position.vy = this->start.vy;
            this->parts.part.position.vz = this->start.vz;
        }
    }
    /* CollisionShape_isTouched, inline */
    if (this->parts.part.flags.bits.touched) {
        this->parts.part.flags.bits.touched = 0;
        if (TrackedHit_hitFighter(&this->parts.unk18)) {
            this->parts.part.flags.bits.disabled = 1;
            ((Omegamon *)this->owner)->unk4AC = 1;
        }
    }
    this->unkCC++;
}

INCLUDE_ASM("game/nonmatchings/fighters/omegamon", func_800A0BD0);

/* Hands parent the next link of its chain (func_800A0830) at position, moving by velocity. The match depends on this
 * being an inline: its copies of both pointers stay in saved registers across the allocation (spawnOneParticle). */
static inline void spawnNextLink(Unk800A0830 *parent, VECTOR *position, Vec3 *velocity) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800A0830(entityOperatorNewAt(0xD4, child), parent->owner, parent->fight, (Vec3 *)position, velocity,
                      parent->index + 1);
    }
    entityAddSibling(&parent->parts.entity, child);
}

/* Spawns the next link of the chain one velocity step ahead (func_800A0A84 calls it for links below 10). */
void func_800A0D94(Unk800A0830 *this) {
    VECTOR position;

    position = this->position;
    position.vx += this->velocity.vx;
    position.vy += this->velocity.vy;
    position.vz += this->velocity.vz;
    spawnNextLink(this, &position, &this->velocity);
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void OmegamonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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
void func_800A0EF8(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800A0F4C(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800A0FA0(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A48C);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A4B4);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", OMEGAMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A504);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A52C);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A554);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", OMEGAMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", OMEGAMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", OMEGAMON_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A6EC);
INCLUDE_RODATA("game/nonmatchings/fighters/omegamon", D_8006A700);
