/* WarGreymon (CHARACTER_WARGREYMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the attacks' trails, the throw's effect, the special move 1's hitbox, effect and trail, the special move 2's
 * effects, the hitbox of its slot 0x128 and the intro's hitbox. BlackWarGreymon derives from it. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

void WarGreymon_spawnAttackTrails(Fighter *this, s32 kind, s32 arg2, s32 arg3, s32 rgb);
Unk800A9944 *func_800A926C(Unk800A9944 *this, s32 fighter, Vec3 *position);
void func_800BE4AC(Unk800BE39C *this);
void func_800BE7D0(Entity *this, s32 arg);
void func_800BEA1C(Entity *this, s32 arg);
extern VtableEntry WARGREYMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable WARGREYMON_SPECIAL1_HITBOX_VTABLE;

extern EntityVtable WARGREYMON_VTABLE;

/* 4 bytes func_800BE39C copies to 0x94 of its entity */
extern CVECTOR D_8006C964;


extern VtableEntry D_8006C98C[];
extern EntityVtable D_8006C9AC;

/* Builds WarGreymon: a Fighter with its own vtable, its eight words 0 but 0x4B0, which is 1. */
WarGreymon *WarGreymon_init(WarGreymon *this) {
    Fighter_init(&this->base);
    this->base.entity.vtable = &WARGREYMON_VTABLE;
    this->unk4A4 = 0;
    this->unk4A8 = 0;
    this->unk4AC = 0;
    this->unk4B0 = 1;
    this->unk4B4 = 0;
    this->unk4B8 = 0;
    this->unk4BC = 0;
    this->unk4C0 = 0;
    return this;
}

/* Ends a move (also BlackWarGreymon's): when 0x4AC is 1, clears it and the z rotation's target. */
void WarGreymon_endMove(WarGreymon *this) {
    if (this->unk4AC == 1) {
        this->unk4AC = 0;
        this->base.targetRotation.vz = 0;
    }
}

/* Attack 2 (also BlackWarGreymon's): Fighter's, with the trails (16, 2, 8, grey) at frame 2. */
void WarGreymon_attack2(Fighter *this, s32 frame) {
    Fighter_attack2(this, frame);
    if (frame == 2) {
        WarGreymon_spawnAttackTrails(this, 16, 2, 8, 0x404040);
    }
}

/* Attack 3 (also BlackWarGreymon's): Fighter's, with the trails (5, 2, 6, grey) at frame 4. */
void WarGreymon_attack3(Fighter *this, s32 frame) {
    Fighter_attack3(this, frame);
    if (frame == 4) {
        WarGreymon_spawnAttackTrails(this, 5, 2, 6, 0x404040);
    }
}

/* Up attack (also BlackWarGreymon's): Fighter's, with the trails (5, 2, 8, grey) at frame 0. */
void WarGreymon_upAttack(Fighter *this, s32 frame) {
    Fighter_upAttack(this, frame);
    if (frame == 0) {
        WarGreymon_spawnAttackTrails(this, 5, 2, 8, 0x404040);
    }
}

/* Dash attack (also BlackWarGreymon's): Fighter's, with the trails (5, 3, 14, white) at frame 2. */
void WarGreymon_dashAttack(Fighter *this, s32 frame) {
    Fighter_dashAttack(this, frame);
    if (frame == 2) {
        WarGreymon_spawnAttackTrails(this, 5, 3, 14, 0xFFFFFF);
    }
}

/* Up attack in the air (also BlackWarGreymon's): Fighter's, with the trails (5, 2, 8, grey) at frame 3. */
void WarGreymon_airUpAttack(Fighter *this, s32 frame) {
    Fighter_airUpAttack(this, frame);
    if (frame == 3) {
        WarGreymon_spawnAttackTrails(this, 5, 2, 8, 0x404040);
    }
}

/* Side attack in the air (also BlackWarGreymon's): Fighter's, with the trails (5, 3, 14, white) at frame 2. */
void WarGreymon_airSideAttack(Fighter *this, s32 frame) {
    Fighter_airSideAttack(this, frame);
    if (frame == 2) {
        WarGreymon_spawnAttackTrails(this, 5, 3, 14, 0xFFFFFF);
    }
}

/* Trail_init's tables for WarGreymon_spawnAttackTrails: three pairs of entries, D_800D34AC for kind 16, D_800D353C
 * otherwise. */
extern TrailPoint D_800D34AC[6];
extern TrailPoint D_800D353C[6];

void Trail_setColors(Trail *this, CVECTOR *colors, s32 count);

/* Spawns the attacks' three trails, one for each pair of entries of its table (D_800D34AC for kind 16, else
 * D_800D353C), with arg2 and arg3, colored rgb (a 0xBBGGRR word, read as its three bytes with cd 0). */
void WarGreymon_spawnAttackTrails(Fighter *this, s32 kind, s32 arg2, s32 arg3, s32 rgb) {
    TrailPoint *entries = kind == 16 ? D_800D34AC : D_800D353C;
    CVECTOR *bytes = (CVECTOR *)&rgb;
    CVECTOR color = { bytes->r, bytes->g, bytes->b };
    TrailPoint *end = entries + 6;

    do {
        Entity *entity = NEW_ENTITY(0x690, Trail_init, this, entries, 2, arg2, arg3);

        if (entity != NULL) {
            Trail_setColors((Trail *)entity, &color, 1);
            Fighter_addEntity(this, entity);
        }
        entries += 2;
    } while (entries != end);
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C844);

struct WarGreymonThrowEffect *WarGreymonThrowEffect_init(struct WarGreymonThrowEffect *this, Fighter *fighter, s32 facing);
extern s32 D_8006C844[];
void WarGreymon_holdOpponent(Fighter *this);

void WarGreymon_throw(WarGreymon *this, s32 frame) {
    Fighter *opponent;
    s32 x, y, z;

    Fighter_throw(&this->base, frame);
    opponent = Fighter_getOpponent(&this->base);
    if (frame < 8) {
        Fighter_accelerateTo(&this->base, 0);
        x = this->base.velocity.vx;
        y = this->base.velocity.vy;
        z = this->base.velocity.vz;
        opponent->velocity.vx = x;
        opponent->velocity.vy = y;
        opponent->velocity.vz = z;
        x = this->base.force.vx;
        y = this->base.force.vy;
        z = this->base.force.vz;
        opponent->force.vx = x;
        opponent->force.vy = y;
        opponent->force.vz = z;
    }
    switch (frame) {
    case 0:
        Fighter_hold(opponent, 8);
        break;
    case 8: {
        s32 clip;

        Fighter_takeHit(opponent, 0x800800, (Vec3 *)&ZERO_VECTOR, 0);
        Fighter_hold(opponent, 57);
        clip = D_8006C844[Fighter_getCharacter(opponent)];
        Fighter_setClip(opponent, clip, 0, 0);
        break;
    }
    case 30:
        if (func_8007758C(opponent)->life <= 0) {
            Fighter_stopActionClip(&this->base);
            return;
        }
        break;
    case 34:
        this->base.force.vy -= ((s64)this->base.mass * 0x7FF8) >> 16;
        break;
    case 57: {
        Entity *entity = (Entity *)NEW_ENTITY(0x28, WarGreymonThrowEffect_init, &this->base, Fighter_getFacing(&this->base));
        Fighter_addEntity(&this->base, entity);
        break;
    }
    }
    if (frame >= 34 && frame < 57) {
        if (this->base.velocity.vy > 0) {
            this->base.force.vy -= this->base.velocity.vy;
        }
    }
    if (frame >= 8 && frame < 57) {
        WarGreymon_holdOpponent(&this->base);
    }
}


/* The offset of point 16 the throw holds the opponent at. */
extern VECTOR WARGREYMON_THROW_GRIP_OFFSET;

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_THROW_GRIP_OFFSET);

/* The throw's hold (WarGreymon_throw): turns the opponent to face the fighter and puts it at point 16, raised by its
 * height, with no force or velocity, its model moved there at once. When the map is in the way across its width
 * (castMapRay), the fighter steps back. */
void WarGreymon_holdOpponent(Fighter *this) {
    VECTOR offset = WARGREYMON_THROW_GRIP_OFFSET;
    Fighter *opponent;
    s32 height;
    s32 facing;
    Vec3 grip;
    Vec3 from;
    Vec3 to;
    Vec3 *position;
    s32 x, y, z;
    Model *model;

    opponent = Fighter_getOpponent(this);
    height = Fighter_getParams(opponent)->height;
    facing = Fighter_getFacing(this);
    if (facing > 0) {
        Fighter_faceLeft(opponent, 0);
    } else {
        Fighter_faceRight(opponent, 0);
    }
    Fighter_getNodePosition(this, (VECTOR *)&grip, 16, &offset);
    grip.vz = 0;
    grip.vy += height;
    /* The match depends on the three words read before any is stored, as vecCopy does. */
    position = Fighter_getPosition(opponent, &from);
    x = position->vx;
    y = position->vy;
    z = position->vz;
    to.vy = y;
    to.vz = z;
    from.vx -= height * facing;
    to.vx = x + height * facing;
    if (castMapRay(&to, &from)) {
        Fighter_accelerateTo(this, -(Fighter_getFacing(this) * 0x71C));
    }
    Fighter_setPosition(opponent, &grip);
    opponent->force.vx = 0;
    opponent->force.vy = 0;
    opponent->force.vz = 0;
    opponent->forcePad = 0;
    opponent->velocity.vx = 0;
    opponent->velocity.vy = 0;
    opponent->velocity.vz = 0;
    opponent->velocityPad = 0;
    model = opponent->model;
    model->group.object.transform.valid = 0;
    Fighter_getOpponentPosition(this, (Vec3 *)model->group.object.transform.local.m.t);
}

