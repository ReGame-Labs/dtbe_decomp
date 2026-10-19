/* The character and stage selection (the task "Character Select", charasel.tim and mapsel.tim). Built as C++:
 * the task has g++'s destructor and virtual table, and CharacterSelectTask_drawGrid passes its DVECTOR by value
 * through one stack copy per call, g++'s argument temporary (C passes the variable itself). */

#include "common.h"
#include <stdio.h>
#include "engine/cd/file_load.h"
#include "engine/debug/system_menu.h"
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/prim/alloc_dr_area.h"
#include "engine/gfx/prim/alloc_poly_f4.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/matrix.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "title/chara_select.h"
#include "title/title_classes.h"
#include "psyq.h"

/* CharacterSelectTask.step */
#define CHARACTER_SELECT_STEP_ENTER 0
#define CHARACTER_SELECT_STEP_PICK 1
#define CHARACTER_SELECT_STEP_ARENA_EXIT 2
#define CHARACTER_SELECT_STEP_RUN_EXIT 3
#define CHARACTER_SELECT_STEP_RUN_LOAD 4
#define CHARACTER_SELECT_STEP_RUN_SHOW 5
#define CHARACTER_SELECT_STEP_ARENA_LOAD 6
#define CHARACTER_SELECT_STEP_ARENA 7
#define CHARACTER_SELECT_STEP_FADE_SONGS 8
#define CHARACTER_SELECT_STEP_FADE_OUT 9
#define CHARACTER_SELECT_STEP_END 10

/* CharacterSelectTask.arenaStep (CharacterSelectTask_drawArena) */
#define ARENA_STEP_INIT 0
#define ARENA_STEP_SLIDE_IN 1
#define ARENA_STEP_OPEN_PICTURE 2
#define ARENA_STEP_PICK 3
#define ARENA_STEP_CONFIRMED 4
#define ARENA_STEP_ZOOM_INIT 5
#define ARENA_STEP_ZOOM_OUT 6
#define ARENA_STEP_FADE_PICTURE 7
#define ARENA_STEP_BLACK_OUT 8
#define ARENA_STEP_END 9

EXTERN_C_BEGIN
extern GridCell GRID_CELLS[];

void CharacterSelectTask_pickCharacter(CharacterSelectSide *side, CharacterSelectTask *task);
void CharacterSelectTask_pickGauge(CharacterSelectSide *side, CharacterSelectTask *task);
void uploadTimAt(u32 *tims, RECT *places, s32 index);
void uploadSidePictures(u32 *file, s32 character, s32 tamer, s32 side);
void uploadRunPictures(u32 *file, s32 character, s32 tamer);
s32 CharacterSelectTask_isTamerPictureWide(CharacterSelectTask *task, s32 side);
void CharacterSelectTask_loadTamerVoice(CharacterSelectTask *task, s32 side, s32 tamer);
void CharacterSelectTask_stepEnter(CharacterSelectTask *task);
void CharacterSelectTask_stepPick(CharacterSelectTask *task);
void CharacterSelectTask_tickHostVoice(CharacterSelectTask *task);
s32 CharacterSelectTask_isBackedOut(CharacterSelectTask *task);
s32 CharacterSelectTask_checkChallenger(CharacterSelectTask *task);
void CharacterSelectTask_joinSide(CharacterSelectTask *task, s32 character, s32 side);
void CharacterSelectTask_pickCharacters(CharacterSelectTask *task);
void CharacterSelectTask_pickVersus(CharacterSelectTask *task);
void CharacterSelectTask_storePicks(CharacterSelectTask *task);
void CharacterSelectTask_stepRunExit(CharacterSelectTask *task);
void CharacterSelectTask_loadRunPictures(CharacterSelectTask *task);
void CharacterSelectTask_waitConfirm(CharacterSelectTask *task);
void CharacterSelectTask_stepArenaExit(CharacterSelectTask *task);
void CharacterSelectTask_loadArenaPictures(CharacterSelectTask *task);
void CharacterSelectTask_stepArena(CharacterSelectTask *task);
void CharacterSelectTask_startFadeOut(CharacterSelectTask *task);
void CharacterSelectTask_stepFadeOut(CharacterSelectTask *task);
void CharacterSelectTask_storeArena(CharacterSelectTask *task);
void CharacterSelectTask_draw(CharacterSelectTask *task, s32 arg);
void getCellPosition(DVECTOR *pos, s32 x, s32 y, s32 cell);
void drawGridBackdrop(u_long *ot, s32 x, s32 y);
void drawScrollingStrip(u_long *ot, s32 x, s32 y, s32 frame);
void drawSelectSideStrips(u_long *ot, s32 frame);
void drawCharacterPortrait(u_long *ot, s32 x, s32 y, s32 index, s32 side);
void func_8006E1A0(u_long *ot, s32 x, s32 y);
void func_8006E368(u_long *ot, s32 x, s32 y);
void drawLeftCellInfo(u_long *ot, s32 index, s32 frame);
void drawRightCellInfo(u_long *ot, s32 index, s32 frame);
void drawGaugeFromRight(u_long *ot, DVECTOR pos, s32 level);
void drawGaugeFromLeft(u_long *ot, DVECTOR pos, s32 level);
void CharacterSelectTask_drawGrid(CharacterSelectTask *task, u_long *ot, s32 x, s32 y, s32 cell0, s32 cell1,
                                  s32 frame);
void drawSelectBackground(CharacterSelectTask *task, u_long *ot);
void func_8006F274(u_long *ot, s32 x, s32 y, s32 flag);
void func_8006F31C(u_long *ot, s32 x, s32 y, s32 flag);
void CharacterSelectTask_drawArena(CharacterSelectTask *task, u_long *ot);
void CharacterSelectTask_pickArena(CharacterSelectTask *task);
void func_8006FE5C(struct Unk8006FE5C *object, Camera *camera);
/* Draws sprite `sprite` of SELECT_SPRITES's table at pos in ot: mirrored left to right as a POLY_FT4 when
 * poly is set, else as a SPRT (with its own DR_TPAGE). */
void drawSelectSprite(u_long *ot, DVECTOR *pos, s32 sprite, s32 poly);
void drawSelectSpriteShaded(u_long *ot, DVECTOR *pos, s32 sprite, s32 shade, s32 poly);
u32 getTriangularSum(s32 count);
EXTERN_C_END

INCLUDE_ASM("title/nonmatchings/chara_select", characterSelectTaskInit);

INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_UP);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_TARGET);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_EYE);
INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_CHARASEL_TIM);

/* The character select's destructor: turns dithering back on, deletes the host, leaves the scene's result
 * for the sequencer (-1 when a side backed out, else CHARACTER_SELECT_CHALLENGE when a player joined in, 0
 * otherwise), unloads the sounds and destroys the entities and the camera; g++ then destroys the Task. */
CharacterSelectTask::~CharacterSelectTask() {
    s32 result;

    displayEnable(CURRENT_DISPLAY, DISPLAY_DITHER);
    entityDelete(&this->chara->entity);
    /* The match depends on the result set, then overridden: a ternary branches around the second load. */
    result = this->challengerJoined != 0;
    if (this->backedOut) {
        result = -1;
    }
    SYSTEM_CONTEXT.sceneResult = result;
    soundControlUnloadVabs(&SOUND_CONTROL);
    soundControlUnloadSeps(&SOUND_CONTROL);
    entityGroupDestroy(&this->entities, DESTROY_BASES);
    cameraDestroy(&this->camera, DESTROY_BASES);
}

INCLUDE_RODATA("title/nonmatchings/chara_select", CELL_NEIGHBORS);
INCLUDE_RODATA("title/nonmatchings/chara_select", CELL_PAIRS);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_pickCharacter);

