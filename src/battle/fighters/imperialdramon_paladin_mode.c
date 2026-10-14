/* Imperialdramon Paladin Mode (CHARACTER_IMPERIALDRAMON_PALADIN_MODE): the fighter createFighter builds for it, a
 * derived class of Imperialdramon's (imperialdramon.c) that keeps most of its moves, and the entities of its own
 * moves: the intro's effect and the special move 3's hitbox and trail. */

#include "common.h"
#include "battle/game_classes.h"

Entity *ImperialdramonPaladinModeIntroEffect_init(Entity *this, Fighter *fighter, Vec3 *position);
extern TrailPoint D_800694B4[];
extern EntityVtable IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE;
extern VtableEntry IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_HIT_VTABLE[];
extern EntityVtable IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable IMPERIALDRAMON_PALADIN_MODE_VTABLE;
Entity *ImperialdramonPaladinModeSpecial3Hitbox_init(void *this, Fighter *fighter, Vec3 *position, Vec3 *velocity,
                                                     Fight *unk228);

/* Builds Imperialdramon Paladin Mode: an Imperialdramon with its own vtable. */
Imperialdramon *ImperialdramonPaladinMode_init(Imperialdramon *this) {
    Imperialdramon_init(this);
    this->base.entity.vtable = &IMPERIALDRAMON_PALADIN_MODE_VTABLE;
    return this;
}

/* Imperialdramon Paladin Mode's destructor. */
void ImperialdramonPaladinMode_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_PALADIN_MODE_VTABLE;
    Imperialdramon_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Intro: spawns its effect at the fighter at frame 1. */
void ImperialdramonPaladinMode_intro(Fighter *this, s32 frame) {
    Vec3 position;
    Entity *child;

    if (frame == 1) {
        Fighter_getPosition(this, &position);
        child = allocEntityMemory(0x74);
        if (child != NULL) {
            ImperialdramonPaladinModeIntroEffect_init(entityOperatorNewAt(0x74, child), this, &position);
        }
        Fighter_addEntity(this, child);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinMode_special3);

/* Spawns the special move 3's hitbox at the fighter, 64 a frame forward. */
void ImperialdramonPaladinMode_spawnSpecial3Hitbox(Fighter *this) {
    Fight *unk228 = Fighter_getScene(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    velocity.vx = Fighter_getFacing(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0xF0);
    if (child != NULL) {
        ImperialdramonPaladinModeSpecial3Hitbox_init(entityOperatorNewAt(0xF0, child), this, &position, &velocity,
                                                     unk228);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode", D_800694B4);

/* Spawns the special move 3's trail (Trail_init with the table D_800694B4). */
void ImperialdramonPaladinMode_spawnSpecial3Trail(Fighter *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, D_800694B4, 2, 8, 15);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeSpecial3Hitbox_init);

/* The special move 3 hitbox's destructor. */
void ImperialdramonPaladinModeSpecial3Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_HIT_VTABLE;
    this->entity.vtable = &IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_VTABLE;
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

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode", D_800694E4);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeSpecial3Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeSpecial3Hitbox_hit);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeSpecial3Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeIntroEffect_init);

/* The intro effect's destructor. */
void ImperialdramonPaladinModeIntroEffect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeIntroEffect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeIntroEffect_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode", IMPERIALDRAMON_PALADIN_MODE_VTABLE);
