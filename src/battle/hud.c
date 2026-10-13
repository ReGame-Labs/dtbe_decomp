/* The battle's messages: READY?, FIGHT!, CONTINUE?, YOU WIN!, TIME UP and the rest. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/gfx/box.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/text/console.h"

extern EntityVtable D_8006DD6C;
/* adds the three words at 0x28 to 0x34 and clears them (the D_8006DD6C entity's) */
struct Unk800CDA20;
void func_800CDFBC(struct Unk800CDA20 *this);

INCLUDE_ASM("game/nonmatchings/hud", func_800CB7B0);

INCLUDE_ASM("game/nonmatchings/hud", func_800CB988);

INCLUDE_ASM("game/nonmatchings/hud", func_800CBA04);

/* The start of the entity class func_800CB7B0 constructs (0x59C bytes): two frame counters it zeroes, and
 * at 0x58C and 0x594 the pointers to member functions of its states (tables D_800D36A4 and D_800D370C). */
typedef struct Unk800CB7B0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s16 unk18; /* from D_8006DEA4 with text: the step of a character (unk20's w and h) */
    /* 0x1A */ s16 unk1A; /* from D_8006DEA4 with text: the width of the text */
    /* 0x1C */ u8 *text;  /* func_800CD140 copies it into unk40 */
    /* 0x20 */ RECT unk20; /* func_800CF528's metrics: x of a new line, w (twice a space), h (a line) */
    /* 0x28 */ RECT unk28; /* where func_800CBAE0 starts drawing the text */
    /* 0x30 */ s32 unk30; /* func_800CBAE0: how many characters it draws */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ Unk800CD140 unk40;
} Unk800CB7B0;

/* A state of D_800D36A4 (entries 1 and 10): counts both frame counters, and deletes the entity after 91
 * frames. */
void func_800CBA80(Unk800CB7B0 *this) {
    this->unk30++;
    this->unk3C++;
    if (this->unk3C >= 91) {
        entityDelete(&this->entity);
    }
}

void func_800CBAE0(Unk800CB7B0 *this, u_long *ot);
void func_800CF2A4(u_long *ot, s32 brightness);

void func_800CF528(u_long *ot, RECT *cursor, RECT *metrics, u8 c, s32 arg4);
void func_800D008C(u_long *ot);

/* A member function (its pointer is in D_800D370C): draws up to 0x30 characters of the text (func_800CF528) from
 * the point at 0x28, then func_800D008C. */
void func_800CBAE0(Unk800CB7B0 *this, u_long *ot) {
    s32 count = this->unk30;
    u8 *c = this->text;
    RECT cursor = this->unk28;

    while (count > 0 && *c != 0) {
        func_800CF528(ot, &cursor, &this->unk20, *c, this->unk34);
        c++;
        count--;
    }
    func_800D008C(ot);
}

void func_800CD56C(Unk800CD140 *this, u_long *ot, s32 x, s32 y);

/* A member function (its pointer is in D_800D370C): draws the text at 0x40 (func_800CD56C) centred on the screen's
 * width, at y 0x50. */
void func_800CBB88(Unk800CB7B0 *this, u_long *ot) {
    s32 width = (this->unk40.length - 1) * this->unk40.unk544;

    func_800CD56C(&this->unk40, ot, (320 - width) / 2, 0x50);
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CBBD4);

/* A member function (its pointer is in D_800D370C): func_800CBAE0, then func_800CF2A4 in the next slot
 * of ot with 90 minus 0x3C. */
void func_800CBE4C(Unk800CB7B0 *this, u_long *ot) {
    func_800CBAE0(this, ot);
    func_800CF2A4(ot + 1, 90 - this->unk3C);
}

#if VERSION_JP
INCLUDE_ASM("game/nonmatchings/hud", func_800CBE90);
#else
/* A state of D_800D36A4: in state 0 counts 0x30 up on even frames to the length less one, then goes to state 1,
 * where 0x30 blinks between the length and one less; deletes the entity after 46 frames. The length is the
 * text's width over the step of a character (JP: 10, and that body is still asm: its registers differ). */
void func_800CBE90(Unk800CB7B0 *this) {
    s32 length = this->unk1A / this->unk18;

    switch (this->unk38) {
    case 0:
        length--;
        /* The match depends on the store-flag test (xori + andi); !(unk3C & 1) gives andi + bnez. */
        if ((this->unk3C ^ 1) & 1) {
            if (++this->unk30 >= length) {
                this->unk38 = 1;
            }
        }
        break;
    case 1:
        if (this->unk3C & 2) {
            length--;
        }
        this->unk30 = length;
        break;
    }
    if (++this->unk3C >= 46) {
        entityDelete(&this->entity);
    }
}
#endif