/* A side's versus gauge pick: left/right move the gauge 0 to VERSUS_GAUGE_MAX (toward the screen's middle is up), confirm picks
 * it and cancel goes back (to the character pick when the gauge is not picked yet). */
void CharacterSelectTask_pickGauge(CharacterSelectSide *side, CharacterSelectTask *task) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 pad = side->index;
    u32 pressed;
    u32 repeated;
    s32 gauge;

    if (side->computer) {
        pad ^= 1;
    }
    pressed = padManagerGetPressed(pads, pad);
    repeated = padManagerGetRepeated(pads, pad);
    if (side->gaugePicked) {
        if (pressed & PAD_CANCEL) {
            side->gaugePicked = 0;
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        }
        return;
    }
    if (pressed & PAD_CANCEL) {
        side->picked = 0;
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        return;
    }
    gauge = side->gauge;
    if (side->index == 0) {
        if (repeated & PADLleft) {
            if (++side->gauge > VERSUS_GAUGE_MAX) {
                side->gauge = VERSUS_GAUGE_MAX;
            }
        }
        if (repeated & PADLright) {
            if (--side->gauge < 0) {
                side->gauge = 0;
            }
        }
    } else {
        if (repeated & PADLright) {
            if (++side->gauge > VERSUS_GAUGE_MAX) {
                side->gauge = VERSUS_GAUGE_MAX;
            }
        }
        if (repeated & PADLleft) {
            if (--side->gauge < 0) {
                side->gauge = 0;
            }
        }
    }
    if (gauge != side->gauge) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
    }
    if (pressed & PAD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        side->gaugePicked = 1;
        side->gaugeFlash = 0x7F;
        TitleChara_setAction(task->chara, 9);
        side->justPicked = 1;
    }
}

/* Loads TIM number index of the run tims into VRAM, its image at places[0] and its CLUT at places[1]
 * instead of the TIM's own rects. */
void uploadTimAt(u32 *tims, RECT *places, s32 index) {
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

/* What the character select knows of a character's tamers (CHARACTER_TAMERS, by character). */
typedef struct CharacterTamers {
    /* 0x00 */ s32 rookie;     /* its l/r TIM has the rookie's picture first */
    /* 0x04 */ s32 tamerPair;  /* its tamer picture is the wide one (two tamers) */
    /* 0x08 */ s32 tamerCount; /* 0 to 2: the tamer pictures in its TIM (2: picked with L1/R1) */
    /* 0x0C */ s32 voices[2][2]; /* by tamer: the first voice line and how many */
} CharacterTamers;

extern CharacterTamers CHARACTER_TAMERS[CHARACTER_COUNT];

/* the kinds of a side's pictures */
#define SIDE_PICTURE_ROOKIE 0
#define SIDE_PICTURE_CHARACTER 1
#define SIDE_PICTURE_TAMER 2
#define SIDE_PICTURE_WIDE_TAMER 3 /* 64 pixels wide */

/* by kind (SIDE_PICTURE_*) and side: where a picture's image and CLUT go in VRAM */
extern RECT SIDE_PICTURE_PLACES[4][FIGHTER_COUNT][2];

INCLUDE_ASM("title/nonmatchings/chara_select", uploadSidePictures);

/* where the run's name plate picture goes */
extern RECT NAME_PLATE_PLACE[2];

/* Uploads the pictures of the run's side from its TIM file as uploadSidePictures does for side 0, then the name
 * plate, the TIM after the tamer pictures. */
void uploadRunPictures(u32 *file, s32 character, s32 tamer) {
    s32 index;
    RECT *places;

    if (CHARACTER_TAMERS[character].rookie == 1) {
        places = SIDE_PICTURE_PLACES[SIDE_PICTURE_ROOKIE][0];
        uploadTimAt(file, places, 0);
        uploadTimAt(file, SIDE_PICTURE_PLACES[SIDE_PICTURE_CHARACTER][0], 1);
        index = 2;
    } else {
        uploadTimAt(file, SIDE_PICTURE_PLACES[SIDE_PICTURE_CHARACTER][0], 0);
        index = 1;
    }
    if (CHARACTER_TAMERS[character].tamerCount != 0) {
        if (CHARACTER_TAMERS[character].tamerPair == 1 || (character == CHARACTER_HOLYDRAMON && tamer == 0) ||
            (character == CHARACTER_TAILMON && tamer == 0) || (character == CHARACTER_STINGMON && tamer == 1) ||
            (character == CHARACTER_WORMMON && tamer == 1)) {
            places = SIDE_PICTURE_PLACES[SIDE_PICTURE_WIDE_TAMER][0];
        } else {
            places = SIDE_PICTURE_PLACES[SIDE_PICTURE_TAMER][0];
        }
        if (CHARACTER_TAMERS[character].tamerCount == 1 || tamer == 0) {
            uploadTimAt(file, places, index);
        } else {
            uploadTimAt(file, places, index + 1);
        }
    }
    index += CHARACTER_TAMERS[character].tamerCount;
    uploadTimAt(file, NAME_PLATE_PLACE, index);
}

/* Whether side's tamer picture is the 64-pixel-wide one (SIDE_PICTURE_WIDE_TAMER): tamer 0 of Holydramon and
 * Tailmon, tamer 1 of Stingmon and Wormmon. */
s32 CharacterSelectTask_isTamerPictureWide(CharacterSelectTask *task, s32 side) {
    if (((task->characters[side] == CHARACTER_HOLYDRAMON || task->characters[side] == CHARACTER_TAILMON) &&
         task->tamers[side] == 0) ||
        (task->characters[side] == CHARACTER_STINGMON && task->tamers[side] == 1) ||
        (task->characters[side] == CHARACTER_WORMMON && task->tamers[side] == 1)) {
        return 1;
    }
    return 0;
}

/* Picks one of the side's tamer's voice lines at random and loads it. */
void CharacterSelectTask_loadTamerVoice(CharacterSelectTask *task, s32 side, s32 tamer) {
    s32 line;
    s32 first;
    s32 count;

    line = mersenneTwisterGenerate(&RANDOM) & 0x1FF;
    if (tamer != 0) {
        count = CHARACTER_TAMERS[task->characters[side]].voices[1][1];
        if (count == 0) {
            return;
        }
        first = CHARACTER_TAMERS[task->characters[side]].voices[1][0];
    } else {
        count = CHARACTER_TAMERS[task->characters[side]].voices[0][1];
        if (count == 0) {
            return;
        }
        first = CHARACTER_TAMERS[task->characters[side]].voices[0][0];
    }
    line -= line / count * count; /* the remainder, written out (the game multiplies back, % gives mfhi) */
    /* the first lines of two of the paired tamers are two-line exchanges, stored as pairs */
    if (CHARACTER_TAMERS[task->characters[side]].tamerPair != 0) {
        if (task->characters[side] == CHARACTER_IMPERIALDRAMON_PALADIN_MODE) {
            if (line < 4) {
                line *= 2;
                task->voiceHasReply[side] = 1;
            } else {
                line += 4;
            }
        }
        if (task->characters[side] == CHARACTER_OMEGAMON) {
            if (line < 5) {
                line *= 2;
                task->voiceHasReply[side] = 1;
            } else {
                line += 5;
            }
        }
    }
    task->voices[side] = first + line;
    soundControlLoadSound(&SOUND_CONTROL, first + line, 1);
}

/* The character select's frame: runs the step, then draws and counts the frame; the end step stores the picks
 * and kills the task. The cases are in the order of their blocks in the asm. */
void CharacterSelectTask::tick(s32 arg) {
    switch (this->step) {
    case CHARACTER_SELECT_STEP_ENTER:
        CharacterSelectTask_stepEnter(this);
        break;
    case CHARACTER_SELECT_STEP_PICK:
        CharacterSelectTask_stepPick(this);
        break;
    case CHARACTER_SELECT_STEP_RUN_EXIT:
        CharacterSelectTask_stepRunExit(this);
        break;
    case CHARACTER_SELECT_STEP_RUN_LOAD:
        CharacterSelectTask_loadRunPictures(this);
        /* fall through */
    case CHARACTER_SELECT_STEP_RUN_SHOW:
        this->position.vy -= 8;
        if (this->position.vy < 0x104) {
            this->position.vy = 0x104;
        }
        this->position.vx = 200;
        this->rotation.vy = 0x880;
        TitleChara_setTransform(this->chara, &this->position, &this->rotation);
        CharacterSelectTask_waitConfirm(this);
        break;
    case CHARACTER_SELECT_STEP_ARENA_EXIT:
        CharacterSelectTask_stepArenaExit(this);
        break;
    case CHARACTER_SELECT_STEP_ARENA_LOAD:
        CharacterSelectTask_loadArenaPictures(this);
        break;
    case CHARACTER_SELECT_STEP_ARENA:
        CharacterSelectTask_stepArena(this);
        break;
    case CHARACTER_SELECT_STEP_FADE_SONGS:
        CharacterSelectTask_startFadeOut(this);
        break;
    case CHARACTER_SELECT_STEP_FADE_OUT:
        CharacterSelectTask_stepFadeOut(this);
        break;
    case CHARACTER_SELECT_STEP_END:
    default:
        CharacterSelectTask_storeArena(this);
        taskKill(this);
        return;
    }
    CharacterSelectTask_draw(this, arg);
    this->frame++;
}

/* The enter step: waits 32 frames, then goes to the pick with the host's lines from the first. */
void CharacterSelectTask_stepEnter(CharacterSelectTask *task) {
    if (++task->unk98 >= 32) {
        task->step = CHARACTER_SELECT_STEP_PICK;
        task->voiceStep = 0;
    }
}

/* The pick step: after the host's lines, a side backing out (not in a challenge) plays the back sound and goes to
 * the fade with nothing picked. Else each side's justPicked is cleared and its flashes count down by 0x10
 * to 0, and a versus pick goes on in CharacterSelectTask_pickVersus, any other in
 * CharacterSelectTask_pickCharacters. */
void CharacterSelectTask_stepPick(CharacterSelectTask *task) {
    s32 i;

    CharacterSelectTask_tickHostVoice(task);
    if (!GAME_STATE.challenge && CharacterSelectTask_isBackedOut(task)) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        task->selectFlags = 0;
        task->step = CHARACTER_SELECT_STEP_FADE_SONGS;
        return;
    }
    for (i = 0; i < FIGHTER_COUNT; i++) {
        task->sides[i].justPicked = 0;
        task->sides[i].pickFlash -= 0x10;
        if (task->sides[i].pickFlash < 0) {
            task->sides[i].pickFlash = 0;
        }
        task->sides[i].gaugeFlash -= 0x10;
        if (task->sides[i].gaugeFlash < 0) {
            task->sides[i].gaugeFlash = 0;
        }
    }
    if ((task->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) != SELECT_VERSUS) {
        CharacterSelectTask_pickCharacters(task);
    } else {
        CharacterSelectTask_pickVersus(task);
    }
}

