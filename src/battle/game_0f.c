/* The class of the constructor func_8007F920 and its kin (virtual tables D_80067C44 to D_80068154). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

/* functions of Unk8007F920 that constructors of its kin call */
void func_8007FCFC(Unk8007F920 *this);
void func_8007FED8(Unk8007F920 *this);
void func_8007FF1C(Unk8007F920 *this);

/* returns whether the word that the pointer at 0x230 of the Unk80076F34 of an Unk8007F920 points to is 1 */
s32 func_8007F98C(Unk8007F920 *this);
Unk8007F920 *func_80080BF0(Unk8007F920 *this, s32 arg);
void func_80081A54(Unk8007F920 *this, s32 arg);
/* the steps func_80081A54 runs with the character's input (func_80081E18, 80081EEC and 80081FC4 don't read it) */
void func_80081B84(Unk8007F920 *this, Unk80075000 *input);
void func_80081E18(Unk8007F920 *this, Unk80075000 *input);
void func_80081EEC(Unk8007F920 *this, Unk80075000 *input);
void func_80081FC4(Unk8007F920 *this, Unk80075000 *input);
Unk8007F920 *func_8007F920(Unk8007F920 *this, s32 unk0);

/* the virtual tables the functions below install, and the data they read */
extern VtableEntry D_80067E0C[];
extern VtableEntry D_80067E7C[];
extern VtableEntry D_80067E44[];
extern VtableEntry D_80068154[];
extern VtableEntry D_80068004[];
extern VtableEntry D_80068074[];
extern VtableEntry D_800680E4[];
extern VtableEntry D_8006811C[];

/* A g++ 2.95 pointer to member function, laid out as a VtableEntry: index
 * -1 for a non-virtual function, which is then func. The game keeps such
 * constants in .rodata (D_80067C54 and the ones around it, each
 * {0xFFFF0000, function}). */
typedef VtableEntry MemberFunction;

typedef struct Unk80080C5C {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} Unk80080C5C;

/* The class of virtual table D_80067E44 (func_80081874): func_8007F920's with a word at 0x10. */
typedef struct Unk80081874 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk80081874;

Unk8007F920 *func_80080778(Unk8007F920 *this, s32 unk0);
/* func_800796F0 on the character at 8, from frame 0 */
void func_8007FC2C(Unk8007F920 *this, s32 clip, s32 blendFrames);
void func_8007FC50(Unk8007F920 *this, s32 clip, s32 blendFrames);
void func_80081438(Unk8007F920 *this, s32 arg);
/* Constructs, in place, the state of virtual table D_80067DD4 (on
 * func_80081A20's) with arg and the step function, which it then calls. */
Unk8007F920 *func_8008209C(Unk8007F920 *this, s32 arg, MemberFunction step);
/* the step functions of func_8008209C's states (pointers to member functions) */
extern MemberFunction D_80067C44;
extern MemberFunction D_80067C4C;
extern MemberFunction D_80067C54;
extern MemberFunction D_80067C5C;
extern MemberFunction D_80067C64;
extern MemberFunction D_80067C6C;
extern MemberFunction D_80067C74;
extern MemberFunction D_80067CCC;
extern MemberFunction D_80067CD4;
extern MemberFunction D_80067CDC;
extern MemberFunction D_80067CE4;
extern MemberFunction D_80067CEC;
extern MemberFunction D_80067CF4;
/* Constructs, in place, the state of virtual table D_80067D94 (on
 * func_8007F920's) with arg and the step function at 0x14. */
Unk8007F920 *func_800823EC(Unk8007F920 *this, s32 arg, MemberFunction step);
extern MemberFunction D_80067D04;
extern MemberFunction D_80067D0C;
extern VtableEntry D_80067D14[];
extern VtableEntry D_80067D54[];
Unk8007F920 *func_8008230C(Unk8007F920 *this);
Unk8007F920 *func_80080FDC(Unk8007F920 *this, s32 state);
Unk8007F920 *func_80081200(Unk8007F920 *this, s32 arg);
Unk8007F920 *func_80082690(Unk8007F920 *this);
Unk8007F920 *func_800826E8(Unk8007F920 *this);

/* Constructs an Unk8007F920 with its value (its virtual table D_80068154, 4 cleared). */
Unk8007F920 *func_8007F920(Unk8007F920 *this, s32 unk0) {
    this->vtable = D_80068154;
    this->unk0 = unk0;
    this->unk4 = 0;
    return this;
}

/* Returns the object at 0x22C of the character at 8. */
Unk8007F920 *func_8007F93C(Unk8007F920 *this) {
    return this->unk8->unk22C;
}

/* Returns the pointer at 0x230 of the character at 8 (func_800792AC's; the header types it s32). */
s32 *func_8007F950(Unk8007F920 *this) {
    return (s32 *)this->unk8->unk230;
}

/* Whether the word the character's 0x230 points to is 1. */
s32 func_8007F964(Unk8007F920 *this) {
    return *func_8007F950(this) == 1;
}

/* Whether the character's flag of func_80079AF8 is clear. */
s32 func_8007F98C(Unk8007F920 *this) {
    /* The match depends on the ^ 1 after the mask: ! xors before masking. */
    return ((this->unk8->flags >> UNK80076F34_FLAG_AF8_BIT) & 1) ^ 1;
}

/* Whether the character's last input (func_800750C4) holds button 0x4. */
s32 func_8007F9AC(Unk8007F920 *this) {
    return (func_800750C4(func_800776C4(this->unk8)) & 0x4) != 0;
}

s32 func_8007F9DC(Unk8007F920 *this);
INCLUDE_ASM("game/nonmatchings/game_0f", func_8007F9DC);

/* Whether the character's last input (func_800750C4) holds PAD_DOWN. */
s32 func_8007FAC0(Unk8007F920 *this) {
    return (func_800750C4(func_800776C4(this->unk8)) & PAD_DOWN) != 0;
}

Unk8007F920 *func_80080BA8(Unk8007F920 *this);
Unk8007F920 *func_80081384(Unk8007F920 *this);
Unk8007F920 *func_800813F8(Unk8007F920 *this);

