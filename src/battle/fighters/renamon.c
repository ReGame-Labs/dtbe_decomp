/* Renamon (CHARACTER_RENAMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's hitbox and effect, and the special move 2's effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

struct RenamonSpecial1Effect *RenamonSpecial1Effect_init(struct RenamonSpecial1Effect *this, Fighter *fighter);
extern VtableEntry RENAMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable RENAMON_SPECIAL1_HITBOX_VTABLE;

/* The special move 1's hitbox (RenamonSpecial1Hitbox_init; its tick spawns func_800A4550's sparkles and
 * func_800A45C8's burst at its position). */
typedef struct RenamonSpecial1Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ s32 ownerHandle;  /* entityGetHandle of the fighter */
    /* 0x84 */ Vec3 position;
    /* 0x90 */ s32 positionPad;
    /* 0x94 */ Vec3 hitOffset;   /* on a hit, the fighter hit's position less this one's */
    /* 0xA0 */ s32 hitOffsetPad;
    /* 0xA4 */ Vec3 velocity;
    /* 0xB0 */ s32 velocityPad;
    /* 0xB4 */ Vec3 unkB4;
    /* 0xC0 */ s32 unkB4Pad;
    /* 0xC4 */ s32 unkC4;        /* 0 to 7 at random */
    /* 0xC8 */ s32 unkC8;        /* the sphere's radius at first: 0x20 to 0x3F at random */
    /* 0xCC */ s32 unkCC;
    /* 0xD0 */ s32 state;        /* 0 at first; 2 once it hits a fighter, 3 when the fighter guards */
    /* 0xD4 */ s32 unkD4;        /* 15 to 30 at random */
    /* 0xD8 */ s32 unkD8;
    /* 0xDC */ s32 vramY;        /* Fighter_getVramY of the fighter */
} RenamonSpecial1Hitbox;

/* Renamon's class (CHARACTER_RENAMON). */
typedef struct Renamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 airAttackLanded; /* the air attack's frames on the ground: the clip stops at the second */
    /* 0x4A8 */ s32 special1AimY; /* the special move 1 hitbox's height: func_800A3710's aim (up -0x200) */
    /* 0x4AC */ s32 special1SoundTimer; /* 15 from Renamon_spawnSpecial1Hitbox; sound 0xCB when it runs out */
    /* 0x4B0 */ s32 vramCopied;         /* set once handleInput has copied vramAreas to vramCopies */
    /* 0x4B4 */ RECT vramAreas[2];      /* in the fighter's VRAM rows; func_800A4F64 moves the copies back here */
    /* 0x4C4 */ RECT vramCopies[2];     /* x and y: where handleInput copies vramAreas (0x80 lower) */
} Renamon;

extern FaceEntry RENAMON_FACES[];
extern EntityVtable RENAMON_VTABLE;

/* Builds Renamon: a Fighter with its own vtable and faces. */
Renamon *Renamon_init(Renamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &RENAMON_VTABLE;
    this->special1SoundTimer = 0;
    Fighter_setFaceTable(&this->base, RENAMON_FACES);
    this->airAttackLanded = 0;
    this->special1AimY = 0;
    this->vramCopied = 0;
    return this;
}

/* Fighter's input step; the special move 1's sound timer counts down to sound 0xCB. The first time, it sets up its
 * VRAM rectangles: copies the two areas of its rows 0x80 lower (MoveImage2) and clears the column at x 0x17F. */
