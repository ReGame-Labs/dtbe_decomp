/* The character and stage selection (the task "Character Select", charasel.tim and mapsel.tim). */

#include "common.h"
#include <stdio.h>
#include "engine/cd/file_load.h"
#include "engine/debug/system_menu.h"
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
#include "engine/system/memory.h"
#include "engine/task/entity.h"
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

/* A cell of the character grid (GRID_CELLS, 20 bytes, by cell: rows 0-5 the specials, 6-14 the rookies,
 * 15-23 their evolutions): picture picks its picture (its texture page, u, v and palette). The asm splits
 * the table at D_80064C44, inside entry 0. */
typedef struct GridCell {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ s16 unk2;
    /* 0x04 */ s32 character;
    /* 0x08 */ s32 evolution; /* a rookie's evolution; any other character itself */
    /* 0x0C */ s32 unkC;
    /* 0x10 */ u16 picture;
    /* 0x12 */ s16 unk12;
} GridCell;

extern GridCell GRID_CELLS[];

/* A side's part of CharacterSelectTask (0x38 bytes, sides at 0xC0 and 0xF8). */
typedef struct CharacterSelectSide {
    /* 0x00 */ s32 picked; /* its character is confirmed; set when the side has nothing to pick */
    /* 0x04 */ s32 gaugePicked; /* its versus gauge is confirmed */
    /* 0x08 */ s32 tamer; /* L1 or R1 held at the pick, for a character with two tamers */
    /* 0x0C */ s32 computer; /* its pick is the computer's (selectFlags SELECT_COMPUTER_0 << side) */
    /* 0x10 */ s32 justPicked; /* on the frame of the pick */
    /* 0x14 */ s32 index; /* of the side */
    /* 0x18 */ s32 cell; /* of its character, CHARACTER_CELLS[character] */
    /* 0x1C */ u8 unk1C[4];
    /* 0x20 */ s32 cellFrame; /* frames since the cursor moved: the cell's info slides in */
    /* 0x24 */ GridCell *entry; /* &GRID_CELLS[cell] */
    /* 0x28 */ s32 pickFlash; /* 0x7F at the pick, down by 0x10 a frame: lightens the cursor */
    /* 0x2C */ s32 gaugeFlash; /* the same for the gauge */
    /* 0x30 */ s32 gauge; /* the versus gauge, 0 to 7: GameState.fighters[side].versusGauge after a versus pick */
    /* 0x34 */ s32 gaugeSlide; /* 0 to 15: the gauge's panel slides out while the side picks it */
} CharacterSelectSide;

/* The character select's task (characterSelectTaskInit, virtual table CHARACTER_SELECT_TASK_VTABLE), a
 * TickTask: both sides pick a character on the grid (and a versus gauge), then the run's pictures show or
 * the arena is picked; the picks go to GAME_STATE. */