/* The host's lines: plays sounds 0x23, 0x24 and 0x25 in turn, each handed to the model to speak (0x24 once
 * 0x23 ended and 90 frames passed, 0x25 likewise after 300); voice step 7 stops the sound if it still plays. */
void CharacterSelectTask_tickHostVoice(CharacterSelectTask *task) {
    if (--task->voiceWait < 0) {
        task->voiceWait = 0;
    }
    switch (task->voiceStep) {
    case 0:
        TitleChara_setAction(task->chara, TITLE_CHARA_TALK);
        task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x23, 0);
        TitleChara_setVoice(task->chara, task->sound);
        task->voiceStep++;
        break;
    case 1:
        if (!isSoundPlaying(task->sound)) {
            task->voiceWait = 90;
            task->voiceStep++;
        }
        break;
    case 2:
        if (!isSoundPlaying(task->sound) && task->voiceWait == 0) {
            TitleChara_setAction(task->chara, TITLE_CHARA_TALK);
            task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x24, 0);
            TitleChara_setVoice(task->chara, task->sound);
            task->voiceStep++;
        }
        break;
    case 3:
        if (!isSoundPlaying(task->sound)) {
            task->voiceWait = 300;
            task->voiceStep++;
        }
        break;
    case 4:
        if (!isSoundPlaying(task->sound) && task->voiceWait == 0) {
            TitleChara_setAction(task->chara, TITLE_CHARA_TALK);
            task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x25, 0);
            TitleChara_setVoice(task->chara, task->sound);
            task->voiceStep = 7;
        }
        break;
    case 7:
        if (isSoundPlaying(task->sound) == 1) {
            stopSound(task->sound);
        }
        task->voiceStep++;
        break;
    }
}

/* Whether a side backed out: cancel pressed on a side being picked that has not picked yet (either side
 * when both are being picked). Sets backedOut and returns it. */
s32 CharacterSelectTask_isBackedOut(CharacterSelectTask *task) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 cancel0 = padManagerGetPressed(pads, 0) & PAD_CANCEL;
    s32 cancel1 = padManagerGetPressed(pads, 1) & PAD_CANCEL;

    switch (task->selectFlags & (SELECT_SIDE_0 | SELECT_SIDE_1)) {
    case SELECT_SIDE_0:
        if (!task->sides[0].picked && cancel0) {
            task->backedOut = 1;
        }
        break;
    case SELECT_SIDE_1:
        if (!task->sides[1].picked && cancel1) {
            task->backedOut = 1;
        }
        break;
    case SELECT_SIDE_0 | SELECT_SIDE_1:
        if (!task->sides[0].picked && !task->sides[1].picked && (cancel0 | cancel1)) {
            task->backedOut = 1;
        }
        break;
    }
    return task->backedOut;
}

extern s32 D_8007B230;

/* A challenger joins by Start on the pad of a side no player is picking: side 0 joins on Agumon (Gabumon when
 * side 1 has Agumon), side 1 on V-mon (Wormmon when side 0 has V-mon), unless D_8007B230 is set. Returns
 * whether one joined. Nothing in the game calls it. */
s32 CharacterSelectTask_checkChallenger(CharacterSelectTask *task) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 start0 = padManagerGetPressed(pads, 0) & PADstart;
    s32 start1 = padManagerGetPressed(pads, 1) & PADstart;

    if ((task->selectFlags & (SELECT_SIDE_0 | SELECT_COMPUTER_0)) != SELECT_SIDE_0 && start0) {
        if (D_8007B230 == 0 && task->characters[1] == CHARACTER_AGUMON) {
            task->characters[0] = CHARACTER_GABUMON;
        } else {
            task->characters[0] = CHARACTER_AGUMON;
        }
        CharacterSelectTask_joinSide(task, task->characters[0], 0);
        return 1;
    }
    if ((task->selectFlags & (SELECT_SIDE_1 | SELECT_COMPUTER_1)) != SELECT_SIDE_1 && start1) {
        if (D_8007B230 == 0 && task->characters[0] == CHARACTER_V_MON) {
            task->characters[1] = CHARACTER_WORMMON;
        } else {
            task->characters[1] = CHARACTER_V_MON;
        }
        CharacterSelectTask_joinSide(task, task->characters[1], 1);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_joinSide);

/* The character picks (not versus): each side that picks moves its cursor and picks; a computer side picks
 * once the other side picked (side 1 also waits for the frame of that pick to pass). Once both sides picked and
 * their flashes faded, stores the picks and leaves for the arena pick (a single fight, or two players) or the
 * run's pictures. */
