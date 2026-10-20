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

void Guilmon_spawnSpecial2Hitbox(Fighter *this, s32 air);

void Guilmon_special2(Fighter *this, s32 frame) {
    Fighter *opponent;
    s32 push;
    s32 opponentPush;

    func_80079AE4(this);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_SPECIAL2, 0, 0, -1);
        push = ((s64)this->mass * 0x38E0) >> 16;
        this->velocity.vx = 0;
        this->force.vy -= push;
        this->force.vx += push * Fighter_getFacing(this);
        Guilmon_spawnSpecial2Hitbox(this, 0);
        this->hitDamage = 0x81AC12E6;
        Fighter_setAttackParts(this, 2);
        opponentPush = ((s64)opponent->mass * 0x1C70) >> 16;
        opponentPush = opponentPush * Fighter_getFacing(this);
        this->hitKnockback.vy = -(((s64)opponent->mass * 0x1C70) >> 16);
        this->hitKnockback.vz = 0;
        this->hitKnockback.vx = opponentPush;
        Fighter_playVoice(this, 0x8C);
        func_80079B14(this, 31);
        break;
    case 20:
        this->attackParts = 0;
        break;
    }
    if (frame >= 1 && frame <= 14) {
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
    if (frame >= 16) {
        this->force.vx -= this->velocity.vx >> 4;
    }
    if (Fighter_isActionClipDone(this)) {
        this->force.vx = 0;
        this->velocity.vx = 0;
    }
}

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

void Guilmon_airSpecial2(Fighter *this, s32 frame) {
    Fighter *opponent;
    s32 push;
    s32 opponentPush;

    func_80079AE4(this);
    opponent = Fighter_getOpponent(this);
    switch (frame) {
    case 0:
        Fighter_playActionClip(this, FIGHTER_CLIP_AIR_SPECIAL2, 2, 0, -1);
        push = ((s64)this->mass * 0x1C70) >> 16;
        this->force.vx += push * Fighter_getFacing(this);
        this->hitDamage = 0x81AC12E6;
        Fighter_setAttackParts(this, 2);
        opponentPush = ((s64)opponent->mass * 0x1C70) >> 16;
        opponentPush = opponentPush * Fighter_getFacing(this);
        this->hitKnockback.vy = -(((s64)opponent->mass * 0x1C70) >> 16);
        this->hitKnockback.vz = 0;
        this->hitKnockback.vx = opponentPush;
        Fighter_playVoice(this, 0x8C);
        func_80079B14(this, 31);
        break;
    case 2:
        Guilmon_spawnSpecial2Hitbox(this, 1);
        break;
    case 10:
        this->attackParts = 0;
        break;
    }
    if (frame < 5) {
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

typedef struct GuilmonSpecial1Hitbox {
    /* 0x000 */ UnkPartsEntity38 base;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ u16 unk84;
    /* 0x086 */ u16 vramY;
    /* 0x088 */ u16 unk88;
    /* 0x08A */ u8 unk8A[6];
    /* 0x090 */ u16 unk90[6];
    /* 0x09C */ VECTOR positions[5];
    /* 0x0EC */ s32 unkEC[5];
    /* 0x100 */ VECTOR velocity;
    /* 0x110 */ SVECTOR corners[4];
    /* 0x130 */ s32 length;
} GuilmonSpecial1Hitbox;

GuilmonSpecial1Hitbox *GuilmonSpecial1Hitbox_init(GuilmonSpecial1Hitbox *this, Fighter *fighter, Vec3 *position,
                                                 VECTOR *velocity, Fight *list);

/* Spawns the special move 1's hitbox at the fighter's position minus 144 on y, flying forward at 80 per frame.
 * Both the ground and air versions shift y by 144. */
void Guilmon_spawnSpecial1Hitbox(Fighter *this, s32 air) {
    Vec3 position;
    VECTOR velocity;
    Fight *list;
    Entity *child;

    list = Fighter_getScene(this);
    Fighter_getPosition(this, &position);
    if (air) {
        position.vy -= 144;
    } else {
        position.vy -= 144;
    }
    velocity.vx = Fighter_getFacing(this) * 80;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x134);
    if (child != NULL) {
        GuilmonSpecial1Hitbox_init(entityOperatorNewAt(0x134, child), this, &position, &velocity, list);
    }
    Fighter_addEntity(this, child);
}

typedef struct GuilmonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 base;
    /* 0x80 */ Fighter *fighter;
    /* 0x84 */ s16 unk84;
    /* 0x86 */ u16 unk86;
    /* 0x88 */ s16 unk88;
    /* 0x8A */ u16 unk8A;
    /* 0x8C */ u16 unk8C;
    /* 0x8E */ u16 unk8E;
    /* 0x90 */ s16 unk90;
    /* 0x92 */ s16 unk92[10];
    /* 0xA6 */ s16 unkA6[10];
    /* 0xBA */ s16 unkBA[10];
    /* 0xCE */ s16 unkCE;
    /* 0xD0 */ s32 unkD0;
    /* 0xD4 */ s32 unkD4;
    /* 0xD8 */ u8 unkD8[0xE4 - 0xD8];
    /* 0xE4 */ VECTOR unkE4;
} GuilmonSpecial2Hitbox;

