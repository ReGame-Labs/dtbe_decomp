/* One of the 24 fighters: its constructor, func_800BB034, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"

void func_800BB2B8(Unk80076F34 *this, s32 kind, s32 arg2, s32 arg3, s32 rgb);
void func_800BE7D0(Entity *this, s32 arg);
void func_800BEA1C(Entity *this, s32 arg);
extern VtableEntry D_8006CAD4[];
extern EntityVtable D_8006CAF4;

extern EntityVtable D_8006CB44;

/* 4 bytes func_800BE39C copies to 0x94 of its entity */
extern CVECTOR D_8006C964;

/* The class func_800BE39C constructs (virtual table D_8006C9AC, and D_8006C98C for its base at 0x18),
 * derived from func_800A926C's. */
typedef struct Unk800BE39C {
    /* 0x00 */ Unk800A9944 base;
} Unk800BE39C;

extern VtableEntry D_8006C98C[];
extern EntityVtable D_8006C9AC;

/* Constructs an Unk800BB034: func_80076F34's entity with virtual table
 * D_8006CB44, its eight words 0 but 0x4B0, which is 1. */
Unk800BB034 *func_800BB034(Unk800BB034 *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006CB44;
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

/* A method of Unk800BB034: when 0x4AC is 1, clears it and the character's 0x3F0. */
void func_800BB08C(Unk800BB034 *this) {
    if (this->unk4AC == 1) {
        this->unk4AC = 0;
        this->base.unk3F0 = 0;
    }
}

/* Overrides func_8007B70C (entry 0xA8 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB0AC(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 16, 2, 8, 0x404040);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 4. */
void func_800BB104(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 4) {
        func_800BB2B8(this, 5, 2, 6, 0x404040);
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 0. */
void func_800BB15C(Unk80076F34 *this, s32 arg) {
    func_8007B80C(this, arg);
    if (arg == 0) {
        func_800BB2B8(this, 5, 2, 8, 0x404040);
    }
}

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB1B0(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 5, 3, 14, 0xFFFFFF);
    }
}

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 3. */
void func_800BB208(Unk80076F34 *this, s32 arg) {
    func_8007BAFC(this, arg);
    if (arg == 3) {
        func_800BB2B8(this, 5, 2, 8, 0x404040);
    }
}

/* Overrides func_8007BB64 (entry 0x108 of Unk80076F34's vtable) in D_8006CB44 and D_8006D044: calls it,
 * then func_800BB2B8 when arg is 2. */
void func_800BB260(Unk80076F34 *this, s32 arg) {
    func_8007BB64(this, arg);
    if (arg == 2) {
        func_800BB2B8(this, 5, 3, 14, 0xFFFFFF);
    }
}

/* func_800C5680's tables for func_800BB2B8: three pairs of entries, D_800D34AC for kind 16, D_800D353C otherwise. */
extern Unk800C5B18Entry D_800D34AC[6];
extern Unk800C5B18Entry D_800D353C[6];

void func_800C5B84(Unk800C5680 *this, CVECTOR *colors, s32 count);

/* Creates three func_800C5680 entities (0x690 bytes) for the character, one for each pair of entries of its table
 * (D_800D34AC for kind 16, else D_800D353C) with arg2 and arg3, colors them rgb (a 0xBBGGRR word, read as its
 * three bytes with cd 0) and hands them to func_8007AD4C. */
void func_800BB2B8(Unk80076F34 *this, s32 kind, s32 arg2, s32 arg3, s32 rgb) {
    Unk800C5B18Entry *entries = kind == 16 ? D_800D34AC : D_800D353C;
    CVECTOR *bytes = (CVECTOR *)&rgb;
    CVECTOR color = { bytes->r, bytes->g, bytes->b };
    Unk800C5B18Entry *end = entries + 6;

    do {
        Entity *entity = NEW_ENTITY(0x690, func_800C5680, this, entries, 2, arg2, arg3);

        if (entity != NULL) {
            func_800C5B84((Unk800C5680 *)entity, &color, 1);
            func_8007AD4C(this, entity);
        }
        entries += 2;
    } while (entries != end);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C844);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BB3CC);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C8A4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BB5F8);