/* The throw's effect: it throws the opponent of a character (Fighter_getOpponent) toward a side. */
typedef struct WarGreymonThrowEffect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 handle; /* the opponent's (entityGetHandle) */
    /* 0x1C */ Fighter *fighter;
    /* 0x20 */ s32 facing; /* Fighter_getFacing of fighter */
    /* 0x24 */ s32 y;      /* the opponent's height (Fighter_getPosition) */
} WarGreymonThrowEffect;

extern EntityVtable WARGREYMON_THROW_EFFECT_VTABLE;

/* Builds the throw's effect for fighter: keeps the opponent's handle and height, and throws the opponent
 * (Fighter_takeHit) with velocity (facing * 0xE38, 0xD548, 0). */
WarGreymonThrowEffect *WarGreymonThrowEffect_init(WarGreymonThrowEffect *this, Fighter *fighter, s32 facing) {
    Fighter *opponent;
    Vec3 vec;

    entityInit(&this->entity, 0);
    this->entity.vtable = &WARGREYMON_THROW_EFFECT_VTABLE;
    opponent = Fighter_getOpponent(fighter);
    this->handle = entityGetHandle(&opponent->entity);
    this->fighter = fighter;
    this->facing = facing;
    Fighter_getPosition(opponent, &vec);
    this->y = vec.vy;
    vec.vx = facing * 0xE38;
    vec.vy = 0xD548;
    vec.vz = 0;
    Fighter_takeHit(opponent, 0x40000000, &vec, 0);
    return this;
}

/* Follows the thrown opponent: while it falls less than 0x800 a tick its height is kept; once on the ground it is
 * stopped where it lies, hit with velocity (facing * 0x4718, -0x2AA8, 0) and raises dust both ways. The effect ends
 * then, or when the opponent is gone or falls 0x800 or more. */
void WarGreymonThrowEffect_tick(WarGreymonThrowEffect *this, s32 arg) {
    Fighter *opponent;
    VECTOR position;

    opponent = (Fighter *)getEntityByHandle(this->handle);
    if (opponent != NULL) {
        /* position is a VECTOR (the velocity below is copied into it whole); its first three words are a Vec3 */
        Fighter_getPosition(opponent, (Vec3 *)&position);
        if (position.vy - this->y >= -0x800) {
            this->y = position.vy;
            if (!Fighter_isOnGround(opponent)) {
                return;
            }
            {
                VECTOR velocity;

                memset(&velocity, 0, sizeof(velocity));
                velocity.vx = this->facing * 0x4718;
                velocity.vy = -0x2AA8;
                position = velocity;
            }
            opponent->position.vz = opponent->lastPosition.vz = 0;
            opponent->velocity.vx = 0;
            opponent->velocity.vy = 0;
            opponent->velocity.vz = 0;
            opponent->velocityPad = 0;
            opponent->force.vx = 0;
            opponent->force.vy = 0;
            opponent->force.vz = 0;
            opponent->forcePad = 0;
            Fighter_takeHit(opponent, 0x408C4000, (Vec3 *)&position, 0);
            Fighter_spawnDust(opponent, 1, 0x60, 3);
            Fighter_spawnDust(opponent, -1, 0x60, 3);
        }
    }
    entityDelete(&this->entity);
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C8B4);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C8FC);

extern TrailPoint D_8006C8B4[];
extern TrailPoint D_8006C8FC[];
struct WarGreymonSpecial1Hitbox;
struct Unk800BCAD8;
struct WarGreymonSpecial1Effect;
struct WarGreymonSpecial1Hitbox *WarGreymonSpecial1Hitbox_init(struct WarGreymonSpecial1Hitbox *this, Fighter *fighter,
                                                                Vec3 *knockback, s32 damage, s32 radius,
                                                                s32 acceleration);
struct Unk800BCAD8 *func_800BCAD8(struct Unk800BCAD8 *this, Fighter *fighter, TrailPoint *entries, s32 count,
                                   s32 arg4, s32 arg5, s32 mode);
struct WarGreymonSpecial1Effect *WarGreymonSpecial1Effect_init(struct WarGreymonSpecial1Effect *this, Fighter *fighter,
                                                                s32 targetLength, s32 tilt);
void Trail_record(Trail *this);
void Trail_fadeColors(Trail *this);

void WarGreymon_special1(WarGreymon *this, s32 frame) {
    Entity *child;
    s32 isWarGreymon;
    Vec3 *facingKnockback;
    s32 damage;
    Vec3 knockback;
    Vec3 *kb;
    s32 *handle;
    s32 i;

    Fighter_special1(&this->base, frame);
    isWarGreymon = Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON;
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, isWarGreymon ? 0x10D : 0x11F);
        this->base.rotation.vy = this->base.targetRotation.vy;
        child = NEW_ENTITY(0x6A0, func_800BCAD8, &this->base, D_8006C8B4, 3, 15, 2, 0);
        if (child != NULL) {
            Fighter_addEntity(&this->base, child);
            this->unk4A4 = entityGetHandle(child);
            Fighter_playSound(&this->base, isWarGreymon ? 0x107 : 0x119);
        }
        break;
    case 1:
        damage = 0x4A814CC;
        knockback.vx = 0;
        knockback.vy = -0x38E0;
        kb = &knockback;
        knockback.vz = 0;
        if (isWarGreymon) {
            damage = 0x4A81000;
        }
        child = NEW_ENTITY(0xA0, WarGreymonSpecial1Hitbox_init, &this->base, kb, damage, 0x100, 12);
        Fighter_addEntity(&this->base, child);
        break;
    case 12:
        child = NEW_ENTITY(0x6A0, func_800BCAD8, &this->base, D_8006C8FC, 3, 10, 7, 1);
        if (child != NULL) {
            Fighter_addEntity(&this->base, child);
            this->unk4A8 = entityGetHandle(child);
            Fighter_playSound(&this->base, isWarGreymon ? 0x107 : 0x119);
        }
        break;
    case 13:
        knockback.vx = Fighter_getFacing(&this->base) * 0x38E0;
        facingKnockback = &knockback;
        damage = 0x45A82999;
        knockback.vy = -0x4718;
        knockback.vz = 0;
        if (isWarGreymon) {
            damage = 0x45A82000;
        }
        kb = facingKnockback;
        child = NEW_ENTITY(0xA0, WarGreymonSpecial1Hitbox_init, &this->base, kb, damage, 0x1C0, 4);
        Fighter_addEntity(&this->base, child);
        break;
    }
    if ((u32)(frame - 16) < 5) {
        child = NEW_ENTITY(0x58, WarGreymonSpecial1Effect_init, &this->base, (frame - 16) * 64 + 0x380,
                           (frame - 16) * 128 - 0x40);
        Fighter_addEntity(&this->base, child);
    }
    modelUpdateAnimation(this->base.model);
    handle = &this->unk4A4;
    for (i = 0; i < 2; i++) {
        Trail *trail = (Trail *)getEntityByHandle(*handle);

        if (trail != NULL) {
            if (trail->recordTimer > 0) {
                Trail_record(trail);
                Trail_fadeColors(trail);
            }
        } else {
            *handle = 0;
        }
        handle++;
    }
    if (Fighter_isActionClipDone(&this->base)) {
        func_80079B0C(&this->base, 15);
    }
}

void func_800BC0CC(WarGreymon *this, s32 arg);
void func_800BC0E0(WarGreymon *this, s32 arg);
void func_800BC13C(WarGreymon *this);
void func_800BC258(WarGreymon *this);
void func_800BC424(WarGreymon *this, s32 angle);
s32 WarGreymon_readAimAngle(WarGreymon *this, s32 *angle, s32 from);

/* Special move 2: frame 0 the voice (its own for CHARACTER_WARGREYMON), the move started with func_800BC0CC(2) and
 * func_800BC0E0(0x18), and the angle WarGreymon_readAimAngle gives, kept when within 0x400 of straight ahead; then by its frame
 * count at 0x4C0 it turns toward the angle (frame 8), finishes (frame 9), cancels the velocity (frames 0-8), looks
 * again from frame 0x11 and runs func_800BC258 from frame 10. */
