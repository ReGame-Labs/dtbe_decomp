/* Imperialdramon Paladin Mode (CHARACTER_IMPERIALDRAMON_PALADIN_MODE): the fighter createFighter builds for it, a
 * derived class of Imperialdramon's (imperialdramon.c) that keeps most of its moves, and the entities of its own
 * moves: the intro's effect and the special move 3's hitbox and trail. */

#include "common.h"
#include "battle/game_classes.h"

/* The intro's effect: two textured quads on the fighter's model, a base and a narrower one that stretches out of its
 * far end, holds, then shrinks back until the effect deletes itself. */
typedef struct ImperialdramonPaladinModeIntroEffect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ u16 frames;  /* counts up to 88, then the stretch shrinks */
    /* 0x1E */ u16 vramY;   /* Fighter_getVramY of the character */
    /* 0x20 */ u16 state;   /* 0 while the stretch grows, then 1 */
    /* 0x22 */ s16 length;  /* the far end of quads[1]: 0x100 (the base's) to 0x200 */
    /* 0x24 */ Vec3 position;
    /* 0x30 */ s32 positionPad;
    /* 0x34 */ SVECTOR quads[2][4]; /* the base, then the stretch: corners in the model's x = 0 plane */
} ImperialdramonPaladinModeIntroEffect;

ImperialdramonPaladinModeIntroEffect *ImperialdramonPaladinModeIntroEffect_init(
    ImperialdramonPaladinModeIntroEffect *this, Fighter *fighter, Vec3 *position);
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

/* The special move 3's hitbox (0xF0 bytes): a sphere at the fighter's node 1 and two quads whose length grows. */
typedef struct ImperialdramonPaladinModeSpecial3Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Imperialdramon *fighter;
    /* 0x84 */ u8 unk84[0x88 - 0x84];
    /* 0x88 */ u16 frames;     /* counts the ticks; the sphere is enabled at 20, the hitbox deleted after 50 */
    /* 0x8A */ s16 vramY;      /* Fighter_getVramY of the fighter */
    /* 0x8C */ u16 unk8C;      /* while set, the length stays */
    /* 0x8E */ s16 length;     /* 0x140 at first, + 0x18 a tick up to 0x2C0 */
    /* 0x90 */ Vec3 position;  /* the fighter's node 1 (offset D_800694E4) */
    /* 0x9C */ s32 positionPad;
    /* 0xA0 */ Vec3 velocity;  /* the constructor's */
    /* 0xAC */ s32 velocityPad;
    /* 0xB0 */ SVECTOR quads[2][4];
} ImperialdramonPaladinModeSpecial3Hitbox;

/* the offset from node 1 where the special move 3's hitbox sits */
extern VECTOR D_800694E4;

/* Grows the length (unless 0x8C is set) into the second quad's last two corners, follows the fighter's node 1 with
 * the sphere, enables the sphere at frame 20 and deletes the hitbox after frame 50. US also clears the fighter's
 * hold flag from frame 26. */
void ImperialdramonPaladinModeSpecial3Hitbox_tick(ImperialdramonPaladinModeSpecial3Hitbox *this) {
    if (this->unk8C == 0) {
        this->length += 0x18;
        if (this->length > 0x2C0) {
            this->length = 0x2C0;
        }
    }
    this->quads[1][2].vy = this->quads[1][3].vy = this->length;
    Fighter_getNodePosition(&this->fighter->base, (VECTOR *)&this->position, 1, &D_800694E4);
    this->parts.part.position = this->position;
    if (this->frames == 20) {
        this->parts.part.flags.bits.disabled = 0;
    }
#if VERSION_US
    if (this->frames >= 26) {
        this->fighter->holding = 0;
    }
#endif
    if (this->frames > 50) {
        entityDelete(&this->parts.entity);
    } else {
        this->frames++;
    }
}

/* On a hit: Hit_apply, then for a fighter hit the part is disabled. The first time (the owner's 0x4A4 clear) the
 * fighter is also pushed (force (facing * 0x38E0, 0x2AA8, 0), velocity 0), 0x4A4 is set, a burst of 16
 * (ParticleBurst_init) is spawned at the fighter and the part's touched bit is cleared. US also clears the owner's
 * hold flag then. */
