/* Renamon (CHARACTER_RENAMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's hitbox and effect, and the special move 2's effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"

struct RenamonSpecial1Effect *RenamonSpecial1Effect_init(struct RenamonSpecial1Effect *this, Fighter *chara);
extern VtableEntry RENAMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable RENAMON_SPECIAL1_HITBOX_VTABLE;

/* The class of func_800A4550 and func_800A45C8, which spawn entities at its
 * two positions. */
typedef struct Unk800A4550 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x84 - 0x18];
    /* 0x84 */ Vec3 unk84;
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ Vec3 unkA4;
} Unk800A4550;

/* Renamon's class (CHARACTER_RENAMON). */
typedef struct Renamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 unk4A8;
    /* 0x4AC */ s32 unk4AC;
    /* 0x4B0 */ s32 unk4B0;
} Renamon;

extern FaceEntry RENAMON_FACES[];
extern EntityVtable RENAMON_VTABLE;

/* Builds Renamon: a Fighter with its own vtable and faces. */
Renamon *Renamon_init(Renamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &RENAMON_VTABLE;
    this->unk4AC = 0;
    Fighter_setFaceTable(&this->base, RENAMON_FACES);
    this->unk4A4 = 0;
    this->unk4A8 = 0;
    this->unk4B0 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_handleInput);

/* Ends a move: back to face 0 (Fighter_setFace). */
void Renamon_endMove(Fighter *this) {
    Fighter_setFace(this, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_airAttack);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_throw);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_special1);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_airSpecial1);

s32 func_800A3754(Fighter *this);
void func_800A37F4(Fighter *this);

/* func_800A3754 when bit 0 of the character's flags at 0x37C is clear, else func_800A37F4. */
void func_800A3710(Fighter *this) {
    if (Fighter_isComputer(this) == 0) {
        func_800A3754(this);
    } else {
        func_800A37F4(this);
    }
}

/* A value from the character's input (InputHistory_getHeld): up gives 0, or -0x200 unless Fighter_getAttackStateId is
 * 30; down gives 0x200; holding the way it faces (Fighter_getFacing) gives 0, anything else -1. */
s32 func_800A3754(Fighter *this) {
    u32 input = InputHistory_getHeld(Fighter_getInput(this));
    s32 facing;

    if (input & PAD_UP) {
        return Fighter_getAttackStateId(this) == FIGHTER_STATE_AIR_SPECIAL1 ? 0 : -0x200;
    }
    if (input & PAD_DOWN) {
        return 0x200;
    }
    facing = Fighter_getFacing(this);
    if (facing > 0 && (input & PAD_RIGHT)) {
        return 0;
    }
    if (facing >= 0) {
        return -1;
    }
    if (input & PAD_LEFT) {
        return 0;
    }
    return -1;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A37F4);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_spawnSpecial1Hitbox);

/* Spawns the special move 1's effect. */
void Renamon_spawnSpecial1Effect(Fighter *this) {
    Entity *child = allocEntityMemory(0x44);

    if (child != NULL) {
        RenamonSpecial1Effect_init(entityOperatorNewAt(0x44, child), this);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", Renamon_special2);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A3C98);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_init);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_hit);

/* Spawns the entity of Sparkle_init between 0x84 and 0xA4, as a sibling. */
void func_800A4550(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x5C);

    if (child != NULL) {
        Sparkle_init(entityOperatorNewAt(0x5C, child), &this->unk84, &this->unkA4, 0x10, 0x408080, 9);
    }
    entityAddSibling(&this->entity, child);
}

/* Spawns the entity of ParticleBurst_init at 0x84, as a sibling. */
void func_800A45C8(Unk800A4550 *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 9, &this->unk84, 0x20, 0x10, 8, 0x408080);
    }
    entityAddSibling(&this->entity, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Hitbox_draw);

extern EntityVtable RENAMON_SPECIAL1_EFFECT_VTABLE;

/* The special move 1's effect (0x44 bytes). */
typedef struct RenamonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *chara;
    /* 0x1C */ Vec3 position; /* the character's at construction (Fighter_getPosition) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 facing; /* Fighter_getFacing: 1 or -1 */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 vramY; /* Fighter_getVramY of the character */
} RenamonSpecial1Effect;

/* Builds the special move 1's effect for chara, at its position and facing its way. */
RenamonSpecial1Effect *RenamonSpecial1Effect_init(RenamonSpecial1Effect *this, Fighter *chara) {
    Vec3 position;
    s32 facing;

    entityInit(&this->entity, 0);
    this->entity.vtable = &RENAMON_SPECIAL1_EFFECT_VTABLE;
    facing = Fighter_getFacing(chara);
    Fighter_getPosition(chara, &position);
    this->chara = chara;
    this->unk2C = 0;
    this->unk30 = 0x80;
    this->facing = facing;
    this->unk38 = 0;
    this->unk3C = 0;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    this->vramY = Fighter_getVramY(chara);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Effect_draw);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A4C04);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A4D10);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A4E1C);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A4F64);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A5064);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", func_800A51DC);

/* Renamon's destructor. */
void Renamon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 1 hitbox's destructor. */
void RenamonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = RENAMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &RENAMON_SPECIAL1_HITBOX_VTABLE;
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

/* The special move 1 effect's destructor. */
void RenamonSpecial1Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9CC, which func_800A4C04 installs. */
void func_800A5438(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006A9A4, which func_800A4F64 installs. */
void func_800A5480(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A924);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A92C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A934);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A93C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A944);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A94C);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A954);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A964);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A984);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A9A4);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A9CC);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_VTABLE);
