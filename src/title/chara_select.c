/* The character and stage selection (the task "Character Select", charasel.tim and mapsel.tim). */

#include "common.h"
#include "engine/cd/file_load.h"
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/prim/alloc_dr_area.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/matrix.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "title/title_classes.h"
#include "psyq.h"

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

/* A side's part of Unk8006CC64 (0x38 bytes, sides at 0xC0 and 0xF8). */
typedef struct Unk8006CC64Side {
    /* 0x00 */ s32 unk0; /* set when the side has nothing to pick (SELECT_SIDE_* and 0x10 << side clear) */
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 computer; /* its pick is the computer's (selectFlags 0x10 << side, SELECT_COMPUTER_1) */
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 index; /* of the side */
    /* 0x18 */ s32 cell; /* of its character, D_80064E20[character] */
    /* 0x1C */ u8 unk1C[4];
    /* 0x20 */ s32 unk20;
    /* 0x24 */ Unk80064C40 *entry; /* &D_80064C40[cell] */
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* copied to GameState.fighters[side].unk14 after a versus pick */
    /* 0x34 */ s32 unk34;
} Unk8006CC64Side;

/* the object of func_8006CC64: 0x88 is the step it is at */
typedef struct Unk8006CC64 {
    /* 0x000 */ Task task;
    /* 0x020 */ u8 unk20[4];
    /* 0x024 */ Camera camera;
    /* 0x060 */ MATRIX view;
    /* 0x080 */ u8 unk80[0x88 - 0x80];
    /* 0x088 */ s32 unk88;
    /* 0x08C */ s32 unk8C;
    /* 0x090 */ s32 unk90;
    /* 0x094 */ s32 unk94;
    /* 0x098 */ s32 unk98; /* frames waited in step 0, then the fade-out's level */
    /* 0x09C */ s32 unk9C; /* the fade-out, 0 to 0xFF */
    /* 0x0A0 */ u8 unkA0[4];
    /* 0x0A4 */ s32 frame; /* counted by the update */
    /* 0x0A8 */ s32 unkA8; /* 3 while func_8006FAD8 picks the arena, 4 once confirmed */
    /* 0x0AC */ s32 unkAC; /* the arena pick's slide-in, 0 to 0xA0 (func_8006F3C4) */
    /* 0x0B0 */ s32 unkB0; /* the arena pick's y, 0x72 to 0x100 */
    /* 0x0B4 */ int unkB4; /* half the height of the arena's picture, 0 to 0x20 (a plain int: see func_8006F3C4) */
    /* 0x0B8 */ s32 unkB8;
    /* 0x0BC */ s32 unkBC; /* counted every frame of steps 3 and 4 of func_8006F3C4 */
    /* 0x0C0 */ Unk8006CC64Side sides[FIGHTER_COUNT];
    /* 0x130 */ s32 selectFlags; /* what is being picked: SELECT_* */
    /* 0x134 */ u8 unk134[0x13C - 0x134];
    /* 0x13C */ s32 unk13C;
    /* 0x140 */ u8 unk140[0x154 - 0x140];
    /* 0x154 */ u32 sound; /* the handle of the sound func_8006BF94 plays */
    /* 0x158 */ s32 unk158; /* func_8006BF94's step */
    /* 0x15C */ s32 unk15C; /* frames left to wait, counted down by func_8006BF94 */
    /* 0x160 */ u8 unk160[0x170 - 0x160];
    /* 0x170 */ s32 unk170; /* the scale (4.12) of the picture func_8006F3C4 zooms in */
    /* 0x174 */ s32 unk174; /* its angle about z */
    /* 0x178 */ s32 unk178;
    /* 0x17C */ s32 unk17C; /* the level of the black over the screen */
    /* 0x180 */ s32 unk180; /* the picture's brightness above 0x80 */
    /* 0x184 */ u8 unk184[0x18C - 0x184];
    /* 0x18C */ Vec3 position; /* of the object at 0x220 (func_80077420) */
    /* 0x198 */ u8 unk198[4];
    /* 0x19C */ Vec3 rotation; /* of the object at 0x220: angles about x, y, z */
    /* 0x1A8 */ u8 unk1A8[4];
    /* 0x1AC */ s32 arena; /* picked */
    /* 0x1B0 */ s32 characters[FIGHTER_COUNT]; /* by side */
    /* 0x1B8 */ s32 unk1B8[FIGHTER_COUNT]; /* by side: GameState.fighters[side].unk8 */
    /* 0x1C0 */ s32 pickable[CHARACTER_COUNT]; /* by cell of the grid */
    /* 0x220 */ struct Unk80076E24 *unk220; /* a title character model entity (title_0a.c) */
    /* 0x224 */ EntityGroup entities;
} Unk8006CC64; /* size 0x234 */