/* Turns this object into another state in place by its state (0) and whether PAD_DOWN is held
 * (func_8007FAC0): held, 9 becomes 10 (func_800813F8) and any other but 2 and 10 becomes 2 (func_80080BA8);
 * not held, 10 becomes 9 (func_80081384) and 2 becomes 1 (func_800808EC). Returns whether it changed. */
s32 func_8007FAF0(Unk8007F920 *this) {
    s32 state = this->unk0;

    if (func_8007FAC0(this)) {
        if (state == 2 || state == 10) {
            return 0;
        }
        if (state == 9) {
            func_800813F8(this);
            return 1;
        }
        func_80080BA8(this);
        return 1;
    }
    if (state == 10) {
        func_80081384(this);
        return 1;
    }
    if (state == 2) {
        func_800808EC(this, 1);
        return 1;
    }
    return 0;
}

/* Returns 0x250 of the character at 8. */
s32 func_8007FB9C(Unk8007F920 *this) {
    return this->unk8->unk250;
}

/* Returns the address of the character's unk3A4. */
Vec3 *func_8007FBB0(Unk8007F920 *this) {
    return &this->unk8->unk3A4;
}

/* Calls func_80078804 for the Unk80076F34 of an Unk8007F920 when func_8007F964 holds. */
void func_8007FBBC(Unk8007F920 *this) {
    if (func_8007F964(this)) {
        func_80078804(this->unk8, 0);
    }
}

/* Calls func_8007881C for the Unk80076F34 of an Unk8007F920 when func_8007F964 holds. */
void func_8007FBF4(Unk8007F920 *this) {
    if (func_8007F964(this)) {
        func_8007881C(this->unk8, 0);
    }
}

/* func_800796F0 on the character at 8, from frame 0. */
void func_8007FC2C(Unk8007F920 *this, s32 clip, s32 blendFrames) {
    func_800796F0(this->unk8, clip, blendFrames, 0);
}

/* func_80079830 on the character at 8, from frame 0, plays -1. */
void func_8007FC50(Unk8007F920 *this, s32 clip, s32 blendFrames) {
    func_80079830(this->unk8, clip, blendFrames, 0, -1);
}

/* Whether the character's model is done with its clip at 0x254 (func_80079A0C) when second is set, else
 * with the one at 0x250 (func_800799A0). */
s32 func_8007FC7C(Unk8007F920 *this, s32 second) {
    s32 done;

    /* The match depends on one result variable: a return in each arm inverts the branch. */
    if (second) {
        done = func_80079A0C(this->unk8);
    } else {
        done = func_800799A0(this->unk8);
    }
    return done;
}