typedef struct CharacterSelectTask {
    /* 0x000 */ Task task;
    /* 0x020 */ s32 updateArg;
    /* 0x024 */ Camera camera;
    /* 0x060 */ MATRIX view;
    /* 0x080 */ u8 unk80[0x88 - 0x80];
    /* 0x088 */ s32 step; /* CHARACTER_SELECT_STEP_* */
    /* 0x08C */ s32 backedOut; /* a side cancelled before picking: the scene's result is -1 */
    /* 0x090 */ s32 challengerJoined; /* the scene's result is CHARACTER_SELECT_CHALLENGE */
    /* 0x094 */ s32 exitTime; /* up by 8 to 0x380 while the model rises */
    /* 0x098 */ s32 unk98; /* frames waited in the enter step, then the fade-out's level */
    /* 0x09C */ s32 fade; /* the fade-out, 0 to 0xFF */
    /* 0x0A0 */ s32 gridSlide; /* 20 down to 0: the grid slides in */
    /* 0x0A4 */ s32 frame; /* counted by the update */
    /* 0x0A8 */ s32 arenaStep; /* ARENA_STEP_* */
    /* 0x0AC */ s32 arenaSlide; /* the arena pick's slide-in, 0 to 0xA0 (CharacterSelectTask_drawArena) */
    /* 0x0B0 */ s32 arenaY; /* the arena pick's y, 0x72 to 0x100 */
    /* 0x0B4 */ int pictureHalfHeight; /* of the arena's picture, 0 to 0x20 (a plain int: see
                                         * CharacterSelectTask_drawArena) */
    /* 0x0B8 */ s32 unkB8;
    /* 0x0BC */ s32 arenaFrame; /* counted every frame of the pick and the confirmation */
    /* 0x0C0 */ CharacterSelectSide sides[FIGHTER_COUNT];
    /* 0x130 */ u32 selectFlags; /* what is being picked: SELECT_* */
    /* 0x134 */ u32 voices[FIGHTER_COUNT]; /* by side: the tamer's voice line (loadTamerVoice) */
    /* 0x13C */ s32 unk13C;
    /* 0x140 */ u8 unk140[0x154 - 0x140];
    /* 0x154 */ u32 sound; /* the handle of the line the host says */
    /* 0x158 */ s32 voiceStep; /* of the host's lines (CharacterSelectTask_tickHostVoice) */
    /* 0x15C */ s32 voiceWait; /* frames left before the next line */
    /* 0x160 */ u8 unk160[0x170 - 0x160];
    /* 0x170 */ s32 zoomScale; /* the scale (4.12) of the picture CharacterSelectTask_drawArena zooms out */
    /* 0x174 */ s32 zoomAngle; /* its angle about z */
    /* 0x178 */ s32 zoomFrames;
    /* 0x17C */ s32 blackLevel; /* the level of the black over the screen */
    /* 0x180 */ s32 zoomBrightness; /* the picture's brightness above 0x80 */
    /* 0x184 */ u8 unk184[0x18C - 0x184];
    /* 0x18C */ Vec3 position; /* of the model (TitleChara_setTransform) */
    /* 0x198 */ u8 unk198[4];
    /* 0x19C */ Vec3 rotation; /* of the model: angles about x, y, z */
    /* 0x1A8 */ u8 unk1A8[4];
    /* 0x1AC */ s32 arena; /* picked */
    /* 0x1B0 */ s32 characters[FIGHTER_COUNT]; /* by side */
    /* 0x1B8 */ s32 tamers[FIGHTER_COUNT]; /* by side: GameState.fighters[side].tamer */
    /* 0x1C0 */ s32 pickable[CHARACTER_COUNT]; /* by cell of the grid */
    /* 0x220 */ struct TitleChara *chara; /* the host, a title character model that reacts and speaks */
    /* 0x224 */ EntityGroup entities;
} CharacterSelectTask; /* size 0x234 */

INCLUDE_ASM("title/nonmatchings/chara_select", characterSelectTaskInit);

INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_UP);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_TARGET);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_CAMERA_EYE);
INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_CHARASEL_TIM);

extern TickTaskVtable CHARACTER_SELECT_TASK_VTABLE;

/* The character select's destructor: turns dithering back on, deletes the host, leaves the scene's result
 * for the sequencer (-1 when a side backed out, else CHARACTER_SELECT_CHALLENGE when a player joined in, 0
 * otherwise), unloads the sounds and destroys the entities, the camera and the Task. */
void CharacterSelectTask_destroy(CharacterSelectTask *this, s32 flags) {
    s32 result;

    this->task.vtable = &CHARACTER_SELECT_TASK_VTABLE.task;
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
    taskDestroy(&this->task, flags);
}

INCLUDE_RODATA("title/nonmatchings/chara_select", CELL_NEIGHBORS);
INCLUDE_RODATA("title/nonmatchings/chara_select", CELL_PAIRS);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_pickCharacter);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_pickGauge);

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

INCLUDE_ASM("title/nonmatchings/chara_select", uploadSidePictures);

INCLUDE_ASM("title/nonmatchings/chara_select", uploadRunPictures);
void uploadRunPictures(void *file, s32 character, s32 tamer);

/* Whether side's tamer picture is the 64-pixel-wide one (WIDE_TAMER_PICTURE_PLACES): tamer 0 of Holydramon and
 * Tailmon, tamer 1 of Stingmon and Wormmon. */
