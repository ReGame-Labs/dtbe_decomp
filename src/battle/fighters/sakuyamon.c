/* Sakuyamon (CHARACTER_SAKUYAMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the attacks' trails, the special move 1's controller and its four hitboxes, the special move 2's hitbox, the
 * special move 3's effect, and the intro's hitbox. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

void Sakuyamon_spawnTrail(Fighter *this, s32 arg);
void func_800A7290(Entity *this, s32 arg);
void func_800A7458(Entity *this, s32 arg);
void func_800A8C24(Entity *this, s32 arg);
void func_800A8F80(Entity *this, s32 arg);
extern VtableEntry D_8006AF2C[];
extern EntityVtable D_8006AF4C;
extern VtableEntry D_8006AFC4[];
extern EntityVtable D_8006AFE4;
extern EntityVtable D_8006B00C;
extern EntityVtable D_8006B03C;
extern EntityVtable D_8006AF74;
extern EntityVtable D_8006AF9C;

/* Sakuyamon's class (CHARACTER_SAKUYAMON): special1 keeps the handles of its four hitboxes (func_800A5B98). */
typedef struct Sakuyamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 childHandles[4]; /* entity handles from func_800A5B98 */
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
} Sakuyamon;

/* The entity class of func_800A6220 (virtual table D_8006B03C), which func_800AA0B4's derives from. */
typedef struct Unk800A6220 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ InputHistory *input; /* the character's (Fighter_getInput) */
    /* 0x20 */ Vec3 position;      /* the character's, at construction */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ VECTOR direction; /* 96 the way the character faces at first; eases to the getDirection one */
} Unk800A6220;

/* The virtual table of func_800A6220's class (D_8006B03C) and of its derived one (D_8006B00C). */
typedef struct Unk800A6220Vtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry getDirection; /* (Unk800A6220 *, VECTOR *out): the computer's is func_800A6550 */
} Unk800A6220Vtable;

Unk800A6220 *func_800A6220(Unk800A6220 *this, Fighter *fighter);

extern EntityVtable SAKUYAMON_VTABLE;

/* The classes of D_8006AF2C and D_8006AE8C (virtual tables of their base at
 * 0x18), whose methods func_800A7F88 and func_800A9944 are. */
typedef struct Unk800A7F88 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ CollisionSphere part;
    /* 0x80 */ s32 fighterHandle;
    /* 0x84 */ Vec3 start; /* func_800A7FEC puts the part at start + unk94 * unkB0 / 4 (its asm) */
    /* 0x90 */ s32 startPad;
    /* 0x94 */ Vec3 unk94; /* the part's centre at first */
    /* 0xA0 */ s32 unk94Pad;
    /* 0xA4 */ s32 unkA4;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 vramY; /* Fighter_getVramY of the fighter */
} Unk800A7F88;

extern VtableEntry D_8006AE8C[];
extern EntityVtable D_8006AEAC;

typedef struct SakuyamonSpecial3Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ VECTOR position; /* the character's node 19 (offset D_8006B198), while state is below 3 */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;   /* + 0x20 a frame */
    /* 0x3C */ s32 unk3C;   /* 0 to 5, a step a frame */
    /* 0x40 */ s32 state;   /* 0 to 3 (SakuyamonSpecial3Effect_tick) */
    /* 0x44 */ s32 frames;
    /* 0x48 */ s32 vramY; /* Fighter_getVramY of the character */
} SakuyamonSpecial3Effect;

extern EntityVtable SAKUYAMON_SPECIAL3_EFFECT_VTABLE;

typedef struct Unk800AA04C {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ Vec3 unk20;
} Unk800AA04C;
extern TrailPoint D_8006B1AC[];

/* Builds Sakuyamon: a Fighter with its own vtable, its seven words 0. The game clears the handles last to first: the
 * match depends on the chained assignment. */
Sakuyamon *Sakuyamon_init(Sakuyamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &SAKUYAMON_VTABLE;
    this->unk4B8 = 0;
    this->unk4A4 = 0;
    this->childHandles[0] = this->childHandles[1] = this->childHandles[2] = this->childHandles[3] = 0;
    this->unk4BC = 0;
    return this;
}

/* Ends a move: func_80079B0C(20) while 0x4A4 is set. */
void Sakuyamon_endMove(Sakuyamon *this) {
    if (this->unk4A4 != 0) {
        func_80079B0C(&this->base, 20);
    }
}

/* Attack 1: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_attack1(Fighter *this, s32 frame) {
    Fighter_attack1(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Attack 2: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Attack 3: Fighter's, with a trail (20) at frame 0. */
void Sakuyamon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 20);
    }
}

/* Up attack: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Dash attack: Fighter's, with a trail (8) at frame 15. */
void Sakuyamon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 15) {
        Sakuyamon_spawnTrail(this, 8);
    }
}

/* Up attack in the air: Fighter's, with a trail (17) at frame 0. */
void Sakuyamon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 17);
    }
}

/* Spawns the attacks' trail (Trail_init with D_8006B1AC, 3, 2) lasting length. */
void Sakuyamon_spawnTrail(Fighter *this, s32 length) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, D_8006B1AC, 3, 2, length);
    }
    Fighter_addEntity(this, child);
}

Fighter *func_800A58C8(Fighter *this);

