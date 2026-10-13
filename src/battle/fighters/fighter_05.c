/* One of the 24 fighters: its constructor, func_8008D21C, which the factory func_80076CB4 calls, and its
 * classes. */

#include "common.h"
#include <abs.h>
#include "battle/game_classes.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"

Entity *func_8008F474(Entity *this, Unk80076F34 *arg, VECTOR *pos, Unk8006ED20 *arg3);
extern VECTOR D_800690E0;
extern Unk800C5B18Entry D_800690F0[];
extern EntityVtable D_800691EC;
extern VtableEntry D_80069214[];
extern EntityVtable D_80069234;
extern EntityVtable D_8006925C;
extern VtableEntry D_80069284[];
extern EntityVtable D_800692A4;
extern EntityVtable D_800692CC;
extern VtableEntry D_800692F4[];
extern EntityVtable D_80069314;
extern EntityVtable D_8006933C;
extern EntityVtable D_80069364;
extern EntityVtable D_8006938C;

/* The entity func_8008F474 constructs (virtual table D_800692CC): ten points
 * scattered around a centre, each with an angle. */
typedef struct Unk8008F474 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x38 - 0x18];
    /* 0x38 */ Vec3 unk38; /* the centre, func_8008F474's third argument */
    /* 0x44 */ s32 unk44;
    /* 0x48 */ VECTOR unk48[10];
    /* 0xE8 */ s32 unkE8[10];
} Unk8008F474;
void func_8008E51C(Unk80076F34 *this);
void func_8008E5C4(Unk80076F34 *this, s32 arg);
void func_8008E900(Unk80076F34 *this);
/* the constructors of the entities spawned below */
Entity *func_8008EAB4(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
Entity *func_800902E0(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);
struct Unk80091498 *func_80091498(struct Unk80091498 *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj);
extern VECTOR D_800690D0;

/* Constructs an object of the entity class of vtable D_8006938C, which derives from func_80076F34's,
 * its fields from 0x4A6 cleared. */
Unk8008D21C *func_8008D21C(Unk8008D21C *this) {
    func_80076F34(&this->base);
    this->base.entity.vtable = &D_8006938C;
    this->unk4A6 = 0;
    this->unk4A8 = 0;
    this->unk4AA = 0;
    this->unk4B0 = 0;
    this->unk4B4 = 0;
    this->unk4B8 = 0;
    this->unk4BC = 0;
    this->unk4C0 = 0;
    this->unk4C4 = 0;
    this->unk4C8 = 0;
    this->unk4CC = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006938C, which func_8008D21C installs. */
void func_8008D27C(Entity *this, s32 flags) {
    this->vtable = &D_8006938C;
    func_800776D0(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Overrides entry 0x20 of Unk80076F34's vtable in D_8006938C and D_80069564: clears 0x4AA and, while 0x4AC
 * is set, calls func_80079274 (1) on func_8007AEEC's character and clears 0x4AC and 0x4A8. */
void func_8008D2D0(Unk8008D21C *this) {
    this->unk4AA = 0;
    if (this->unk4AC != 0) {
        func_80079274(func_8007AEEC(&this->base), 1);
        this->unk4AC = 0;
        this->unk4A8 = 0;
    }
}

/* Overrides func_8007B884 (entry 0xC0 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it,
 * then func_8007A6FC when arg is 17. */
void func_8008D31C(Unk80076F34 *this, s32 arg) {
    func_8007B884(this, arg);
    if (arg == 17) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008D368);

/* Overrides func_8007B6A0 (entry 0xA0 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it;
 * at arg 4 clears unk3A4.vx and moves unk3B4 by unk3C4 * 0x71C0 (16.16) toward func_80079E10's side, at
 * arg 7 calls func_8008E900; from 5 on, moves unk3B4 back by a sixteenth of unk3A4. The match depends on
 * the step being an s64 local: the game keeps both halves of the product across the call. */
void func_8008D584(Unk80076F34 *this, s32 arg) {
    func_8007B6A0(this, arg);
    switch (arg) {
    case 4: {
        s64 step = ((s64)this->unk3C4 * 0x71C0) >> 16;

        this->unk3A4.vx = 0;
        this->unk3B4.vx += (s32)step * func_80079E10(this);
        break;
    }
    case 7:
        func_8008E900(this);
        break;
    }
    if (arg >= 5) {
        this->unk3B4.vx -= this->unk3A4.vx >> 4;
    }
}

/* An override of Unk80076F34's method at 0xA8 (virtual tables D_8006938C and
 * D_80069564): func_8007B70C, then func_8008E900 when arg is 8. */
void func_8008D650(Unk80076F34 *this, s32 arg) {
    func_8007B70C(this, arg);
    if (arg == 8) {
        func_8008E900(this);
    }
}

/* Overrides func_8007B784 (entry 0xB0 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it,
 * then func_8007A6FC when arg is 22. */
void func_8008D694(Unk80076F34 *this, s32 arg) {
    func_8007B784(this, arg);
    if (arg == 22) {
        func_8007A6FC(this, 1, 80, 8);
    }
}

/* The override of Unk80076F34's method at 0xD8 for virtual tables D_8006938C and D_80069564: for arg 0
 * starts clip 0x16, func_8008E5C4(0), sets 0x334 to 0x1E and plays sound 0x9A (0xA3 unless 0x218 is 5);
 * for arg 10 spawns func_8008E51C's entity. */
void func_8008D6E0(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x16, 0, 0, -1);
        func_8008E5C4(this, 0);
        func_80079B0C(this, 0x1E);
        if (func_80077568(this) == 5) {
            func_800784F4(this, 0x9A);
        } else {
            func_800784F4(this, 0xA3);
        }
        break;
    case 10:
        func_8008E51C(this);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008D788);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008D97C);

/* Overrides func_8007BAFC (entry 0x100 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it;
 * at step 5 unk3B4.vy falls by unk3C4 * 0x6388 (16.16); a rising unk3A4 slows unk3B4. */
void func_8008DB98(Unk80076F34 *this, s32 step) {
    func_8007BAFC(this, step);
    if (step == 5) {
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x6388) >> 16;
    }
    if (this->unk3A4.vy > 0) {
        this->unk3B4.vy -= this->unk3A4.vy;
    }
}

/* The override of Unk80076F34's method at 0x110 for virtual tables D_8006938C and D_80069564: for arg 0
 * starts clip 0x17, func_8008E5C4(1), sets 0x334 to 0x14 and plays sound 0x9A (0xA3 unless 0x218 is 5);
 * for arg 10 spawns func_8008E51C's entity. */
void func_8008DC24(Unk80076F34 *this, s32 arg) {
    switch (arg) {
    case 0:
        func_80079830(this, 0x17, 2, 0, -1);
        func_8008E5C4(this, 1);
        func_80079B0C(this, 0x14);
        if (func_80077568(this) == 5) {
            func_800784F4(this, 0x9A);
        } else {
            func_800784F4(this, 0xA3);
        }
        break;
    case 10:
        func_8008E51C(this);
        break;
    }
}

/* Overrides func_8007B80C (entry 0xB8 of Unk80076F34's vtable) in D_8006938C and D_80069564: calls it;
 * at step 6 unk3B4.vy falls by unk3C4 * 0x71C0 (16.16), at step 16 it is cleared. */
void func_8008DCCC(Unk80076F34 *this, s32 step) {
    func_8007B80C(this, step);
    switch (step) {
    case 6:
        this->unk3B4.vy -= ((s64)this->unk3C4 * 0x71C0) >> 16;
        break;
    case 16:
        this->unk3B4.vy = 0;
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008DD50);

s32 func_80078D64(Unk80076F34 *this);
Unk80076F34 *func_80079CF4(Unk80076F34 *this, s32 index);
VECTOR *func_8008DD50(VECTOR *out, Unk8008D21C *this);
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Unk80076F34 *owner, Unk8006ED20 *unk228, Vec3 *pos, s32 size);
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Unk80076F34 *owner, Unk8006ED20 *unk228, Vec3 *pos, s32 size);

