/* Sakuyamon (CHARACTER_SAKUYAMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the attacks' trails, the special move 1's controller and its four hitboxes, the special move 2's hitbox, the
 * special move 3's effect, and the intro's hitbox. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

void Sakuyamon_spawnTrail(Fighter *this, s32 arg);
void func_800A7290(Entity *this, s32 arg);
void func_800A7458(Entity *this, s32 arg);
void func_800A8C24(Entity *this, s32 arg);
void func_800A8F80(Entity *this, s32 arg);
extern VtableEntry D_8006AF2C[];
extern EntityVtable D_8006AF4C;
extern VtableEntry D_8006AFC4[];
extern EntityVtable D_8006AFE4;
extern EntityVtable D_8006B00C;
extern EntityVtable D_8006B03C;

/* Sakuyamon's class (CHARACTER_SAKUYAMON): special1 keeps the handles of its four hitboxes (func_800A5B98). */
typedef struct Sakuyamon {
    /* 0x000 */ Fighter base;
    /* 0x4A4 */ s32 unk4A4;
    /* 0x4A8 */ s32 childHandles[4]; /* entity handles from func_800A5B98 */
    /* 0x4B8 */ s32 unk4B8;
    /* 0x4BC */ s32 unk4BC;
} Sakuyamon;

/* The entity class of func_800A6220 (virtual table D_8006B03C), which func_800AA0B4's derives from. */
typedef struct Unk800A6220 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ InputHistory *input; /* the character's (Fighter_getInput) */
    /* 0x20 */ Vec3 position;      /* the character's, at construction */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ s32 unk30; /* 96 the way the character faces */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800A6220;

Unk800A6220 *func_800A6220(Unk800A6220 *this, Fighter *fighter);

extern EntityVtable SAKUYAMON_VTABLE;

/* The classes of D_8006AF2C and D_8006AE8C (virtual tables of their base at
 * 0x18), whose methods func_800A7F88 and func_800A9944 are. */
typedef struct Unk800A7F88 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Hit unk18;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ CollisionSphere part;
    /* 0x80 */ s32 unk80;
    /* 0x84 */ Vec3 start; /* func_800A7FEC puts the part at start + unk94 * unkB0 / 4 (its asm) */
    /* 0x90 */ s32 startPad;
    /* 0x94 */ Vec3 unk94;
    /* 0xA0 */ u8 unkA0[0xA8 - 0xA0];
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
} Unk800A7F88;

extern VtableEntry D_8006AE8C[];
extern EntityVtable D_8006AEAC;

typedef struct SakuyamonSpecial3Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fighter *fighter;
    /* 0x1C */ VECTOR position; /* the character's node 19 (offset D_8006B198), while state is below 3 */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;   /* + 0x20 a frame */
    /* 0x3C */ s32 unk3C;   /* 0 to 5, a step a frame */
    /* 0x40 */ s32 state;   /* 0 to 3 (SakuyamonSpecial3Effect_tick) */
    /* 0x44 */ s32 frames;
    /* 0x48 */ s32 vramY; /* Fighter_getVramY of the character */
} SakuyamonSpecial3Effect;

extern EntityVtable SAKUYAMON_SPECIAL3_EFFECT_VTABLE;

typedef struct Unk800AA04C {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ Vec3 unk20;
} Unk800AA04C;
extern TrailPoint D_8006B1AC[];

/* Builds Sakuyamon: a Fighter with its own vtable, its seven words 0. The game clears the handles last to first: the
 * match depends on the chained assignment. */
Sakuyamon *Sakuyamon_init(Sakuyamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &SAKUYAMON_VTABLE;
    this->unk4B8 = 0;
    this->unk4A4 = 0;
    this->childHandles[0] = this->childHandles[1] = this->childHandles[2] = this->childHandles[3] = 0;
    this->unk4BC = 0;
    return this;
}

/* Ends a move: func_80079B0C(20) while 0x4A4 is set. */
void Sakuyamon_endMove(Sakuyamon *this) {
    if (this->unk4A4 != 0) {
        func_80079B0C(&this->base, 20);
    }
}

/* Attack 1: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_attack1(Fighter *this, s32 frame) {
    Fighter_attack1(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Attack 2: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Attack 3: Fighter's, with a trail (20) at frame 0. */
void Sakuyamon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 20);
    }
}

/* Up attack: Fighter's, with a trail (10) at frame 0. */
void Sakuyamon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 10);
    }
}

/* Dash attack: Fighter's, with a trail (8) at frame 15. */
void Sakuyamon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 15) {
        Sakuyamon_spawnTrail(this, 8);
    }
}