/* Throw: Fighter's, with the fighter braking to a stop. Frame 0 holds the opponent; frame 1 stops it and hits it up;
 * at frame 18, if the fighter is on the ground and func_800A58C8 finds the opponent, it is stopped and hit
 * forward and down, else the clip stops. Each hit has a spark. */
void Sakuyamon_throw(Fighter *this, s32 frame) {
    Fighter *opponent;
    Vec3 vel;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    Fighter_accelerateTo(this, 0);
    switch (frame) {
    case 0:
        Fighter_hold(opponent, -1);
        break;
    case 1:
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        opponent->force.vx = 0;
        opponent->force.vy = 0;
        opponent->force.vz = 0;
        opponent->forcePad = 0;
        vel.vx = 0;
        vel.vy = -0x6388;
        vel.vz = 0;
        Fighter_takeHit(opponent, 0x408C1000, &vel, 0);
        Fighter_spawnHitSpark(this, 0x100);
        break;
    case 18:
        if (Fighter_isOnGround(this) && func_800A58C8(this) != NULL) {
            opponent->velocity.vx = 0;
            opponent->velocity.vy = 0;
            opponent->velocity.vz = 0;
            opponent->velocityPad = 0;
            opponent->force.vx = 0;
            opponent->force.vy = 0;
            opponent->force.vz = 0;
            opponent->forcePad = 0;
            vel.vx = Fighter_getFacing(this) * 0x6388;
            vel.vy = 0x8714;
            vel.vz = 0;
            Fighter_takeHit(opponent, 0x408C2000, &vel, 0);
            Fighter_spawnHitSpark(this, 0x100);
        } else {
            Fighter_stopActionClip(this);
        }
        break;
    }
}

extern VECTOR D_8006AB8C;

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AB8C);

/* func_80079E50 with D_8006AB8C, 0x100000 (0x400 squared: Fighter_getOpponentInReach passes a squared distance) and
 * 0x2AA; Sakuyamon_throw tests it at frame 18. */
Fighter *func_800A58C8(Fighter *this) {
    return func_80079E50(this, &D_8006AB8C, 0x100000, 0x2AA);
}

/* Sakuyamon_special1's four corners (+-0x200, +-0x200), passed in pairs to func_800A5B98. One array: the game
 * derives the second address from the first (a1 + 0x10, a1 - 0x30). */
extern VECTOR D_8006AB9C[4];

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AB9C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABAC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABBC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABCC);

void func_800A5AD0(Sakuyamon *this);
s32 func_800A5B98(Sakuyamon *this, VECTOR *from, VECTOR *to);

/* Special move 1: frame 0 the voice and clip 22, frame 15 the controller (func_800A5AD0), frames 16, 21, 26 and 31 keep
 * the handle of a hitbox for a pair of corners (func_800A5B98); the force brakes half the horizontal velocity. From
 * frame 34 it clears the handles of dead hitboxes and ends with Fighter_stopActionClip, unless button 0x40 is up, the
 * fighter is on the ground, one hitbox lives, Fight_isRoundOver does not hold and either func_800777BC holds or the
 * gauge is positive. */
void Sakuyamon_special1(Sakuyamon *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xD8);
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_SPECIAL1, 0, 0, 32);
        break;
    case 15:
        func_800A5AD0(this);
        break;
    case 16:
        this->childHandles[0] = func_800A5B98(this, &D_8006AB9C[0], &D_8006AB9C[1]);
        break;
    case 21:
        this->childHandles[1] = func_800A5B98(this, &D_8006AB9C[1], &D_8006AB9C[0]);
        break;
    case 26:
        this->childHandles[2] = func_800A5B98(this, &D_8006AB9C[2], &D_8006AB9C[1]);
        break;
    case 31:
        this->childHandles[3] = func_800A5B98(this, &D_8006AB9C[3], &D_8006AB9C[0]);
        break;
    }
    this->base.force.vx -= this->base.velocity.vx >> 1;
    if (frame >= 34) {
        if (!(InputHistory_getPressed(Fighter_getInput(&this->base)) & 0x40) && Fighter_isOnGround(&this->base)) {
            s32 alive = 0;
            s32 i;

            for (i = 0; i < 4; i++) {
                if (getEntityByHandle(this->childHandles[i]) != NULL) {
                    alive++;
                } else {
                    this->childHandles[i] = 0;
                }
            }
            if (alive != 0 && !Fight_isRoundOver(Fighter_getScene(&this->base))
                && (func_800777BC(&this->base) || func_8007758C(&this->base)->gauge > 0)) {
                return;
            }
        }
        Fighter_stopActionClip(&this->base);
    }
}

/* func_800AA0B4's body: the constructor of the class of vtable D_8006B00C, derived from func_800A6220's, which
 * g++ inlined into func_800A5AD0. */
static inline Unk800A6220 *constructUnk800AA0B4(Unk800A6220 *this, Fighter *fighter) {
    func_800A6220(this, fighter);
    this->entity.vtable = &D_8006B00C;
    return this;
}

/* Creates the entity of func_800A6220's class, or of its derived class (D_8006B00C) when Fighter_isComputer, hands it
 * to Fighter_addEntity and keeps its handle at 0x4A4, when it was allocated. */