/* The entity class of vtable D_800691C4: func_800C32E0's (0x48 bytes) with a halfword more; its draw is
 * func_80091328 and its destructor func_80091998. */
typedef struct Unk800919EC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0x48 - 0x18];
    /* 0x48 */ u16 unk48;
} Unk800919EC;

extern EntityVtable D_800691C4;

/* func_800919EC's body, which func_8008DE84 has inline. */
static inline void unk800919ECConstruct(Unk800919EC *this, u16 unk48, Vec3 *position, VECTOR *unk, s32 unk3,
                                        u32 color) {
    func_800C32E0((struct Unk800C32E0 *)this, position, unk, unk3, color);
    this->entity.vtable = &D_800691C4;
    this->unk48 = unk48;
}

/* Ends the hold: clears the flags func_8008DE84 keeps from 0x4A6 to 0x4AC and 0x448. */
static inline void func_8008DE84_release(Unk8008D21C *this) {
    this->unk4A6 = 0;
    this->base.unk448 = 0;
    this->unk4A8 = 0;
    this->unk4AC = 0;
    this->unk4AA = 0;
}

/* A step of the hold func_8008D97C runs (fighter_05 and fighter_06): in the first 20 steps it grabs the partner
 * (func_8007AEEC) when the partner can be held, then carries it along func_8008DD50's path; given steps spawn
 * effects and set the hit (0x4A6, 0x44C), and the end of the hit spawns a burst at the partner. The JP version
 * lets go when func_8006F190 says so; the US version also calls func_80078EF8 on the partner when it grabs it
 * and sets the partner's 0x48C to 2 after step 20. */
