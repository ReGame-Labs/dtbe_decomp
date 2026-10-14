/* BlackWarGreymon (CHARACTER_BLACKWARGREYMON): the fighter createFighter builds for it, derived from WarGreymon (whose
 * methods it keeps but five), and the entities its own slots spawn: the slot 0x120 effect and the slot 0x128 hitbox. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

extern EntityVtable D_8006D01C;
extern EntityVtable BLACKWARGREYMON_VTABLE;

/* BlackWarGreymon's class (CHARACTER_BLACKWARGREYMON), derived from WarGreymon's. */
typedef struct BlackWarGreymon {
    /* 0x000 */ WarGreymon base;
    /* 0x4C4 */ s32 unk4C4;
} BlackWarGreymon;

Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4);
extern CVECTOR D_8006CFA8;
extern VECTOR D_8006CF98;
void func_800C1CE0(BlackWarGreymon *this);

/* Builds BlackWarGreymon: a WarGreymon with its own vtable, 0x4C4 0. */
BlackWarGreymon *BlackWarGreymon_init(BlackWarGreymon *this) {
    WarGreymon_init(&this->base);
    this->base.base.entity.vtable = &BLACKWARGREYMON_VTABLE;
    this->unk4C4 = 0;
    return this;
}

/* Input: Fighter's (Fighter_handleInput), then this class's own step (func_800C1CE0). */
void BlackWarGreymon_handleInput(void *this, void *arg) {
    Fighter_handleInput(this, arg);
    func_800C1CE0(this);
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF84);

INCLUDE_ASM("game/nonmatchings/fighters/blackwargreymon", BlackWarGreymon_intro);

INCLUDE_ASM("game/nonmatchings/fighters/blackwargreymon", BlackWarGreymon_throw);

/* the velocity func_800C1CE0 throws the opponent with */
extern VECTOR D_8006CF88;

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF88);

/* This class's own step (BlackWarGreymon_handleInput): when the opponent (Fighter_getOpponent) is over 0x800 above its
 * last position, 0x4C4 goes back to 0; at 1, once the opponent is on the ground, stops its force and velocity, throws
 * it with D_8006CF88 (Fighter_takeHit) and sets 0x4C4 to 2. */
void func_800C1CE0(BlackWarGreymon *this) {
    Fighter *opponent = Fighter_getOpponent(&this->base.base);
    VECTOR velocity;

    if ((opponent->position.vy >> 8) - (opponent->lastPosition.vy >> 8) < -0x800) {
        this->unk4C4 = 0;
        return;
    }
    if (this->unk4C4 == 1 && Fighter_isOnGround(opponent)) {
        velocity = D_8006CF88;
        opponent->force.vx = 0;
        opponent->force.vy = 0;
        opponent->force.vz = 0;
        opponent->forcePad = 0;
        opponent->velocity.vx = 0;
        opponent->velocity.vy = 0;
        opponent->velocity.vz = 0;
        opponent->velocityPad = 0;
        Fighter_takeHit(opponent, 0x408C1000, (Vec3 *)&velocity, 0);
        this->unk4C4 = 2;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CF98);

/* Whether point 10 moved by D_8006CF98 is nearer the opponent's top (func_8007A6CC) than its height plus 512. */
s32 func_800C1DBC(Fighter *this) {
    s32 reach;
    VECTOR node;
    VECTOR way;
    s32 x, y, z;

    reach = Fighter_getParams(Fighter_getOpponent(this))->height + 512;
    Fighter_getNodePosition(this, &node, 10, &D_8006CF98);
    func_8007A6CC(this, (Vec3 *)&way);
    /* The match depends on the vecSubInPlace shape: read way whole, subtract, then store. */
    x = way.vx;
    y = way.vy;
    z = way.vz;
    x -= node.vx;
    y -= node.vy;
    z -= node.vz;
    way.vx = x;
    way.vy = y;
    way.vz = z;
    return vecGetLength(&way) < reach;
}

/* Creates an entity of the class of vtable D_8006D01C (0x120 bytes) with g++'s new, and hands it
 * to Fighter_addEntity. Its constructor func_800C27E4 is inlined. */
void func_800C1E64(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006D01C;
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/blackwargreymon", BlackWarGreymon_special3);

/* Creates an entity of func_800C2264's class (0xD4 bytes) at pos with g++'s new and hands it to Fighter_addEntity, then
 * plays sound 0x11B, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800C2004(Fighter *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800C2264(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x200);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0x11B);
    this->invulnerableFrames = 10;
    Fighter_addGauge(this, -0x10000);
}

INCLUDE_ASM("game/nonmatchings/fighters/blackwargreymon", func_800C20A8);

extern VtableEntry D_8006CFCC[];
extern EntityVtable D_8006CFEC;

/* Constructs the class of virtual table D_8006CFEC (and D_8006CFCC for its base at 0x18) on
 * func_800BDA50's, with 0x5B05333 and 0 in its base's 0x8 and 0xC. */
Unk800BDA50 *func_800C2264(Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    func_800BDA50(this, owner, pos, arg3, arg4);
    this->unk18.vtable = D_8006CFCC;
    this->entity.vtable = &D_8006CFEC;
    Hit_set(&this->unk18, 0x5B05333, 0);
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFA8);

/* The method at 0x20 of virtual table D_8006CFEC: adds func_800BE39C's and ParticleBurst_init's entities at 0xA4
 * as siblings, then plays sound 0x11C. */
void func_800C22C0(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB0A666, 0x500, &D_8006CFA8);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0xC0, 0xC0, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x11C, 0);
}

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFAC);

INCLUDE_ASM("game/nonmatchings/fighters/blackwargreymon", func_800C23B8);

/* BlackWarGreymon's destructor. */
void BlackWarGreymon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D01C, which func_800C1E64 and func_800C27E4 install. */
void func_800C279C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006D01C, which derives from func_800BD490's. */
Entity *func_800C27E4(Entity *this, Fighter *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006D01C;
    return this;
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800C2818(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
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

INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFCC);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006CFEC);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", D_8006D01C);
INCLUDE_RODATA("game/nonmatchings/fighters/blackwargreymon", BLACKWARGREYMON_VTABLE);
