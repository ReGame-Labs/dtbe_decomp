/* The character and stage selection (the task "Character Select", charasel.tim and mapsel.tim). */

#include "common.h"
#include "engine/cd/file_load.h"
#include "engine/game/game_state.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "title/title_classes.h"
#include "psyq.h"

/* A side's part of Unk8006CC64 (0x38 bytes, sides at 0xC0 and 0xF8). The start is inferred: the init's loop
 * reaches down to 0xC8, and 0x130 follows side 2. */
typedef struct Unk8006CC64Side {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s32 unk10;
    /* 0x14 */ u8 unk14[0x28 - 0x14];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* copied to GameState.fighters[side].unk14 after a versus pick */
    /* 0x34 */ u8 unk34[4];
} Unk8006CC64Side;

/* the object of func_8006CC64: 0x88 is the step it is at */
typedef struct Unk8006CC64 {
    /* 0x000 */ u8 unk0[0x88];
    /* 0x088 */ s32 unk88;
    /* 0x08C */ u8 unk8C[0x94 - 0x8C];
    /* 0x094 */ s32 unk94;
    /* 0x098 */ s32 unk98; /* frames waited in step 0, then the fade-out's level */
    /* 0x09C */ s32 unk9C; /* the fade-out, 0 to 0xFF */
    /* 0x0A0 */ u8 unkA0[0xC0 - 0xA0];
    /* 0x0C0 */ Unk8006CC64Side sides[FIGHTER_COUNT];
    /* 0x130 */ s32 selectFlags; /* what is being picked: SELECT_* */
    /* 0x134 */ u8 unk134[0x158 - 0x134];
    /* 0x158 */ s32 unk158;
    /* 0x15C */ u8 unk15C[0x18C - 0x15C];
    /* 0x18C */ Vec3 position; /* of the object at 0x220 (func_80077420) */
    /* 0x198 */ u8 unk198[4];
    /* 0x19C */ Vec3 rotation; /* of the object at 0x220: angles about x, y, z */
    /* 0x1A8 */ u8 unk1A8[4];
    /* 0x1AC */ s32 arena; /* picked */
    /* 0x1B0 */ u8 unk1B0[0x220 - 0x1B0];
    /* 0x220 */ void *unk220;
} Unk8006CC64;

INCLUDE_ASM("title/nonmatchings/chara_select", characterSelectTaskInit);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B104);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B1DC);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B568);

/* Loads TIM number index of the run tims into VRAM, its image at places[0] and its CLUT at places[1]
 * instead of the TIM's own rects. */
void func_8006B700(u32 *tims, RECT *places, s32 index) {
    TIM_IMAGE image;

    func_800585B0(findTim(tims, index));
    if (ReadTIM(&image) != NULL) {
        if (image.paddr != NULL) {
            func_80057F98(&places[0], image.paddr);
        }
        if (image.caddr != NULL) {
            func_80057F98(&places[1], image.caddr);
        }
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B770);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B924);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BAE8);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BB74);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BD18);

void func_8006BF94(Unk8006CC64 *this);
s32 func_8006C144(Unk8006CC64 *this);
void func_8006C408(Unk8006CC64 *this);
void func_8006C544(Unk8006CC64 *this);
void func_8006FAD8(void);

/* Step 0: waits 32 frames, then goes to step 1 with 0x158 cleared. */
void func_8006BE7C(Unk8006CC64 *this) {
    if (++this->unk98 >= 32) {
        this->unk88 = 1;
        this->unk158 = 0;
    }
}

/* Step 1, the picking: after func_8006BF94, func_8006C144 ending it (not in a challenge) plays sound 0x11 and
 * goes to step 8 with nothing picked. Else each side's 0x10 is cleared and its 0x28 and 0x2C count down by 0x10
 * to 0, and a versus pick goes on in func_8006C544, any other in func_8006C408. */