INCLUDE_ASM("game/nonmatchings/hud", func_800CBF6C);

void func_800CD140(Unk800CD140 *this, u8 *text);
void func_800CD248(Unk800CD140 *this, s32 arg1, s32 arg2, s32 start, s32 count);
void func_800CD3A8(Unk800CD140 *this, s32 value);
void func_800CD474(Unk800CD140 *this, s32 value);

/* A state of D_800D36A4: in state 0 copies the text into 0x40 and moves its first two characters apart; in 1
 * brings them in by quarters while the color goes up to 0xFF, in 2 fades it from 0xFF down to 0x80, waits 17
 * frames in 3, and in 4 darkens it while the two characters drift apart again, deleting the entity after 17
 * frames. The match depends on this being a copy of the parameter (it then takes s1 and the text s2) and
 * on state 1's loop having a counter of its own (seed: Ashar2shahid's US func_800CC2C4). */
void func_800CBFF0(Unk800CB7B0 *arg) {
    Unk800CB7B0 *this = arg;
    Unk800CD140 *text = &this->unk40;
    s32 i;
    s32 way;

    switch (this->unk38) {
    case 0:
        func_800CD140(text, this->text);
        func_800CD248(text, 0x58, -0xB0, 0, 1);
        func_800CD248(text, -0x58, 0xB0, 1, 1);
        func_800CD3A8(text, 0x30);
        this->unk38++;
        break;
    case 1: {
        s32 n;

        for (n = 0; n < 2; n++) {
            text->chars[n].unk0 >>= 2;
            text->chars[n].unk2 >>= 2;
        }
        func_800CD474(text, (this->unk3C << 5) + 0x7F);
        if (this->unk3C >= 4) {
            this->unk3C = 0;
            this->unk38++;
        }
        break;
    }
    case 2:
        func_800CD474(text, 0xFF - (this->unk3C << 4));
        if (this->unk3C >= 8) {
            func_800CD474(text, 0x80);
            this->unk38++;
        }
        break;
    case 3:
        if (this->unk3C >= 17) {
            this->unk3C = 0;
            this->unk38++;
        }
        break;
    case 4:
        func_800CD474(text, 0x80 - (this->unk3C << 3));
        for (i = 0; i < 2; i++) {
            way = (i == 0) ? 1 : -1;
            text->chars[i].unk0 -= way * this->unk3C;
            text->chars[i].unk2 += way * (this->unk3C << 1);
        }
        if (this->unk3C >= 17) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->unk3C++;
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CC230);

INCLUDE_ASM("game/nonmatchings/hud", func_800CC3E4);

void func_800CC3E4(Unk800CB7B0 *this);
void func_800CD358(Unk800CD140 *this, s32 value, s32 start, s32 count);

/* A state of D_800D36A4: func_800CC3E4, then in state 1, on its first frame, unkC 2 for 4 characters of the text
 * from the 4th (func_800CD358); in state 3 every character's color 0x80 (func_800CD474). */
void func_800CC6DC(Unk800CB7B0 *this) {
    Unk800CD140 *text;

    func_800CC3E4(this);
    /* The match depends on the text's address taken once, before the switch. */
    text = &this->unk40;
    switch (this->unk38) {
    case 1:
        if (this->unk3C == 1) {
            func_800CD358(text, 2, 4, 4);
        }
        break;
    case 3:
        func_800CD474(text, 0x80);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CC750);

INCLUDE_ASM("game/nonmatchings/hud", func_800CC93C);

INCLUDE_ASM("game/nonmatchings/hud", func_800CCA90);

INCLUDE_ASM("game/nonmatchings/hud", func_800CCC0C);

INCLUDE_ASM("game/nonmatchings/hud", func_800CCE18);

INCLUDE_ASM("game/nonmatchings/hud", func_800CD068);

/* How many characters func_800CD0F4 shows in the visible half of its blink. */
#if VERSION_JP
#define FUNC_800CD0F4_SHOWN 16
#else
#define FUNC_800CD0F4_SHOWN 4
#endif

/* A state of D_800D36A4: on its first frame goes to state 1 and moves the text (JP: y 0x78; US: x and y 0x20), then
 * shows the text (0x30 characters) or nothing, by turns of 16 frames. */
void func_800CD0F4(Unk800CB7B0 *this) {
    if (this->unk38 == 0) {
        this->unk38 = 1;
#if VERSION_JP
        this->unk28.y = 0x78;
        this->unk20.y = 0x78;
        this->unk30 = 13;
#else
        this->unk28.x = 0x20;
        this->unk20.x = 0x20;
        this->unk28.y = 0x20;
        this->unk20.y = 0x20;
#endif
    }
    this->unk30 = (this->unk3C & 0x10) ? 0 : FUNC_800CD0F4_SHOWN;
    this->unk3C++;
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CD140);

/* Sets every character's unk0 and unk2. */
void func_800CD210(Unk800CD140 *this, s32 unk0, s32 unk2) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk0 = unk0;
        this->chars[i].unk2 = unk2;
    }
}

