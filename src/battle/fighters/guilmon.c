/* Guilmon (CHARACTER_GUILMON): the fighter createFighter builds for it, its moves and the hitboxes of its special
 * moves 1 and 2. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/sin_cos.h"
#include "engine/gfx/camera.h"
#include "engine/math/matrix.h"

extern SVECTOR D_80068B8C[];


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
    /* 0x88 */ u16 unk88;
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

static inline VECTOR *inline_fn(VECTOR *arg0) {
    return arg0;
}

void GuilmonSpecial1Hitbox_draw(GuilmonSpecial1Hitbox *this, Camera *camera) {
    MatrixWords m;
    PolyFT4Shape shape;
    s32 dir;
    s32 scale;
    u8 *packet;
    u_long *slots;
    s32 zShift;
    MATRIX *matrix;
    s32 angle;
    s32 i;
    s32 j;
    VECTOR *point;
    s32 x;
    s32 y;
    s32 z;
    u32 half;
    u8 u;
    u8 alpha;

    half = this->unk84 >> 1;
    u = (half % 5) * 32;

    dir = 1;
    if (this->unk84 & 1) {
        dir = -1;
    }

    do {
        scale = 0x1000;
    } while (0);
    packet = PRIM_BUFFER_FREE;

    shape.corners[0] = &this->corners[0];
    shape.corners[1] = &this->corners[1];
    shape.corners[2] = &this->corners[2];
    shape.corners[3] = &this->corners[3];
    ((u8 *)(&shape.uv3))[0] = u + 0x1F;
    ((u8 *)(&shape.uv1))[0] = u + 0x1F;
    ((u8 *)(&shape.uv2))[0] = u;
    ((u8 *)(&shape.uv0))[0] = u;
    ((u8 *)(&shape.uv1))[1] = 0;
    ((u8 *)(&shape.uv0))[1] = 0;
    ((u8 *)(&shape.uv3))[1] = 0x1F;
    ((u8 *)(&shape.uv2))[1] = 0x1F;
    shape.flags = 9;
    shape.tpage = (((((0 & 0x3) << 7) | ((1 & 0x3) << 5)) | ((this->vramY & 0x100) >> 4)) |
                   ((0x1C0 & 0x3FF) >> 6)) |
                  ((this->vramY & 0x200) << 2);
    shape.clut = ((this->vramY + 0x7F) << 6) | ((0x1C0 >> 4) & 0x3F);

    slots = camera->ot.slots;
    zShift = camera->zShift;

    for (i = 0; i < this->unk88; i++) {
        if (this->unk90[i] == 2) {
            continue;
        }
        matrix = &m.m;
        point = inline_fn(&this->positions[i]);
        x = point->vx;
        y = point->vy;
        z = point->vz;
        m.words[0] = 0x1000;
        m.words[1] = 0;
        m.words[2] = 0x1000;
        m.words[3] = 0;
        m.words[4] = 0x1000;
        m.m.t[0] = x;
        m.m.t[1] = y;
        m.m.t[2] = z;
        angle = dir * scale;
        matrixScale(matrix, angle, scale, scale);
        scale -= 0x200;
        setModelMatrix(matrix);
        alpha = this->unk8A[i];
        shape.color = (alpha | (alpha << 8)) | (alpha << 16);
        j = 1;
        do {
            packet = buildPolyFT4(&shape, packet, slots, zShift);
        } while ((--j) >= 0);
    }

    PRIM_BUFFER_FREE = packet;
}

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

/* Draws the special move 2's flame aura: an initial POLY_FT4 sprite at offset (0, 0, 0)
 * followed by 9 rotating POLY_GT4 quads radiating outwards. */