void func_8008DE84(Unk8008D21C *this, s32 step) {
    Vec3 above;
    Vec3 vec;
    Vec3 ownPosition;
    Unk80076F34 *partner = func_8007AEEC(&this->base);
    Vec3 *p;
    Entity *child;
    s32 i;
    s32 dx;
    s32 x, y, z;

#if VERSION_JP
    if (func_8006F190(func_80077580(&this->base))) {
        if (this->unk4A8 != 0) {
            func_800775D8(partner, &vec);
            vec.vx = this->unk4F0;
            func_800775B0(partner, &vec);
        }
        func_80079320(&this->base);
        func_8008DE84_release(this);
        return;
    }
#endif
    this->unk4AC = 1;
    /* The match depends on the range test: the game compares the step unsigned. */
    if (step >= 0 && step < 20 && this->unk4A8 == 0) {
        if (!func_80078DE0(partner) && !func_80078D64(partner) && func_80079CF4(&this->base, 0) != NULL) {
#if !VERSION_JP
            func_80078EF8(partner, 0x40000001, (Vec3 *)&D_80010814, 0);
#endif
            func_80079274(partner, 60 - step);
            this->unk4A8 = 1;
            partner->unk3A4.vx = 0;
            partner->unk3A4.vy = 0;
            partner->unk3A4.vz = 0;
            partner->unk3B0 = 0;
            this->unk4D0.vx = 0;
            this->unk4D0.vy = 0;
            this->unk4D0.vz = 0;
            this->unk4DC = 0;
            this->unk4E0.vx = 0;
            this->unk4E0.vy = 0;
            this->unk4E0.vz = 0;
            this->unk4EC = 0;
            /* The match depends on reading the position's three words before any store, as vecCopy does. */
            p = func_800775D8(partner, &vec);
            x = p->vx;
            y = p->vy;
            z = p->vz;
            this->base.unk3A4.vx = 0;
            this->base.unk3A4.vy = 0;
            this->base.unk3A4.vz = 0;
            this->base.unk3B0 = 0;
            this->unk4C0 = x;
            this->unk4C4 = y;
            this->unk4C8 = z;
            this->unk4B0 = x;
            this->unk4B4 = y;
            this->unk4B8 = z;
            func_800775D8(&this->base, &ownPosition);
            dx = ownPosition.vx - vec.vx;
            if (ABS(dx) < 0x140) {
                this->base.unk3B4.vx = dx >= 0 ? 0x1C70 : -0x1C70;
            }
        }
    }
    if (this->unk4A8 != 0 && this->unk4A4 == 0) {
#if !VERSION_JP
        if (step > 20) {
            partner->unk48C = 2;
        }
#endif
        func_8008DD50((VECTOR *)&vec, this);
        func_800775B0(partner, &vec);
    }
    switch (step) {
    case 0: {
        /* The match depends on unk228 and spark being set once in their block: sched1 then sinks the copy
         * of the result next to the call that follows. */
        Unk8006ED20 *unk228;

        func_800775D8(partner, &vec);
        this->unk4A4 = 0;
        this->unk4F0 = vec.vx;
        func_80079830(&this->base, 0x1A, 0, 0, -1);
        func_80077448(&this->base, -0x10000);
        unk228 = func_80077580(&this->base);
        func_800775D8(&this->base, &vec);
        for (i = 0; i < 16; i++) {
            Entity *spark = allocEntityMemory(0x58);

            if (spark != NULL) {
                func_8008E9B4(entityOperatorNewAt(0x58, spark), &this->base, unk228, &vec, 0x400 + i * 0x180);
            }
            func_8007AD4C(&this->base, spark);
        }
        this->base.unk3A4.vx = 0;
        this->base.unk3A4.vy = 0;
        this->base.unk3A4.vz = 0;
        this->base.unk3B0 = 0;
        this->base.unk498 = 10;
        break;
    }
    case 3: {
        /* The match depends on push being an s64 local: the game keeps both halves of it across the call. */
        s64 push = ((s64)this->base.unk3C4 * 0x38E0) >> 16;

        this->base.unk3B4.vx += (s32)push * func_80079E10(&this->base);
        break;
    }
    case 6:
        this->unk4A6 = 4;
        this->base.unk44C = 0x5B01500;
        this->unk4AA = 1;
        break;
    case 12:
        this->unk4A6 = 8;
        this->base.unk44C = 0x5B01500;
        this->unk4AA = 1;
        break;
    case 19: {
        s64 push = ((s64)this->base.unk3C4 * 0x1C70) >> 16;

        this->base.unk3B4.vx += (s32)push * func_80079E10(&this->base);
        this->base.unk3B4.vy -= ((s64)this->base.unk3C4 * 0x71C0) >> 16;
        break;
    }
    case 20:
        if (this->unk4A8 != 0) {
            vec.vx = 0;
            vec.vy = -0x78DC;
            vec.vz = 0;
            this->unk4D0 = vec;
            this->unk4E0.vx = 0;
            this->unk4E0.vy = 0;
        }
        this->unk4A6 = 0x10;
        this->base.unk44C = 0x5B01500;
        this->unk4AA = 1;
        break;
    case 10:
    case 27:
        this->base.unk3A4.vx = 0;
        break;
    case 28:
        this->unk4A6 = 0x20;
        this->unk4E0.vx = 0;
        this->base.unk44C = 0x5B01500;
        this->unk4AA = 1;
        break;
    case 42:
        this->base.unk3B4.vy = 0;
        break;
    case 50:
        if (this->unk4A8 != 0) {
            vec.vx = 0;
            vec.vy = 0x1C70;
            vec.vz = 0;
            func_80078EF8(partner, 0x45B01500, (Vec3 *)&D_80010814, 0);
            this->base.unk3A4.vx = 0;
            this->unk4AA = 1;
            this->unk4D0 = vec;
        }
        this->unk4A6 = 0;
        break;
    }
    if (this->unk4A6 != 0) {
        func_8007A0E4(&this->base, this->unk4A6);
    } else {
        this->base.unk448 = 0;
    }
    if (!func_80079A0C(&this->base)) {
#if VERSION_JP
        if (this->base.unk44C == 0 && this->unk4AA != 0) {
#else
        if (!func_80078DE0(partner) && this->unk4AA != 0 && this->unk4A8 != 0) {
#endif
            Unk8006ED20 *unk228 = func_80077580(&this->base);

            func_8007A650(partner, &above);
            child = allocEntityMemory(0x4C);
            if (child != NULL) {
                unk800919ECConstruct(entityOperatorNewAt(0x4C, child), func_8007768C(&this->base), &above,
                                     &D_80010814, 0x180, 0xFFFFFF);
            }
            func_8007AD4C(&this->base, child);
            for (i = 0; i < 8; i++) {
                Entity *spark = allocEntityMemory(0x58);

                if (spark != NULL) {
                    func_8008EA58(entityOperatorNewAt(0x58, spark), partner, unk228, &above, 0x100 + i * 0x40);
                }
                func_8007AD4C(&this->base, spark);
            }
            this->unk4AA = 0;
        }
    } else {
        func_8008DE84_release(this);
    }
}

/* Spawns func_800C32E0's entity (0x48 bytes) at the partner's position (func_8007AEEC, func_800775D8),
 * with no velocity, 0x60 and color 0x808080. */
void func_8008E48C(Unk80076F34 *this) {
    Unk80076F34 *partner = func_8007AEEC(this);
    Vec3 position;
    VECTOR velocity;
    Entity *child;

    velocity.vx = 0;
    velocity.vy = 0;
    velocity.vz = 0;
    velocity.pad = 0;
    func_800775D8(partner, &position);
    child = allocEntityMemory(0x48);
    if (child != NULL) {
        func_800C32E0(entityOperatorNewAt(0x48, child), &position, &velocity, 0x60, 0x808080);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800690D0);

/* Spawns the entity of func_8008EAB4 at an offset from node 0xC of the
 * character's model, with velocity (64 forward, 0, 0). */
void func_8008E51C(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_8007A4A4(this, (VECTOR *)&position, 0xC, &D_800690D0);
    velocity.vx = func_80079E10(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    child = allocEntityMemory(0x160);
    if (child != NULL) {
        func_8008EAB4(entityOperatorNewAt(0x160, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800690E0);

/* Creates an entity of func_8008F474's class (0x150 bytes) at the owner's point 12 moved by D_800690E0,
 * with arg in the vector's pad, and hands it to func_8007AD4C. */
void func_8008E5C4(Unk80076F34 *this, s32 arg) {
    Unk8006ED20 *obj = func_80077580(this);
    VECTOR pos;
    Entity *child;

    func_8007A4A4(this, &pos, 12, &D_800690E0);
    pos.pad = arg;
    child = allocEntityMemory(0x150);
    if (child != NULL) {
        func_8008F474(entityOperatorNewAt(0x150, child), this, &pos, obj);
    }
    func_8007AD4C(this, child);
}

Entity *func_8008FD18(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Unk8006ED20 *unk228);

/* Spawns the entity of func_8008FD18 (0xA4 bytes) at node 0xC of the character's model, with
 * velocity (64 forward, 0, 0). */
void func_8008E658(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    VECTOR offset;
    Entity *child;

    velocity.vx = func_80079E10(this) * 64;
    velocity.vy = 0;
    velocity.vz = 0;
    offset.vx = 0;
    offset.vy = 0;
    offset.vz = 0;
    offset.pad = 0;
    func_8007A4A4(this, (VECTOR *)&position, 0xC, &offset);
    child = allocEntityMemory(0xA4);
    if (child != NULL) {
        func_8008FD18(entityOperatorNewAt(0xA4, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800690F0);

/* Creates an entity of func_800C5680's class (0x690 bytes) with g++'s new and the table
 * D_800690F0, and hands it to func_8007AD4C. */
void func_8008E70C(Unk80076F34 *this) {
    Entity *child = allocEntityMemory(0x690);

    if (child != NULL) {
        func_800C5680(entityOperatorNewAt(0x690, child), this, D_800690F0, 3, 8, 14);
    }
    func_8007AD4C(this, child);
}

/* Spawns the entity of func_800902E0 at the character (func_800775D8), with
 * velocity (256 forward, -64, 0). */
void func_8008E784(Unk80076F34 *this) {
    Unk8006ED20 *unk228 = func_80077580(this);
    Vec3 position;
    Vec3 velocity;
    Entity *child;

    func_800775D8(this, &position);
    velocity.vx = func_80079E10(this) * 256;
    velocity.vy = -64;
    velocity.vz = 0;
    child = allocEntityMemory(0x920);
    if (child != NULL) {
        func_800902E0(entityOperatorNewAt(0x920, child), this, &position, &velocity, unk228);
    }
    func_8007AD4C(this, child);
}

extern VECTOR D_80069138;
Entity *func_80090F04(void *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, Vec3 *unk4);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069138);

/* Spawns the entity of func_80090F04 (0xAC bytes) at node 1 of the character's model moved by D_80069138
 * and 256 forward, with velocity (256 forward, 0, 0) and (0x71C0 forward, -0x38E0, 0). */
void func_8008E824(Unk80076F34 *this) {
    Vec3 position;
    Vec3 velocity;
    Vec3 unk4;
    Entity *child;

    func_8007A4A4(this, (VECTOR *)&position, 1, &D_80069138);
    position.vx += func_80079E10(this) * 256;
    velocity.vx = func_80079E10(this) * 256;
    velocity.vy = 0;
    velocity.vz = 0;
    unk4.vx = func_80079E10(this) * 0x71C0;
    unk4.vy = -0x38E0;
    unk4.vz = 0;
    child = allocEntityMemory(0xAC);
    if (child != NULL) {
        func_80090F04(entityOperatorNewAt(0xAC, child), this, &position, &velocity, &unk4);
    }
    func_8007AD4C(this, child);
}

/* Spawns a burst of 8 (func_800C62EC, kind 6) at the character moved 0xC0 forward and 0x20 up. */
void func_8008E900(Unk80076F34 *this) {
    Vec3 position;
    Entity *child;

    func_800775D8(this, &position);
    position.vx += func_80079E10(this) * 0xC0;
    position.vy -= 0x20;
    child = allocEntityMemory(0x3C);
    if (child != NULL) {
        func_800C62EC(entityOperatorNewAt(0x3C, child), 6, &position, 0x10, 0x50, 8, 0xA0A0A0);
    }
    func_8007AD4C(this, child);
}

/* Constructs an Unk8008E9B4 (vtable D_80069364) for owner: func_800C3CFC with pos, arg4 and 0xFFC080,
 * then 6 in unk50. */
Unk8008E9B4 *func_8008E9B4(Unk8008E9B4 *this, Unk80076F34 *owner, Unk8006ED20 *unk228, Vec3 *pos, s32 size) {
    func_800C3CFC(&this->base, pos, size, 0xFFC080);
    /* The match depends on this order: vtable, unk54, unk50. */
    this->base.entity.vtable = &D_80069364;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

/* The gather of the Unk8008E9B4 of D_80069364: func_800C3E64, then puts the world position of the
 * owner's point 12 (func_8007A4A4) at 0x28. */
void func_8008EA10(Unk8008E9B4 *this, s32 arg) {
    VECTOR origin;

    func_800C3E64(&this->base);
    /* The match depends on filling origin after the call, one field at a time. */
    origin.vx = 0;
    origin.vy = 0;
    origin.vz = 0;
    origin.pad = 0;
    func_8007A4A4(this->unk54, &this->base.unk28, 12, &origin);
}

/* Constructs an Unk8008E9B4 with vtable D_8006933C (see func_8008E9B4). */
Unk8008E9B4 *func_8008EA58(Unk8008E9B4 *this, Unk80076F34 *owner, Unk8006ED20 *unk228, Vec3 *pos, s32 size) {
    func_800C3CFC(&this->base, pos, size, 0xFFC080);
    /* The match depends on this order, as in func_8008E9B4. */
    this->base.entity.vtable = &D_8006933C;
    this->unk54 = owner;
    this->base.unk50 = 6;
    return this;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008EAB4);

/* The destructor of the UnkPartsEntity38 class of vtables D_80069314 and D_800692F4, which
 * func_8008EAB4 installs. */
void func_8008ED14(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_800692F4;
    this->entity.vtable = &D_80069314;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008EDB4);

/* The second base's first method for virtual table D_800692F4: func_8007BDF0 with sum; when func_80078878
 * finds a character in sum, takes its position (unused) and sets bit 0 of the part's flags. */
void func_8008EFDC(UnkPartsEntity38 *this, Unk80075EE4 *sum) {
    Entity *chara;
    Vec3 position;

    func_8007BDF0(&this->base, sum);
    chara = func_80078878(sum);
    if (chara != NULL) {
        func_800775D8((Unk80076F34 *)chara, &position);
        this->part.flags |= 1;
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008F038);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008F474);

/* The destructor of the entity class of vtable D_800692CC, which func_8008F474 installs. */
void func_8008F64C(Entity *this, s32 flags) {
    this->vtable = &D_800692CC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069148);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008F6A0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008F7F8);

/* The direction (ratan2) from the centre at 0x38 to point index, on x and y. */
s16 func_8008F9B0(Unk8008F474 *this, s32 index) {
    return ratan2(this->unk48[index].vx - this->unk38.vx, this->unk48[index].vy - this->unk38.vy);
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008F9F4);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008FD18);

/* The destructor of the UnkPartsEntity38 class of vtables D_800692A4 and D_80069284, which
 * func_8008FD18 installs. */
void func_8008FEF0(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80069284;
    this->entity.vtable = &D_800692A4;
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

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8008FF90);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_80090074);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069158);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_8009013C);

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069178);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_800902E0);

/* The destructor of the entity class of vtable D_8006925C, which func_800902E0 installs. */
void func_8009050C(Entity *this, s32 flags) {
    this->vtable = &D_8006925C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class of vtable D_8006925C (0x920 bytes, func_800902E0 constructs it for a character): 64 particles
 * drawn to a point of the character, with a square of four corners. */
typedef struct Unk800902E0 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Unk80076F34 *chara;
    /* 0x01C */ Unk8006ED20 *obj;    /* func_800902E0's fifth argument; func_8006F004 draws numbers from it */
    /* 0x020 */ s16 unk20;           /* frames in state 2 */
    /* 0x022 */ u8 unk22;            /* 0x80 from func_800902E0, eased to 0 */
    /* 0x023 */ u8 unk23;            /* 0 from func_800902E0, eased to 0x84 */
    /* 0x024 */ s16 side;            /* the sign of the velocity's x */
    /* 0x026 */ u16 timer;           /* counts the frames */
    /* 0x028 */ u16 state;
    /* 0x02A */ s16 flags;           /* func_8007768C of the character */
    /* 0x02C */ u16 count;           /* particles in use: 16 more every frame, up to 64 */
    /* 0x02E */ u8 unk2E[0x34 - 0x2E];
    /* 0x034 */ s32 unk34;
    /* 0x038 */ u8 unk38[0x48 - 0x38];
    /* 0x048 */ VECTOR position;     /* the character's point 1 moved by D_80069188 (func_8007A4A4) */
    /* 0x058 */ VECTOR unk58;        /* position, 128 toward side */
    /* 0x068 */ u8 unk68[0x78 - 0x68];
    /* 0x078 */ SVECTOR corners[4];  /* (+-0x80, +-0x40, 0) */
    /* 0x098 */ s32 unk98;           /* 0x1000 from func_800902E0, eased to 0x3200 */
    /* 0x09C */ s32 unk9C;           /* 0x1000 from func_800902E0, halved every frame of state 3 */
    /* 0x0A0 */ s16 unkA0[16][4];
    /* 0x120 */ VECTOR particles[64]; /* positions */
    /* 0x520 */ VECTOR velocities[64];
} Unk800902E0; /* size 0x920 */

extern VECTOR D_80069188;

/* vecAddInPlace (engine/math/vector_ops.c), inlined */
static inline VECTOR *vecAddInPlaceInline(VECTOR *a, VECTOR *b) {
    s32 x = a->vx;
    s32 y = a->vy;
    s32 z = a->vz;

    x += b->vx;
    y += b->vy;
    z += b->vz;
    a->vx = x;
    a->vy = y;
    a->vz = z;
    return a;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069188);

/* The update of Unk800902E0: follows the character's point 1 moved by D_80069188; moves the particles, and puts
 * one that came within 32 of the point back at random, a quarter of its offset as velocity; eases unk22 and unk23.
 * Then by state: after 6 frames makes func_80091498's entity, grows unk98 toward 0x3200 while unk58 bobs, waits
 * 12 frames, halves unk9C below 5 and deletes itself. */
void func_80090560(Unk800902E0 *this) {
    VECTOR offset = D_80069188;
    VECTOR distance;
    VECTOR *particle;
    VECTOR *velocity;
    Entity *child;
    s32 i;

    func_8007A4A4(this->chara, &this->position, 1, &offset);
    this->unk58.vy = this->position.vy;
    this->unk58.vz = this->position.vz;
    this->unk58.vx = this->position.vx + (this->side << 7);
    this->count += 16;
    if (this->count > 64) {
        this->count = 64;
    }
    for (i = 0; i < this->count; i++) {
        /* The match depends on velocity's address taken before particle's (loop makes the later one the base
         * register) and on vecAddInPlace inlined; the reset below indexes the arrays again. */
        velocity = &this->velocities[i];
        particle = &this->particles[i];
        vecAddInPlaceInline(particle, velocity);
        distance.vx = particle->vx - this->position.vx;
        distance.vy = particle->vy - this->position.vy;
        distance.vz = particle->vz - this->position.vz;
        if (vecGetLength(&distance) < 32) {
            this->particles[i].vx = (func_8006F004(this->obj) & 0xFFF) - 0x800;
            this->particles[i].vy = (func_8006F004(this->obj) & 0xFFF) - 0x800;
            this->velocities[i].vx = -this->particles[i].vx >> 2;
            this->velocities[i].vy = -this->particles[i].vy >> 2;
        }
    }
    if (this->unk23 != 0x80) {
        this->unk22 += (0 - this->unk22) >> 2;
        this->unk23 += (0x84 - this->unk23) >> 2;
    } else {
        this->unk22 = 0;
    }
    switch (this->state) {
    case 0:
        if (this->timer < 6) {
            break;
        }
        child = allocEntityMemory(0x38);
        if (child != NULL) {
            /* position is a VECTOR here, a Vec3 to func_80091498 (it reads the three words) */
            func_80091498(entityOperatorNewAt(0x38, child), this->chara, (Vec3 *)&this->position, this->obj);
        }
        func_8007AD4C(this->chara, child);
        this->state++;
        /* fallthrough */
    case 1:
        /* The match depends on the sine as the first operand (unk58.vy += ... swaps the registers). */
        this->unk58.vy = (COS_SIN_TABLE[(this->timer << 10) & 0xC00].sin >> 7) + this->unk58.vy;
        this->unk98 += (0x3200 - this->unk98) >> 1;
        if (this->unk98 < 0x3100) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 2:
        this->unk20++;
        if (this->unk20 < 12) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 3:
        this->unk9C -= this->unk9C >> 1;
        if (this->unk9C >= 5) {
            break;
        }
        this->state++;
        /* fallthrough */
    case 4:
        entityDelete(&this->entity);
        return;
    }
    this->timer++;
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_800908E8);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_80090F04);

/* The destructor of the UnkPartsEntity38 class of vtables D_80069234 and D_80069214, which
 * func_80090F04 installs. */
void func_80091060(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_80069214;
    this->entity.vtable = &D_80069234;
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

/* The class func_80090F04 constructs (0xAC bytes, virtual tables D_80069234 and D_80069214, spawned by
 * func_8008E824): parts that move from pos by vel every frame until they have gone 0x3000. */
typedef struct Unk80090F04 {
    /* 0x00 */ UnkPartsEntity38 parts;
    /* 0x80 */ Unk8008D21C *owner;
    /* 0x84 */ s32 speed;    /* vel's length (vecGetLength) */
    /* 0x88 */ s32 distance; /* grows by speed every frame */
    /* 0x8C */ Vec3 vel;
    /* 0x98 */ s32 unk98;
    /* 0x9C */ Vec3 pos;
    /* 0xA8 */ s32 unkA8;
} Unk80090F04;

/* Unk80090F04's update: moves pos by vel and puts the part there; deletes itself when the owner's 0x4A4 is
 * set or its 0x4AC is clear, or once it has gone over 0x3000. */
void func_80091100(Unk80090F04 *this) {
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    s32 x = this->pos.vx;
    s32 y = this->pos.vy;
    s32 z = this->pos.vz;

    x += this->vel.vx;
    y += this->vel.vy;
    z += this->vel.vz;
    this->pos.vx = x;
    this->pos.vy = y;
    this->pos.vz = z;
    this->parts.part.unk34.vx = x;
    this->parts.part.unk34.vy = y;
    this->parts.part.unk34.vz = z;
    if (this->owner->unk4A4 != 0 || this->owner->unk4AC == 0) {
        entityDelete(&this->parts.entity);
    } else {
        this->distance += this->speed;
        if (this->distance > 0x3000) {
            entityDelete(&this->parts.entity);
        }
    }
}

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_800911EC);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_80091328);