/* Up attack in the air: Fighter's, with a trail (17) at frame 0. */
void Sakuyamon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    if (frame == 0) {
        Sakuyamon_spawnTrail(this, 17);
    }
}

/* Spawns the attacks' trail (Trail_init with D_8006B1AC, 3, 2) lasting length. */
void Sakuyamon_spawnTrail(Fighter *this, s32 length) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        Trail_init(entityOperatorNewAt(0x690, child), this, D_8006B1AC, 3, 2, length);
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", Sakuyamon_throw);
extern VECTOR D_8006AB8C;

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AB8C);

/* func_80079E50 with D_8006AB8C, 0x100000 and 0x2AA. */
void func_800A58C8(Fighter *this) {
    func_80079E50(this, &D_8006AB8C, 0x100000, 0x2AA);
}

/* Sakuyamon_special1's four corners (+-0x200, +-0x200), passed in pairs to func_800A5B98. One array: the game
 * derives the second address from the first (a1 + 0x10, a1 - 0x30). */
extern VECTOR D_8006AB9C[4];

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AB9C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABAC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABBC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ABCC);

void func_800A5AD0(Sakuyamon *this);
s32 func_800A5B98(Sakuyamon *this, VECTOR *from, VECTOR *to);

/* Special move 1: frame 0 the voice and clip 22, frame 15 the controller (func_800A5AD0), frames 16, 21, 26 and 31 keep
 * the handle of a hitbox for a pair of corners (func_800A5B98); the force brakes half the horizontal velocity. From
 * frame 34 it clears the handles of dead hitboxes and ends with Fighter_stopActionClip, unless button 0x40 is up, the
 * fighter is on the ground, one hitbox lives, Fight_isRoundOver does not hold and either func_800777BC holds or the
 * gauge is positive. */
void Sakuyamon_special1(Sakuyamon *this, s32 frame) {
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xD8);
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_SPECIAL1, 0, 0, 32);
        break;
    case 15:
        func_800A5AD0(this);
        break;
    case 16:
        this->childHandles[0] = func_800A5B98(this, &D_8006AB9C[0], &D_8006AB9C[1]);
        break;
    case 21:
        this->childHandles[1] = func_800A5B98(this, &D_8006AB9C[1], &D_8006AB9C[0]);
        break;
    case 26:
        this->childHandles[2] = func_800A5B98(this, &D_8006AB9C[2], &D_8006AB9C[1]);
        break;
    case 31:
        this->childHandles[3] = func_800A5B98(this, &D_8006AB9C[3], &D_8006AB9C[0]);
        break;
    }
    this->base.force.vx -= this->base.velocity.vx >> 1;
    if (frame >= 34) {
        if (!(InputHistory_getPressed(Fighter_getInput(&this->base)) & 0x40) && Fighter_isOnGround(&this->base)) {
            s32 alive = 0;
            s32 i;

            for (i = 0; i < 4; i++) {
                if (getEntityByHandle(this->childHandles[i]) != NULL) {
                    alive++;
                } else {
                    this->childHandles[i] = 0;
                }
            }
            if (alive != 0 && !Fight_isRoundOver(Fighter_getScene(&this->base))
                && (func_800777BC(&this->base) || func_8007758C(&this->base)->gauge > 0)) {
                return;
            }
        }
        Fighter_stopActionClip(&this->base);
    }
}

/* func_800AA0B4's body: the constructor of the class of vtable D_8006B00C, derived from func_800A6220's, which
 * g++ inlined into func_800A5AD0. */
static inline Unk800A6220 *constructUnk800AA0B4(Unk800A6220 *this, Fighter *fighter) {
    func_800A6220(this, fighter);
    this->entity.vtable = &D_8006B00C;
    return this;
}

/* Creates the entity of func_800A6220's class, or of its derived class (D_8006B00C) when Fighter_isComputer, hands it
 * to Fighter_addEntity and keeps its handle at 0x4A4, when it was allocated. */
void func_800A5AD0(Sakuyamon *this) {
    Entity *entity;

    if (Fighter_isComputer(&this->base) == 0) {
        entity = NEW_ENTITY(0x40, func_800A6220, &this->base);
    } else {
        entity = NEW_ENTITY(0x40, constructUnk800AA0B4, &this->base);
    }
    if (entity != NULL) {
        Fighter_addEntity(&this->base, entity);
        this->unk4A4 = entityGetHandle(entity);
    }
}

UnkPartsEntity38 *func_800A65F8(UnkPartsEntity38 *this, Fighter *fighter, s32 handle, VECTOR *from, VECTOR to);
Entity *func_800A6EB8(Entity *this, s32 handle);
Entity *func_800A7694(Entity *this, s32 handle, s32 unk);