void GuilmonSpecial2Hitbox_draw(GuilmonSpecial2Hitbox *this, Camera *camera) {
    MatrixWords m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    SVECTOR *new_var7;
    PolyGT4Shape shape2;
    int new_var5;
    MATRIX *mat;
    s32 i;
    VECTOR *point;
    s32 x;
    s32 y;
    s32 z;
    s32 dist0;
    s32 dist1;
    s32 angle0;
    s32 angle1;
    s32 cos0;
    int new_var6;
    s32 sin0;
    s32 cos1;
    s32 sin1;
    u8 c0;
    unsigned long c1;
    u32 color0;
    u32 color1;
    s16 *pA6;
    s16 *p92;
    s32 v5f;
    s32 v20;
    int new_var3;
    s32 v40;
    s32 v7f;
    CosSin *table;
    PolyGT4Shape *s2_ptr;
    SVECTOR *corn0;
    SVECTOR *corn1;
    SVECTOR *corn2;
    SVECTOR *corn3;
    s32 next_i;
    s32 zero = 0;

    mat = &m.m;
    shape.flags = 0;
    m.words[1] = 0;
    m.words[3] = 0;
    m.m.t[0] = 0;
    m.m.t[1] = 0;
    m.m.t[2] = 0;
    m.words[4] = (m.words[2] = (m.words[0] = *(u16 *)&this->unkD0));
    point = inline_fn((VECTOR *)&this->unkE4);
    x = point->vx;
    y = point->vy;
    z = point->vz;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    setModelMatrix(mat);

    shape.tpage = (((((0 & 0x3) << 7) | ((1 & 0x3) << 5)) | ((this->unk88 & 0x100) >> 4)) | ((0x1C0 & 0x3ff) >> 6)) |
                  ((this->unk88 & 0x200) << 2);
    shape.flags = (shape.flags & (~3)) | 9;
    if ((this->unk86 ^ 1) & 1) {
        ((u8 *)(&shape.uv0))[0] = 0xB0;
        ((u8 *)(&shape.uv1))[0] = 0xEF;
        ((u8 *)(&shape.uv2))[0] = 0xB0;
        ((u8 *)(&shape.uv3))[0] = 0xEF;
    } else {
        ((u8 *)(&shape.uv0))[0] = 0xEF;
        ((u8 *)(&shape.uv1))[0] = 0xB0;
        ((u8 *)(&shape.uv2))[0] = 0xEF;
        ((u8 *)(&shape.uv3))[0] = 0xB0;
    }
    ((u8 *)(&shape.uv0))[1] = 0xC0;
    ((u8 *)(&shape.uv1))[1] = 0xC0;
    ((u8 *)(&shape.uv2))[1] = 0xFF;
    ((u8 *)(&shape.uv3))[1] = 0xFF;
    shape.color = 0x808080;
    shape.corners[0] = &D_80068B8C[0];
    shape.corners[1] = &D_80068B8C[1];
    new_var7 = &D_80068B8C[2];
    shape.corners[2] = new_var7;
    shape.corners[3] = &D_80068B8C[3];
    shape.clut = ((this->unk88 + 0xF8) << 6) | ((0x1C0 >> 4) & 0x3f);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + 1);

    shape2.flags = 0;
    if (this->unk84 >= (0 + 1)) {
        m.words[0] = 0x1000;
        m.words[1] = 0;
        m.words[2] = 0x1000;
        m.words[3] = 0;
        m.words[4] = 0x1000;
        m.m.t[0] = 0;
        m.m.t[1] = 0;
        m.m.t[2] = 0;
    } else {
        matrixInitRotationY(&m.m, 0x800);
    }
    {
        VECTOR *pos = (VECTOR *)&this->unkD4;
        m.m.t[0] = pos->vx;
        m.m.t[1] = pos->vy;
        m.m.t[2] = pos->vz;
    }
    mat = &m.m;
    setModelMatrix(mat);

    v5f = 0x5F;
    v20 = 0x20;
    v40 = 0x40;
    v7f = 0x7F;
    i = 0;
    pA6 = this->unkA6;
    corn0 = &corners[0];
    corn1 = &corners[1];
    corn2 = &corners[2];
    corn3 = &corners[3];
    s2_ptr = &shape2;
    table = COS_SIN_TABLE;
    shape2.tpage = (((((0 & 0x3) << 7) | ((1 & 0x3) << 5)) | ((this->unk88 & 0x100) >> 4)) | ((0x1C0 & 0x3ff) >> 6)) |
                   ((this->unk88 & 0x200) << 2);
    p92 = this->unk92;
    shape2.flags = (shape2.flags & (~3)) | 9;

    for (; i < 9; i++) {
        switch (i) {
        case 0:
            new_var5 = 0x3F;
            do {
                shape2.clut = ((this->unk88 + 0x7D) << 6) | ((0x1C0 >> 4) & 0x3f);
                shape2.u0 = 0;
                shape2.v0 = v5f;
                shape2.u1 = new_var5;
                shape2.v1 = v5f;
                shape2.u2 = 0;
                shape2.v2 = v20;
                shape2.u3 = new_var5;
                shape2.v3 = v20;
            } while (0);
            break;

        case 8:
            if ((this->unk86 ^ 1) & 1) {
                shape2.u0 = 0x80;
                shape2.u1 = 0xBF;
                shape2.u2 = 0x80;
                shape2.u3 = 0xBF;
                shape2.clut = ((this->unk88 + 0x7E) << 6) | ((0x1C0 >> 4) & 0x3f);
            } else {
                shape2.u0 = 0xC0;
                shape2.u1 = 0xFF;
                shape2.u2 = 0xC0;
                shape2.u3 = 0xFF;
                shape2.clut = ((this->unk88 + 0x7B) << 6) | ((0x1C0 >> 4) & 0x3f);
            }
            shape2.v0 = v5f;
            shape2.v1 = v5f;
            shape2.v2 = v20;
            shape2.v3 = v20;
            break;

        default:
            if ((this->unk86 ^ 1) & 1) {
                shape2.u0 = v40;
                shape2.u1 = v7f;
                shape2.u2 = v40;
                shape2.u3 = v7f;
            } else if (camera) {
                shape2.u0 = v7f;
                shape2.u1 = v40;
                shape2.u2 = v7f;
                shape2.u3 = v40;
            } else {
                shape2.u0 = v7f;
                shape2.u1 = v40;
                shape2.u2 = v7f;
                shape2.u3 = v40;
            }
            shape2.v0 = v5f;
            shape2.v1 = v5f;
            shape2.v2 = v20;
            shape2.v3 = v20;
            shape2.clut = ((this->unk88 + 0x7C) << 6) | ((0x1C0 >> 4) & 0x3f);
            break;
        }

        {
            u16 unk8c;
            s32 base_dist;
            next_i = i + 1;
            unk8c = this->unk8C;
            if (zero == 0) {
                base_dist = i * 32;
                if (unk8c == 0 && i >= 5) {
                    new_var6 = i << 3;
                    new_var6 -= 8;
                    dist0 = i * new_var6;
                    dist1 = next_i * (i << 3);
                } else {
                    dist0 = base_dist;
                    dist1 = next_i * 32;
                }
            }
        }

        if (*(pA6 = pA6, pA6 + i) >= 2) {
            dist0 += 0x80 - *(p92 = p92, p92 + i);
        }
        if (2 <= *(pA6 = pA6, pA6 + next_i)) {
            dist1 += 0x80 - *(p92 = p92, p92 + next_i);
        }

        dist0 += 0x50;
        dist1 += 0x50;

        shape2.corners[0] = corn0;
        shape2.corners[1] = corn1;
        shape2.corners[2] = corn2;
        shape2.corners[3] = corn3;
        corners[3].vz = 0;
        corners[2].vz = 0;
        new_var3 = 1;
        corners[1].vz = 0;
        corners[0].vz = 0;

        angle0 = ((i << 8) - 0x100) & 0xFFF;
        cos0 = table[angle0].cos;
        sin0 = -COS_SIN_TABLE[angle0].sin;
        corners[0].vx = (cos0 * 19) >> 10;
        corners[0].vy = (sin0 * 19) >> 10;
        corners[new_var3].vx = (cos0 * dist0) >> 12;
        corners[new_var3].vy = (sin0 * dist0) >> 12;

        angle1 = ((next_i << 8) - 0x100) & 0xFFF;
        cos1 = table[angle1].cos;
        sin1 = -COS_SIN_TABLE[angle1].sin;
        corners[2].vx = (cos1 * 19) >> 10;
        corners[2].vy = (sin1 * 19) >> 10;
        corners[3].vx = (cos1 * dist1) >> 12;
        corners[3].vy = (sin1 * dist1) >> 12;

        c0 = *(u8 *)(p92 = p92, (u8 *)p92 + (9 - i) * 2);
        c1 = *(u8 *)(p92 = p92, (u8 *)p92 + (9 - next_i) * 2);
        color0 = (c0 | (c0 << 8)) | (c0 << 16);
        color1 = (c1 | (c1 << 8)) | (c1 << 16);
        shape2.colors[0] = (shape2.colors[1] = color0);
        shape2.colors[2] = (shape2.colors[3] = color1);

        PRIM_BUFFER_FREE = buildPolyGT4(s2_ptr, PRIM_BUFFER_FREE, camera->ot.slots, camera->zShift + 1);
    }
}


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