void CharacterSelectTask_pickCharacters(CharacterSelectTask *task) {
    if (task->selectFlags & SELECT_SIDE_0) {
        CharacterSelectTask_pickCharacter(&task->sides[0], task);
    } else if ((task->selectFlags & SELECT_COMPUTER_0) && task->sides[1].picked) {
        CharacterSelectTask_pickCharacter(&task->sides[0], task);
    }
    if (task->selectFlags & SELECT_SIDE_1) {
        CharacterSelectTask_pickCharacter(&task->sides[1], task);
    } else if ((task->selectFlags & SELECT_COMPUTER_1) && task->sides[0].picked && !task->sides[0].justPicked) {
        CharacterSelectTask_pickCharacter(&task->sides[1], task);
    }
    if (task->sides[0].picked && !task->sides[0].pickFlash && task->sides[1].picked && !task->sides[1].pickFlash) {
        CharacterSelectTask_storePicks(task);
        if (GAME_STATE.singleFight ||
            (task->selectFlags & (SELECT_SIDE_0 | SELECT_SIDE_1)) == (SELECT_SIDE_0 | SELECT_SIDE_1)) {
            task->step = CHARACTER_SELECT_STEP_ARENA_EXIT;
        } else {
            task->step = CHARACTER_SELECT_STEP_RUN_EXIT;
        }
    }
}

/* A side's versus pick: its character, then its gauge. */
static inline void pickCharacterOrGauge(CharacterSelectSide *side, CharacterSelectTask *task) {
    if (!side->picked) {
        CharacterSelectTask_pickCharacter(side, task);
    } else {
        CharacterSelectTask_pickGauge(side, task);
    }
}

/* A player's side's versus pick: as pickCharacterOrGauge, but its gauge waits while side 1 is the computer's and
 * the other side has its character picked. */
static inline void pickPlayerCharacterOrGauge(CharacterSelectSide *side, CharacterSelectSide *other,
                                              CharacterSelectTask *task) {
    if (!side->picked) {
        CharacterSelectTask_pickCharacter(side, task);
    } else if (!(task->selectFlags & SELECT_COMPUTER_1) || !other->picked) {
        CharacterSelectTask_pickGauge(side, task);
    }
}

/* The versus picks: each side picks its character, then its gauge (a player's side as pickPlayerCharacterOrGauge
 * says). A computer side picks once the other side picked its gauge (side 1 also waits for the frame of that pick
 * to pass). Once both gauges are picked and their flashes faded, stores the picks and leaves for the arena pick. */
void CharacterSelectTask_pickVersus(CharacterSelectTask *task) {
    if (task->selectFlags & SELECT_SIDE_0) {
        pickPlayerCharacterOrGauge(&task->sides[0], &task->sides[1], task);
    } else if ((task->selectFlags & SELECT_COMPUTER_0) && task->sides[1].gaugePicked) {
        pickCharacterOrGauge(&task->sides[0], task);
    }
    if (task->selectFlags & SELECT_SIDE_1) {
        pickPlayerCharacterOrGauge(&task->sides[1], &task->sides[0], task);
    } else if ((task->selectFlags & SELECT_COMPUTER_1) && task->sides[0].gaugePicked && !task->sides[0].justPicked) {
        pickCharacterOrGauge(&task->sides[1], task);
    }
    if (task->sides[0].gaugePicked && !task->sides[0].gaugeFlash && task->sides[1].gaugePicked &&
        !task->sides[1].gaugeFlash) {
        CharacterSelectTask_storePicks(task);
        task->step = CHARACTER_SELECT_STEP_ARENA_EXIT;
    }
}

INCLUDE_RODATA("title/nonmatchings/chara_select", SIDE_SELECT_MASKS);

/* by side: the selectFlags that pick its character, for a player or the computer */
extern s32 SIDE_SELECT_MASKS[FIGHTER_COUNT];

/* Stores each picked side's character and tamer in GAME_STATE, recolors a side when needed, and starts a
 * run for an unpicked side (it gets the run's character). */
void CharacterSelectTask_storePicks(CharacterSelectTask *task) {
    s32 unpicked = -1;
    s32 side;
    s32 character;
    s32 tamer;

    for (side = 0; side < FIGHTER_COUNT; side++) {
        if (!(task->selectFlags & SIDE_SELECT_MASKS[side])) {
            unpicked = side;
            continue;
        }
        character = GRID_CELLS[task->sides[side].cell].character;
        if (CHARACTER_TAMERS[character].tamerCount == 2) {
            tamer = task->sides[side].tamer;
        } else {
            tamer = 0;
        }
        task->characters[side] = character;
        task->tamers[side] = tamer;
        GAME_STATE.fighters[side].character = character;
        GAME_STATE.fighters[side].tamer = tamer;
    }
    if (GAME_STATE.singleFight) {
        GAME_STATE.lastCharacters[0] = task->characters[0];
        GAME_STATE.lastCharacters[1] = task->characters[1];
        GAME_STATE.fighters[0].altColor = 0;
        gameStateSetAltColor(&GAME_STATE, 1);
    } else if (GAME_STATE.challenge) {
        /* The match depends on the test in a local: a single-bit test in the call is shifted down. */
        s32 side1Picked = task->selectFlags & SELECT_SIDE_1;

        gameStateSetAltColor(&GAME_STATE, side1Picked != 0);
    }
    if (!GAME_STATE.challenge && unpicked >= 0) {
        gameStateStartRun(&GAME_STATE);
        task->characters[unpicked] = GAME_STATE.fighters[unpicked].character;
        GAME_STATE.fighters[unpicked].tamer = 0;
        task->tamers[unpicked] = 0;
        gameStateSetAltColor(&GAME_STATE, unpicked ^ 1);
    }
}

/* The run's exit: moves the model up by 4 and turns it about y; once exitTime reaches 0x380 (8 a call),
 * goes to load the run's pictures with the model back at height 500. */
void CharacterSelectTask_stepRunExit(CharacterSelectTask *task) {
    task->position.vy -= 4;
    task->rotation.vy += 0x30;
    TitleChara_setTransform(task->chara, &task->position, &task->rotation);
    task->exitTime += 8;
    if (task->exitTime >= 0x380) {
        task->step = CHARACTER_SELECT_STEP_RUN_LOAD;
        task->position.vy = 500;
    }
}

INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_MAPSEL_TIM);
INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_LEFT_PICTURES_TIM);

extern char PATH_MAPSEL_TIM[];
extern char PATH_LEFT_PICTURES_TIM[]; /* a format: the picked character's pictures file */

/* Loads the run's pictures: unloads sound 0x17 and the host's first line (0x23), uploads MAPSEL.TIM, then the
 * picked side's pictures file (side 1's when it picks, else side 2's) and its tamer's voice; goes to the show
 * step. */
void CharacterSelectTask_loadRunPictures(CharacterSelectTask *task) {
    char path[64];
    u32 *file;

    soundControlUnloadSound(&SOUND_CONTROL, 0x17);
    soundControlUnloadSound(&SOUND_CONTROL, 0x23);
    file = (u32 *)loadFile(PATH_MAPSEL_TIM);
    uploadTims(file);
    mainHeapFree(file);
    if (task->selectFlags & SELECT_SIDE_0) {
        sprintf(path, PATH_LEFT_PICTURES_TIM, task->characters[0]);
        file = (u32 *)loadFile(path);
        uploadRunPictures(file, task->characters[0], task->tamers[0]);
        mainHeapFree(file);
        CharacterSelectTask_loadTamerVoice(task, 0, task->tamers[0]);
    } else {
        sprintf(path, PATH_LEFT_PICTURES_TIM, task->characters[1]);
        file = (u32 *)loadFile(path);
        uploadRunPictures(file, task->characters[1], task->tamers[1]);
        mainHeapFree(file);
        CharacterSelectTask_loadTamerVoice(task, 1, task->tamers[1]);
    }
    task->step = CHARACTER_SELECT_STEP_RUN_SHOW;
}