/* Builds the entities for a pair of corners while the entity at 0x4A4 is alive: func_800A65F8's (0xD8 bytes),
 * then func_800A6EB8's (0xC8) on its handle and func_800A7694's two (0x78, 0 and 1) on that one's handle, each
 * handed to Fighter_addEntity; plays sound 0xD1. Returns the first one's handle, or 0. */
s32 func_800A5B98(Sakuyamon *this, VECTOR *from, VECTOR *to) {
    Entity *parts;
    s32 handle;

    if (getEntityByHandle(this->unk4A4) == NULL) {
        return 0;
    }
    handle = this->unk4A4;
    parts = NEW_ENTITY(0xD8, func_800A65F8, &this->base, handle, from, *to);
    if (parts != NULL) {
        Entity *entity;

        Fighter_addEntity(&this->base, parts);
        entity = NEW_ENTITY(0xC8, func_800A6EB8, entityGetHandle(parts));
        if (entity != NULL) {
            Fighter_addEntity(&this->base, entity);
            Fighter_addEntity(&this->base, NEW_ENTITY(0x78, func_800A7694, entityGetHandle(entity), 0));
            Fighter_addEntity(&this->base, NEW_ENTITY(0x78, func_800A7694, entityGetHandle(entity), 1));
        }
        Fighter_playSound(&this->base, 0xD1);
        return entityGetHandle(parts);
    }
    return 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", Sakuyamon_special2);

void func_800A5F94(Sakuyamon *this);
void func_800A605C(Sakuyamon *this, Vec3 *velocity);

/* Special move 2 in the air: Fighter's; the voice at frame 0, the hitbox at frame 4 (func_800A5F94), thrown forward and
 * down by 128 at frame 16 (func_800A605C). The force brakes to a quarter of the horizontal velocity, and stops a fall
 * with a lift of 0x71C times the mass (16.16). */
void Sakuyamon_airSpecial2(Sakuyamon *this, s32 frame) {
    Vec3 velocity;

    Fighter_airSpecial2(&this->base, frame);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0xD9);
        break;
    case 4:
        func_800A5F94(this);
        break;
    case 16:
        velocity.vx = Fighter_getFacing(&this->base) * 128;
        velocity.vy = 128;
        velocity.vz = 0;
        func_800A605C(this, &velocity);
        break;
    }
    this->base.force.vx = -this->base.velocity.vx >> 2;
    if (this->base.velocity.vy > 0) {
        /* The match depends on lift being a local of its own (computed inline, other registers) */
        s32 lift = ((s64)this->base.mass * 0x71C) >> 16;

        this->base.force.vy = -this->base.velocity.vy - lift;
    }
}

Entity *func_800A7C90(void *this, Fighter *fighter, VECTOR *point, Vec3 *position);

/* Creates func_800A7C90's entity (0xB8 bytes) at node 2 and at the character's position moved 384 the way it
 * faces, hands it to Fighter_addEntity and keeps its handle at 0x4B8, when it was allocated. */
void func_800A5F94(Sakuyamon *this) {
    VECTOR point;
    Vec3 position;
    Entity *entity;

    Fighter_getNodePosition(&this->base, &point, 2, &ZERO_VECTOR);
    Fighter_getPosition(&this->base, &position);
    position.vx += Fighter_getFacing(&this->base) * 384;
    entity = NEW_ENTITY(0xB8, func_800A7C90, &this->base, &point, &position);
    if (entity != NULL) {
        Fighter_addEntity(&this->base, entity);
        this->unk4B8 = entityGetHandle(entity);
    }
}

void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8);

/* Throws the entity of handle 0x4B8, when it still exists (func_800A80B4 with velocity), and plays sound
 * 0xD2. */