INCLUDE_ASM("title/nonmatchings/chara_select", characterSelectTaskInit);

INCLUDE_RODATA("title/nonmatchings/chara_select", D_8006480C);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_8006481C);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_8006482C);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_8006483C);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B104);

INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064850);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_800649D0);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B1DC);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B568);

/* Loads TIM number index of the run tims into VRAM, its image at places[0] and its CLUT at places[1]
 * instead of the TIM's own rects. */
void func_8006B700(u32 *tims, RECT *places, s32 index) {
    TIM_IMAGE image;

    OpenTIM((u_long *)findTim(tims, index));
    if (ReadTIM(&image) != NULL) {
        if (image.paddr != NULL) {
            LoadImage2(&places[0], image.paddr);
        }
        if (image.caddr != NULL) {
            LoadImage2(&places[1], image.caddr);
        }
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B770);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006B924);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BAE8);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006BB74);

void func_8006BE7C(Unk8006CC64 *this);
void func_8006BEA8(Unk8006CC64 *this);
void func_8006BF94(Unk8006CC64 *this);
s32 func_8006C144(Unk8006CC64 *this);
void func_8006C408(Unk8006CC64 *this);
void func_8006C544(Unk8006CC64 *this);
void func_8006C8D0(Unk8006CC64 *this);
void func_8006C940(Unk8006CC64 *this);
void func_8006CA54(Unk8006CC64 *this);
void func_8006CA94(Unk8006CC64 *this);
void func_8006CAFC(Unk8006CC64 *this);
void func_8006CC2C(Unk8006CC64 *this);
void func_8006CC64(Unk8006CC64 *this);
void func_8006CC94(Unk8006CC64 *this);
void func_8006CCD4(Unk8006CC64 *this);
void func_8006CD6C(Unk8006CC64 *this, s32 arg);
void func_8006FAD8(Unk8006CC64 *this);

/* The character select's frame: runs the step at 0x88, then draws and counts the frame; step 10 stores the picks
 * and kills the task. The cases are in the order of their blocks in the asm. */
void func_8006BD18(Unk8006CC64 *this, s32 arg) {
    switch (this->unk88) {
    case 0:
        func_8006BE7C(this);
        break;
    case 1:
        func_8006BEA8(this);
        break;
    case 3:
        func_8006C8D0(this);
        break;
    case 4:
        func_8006C940(this);
        /* fall through */
    case 5:
        this->position.vy -= 8;
        if (this->position.vy < 0x104) {
            this->position.vy = 0x104;
        }
        this->position.vx = 200;
        this->rotation.vy = 0x880;
        func_80077420(this->unk220, &this->position, &this->rotation);
        func_8006CA54(this);
        break;
    case 2:
        func_8006CA94(this);
        break;
    case 6:
        func_8006CAFC(this);
        break;
    case 7:
        func_8006CC2C(this);
        break;
    case 8:
        func_8006CC64(this);
        break;
    case 9:
        func_8006CC94(this);
        break;
    case 10:
    default:
        func_8006CCD4(this);
        taskKill(&this->task);
        return;
    }
    func_8006CD6C(this, arg);
    this->frame++;
}

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