void func_800A5AD0(Sakuyamon *this) {
    Entity *entity;

    if (Fighter_isComputer(&this->base) == 0) {
        entity = NEW_ENTITY(0x40, func_800A6220, &this->base);
    } else {
        entity = NEW_ENTITY(0x40, constructUnk800AA0B4, &this->base);
    }
    if (entity != NULL) {
        Fighter_addEntity(&this->base, entity);
        this->unk4A4 = entityGetHandle(entity);
    }
}

UnkPartsEntity38 *func_800A65F8(UnkPartsEntity38 *this, Fighter *fighter, s32 handle, VECTOR *from, VECTOR to);
/* The entity func_800A65F8 builds (0xD8 bytes, an UnkPartsEntity38 at its start). */
typedef struct Unk800A65F8 {
    /* 0x00 */ UnkPartsEntity38 base;
    /* 0x80 */ Fighter *fighter;
    /* 0x84 */ s32 handle;
    /* 0x88 */ VECTOR offset;   /* from: turned about axis around the Unk800A6220 (func_800A6AB0) */
    /* 0x98 */ VECTOR axis;     /* to, normalized */
    /* 0xA8 */ s32 angle;       /* 0 at first; +0x40 a frame */
    /* 0xAC */ s32 unkAC;       /* 12 at first */
    /* 0xB0 */ VECTOR position; /* func_8007A650 of the fighter at first; the sphere's centre */
    /* 0xC0 */ s32 unkC0; /* -4 to 4: one step a frame toward the side the sphere moves */
    /* 0xC4 */ s32 unkC4; /* half unkC0's size, or 2 to 7 by the frame count once that is 2 */
    /* 0xC8 */ s32 unkC8; /* 0 at first; func_800A6BE4 keeps it within the distance back to the fighter; the
                           * sphere's radius is twice it */
    /* 0xCC */ s32 unkCC; /* 4 less a frame once the Unk800A6220 is gone; deleted at 0 */
    /* 0xD0 */ s32 frames;
    /* 0xD4 */ s32 vramY;       /* Fighter_getVramY of the fighter */
} Unk800A65F8;

/* A point of an Unk800A6EB8. */
typedef struct Unk800A6EB8Point {
    /* 0x00 */ Vec3 position;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10; /* the point's index & 7 at first */
    /* 0x14 */ s32 unk14;
} Unk800A6EB8Point;

/* The entity of vtable D_8006AF9C (func_800A6EB8, 0xC8 bytes): six points, all at an Unk800A65F8's position at
 * first. */
typedef struct Unk800A6EB8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 handle; /* the Unk800A65F8's */
    /* 0x1C */ s32 unk1C;  /* 0x80 at first; the Unk800A7694s copy it every tick */
    /* 0x20 */ u8 unk20[4];
    /* 0x24 */ s32 vramY;  /* the Unk800A65F8's, 0 without one */
    /* 0x28 */ VECTOR unk28;
    /* 0x38 */ Unk800A6EB8Point points[6];
} Unk800A6EB8;

/* The entity of vtable D_8006AF74 (func_800A7694, 0x78 bytes), two per Unk800A6EB8: one on each side. */
typedef struct Unk800A7694 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 handle;        /* the Unk800A6EB8's: the tick deletes the entity once it is gone */
    /* 0x1C */ MatrixWords unk1C; /* the tick's rotation and scale */
    /* 0x3C */ MatrixWords unk3C; /* unk1C times a rotation moved 0x40 along y */
    /* 0x5C */ s32 side;          /* 0 or 1: the pair of the parent's points it follows, the rotations' signs */
    /* 0x60 */ s32 unk60;         /* the parent's unk1C, copied every tick */
    /* 0x64 */ s32 unk64;         /* frames & 3 */
    /* 0x68 */ s32 unk68;
    /* 0x6C */ s32 unk6C;
    /* 0x70 */ s32 frames;
    /* 0x74 */ s32 vramY;         /* the parent's, 0 without one */
} Unk800A7694;

Unk800A6EB8 *func_800A6EB8(Unk800A6EB8 *this, s32 handle);
Unk800A7694 *func_800A7694(Unk800A7694 *this, s32 handle, s32 side);

/* Builds the entities for a pair of corners while the entity at 0x4A4 is alive: func_800A65F8's (0xD8 bytes),
 * then func_800A6EB8's (0xC8) on its handle and func_800A7694's two (0x78, 0 and 1) on that one's handle, each
 * handed to Fighter_addEntity; plays sound 0xD1. Returns the first one's handle, or 0. */
s32 func_800A5B98(Sakuyamon *this, VECTOR *from, VECTOR *to) {
    Entity *parts;
    s32 handle;

    if (getEntityByHandle(this->unk4A4) == NULL) {
        return 0;
    }
    handle = this->unk4A4;
    parts = NEW_ENTITY(0xD8, func_800A65F8, &this->base, handle, from, *to);
    if (parts != NULL) {
        Entity *entity;

        Fighter_addEntity(&this->base, parts);
        entity = NEW_ENTITY(0xC8, func_800A6EB8, entityGetHandle(parts));
        if (entity != NULL) {
            Fighter_addEntity(&this->base, entity);
            Fighter_addEntity(&this->base, NEW_ENTITY(0x78, func_800A7694, entityGetHandle(entity), 0));
            Fighter_addEntity(&this->base, NEW_ENTITY(0x78, func_800A7694, entityGetHandle(entity), 1));
        }
        Fighter_playSound(&this->base, 0xD1);
        return entityGetHandle(parts);
    }
    return 0;
}

