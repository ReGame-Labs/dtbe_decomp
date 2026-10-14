/* Holydramon (CHARACTER_HOLYDRAMON): the fighter createFighter builds for it, its moves, and the entities they
 * spawn: the special move 1's hitbox and effect, the special move 2's hitboxes, and the special move 3's effects. */

#include "common.h"
#include "battle/game_classes.h"

struct HolydramonSpecial1Hitbox *HolydramonSpecial1Hitbox_init(struct HolydramonSpecial1Hitbox *this, Fighter *fighter,
                                                               Vec3 *position, VECTOR *velocity, Fight *fight);
struct HolydramonSpecial1Effect *HolydramonSpecial1Effect_init(struct HolydramonSpecial1Effect *this, Fighter *fighter);
extern EntityVtable D_80068D84;
extern EntityVtable D_80068DAC;
extern EntityVtable D_80068DD4;
extern EntityVtable D_80068DFC;
extern VtableEntry HOLYDRAMON_SPECIAL2_HITBOX_HIT_VTABLE[];
extern EntityVtable HOLYDRAMON_SPECIAL2_HITBOX_VTABLE;
extern EntityVtable HOLYDRAMON_SPECIAL1_EFFECT_VTABLE;
extern VtableEntry HOLYDRAMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable HOLYDRAMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable HOLYDRAMON_VTABLE;

typedef struct Holydramon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s16 unk4A4;
    /* 0x4A6 */ s16 unk4A6;
    /* 0x4A8 */ s16 unk4A8;
} Holydramon;
void Holydramon_spawnSpecial1Hitbox(struct Holydramon *this);
void Holydramon_spawnSpecial1Effect(Fighter *this);
/* an offset from a node of the character's model */
extern VECTOR D_80010824; /* a zero offset in the executable's .rodata */

/* Builds Holydramon: a Fighter with its own vtable. */
Holydramon *Holydramon_init(Holydramon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &HOLYDRAMON_VTABLE;
    this->unk4A4 = 0;
    this->unk4A6 = 0;
    this->unk4A8 = 0;
    return this;
}

/* Holydramon's destructor. */
void Holydramon_destroy(Entity *this, s32 flags) {
    this->vtable = &HOLYDRAMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", Holydramon_throw);

/* Special move 1: the voice and its effect at frame 1, the hitbox and the shot sound at frame 6; until frame 7 the
 * force brakes a sixteenth of the horizontal velocity. */
void Holydramon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Fighter_playVoice(this, 0x92);
        Holydramon_spawnSpecial1Effect(this);
    }
    if (frame == 6) {
        Holydramon_spawnSpecial1Hitbox((Holydramon *)this);
        Fighter_playSound(this, 0x8D);
    }
    if (frame < 7) {
        this->force.vx += -this->velocity.vx >> 4;
    }
}

/* Special move 1 in the air: as on the ground, with a push backward at frame 6, and until frame 7 braking a fall too.
 */
void Holydramon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    func_80079AE4(this);
    if (frame == 1) {
        Fighter_playVoice(this, 0x92);
        Holydramon_spawnSpecial1Effect(this);
    }
    if (frame == 6) {
        Holydramon_spawnSpecial1Hitbox((Holydramon *)this);
        Fighter_playSound(this, 0x8D);
        this->force.vx -= Fighter_getFacing(this) * 0x3000;
    }
    if (frame < 7) {
        this->force.vx -= this->velocity.vx >> 4;
        if (this->velocity.vy > 0) {
            this->force.vy -= this->velocity.vy;
        }
    }
}

/* Fires the special move 1's hitbox from the fighter's node 12, 96 a frame forward (the facing kept at 0x4A4), and
 * calls func_80079B0C(30). */
void Holydramon_spawnSpecial1Hitbox(Holydramon *this) {
    Vec3 position;
    VECTOR velocity;
    Fight *obj;
    Entity *child;

    Fighter_getNodePosition(&this->base, (VECTOR *)&position, 12, &D_80010824);
    this->unk4A4 = Fighter_getFacing(&this->base);
    velocity.vx = this->unk4A4 * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    obj = Fighter_getScene(&this->base);
    child = allocEntityMemory(0x2CC);
    if (child != NULL) {
        HolydramonSpecial1Hitbox_init(entityOperatorNewAt(0x2CC, child), &this->base, &position, &velocity, obj);
    }
    Fighter_addEntity(&this->base, child);
    func_80079B0C(&this->base, 30);
}