void Renamon_handleInput(Renamon *this, void *input) {
    RECT column;
    s32 vramY;
    s32 i;
    s32 ranOut;

    Fighter_handleInput(this, input);
    /* The match depends on ranOut being a value (the game's store-flag), set after the call. */
    ranOut = 0;
    if (this->special1SoundTimer > 0) {
        this->special1SoundTimer--;
        ranOut = this->special1SoundTimer == 0;
    }
    if (ranOut) {
        Fighter_playSound(&this->base, 0xCB);
    }
    if (this->vramCopied == 0) {
        this->vramCopied = 1;
        vramY = Fighter_getVramY(&this->base);
        this->vramAreas[0].x = 0x140;
        this->vramAreas[0].y = vramY;
        this->vramAreas[0].w = 0x38;
        this->vramAreas[0].h = 0x40;
        this->vramAreas[1].x = 0x140;
        this->vramAreas[1].y = vramY + 0x40;
        this->vramAreas[1].w = 0x20;
        this->vramAreas[1].h = 0x40;
        this->vramCopies[0].x = 0x140;
        this->vramCopies[0].y = vramY + 0x80;
        this->vramCopies[0].w = 0x38;
        this->vramCopies[0].h = 0x40;
        this->vramCopies[1].x = 0x150;
        this->vramCopies[1].y = vramY + 0xC0;
        this->vramCopies[1].w = 0x20;
        this->vramCopies[1].h = 0x40;
        /* The match depends on one pointer per array, area first: the game walks two registers. */
        for (i = 0; i < 2; i++) {
            RECT *area = &this->vramAreas[i];
            RECT *copy = &this->vramCopies[i];

            MoveImage2(area, copy->x, copy->y);
        }
        column.x = 0x17F;
        column.y = vramY;
        column.w = 1;
        column.h = 0x80;
        ClearImage2(&column, 0, 0, 0);
    }
}

/* Ends a move: back to face 0 (Fighter_setFace). */
void Renamon_endMove(Fighter *this) {
    Fighter_setFace(this, 0);
}

/* Air attack, a dive: its clip at frame 0; before frame 8 the force brakes a quarter of the horizontal velocity and
 * any fall; at frame 8 it stops and runs the params' air attack; from frame 9 it dives forward and down. On the ground
 * it slows down and stops its clip at the second landed frame. */
void Renamon_airAttack(Renamon *this, s32 frame) {
    Attack *attack;

    func_80079AE4(&this->base);
    attack = &Fighter_getParams(&this->base)->attacks[8];
    if (frame == 0) {
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_AIR_ATTACK, 2, 0, -1);
        this->airAttackLanded = 0;
    }
    if (frame < 8) {
        this->base.force.vx = -this->base.velocity.vx >> 2;
        if (this->base.velocity.vy > 0) {
            this->base.force.vy = -this->base.velocity.vy;
        }
    }
    if (frame == 8) {
        this->base.force.vx = 0;
        this->base.force.vy = 0;
        this->base.force.vz = 0;
        this->base.forcePad = 0;
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.velocity.vz = 0;
        this->base.velocityPad = 0;
        Fighter_runAttack(&this->base, 0, attack, 1);
        Fighter_setAttackParts(&this->base, attack->parts);
    }
    if (frame >= 9) {
        /* The match depends on the two pushes in their own locals. */
        s32 push = ((s64)(Fighter_getFacing(&this->base) * 0x71C) * this->base.mass) >> 16;
        s32 drop = ((s64)this->base.mass * 0x71C) >> 16;

        this->base.force.vx += push;
        this->base.force.vy += drop;
    }
    if (Fighter_isOnGround(&this->base)) {
        this->base.force.vy = 0;
        this->base.force.vx -= this->base.velocity.vx >> 1;
        if (++this->airAttackLanded >= 2) {
            Fighter_stopActionClip(&this->base);
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_throw);

s32 func_800A3710(Fighter *this);
void Renamon_spawnSpecial1Effect(Fighter *this);
void Renamon_spawnSpecial1Hitbox(Renamon *this, Vec3 *offset);

/* Special move 1: Fighter's; frame 0 the voice, a stop, face 1 and the effect; frame 12 the hitbox 512 forward at
 * the aim's height (special1AimY); frame 50 face 0. Before frame 12 the aim (func_800A3710) is kept unless it is -1. */
void Renamon_special1(Renamon *this, s32 frame) {
    Vec3 offset;
    s32 aim;

    Fighter_special1(&this->base, frame);
    func_80079AE4(&this->base);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xCF);
        this->special1AimY = 0;
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.velocity.vz = 0;
        this->base.velocityPad = 0;
        Fighter_setFace(&this->base, 1);
        Renamon_spawnSpecial1Effect(&this->base);
        break;
    case 12:
        offset.vx = Fighter_getFacing(&this->base) * 512;
        offset.vy = this->special1AimY;
        offset.vz = 0;
        Renamon_spawnSpecial1Hitbox(this, &offset);
        break;
    case 50:
        Fighter_setFace(&this->base, 0);
        break;
    }
    if (frame < 12) {
        aim = func_800A3710(&this->base);
        if (aim != -1) {
            this->special1AimY = aim;
        }
    }
    this->base.unk4A0 = 1;
}

