/* Impmon (CHARACTER_IMPMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * throw's particle, the special move 1's projectile (and the particles it sheds), the intro's effect, and the
 * special move 2's effect and its hitbox. */

#include "common.h"
#include "battle/game_classes.h"

struct ImpmonIntroEffect *ImpmonIntroEffect_init(struct ImpmonIntroEffect *this, Fighter *fighter, Fight *fight);
struct ImpmonSpecial1Projectile *ImpmonSpecial1Projectile_init(struct ImpmonSpecial1Projectile *this, Fighter *fighter,
                                                               Fight *fight, Vec3 *position, Vec3 *velocity);
extern EntityVtable IMPMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable IMPMON_SPECIAL2_EFFECT_VTABLE;
extern EntityVtable IMPMON_INTRO_EFFECT_VTABLE;
extern EntityVtable D_800697A4;
extern EntityVtable IMPMON_VTABLE;

/* The intro's effect (0x58 bytes): grows at the character's point 3, then flies toward the opponent and fades. */
typedef struct ImpmonIntroEffect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ VECTOR position; /* the character's point 3 moved by D_80069700 (Fighter_getNodePosition) */
    /* 0x2C */ VECTOR velocity;
    /* 0x3C */ s32 state;
    /* 0x40 */ s32 distance; /* the sum of |velocity.vx| */
    /* 0x44 */ s32 counter;  /* frames in the state */
    /* 0x48 */ s32 cell;     /* cycles 0-2, then 6-11, then counts from 3 to 6 */
    /* 0x4C */ s32 unk4C;    /* grows by 15 up to 60, then by 30 */
    /* 0x50 */ s32 ticks;    /* counts the ticks */
    /* 0x54 */ s16 vramY;    /* Fighter_getVramY of the character */
} ImpmonIntroEffect;
void Impmon_spawnSpecial1Projectile(Fighter *this);
/* the constructor of the throw's particle */
Entity *ImpmonThrowParticle_init(Entity *this, Fighter *fighter, Vec3 *position, Vec3 *velocity);

/* Impmon's class (CHARACTER_IMPMON). */
typedef struct Impmon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 special2Stopped; /* cleared at special2's frame 0, set by Impmon_stopSpecial2 */
    /* 0x4AC */ s32 special2Frames; /* counts special2's frames until it stops; nothing reads it */
    /* 0x4B0 */ Vec3 special2Start;  /* the fighter's position at special2's frame 0 */
} Impmon;

extern FaceEntry IMPMON_FACES[];

/* Builds Impmon: a Fighter with its own vtable and faces. */
Impmon *Impmon_init(Impmon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &IMPMON_VTABLE;
    this->unk4A4 = 0;
    Fighter_setFaceTable(&this->base, IMPMON_FACES);
    return this;
}

/* Impmon's destructor. */
void Impmon_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void Impmon_stopSpecial2(Impmon *this);

/* Fighter's input step, then unk4A4 counts down; it is set to 30 while the opponent's clip is 15, or while it runs and
 * this one's clip is attack 1 or 2. */
void Impmon_handleInput(Impmon *this, void *input) {
    s32 clip;

    Fighter_handleInput(this, input);
    if (this->unk4A4 != 0) {
        this->unk4A4--;
    }
    if (func_80079E2C(Fighter_getOpponent(&this->base)) == 15) {
        this->unk4A4 = 30;
    }
    if (this->unk4A4 != 0) {
        clip = func_80079E2C(&this->base);
        /* The match depends on the two tests being nested ifs: && folds them into one range test. */
        if (clip < FIGHTER_CLIP_ATTACK3) {
            if (clip >= FIGHTER_CLIP_ATTACK1) {
                this->unk4A4 = 30;
            }
        }
    }
}

/* Ends a move: stops the special move 2. */
void Impmon_endMove(void *this) {
    Impmon_stopSpecial2(this);
}

/* Intro: spawns its effect at frame 15. */
void Impmon_intro(Fighter *this, s32 frame) {
    Fight *fight;
    Entity *child;

    if (frame == 15) {
        fight = Fighter_getScene(this);
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            ImpmonIntroEffect_init(entityOperatorNewAt(0x58, child), this, fight);
        }
        Fighter_addEntity(this, child);
    }
}