void WarGreymon_special2(WarGreymon *this, s32 frame) {
    s32 angle;
    s32 target;
    s32 facing;

    func_80079AE4(&this->base);
    if (frame == 0) {
        if (Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON) {
            Fighter_playVoice(&this->base, 0x10E);
        } else {
            Fighter_playVoice(&this->base, 0x120);
        }
        this->unk4B0 = 1;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x18);
        facing = Fighter_getFacing(&this->base);
        /* The match depends on shifting the facing at each use: the shift then comes after the
         * other arguments */
        WarGreymon_readAimAngle(this, &angle, facing << 10);
        this->unk4B8 = abs(angle) <= 0x400 ? angle : facing << 10;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.rotation.vz
                                + ((((this->unk4B8 - this->base.rotation.vz - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 9) {
        this->base.force.vx = -this->base.velocity.vx;
        this->base.force.vy = -this->base.velocity.vy;
        this->base.force.vz = -this->base.velocity.vz;
        this->base.forcePad = 0;
    }
    if (this->unk4C0 >= 0x11 && WarGreymon_readAimAngle(this, &target, this->base.rotation.vz)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 10) {
        func_800BC258(this);
    }
    if (Fighter_isActionClipDone(&this->base)) {
        this->base.targetRotation.vz = 0;
    }
    this->unk4C0++;
}

/* Special move 2 in the air: WarGreymon_special2 with func_800BC0E0(0x19) and 0x4B0 cleared first, keeping
 * WarGreymon_readAimAngle's angle as it is, and frames 3, 11 and 4 for 9, 0x11 and 10. */
void WarGreymon_airSpecial2(WarGreymon *this, s32 frame) {
    s32 angle;
    s32 target;

    func_80079AE4(&this->base);
    if (frame == 0) {
        if (Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON) {
            Fighter_playVoice(&this->base, 0x10E);
        } else {
            Fighter_playVoice(&this->base, 0x120);
        }
        this->unk4B0 = 0;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x19);
        angle = Fighter_getFacing(&this->base) << 10;
        WarGreymon_readAimAngle(this, &angle, angle);
        this->unk4B8 = angle;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.rotation.vz
                                + ((((this->unk4B8 - this->base.rotation.vz - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 3) {
        this->base.force.vx = -this->base.velocity.vx;
        this->base.force.vy = -this->base.velocity.vy;
        this->base.force.vz = -this->base.velocity.vz;
        this->base.forcePad = 0;
    }
    if (this->unk4C0 >= 11 && WarGreymon_readAimAngle(this, &target, this->base.rotation.vz)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 4) {
        func_800BC258(this);
    }
    if (Fighter_isActionClipDone(&this->base)) {
        this->base.targetRotation.vz = 0;
    }
    this->unk4C0++;
}

/* Sets 0x4AC, 0x4B4 to arg, and clears 0x448 of the character. */
void func_800BC0CC(WarGreymon *this, s32 arg) {
    this->unk4AC = 1;
    this->unk4B4 = arg;
    this->base.attackParts = 0;
}

/* When 0x4B4 is above 0: plays clip (Fighter_playActionClip), clears unk3A4 and 0x3B0, 0x4BC, 0x4C0 and the
 * character's 0x448. */
void func_800BC0E0(WarGreymon *this, s32 clip) {
    if (this->unk4B4 > 0) {
        Fighter_playActionClip(&this->base, clip, 0, 0, -1);
        this->base.velocity.vx = 0;
        this->base.velocity.vy = 0;
        this->base.velocity.vz = 0;
        this->base.velocityPad = 0;
        this->unk4BC = 0;
        this->unk4C0 = 0;
        this->base.attackParts = 0;
    }
}

/* While 0x4B4 is above 0: takes one from it and launches the fighter along its target angle (0x6388 of its mass),
 * plus a lift of 0x2AA8 of its mass while unk4B0 is set (0xE38 without), with all attack parts, its sound and
 * func_80079B14(0x32). */
void func_800BC13C(WarGreymon *this) {
    s32 push;
    s32 sin, cos;

    if (this->unk4B4 > 0) {
        push = ((s64)this->base.mass * 0x6388) >> 16;
        this->unk4B4--;
        sin = COS_SIN_TABLE[this->base.targetRotation.vz & 0xFFF].sin;
        cos = COS_SIN_TABLE[this->base.targetRotation.vz & 0xFFF].cos;
        this->base.force.vx = (push * sin) >> 12;
        this->base.force.vy = -(push * cos) >> 12;
        /* The match depends on the table read through the field in both lines, before any store, and on the lift
         * written whole in each arm (cross-jumped to one multiply). */
        if (this->unk4B0) {
            this->base.force.vy -= ((s64)this->base.mass * 0x2AA8) >> 16;
        } else {
            this->base.force.vy -= ((s64)this->base.mass * 0xE38) >> 16;
        }
        this->base.attackParts = -1;
        if (Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON) {
            Fighter_playSound(&this->base, 0x108);
        } else {
            Fighter_playSound(&this->base, 0x11A);
        }
        func_80079B14(&this->base, 0x32);
    }
}

/* Special move 2's flight (from frame 10): brakes the velocity by 1/32, lifts by unk4BC (or 0x71C of the mass
 * without unk4B0), whose lift grows by a sixth of it up to that; every other frame below 0x19 runs the
 * vtable's slot 0x120; every 4 frames resets the hit (damage by character, knockback half the velocity back);
 * every 8 frames sets hitKind to 1. */
void func_800BC258(WarGreymon *this) {
    s32 lift;
    s32 *forceY;
    s32 y;

    this->base.force.vx -= this->base.velocity.vx >> 5;
    this->base.force.vy -= this->base.velocity.vy >> 5;
    forceY = &this->base.force.vy;
    lift = ((s64)this->base.mass * 0x71C) >> 16;
    if (this->unk4B0) {
        y = this->base.force.vy - this->unk4BC;
    } else {
        y = this->base.force.vy - lift;
    }
    *forceY = y;
    this->unk4BC += lift / 6;
    if (lift < this->unk4BC) {
        this->unk4BC = lift;
    }
    if (this->unk4C0 < 0x19 && (this->unk4C0 & 1)) {
        WarGreymonVtable *vtable = GET_WARGREYMON_VTABLE(this);

        vtable->unk120.func((u8 *)this + vtable->unk120.delta);
    }
    if ((this->unk4C0 & 3) == 0) {
        if (Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON) {
            this->base.hitDamage = 0x5AC0400;
        } else {
            this->base.hitDamage = 0x4AC0533;
        }
        this->base.hitKnockback.vz = 0;
        this->base.hitKnockback.vx = -this->base.velocity.vx >> 1;
        this->base.hitKnockback.vy = -this->base.velocity.vy;
    }
    if ((this->unk4C0 & 7) == 0) {
        this->base.hitKind = 1;
    }
}

/* Creates an entity of func_800BD490's class (0x120 bytes) with g++'s new, and hands it to Fighter_addEntity. */
void func_800BC3C8(Fighter *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        func_800BD490(entityOperatorNewAt(0x120, child), this);
    }
    Fighter_addEntity(this, child);
}

/* While 0x4B4 is positive, sets the character's angle (0x3E0) and 0x3F0 to angle. */
void func_800BC424(WarGreymon *this, s32 angle) {
    if (this->unk4B4 > 0) {
        this->base.rotation.vz = angle;
        this->base.targetRotation.vz = angle;
    }
}

/* While 0x4B4 is positive and triangle is pressed, sets *angle from the d-pad held (0 up, 0x200 a step clockwise, from
 * when no direction or another mix is held) and returns 1; else returns 0. */
s32 WarGreymon_readAimAngle(WarGreymon *this, s32 *angle, s32 from) {
    InputHistory *input;
    s32 held;

    if (this->unk4B4 > 0) {
        input = Fighter_getInput(&this->base);
        InputHistory_setMirrored(input, 0);
        held = InputHistory_getHeld(input);
        if (InputHistory_getPressed(input) & PADRup) {
            switch (held & (PAD_UP | PAD_RIGHT | PAD_DOWN | PAD_LEFT)) {
            case PAD_UP:
                *angle = 0;
                break;
            case PAD_UP | PAD_RIGHT:
                *angle = 0x200;
                break;
            case PAD_RIGHT:
                *angle = 0x400;
                break;
            case PAD_RIGHT | PAD_DOWN:
                *angle = 0x600;
                break;
            case PAD_DOWN:
                *angle = 0x800;
                break;
            case PAD_DOWN | PAD_LEFT:
                *angle = -0x600;
                break;
            case PAD_LEFT:
                *angle = -0x400;
                break;
            case PAD_LEFT | PAD_UP:
                *angle = -0x200;
                break;
            default:
                *angle = from;
                break;
            }
            return 1;
        }
    }
    return 0;
}

/* Special move 3: frame 0 the voice and the slot 0x128 hitbox at (144 forward, 144) (arg 0x2A), frame 4 a push back
 * and up; from frame 5 a fall is stopped and half the velocity pushes against it. */
void WarGreymon_special3(WarGreymon *this, s32 frame) {
    Vec3 offset;
    WarGreymonVtable *vtable;

    Fighter_special3(&this->base, frame);
    func_80079AE4(&this->base);
    switch (frame) {
    case 0:
        Fighter_playVoice(&this->base, 0x10F);
        offset.vx = Fighter_getFacing(&this->base) * 144;
        offset.vy = 144;
        offset.vz = 0;
        vtable = GET_WARGREYMON_VTABLE(this);
        vtable->unk128.func((u8 *)this + vtable->unk128.delta, &offset, 0x2A);
        break;
    case 4: {
        s64 push = ((s64)this->base.mass * 0x1C70) >> 16;

        this->base.force.vx -= (s32)push * Fighter_getFacing(&this->base);
        this->base.velocity.vy = 0;
        this->base.force.vy -= ((s64)this->base.mass * 0x71C0) >> 16;
        break;
    }
    }
    if (frame >= 5 && this->base.velocity.vy > 0) {
        this->base.velocity.vy = 0;
        this->base.force.vx = -this->base.velocity.vx >> 1;
    }
}

/* Special move 3 in the air: frame 0 the voice (its own for CHARACTER_WARGREYMON) and the slot 0x128 hitbox at
 * (144 forward, 144) (arg 0x21); a fall is stopped and a sixteenth of the velocity pushes against it. */
void WarGreymon_airSpecial3(WarGreymon *this, s32 frame) {
    Vec3 offset;
    WarGreymonVtable *vtable;

    Fighter_airSpecial3(&this->base, frame);
    func_80079AE4(&this->base);
    if (frame == 0) {
        if (Fighter_getCharacter(&this->base) == CHARACTER_WARGREYMON) {
            Fighter_playVoice(&this->base, 0x10F);
        } else {
            Fighter_playVoice(&this->base, 0x121);
        }
        offset.vx = Fighter_getFacing(&this->base) * 144;
        offset.vy = 144;
        offset.vz = 0;
        vtable = GET_WARGREYMON_VTABLE(this);
        vtable->unk128.func((u8 *)this + vtable->unk128.delta, &offset, 0x21);
    }
    if (this->base.velocity.vy > 0) {
        this->base.velocity.vy = 0;
        this->base.force.vx = -this->base.velocity.vx >> 4;
    }
}

/* Creates an entity of func_800BDA50's class (0xD4 bytes) at pos with g++'s new and hands it to Fighter_addEntity, then
 * plays sound 0x109, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800BC7CC(Fighter *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800BDA50(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x300);
    }
    Fighter_addEntity(this, child);
    Fighter_playSound(this, 0x109);
    this->invulnerableFrames = 10;
    Fighter_addGauge(this, -0x10000);
}

/* The special move 1's hitbox (0xA0 bytes): a sphere that starts its radius in front of the fighter and up, speeds
 * forward and grows by 8 a frame, for 13 frames. */
typedef struct WarGreymonSpecial1Hitbox {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Vec3 position;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ s32 speed;        /* 0, then the acceleration more a frame */
    /* 0x94 */ s32 acceleration; /* forward */
    /* 0x98 */ s32 radius;
    /* 0x9C */ s32 frames;
} WarGreymonSpecial1Hitbox;

/* Builds the special move 1's hitbox for the fighter: its hit, and a sphere of the radius placed the radius in front
 * of the fighter and up, accelerating forward; adds the sphere to the fighter's side. */
WarGreymonSpecial1Hitbox *WarGreymonSpecial1Hitbox_init(WarGreymonSpecial1Hitbox *this, Fighter *fighter,
                                                        Vec3 *knockback, s32 damage, s32 radius,
                                                        s32 acceleration) {
    Fight *fight;
    s32 x, y, z;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, damage, knockback, 2);
    this->parts.base.vtable = WARGREYMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->parts.entity.vtable = &WARGREYMON_SPECIAL1_HITBOX_VTABLE;
    CollisionSphere_init(&this->parts.part, &this->position, radius, &this->parts.base);
    Fighter_getPosition(fighter, &this->position);
    this->position.vx += radius * Fighter_getFacing(fighter);
    this->position.vy -= radius;
    this->speed = 0;
    this->acceleration = acceleration * Fighter_getFacing(fighter);
    this->radius = radius;
    this->frames = 0;
    this->parts.part.radius = radius;
    this->parts.part.entity = &this->parts.entity;
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    this->parts.part.position.vx = x;
    this->parts.part.position.vy = y;
    this->parts.part.position.vz = z;
    fight = Fighter_getScene(fighter);
    if (Fighter_getSide(fighter) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    return this;
}

void WarGreymonSpecial1Hitbox_tick(WarGreymonSpecial1Hitbox *this) {
    this->position.vx += this->speed;
    this->speed += this->acceleration;
    this->radius += 8;
    this->parts.part.position.vx = this->position.vx;
    this->parts.part.position.vy = this->position.vy;
    this->parts.part.position.vz = this->position.vz;
    this->frames++;
    this->parts.part.radius = this->radius;

    if (this->frames >= 13) {
        entityDelete(&this->parts.entity);
    }
}

/* On a hit: Hit_apply with sum, then the hitbox deletes itself when getHitFighter finds a fighter in sum. */
void WarGreymonSpecial1Hitbox_hit(UnkPartsEntity38 *this, CollisionSphere *sum) {
    Hit_apply(&this->base, sum);
    if (getHitFighter(sum) != NULL) {
        entityDelete(&this->entity);
    }
}

/* The virtual table of Trail: a method at 0x20 (Trail_drawStrips). */
typedef struct Unk800C5680Vtable {
    /* 0x00 */ EntityVtable base;
    /* 0x20 */ VtableEntry unk20;
} Unk800C5680Vtable;

/* An Trail's virtual table as its own class sees it. */
#define UNK800C5680_VTABLE(object) ((Unk800C5680Vtable *)(object)->entity.vtable)

/* The entity class func_800BCAD8 constructs (virtual table D_8006CA7C): an Trail that slides along x,
 * faster each frame toward the character's facing. */
typedef struct Unk800BCAD8 {
    /* 0x000 */ Trail base;
    /* 0x690 */ s32 x;     /* the x of its matrix (func_800BCE1C) */
    /* 0x694 */ s32 speed; /* added to x each frame */
    /* 0x698 */ s32 facing; /* Fighter_getFacing: speed grows by facing * 10 */
    /* 0x69C */ s32 mode;  /* 1: func_800BCC7C once the timer ends */
} Unk800BCAD8;

extern EntityVtable D_8006CA7C;
void func_800BCBD8(Unk800BCAD8 *this);
void func_800BCC7C(Unk800BCAD8 *this);

/* Constructs an Unk800BCAD8 for fighter: Trail_init's entity with x and speed 0, the character's facing and
 * mode. */
Unk800BCAD8 *func_800BCAD8(Unk800BCAD8 *this, Fighter *fighter, TrailPoint *entries, s32 count,
                           s32 arg4, s32 arg5, s32 mode) {
    Trail_init(&this->base.entity, fighter, entries, count, arg4, arg5);
    this->base.entity.vtable = &D_8006CA7C;
    this->x = 0;
    this->speed = 0;
    this->facing = Fighter_getFacing(fighter);
    this->mode = mode;
    return this;
}

/* The tick method of Unk800BCAD8: moves x by speed and speed by facing * 10, func_800BCBD8, in mode 1
 * func_800BCC7C once the timer ends, then the base's tick (Trail_tick). */
void func_800BCB4C(Unk800BCAD8 *this, CollisionSphere *sum) {
    this->x += this->speed;
    this->speed += this->facing * 10;
    func_800BCBD8(this);
    if (this->mode == 1 && this->base.recordTimer <= 0) {
        func_800BCC7C(this);
    }
    Trail_tick(&this->base, sum);
}

void func_800BCBD8(Unk800BCAD8 *this) {
    s32 delta;
    s32 i;
    s32 j;
    s32 count = this->base.length - 1;
    s32 valid = this->base.validCount - 1;
    s32 head = this->base.head;

    if (valid < count) {
        count = valid;
    }

    delta = this->x >> 2;
    for (i = 0; i < 2; i++, delta >>= 2) {
        TrailSample *samples = this->base.samples[i];

        for (j = 0; j <= count; j++) {
            TrailSample *sample = &samples[(head - j) & 0x1F];

            sample->x -= delta;
        }
    }
}


struct Unk800BD1C4;
struct Unk800BD1C4 *func_800BD1C4(struct Unk800BD1C4 *this, VECTOR position, s32 vramY);

/* Sheds a piece (func_800BD1C4) from each recorded sample of the third point, scattered in x by up to the speed
 * and in y by a random 0..0x7F, as siblings of the trail. The match depends on RANDOM's address taken into a
 * local before the loop, and on the position copied for the call only after entityOperatorNewAt. */
void func_800BCC7C(Unk800BCAD8 *this) {
    s32 i;
    s32 spread;
    s32 count = this->base.length - 1;
    s32 valid = this->base.validCount - 1;
    s32 head = this->base.head;
    s32 vramY;
    TrailSample *samples;
    MersenneTwister *rng;

    if (valid < count) {
        count = valid;
    }
    spread = abs(this->speed) + 1;
    vramY = Fighter_getVramY(this->base.fighter);
    samples = this->base.samples[2];
    rng = &RANDOM;
    for (i = 0; i <= count; i++) {
        TrailSample *sample = &samples[(head - i) & 0x1F];
        VECTOR position;
        VECTOR arg;
        Entity *child;
        s32 r;

        position.vx = sample->x + this->x - mersenneTwisterGenerate(rng) % spread;
        r = mersenneTwisterGenerate(rng);
        position.vy = sample->y + r % 128;
        position.vz = sample->z;
        child = allocEntityMemory(0x48);
        if (child != NULL) {
            func_800BD1C4(entityOperatorNewAt(0x48, child), ({ arg = position; arg; }), vramY);
        }
        entityAddSibling(&this->base.entity, child);
    }
}

/* The draw method of Unk800BCAD8: sets the model matrix to a translation by x, then runs the method at 0x20
 * (Trail_drawStrips). */
void func_800BCE1C(Unk800BCAD8 *this, s32 arg) {
    MatrixWords m;
    Unk800C5680Vtable *vtable;

    m.words[0] = 0x1000;
    m.words[1] = 0;
    m.words[2] = 0x1000;
    m.words[3] = 0;
    m.words[4] = 0x1000;
    m.m.t[0] = this->x;
    m.m.t[1] = 0;
    m.m.t[2] = 0;
    setModelMatrix(&m.m);
    vtable = UNK800C5680_VTABLE(&this->base);
    vtable->unk20.func((u8 *)this + vtable->unk20.delta, arg);
}

/* The special move 1's effect: a matrix at the character's position, turned to its facing, that moves along x; its quad
 * grows to targetLength, then fades. */
typedef struct WarGreymonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MatrixWords matrix;
    /* 0x38 */ s32 x;      /* the translation along the matrix's x, 4 more each frame */
    /* 0x3C */ s32 facing; /* Fighter_getFacing */
    /* 0x40 */ s32 length;       /* of the drawn quad along x; eases to targetLength, by half the difference a frame */
    /* 0x44 */ s32 targetLength; /* WarGreymonSpecial1Effect_init's third argument */
    /* 0x48 */ s32 shade;        /* of the quad's far end: 0x80, then 0x10 less each frame once fading */
    /* 0x4C */ s32 fading;       /* set once length reaches targetLength */
    /* 0x50 */ s32 frames;
    /* 0x54 */ s32 vramY;  /* Fighter_getVramY */
} WarGreymonSpecial1Effect;

extern EntityVtable WARGREYMON_SPECIAL1_EFFECT_VTABLE;

/* Sets the matrix to a translation by v, as matrixInitTranslationVec (engine/math/matrix_ops.c) does. The match
 * depends on this taking the effect, not the matrix: the stores go through this. */
static inline void WarGreymonSpecial1Effect_placeAt(WarGreymonSpecial1Effect *this, VECTOR *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    this->matrix.words[0] = 0x1000;
    this->matrix.words[1] = 0;
    this->matrix.words[2] = 0x1000;
    this->matrix.words[3] = 0;
    this->matrix.words[4] = 0x1000;
    this->matrix.m.t[0] = x;
    this->matrix.m.t[1] = y;
    this->matrix.m.t[2] = z;
}

/* Builds the special move 1's effect at fighter's position, tilt / 4 higher, turned by tilt against its facing (and
 * half a turn about y when it faces left); length eases from 0 to targetLength. */
WarGreymonSpecial1Effect *WarGreymonSpecial1Effect_init(WarGreymonSpecial1Effect *this, Fighter *fighter, s32 targetLength,
                                                        s32 tilt) {
    VECTOR position;
    s32 facing;

    entityInit(&this->entity, 0);
    this->entity.vtable = &WARGREYMON_SPECIAL1_EFFECT_VTABLE;
    facing = Fighter_getFacing(fighter);
    Fighter_getPosition(fighter, (Vec3 *)&position);
    position.vy -= tilt >> 2;
    WarGreymonSpecial1Effect_placeAt(this, &position);
    matrixRotateZ(&this->matrix.m, -tilt * facing);
    if (Fighter_getFacing(fighter) < 0) {
        matrixRotateY(&this->matrix.m, 0x800);
    }
    this->x = 0;
    this->facing = facing;
    this->length = 0;
    this->targetLength = targetLength;
    this->shade = 0x80;
    this->fading = 0;
    this->frames = 0;
    this->vramY = Fighter_getVramY(fighter);
    return this;
}

/* The special move 1 effect's tick: moves the matrix by x, x += 4; eases length to targetLength, then fades shade by
 * 0x10 and deletes the entity once it is below 0. */
void WarGreymonSpecial1Effect_tick(WarGreymonSpecial1Effect *this) {
    s32 step;

    matrixTranslate(&this->matrix.m, this->x, 0, 0);
    this->x += 4;
    if (this->fading != 0) {
        this->shade -= 0x10;
        if (this->shade < 0) {
            entityDelete(&this->entity);
            return;
        }
    } else {
        step = (this->targetLength - this->length) >> 1;
        this->length += step;
        if (step == 0) {
            this->fading = 1;
        }
    }
    this->frames++;
}

/* Draws the effect: a quad from length / 8 to length along the matrix's x, 0x200 high, black at the near end and shade
 * (red, half green and blue) at the far end, with the 32 by 32 picture of frame & 3, semi-transparent. */
void WarGreymonSpecial1Effect_draw(WarGreymonSpecial1Effect *this, Camera *camera) {
    SVECTOR corners[4];
    PolyGT4Shape shape;
    s32 length;
    s32 vramY;
    u8 u;
    s32 half;
    u_long *ot;
    s32 shift;

    setModelMatrix(&this->matrix.m);
    corners[0].vy = corners[2].vy = 0x100;
    corners[1].vy = corners[3].vy = -0x100;
    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    corners[0].vz = corners[1].vz = corners[2].vz = corners[3].vz = 0;
    shape.v0 = shape.v1 = 0xC0;
    shape.v2 = shape.v3 = 0xDF;
    shape.colors[0] = shape.colors[1] = 0;
    length = this->length;
    corners[0].vx = corners[1].vx = length >> 3;
    corners[2].vx = corners[3].vx = length;
    u = (this->frames & 3) << 5;
    shape.u0 = shape.u2 = u;
    shape.u1 = shape.u3 = u + 0x1F;
    vramY = this->vramY;
    ot = camera->ot.slots;
    shift = camera->zShift;
    half = this->shade >> 1;
    shape.colors[2] = shape.colors[3] = this->shade | (half << 8) | (half << 16);
    shape.clut = getClut(0x1C0, vramY + 0xF7);
    shape.tpage = getTPage(0, 1, 0x1C0, vramY);
    shape.flags = 9;
    PRIM_BUFFER_FREE = buildPolyGT4(&shape, PRIM_BUFFER_FREE, ot, shift);
}

/* The entity func_800BD1C4 constructs (0x48 bytes, virtual table D_8006CA54): a piece thrown up from a
 * position, falling and growing in one side and shrinking in the other for 0x14 frames (func_800BD254). */
typedef struct Unk800BD1C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ VECTOR position;
    /* 0x28 */ Vec3 velocity; /* vy grows by 1 each frame; vx takes a random term */
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ s32 scaleX; /* 0x40, 2 more each frame: the draw's matrix's x scale */
    /* 0x3C */ s32 scaleY; /* 0x40, 2 less each frame: its y scale */
    /* 0x40 */ s32 life;   /* frames left: the entity deletes itself below 0 */
    /* 0x44 */ s32 vramY;  /* func_800BD1C4's last argument: the VRAM half of the clut and tpage */
} Unk800BD1C4;

extern EntityVtable D_8006CA54;

/* Constructs func_800BD1C4's entity at position, thrown up at 0x14, for 0x14 frames. */
Unk800BD1C4 *func_800BD1C4(Unk800BD1C4 *this, VECTOR position, s32 vramY) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006CA54;
    this->velocity.vx = 0;
    this->velocity.vy = -0x14;
    this->velocity.vz = 0;
    this->scaleY = 0x40;
    this->scaleX = 0x40;
    this->vramY = vramY;
    this->life = 0x14;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    return this;
}

/* func_800BD1C4's tick: moves by the velocity, halves its vx and adds a random -7..7, falls faster by 1, grows unk38
 * and shrinks unk3C by 2, and deletes itself once its frames run out. The match depends on reading the position whole
 * before adding (as vecAddInPlace). */
void func_800BD254(Unk800BD1C4 *this) {
    s32 x, y, z;

    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->velocity.vx = (this->velocity.vx >> 1) + (s32)mersenneTwisterGenerate(&RANDOM) % 8;
    this->velocity.vy++;
    this->scaleX += 2;
    this->scaleY -= 2;
    this->life--;
    if (this->life < 0) {
        entityDelete(&this->entity);
    }
}

/* Copies v into a matrix's translation, the three words read before any is stored. */
static inline void setTranslationWG(long *t, Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    t[0] = x;
    t[1] = y;
    t[2] = z;
}

/* func_800BD1C4's draw: SQUARE_CORNERS scaled by scaleX and scaleY at the position, with the 16 by 16 picture at
 * (0, 0xE0), grey by life * 128 / 20, semi-transparent.
 * The match depends on the translation copied first, through setTranslationWG. */
void func_800BD334(Unk800BD1C4 *this, Camera *camera) {
    MatrixWords m;
    PolyFT4Shape shape;
    u_long *ot;
    s32 shift;
    s32 shade;
    s32 vramY;
    s32 sy;

    setTranslationWG(m.m.t, (Vec3 *)&this->position);
    sy = this->scaleY;
    m.words[1] = 0;
    m.words[3] = 0;
    m.words[4] = 0x1000;
    m.words[0] = (u16)this->scaleX;
    m.words[2] = sy & 0xFFFF;
    ot = camera->ot.slots;
    shift = camera->zShift;
    setModelMatrix(&m.m);
    shape.corners[0] = &SQUARE_CORNERS[0];
    shape.corners[1] = &SQUARE_CORNERS[1];
    shape.corners[2] = &SQUARE_CORNERS[2];
    shape.corners[3] = &SQUARE_CORNERS[3];
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0xF;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0xE0;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xEF;
    shape.flags = 9;
    shade = this->life * 128 / 20;
    vramY = this->vramY;
    shape.clut = getClut(0x1C0, vramY + 0xF9);
    shape.tpage = getTPage(0, 1, 0x1C0, vramY);
    shape.color = shade | (shade << 8) | (shade << 16);
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, shift);
}

/* The entity func_800BD490 constructs (0x120 bytes, virtual table D_8006CA2C): the effect the WarGreymonVtable slot
 * 0x120 spawns, drawn as a band of 8 quads. */
typedef struct Unk800BD490 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ MATRIX matrix;          /* func_800BD8DC's model matrix */
    /* 0x3C */ SVECTOR band[8][2];     /* the band's points, two per step */
    /* 0xBC */ VECTOR unkBC;           /* the newest point: out by 0x18 and up by unk10C a tick */
    /* 0xCC */ VECTOR unkCC[4];        /* the last 4 of unkBC, by unk118 */
    /* 0x10C */ s32 unk10C;            /* unkBC's rise, growing by 8 */
    /* 0x110 */ s32 unk110;            /* the turn about y */
    /* 0x114 */ s32 shade;             /* red, and a quarter of it green */
    /* 0x118 */ s32 unk118;
    /* 0x11C */ s32 vramY;             /* the VRAM half of the clut and tpage */
} Unk800BD490;

