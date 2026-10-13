/* The computer's input: pad words made from random numbers for a side without a pad. */

#include "common.h"
#include "battle/game_classes.h"

/* The class that func_8007C17C constructs on Unk8007C084 (virtual table D_8006569C, 0x6C bytes): the object of
 * func_800702E0, the virtual table slot the stages override. */
typedef struct Unk8007C17C {
    /* 0x00 */ Unk8007C084 base;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC[0x1C - 0xC];
    /* 0x1C */ Vec3 unk1C; /* the position of the part func_8007EDD8 found */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ Vec3 unk2C; /* that part's unk20 */
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ s32 unk3C; /* that part's unk44, in 1/256ths */
    /* 0x40 */ s32 unk40; /* whether func_8007EDD8 found a part */
    /* 0x44 */ u8 unk44[0x4C - 0x44];
    /* 0x4C */ s16 unk4C;
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ s16 unk50;
    /* 0x52 */ s16 unk52;
    /* 0x54 */ s16 unk54;
    /* 0x56 */ s16 unk56;
    /* 0x58 */ s16 unk58;
    /* 0x5A */ s16 unk5A; /* -1 after the constructor */
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ u8 unk60[0x62 - 0x60];
    /* 0x62 */ s16 unk62;
    /* 0x64 */ u8 unk64[0x6C - 0x64];
} Unk8007C17C;

extern VtableEntry D_8006569C[];

/* Constructs an Unk8007C17C on func_8007C084's with arg: clears its fields, 0x5A to -1. */
Unk8007C17C *func_8007C17C(Unk8007C17C *this, s32 arg) {
    func_8007C084(&this->base, arg);
    this->base.vtable = D_8006569C;
    this->unk8 = 0;
    this->unkA = 0;
    this->unk40 = 0;
    this->unk4E = 0;
    this->unk50 = 0;
    this->unk52 = 0;
    this->unk54 = 0;
    this->unk62 = 0;
    this->unk56 = 0;
    this->unk58 = 0;
    this->unk4C = 0;
    this->unk5A = -1;
    this->unk5C = 0;
    this->unk5E = 0;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/cpu_input", D_80065434);

INCLUDE_ASM("game/nonmatchings/cpu_input", func_8007C1EC);

/* Looks for the part nearest to chara's func_8007A650 point in one of the battle object's two lists (picked by
 * func_80077574): keeps its position, its unk20 and its unk44 (in 1/256ths), and
 * returns whether there was one. */
s32 func_8007EDD8(Unk8007C17C *this, Unk80076F34 *chara) {
    Unk8006ED20 *battle = D_800D377C;
    Vec3 point;
    Unk80075EE4 *part;

    func_8007A650(chara, &point);
    this->unk40 = 0;
    /* The match depends on testing == 0: the game puts the 0x230 list in the branch's delay slot. */
    part = func_80075D44(func_80077574(chara) == 0 ? &battle->unk228 : &battle->unk230, &point, &this->unk1C);
    if (part != NULL) {
        s32 x, y, z;

        this->unk40 = 1;
        /* The match depends on reading the three words before storing any, as vecCopy does. */
        x = part->unk20.vx;
        y = part->unk20.vy;
        z = part->unk20.vz;
        this->unk2C.vx = x;
        this->unk2C.vy = y;
        this->unk2C.vz = z;
        this->unk3C = part->unk44 << 8;
    }
    return this->unk40;
}

/* Returns flags with 0x2000 set when side is not negative, else with 0x8000. */
s32 func_8007EE80(s32 flags, s32 side) {
    /* The match depends on the early return for a negative side (an if/else gives one exit). */
    if (side < 0) {
        return flags | 0x8000;
    }
    return flags | 0x2000;
}

/* The destructor of func_8007C17C's object (virtual table D_8006569C): Unk8007C084's. */
void func_8007EE98(Unk8007C084 *this, s32 flags) {
    func_8007C09C(this, flags);
}

INCLUDE_RODATA("game/nonmatchings/cpu_input", D_8006569C);