/* Spawns the special move 1's effect. */
void Holydramon_spawnSpecial1Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        HolydramonSpecial1Effect_init(entityOperatorNewAt(0x48, child), this);
    }
    Fighter_addEntity(this, child);
}
void Holydramon_spawnSpecial2Hitbox(Holydramon *this, s32 angle);

/* Special move 2: at frame 0 clears 0x4A8, func_80079B14(60), the voice and the sound; on frames 3 to 25 a hitbox a
 * frame toward the facing, braking a sixteenth of the horizontal velocity. */
void Holydramon_special2(Holydramon *this, s32 frame) {
    Fighter_special2(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        Fighter_playVoice(&this->base, 0x93);
        Fighter_playSound(&this->base, 0x8E);
    }
    if (frame >= 3 && frame < 26) {
        if (Fighter_getFacing(&this->base) > 0) {
            Holydramon_spawnSpecial2Hitbox(this, 0);
        } else {
            Holydramon_spawnSpecial2Hitbox(this, 0x800);
        }
        this->base.force.vx += -this->base.velocity.vx >> 4;
    }
}

/* Special move 2 in the air: as on the ground, on frames 0 to 25, braking a fall too. */
void Holydramon_airSpecial2(Holydramon *this, s32 frame) {
    Fighter_airSpecial2(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        this->unk4A8 = 0;
        func_80079B14(&this->base, 60);
        Fighter_playVoice(&this->base, 0x93);
        Fighter_playSound(&this->base, 0x8E);
    }
    if (frame < 26) {
        if (Fighter_getFacing(&this->base) > 0) {
            Holydramon_spawnSpecial2Hitbox(this, 0);
        } else {
            Holydramon_spawnSpecial2Hitbox(this, 0x800);
        }
        this->base.force.vx -= this->base.velocity.vx >> 4;
        if (this->base.velocity.vy > 0) {
            this->base.force.vy -= this->base.velocity.vy;
        }
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068D64);

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", Holydramon_spawnSpecial2Hitbox);

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", Holydramon_special3);

struct Unk8008B62C *func_8008B62C(struct Unk8008B62C *this, Fighter *fighter, Vec3 *position);

/* Spawns a special3 effect (vtable D_80068DAC) at the opponent, up to 0x120 to either side at random. */
void func_8008A91C(Holydramon *this) {
    Fight *fight = Fighter_getScene(&this->base);
    Vec3 position;
    Entity *child;

    Fighter_getPosition(Fighter_getOpponent(&this->base), &position);
    position.vx += ((s32)(mersenneTwisterGenerate(&fight->random) & 0x3F) - 0x20) * 9;
    child = allocEntityMemory(0x38);
    if (child != NULL) {
        func_8008B62C(entityOperatorNewAt(0x38, child), &this->base, &position);
    }
    Fighter_addEntity(&this->base, child);
}

/* The special move 1's hitbox: it flies straight for 65 frames. */
typedef struct HolydramonSpecial1Hitbox {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Hit hit;
    /* 0x034 */ u8 unk34[0x38 - 0x34];
    /* 0x038 */ CollisionSphere sphere;
    /* 0x080 */ Fighter *fighter;
    /* 0x084 */ Vec3 position;
    /* 0x090 */ u8 unk90[0x2A4 - 0x90];
    /* 0x2A4 */ Vec3 velocity;
    /* 0x2B0 */ u8 unk2B0[0x2B4 - 0x2B0];
    /* 0x2B4 */ s32 unk2B4;
    /* 0x2B8 */ s32 unk2B8;
    /* 0x2BC */ s32 speed; /* the velocity's length */
    /* 0x2C0 */ s32 facing; /* the character's (Fighter_getFacing) */
    /* 0x2C4 */ s32 age;
    /* 0x2C8 */ s16 vramY; /* Fighter_getVramY of the character */
} HolydramonSpecial1Hitbox;

/* Builds the special move 1's hitbox for fighter at position, flying at velocity, its sphere in its side's list of
 * fight. */
HolydramonSpecial1Hitbox *HolydramonSpecial1Hitbox_init(HolydramonSpecial1Hitbox *this, Fighter *fighter,
                                                        Vec3 *position, VECTOR *velocity, Fight *fight) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->hit, fighter, 0x6A81800, (Vec3 *)velocity, 2);
    this->hit.vtable = HOLYDRAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &HOLYDRAMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&this->sphere, position, 0xC0, &this->hit);
    v = *velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    Hit_setKnockback(&this->hit, &v);
    this->fighter = fighter;
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
    this->unk2B4 = 0;
    this->unk2B8 = 0;
    this->speed = vecGetLength(velocity);
    this->facing = Fighter_getFacing(this->fighter);
    this->age = 0;
    this->vramY = Fighter_getVramY(fighter);
    this->sphere.entity = &this->entity;
    if (Fighter_getSide(this->fighter) == 0) {
        Fight_addSide0Shape(fight, &this->sphere.link);
    } else {
        Fight_addSide1Shape(fight, &this->sphere.link);
    }
    return this;
}

