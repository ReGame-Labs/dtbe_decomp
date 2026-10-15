/* SaintGargomon (CHARACTER_SAINTGARGOMON): the fighter createFighter builds for it, its moves, and the entities they
 * spawn: the special move 1's hitbox and the special move 3's four kinds of hitboxes and their children. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/gfx/prim/build_line_f3.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/sound/sound_control.h"

/* Entities with a second base at 0x18: the virtual tables of that base move
 * `this` back by 0x18 to reach them. */
typedef struct Unk800ACD74 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
} Unk800ACD74;

/* the end of an Unk800AC470: spawns two entities and deletes it */
void func_800AC674(Unk800AC470 *this);
Entity *func_800AD5BC(Entity *this, Fighter *arg1, Fight *arg2, Vec3 *pos, s32 arg4, s32 arg5);
extern VtableEntry D_8006B314[];
extern EntityVtable D_8006B334;
extern VtableEntry D_8006B35C[];
extern EntityVtable D_8006B37C;
extern VtableEntry D_8006B3A4[];
extern EntityVtable D_8006B3C4;
extern VtableEntry D_8006B3EC[];
extern EntityVtable D_8006B40C;
extern VtableEntry D_8006B434[];
extern EntityVtable D_8006B454;
extern VtableEntry SAINTGARGOMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable SAINTGARGOMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable SAINTGARGOMON_VTABLE;

/* SaintGargomon's class (CHARACTER_SAINTGARGOMON): special3 keeps the handle of its sound. */
typedef struct SaintGargomon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ u32 soundHandle; /* special3's sound (Fighter_playSound), stopped at its end */
} SaintGargomon;
void func_800AB830(UnkPartsEntity38 *this);

/* Builds SaintGargomon: a Fighter with its own vtable, 0x4A4 0. */
SaintGargomon *SaintGargomon_init(SaintGargomon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &SAINTGARGOMON_VTABLE;
    this->unk4A4 = 0;
    return this;
}

