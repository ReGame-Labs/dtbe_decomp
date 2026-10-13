#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "psyq.h"
#include "engine/math/random.h"
#include "engine/system/memory.h"

/* The stage's object of func_800D3BD8, derived from func_8007C084's class. */
typedef struct Unk800D390C {
    /* 0x00 */ Unk8007C084 base;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ s32 unk10;
} Unk800D390C;

/* The stage's class, derived from func_800D02C8's (virtual table D_800D3894). */
typedef struct Unk800D3AA8 {
    /* 0x000 */ Unk800D02C8 base;
    /* 0xDA8 */ u8 unkDA8[0xF30 - 0xDA8];
    /* 0xF30 */ s16 unkF30[2]; /* by side: 0xFE0's values, copied each update (func_800D47CC) */
    /* 0xF34 */ s16 unkF34; /* 15 here; 30 in st07's and st09's */
    /* 0xF36 */ s16 unkF36;
    /* 0xF38 */ u8 unkF38[0xF68 - 0xF38];
    /* 0xF68 */ s16 unkF68;
    /* 0xF6A */ u16 unkF6A;
    /* 0xF6C */ u8 unkF6C[0xF70 - 0xF6C];
    /* 0xF70 */ RECT unkF70;     /* 16 by 1 pixels at (0x300, 0xFC): a CLUT */
    /* 0xF78 */ u16 unkF78[16]; /* that CLUT read back from VRAM (func_800D3FB4) */
    /* 0xF98 */ u16 unkF98[16]; /* and a second copy */
    /* 0xFB8 */ s16 unkFB8;
    /* 0xFBA */ s16 unkFBA;
    /* 0xFBC */ s16 unkFBC;
    /* 0xFBE */ s16 unkFBE;
    /* 0xFC0 */ u8 unkFC0[0xFC8 - 0xFC0];
    /* 0xFC8 */ s16 unkFC8;
    /* 0xFCA */ s16 unkFCA;
    /* 0xFCC */ s16 unkFCC;
    /* 0xFCE */ u8 unkFCE[0xFD0 - 0xFCE];
    /* 0xFD0 */ s32 unkFD0;
    /* 0xFD4 */ s32 unkFD4;
    /* 0xFD8 */ MeshScene *unkFD8; /* what func_8006F1B8 returns, at the init */
    /* 0xFDC */ s16 unkFDC;
    /* 0xFDE */ s16 unkFDE;
    /* 0xFE0 */ s16 unkFE0[2]; /* by side: 120 at the init (func_800D47A4) */
} Unk800D3AA8;

extern VtableEntry D_800D3874[];
extern VtableEntry D_800D3894[];

Unk800D390C *func_800D390C(Unk800D390C *this, s32 arg);

/* Constructs the stage's object of func_800D3BD8 (virtual table D_800D3874). */
Unk800D390C *func_800D390C(Unk800D390C *this, s32 arg) {
    func_8007C084(&this->base, arg);
    this->base.vtable = D_800D3874;
    this->unk8 = 0;
    this->unkC = 0;
    this->unk10 = 0;
    return this;
}

/* Selects 0..2 in func_800D394C (the period ranges 1-10, 2-7 and 3-6) */
extern s32 D_8005FB84;

/* Unk800D390C's tick: returns 0x80 once every unk10 calls (counted at unkC), and draws a new random
 * period at the start of each one, from the range D_8005FB84 selects; returns 0 otherwise.
 * The match depends on the roll taking the random value before its own "%=" (an initializer with the
 * "%" keeps the remainder out of $v0), and on the count read into a local before the increment. */
s32 func_800D394C(Unk800D390C *this) {
    s32 count;

    if (this->unk8 == 0) {
        switch (D_8005FB84) {
        case 0: {
            s32 roll = mersenneTwisterGenerate(&RANDOM);

            roll %= 10;
            this->unk10 = (roll < 0 ? -roll : roll) + 1;
            break;
        }
        case 1: {
            s32 roll = mersenneTwisterGenerate(&RANDOM);

            roll %= 6;
            this->unk10 = (roll < 0 ? -roll : roll) + 2;
            break;
        }
        case 2:
            this->unk10 = (mersenneTwisterGenerate(&RANDOM) & 3) + 3;
            break;
        }
        this->unkC = 0;
        this->unk8++;
    }
    count = this->unkC;
    this->unkC = count + 1;
    if ((count + 2) % this->unk10 != 0) {
        return 0;
    }
    this->unk8 = 0;
    return 0x80;
}