s32 CharacterSelectTask_isTamerPictureWide(CharacterSelectTask *this, s32 side) {
    if (((this->characters[side] == CHARACTER_HOLYDRAMON || this->characters[side] == CHARACTER_TAILMON) &&
         this->tamers[side] == 0) ||
        (this->characters[side] == CHARACTER_STINGMON && this->tamers[side] == 1) ||
        (this->characters[side] == CHARACTER_WORMMON && this->tamers[side] == 1)) {
        return 1;
    }
    return 0;
}

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_loadTamerVoice);
void CharacterSelectTask_loadTamerVoice(CharacterSelectTask *this, s32 side, s32 tamer);

void CharacterSelectTask_stepEnter(CharacterSelectTask *this);
void CharacterSelectTask_stepPick(CharacterSelectTask *this);
void CharacterSelectTask_tickHostVoice(CharacterSelectTask *this);
s32 CharacterSelectTask_isBackedOut(CharacterSelectTask *this);
void CharacterSelectTask_pickCharacters(CharacterSelectTask *this);
void CharacterSelectTask_pickVersus(CharacterSelectTask *this);
void CharacterSelectTask_stepRunExit(CharacterSelectTask *this);
void CharacterSelectTask_loadRunPictures(CharacterSelectTask *this);
void CharacterSelectTask_waitConfirm(CharacterSelectTask *this);
void CharacterSelectTask_stepArenaExit(CharacterSelectTask *this);
void CharacterSelectTask_loadArenaPictures(CharacterSelectTask *this);
void CharacterSelectTask_stepArena(CharacterSelectTask *this);
void CharacterSelectTask_startFadeOut(CharacterSelectTask *this);
void CharacterSelectTask_stepFadeOut(CharacterSelectTask *this);
void CharacterSelectTask_storeArena(CharacterSelectTask *this);
void CharacterSelectTask_draw(CharacterSelectTask *this, s32 arg);
void CharacterSelectTask_pickArena(CharacterSelectTask *this);

/* The character select's frame: runs the step, then draws and counts the frame; the end step stores the picks
 * and kills the task. The cases are in the order of their blocks in the asm. */
void CharacterSelectTask_tick(CharacterSelectTask *this, s32 arg) {
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
        taskKill(&this->task);
        return;
    }
    CharacterSelectTask_draw(this, arg);
    this->frame++;
}

/* The enter step: waits 32 frames, then goes to the pick with the host's lines from the first. */
void CharacterSelectTask_stepEnter(CharacterSelectTask *this) {
    if (++this->unk98 >= 32) {
        this->step = CHARACTER_SELECT_STEP_PICK;
        this->voiceStep = 0;
    }
}

/* The pick step: after the host's lines, a side backing out (not in a challenge) plays the back sound and goes to
 * the fade with nothing picked. Else each side's justPicked is cleared and its flashes count down by 0x10
 * to 0, and a versus pick goes on in CharacterSelectTask_pickVersus, any other in
 * CharacterSelectTask_pickCharacters. */
void CharacterSelectTask_stepPick(CharacterSelectTask *this) {
    s32 i;

    CharacterSelectTask_tickHostVoice(this);
    if (!GAME_STATE.challenge && CharacterSelectTask_isBackedOut(this)) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_BACK, 0);
        this->selectFlags = 0;
        this->step = CHARACTER_SELECT_STEP_FADE_SONGS;
        return;
    }
    for (i = 0; i < FIGHTER_COUNT; i++) {
        this->sides[i].justPicked = 0;
        this->sides[i].pickFlash -= 0x10;
        if (this->sides[i].pickFlash < 0) {
            this->sides[i].pickFlash = 0;
        }
        this->sides[i].gaugeFlash -= 0x10;
        if (this->sides[i].gaugeFlash < 0) {
            this->sides[i].gaugeFlash = 0;
        }
    }
    if ((this->selectFlags & (SELECT_VERSUS | SELECT_BONUS)) != SELECT_VERSUS) {
        CharacterSelectTask_pickCharacters(this);
    } else {
        CharacterSelectTask_pickVersus(this);
    }
}

