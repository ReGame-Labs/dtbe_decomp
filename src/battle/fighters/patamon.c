/* Patamon (CHARACTER_PATAMON): the fighter createFighter builds for it, its moves (a throw that moves the opponent
 * along a line), and the entities they spawn: the special move 1's hitbox (and its children) and the special move
 * 2's effect. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/math/sin_cos.h"

extern FaceEntry PATAMON_FACES[];
extern EntityVtable D_8006A784;
extern EntityVtable PATAMON_SPECIAL1_EFFECT_VTABLE;
extern EntityVtable PATAMON_VTABLE;
struct PatamonSpecial1Hitbox *PatamonSpecial1Hitbox_init(struct PatamonSpecial1Hitbox *this, Fighter *fighter,
                                                          Vec3 *position, VECTOR *velocity, Fight *fight);

/* Builds Patamon: a Fighter with its own vtable and faces. */
Patamon *Patamon_init(Patamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &PATAMON_VTABLE;
    Fighter_setFaceTable(&this->base, PATAMON_FACES);
    this->special1Active = 0;
    return this;
}

/* Patamon's destructor. */
void Patamon_destroy(Entity *this, s32 flags) {
    this->vtable = &PATAMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: nothing to do. */
void Patamon_endMove(void) {
}

void func_800A11B0(Patamon *this);

/* getSin (engine/math/sin_cos.c), inline. */
static inline s16 getSinInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].sin;
}

/* The intro: for 48 frames 0x3C8 eases along a sine from 0x1400 behind and 0xA00 up to (0, 0) (func_800A11B0), then
 * is cleared. */
void Patamon_intro(Patamon *this, s32 frame) {
    if (frame == 0) {
        this->unk4B8 = 0;
        this->unk4CC.vx = 0;
        this->unk4CC.vy = 0;
        this->unk4CC.vz = 0;
        this->unk4CC.pad = 0;
        this->unk4DC.vx = 0;
        this->unk4DC.vy = 0;
        this->unk4DC.vz = 0;
        this->unk4DC.pad = 0;
        this->unk4CC.vx = -(Fighter_getFacing(&this->base) * 0x1400);
        this->unk4CC.vy = -0xA00;
    }
    if (frame < 48) {
        /* The match depends on getSin's inline (the table read in place computes its address first). */
        this->unk4B8 = getSinInline(frame * 21);
        func_800A11B0(this);
    } else {
        this->base.unk3C8.vx = 0;
        this->base.unk3C8.vy = 0;
        this->base.unk3C8.vz = 0;
        this->base.unk3C8.pad = 0;
    }
}

/* Sets out's x and y to the point at t (4.12) of the way from from to to. */
void func_800A1160(Patamon *this, VECTOR *from, VECTOR *to, s32 t, VECTOR *out) {
    out->vx = from->vx + (((to->vx - from->vx) * t) >> 12);
    out->vy = from->vy + (((to->vy - from->vy) * t) >> 12);
}

/* Clamps 0x4B8 to 0..0x1000 and puts the character's 0x3C8 that far from 0x4CC to 0x4DC (func_800A1160). */
void func_800A11B0(Patamon *this) {
    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &this->base.unk3C8);
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_throw);

/* Clamps 0x4B8 to 0..0x1000 and sets fighter moving (0x3B4, in 1/256ths) from its position to the point
 * that far from 0x4CC to 0x4DC (func_800A1160), clearing 0x3A4..0x3B0 and 0x3BC. */
void func_800A1760(Patamon *this, Fighter *fighter) {
    Vec3 position;
    VECTOR target;

    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    Fighter_getPosition(fighter, &position);
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &target);
    fighter->force.vz = 0;
    fighter->velocity.vx = 0;
    fighter->velocity.vy = 0;
    fighter->velocity.vz = 0;
    fighter->velocityPad = 0;
    fighter->force.vx = (target.vx - position.vx) << 8;
    fighter->force.vy = (target.vy - position.vy) << 8;
}