GuilmonSpecial2Hitbox *GuilmonSpecial2Hitbox_init(GuilmonSpecial2Hitbox *this, Fighter *fighter, Vec3 *position,
                                                 VECTOR *velocity, Fight *list);
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

GuilmonSpecial1Hitbox *GuilmonSpecial1Hitbox_init(GuilmonSpecial1Hitbox *this, Fighter *fighter, Vec3 *position,
                                                 VECTOR *velocity, Fight *list) {
    VECTOR knockback;
    s32 vx;
    s32 i;
    s32 alpha;
    VECTOR *pPos;
    u16 *p90;
    s32 *pEC;
    u8 *p8A;
    Hit *base;
    ListNode *node;

    entityInit(&this->base.entity, 0x30000);
    base = &this->base.base;
    Hit_init(base, fighter, 0x5A81E00, (Vec3 *)velocity, 3);
    this->base.base.vtable = GUILMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->base.entity.vtable = &GUILMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&this->base.part, position, 0x80, base);

    vx = velocity->vx << 6;
    knockback.pad = velocity->pad;
    knockback.vz = 0;
    knockback.vx = vx;
    knockback.vy = -(vx < 0 ? -vx : vx);
    Hit_setKnockback(base, &knockback);

    this->vramY = Fighter_getVramY(fighter);
    this->fighter = fighter;

    if (velocity->vx > 0) {
        this->corners[0].vx = this->corners[2].vx = -0x80;
        this->corners[1].vx = this->corners[3].vx = 0x80;
    } else {
        this->corners[0].vx = this->corners[2].vx = 0x80;
        this->corners[1].vx = this->corners[3].vx = -0x80;
    }

    this->corners[1].vy = -0x80;
    this->corners[0].vy = -0x80;
    this->corners[3].vy = 0x80;
    this->corners[2].vy = 0x80;
    this->corners[3].vz = 0;
    this->corners[2].vz = 0;
    this->corners[1].vz = 0;
    this->corners[0].vz = 0;

    {
        s32 x = velocity->vx;
        s32 y = velocity->vy;
        s32 z = velocity->vz;
        this->velocity.vx = x;
        this->velocity.vy = y;
        this->velocity.vz = z;
    }
    this->length = vecGetLength(velocity);

    i = 0;
    node = &this->base.part.link;
    p8A = this->unk8A;
    pPos = this->positions;
    pEC = this->unkEC;
    p90 = this->unk90;
    alpha = -0x80;
    this->unk84 = 0;
    this->unk88 = 0;

    do {
        s32 x, y, z;
        p8A[i] = alpha;
        *p90 = 0;
        *pEC = 0;
        pEC++;
        p90++;
        alpha -= 0x19;
        x = position->vx;
        y = position->vy;
        z = position->vz;
        i++;
        pPos->vx = x;
        pPos->vy = y;
        pPos->vz = z;
        pPos++;
    } while (i < 5);

    this->base.part.entity = &this->base.entity;
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(list, node);
    } else {
        Fight_addSide1Shape(list, node);
    }

    this->base.part.flags.word |= 1;
    Fighter_playSound(this->fighter, 0x87);

    return this;
}

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