/* Constructs func_800BD490's entity for a fighter: the fighter's model matrix and every point of unkBC, unkCC and
 * the band at (0, -0x280, 0). It copies the matrix as a block move (four loads then four stores), which GCC 2.95.2
 * does not emit for 32 bytes (see "Block moves" in TODO.md). */
INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD490);

/* Ticks func_800BD490's effect: follows the fighter's model matrix while it attacks and rebuilds the band. It copies
 * the matrix as a block move (four loads then four stores), which GCC 2.95.2 does not emit for 32 bytes (see "Block
 * moves" in TODO.md). */
INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD650);

/* Draws func_800BD490's effect: 8 quads around the band, each from a step's first point to the next step's two
 * points and the second point of the step after, with the 32 by 32 picture at (0xE0, 0xC0), black at the start and
 * shade at the end, semi-transparent. The match depends on reading vramY once and on both following steps
 * computed before either wraps. */
void func_800BD8DC(Unk800BD490 *this, Camera *camera) {
    PolyGT4Shape shape;
    u_long *ot;
    s32 shift;
    u8 *packet;
    s32 i;
    s32 next;
    s32 after;
    s32 vramY;

    setModelMatrix(&this->matrix);
    ot = camera->ot.slots;
    shift = camera->zShift;
    packet = PRIM_BUFFER_FREE;
    shape.u0 = shape.u2 = 0xE0;
    shape.u1 = shape.u3 = 0xFF;
    shape.v0 = shape.v1 = 0xC0;
    shape.v2 = shape.v3 = 0xDF;
    shape.colors[0] = shape.colors[1] = 0;
    shape.flags = 9;
    shape.colors[2] = shape.colors[3] = this->shade | ((this->shade >> 2) << 8);
    vramY = this->vramY;
    shape.clut = getClut(0x1C0, vramY + 0xFA);
    shape.tpage = getTPage(0, 1, 0x1C0, vramY);
    for (i = 0; i < 8; i++) {
        next = i + 1;
        after = i + 2;
        if (next >= 8) {
            next -= 8;
        }
        if (after >= 8) {
            after -= 8;
        }
        shape.corners[0] = &this->band[i][0];
        shape.corners[1] = &this->band[next][0];
        shape.corners[2] = &this->band[next][1];
        shape.corners[3] = &this->band[after][1];
        packet = buildPolyGT4(&shape, packet, ot, shift);
    }
    PRIM_BUFFER_FREE = packet;
}

