/* One of the 24 fighters: its constructor, func_800AA390, which the factory func_80076CB4 calls, and its
 * classes. */

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
    /* 0x18 */ Unk8007BD8C unk18;
} Unk800ACD74;

/* the end of an Unk800AC470: spawns two entities and deletes it */
void func_800AC674(Unk800AC470 *this);
Entity *func_800AD5BC(Entity *this, Unk80076F34 *arg1, Unk8006ED20 *arg2, Vec3 *pos, s32 arg4, s32 arg5);
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
extern VtableEntry D_8006B47C[];
extern EntityVtable D_8006B49C;
extern EntityVtable D_8006B4C4;

typedef struct Unk800AA390 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ u32 unk4A8; /* a sound's handle (func_800AACC0) */
} Unk800AA390;
void func_800AB830(UnkPartsEntity38 *this);

/* Constructs an entity of the class of virtual table D_8006B4C4, derived from func_80076F34's. */
Unk800AA390 *func_800AA390(Unk800AA390 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006B4C4;
    this->unk4A4 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006B4C4, which func_800AA390 installs. */
void func_800AA3C8(Entity *this, s32 flags) {
    this->vtable = &D_8006B4C4;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* func_8007AE34 on the character func_8007AEEC gives for this one. */
void func_800AA41C(Unk80076F34 *this) {
    func_8007AE34(func_8007AEEC(this));
}

/* Unk80076F34's method at 0xD4 for virtual table D_8006B4C4: func_8007B978; under arg 24 func_8007835C(0) and the
 * partner (func_8007AEEC) takes the velocities at 0x3A4 and 0x3B4; arg 0 starts the partner's clip 13; arg 24, when
 * the partner is within 0x200 by 0x80, throws it (func_80078EF8) and spawns func_800C32E0's entity at node 9,
 * else stops it (func_80079320). */
void func_800AA444(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Vec3 partnerPosition;
    Vec3 velocity;
    Vec3 node;
    VECTOR offset;
    Unk80076F34 *partner;
    Entity *child;
    s32 dx, dy;

    func_8007B978(this, arg);
    partner = func_8007AEEC(this);
    if (arg < 24) {
        s32 x, y, z;

        func_8007835C(this, 0);
        /* The match depends on reading each vector's three words before storing any, as vecCopy does. */
        x = this->unk3A4.vx;
        y = this->unk3A4.vy;
        z = this->unk3A4.vz;
        partner->unk3A4.vx = x;
        partner->unk3A4.vy = y;
        partner->unk3A4.vz = z;
        x = this->unk3B4.vx;
        y = this->unk3B4.vy;
        z = this->unk3B4.vz;
        partner->unk3B4.vx = x;
        partner->unk3B4.vy = y;
        partner->unk3B4.vz = z;
    }
    switch (arg) {
    case 0:
        func_80079274(partner, 24);
        func_800796F0(partner, 13, 0, 0);
        break;
    case 24:
        func_800775D8(this, &position);
        func_8007A69C(this, &partnerPosition);
        dx = ABS(position.vx - partnerPosition.vx);
        dy = ABS(position.vy - partnerPosition.vy);
        if (dx < 0x200 && dy < 0x80) {
            velocity.vx = (s16)func_80079E10(this) * 18200;
            velocity.vy = -0x4718;
            velocity.vz = 0;
            func_80078EF8(partner, 0xC38C3400, &velocity, 0);
            memset(&offset, 0, sizeof(offset));
            func_8007A4A4(this, (VECTOR *)&node, 9, &offset);
            child = allocEntityMemory(0x48);
            if (child != NULL) {
                func_800C32E0(entityOperatorNewAt(0x48, child), &node, &D_80010814, 0xC0, 0x808080);
            }
            func_8007AD4C(this, child);
        } else {
            func_80079274(partner, 0);
            func_80079320(this);
        }
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AA654);

void func_800AAB4C(Unk80076F34 *this, s32 arg);

/* The override of Unk80076F34's method at 0xD8 for virtual table D_8006B4C4: func_8007B9C8; arg 0 plays
 * sound 0xDF, arg 7 calls func_800AAB4C with -21 and plays sound 0xDB, and below 8 the x at 0x3B4 moves by
 * a sixteenth of the x at 0x3A4, backwards. */
void func_800AA9F0(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    func_80079AE4(this);
    if (arg == 0) {
        func_800784F4(this, 0xDF);
    }
    if (arg == 7) {
        func_800AAB4C(this, -21);
        func_80078540(this, 0xDB);
    }
    if (arg < 8) {
        this->unk3B4.vx += -this->unk3A4.vx >> 4;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006B4C4: func_8007BBE8 and
 * func_80079AE4; arg 0 plays sounds 0xDF and 0xDB, arg 7 calls func_800AAB4C with 0 and pushes vx at 0x3B4
 * back by 0x3000; below 8 the vector at 0x3B4 slows by the one at 0x3A4 (an eighth of x, all of a positive
 * y). */
void func_800AAA84(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    func_80079AE4(this);
    if (arg == 0) {
        func_800784F4(this, 0xDF);
        func_80078540(this, 0xDB);
    }
    if (arg == 7) {
        func_800AAB4C(this, 0);
        this->unk3B4.vx -= func_80079E10(this) * 0x3000;
    }
    if (arg < 8) {
        this->unk3B4.vx -= this->unk3A4.vx >> 3;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B1F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B204);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AAB4C);

void func_800AADB0(Unk800AA390 *this, s32 arg);
void func_800AB09C(Unk800AA390 *this, s32 step);
void func_800AAF30(Unk800AA390 *this, s32 step);
void func_800AB4EC(Unk800AA390 *this, s32 arg);

/* The override of Unk80076F34's method at 0xE8 for virtual table D_8006B4C4: func_8007BA30 and
 * func_80079AE4; arg 0 calls func_80077448(-0x10000), plays sound 0xE1 and sets 0x498 to 10; arg 0x13 plays
 * sound 0xDB and keeps its handle at 0x4A8, which arg 0x47 stops; from 0x13 to 0x46 it runs func_800AB09C
 * (after func_800AADB0 on odd args), and every eighth arg func_800AAF30 and func_800AB4EC with arg / 8. */
void func_800AACC0(Unk800AA390 *this, s32 arg) {
    func_8007BA30(&this->base, arg);
    func_80079AE4(&this->base);
    if (arg == 0) {
        func_80077448(&this->base, -0x10000);
        func_800784F4(&this->base, 0xE1);
        this->base.unk498 = 10;
    }
    if (arg == 0x13) {
        this->unk4A8 = func_80078540(&this->base, 0xDB);
    }
    if (arg == 0x47 && this->unk4A8 != 0) {
        stopSound(this->unk4A8);
    }
    if (arg >= 0x13 && arg < 0x47) {
        if (arg & 1) {
            func_800AADB0(this, arg);
        }
        func_800AB09C(this, arg - 0x13);
        if ((arg & 7) == 0) {
            func_800AAF30(this, arg - 0x13);
            func_800AB4EC(this, arg >> 3);
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AADB0);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B214);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AAF30);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B224);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AB09C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AB4EC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AB630);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B49C and D_8006B47C, which
 * func_800AB630 installs. */
void func_800AB790(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B47C;
    this->entity.vtable = &D_8006B49C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AB830);

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AB9F4(UnkPartsEntity38 *this, void *arg) {
    func_8007BDF0(&this->base, arg);
    func_800AB830(this);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800ABA24);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800ABB54);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B2B4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800ABD08);

/* The class func_800ABE6C constructs (virtual tables D_8006B454, and D_8006B434 for its base at 0x18). */
typedef struct Unk800ABE6C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8007BD8C unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ Unk80075EE4 unk38;
    /* 0x80 */ Unk80076F34 *chara;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0x94 - 0x90];
    /* 0x94 */ Vec3 unk94; /* where func_800AC138 puts it: position moved by the velocity */
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ Vec3 velocity;
    /* 0xB0 */ u8 unkB0[0xB4 - 0xB0];
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s16 facing; /* the character's (func_80079E10) */
    /* 0xBA */ s16 vramY;  /* the character's 0x440 (func_8007768C) */
    /* 0xBC */ u16 unkBC; /* 1 once func_800AC080 made its burst */
} Unk800ABE6C;