/* Special move 1 in the air: Fighter's; frame 0 the voice, the aim's height at 0x200, face 1 and the effect; frame 4
 * the hitbox 512 forward at the aim's height (special1AimY); frame 25 face 0. Before frame 4 the aim (func_800A3710)
 * is kept unless it is -1; from frame 4 the fighter stops and hangs, its fall braked. */
void Renamon_airSpecial1(Renamon *this, s32 frame) {
    Vec3 offset;
    s32 aim;

    Fighter_airSpecial1(&this->base, frame);
    func_80079AE4(&this->base);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xCF);
        this->special1AimY = 0x200;
        Fighter_setFace(&this->base, 1);
        Renamon_spawnSpecial1Effect(&this->base);
        break;
    case 4:
        offset.vx = Fighter_getFacing(&this->base) * 512;
        offset.vy = this->special1AimY;
        offset.vz = 0;
        Renamon_spawnSpecial1Hitbox(this, &offset);
        break;
    case 25:
        Fighter_setFace(&this->base, 0);
        break;
    }
    if (frame < 4) {
        aim = func_800A3710(&this->base);
        if (aim != -1) {
            this->special1AimY = aim;
        }
    } else {
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.force.vx = 0;
        this->base.force.vy -= ((s64)this->base.mass * 0x71C) >> 16;
    }
    this->base.unk4A0 = 1;
}

s32 func_800A3754(Fighter *this);
s32 func_800A37F4(Fighter *this);

/* The special move 1's aim: func_800A3754 (the input's) for a player, func_800A37F4 for the computer. The match
 * depends on one result variable (two returns flip the test). */
s32 func_800A3710(Fighter *this) {
    s32 aim;

    if (Fighter_isComputer(this) == 0) {
        aim = func_800A3754(this);
    } else {
        aim = func_800A37F4(this);
    }
    return aim;
}

/* A value from the character's input (InputHistory_getHeld): up gives 0, or -0x200 unless Fighter_getAttackStateId is
 * 30; down gives 0x200; holding the way it faces (Fighter_getFacing) gives 0, anything else -1. */
s32 func_800A3754(Fighter *this) {
    u32 input = InputHistory_getHeld(Fighter_getInput(this));
    s32 facing;

    if (input & PAD_UP) {
        return Fighter_getAttackStateId(this) == FIGHTER_STATE_AIR_SPECIAL1 ? 0 : -0x200;
    }
    if (input & PAD_DOWN) {
        return 0x200;
    }
    facing = Fighter_getFacing(this);
    if (facing > 0 && (input & PAD_RIGHT)) {
        return 0;
    }
    if (facing >= 0) {
        return -1;
    }
    if (input & PAD_LEFT) {
        return 0;
    }
    return -1;
}

/* The computer's func_800A3754: aims from the character's point (func_8007A650) at func_8007A6CC's, not below it in
 * the air special move 1. Facing away gives -1; otherwise -0x200 for over 0x100 of angle down, 0x200 for over 0x100
 * up, else 0. The match depends on one variable for the side and then the angle (both in s0), the absolute value
 * written in ratan2's argument and the angle tested against -0x100 first. */