/* The eight words at 0xD38 of func_800D02C8's class, which the stages' constructors fill (game.h's
 * Unk800D02C8 leaves 0xD38-0xDA4 as a gap). */
#define BASE_TABLE(stage) ((s32 *)(stage)->base.unkD38)

/* Constructs the stage's object: func_800D02C8's, with virtual table D_800D3894 and the eight words at 0xD38, and clears 0xFD0 and 0xFD4. */
Unk800D3AA8 *func_800D3AA8(Unk800D3AA8 *this) {
    func_800D02C8(&this->base);
    this->base.base.vtable = D_800D3894;
    BASE_TABLE(this)[0] = 9;
    BASE_TABLE(this)[1] = 10;
    BASE_TABLE(this)[2] = 14;
    BASE_TABLE(this)[3] = 11;
    BASE_TABLE(this)[4] = 12;
    BASE_TABLE(this)[5] = 8;
    BASE_TABLE(this)[6] = 15;
    BASE_TABLE(this)[7] = 16;
    this->unkFD0 = this->unkFD4 = 0;
    return this;
}

/* Destroys the stage's object: virtual table D_800D3894, then func_800D02C8's destructor. */
void func_800D3B24(Unk800D3AA8 *this, s32 flags) {
    this->base.base.vtable = D_800D3894;
    func_800D02FC(&this->base, flags);
}

/* what the stage's own init calls */
void func_800D3C0C(Unk800D3AA8 *this);
void func_800D3FB4(Unk800D3AA8 *this);
void func_800D40BC(Unk800D3AA8 *this);
void func_800D4390(Unk800D3AA8 *this);
void func_800D47A4(Unk800D3AA8 *this);

/* The stage's init: func_800D02C8's, then its own state and objects; 0xF68 ends at 120 and 0xFBA and
 * 0xFBC at -1 (st07's func_800D3BB0). */
void func_800D3B4C(Unk800D3AA8 *this) {
    func_800D0364(&this->base);
    func_800D3C0C(this);
    func_800700E4(&this->base.base, -0x500, 0x500, -0xC80, 0x400, 0);
    this->unkFD8 = func_8006F1B8(&this->base.base);
    func_800D40BC(this);
    func_800D4390(this);
    func_800D47A4(this);
    func_800D3FB4(this);
    this->unkF68 = 120;
    this->unkFBA = -1;
    this->unkFBC = -1;
}

/* The stage's override of func_800702E0: creates its own 0x14-byte object for arg. */
Unk800D390C *func_800D3BD8(void *this, s32 unk, s32 arg) {
    return func_800D390C(operatorNew(0x14), arg);
}

/* Clears 0xFB8, 0xF6A and 0xFBE of the stage (func_800D3B4C calls it after func_800D0364). */
void func_800D3C0C(Unk800D3AA8 *this) {
    this->unkFB8 = 0;
    this->unkF6A = 0;
    this->unkFBE = 0;
}

INCLUDE_ASM("st08/nonmatchings/st08", func_800D3C1C);

/* the stage's own steps */
void func_800D3DF8(Unk800D3AA8 *this);
void func_800D4018(Unk800D3AA8 *this);
void func_800D40DC(Unk800D3AA8 *this);
void func_800D4590(Unk800D3AA8 *this);
void func_800D47CC(Unk800D3AA8 *this);

/* The stage's update: func_800D02C8's, then its own five steps, and counts the update at 0xFB8. */
void func_800D3DA0(Unk800D3AA8 *this) {
    func_800D06D0(&this->base);
    func_800D40DC(this);
    func_800D4590(this);
    func_800D4018(this);
    func_800D47CC(this);
    func_800D3DF8(this);
    this->unkFB8++;
}