/* The entity func_800BB78C constructs (0x28 bytes, virtual table D_8006CB1C): it throws the partner of a
 * character (func_8007AEEC) toward a side. */
typedef struct Unk800BB78C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 handle; /* the partner's (entityGetHandle) */
    /* 0x1C */ Unk80076F34 *chara;
    /* 0x20 */ s32 facing; /* func_80079E10 of chara */
    /* 0x24 */ s32 y;      /* the partner's height (func_800775D8) */
} Unk800BB78C;

extern EntityVtable D_8006CB1C;

/* Constructs func_800BB78C's entity for chara: keeps the partner's handle and height, and throws the partner
 * (func_80078EF8) with velocity (facing * 0xE38, 0xD548, 0). */
Unk800BB78C *func_800BB78C(Unk800BB78C *this, Unk80076F34 *chara, s32 facing) {
    Unk80076F34 *partner;
    Vec3 vec;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006CB1C;
    partner = func_8007AEEC(chara);
    this->handle = entityGetHandle(&partner->entity);
    this->chara = chara;
    this->facing = facing;
    func_800775D8(partner, &vec);
    this->y = vec.vy;
    vec.vx = facing * 0xE38;
    vec.vy = 0xD548;
    vec.vz = 0;
    func_80078EF8(partner, 0x40000000, &vec, 0);
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BB850);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C8B4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C8FC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BB9A4);

void func_800BC0CC(Unk800BB034 *this, s32 arg);
void func_800BC0E0(Unk800BB034 *this, s32 arg);
void func_800BC13C(Unk800BB034 *this);
void func_800BC258(Unk800BB034 *this);
void func_800BC424(Unk800BB034 *this, s32 angle);
s32 func_800BC444(Unk800BB034 *this, s32 *angle, s32 from);

/* The method at 0xE4 of Unk800BB034 (virtual table D_8006CB44): on the first call (arg 0) plays sound 0x10E
 * or 0x120, starts with func_800BC0CC(2), func_800BC0E0(0x18) and the angle func_800BC444 gives (kept when
 * within 0x400 of straight ahead); then by its frame count at 0x4C0 turns toward 0x3E0 (frame 8),
 * finishes (frame 9), mirrors 0x3A4 into 0x3B4 (frames 0-2), looks again from frame 11. */