s32 func_800A37F4(Fighter *this) {
    Vec3 from;
    Vec3 to;
    s32 direction;
    s32 dx;

    func_8007A650(this, &from);
    func_8007A6CC(this, &to);
    if (Fighter_getAttackStateId(this) == FIGHTER_STATE_AIR_SPECIAL1 && to.vy < from.vy) {
        to.vy = from.vy;
    }
    direction = 1;
    if (to.vx - from.vx < 0) {
        direction = -1;
    }
    if (Fighter_getFacing(this) != direction) {
        return -1;
    }
    dx = to.vx - from.vx;
    direction = ratan2(to.vy - from.vy, dx < 0 ? -dx : dx);
    if (direction >= -0x100) {
        return direction > 0x100 ? 0x200 : 0;
    }
    return -0x200;
}

RenamonSpecial1Hitbox *RenamonSpecial1Hitbox_init(RenamonSpecial1Hitbox *this, Fighter *fighter, Vec3 *position,
                                                  Vec3 *point, Vec3 *offset);

/* Spawns one special move 1 hitbox and hands it to Fighter_addEntity. The match depends on this inline: it keeps
 * &position in a saved register. */
static inline void spawnSpecial1Hitbox(Renamon *this, Vec3 *position, Vec3 *point, Vec3 *offset) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        RenamonSpecial1Hitbox_init(entityOperatorNewAt(0xE0, child), &this->base, position, point, offset);
    }
    Fighter_addEntity(&this->base, child);
}

/* Spawns the special move 1's 48 hitboxes at the fighter's position, each with a point up to 0xFF ahead, behind or
 * above 0x180 up from it, and the offset; then starts the sound timer (15) and func_80079B0C(60). */
void Renamon_spawnSpecial1Hitbox(Renamon *this, Vec3 *offset) {
    MersenneTwister *random = &RANDOM;
    Vec3 position;
    Vec3 point;
    s32 i;

    for (i = 0; i < 48; i++) {
        s32 x, y, z;

        Fighter_getPosition(&this->base, &position);
        /* The match depends on the position read into x, y and z before it is stored. */
        x = position.vx;
        y = position.vy;
        z = position.vz;
        point.vx = x;
        point.vy = y;
        point.vz = z;
        point.vx += (s32)mersenneTwisterGenerate(random) % 256;
        /* The match depends on this form: (vy - 0x180) - r, not vy - (r + 0x180). */
        point.vy += -0x180 - (s32)mersenneTwisterGenerate(random) % 256;
        point.vz += (s32)mersenneTwisterGenerate(random) % 256;
        spawnSpecial1Hitbox(this, &position, &point, offset);
    }
    this->special1SoundTimer = 15;
    func_80079B0C(&this->base, 60);
}

/* Spawns the special move 1's effect. */
void Renamon_spawnSpecial1Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        RenamonSpecial1Effect_init(entityOperatorNewAt(0x44, child), this);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_special2);

/* A burst of 9 light blue particles at a fighter's point (func_8007A650), for the swap. */
static inline Entity *Renamon_newSwapBurst(Fighter *fighter) {
    Vec3 position;
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 9, func_8007A650(fighter, &position), 0x40, 0x20, -0x1E,
                           0x80C0FF);
    }
    return child;
}

/* Special move 2's swap (Renamon_special2): trades position, last position, velocity and force with the opponent (each
 * a Vec3 and its pad word, four words), back on the z = 0 plane, with a burst at both; then faces the opponent, is
 * invulnerable for 8 frames, func_80079B14(15) and sound 0xCC. */
void Renamon_swapWithOpponent(Fighter *this) {
    Fighter *opponent = Fighter_getOpponent(this);

    swapFourWords((u32 *)&this->position, (u32 *)&opponent->position);
    swapFourWords((u32 *)&this->lastPosition, (u32 *)&opponent->lastPosition);
    swapFourWords((u32 *)&this->velocity, (u32 *)&opponent->velocity);
    swapFourWords((u32 *)&this->force, (u32 *)&opponent->force);
    this->position.vz = 0;
    Fighter_addEntity(this, Renamon_newSwapBurst(this));
    Fighter_addEntity(this, Renamon_newSwapBurst(opponent));
    if (opponent->position.vx - this->position.vx < 0) {
        Fighter_faceLeft(this, 0);
    } else {
        Fighter_faceRight(this, 0);
    }
    this->invulnerableFrames = 8;
    func_80079B14(this, 15);
    Fighter_playSound(this, 0xCC);
}

