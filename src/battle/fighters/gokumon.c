/* Gokumon (CHARACTER_GOKUMON): the fighter createFighter builds for it, its moves, and the entities they spawn: the
 * attacks' trails, the special move 1's hitbox, the special move 3's hitbox and burst, and the effect of the throw
 * and the special move 2. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/matrix.h"
#include "engine/sound/sound_control.h"

void Gokumon_spawnTrail(Fighter *this, s32 arg1, s32 arg2, s32 rgb, s32 alt);
void Gokumon_spawnTrails(Fighter *this, s32 arg1, s32 arg2, s32 rgb);
s32 Gokumon_getAttackTrailColor(Fighter *this);

/* whether the character's fighter (by Fighter_getSide) has the other colors (altColor) */
s32 Gokumon_hasAltColor(Fighter *this);
Entity *GokumonSpecial1Hitbox_init(Entity *this, Fighter *arg);
Entity *GokumonSpecial3Hitbox_init(Entity *this, Fighter *arg, Vec3 *vec);
extern EntityVtable GOKUMON_SPECIAL3_HITBOX_VTABLE;
extern EntityVtable GOKUMON_SPECIAL1_HITBOX_VTABLE;
extern EntityVtable GOKUMON_VTABLE;
extern s32 GOKUMON_ATTACK_TRAIL_COLORS[];
extern s32 GOKUMON_SPECIAL_TRAIL_COLORS[];
void Gokumon_spawnSpecial3Hitbox(Fighter *this);
void func_800C00EC(Fighter *this);
void Gokumon_spawnSpecial3Burst(Fighter *this);

/* Builds Gokumon: a plain Fighter with its own vtable. */
Fighter *Gokumon_init(Fighter *this) {
    Fighter_init(this);
    this->entity.vtable = &GOKUMON_VTABLE;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CC7C);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CCC4);

/* Trail_init's tables for Gokumon_spawnTrail: three entries each, D_8006CCC4 when alt, else D_8006CC7C. */
extern TrailPoint D_8006CC7C[3];
extern TrailPoint D_8006CCC4[3];

void Trail_setColors(Trail *this, CVECTOR *colors, s32 count);

/* Spawns a trail: three entries of its table (D_8006CCC4 when alt, else D_8006CC7C), arg1 and arg2, colored rgb (an
 * 0xBBGGRR word, read as its three bytes) and half rgb. */
void Gokumon_spawnTrail(Fighter *this, s32 arg1, s32 arg2, s32 rgb, s32 alt) {
    TrailPoint *entries = alt ? D_8006CCC4 : D_8006CC7C;
    CVECTOR *bytes = (CVECTOR *)&rgb;
    CVECTOR colors[2] = { { bytes->r, bytes->g, bytes->b }, { bytes->r >> 1, bytes->g >> 1, bytes->b >> 1 } };
    Entity *entity = NEW_ENTITY(0x690, Trail_init, this, entries, 3, arg1, arg2);

    if (entity != NULL) {
        Trail_setColors((Trail *)entity, colors, 2);
        Fighter_addEntity(this, entity);
    }
}

/* Trail_init's table for Gokumon_spawnTrails: three groups of three entries. */
extern TrailPoint D_800D35CC[9];

/* Spawns three trails as Gokumon_spawnTrail (alt 0), one for each group of three entries of D_800D35CC. */
void Gokumon_spawnTrails(Fighter *this, s32 arg1, s32 arg2, s32 rgb) {
    TrailPoint *entries = D_800D35CC;
    CVECTOR *bytes = (CVECTOR *)&rgb;
    CVECTOR colors[2] = { { bytes->r, bytes->g, bytes->b }, { bytes->r >> 1, bytes->g >> 1, bytes->b >> 1 } };
    TrailPoint *end = entries + 9;

    do {
        Entity *entity = NEW_ENTITY(0x690, Trail_init, this, entries, 3, arg1, arg2);

        if (entity != NULL) {
            Trail_setColors((Trail *)entity, colors, 2);
            Fighter_addEntity(this, entity);
        }
        entries += 3;
    } while (entries != end);
}

/* Whether the fighter wears the other colors (both sides look alike). */
s32 Gokumon_hasAltColor(Fighter *this) {
    return GAME_STATE.fighters[Fighter_getSide(this)].altColor;
}

/* The attacks' trail color: GOKUMON_ATTACK_TRAIL_COLORS's second word in the other colors, else its first. */
s32 Gokumon_getAttackTrailColor(Fighter *this) {
    return GOKUMON_ATTACK_TRAIL_COLORS[Gokumon_hasAltColor(this) != 0];
}