void func_800BBD18(Unk800BB034 *this, s32 arg) {
    s32 angle;
    s32 target;
    s32 facing;

    func_80079AE4(&this->base);
    if (arg == 0) {
        if (func_80077568(&this->base) == 0x15) {
            func_800784F4(&this->base, 0x10E);
        } else {
            func_800784F4(&this->base, 0x120);
        }
        this->unk4B0 = 1;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x18);
        facing = func_80079E10(&this->base);
        /* The match depends on shifting the facing at each use: the shift then comes after the
         * other arguments */
        func_800BC444(this, &angle, facing << 10);
        this->unk4B8 = abs(angle) <= 0x400 ? angle : facing << 10;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.unk3E0 + ((((this->unk4B8 - this->base.unk3E0 - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 9) {
        this->base.unk3B4.vx = -this->base.unk3A4.vx;
        this->base.unk3B4.vy = -this->base.unk3A4.vy;
        this->base.unk3B4.vz = -this->base.unk3A4.vz;
        this->base.unk3C0 = 0;
    }
    if (this->unk4C0 >= 0x11 && func_800BC444(this, &target, this->base.unk3E0)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 10) {
        func_800BC258(this);
    }
    if (func_80079A0C(&this->base)) {
        this->base.unk3F0 = 0;
    }
    this->unk4C0++;
}

/* The method at 0x11C of Unk800BB034: func_800BBD18 with func_800BC0E0(0x19) and 0x4B0 cleared first,
 * keeping func_800BC444's angle as it is, and frames 3 and 11 for 9 and 0x11. */
void func_800BBF00(Unk800BB034 *this, s32 arg) {
    s32 angle;
    s32 target;

    func_80079AE4(&this->base);
    if (arg == 0) {
        if (func_80077568(&this->base) == 0x15) {
            func_800784F4(&this->base, 0x10E);
        } else {
            func_800784F4(&this->base, 0x120);
        }
        this->unk4B0 = 0;
        func_800BC0CC(this, 2);
        func_800BC0E0(this, 0x19);
        angle = func_80079E10(&this->base) << 10;
        func_800BC444(this, &angle, angle);
        this->unk4B8 = angle;
    }
    switch (this->unk4C0) {
    case 8:
        func_800BC424(this, this->base.unk3E0 + ((((this->unk4B8 - this->base.unk3E0 - 0x800) & 0xFFF) - 0x800) >> 1));
        break;
    case 9:
        func_800BC424(this, this->unk4B8);
        func_800BC13C(this);
        break;
    }
    if (this->unk4C0 < 3) {
        this->base.unk3B4.vx = -this->base.unk3A4.vx;
        this->base.unk3B4.vy = -this->base.unk3A4.vy;
        this->base.unk3B4.vz = -this->base.unk3A4.vz;
        this->base.unk3C0 = 0;
    }
    if (this->unk4C0 >= 11 && func_800BC444(this, &target, this->base.unk3E0)) {
        this->unk4B0 = 0;
        this->unk4B8 = target;
        func_800BC0E0(this, 0x19);
    }
    if (this->unk4C0 >= 4) {
        func_800BC258(this);
    }
    if (func_80079A0C(&this->base)) {
        this->base.unk3F0 = 0;
    }
    this->unk4C0++;
}

/* Sets 0x4AC, 0x4B4 to arg, and clears 0x448 of the character. */
void func_800BC0CC(Unk800BB034 *this, s32 arg) {
    this->unk4AC = 1;
    this->unk4B4 = arg;
    this->base.unk448 = 0;
}

/* When 0x4B4 is above 0: plays clip (func_80079830), clears unk3A4 and 0x3B0, 0x4BC, 0x4C0 and the
 * character's 0x448. */
void func_800BC0E0(Unk800BB034 *this, s32 clip) {
    if (this->unk4B4 > 0) {
        func_80079830(&this->base, clip, 0, 0, -1);
        this->base.unk3A4.vx = 0;
        this->base.unk3A4.vy = 0;
        this->base.unk3A4.vz = 0;
        this->base.unk3B0 = 0;
        this->unk4BC = 0;
        this->unk4C0 = 0;
        this->base.unk448 = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC13C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC258);

/* Creates an entity of func_800BD490's class (0x120 bytes) with g++'s new, and hands it to func_8007AD4C. */
void func_800BC3C8(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x120);

    if (child != NULL) {
        func_800BD490(entityOperatorNewAt(0x120, child), this);
    }
    func_8007AD4C(this, child);
}

/* While 0x4B4 is positive, sets the character's angle (0x3E0) and 0x3F0 to angle. */
void func_800BC424(Unk800BB034 *this, s32 angle) {
    if (this->unk4B4 > 0) {
        this->base.unk3E0 = angle;
        this->base.unk3F0 = angle;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC444);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC59C);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC6F4);

/* Creates an entity of func_800BDA50's class (0xD4 bytes) at pos with g++'s new and hands it to func_8007AD4C, then
 * plays sound 0x109, sets unk498 to 10 and takes the level of the object at 0x238 down to 0. */
void func_800BC7CC(Unk80076F34 *this, Vec3 *pos, s32 arg) {
    Entity *child = allocEntityMemory(0xD4);

    if (child != NULL) {
        func_800BDA50(entityOperatorNewAt(0xD4, child), this, pos, arg, 0x300);
    }
    func_8007AD4C(this, child);
    func_80078540(this, 0x109);
    this->unk498 = 10;
    func_80077448(this, -0x10000);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC870);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BC9DC);

/* The second base's first method for virtual table D_8006CAD4: func_8007BDF0 with sum, then deletes the
 * entity when func_80078878 finds an entity in sum. */
void func_800BCA70(UnkPartsEntity38 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->base, sum);
    if (func_80078878(sum) != NULL) {
        entityDelete(&this->entity);
    }
}