/* The stage's last update step, until 0xF6A is set: a side whose 0xFE0 reached 0 gets func_80079350
 * (when bit 0 of its record is clear, counted at 0xFBE) and func_80077448(0x10000), and 0xFBC names it;
 * with both, 0xFBC and 0xFBA go to -1. 0xD28 == 1 calls func_800D0B80 and sets 0xFBA to 170.
 * The match depends on the bit tests written "(x ^ 1) & 1" (xori, andi; "!(x & 1)" branches on andi). */
void func_800D3DF8(Unk800D3AA8 *this) {
    s16 done = 0;
    u16 first;
    u16 second;

    if (this->unkF6A != 0) {
        return;
    }
    first = D_800656D4[func_80077568(func_800700C8(&this->base.base, 0))].unk0;
    second = D_800656D4[func_80077568(func_800700C8(&this->base.base, 1))].unk0;
    if (this->unkFE0[0] == 0) {
        this->unkFBC = 0;
        if ((first ^ 1) & 1) {
            func_80079350(func_800700C8(&this->base.base, 0));
            this->unkFBE++;
        }
        func_80077448(func_800700C8(&this->base.base, 0), 0x10000);
        this->unkF6A = 1;
        done = 1;
    }
    if (this->unkFE0[1] == 0) {
        this->unkFBC = 1;
        if ((second ^ 1) & 1) {
            func_80079350(func_800700C8(&this->base.base, 1));
            this->unkFBE++;
        }
        func_80077448(func_800700C8(&this->base.base, 1), 0x10000);
        this->unkF6A = 1;
        done++;
    }
    if (done == 2) {
        this->unkF6A = 1;
        this->unkFBC = -1;
        this->unkFBA = -1;
    }
    if (this->base.base.unkD28 == 1) {
        func_800D0B80(&this->base);
        this->unkF6A = 1;
        this->unkFBC = -1;
        this->unkFBA = 170;
    }
}

/* Reads the 16-colour CLUT at (0x300, 0xFC) back from VRAM twice, into 0xF78 and 0xF98. */
void func_800D3FB4(Unk800D3AA8 *this) {
    this->unkF70.x = 0x300;
    this->unkF70.y = 0xFC;
    this->unkF70.w = 16;
    this->unkF70.h = 1;
    StoreImage(&this->unkF70, (u_long *)this->unkF78);
    StoreImage(&this->unkF70, (u_long *)this->unkF98);
}

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4018);

/* Clears the halfwords at 0xFDC, 0xFDE and 0xFC8-0xFCC and sets 0xF34 to 15 (st07's func_800D3E94). */
void func_800D40BC(Unk800D3AA8 *this) {
    this->unkFDC = 0;
    this->unkFDE = 0;
    this->unkFC8 = 0;
    this->unkFCC = 0;
    this->unkFCA = 0;
    this->unkF34 = 15;
}

INCLUDE_ASM("st08/nonmatchings/st08", func_800D40DC);

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4390);

INCLUDE_ASM("st08/nonmatchings/st08", func_800D4590);

/* Sets both sides' 0xFE0 and 0xF36 to 120 (the stage's init calls it).
 * The match depends on the sides' value in a local set before the loop (its 120 is loaded first). */
void func_800D47A4(Unk800D3AA8 *this) {
    s16 time = 120;
    s32 side;

    for (side = 1; side >= 0; side--) {
        this->unkFE0[side] = time;
    }
    this->unkF36 = 120;
}

/* Copies both sides' 0xFE0 into 0xF30 (the last of the stage's update steps before func_800D3DF8).
 * The match depends on the value going through an int (lh, not lhu); gcc turns the loop into a countdown. */
void func_800D47CC(Unk800D3AA8 *this) {
    s32 side;
    s32 value;

    for (side = 0; side < 2; side++) {
        value = this->unkFE0[side];
        this->unkF30[side] = value;
    }
}

/* Unk800D390C's destructor: Unk8007C084's. */
void func_800D47F0(Unk800D390C *this, s32 flags) {
    func_8007C09C(&this->base, flags);
}