void func_800A5F94(Sakuyamon *this);
void func_800A605C(Sakuyamon *this, Vec3 *velocity);

/* Special move 2: Fighter's; leaving the ground before frame 16 stops the clip. The voice at frame 0, the hitbox at
 * frame 3 (func_800A5F94), thrown forward and up by 128 at frame 16 (func_800A605C); at frame 20 a stop, a hop of
 * 0x3FFC times the mass (16.16) and dust both ways. The force brakes to a quarter of the horizontal velocity. */
void Sakuyamon_special2(Sakuyamon *this, s32 frame) {
    Vec3 velocity;

    Fighter_special2(&this->base, frame);
    if (frame < 16 && !Fighter_isOnGround(&this->base)) {
        Fighter_stopActionClip(&this->base);
        return;
    }
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xD9);
        break;
    case 3:
        func_800A5F94(this);
        break;
    case 16:
        velocity.vx = Fighter_getFacing(&this->base) * 128;
        velocity.vy = -128;
        velocity.vz = 0;
        func_800A605C(this, &velocity);
        break;
    case 20:
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.velocity.vz = 0;
        this->base.velocityPad = 0;
        this->base.force.vy = -(((s64)this->base.mass * 0x3FFC) >> 16);
        Fighter_spawnDust(&this->base, 1, 0x60, 2);
        Fighter_spawnDust(&this->base, -1, 0x60, 2);
        break;
    }
    this->base.force.vx = -this->base.velocity.vx >> 2;
}

/* Special move 2 in the air: Fighter's; the voice at frame 0, the hitbox at frame 4 (func_800A5F94), thrown forward and
 * down by 128 at frame 16 (func_800A605C). The force brakes to a quarter of the horizontal velocity, and stops a fall
 * with a lift of 0x71C times the mass (16.16). */
void Sakuyamon_airSpecial2(Sakuyamon *this, s32 frame) {
    Vec3 velocity;

    Fighter_airSpecial2(&this->base, frame);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xD9);
        break;
    case 4:
        func_800A5F94(this);
        break;
    case 16:
        velocity.vx = Fighter_getFacing(&this->base) * 128;
        velocity.vy = 128;
        velocity.vz = 0;
        func_800A605C(this, &velocity);
        break;
    }
    this->base.force.vx = -this->base.velocity.vx >> 2;
    if (this->base.velocity.vy > 0) {
        /* The match depends on lift being a local of its own (computed inline, other registers) */
        s32 lift = ((s64)this->base.mass * 0x71C) >> 16;

        this->base.force.vy = -this->base.velocity.vy - lift;
    }
}

Unk800A7F88 *func_800A7C90(Unk800A7F88 *this, Fighter *fighter, VECTOR *point, Vec3 *position);

/* Creates func_800A7C90's entity (0xB8 bytes) at node 2 and at the character's position moved 384 the way it
 * faces, hands it to Fighter_addEntity and keeps its handle at 0x4B8, when it was allocated. */
void func_800A5F94(Sakuyamon *this) {
    VECTOR point;
    Vec3 position;
    Entity *entity;

    Fighter_getNodePosition(&this->base, &point, 2, &ZERO_VECTOR);
    Fighter_getPosition(&this->base, &position);
    position.vx += Fighter_getFacing(&this->base) * 384;
    entity = NEW_ENTITY(0xB8, func_800A7C90, &this->base, &point, &position);
    if (entity != NULL) {
        Fighter_addEntity(&this->base, entity);
        this->unk4B8 = entityGetHandle(entity);
    }
}

void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8);

/* Throws the entity of handle 0x4B8, when it still exists (func_800A80B4 with velocity), and plays sound
 * 0xD2. */
void func_800A605C(Sakuyamon *this, Vec3 *velocity) {
    Entity *entity = getEntityByHandle(this->unk4B8);

    if (entity != NULL) {
        func_800A80B4((Unk800A7F88 *)entity, (VECTOR *)velocity, 0x47AC1400);
        Fighter_playSound(&this->base, 0xD2);
    }
}

void Sakuyamon_spawnSpecial3Effect(Fighter *this);

/* Special move 3: Fighter's; at frame 0 the voice, 10 invulnerable frames, a push up and Fighter_addGauge(-0x8000);
 * from frame 26 the first landing spawns the effect, after which leaving the ground stops the clip. */
void Sakuyamon_special3(Sakuyamon *this, s32 frame) {
    Fighter_special3(&this->base, frame);
    if (frame == 0) {
        s64 push;

        Fighter_playVoice(&this->base, 0xDA);
        push = ((s64)this->base.mass * 0x5550) >> 16;
        this->unk4BC = 0;
        this->base.invulnerableFrames = 10;
        this->base.force.vy = -(s32)push;
        Fighter_addGauge(&this->base, -0x8000);
    }
    if (this->unk4BC == 0) {
        if (frame >= 26 && Fighter_isOnGround(&this->base)) {
            Sakuyamon_spawnSpecial3Effect(&this->base);
            this->unk4BC++;
        }
    } else if (!Fighter_isOnGround(&this->base)) {
        Fighter_stopActionClip(&this->base);
    }
}

/* the offset from node 19 where Sakuyamon_spawnSpecial3Effect spawns the effect */
extern VECTOR D_8006B198;

