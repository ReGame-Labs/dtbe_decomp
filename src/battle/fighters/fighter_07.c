/* One of the 24 fighters: its constructor, func_80092A68, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"

struct Unk80093AD8 *func_80093AD8(struct Unk80093AD8 *this, Unk80076F34 *chara, Unk8006ED20 *obj);
struct Unk80093460 *func_80093460(struct Unk80093460 *this, Unk80076F34 *chara, Unk8006ED20 *obj, Vec3 *position,
                                  Vec3 *velocity);
extern EntityVtable D_8006972C;
extern EntityVtable D_80069754;
extern EntityVtable D_8006977C;
extern EntityVtable D_800697A4;
extern EntityVtable D_8006983C;

/* The entity classes func_80093AD8 and func_800A8858 construct (virtual
 * tables D_8006977C and D_8006AEDC) for a character. */
typedef struct Unk80093AD8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ VECTOR position; /* the character's point 3 moved by D_80069700 (func_8007A4A4) */
    /* 0x2C */ VECTOR velocity;
    /* 0x3C */ s32 state;
    /* 0x40 */ s32 distance; /* the sum of |velocity.vx| */
    /* 0x44 */ s32 counter;  /* frames in the state */
    /* 0x48 */ s32 unk48;    /* cycles 0-2, then 6-11, then counts from 3 to 6 */
    /* 0x4C */ s32 unk4C;    /* grows by 15 up to 60, then by 30 */
    /* 0x50 */ s32 unk50;    /* counts the updates */
    /* 0x54 */ s16 unk54; /* the character's 0x440 (func_8007768C) */
} Unk80093AD8;
void func_80092F58(Unk80076F34 *this);
/* constructs an entity (on func_800C2F14's) for chara at position with velocity */
Entity *func_80093274(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity);

/* The character class of virtual table D_8006983C (constructor func_80092A68). */
typedef struct Unk80092A68 {
    /* 0x000 */ Unk80076F34 base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8; /* func_80093144 sets it */
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ Vec3 unk4B0; /* func_80093150 measures the distance from it */
} Unk80092A68;

extern Unk80079B24Entry D_800696DC[];

/* Constructs a character of the class of vtable D_8006983C (which derives from Unk80076F34) with the
 * table D_800696DC. */
Unk80092A68 *func_80092A68(Unk80092A68 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006983C;
    this->unk4A4 = 0;
    func_80079B24(&this->base, D_800696DC);
    return this;
}

/* The destructor of the entity class of vtable D_8006983C, which func_80092A68 installs. */
void func_80092AB0(Entity *this, s32 flags) {
    this->vtable = &D_8006983C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_80093144(Unk80092A68 *this);

/* A method of the class of virtual table D_8006983C: the base step (func_80078910), then unk4A4 counts
 * down; it is set to 30 while the partner's func_80079E2C is 15, or while it runs and this one's is 5 or 6. */
void func_80092B04(Unk80092A68 *this, void *arg) {
    s32 state;

    func_80078910(this, arg);
    if (this->unk4A4 != 0) {
        this->unk4A4--;
    }
    if (func_80079E2C(func_8007AEEC(&this->base)) == 15) {
        this->unk4A4 = 30;
    }
    if (this->unk4A4 != 0) {
        state = func_80079E2C(&this->base);
        /* The match depends on the two tests being nested ifs: && folds them into one range test. */
        if (state < 7) {
            if (state >= 5) {
                this->unk4A4 = 30;
            }
        }
    }
}

/* A method of the class of virtual table D_8006983C: func_80093144, which sets its field 0x4A8. */
void func_80092B90(void *this) {
    func_80093144(this);
}

/* For arg 15, creates func_80093AD8's entity (0x58 bytes) for the character and hands it to
 * func_8007AD4C. */
void func_80092BB0(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj;
    Entity *child;

    if (arg == 15) {
        obj = func_80077580(this);
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_80093AD8(entityOperatorNewAt(0x58, child), this, obj);
        }
        func_8007AD4C(this, child);
    }
}

/* The override of Unk80076F34's method at 0xB0 for virtual table D_8006983C: func_8007B784,
 * then for arg 5 pushes vx at 0x3B4 by field 0x8 of func_80077698's entry 2 times the scale at 0x3C4,
 * along the facing. */