/* The host's lines: plays sounds 0x23, 0x24 and 0x25 in turn, each handed to the model to speak (0x24 once
 * 0x23 ended and 90 frames passed, 0x25 likewise after 300); voice step 7 stops the sound if it still plays. */
void CharacterSelectTask_tickHostVoice(CharacterSelectTask *this) {
    if (--this->voiceWait < 0) {
        this->voiceWait = 0;
    }
    switch (this->voiceStep) {
    case 0:
        TitleChara_setAction(this->chara, TITLE_CHARA_TALK);
        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x23, 0);
        TitleChara_setVoice(this->chara, this->sound);
        this->voiceStep++;
        break;
    case 1:
        if (!isSoundPlaying(this->sound)) {
            this->voiceWait = 90;
            this->voiceStep++;
        }
        break;
    case 2:
        if (!isSoundPlaying(this->sound) && this->voiceWait == 0) {
            TitleChara_setAction(this->chara, TITLE_CHARA_TALK);
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x24, 0);
            TitleChara_setVoice(this->chara, this->sound);
            this->voiceStep++;
        }
        break;
    case 3:
        if (!isSoundPlaying(this->sound)) {
            this->voiceWait = 300;
            this->voiceStep++;
        }
        break;
    case 4:
        if (!isSoundPlaying(this->sound) && this->voiceWait == 0) {
            TitleChara_setAction(this->chara, TITLE_CHARA_TALK);
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x25, 0);
            TitleChara_setVoice(this->chara, this->sound);
            this->voiceStep = 7;
        }
        break;
    case 7:
        if (isSoundPlaying(this->sound) == 1) {
            stopSound(this->sound);
        }
        this->voiceStep++;
        break;
    }
}

/* Whether a side backed out: cancel pressed on a side being picked that has not picked yet (either side
 * when both are being picked). Sets backedOut and returns it. */
s32 CharacterSelectTask_isBackedOut(CharacterSelectTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 cancel0 = padManagerGetPressed(pads, 0) & PAD_CANCEL;
    s32 cancel1 = padManagerGetPressed(pads, 1) & PAD_CANCEL;

    switch (this->selectFlags & (SELECT_SIDE_0 | SELECT_SIDE_1)) {
    case SELECT_SIDE_0:
        if (!this->sides[0].picked && cancel0) {
            this->backedOut = 1;
        }
        break;
    case SELECT_SIDE_1:
        if (!this->sides[1].picked && cancel1) {
            this->backedOut = 1;
        }
        break;
    case SELECT_SIDE_0 | SELECT_SIDE_1:
        if (!this->sides[0].picked && !this->sides[1].picked && (cancel0 | cancel1)) {
            this->backedOut = 1;
        }
        break;
    }
    return this->backedOut;
}

extern s32 D_8007B230;

void CharacterSelectTask_joinSide(CharacterSelectTask *this, s32 character, s32 side);

/* A challenger joins by Start on the pad of a side no player is picking: side 0 joins on Agumon (Gabumon when
 * side 1 has Agumon), side 1 on V-mon (Wormmon when side 0 has V-mon), unless D_8007B230 is set. Returns
 * whether one joined. Nothing in the game calls it. */
s32 CharacterSelectTask_checkChallenger(CharacterSelectTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    s32 start0 = padManagerGetPressed(pads, 0) & PADstart;
    s32 start1 = padManagerGetPressed(pads, 1) & PADstart;

    if ((this->selectFlags & (SELECT_SIDE_0 | SELECT_COMPUTER_0)) != SELECT_SIDE_0 && start0) {
        if (D_8007B230 == 0 && this->characters[1] == CHARACTER_AGUMON) {
            this->characters[0] = CHARACTER_GABUMON;
        } else {
            this->characters[0] = CHARACTER_AGUMON;
        }
        CharacterSelectTask_joinSide(this, this->characters[0], 0);
        return 1;
    }
    if ((this->selectFlags & (SELECT_SIDE_1 | SELECT_COMPUTER_1)) != SELECT_SIDE_1 && start1) {
        if (D_8007B230 == 0 && this->characters[0] == CHARACTER_V_MON) {
            this->characters[1] = CHARACTER_WORMMON;
        } else {
            this->characters[1] = CHARACTER_V_MON;
        }
        CharacterSelectTask_joinSide(this, this->characters[1], 1);
        return 1;
    }
    return 0;
}

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_joinSide);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_pickCharacters);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_pickVersus);