/* Attack 3: Fighter's, and at frame 5 a push forward by the attack's lungeX (Fighter_getParams) times the mass. */
void Impmon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 5) {
        s32 push = ((s64)Fighter_getParams(this)->attacks[2].lungeX * this->mass) >> 16;

        this->force.vx += push * Fighter_getFacing(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", Impmon_throw);

/* Spawns the throw's particle 0x84 above the fighter, 24 a frame forward. */
void Impmon_spawnThrowParticle(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vy -= 0x84;
    velocity.vx = Fighter_getFacing(this) * 24;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        ImpmonThrowParticle_init(entityOperatorNewAt(0x58, child), this, &position, &velocity);
    }
    Fighter_addEntity(this, child);
}

/* Special move 1: Fighter's, with the voice at frame 0 and the projectile at frame 10. */
void Impmon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xAC);
    }
    if (frame == 10) {
        Impmon_spawnSpecial1Projectile(this);
    }
}

/* Special move 1 in the air: Fighter's, with the voice and a sound at frame 0 and the projectile at frame 14; until
 * frame 15 the force brakes a sixteenth of the horizontal velocity and any fall. */
void Impmon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0xAC);
        Fighter_playSound(this, 0xA6);
    }
    if (frame == 14) {
        Impmon_spawnSpecial1Projectile(this);
    }
    if (frame < 15) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

/* Fires the special move 1's projectile 0x84 above the fighter, 144 a frame forward, with its sound. */
void Impmon_spawnSpecial1Projectile(Fighter *this) {
    Vec3 position;
    Vec3 velocity;
    Fight *fight;
    Entity *child;

    Fighter_getPosition(this, &position);
    position.vy -= 0x84;
    velocity.vx = Fighter_getFacing(this) * 144;
    velocity.vy = 0;
    velocity.vz = 0;
    fight = Fighter_getScene(this);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        ImpmonSpecial1Projectile_init(entityOperatorNewAt(0xD4, child), this, fight, &position, &velocity);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xA7);
}

s32 Impmon_getSpecial2Distance(Impmon *this);
void Impmon_spawnSpecial2Effect(Impmon *this);

/* Special move 2: Fighter's; at frame 0 it keeps the start position and spawns the effect with a voice. From frame 6
 * it stops, playing the air special 2 clip once, when the fighter is more than 256 away from the start, the button
 * 0x10 is let go, or after frame 150. */
void Impmon_special2(Impmon *this, s32 frame) {
    Fighter_special2(&this->base, frame);
    if (frame == 0) {
        this->special2Frames = 0;
        this->special2Stopped = 0;
        Fighter_getPosition(&this->base, &this->special2Start);
        Impmon_spawnSpecial2Effect(this);
        Fighter_playVoice(&this->base, 0xAD);
    }
    if (this->special2Stopped) {
        return;
    }
    if (frame >= 6) {
        if (Impmon_getSpecial2Distance(this) > 256) {
            Impmon_stopSpecial2(this);
            Fighter_playActionClip(&this->base, FIGHTER_CLIP_AIR_SPECIAL2, 2, 0, 1);
        } else if (!(InputHistory_getHeld(Fight_getInput(Fighter_getScene(&this->base),
                                                          Fighter_getSide(&this->base))) & 0x10) ||
                   frame > 150) {
            Impmon_stopSpecial2(this);
            Fighter_playActionClip(&this->base, FIGHTER_CLIP_AIR_SPECIAL2, 2, 0, 1);
        }
    }
    this->special2Frames++;
}

/* Stops the special move 2: its effect spawns no more hitboxes. */
void Impmon_stopSpecial2(Impmon *this) {
    this->special2Stopped = 1;
}

/* How far the fighter has moved since the special move 2 started. */
s32 Impmon_getSpecial2Distance(Impmon *this) {
    Vec3 position;
    VECTOR delta;
    s32 x, y, z;

    Fighter_getPosition(&this->base, &position);
    /* The match depends on the difference reading everything before writing (as vecSub). */
    x = position.vx;
    y = position.vy;
    z = position.vz;
    x -= this->special2Start.vx;
    y -= this->special2Start.vy;
    z -= this->special2Start.vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    return vecGetLength(&delta);
}

Entity *ImpmonSpecial2Effect_init(void *this, Fighter *fighter, Vec3 *position, s32 facing, Fight *fight);