/* Sets unk0 and unk2 of count characters from start on (as far as the text goes). */
void func_800CD248(Unk800CD140 *this, s32 arg1, s32 arg2, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unk0 = arg1;
        this->chars[i].unk2 = arg2;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CD29C);

/* Sets every character's unkC. */
void func_800CD324(Unk800CD140 *this, s32 unkC) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unkC = unkC;
    }
}

/* Sets unkC of count characters from start on (as far as the text goes). */
void func_800CD358(Unk800CD140 *this, s32 value, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unkC = value;
    }
}

/* Sets every character's unk4 and unk6 to the same value. */
void func_800CD3A8(Unk800CD140 *this, s32 value) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk4 = value;
        this->chars[i].unk6 = value;
    }
}

/* Sets every character's unk4 and unk6. */
void func_800CD3E0(Unk800CD140 *this, s32 unk4, s32 unk6) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk4 = unk4;
        this->chars[i].unk6 = unk6;
    }
}

/* Sets unk4 and unk6 of count characters from start on (as far as the text goes). */
void func_800CD418(Unk800CD140 *this, s32 arg1, s32 arg2, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].unk4 = arg1;
        this->chars[i].unk6 = arg2;
    }
}

/* Sets unk548. */
void func_800CD46C(Unk800CD140 *this, s32 value) {
    this->unk548 = value;
}

/* Sets every character's color to the gray value. */
void func_800CD474(Unk800CD140 *this, s32 value) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].color.r = this->chars[i].color.g = this->chars[i].color.b = value;
    }
}

/* Sets every character's color cd byte. */
void func_800CD4B0(Unk800CD140 *this, s32 value) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].color.cd = value;
    }
}

/* Sets the color's cd byte of count characters from start on (as far as the text goes). */
void func_800CD4E4(Unk800CD140 *this, s32 value, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].color.cd = value;
    }
}

/* Sets every character's unk10, adding step for each next one. */
void func_800CD534(Unk800CD140 *this, s32 unk10, s32 step) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk10 = unk10;
        unk10 += step;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CD56C);

/* The entity class of virtual table D_8006DD94 (func_800CD6D4, 0x2C bytes), which func_80071460 makes for the
 * player's side. */
typedef struct Unk800CD6D4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 side; /* gameStateGetPlayerSide */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
} Unk800CD6D4;

extern EntityVtable D_8006DD94;
extern s32 D_8005FB98;

/* Constructs an entity of D_8006DD94 for side, with 0x24 at 0x149, clears D_8005FB98 and starts music track 0x11
 * (once). */
Unk800CD6D4 *func_800CD6D4(Unk800CD6D4 *this, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DD94;
    this->unk24 = 0x149;
    this->side = side;
    this->unk20 = 0;
    this->unk28 = 0;
    this->unk1C = 0;
    D_8005FB98 = 0;
    stopMusic();
    seekMusicTrack(0x11, 1);
    soundControlResumeMusic(&SOUND_CONTROL);
    return this;
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CD754);

INCLUDE_ASM("game/nonmatchings/hud", func_800CD8C8);

INCLUDE_ASM("game/nonmatchings/hud", func_800CDA20);

/* The destructor of the entity class of vtable D_8006DD6C, which func_800CDA20 installs:
 * func_800CDFBC first, then the entity. */
void func_800CDB78(Entity *this, s32 flags) {
    this->vtable = &D_8006DD6C;
    func_800CDFBC((struct Unk800CDA20 *)this);
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CDBD4);

/* The entity class of virtual table D_8006DD6C (func_800CDA20) for a player: three amounts that func_800CDFBC adds
 * into a total. */
typedef struct Unk800CDA20 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 player; /* the pad port padManagerGetPressed reads */
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 amounts[3]; /* func_80070F40 with 0 to 2 (or 3, 4) */
    /* 0x34 */ s32 total;      /* func_8006EF6C */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ RECT unk4C;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
    /* 0x5C */ s32 unk5C;
} Unk800CDA20;

/* The buttons that make func_800CDF14 skip: the four face buttons, and START too in US. */
#if VERSION_JP
#define FUNC_800CDF14_SKIP_BUTTONS 0xF0
#else
#define FUNC_800CDF14_SKIP_BUTTONS 0x8F0
#endif

/* When the player presses a skip button: adds the amounts into the total at once (func_800CDFBC) and sets the
 * fields from 0x38 to their final values. */