/* Constructs Unk800ABE6C for chara at position, moving by velocity, and adds the part at 0x38 to one of list's two
 * lists (func_80077574 picks which). */
Unk800ABE6C *func_800ABE6C(Unk800ABE6C *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, chara, 0x7B00100, (Vec3 *)velocity, 0);
    this->unk18.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
    func_80075EE4(&this->unk38, position, 0x40, &this->unk18);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    func_8007BDD4(&this->unk18, &v);
    this->chara = chara;
    this->facing = func_80079E10(chara);
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
    this->unkB4 = 0;
    this->unkBC = 0;
    this->vramY = func_8007768C(chara);
    this->unk38.unk14 = &this->entity;
    if (func_80077574(this->chara) == 0) {
        func_8006F044(list, &this->unk38.node);
    } else {
        func_8006F024(list, &this->unk38.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B454 and D_8006B434, which
 * func_800ABE6C installs. */
void func_800ABFE0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B434;
    this->entity.vtable = &D_8006B454;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The second base's first method for func_800ABE6C's class: func_8007BDF0 with sum; the first time (0xBC
 * still 0), adds a burst of 2 (func_800C62EC) at 0x94 as a sibling and sets bit 0 of the part's flags. */
void func_800AC080(Unk800ABE6C *this, void *sum) {
    Entity *child;
    s32 radius;

    func_8007BDF0(&this->unk18, sum);
    if (this->unkBC == 0) {
        radius = this->velocity.vx >> 2;
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->unk94, radius, 0x60, 2, 0x808080);
        }
        entityAddSibling(&this->entity, child);
        this->unkBC = 1;
        this->unk38.flags |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AC138);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B2D4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AC21C);