void func_800A605C(Sakuyamon *this, Vec3 *velocity) {
    Entity *entity = getEntityByHandle(this->unk4B8);

    if (entity != NULL) {
        func_800A80B4((Unk800A7F88 *)entity, (VECTOR *)velocity, 0x47AC1400);
        Fighter_playSound(&this->base, 0xD2);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", Sakuyamon_special3);

/* the offset from node 19 where Sakuyamon_spawnSpecial3Effect spawns the effect */
extern VECTOR D_8006B198;

SakuyamonSpecial3Effect *SakuyamonSpecial3Effect_init(SakuyamonSpecial3Effect *this, Fighter *fighter,
                                                      VECTOR *position);

/* Spawns the special move 3's effect at node 19 (offset D_8006B198), then Fighter_addGauge(-0x10000) and its sound. */
void Sakuyamon_spawnSpecial3Effect(Fighter *this) {
    VECTOR position;
    Entity *child;

    Fighter_getNodePosition(this, &position, 19, &D_8006B198);
    child = allocEntityMemory(0x4C);
    if (child != NULL) {
        SakuyamonSpecial3Effect_init(entityOperatorNewAt(0x4C, child), this, &position);
    }
    Fighter_addEntity(this, child);
    Fighter_addGauge(this, -0x10000);
    Fighter_playSound(this, 0xD3);
}

/* Constructs an entity that follows the character: its input, its position, and 96 the way it faces. */
Unk800A6220 *func_800A6220(Unk800A6220 *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006B03C;
    this->fighter = fighter;
    this->input = Fighter_getInput(fighter);
    func_8007A650(fighter, &this->position);
    this->unk30 = Fighter_getFacing(fighter) * 96;
    this->unk34 = 0;
    this->unk38 = 0;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A62A0);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A63D8);

/* Method 0x20 of the computer's class (D_8006B00C): the flat way, 96 long, from position to the opponent's top. */
VECTOR *func_800A6550(Unk800A6220 *this, VECTOR *out) {
    s32 x, y, z;

    /* func_8007A6CC writes only vx, vy and vz */
    func_8007A6CC(this->fighter, (Vec3 *)out);
    /* The match depends on the vecSubInPlace shape: read out whole, subtract, then store. */
    x = out->vx;
    y = out->vy;
    z = out->vz;
    x -= this->position.vx;
    y -= this->position.vy;
    z -= this->position.vz;
    out->vx = x;
    out->vy = y;
    out->vz = z;
    vecNormalize(out, out);
    out->vz = 0;
    out->vx = (out->vx * 96) >> 12;
    out->vy = (out->vy * 96) >> 12;
    return out;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A65F8);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6778);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A68FC);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6AB0);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6BE4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6CF4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6EB8);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A6F8C);

/* The draw of the entities of D_8006AF9C: func_800A7290, then func_800A7458. */
void func_800A7254(Entity *this, s32 arg) {
    func_800A7290(this, arg);
    func_800A7458(this, arg);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7290);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7458);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7694);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7768);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A79D0);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7C90);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7DD4);

/* The second base's Hit_apply with arg, then disables the part when getHitFighter finds an entity in arg and 0xAC is
 * 0. */
void func_800A7F88(Unk800A7F88 *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL && this->unkAC == 0) {
        this->part.flags |= COLLISION_SHAPE_DISABLED;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A7FEC);

/* Keeps vec at 0x94, clears 0xA8 and 0xB0, counts the call in 0xAC, and sets the second base's vector to
 * vec's unit direction with x doubled, y quadrupled and z cleared, and its word at 0x8 to unk8. */
void func_800A80B4(Unk800A7F88 *this, VECTOR *vec, s32 unk8) {
    VECTOR dir;
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unkA8 = 0;
    this->unkB0 = 0;
    this->unk94.vx = x;
    this->unk94.vy = y;
    this->unk94.vz = z;
    this->unkAC++;
    vecNormalize(&dir, vec);
    dir.vx <<= 1;
    dir.vy <<= 2;
    dir.vz = 0;
    Hit_setKnockback(&this->unk18, &dir);
    Hit_set(&this->unk18, unk8, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8150);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AC5C);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8274);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A83E0);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A84E4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A86A8);

/* Builds the special move 3's effect for fighter, its fields 0; its one caller (Sakuyamon_spawnSpecial3Effect) also
 * passes a position, which it does not read. */
SakuyamonSpecial3Effect *SakuyamonSpecial3Effect_init(SakuyamonSpecial3Effect *this, Fighter *fighter,
                                                      VECTOR *position) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &SAKUYAMON_SPECIAL3_EFFECT_VTABLE;
    this->fighter = fighter;
    this->position.vx = 0;
    this->position.vy = 0;
    this->position.vz = 0;
    this->position.pad = 0;
    this->unk2C = 0;
    this->unk30 = 0;
    this->unk34 = 0;
    this->unk38 = 0;
    this->unk3C = 0;
    this->state = 0;
    this->frames = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", SakuyamonSpecial3Effect_tick);

/* The draw of the entities of SAKUYAMON_SPECIAL3_EFFECT_VTABLE: func_800A8C24, then func_800A8F80. */
void SakuyamonSpecial3Effect_draw(Entity *this, s32 arg) {
    func_800A8C24(this, arg);
    func_800A8F80(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AC7C);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8C24);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACC4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACCC);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A8F80);

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006ACD4);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A926C);