SakuyamonSpecial3Effect *SakuyamonSpecial3Effect_init(SakuyamonSpecial3Effect *this, Fighter *fighter,
                                                      VECTOR *position);

/* Spawns the special move 3's effect at node 19 (offset D_8006B198), then Fighter_addGauge(-0x10000) and its sound. */
void Sakuyamon_spawnSpecial3Effect(Fighter *this) {
    VECTOR position;
    Entity *child;

    Fighter_getNodePosition(this, &position, 19, &D_8006B198);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        SakuyamonSpecial3Effect_init(entityOperatorNewAt(0x4C, child), this, &position);
    }
    Fighter_addEntity(this, child);
    Fighter_addGauge(this, -0x10000);
    Fighter_playSound(this, 0xD3);
}

/* Constructs an entity that follows the character: its input, its position, and 96 the way it faces. */
Unk800A6220 *func_800A6220(Unk800A6220 *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B03C;
    this->fighter = fighter;
    this->input = Fighter_getInput(fighter);
    func_8007A650(fighter, &this->position);
    this->direction.vx = Fighter_getFacing(fighter) * 96;
    this->direction.vy = 0;
    this->direction.vz = 0;
    return this;
}

/* Deletes itself once the character's attack is over; else eases direction a quarter of the way to getDirection's
 * (x and y, steps under 2 dropped) and moves position by it. */
void func_800A62A0(Unk800A6220 *this) {
    Unk800A6220Vtable *vtable;
    VECTOR target;
    VECTOR direction;
    s32 x, y, z;
    s32 dx, dy;

    if (Fighter_isAttackIdle(this->fighter)) {
        entityDelete(&this->entity);
        return;
    }
    direction = this->direction;
    vtable = (Unk800A6220Vtable *)this->entity.vtable;
    vtable->getDirection.func((u8 *)this + vtable->getDirection.delta, &target);
    dx = (target.vx - direction.vx) >> 2;
    if (ABS(dx) < 2) {
        dx = 0;
    }
    dy = (target.vy - direction.vy) >> 2;
    if (ABS(dy) < 2) {
        dy = 0;
    }
    direction.vx += dx;
    direction.vy += dy;
    /* The match depends on the vecAdd shape: read the position whole, add, then store. */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += direction.vx;
    y += direction.vy;
    z += direction.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    /* stored after the position: earlier, direction.vz takes the first free register (allocation) */
    this->direction.vx = direction.vx;
    this->direction.vy = direction.vy;
    this->direction.vz = direction.vz;
}

/* Method 0x20 of the player's class (D_8006B03C): 96 the way the d-pad is held (the diagonals 96 times sin 45 on
 * each axis), or none. */
VECTOR *func_800A63D8(Unk800A6220 *this, VECTOR *out) {
    InputHistory *input = this->input;
    u32 held;
    s32 diagonal;

    InputHistory_setMirrored(input, 0);
    held = InputHistory_getHeld(input) & (PAD_UP | PAD_RIGHT | PAD_DOWN | PAD_LEFT);
    if (held != 0) {
        diagonal = COS_SIN_TABLE[0x200].sin * 96 >> 12;
        switch (held) {
        case PAD_UP:
            out->vx = 0;
            out->vy = -96;
            out->vz = 0;
            return out;
        case PAD_UP | PAD_RIGHT:
            out->vx = diagonal;
            out->vy = -diagonal;
            out->vz = 0;
            return out;
        case PAD_RIGHT:
            out->vx = 96;
            out->vy = 0;
            out->vz = 0;
            return out;
        case PAD_RIGHT | PAD_DOWN:
            out->vx = diagonal;
            out->vy = diagonal;
            out->vz = 0;
            return out;
        case PAD_DOWN:
            out->vx = 0;
            out->vy = 96;
            out->vz = 0;
            return out;
        case PAD_DOWN | PAD_LEFT:
            out->vx = -diagonal;
            out->vy = diagonal;
            out->vz = 0;
            return out;
        case PAD_LEFT:
            out->vx = -96;
            out->vy = 0;
            out->vz = 0;
            return out;
        case PAD_LEFT | PAD_UP:
            out->vx = -diagonal;
            out->vy = -diagonal;
            out->vz = 0;
            return out;
        }
    }
    out->vx = 0;
    out->vy = 0;
    out->vz = 0;
    out->pad = 0;
    return out;
}

/* Method 0x20 of the computer's class (D_8006B00C): the flat way, 96 long, from position to the opponent's top. */
VECTOR *func_800A6550(Unk800A6220 *this, VECTOR *out) {
    s32 x, y, z;

    /* func_8007A6CC writes only vx, vy and vz */
    func_8007A6CC(this->fighter, (Vec3 *)out);
    /* The match depends on the vecSubInPlace shape: read out whole, subtract, then store. */
    x = out->vx;
    y = out->vy;
    z = out->vz;
    x -= this->position.vx;
    y -= this->position.vy;
    z -= this->position.vz;
    out->vx = x;
    out->vy = y;
    out->vz = z;
    vecNormalize(out, out);
    out->vz = 0;
    out->vx = (out->vx * 96) >> 12;
    out->vy = (out->vy * 96) >> 12;
    return out;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A65F8);

s32 func_800A6AB0(Unk800A65F8 *this, Entity *owner);
s32 func_800A6BE4(Unk800A65F8 *this);