/* Constructs func_800BDA50's entity for owner at pos: its base at 0x18 (Hit_init, 0x5B04000) and its
 * member at 0x38 (CollisionSphere_init) on no velocity, the owner's handle, arg3 at 0xC8 and arg4 at 0xB8. */
Unk800BDA50 *func_800BDA50(Unk800BDA50 *this, Fighter *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    s32 x, y, z;

    entityInit(&this->entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->unk18, owner, 0x5B04000, (Vec3 *)&ZERO_VECTOR, 0);
    this->unk18.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
    CollisionSphere_init(&this->unk38, (Vec3 *)&ZERO_VECTOR, 0, &this->unk18);
    this->unk38.entity = &this->entity;
    this->unk80 = entityGetHandle(&owner->entity);
    this->unk84 = 0;
    this->unk88 = 0;
    this->unk8C = 0;
    this->unk90 = 0;
    /* The match depends on reading the vector's three words before storing any, as vecCopy does. */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->unk94.vx = x;
    this->unk94.vy = y;
    this->unk94.vz = z;
    this->unkA4.vx = 0;
    this->unkA4.vy = 0;
    this->unkA4.vz = 0;
    this->unkB0 = 0;
    this->unkB4 = 0;
    this->unkB8 = arg4;
    this->unkBC = 0;
    this->unkC0 = 0;
    this->unkC4 = 0;
    this->unkC8 = arg3;
    this->unkCC = 0;
    this->vramY = Fighter_getVramY(owner);
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C944);