void func_800CDF14(Unk800CDA20 *this) {
    if (padManagerGetPressed(PAD_MANAGER_INSTANCE, this->player) & FUNC_800CDF14_SKIP_BUTTONS) {
        func_800CDFBC(this);
        this->unk38 = 0xF2;
        this->unk3C = 0;
        this->unk40 = 0;
        this->unk44 = 0xE8;
        this->unk48 = 0xC5;
        if (this->unk20) {
            this->unk4C.x = 0x40;
            this->unk4C.y = 0x94;
            this->unk4C.w = 0x60;
            this->unk4C.h = 0x18;
        }
        this->unk54 = 10;
        this->unk58 = 0x80;
        this->unk5C = 7;
    }
}

/* Adds the three amounts into the total and clears them. */
void func_800CDFBC(Unk800CDA20 *this) {
    s32 i;

    for (i = 0; i < 3; i++) {
        this->total += this->amounts[i];
        this->amounts[i] = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CDFEC);

INCLUDE_ASM("game/nonmatchings/hud", func_800CE240);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DC90);

INCLUDE_ASM("game/nonmatchings/hud", func_800CE388);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCB8);

INCLUDE_ASM("game/nonmatchings/hud", func_800CEA40);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCC0);

INCLUDE_ASM("game/nonmatchings/hud", func_800CEB0C);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCDC);

INCLUDE_ASM("game/nonmatchings/hud", func_800CED10);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE0);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE4);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE8);

INCLUDE_ASM("game/nonmatchings/hud", func_800CEEDC);

/* Draws one semi-transparent sprite: the place and size from rect, the texture coordinates and clut from uvClut. */
void func_800CF0A0(u_long *ot, RectWords *rect, u32 *uvClut) {
    SpriteWords *sprite = (SpriteWords *)allocSprt();

    sprite->rgbCode = (((SPRT *)sprite)->code << 24) | 0x808080;
    sprite->xy = rect->xy;
    sprite->uvClut = *uvClut;
    sprite->wh = rect->wh;
    /* The match depends on setting the semi-transparency after the size. */
    setSemiTrans((SPRT *)sprite, 1);
    AddPrim(ot, sprite);
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CF124);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCEC);

INCLUDE_ASM("game/nonmatchings/hud", func_800CF2A4);

/* Gives where the font has a character, as (v << 8) | u: the letters, then '0' to ':' (counted from '.'), then '!',
 * '?' and '_'; -1 for any other character. */
s32 func_800CF460(u8 c) {
    s32 index = c;
    u32 v;
    u32 u;

    if ((u8)(index - 'A') < 26) {
        index = (u8)(index - 'A');
        v = (index << 2) & 0xE0;
        u = (index << 5) & 0xFF;
    } else if ((u32)(index - '0') < 11) {
        index = (u8)(index - '.');
        v = (((index & 0xF8) << 2) + 0x60) & 0xFF;
        u = (index << 5) & 0xFF;
    } else {
        switch (index) {
        case '!':
            u = 0xA0;
            v = 0x80;
            break;
        case '?':
            u = 0xC0;
            v = 0x80;
            break;
        case '_':
            u = 0xE0;
            v = 0x80;
            break;
        default:
            return -1;
        }
    }
    return (v << 8) | u;
}

INCLUDE_ASM("game/nonmatchings/hud", func_800CF528);

INCLUDE_ASM("game/nonmatchings/hud", func_800CF6F0);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD04);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD14);

INCLUDE_ASM("game/nonmatchings/hud", func_800CF994);

INCLUDE_ASM("game/nonmatchings/hud", func_800CFB3C);

INCLUDE_ASM("game/nonmatchings/hud", func_800CFD10);

INCLUDE_ASM("game/nonmatchings/hud", func_800CFE48);

/* The destructor of the entity class of vtable D_8006DDBC, which func_800CB7B0 installs. */
void func_800CFFB4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DD94, which func_800CD6D4 installs. */
void func_800CFFFC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DD44, which func_800CFB3C installs. */
void func_800D0044(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Adds a DR_TPAGE (tpage 30) to the ordering table slot. */
void func_800D008C(u_long *ot) {
    AddPrim(ot, allocDrTpageMode(0, 0, 30));
}

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD44);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD6C);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD94);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DDBC);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DDE4);
#if VERSION_JP
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DDFC);
#else
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DDEC);
#endif
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE0C);
#if VERSION_US
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE20);
#endif
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE30);
#if VERSION_JP
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE3C);
#else
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE34);
#endif
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE40);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE4C);
#if VERSION_JP
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE58);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE60);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE68);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE70);
#else
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE54);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE5C);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE64);
#endif
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE7C);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE88);
#if VERSION_JP
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE94);
#else
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DE84);
#endif
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DEA4);