/* The end of the run's pictures steps: the confirm button on any pad goes to the fade. */
void CharacterSelectTask_waitConfirm(CharacterSelectTask *task) {
    if (padManagerGetPressed(PAD_MANAGER_INSTANCE, -1) & PAD_CONFIRM) {
        task->step = CHARACTER_SELECT_STEP_FADE_SONGS;
    }
}

/* The arena's exit: moves the model up by 4 and turns it about y; once exitTime reaches 0x380 (8 a call),
 * goes to load the arena pick's pictures. */
void CharacterSelectTask_stepArenaExit(CharacterSelectTask *task) {
    task->position.vy -= 4;
    task->rotation.vy += 0x30;
    TitleChara_setTransform(task->chara, &task->position, &task->rotation);
    task->exitTime += 8;
    if (task->exitTime >= 0x380) {
        task->step = CHARACTER_SELECT_STEP_ARENA_LOAD;
    }
}

/* Loads the arena pick's pictures: unloads sound 0x17 and the host's first line (0x23), uploads MAPSEL.TIM,
 * then the left side's pictures file and the right side's, each with its tamer's voice; goes to the arena step
 * and loads sound 0x27. */
void CharacterSelectTask_loadArenaPictures(CharacterSelectTask *task) {
    char path[64];
    u32 *file;

    soundControlUnloadSound(&SOUND_CONTROL, 0x17);
    soundControlUnloadSound(&SOUND_CONTROL, 0x23);
    file = (u32 *)loadFile(PATH_MAPSEL_TIM);
    uploadTims(file);
    mainHeapFree(file);
    task->voiceHasReply[0] = 0;
    task->voiceHasReply[1] = 0;
    sprintf(path, PATH_LEFT_PICTURES_TIM, task->characters[0]);
    file = (u32 *)loadFile(path);
    uploadSidePictures(file, task->characters[0], task->tamers[0], 0);
    mainHeapFree(file);
    CharacterSelectTask_loadTamerVoice(task, 0, task->tamers[0]);
    sprintf(path, "title/map/r%02d.tim", task->characters[1]);
    file = (u32 *)loadFile(path);
    uploadSidePictures(file, task->characters[1], task->tamers[1], 1);
    mainHeapFree(file);
    CharacterSelectTask_loadTamerVoice(task, 1, task->tamers[1]);
    task->step = CHARACTER_SELECT_STEP_ARENA;
    task->voiceWait = 0;
    task->voiceStep = 0;
    soundControlLoadSound(&SOUND_CONTROL, 0x27, 1);
}

/* The arena step: picks the arena; back at the pick step, clears exitTime. */
void CharacterSelectTask_stepArena(CharacterSelectTask *task) {
    CharacterSelectTask_pickArena(task);
    if (task->step == CHARACTER_SELECT_STEP_PICK) {
        task->exitTime = 0;
    }
}

/* Starts fading the songs out (step 0x44) and goes to the fade out. */
void CharacterSelectTask_startFadeOut(CharacterSelectTask *task) {
    fadeSongs(0x44);
    task->step = CHARACTER_SELECT_STEP_FADE_OUT;
}

/* The fade out: by 4 a frame, then the end step; 0x98 follows as (0xFF - fade) / 8. */
void CharacterSelectTask_stepFadeOut(CharacterSelectTask *task) {
    task->fade += 4;
    if (task->fade >= 0x100) {
        task->fade = 0xFF;
        task->step = CHARACTER_SELECT_STEP_END;
    }
    task->unk98 = (0xFF - task->fade) >> 3;
}

/* The end of the scene: stores what was picked in GAME_STATE (the arena of a versus fight or a bonus game, and
 * each side's gauge after a versus pick). The match depends on the arena read into a local and stored to
 * GAME_STATE.arena first: an assignment chain stores right to left, and the bonus game reads the field again. */