/* The special move 1 hitbox's destructor. */
void HolydramonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = HOLYDRAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &HOLYDRAMON_SPECIAL1_HITBOX_VTABLE;
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

/* g++'s new of the guard flash (vtable D_8006D4D4, 0x58 bytes) at owner's position: NULL when the allocation fails. */
static inline Entity *HolydramonSpecial1Hitbox_newFlash(HolydramonSpecial1Hitbox *owner) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        func_8006D4D4_init(entityOperatorNewAt(0x58, child), (VECTOR *)&owner->position);
    }
    return child;
}

/* g++'s new of ParticleBurst_init's burst (0x3C bytes, kind 4, five particles) at owner's position: NULL when the
 * allocation fails. */
static inline Entity *HolydramonSpecial1Hitbox_newBurst(HolydramonSpecial1Hitbox *owner) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 4, &owner->position, 0x20, 0x100, 5, 0x808080);
    }
    return child;
}

/* On a hit: Hit_apply; when it struck a fighter it disables its sphere and adds a flash when the fighter guards, or a
 * burst of 5 particles. */
void HolydramonSpecial1Hitbox_hit(HolydramonSpecial1Hitbox *this, CollisionSphere *sum) {
    Entity *target;

    Hit_apply(&this->hit, sum);
    target = sum->entity;
    if (target != NULL && entityGetKind(target) == ENTITY_KIND_FIGHTER) {
        this->sphere.flags |= COLLISION_SHAPE_DISABLED;
        /* The match depends on the two news as inlines taking the owner (a ternary of their results). */
        entityAddSibling(&this->entity, Fighter_isGuarding((Fighter *)target) ? HolydramonSpecial1Hitbox_newFlash(this)
                                                                             : HolydramonSpecial1Hitbox_newBurst(this));
    }
}

/* Flies by its velocity, the sphere with it, for 65 frames, then deletes itself. The match depends on the unit's
 * -fno-cse-follow-jumps (Makefile): the else block reloads 0x2C4. */