/* The entity class of vtable D_800691EC (0x38 bytes), made for a character 0x40 below a position. */
typedef struct Unk80091498 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076F34 *chara;
    /* 0x1C */ s16 facing; /* func_80079E10 of the character: 1 or -1 */
    /* 0x1E */ u16 unk1E; /* frames: func_80091584 counts it up to 41 */
    /* 0x20 */ u8 unk20;
    /* 0x21 */ u8 unk21;
    /* 0x22 */ s16 unk22; /* the character's 0x440 (func_8007768C) */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ Vec3 position;
} Unk80091498;

/* Constructs the entity of vtable D_800691EC for a character, 0x40 below position. obj is not read
 * (func_80090560 passes it). */
Unk80091498 *func_80091498(Unk80091498 *this, Unk80076F34 *chara, Vec3 *position, Unk8006ED20 *obj) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800691EC;
    this->unk22 = func_8007768C(chara);
    this->chara = chara;
    this->unk1E = 0;
    this->unk20 = 0;
    this->facing = func_80079E10(chara);
    this->unk24 = 0x1C00;
    x = position->vx;
    y = position->vy + 0x40;
    z = position->vz;
    /* The match depends on storing vy last. */
    this->position.vx = x;
    this->position.vz = z;
    this->position.vy = y;
    return this;
}