/* Builds a special move 1 hitbox of fighter at position (a sphere of random radius 0x20 to 0x3F), at rest, with point
 * and offset kept, a random 0-7 at 0xC4 and 15-30 at 0xD4. */
RenamonSpecial1Hitbox *RenamonSpecial1Hitbox_init(RenamonSpecial1Hitbox *this, Fighter *fighter, Vec3 *position,
                                                  Vec3 *point, Vec3 *offset) {
    s32 radius;
    s32 x, y, z;
    s32 px, py, pz;
    s32 ox, oy, oz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, 0x4A80300, (Vec3 *)&ZERO_VECTOR, 0);
    this->parts.base.vtable = RENAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &RENAMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0, &this->parts.base);
    radius = (mersenneTwisterGenerate(&RANDOM) & 0x1F) + 0x20;
    this->parts.part.entity = &this->parts.entity;
    this->parts.part.radius = radius;
    this->ownerHandle = entityGetHandle(&fighter->entity);
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    px = point->vx;
    py = point->vy;
    pz = point->vz;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocityPad = 0;
    this->hitOffset.vx = px;
    this->hitOffset.vy = py;
    this->hitOffset.vz = pz;
    ox = offset->vx;
    oy = offset->vy;
    oz = offset->vz;
    this->unkB4.vx = ox;
    this->unkB4.vy = oy;
    this->unkB4.vz = oz;
    this->unkC4 = mersenneTwisterGenerate(&RANDOM) & 7;
    this->unkC8 = radius;
    this->unkCC = 0;
    this->state = 0;
    this->unkD4 = (mersenneTwisterGenerate(&RANDOM) & 0xF) + 0xF;
    this->unkD8 = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_tick);

/* On a hit: Hit_apply; a guarding fighter bounces it back at a quarter of its speed (state 3, sound 0xB), any other
 * keeps its offset from it (state 2); either way the sphere leaves its list. The match depends on reading the offset
 * whole before subtracting (as vecSubInPlace). */
void RenamonSpecial1Hitbox_hit(RenamonSpecial1Hitbox *this, CollisionSphere *sum) {
    Fighter *target;
    ListNode *node;
    s32 x, y, z;

    Hit_apply(&this->parts.base, sum);
    target = (Fighter *)getHitFighter(sum);
    if (target == NULL) {
        return;
    }
    if (Fighter_isGuarding(target)) {
        this->state = 3;
        this->velocity.vx = -this->velocity.vx >> 2;
        this->velocity.vy = -this->velocity.vy >> 2;
        soundControlPlaySound(&SOUND_CONTROL, 0xB, 0);
    } else {
        Fighter_getPosition(target, &this->hitOffset);
        this->state = 2;
        x = this->hitOffset.vx;
        y = this->hitOffset.vy;
        z = this->hitOffset.vz;
        x -= this->position.vx;
        y -= this->position.vy;
        z -= this->position.vz;
        this->hitOffset.vx = x;
        this->hitOffset.vy = y;
        this->hitOffset.vz = z;
    }
    /* listRemove(&this->parts.part.link), in the access order the game's compiler used */
    node = &this->parts.part.link;
    this->parts.part.link.next->prev = this->parts.part.link.prev;
    node->prev->next = this->parts.part.link.next;
    this->parts.part.link.next = node;
    this->parts.part.link.prev = node;
}

/* Spawns the entity of Sparkle_init at the hitbox's position with its velocity, as a sibling. */
void func_800A4550(RenamonSpecial1Hitbox *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        Sparkle_init(entityOperatorNewAt(0x5C, child), &this->position, &this->velocity, 0x10, 0x408080, 9);
    }
    entityAddSibling(&this->parts.entity, child);
}