/* SaintGargomon's destructor. */
void SaintGargomon_destroy(Entity *this, s32 flags) {
    this->vtable = &SAINTGARGOMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: releases the opponent (Fighter_detach). */
void SaintGargomon_endMove(Fighter *this) {
    Fighter_detach(Fighter_getOpponent(this));
}

/* Throw: Fighter's; until frame 24 the fighter stops (Fighter_accelerateTo(0)) and the opponent takes its velocity and
 * force; frame 0 holds the opponent in FIGHTER_CLIP_FLINCH; frame 24, when the opponent is within 0x200 by 0x80, throws
 * it (Fighter_takeHit) with a spark at node 9, else lets it go (Fighter_stopActionClip). */
void SaintGargomon_throw(Fighter *this, s32 frame) {
    Vec3 position;
    Vec3 opponentPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Fighter *opponent;
    Entity *child;
    s32 dx, dy;

    Fighter_throw(this, frame);
    opponent = Fighter_getOpponent(this);
    if (frame < 24) {
        s32 x, y, z;

        Fighter_accelerateTo(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->velocity.vx;
        y = this->velocity.vy;
        z = this->velocity.vz;
        opponent->velocity.vx = x;
        opponent->velocity.vy = y;
        opponent->velocity.vz = z;
        x = this->force.vx;
        y = this->force.vy;
        z = this->force.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
    }
    switch (frame) {
    case 0:
        Fighter_hold(opponent, 24);
        Fighter_setClip(opponent, FIGHTER_CLIP_FLINCH, 0, 0);
        break;
    case 24:
        Fighter_getPosition(this, &position);
        Fighter_getOpponentPosition(this, &opponentPosition);
        dx = ABS(position.vx - opponentPosition.vx);
        dy = ABS(position.vy - opponentPosition.vy);
        if (dx < 0x200 && dy < 0x80) {
            velocity.vx = (s16)Fighter_getFacing(this) * 18200;
            velocity.vy = -0x4718;
            velocity.vz = 0;
            Fighter_takeHit(opponent, 0xC38C3400, &velocity, 0);
            memset(&offset, 0, sizeof(offset));
            Fighter_getNodePosition(this, (VECTOR *)&node, 9, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                HitSpark_init(entityOperatorNewAt(0x48, child), &node, &ZERO_VECTOR, 0xC0, 0x808080);
            }
            Fighter_addEntity(this, child);
        } else {
            Fighter_hold(opponent, 0);
            Fighter_stopActionClip(this);
        }
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", SaintGargomon_special2);

void SaintGargomon_spawnSpecial1Hitbox(Fighter *this, s32 arg);

/* Special move 1: Fighter's; the voice at frame 0, the hitbox (spawnSpecial1Hitbox(-21)) and sound 0xDB at frame 7;
 * until frame 8 the force brakes a sixteenth of the horizontal velocity. */
void SaintGargomon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playVoice(this, 0xDF);
    }
    if (frame == 7) {
        SaintGargomon_spawnSpecial1Hitbox(this, -21);
        Fighter_playSound(this, 0xDB);
    }
    if (frame < 8) {
        this->force.vx += -this->velocity.vx >> 4;
    }
}

/* Special move 1 in the air: Fighter's; the voice and sound 0xDB at frame 0, the hitbox (spawnSpecial1Hitbox(0)) at
 * frame 7 with a push back by 0x3000; until frame 8 the force brakes an eighth of the horizontal velocity and all of a
 * fall. */
void SaintGargomon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    if (frame == 0) {
        Fighter_playVoice(this, 0xDF);
        Fighter_playSound(this, 0xDB);
    }
    if (frame == 7) {
        SaintGargomon_spawnSpecial1Hitbox(this, 0);
        this->force.vx -= Fighter_getFacing(this) * 0x3000;
    }
    if (frame < 8) {
        this->force.vx -= this->velocity.vx >> 3;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B1F4);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B204);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", SaintGargomon_spawnSpecial1Hitbox);

void func_800AADB0(SaintGargomon *this, s32 arg);
void func_800AB09C(SaintGargomon *this, s32 step);
void func_800AAF30(SaintGargomon *this, s32 step);
void func_800AB4EC(SaintGargomon *this, s32 arg);

/* Special move 3: Fighter's; frame 0 spends a gauge bar, plays the voice and sets 10 invulnerable frames; frame 0x13
 * starts the sound, which frame 0x47 stops; in between it spawns the hitboxes: func_800AB09C every frame (after
 * func_800AADB0 on odd frames), and func_800AAF30 and func_800AB4EC every eighth frame. */
void SaintGargomon_special3(SaintGargomon *this, s32 frame) {
    Fighter_special3(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        Fighter_addGauge(&this->base, -0x10000);
        Fighter_playVoice(&this->base, 0xE1);
        this->base.invulnerableFrames = 10;
    }
    if (frame == 0x13) {
        this->soundHandle = Fighter_playSound(&this->base, 0xDB);
    }
    if (frame == 0x47 && this->soundHandle != 0) {
        stopSound(this->soundHandle);
    }
    if (frame >= 0x13 && frame < 0x47) {
        if (frame & 1) {
            func_800AADB0(this, frame);
        }
        func_800AB09C(this, frame - 0x13);
        if ((frame & 7) == 0) {
            func_800AAF30(this, frame - 0x13);
            func_800AB4EC(this, frame >> 3);
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AADB0);

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B214);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AAF30);

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B224);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AB09C);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AB4EC);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", SaintGargomonSpecial1Hitbox_init);

/* The special move 1's hitbox's destructor. */
void SaintGargomonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = SAINTGARGOMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &SAINTGARGOMON_SPECIAL1_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AB830);

/* On a hit: Hit_apply with sum, then func_800AB830 (a spray and a child hitbox). */
void SaintGargomonSpecial1Hitbox_hit(UnkPartsEntity38 *this, void *sum) {
    Hit_apply(&this->base, sum);
    func_800AB830(this);
}

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", SaintGargomonSpecial1Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800ABB54);

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B2B4);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", SaintGargomonSpecial1Hitbox_draw);