extern VECTOR D_8006C944[2];
VECTOR *Fighter_getNodesMidpoint(Fighter *this, VECTOR *out, s32 node0, VECTOR *offset0, s32 node1, VECTOR *offset1);

/* Ticks func_800BDA50's entity, a state machine in unkC4: 0 holds it between the owner's nodes 0x10 and 5
 * (offsets D_8006C944 raised by the radius, which swells as a cosine of unkCC over unkC8), then gives the hit
 * its knockback, adds the sphere to the owner's side and moves on (or fades out, 3, when the owner is gone, idle
 * or in body state 0x13); 1 flies by the velocity with the sphere along until unkCC passes 0x80; 2 homes on unkA4
 * by an eighth of the way each tick until within 0x100, then fades; 3 grows and fades until deleted. The match
 * depends on state 2's own locals and the order of its loads and stores. */
void func_800BDB80(Unk800BDA50 *this) {
    Fight *fight = CURRENT_FIGHT;
    Fighter *owner = (Fighter *)getEntityByHandle(this->unk80);
    VECTOR offsets[2];
    VECTOR kb;
    s32 x, y, z;
    s32 angle;

    switch (this->unkC4) {
    case 0:
        if (owner != NULL && !Fighter_isAttackIdle(owner) && Fighter_getBodyStateId(owner) != 0x13) {
            angle = (this->unkCC << 10) / this->unkC8;
            this->unkB4 = this->unkB8 - ((this->unkB8 * COS_SIN_TABLE[angle & 0xFFF].cos) >> 12);
            offsets[0] = D_8006C944[0];
            offsets[1] = D_8006C944[1];
            offsets[0].vy += this->unkB4;
            offsets[1].vy += this->unkB4;
            Fighter_getNodesMidpoint(owner, (VECTOR *)&this->unk84, 0x10, &offsets[1], 5, &offsets[0]);
            if (this->unkCC > this->unkC8) {
                memset(&kb, 0, sizeof(kb));
                kb.vy = -0x4000;
                kb.vx = this->unk94.vx << 7;
                offsets[1] = kb;
                offsets[0] = offsets[1];
                Hit_setKnockback(&this->unk18, &offsets[0]);
                if (Fighter_getSide(owner) == 0) {
                    Fight_addSide0Shape(fight, &this->unk38.link);
                } else {
                    Fight_addSide1Shape(fight, &this->unk38.link);
                }
                this->unkC4++;
            }
        } else {
            this->unkC4 = 3;
            this->unk38.flags.word |= 1;
        }
        break;
    case 1:
        x = this->unk84;
        y = this->unk88;
        z = this->unk8C;
        x += this->unk94.vx;
        y += this->unk94.vy;
        z += this->unk94.vz;
        this->unk84 = x;
        this->unk88 = y;
        this->unk8C = z;
        this->unk38.position.vx = x;
        this->unk38.position.vy = y;
        this->unk38.position.vz = z;
        this->unk38.radius = this->unkB4;
        if (this->unkCC <= 0x80) {
            break;
        }
        goto destroy;
    case 2: {
        s32 x, y, z, tx, ty, tz;
        x = this->unk84;
        y = this->unk88;
        z = this->unk8C;
        tx = this->unkA4.vx;
        ty = this->unkA4.vy;
        x += this->unk94.vx;
        tz = this->unkA4.vz;
        y += this->unk94.vy;
        z += this->unk94.vz;
        tx -= x;
        ty -= y;
        this->unk84 = x;
        this->unk88 = y;
        this->unk94.vx = tx;
        tz -= z;
        this->unk8C = z;
        this->unk94.vz = tz;
        this->unk94.vy = ty;
        }
        this->unkB4 = vecGetLength((VECTOR *)&this->unk94);
        this->unk94.vx >>= 3;
        this->unk94.vy >>= 3;
        this->unk94.vz >>= 3;
        if (this->unkB4 < 0x100) {
            goto destroy;
        }
    case 3:
        this->unkB4 += 0x10;
        this->unkC0 -= 8;
        if (this->unkC0 <= 0) {
        destroy:
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->unkC0 += 4;
    if (this->unkC0 > 0x80) {
        this->unkC0 = 0x80;
    }
    this->unkBC += 0x60;
    this->unkCC++;
}

/* The virtual table of func_800BDA50's class (D_8006C9FC): Entity's methods, then the one at 0x20 (func_800BDFCC). */
typedef struct Unk800BDA50Vtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry unk20;
} Unk800BDA50Vtable;

/* The second base's first method for virtual table D_8006C9DC: Hit_apply with sum; when getHitFighter finds a
 * character in sum, takes its position to 0xA4, sets 0xC4 to 2, runs the method at 0x20 with Fighter_getOpponent's
 * character and unlinks the part at 0x38. */
void func_800BDF28(Unk800BDA50 *this, CollisionSphere *sum) {
    Entity *fighter;
    Fighter *owner;
    Unk800BDA50Vtable *vtable;
    ListNode *node;

    Hit_apply(&this->unk18, sum);
    fighter = getHitFighter(sum);
    if (fighter != NULL) {
        Fighter_getPosition((Fighter *)fighter, &this->unkA4);
        this->unkC4 = 2;
        owner = Fighter_getOpponent((Fighter *)fighter);
        vtable = (Unk800BDA50Vtable *)this->entity.vtable;
        vtable->unk20.func((u8 *)this + vtable->unk20.delta, owner);
        node = &this->unk38.link;
        this->unk38.link.next->prev = this->unk38.link.prev;
        node->prev->next = this->unk38.link.next;
        this->unk38.link.next = node;
        this->unk38.link.prev = node;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C964);

/* The method at 0x20 of virtual table D_8006C9FC: adds func_800BE39C's and func_800C6088's entities at 0xA4
 * as siblings, then plays sound 0x10A. */
void func_800BDFCC(Unk800BDA50 *this, s32 arg) {
    Entity *child = allocEntityMemory(0xE0);

    if (child != NULL) {
        func_800BE39C(entityOperatorNewAt(0xE0, child), arg, &this->unkA4, 0x5DB08000, 0x700, &D_8006C964);
    }
    entityAddSibling(&this->entity, child);
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C6088(entityOperatorNewAt(0x3C, child), 3, &this->unkA4, 0x700, 0x100, 0x30, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x10A, 0);
}

#define UV_BYTES(uv) ((u8 *)&(uv))

static inline void setTranslation(long *t, Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    t[0] = x;
    t[1] = y;
    t[2] = z;
}

/* Draws the explosion as two frames of four corners around this->unk84, with the
 * 32 by 32 picture at (0x1C0, vramY). The match depends on the camera taken as an
 * s32 and copied to a local (which gives camera $s5 over ot's $s6), and on the corner
 * stores in this order. */
void func_800BE0C4(Unk800BDA50 *this, s32 cameraArg) {
    Camera *camera = (Camera *)cameraArg;
    MATRIX matrix;
    MATRIX *m = &matrix;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    PolyFT4Shape *s;
    u8 *packet;
    u_long *ot;
    s32 vramY;
    s32 fade;
    s32 val;
    s32 new_var;

    ot = camera->ot.slots;
    packet = PRIM_BUFFER_FREE;
    matrix = camera->view->rotation;
    setTranslation(matrix.t, (Vec3 *)&this->unk84);
    new_var = ~3;
    matrixRotateZ(m, this->unkBC);
    setModelMatrix(m);

    corners[3].vz = 0;
    corners[2].vz = 0;
    corners[1].vz = 0;
    corners[0].vz = 0;

    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];

    shape.flags = 9;

    corners[1].vy = -this->unkB4;
    corners[0].vy = -this->unkB4;
    corners[2].vx = -this->unkB4;
    corners[0].vx = -this->unkB4;

    vramY = this->vramY;

    corners[3].vy = this->unkB4;
    corners[2].vy = this->unkB4;
    corners[3].vx = this->unkB4;
    corners[1].vx = this->unkB4;

    shape.tpage = ((((vramY & 0x100) >> 4) | 0x27) | ((vramY & 0x200) << 2));
    s = &shape;

    if (this->unkC0 < 0x80) {
        fade = 0x80 - this->unkC0;
        shape.color = fade | (fade << 8) | (fade << 16);
        UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0xC0;
        UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0xDF;
        UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0xC0;
        UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xDF;
        shape.clut = getClut(0x1C0, vramY + 0xFB);
        packet = buildPolyFT4(s, packet, ot, camera->zShift);
    }

    fade = this->unkC0;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0x80;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xBF;
    shape.clut = getClut(0x1C0, vramY + 0xFC);
    val = (*(u8 *)&this->unkCC & 1) << 6;
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = val;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = val + 0x3F;
    shape.color = fade | (fade << 8) | (fade << 16);
    packet = buildPolyFT4(s, packet, ot, camera->zShift);

    shape.color = (fade >> 1) | ((fade >> 1) << 8) | ((fade >> 1) << 16);
    shape.clut = getClut(0x1C0, vramY + 0xFD);
    shape.tpage = ((((vramY & 0x100) >> 4) | 0x47) | ((vramY & 0x200) << 2));
    shape.flags = (shape.flags & new_var) | 0xA;
    packet = buildPolyFT4(s, packet, ot, camera->zShift);

    PRIM_BUFFER_FREE = packet;
}

/* Builds func_800BE39C's class (virtual table D_8006C9AC, D_8006C98C for its Hit at 0x18):
 * func_800A926C's constructor, then sets its knockback and color. */
Unk800BE39C *func_800BE39C(Unk800BE39C *this, s32 fighter, Vec3 *position, u32 damage, s32 arg4, CVECTOR *color) {
    Hit *hit;
    ListNode *node;
    VECTOR knockback;

    func_800A926C(&this->base, fighter, position);
    hit = &this->base.unk18;
    this->base.unk18.vtable = D_8006C98C;
    this->base.entity.vtable = &D_8006C9AC;

    if (damage == 0) {
        node = &this->base.unk38.link;
        this->base.unk38.link.next->prev = this->base.unk38.link.prev;
        node->prev->next = this->base.unk38.link.next;
        this->base.unk38.link.next = node;
        this->base.unk38.link.prev = node;
    } else {
        knockback.vx = 0;
        knockback.vy = -0x1C70;
        knockback.vz = 0;
        Hit_setKnockback(hit, &knockback);
        Hit_set(hit, damage, 8);
    }

    this->base.color = *color;
    this->base.unk98 = 0x80;
    this->base.unk9A = 0x80;
    this->base.unk9C = 0x40;
    this->base.unk9E = 0x40;
    this->unkD8 = arg4;
    this->unkDC = 0;
    this->base.clut = getClut(0x1C0, this->base.vramY + 0xFE);
    func_800BE4AC(this);
    return this;
}

/* Stops the sound at 0xD4. */
void func_800BE4AC(Unk800BE39C *this) {
    stopSound(this->base.unkD4);
    this->base.unkD4 = 0;
}

typedef struct UnkBE4DC {
    u8 _00[0x14];
    EntityVtable *vtable; /* 0x14 */
    u8 _18[0x38 - 0x18];
    ListNode link;        /* 0x38 */
    u8 _40[0x7C - 0x40];
    s32 unk7C;            /* 0x7C */
    s32 unk80;            /* 0x80 */
    Vec3 position;        /* 0x84 */
    u8 _90[0x94 - 0x90];
    CVECTOR color;        /* 0x94 */
    u8 _98[0xA0 - 0x98];
    u16 clut;             /* 0xA0 */
    s16 facing;           /* 0xA2 */
    u8 unkA4[5];          /* 0xA4 */
    u8 unkA9[9];          /* 0xA9 */
    u8 _B2[0xB4 - 0xB2];
    s32 unkB4;            /* 0xB4 */
    s32 unkB8;            /* 0xB8 */
    s32 unkBC;            /* 0xBC */
    s32 unkC0;            /* 0xC0 */
    s32 unkC4;            /* 0xC4 */
    s32 unkC8;            /* 0xC8 */
    s32 unkCC;            /* 0xCC */
    s32 vramY;            /* 0xD0 */
    u32 unkD4;            /* 0xD4 */
    s32 unkD8;            /* 0xD8 */
    s32 unkDC;            /* 0xDC */
} UnkBE4DC;

/* Ticks D_8006C9AC's entity: grows unkB8 toward unkD8 (state 0), spawns its func_800C6088 child once unkCC passes 0x28
 * (state 1), then fades by the distance left to unkD8 + 0x200 (state 2), and scrolls its u and v bytes.
 * The match depends on state 0's start in a block of its own and on state 2's shade clamped from a copy of
 * abs(diff) >> 2. */
void func_800BE4DC(UnkBE4DC *this) {
    Entity *child;
    s32 diff;
    s32 end;
    s32 start;
    s32 i;
    s32 v;
    s32 shade;
    s32 a2;
    s32 b;
    u8 *p;
    u8 *q;
    u8 *r;
    ListNode *node;

    switch (this->unkC8) {
    case 0:
        {
            s32 s = this->unkB8 - 8;
            this->unkB8 += (this->unkD8 - s) >> 3;
        }
        if (this->unkB8 >= this->unkD8) {
            this->unkC8 = 1;
        }
        break;
    case 1:
        if (this->unkCC >= 0x29) {
            child = allocEntityMemory(0x3C);
            if (child != NULL) {
                func_800C6088(entityOperatorNewAt(0x3C, child), 9, &this->position, this->unkB8, 0x20, -0x50,
                              *(s32 *)&this->color);
            }
            entityAddSibling((Entity *)this, child);
            node = &this->link;
            this->link.next->prev = this->link.prev;
            node->prev->next = this->link.next;
            this->link.next = node;
            this->link.prev = node;
            this->unkC8++;
        }
        break;
    case 2:
        end = this->unkD8 + 0x200;
        start = this->unkB8;
        diff = end - start;
        this->unkB8 = start + (diff >> 3);
        v = abs(diff) >> 2;
        if (v < 0x80) {
            shade = v;
        } else {
            shade = 0x80;
        }
        this->unkBC = shade;
        if (shade < 8) {
            entityDelete((Entity *)this);
            return;
        }
        break;
    }

    a2 = (this->unkCC >> 1) & 0x3F;
    b = (u8)this->unkCC & 0x3F;
    if (this->facing > 0) {
        a2 = (0x3F - a2) & 0xFF;
    }
    q = this->unkA9;
    i = 4;
    p = &this->unkA4[4];
    do {
        *p-- = a2 + i * 0x10;
    } while (--i >= 0);
    i = 8;
    r = q + 8;
    do {
        *r-- = b + i * 8;
    } while (--i >= 0);
    this->unk7C = this->unkB8;
    this->unkDC += (this->unkD8 - this->unkDC) >> 3;
    this->unkCC++;
}

/* The second base's first method for virtual table D_8006C98C: Hit_apply with sum, then unlinks the part at
 * 0x38 when getHitFighter finds an entity in sum. */
void func_800BE71C(Unk800BE39C *this, CollisionSphere *sum) {
    ListNode *node;

    Hit_apply(&this->base.unk18, sum);
    if (getHitFighter(sum) != NULL) {
        node = &this->base.unk38.link;
        this->base.unk38.link.next->prev = this->base.unk38.link.prev;
        node->prev->next = this->base.unk38.link.next;
        this->base.unk38.link.next = node;
        this->base.unk38.link.prev = node;
    }
}

/* The draw of the entities of D_8006C9AC: func_800A9998, func_800BE7D0, then func_800BEA1C. */
void func_800BE788(Entity *this, s32 arg) {
    func_800A9998(this, arg);
    func_800BE7D0(this, arg);
    func_800BEA1C(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C968);

typedef struct {
    s32 words[8];
} Corners4;

extern Corners4 D_8006C968;

/* The second draw of D_8006C9AC's entities: four quarters of a square of side 2 * (unkB8 - unkB8 / 4) facing the
 * camera at the position, from D_8006C968's corners, with the 64 by 64 picture at (0xC0, 0x80), grey by unkBC / 2,
 * semi-transparent. The match depends on packet read from PRIM_BUFFER_FREE after the translation, ot and shift (which
 * gives packet $s0 and this $s1), and on the dead store to cameraArg. */
void func_800BE7D0(Entity *arg, s32 cameraArg) {
    UnkBE4DC *this = (UnkBE4DC *)arg;
    Camera *camera = (Camera *)cameraArg;
    MATRIX m;
    MATRIX *mp = &m;
    SVECTOR corners[4];
    PolyFT4Shape shape;
    u8 *packet;
    u_long *ot;
    s32 shift;
    s32 shade;

    m = camera->view->rotation;
    cameraArg = this->unkB8 >> 2;
    cameraArg = this->unkB8 - cameraArg;
    setTranslationWG(m.t, &this->position);
    ot = camera->ot.slots;
    shift = camera->zShift;
    packet = PRIM_BUFFER_FREE;
    matrixScale(mp, this->unkB8 - (this->unkB8 >> 2), this->unkB8 - (this->unkB8 >> 2), this->unkB8 - (this->unkB8 >> 2));
    setModelMatrix(mp);
    shape.corners[0] = &corners[0];
    shape.corners[1] = &corners[1];
    shape.corners[2] = &corners[2];
    shape.corners[3] = &corners[3];
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0xC0;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0xFF;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = 0x80;
    *(Corners4 *)corners = D_8006C968;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = 0xBF;
    shade = this->unkBC >> 1;
    shape.color = shade | (shade << 8) | (shade << 16);
    shape.clut = this->clut;
    shape.tpage = (((this->vramY & 0x100) >> 4) | 0x27) | ((this->vramY & 0x200) << 2);
    shape.flags = 9;
    packet = buildPolyFT4(&shape, packet, ot, shift);
    corners[0].vx = corners[2].vx = 0x1000;
    packet = buildPolyFT4(&shape, packet, ot, shift);
    corners[0].vy = corners[1].vy = 0x1000;
    packet = buildPolyFT4(&shape, packet, ot, shift);
    corners[0].vx = corners[2].vx = -0x1000;
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, packet, ot, shift);
}
/* Sets w to the identity with translation v. The diagonal's later words copied from words[0] keep their stores
 * relative to sp, as in the original. */
static inline void setIdentityAt(MatrixWords *w, Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    w->words[0] = 0x1000;
    w->words[1] = 0;
    w->words[2] = w->words[0];
    w->words[3] = 0;
    w->words[4] = w->words[0];
    w->m.t[0] = x;
    w->m.t[1] = y;
    w->m.t[2] = z;
}

/* The third draw of D_8006C9AC's entities: a ring of 17 spokes (each a point at radius 0x1000 and one at 5/4 of it,
 * at angles of 0x80 steps from the opposite direction), shaded by unkBC times the sine, drawn twice as 15
 * POLY_GT4s with the 32 by 32 picture at ((unkCC & 3) * 32, 0xC0): at the position, turned by +0x200 and then
 * -0x200 about z, by 0x80 about x and scaled by unkDC. */
void func_800BEA1C(Entity *arg, s32 cameraArg) {
    UnkBE4DC *this = (UnkBE4DC *)arg;
    Camera *camera = (Camera *)cameraArg;
    SVECTOR bandArr[17][2];
    u32 colorArr[17];
    PolyGT4Shape shape;
    MatrixWords m;
    SVECTOR (*band)[2];
    MATRIX *mp;
    u_long *ot;
    s32 shift;
    u8 *packet;
    s32 i;
    s32 scale;
    s32 u;
    s32 vramY;
    s32 bc;
    SVECTOR *p;
    u32 *col;

    ot = camera->ot.slots;
    shift = camera->zShift;
    packet = PRIM_BUFFER_FREE;
    bc = this->unkBC;
    band = bandArr;
    col = colorArr;
    for (i = 0; i < 17; i++) {
        s32 angle = (i << 7) & 0xF80;
        s16 c, s;
        s32 shade;

        p = band[i];
        p[0].vy = 0;
        p[1].vy = 0;
        c = COS_SIN_TABLE[angle].cos;
        s = COS_SIN_TABLE[angle].sin;
        c = -c;
        s = -s;
        p[0].vx = c;
        p[0].vz = s;
        p[1].vx = c + (c >> 2);
        p[1].vz = s + (s >> 2);
        shade = -(s * bc) >> 12;
        col[i] = shade | (shade << 8) | (shade << 16);
    }
    mp = &m.m;
    shape.v0 = shape.v1 = 0xC0;
    shape.v2 = shape.v3 = 0xDF;
    shape.flags = 9;
    u = ((u8)this->unkCC & 3) << 5;
    shape.u0 = shape.u2 = u;
    shape.u1 = shape.u3 = u + 0x1F;
    vramY = this->vramY;
    shape.clut = getClut(0x1C0, vramY + 0xF7);
    shape.tpage = (((vramY & 0x100) >> 4) | 0x27) | ((vramY & 0x200) << 2);
    setIdentityAt(&m, &this->position);
    scale = this->unkDC;
    matrixSetRotationZ(mp, 0x200);
    matrixRotateX(mp, 0x80);
    matrixScale(mp, scale, scale, scale);
    setModelMatrix(mp);
    for (i = 0; i < 15; i++) {
        shape.colors[0] = shape.colors[2] = col[i];
        shape.colors[1] = shape.colors[3] = col[i + 2];
        shape.corners[0] = &band[i][0];
        shape.corners[1] = &band[i + 2][0];
        shape.corners[2] = &band[i][1];
        shape.corners[3] = &band[i + 2][1];
        packet = buildPolyGT4(&shape, packet, ot, shift);
    }
    matrixSetRotationZ(mp, -0x200);
    matrixRotateX(mp, 0x80);
    matrixScale(mp, scale, scale, scale);
    setModelMatrix(mp);
    for (i = 0; i < 15; i++) {
        shape.colors[0] = shape.colors[2] = col[i];
        shape.colors[1] = shape.colors[3] = col[i + 2];
        shape.corners[0] = &band[i][0];
        shape.corners[1] = &band[i + 2][0];
        shape.corners[2] = &band[i][1];
        shape.corners[3] = &band[i + 2][1];
        packet = buildPolyGT4(&shape, packet, ot, shift);
    }
    PRIM_BUFFER_FREE = packet;
}

/* WarGreymon's destructor. */
void WarGreymon_destroy(Entity *this, s32 flags) {
    Fighter_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The throw effect's destructor. */
void WarGreymonThrowEffect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The special move 1 hitbox's destructor. */
void WarGreymonSpecial1Hitbox_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = WARGREYMON_SPECIAL1_HITBOX_HIT_VTABLE;
    this->entity.vtable = &WARGREYMON_SPECIAL1_HITBOX_VTABLE;
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
void WarGreymonSpecial1Effect_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA7C, which func_800BCAD8 installs. */
void func_800BEEDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA54, which func_800BD1C4 installs. */
void func_800BEF24(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CA2C, which func_800BD490 installs. */
void func_800BEF6C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006C9FC and D_8006C9DC,
 * which func_800BDA50 installs: game has 2 identical ones. */
void func_800BEFB4(UnkPartsEntity38 *this, s32 flags) {
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

/* The destructor of func_800BE39C's class: its virtual tables, then func_800A926C's destructor. */
void func_800BF054(Unk800BE39C *this, s32 flags) {
    this->base.unk18.vtable = D_8006C98C;
    this->base.entity.vtable = &D_8006C9AC;
    func_800A9428(&this->base, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C98C);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C9AC);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C9DC);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C9FC);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006CA2C);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006CA54);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006CA7C);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_SPECIAL1_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_SPECIAL1_HITBOX_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_SPECIAL1_HITBOX_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_THROW_EFFECT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", WARGREYMON_VTABLE);