void HolydramonSpecial1Hitbox_tick(HolydramonSpecial1Hitbox *this) {
    if (this->age > 64) {
        entityDelete(&this->entity);
    } else {
        s32 x = this->position.vx;
        s32 y = this->position.vy;
        s32 z = this->position.vz;

        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
        this->sphere.position.vx = x;
        this->sphere.position.vy = y;
        this->sphere.position.vz = z;
        this->age++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", HolydramonSpecial1Hitbox_draw);

/* The special move 1's effect, at the fighter for 11 frames. */
typedef struct HolydramonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* the character's (Fighter_getPosition) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 age;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s16 vramY; /* Fighter_getVramY of the character */
} HolydramonSpecial1Effect;

/* Builds the special move 1's effect for fighter, at its position. */
HolydramonSpecial1Effect *HolydramonSpecial1Effect_init(HolydramonSpecial1Effect *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &HOLYDRAMON_SPECIAL1_EFFECT_VTABLE;
    this->fighter = fighter;
    Fighter_getPosition(fighter, &this->position);
    this->unk2C = 0x40;
    this->unk30 = 0x40;
    this->unk34 = 0x80;
    this->unk38 = 0;
    this->age = 0;
    this->unk40 = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The special move 1 effect's destructor. */
void HolydramonSpecial1Effect_destroy(Entity *this, s32 flags) {
    this->vtable = &HOLYDRAMON_SPECIAL1_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Deletes itself after 10 frames. The match depends on taking the Entity the virtual call passes and its class in a
 * local: a pointer param's REG_EQUIV doubles its live length (-dl: 20 insns, 10 for the local), so global-alloc would
 * give the count $v1 before `this`. */
void HolydramonSpecial1Effect_tick(Entity *entity) {
    HolydramonSpecial1Effect *this = (HolydramonSpecial1Effect *)entity;

    if (this->age < 10) {
        this->age++;
    } else {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068D74);

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", HolydramonSpecial1Effect_draw);

/* The entity class of virtual table D_80068DAC (constructor func_8008B62C, made by func_8008A91C). */
typedef struct Unk8008B62C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x30 - 0x28];
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s16 unk34; /* Fighter_getVramY of the character */
} Unk8008B62C;

/* Constructs the entity of virtual table D_80068DAC for fighter at position. The match depends on reading
 * the three words before storing any, as vecCopy does, and on storing fighter between the reads and the
 * writes. */
Unk8008B62C *func_8008B62C(Unk8008B62C *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DAC;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->fighter = fighter;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk30 = 0;
    this->unk34 = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_80068DAC, which func_8008B62C installs. */
void func_8008B6A8(Entity *this, s32 flags) {
    this->vtable = &D_80068DAC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", func_8008B6FC);

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", func_8008B860);

/* The entity of func_8008BE04 (vtable D_80068D84, 0x34 bytes): a character's, with its 0x440 (Fighter_getVramY). */
typedef struct Unk8008BE04 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* the character's (Fighter_getPosition) */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s16 vramY; /* the character's 0x440 (Fighter_getVramY) */
} Unk8008BE04; /* size 0x34 */

/* Constructs the character's entity of vtable D_80068D84. */
Unk8008BE04 *func_8008BE04(Unk8008BE04 *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068D84;
    this->fighter = fighter;
    this->unk2C = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_80068D84, which func_8008BE04 installs. */
void func_8008BE5C(Entity *this, s32 flags) {
    this->vtable = &D_80068D84;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of D_80068D84's entity: follows the character's position, and deletes itself after 24
 * frames (count at 0x2C).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x2C. */
void func_8008BEB0(Unk8008BE04 *this) {
    Fighter_getPosition(this->fighter, &this->position);
    if (this->unk2C >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk2C++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", func_8008BF2C);

/* A hitbox of the special move 2 (one a frame): like Unk800ACA60, with a flag at 0xBA. */
typedef struct HolydramonSpecial2Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Holydramon *owner;
    /* 0x84 */ Fight *fight; /* the part goes into its side's list of it */
    /* 0x88 */ Vec3 position;
    /* 0x94 */ s32 unk94;
    /* 0x98 */ Vec3 velocity;
    /* 0xA4 */ u8 unkA4[0xB4 - 0xA4];
    /* 0xB4 */ s32 age;
    /* 0xB8 */ s16 vramY;   /* Fighter_getVramY of the owner */
    /* 0xBA */ u16 stopped; /* set with the part's COLLISION_SHAPE_DISABLED: while the owner's 0x4A8 is set, and after
                             * 32 frames; it then slows by a quarter a frame */
} HolydramonSpecial2Hitbox;

/* Builds a hitbox of the special move 2 for fighter at position, moving by velocity (its knockback 4 times that), its
 * part in its side's list of fight; stopped if fighter's 0x4A8 is set. */
HolydramonSpecial2Hitbox *HolydramonSpecial2Hitbox_init(HolydramonSpecial2Hitbox *this, Holydramon *fighter,
                                                        Vec3 *position, VECTOR *velocity, Fight *fight) {
    VECTOR v;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, &fighter->base, 0x2AC0D00, (Vec3 *)velocity, 0);
    this->parts.base.vtable = HOLYDRAMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &HOLYDRAMON_SPECIAL2_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0x80, &this->parts.base);
    v = *velocity;
    v.vx <<= 2;
    v.vy <<= 2;
    v.vz <<= 2;
    Hit_setKnockback(&this->parts.base, &v);
    this->owner = fighter;
    this->fight = fight;
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
    this->age = 0;
    this->stopped = 0;
    this->vramY = Fighter_getVramY(&fighter->base);
    this->parts.part.entity = &this->parts.entity;
    if (Fighter_getSide(&this->owner->base) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    if (this->owner->unk4A8 != 0) {
        this->stopped = 1;
        this->parts.part.flags |= COLLISION_SHAPE_DISABLED;
    }
    return this;
}

/* The special move 2 hitbox's destructor. */
void HolydramonSpecial2Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = HOLYDRAMON_SPECIAL2_HITBOX_HIT_VTABLE;
    this->entity.vtable = &HOLYDRAMON_SPECIAL2_HITBOX_VTABLE;
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

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", HolydramonSpecial2Hitbox_hit);

/* Moves by its velocity with the part following; stopped while the owner's 0x4A8 is set and after 32 frames, then slows
 * by a quarter a frame; deletes itself after 40. The match depends on the unit's -fno-cse-follow-jumps (Makefile): the
 * else block reloads 0xB4. */
void HolydramonSpecial2Hitbox_tick(HolydramonSpecial2Hitbox *this) {
    s32 x, y, z;

    if (this->owner->unk4A8 != 0) {
        this->stopped = 1;
        this->parts.part.flags |= COLLISION_SHAPE_DISABLED;
    }
    if (this->stopped) {
        this->velocity.vx -= this->velocity.vx >> 2;
        this->velocity.vy -= this->velocity.vy >> 2;
        this->velocity.vz -= this->velocity.vz >> 2;
    }
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
    if (++this->age > 40) {
        entityDelete(&this->parts.entity);
    } else if (this->age > 32) {
        this->stopped = 1;
        this->parts.part.flags |= COLLISION_SHAPE_DISABLED;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", HolydramonSpecial2Hitbox_draw);

/* The entity class of virtual table D_80068DFC (func_8008C94C, made by HolydramonSpecial2Hitbox_hit): a frame count at
 * 0x28. */
typedef struct Unk8008C94C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* func_8007A6CC of the character */
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s16 vramY; /* the character's 0x440 (Fighter_getVramY) */
} Unk8008C94C;

/* Constructs the entity of virtual table D_80068DFC for fighter, at func_8007A6CC's position. */
Unk8008C94C *func_8008C94C(Unk8008C94C *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DFC;
    func_8007A6CC(fighter, &this->position);
    this->vramY = Fighter_getVramY(fighter);
    this->unk28 = 0;
    return this;
}

/* The destructor of the entity class of vtable D_80068DFC, which func_8008C94C installs. */
void func_8008C9AC(Entity *this, s32 flags) {
    this->vtable = &D_80068DFC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of D_80068DFC's entity: deletes itself after 24 frames (count at 0x28).
 * The match depends on the unit's -fno-cse-follow-jumps (Makefile): the else block reloads 0x28. */
void func_8008CA00(Unk8008C94C *this) {
    if (this->unk28 >= 24) {
        entityDelete(&this->entity);
    } else {
        this->unk28++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", func_8008CA74);

/* The entity class of virtual table D_80068DD4 (func_8008CE68, 0x40 bytes, made by HolydramonSpecial2Hitbox_hit). */
typedef struct Unk8008CE68 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* func_8007A6CC of the character */
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ Vec3 velocity; /* x and y random, z 0; loses a quarter every tick */
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ s32 frames;
    /* 0x3C */ s16 vramY; /* the character's 0x440 (Fighter_getVramY) */
} Unk8008CE68;

/* Constructs the entity of virtual table D_80068DD4 for fighter, at func_8007A6CC's position, with a velocity of x
 * and y random in -0x40 to 0x3E (fight's generator). It reads the character's 0x440 twice. */
Unk8008CE68 *func_8008CE68(Unk8008CE68 *this, Fighter *fighter, Fight *fight) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_80068DD4;
    func_8007A6CC(fighter, &this->position);
    this->vramY = Fighter_getVramY(fighter);
    this->velocity.vx = ((s32)(mersenneTwisterGenerate(&fight->random) & 0x3F) - 0x20) * 2;
    this->velocity.vy = ((s32)(mersenneTwisterGenerate(&fight->random) & 0x3F) - 0x20) * 2;
    this->velocity.vz = 0;
    this->frames = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The destructor of the entity class of vtable D_80068DD4, which func_8008CE68 installs. */
void func_8008CF18(Entity *this, s32 flags) {
    this->vtable = &D_80068DD4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The tick of func_8008CE68's entity: for 24 frames it drifts by its velocity, which loses a quarter every frame; then
 * it deletes itself. */
void func_8008CF6C(Unk8008CE68 *this) {
    s32 x, y, z;

    if (this->frames >= 24) {
        entityDelete(&this->entity);
    } else {
        /* The match depends on counting the frame first, and on the velocity slowing in place before the
         * position is read whole (HolydramonSpecial2Hitbox_tick's shape). */
        this->frames++;
        this->velocity.vx -= this->velocity.vx >> 2;
        this->velocity.vy -= this->velocity.vy >> 2;
        this->velocity.vz -= this->velocity.vz >> 2;
        x = this->position.vx;
        y = this->position.vy;
        z = this->position.vz;
        x += this->velocity.vx;
        y += this->velocity.vy;
        z += this->velocity.vz;
        this->position.vx = x;
        this->position.vy = y;
        this->position.vz = z;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/holydramon", func_8008D028);

INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068D84);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068DAC);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068DD4);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", D_80068DFC);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_SPECIAL2_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_SPECIAL2_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/holydramon", HOLYDRAMON_VTABLE);