void func_80092C2C(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 5) {
        s32 push = ((s64)func_80077698(this)->entries[2].unk8[0] * this->unk3C4) >> 16;

        this->unk3B4.vx += push * func_80079E10(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80092CBC);

/* Spawns the entity of func_80093274 0x84 above the character (func_800775D8), with velocity
 * (24 forward, 0, 0). */
void func_80092DCC(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x84;
    velocity.vx = func_80079E10(this) * 24;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x58);
    if (child != NULL) {
        func_80093274(entityOperatorNewAt(0x58, child), this, &position, &velocity);
    }
    func_8007AD4C(this, child);
}

/* Overrides func_8007B9C8: calls it, plays sound 0xAC at step 0 and spawns func_80092F58's entity
 * at step 10. */
void func_80092E64(Unk80076F34 *this, s32 arg) {
    func_8007B9C8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xAC);
    }
    if (arg == 10) {
        func_80092F58(this);
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual table D_8006983C: func_8007BBE8; arg 0 plays
 * sounds 0xAC and 0xA6, arg 14 calls func_80092F58, and below 15 the position at 0x3B4 moves back by a
 * sixteenth of the x at 0x3A4 and by the y at 0x3A8 when that is positive. */
void func_80092EBC(Unk80076F34 *this, s32 arg) {
    func_8007BBE8(this, arg);
    if (arg == 0) {
        func_800784F4(this, 0xAC);
        func_80078540(this, 0xA6);
    }
    if (arg == 14) {
        func_80092F58(this);
    }
    if (arg < 15) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
        if (this->unk3A4.vy > 0) {
            this->unk3B4.vy -= this->unk3A4.vy;
        }
    }
}

/* Creates func_80093460's entity (0xD4 bytes) 0x84 above the character, moving 144 a frame toward
 * func_80079E10's side; hands it to func_8007AD4C and plays sound 0xA7. */
void func_80092F58(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Unk8006ED20 *obj;
    Entity *child;

    func_800775D8(this, &position);
    position.vy -= 0x84;
    velocity.vx = func_80079E10(this) * 144;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = func_80077580(this);
    child = allocEntityMemory(0xD4);
    if (child != NULL) {
        func_80093460(entityOperatorNewAt(0xD4, child), this, obj, &position, &velocity);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0xA7);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80093018);

/* Sets unk4A8 of a character of the class of virtual table D_8006983C. */
void func_80093144(Unk80092A68 *this) {
    this->unk4A8 = 1;
}

/* The distance (vecGetLength) from unk4B0 to the character's position. */
s32 func_80093150(Unk80092A68 *this) {
    Vec3 position;
    VECTOR delta;
    s32 x, y, z;

    func_800775D8(&this->base, &position);
    /* The match depends on the difference reading everything before writing (as vecSub). */
    x = position.vx;
    y = position.vy;
    z = position.vz;
    x -= this->unk4B0.vx;
    y -= this->unk4B0.vy;
    z -= this->unk4B0.vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    return vecGetLength(&delta);
}

Entity *func_8009401C(void *this, Unk80076F34 *chara, Vec3 *position, s32 facing, Unk8006ED20 *obj);

/* Spawns func_8009401C's entity (0x54 bytes) 384 in front of and 16 above the character, with its facing. */
void func_800931B0(Unk80092A68 *this) {
    Vec3 position;
    Unk8006ED20 *obj;
    Entity *child;

    func_800775D8(&this->base, &position);
    position.vy -= 16;
    position.vx += func_80079E10(&this->base) * 384;
    obj = func_80077580(&this->base);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_8009401C(entityOperatorNewAt(0x54, child), &this->base, &position, func_80079E10(&this->base), obj);
    }
    func_8007AD4C(&this->base, child);
}

extern u16 D_800696EC[];
extern EntityVtable D_80069814;

/* Constructs an entity of D_80069814: func_800C2F14 with frame D_800696EC, size 0x33, color 0x646464 and the
 * character's 0x440 (func_8007768C) as flags. */
Entity *func_80093274(Entity *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity) {
    func_800C2F14((Unk800C2F14 *)this, D_800696EC, (VECTOR *)position, (VECTOR *)velocity, 0x33, 0x646464,
                  func_8007768C(chara));
    this->vtable = &D_80069814;
    return this;
}