/* Spawns the entity of ParticleBurst_init at the hitbox's position, as a sibling. */
void func_800A45C8(RenamonSpecial1Hitbox *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 9, &this->position, 0x20, 0x10, 8, 0x408080);
    }
    entityAddSibling(&this->parts.entity, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_draw);

extern EntityVtable RENAMON_SPECIAL1_EFFECT_VTABLE;

/* The special move 1's effect (0x44 bytes). */
/* RenamonSpecial1Effect.state */
enum {
    RENAMON_SPECIAL1_EFFECT_GROW,  /* to size 0x180 */
    RENAMON_SPECIAL1_EFFECT_PULSE, /* until frame 11 or the attack is over */
    RENAMON_SPECIAL1_EFFECT_FADE   /* grows to 0x380 and fades, then deletes itself */
};

typedef struct RenamonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* the character's at construction (Fighter_getPosition) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 size;       /* half the square's side: grows to 0x180, pulses, then grows to 0x380 */
    /* 0x30 */ s32 brightness; /* 0x80, then fades as the size nears 0x380 */
    /* 0x34 */ s32 facing;     /* Fighter_getFacing: 1 or -1 */
    /* 0x38 */ s32 state;      /* RENAMON_SPECIAL1_EFFECT_* */
    /* 0x3C */ s32 frames;
    /* 0x40 */ s32 vramY; /* Fighter_getVramY of the character */
} RenamonSpecial1Effect;