/* The destructor of func_800A926C's class: stops its sound, takes its member at 0x38 out of its list. */
void func_800A9428(Unk800A9944 *this, s32 flags) {
    ListNode *link;

    this->unk18.vtable = D_8006AE8C;
    this->entity.vtable = &D_8006AEAC;
    stopSound(this->unkD4);
    this->unk38.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk38.node) written out, as in st03's func_800D8D24 */
    link = &this->unk38.link;
    this->unk38.link.next->prev = this->unk38.link.prev;
    link->prev->next = this->unk38.link.next;
    this->unk38.link.next = link;
    this->unk38.link.prev = link;
    this->unk18.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Plays sound 0xD4 and keeps its handle at 0xD4 (func_800A926C's class; the destructor stops it). */
void func_800A94D0(Unk800A9944 *this) {
    this->unkD4 = soundControlPlaySound(&SOUND_CONTROL, 0xD4, 0);
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9508);

/* The second base's Hit_apply with arg, then halves 0xB4 when getHitFighter finds an entity in arg. */
void func_800A9944(Unk800A9944 *this, void *arg) {
    Hit_apply(&this->unk18, arg);
    if (getHitFighter(arg) != NULL) {
        this->unkB4 >>= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9998);

/* The class func_800A9C4C constructs (virtual table D_8006AE64). */
typedef struct Unk800A9C4C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* three RANDOM values */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38; /* eases towards 0x804 (func_800A9D0C) */
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 vramY; /* Fighter_getVramY of the owner */
} Unk800A9C4C;

extern EntityVtable D_8006AE64;

/* Constructs the entity of virtual table D_8006AE64 for fighter at position, with three random words. */
Unk800A9C4C *func_800A9C4C(Unk800A9C4C *this, Fighter *fighter, Vec3 *position) {
    s32 x, y, z;
    s32 random0, random1, random2;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006AE64;
    /* The match depends on reading position whole (a struct copy: 26 diffs) and on drawing the three random
     * words before storing any. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    random0 = mersenneTwisterGenerate(&RANDOM);
    random1 = mersenneTwisterGenerate(&RANDOM);
    random2 = mersenneTwisterGenerate(&RANDOM);
    this->unk28 = random0;
    this->unk2C = random1;
    this->unk30 = random2;
    this->unk38 = 0;
    this->unk3C = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The tick of func_800A9C4C's entity: 0x38 eases a quarter of the way to 0x804 every frame, and the
 * entity deletes itself once it reaches 0x800. */
void func_800A9D0C(Unk800A9C4C *this) {
    this->unk38 += (0x804 - this->unk38) >> 2;
    if (this->unk38 >= 0x800) {
        entityDelete(&this->entity);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE40);

INCLUDE_ASM("game/nonmatchings/fighters/sakuyamon", func_800A9D70);

/* Sakuyamon's destructor. */
void Sakuyamon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006B03C, which func_800A6220 installs. */
void func_800AA004(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Copies the vector at 0x20 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800AA04C(Unk800AA04C *this, Vec3 *out) {
    s32 x = this->unk20.vx;
    s32 y = this->unk20.vy;
    s32 z = this->unk20.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* The destructor of the entity class of vtable D_8006B00C, which func_800A5AD0 and func_800AA0B4 install. */
void func_800AA06C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006B00C, which derives from func_800A6220's. */
Unk800A6220 *func_800AA0B4(Unk800A6220 *this, Fighter *fighter) {
    return constructUnk800AA0B4(this, fighter);
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AFE4 and D_8006AFC4, which
 * func_800A65F8 installs. */
void func_800AA0E8(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AFC4;
    this->entity.vtable = &D_8006AFE4;
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

/* The destructor of the entity class of vtable D_8006AF9C, which func_800A6EB8 installs. */
void func_800AA188(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AF74, which func_800A7694 installs. */
void func_800AA1D0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006AF4C and D_8006AF2C, which
 * func_800A7C90 installs. */
void func_800AA218(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006AF2C;
    this->entity.vtable = &D_8006AF4C;
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

/* The destructor of the entity class of vtable D_8006AF04, which func_800A83E0 installs. */
void func_800AA2B8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 3 effect's destructor. */
void SakuyamonSpecial3Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006AE64, which func_800A9C4C installs. */
void func_800AA348(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE64);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AE8C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AEAC);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", SAKUYAMON_SPECIAL3_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF04);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF2C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF4C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF74);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AF9C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AFC4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006AFE4);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B00C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B03C);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", SAKUYAMON_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B198);
INCLUDE_RODATA("game/nonmatchings/fighters/sakuyamon", D_8006B1AC);