/* The class func_800ABE6C constructs (virtual tables D_8006B454, and D_8006B434 for its base at 0x18). */
typedef struct Unk800ABE6C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ CollisionSphere unk38;
    /* 0x80 */ Fighter *fighter;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0x94 - 0x90];
    /* 0x94 */ Vec3 reach; /* position, frames * 128 ahead (func_800AC138); the burst appears there */
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ Vec3 velocity;
    /* 0xB0 */ u8 unkB0[0xB4 - 0xB0];
    /* 0xB4 */ s32 frames;
    /* 0xB8 */ s16 facing; /* the character's (Fighter_getFacing) */
    /* 0xBA */ s16 vramY;  /* the character's 0x440 (Fighter_getVramY) */
    /* 0xBC */ u16 unkBC; /* 1 once func_800AC080 made its burst */
} Unk800ABE6C;

/* Constructs Unk800ABE6C for fighter at position, moving by velocity, and adds the part at 0x38 to one of list's two
 * lists (Fighter_getSide picks which). */
Unk800ABE6C *func_800ABE6C(Unk800ABE6C *this, Fighter *fighter, Vec3 *position, VECTOR *velocity,
                           Fight *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->unk18, fighter, 0x7B00100, (Vec3 *)velocity, 0);
    this->unk18.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
    CollisionSphere_init(&this->unk38, position, 0x40, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    Hit_setKnockback(&this->unk18, &v);
    this->fighter = fighter;
    this->facing = Fighter_getFacing(fighter);
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->frames = 0;
    this->unkBC = 0;
    this->vramY = Fighter_getVramY(fighter);
    this->unk38.entity = &this->entity;
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(list, &this->unk38.link);
    } else {
        Fight_addSide1Shape(list, &this->unk38.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B454 and D_8006B434, which
 * func_800ABE6C installs. */
void func_800ABFE0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
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

/* The second base's first method for func_800ABE6C's class: Hit_apply with sum; the first time (0xBC
 * still 0), adds a burst of 2 (ParticleBurst_init) at its reach as a sibling and sets bit 0 of the part's flags. */
void func_800AC080(Unk800ABE6C *this, void *sum) {
    Entity *child;
    s32 radius;

    Hit_apply(&this->unk18, sum);
    if (this->unkBC == 0) {
        radius = this->velocity.vx >> 2;
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->reach, radius, 0x60, 2, 0x808080);
        }
        entityAddSibling(&this->entity, child);
        this->unkBC = 1;
        this->unk38.flags |= COLLISION_SHAPE_DISABLED;
    }
}

/* The tick of Unk800ABE6C: puts its reach and its part frames * 128 ahead of position; deletes itself when the fighter
 * is gone or its attack is idle, else after 7 frames. The match depends on a return after each of the first two
 * deletes: under the unit's -fno-cse-follow-jumps the call's block then reloads the fighter. */
void func_800AC138(Unk800ABE6C *this) {
    this->unk38.position.vx = this->reach.vx = this->position.vx + this->facing * (this->frames << 7);
    this->unk38.position.vy = this->reach.vy = this->position.vy;
    this->unk38.position.vz = this->reach.vz = this->position.vz;
    if (this->fighter == NULL) {
        entityDelete(&this->entity);
        return;
    }
    if (Fighter_isAttackIdle(this->fighter)) {
        entityDelete(&this->entity);
        return;
    }
    this->frames++;
    if (this->frames >= 7) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B2D4);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AC21C);