void func_8006BEA8(Unk8006CC64 *this) {
    s32 i;

    func_8006BF94(this);
    if (!GAME_STATE.challenge && func_8006C144(this)) {
        soundControlPlaySound(&SOUND_CONTROL, 0x11, 0);
        this->selectFlags = 0;
        this->unk88 = 8;
        return;
    }
    for (i = 0; i < FIGHTER_COUNT; i++) {
        this->sides[i].unk10 = 0;
        this->sides[i].unk28 -= 0x10;
        if (this->sides[i].unk28 < 0) {
            this->sides[i].unk28 = 0;
        }
        this->sides[i].unk2C -= 0x10;
        if (this->sides[i].unk2C < 0) {
            this->sides[i].unk2C = 0;
        }
    }
    if ((this->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) != SELECT_VERSUS) {
        func_8006C408(this);
    } else {
        func_8006C544(this);
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BF94);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C144);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C244);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C350);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C408);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C544);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C6E4);

/* Moves the object at 0x220 up by 4 and turns it about y; once 0x94 reaches 0x380
 * (8 a call), goes to step 4 with the object back at height 500. */
void func_8006C8D0(Unk8006CC64 *this) {
    this->position.vy -= 4;
    this->rotation.vy += 0x30;
    func_80077420(this->unk220, &this->position, &this->rotation);
    this->unk94 += 8;
    if (this->unk94 >= 0x380) {
        this->unk88 = 4;
        this->position.vy = 500;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C940);

/* The button that confirms: circle in Japan, cross in the US. */
#if VERSION_JP
#define PAD_CONFIRM 0x20
#else
#define PAD_CONFIRM 0x40
#endif

/* Step 7: the confirm button on any pad goes to step 8. */
void func_8006CA54(Unk8006CC64 *this) {
    if (padManagerGetPressed(PAD_MANAGER_INSTANCE, -1) & PAD_CONFIRM) {
        this->unk88 = 8;
    }
}

/* Moves the object at 0x220 up by 4 and turns it about y; once 0x94 reaches 0x380
 * (8 a call), goes to step 6. */
void func_8006CA94(Unk8006CC64 *this) {
    this->position.vy -= 4;
    this->rotation.vy += 0x30;
    func_80077420(this->unk220, &this->position, &this->rotation);
    this->unk94 += 8;
    if (this->unk94 >= 0x380) {
        this->unk88 = 6;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006CAFC);

/* Calls func_8006FAD8; back at step 1, clears 0x94. */
void func_8006CC2C(Unk8006CC64 *this) {
    func_8006FAD8();
    if (this->unk88 == 1) {
        this->unk94 = 0;
    }
}

/* Starts fading the songs out (step 0x44) and sets 0x88 to 9. */
void func_8006CC64(Unk8006CC64 *this) {
    fadeSongs(0x44);
    this->unk88 = 9;
}

/* Step 9: fades out by 4 a frame and goes to step 10 once done; 0x98 follows as (0xFF - fade) / 8. */
void func_8006CC94(Unk8006CC64 *this) {
    this->unk9C += 4;
    if (this->unk9C >= 0x100) {
        this->unk9C = 0xFF;
        this->unk88 = 10;
    }
    this->unk98 = (0xFF - this->unk9C) >> 3;
}

/* The end of the scene: stores what was picked in GAME_STATE (the arena of a versus fight or a bonus game, and
 * each side's 0x30 after a versus pick). The match depends on the arena read into a local and stored to
 * GAME_STATE.arena first: an assignment chain stores right to left, and the bonus game reads the field again. */
void func_8006CCD4(Unk8006CC64 *this) {
    if (this->selectFlags & SELECT_VERSUS) {
        s32 arena = this->arena;

        GAME_STATE.arena = arena;
        GAME_STATE.lastVersusArena = arena;
    }
    if (this->selectFlags & SELECT_BONUS) {
        s32 arena = this->arena;

        GAME_STATE.arena = arena;
        GAME_STATE.lastBonusArena = arena;
        GAME_STATE.bonusGame = this->arena - ARENA_BONUS;
    }
    if ((this->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) == SELECT_VERSUS) {
        GAME_STATE.fighters[0].unk14 = this->sides[0].unk30;
        GAME_STATE.fighters[1].unk14 = this->sides[1].unk30;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006CD6C);

/* The place of cell `cell` of the character grid, from (x, y): row 1 holds cells 0-5 with a gap after the third,
 * row 2 cells 6-14, row 3 the rest, 30 pixels apart. */
void func_8006D98C(DVECTOR *pos, s32 x, s32 y, s32 cell) {
    if (cell < 6) {
        s32 left = cell * 30;

        pos->vx = left + 8;
        if (cell >= 3) {
            pos->vx = left + 0x62;
        }
        pos->vy = 11;
    } else if (cell < 15) {
        pos->vx = (cell - 6) * 30 + 8;
        pos->vy = 0x29;
    } else {
        pos->vx = (cell - 15) * 30 + 8;
        pos->vy = 0x47;
    }
    pos->vx += x;
    pos->vy += y;
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DA20);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DBA4);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DCCC);

/* An entry of D_80064C40 (20 bytes): unk10 picks a picture (its texture page, u, v and palette). The asm splits the
 * table at D_80064C44, inside entry 0. */
typedef struct Unk80064C40 {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u16 picture;
    /* 0x12 */ s16 unk12;
} Unk80064C40;

extern Unk80064C40 D_80064C40[];

void func_8006DFA4(u_long *ot, s32 x, s32 y, s32 index, s32 arg4);
void func_8006E1A0(u_long *ot, s32 x, s32 y);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DFA4);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006E1A0);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006E368);

