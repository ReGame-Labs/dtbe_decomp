/* Patamon (CHARACTER_PATAMON): the fighter createFighter builds for it, its moves (a throw that moves the opponent
 * along a line), and the entities they spawn: the special move 1's hitbox (and its children) and the special move
 * 2's effect. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"

extern FaceEntry PATAMON_FACES[];
extern EntityVtable D_8006A784;
extern EntityVtable D_8006A7AC;
extern EntityVtable PATAMON_VTABLE;
Entity *PatamonSpecial1Hitbox_init(Entity *this, Fighter *chara, Vec3 *position, Vec3 *velocity, Fight *unk228);

/* Builds Patamon: a Fighter with its own vtable and faces. */
Patamon *Patamon_init(Patamon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &PATAMON_VTABLE;
    Fighter_setFaceTable(&this->base, PATAMON_FACES);
    this->special1Active = 0;
    return this;
}

/* Patamon's destructor. */
void Patamon_destroy(Entity *this, s32 flags) {
    this->vtable = &PATAMON_VTABLE;
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Ends a move: nothing to do. */
void Patamon_endMove(void) {
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_intro);

/* Sets out's x and y to the point at t (4.12) of the way from from to to. */
void func_800A1160(Patamon *this, VECTOR *from, VECTOR *to, s32 t, VECTOR *out) {
    out->vx = from->vx + (((to->vx - from->vx) * t) >> 12);
    out->vy = from->vy + (((to->vy - from->vy) * t) >> 12);
}

/* Clamps 0x4B8 to 0..0x1000 and puts the character's 0x3C8 that far from 0x4CC to 0x4DC (func_800A1160). */
void func_800A11B0(Patamon *this) {
    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &this->base.unk3C8);
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_throw);

/* Clamps 0x4B8 to 0..0x1000 and sets chara moving (0x3B4, in 1/256ths) from its position to the point
 * that far from 0x4CC to 0x4DC (func_800A1160), clearing 0x3A4..0x3B0 and 0x3BC. */
void func_800A1760(Patamon *this, Fighter *chara) {
    Vec3 position;
    VECTOR target;

    if (this->unk4B8 < 0) {
        this->unk4B8 = 0;
    }
    if (this->unk4B8 > 0x1000) {
        this->unk4B8 = 0x1000;
    }
    Fighter_getPosition(chara, &position);
    func_800A1160(this, &this->unk4CC, &this->unk4DC, this->unk4B8, &target);
    chara->force.vz = 0;
    chara->velocity.vx = 0;
    chara->velocity.vy = 0;
    chara->velocity.vz = 0;
    chara->velocityPad = 0;
    chara->force.vx = (target.vx - position.vx) << 8;
    chara->force.vy = (target.vy - position.vy) << 8;
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_special1);

void Patamon_spawnSpecial1Hitbox(Fighter *this);

/* Special move 1 in the air: at frame 0 stops when the hitbox is still out (0x4A4), else the voice and the clip; the
 * hitbox at frame 3; before frame 17 the force brakes a sixteenth of the horizontal velocity and any fall. */
void Patamon_airSpecial1(Patamon *this, s32 frame) {
    if (frame == 0) {
        if (this->special1Active != 0) {
            Fighter_stopActionClip(&this->base);
            return;
        }
        Fighter_playVoice(&this->base, 0xC9);
        Fighter_playActionClip(&this->base, FIGHTER_CLIP_AIR_SPECIAL1, 2, 0, -1);
    }
    if (frame == 3) {
        Patamon_spawnSpecial1Hitbox(&this->base);
    }
    if (frame < 17) {
        this->base.force.vx -= this->base.velocity.vx >> 4;
        if (this->base.velocity.vy > 0) {
            this->base.force.vy -= this->base.velocity.vy;
        }
    }
}

/* Spawns the special move 1's hitbox 64 in front of the fighter and 0x60 up, 96 a frame forward, then its sound and
 * func_80079B0C(40). */