/* The special moves' trail color: GOKUMON_SPECIAL_TRAIL_COLORS's second word in the other colors, else its first. */
s32 Gokumon_getSpecialTrailColor(Fighter *this) {
    return GOKUMON_SPECIAL_TRAIL_COLORS[Gokumon_hasAltColor(this) != 0];
}

/* Attack 1: Fighter's, with the trails (2, 4, attack color) at frame 0. */
void Gokumon_attack1(Fighter *this, s32 frame) {
    Fighter_attack1(this, frame);
    if (frame == 0) {
        Gokumon_spawnTrails(this, 2, 4, Gokumon_getAttackTrailColor(this));
    }
}

/* Attack 2: Fighter's, with a trail (2, 9, attack color) at frame 0. */
void Gokumon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 0) {
        Gokumon_spawnTrail(this, 2, 9, Gokumon_getAttackTrailColor(this), 0);
    }
}

/* Attack 3: Fighter's, with a trail (4, 0x12, attack color) at frame 2; frame 8 the hit takes the attack 3 damage of
 * its params with bit 31 set. */
void Gokumon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 2) {
        Gokumon_spawnTrail(this, 4, 0x12, Gokumon_getAttackTrailColor(this), 0);
    }
    if (frame == 8) {
        this->hitDamage = Fighter_getParams(this)->attacks[2].hitDamage | 0x80000000;
    }
}

/* Up attack: Fighter's, with the trails (2, 10, attack color) at frame 0. */
void Gokumon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 0) {
        Gokumon_spawnTrails(this, 2, 10, Gokumon_getAttackTrailColor(this));
    }
}

/* Dash attack: Fighter's, with a trail (2, 17, attack color) at frame 0. */
void Gokumon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 0) {
        Gokumon_spawnTrail(this, 2, 17, Gokumon_getAttackTrailColor(this), 0);
    }
}

/* Up attack in the air: Fighter's, with a trail (2, 12, attack color, alt table) at frame 2. */
void Gokumon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    if (frame == 2) {
        Gokumon_spawnTrail(this, 2, 12, Gokumon_getAttackTrailColor(this), 1);
    }
}

/* Side attack in the air: Fighter's, with a trail (2, 16, attack color) at frame 2. */
void Gokumon_airSideAttack(Fighter *this, s32 frame) {
    Fighter_airSideAttack(this, frame);
    if (frame == 2) {
        Gokumon_spawnTrail(this, 2, 16, Gokumon_getAttackTrailColor(this), 0);
    }
}