void Patamon_spawnSpecial1Hitbox(Fighter *this);

/* Special move 1: at frame 0 stops when the hitbox is still out, else the voice and the clip; the hitbox at frame
 * 20. */
void Patamon_special1(Patamon *this, s32 frame) {
    if (frame == 0) {
        if (this->special1Active != 0) {
            Fighter_stopActionClip(&this->base);
            return;
        }
        Fighter_playVoice(&this->base, 0xC9);
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
    }
    if (frame == 20) {
        Patamon_spawnSpecial1Hitbox(&this->base);
    }
}

/* Special move 1 in the air: at frame 0 stops when the hitbox is still out (0x4A4), else the voice and the clip; the
 * hitbox at frame 3; before frame 17 the force brakes a sixteenth of the horizontal velocity and any fall. */
void Patamon_airSpecial1(Patamon *this, s32 frame) {
    if (frame == 0) {
        if (this->special1Active != 0) {
            Fighter_stopActionClip(&this->base);
            return;
        }
        Fighter_playVoice(&this->base, 0xC9);
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_AIR_SPECIAL1, 2, 0, -1);
    }
    if (frame == 3) {
        Patamon_spawnSpecial1Hitbox(&this->base);
    }
    if (frame < 17) {
        this->base.force.vx -= this->base.velocity.vx >> 4;
        if (this->base.velocity.vy > 0) {
            this->base.force.vy -= this->base.velocity.vy;
        }
    }
}

/* Spawns the special move 1's hitbox 64 in front of the fighter and 0x60 up, 96 a frame forward, then its sound and
 * func_80079B0C(40). */
void Patamon_spawnSpecial1Hitbox(Fighter *this) {
    s32 facing = Fighter_getFacing(this);
    Fight *unk228;
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    velocity.vx = facing * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 64;
    position.vy -= 0x60;
    unk228 = Fighter_getScene(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        PatamonSpecial1Hitbox_init(entityOperatorNewAt(0xC8, child), this, &position, &velocity, unk228);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xC6);
    func_80079B0C(this, 40);
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_special2);

/* an axis next to AXIS_Y and AXIS_Z in the executable's data */
extern VECTOR D_80010874;

/* Whether the character Fighter_getOpponent gives is in reach: when Fighter_isThrowable holds for it, what
 * func_80079E50 finds within both func_80079DD8 sizes plus 0xA0 (squared) around AXIS_Z, else around
 * D_80010874; NULL otherwise. */
Fighter *func_800A1CDC(Fighter *this) {
    Fighter *opponent = Fighter_getOpponent(this);
    s32 reach;
    s32 range;

    if (!Fighter_isThrowable(opponent)) {
        return NULL;
    }
    reach = func_80079DD8(this) + func_80079DD8(opponent) + 0xA0;
    range = reach * reach;
    /* The match depends on reusing opponent for the character found (a local of its own stays in $v0) */
    opponent = func_80079E50(this, &AXIS_Z, range, 0x200);
    if (opponent != NULL) {
        return opponent;
    }
    return func_80079E50(this, &D_80010874, range, 0x200);
}

/* A value by the opponent's character: 0x3FFC for Patamon, 0x4718 for Renamon, Seraphimon and Wormmon, 0x4E34 for any
 * other. */
s32 func_800A1D84(Fighter *this) {
    s32 value;

    switch (Fighter_getCharacter(Fighter_getOpponent(this))) {
    case CHARACTER_PATAMON:
        value = 0x3FFC;
        break;
    case CHARACTER_RENAMON:
    case CHARACTER_SERAPHIMON:
    case CHARACTER_WORMMON:
        value = 0x4718;
        break;
    default:
        value = 0x4E34;
        break;
    }
    return value;
}

/* Creates an entity of the class of vtable D_8006A784 (0x120 bytes) with g++'s new, and hands it
 * to Fighter_addEntity. Its constructor func_800A2F64 is inlined. */