/* The special move 2's effect (0x54 bytes): a quad turning at a point in front of the fighter, which spawns the
 * move's hitboxes while the move lasts (ImpmonSpecial2Effect_tick). */
typedef struct ImpmonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Impmon *fighter;
    /* 0x1C */ Fight *fight;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ s32 facing;
    /* 0x34 */ s32 halfWidth;   /* 30, then 50 wider a frame up to 180; 30 narrower a frame once stopping */
    /* 0x38 */ s32 height;      /* 120 */
    /* 0x3C */ s32 angle;       /* the quad's Y rotation, 48 more a frame */
    /* 0x40 */ s32 hitboxCount; /* the hitboxes spawned so far */
    /* 0x44 */ s32 stopping;
    /* 0x48 */ s32 hitboxDelay; /* frames until the next hitbox */
    /* 0x4C */ s32 ticks;
    /* 0x50 */ s16 vramY;
} ImpmonSpecial2Effect;

/* Spawns the special move 2's effect 384 in front of the fighter and 16 up, with its facing. */
void Impmon_spawnSpecial2Effect(Impmon *this) {
    Vec3 position;
    Fight *fight;
    Entity *child;

    Fighter_getPosition(&this->base, &position);
    position.vy -= 16;
    position.vx += Fighter_getFacing(&this->base) * 384;
    fight = Fighter_getScene(&this->base);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        ImpmonSpecial2Effect_init(entityOperatorNewAt(0x54, child), &this->base, &position,
                                  Fighter_getFacing(&this->base), fight);
    }
    Fighter_addEntity(&this->base, child);
}

extern u16 D_800696EC[];
extern EntityVtable IMPMON_THROW_PARTICLE_VTABLE;

/* Builds the throw's particle: Particle_init with the cell D_800696EC, size 0x33, grey, and the fighter's vramY. */
Entity *ImpmonThrowParticle_init(Entity *this, Fighter *fighter, Vec3 *position, Vec3 *velocity) {
    Particle_init((Particle *)this, D_800696EC, (VECTOR *)position, (VECTOR *)velocity, 0x33, 0x646464,
                  Fighter_getVramY(fighter));
    this->vtable = &IMPMON_THROW_PARTICLE_VTABLE;
    return this;
}

/* The grey fades by 8 a frame of age from 200 and the sprite grows by 10, then Particle_tick. */
void ImpmonThrowParticle_tick(Particle *this, s32 arg) {
    s32 gray = 200 - (this->age << 3);

    this->sprite.color = gray | (gray << 8) | (gray << 16);
    this->sprite.size += 10;
    Particle_tick(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonThrowParticle_draw);

/* The special move 1's projectile (0xD4 bytes, a Projectile): every other frame it sheds func_800938A4's
 * particles. */
typedef struct ImpmonSpecial1Projectile {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ Fighter *fighter;
    /* 0xC4 */ Fight *fight;
    /* 0xC8 */ s32 unkC8; /* counts 0 to 5 in ImpmonSpecial1Projectile_tick */
    /* 0xCC */ u32 unkCC; /* a color, 0x707070 */
    /* 0xD0 */ u32 unkD0; /* a color, 0x303030 */
} ImpmonSpecial1Projectile;

extern EntityVtable IMPMON_SPECIAL1_PROJECTILE_VTABLE;
extern VtableEntry IMPMON_SPECIAL1_PROJECTILE_HIT_VTABLE[];

/* Builds the special move 1's projectile: Projectile_init (size 0x60, 0x800, 3), its vtables, fighter, fight and two
 * colors; Hit_set with 0x5A81800 and 2. */
ImpmonSpecial1Projectile *ImpmonSpecial1Projectile_init(ImpmonSpecial1Projectile *this, Fighter *fighter, Fight *fight,
                                                        Vec3 *position, Vec3 *velocity) {
    Projectile_init(&this->base, fighter, position, velocity, 0x60, 0, 0x800, 3);
    this->base.parts.base.vtable = IMPMON_SPECIAL1_PROJECTILE_HIT_VTABLE;
    this->base.parts.entity.vtable = &IMPMON_SPECIAL1_PROJECTILE_VTABLE;
    this->fighter = fighter;
    this->fight = fight;
    this->unkC8 = 0;
    this->unkCC = 0x707070;
    this->unkD0 = 0x303030;
    Hit_set(&this->base.parts.base, 0x5A81800, 2);
    return this;
}
Entity *func_800938A4(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel);

/* Every other frame sheds a particle (func_800938A4) at its position, drifting back at a sixteenth of its speed and up
 * or down by -8..7 at random; unkC8 counts 0 to 5; then Projectile_tick. */
void ImpmonSpecial1Projectile_tick(ImpmonSpecial1Projectile *this, s32 arg) {
    VECTOR velocity;
    Entity *child;

    if (this->base.age & 1) {
        velocity.vy = (mersenneTwisterGenerate(&this->fight->random) & 0xF) - 8;
        velocity.vz = 0;
        velocity.vx = -this->base.velocity.vx >> 4;
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_800938A4(entityOperatorNewAt(0x58, child), (s32)this->fighter, (VECTOR *)&this->base.position,
                          &velocity);
        }
        entityAddSibling(&this->base.parts.entity, child);
    }
    this->unkC8++;
    if (this->unkC8 >= 6) {
        this->unkC8 = 0;
    }
    Projectile_tick(&this->base, arg);
}