void GuilmonSpecial1Hitbox_tick(GuilmonSpecial1Hitbox *this) {
    s32 active;
    s32 i;
    u16 *p90;
    VECTOR *pPos;
    u8 *p8A;
    s32 *pEC;
    s32 len;
    u32 alpha;
    s32 x, y, z;
    s32 val;
    s32 state;
    int new_var;

    if (++this->unk88 < 6) {
    } else {
        this->unk88 = 5;
    }

    active = 0;
    i = 0;

    if (this->unk88 != 0) {
        new_var = 1;
        p90 = this->unk90;
        pPos = this->positions;
        p8A = this->unk8A;
        pEC = this->unkEC;
        do {
            state = *p90;
            if (state == new_var) {
                goto state1_block;
            }
            if (state >= 2) {
                goto next;
            }
            if (state != 0) {
                goto next;
            }

            x = pPos->vx;
            y = pPos->vy;
            z = pPos->vz;
            x += this->velocity.vx;
            y += this->velocity.vy;
            z += this->velocity.vz;
            pPos->vx = x;
            pPos->vy = y;
            pPos->vz = z;

            val = *pEC;
            len = this->length;
            val += len < 0 ? -len : len;
            *pEC = val;
            active = 1;
            if (*pEC >= 0xA01) {
                (*p90)++;
            } else {
                goto next;
            }

        state1_block:
            alpha = *p8A;
            alpha -= alpha >> 2;
            *p8A = alpha;
            active = 1;
            if ((u8)alpha < 5) {
                (*p90)++;
            }

        next:
            p90++;
            pPos++;
            p8A++;
            pEC++;
        } while (++i < this->unk88);
    }

    if (this->unk84 == 2) {
        this->base.part.flags.word &= ~1;
    }

    if (active == 0) {
        entityDelete(&this->base.entity);
        return;
    }

    this->base.part.position.vx = this->positions[0].vx;
    this->base.part.position.vy = this->positions[0].vy;
    this->base.part.position.vz = this->positions[0].vz;
    this->unk84++;
}

/* On a hit: Hit_apply, a hit spark on the fighter it struck, a burst of 6 particles at 0x9C, and it deletes itself. */
void GuilmonSpecial1Hitbox_hit(GuilmonSpecial1Hitbox *this, CollisionSphere *sum) {
    Entity *target;
    Entity *child;

    Hit_apply(&this->base.base, sum);
    target = getHitFighter(sum);
    if (target != NULL) {
        entityAddSibling(&this->base.entity, createHitSpark(target, (Vec3 *)&this->velocity, 0x80));
    }
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, (Vec3 *)&this->positions[0], this->length >> 2, 0x60, 6, 0x808080);
    }
    entityAddSibling(&this->base.entity, child);
    entityDelete(&this->base.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/guilmon", GuilmonSpecial1Hitbox_draw);

GuilmonSpecial2Hitbox *GuilmonSpecial2Hitbox_init(GuilmonSpecial2Hitbox *this, Fighter *fighter, Vec3 *position,
                                                 VECTOR *velocity, Fight *list) {
    VECTOR offset;
    VECTOR knockback;
    s32 vx;
    s32 i;
    Hit *base;
    CollisionSphere *part;
    s16 *p92, *pA6, *pBA;

    entityInit(&this->base.entity, 0x30000);
    base = &this->base.base;
    Hit_init(base, fighter, 0x85AC1B00, (Vec3 *)velocity, 0);
    part = &this->base.part;
    this->base.base.vtable = GUILMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->base.entity.vtable = &GUILMON_SPECIAL2_HITBOX_VTABLE;
    CollisionSphere_init(part, position, 0x60, base);

    offset = GUILMON_SPECIAL2_HITBOX_OFFSET;
    vx = velocity->vx << 6;
    knockback.pad = velocity->pad;
    knockback.vz = 0;
    knockback.vx = vx;
    knockback.vy = -(vx < 0 ? -vx : vx);
    Hit_setKnockback(base, &knockback);

    this->unk88 = Fighter_getVramY(fighter);
    this->fighter = fighter;
    this->unk8A = 0;
    this->unk86 = 0;
    this->unk90 = 0;
    this->unkD0 = 0x1000;
    this->unk8C = velocity->vz;
    this->unk84 = Fighter_getFacing(fighter);
    this->base.part.entity = &this->base.entity;

    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(list, (ListNode *)part);
    } else {
        Fight_addSide1Shape(list, (ListNode *)part);
    }

    Fighter_getNodePosition(this->fighter, (VECTOR *)&this->unkE4, 16, &offset);
    func_8007A650(this->fighter, (Vec3 *)&this->unkD4);

    pBA = this->unkBA;
    pA6 = this->unkA6;
    p92 = this->unk92;
    i = 9;
    this->unkD4 += (s32)this->unk84 << 6;
    do {
        *p92 = 0;
        *pA6 = 0;
        *pBA = 0;
        pBA++;
        pA6++;
        p92++;
    } while (--i >= 0);

    if (this->unk8C == 0) {
        this->unk8E = 0;
    } else {
        this->unk8E = 2;
    }

    Fighter_playSound(this->fighter, 0x88);
    return this;
}

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