void ImperialdramonPaladinModeSpecial3Hitbox_hit(ImperialdramonPaladinModeSpecial3Hitbox *this, CollisionSphere *sum) {
    Fighter *target;
    Vec3 position;
    Vec3 force;
    s16 facing;
    s32 spread;
    Entity *child;

    Hit_apply(&this->parts.base, sum);
    target = (Fighter *)getHitFighter(sum);
    if (target == NULL) {
        return;
    }
    if (this->fighter->unk4A4 == 0) {
        Fighter_getPosition(target, &position);
        this->parts.part.flags.bits.disabled = 1;
        facing = Fighter_getFacing(&this->fighter->base);
        force.vx = facing * 0x38E0;
        force.vy = 0x2AA8;
        force.vz = 0;
        {
            s32 x = force.vx;
            s32 y = force.vy;
            s32 z = force.vz;

            target->force.vx = x;
            target->force.vy = y;
            target->force.vz = z;
        }
        target->velocity.vx = 0;
        target->velocity.vy = 0;
        target->velocity.vz = 0;
        target->velocityPad = 0;
        this->fighter->unk4A4 = 1;
#if VERSION_US
        this->fighter->holding = 0;
#endif
        spread = this->velocity.vx >> 1;
        child = allocEntityMemory(0x3C);
        if (child != NULL) {
            ParticleBurst_init(entityOperatorNewAt(0x3C, child), 4, &position, spread, 0x120, 0x10, 0x808080);
        }
        entityAddSibling(&this->parts.entity, child);
        this->parts.part.flags.bits.touched = 0;
    } else {
        this->parts.part.flags.bits.disabled = 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeSpecial3Hitbox_draw);

/* Builds the intro's effect on the fighter: the base quad, and the second quad still flat at the base's far end. The
 * match depends on the chained corner assignments (their store order) and on reading position's three words before
 * storing any, as vecCopy does. */
ImperialdramonPaladinModeIntroEffect *ImperialdramonPaladinModeIntroEffect_init(
    ImperialdramonPaladinModeIntroEffect *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE;
    this->vramY = Fighter_getVramY(fighter);
    this->fighter = fighter;
    this->quads[0][0].vx = this->quads[0][1].vx = this->quads[0][2].vx = this->quads[0][3].vx = 0;
    this->quads[0][0].vy = this->quads[0][1].vy = 0x40;
    this->quads[0][2].vy = this->quads[0][3].vy = 0xC0;
    this->quads[0][0].vz = this->quads[0][2].vz = 0;
    this->quads[0][1].vz = this->quads[0][3].vz = 0x100;
    this->quads[1][0].vx = this->quads[1][1].vx = this->quads[1][2].vx = this->quads[1][3].vx = 0;
    this->quads[1][0].vy = this->quads[1][1].vy = 0x60;
    this->quads[1][2].vy = this->quads[1][3].vy = 0xA0;
    this->quads[1][0].vz = this->quads[1][2].vz = 0x100;
    this->quads[1][1].vz = this->quads[1][3].vz = 0x100;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->length = 0x100;
    this->state = 0;
    this->frames = 0;
    return this;
}

/* The intro effect's destructor. */
void ImperialdramonPaladinModeIntroEffect_destroy(Entity *this, s32 flags) {
    this->vtable = &IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Stretches the second quad by 0x20 a frame up to 0x200; from frame 88 shrinks it by 0x20 a frame and deletes the
 * effect once it is back at the base. The match depends on the switch: its case label starts a new CSE block, so the
 * state is loaded again for the increment. */
void ImperialdramonPaladinModeIntroEffect_tick(ImperialdramonPaladinModeIntroEffect *this) {
    switch (this->state) {
    case 0:
        this->length += 0x20;
        if (this->length >= 0x200) {
            this->length = 0x200;
            this->state++;
        }
        break;
    }
    this->quads[1][1].vz = this->quads[1][3].vz = this->length;
    if (this->frames >= 88) {
        this->length -= 0x20;
        if (this->length <= 0x100) {
            entityDelete(&this->entity);
        }
    } else {
        this->frames++;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/imperialdramon_paladin_mode", ImperialdramonPaladinModeIntroEffect_draw);

INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_INTRO_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode",
               IMPERIALDRAMON_PALADIN_MODE_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/imperialdramon_paladin_mode", IMPERIALDRAMON_PALADIN_MODE_VTABLE);
