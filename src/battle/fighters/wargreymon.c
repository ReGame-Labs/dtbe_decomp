/* WarGreymon (CHARACTER_WARGREYMON): the fighter createFighter builds for it, its moves, and the entities they spawn:
 * the attacks' trails, the throw's effect, the special move 1's hitbox, effect and trail, the special move 2's
 * effects, the hitbox of its slot 0x128 and the intro's hitbox. BlackWarGreymon derives from it. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/sound/sound_control.h"

void WarGreymon_spawnAttackTrails(Fighter *this, s32 kind, s32 arg2, s32 arg3, s32 rgb);
void func_800BE7D0(Entity *this, s32 arg);
void func_800BEA1C(Entity *this, s32 arg);
extern VtableEntry WARGREYMON_SPECIAL1_HITBOX_HIT_VTABLE[];
extern EntityVtable WARGREYMON_SPECIAL1_HITBOX_VTABLE;

extern EntityVtable WARGREYMON_VTABLE;

/* 4 bytes func_800BE39C copies to 0x94 of its entity */
extern CVECTOR D_8006C964;

/* The class func_800BE39C constructs (virtual table D_8006C9AC, and D_8006C98C for its base at 0x18),
 * derived from func_800A926C's. */
typedef struct Unk800BE39C {
    /* 0x00 */ Unk800A9944 base;
} Unk800BE39C;

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

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C8A4);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BB5F8);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymonThrowEffect_tick);

INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C8B4);
INCLUDE_RODATA("game/nonmatchings/fighters/wargreymon", D_8006C8FC);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymon_special1);

void func_800BC0CC(WarGreymon *this, s32 arg);
void func_800BC0E0(WarGreymon *this, s32 arg);
void func_800BC13C(WarGreymon *this);
void func_800BC258(WarGreymon *this);
void func_800BC424(WarGreymon *this, s32 angle);
s32 func_800BC444(WarGreymon *this, s32 *angle, s32 from);

/* Special move 2: frame 0 the voice (its own for CHARACTER_WARGREYMON), the move started with func_800BC0CC(2) and
 * func_800BC0E0(0x18), and the angle func_800BC444 gives, kept when within 0x400 of straight ahead; then by its frame
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
        func_800BC444(this, &angle, facing << 10);
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
    if (this->unk4C0 >= 0x11 && func_800BC444(this, &target, this->base.rotation.vz)) {
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
 * func_800BC444's angle as it is, and frames 3, 11 and 4 for 9, 0x11 and 10. */
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
        func_800BC444(this, &angle, angle);
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
    if (this->unk4C0 >= 11 && func_800BC444(this, &target, this->base.rotation.vz)) {
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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BC13C);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BC444);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymon_special3);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymon_airSpecial3);

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

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymonSpecial1Effect_init);

/* The special move 1's effect: a matrix at the character's position, turned to its facing, that moves along x; 0x40
 * eases to 0x44, then 0x48 fades. */
typedef struct WarGreymonSpecial1Effect {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MATRIX matrix;
    /* 0x38 */ s32 x;      /* the translation along the matrix's x, 4 more each frame */
    /* 0x3C */ s32 facing; /* Fighter_getFacing */
    /* 0x40 */ s32 unk40;  /* eases to unk44, by half the difference each frame */
    /* 0x44 */ s32 unk44;  /* WarGreymonSpecial1Effect_init's third argument */
    /* 0x48 */ s32 unk48;  /* 0x80, then 0x10 less each frame once unk4C is set */
    /* 0x4C */ s32 unk4C;  /* set once unk40 reaches unk44 */
    /* 0x50 */ s32 frames;
    /* 0x54 */ s32 vramY;  /* Fighter_getVramY */
} WarGreymonSpecial1Effect;

/* The special move 1 effect's tick: moves the matrix by x, x += 4; eases unk40 to unk44, then fades unk48 by
 * 0x10 and deletes the entity once it is below 0. */
void WarGreymonSpecial1Effect_tick(WarGreymonSpecial1Effect *this) {
    s32 step;

    matrixTranslate(&this->matrix, this->x, 0, 0);
    this->x += 4;
    if (this->unk4C != 0) {
        this->unk48 -= 0x10;
        if (this->unk48 < 0) {
            entityDelete(&this->entity);
            return;
        }
    } else {
        step = (this->unk44 - this->unk40) >> 1;
        this->unk40 += step;
        if (step == 0) {
            this->unk4C = 1;
        }
    }
    this->frames++;
}

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", WarGreymonSpecial1Effect_draw);

/* The entity func_800BD1C4 constructs (0x48 bytes, virtual table D_8006CA54): a piece thrown up from a
 * position, falling and growing in one side and shrinking in the other for 0x14 frames (func_800BD254). */
typedef struct Unk800BD1C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[0x28 - 0x24];
    /* 0x28 */ Vec3 velocity; /* vy grows by 1 each frame; vx takes a random term */
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ s32 unk38; /* 0x40, 2 more each frame */
    /* 0x3C */ s32 unk3C; /* 0x40, 2 less each frame */
    /* 0x40 */ s32 life;  /* frames left: the entity deletes itself below 0 */
    /* 0x44 */ s32 unk44; /* func_800BD1C4's last argument */
} Unk800BD1C4;

extern EntityVtable D_8006CA54;

/* Constructs func_800BD1C4's entity at position, thrown up at 0x14, for 0x14 frames. */
Unk800BD1C4 *func_800BD1C4(Unk800BD1C4 *this, VECTOR position, s32 arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006CA54;
    this->velocity.vx = 0;
    this->velocity.vy = -0x14;
    this->velocity.vz = 0;
    this->unk3C = 0x40;
    this->unk38 = 0x40;
    this->unk44 = arg;
    this->life = 0x14;
    this->position.vx = position.vx;
    this->position.vy = position.vy;
    this->position.vz = position.vz;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD254);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD334);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD490);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD650);

INCLUDE_ASM("game/nonmatchings/fighters/wargreymon", func_800BD8DC);

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