/* listRemove(&parts->part.link), in the access order the game's compiler used (the match depends on it) */
static inline void removePart(UnkPartsEntity38 *parts) {
    ListNode *node = &parts->part.link;

    parts->part.link.next->prev = parts->part.link.prev;
    node->prev->next = parts->part.link.next;
    parts->part.link.next = node;
    parts->part.link.prev = node;
}

/* Unk800A65F8's tick: turns about the Unk800A6220 of its handle (func_800A6AB0) while it lives; else, out of the
 * hit list, it goes back toward its fighter (func_800A6BE4) and is deleted once unkCC runs out or unkC8 falls under
 * 0x11. Then unkC0 steps toward the side it moved (within -4..4), the sphere takes the position and twice unkC8, and
 * the frame count goes on. The match depends on the clamp as one conditional expression. */
void func_800A6778(Unk800A65F8 *this) {
    Entity *owner;
    s32 dx;
    s32 tilt;
    s32 size;
    s32 x, y, z;

    owner = getEntityByHandle(this->handle);
    if (owner != NULL) {
        dx = func_800A6AB0(this, owner);
    } else {
        removePart(&this->base);
        dx = func_800A6BE4(this);
        this->unkCC -= 4;
        if (this->unkCC <= 0 || this->unkC8 < 0x11) {
            entityDelete(&this->base.entity);
            return;
        }
    }
    tilt = this->unkC0;
    if (dx != 0) {
        if (dx < 0) {
            tilt--;
        } else {
            tilt++;
        }
    }
    tilt = tilt < -4 ? -4 : tilt > 4 ? 4 : tilt;
    size = abs(tilt) >> 1;
    if (size >= 2) {
        size = this->frames % 6 + 2;
    }
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    this->base.part.position.vx = x;
    this->base.part.position.vy = y;
    this->base.part.position.vz = z;
    this->unkC0 = tilt;
    this->unkC4 = size;
    this->base.part.radius = this->unkC8 << 1;
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A68FC);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6AB0);

/* Takes 2 from value, 4 while it is under limit, and caps it at limit. The match depends on this being an expanded
 * MIN(*value -= 2, limit), side effect evaluated twice: value is stored twice, the last time through the pointer. */
static inline void shrinkTo(s32 *value, s32 limit) {
    *value = (*value -= 2) < limit ? (*value -= 2) : limit;
}

/* Moves the sphere an eighth of the way back to its fighter's func_8007A650 point and returns that step's x; unkC8
 * drops by 2, by 4 while under the distance left, and never stays over it. */
s32 func_800A6BE4(Unk800A65F8 *this) {
    VECTOR position = this->position;
    VECTOR delta;
    Vec3 target;
    s32 x, y, z;

    func_8007A650(this->fighter, &target);
    /* The match depends on the vecSub shape: read both vectors whole, subtract, then store. */
    x = target.vx;
    y = target.vy;
    z = target.vz;
    x -= position.vx;
    y -= position.vy;
    z -= position.vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    shrinkTo(&this->unkC8, vecGetLength(&delta));
    delta.vx = (target.vx - position.vx) >> 3;
    delta.vy = (target.vy - position.vy) >> 3;
    delta.vz = (target.vz - position.vz) >> 3;
    position.vx += delta.vx;
    position.vy += delta.vy;
    position.vz += delta.vz;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    return delta.vx;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6CF4);

/* Builds an Unk800A6EB8 on the Unk800A65F8 of handle: its six points at that entity's position (at the origin
 * without one). The match depends on reading the position's three words before storing any (as vecCopy does) and on
 * the loop walking a point pointer next to its index. */