/* Constructs an Unk800AC470 (vtables D_8006B40C and D_8006B3EC): the same as func_800ACA60. */
Unk800AC470 *func_800AC470(Unk800AC470 *this, Unk80076F34 *owner, VECTOR *pos, VECTOR *vel, Unk8006ED20 *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B3EC;
    this->parts.entity.vtable = &D_8006B40C;
    func_80075EE4(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    func_8007BDD4(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->frames = 0;
    this->unkAC = func_8007768C(owner);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(arg4, &this->parts.part.node);
    } else {
        func_8006F024(arg4, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B40C and D_8006B3EC, which
 * func_800AC470 installs. */
void func_800AC5D4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3EC;
    this->entity.vtable = &D_8006B40C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Spawns an entity of func_800AD5BC's class and one of func_800C62EC's at pos, then deletes itself
 * (func_800ACC64 without the sound). */
void func_800AC674(Unk800AC470 *this) {
    Entity *child;
    s32 vx;

    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AC778(Unk800AC470 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800AC674(this);
}

/* The gather of Unk800AC470 (D_8006B40C): the same as func_800ACDA4, with func_800AC674. */
void func_800AC7A8(Unk800AC470 *this, s32 arg) {
    s32 x, y, z;

    x = this->pos.vx;
    y = this->pos.vy;
    z = this->pos.vz;
    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
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

    pos = *(VECTOR *)&this->pos;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->vel.vy, this->vel.vx));
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
    vramY = (u16)this->unkAC;
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
 * go in one of arg4's two lists, by func_80077574 of the owner. */
Unk800ACA60 *func_800ACA60(Unk800ACA60 *this, Unk80076F34 *owner, VECTOR *pos, VECTOR *vel, Unk8006ED20 *arg4) {
    VECTOR scaled;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, owner, 0, (Vec3 *)vel, 0);
    this->parts.base.vtable = D_8006B35C;
    this->parts.entity.vtable = &D_8006B37C;
    func_80075EE4(&this->parts.part, (Vec3 *)pos, 0x40, &this->parts.base);
    /* The match depends on copying vel whole, then shifting; and below, on separate locals for the copies
     * of pos and vel, frames being cleared after them. */
    scaled = *vel;
    scaled.vx <<= 5;
    scaled.vy <<= 5;
    scaled.vz <<= 5;
    func_8007BDD4(&this->parts.base, &scaled);
    this->owner = owner;
    this->unk84 = arg4;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = vel->vx;
    vy = vel->vy;
    vz = vel->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    this->frames = 0;
    this->unkAC = func_8007768C(owner);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(arg4, &this->parts.part.node);
    } else {
        func_8006F024(arg4, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B37C and D_8006B35C, which
 * func_800ACA60 installs. */
void func_800ACBC4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B35C;
    this->entity.vtable = &D_8006B37C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of func_800C62EC's at
 * pos, then deletes itself. */
void func_800ACC64(Unk800ACA60 *this) {
    Entity *child;
    s32 vx;

    func_80078540(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->unk84, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800ACD74(Unk800ACA60 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800ACC64(this);
}

/* The gather of Unk800ACA60 (D_8006B37C): moves pos by vel, puts the part there, and after 12 frames
 * calls func_800ACC64. */
void func_800ACDA4(Unk800ACA60 *this, s32 arg) {
    s32 x, y, z;

    x = this->pos.vx;
    y = this->pos.vy;
    z = this->pos.vz;
    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    this->frames++;
    if (this->frames >= 12) {
        func_800ACC64(this);
    }
}

/* Entry 0x14 of vtable D_8006B37C: draws the part at pos, turned along its velocity (ratan2): a 0x60 square
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

    pos = *(VECTOR *)&this->pos;
    packet = PRIM_BUFFER_FREE;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    matrixInitRotationZ(&m, ratan2(this->vel.vy, this->vel.vx));
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
    vramY = (u16)this->unkAC;
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

/* Constructs Unk800AD050 for chara at position, moving by velocity, keeps where the character is, and adds the
 * part at 0x38 to one of list's two lists (func_80077574 picks which). */
Unk800AD050 *func_800AD050(Unk800AD050 *this, Unk80076F34 *chara, Vec3 *position, VECTOR *velocity,
                           Unk8006ED20 *list) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, 0x30000);
    func_8007BD8C(&this->parts.base, chara, 0, (Vec3 *)velocity, 0);
    this->parts.base.vtable = D_8006B3A4;
    this->parts.entity.vtable = &D_8006B3C4;
    func_80075EE4(&this->parts.part, position, 0x40, &this->parts.base);
    v = *velocity;
    v.vx <<= 5;
    v.vy <<= 5;
    v.vz <<= 5;
    func_8007BDD4(&this->parts.base, &v);
    this->owner = chara;
    this->list = list;
    /* The match depends on reading each vector's three words before storing any, as vecCopy does, with
     * locals of its own for each vector */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->vel.vx = vx;
    this->vel.vy = vy;
    this->vel.vz = vz;
    func_800775D8(chara, &this->charaPosition);
    this->unkB8 = 0;
    this->unkBC = 10;
    this->unkC0 = 0;
    this->unkC8 = 0;
    this->unkCC = func_8007768C(chara);
    this->parts.part.unk14 = &this->parts.entity;
    if (func_80077574(this->owner) == 0) {
        func_8006F044(list, &this->parts.part.node);
    } else {
        func_8006F024(list, &this->parts.part.node);
    }
    return this;
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B3C4 and D_8006B3A4, which
 * func_800AD050 installs. */
void func_800AD1D0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B3A4;
    this->entity.vtable = &D_8006B3C4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0x4D of the owner, spawns an entity of func_800AD5BC's class and one of func_800C62EC's at
 * pos, then deletes itself (the same as func_800ACC64, for Unk800AD050). */
void func_800AD270(Unk800AD050 *this) {
    Entity *child;
    s32 vx;

    func_80078540(this->owner, 0x4D);
    child = allocEntityMemory(0xB0);
    if (child != NULL) {
        func_800AD5BC(entityOperatorNewAt(0xB0, child), this->owner, this->list, &this->pos, 0x280, 0x7B00500);
    }
    entityAddSibling(&this->parts.entity, child);
    /* The match depends on reading vel.vx before allocating the second entity. */
    vx = this->vel.vx;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 3, &this->pos, vx, 0x100, 3, 0x808080);
    }
    entityAddSibling(&this->parts.entity, child);
    entityDelete(&this->parts.entity);
}

/* Runs the second base (at 0x18) with arg, then the entity's own step. */
void func_800AD380(Unk800AD050 *this, void *arg) {
    func_8007BDF0(&this->parts.base, arg);
    func_800AD270(this);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AD3B0);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B2F0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AD450);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AD5BC);

/* The destructor of the UnkPartsEntity38 class of vtables D_8006B334 and D_8006B314, which
 * func_800AD5BC installs. */
void func_800AD6F8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006B314;
    this->entity.vtable = &D_8006B334;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The second base's func_8007BDF0 with arg, then sets bit 0 of 0x44. */
void func_800AD798(Unk800AD5BC *this, void *arg) {
    func_8007BDF0(&this->unk18, arg);
    this->unk44 |= 1;
}

/* func_800AD5BC's class with its own fields (Unk800AD5BC is the part it shares with func_800893BC's). */
typedef struct Unk800AD7CC {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk80076F34 *owner;
    /* 0x84 */ Vec3 position;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ s32 frames;
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s16 vramY; /* the owner's 0x440 (func_8007768C) */
    /* 0xAE */ s16 unkAE;
} Unk800AD7CC;

/* The update of func_800AD5BC's entity: puts its part at the position, and deletes itself after 8 frames.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0xA4. */
void func_800AD7CC(Unk800AD7CC *this) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    if (this->frames >= 8) {
        entityDelete(&this->parts.entity);
    } else {
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_14", func_800AD848);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B314);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B334);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B35C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B37C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B3A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B3C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B3EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B40C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B434);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B454);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B47C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B49C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_14", D_8006B4C4);
