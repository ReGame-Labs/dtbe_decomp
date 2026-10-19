/* Guilmon (CHARACTER_GUILMON): the fighter createFighter builds for it, its moves and the hitboxes of its special
 * moves 1 and 2. */

#include "common.h"
#include "battle/game_classes.h"

extern FaceEntry GUILMON_FACES[];
extern VtableEntry GUILMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable GUILMON_SPECIAL2_HITBOX_VTABLE;
extern VtableEntry GUILMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable GUILMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable GUILMON_VTABLE;

/* Builds Guilmon: a Fighter with its own vtable and faces. */
Fighter *Guilmon_init(Fighter *this) {
    Fighter_init(this);
    this->entity.vtable = &GUILMON_VTABLE;
    Fighter_setFaceTable(this, GUILMON_FACES);
    return this;
}

/* Guilmon's destructor. */
void Guilmon_destroy(Entity *this, s32 flags) {
    this->vtable = &GUILMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The attack 2, pushing forward on frames 1 to 4. */
void Guilmon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame >= 1 && frame <= 4) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 9100);
    }
}

/* The attack 3, pushing forward on frames 16 to 19. */
void Guilmon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame >= 16 && frame <= 19) {
        Fighter_accelerateTo(this, Fighter_getFacing(this) * 18200);
    }
}

/* Throw: Fighter's; at frame 0 the opponent is held (10) flinching and both fighters stop; at frame 10 it is attached
 * to node 15 with its model moved to (0x20, 0x48, 0) if the fighter is on the ground, else let go; at frame 32 it is
 * hit, let go and pushed forward. */
void Guilmon_throw(Fighter *this, s32 frame) {
    Fighter *opponent;
    Model *model;
    Vec3 push;
    s16 side;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_hold(opponent, 10);
        Fighter_setClip(opponent, FIGHTER_CLIP_FLINCH, 0, 0);
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->velocity.vz = 0;
        this->velocityPad = 0;
        break;
    case 10:
        if (Fighter_isOnGround(this)) {
            Fighter_attachTo(opponent, this, 15);
            model = opponent->model;
            model->group.object.transform.valid = 0;
            model->group.object.transform.local.m.t[0] = 0x20;
            model->group.object.transform.local.m.t[1] = 0x48;
            model->group.object.transform.local.m.t[2] = 0;
        } else {
            Fighter_detach(opponent);
            Fighter_stopActionClip(this);
        }
        break;
    case 32: {
        s32 x, y, z;

        side = Fighter_getFacing(this);
        push.vx = side * 27300;
        push.vy = 0;
        push.vz = 0;
        /* ZERO_VECTOR is a VECTOR; only its first three words are read here */
        Fighter_takeHit(opponent, 0x598C2000, (Vec3 *)&ZERO_VECTOR, 2);
        Fighter_detach(opponent);
        /* The match depends on the vector read into x, y and z before it is stored. */
        x = push.vx;
        y = push.vy;
        z = push.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
        break;
    }
    }
}

void Guilmon_spawnSpecial1Hitbox(Fighter *this, s32 air);

/* Special move 1: its clip, the voice and func_80079B0C(27) at frame 0; the hitbox at frame 14. */
void Guilmon_special1(Fighter *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL1, 0, 0, -1);
        func_80079B0C(this, 27);
        Fighter_playVoice(this, 0x8B);
        break;
    case 14:
        Guilmon_spawnSpecial1Hitbox(this, 0);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", Guilmon_special2);

/* Special move 1 in the air: at frame 0 its clip, a stop, func_80079B0C(27) and the voice; at frame 5 a push back
 * and the hitbox; from frame 6 a rise is braked and the horizontal velocity loses 1/128 a frame. */