/* On a hit: Hit_apply, the projectile's burst, then deletes itself. */
void ImpmonSpecial1Projectile_hit(ImpmonSpecial1Projectile *this, CollisionSphere *sum) {
    Hit_apply(&this->base.parts.base, sum);
    Projectile_spawnBurst(&this->base, 0);
    entityDelete(&this->base.parts.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial1Projectile_draw);

/* Constructs an entity of D_800697A4: Particle_initById with entry 9 of PARTICLE_ANIMS, 0x19 and 0x202080. */
Entity *func_800938A4(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    Particle_initById((Particle *)this, 9, pos, vel, 0x19, 0x202080, 0);
    this->vtable = &D_800697A4;
    return this;
}

/* The tick of D_800697A4's entity: nine times moves it by its velocity and fades its red (128 - 16 a step, green
 * and blue a quarter of it), then deletes it.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads unk54. */
void func_800938F4(Particle *this, s32 arg) {
    if (this->age >= 9) {
        entityDelete(&this->entity);
    } else {
        /* The match depends on reading the three words before storing any, as vecAddInPlace does, and on counting
         * the step last. */
        s32 x = this->sprite.position.vx;
        s32 y = this->sprite.position.vy;
        s32 z = this->sprite.position.vz;
        s32 red;

        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->sprite.position.vx = x;
        this->sprite.position.vy = y;
        this->sprite.position.vz = z;
        red = 0x80 - this->age * 16;
        this->sprite.color = red | ((red >> 2) << 8) | ((red >> 2) << 16);
        this->age++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", func_800939B0);

/* Builds the intro's effect for fighter, its fields 0. fight is not read (Impmon_intro passes it). */
ImpmonIntroEffect *ImpmonIntroEffect_init(ImpmonIntroEffect *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &IMPMON_INTRO_EFFECT_VTABLE;
    this->fighter = fighter;
    this->position.vx = 0;
    this->position.vy = 0;
    this->position.vz = 0;
    this->position.pad = 0;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocity.pad = 0;
    this->state = 0;
    /* the game clears 0x40 after 0x48 and 0x4C */
    this->cell = 0;
    this->unk4C = 0;
    this->distance = 0;
    this->ticks = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The intro effect's destructor. */
void ImpmonIntroEffect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPMON_INTRO_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_8006968C);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_80069694);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_8006969C);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696A4);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696AC);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696B4);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696BC);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696CC);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800696EC);
extern VECTOR D_80069700;

INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_80069700);

/* Grows at the character's point 3 for 19 frames, flies forward at 48 until it has gone three quarters of the way to
 * the opponent (Fighter_getOpponentPosition), slows down for 3 frames, then fades and deletes itself; every frame but
 * the last moves it by its velocity. */