/* Plays sounds 0x23, 0x24 and 0x25 in turn, each handed to the model at 0x220 (0x24 once 0x23 ended and 90 frames
 * passed, 0x25 likewise after 300); step 7 stops the sound if it still plays. */
void func_8006BF94(Unk8006CC64 *this) {
    if (--this->unk15C < 0) {
        this->unk15C = 0;
    }
    switch (this->unk158) {
    case 0:
        func_800773C8(this->unk220, 11);
        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x23, 0);
        func_800773E8(this->unk220, this->sound);
        this->unk158++;
        break;
    case 1:
        if (!isSoundPlaying(this->sound)) {
            this->unk15C = 90;
            this->unk158++;
        }
        break;
    case 2:
        if (!isSoundPlaying(this->sound) && this->unk15C == 0) {
            func_800773C8(this->unk220, 11);
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x24, 0);
            func_800773E8(this->unk220, this->sound);
            this->unk158++;
        }
        break;
    case 3:
        if (!isSoundPlaying(this->sound)) {
            this->unk15C = 300;
            this->unk158++;
        }
        break;
    case 4:
        if (!isSoundPlaying(this->sound) && this->unk15C == 0) {
            func_800773C8(this->unk220, 11);
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x25, 0);
            func_800773E8(this->unk220, this->sound);
            this->unk158 = 7;
        }
        break;
    case 7:
        if (isSoundPlaying(this->sound) == 1) {
            stopSound(this->sound);
        }
        this->unk158++;
        break;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C144);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C244);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C350);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C408);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C544);

INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064AE0);

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

INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064AE8);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064AFC);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006C940);

/* The button that confirms: circle in Japan, cross in the US. */
#if VERSION_JP
#define PAD_CONFIRM 0x20
#else
#define PAD_CONFIRM 0x40
#endif

/* The end of steps 4 and 5: the confirm button on any pad goes to step 8. */
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
    func_8006FAD8(this);
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

/* An arena's picture in VRAM: its place in the texture page, its palette and the page. */
typedef struct Unk80065268 {
    /* 0x0 */ u8 u;
    /* 0x1 */ u8 v;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u16 clut;
    /* 0x6 */ u16 tpage;
} Unk80065268;

/* by arena */
extern Unk80065268 D_80065268[];

void func_800700A4(u_long *ot, DVECTOR *pos, s32 sprite, s32 shade, s32 poly);

/* The arena pick's animation (its step at 0xA8) and drawing. Up to step 4 it slides the pick in and opens the
 * picture of the arena (D_80065268); from step 5 a picture zooms out and the screen fades to black, then step 9
 * goes to step 8 of the scene. The pick's sprites take rect's x and y as their position: the game keeps one stack
 * slot for both. The match depends on unkB4 being a plain int (through the s32 typedef, narrowing the picture's
 * bottom y to a short reassociates it as (y + 0x2B) - h), on one variable for the scale and on the translation
 * being set before the scale words. */