/* Constructs an Unk800AC470 (vtables D_8006B40C and D_8006B3EC): the same as func_800ACA60. */
Unk800AC470 *func_800AC470(Unk800AC470 *this, Fighter *owner, VECTOR *pos, VECTOR *vel, Fight *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B3EC;
    this->parts.entity.vtable = &D_8006B40C;
    CollisionSphere_init(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    Hit_setKnockback(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->frames = 0;
    this->vramY = Fighter_getVramY(owner);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(this->owner) == 0) {
        Fight_addSide0Shape(arg4, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(arg4, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B40C and D_8006B3EC, which
 * func_800AC470 installs. */
void func_800AC5D4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3EC;
    this->entity.vtable = &D_8006B40C;
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

/* Spawns an entity of func_800AD5BC's class and one of ParticleBurst_init's at position, then deletes itself
 * (func_800ACC64 without the sound). */
void func_800AC674(Unk800AC470 *this) {
    Entity *child;
    s32 vx;

    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->position, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading velocity.vx before allocating the second entity. */
    vx = this->velocity.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->position, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AC778(Unk800AC470 *this, void *arg) {
    Hit_apply(&this->parts.base, arg);
    func_800AC674(this);
}

/* The tick of Unk800AC470 (D_8006B40C): the same as func_800ACDA4, with func_800AC674. */
void func_800AC7A8(Unk800AC470 *this, s32 arg) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    this->frames++;
    if (this->frames >= 12) {
        func_800AC674(this);
    }
}

/* The shape buildPolyG4 projects (engine/gfx/prim/build_line_f3.h takes it as void *): four corners and a color
 * for each. */
typedef struct PolyG4Shape {
    /* 0x00 */ u32 flags;
    /* 0x04 */ SVECTOR *corners[4];
    /* 0x14 */ u32 colors[4];
} PolyG4Shape; /* size 0x24 */

/* PolyFT4Shape's uv words as their u and v bytes: game_classes.h types them u16, and the game stores the bytes
 * (RESULT.md). */
#define UV_BYTES(uv) ((u8 *)&(uv))

/* matrixSetTranslationVec (engine/math/matrix_ops.c), which game.bin inlines. */
static inline MATRIX *matrixSetTranslationVecInline(MATRIX *m, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    m->t[0] = x;
    m->t[1] = y;
    m->t[2] = z;
    return m;
}

/* Entry 0x14 of vtable D_8006B40C: the same as func_800ACE1C with a 0x60 by 0x30 picture from the next
 * texture rows and an orange-to-red trail. */
void func_800AC820(Unk800AC470 *this, Camera *camera) {
    VECTOR pos;
    MATRIX m;
    SVECTOR corners[4];
    PolyFT4Shape sprite;
    PolyG4Shape trail;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 vramY;

    pos = *(VECTOR *)&this->position;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->velocity.vy, this->velocity.vx));
    matrixSetTranslationVecInline(&m, &pos);
    setModelMatrix(&m);
    corners[0].vx = corners[2].vx = -0x30;
    corners[1].vx = corners[3].vx = 0x30;
    corners[0].vy = corners[1].vy = -0x18;
    corners[2].vy = corners[3].vy = 0x18;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    sprite.flags = 0;
    sprite.corners[0] = &corners[0];
    sprite.corners[1] = &corners[1];
    sprite.corners[2] = &corners[2];
    sprite.corners[3] = &corners[3];
    sprite.color = 0x808080;
    /* The match depends on the u bytes before the v bytes, each pair set in one chain. */
    UV_BYTES(sprite.uv0)[0] = UV_BYTES(sprite.uv2)[0] = 0x20;
    UV_BYTES(sprite.uv1)[0] = UV_BYTES(sprite.uv3)[0] = 0x3F;
    UV_BYTES(sprite.uv0)[1] = UV_BYTES(sprite.uv1)[1] = 0xD0;
    UV_BYTES(sprite.uv2)[1] = UV_BYTES(sprite.uv3)[1] = 0xDF;
    /* The match depends on reading vramY unsigned (lhu, then sra in getTPage): the field is a u16 row. */
    vramY = (u16)this->vramY;
    sprite.clut = getClut(0x180, vramY + 0xF4);
    sprite.tpage = getTPage(0, 1, 0x180, vramY);
    packet = buildPolyFT4(&sprite, packet, ot, zShift);
    corners[0].vx = -0x18;
    corners[0].vy = 0;
    corners[1].vx = -0x60;
    corners[1].vy = -0x10;
    corners[2].vx = -0x60;
    corners[2].vy = 0x10;
    corners[3].vy = 0x10;
    trail.corners[0] = &corners[0];
    trail.corners[1] = &corners[1];
    trail.corners[2] = &corners[2];
    trail.corners[3] = &corners[3];
    trail.colors[0] = 0x3C80;
    trail.colors[1] = 0x80;
    trail.colors[2] = 0x80;
    trail.colors[3] = 0;
    trail.flags = 9;
    corners[3].vx = ((this->frames & 3) << 4) - 0x1E0;
    PRIM_BUFFER_FREE = buildPolyG4(&trail, packet, ot, zShift);
}

/* Constructs an Unk800ACA60 (vtables D_8006B37C and D_8006B35C) of owner at pos, moving by vel: its parts
 * go in one of arg4's two lists, by Fighter_getSide of the owner. */
Unk800ACA60 *func_800ACA60(Unk800ACA60 *this, Fighter *owner, VECTOR *pos, VECTOR *vel, Fight *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B35C;
    this->parts.entity.vtable = &D_8006B37C;
    CollisionSphere_init(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    Hit_setKnockback(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->frames = 0;
    this->vramY = Fighter_getVramY(owner);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(this->owner) == 0) {
        Fight_addSide0Shape(arg4, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(arg4, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B37C and D_8006B35C, which
 * func_800ACA60 installs. */
void func_800ACBC4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B35C;
    this->entity.vtable = &D_8006B37C;
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

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of ParticleBurst_init's at
 * position, then deletes itself. */
void func_800ACC64(Unk800ACA60 *this) {
    Entity *child;
    s32 vx;

    Fighter_playSound(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->position, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading velocity.vx before allocating the second entity. */
    vx = this->velocity.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->position, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800ACD74(Unk800ACA60 *this, void *arg) {
    Hit_apply(&this->parts.base, arg);
    func_800ACC64(this);
}

/* The tick of Unk800ACA60 (D_8006B37C): moves position by velocity, puts the part there, and after 12 frames
 * calls func_800ACC64. */
void func_800ACDA4(Unk800ACA60 *this, s32 arg) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    this->frames++;
    if (this->frames >= 12) {
        func_800ACC64(this);
    }
}

/* Entry 0x14 of vtable D_8006B37C: draws the part at position, turned along its velocity (ratan2): a 0x60 square
 * textured from the owner's VRAM rows (vramY at 0xAC), then a gouraud quad (blue to black) behind it whose tip
 * flickers with frames. */
void func_800ACE1C(Unk800ACA60 *this, Camera *camera) {
    VECTOR pos;
    MATRIX m;
    SVECTOR corners[4];
    PolyFT4Shape sprite;
    PolyG4Shape trail;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 vramY;

    pos = *(VECTOR *)&this->position;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->velocity.vy, this->velocity.vx));
    matrixSetTranslationVecInline(&m, &pos);
    setModelMatrix(&m);
    corners[0].vx = corners[2].vx = -0x30;
    corners[1].vx = corners[3].vx = 0x30;
    corners[0].vy = corners[1].vy = -0x30;
    corners[2].vy = corners[3].vy = 0x30;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    sprite.flags = 0;
    sprite.corners[0] = &corners[0];
    sprite.corners[1] = &corners[1];
    sprite.corners[2] = &corners[2];
    sprite.corners[3] = &corners[3];
    sprite.color = 0x808080;
    /* The match depends on the u bytes before the v bytes, each pair set in one chain. */
    UV_BYTES(sprite.uv0)[0] = UV_BYTES(sprite.uv2)[0] = 0;
    UV_BYTES(sprite.uv1)[0] = UV_BYTES(sprite.uv3)[0] = 0x1F;
    UV_BYTES(sprite.uv0)[1] = UV_BYTES(sprite.uv1)[1] = 0xC0;
    UV_BYTES(sprite.uv2)[1] = UV_BYTES(sprite.uv3)[1] = 0xDF;
    /* The match depends on reading vramY unsigned (lhu, then sra in getTPage): the field is a u16 row. */
    vramY = (u16)this->vramY;
    sprite.clut = getClut(0x180, vramY + 0xF3);
    sprite.tpage = getTPage(0, 1, 0x180, vramY);
    packet = buildPolyFT4(&sprite, packet, ot, zShift);
    corners[0].vx = -0x18;
    corners[0].vy = 0;
    corners[1].vx = -0x60;
    corners[1].vy = -0x10;
    corners[2].vx = -0x60;
    corners[2].vy = 0x10;
    corners[3].vy = 0x10;
    trail.corners[0] = &corners[0];
    trail.corners[1] = &corners[1];
    trail.corners[2] = &corners[2];
    trail.corners[3] = &corners[3];
    trail.colors[0] = 0x803C00;
    trail.colors[1] = 0x800000;
    trail.colors[2] = 0x800000;
    trail.colors[3] = 0;
    trail.flags = 9;
    corners[3].vx = ((this->frames & 3) << 4) - 0x1E0;
    PRIM_BUFFER_FREE = buildPolyG4(&trail, packet, ot, zShift);
}

/* Constructs Unk800AD050 for fighter at position, moving by velocity, keeps where the character is, and adds the
 * part at 0x38 to one of list's two lists (Fighter_getSide picks which). */
Unk800AD050 *func_800AD050(Unk800AD050 *this, Fighter *fighter, Vec3 *position, VECTOR *velocity,
                           Fight *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, 0, (Vec3 *)velocity, 0);
    this->parts.base.vtable = D_8006B3A4;
    this->parts.entity.vtable = &D_8006B3C4;
    CollisionSphere_init(&this->parts.part, position, 0x40, &this->parts.base);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    Hit_setKnockback(&this->parts.base, &v);
    this->owner = fighter;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    Fighter_getPosition(fighter, &this->fighterPosition);
    this->acceleration.vx = 0;
    this->acceleration.vy = 10;
    this->acceleration.vz = 0;
    this->frames = 0;
    this->vramY = Fighter_getVramY(fighter);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(this->owner) == 0) {
        Fight_addSide0Shape(list, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(list, &this->parts.part.link);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B3C4 and D_8006B3A4, which
 * func_800AD050 installs. */
void func_800AD1D0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3A4;
    this->entity.vtable = &D_8006B3C4;
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

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of ParticleBurst_init's at
 * position, then deletes itself (the same as func_800ACC64, for Unk800AD050). */
void func_800AD270(Unk800AD050 *this) {
    Entity *child;
    s32 vx;

    Fighter_playSound(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->list, &this->position, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading velocity.vx before allocating the second entity. */
    vx = this->velocity.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->position, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AD380(Unk800AD050 *this, void *arg) {
    Hit_apply(&this->parts.base, arg);
    func_800AD270(this);
}

/* The tick of Unk800AD050: falls (velocity += acceleration), moves, puts the part there, and bursts
 * (func_800AD270) once it is below the height its owner had when it was made. */
void func_800AD3B0(Unk800AD050 *this) {
    s32 vx, vy, vz;
    s32 x, y, z;

    /* The match depends on reading both vectors whole before any store (in place: 30 diffs). */
    vx = this->velocity.vx;
    vy = this->velocity.vy;
    vz = this->velocity.vz;
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    vx += this->acceleration.vx;
    vy += this->acceleration.vy;
    vz += this->acceleration.vz;
    x += vx;
    y += vy;
    z += vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    if (this->fighterPosition.vy < y) {
        func_800AD270(this);
    }
    this->frames++;
}

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B2F0);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AD450);

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AD5BC);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B334 and D_8006B314, which
 * func_800AD5BC installs. */
void func_800AD6F8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B314;
    this->entity.vtable = &D_8006B334;
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

/* The second base's Hit_apply with arg, then sets bit 0 of 0x44. */
void func_800AD798(Unk800AD5BC *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    this->unk44 |= 1;
}

/* func_800AD5BC's class with its own fields (Unk800AD5BC is the part it shares with GuilmonSpecial2Hitbox_init's). */
typedef struct Unk800AD7CC {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Fighter *owner;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ s32 frames;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s16 vramY; /* the owner's 0x440 (Fighter_getVramY) */
    /* 0xAE */ s16 unkAE;
} Unk800AD7CC;

/* The tick of func_800AD5BC's entity: puts its part at the position, and deletes itself after 8 frames.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0xA4. */
void func_800AD7CC(Unk800AD7CC *this) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    if (this->frames >= 8) {
        entityDelete(&this->parts.entity);
    } else {
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/saintgargomon", func_800AD848);

INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B314);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B334);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B35C);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B37C);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B3A4);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B3C4);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B3EC);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B40C);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B434);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", D_8006B454);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", SAINTGARGOMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", SAINTGARGOMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/saintgargomon", SAINTGARGOMON_VTABLE);