void GuilmonSpecial2Hitbox_tick(GuilmonSpecial2Hitbox *this) {
    VECTOR offset_vec;
    s32 active;
    s32 step;
    s32 offset;
    s32 i;
    s16 *pA6;
    u16 unk8E;
    s32 cond;
    s32 one;
    s16 *pBA;

    offset_vec = GUILMON_SPECIAL2_HITBOX_OFFSET;
    Fighter_getNodePosition(this->fighter, (VECTOR *)&this->unkE4, 16, &offset_vec);
    func_8007A650(this->fighter, (Vec3 *)&this->unkD4);
    unk8E = this->unk8E;
    this->unkD4 += (s32)this->unk84 << 6;

    active = 0;

    switch ((s16)this->unk8E) {
    case 0:
        if (++this->unk8A < 4) {
            break;
        }
        this->unk8A = 4;
        this->unk90 = 0;
        this->unk8E = unk8E + 1;
        /* fallthrough */
    case 1:
        if (++this->unk90 < 6) {
            break;
        }
        this->unk8E++;
        /* fallthrough */
    case 2:
        if (++this->unk8A < 10) {
            break;
        }
        this->unk8A = 10;
        this->unk8E++;
        break;
    }

    step = (this->unk8C == 0) ? 8 : 16;

    i = 0;
    if (i < this->unk8A) {
        one = 1;
        pA6 = this->unkA6;
        offset = 0;
        do {
            if (*pA6 == one) {
                goto case1;
            }
            if (*pA6 < 2) {
                if (*pA6 == 0) {
                    goto case0;
                }
                goto next;
            }
            if (*pA6 == 2) {
                goto case2;
            }
            goto next;

        case0:
            pA6[-10] += 0x40 + step;
            if ((s16)pA6[-10] < 0x80) {
                active = 1;
                goto next;
            }
            pA6[-10] = 0x80;
            (*pA6)++;
            /* fallthrough */
        case1:
            pBA = this->unkBA;
            cond = (s16)this->unk8E < 2;
            active = 1;
            if (!cond) {
                (*(s16 *)((u8 *)pBA + offset))++;
            }
            if (*(s16 *)((u8 *)pBA + offset) < 8) {
                goto next;
            }
            (*pA6)++;
            /* fallthrough */
        case2:
            pA6[-10] -= 0x10 + step;
            active = 1;
            if ((s16)pA6[-10] > 0) {
                goto next;
            }
            pA6[-10] = 0;
            (*pA6)++;
            goto next;

        next:
            pA6++;
            offset += 2;
        } while (++i < this->unk8A);
    }

    if (active == 0) {
        entityDelete(&this->base.entity);
        return;
    }

    if (this->unk8C == 0) {
        cond = this->unk86 < 11;
    } else {
        cond = this->unk86 < 5;
    }
    if (!cond) {
        this->base.part.flags.word |= 1;
        this->unkD0 += (-this->unkD0) >> 3;
    }

    this->base.part.position.vx = this->unkE4.vx;
    this->base.part.position.vy = this->unkE4.vy;
    this->base.part.position.vz = this->unkE4.vz;
    this->unk86++;
}

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