/* Calls the character's virtual method at 0x38 with 0x4, then increments 0x4. */
void func_8007FCB8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk38.func((u8 *)chara + vtable->unk38.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x40 with 0x4, then increments 0x4. */
void func_8007FCFC(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk40.func((u8 *)chara + vtable->unk40.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x48 with 0x4, then increments 0x4. */
void func_8007FD40(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk48.func((u8 *)chara + vtable->unk48.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x50 with 0x4, then increments 0x4. */
void func_8007FD84(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk50.func((u8 *)chara + vtable->unk50.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x58 with 0x4, then increments 0x4. */
void func_8007FDC8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk58.func((u8 *)chara + vtable->unk58.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x60 with 0x4, then increments 0x4. */
void func_8007FE0C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk60.func((u8 *)chara + vtable->unk60.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x68 with 0x4, then increments 0x4. */
void func_8007FE50(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk68.func((u8 *)chara + vtable->unk68.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x70 with 0x4, then increments 0x4. */
void func_8007FE94(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk70.func((u8 *)chara + vtable->unk70.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x78 with 0x4, then increments 0x4. */
void func_8007FED8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk78.func((u8 *)chara + vtable->unk78.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x80 with 0x4, then increments 0x4. */
void func_8007FF1C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk80.func((u8 *)chara + vtable->unk80.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x88 with 0x4, then increments 0x4. */
void func_8007FF60(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk88.func((u8 *)chara + vtable->unk88.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x90 with 0x4, then increments 0x4. */
void func_8007FFA4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk90.func((u8 *)chara + vtable->unk90.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x98 with 0x4, then increments 0x4. */
void func_8007FFE8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk98.func((u8 *)chara + vtable->unk98.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xA0 with 0x4, then increments 0x4. */
void func_8008002C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkA0.func((u8 *)chara + vtable->unkA0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xA8 with 0x4, then increments 0x4. */
void func_80080070(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkA8.func((u8 *)chara + vtable->unkA8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xB0 with 0x4, then increments 0x4. */
void func_800800B4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkB0.func((u8 *)chara + vtable->unkB0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xB8 with 0x4, then increments 0x4. */
void func_800800F8(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkB8.func((u8 *)chara + vtable->unkB8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xC0 with 0x4, then increments 0x4. */
void func_8008013C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkC0.func((u8 *)chara + vtable->unkC0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xC8 with 0x4, then increments 0x4. */
void func_80080180(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkC8.func((u8 *)chara + vtable->unkC8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xD0 with 0x4, then increments 0x4. */
void func_800801C4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkD0.func((u8 *)chara + vtable->unkD0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xD8 with 0x4, then increments 0x4. */
void func_80080208(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkD8.func((u8 *)chara + vtable->unkD8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xE0 with 0x4, then increments 0x4. */
void func_8008024C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkE0.func((u8 *)chara + vtable->unkE0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xE8 with 0x4, then increments 0x4. */
void func_80080290(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkE8.func((u8 *)chara + vtable->unkE8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xF0 with 0x4, then increments 0x4. */
void func_800802D4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkF0.func((u8 *)chara + vtable->unkF0.delta, this->unk4++);
}

/* Calls the character's virtual method at 0xF8 with 0x4, then increments 0x4. */
void func_80080318(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unkF8.func((u8 *)chara + vtable->unkF8.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x100 with 0x4, then increments 0x4. */
void func_8008035C(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk100.func((u8 *)chara + vtable->unk100.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x108 with 0x4, then increments 0x4. */
void func_800803A0(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk108.func((u8 *)chara + vtable->unk108.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x110 with 0x4, then increments 0x4. */
void func_800803E4(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk110.func((u8 *)chara + vtable->unk110.delta, this->unk4++);
}

/* Calls the character's virtual method at 0x118 with 0x4, then increments 0x4. */
void func_80080428(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk118.func((u8 *)chara + vtable->unk118.delta, this->unk4++);
}

/* Reads a press of PAD_LEFT or PAD_RIGHT in the last input (func_800750C4): looks back 10 frames
 * (func_8007516C) and returns 3 (left) or 4 (right) when func_80075454 finds the button twice there, else 1 or
 * 2; 0 without either. */
s32 func_8008046C(Unk80075000 *input) {
    u32 buttons = func_800750C4(input);

    /* The match depends on the full if / else if / else chain: a plain `return 0` after them is moved to the
     * end with the branch inverted. */
    if (buttons & PAD_LEFT) {
        func_8007516C(input, 10);
        if (func_80075454(input, PAD_LEFT, 2, 0)) {
            return 3;
        }
        return 1;
    } else if (buttons & PAD_RIGHT) {
        func_8007516C(input, 10);
        if (func_80075454(input, PAD_RIGHT, 2, 0)) {
            return 4;
        }
        return 2;
    } else {
        return 0;
    }
}

/* func_80081438 with buttons, unless the state of the object at 0x22C (func_8007F93C) is 9 without bit 28 of
 * buttons or 10 without bit 27. */
void func_80080510(Unk8007F920 *this, u32 buttons) {
    /* The match depends on case 9 breaking on its bit: written as case 10, the two tests are merged. */
    switch (func_8007F93C(this)->unk0) {
    case 9:
        if (buttons & 0x10000000) {
            break;
        }
        return;
    case 10:
        if (!(buttons & 0x8000000)) {
            return;
        }
        break;
    }
    func_80081438(this, buttons);
}

/* Calls the character's virtual method at 0x20, then makes this a func_800819E0 object in place. */
void func_80080590(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk20.func((u8 *)chara + vtable->unk20.delta);
    func_800819E0(this);
}

/* The class of virtual table D_800680AC (func_800805DC): func_8007F920's with a flag at 0x10 and a frame
 * count at 0x14. */
typedef struct Unk800805DC {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
} Unk800805DC;

extern VtableEntry D_800680AC[];
s32 func_8007FC7C(Unk8007F920 *this, s32 arg);

INCLUDE_RODATA("game/nonmatchings/game_0f", D_800656D4);

/* Constructs the object of virtual table D_800680AC with 0 at 0. 0x10 is set unless the low two bits
 * of the character's D_800656D4 entry are 1; when it is clear, the character's clip 0x1E is run in
 * (func_800797B8). The match depends on taking the table into a local before the call: the game
 * keeps its address in a saved register across it. */
Unk800805DC *func_800805DC(Unk800805DC *this) {
    Unk80077698 *table;

    func_8007F920(&this->base, 0);
    this->base.vtable = D_800680AC;
    table = D_800656D4;
    this->unk10 = (table[func_80077568(this->base.unk8)].unk0 & 3) != 1;
    if (!this->unk10) {
        func_800797B8(this->base.unk8, 0x1E);
    }
    this->unk14 = 0;
    func_8007FCB8(&this->base);
    return this;
}

/* The tick of D_800680AC's class: at frame 10 starts the character's clip 0x1E (directly when 0x10 is
 * set, else blended in over 0x32 frames); from frame 90, or from frame 11 once func_8007FC7C holds,
 * becomes a func_800808EC object (1), else calls func_8007FCB8. */
void func_80080670(Unk800805DC *this) {
    if (++this->unk14 == 10) {
        if (this->unk10) {
            func_8007FC2C(&this->base, 0x1E, 0);
        } else {
            func_800796F0(this->base.unk8, 0x1E, 0, 0);
            func_800796F0(this->base.unk8, 0x1E, 0x32, 1);
        }
    }
    if (this->unk14 >= 90 || (this->unk14 >= 11 && func_8007FC7C(&this->base, 0))) {
        func_800808EC(&this->base, 1);
    } else {
        func_8007FCB8(&this->base);
    }
}

/* Constructs the object of virtual table D_8006811C with 0x11 at 0 and starts the
 * character's clip 0x1D, blended over 10 frames. */
Unk8007F920 *func_80080730(Unk8007F920 *this) {
    func_8007F920(this, 0x11);
    this->vtable = D_8006811C;
    func_8007FC2C(this, 0x1D, 10);
    return this;
}

/* Constructs an object of the class of vtable D_8006811C, which derives from func_8007F920's. */
Unk8007F920 *func_80080778(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_8006811C;
    return this;
}

/* Constructs the object of virtual table D_8006811C (func_80080778) with 0x12 at 0
 * and starts the character's clip 0x1C, blended over 10 frames. */
Unk8007F920 *func_800807AC(Unk8007F920 *this) {
    func_80080778(this, 0x12);
    this->vtable = D_8006811C;
    func_8007FC2C(this, 0x1C, 10);
    return this;
}

/* The class of virtual table D_8006803C (func_800807F4): func_8007F920's with a frame count at 0x10. */
typedef struct Unk800807F4 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk800807F4;

extern VtableEntry D_8006803C[];

/* Makes this an object of virtual table D_8006803C (0x13 at 0) that counts down `frames`: gives the
 * character clip -1 (func_8007FC2C) and, when func_8007F1C0 holds for the character's object at 0x238,
 * calls func_8007F1B8 on it. */
void *func_800807F4(Unk8007F920 *this, s32 frames) {
    Unk8007F184 *object;

    func_8007F920(this, 0x13);
    this->vtable = D_8006803C;
    ((Unk800807F4 *)this)->unk10 = frames;
    func_8007FC2C(this, -1, 0);
    object = func_8007758C(this->unk8);
    if (func_8007F1C0(object)) {
        func_8007F1B8(object);
    }
    return this;
}

/* The class of virtual table D_80067EB4 (func_800816B4): func_8007F920's with a frame count at 0x10. */
typedef struct Unk800816B4 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk800816B4;

Unk8007F920 *func_800816B4(Unk8007F920 *this, s32 state);

/* The tick of D_8006803C's class: once the 0x20 of the character's object at 0x238 is down to 0,
 * becomes a func_800816B4 object (0xE); else when the count at 0x10 is out, a func_800808EC object (1);
 * else counts it down. */
void func_80080874(Unk800807F4 *this) {
    if (func_8007758C(this->base.unk8)->unk20 <= 0) {
        func_800816B4(&this->base, 0xE);
    } else if (this->unk10 == 0) {
        func_800808EC(&this->base, 1);
    } else if (this->unk10 > 0) {
        this->unk10--;
    }
}

/* The class of virtual table D_80067FCC (func_80080EBC): D_80068004's (func_80080C5C) with a word at
 * 0x1C. */
typedef struct Unk80080EBC {
    /* 0x00 */ Unk80080C5C base;
    /* 0x18 */ u8 unk18[0x1C - 0x18];
    /* 0x1C */ s32 unk1C;
} Unk80080EBC;

Unk8007F920 *func_80080EBC(Unk8007F920 *this, s32 arg1, s32 arg2);

/* Makes this an object of virtual table D_80068074 with `state` at 0. When bit 2 of the character's flags
 * is clear it becomes a func_80080EBC object (0, 0) instead; in state 1 it starts the character's clip 0,
 * blended over 4 frames. */
Unk8007F920 *func_800808EC(Unk8007F920 *this, s32 state) {
    func_8007F920(this, state);
    this->vtable = D_80068074;
    if (!func_800778BC(this->unk8)) {
        func_80080EBC(this, 0, 0);
    } else if (this->unk0 == 1) {
        func_8007FC2C(this, 0, 4);
    }
    return this;
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067AD4);

/* The tick of D_80068074's class: without func_800778BC becomes a func_80080EBC object (0, 0); else unless
 * func_8007F9DC or (with func_8007F964) func_8007FAF0 changed the state: with func_8007F964 in state 1, 3, 4, 5
 * or 6 a PAD_LEFT/PAD_RIGHT press (func_8008046C) makes it a func_80080FDC (5, 6) or func_80081200 (7, 8)
 * object; otherwise, once the 0x20 of the character's object at 0x238 is down to 0, a func_800816B4 object (0xE). */
void func_80080968(Unk8007F920 *this) {
    if (!func_800778BC(this->unk8)) {
        func_80080EBC(this, 0, 0);
        return;
    }
    if (func_8007F9DC(this)) {
        return;
    }
    if (func_8007F964(this) && func_8007FAF0(this)) {
        return;
    }
    /* The match depends on the loop its first break leaves (as in func_80081B84): the game rolls that test to the
     * loop's end and enters it with a jump; a plain if gives the test first. The jump table starts at case 0. */
    do {
        if (!func_8007F964(this) || !(this->unk0 == 1 || this->unk0 == 4 || this->unk0 == 3 || this->unk0 == 5 ||
                                      this->unk0 == 6)) {
            break;
        }
        switch (func_8008046C(func_800776C4(this->unk8))) {
        case 0:
            break;
        case 1:
            func_80080FDC(this, 5);
            return;
        case 2:
            func_80080FDC(this, 6);
            return;
        case 3:
            func_80081200(this, 7);
            return;
        case 4:
            func_80081200(this, 8);
            return;
        }
    } while (0);
    if (func_8007758C(this->unk8)->unk20 <= 0) {
        func_800816B4(this, 0xE);
    }
}

/* The method at 0x10 of D_80068074: for a positive arg, when func_8007F964 holds and the character's
 * object at 0x22C is not in state 2, 9 or 10, becomes a func_80080BF0 object (0). */
void func_80080ADC(Unk8007F920 *this, s32 arg) {
    if (arg > 0 && func_8007F964(this)) {
        switch (func_8007F93C(this)->unk0) {
        case 2:
        case 9:
        case 10:
            break;
        default:
            func_80080BF0(this, 0);
            break;
        }
    }
}

/* The method at 0x18 of D_80068074: when func_80077848 and func_8007F964 hold, becomes a func_80080EBC
 * object (1, 0) and sets the character's 0x48C to 8 (the US build calls func_8007A718 for that). */
void func_80080B4C(Unk8007F920 *this) {
    if (func_80077848(this->unk8) && func_8007F964(this)) {
        func_80080EBC(this, 1, 0);
#if VERSION_JP
        this->unk8->unk48C = 8;
#else
        func_8007A718(this->unk8);
#endif
    }
}

/* Constructs an object of the class of vtable D_80068074 with func_800808EC and 2, then calls
 * func_8007FE94 and func_8007F9DC. */
Unk8007F920 *func_80080BA8(Unk8007F920 *this) {
    func_800808EC(this, 2);
    this->vtable = D_80068074;
    func_8007FE94(this);
    func_8007F9DC(this);
    return this;
}

/* Makes this an object of virtual table D_80068004 with 3 at 0: plays sound 8, keeps arg + 1 at 0x10
 * and clears the count at 0x14. */
Unk8007F920 *func_80080BF0(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, 3);
    this->vtable = D_80068004;
    soundControlPlaySound(&SOUND_CONTROL, 8, 0);
    ((Unk80080C5C *)this)->unk10 = arg + 1;
    func_8007FE50(this);
    ((Unk80080C5C *)this)->unk14 = 0;
    return this;
}

/* Constructs the class of virtual table D_80068004 on func_8007F920's. */
Unk80080C5C *func_80080C5C(Unk80080C5C *this, s32 arg1, s32 arg2) {
    func_8007F920(&this->base, arg1);
    this->base.vtable = D_80068004;
    this->unk10 = arg2;
    return this;
}

Vec3 *func_8007FBB0(Unk8007F920 *this);
void func_80080D38(Unk80080C5C *this);

/* The tick of D_80068004's class: once the vy of the character's 0x3A4 is not negative, becomes a
 * func_80080EBC object (0x10, 1); else, from the second frame on, moves the character's 0x3B4.vy by its
 * scale at 0x3C4 times 0x71C, then calls func_80080D38. The match depends on the step being a local
 * of its own: added in place, the old vy takes the step's register. */
void func_80080CA0(Unk80080C5C *this) {
    s32 vy = func_8007FBB0(&this->base)->vy;
    Unk80076F34 *chara = this->base.unk8;

    if (vy >= 0) {
        func_80080EBC(&this->base, this->unk10, 1);
    } else {
        if (this->unk14) {
            s32 delta = ((s64)chara->unk3C4 * 0x71C) >> 16;

            chara->unk3B4.vy += delta;
        }
        this->unk14 = 1;
        func_80080D38(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_80080D38);

/* The method at 0x10 of D_80068004 and D_80067FCC: clears 0x14, then for a positive arg, when
 * func_8007F964 holds and 0x10 is below 2, becomes a func_80080BF0 object (0x10). */
void func_80080E68(Unk80080C5C *this, s32 arg) {
    this->unk14 = 0;
    if (arg > 0 && func_8007F964(&this->base) && this->unk10 < 2) {
        func_80080BF0(&this->base, this->unk10);
    }
}

extern VtableEntry D_80067FCC[];

/* Makes this an object of virtual table D_80067FCC: func_80080C5C's (4, arg1) with arg2 at 0x1C, starting
 * the character's clip 4, blended over 6 frames. */
Unk8007F920 *func_80080EBC(Unk8007F920 *this, s32 arg1, s32 arg2) {
    func_80080C5C((Unk80080C5C *)this, 4, arg1);
    this->vtable = D_80067FCC;
    ((Unk80080EBC *)this)->unk1C = arg2;
    func_8007FC2C(this, 4, 6);
    return this;
}

/* The tick of D_80067FCC's class: when bit 2 of the character's flags is set and its 0x48C is down to 0,
 * calls func_8007A6FC (1 and -1, 0x60, 2 or 3 after func_80077794), plays sound 9 and becomes a
 * func_800808EC object (1); else calls func_80080D38. */
void func_80080F1C(Unk80080EBC *this) {
    Unk80076F34 *chara = this->base.base.unk8;
    s32 arg;

    if (func_800778BC(chara) && chara->unk48C <= 0) {
        arg = func_80077794(chara) ? 2 : 3;
        func_8007A6FC(chara, 1, 0x60, arg);
        func_8007A6FC(chara, -1, 0x60, arg);
        soundControlPlaySound(&SOUND_CONTROL, 9, 0);
        func_800808EC(&this->base.base, 1);
    } else {
        func_80080D38(&this->base);
    }
}

extern VtableEntry D_80067F94[];

/* Makes this an object of virtual table D_80067F94 with `state` at 0 and plays sound 0xB; states 5 and 6
 * call func_8007FD40 and func_8007FD84. */
Unk8007F920 *func_80080FDC(Unk8007F920 *this, s32 state) {
    func_8007F920(this, state);
    this->vtable = D_80067F94;
    soundControlPlaySound(&SOUND_CONTROL, 0xB, 0);
    if (this->unk0 == 5) {
        func_8007FD40(this);
    }
    if (this->unk0 == 6) {
        func_8007FD84(this);
    }
    return this;
}

/* Without func_800778BC becomes a func_80080EBC object (0, 0); else unless func_8007F9DC changed the state, by
 * the PAD_LEFT/PAD_RIGHT press (func_8008046C): none, or func_8007F964 false, becomes a func_800808EC object (1);
 * 1 and 2 become a func_80080FDC object (5 or 6) unless already in it, then func_8007FD40 or func_8007FD84;
 * 3 and 4 a func_80081200 object (7 or 8). */
void func_80081054(Unk8007F920 *this) {
    s32 press;
    s32 state;

    if (!func_800778BC(this->unk8)) {
        func_80080EBC(this, 0, 0);
        return;
    }
    if (func_8007F9DC(this)) {
        return;
    }
    press = func_8008046C(func_800776C4(this->unk8));
    if (press == 0 || !func_8007F964(this)) {
        func_800808EC(this, 1);
        return;
    }
    state = this->unk0;
    switch (press) {
    /* press is not 0 here, but the game's jump table starts at case 0 (it indexes it by press, not press - 1) */
    case 0:
        if (state == 5) {
            func_8007FD40(this);
        }
        if (state == 6) {
            func_8007FD84(this);
        }
        break;
    case 1:
        if (state != 5) {
            func_80080FDC(this, 5);
        }
        func_8007FD40(this);
        break;
    case 2:
        if (state != 6) {
            func_80080FDC(this, 6);
        }
        func_8007FD84(this);
        break;
    case 3:
        func_80081200(this, 7);
        break;
    case 4:
        func_80081200(this, 8);
        break;
    }
}

/* The method at 0x10 of D_80067F5C and D_80067F94: func_80080BF0(0) when func_8007F964 holds and arg is
 * positive. */
void func_800811B8(Unk8007F920 *this, s32 arg) {
    if (func_8007F964(this) && arg > 0) {
        func_80080BF0(this, 0);
    }
}

extern VtableEntry D_80067F5C[];

/* Makes this an object of virtual table D_80067F5C on func_80080FDC's (arg); states 7 and 8 call
 * func_8007FDC8 and func_8007FE0C. */
Unk8007F920 *func_80081200(Unk8007F920 *this, s32 arg) {
    s32 state;

    func_80080FDC(this, arg);
    this->vtable = D_80067F5C;
    state = this->unk0;
    if (state == 7) {
        func_8007FDC8(this);
    }
    if (state == 8) {
        func_8007FE0C(this);
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_80081264);

/* Constructs an object of the class of vtable D_80068074 with func_800808EC and 9, then calls
 * func_8007FED8. */
Unk8007F920 *func_80081384(Unk8007F920 *this) {
    func_800808EC(this, 9);
    this->vtable = D_80068074;
    func_8007FED8(this);
    return this;
}

/* Constructs an object of the class of vtable D_80068074, which derives from func_800808EC's. */
Unk8007F920 *func_800813C4(Unk8007F920 *this, s32 arg) {
    func_800808EC(this, arg);
    this->vtable = D_80068074;
    return this;
}

/* Constructs an object of the class of vtable D_80068074 with func_800813C4 and 10, then calls
 * func_8007FF1C. */
Unk8007F920 *func_800813F8(Unk8007F920 *this) {
    func_800813C4(this, 10);
    this->vtable = D_80068074;
    func_8007FF1C(this);
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_80081438);

/* The class of virtual table D_80067F24 (func_80081438): func_8007F920's with a frame count at 0x10. */
typedef struct Unk80081438 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk80081438;

Unk8007F920 *func_800815C4(Unk8007F920 *this);

/* The tick of D_80067F24's class: counts 0x10 down; when bit 2 of the character's flags is clear becomes
 * a func_800815C4 object, once the 0x20 of its object at 0x238 is down to 0 a func_800816B4 object (0xE),
 * else when the count is out a func_800808EC object (1). */
void func_80081528(Unk80081438 *this) {
    Unk80076F34 *chara = this->base.unk8;
    Unk8007F184 *object = func_8007758C(chara);

    this->unk10--;
    if (!func_800778BC(chara)) {
        func_800815C4(&this->base);
    } else if (object->unk20 <= 0) {
        func_800816B4(&this->base, 0xE);
    } else if (this->unk10 < 0) {
        func_800808EC(&this->base, 1);
    }
}

/* The class of virtual table D_80067EEC (func_800815C4): func_8007F920's with a frame count at 0x10. */
typedef struct Unk800815C4 {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
} Unk800815C4;

extern VtableEntry D_80067EEC[];

/* Makes this an object of virtual table D_80067EEC with 0xC at 0: clears the character's 0x494
 * (func_8007A12C with 0), counts 4 frames at 0x10 and calls func_8007FFE8. */
Unk8007F920 *func_800815C4(Unk8007F920 *this) {
    func_8007F920(this, 0xC);
    this->vtable = D_80067EEC;
    func_8007A12C(this->unk8, 0);
    ((Unk800815C4 *)this)->unk10 = 4;
    func_8007FFE8(this);
    return this;
}

/* The tick of D_80067EEC's class: once the count at 0x10 runs out and bit 2 of the character's flags is
 * set, calls func_8007A6FC (1 and -1, 0x60, 2 or 3 after func_80077794) and becomes a func_800816B4
 * object (0xE). */
void func_80081618(Unk800815C4 *this) {
    s32 arg;

    if (--this->unk10 < 0 && func_800778BC(this->base.unk8)) {
        arg = func_80077794(this->base.unk8) ? 2 : 3;
        func_8007A6FC(this->base.unk8, 1, 0x60, arg);
        func_8007A6FC(this->base.unk8, -1, 0x60, arg);
        func_800816B4(&this->base, 0xE);
    }
}

extern VtableEntry D_80067EB4[];

/* Makes this an object of virtual table D_80067EB4 with `state` at 0: state 0xE starts the character's
 * clip 0x11 (blended over 2 frames) with no count at 0x10; any other starts clip 0x10 (over 1 frame) and
 * counts 0x16 frames. The match depends on the switch: the game tests 0xD, then 0xE, and falls into the
 * 0xD arm. */
Unk8007F920 *func_800816B4(Unk8007F920 *this, s32 state) {
    func_8007F920(this, state);
    this->vtable = D_80067EB4;
    switch (state) {
    case 0xD:
    default:
        func_8007FC2C(this, 0x10, 1);
        ((Unk800816B4 *)this)->unk10 = 0x16;
        break;
    case 0xE:
        func_8007FC2C(this, 0x11, 2);
        ((Unk800816B4 *)this)->unk10 = 0;
        break;
    }
    return this;
}

Unk8007F920 *func_800817B4(Unk8007F920 *this);

/* The tick of D_80067EB4's class: while the 0x20 of the character's object at 0x238 is above 0, counts
 * 0x10 down; once it is out, bit 2 of the character's flags is set and func_8007FC7C holds, becomes a
 * func_800817B4 object. */
void func_80081734(Unk800816B4 *this) {
    if (func_8007758C(this->base.unk8)->unk20 > 0 && --this->unk10 <= 0 && func_800778BC(this->base.unk8)
        && func_8007FC7C(&this->base, 0)) {
        func_800817B4(&this->base);
    }
}

/* Constructs an object of the class of vtable D_80067E7C with func_8007F920 and 15, then calls
 * func_8007FCFC. */
Unk8007F920 *func_800817B4(Unk8007F920 *this) {
    func_8007F920(this, 15);
    this->vtable = D_80067E7C;
    func_8007FCFC(this);
    return this;
}

Unk80081874 *func_80081874(Unk80081874 *this);

/* The tick of D_80067E7C's class (func_800817B4): once func_8007FC7C holds, becomes a func_80081874
 * object when func_8007F1C0 holds for the character's object at 0x238, else sets the character's 0x498 to
 * 8 and becomes a func_800808EC object (1); until then calls func_8007FCFC. */
void func_800817F4(Unk8007F920 *this) {
    if (func_8007FC7C(this, 0)) {
        if (func_8007F1C0(func_8007758C(this->unk8))) {
            func_80081874((Unk80081874 *)this);
        } else {
            this->unk8->unk498 = 8;
            func_800808EC(this, 1);
        }
    } else {
        func_8007FCFC(this);
    }
}

/* Constructs an object of the class of vtable D_80067E44 with func_8007F920 and 16, calls
 * func_8007FF60 and clears 0x10. */
Unk80081874 *func_80081874(Unk80081874 *this) {
    func_8007F920(&this->base, 16);
    this->base.vtable = D_80067E44;
    func_8007FF60(&this->base);
    this->unk10 = 0;
    return this;
}

/* The tick of D_80067E44's class: once the 0x20 of the character's object at 0x238 is down to 0, becomes
 * a func_800816B4 object (0xE); else calls func_8007FF60, plays sound 0xC every 22 frames (count at 0x10)
 * and, once func_8007FC7C holds, calls func_8007F1B8 on that object and becomes a func_800808EC object
 * (1). */
void func_800818B8(Unk80081874 *this) {
    Unk8007F184 *object = func_8007758C(this->base.unk8);

    if (object->unk20 <= 0) {
        func_800816B4(&this->base, 0xE);
    } else {
        func_8007FF60(&this->base);
        if (this->unk10++ % 22 == 0) {
            soundControlPlaySound(&SOUND_CONTROL, 0xC, 0);
        }
        if (func_8007FC7C(&this->base, 0)) {
            func_8007F1B8(object);
            func_800808EC(&this->base, 1);
        }
    }
}

/* The method at 0x28 of D_80067E44: func_8007F1B8 on the character's object at 0x238, then
 * func_80081438. */
void func_80081998(Unk8007F920 *this, s32 arg) {
    func_8007F1B8(func_8007758C(this->unk8));
    func_80081438(this, arg);
}

/* Constructs an object of the class of vtable D_80067E0C with func_8007F920 and 1, and clears the
 * character's 0x448. */
Unk8007F920 *func_800819E0(Unk8007F920 *this) {
    func_8007F920(this, 1);
    this->vtable = D_80067E0C;
    this->unk8->unk448 = 0;
    return this;
}

/* Constructs an object of the class of vtable D_80067E0C, which derives from func_8007F920's. */
Unk8007F920 *func_80081A20(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_80067E0C;
    return this;
}

/* Clears the character's 0x448 to 0x460 (but 0x450), then unless the state of its 0x22C object (func_8007F93C)
 * is 0, 9 to 16 or 19, or 2 with arg not 0, runs the step arg (0 to 3) with its input (func_800776C4). */
void func_80081A54(Unk8007F920 *this, s32 arg) {
    Unk80076F34 *chara;
    Unk80075000 *input;

    this->unk8->unk448 = 0;
    this->unk8->unk44C = 0;
    /* The match depends on these four through a local: the game reloads 8 after each store above, not here. */
    chara = this->unk8;
    chara->unk454 = 0;
    chara->unk458 = 0;
    chara->unk45C = 0;
    chara->unk460 = 0;
    input = func_800776C4(this->unk8);
    switch (func_8007F93C(this)->unk0) {
    case 0:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 19:
        break;
    case 2:
        if (arg != 0) {
            break;
        }
        /* fallthrough */
    default:
        switch (arg) {
        case 0:
            func_80081B84(this, input);
            break;
        case 1:
            func_80081E18(this, input);
            break;
        case 2:
            func_80081EEC(this, input);
            break;
        case 3:
            func_80081FC4(this, input);
            break;
        }
        break;
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C44);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C4C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C54);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C5C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C64);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C6C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C74);

/* Step 0 of func_80081A54: by the character's clip (func_8007FB9C) and its input, picks the next state of
 * func_8008209C (0x17 to 0x1C) with its step function and calls func_800788B4, or func_8008230C. */
void func_80081B84(Unk8007F920 *this, Unk80075000 *input) {
    s32 clip = func_8007FB9C(this);
    Unk80076F34 *chara = this->unk8;

    switch (clip) {
    case 3:
    case 4:
        func_8007516C(input, 4);
        if (func_800751A8(input, 0x1000, 0)) {
            if (func_800778D0(this->unk8, 0)) {
                func_800788B4(chara, 7, 400);
                func_8008209C(this, 0x1B, D_80067C44);
            }
        } else if (func_800750C4(input) & 0xA000) {
            if (func_800778D0(this->unk8, 1)) {
                func_800788B4(chara, 9, 400);
                func_8008209C(this, 0x1C, D_80067C4C);
            }
        } else {
            func_800788B4(chara, 8, 400);
            func_8008209C(this, 0x1C, D_80067C54);
        }
        break;
    case 2:
        func_800788B4(chara, 6, 300);
        func_8008209C(this, 0x1A, D_80067C5C);
        break;
    case 19:
        func_800788B4(chara, 5, 100);
        func_8008209C(this, 0x19, D_80067C64);
        break;
    case 21:
        break;
    default:
        /* The match depends on a loop around the 0x2000 test that its break leaves: the game rolls that test to the
         * loop's end (entered with a jump), hoists &D_80067C6C out of it and moves the 0x17 block before it, which
         * a plain if/else never gives. A do/while (0) and a for (;;) ending in return both match. */
        do {
            if (func_80079CF4(this->unk8, 0)) {
                func_80078848(this->unk8, input);
                func_8007516C(input, 4);
                if (func_800751A8(input, 0x2000, 0)) {
                    func_800788B4(chara, 3, 400);
                    func_8008209C(this, 0x18, D_80067C6C);
                    break;
                }
            }
            func_8007516C(input, 4);
            if (func_800751A8(input, 0x1000, 0)) {
                func_800788B4(chara, 4, 200);
                func_8008209C(this, 0x17, D_80067C74);
            } else {
                func_8008230C(this);
            }
        } while (0);
        break;
    }
}

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x17, and if func_800778D0 does for 2, turns into state 0x1E stepping with
 * D_80067CCC; with any other value, unless func_800798F4 says so for 0x16,
 * into state 0x1D with D_80067CD4. Either way then func_800788B4 with 10, 100. */
void func_80081E18(Unk8007F920 *this, Unk80075000 *input) {
    s32 value = func_8007FB9C(this);
    Unk80076F34 *chara = this->unk8;

    switch (value) {
    case 3:
    case 4:
        if (!func_800798F4(chara, 0x17) && func_800778D0(chara, 2)) {
            func_8008209C(this, 0x1E, D_80067CCC);
            func_800788B4(chara, 0xA, 100);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x16)) {
            func_8008209C(this, 0x1D, D_80067CD4);
            func_800788B4(chara, 0xA, 100);
        }
        break;
    }
}

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x19, and if func_800778D0 does for 3, turns into state 0x20 stepping with
 * D_80067CDC; with any other value, unless func_800798F4 says so for 0x18,
 * into state 0x1F with D_80067CE4. Either way then func_800788B4 with 11, 1000. */
void func_80081EEC(Unk8007F920 *this, Unk80075000 *input) {
    s32 value = func_8007FB9C(this);
    Unk80076F34 *chara = this->unk8;

    switch (value) {
    case 3:
    case 4:
        /* the game reads 0x8 again here, where its siblings use chara */
        if (!func_800798F4(chara, 0x19) && func_800778D0(this->unk8, 3)) {
            func_8008209C(this, 0x20, D_80067CDC);
            func_800788B4(chara, 0xB, 1000);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x18)) {
            func_8008209C(this, 0x1F, D_80067CE4);
            func_800788B4(chara, 0xB, 1000);
        }
        break;
    }
}

/* With the character's clip (0x250) 3 or 4: unless func_800798F4 says so for
 * 0x1B, and if func_800778D0 does for 4, turns into state 0x22 stepping with
 * D_80067CEC; with any other value, unless func_800798F4 says so for 0x1A,
 * into state 0x21 with D_80067CF4. Either way then func_800788B4 with 12, 3000. */
void func_80081FC4(Unk8007F920 *this, Unk80075000 *input) {
    Unk80076F34 *chara = this->unk8;
    s32 value = func_8007FB9C(this);

    switch (value) {
    case 3:
    case 4:
        if (!func_800798F4(chara, 0x1B) && func_800778D0(chara, 4)) {
            func_8008209C(this, 0x22, D_80067CEC);
            func_800788B4(chara, 0xC, 3000);
        }
        break;
    default:
        if (!func_800798F4(chara, 0x1A)) {
            func_8008209C(this, 0x21, D_80067CF4);
            func_800788B4(chara, 0xC, 3000);
        }
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_8008209C);

INCLUDE_ASM("game/nonmatchings/game_0f", func_80082158);

/* The method at 0x28 of D_80067DD4: calls the character's virtual method at 0x20, then resets the object
 * (func_800819E0) and calls func_8007FC50 on it with -1 and 0. */
void func_80082270(Unk8007F920 *this) {
    Unk80076F34 *chara = this->unk8;
    Unk80076F34Vtable *vtable = UNK80076F34_VTABLE(chara);

    vtable->unk20.func((u8 *)chara + vtable->unk20.delta);
    func_800819E0(this);
    func_8007FC50(this, -1, 0);
}

/* func_80081A54 unless func_8007F98C holds. */
void func_800822CC(Unk8007F920 *this, s32 arg) {
    if (!func_8007F98C(this)) {
        func_80081A54(this, arg);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_8008230C);

INCLUDE_ASM("game/nonmatchings/game_0f", func_800823EC);

INCLUDE_ASM("game/nonmatchings/game_0f", func_800824B8);

/* The class of virtual table D_80067D94 (func_800823EC): func_8007F920's with a flag at 0x10 and the
 * step function at 0x14. */
typedef struct Unk800823EC {
    /* 0x00 */ Unk8007F920 base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ MemberFunction step;
} Unk800823EC;

/* The method at 0x20 of D_80067D54 and D_80067D94: unless func_8007F98C holds or arg is set, sets 0x10,
 * calls func_8007FC50 with -1 and 0, then its own method at 0x30 (func_8008261C in D_80067D94). */
void func_800825AC(Unk800823EC *this, s32 arg) {
    if (!func_8007F98C(&this->base) && arg == 0) {
        this->unk10 = 1;
        func_8007FC50(&this->base, -1, 0);
        this->base.vtable[6].func((u8 *)this + this->base.vtable[6].delta);
    }
}

/* Turns the object into the class of virtual table D_80067D54 (func_80082690 constructs it in place). */
Unk8007F920 *func_8008261C(Unk8007F920 *this) {
    return func_80082690(this);
}

/* Resets the object (func_800819E0), then calls func_8007FC50 on it with -1 and 0. */
void func_8008263C(void *this) {
    func_800819E0(this);
    func_8007FC50(this, -1, 0);
}

/* Turns the object into the class of virtual table D_80067D14 (func_800826E8 constructs it in place). */
Unk8007F920 *func_80082670(Unk8007F920 *this) {
    return func_800826E8(this);
}

/* Constructs the state 0x15 stepping with D_80067D04 (virtual table D_80067D54), then
 * func_800788B4 on the character with 1, 100. */
Unk8007F920 *func_80082690(Unk8007F920 *this) {
    func_800823EC(this, 0x15, D_80067D04);
    this->vtable = D_80067D54;
    func_800788B4(this->unk8, 1, 100);
    return this;
}

/* Constructs the state 0x16 stepping with D_80067D0C (virtual table D_80067D14), then
 * func_800788B4 on the character with 2, 200. */
Unk8007F920 *func_800826E8(Unk8007F920 *this) {
    func_800823EC(this, 0x16, D_80067D0C);
    this->vtable = D_80067D14;
    func_800788B4(this->unk8, 2, 200);
    return this;
}

void func_80082740(void) {
}

void func_80082748(void) {
}

void func_80082750(void) {
}

void func_80082758(void) {
}

void func_80082760(void) {
}

/* Returns its second argument (no caller is known: a virtual method, as its neighbours, is likely). */
s32 func_80082768(Unk8007F920 *this, s32 arg) {
    return arg;
}

/* Returns the word at 0 (no caller is known; between the Unk8007F920 methods, that is the state number). */
s32 func_80082770(Unk8007F920 *this) {
    return this->unk0;
}

/* Constructs an object of the class of vtable D_8006811C, which derives from func_8007F920's. */
Unk8007F920 *func_8008277C(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_8006811C;
    return this;
}

/* Constructs an object of the class of vtable D_800680E4, which derives from func_8007F920's. */
Unk8007F920 *func_800827B0(Unk8007F920 *this, s32 arg) {
    func_8007F920(this, arg);
    this->vtable = D_800680E4;
    return this;
}

void func_800827E4(void) {
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CCC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CD4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CDC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CE4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CEC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CF4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CFC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D04);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D0C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D14);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D54);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D94);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067DD4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067E0C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067E44);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067E7C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067EB4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067EEC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067F24);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067F5C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067F94);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067FCC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80068004);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_8006803C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80068074);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_800680AC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_800680E4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_8006811C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80068154);
