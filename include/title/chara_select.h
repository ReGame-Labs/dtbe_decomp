#ifndef DTBE_TITLE_CHARA_SELECT_H
#define DTBE_TITLE_CHARA_SELECT_H

/* The character select's task and its sides (title/chara_select.cpp, C++ only). */

/* CharacterSelectTask's virtual table stays in the overlay's assembly (.rodata), so chara_select.cpp writes
 * it out nowhere. */
#pragma interface

#include "common.h"
#include "engine/game/game_state.h"
#include "engine/gfx/camera.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"
#include "title/title_classes.h"

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

/* The character select's task (characterSelectTaskInit): both sides pick a character on the grid (and a
 * versus gauge), then the run's pictures show or the arena is picked; the picks go to GAME_STATE. Its update
 * keeps the argument and calls tick, as the other TickTasks of the overlay. */
class CharacterSelectTask : public Task {
public:
    /* 0x020 */ s32 updateArg; /* the argument of the last update */
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
    /* 0x140 */ s32 voiceHasReply[FIGHTER_COUNT]; /* by side: the voice line is the first of a two-line exchange
                                                    * (the draw plays the next line after it) */
    /* 0x148 */ u8 unk148[0x154 - 0x148];
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

    CharacterSelectTask(struct SystemContext *context, s32 selectFlags) __asm__("characterSelectTaskInit");
    ~CharacterSelectTask();
    void update(s32 arg) __asm__("CharacterSelectTask_update");
    /* the frame: runs the step, then draws */
    virtual void tick(s32 arg) __asm__("CharacterSelectTask_tick");
}; /* size 0x234 */

#endif /* DTBE_TITLE_CHARA_SELECT_H */
