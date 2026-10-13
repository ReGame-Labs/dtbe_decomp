/* A side's HUD entity: drawn on the left and mirrored on the right, with two gauges and a counter. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/text/console.h"

/* two of these are at 0x18 and 0x28 of the entity func_800733C4 constructs,
 * each set up by func_800735E8 with 0x10000 at 0xC */
typedef struct Unk800735E8 {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} Unk800735E8;

INCLUDE_ASM("game/nonmatchings/side_hud", func_800733C4);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800734AC);

/* Sets 0 to unk0 and 0xC to unkC, clearing 4 and 8. */
void func_800735E8(Unk800735E8 *this, s32 unk0, s32 unkC) {
    this->unk0 = unk0;
    this->unk4 = 0;
    this->unk8 = 0;
    this->unkC = unkC;
}

/* Sets unk4 to length scaled by unk0 (0x10000 for all of it), and moves unk8 a step towards it; while unk0 is
 * positive, both are 2 at least. */
void func_800735FC(Unk800735E8 *this, s32 length) {
    this->unk4 = length * this->unk0 / 0x10000;
    if (this->unk8 != this->unk4) {
        s32 shown;

        if (this->unk8 < this->unk4) {
            shown = this->unk8 + 1;
        } else {
            shown = this->unk8 - 1;
        }
        this->unk8 = shown;
    }
    if (this->unk0 > 0) {
        if (this->unk4 < 2) {
            this->unk4 = 2;
        }
        if (this->unk8 < 2) {
            this->unk8 = 2;
        }
    }
}

/* The counter at 0x64 of the HUD entity (func_80073684 clears it, func_80073708 draws it). */
typedef struct Unk80073684 {
    /* 0x0 */ s32 value;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
} Unk80073684;

/* The HUD entity of a side that func_800733C4 constructs (virtual table D_800650CC). */
typedef struct Unk800733C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800735E8 unk18;
    /* 0x28 */ Unk800735E8 unk28;
    /* 0x38 */ void *unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 side; /* 0: func_80073AC0 draws it, else func_80073BB4 (func_80073A44) */
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ s32 unk5C;
    /* 0x60 */ s32 unk60;
    /* 0x64 */ Unk80073684 counter;
    /* 0x70 */ s32 visible; /* func_80073A44 draws nothing while 0 */
    /* 0x74 */ s32 unk74;
} Unk800733C4;

/* Clears the counter. */
void func_80073684(Unk80073684 *counter) {
    counter->value = 0;
    counter->unk4 = 0;
    counter->unk8 = 0;
}

/* Runs the counter's timer at 8 down to 0: by an eighth above 0x100, by 4 below. The match depends on the step
 * stored through a pointer to the timer (the game computes its address before the test) and kept in a variable of
 * its own. */
void func_80073694(Unk80073684 *counter) {
    s32 *timer = &counter->unk8;
    s32 value = *timer;
    s32 next;

    if (value > 0x100) {
        next = value - (value >> 3);
    } else {
        next = value - 4;
    }
    *timer = next;
    if (counter->unk8 <= 0) {
        counter->unk8 = 0;
    }
}

/* Sets the counter's unk4 to count; a count of 2 or more above the last one also shows it (value) and restarts the
 * timer at 0x200. */
void func_800736D8(Unk80073684 *counter, s32 count) {
    if (count >= 2 && counter->unk4 < count) {
        counter->unk8 = 0x200;
        counter->value = count;
    }
    counter->unk4 = count;
}

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073708);

void func_80073AC0(Unk800733C4 *this, u_long *ot);
void func_80073BB4(Unk800733C4 *this, u_long *ot);
void func_80074A88(Unk800733C4 *this, Camera *camera);

/* Draws the HUD (method 0x10 of virtual table D_800650CC) of its side, after the last slot of the camera's
 * ordering table, then func_80074A88 while it is still visible. */
void func_80073A44(Unk800733C4 *this, Camera *camera) {
    u_long *ot;

    if (this->visible) {
        ot = camera->ot.lastDrawn + 1;
        if (this->side == 0) {
            func_80073AC0(this, ot);
        } else {
            func_80073BB4(this, ot);
        }
        if (this->visible) {
            func_80074A88(this, camera);
        }
    }
}

/* the parts of the HUD, each drawn at (x, y) */
void func_80073C80(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073ED4(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073FB8(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_800740F0(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80074250(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80074528(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_800749BC(Unk800733C4 *this, u_long *ot, s32 x, s32 y);
void func_80073708(Unk80073684 *counter, u_long *ot, s32 x, s32 y);

/* Draws the HUD of the left side: its parts, then the counter at x 0xC8 (0xBC from 10 up), y 0x30. */
void func_80073AC0(Unk800733C4 *this, u_long *ot) {
    Unk80073684 *counter;
    s32 x;

    func_80073FB8(this, ot, 0x94, 0xCD);
    func_80074250(this, ot, 0xF, 0xD8);
    func_80074528(this, ot, 0x41, 0xCC);
    func_80073ED4(this, ot, 0x76, 0xBE);
    func_800740F0(this, ot, 0x43, 0xBE);
    func_800749BC(this, ot, 4, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    /* The match depends on the counter pointer: its address is computed before the test, as in the game. */
    counter = &this->counter;
    if (counter->value < 10) {
        x = 0xC8;
    } else {
        x = 0xBC;
    }
    func_80073708(counter, ot + 2, x, 0x30);
    func_80073C80(this, ot, 0xF, 0xBD);
}

/* Draws the HUD of the right side: func_80073AC0's parts but the first, mirrored, and the counter at x 0x1C. */
void func_80073BB4(Unk800733C4 *this, u_long *ot) {
    func_80074250(this, ot, 0xB9, 0xD8);
    func_80074528(this, ot, 0xB9, 0xCC);
    func_80073ED4(this, ot, 0xBF, 0xBE);
    func_800740F0(this, ot, 0xCD, 0xBE);
    func_800749BC(this, ot, 0xA0, 0xB8);
    AddPrim(ot, allocDrTpageMode(0, 0, 0xF));
    func_80073708(&this->counter, ot + 2, 0x1C, 0x30);
    func_80073C80(this, ot, 0x102, 0xBD);
}

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073C80);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073D78);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073ED4);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80073FB8);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065044);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800740F0);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065060);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065064);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80074250);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065074);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065078);
INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065088);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80074528);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800747F8);

INCLUDE_ASM("game/nonmatchings/side_hud", func_800749BC);

INCLUDE_RODATA("game/nonmatchings/side_hud", D_80065098);

INCLUDE_ASM("game/nonmatchings/side_hud", func_80074A88);

/* The destructor of the entity class of vtable D_800650CC, which func_800733C4 installs. */
void func_80074FAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/side_hud", D_800650CC);