Unk800A6EB8 *func_800A6EB8(Unk800A6EB8 *this, s32 handle) {
    Unk800A65F8 *parts;
    VECTOR position;
    Unk800A6EB8Point *point;
    s32 x, y, z;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AF9C;
    parts = (Unk800A65F8 *)getEntityByHandle(handle);
    this->handle = handle;
    this->unk1C = 0x80;
    this->vramY = 0;
    if (parts != NULL) {
        x = parts->position.vx;
        y = parts->position.vy;
        z = parts->position.vz;
        position.vx = x;
        position.vy = y;
        position.vz = z;
        this->vramY = parts->vramY;
    } else {
        position.vx = 0;
        position.vy = 0;
        position.vz = 0;
        position.pad = 0;
    }
    for (i = 0, point = this->points; i < 6; i++) {
        point->position = *(Vec3 *)&position; /* the three components, as a Vec3 */
        point->unk10 = i & 7;
        point->unk14 = 0;
        point++;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6F8C);

/* The draw of the entities of D_8006AF9C: func_800A7290, then func_800A7458. */
void func_800A7254(Entity *this, s32 arg) {
    func_800A7290(this, arg);
    func_800A7458(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7290);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7458);

/* Builds an Unk800A7694 on side (0 or 1) of the Unk800A6EB8 of handle, with identity matrices. */
Unk800A7694 *func_800A7694(Unk800A7694 *this, s32 handle, s32 side) {
    Unk800A6EB8 *parent;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AF74;
    parent = (Unk800A6EB8 *)getEntityByHandle(handle);
    this->handle = handle;
    this->unk1C.words[0] = 0x1000;
    this->unk1C.words[1] = 0;
    this->unk1C.words[2] = 0x1000;
    this->unk1C.words[3] = 0;
    this->unk1C.words[4] = 0x1000;
    this->unk1C.words[5] = 0;
    this->unk1C.words[6] = 0;
    this->unk1C.words[7] = 0;
    this->unk3C.words[0] = 0x1000;
    this->unk3C.words[1] = 0;
    this->unk3C.words[2] = 0x1000;
    this->unk3C.words[3] = 0;
    this->unk3C.words[4] = 0x1000;
    this->unk3C.words[5] = 0;
    this->unk3C.words[6] = 0;
    this->unk3C.words[7] = 0;
    this->side = side & 1;
    this->unk60 = 0x80;
    this->unk64 = 0;
    this->unk68 = 0;
    this->unk6C = 0;
    this->frames = 0;
    if (parent != NULL) {
        this->vramY = parent->vramY;
    } else {
        this->vramY = 0;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7768);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A79D0);

/* Builds the entity func_800A5F94 spawns: a hit part (damage word 0x800001, radius 0) at position on the fighter's
 * side, starting from point. */
Unk800A7F88 *func_800A7C90(Unk800A7F88 *this, Fighter *fighter, VECTOR *point, Vec3 *position) {
    s32 sx, sy, sz;
    s32 x, y, z;
    Fight *list;

    entityInit(&this->entity, ENTITY_KIND_FIGHTER_OBJECT);
    /* ZERO_VECTOR is a VECTOR; only its first three words are read here */
    Hit_init(&this->unk18, fighter, 0x800001, (Vec3 *)&ZERO_VECTOR, 0);
    this->unk18.vtable = D_8006AF2C;
    this->entity.vtable = &D_8006AF4C;
    CollisionSphere_init(&this->part, position, 0, &this->unk18);
    this->fighterHandle = entityGetHandle(&fighter->entity);
    sx = point->vx;
    sy = point->vy;
    sz = point->vz;
    this->start.vx = sx;
    this->start.vy = sy;
    this->start.vz = sz;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unkA4 = 0;
    this->unkA8 = 0;
    this->unkAC = 0;
    this->unkB0 = 0;
    this->unk94.vx = x;
    this->unk94.vy = y;
    this->unk94.vz = z;
    this->vramY = Fighter_getVramY(fighter);
    list = Fighter_getScene(fighter);
    if (Fighter_getSide(fighter) == 0) {
        Fight_addSide0Shape(list, &this->part.link);
    } else {
        Fight_addSide1Shape(list, &this->part.link);
    }
    this->part.entity = &this->entity;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7DD4);

/* The second base's Hit_apply with arg, then disables the part when getHitFighter finds an entity in arg and 0xAC is
 * 0. */
void func_800A7F88(Unk800A7F88 *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL && this->unkAC == 0) {
        this->part.flags.bits.disabled = 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7FEC);

/* Keeps vec at 0x94, clears 0xA8 and 0xB0, counts the call in 0xAC, and sets the second base's vector to
 * vec's unit direction with x doubled, y quadrupled and z cleared, and its word at 0x8 to unk8. */
void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8) {
    VECTOR dir;
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unkA8 = 0;
    this->unkB0 = 0;
    this->unk94.vx = x;
    this->unk94.vy = y;
    this->unk94.vz = z;
    this->unkAC++;
    vecNormalize(&dir, vec);
    dir.vx <<= 1;
    dir.vy <<= 2;
    dir.vz = 0;
    Hit_setKnockback(&this->unk18, &dir);
    Hit_set(&this->unk18, unk8, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8150);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AC5C);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8274);

extern EntityVtable D_8006AF04;

/* The entity class of vtable D_8006AF04 (0x60 bytes) that func_800A8150 spawns: it moves by its velocity, steered
 * toward its steer vector for steerFrames ticks (func_800A84E4), growing and fading. */
typedef struct Unk800A83E0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 positionPad;
    /* 0x28 */ Vec3 velocity;
    /* 0x34 */ s32 velocityPad;
    /* 0x38 */ Vec3 steer;       /* target less velocity at first; the velocity takes a tenth of the gap a tick */
    /* 0x44 */ s32 steerPad;
    /* 0x48 */ s32 size;         /* 0x60 at first, + 4 a tick */
    /* 0x4C */ s32 animFrame;    /* 0 to 7 */
    /* 0x50 */ s32 angle;        /* the velocity's (ratan2) */
    /* 0x54 */ s32 brightness;   /* 0xFF at first, - 12 a tick; the entity deletes itself at 0 */
    /* 0x58 */ s32 steerFrames;  /* at least 1 */
    /* 0x5C */ s32 unk5C;  /* the spawner's vramY */
} Unk800A83E0;

/* Builds the entity at position with velocity, steering toward target for steerFrames ticks (at least 1). The match
 * depends on each vector read whole into its own block's locals before it is stored (as vecCopy and vecSub do) and
 * on the clamp written as a conditional expression. */
Unk800A83E0 *func_800A83E0(Unk800A83E0 *this, Vec3 *position, Vec3 *velocity, Vec3 *target, s32 steerFrames,
                           s32 arg5) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AF04;
    {
        s32 x = position->vx;
        s32 y = position->vy;
        s32 z = position->vz;

        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;

        this->velocity.vx = x;
        this->velocity.vy = y;
        this->velocity.vz = z;
    }
    {
        s32 x = target->vx;
        s32 y = target->vy;
        s32 z = target->vz;

        x -= velocity->vx;
        y -= velocity->vy;
        z -= velocity->vz;
        this->steer.vx = x;
        this->steer.vy = y;
        this->steer.vz = z;
    }
    this->size = 0x60;
    this->animFrame = 0;
    this->angle = ratan2(velocity->vy, velocity->vx);
    this->brightness = 0xFF;
    this->steerFrames = steerFrames > 0 ? steerFrames : 1;
    this->unk5C = arg5;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A84E4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A86A8);