void func_8006F3C4(Unk8006CC64 *this, u_long *ot) {
    switch (this->unkA8) {
    case 0:
        this->unkAC = 0;
        this->unkB0 = 0x72;
        this->unkB4 = 0;
        this->unkB8 = 0;
        this->unkBC = 0;
        break;
    case 1:
        this->unkAC += 8;
        if (this->unkAC > 0xA0) {
            this->unkAC = 0xA0;
            this->unkA8++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        this->unkB8++;
        break;
    case 2:
        this->unkB4 += 2;
        if (this->unkB4 > 0x20) {
            this->unkB4 = 0x20;
            this->unkA8++;
        }
        break;
    case 4:
        this->unkB0 += 8;
        if (this->unkB0 > 0x100) {
            this->unkB0 = 0x100;
        }
        if (--this->unkB8 < 0) {
            this->unkB8 = 0;
        }
        if (this->unkB0 == 0x100 && this->unkB8 == 0) {
            this->unkA8++;
        }
        /* fall through */
    case 3:
        this->unkBC++;
        break;
    case 5:
        this->unk170 = 0x4000;
        this->unk174 = 0;
        this->unk17C = 0;
        this->unk178 = 0;
        this->unk180 = 0;
        this->unkA8++;
        /* fall through */
    case 6:
        this->unk170 -= 0x200;
        if (this->unk170 < 0x1000) {
            this->unk170 = 0x1000;
            this->unk180 = 0x7F;
            this->unkA8++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        break;
    case 7:
        this->unk180 -= 2;
        if (this->unk180 < 0) {
            this->unk180 = 0;
        }
        if (this->unk178++ > 60) {
            this->unkA8++;
        }
        break;
    case 8:
        this->unk17C += 8;
        if (this->unk17C >= 0xFF) {
            this->unk17C = 0xFF;
            this->unkA8++;
        }
        this->unk170 += 0x200;
        this->unk174 += 0x80;
        break;
    case 9:
        this->unk88 = 8;
        break;
    }
    if (this->unkA8 >= 5) {
        if (this->unk17C != 0) {
            DR_TPAGE *tpage = allocDrTpage(0x25);
            TILE *tile = allocTile();

            setRGB0(tile, this->unk17C, this->unk17C, this->unk17C);
            setXY0(tile, 0, 0);
            setWH(tile, 320, 240);
            SetSemiTrans(tile, 1);
            AddPrim(ot, tile);
            AddPrim(ot, tpage);
        }
        {
            VECTOR corners[4] = {
                { -76, -40, 0 },
                { 75, -40, 0 },
                { -76, 39, 0 },
                { 75, 39, 0 },
            };
            VECTOR screen[4];
            MatrixWords m;
            u32 scale = (u16)this->unk170;
            POLY_FT4 *poly;

            m.m.t[0] = 0xA0;
            m.m.t[1] = 0xA0;
            m.m.t[2] = 0;
            m.words[0] = scale;
            m.words[1] = 0;
            m.words[2] = scale;
            m.words[3] = 0;
            m.words[4] = scale;
            matrixRotateZ(&m.m, this->unk174);
            matrixTransformVecs(&m.m, screen, corners, 4);
            poly = allocPolyFT4();
            setXY4(poly, screen[0].vx, screen[0].vy, screen[1].vx, screen[1].vy, screen[2].vx, screen[2].vy,
                   screen[3].vx, screen[3].vy);
            setUV4(poly, 0, 0, 0x97, 0, 0, 0x4F, 0x97, 0x4F);
            setRGB0(poly, this->unk180 + 0x80, this->unk180 + 0x80, this->unk180 + 0x80);
            poly->tpage = 0x88;
            poly->clut = 0x3D98;
            setSemiTrans(poly, 1);
            AddPrim(ot, poly);
        }
    } else {
        RECT rect;

        rect.x = this->unkAC - 0x50;
        rect.y = this->unkB0;
        func_8006FEB8(ot, (DVECTOR *)&rect, 9, 0);
        rect.x = 320 - this->unkAC;
        func_8006FEB8(ot, (DVECTOR *)&rect, 10, 0);
        if (this->unkB4 > 0) {
            Unk80065268 *picture = &D_80065268[this->arena];
            POLY_FT4 *poly = allocPolyFT4();

            setRGB0(poly, 0x80, 0x80, 0x80);
            setXYWH(poly, 0x78, this->unkB0 + 0x2B - this->unkB4, 0x50, this->unkB4 * 2);
            setUVWH(poly, picture->u, picture->v, 0x4F, 0x3F);
            poly->tpage = picture->tpage;
            poly->clut = picture->clut;
            SetSemiTrans(poly, 0);
            AddPrim(ot, poly);
        }
        getDrawBufferRect(&rect);
        AddPrim(ot, allocDrArea(&rect));
        if (this->unkAC == 0xA0) {
            rect.x = 0xE8 - (u8)this->unkBC;
            rect.y = this->unkB0 + 0x5A;
            func_8006FEB8(ot, (DVECTOR *)&rect, 13, 0);
        }
        getDrawBufferSubRect(&rect, 0x5A, this->unkB0 + 0x5A, 0x8C, 0x12);
        if (rect.h > 0) {
            AddPrim(ot, allocDrArea(&rect));
        }
        rect.x = this->unkAC - 0x48;
        rect.y = this->unkB0 + 0x5A;
        func_800700A4(ot, (DVECTOR *)&rect, 11, 0x80, 0);
        rect.x = 320 - this->unkAC;
        func_800700A4(ot, (DVECTOR *)&rect, 12, 0x80, 0);
    }
}

/* The arena pick, while 0xA8 is 3: plays sounds 0x27, 0x28 and 0x29 in turn (0x29 again every 300 frames),
 * left and right step through the arenas (a fight's 0 to 5, the boss's too once a run was won; with SELECT_BONUS
 * the bonus games, skipping those whose bit of save.unk18 is clear) and the confirm button sets 0xA8 to 4.
 * The match depends on save.unk18 being a u32 (with an s32 mask, fold turns the bit test into a shift of the
 * mask) and on each loop having its own counter. */
void func_8006FAD8(Unk8006CC64 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    u32 pressed = padManagerGetPressed(pads, -1);
    u32 repeated = padManagerGetRepeated(pads, -1);
    s32 first;
    s32 last;

    if (this->unkA8 != 3) {
        return;
    }
    if (--this->unk15C < 0) {
        this->unk15C = 0;
    }
    this->unk15C--;
    switch (this->unk158) {
    case 0:
        this->unk15C = 60;
        break;
    case 1:
        if (this->unk15C == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x27, 0);
            this->unk158++;
        }
        break;
    case 2:
        if (!isSoundPlaying(this->sound)) {
            this->unk15C = 90;
            this->unk158++;
        }
        break;
    case 3:
        if (this->unk15C == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x28, 0);
            this->unk158++;
        }
        break;
    case 4:
        if (!isSoundPlaying(this->sound)) {
            this->unk15C = 300;
            this->unk158++;
        }
        break;
    case 5:
        if (this->unk15C == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x29, 0);
            this->unk158 = 4;
        }
        break;
    case 7:
        if (isSoundPlaying(this->sound) == 1) {
            stopSound(this->sound);
        }
        this->unk158++;
        break;
    }
    first = 0;
    last = ARENA_PICK_COUNT - 1;
    if (GAME_STATE.save.cleared & 0xFFF) {
        last = ARENA_BOSS;
    }
    if (this->selectFlags & SELECT_BONUS) {
        first = ARENA_BONUS;
        last = ARENA_BONUS + BONUS_GAME_COUNT - 1;
    }
    if (repeated & (PAD_LEFT | PAD_RIGHT)) {
        soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
    }
    if (!(this->selectFlags & SELECT_BONUS)) {
        if (repeated & PAD_LEFT) {
            if (--this->arena < first) {
                this->arena = last;
            }
        }
        if (repeated & PAD_RIGHT) {
            if (++this->arena > last) {
                this->arena = first;
            }
        }
    } else {
        if (repeated & PAD_LEFT) {
            s32 i;

            for (i = 0; i < BONUS_GAME_COUNT; i++) {
                if (--this->arena < first) {
                    this->arena = last;
                }
                if (GAME_STATE.save.unk18 & (1 << this->arena)) {
                    break;
                }
            }
        }
        if (repeated & PAD_RIGHT) {
            s32 i;

            for (i = 0; i < BONUS_GAME_COUNT; i++) {
                if (++this->arena > last) {
                    this->arena = first;
                }
                if (GAME_STATE.save.unk18 & (1 << this->arena)) {
                    break;
                }
            }
        }
    }
    if (pressed & PAD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
        this->unkA8 = 4;
    }
}

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

INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064C10);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064C40);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064C44);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064E20);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064E80);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064E88);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80065120);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80065140);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80065160);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80065180);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_800651A0);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_800651B0);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80065268);