/* The virtual table of Unk800C5680: a method at 0x20 (func_800C5C4C). */
typedef struct Unk800C5680Vtable {
    /* 0x00 */ EntityVtable base;
    /* 0x20 */ VtableEntry unk20;
} Unk800C5680Vtable;

/* An Unk800C5680's virtual table as its own class sees it. */
#define UNK800C5680_VTABLE(object) ((Unk800C5680Vtable *)(object)->entity.vtable)

/* The entity class func_800BCAD8 constructs (virtual table D_8006CA7C): an Unk800C5680 that slides along x,
 * faster each frame toward the character's facing. */
typedef struct Unk800BCAD8 {
    /* 0x000 */ Unk800C5680 base;
    /* 0x690 */ s32 x;     /* the x of its matrix (func_800BCE1C) */
    /* 0x694 */ s32 speed; /* added to x each frame */
    /* 0x698 */ s32 facing; /* func_80079E10: speed grows by facing * 10 */
    /* 0x69C */ s32 mode;  /* 1: func_800BCC7C once the timer ends */
} Unk800BCAD8;

extern EntityVtable D_8006CA7C;
void func_800BCBD8(Unk800BCAD8 *this);
void func_800BCC7C(Unk800BCAD8 *this);

/* Constructs an Unk800BCAD8 for chara: func_800C5680's entity with x and speed 0, the character's facing and
 * mode. */
Unk800BCAD8 *func_800BCAD8(Unk800BCAD8 *this, Unk80076F34 *chara, Unk800C5B18Entry *entries, s32 count,
                           s32 arg4, s32 arg5, s32 mode) {
    func_800C5680(&this->base.entity, chara, entries, count, arg4, arg5);
    this->base.entity.vtable = &D_8006CA7C;
    this->x = 0;
    this->speed = 0;
    this->facing = func_80079E10(chara);
    this->mode = mode;
    return this;
}

/* The gather method of Unk800BCAD8: moves x by speed and speed by facing * 10, func_800BCBD8, in mode 1
 * func_800BCC7C once the timer ends, then the base's gather (func_800C5778). */
void func_800BCB4C(Unk800BCAD8 *this, Unk80075EE4 *sum) {
    this->x += this->speed;
    this->speed += this->facing * 10;
    func_800BCBD8(this);
    if (this->mode == 1 && this->base.unk24 <= 0) {
        func_800BCC7C(this);
    }
    func_800C5778(&this->base, sum);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BCBD8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BCC7C);

/* The update method of Unk800BCAD8: sets the model matrix to a translation by x, then runs the method at 0x20
 * (func_800C5C4C). */
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BCE94);

/* The entity func_800BCE94 constructs for a character (virtual table D_8006CAAC): a matrix at the
 * character's position, turned to its facing, that moves along x; 0x40 eases to 0x44, then 0x48 fades. */
typedef struct Unk800BCE94 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MATRIX matrix;
    /* 0x38 */ s32 x;      /* the translation along the matrix's x, 4 more each frame */
    /* 0x3C */ s32 facing; /* func_80079E10 */
    /* 0x40 */ s32 unk40;  /* eases to unk44, by half the difference each frame */
    /* 0x44 */ s32 unk44;  /* func_800BCE94's third argument */
    /* 0x48 */ s32 unk48;  /* 0x80, then 0x10 less each frame once unk4C is set */
    /* 0x4C */ s32 unk4C;  /* set once unk40 reaches unk44 */
    /* 0x50 */ s32 frames;
    /* 0x54 */ s32 vramY;  /* func_8007768C */
} Unk800BCE94;

/* The gather method of Unk800BCE94: moves the matrix by x, x += 4; eases unk40 to unk44, then fades unk48 by
 * 0x10 and deletes the entity once it is below 0. */
void func_800BCFBC(Unk800BCE94 *this) {
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD07C);

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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD254);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD334);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD490);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD650);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BD8DC);

/* Constructs func_800BDA50's entity for owner at pos: its base at 0x18 (func_8007BD8C, 0x5B04000) and its
 * member at 0x38 (func_80075EE4) on no velocity, the owner's handle, arg3 at 0xC8 and arg4 at 0xB8. */