INCLUDE_RODATA("title/nonmatchings/chara_select", SIDE_SELECT_MASKS);

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_storePicks);

/* The run's exit: moves the model up by 4 and turns it about y; once exitTime reaches 0x380 (8 a call),
 * goes to load the run's pictures with the model back at height 500. */
void CharacterSelectTask_stepRunExit(CharacterSelectTask *this) {
    this->position.vy -= 4;
    this->rotation.vy += 0x30;
    TitleChara_setTransform(this->chara, &this->position, &this->rotation);
    this->exitTime += 8;
    if (this->exitTime >= 0x380) {
        this->step = CHARACTER_SELECT_STEP_RUN_LOAD;
        this->position.vy = 500;
    }
}

INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_MAPSEL_TIM);
INCLUDE_RODATA("title/nonmatchings/chara_select", PATH_LEFT_PICTURES_TIM);

extern char PATH_MAPSEL_TIM[];
extern char PATH_LEFT_PICTURES_TIM[]; /* a format: the picked character's pictures file */

/* Loads the run's pictures: unloads sound 0x17 and the host's first line (0x23), uploads MAPSEL.TIM, then the
 * picked side's pictures file (side 1's when it picks, else side 2's) and its tamer's voice; goes to the show
 * step. */
void CharacterSelectTask_loadRunPictures(CharacterSelectTask *this) {
    char path[64];
    void *file;

    soundControlUnloadSound(&SOUND_CONTROL, 0x17);
    soundControlUnloadSound(&SOUND_CONTROL, 0x23);
    file = loadFile(PATH_MAPSEL_TIM);
    uploadTims(file);
    mainHeapFree(file);
    if (this->selectFlags & SELECT_SIDE_0) {
        sprintf(path, PATH_LEFT_PICTURES_TIM, this->characters[0]);
        file = loadFile(path);
        uploadRunPictures(file, this->characters[0], this->tamers[0]);
        mainHeapFree(file);
        CharacterSelectTask_loadTamerVoice(this, 0, this->tamers[0]);
    } else {
        sprintf(path, PATH_LEFT_PICTURES_TIM, this->characters[1]);
        file = loadFile(path);
        uploadRunPictures(file, this->characters[1], this->tamers[1]);
        mainHeapFree(file);
        CharacterSelectTask_loadTamerVoice(this, 1, this->tamers[1]);
    }
    this->step = CHARACTER_SELECT_STEP_RUN_SHOW;
}


/* The end of the run's pictures steps: the confirm button on any pad goes to the fade. */
void CharacterSelectTask_waitConfirm(CharacterSelectTask *this) {
    if (padManagerGetPressed(PAD_MANAGER_INSTANCE, -1) & PAD_CONFIRM) {
        this->step = CHARACTER_SELECT_STEP_FADE_SONGS;
    }
}

/* The arena's exit: moves the model up by 4 and turns it about y; once exitTime reaches 0x380 (8 a call),
 * goes to load the arena pick's pictures. */
void CharacterSelectTask_stepArenaExit(CharacterSelectTask *this) {
    this->position.vy -= 4;
    this->rotation.vy += 0x30;
    TitleChara_setTransform(this->chara, &this->position, &this->rotation);
    this->exitTime += 8;
    if (this->exitTime >= 0x380) {
        this->step = CHARACTER_SELECT_STEP_ARENA_LOAD;
    }
}

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_loadArenaPictures);

/* The arena step: picks the arena; back at the pick step, clears exitTime. */
void CharacterSelectTask_stepArena(CharacterSelectTask *this) {
    CharacterSelectTask_pickArena(this);
    if (this->step == CHARACTER_SELECT_STEP_PICK) {
        this->exitTime = 0;
    }
}

/* Starts fading the songs out (step 0x44) and goes to the fade out. */
void CharacterSelectTask_startFadeOut(CharacterSelectTask *this) {
    fadeSongs(0x44);
    this->step = CHARACTER_SELECT_STEP_FADE_OUT;
}