/* The update of D_80069814's entity (over func_800C3134): the gray color fades with unk54 (200 - 8 per step)
 * and the sprite grows by 10, then the base update. */
void func_800932F4(Unk800C2F14 *this, s32 arg) {
    s32 gray = 200 - (this->unk54 << 3);

    this->sprite.color = gray | (gray << 8) | (gray << 16);
    this->sprite.size += 10;
    func_800C3134(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80093340);

/* The class func_80093460 constructs (0xD4 bytes, virtual tables D_800697EC, and D_800697CC for the base at
 * 0x18), derived from Unk800C7024: while bit 0 of unkB8 is set it sheds func_800938A4's sprites. */
typedef struct Unk80093460 {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ Unk80076F34 *chara;
    /* 0xC4 */ Unk8006ED20 *obj;
    /* 0xC8 */ s32 unkC8; /* counts 0 to 5 in func_80093514 */
    /* 0xCC */ u32 unkCC; /* a color, 0x707070 */
    /* 0xD0 */ u32 unkD0; /* a color, 0x303030 */
} Unk80093460;

extern EntityVtable D_800697EC;
extern VtableEntry D_800697CC[];

/* Constructs func_800C7024's entity with 0x60, 0, 0x800, 3, then the virtual tables D_800697EC (and D_800697CC
 * for the base at 0x18), chara, obj and the two colors; func_8007BDC8 with 0x5A81800 and 2. */
Unk80093460 *func_80093460(Unk80093460 *this, Unk80076F34 *chara, Unk8006ED20 *obj, Vec3 *position,
                           Vec3 *velocity) {
    func_800C7024(&this->base, chara, position, velocity, 0x60, 0, 0x800, 3);
    this->base.parts.base.vtable = D_800697CC;
    this->base.parts.entity.vtable = &D_800697EC;
    this->chara = chara;
    this->obj = obj;
    this->unkC8 = 0;
    this->unkCC = 0x707070;
    this->unkD0 = 0x303030;
    func_8007BDC8(&this->base.parts.base, 0x5A81800, 2);
    return this;
}
Entity *func_800938A4(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel);

/* Unk80093460's update: while bit 0 of unkB8 is set, spawns func_800938A4's sprite at pos as a sibling, moving
 * back at a sixteenth of vel.vx and up or down by a random -8..7; unkC8 counts 0 to 5; then the base update. */
void func_80093514(Unk80093460 *this, s32 arg) {
    VECTOR velocity;
    Entity *child;

    if (this->base.unkB8 & 1) {
        velocity.vy = (mersenneTwisterGenerate(&this->obj->random) & 0xF) - 8;
        velocity.vz = 0;
        velocity.vx = -this->base.vel.vx >> 4;
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_800938A4(entityOperatorNewAt(0x58, child), (s32)this->chara, (VECTOR *)&this->base.pos, &velocity);
        }
        entityAddSibling(&this->base.parts.entity, child);
    }
    this->unkC8++;
    if (this->unkC8 >= 6) {
        this->unkC8 = 0;
    }
    func_800C71DC(&this->base, arg);
}

/* The second base's first method of Unk80093460: func_8007BDF0 with sum, func_800C73A8 with 0, then deletes
 * itself. */
void func_800935E8(Unk80093460 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->base.parts.base, sum);
    func_800C73A8(&this->base, 0);
    entityDelete(&this->base.parts.entity);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80093640);

/* Constructs an entity of D_800697A4: func_800C2FBC with entry 9 of D_8006D5C4, 0x19 and 0x202080. */
Entity *func_800938A4(Entity *this, s32 arg1, VECTOR *pos, VECTOR *vel) {
    func_800C2FBC((Unk800C2F14 *)this, 9, pos, vel, 0x19, 0x202080, 0);
    this->vtable = &D_800697A4;
    return this;
}