/* Draws entry index's picture (128 x 24, from D_80064C40) sliding in from the left with frame, then
 * func_8006E1A0 at its x and func_8006DFA4 for the entry. */
void func_8006E534(u_long *ot, s32 index, s32 frame) {
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 picture;
    s32 x;
    s32 clutX;
    s32 right; /* the picture sits in the right half of its page */

    /* The match depends on the halfword read as u16 and cast to s16 into an s32 (lhu and two shifts, and an
     * SImode % 10), and below on the u flag kept in a local (as an expression, fold turns it into a branch). */
    picture = (s16)D_80064C40[index].picture;
    tpage = allocDrTpage(picture >= 10 ? 13 : 12);
    sprt = allocSprt();
    x = (frame - 16) * 16;
    if (x > 0x90) {
        x = 0x90;
    }
    right = picture < 10 || picture >= 20;
    setXY0(sprt, x - 0x80, 100);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, right << 7, (picture % 10) * 24);
    clutX = picture >= 16 ? 0x310 : 0x300;
    sprt->clut = getClut(clutX, 0xF0 + (picture & 0xF));
    setWH(sprt, 0x80, 0x18);
    setSemiTrans(sprt, 0);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    func_8006E1A0(ot, x, 0x70);
    x = frame * 8;
    if (x > 0x80) {
        x = 0x80;
    }
    func_8006DFA4(ot, x - 0x6C, 0xC, index, 0);
}

void func_8006E368(u_long *ot, s32 x, s32 y);

/* As func_8006E534 from the right: entry index's picture slides in from the right edge (0x140) with frame,
 * then func_8006E368 at its x and func_8006DFA4 for the entry. */
void func_8006E6F0(u_long *ot, s32 index, s32 frame) {
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 picture;
    s32 slide;
    s32 x;
    s32 clutX;
    s32 right; /* the picture sits in the right half of its page */

    /* The match depends on the halfword read as u16 and cast to s16 into an s32, and on the u flag kept in a
     * local, as in func_8006E534. */
    picture = (s16)D_80064C40[index].picture;
    tpage = allocDrTpage(picture >= 10 ? 13 : 12);
    sprt = allocSprt();
    slide = (frame - 16) * 16;
    if (slide > 0x90) {
        slide = 0x90;
    }
    x = 0x140 - slide;
    right = picture < 10 || picture >= 20;
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setXY0(sprt, x, 0x10);
    setUV0(sprt, right << 7, (picture % 10) * 24);
    clutX = picture >= 16 ? 0x310 : 0x300;
    sprt->clut = getClut(clutX, 0xF0 + (picture & 0xF));
    setWH(sprt, 0x80, 0x18);
    setSemiTrans(sprt, 0);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    func_8006E368(ot, x, 0x1C);
    slide = frame * 8;
    if (slide > 0x80) {
        slide = 0x80;
    }
    func_8006DFA4(ot, 0x140 - slide, 0x21, index, 1);
}

/* Draws an 8-step gauge at pos: (level + 1) orange steps of 11, filled from the right, over a black
 * 0x58 x 13 bar. */