/* The fade out: by 4 a frame, then the end step; 0x98 follows as (0xFF - fade) / 8. */
void CharacterSelectTask_stepFadeOut(CharacterSelectTask *this) {
    this->fade += 4;
    if (this->fade >= 0x100) {
        this->fade = 0xFF;
        this->step = CHARACTER_SELECT_STEP_END;
    }
    this->unk98 = (0xFF - this->fade) >> 3;
}

/* The end of the scene: stores what was picked in GAME_STATE (the arena of a versus fight or a bonus game, and
 * each side's gauge after a versus pick). The match depends on the arena read into a local and stored to
 * GAME_STATE.arena first: an assignment chain stores right to left, and the bonus game reads the field again. */
void CharacterSelectTask_storeArena(CharacterSelectTask *this) {
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
        GAME_STATE.fighters[0].versusGauge = this->sides[0].gauge;
        GAME_STATE.fighters[1].versusGauge = this->sides[1].gauge;
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

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DA20);

/* Draws the grid's scrolling strip: a 112x16 strip (v 0xEC of the page at (640, 256), semi-transparent) moving
 * left with frame (a pixel a frame from x + 0xBD, wrapping every 256 frames), clipped to the 160x16 window at
 * (x + 0x40, y + 0x14); the draw area is set back to the whole buffer after it. */