/* Builds the special move 3's effect for fighter, its fields 0; its one caller (Sakuyamon_spawnSpecial3Effect) also
 * passes a position, which it does not read. */
SakuyamonSpecial3Effect *SakuyamonSpecial3Effect_init(SakuyamonSpecial3Effect *this, Fighter *fighter,
                                                      VECTOR *position) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &SAKUYAMON_SPECIAL3_EFFECT_VTABLE;
    this->fighter = fighter;
    this->position.vx = 0;
    this->position.vy = 0;
    this->position.vz = 0;
    this->position.pad = 0;
    this->unk2C = 0;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->state = 0;
    this->frames = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", SakuyamonSpecial3Effect_tick);

/* The draw of the entities of SAKUYAMON_SPECIAL3_EFFECT_VTABLE: func_800A8C24, then func_800A8F80. */
void SakuyamonSpecial3Effect_draw(Entity *this, s32 arg) {
    func_800A8C24(this, arg);
    func_800A8F80(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AC7C);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8C24);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACC4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACCC);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8F80);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACD4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A926C);

/* The destructor of func_800A926C's class: stops its sound, takes its member at 0x38 out of its list. */
void func_800A9428(Unk800A9944 *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006AE8C;
    this->entity.vtable = &D_8006AEAC;
    stopSound(this->unkD4);
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

/* Plays sound 0xD4 and keeps its handle at 0xD4 (func_800A926C's class; the destructor stops it). */
void func_800A94D0(Unk800A9944 *this) {
    this->unkD4 = soundControlPlaySound(&SOUND_CONTROL, 0xD4, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9508);

/* The second base's Hit_apply with arg, then halves 0xB4 when getHitFighter finds an entity in arg. */
void func_800A9944(Unk800A9944 *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL) {
        this->unkB4 >>= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9998);

/* The class func_800A9C4C constructs (virtual table D_8006AE64). */
typedef struct Unk800A9C4C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* three RANDOM values */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38; /* eases towards 0x804 (func_800A9D0C) */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 vramY; /* Fighter_getVramY of the owner */
} Unk800A9C4C;

extern EntityVtable D_8006AE64;

/* Constructs the entity of virtual table D_8006AE64 for fighter at position, with three random words. */
Unk800A9C4C *func_800A9C4C(Unk800A9C4C *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;
    s32 random0, random1, random2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AE64;
    /* The match depends on reading position whole (a struct copy: 26 diffs) and on drawing the three random
     * words before storing any. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    random0 = mersenneTwisterGenerate(&RANDOM);
    random1 = mersenneTwisterGenerate(&RANDOM);
    random2 = mersenneTwisterGenerate(&RANDOM);
    this->unk28 = random0;
    this->unk2C = random1;
    this->unk30 = random2;
    this->unk38 = 0;
    this->unk3C = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The tick of func_800A9C4C's entity: 0x38 eases a quarter of the way to 0x804 every frame, and the
 * entity deletes itself once it reaches 0x800. */
void func_800A9D0C(Unk800A9C4C *this) {
    this->unk38 += (0x804 - this->unk38) >> 2;
    if (this->unk38 >= 0x800) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE40);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9D70);

/* Sakuyamon's destructor. */
void Sakuyamon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006B03C, which func_800A6220 installs. */
void func_800AA004(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Copies the vector at 0x20 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800AA04C(Unk800AA04C *this, Vec3 *out) {
    s32 x = this->unk20.vx;
    s32 y = this->unk20.vy;
    s32 z = this->unk20.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* The destructor of the entity class of vtable D_8006B00C, which func_800A5AD0 and func_800AA0B4 install. */
void func_800AA06C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006B00C, which derives from func_800A6220's. */
Unk800A6220 *func_800AA0B4(Unk800A6220 *this, Fighter *fighter) {
    return constructUnk800AA0B4(this, fighter);
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AFE4 and D_8006AFC4, which
 * func_800A65F8 installs. */
void func_800AA0E8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AFC4;
    this->entity.vtable = &D_8006AFE4;
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

/* The destructor of the entity class of vtable D_8006AF9C, which func_800A6EB8 installs. */
void func_800AA188(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF74, which func_800A7694 installs. */
void func_800AA1D0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AF4C and D_8006AF2C, which
 * func_800A7C90 installs. */
void func_800AA218(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AF2C;
    this->entity.vtable = &D_8006AF4C;
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

/* The destructor of the entity class of vtable D_8006AF04, which func_800A83E0 installs. */
void func_800AA2B8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 3 effect's destructor. */
void SakuyamonSpecial3Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AE64, which func_800A9C4C installs. */
void func_800AA348(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE64);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE8C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AEAC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", SAKUYAMON_SPECIAL3_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF04);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF2C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF4C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF74);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF9C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AFC4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AFE4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B00C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B03C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", SAKUYAMON_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B198);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B1AC);