void Patamon_spawnSpecial1Hitbox(Fighter *this) {
    s32 facing = Fighter_getFacing(this);
    Fight *unk228;
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    Fighter_getPosition(this, &position);
    velocity.vx = facing * 96;
    velocity.vy = 0;
    velocity.vz = 0;
    position.vx += facing * 64;
    position.vy -= 0x60;
    unk228 = Fighter_getScene(this);
    child = allocEntityMemory(0xC8);
    if (child != NULL) {
        PatamonSpecial1Hitbox_init(entityOperatorNewAt(0xC8, child), this, &position, &velocity, unk228);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0xC6);
    func_80079B0C(this, 40);
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", Patamon_special2);

/* an axis next to AXIS_Y and AXIS_Z in the executable's data */
extern VECTOR D_80010874;

/* Whether the character Fighter_getOpponent gives is in reach: when Fighter_isThrowable holds for it, what
 * func_80079E50 finds within both func_80079DD8 sizes plus 0xA0 (squared) around AXIS_Z, else around
 * D_80010874; NULL otherwise. */
Fighter *func_800A1CDC(Fighter *this) {
    Fighter *opponent = Fighter_getOpponent(this);
    s32 reach;
    s32 range;

    if (!Fighter_isThrowable(opponent)) {
        return NULL;
    }
    reach = func_80079DD8(this) + func_80079DD8(opponent) + 0xA0;
    range = reach * reach;
    /* The match depends on reusing opponent for the character found (a local of its own stays in $v0) */
    opponent = func_80079E50(this, &AXIS_Z, range, 0x200);
    if (opponent != NULL) {
        return opponent;
    }
    return func_80079E50(this, &D_80010874, range, 0x200);
}

/* A value by the opponent's character: 0x3FFC for Patamon, 0x4718 for Renamon, Seraphimon and Wormmon, 0x4E34 for any
 * other. */
s32 func_800A1D84(Fighter *this) {
    s32 value;

    switch (Fighter_getCharacter(Fighter_getOpponent(this))) {
    case CHARACTER_PATAMON:
        value = 0x3FFC;
        break;
    case CHARACTER_RENAMON:
    case CHARACTER_SERAPHIMON:
    case CHARACTER_WORMMON:
        value = 0x4718;
        break;
    default:
        value = 0x4E34;
        break;
    }
    return value;
}

/* Creates an entity of the class of vtable D_8006A784 (0x120 bytes) with g++'s new, and hands it
 * to Fighter_addEntity. Its constructor func_800A2F64 is inlined. */
void func_800A1DFC(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        Entity *e = entityOperatorNewAt(0x120, child);

        func_800BD490(e, this);
        e->vtable = &D_8006A784;
    }
    Fighter_addEntity(this, child);
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", PatamonSpecial1Hitbox_init);

/* The special move 1's hitbox (0xC8 bytes): while it lives its fighter's special1Active is set. */
typedef struct PatamonSpecial1Hitbox {
    /* 0x00 */ UnkPartsEntity3C parts;
    /* 0x84 */ Patamon *chara;
} PatamonSpecial1Hitbox;

extern EntityVtable PATAMON_SPECIAL1_HITBOX_VTABLE;

/* The special move 1 hitbox's destructor: clears the fighter's special1Active and calls its func_80079B0C(1). */
void PatamonSpecial1Hitbox_destroy(PatamonSpecial1Hitbox *this, s32 flags) {
    ListNode *node;

    this->parts.entity.vtable = &PATAMON_SPECIAL1_HITBOX_VTABLE;
    this->chara->special1Active = 0;
    func_80079B0C(&this->chara->base, 1);
    this->parts.part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->parts.part.link;
    this->parts.part.link.next->prev = this->parts.part.link.prev;
    node->prev->next = this->parts.part.link.next;
    this->parts.part.link.next = node;
    this->parts.part.link.prev = node;
    this->parts.unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->parts.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", PatamonSpecial1Hitbox_tick);

INCLUDE_ASM("game/nonmatchings/fighters/patamon", PatamonSpecial1Hitbox_draw);

INCLUDE_ASM("game/nonmatchings/fighters/patamon", func_800A2700);

/* The destructor of the entity class of vtable D_8006A7AC, which func_800A2700 installs. */
void func_800A2798(Entity *this, s32 flags) {
    this->vtable = &D_8006A7AC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/patamon", func_800A27EC);

INCLUDE_ASM("game/nonmatchings/fighters/patamon", func_800A2870);

INCLUDE_ASM("game/nonmatchings/fighters/patamon", func_800A2C70);

/* The destructor of the entity class of vtable D_8006A784, which func_800A1DFC and func_800A2F64 install. */
void func_800A2F1C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an object of the entity class of vtable D_8006A784, which derives from func_800BD490's. */
Entity *func_800A2F64(Entity *this, Fighter *arg) {
    func_800BD490(this, arg);
    this->vtable = &D_8006A784;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A714);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A71C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A724);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A72C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A734);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A73C);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A744);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A754);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_FACES);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A774);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A784);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", D_8006A7AC);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/patamon", PATAMON_VTABLE);
