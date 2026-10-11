#include "common.h"
#include "battle/game.h"
#include "psyq.h"
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
    /* 0xDA8 */ u8 unkDA8[0xF34 - 0xDA8];
    /* 0xF34 */ s16 unkF34; /* 15 here; 30 in st07's and st09's */
    /* 0xF36 */ u8 unkF36[0xF6A - 0xF36];
    /* 0xF6A */ s16 unkF6A;
    /* 0xF6C */ u8 unkF6C[0xF70 - 0xF6C];
    /* 0xF70 */ RECT unkF70;     /* 16 by 1 pixels at (0x300, 0xFC): a CLUT */
    /* 0xF78 */ u16 unkF78[16]; /* that CLUT read back from VRAM (func_800D3FB4) */
    /* 0xF98 */ u16 unkF98[16]; /* and a second copy */
    /* 0xFB8 */ s16 unkFB8;
    /* 0xFBA */ u8 unkFBA[0xFBE - 0xFBA];
    /* 0xFBE */ s16 unkFBE;
    /* 0xFC0 */ u8 unkFC0[0xFC8 - 0xFC0];
    /* 0xFC8 */ s16 unkFC8;
    /* 0xFCA */ s16 unkFCA;
    /* 0xFCC */ s16 unkFCC;
    /* 0xFCE */ u8 unkFCE[0xFD0 - 0xFCE];
    /* 0xFD0 */ s32 unkFD0;
    /* 0xFD4 */ s32 unkFD4;
    /* 0xFD8 */ u8 unkFD8[0xFDC - 0xFD8];
    /* 0xFDC */ s16 unkFDC;
    /* 0xFDE */ s16 unkFDE;
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

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D394C);

Unk800D02C8 *func_800D02C8(Unk800D02C8 *this);

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

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D3B4C);

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

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D3C1C);

/* func_800D02C8's class's update, and the stage's own steps */
void func_800D06D0(Unk800D02C8 *this);
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

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D3DF8);

/* Reads the 16-colour CLUT at (0x300, 0xFC) back from VRAM twice, into 0xF78 and 0xF98. */
void func_800D3FB4(Unk800D3AA8 *this) {
    this->unkF70.x = 0x300;
    this->unkF70.y = 0xFC;
    this->unkF70.w = 16;
    this->unkF70.h = 1;
    func_80055B88(&this->unkF70, (u_long *)this->unkF78);
    func_80055B88(&this->unkF70, (u_long *)this->unkF98);
}

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D4018);

/* Clears the halfwords at 0xFDC, 0xFDE and 0xFC8-0xFCC and sets 0xF34 to 15 (st07's func_800D3E94). */
void func_800D40BC(Unk800D3AA8 *this) {
    this->unkFDC = 0;
    this->unkFDE = 0;
    this->unkFC8 = 0;
    this->unkFCC = 0;
    this->unkFCA = 0;
    this->unkF34 = 15;
}

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D40DC);

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D4390);

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D4590);

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D47A4);

INCLUDE_ASM("asm/jp/st08/nonmatchings/st08", func_800D47CC);

/* Unk800D390C's destructor: Unk8007C084's. */
void func_800D47F0(Unk800D390C *this, s32 flags) {
    func_8007C09C(&this->base, flags);
}