/* Attack in the air: Fighter's, with a trail (2, 8, attack color) at frame 2. */
void Gokumon_airAttack(Fighter *this, s32 frame) {
    Fighter_airAttack(this, frame);
    if (frame == 2) {
        Gokumon_spawnTrail(this, 2, 8, Gokumon_getAttackTrailColor(this), 0);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD0C);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", Gokumon_throw);

void Gokumon_spawnSpecial1Hitbox(Fighter *this);

/* Special move 1: Fighter's, braking to a stop; frame 0 a hit of flag 0x800000 without knockback, 4 invulnerable frames
 * and voice 0x118; frame 4 the hitbox and func_80079B0C(0x3C); frame 14 the trails (8, 0x10, special color); the model
 * parts 0x20 hit. */
void Gokumon_special1(Fighter *this, s32 frame) {
    Fighter_special1(this, frame);
    Fighter_accelerateTo(this, 0);
    if (frame == 0) {
        this->hitDamage = 0x800000;
        this->hitKnockback.vx = 0;
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        this->hitKnockbackPad = 0;
        this->hitKind = 1;
        this->invulnerableFrames = 4;
        Fighter_playVoice(this, 0x118);
    }
    if (frame == 4) {
        Gokumon_spawnSpecial1Hitbox(this);
        func_80079B0C(this, 0x3C);
    }
    if (frame == 14) {
        Gokumon_spawnTrails(this, 8, 0x10, Gokumon_getSpecialTrailColor(this));
    }
    this->attackParts = 0x20;
}

/* Special move 1 in the air: Fighter's; frame 0 as on the ground; frame 4 the hitbox; from frame 4 the velocity and the
 * horizontal force stopped and a push up of 0x71C; frame 14 the trails (8, 0x13, special color); the model parts 0x20
 * hit. */
void Gokumon_airSpecial1(Fighter *this, s32 frame) {
    Fighter_airSpecial1(this, frame);
    if (frame == 0) {
        this->hitDamage = 0x800000;
        this->hitKnockback.vx = 0;
        this->hitKnockback.vy = 0;
        this->hitKnockback.vz = 0;
        this->hitKnockbackPad = 0;
        this->hitKind = 1;
        this->invulnerableFrames = 4;
        Fighter_playVoice(this, 0x118);
    }
    if (frame == 4) {
        Gokumon_spawnSpecial1Hitbox(this);
    }
    if (frame >= 4) {
        this->velocity.vx = 0;
        this->velocity.vy = 0;
        this->force.vx = 0;
        this->force.vy -= 0x71C;
    }
    if (frame == 14) {
        Gokumon_spawnTrails(this, 8, 0x13, Gokumon_getSpecialTrailColor(this));
    }
    this->attackParts = 0x20;
}

/* Spawns the special move 1's hitbox. */
void Gokumon_spawnSpecial1Hitbox(Fighter *this) {
    Entity *child = allocEntityMemory(0xB0);

    if (child != NULL) {
        GokumonSpecial1Hitbox_init(entityOperatorNewAt(0xB0, child), this);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", Gokumon_special2);

/* Special move 3: Fighter's; frame 0 voice 0x116, 10 invulnerable frames and a gauge unit spent; frame 4 the hitbox,
 * from frame 5 func_800C00EC; every frame the burst, and (JP only) the clip stopped when the round is over. */
void Gokumon_special3(Fighter *this, s32 frame) {
    Fighter_special3(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0x116);
        this->invulnerableFrames = 10;
        Fighter_addGauge(this, -0x10000);
    }
    if (frame == 4) {
        Gokumon_spawnSpecial3Hitbox(this);
    }
    if (frame >= 5) {
        func_800C00EC(this);
    }
    Gokumon_spawnSpecial3Burst(this);
#if VERSION_JP
    if (Fight_isRoundOver(Fighter_getScene(this))) {
        Fighter_stopActionClip(this);
    }
#endif
}

/* Special move 3 in the air: as on the ground, with a push up of 0x71C every frame. */
void Gokumon_airSpecial3(Fighter *this, s32 frame) {
    Fighter_airSpecial3(this, frame);
    if (frame == 0) {
        Fighter_playVoice(this, 0x116);
        this->invulnerableFrames = 10;
        Fighter_addGauge(this, -0x10000);
    }
    if (frame == 4) {
        Gokumon_spawnSpecial3Hitbox(this);
    }
    if (frame >= 5) {
        func_800C00EC(this);
    }
    this->force.vy -= 0x71C;
    Gokumon_spawnSpecial3Burst(this);
#if VERSION_JP
    if (Fight_isRoundOver(Fighter_getScene(this))) {
        Fighter_stopActionClip(this);
    }
#endif
}

/* Spawns the special move 3's hitbox at the fighter. */
void Gokumon_spawnSpecial3Hitbox(Fighter *this) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    child = allocEntityMemory(0x2AC);
    if (child != NULL) {
        GokumonSpecial3Hitbox_init(entityOperatorNewAt(0x2AC, child), this, &position);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", func_800C00EC);

/* Spawns the special move 3's burst (ParticleBurst_init) at the fighter. */
void Gokumon_spawnSpecial3Burst(Fighter *this) {
    Vec3 position;
    Entity *child;

    Fighter_getPosition(this, &position);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &position, 0x50, 0x60, 1, 0x808080);
    }
    Fighter_addEntity(this, child);
}

/* A point of Unk800C0298's trail. */
typedef struct Unk800C0298Point {
    /* 0x0 */ Vec3 position;
    /* 0xC */ s32 unkC;
} Unk800C0298Point;

/* The entity func_800C0298 constructs for a character (virtual table D_8006CE24): the character's last eight
 * positions, kept in a ring while it records and dropped one by one after. */
typedef struct Unk800C0298 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *owner;
    /* 0x1C */ Unk800C0298Point trail[8];
    /* 0x9C */ s32 fading; /* 0 while it records */
    /* 0xA0 */ s32 newest; /* the trail's newest point */
    /* 0xA4 */ s32 count;  /* the points in use, up to 8 */
    /* 0xA8 */ s32 unkA8;  /* whether the character faced left (Fighter_getFacing) */
    /* 0xAC */ s32 frames; /* frames recorded */
    /* 0xB0 */ s32 vramY;  /* Fighter_getVramY of the character */
} Unk800C0298; /* size 0xB4 */

extern EntityVtable D_8006CE24;

/* Constructs an Unk800C0298 for owner: the whole trail at the owner's position. The match depends on the pointer
 * walk over the trail (an indexed this->trail[i] folds 0x1C into the stores). */