void drawScrollingStrip(u_long *ot, s32 x, s32 y, u8 frame) {
    /* The match depends on the scroll in its own variable (one expression folds to x + 0xBD - frame) and on the
     * order of the SPRT's fields below. */
    s32 scroll = frame - 0xBD;
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

INCLUDE_ASM("title/nonmatchings/chara_select", func_8006DCCC);

void drawCharacterPortrait(u_long *ot, s32 x, s32 y, s32 index, s32 side);
void func_8006E1A0(u_long *ot, s32 x, s32 y);

INCLUDE_ASM("title/nonmatchings/chara_select", drawCharacterPortrait);

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

void func_8006E368(u_long *ot, s32 x, s32 y);

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

INCLUDE_ASM("title/nonmatchings/chara_select", CharacterSelectTask_drawGrid);

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

/* Draws sprite `sprite` of SELECT_SPRITES's table at pos in ot: mirrored left to right as a POLY_FT4 when
 * poly is set, else as a SPRT (with its own DR_TPAGE). */
void drawSelectSprite(u_long *ot, DVECTOR *pos, s32 sprite, s32 poly);

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

void drawSelectSpriteShaded(u_long *ot, DVECTOR *pos, s32 sprite, s32 shade, s32 poly);

/* The arena pick's animation (ARENA_STEP_*) and drawing. Up to the confirmation it slides the pick in and
 * opens the picture of the arena (ARENA_PICTURES); then a picture zooms out and the screen fades to black,
 * and the scene goes to its fade. The pick's sprites take rect's x and y as their position: the game keeps
 * one stack slot for both. The match depends on pictureHalfHeight being a plain int (through the s32 typedef,
 * narrowing the picture's bottom y to a short reassociates it as (y + 0x2B) - h), on one variable for the
 * scale and on the translation being set before the scale words. */
void CharacterSelectTask_drawArena(CharacterSelectTask *this, u_long *ot) {
    switch (this->arenaStep) {
    case ARENA_STEP_INIT:
        this->arenaSlide = 0;
        this->arenaY = 0x72;
        this->pictureHalfHeight = 0;
        this->unkB8 = 0;
        this->arenaFrame = 0;
        break;
    case ARENA_STEP_SLIDE_IN:
        this->arenaSlide += 8;
        if (this->arenaSlide > 0xA0) {
            this->arenaSlide = 0xA0;
            this->arenaStep++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        this->unkB8++;
        break;
    case ARENA_STEP_OPEN_PICTURE:
        this->pictureHalfHeight += 2;
        if (this->pictureHalfHeight > 0x20) {
            this->pictureHalfHeight = 0x20;
            this->arenaStep++;
        }
        break;
    case ARENA_STEP_CONFIRMED:
        this->arenaY += 8;
        if (this->arenaY > 0x100) {
            this->arenaY = 0x100;
        }
        if (--this->unkB8 < 0) {
            this->unkB8 = 0;
        }
        if (this->arenaY == 0x100 && this->unkB8 == 0) {
            this->arenaStep++;
        }
        /* fall through */
    case ARENA_STEP_PICK:
        this->arenaFrame++;
        break;
    case ARENA_STEP_ZOOM_INIT:
        this->zoomScale = 0x4000;
        this->zoomAngle = 0;
        this->blackLevel = 0;
        this->zoomFrames = 0;
        this->zoomBrightness = 0;
        this->arenaStep++;
        /* fall through */
    case ARENA_STEP_ZOOM_OUT:
        this->zoomScale -= 0x200;
        if (this->zoomScale < 0x1000) {
            this->zoomScale = 0x1000;
            this->zoomBrightness = 0x7F;
            this->arenaStep++;
            soundControlPlaySound(&SOUND_CONTROL, 0x5D, 0);
        }
        break;
    case ARENA_STEP_FADE_PICTURE:
        this->zoomBrightness -= 2;
        if (this->zoomBrightness < 0) {
            this->zoomBrightness = 0;
        }
        if (this->zoomFrames++ > 60) {
            this->arenaStep++;
        }
        break;
    case ARENA_STEP_BLACK_OUT:
        this->blackLevel += 8;
        if (this->blackLevel >= 0xFF) {
            this->blackLevel = 0xFF;
            this->arenaStep++;
        }
        this->zoomScale += 0x200;
        this->zoomAngle += 0x80;
        break;
    case ARENA_STEP_END:
        this->step = CHARACTER_SELECT_STEP_FADE_SONGS;
        break;
    }
    if (this->arenaStep >= ARENA_STEP_ZOOM_INIT) {
        if (this->blackLevel != 0) {
            DR_TPAGE *tpage = allocDrTpage(0x25);
            TILE *tile = allocTile();

            setRGB0(tile, this->blackLevel, this->blackLevel, this->blackLevel);
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
            u32 scale = (u16)this->zoomScale;
            POLY_FT4 *poly;

            m.m.t[0] = 0xA0;
            m.m.t[1] = 0xA0;
            m.m.t[2] = 0;
            m.words[0] = scale;
            m.words[1] = 0;
            m.words[2] = scale;
            m.words[3] = 0;
            m.words[4] = scale;
            matrixRotateZ(&m.m, this->zoomAngle);
            matrixTransformVecs(&m.m, screen, corners, 4);
            poly = allocPolyFT4();
            setXY4(poly, screen[0].vx, screen[0].vy, screen[1].vx, screen[1].vy, screen[2].vx, screen[2].vy,
                   screen[3].vx, screen[3].vy);
            setUV4(poly, 0, 0, 0x97, 0, 0, 0x4F, 0x97, 0x4F);
            setRGB0(poly, this->zoomBrightness + 0x80, this->zoomBrightness + 0x80, this->zoomBrightness + 0x80);
            poly->tpage = 0x88;
            poly->clut = 0x3D98;
            setSemiTrans(poly, 1);
            AddPrim(ot, poly);
        }
    } else {
        RECT rect;

        rect.x = this->arenaSlide - 0x50;
        rect.y = this->arenaY;
        drawSelectSprite(ot, (DVECTOR *)&rect, 9, 0);
        rect.x = 320 - this->arenaSlide;
        drawSelectSprite(ot, (DVECTOR *)&rect, 10, 0);
        if (this->pictureHalfHeight > 0) {
            SelectSprite *picture = &ARENA_PICTURES[this->arena];
            POLY_FT4 *poly = allocPolyFT4();

            setRGB0(poly, 0x80, 0x80, 0x80);
            setXYWH(poly, 0x78, this->arenaY + 0x2B - this->pictureHalfHeight, 0x50, this->pictureHalfHeight * 2);
            setUVWH(poly, picture->u, picture->v, 0x4F, 0x3F);
            poly->tpage = picture->tpage;
            poly->clut = picture->clut;
            SetSemiTrans(poly, 0);
            AddPrim(ot, poly);
        }
        getDrawBufferRect(&rect);
        AddPrim(ot, allocDrArea(&rect));
        if (this->arenaSlide == 0xA0) {
            rect.x = 0xE8 - (u8)this->arenaFrame;
            rect.y = this->arenaY + 0x5A;
            drawSelectSprite(ot, (DVECTOR *)&rect, 13, 0);
        }
        getDrawBufferSubRect(&rect, 0x5A, this->arenaY + 0x5A, 0x8C, 0x12);
        if (rect.h > 0) {
            AddPrim(ot, allocDrArea(&rect));
        }
        rect.x = this->arenaSlide - 0x48;
        rect.y = this->arenaY + 0x5A;
        drawSelectSpriteShaded(ot, (DVECTOR *)&rect, 11, 0x80, 0);
        rect.x = 320 - this->arenaSlide;
        drawSelectSpriteShaded(ot, (DVECTOR *)&rect, 12, 0x80, 0);
    }
}

/* The arena pick, while its step is ARENA_STEP_PICK: plays sounds 0x27, 0x28 and 0x29 in turn (0x29 again
 * every 300 frames), left and right step through the arenas (a fight's 0 to 5, the boss's too once a run was
 * won; with SELECT_BONUS the bonus games, skipping those not unlocked yet) and the confirm
 * button confirms it.
 * The match depends on save.bonusGamesUnlocked being a u32 (with an s32 mask, fold turns the bit test into a
 * shift of the mask) and on each loop having its own counter. */
void CharacterSelectTask_pickArena(CharacterSelectTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    u32 pressed = padManagerGetPressed(pads, -1);
    u32 repeated = padManagerGetRepeated(pads, -1);
    s32 first;
    s32 last;

    if (this->arenaStep != ARENA_STEP_PICK) {
        return;
    }
    if (--this->voiceWait < 0) {
        this->voiceWait = 0;
    }
    this->voiceWait--;
    switch (this->voiceStep) {
    case 0:
        this->voiceWait = 60;
        break;
    case 1:
        if (this->voiceWait == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x27, 0);
            this->voiceStep++;
        }
        break;
    case 2:
        if (!isSoundPlaying(this->sound)) {
            this->voiceWait = 90;
            this->voiceStep++;
        }
        break;
    case 3:
        if (this->voiceWait == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x28, 0);
            this->voiceStep++;
        }
        break;
    case 4:
        if (!isSoundPlaying(this->sound)) {
            this->voiceWait = 300;
            this->voiceStep++;
        }
        break;
    case 5:
        if (this->voiceWait == 0) {
            this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x29, 0);
            this->voiceStep = 4;
        }
        break;
    case 7:
        if (isSoundPlaying(this->sound) == 1) {
            stopSound(this->sound);
        }
        this->voiceStep++;
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
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
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
                if (GAME_STATE.save.bonusGamesUnlocked & ARENA_BIT(this->arena)) {
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
                if (GAME_STATE.save.bonusGamesUnlocked & ARENA_BIT(this->arena)) {
                    break;
                }
            }
        }
    }
    if (pressed & PAD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        this->arenaStep = ARENA_STEP_CONFIRMED;
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
void CharacterSelectTask_update(TickTask *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_SELECT_TASK_VTABLE);
INCLUDE_RODATA("title/nonmatchings/chara_select", GRID_CELLS);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064C44);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_CELLS);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_TAMERS);
INCLUDE_RODATA("title/nonmatchings/chara_select", D_80064E88);
INCLUDE_RODATA("title/nonmatchings/chara_select", ROOKIE_PICTURE_PLACES);
INCLUDE_RODATA("title/nonmatchings/chara_select", CHARACTER_PICTURE_PLACES);
INCLUDE_RODATA("title/nonmatchings/chara_select", TAMER_PICTURE_PLACES);
INCLUDE_RODATA("title/nonmatchings/chara_select", WIDE_TAMER_PICTURE_PLACES);
INCLUDE_RODATA("title/nonmatchings/chara_select", NAME_PLATE_PLACE);
INCLUDE_RODATA("title/nonmatchings/chara_select", SELECT_SPRITES);
INCLUDE_RODATA("title/nonmatchings/chara_select", ARENA_PICTURES);