/* The update of D_800697A4's entity: nine times moves it by its velocity and fades its red (128 - 16 a step, green
 * and blue a quarter of it), then deletes it.
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads unk54. */
void func_800938F4(Unk800C2F14 *this, s32 arg) {
    if (this->unk54 >= 9) {
        entityDelete(&this->entity);
    } else {
        /* The match depends on reading the three words before storing any, as vecAddInPlace does, and on counting
         * the step last. */
        s32 x = this->sprite.pos.vx;
        s32 y = this->sprite.pos.vy;
        s32 z = this->sprite.pos.vz;
        s32 red;

        x += this->unk44.vx;
        y += this->unk44.vy;
        z += this->unk44.vz;
        this->sprite.pos.vx = x;
        this->sprite.pos.vy = y;
        this->sprite.pos.vz = z;
        red = 0x80 - this->unk54 * 16;
        this->sprite.color = red | ((red >> 2) << 8) | ((red >> 2) << 16);
        this->unk54++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_800939B0);

/* Constructs an Unk80093AD8 for the character, its fields 0. obj is not read (func_80092BB0 passes it). */
Unk80093AD8 *func_80093AD8(Unk80093AD8 *this, Unk80076F34 *chara, Unk8006ED20 *obj) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006977C;
    this->chara = chara;
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
    this->unk48 = 0;
    this->unk4C = 0;
    this->distance = 0;
    this->unk50 = 0;
    this->unk54 = func_8007768C(chara);
    return this;
}

/* The destructor of the entity class of vtable D_8006977C, which func_80093AD8 installs. */
void func_80093B60(Entity *this, s32 flags) {
    this->vtable = &D_8006977C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_8006968C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_80069694);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_8006969C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696BC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800696EC);
extern VECTOR D_80069700;

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_80069700);

/* The update of Unk80093AD8: grows at the character's point 3 for 19 frames, flies forward at 48 until it has
 * gone three quarters of the way to the partner (func_8007A69C), slows down for 3 frames, then fades and deletes
 * itself; every frame but the last moves it by its velocity. */
void func_80093BB4(Unk80093AD8 *this) {
    Vec3 partner;
    s32 dx;
    s32 x, y, z;

    switch (this->state) {
    case 0:
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 1:
        this->unk48++;
        if (this->unk48 >= 3) {
            this->unk48 = 0;
        }
        if (this->unk4C < 60) {
            this->unk4C += 15;
        }
        func_8007A4A4(this->chara, &this->position, 3, &D_80069700);
        this->position.vy -= 0x33;
        this->counter++;
        if (this->counter < 19) {
            break;
        }
        this->velocity.vx = func_80079E10(this->chara) * 48;
        this->unk48 = 6;
        this->state++;
        /* fallthrough */
    case 2:
        this->unk48++;
        if (this->unk48 >= 12) {
            this->unk48 = 6;
        }
        func_8007A69C(this->chara, &partner);
        dx = partner.vx - this->position.vx;
        /* The match depends on the absolute value as a conditional expression (an if negates it in a copy). */
        dx = dx < 0 ? -dx : dx;
        if (this->distance < dx - (dx >> 2)) {
            break;
        }
        this->counter = 0;
        this->state++;
        /* fallthrough */
    case 3:
        this->unk48++;
        if (this->unk48 >= 12) {
            this->unk48 = 6;
        }
        this->velocity.vx >>= 2;
        this->counter++;
        if (this->counter < 3) {
            break;
        }
        this->counter = 0;
        this->unk48 = 3;
        this->state++;
        break;
    case 4:
        this->velocity.vx >>= 2;
        this->unk4C += 30;
        this->unk48++;
        if (this->unk48 < 6) {
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
    this->unk50++;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80093E30);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_8009401C);

/* The destructor of the entity class of vtable D_80069754, which func_8009401C installs. */
void func_800940DC(Entity *this, s32 flags) {
    this->vtable = &D_80069754;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80094130);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80094258);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_800942D4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_800945F8);

/* The destructor of the UnkPartsEntity3C class of vtable D_8006972C, which func_800945F8 installs. */
void func_800947EC(UnkPartsEntity3C *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006972C;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->unk18.base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80094880);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_07", func_80094AF8);

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80094DC8(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_80094E1C(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
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

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_80094EBC(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA_ALIGN8("game/nonmatchings/fighters/fighter_07", D_8006972C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_80069754);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_8006977C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800697A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800697CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_800697EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_80069814);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_07", D_8006983C);