void func_800A1DFC(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006A784;
    }
    Fighter_addEntity(this, child);
}

/* The special move 1's hitbox (0xC8 bytes): while it lives its fighter's special1Active is set. */
typedef struct PatamonSpecial1Hitbox {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Patamon *fighter;
    /* 0x88 */ VECTOR position;
    /* 0x98 */ Vec3 velocity;
    /* 0xA4 */ s32 velocityPad;
    /* 0xA8 */ s32 ticks;
    /* 0xAC */ s32 unkAC;  /* bit 0 once it stops hitting (tick 36, unkC2 -1 or a touch); bit 1 after a touch */
    /* 0xB0 */ s32 unkB0; /* 0x100 at first */
    /* 0xB4 */ s32 unkB4; /* 0xC0 at first */
    /* 0xB8 */ s32 radius; /* the sphere's: grows by 0x33 to 0xC0 while it hits, then shrinks to its end */
    /* 0xBC */ s32 unkBC;
    /* 0xC0 */ s16 unkC0;
    /* 0xC2 */ s16 unkC2;
    /* 0xC4 */ u16 vramY; /* Fighter_getVramY of the fighter */
} PatamonSpecial1Hitbox;

extern EntityVtable PATAMON_SPECIAL1_HITBOX_VTABLE;

/* Builds the special move 1 hitbox of fighter at position, moving by velocity: a sphere in its side's list of fight,
 * hitting 0x7A81AA0 with a knockback of 64 times velocity's x and as much up; it sets the fighter's
 * special1Active. */