void Guilmon_airSpecial1(Fighter *this, s32 frame) {
    func_80079AE4(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL1, 0, 0, -1);
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        func_80079B0C(this, 27);
        Fighter_playVoice(this, 0x8B);
        break;
    case 5: {
        s32 push = ((s64)this->mass * 0x1C70) >> 16;

        this->force.vx += push * -Fighter_getFacing(this);
        Guilmon_spawnSpecial1Hitbox(this, 1);
        break;
    }
    }
    if (frame >= 6) {
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
        this->force.vx -= this->velocity.vx >> 7;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", Guilmon_airSpecial2);

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", Guilmon_spawnSpecial1Hitbox);

UnkPartsEntity38 *GuilmonSpecial2Hitbox_init(UnkPartsEntity38 *this, Fighter *fighter, Vec3 *position, VECTOR *velocity,
                                             Fight *list);
extern VECTOR GUILMON_SPECIAL2_HITBOX_OFFSET;

/* Spawns the special move 2's hitbox at the fighter's point 16 (moved by GUILMON_SPECIAL2_HITBOX_OFFSET), flying
 * forward at 80 per frame. Its velocity's z is air (1 from airSpecial2). The top of the fighter, 64 forward, is
 * computed first and then overwritten by the point. */
void Guilmon_spawnSpecial2Hitbox(Fighter *this, s32 air) {
    VECTOR offset = GUILMON_SPECIAL2_HITBOX_OFFSET;
    Vec3 position;
    VECTOR velocity;
    Fight *list;
    Entity *child;

    list = Fighter_getScene(this);
    func_8007A650(this, &position);
    position.vx += Fighter_getFacing(this) * 64;
    Fighter_getNodePosition(this, (VECTOR *)&position, 16, &offset);
    velocity.vx = Fighter_getFacing(this) * 80;
    velocity.vy = 0;
    velocity.vz = air;
    child = allocEntityMemory(0xF4);
    if (child != NULL) {
        GuilmonSpecial2Hitbox_init(entityOperatorNewAt(0xF4, child), this, &position, &velocity, list);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial1Hitbox_init);

/* The special move 1 hitbox's destructor. */
void GuilmonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = GUILMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &GUILMON_SPECIAL1_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial1Hitbox_tick);

/* The special move 1's hitbox (an UnkPartsEntity38), as far as GuilmonSpecial1Hitbox_hit needs it. */
typedef struct GuilmonSpecial1Hitbox {
    /* 0x000 */ UnkPartsEntity38 base;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ u8 unk84[0x9C - 0x84];
    /* 0x09C */ Vec3 unk9C; /* the first of a run of positions */
    /* 0x0A8 */ u8 unkA8[0x100 - 0xA8];
    /* 0x100 */ Vec3 unk100; /* a constructor argument */
    /* 0x10C */ u8 unk10C[0x110 - 0x10C];
    /* 0x110 */ SVECTOR corners[4];
    /* 0x130 */ s32 unk130; /* the length of unk100 */
} GuilmonSpecial1Hitbox;

/* On a hit: Hit_apply, a hit spark on the fighter it struck, a burst of 6 particles at 0x9C, and it deletes itself. */
void GuilmonSpecial1Hitbox_hit(GuilmonSpecial1Hitbox *this, CollisionSphere *sum) {
    Entity *target;
    Entity *child;

    Hit_apply(&this->base.base, sum);
    target = getHitFighter(sum);
    if (target != NULL) {
        entityAddSibling(&this->base.entity, createHitSpark(target, &this->unk100, 0x80));
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->unk9C, this->unk130 >> 2, 0x60, 6, 0x808080);
    }
    entityAddSibling(&this->base.entity, child);
    entityDelete(&this->base.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial1Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial2Hitbox_init);

/* The special move 2 hitbox's destructor. */
void GuilmonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = GUILMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &GUILMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial2Hitbox_tick);

/* On a hit: sets bit 0 of 0x44 (its tick reads it). */
void GuilmonSpecial2Hitbox_hit(Unk800AD5BC *this) {
    this->unk44 |= 1;
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial2Hitbox_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B1C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B24);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B2C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B34);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B3C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B44);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B4C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B5C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_SPECIAL2_HITBOX_OFFSET);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", D_80068B8C);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/guilmon", GUILMON_VTABLE);