void ImpmonIntroEffect_tick(ImpmonIntroEffect *this) {
    Vec3 opponent;
    s32 dx;
    s32 x, y, z;

    switch (this->state) {
    case 0:
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 1:
        this->cell++;
        if (this->cell >= 3) {
            this->cell = 0;
        }
        if (this->unk4C < 60) {
            this->unk4C += 15;
        }
        Fighter_getNodePosition(this->fighter, &this->position, 3, &D_80069700);
        this->position.vy -= 0x33;
        this->counter++;
        if (this->counter < 19) {
            break;
        }
        this->velocity.vx = Fighter_getFacing(this->fighter) * 48;
        this->cell = 6;
        this->state++;
        /* fallthrough */
    case 2:
        this->cell++;
        if (this->cell >= 12) {
            this->cell = 6;
        }
        Fighter_getOpponentPosition(this->fighter, &opponent);
        dx = opponent.vx - this->position.vx;
        /* The match depends on the absolute value as a conditional expression (an if negates it in a copy). */
        dx = dx < 0 ? -dx : dx;
        if (this->distance < dx - (dx >> 2)) {
            break;
        }
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 3:
        this->cell++;
        if (this->cell >= 12) {
            this->cell = 6;
        }
        this->velocity.vx >>= 2;
        this->counter++;
        if (this->counter < 3) {
            break;
        }
        this->counter = 0;
        this->cell = 3;
        this->state++;
        break;
    case 4:
        this->velocity.vx >>= 2;
        this->unk4C += 30;
        this->cell++;
        if (this->cell < 6) {
            break;
        }
        entityDelete(&this->entity);
        return;
    }
    /* The match depends on the three words read before any store and on adding to distance before storing them. */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->distance += this->velocity.vx < 0 ? -this->velocity.vx : this->velocity.vx;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->ticks++;
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonIntroEffect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial2Effect_init);

/* The special move 2 effect's destructor. */
void ImpmonSpecial2Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPMON_SPECIAL2_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void ImpmonSpecial2Effect_spawnHitbox(ImpmonSpecial2Effect *this);
Entity *ImpmonSpecial2Hitbox_init(Entity *this, Fighter *fighter, Vec3 *position, s32 facing, s32 index, Fight *fight);

/* Spawns a hitbox every hitboxDelay frames (half the count so far, plus 1) until the move stops or 150 frames
 * pass; the effect then widens no more, plays sound 0xA8 if it made any hitbox, and shrinks away. */
void ImpmonSpecial2Effect_tick(ImpmonSpecial2Effect *this) {
    Impmon *fighter;
    s32 stopped;

    fighter = this->fighter;
    stopped = fighter->special2Stopped;
    if (this->ticks > 150) {
        stopped = 1;
    }
    if (!stopped) {
        if (--this->hitboxDelay == 0) {
            ImpmonSpecial2Effect_spawnHitbox(this);
            this->hitboxDelay = (this->hitboxCount >> 1) + 1;
        }
    } else {
        if (!this->stopping && this->hitboxCount != 0) {
            Fighter_playSound(&fighter->base, 0xA8);
        }
        this->stopping = 1;
    }
    if (this->stopping) {
        this->halfWidth -= 30;
        if (this->halfWidth <= 0) {
            entityDelete(&this->entity);
            return;
        }
    } else if (this->halfWidth < 180) {
        this->halfWidth += 50;
    }
    this->angle += 48;
    this->ticks++;
}

/* Spawns the next hitbox at the effect's position, with its facing and the count of hitboxes before it. */
void ImpmonSpecial2Effect_spawnHitbox(ImpmonSpecial2Effect *this) {
    Entity *hitbox;

    hitbox = allocEntityMemory(0xD4);
    if (hitbox != NULL) {
        ImpmonSpecial2Hitbox_init(entityOperatorNewAt(0xD4, hitbox), &this->fighter->base, &this->position, this->facing,
                                  this->hitboxCount++, this->fight);
    }
    entityAddSibling(&this->entity, hitbox);
}

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial2Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial2Hitbox_init);

/* The destructor of the UnkPartsEntity3C class of vtable IMPMON_SPECIAL2_HITBOX_VTABLE, which ImpmonSpecial2Hitbox_init installs. */
void ImpmonSpecial2Hitbox_destroy(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &IMPMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial2Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/impmon", ImpmonSpecial2Hitbox_draw);

/* The throw particle's destructor, a copy of Particle's (game has 15 identical ones). */
void ImpmonThrowParticle_destroy(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's destructor, a copy of Projectile's (game has 11 identical ones). */
void ImpmonSpecial1Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
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
void func_80094EBC(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA_ALIGN8("game/nonmatchings/fighters/impmon", IMPMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_INTRO_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", D_800697A4);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_SPECIAL1_PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_SPECIAL1_PROJECTILE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_THROW_PARTICLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/impmon", IMPMON_VTABLE);