PatamonSpecial1Hitbox *PatamonSpecial1Hitbox_init(PatamonSpecial1Hitbox *this, Fighter *fighter, Vec3 *position,
                                                  VECTOR *velocity, Fight *fight) {
    VECTOR knockback;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    this->parts.entity.vtable = &PATAMON_SPECIAL1_HITBOX_VTABLE;
    TrackedHit_init(&this->parts.unk18, fighter, 0x7A81AA0, (Vec3 *)velocity, 0);
    CollisionSphere_init(&this->parts.part, position, 0, &this->parts.unk18);
    knockback = *velocity;
    knockback.vx <<= 6;
    knockback.vy = -abs(knockback.vx);
    knockback.vz = 0;
    Hit_setKnockback(&this->parts.unk18.base, &knockback);
    this->parts.part.entity = &this->parts.entity;
    this->fighter = (Patamon *)fighter;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->ticks = 0;
    this->unkAC = 0;
    this->unkB0 = 0x100;
    this->unkB4 = 0xC0;
    this->radius = 0;
    this->unkC0 = 0;
    this->unkC2 = 0;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->vramY = Fighter_getVramY(fighter);
    if (Fighter_getSide(&this->fighter->base) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    this->fighter->special1Active = 1;
    return this;
}

/* The special move 1 hitbox's destructor: clears the fighter's special1Active and calls its func_80079B0C(1). */
void PatamonSpecial1Hitbox_destroy(PatamonSpecial1Hitbox *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &PATAMON_SPECIAL1_HITBOX_VTABLE;
    this->fighter->special1Active = 0;
    func_80079B0C(&this->fighter->base, 1);
    this->parts.part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->parts.part.link;
    this->parts.part.link.next->prev = this->parts.part.link.prev;
    node->prev->next = this->parts.part.link.next;
    this->parts.part.link.next = node;
    this->parts.part.link.prev = node;
    this->parts.unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

struct PatamonSpecial1Effect *PatamonSpecial1Effect_init(struct PatamonSpecial1Effect *this, Vec3 *position);

/* Moves by the velocity (braking x for its first 3 ticks). While it hits, its sphere follows it and grows; a touch, tick
 * 36 or unkC2 -1 disables the sphere. Then it shrinks with an effect a tick and deletes itself once its radius is
 * gone. */
void PatamonSpecial1Hitbox_tick(PatamonSpecial1Hitbox *this) {
    s32 x, y, z;
    Entity *child;

    /* The match depends on vecAddInPlace's shape: the position read whole, then added to, then stored. */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (this->ticks < 3) {
        this->velocity.vx -= this->velocity.vx >> 3;
    }
    if (this->ticks >= 36 || this->unkC2 == -1) {
        this->unkAC |= 1;
        this->parts.part.flags.bits.disabled = 1;
    }
    if (this->unkAC == 0) {
        this->parts.part.radius = this->radius;
        this->radius += 0x33;
        /* The match depends on this copy reading its three words before any store, in its own scope. */
        {
            s32 px = this->position.vx;
            s32 py = this->position.vy;
            s32 pz = this->position.vz;

            this->parts.part.position.vx = px;
            this->parts.part.position.vy = py;
            this->parts.part.position.vz = pz;
        }
        if (this->radius > 0xC0) {
            this->radius = 0xC0;
        }
        if (this->parts.part.flags.bits.touched) {
            this->parts.part.flags.bits.disabled = 1;
            this->unkAC |= 3;
        }
    } else {
        this->radius -= 0x33;
        if (this->radius <= 0) {
            entityDelete(&this->parts.entity);
            return;
        }
        child = allocEntityMemory(0x4C);
        if (child != NULL) {
            PatamonSpecial1Effect_init(entityOperatorNewAt(0x4C, child), (Vec3 *)&this->position);
        }
        entityAddSibling(&this->parts.entity, child);
    }
    this->unkC0 = 1;
    this->ticks++;
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", PatamonSpecial1Hitbox_draw);

/* The special move 1's effect (0x4C bytes), which the hitbox leaves every frame while it shrinks: a ring of 16
 * quads around a point that widens for 8 frames. */
typedef struct PatamonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR position;
    /* 0x28 */ s32 radius[2]; /* the ring's two edges: 0x80 and 0x4C, 46 and 51 more a frame */
    /* 0x30 */ s32 scale[2];  /* of the cosine and sine parts, 0x100 = 1 */
    /* 0x38 */ u32 color;
    /* 0x3C */ s32 angle;     /* of the first quad */
    /* 0x40 */ s32 angleStep; /* between the quads */
    /* 0x44 */ s16 quads;
    /* 0x46 */ s16 ticked;    /* set by the tick, cleared by the draw, which draws only when it is set */
    /* 0x48 */ s32 frames;
} PatamonSpecial1Effect;

/* Builds the special move 1's effect at the position: a grey ring of 16 quads. */
PatamonSpecial1Effect *PatamonSpecial1Effect_init(PatamonSpecial1Effect *this, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &PATAMON_SPECIAL1_EFFECT_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->radius[0] = 0x80;
    this->radius[1] = 0x4C;
    this->scale[0] = 0x100;
    this->scale[1] = 0x100;
    this->color = 0x808080;
    this->angle = 0;
    this->angleStep = 0x100;
    this->quads = 16;
    this->frames = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* The special move 1 effect's destructor. */
void PatamonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &PATAMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Widens the ring for 8 frames, then deletes it. */
void PatamonSpecial1Effect_tick(PatamonSpecial1Effect *this) {
    if (this->frames >= 8) {
        entityDelete(&this->entity);
    } else {
        this->ticked = 1;
        this->radius[0] += 46;
        this->radius[1] += 51;
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", PatamonSpecial1Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/patamon", func_800A2C70);

/* The destructor of the entity class of vtable D_8006A784, which func_800A1DFC and func_800A2F64 install. */
void func_800A2F1C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006A784, which derives from func_800BD490's. */
Entity *func_800A2F64(Entity *this, Fighter *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006A784;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A714);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A71C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A724);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A72C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A734);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A73C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A744);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A754);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A774);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A784);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_VTABLE);
