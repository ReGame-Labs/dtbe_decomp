/* WarGreymon (CHARACTER_WARGREYMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the attacks' trails, the throw's effect, the special move 1's hitbox, effect and trail, the special move 2's
 * effects, the hitbox of its slot 0x128 and the intro's hitbox. BlackWarGreymon derives from it. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

void WarGreymon_spawnAttackTrails(Fighter *this, s32 kind, s32 arg2, s32 arg3, s32 rgb);
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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymon_throw);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymon_special1);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BC258);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymonSpecial1Hitbox_tick);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BCBD8);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BCC7C);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD334);

/* The entity func_800BD490 constructs (0x120 bytes, virtual table D_8006CA2C): the effect the WarGreymonVtable slot
 * 0x120 spawns, drawn as a band of 8 quads. */
typedef struct Unk800BD490 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ MATRIX matrix;          /* func_800BD8DC's model matrix */
    /* 0x3C */ SVECTOR band[8][2];     /* the band's points, two per step */
    /* 0xBC */ u8 unkBC[0x114 - 0xBC];
    /* 0x114 */ s32 shade;             /* red, and a quarter of it green */
    /* 0x118 */ s32 unk118;
    /* 0x11C */ s32 vramY;             /* the VRAM half of the clut and tpage */
} Unk800BD490;

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD490);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BDB80);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BE0C4);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BE39C);

/* Stops the sound at 0xD4. */
void func_800BE4AC(Unk800BE39C *this) {
    stopSound(this->base.unkD4);
    this->base.unkD4 = 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BE4DC);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BE7D0);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BEA1C);

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