Unk800BDA50 *func_800BDA50(Unk800BDA50 *this, Unk80076F34 *owner, Vec3 *pos, s32 arg3, s32 arg4) {
    s32 x, y, z;

    entityInit(&this->entity, 0x30000);
    func_8007BD8C(&this->unk18, owner, 0x5B04000, (Vec3 *)&D_80010814, 0);
    this->unk18.vtable = D_8006C9DC;
    this->entity.vtable = &D_8006C9FC;
    func_80075EE4(&this->unk38, (Vec3 *)&D_80010814, 0, &this->unk18);
    this->unk38.unk14 = &this->entity;
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
    this->vramY = func_8007768C(owner);
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C944);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BDB80);

/* The virtual table of func_800BDA50's class (D_8006C9FC): Entity's methods, then the one at 0x20 (func_800BDFCC). */
typedef struct Unk800BDA50Vtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry unk20;
} Unk800BDA50Vtable;

/* The second base's first method for virtual table D_8006C9DC: func_8007BDF0 with sum; when func_80078878 finds a
 * character in sum, takes its position to 0xA4, sets 0xC4 to 2, runs the method at 0x20 with func_8007AEEC's
 * character and unlinks the part at 0x38. */
void func_800BDF28(Unk800BDA50 *this, Unk80075EE4 *sum) {
    Entity *chara;
    Unk80076F34 *owner;
    Unk800BDA50Vtable *vtable;
    ListNode *node;

    func_8007BDF0(&this->unk18, sum);
    chara = func_80078878(sum);
    if (chara != NULL) {
        func_800775D8((Unk80076F34 *)chara, &this->unkA4);
        this->unkC4 = 2;
        owner = func_8007AEEC((Unk80076F34 *)chara);
        vtable = (Unk800BDA50Vtable *)this->entity.vtable;
        vtable->unk20.func((u8 *)this + vtable->unk20.delta, owner);
        node = &this->unk38.node;
        this->unk38.node.next->prev = this->unk38.node.prev;
        node->prev->next = this->unk38.node.next;
        this->unk38.node.next = node;
        this->unk38.node.prev = node;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C964);

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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BE0C4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BE39C);

/* Stops the sound at 0xD4. */
void func_800BE4AC(Unk800BE39C *this) {
    stopSound(this->base.unkD4);
    this->base.unkD4 = 0;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BE4DC);

/* The second base's first method for virtual table D_8006C98C: func_8007BDF0 with sum, then unlinks the part at
 * 0x38 when func_80078878 finds an entity in sum. */
void func_800BE71C(Unk800BE39C *this, Unk80075EE4 *sum) {
    ListNode *node;

    func_8007BDF0(&this->base.unk18, sum);
    if (func_80078878(sum) != NULL) {
        node = &this->base.unk38.node;
        this->base.unk38.node.next->prev = this->base.unk38.node.prev;
        node->prev->next = this->base.unk38.node.next;
        this->base.unk38.node.next = node;
        this->base.unk38.node.prev = node;
    }
}

/* The update of the entities of D_8006C9AC: func_800A9998, func_800BE7D0, then func_800BEA1C. */
void func_800BE788(Entity *this, s32 arg) {
    func_800A9998(this, arg);
    func_800BE7D0(this, arg);
    func_800BEA1C(this, arg);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C968);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BE7D0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_21", func_800BEA1C);

/* The destructor of the entity class of vtable D_8006CB44, which func_800BB034 installs. */
void func_800BED64(Entity *this, s32 flags) {
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CB1C, which func_800BB78C installs. */
void func_800BEDAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the UnkPartsEntity38 class of vtables D_8006CAF4 and D_8006CAD4, which
 * func_800BC870 installs. */
void func_800BEDF4(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006CAD4;
    this->entity.vtable = &D_8006CAF4;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006CAAC, which func_800BCE94 installs. */
void func_800BEE94(Entity *this, s32 flags) {
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
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
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

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C98C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C9AC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C9DC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006C9FC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CA2C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CA54);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CA7C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CAAC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CAD4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CAF4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CB1C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_21", D_8006CB44);