/* Builds the special move 1's effect for fighter, at its position and facing its way. */
RenamonSpecial1Effect *RenamonSpecial1Effect_init(RenamonSpecial1Effect *this, Fighter *fighter) {
    Vec3 position;
    s32 facing;

    entityInit(&this->entity, 0);
    this->entity.vtable = &RENAMON_SPECIAL1_EFFECT_VTABLE;
    facing = Fighter_getFacing(fighter);
    Fighter_getPosition(fighter, &position);
    this->fighter = fighter;
    this->size = 0;
    this->brightness = 0x80;
    this->facing = facing;
    this->state = RENAMON_SPECIAL1_EFFECT_GROW;
    this->frames = 0;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The special move 1 effect's frame: it stays 128 in front of the character and 0xE0 up; it grows to 0x180, pulses
 * (size ^ 0x10) until frame 11 or the character's attack is over, then grows to 0x380 fading and deletes itself. */
void RenamonSpecial1Effect_tick(RenamonSpecial1Effect *this) {
    Vec3 position;

    Fighter_getPosition(this->fighter, &position);
    position.vx += this->facing << 7;
    position.vy -= 0xE0;
    this->position = position;
    switch (this->state) {
    case RENAMON_SPECIAL1_EFFECT_GROW:
        this->size += (0x181 - this->size) >> 1;
        if (this->size >= 0x180) {
            this->size = 0x180;
            this->state = RENAMON_SPECIAL1_EFFECT_PULSE;
        }
        break;
    case RENAMON_SPECIAL1_EFFECT_PULSE:
        this->size ^= 0x10;
        if (this->frames >= 11 || Fighter_isAttackIdle(this->fighter)) {
            this->state++;
        }
        break;
    case RENAMON_SPECIAL1_EFFECT_FADE:
        this->size += (0x383 - this->size) >> 2;
        this->brightness = (0x380 - this->size) >> 2;
        if (this->size >= 0x380) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Effect_draw);

/* The special move 2's effect (Renamon_special2 spawns it, 0x44 bytes): a sprite rising and drifting in x and z. */
typedef struct RenamonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* the character's, moved by up to 127 in x and z */
    /* 0x24 */ s32 positionPad;
    /* 0x28 */ Vec3 velocity;
    /* 0x34 */ s32 velocityPad;
    /* 0x38 */ s32 framesLeft; /* 0x18 to 0x27 at construction; it also picks the drift angle and the cell */
    /* 0x3C */ s32 vramY; /* Fighter_getVramY of the character */
    /* 0x40 */ s32 clutY; /* vramY + 0xFE, or + 0xFF one time in four */
} RenamonSpecial2Effect;

extern EntityVtable RENAMON_SPECIAL2_EFFECT_VTABLE;

/* Constructs the effect at the character's position moved by 0 to 127 in x and z, still, with 0x18 to 0x27
 * frames to live and the character's clut row 0xFE further down, or 0xFF one time in four.
 * The match depends on the generator's address taken in a local after the vtable is set. */
RenamonSpecial2Effect *RenamonSpecial2Effect_init(RenamonSpecial2Effect *this, Fighter *fighter) {
    MersenneTwister *random;

    entityInit(&this->entity, 0);
    this->entity.vtable = &RENAMON_SPECIAL2_EFFECT_VTABLE;
    random = &RANDOM;
    Fighter_getPosition(fighter, &this->position);
    this->position.vx += (s32)mersenneTwisterGenerate(random) % 128;
    this->position.vz += (s32)mersenneTwisterGenerate(random) % 128;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocityPad = 0;
    this->framesLeft = (mersenneTwisterGenerate(random) & 0xF) + 0x18;
    this->vramY = Fighter_getVramY(fighter);
    this->clutY = this->vramY + 0xFE;
    this->clutY += (mersenneTwisterGenerate(random) & 3) == 0;
    return this;
}

/* The effect's tick: drifts in x and z by up to 3 steps along the angle framesLeft * 0x100, rises by 8 to 23,
 * moves, and deletes itself when framesLeft runs out. */
void RenamonSpecial2Effect_tick(RenamonSpecial2Effect *this) {
    s32 step;
    s32 x, y, z;

    step = mersenneTwisterGenerate(&RANDOM) & 3;
    this->velocity.vx += (step * COS_SIN_TABLE[(this->framesLeft << 8) & 0xF00].sin) >> 12;
    this->velocity.vz += (step * COS_SIN_TABLE[(this->framesLeft << 8) & 0xF00].cos) >> 12;
    this->velocity.vy = -(mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->framesLeft--;
    if (this->framesLeft < 0) {
        entityDelete(&this->entity);
    }
}

/* PolyFT4Shape's uv words as their u and v bytes (as in saintgargomon.c): game_classes.h types them u16, and the game
 * stores the bytes. */
#define UV_BYTES(uv) ((u8 *)&(uv))

/* The corners of a 64 by 64 square around the origin. */
extern SVECTOR RENAMON_SPECIAL2_EFFECT_CORNERS[4];

/* Draws the effect: the square RENAMON_SPECIAL2_EFFECT_CORNERS at its position, with the 16 by 16 cell (framesLeft & 3)
 * of the texture row at 0x30 and the clut row clutY, with buildPolyFT4.
 * The match depends on the matrix built as matrixInitTranslationVec does, reading the position through a pointer, on
 * u in a u8 (lbu) and on the color set after the uv bytes. */
void RenamonSpecial2Effect_draw(RenamonSpecial2Effect *this, Camera *camera) {
    MatrixWords m;
    PolyFT4Shape shape;
    Vec3 *pos;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 x, y, z;
    u8 u;

    pos = &this->position;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    packet = PRIM_BUFFER_FREE;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    setModelMatrix(&m.m);
    shape.flags = 0;
    shape.corners[0] = &RENAMON_SPECIAL2_EFFECT_CORNERS[0];
    shape.corners[1] = &RENAMON_SPECIAL2_EFFECT_CORNERS[1];
    shape.corners[2] = &RENAMON_SPECIAL2_EFFECT_CORNERS[2];
    shape.corners[3] = &RENAMON_SPECIAL2_EFFECT_CORNERS[3];
    u = ((this->framesLeft & 3) << 4) | 0xC0;
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = u;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = u + 0xF;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0x30;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0x3F;
    shape.color = 0x808080;
    shape.clut = getClut(0x1D0, this->clutY);
    shape.tpage = getTPage(0, 0, 0x1C0, this->vramY);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, zShift);
}

extern EntityVtable D_8006A9A4;

/* The entity class of vtable D_8006A9A4 (0xB4 bytes) that the special move 2 spawns: it moves two VRAM rectangles
 * of the fighter's (func_800A51DC, DR_MOVE), first as 56 one-pixel columns of growing heights (func_800A5064). */
typedef struct Unk800A4F64 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ RECT from[2];     /* the VRAM rectangles moved */
    /* 0x2C */ RECT to[2];       /* x and y: where to */
    /* 0x3C */ s32 state;        /* func_800A51DC's: 0 the columns, 1 the whole rectangles */
    /* 0x40 */ s32 frames;
    /* 0x44 */ s16 heights[56];  /* of the columns: + 6 to 9 a frame, up to 0x80 */
} Unk800A4F64;

