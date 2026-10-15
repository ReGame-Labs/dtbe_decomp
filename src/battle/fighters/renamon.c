/* Renamon (CHARACTER_RENAMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * special move 1's hitbox and effect, and the special move 2's effects. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"

struct RenamonSpecial1Effect *RenamonSpecial1Effect_init(struct RenamonSpecial1Effect *this, Fighter *fighter);
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
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ Vec3 position; /* the character's at construction (Fighter_getPosition) */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 facing; /* Fighter_getFacing: 1 or -1 */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 vramY; /* Fighter_getVramY of the character */
} RenamonSpecial1Effect;

/* Builds the special move 1's effect for fighter, at its position and facing its way. */
RenamonSpecial1Effect *RenamonSpecial1Effect_init(RenamonSpecial1Effect *this, Fighter *fighter) {
    Vec3 position;
    s32 facing;

    entityInit(&this->entity, 0);
    this->entity.vtable = &RENAMON_SPECIAL1_EFFECT_VTABLE;
    facing = Fighter_getFacing(fighter);
    Fighter_getPosition(fighter, &position);
    this->fighter = fighter;
    this->unk2C = 0;
    this->unk30 = 0x80;
    this->facing = facing;
    this->unk38 = 0;
    this->unk3C = 0;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Effect_tick);

INCLUDE_ASM("game/nonmatchings/fighters/renamon", RenamonSpecial1Effect_draw);

/* The special move 2's effect (Renamon_special2 spawns it, 0x44 bytes): a sprite rising and drifting in x and z. */
typedef struct RenamonSpecial2Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position; /* the character's, moved by up to 127 in x and z */
    /* 0x24 */ s32 positionPad;
    /* 0x28 */ Vec3 velocity;
    /* 0x34 */ s32 velocityPad;
    /* 0x38 */ s32 framesLeft; /* 0x18 to 0x27 at construction; it also picks the drift angle and the cell */
    /* 0x3C */ s32 vramY; /* Fighter_getVramY of the character */
    /* 0x40 */ s32 clutY; /* vramY + 0xFE, or + 0xFF one time in four */
} RenamonSpecial2Effect;

extern EntityVtable RENAMON_SPECIAL2_EFFECT_VTABLE;

/* Constructs the effect at the character's position moved by 0 to 127 in x and z, still, with 0x18 to 0x27
 * frames to live and the character's clut row 0xFE further down, or 0xFF one time in four.
 * The match depends on the generator's address taken in a local after the vtable is set. */
RenamonSpecial2Effect *RenamonSpecial2Effect_init(RenamonSpecial2Effect *this, Fighter *fighter) {
    MersenneTwister *random;

    entityInit(&this->entity, 0);
    this->entity.vtable = &RENAMON_SPECIAL2_EFFECT_VTABLE;
    random = &RANDOM;
    Fighter_getPosition(fighter, &this->position);
    this->position.vx += (s32)mersenneTwisterGenerate(random) % 128;
    this->position.vz += (s32)mersenneTwisterGenerate(random) % 128;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocityPad = 0;
    this->framesLeft = (mersenneTwisterGenerate(random) & 0xF) + 0x18;
    this->vramY = Fighter_getVramY(fighter);
    this->clutY = this->vramY + 0xFE;
    this->clutY += (mersenneTwisterGenerate(random) & 3) == 0;
    return this;
}

/* The effect's tick: drifts in x and z by up to 3 steps along the angle framesLeft * 0x100, rises by 8 to 23,
 * moves, and deletes itself when framesLeft runs out. */
void RenamonSpecial2Effect_tick(RenamonSpecial2Effect *this) {
    s32 step;
    s32 x, y, z;

    step = mersenneTwisterGenerate(&RANDOM) & 3;
    this->velocity.vx += (step * COS_SIN_TABLE[(this->framesLeft << 8) & 0xF00].sin) >> 12;
    this->velocity.vz += (step * COS_SIN_TABLE[(this->framesLeft << 8) & 0xF00].cos) >> 12;
    this->velocity.vy = -(mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->framesLeft--;
    if (this->framesLeft < 0) {
        entityDelete(&this->entity);
    }
}

/* PolyFT4Shape's uv words as their u and v bytes (as in saintgargomon.c): game_classes.h types them u16, and the game
 * stores the bytes. */
#define UV_BYTES(uv) ((u8 *)&(uv))

/* The corners of a 64 by 64 square around the origin. */
extern SVECTOR RENAMON_SPECIAL2_EFFECT_CORNERS[4];

/* Draws the effect: the square RENAMON_SPECIAL2_EFFECT_CORNERS at its position, with the 16 by 16 cell (framesLeft & 3)
 * of the texture row at 0x30 and the clut row clutY, with buildPolyFT4.
 * The match depends on the matrix built as matrixInitTranslationVec does, reading the position through a pointer, on
 * u in a u8 (lbu) and on the color set after the uv bytes. */
void RenamonSpecial2Effect_draw(RenamonSpecial2Effect *this, Camera *camera) {
    MatrixWords m;
    PolyFT4Shape shape;
    Vec3 *pos;
    u8 *packet;
    u_long *ot;
    s32 zShift;
    s32 x, y, z;
    u8 u;

    pos = &this->position;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    packet = PRIM_BUFFER_FREE;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    ot = camera->ot.slots;
    zShift = camera->zShift;
    setModelMatrix(&m.m);
    shape.flags = 0;
    shape.corners[0] = &RENAMON_SPECIAL2_EFFECT_CORNERS[0];
    shape.corners[1] = &RENAMON_SPECIAL2_EFFECT_CORNERS[1];
    shape.corners[2] = &RENAMON_SPECIAL2_EFFECT_CORNERS[2];
    shape.corners[3] = &RENAMON_SPECIAL2_EFFECT_CORNERS[3];
    u = ((this->framesLeft & 3) << 4) | 0xC0;
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = u;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = u + 0xF;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0x30;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0x3F;
    shape.color = 0x808080;
    shape.clut = getClut(0x1D0, this->clutY);
    shape.tpage = getTPage(0, 0, 0x1C0, this->vramY);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, zShift);
}

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

/* The special move 2 effect's destructor. */
void RenamonSpecial2Effect_destroy(Entity *this, s32 flags) {
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
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL2_EFFECT_CORNERS);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", D_8006A9A4);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL2_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/renamon", RENAMON_VTABLE);