/* The destructor of the entity class of vtable D_800691EC, which func_80091498 installs. */
void func_80091530(Entity *this, s32 flags) {
    this->vtable = &D_800691EC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk80091498's update: unk20 eases a quarter of the way to 0x80, after 10 frames unk24 eases a quarter
 * of the way to 0, and after 40 frames it deletes itself. */
void func_80091584(Unk80091498 *this) {
    this->unk20 += (0x80 - this->unk20) >> 2;
    if (this->unk1E > 10) {
        this->unk24 += -this->unk24 >> 2;
    }
    if (this->unk1E > 40) {
        entityDelete(&this->entity);
    } else {
        this->unk1E++;
    }
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800691B0);

INCLUDE_ASM("game/nonmatchings/fighters/fighter_05", func_80091620);

/* The destructor of the entity class of vtable D_80069364, which func_8008E9B4 installs. */
void func_80091908(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006933C, which func_8008EA58 installs. */
void func_80091950(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D4FC, which func_800C32E0
 * installs: game has 2 identical ones. */
void func_80091998(Entity *this, s32 flags) {
    this->vtable = &D_8006D4FC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs an Unk800919EC: func_800C32E0's entity at position, then unk48 and the vtable D_800691C4. */
Unk800919EC *func_800919EC(Unk800919EC *this, u16 unk48, Vec3 *position, VECTOR *unk, s32 unk3, u32 color) {
    /* The match depends on func_800C32E0 being declared to return this (it does): its v0 sends the vtable to v1. */
    func_800C32E0((struct Unk800C32E0 *)this, position, unk, unk3, color); /* the base class constructor */
    this->unk48 = unk48;
    this->entity.vtable = &D_800691C4;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800691C4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800691EC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069214);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069234);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_8006925C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069284);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800692A4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800692CC);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_800692F4);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069314);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_8006933C);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_80069364);
INCLUDE_RODATA("game/nonmatchings/fighters/fighter_05", D_8006938C);