/* Builds the entity for fighter, copying the rectangles from and to, all its columns at height 0. */
Unk800A4F64 *func_800A4F64(Unk800A4F64 *this, Fighter *fighter, RECT *from, RECT *to) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006A9A4;
    this->fighter = fighter;
    this->from[0] = from[0];
    this->from[1] = from[1];
    this->to[0] = to[0];
    this->to[1] = to[1];
    this->state = 0;
    this->frames = 0;
    for (i = 55; i >= 0; i--) {
        this->heights[i] = 0;
    }
    return this;
}

/* Grows the columns by 6 to 9 a frame, up to 0x80, until frame 24 or until the fighter's attack stops or it is held;
 * once the draw has moved the whole rectangles, sets faces 1 then 0 and deletes the entity. */
void func_800A5064(Unk800A4F64 *entity) {
    /* The match depends on the copy: as the parameter, this and the loop counter swap s2 and s3. */
    Unk800A4F64 *this = entity;
    s32 interrupted = 0;
    MersenneTwister *random;
    s32 i;

    if (Fighter_isAttackIdle(this->fighter) || Fighter_getBodyStateId(this->fighter) == FIGHTER_STATE_HELD) {
        interrupted = 1;
    }
    random = &RANDOM;
    switch (this->state) {
    case 0:
        for (i = 0; i < 56; i++) {
            s32 height = this->heights[i];

            /* `height + 6 + rand` would fold to height + (rand + 6) */
            height += 6 + (mersenneTwisterGenerate(random) & 3);
            if (height > 0x80) {
                height = 0x80;
            }
            this->heights[i] = height;
        }
        if (this->frames >= 24 || interrupted) {
            this->state++;
        }
        break;
    case 1:
        break;
    case 2:
        Fighter_setFace(this->fighter, 1);
        Fighter_setFace(this->fighter, 0);
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
}

/* Moves the VRAM rectangles: in state 0 the 56 one-pixel columns at to[0], each copied from the column just right of
 * them at its height; then, once, the whole rectangles from to their destinations. */
void func_800A51DC(Unk800A4F64 *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    u_long *ot = camera->ot.slots;
    s32 i;

    switch (this->state) {
    case 0: {
        RECT column;
        s16 x = this->to[0].x;
        s16 y = this->to[0].y;

        memset(&column, 0, sizeof(column));
        column.w = 1;
        column.x = x + 63;
        column.y = y;
        for (i = 0; i < 56; i++) {
            column.h = this->heights[i];
            AddPrim(ot, allocDrMove(&column, x + i, y));
        }
        break;
    }
    case 1:
        for (i = 0; i < 2; i++) {
            AddPrim(ot, allocDrMove(&this->from[i], this->to[i].x, this->to[i].y));
        }
        this->state++;
        break;
    }
}

/* Renamon's destructor. */
void Renamon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 1 hitbox's destructor. */
void RenamonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = RENAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &RENAMON_SPECIAL1_HITBOX_VTABLE;
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

/* The special move 1 effect's destructor. */
void RenamonSpecial1Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 2 effect's destructor. */
void RenamonSpecial2Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9A4, which func_800A4F64 installs. */
void func_800A5480(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A924);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A92C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A934);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A93C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A944);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A94C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A954);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A964);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL2_EFFECT_CORNERS);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A9A4);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_VTABLE);
