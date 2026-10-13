/* The class of the constructor func_8007EEB8 (virtual table D_800656BC). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"

INCLUDE_ASM("game/nonmatchings/game_0e", func_8007EEB8);

/* The object of func_8007EEB8 (virtual table D_800656BC, at 0x38 after its first fields as g++ places it): two
 * characters it owns, the first again at 8. */
typedef struct Unk8007EEB8 {
    /* 0x00 */ Unk80076F34 *chara;
    /* 0x04 */ Unk80076F34 *other; /* may be chara itself */
    /* 0x08 */ Unk80076F34 *unk8;
    /* 0x0C */ u8 unkC[0x38 - 0xC];
    /* 0x38 */ VtableEntry *vtable;
} Unk8007EEB8;

/* Takes its character out of its entity list, and the other one when it is not the same. */
void func_8007EF4C(Unk8007EEB8 *this) {
    listRemove(&this->chara->entity.link);
    if (this->chara != this->other) {
        listRemove(&this->other->entity.link);
    }
}

extern VtableEntry D_800656BC[];

/* The destructor of func_8007EEB8's object: deletes its character, and the other one when it is not the same. */
void func_8007EFC0(Unk8007EEB8 *this, s32 flags) {
    this->vtable = D_800656BC;
    entityDelete(&this->chara->entity);
    /* The match depends on this operand order (0 loads first). */
    if (this->chara != this->other) {
        entityDelete(&this->other->entity);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Unless 0x2C is set: when func_80077794 or func_800777BC holds for the character, adds half of amount to 0x24
 * (func_8007F1FC); takes amount off 0x20, down to 0. */
void func_8007F068(Unk8007F184 *this, s32 amount) {
    if (this->unk2C == 0) {
        if (func_80077794(this->base.unk8) || func_800777BC(this->base.unk8)) {
            func_8007F1FC(this, amount / 2);
        }
        /* The match depends on amount taking the result (a new variable swaps the saved registers). */
        amount = this->unk20 - amount;
        if (amount < 0) {
            amount = 0;
        }
        this->unk20 = amount;
    }
}

/* Adds amount to 0x20, kept between 0 and 0x1C. */
void func_8007F0F4(Unk8007F184 *this, s32 amount) {
    amount += this->unk20;
    if (amount > this->unk1C) {
        amount = this->unk1C;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->unk20 = amount;
}

/* Unless 0x2C is set, adds amount << 12 to 0x28; when that wraps around, 0x28 takes the table's unk14
 * instead (the value func_8007F1C0 compares it with). */
void func_8007F124(Unk8007F184 *this, s32 amount) {
    s32 value;

    if (this->unk2C == 0) {
        value = this->unk28 + (amount << 12);
        if (value < this->unk28) {
            value = func_80077698(this->base.unk8)->unk14;
        }
        this->unk28 = value;
    }
}

/* Copies field 0x14 of what func_80077698 returns for the character to 0x28. */
void func_8007F184(Unk8007F184 *this) {
    this->unk28 = func_80077698(this->base.unk8)->unk14;
}

/* Clears 0x28. */
void func_8007F1B8(Unk8007F184 *this) {
    this->unk28 = 0;
}

/* Whether 0x28 has reached the table's unk14. */
s32 func_8007F1C0(Unk8007F184 *this) {
    return this->unk28 >= func_80077698(this->base.unk8)->unk14;
}

/* Adds amount to 0x24, kept between 0 and 0x10000. */
void func_8007F1FC(Unk8007F184 *this, s32 amount) {
    amount += this->unk24;
    if (amount > 0x10000) {
        amount = 0x10000;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->unk24 = amount;
}

void func_8007F7F4(Unk8007F184 *this);

/* Unless 0x2C is set: until func_8007F1C0 holds, takes 0x44 off 0x28 (down to 0); when bit 0 of the table's
 * first word is set and func_800777BC does not hold, takes 0x49 off 0x24 (down to 0), and once 0x24 is empty
 * calls func_8007F7F4 when func_8007AED0 holds for the character. */
void func_8007F22C(Unk8007F184 *this) {
    Unk80076F34 *chara;
    u16 bits;

    if (this->unk2C == 0) {
        chara = this->base.unk8;
        bits = func_80077698(chara)->unk0;
        if (!func_8007F1C0(this)) {
            this->unk28 -= 0x44;
            if (this->unk28 < 0) {
                this->unk28 = 0;
            }
        }
        if ((bits & 1) && !func_800777BC(chara)) {
            this->unk24 -= 0x49;
            if (this->unk24 < 0) {
                this->unk24 = 0;
            }
            if (this->unk24 <= 0 && func_8007AED0(chara)) {
                func_8007F7F4(this);
            }
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0e", func_8007F300);

/* Sets 0x2C: the amounts stop changing. */
void func_8007F59C(Unk8007F184 *this) {
    this->unk2C = 1;
}

/* Whether 0x24 is above 0xFFFF, unless bit 0 of the first word of the character's table (by func_80077568) is
 * set, func_8007AED0 does not hold or func_800792B8 gives 11 to 16 or 19. */
s32 func_8007F5A8(Unk8007F184 *this) {
    Unk80077698 *tables = D_800656D4;
    s32 state;

    if (tables[func_80077568(this->base.unk8)].unk0 & 1) {
        return 0;
    }
    if (!func_8007AED0(this->base.unk8)) {
        return 0;
    }
    state = func_800792B8(this->base.unk8);
    /* The match depends on one if per state: an || chain is folded into a range test (11 to 16). */
    if (state == 11) {
        return 0;
    }
    if (state == 12) {
        return 0;
    }
    if (state == 13) {
        return 0;
    }
    if (state == 14) {
        return 0;
    }
    if (state == 15) {
        return 0;
    }
    if (state == 16) {
        return 0;
    }
    if (state == 19) {
        return 0;
    }
    return this->unk24 > 0xFFFF;
}

/* When func_800777BC holds for the character, whether 0x24 is above 0xFFFF; else whether it is not 0, when bit 0
 * of the first word of the character's table (by func_80077568) is set. */
s32 func_8007F674(Unk8007F184 *this) {
    Unk80077698 *tables = D_800656D4;
    u16 bits = tables[func_80077568(this->base.unk8)].unk0;
    s32 result;

    if (func_800777BC(this->base.unk8)) {
        return this->unk24 > 0xFFFF;
    }
    /* The match depends on result starting at 0 (an if/else returns straight from $v0). */
    result = 0;
    if (bits & 1) {
        result = this->unk24 != 0;
    }
    return result;
}

/* Gives the character a new handle (entityRenewHandle) and sets 0x30. */
void func_8007F6FC(Unk8007F6FC *this) {
    entityRenewHandle(&this->base.unk8->entity);
    this->unk30 = 1;
}

void func_8007F7A0(Unk8007F184 *this);

/* When func_800777BC holds for the character and bit 1 of its table's first word is clear: func_8007F7A0, then
 * 30 at 0x10 and 3 at 0x18. */
void func_8007F730(Unk8007F184 *this) {
    Unk80076F34 *chara = this->base.unk8;

    if (func_800777BC(chara) && !(func_80077698(chara)->unk0 & 2)) {
        func_8007F7A0(this);
        this->unk10 = 30;
        this->unk18 = 3;
    }
}

/* Plays sound 0x800E and adds func_800C49E4's entity (0x48 bytes) for this one as a sibling of the character. */
void func_8007F7A0(Unk8007F184 *this) {
    soundControlPlaySound(&SOUND_CONTROL, 0x800E, 0);
    entityAddSibling(&this->base.unk8->entity, &func_800C49E4(entityOperatorNew(0x48), &this->base)->entity);
}

/* Unless 0x14 is above 0: sets it to 16, plays sound 0x800F, adds func_800C4F74's entity (0x4A8 bytes) for this
 * one as a sibling of the character, renews the character's handle, calls func_80079B0C, func_80079B14 and
 * func_80079B1C with 30 on it, and clears 0x28 (func_8007F1B8). */
void func_8007F7F4(Unk8007F184 *this) {
    Unk80076F34 *chara;

    if (this->unk14 <= 0) {
        chara = this->base.unk8;
        this->unk14 = 16;
        soundControlPlaySound(&SOUND_CONTROL, 0x800F, 0);
        entityAddSibling(&chara->entity, &func_800C4F74(entityOperatorNew(0x4A8), &this->base)->entity);
        entityRenewHandle(&chara->entity);
        func_80079B0C(chara, 30);
        func_80079B14(chara, 30);
        func_80079B1C(chara, 30);
        func_8007F1B8(this);
    }
}

/* Switches the character at 8 to the other of the two (to the first when it was the second), calls
 * func_80079534 on the new one with the old one, and renews the old one's handle. */
void func_8007F89C(Unk8007EEB8 *this) {
    Unk80076F34 *previous = this->unk8;
    Unk80076F34 *next = this->other;

    if (previous == next) {
        next = this->chara;
    }
    this->unk8 = next;
    func_80079534(next, previous);
    entityRenewHandle(&previous->entity);
}

/* Returns the character at 8; no caller found. */
Unk80076F34 *func_8007F8E4(Unk8007F184 *this) {
    return this->base.unk8;
}

/* Returns 0x20; no caller found. */
s32 func_8007F8F0(Unk8007F184 *this) {
    return this->unk20;
}

/* Returns the most 0x20 can hold (0x1C); no caller found. */
s32 func_8007F8FC(Unk8007F184 *this) {
    return this->unk1C;
}

/* Returns 0x24; no caller found. */
s32 func_8007F908(Unk8007F184 *this) {
    return this->unk24;
}

/* Returns 0x28; no caller found. */
s32 func_8007F914(Unk8007F184 *this) {
    return this->unk28;
}

INCLUDE_RODATA("game/nonmatchings/game_0e", D_800656BC);