void func_8006E8BC(u_long *ot, DVECTOR pos, s32 level) {
    DR_TPAGE *tpage;
    TILE *tile;
    s32 width;

    tpage = allocDrTpage(5);
    tile = allocTile();
    setRGB0(tile, 0xFF, 0x80, 0);
    width = (level + 1) * 11;
    setXY0(tile, pos.vx + 0x5C - width, pos.vy + 0xF);
    setWH(tile, width, 0xD);
    SetSemiTrans(tile, 0);
    AddPrim(ot, tile);
    tile = allocTile();
    setRGB0(tile, 0, 0, 0);
    setXY0(tile, pos.vx + 4, pos.vy + 0xF);
    setWH(tile, 0x58, 0xD);
    SetSemiTrans(tile, 0);
    AddPrim(ot, tile);
    AddPrim(ot, tpage);
}

/* Draws an 8-step gauge at pos: (level + 1) orange steps of 11, filled from the left, over a black
 * 0x58 x 13 bar. */
void func_8006E9D8(u_long *ot, DVECTOR pos, s32 level) {
    DR_TPAGE *tpage;
    TILE *tile;
    s32 width;

    tpage = allocDrTpage(5);
    tile = allocTile();
    setRGB0(tile, 0xFF, 0x80, 0);
    width = (level + 1) * 11;
    setXY0(tile, pos.vx + 4, pos.vy + 0xF);
    setWH(tile, width, 0xD);
    SetSemiTrans(tile, 0);
    AddPrim(ot, tile);
    tile = allocTile();
    setRGB0(tile, 0, 0, 0);
    setXY0(tile, pos.vx + 4, pos.vy + 0xF);
    setWH(tile, 0x58, 0xD);
    SetSemiTrans(tile, 0);
    AddPrim(ot, tile);
    AddPrim(ot, tpage);
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006EAF0);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006EED4);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006EF94);

/* Draws sprite `sprite` of D_800651B0's table at pos in ot: mirrored left to right as a POLY_FT4 when
 * poly is set, else as a SPRT (with its own DR_TPAGE). */
void func_8006FEB8(u_long *ot, DVECTOR *pos, s32 sprite, s32 poly);

/* Draws sprites 7, 14 (when flag is set) and 16 of the title's table at (x, y) in ot. */
void func_8006F274(u_long *ot, s32 x, s32 y, s32 flag) {
    DVECTOR pos;

    pos.vx = x;
    pos.vy = y;
    func_8006FEB8(ot, &pos, 7, 0);
    if (flag) {
        pos.vx = x + 0x3A;
        pos.vy = y + 0x30;
        func_8006FEB8(ot, &pos, 14, 0);
    }
    pos.vx = x + 0xA;
    pos.vy = y + 8;
    func_8006FEB8(ot, &pos, 16, 0);
}

/* Draws sprites 8, 15 (when flag is set) and 17 of the title's table at (x, y) in ot. */
void func_8006F31C(u_long *ot, s32 x, s32 y, s32 flag) {
    DVECTOR pos;

    pos.vx = x;
    pos.vy = y;
    func_8006FEB8(ot, &pos, 8, 0);
    if (flag) {
        pos.vx = x + 0xF;
        pos.vy = y + 0x30;
        func_8006FEB8(ot, &pos, 15, 0);
    }
    pos.vx = x + 0xF;
    pos.vy = y + 8;
    func_8006FEB8(ot, &pos, 17, 0);
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006F3C4);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006FAD8);

/* An object with a model at 0x84; nothing references func_8006FE5C. */
typedef struct Unk8006FE5C {
    /* 0x00 */ u8 unk0[0x84];
    /* 0x84 */ Model *model;
} Unk8006FE5C;

/* Animates the model at 0x84 and draws it in camera. */
void func_8006FE5C(Unk8006FE5C *this, Camera *camera) {
    Model *model = this->model;

    modelUpdateAnimation(model);
    SCENE_OBJECT_INVALIDATE(&model->group.object);
    sceneObjectDrawInView(&model->group.object, camera);
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006FEB8);

INCLUDE_ASM("title/nonmatchings/chara_select", func_800700A4);

/* The sum 0 + 1 + ... + count - 1. */
u32 func_80070298(s32 count) {
    s32 i;
    u32 sum = 0;

    for (i = 0; i < count; i++) {
        sum += i;
    }
    return sum;
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_800702C4(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}