void CharacterSelectTask_storeArena(CharacterSelectTask *task) {
    if (task->selectFlags & SELECT_VERSUS) {
        s32 arena = task->arena;

        GAME_STATE.arena = arena;
        GAME_STATE.lastVersusArena = arena;
    }
    if (task->selectFlags & SELECT_BONUS) {
        s32 arena = task->arena;

        GAME_STATE.arena = arena;
        GAME_STATE.lastBonusArena = arena;
        GAME_STATE.bonusGame = task->arena - ARENA_BONUS;
    }
    if ((task->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) == SELECT_VERSUS) {
        GAME_STATE.fighters[0].versusGauge = task->sides[0].gauge;
        GAME_STATE.fighters[1].versusGauge = task->sides[1].gauge;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_draw);

/* The place of cell `cell` of the character grid, from (x, y): row 1 holds cells 0-5 with a gap after the third,
 * row 2 cells 6-14, row 3 the rest, 30 pixels apart. */
void getCellPosition(DVECTOR *pos, s32 x, s32 y, s32 cell) {
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

/* Draws the grid's backdrop, drawn behind its cells: a 288x116 picture at (x, y) in three SPRTs from the 8-bit
 * pages at (512, 0), (576, 0) and (640, 0), row 0x78: 60 columns from u 0x44, then 128 and 100 from u 0. */
void drawGridBackdrop(u_long *ot, s32 x, s32 y) {
    DR_TPAGE *tpage;
    SPRT *sprt;

    tpage = allocDrTpage(getTPage(1, 0, 512, 0));
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setXY0(sprt, x, y);
    setUV0(sprt, 0x44, 0x78);
    setWH(sprt, 0x3C, 0x74);
    sprt->clut = 0x3DD8;
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);

    tpage = allocDrTpage(getTPage(1, 0, 576, 0));
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setXY0(sprt, x + 0x3C, y);
    setUV0(sprt, 0, 0x78);
    setWH(sprt, 0x80, 0x74);
    sprt->clut = 0x3DD8;
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);

    tpage = allocDrTpage(getTPage(1, 0, 640, 0));
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setXY0(sprt, x + 0xBC, y);
    setUV0(sprt, 0, 0x78);
    setWH(sprt, 0x64, 0x74);
    sprt->clut = 0x3DD8;
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

/* Draws the grid's scrolling strip: a 112x16 strip (v 0xEC of the page at (640, 256), semi-transparent) moving
 * left with frame (a pixel a frame from x + 0xBD, wrapping every 256 frames), clipped to the 160x16 window at
 * (x + 0x40, y + 0x14); the draw area is set back to the whole buffer after it. */
void drawScrollingStrip(u_long *ot, s32 x, s32 y, s32 frame) {
    /* The match depends on the scroll in its own variable (one expression folds to x + 0xBD - frame) and on the
     * order of the SPRT's fields below; frame is an s32 cut to a byte here, as drawGrid passes it unmasked. */
    s32 scroll = (u8)frame - 0xBD;
    s32 left = x - scroll;
    RECT area;
    RECT window;
    DR_TPAGE *tpage;
    SPRT *sprt;

    getDrawBufferRect(&area);
    AddPrim(ot, allocDrArea(&area));
    tpage = allocDrTpage(getTPage(0, 1, 640, 0));
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setXY0(sprt, left, y + 0x14);
    setUV0(sprt, 0, 0xEC);
    sprt->clut = 0x3D72;
    setWH(sprt, 0x70, 0x10);
    setSemiTrans(sprt, 1);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    getDrawBufferSubRect(&window, x + 0x40, y + 0x14, 0xA0, 0x10);
    if (window.h > 0) {
        AddPrim(ot, allocDrArea(&window));
    }
}

/* The select screen's drawSideStrips: three strips of 16 x 0x7C texels a side (page 0xA), scrolling with frame
 * (down on the right at x 0x138, up on the left at x 8) and opening to their full width over the first 8 frames,
 * then the two dark bars beside them. */
void drawSelectSideStrips(u_long *ot, s32 frame) {
    s32 halfWidth = frame;
    s32 i;
    POLY_F4 *bar;

    if (halfWidth > 8) {
        halfWidth = 8;
    }
    for (i = 0; i < 3; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 0x80 + (frame & 0x7F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        setXYWH(poly, 0x138 - halfWidth, y - 0x80, halfWidth * 2, 0x80);
        setUV4(poly, 0xC8, 0x78, 0xD8, 0x78, 0xC8, 0xF4, 0xD8, 0xF4);
        poly->tpage = 0xA;
        poly->clut = 0x3C72;
        SetSemiTrans(poly, 0);
        AddPrim(ot, poly);
    }
    for (i = 0; i < 3; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 0x80 - (frame & 0x7F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        setXYWH(poly, 8 - halfWidth, y, halfWidth * 2, 0x80);
        setUV4(poly, 0xD8, 0x78, 0xE8, 0x78, 0xD8, 0xF4, 0xE8, 0xF4);
        poly->tpage = 0xA;
        poly->clut = 0x3CB2;
        SetSemiTrans(poly, 0);
        AddPrim(ot, poly);
    }
    bar = allocPolyF4();
    setRGB0(bar, 0, 0, 0);
    setXY4(bar, 2, 0, 14, 0, 2, 0xFF, 14, 0xFF);
    SetSemiTrans(bar, 1);
    AddPrim(ot, bar);
    bar = allocPolyF4();
    setRGB0(bar, 0, 0, 0);
    setXY4(bar, 0x132, 0, 0x13E, 0, 0x132, 0xFF, 0x13E, 0xFF);
    SetSemiTrans(bar, 1);
    AddPrim(ot, bar);
    AddPrim(ot, allocDrTpage(0x15));
}

/* Draws grid entry index's portrait (108 x 120, an 8-bit picture: GRID_CELLS' picture 0-15 on the pages
 * from x 320 at y 256, 16 and up from x 384 at y 0, eight to a row, two rows) at (x, y). side is unused. */
void drawCharacterPortrait(u_long *ot, s32 x, s32 y, s32 index, s32 side) {
    /* The match depends on picture being an s16 (lh; an s32 reads it with lhu). */
    s16 picture = GRID_CELLS[index].picture;
    SPRT *sprt = allocSprt();
    DR_TPAGE *tpage;
    s32 column; /* the portrait's place in its row */
    s32 v;

    setXY0(sprt, x, y);
    if (picture < 8) {
        column = picture;
        tpage = allocDrTpage(getTPage(1, 0, 320 + (column * 54 & 0x3C0), 256));
        v = 0;
    } else if (picture < 16) {
        column = picture - 8;
        tpage = allocDrTpage(getTPage(1, 0, 320 + (column * 54 & 0x3C0), 256));
        v = 120;
    } else if (picture < 21) {
        column = picture - 16;
        tpage = allocDrTpage(getTPage(1, 0, 384 + (column * 54 & 0x3C0), 0));
        v = 0;
    } else {
        column = picture - 21;
        tpage = allocDrTpage(getTPage(1, 0, 384 + (column * 54 & 0x3C0), 0));
        v = 120;
    }
    sprt->u0 = column * 108 & 0x7F;
    sprt->v0 = v;
    if (picture < 16) {
        sprt->clut = getClut(320, 496 + picture);
    } else {
        sprt->clut = getClut(384, 232 + picture);
    }
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setWH(sprt, 108, 120);
    setSemiTrans(sprt, 0);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006E1A0);

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006E368);

/* Draws entry index's picture (128 x 24, from GRID_CELLS) sliding in from the left with frame, then
 * func_8006E1A0 at its x and drawCharacterPortrait for the entry. */
void drawLeftCellInfo(u_long *ot, s32 index, s32 frame) {
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 picture;
    s32 x;
    s32 clutX;
    s32 right; /* the picture sits in the right half of its page */

    /* The match depends on the halfword read as u16 and cast to s16 into an s32 (lhu and two shifts, and an
     * SImode % 10), and below on the u flag kept in a local (as an expression, fold turns it into a branch). */
    picture = (s16)GRID_CELLS[index].picture;
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
    drawCharacterPortrait(ot, x - 0x6C, 0xC, index, 0);
}

/* As drawLeftCellInfo from the right: entry index's picture slides in from the right edge (0x140) with frame,
 * then func_8006E368 at its x and drawCharacterPortrait for the entry. */
void drawRightCellInfo(u_long *ot, s32 index, s32 frame) {
    DR_TPAGE *tpage;
    SPRT *sprt;
    s32 picture;
    s32 slide;
    s32 x;
    s32 clutX;
    s32 right; /* the picture sits in the right half of its page */

    /* The match depends on the halfword read as u16 and cast to s16 into an s32, and on the u flag kept in a
     * local, as in drawLeftCellInfo. */
    picture = (s16)GRID_CELLS[index].picture;
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
    drawCharacterPortrait(ot, 0x140 - slide, 0x21, index, 1);
}

/* Draws an 8-step gauge at pos: (level + 1) orange steps of 11, filled from the right, over a black
 * 0x58 x 13 bar. */
void drawGaugeFromRight(u_long *ot, DVECTOR pos, s32 level) {
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
void drawGaugeFromLeft(u_long *ot, DVECTOR pos, s32 level) {
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

/* Draws the character grid at (x, y) with the sides' cursors on cell0 and cell1 (-1 for none; sprite 3 for a
 * computer's cursor, else 0 and 1, lightened by the pick's flash), the cover of each cell not pickable, the
 * backdrop, the strips and the title; in a versus pick the gauge panels slide down while their side picks (out
 * once it has picked) with each side's gauge; then the info of each cursor's cell while it is pickable. */
void CharacterSelectTask_drawGrid(CharacterSelectTask *task, u_long *ot, s32 x, s32 y, s32 cell0, s32 cell1,
                                  s32 frame) {
    DVECTOR pos;
    s32 cell;

    drawSelectSideStrips(ot, frame);
    if (cell0 >= 0) {
        getCellPosition(&pos, x, y, cell0);
        if (task->sides[0].computer) {
            drawSelectSpriteShaded(ot, &pos, 3, task->sides[0].pickFlash + 0x80, 0);
        } else {
            drawSelectSpriteShaded(ot, &pos, 0, task->sides[0].pickFlash + 0x80, 0);
        }
    }
    if (cell1 >= 0) {
        getCellPosition(&pos, x, y, cell1);
        if (task->sides[1].computer) {
            drawSelectSpriteShaded(ot, &pos, 3, task->sides[1].pickFlash + 0x80, 0);
        } else {
            drawSelectSpriteShaded(ot, &pos, 1, task->sides[1].pickFlash + 0x80, 0);
        }
    }
    for (cell = 0; cell < CHARACTER_COUNT; cell++) {
        if (task->pickable[cell] == 0) {
            getCellPosition(&pos, x, y, cell);
            drawSelectSprite(ot, &pos, 2, 0);
        }
    }
    drawGridBackdrop(ot, x, y);
    drawScrollingStrip(ot, x, y, frame);
    pos.vx = x + 100;
    pos.vy = y + 14;
    drawSelectSprite(ot, &pos, 4, 0);
    if ((task->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) == SELECT_VERSUS) {
        if (task->sides[0].picked) {
            if (--task->sides[0].gaugeSlide < 0) {
                task->sides[0].gaugeSlide = 0;
            }
        } else if (++task->sides[0].gaugeSlide >= 16) {
            task->sides[0].gaugeSlide = 15;
        }
        pos.vx = x + 7;
        /* the match depends on the shift: `* 4` loads gaugeSlide whole (lw), the game reads its low half */
        pos.vy = y - 25 + (task->sides[0].gaugeSlide << 2);
        drawSelectSpriteShaded(ot, &pos, 5, task->sides[0].gaugeFlash + 0x80, 0);
        drawGaugeFromRight(ot, pos, task->sides[0].gauge);
        if (task->sides[1].picked) {
            if (--task->sides[1].gaugeSlide < 0) {
                task->sides[1].gaugeSlide = 0;
            }
        } else if (++task->sides[1].gaugeSlide >= 16) {
            task->sides[1].gaugeSlide = 15;
        }
        pos.vx = x + 0xB9;
        pos.vy = y - 25 + (task->sides[1].gaugeSlide << 2);
        drawSelectSpriteShaded(ot, &pos, 6, task->sides[1].gaugeFlash + 0x80, 0);
        drawGaugeFromLeft(ot, pos, task->sides[1].gauge);
    }
    /* pickable[-1] is read before the test, as the game does */
    if (task->pickable[cell0] == 1 && cell0 >= 0) {
        drawLeftCellInfo(ot, cell0, ++task->sides[0].cellFrame);
    }
    if (task->pickable[cell1] == 1 && cell1 >= 0) {
        drawRightCellInfo(ot, cell1, ++task->sides[1].cellFrame);
    }
}

/* Draws the select screen's background: five 64 by 240 strips across the screen, strip i from the 4-bit
 * texels at VRAM x 704 + i * 16 (u i * 64 of its page), clut 0x3C32, in neutral gray. task is unused. */
void drawSelectBackground(CharacterSelectTask *task, u_long *ot) {
    s32 i;

    for (i = 0; i < 5; i++) {
        DR_TPAGE *tpage = allocDrTpage(getTPage(0, 0, 704 + i * 16, 0));
        SPRT *sprt = allocSprt();

        setRGB0(sprt, 0x80, 0x80, 0x80);
        setXY0(sprt, i * 64, 0);
        setUV0(sprt, i * 64, 0);
        sprt->clut = 0x3C32;
        setWH(sprt, 64, 240);
        AddPrim(ot, sprt);
        AddPrim(ot, tpage);
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006EF94);

/* Draws sprites 7, 14 (when flag is set) and 16 of the title's table at (x, y) in ot. */
void func_8006F274(u_long *ot, s32 x, s32 y, s32 flag) {
    DVECTOR pos;

    pos.vx = x;
    pos.vy = y;
    drawSelectSprite(ot, &pos, 7, 0);
    if (flag) {
        pos.vx = x + 0x3A;
        pos.vy = y + 0x30;
        drawSelectSprite(ot, &pos, 14, 0);
    }
    pos.vx = x + 0xA;
    pos.vy = y + 8;
    drawSelectSprite(ot, &pos, 16, 0);
}

/* Draws sprites 8, 15 (when flag is set) and 17 of the title's table at (x, y) in ot. */
void func_8006F31C(u_long *ot, s32 x, s32 y, s32 flag) {
    DVECTOR pos;

    pos.vx = x;
    pos.vy = y;
    drawSelectSprite(ot, &pos, 8, 0);
    if (flag) {
        pos.vx = x + 0xF;
        pos.vy = y + 0x30;
        drawSelectSprite(ot, &pos, 15, 0);
    }
    pos.vx = x + 0xF;
    pos.vy = y + 8;
    drawSelectSprite(ot, &pos, 17, 0);
}

/* An arena's picture in VRAM: its place in the texture page, its palette and the page. */
typedef struct SelectSprite {
    /* 0x0 */ u8 u;
    /* 0x1 */ u8 v;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u16 clut;
    /* 0x6 */ u16 tpage;
} SelectSprite;

/* by arena */
extern SelectSprite ARENA_PICTURES[];

/* The arena pick's animation (ARENA_STEP_*) and drawing. Up to the confirmation it slides the pick in and
 * opens the picture of the arena (ARENA_PICTURES); then a picture zooms out and the screen fades to black,
 * and the scene goes to its fade. The pick's sprites take rect's x and y as their position: the game keeps
 * one stack slot for both. The match depends on pictureHalfHeight being a plain int (through the s32 typedef,
 * narrowing the picture's bottom y to a short reassociates it as (y + 0x2B) - h), on one variable for the
 * scale and on the translation being set before the scale words. */
void CharacterSelectTask_drawArena(CharacterSelectTask *task, u_long *ot) {
    switch (task->arenaStep) {
    case ARENA_STEP_INIT:
        task->arenaSlide = 0;
        task->arenaY = 0x72;
        task->pictureHalfHeight = 0;
        task->unkB8 = 0;
        task->arenaFrame = 0;
        break;
    case ARENA_STEP_SLIDE_IN:
        task->arenaSlide += 8;
        if (task->arenaSlide > 0xA0) {
            task->arenaSlide = 0xA0;
            task->arenaStep++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        task->unkB8++;
        break;
    case ARENA_STEP_OPEN_PICTURE:
        task->pictureHalfHeight += 2;
        if (task->pictureHalfHeight > 0x20) {
            task->pictureHalfHeight = 0x20;
            task->arenaStep++;
        }
        break;
    case ARENA_STEP_CONFIRMED:
        task->arenaY += 8;
        if (task->arenaY > 0x100) {
            task->arenaY = 0x100;
        }
        if (--task->unkB8 < 0) {
            task->unkB8 = 0;
        }
        if (task->arenaY == 0x100 && task->unkB8 == 0) {
            task->arenaStep++;
        }
        /* fall through */
    case ARENA_STEP_PICK:
        task->arenaFrame++;
        break;
    case ARENA_STEP_ZOOM_INIT:
        task->zoomScale = 0x4000;
        task->zoomAngle = 0;
        task->blackLevel = 0;
        task->zoomFrames = 0;
        task->zoomBrightness = 0;
        task->arenaStep++;
        /* fall through */
    case ARENA_STEP_ZOOM_OUT:
        task->zoomScale -= 0x200;
        if (task->zoomScale < 0x1000) {
            task->zoomScale = 0x1000;
            task->zoomBrightness = 0x7F;
            task->arenaStep++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        break;
    case ARENA_STEP_FADE_PICTURE:
        task->zoomBrightness -= 2;
        if (task->zoomBrightness < 0) {
            task->zoomBrightness = 0;
        }
        if (task->zoomFrames++ > 60) {
            task->arenaStep++;
        }
        break;
    case ARENA_STEP_BLACK_OUT:
        task->blackLevel += 8;
        if (task->blackLevel >= 0xFF) {
            task->blackLevel = 0xFF;
            task->arenaStep++;
        }
        task->zoomScale += 0x200;
        task->zoomAngle += 0x80;
        break;
    case ARENA_STEP_END:
        task->step = CHARACTER_SELECT_STEP_FADE_SONGS;
        break;
    }
    if (task->arenaStep >= ARENA_STEP_ZOOM_INIT) {
        if (task->blackLevel != 0) {
            DR_TPAGE *tpage = allocDrTpage(0x25);
            TILE *tile = allocTile();

            setRGB0(tile, task->blackLevel, task->blackLevel, task->blackLevel);
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
            u32 scale = (u16)task->zoomScale;
            POLY_FT4 *poly;

            m.m.t[0] = 0xA0;
            m.m.t[1] = 0xA0;
            m.m.t[2] = 0;
            m.words[0] = scale;
            m.words[1] = 0;
            m.words[2] = scale;
            m.words[3] = 0;
            m.words[4] = scale;
            matrixRotateZ(&m.m, task->zoomAngle);
            matrixTransformVecs(&m.m, screen, corners, 4);
            poly = allocPolyFT4();
            setXY4(poly, screen[0].vx, screen[0].vy, screen[1].vx, screen[1].vy, screen[2].vx, screen[2].vy,
                   screen[3].vx, screen[3].vy);
            setUV4(poly, 0, 0, 0x97, 0, 0, 0x4F, 0x97, 0x4F);
            setRGB0(poly, task->zoomBrightness + 0x80, task->zoomBrightness + 0x80, task->zoomBrightness + 0x80);
            poly->tpage = 0x88;
            poly->clut = 0x3D98;
            setSemiTrans(poly, 1);
            AddPrim(ot, poly);
        }
    } else {
        RECT rect;

        rect.x = task->arenaSlide - 0x50;
        rect.y = task->arenaY;
        drawSelectSprite(ot, (DVECTOR *)&rect, 9, 0);
        rect.x = 320 - task->arenaSlide;
        drawSelectSprite(ot, (DVECTOR *)&rect, 10, 0);
        if (task->pictureHalfHeight > 0) {
            SelectSprite *picture = &ARENA_PICTURES[task->arena];
            POLY_FT4 *poly = allocPolyFT4();

            setRGB0(poly, 0x80, 0x80, 0x80);
            setXYWH(poly, 0x78, task->arenaY + 0x2B - task->pictureHalfHeight, 0x50, task->pictureHalfHeight * 2);
            setUVWH(poly, picture->u, picture->v, 0x4F, 0x3F);
            poly->tpage = picture->tpage;
            poly->clut = picture->clut;
            SetSemiTrans(poly, 0);
            AddPrim(ot, poly);
        }
        getDrawBufferRect(&rect);
        AddPrim(ot, allocDrArea(&rect));
        if (task->arenaSlide == 0xA0) {
            rect.x = 0xE8 - (u8)task->arenaFrame;
            rect.y = task->arenaY + 0x5A;
            drawSelectSprite(ot, (DVECTOR *)&rect, 13, 0);
        }
        getDrawBufferSubRect(&rect, 0x5A, task->arenaY + 0x5A, 0x8C, 0x12);
        if (rect.h > 0) {
            AddPrim(ot, allocDrArea(&rect));
        }
        rect.x = task->arenaSlide - 0x48;
        rect.y = task->arenaY + 0x5A;
        drawSelectSpriteShaded(ot, (DVECTOR *)&rect, 11, 0x80, 0);
        rect.x = 320 - task->arenaSlide;
        drawSelectSpriteShaded(ot, (DVECTOR *)&rect, 12, 0x80, 0);
    }
}

/* The arena pick, while its step is ARENA_STEP_PICK: plays sounds 0x27, 0x28 and 0x29 in turn (0x29 again
 * every 300 frames), left and right step through the arenas (a fight's 0 to 5, the boss's too once a run was
 * won; with SELECT_BONUS the bonus games, skipping those not unlocked yet) and the confirm
 * button confirms it.
 * The match depends on save.bonusGamesUnlocked being a u32 (with an s32 mask, fold turns the bit test into a
 * shift of the mask) and on each loop having its own counter. */
void CharacterSelectTask_pickArena(CharacterSelectTask *task) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    u32 pressed = padManagerGetPressed(pads, -1);
    u32 repeated = padManagerGetRepeated(pads, -1);
    s32 first;
    s32 last;

    if (task->arenaStep != ARENA_STEP_PICK) {
        return;
    }
    if (--task->voiceWait < 0) {
        task->voiceWait = 0;
    }
    task->voiceWait--;
    switch (task->voiceStep) {
    case 0:
        task->voiceWait = 60;
        break;
    case 1:
        if (task->voiceWait == 0) {
            task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x27, 0);
            task->voiceStep++;
        }
        break;
    case 2:
        if (!isSoundPlaying(task->sound)) {
            task->voiceWait = 90;
            task->voiceStep++;
        }
        break;
    case 3:
        if (task->voiceWait == 0) {
            task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x28, 0);
            task->voiceStep++;
        }
        break;
    case 4:
        if (!isSoundPlaying(task->sound)) {
            task->voiceWait = 300;
            task->voiceStep++;
        }
        break;
    case 5:
        if (task->voiceWait == 0) {
            task->sound = soundControlPlaySound(&SOUND_CONTROL, 0x29, 0);
            task->voiceStep = 4;
        }
        break;
    case 7:
        if (isSoundPlaying(task->sound) == 1) {
            stopSound(task->sound);
        }
        task->voiceStep++;
        break;
    }
    first = 0;
    last = ARENA_PICK_COUNT - 1;
    if (GAME_STATE.save.cleared & 0xFFF) {
        last = ARENA_BOSS;
    }
    if (task->selectFlags & SELECT_BONUS) {
        first = ARENA_BONUS;
        last = ARENA_BONUS + BONUS_GAME_COUNT - 1;
    }
    if (repeated & (PAD_LEFT | PAD_RIGHT)) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
    }
    if (!(task->selectFlags & SELECT_BONUS)) {
        if (repeated & PAD_LEFT) {
            if (--task->arena < first) {
                task->arena = last;
            }
        }
        if (repeated & PAD_RIGHT) {
            if (++task->arena > last) {
                task->arena = first;
            }
        }
    } else {
        if (repeated & PAD_LEFT) {
            s32 i;

            for (i = 0; i < BONUS_GAME_COUNT; i++) {
                if (--task->arena < first) {
                    task->arena = last;
                }
                if (GAME_STATE.save.bonusGamesUnlocked & ARENA_BIT(task->arena)) {
                    break;
                }
            }
        }
        if (repeated & PAD_RIGHT) {
            s32 i;

            for (i = 0; i < BONUS_GAME_COUNT; i++) {
                if (++task->arena > last) {
                    task->arena = first;
                }
                if (GAME_STATE.save.bonusGamesUnlocked & ARENA_BIT(task->arena)) {
                    break;
                }
            }
        }
    }
    if (pressed & PAD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        task->arenaStep = ARENA_STEP_CONFIRMED;
    }
}

/* An object with a model at 0x84; nothing references func_8006FE5C. */
typedef struct Unk8006FE5C {
    /* 0x00 */ u8 unk0[0x84];
    /* 0x84 */ Model *model;
} Unk8006FE5C;

/* Animates the model at 0x84 and draws it in camera. */
void func_8006FE5C(Unk8006FE5C *object, Camera *camera) {
    Model *model = object->model;

    modelUpdateAnimation(model);
    SCENE_OBJECT_INVALIDATE(&model->group.object);
    sceneObjectDrawInView(&model->group.object, camera);
}

INCLUDE_ASM("title/nonmatchings/chara_select", drawSelectSprite);

INCLUDE_ASM("title/nonmatchings/chara_select", drawSelectSpriteShaded);

/* The sum 0 + 1 + ... + count - 1. */
u32 getTriangularSum(s32 count) {
    s32 i;
    u32 sum = 0;

    for (i = 0; i < count; i++) {
        sum += i;
    }
    return sum;
}

/* Update: keeps arg and runs the task's tick. */
void CharacterSelectTask::update(s32 arg) {
    this->updateArg = arg;
    tick(arg);
}

INCLUDE_RODATA("title/nonmatchings/chara_select", _vt.19CharacterSelectTask);
INCLUDE_RODATA("title/nonmatchings/chara_select", GRID_CELLS);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064C44);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_CELLS);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_TAMERS);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064E88);
INCLUDE_RODATA("title/nonmatchings/chara_select", SIDE_PICTURE_PLACES);
INCLUDE_RODATA("title/nonmatchings/chara_select", NAME_PLATE_PLACE);
INCLUDE_RODATA("title/nonmatchings/chara_select", SELECT_SPRITES);
INCLUDE_RODATA("title/nonmatchings/chara_select", ARENA_PICTURES);