Unk800C0298 *func_800C0298(Unk800C0298 *this, Fighter *owner) {
    Vec3 position;
    Unk800C0298Point *point;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006CE24;
    this->owner = owner;
    this->fading = 0;
    this->newest = 0;
    this->count = 0;
    this->unkA8 = Fighter_getFacing(owner) < 1;
    this->frames = 0;
    this->vramY = Fighter_getVramY(owner);
    Fighter_getPosition(owner, &position);
    point = this->trail;
    for (i = 0; i < 8; i++) {
        point->position = position;
        point++;
    }
    return this;
}

/* The first method of Unk800C0298 (virtual table D_8006CE24): for 9 frames writes the owner's position to the next
 * point of the ring, growing the count to 8; then drops a point a frame and deletes itself at none. */
void func_800C0344(Unk800C0298 *this) {
    if (this->fading == 0) {
        if (++this->newest >= 8) {
            this->newest = 0;
        }
        if (++this->count >= 8) {
            this->count = 8;
        }
        Fighter_getPosition(this->owner, &this->trail[this->newest].position);
        if (++this->frames >= 9) {
            this->fading++;
        }
    } else if (--this->count <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD1C);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD5C);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", func_800C0434);

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD60);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial1Hitbox_init);

/* The special move 1 hitbox's destructor: stops its sound, takes the part out of its list, then destroys the entity. */
void GokumonSpecial1Hitbox_destroy(GokumonSpecial1Hitbox *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &GOKUMON_SPECIAL1_HITBOX_VTABLE;
    stopSound(this->soundHandle);
    this->parts.part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.link;
    this->parts.part.link.next->prev = this->parts.part.link.prev;
    node->prev->next = this->parts.part.link.next;
    this->parts.part.link.next = node;
    this->parts.part.link.prev = node;
    this->parts.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial1Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial1Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial3Hitbox_init);

/* The special move 3 hitbox's destructor: the same as the special move 1 hitbox's. */
void GokumonSpecial3Hitbox_destroy(GokumonSpecial3Hitbox *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &GOKUMON_SPECIAL3_HITBOX_VTABLE;
    stopSound(this->soundHandle);
    this->parts.part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->parts.part.node), in the access order the game's compiler used */
    node = &this->parts.part.link;
    this->parts.part.link.next->prev = this->parts.part.link.prev;
    node->prev->next = this->parts.part.link.next;
    this->parts.part.link.next = node;
    this->parts.part.link.prev = node;
    this->parts.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial3Hitbox_tick);

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD70);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", func_800C1180);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", func_800C12A4);

INCLUDE_ASM("game/nonmatchings/fighters/gokumon", GokumonSpecial3Hitbox_draw);

/* the frame of func_800C16F8's sprite */
extern u16 D_8006CD94[];

/* the virtual table of func_800C16F8's sprite class */
extern EntityVtable D_8006CDAC;

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CD94);

/* Constructs a sprite entity of the class of vtable D_8006CDAC: Particle_init with the frame D_8006CD94, size 0x40
 * and grey. */
Particle *func_800C16F8(Particle *this, VECTOR *pos, VECTOR *vel, s32 flags) {
    Particle_init(this, D_8006CD94, pos, vel, 0x40, 0x808080, flags);
    this->entity.vtable = &D_8006CDAC;
    return this;
}

/* The draw of func_800C16F8's sprite: a square of side 2 * size around the origin, placed at the sprite's position by a
 * translation matrix, with buildPolyFT4.
 * The match depends on the matrix built as matrixInitTranslationVec does, reading the position through a pointer. */
void func_800C1758(Particle *this, Camera *camera) {
    MatrixWords m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u_long *ot;
    s32 shift;
    VECTOR *pos;
    s32 x, y, z;
    s32 size;

    pos = &this->sprite.position;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    ot = camera->ot.slots;
    shift = camera->zShift;
    setModelMatrix(&m.m);
    size = this->sprite.size;
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    shape.flags = 0;
    corners[0].vx = corners[2].vx = -size;
    corners[1].vx = corners[3].vx = size;
    corners[0].vy = corners[1].vy = -size;
    corners[2].vy = corners[3].vy = size;
    Particle_buildShape(this, &shape, corners);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, shift);
}

/* Gokumon's destructor. */
void Gokumon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CE24, which func_800C0298 installs. */
void func_800C1888(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800C18D0(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CDAC);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", GOKUMON_SPECIAL3_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", GOKUMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", D_8006CE24);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", GOKUMON_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", GOKUMON_ATTACK_TRAIL_COLORS);
INCLUDE_RODATA("game/nonmatchings/fighters/gokumon", GOKUMON_SPECIAL_TRAIL_COLORS);
