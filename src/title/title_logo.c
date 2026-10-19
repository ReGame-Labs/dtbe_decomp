/* The title screen (the task "Title Logo", title/title.tim). */

#include "common.h"
#include <abs.h>
#include <libetc.h>
#include "engine/cd/file_load.h"
#include "engine/debug/system_menu.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/matrix.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/sound/sound.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "title/title_classes.h"

/* TitleLogoTask.step, the index of TITLE_LOGO_STEPS */
#define TITLE_LOGO_STEP_WAIT 0
#define TITLE_LOGO_STEP_SHOW_LOGO 1
#define TITLE_LOGO_STEP_WAIT_START 2
#define TITLE_LOGO_STEP_HIDE_LOGO 3
#define TITLE_LOGO_STEP_FADE_SONGS 4
#define TITLE_LOGO_STEP_FADE_OUT 5
#define TITLE_LOGO_STEP_END (-1) /* the task ends once the fade is done */

/* the virtual tables of the entity classes below */
extern EntityVtable TITLE_SPARKLE_VTABLE;
extern EntityVtable TITLE_STREAK_VTABLE;
extern EntityVtable D_800645E4;
extern EntityVtable TITLE_LOGO_PIECE_VTABLE;

/* The title logo task (titleLogoTaskInit, virtual table TITLE_LOGO_TASK_VTABLE), which TitleLogoTask_tick runs in
 * Japan. The US title screen was rewritten: its object has another sprite and other fields, so the layout
 * is per version. Offsets and unk names are the Japanese ones; the US object has 0x28 more bytes before
 * startPressed and 0x2C more before camera. */
typedef struct TitleLogoTask {
    /* 0x0000 */ Task task;
    /* 0x0020 */ s32 updateArg;
#if VERSION_JP
    /* 0x0024 */ EntityGroup group;
    /* 0x0034 */ Sprite *copyright;  /* title.tim's copyright lines */
    /* 0x0038 */ Sprite *pressStart; /* its "press the Start button", drawn by TitleLogoTask_drawPressStart */
    /* 0x003C */ RECT clutRect;      /* the CLUT of title.tim's third image */
#else
    /* 0x0024 */ RECT clutRect;  /* 16 by 1 pixels at (0x200, 0xFD): a CLUT */
    /* 0x002C */ u16 clut[16];   /* that CLUT, read back from VRAM by TitleLogoTask_readClut */
    /* 0x004C */ u16 clutPhase; /* the phase of the sine func_8007236C pulses the CLUT with */
    /* 0x004E */ u8 unk4E[0x50 - 0x4E];
    /* 0x0050 */ EntityGroup group;
    /* 0x0060 */ Sprite *sprites[3];
#endif
    /* 0x0044 */ s32 startPressed; /* Start was pressed on the title */
    /* 0x0048 */ s32 unk48;
    /* 0x004C */ s32 stepFrame; /* frames since the step began */
    /* 0x0050 */ s32 spawnRate;      /* added to spawnCounter a frame: 2, up to 8 from the logo's exit */
    /* 0x0054 */ s32 logoBrightness; /* 0x80 up to 0xFF */
    /* 0x0058 */ s32 logoScale;      /* 0x1000 the logo's own size */
    /* 0x005C */ s32 promptPhase;    /* the angle the press Start prompt blinks with */
    /* 0x0060 */ s32 shownCopyrightLevel; /* the copyright's level while the logo shows (80 full); Japan copies it into
                                         * copyrightLevel each frame */
    /* 0x0064 */ MATRIX view;
    /* 0x0084 */ SVECTOR ringPoints[6][33]; /* the tunnel's rings (TitleLogoTask_initTunnel) */
    /* 0x06B4 */ s16 lineLevel; /* the tunnel lines' level (TitleLogoTask_drawRingLines); Japan makes streaks while
                                 * it is not 0 */
    /* 0x06B6 */ s16 ringCount;  /* the rings TitleLogoTask_drawRingLines draws, plus one: one more each 64 frames */
    /* 0x06B8 */ s32 ringScales[16]; /* each drawn ring's scale, 0x1000 for 1 */
    /* 0x06F8 */ s16 copyrightLevel; /* the copyright's brightness, 80 for full */
    /* 0x06FA */ s16 unk6FA; /* the US logo's phase (TitleLogoTask_stepShowLogo) */
#if VERSION_US
    /* 0x06FC */ s16 shakeX; /* the US logo's shake: a damped sine added to its position */
    /* 0x06FE */ s16 shakeY;
#endif
    /* 0x06FC */ MersenneTwister random; /* the entities' random angles (TitleStreak_init) */
    /* 0x10C0 */ Camera camera;
    /* 0x10FC */ s32 step; /* TITLE_LOGO_STEP_* */
    /* 0x1100 */ s32 spawnCounter; /* an entity is made each time its low bits are 0 */
    /* 0x1104 */ u16 soundStarted;
    /* 0x1106 */ u8 unk1106[0x1108 - 0x1106];
    /* 0x1108 */ u32 sound; /* a sound's handle (stopSound) */
#if VERSION_US
    /* 0x110C */ s16 unk110C; /* 0x1138 in the US object, which alone has it */
#endif
} TitleLogoTask;

/* the title logo task's virtual table */
extern TickTaskVtable TITLE_LOGO_TASK_VTABLE;

/* The entity class of vtable TITLE_STREAK_VTABLE (TitleStreak_init), which moves until it is out of the title. */
typedef struct TitleStreak {
    /* 0x00 */ Entity entity;
    /* 0x18 */ TitleLogoTask *owner;
    /* 0x1C */ VECTOR position;
    /* 0x2C */ VECTOR velocity; /* from a random angle */
} TitleStreak;

/* The entity class of vtable TITLE_SPARKLE_VTABLE (TitleSparkle_init): TitleStreak's fields, then its speed (the
 * velocity's length) and the distance it went. */
typedef struct TitleSparkle {
    /* 0x00 */ Entity entity;
    /* 0x18 */ TitleLogoTask *owner;
    /* 0x1C */ VECTOR position;
    /* 0x2C */ VECTOR velocity;
    /* 0x3C */ s16 frame;
    /* 0x3E */ s16 brightness; /* rises to 0x80 */
    /* 0x40 */ u32 randomAngle; /* the random number its angle comes from */
    /* 0x44 */ s32 speed;
    /* 0x48 */ s32 distance;
} TitleSparkle;

#if VERSION_JP
/* The entity class of vtable D_800645E4 (func_80068E0C, Japan only, which nothing makes): a semi-transparent
 * quad that waits, then drifts from a random point of an ellipse to (256, 240) in 0x78 frames, growing and
 * fading. Positions are 20.12 fixed point. */
typedef struct Unk800645E4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 x;
    /* 0x1C */ s32 y;
    /* 0x20 */ s32 vx; /* the distance to (256, 240) over 0x78 frames */
    /* 0x24 */ s32 vy;
    /* 0x28 */ s32 scale; /* 0x1000 for 1 */
    /* 0x2C */ s32 frame; /* starts at -20 times its index: a delay, drawn from 0 */
    /* 0x30 */ u8 unk30;  /* a pair from D_80064664, picked by the index; the draw scales unk31 */
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 brightness; /* 0xF0, down by 1 a frame */
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u16 unk34;
} Unk800645E4;
#endif

/* Destroys a sprite and frees it, as `delete` does: nothing for NULL. */
static inline void spriteDelete(Sprite *sprite) {
    if (sprite != NULL) {
        spriteDestroy(sprite, DESTROY_DELETE);
    }
}

void TitleLogoTask_drawRingLines(TitleLogoTask *this);
void TitleLogoTask_drawPanels(TitleLogoTask *this);
void TitleLogoTask_checkSkip(TitleLogoTask *this);
#if VERSION_US
void func_8007236C(TitleLogoTask *this);
#endif

/* the title camera's eye, target and up */
extern VECTOR TITLE_CAMERA_UP;
extern VECTOR TITLE_CAMERA_TARGET;
extern VECTOR TITLE_CAMERA_EYE;
extern char PATH_TITLE_TIM[]; /* "title/title.tim" */

void TitleLogoTask_initTunnel(TitleLogoTask *this);
#if VERSION_US
void TitleLogoTask_readClut(TitleLogoTask *this);
#endif

/* an angle that turns by 4 a frame, and the sum of its sines (at 1/256) */
extern s32 TITLE_SPIN_PHASE;
extern s32 TITLE_SPIN;

/* The title logo task's constructor: its camera looking from TITLE_CAMERA_EYE on a 512x480 screen, title.tim's
 * images uploaded and its sprites made (the copyright lines and the press Start prompt; the US title has three
 * other sprites and reads its CLUT back), the sounds loaded, the tunnel built and the screen fading in. */
TitleLogoTask *titleLogoTaskInit(TitleLogoTask *this) {
    Display *display;
    u32 *file;
    TIM_IMAGE tim;

    taskInit(&this->task, 0x2000, "Title Logo");
    this->task.vtable = &TITLE_LOGO_TASK_VTABLE.task;
    entityGroupInit(&this->group);
    mersenneTwisterInit(&this->random);
    cameraInit(&this->camera, 0);
    soundControlLoadSound(&SOUND_CONTROL, 0x8000, 1);
    cameraReset(&this->camera);
    matrixInitLookAt(&this->view, &TITLE_CAMERA_EYE, &TITLE_CAMERA_TARGET, &TITLE_CAMERA_UP);
    cameraSetScreen(&this->camera, 512, 480, 16, 15);
    cameraSetView(&this->camera, &this->view);
    display = CURRENT_DISPLAY;
    displaySetSize(display, 512, 480);
    displayClear(display);
    displaySetVsyncMode(display, 0);
    displaySetClearColor(display, 0, 0, 0);
    displayWaitFrame(display);
    this->startPressed = 0;
    this->step = 0;
    this->unk48 = 0;
    this->stepFrame = 0;
    this->logoBrightness = 0x80;
    this->logoScale = 0;
#if VERSION_US
    this->unk6FA = 0;
    this->shakeX = 0;
    this->shakeY = 0;
#endif
    file = loadFile(PATH_TITLE_TIM);
    uploadTims(file);
#if VERSION_JP
    OpenTIM((u_long *)findTim(file, 2));
    ReadTIM(&tim);
    this->clutRect = *tim.crect;
    ReadTIM(&tim);
    this->copyright = spriteInitFromTim(operatorNew(sizeof(Sprite)), &tim, 0);
    ReadTIM(&tim);
    this->pressStart = spriteInitFromTim(operatorNew(sizeof(Sprite)), &tim, 0);
#else
    OpenTIM((u_long *)findTim(file, 3));
    ReadTIM(&tim);
    this->sprites[0] = spriteInitFromTim(operatorNew(sizeof(Sprite)), &tim, 0);
    OpenTIM((u_long *)findTim(file, 8));
    ReadTIM(&tim);
    this->sprites[1] = spriteInitFromTim(operatorNew(sizeof(Sprite)), &tim, 0);
    ReadTIM(&tim);
    this->sprites[2] = spriteInitFromTim(operatorNew(sizeof(Sprite)), &tim, 0);
#endif
    mainHeapFree(file);
    this->promptPhase = 0;
#if VERSION_JP
    this->copyright->x = 0x60;
    this->copyright->y = 0x170;
    this->copyright->flags = (this->copyright->flags & ~SPRITE_ABR_MASK) | SPRITE_SEMI_TRANS | 1;
#else
    this->sprites[0]->x = 0x24;
    this->sprites[0]->y = 0x170;
    this->sprites[0]->flags = (this->sprites[0]->flags & ~SPRITE_ABR_MASK) | SPRITE_SEMI_TRANS | 1;
    this->sprites[1]->x = 0x74;
    this->sprites[1]->y = 0x52;
    this->sprites[1]->flags = (this->sprites[1]->flags & ~SPRITE_ABR_MASK) | SPRITE_SEMI_TRANS | 1;
    this->sprites[2]->x = 0x20;
    this->sprites[2]->y = 0x22;
#endif
    this->shownCopyrightLevel = 0;
    this->copyrightLevel = 0;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x8000, 0);
    this->spawnCounter = 0;
#if VERSION_JP
    this->ringCount = 0;
#else
    this->lineLevel = 0;
#endif
    this->spawnRate = 2;
    TitleLogoTask_initTunnel(this);
    TITLE_SPIN = 0;
    TITLE_SPIN_PHASE = 0;
    this->soundStarted = 0;
    soundControlLoadSound(&SOUND_CONTROL, 0x16, 1);
#if VERSION_US
    TitleLogoTask_readClut(this);
#endif
    return this;
}

#if VERSION_JP
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_UP);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_TARGET);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_EYE);
INCLUDE_RODATA("title/nonmatchings/title_logo", PATH_TITLE_TIM);
#endif

/* The destructor of the title logo task: stops its sound, gives the display back its 320 x 240 size, tells
 * the system whether Start was left unpressed (SYSTEM_CONTEXT.sceneResult), deletes the sprites, unloads the
 * sounds, fades the screen in, then destroys the camera, the entities and the task. The US object has a
 * third sprite and deletes them in the other order. */
void TitleLogoTask_destroy(TitleLogoTask *this, s32 flags) {
    Display *display;

    this->task.vtable = &TITLE_LOGO_TASK_VTABLE.task;
    stopSound(this->sound);
    display = CURRENT_DISPLAY;
    displayEnable(display, 0);
    displaySetSize(display, 0x140, 0xF0);
    SYSTEM_CONTEXT.sceneResult = this->startPressed == 0;
#if VERSION_JP
    spriteDelete(this->pressStart);
    spriteDelete(this->copyright);
#else
    spriteDelete(this->sprites[0]);
    spriteDelete(this->sprites[1]);
    spriteDelete(this->sprites[2]);
#endif
    soundControlUnloadSeps(&SOUND_CONTROL);
    soundControlUnloadVabs(&SOUND_CONTROL);
    fadeControlFadeIn(&FADE_CONTROL, 0);
    cameraDestroy(&this->camera, DESTROY_BASES);
    entityGroupDestroy(&this->group, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}


/* the title logo task's step functions, by step */
extern MemberFunction TITLE_LOGO_STEPS[];

void func_80067CC4(TitleLogoTask *this);
void TitleLogoTask_drawLogo(TitleLogoTask *this, u_long *ot);
void TitleLogoTask_drawPressStart(TitleLogoTask *this, u_long *ot);
TitleStreak *TitleStreak_init(TitleStreak *this, TitleLogoTask *owner);
TitleSparkle *TitleSparkle_init(TitleSparkle *this, TitleLogoTask *owner);

#if VERSION_JP
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_LOGO_STEPS);

/* The title screen's frame: kills the task once the fade ends at the end step, else runs the step, updates
 * and draws the entities, the logo and the screen fade, and makes an entity every 4 or 32 counts. */
void TitleLogoTask_tick(TitleLogoTask *this) {
    s16 index;
    s16 delta;
    s32 offset;
    VirtualFunction method;
    void (*func)();
    u_long *ot;

    cameraClearOt(&this->camera);
    if (this->task.killed) {
        return;
    }
    if (this->step < 0) {
        if (fadeControlIsDone(&FADE_CONTROL)) {
            taskKill(&this->task);
        }
        return;
    }
    /* (this->*TITLE_LOGO_STEPS[this->step])(), as g++ 2.95 expands it. The match depends on the virtual entry read
     * as a register pair (VirtualFunction), on the table entry indexed again for the delta, and on the delta
     * read once before the two sums are chosen. */
    index = TITLE_LOGO_STEPS[this->step].index;
    if (index >= 0) {
        method = (*(VirtualFunction **)((u8 *)this + TITLE_LOGO_STEPS[this->step].u.vtableOffset))[index - 1];
        func = method.entry.func;
    } else {
        func = TITLE_LOGO_STEPS[this->step].u.func;
    }
    delta = TITLE_LOGO_STEPS[this->step].delta;
    offset = index >= 0 ? method.entry.delta + delta : delta;
    func((u8 *)this + offset);

    ot = FRAME_OT.firstDrawn + 5;
    entityGroupTick(&this->group, (s32)this);
    entityGroupDraw(&this->group, (s32)this);
    TitleLogoTask_drawLogo(this, ot);
    TitleLogoTask_drawPressStart(this, ot);
    if (this->copyrightLevel != 0) {
        s32 level = (this->copyrightLevel << 7) / 80;
        u8 gray;

        if (level > 0x80) {
            level = 0x80;
        }
        gray = level;
        this->copyright->color = gray | (gray << 8) | (gray << 16);
        PRIM_BUFFER_FREE = spriteDraw(this->copyright, PRIM_BUFFER_FREE, ot);
    }
    func_80067CC4(this);
    if (this->lineLevel != 0) {
        if (!(this->spawnCounter & 3)) {
            Entity *entity = NEW_ENTITY(0x40, TitleStreak_init, this);


            if (entity != NULL) {
                entityGroupAppend(&this->group, entity);
            }
        }
    } else if (!(this->spawnCounter & 0x1F)) {
        Entity *entity = NEW_ENTITY(sizeof(TitleSparkle), TitleSparkle_init, this);


        if (entity != NULL) {
            entityGroupAppend(&this->group, entity);
        }
    }
    if (this->step >= TITLE_LOGO_STEP_HIDE_LOGO) {
        if (++this->spawnRate > 8) {
            this->spawnRate = 8;
        }
    }
    this->spawnCounter += this->spawnRate;
    orderingTableLinkAfter(&this->camera.ot, FRAME_OT.firstDrawn + 5);
    TITLE_SPIN += COS_SIN_TABLE[TITLE_SPIN_PHASE & 0xFFF].sin >> 8;
    TITLE_SPIN_PHASE += 4;
}

/* Waits 0x100 frames, then moves to the next step; Start or circle skips ahead (TitleLogoTask_checkSkip). The
 * copyright takes the level the steps set. */
void TitleLogoTask_stepWait(TitleLogoTask *this) {
    if (++this->stepFrame > 0x100) {
        this->stepFrame = 0;
        this->step++;
    }
    TitleLogoTask_checkSkip(this);
    this->copyrightLevel = this->shownCopyrightLevel;
}

/* The logo's entrance over 0x78 frames: on the first frame the entities go and the press Start prompt turns
 * opaque; the logo brightens by 3 a frame and grows to 0x1000 (half of 0x1000 * frame / 60), and the copyright
 * follows the frame (x 80 / 60). Then moves to the next step. Start or circle skips ahead
 * (TitleLogoTask_checkSkip); the copyright takes the level. */
void TitleLogoTask_stepShowLogo(TitleLogoTask *this) {
    s32 frame;

    if (this->stepFrame == 0) {
        entityGroupDestroyAll(&this->group);
        this->pressStart->flags &= ~SPRITE_SEMI_TRANS;
    }
    this->logoBrightness += 3;
    if (this->logoBrightness > 0xFF) {
        this->logoBrightness = 0xFF;
    }
    frame = this->stepFrame;
    this->logoScale = ((frame << 12) / 60) >> 1;
    this->shownCopyrightLevel = frame * 80 / 60;
    this->stepFrame = frame + 1;
    if (frame >= 0x78) {
        this->stepFrame = 0;
        this->step++;
    }
    TitleLogoTask_checkSkip(this);
    this->copyrightLevel = this->shownCopyrightLevel;
}
#else
/* Reads the 16-colour CLUT at (0x200, 0xFD) back from VRAM into clut. */
void TitleLogoTask_readClut(TitleLogoTask *this) {
    this->clutRect.x = 0x200;
    this->clutRect.y = 0xFD;
    this->clutPhase = 0;
    this->clutRect.w = 16;
    this->clutRect.h = 1;
    StoreImage(&this->clutRect, (u_long *)this->clut);
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_8007236C);

/* The title screen's frame (US): the Japanese copy's, but the copyright lines and the second picture are three
 * sprites, the middle one moved by the logo's shake while it settles. The step call is the Japanese copy's. */
void TitleLogoTask_tick(TitleLogoTask *this) {
    s16 index;
    s16 delta;
    s32 offset;
    VirtualFunction method;
    void (*func)();
    u_long *ot;
    s32 level;

    cameraClearOt(&this->camera);
    if (this->task.killed) {
        return;
    }
    if (this->step < 0) {
        if (fadeControlIsDone(&FADE_CONTROL)) {
            taskKill(&this->task);
        }
        return;
    }
    index = TITLE_LOGO_STEPS[this->step].index;
    if (index >= 0) {
        method = (*(VirtualFunction **)((u8 *)this + TITLE_LOGO_STEPS[this->step].u.vtableOffset))[index - 1];
        func = method.entry.func;
    } else {
        func = TITLE_LOGO_STEPS[this->step].u.func;
    }
    delta = TITLE_LOGO_STEPS[this->step].delta;
    offset = index >= 0 ? method.entry.delta + delta : delta;
    func((u8 *)this + offset);

    ot = FRAME_OT.firstDrawn + 5;
    entityGroupTick(&this->group, (s32)this);
    entityGroupDraw(&this->group, (s32)this);
    level = (this->copyrightLevel << 7) / 80;
    if (level > 0x80) {
        level = 0x80;
    }
    if (this->copyrightLevel != 0 && this->step >= TITLE_LOGO_STEP_WAIT_START) {
        u8 gray = level;
        u32 color = gray | (gray << 8) | (gray << 16);

        this->sprites[0]->color = color;
        PRIM_BUFFER_FREE = spriteDraw(this->sprites[0], PRIM_BUFFER_FREE, ot);
        this->sprites[2]->color = color;
        PRIM_BUFFER_FREE = spriteDraw(this->sprites[2], PRIM_BUFFER_FREE, ot);
    }
    if (this->step == TITLE_LOGO_STEP_SHOW_LOGO && this->unk6FA == 2) {
        this->sprites[1]->x = this->shakeX + 0x74;
        this->sprites[1]->y = this->shakeY + 0x52;
        this->sprites[1]->color = 0x808080;
        PRIM_BUFFER_FREE = spriteDraw(this->sprites[1], PRIM_BUFFER_FREE, ot);
    } else if (this->step >= TITLE_LOGO_STEP_WAIT_START) {
        u8 gray = level;

        this->sprites[1]->x = 0x74;
        this->sprites[1]->y = 0x52;
        this->sprites[1]->color = gray | (gray << 8) | (gray << 16);
        PRIM_BUFFER_FREE = spriteDraw(this->sprites[1], PRIM_BUFFER_FREE, ot);
    }
    TitleLogoTask_drawLogo(this, ot);
    TitleLogoTask_drawPressStart(this, ot);
    func_80067CC4(this);
    if (this->unk110C != 0) {
        if (!(this->spawnCounter & 3)) {
            Entity *entity = NEW_ENTITY(0x40, TitleStreak_init, this);

            if (entity != NULL) {
                entityGroupAppend(&this->group, entity);
            }
        }
    } else if (!(this->spawnCounter & 0x1F)) {
        Entity *entity = NEW_ENTITY(sizeof(TitleSparkle), TitleSparkle_init, this);

        if (entity != NULL) {
            entityGroupAppend(&this->group, entity);
        }
    }
    if (this->step >= TITLE_LOGO_STEP_HIDE_LOGO) {
        if (++this->spawnRate > 8) {
            this->spawnRate = 8;
        }
    }
    this->spawnCounter += this->spawnRate;
    orderingTableLinkAfter(&this->camera.ot, FRAME_OT.firstDrawn + 5);
    TITLE_SPIN += COS_SIN_TABLE[TITLE_SPIN_PHASE & 0xFFF].sin >> 8;
    TITLE_SPIN_PHASE += 4;
}

/* Waits 0x100 frames, then moves to the next step; Start or circle skips ahead (TitleLogoTask_checkSkip). */
void TitleLogoTask_stepWait(TitleLogoTask *this) {
    if (++this->stepFrame > 0x100) {
        this->stepFrame = 0;
        this->step++;
    }
    TitleLogoTask_checkSkip(this);
}

/* The US logo's entrance over 0x78 frames: brightens the logo by 3 a frame; its scale grows to 0x1800
 * (phase 0), shrinks back to 0x1000 (phase 1), then 0x6FC and 0x6FE shake with sines over a growing
 * divisor (phase 2); 0x60 follows the frame (x 80 / 60). Then clears the shake and moves to the next step.
 * Start or circle skips ahead (TitleLogoTask_checkSkip). */
void TitleLogoTask_stepShowLogo(TitleLogoTask *this) {
    s32 frame;

    if (this->stepFrame == 0) {
        entityGroupDestroyAll(&this->group);
    }
    this->logoBrightness += 3;
    if (this->logoBrightness > 0xFF) {
        this->logoBrightness = 0xFF;
    }
    switch (this->unk6FA) {
    case 0:
        this->logoScale = ((this->stepFrame << 15) / 60) >> 1;
        if (this->logoScale < 0x1800) {
            break;
        }
        this->unk6FA++;
        /* fallthrough */
    case 1:
        /* The match depends on the doubling written apart from the * 100: as * 200, gcc builds
         * another shift-and-add chain. */
        this->logoScale = 0x1800 - ((this->stepFrame - 23) << 1) * 100;
        if (this->logoScale > 0x1000) {
            break;
        }
        this->logoScale = 0x1000;
        this->unk6FA++;
        /* fallthrough */
    case 2: {
        s32 t = this->stepFrame - 34;
        s32 divisor = t * 46 + 0x80;

        this->shakeX = COS_SIN_TABLE[(t << 10) & 0xFFF].sin / divisor;
        this->shakeY = COS_SIN_TABLE[(t << 8) & 0xFFF].sin / divisor;
        break;
    }
    }
    frame = this->stepFrame;
    this->shownCopyrightLevel = frame * 80 / 60;
    this->stepFrame = frame + 1;
    if (frame >= 0x78) {
        this->stepFrame = 0;
        this->shakeX = 0;
        this->shakeY = 0;
        this->step++;
    }
    TitleLogoTask_checkSkip(this);
}
#endif

void TitleLogoTask_checkStart(TitleLogoTask *this);

/* the frames the title waits for Start before it moves on */
#if VERSION_JP
#define TITLE_WAIT_START_FRAMES 2400
#else
#define TITLE_WAIT_START_FRAMES 1680
#endif

/* the smaller of a and b; the one it returns is evaluated twice */
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/* Raises *level by 2 (MIN evaluates the increment twice) up to 80. */
static inline void raiseLevel(s32 *level) {
    *level = MIN(++*level, 80);
}

/* Waits for Start (TitleLogoTask_checkStart): starts sound 0x16 once, dims the logo by 4 a frame down to 0x80,
 * blinks the press Start prompt and raises the copyright by 2 a frame up to 80; after
 * TITLE_WAIT_START_FRAMES moves to the next step. */
void TitleLogoTask_stepWaitStart(TitleLogoTask *this) {
    if (!this->soundStarted) {
        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x16, 0);
        this->soundStarted = 1;
    }
    this->logoBrightness -= 4;
    if (this->logoBrightness < 0x80) {
        this->logoBrightness = 0x80;
    }
    this->promptPhase += 0x30;
    /* The match depends on the double increment and on the level raised by an inline that takes its address
     * (the store after the join goes through it). */
    raiseLevel(&this->shownCopyrightLevel);
    if (++this->stepFrame >= TITLE_WAIT_START_FRAMES) {
        this->stepFrame = 0;
        this->step++;
    }
    TitleLogoTask_checkStart(this);
    this->copyrightLevel = this->shownCopyrightLevel;
}

/* Grows the logo's scale by 5 times the step's frame (up to 0x8000), takes the copyright's level down by 8
 * to 0 and brightens the logo * by 0x12 (up to 0xFF); after 0x3C frames moves to the next step. */
void TitleLogoTask_stepHideLogo(TitleLogoTask *this) {
    this->logoScale += this->stepFrame * 5;
    this->copyrightLevel -= 8;
    if (this->copyrightLevel < 0) {
        this->copyrightLevel = 0;
    }
    if (this->logoScale > 0x8000) {
        this->logoScale = 0x8000;
    }
    this->logoBrightness += 0x12;
    if (this->logoBrightness > 0xFF) {
        this->logoBrightness = 0xFF;
    }
    if (++this->stepFrame > 0x3C) {
        this->step++;
    }
}

/* Starts fading the songs out (step 0x44) and moves to the next step. */
void TitleLogoTask_stepFadeSongs(TitleLogoTask *this) {
    fadeSongs(0x44);
    this->step++;
}

/* Fades the screen out, over 0x40 frames once Start was pressed (else 0x80), and ends the steps. */
void TitleLogoTask_stepFadeOut(TitleLogoTask *this) {
    fadeControlFadeOut(&FADE_CONTROL, this->startPressed != 0 ? 0x40 : 0x80);
    this->step = TITLE_LOGO_STEP_END;
}

void func_80067B60(void) {
}

void func_80067B68(void) {
}

void func_80067B70(void) {
}

void func_80067B78(void) {
}

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_initTunnel);

/* Draws the object's two parts: both callees write primitives at PRIM_BUFFER_FREE. The US build calls
 * func_8007236C first. */
void func_80067CC4(TitleLogoTask *this) {
#if VERSION_US
    func_8007236C(this);
#endif
    TitleLogoTask_drawPanels(this);
    TitleLogoTask_drawRingLines(this);
}

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_drawRingLines);

/* Draws the tunnel's walls: between each of the first five rings and the next, 32 shaded POLY_GT4 panels (page 8,
 * the CLUT at (0x200, 0xFF), US 0xFD, 16 texels of a 12-panel strip each, scrolling with spawnCounter), turned by TITLE_SPIN 0x800 in
 * front of the camera. A ring's brightness is ring * 21 / 80 of the shown level (up to 80), raised by the step's
 * frame while the logo hides. Nothing while the level is 0. */
void TitleLogoTask_drawPanels(TitleLogoTask *this) {
    MATRIX m;
    PolyGT4Shape shape;
    u16 v = ~this->spawnCounter & 0x3F;
    u8 *packet = PRIM_BUFFER_FREE;
    u_long *ot;
    s32 shift;
    s32 ring;
    s32 i;

    shape.flags = 0;
    ot = this->camera.ot.slots;
    shift = this->camera.zShift;
    if (this->shownCopyrightLevel == 0) {
        return;
    }
    shape.tpage = 8;
#if VERSION_JP
    shape.clut = getClut(0x200, 0xFF);
#else
    shape.clut = getClut(0x200, 0xFD); /* the CLUT func_8007236C pulses */
#endif
    matrixInitRotationZ(&m, TITLE_SPIN);
    m.t[0] = 0;
    m.t[1] = 0;
    m.t[2] = 0x800;
    setModelMatrix(&m);
    for (ring = 0; ring < 5; ring++) {
        u8 level = this->shownCopyrightLevel;
        s32 ringShade;
        u16 inner;
        u16 outer;

        if (level > 80) {
            level = 80;
        }
        ringShade = level * 21;
        inner = ring * ringShade / 80;
        outer = (ring + 1) * ringShade / 80;
        if (this->step == TITLE_LOGO_STEP_HIDE_LOGO) {
            inner += this->stepFrame;
            if (inner > 0xFF) {
                inner = 0xFF;
            }
            outer += this->stepFrame;
            if (outer > 0xFF) {
                outer = 0xFF;
            }
        }
        /* The match depends on this order of the stores: loop.c hoists the invariants it meets first (v + 0x3F,
         * the inner colour) and leaves the outer colour, met last, to its second pass. */
        for (i = 0; i < 32; i++) {
            s16 u = (i % 12) * 16;

            shape.u0 = u;
            shape.v0 = v;
            shape.u1 = u + 15;
            shape.v1 = v;
            shape.u2 = u;
            shape.v2 = v + 0x3F;
            shape.u3 = u + 15;
            shape.v3 = v + 0x3F;
            shape.colors[0] = shape.colors[1] = inner | (inner << 8) | (inner << 16);
            shape.corners[0] = &this->ringPoints[ring][i];
            shape.corners[1] = &this->ringPoints[ring][i + 1];
            shape.corners[2] = &this->ringPoints[ring + 1][i];
            shape.corners[3] = &this->ringPoints[ring + 1][i + 1];
            shape.colors[2] = shape.colors[3] = outer | (outer << 8) | (outer << 16);
            packet = buildPolyGT4(&shape, packet, ot, shift);
        }
    }
    PRIM_BUFFER_FREE = packet;
}

#if VERSION_JP
INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_spawnLogoPieces);

INCLUDE_ASM("title/nonmatchings/title_logo", func_800683D0);

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_drawLogo);
#else
/* Draws the logo as two POLY_FT4 halves of 204x207 texels (pages 0x8A and 0x8C), scaled by logoScale about the
 * screen's centre line and moved by the shake. The match depends on `(logoScale << 1) * 0x58`: the game multiplies
 * 88 by a register holding twice the scale (`* 0xB0` gives another shift-and-add sequence), and on the shake added
 * in its own statements (in one expression the constants fold into the shake's add). */
void TitleLogoTask_drawLogo(TitleLogoTask *this, u_long *ot) {
    s32 halfWidth;
    s32 x;
    s32 top;
    s32 height;
    POLY_FT4 *poly;

    if (this->logoScale == 0) {
        return;
    }
    halfWidth = (this->logoScale * 0xCC) >> 12;
    x = 0x100 - halfWidth;
    top = 0x112 - (((this->logoScale << 1) * 0x58) >> 12);
    x += this->shakeX;
    top += this->shakeY;
    height = (this->logoScale * 0xCF) >> 12;
    poly = allocPolyFT4();
    poly->r0 = this->logoBrightness;
    poly->g0 = this->logoBrightness;
    poly->b0 = this->logoBrightness;
    setXY4(poly, x, top, x + halfWidth, top, x, top + height, x + halfWidth, top + height);
    setUV4(poly, 0, 0, 0xCC, 0, 0, 0xCF, 0xCC, 0xCF);
    poly->tpage = 0x8A;
    poly->clut = 0x3FE8;
    AddPrim(ot, poly);

    x = this->shakeX + 0x100;
    poly = allocPolyFT4();
    poly->r0 = this->logoBrightness;
    poly->g0 = this->logoBrightness;
    poly->b0 = this->logoBrightness;
    setXY4(poly, x, top, x + halfWidth, top, x, top + height, x + halfWidth, top + height);
    setUV4(poly, 0, 0, 0xCC, 0, 0, 0xCF, 0xCC, 0xCF);
    poly->tpage = 0x8C;
    poly->clut = 0x3FE8;
    AddPrim(ot, poly);
}
#endif

#if VERSION_JP
#define PRESS_START_Y 0x110
#else
#define PRESS_START_Y 0x142
#endif

/* Draws the press Start prompt (title.tim's 255 x 31 at (0, 0x58), clut 0x7FEB) at (0x80, PRESS_START_Y),
 * semi-transparent, blinking with promptPhase: as bright as 0x60 - 3/128 of its cosine, at most 0x80. Only in the
 * steps before the logo's exit. */
void TitleLogoTask_drawPressStart(TitleLogoTask *this, u_long *ot) {
    if (this->step < TITLE_LOGO_STEP_HIDE_LOGO && this->step != TITLE_LOGO_STEP_END) {
        s32 level = 0x60 - ((COS_SIN_TABLE[this->promptPhase & 0xFFF].cos * 3) >> 7);
        u8 brightness;

        /* The match depends on this if/else order (jump sets 0x80 first, in the branch's delay slot). */
        if (level <= 0x80) {
            brightness = level;
        } else {
            brightness = 0x80;
        }

        if (brightness != 0) {
            POLY_FT4 *poly = allocPolyFT4();

            setRGB0(poly, brightness, brightness, brightness);
            setXY4(poly, 0x80, PRESS_START_Y, 0x17F, PRESS_START_Y, 0x80, PRESS_START_Y + 0x1F, 0x17F,
                   PRESS_START_Y + 0x1F);
            setUV4(poly, 0, 0x58, 0xFF, 0x58, 0, 0x77, 0xFF, 0x77);
            poly->tpage = 0x3A;
            poly->clut = 0x7FEB;
            SetSemiTrans(poly, 1);
            AddPrim(ot, poly);
        }
    }
}

/* On Start or circle on either pad: destroys the group's entities and moves to the wait for Start, the
 * logo at full (brightness 0xFF, scale 0x1000, 0x60 = 0x50); the US build also clears 0x110C and the
 * shake at 0x6FC and 0x6FE. */
void TitleLogoTask_checkSkip(TitleLogoTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on both presses or-ed in one expression: the first result then takes the pad
     * manager's register (a variable for it costs a third one). */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & (PADstart | PADRright)) {
        entityGroupDestroyAll(&this->group);
        this->logoBrightness = 0xFF;
        this->logoScale = 0x1000;
        this->shownCopyrightLevel = 0x50;
        this->step = TITLE_LOGO_STEP_WAIT_START;
#if VERSION_US
        this->unk110C = 0;
        this->shakeX = 0;
        this->shakeY = 0;
#endif
    }
}

/* On Start on either pad: moves to the logo's exit (startPressed set, the prompt's phase and stepFrame
 * cleared). */
void TitleLogoTask_checkStart(TitleLogoTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on one expression, as in TitleLogoTask_checkSkip. */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & PADstart) {
        this->startPressed = 1;
        this->promptPhase = 0;
        this->step = TITLE_LOGO_STEP_HIDE_LOGO;
        this->stepFrame = 0;
    }
}

#if VERSION_JP
/* TitleLogoPiece's phases (TitleLogoPiece_tick) */
#define TITLE_LOGO_PIECE_WAIT 0   /* until the timer, negative, reaches 0 */
#define TITLE_LOGO_PIECE_ORBIT 1  /* circles (256, 240) for 200 frames, the radius easing to its target */
#define TITLE_LOGO_PIECE_RETURN 2 /* flies back to its home and stops spinning */
#define TITLE_LOGO_PIECE_REST 3   /* at home for 0x80 frames, then deleted */

/* The entity class of vtable TITLE_LOGO_PIECE_VTABLE (TitleLogoPiece_init, Japan only): an 8x8 square of the
 * logo's texture that waits, circles the screen's centre, then flies back to its place in the logo, spinning.
 * Positions are 20.12 fixed point. */
typedef struct TitleLogoPiece {
    /* 0x00 */ Entity entity;
    /* 0x18 */ union {
        struct {
            s32 angle;        /* about (256, 240), 0x1000 a turn */
            s32 radius;
            s32 speed;        /* added to the angle each frame, -15 to 15 */
            s32 targetRadius; /* the radius eases to it, 0 to 426 */
        } orbit;
        struct {
            s32 x;
            s32 y;
            s32 vx; /* the way home over the return's frames */
            s32 vy;
        } flight;
    } move; /* by phase: the orbit, then the flight home */
    /* 0x28 */ s32 homeX; /* its place in the logo: (u, v) from (159, 120) */
    /* 0x2C */ s32 homeY;
    /* 0x30 */ s16 phase; /* TITLE_LOGO_PIECE_* */
    /* 0x32 */ s16 timer;
    /* 0x34 */ u8 u;      /* of its square of the logo's texture */
    /* 0x35 */ u8 v;
    /* 0x36 */ u8 unk36[2];
    /* 0x38 */ s32 spin;  /* the square's angle about z */
} TitleLogoPiece; /* size 0x3C */

/* The constructor of the entity class of vtable TITLE_LOGO_PIECE_VTABLE: the piece at (u, v) of the logo, with a
 * random start angle, orbit speed and radius, and a random wait of up to 63 frames. */
TitleLogoPiece *TitleLogoPiece_init(TitleLogoPiece *this, TitleLogoTask *owner, s16 u, s16 v) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &TITLE_LOGO_PIECE_VTABLE;
    this->move.orbit.angle = mersenneTwisterGenerate(&owner->random) & 0xFFF;
    this->move.orbit.radius = 0;
    this->move.orbit.speed = (s32)mersenneTwisterGenerate(&owner->random) % 16;
    this->move.orbit.targetRadius = mersenneTwisterGenerate(&owner->random) % (426 << 12);
    this->homeX = (u + 159) << 12;
    this->homeY = (v + 120) << 12;
    this->phase = TITLE_LOGO_PIECE_WAIT;
    this->timer = -(mersenneTwisterGenerate(&owner->random) & 0x3F);
    this->u = u;
    this->v = v;
    this->spin = 0;
    return this;
}

/* The destructor of the entity class of vtable TITLE_LOGO_PIECE_VTABLE, which TitleLogoPiece_init installs. */
void TitleLogoPiece_destroy(Entity *this, s32 flags) {
    this->vtable = &TITLE_LOGO_PIECE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The piece's tick: waits for its timer; orbits for 200 frames (spinning, the angle moved by speed, the radius
 * easing to its target), then flies home from its orbit's point over 64 to 127 frames, facing the way, its spin
 * slowing; rests at home for 0x80 frames and deletes itself. */
void TitleLogoPiece_tick(TitleLogoPiece *this, TitleLogoTask *owner) {
    s32 x;
    s32 y;
    s32 sin; /* The match depends on the sine read before the ellipse's height is divided. */

    switch (this->phase) {
    case TITLE_LOGO_PIECE_WAIT:
        if (++this->timer >= 0) {
            this->phase++;
        }
        break;
    case TITLE_LOGO_PIECE_ORBIT:
        this->spin += this->move.orbit.speed << 4;
        this->move.orbit.angle += this->move.orbit.speed;
        this->move.orbit.radius += (this->move.orbit.targetRadius - this->move.orbit.radius) >> 6;
        if (++this->timer >= 200) {
            this->timer = (mersenneTwisterGenerate(&owner->random) & 0x3F) + 0x40;
            /* the orbit's point, where the flight starts (the flight's fields share the orbit's) */
            x = (this->move.orbit.radius >> 12) * COS_SIN_TABLE[this->move.orbit.angle & 0xFFF].cos + (256 << 12);
            sin = COS_SIN_TABLE[this->move.orbit.angle & 0xFFF].sin;
            y = (this->move.orbit.radius * 480 / 512 >> 12) * sin + (240 << 12);
            this->move.flight.x = x;
            this->move.flight.y = y;
            this->move.flight.vx = (this->homeX - x) / this->timer;
            this->move.flight.vy = (this->homeY - y) / this->timer;
            this->spin = ratan2(this->move.flight.vy, this->move.flight.vx);
            this->phase++;
        }
        break;
    case TITLE_LOGO_PIECE_RETURN:
        this->spin -= this->spin >> 4;
        this->move.flight.x += this->move.flight.vx;
        this->move.flight.y += this->move.flight.vy;
        if (--this->timer <= 0) {
            this->timer = 0;
            this->move.flight.vx = 0;
            this->move.flight.vy = 0;
            this->spin = 0;
            this->phase++;
            this->move.flight.x = this->homeX;
            this->move.flight.y = this->homeY;
        }
        break;
    default:
        if (++this->timer >= 0x80) {
            entityDelete(&this->entity);
        }
        break;
    }
}

/* Draws the piece (nothing while it waits): its 8x8 square of the logo at (u, v) of page 0x9A, turned by spin
 * about its centre, at its orbit's point (an ellipse around (256, 240), 480/512 as high as wide) or its flight's. */
void TitleLogoPiece_draw(TitleLogoPiece *this) {
    u_long *ot = FRAME_OT.firstDrawn + 3;
    s16 x;
    s32 y;

    if (this->phase == TITLE_LOGO_PIECE_WAIT) {
        return;
    }
    if (this->phase == TITLE_LOGO_PIECE_ORBIT) {
        s32 sin; /* The match depends on the sine read before the ellipse's height is divided. */

        x = ((this->move.orbit.radius >> 12) * COS_SIN_TABLE[this->move.orbit.angle & 0xFFF].cos >> 12) + 256;
        sin = COS_SIN_TABLE[this->move.orbit.angle & 0xFFF].sin;
        y = ((this->move.orbit.radius * 480 / 512 >> 12) * sin >> 12) + 240;
    } else {
        x = this->move.flight.x >> 12;
        y = (s16)(this->move.flight.y >> 12);
    }
    {
        /* The match depends on the corners initialized here, after the position. */
        VECTOR corners[4] = { { -4, -4, 0 }, { 4, -4, 0 }, { -4, 4, 0 }, { 4, 4, 0 } };
        MatrixWords m;
        POLY_FT4 *poly;

        m.words[0] = ONE;
        m.words[1] = 0;
        m.words[2] = ONE;
        m.words[3] = 0;
        m.words[4] = ONE;
        m.m.t[0] = x;
        m.m.t[1] = y;
        m.m.t[2] = 0;
        matrixSetRotationZ(&m.m, this->spin);
        matrixTransformVecs(&m.m, corners, corners, 4);
        poly = allocPolyFT4();
        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = corners[0].vx;
        poly->y0 = corners[0].vy;
        poly->x1 = corners[1].vx;
        poly->y1 = corners[1].vy;
        poly->x2 = corners[2].vx;
        poly->y2 = corners[2].vy;
        poly->x3 = corners[3].vx;
        poly->y3 = corners[3].vy;
        poly->u0 = this->u;
        poly->v0 = this->v;
        poly->u1 = this->u + 8;
        poly->v1 = this->v;
        poly->u2 = this->u;
        poly->v2 = this->v + 8;
        poly->u3 = this->u + 8;
        poly->v3 = this->v + 8;
        poly->tpage = 0x9A;
        poly->clut = 0x3FE8;
        AddPrim(ot, poly);
    }
}

/* the (unk30, unk31) pair of each index of Unk800645E4. The match depends on the const: its loads move above
 * the stores to the object. */
extern const u8 D_80064664[][2];

/* Unk800645E4's constructor: starts at a random point of the ellipse around (256, 240) (radii 0x180 and 0x1E0
 * in 20.12, the angle spread by index), heading there in 0x78 frames after a delay of 20 frames per index. */
Unk800645E4 *func_80068E0C(Unk800645E4 *this, TitleLogoTask *owner, s32 index, u16 unk34) {
    s32 angle;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800645E4;
    angle = index * 0x249 + (mersenneTwisterGenerate(&owner->random) & 0x7FF);
    this->x = 0x100000 - COS_SIN_TABLE[angle & 0xFFF].cos * 0x180;
    this->y = 0xF0000 - COS_SIN_TABLE[angle & 0xFFF].sin * 0x1E0;
    this->vx = (0x100000 - this->x) / 0x78;
    this->vy = (0xF0000 - this->y) / 0x78;
    this->scale = 0x1000;
    this->frame = -index * 20;
    this->unk30 = D_80064664[index][0];
    this->unk31 = D_80064664[index][1];
    this->brightness = 0xF0;
    this->unk34 = unk34;
    return this;
}

/* The destructor of the entity class of vtable D_800645E4, which func_80068E0C installs. */
void func_80068F6C(Entity *this, s32 flags) {
    this->vtable = &D_800645E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800645E4's tick: once its delay is over it moves, grows by 0x33 and fades by 1 a frame; it deletes
 * itself after frame 0xF0. */
void func_80068FC0(Unk800645E4 *this) {
    if (this->frame > 0) {
        this->x += this->vx;
        this->y += this->vy;
        this->scale += 0x33;
        this->brightness--;
    }
    if (++this->frame > 0xF0) {
        entityDelete(&this->entity);
    }
}

/* Unk800645E4's draw, from frame 0: a semi-transparent POLY_FT4 of the unk31 by 60 texels at (unk30, 0xB2) of
 * page 0x2A, clut unk34, scaled by scale and centred on (x, y), its shade brightness * 192 / 240. */
void func_80069064(Unk800645E4 *this) {
    u_long *ot;
    s16 w, h, left, top;
    u8 shade;
    POLY_FT4 *poly;

    if (this->frame < 0) {
        return;
    }
    ot = FRAME_OT.firstDrawn + 3;
    w = (this->scale * this->unk31) >> 12;
    h = (this->scale * 60) >> 12;
    left = (this->x >> 12) - (w >> 1);
    top = (this->y >> 12) - (h >> 1);
    shade = this->brightness * 192 / 240;
    poly = allocPolyFT4();
    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    poly->x0 = left;
    poly->y0 = top;
    poly->x1 = left + w;
    poly->y1 = top;
    poly->x2 = left;
    poly->y2 = top + h;
    poly->x3 = left + w;
    poly->y3 = top + h;
    poly->u0 = this->unk30;
    poly->v0 = 0xB2;
    poly->u1 = this->unk30 + this->unk31;
    poly->v1 = 0xB2;
    poly->u2 = this->unk30;
    poly->v2 = 0xEE;
    poly->u3 = this->unk30 + this->unk31;
    poly->v3 = 0xEE;
    poly->tpage = 0x2A;
    poly->clut = this->unk34;
    SetSemiTrans(poly, 1);
    AddPrim(ot, poly);
}
#endif

/* The constructor of the entity class of vtable TITLE_STREAK_VTABLE: at the origin of the owner (the title
 * logo task), moving at 1/512 of the sine and cosine of a random angle. */
TitleStreak *TitleStreak_init(TitleStreak *this, TitleLogoTask *owner) {
    u32 angle;

    entityInit(&this->entity, 0);
    this->entity.vtable = &TITLE_STREAK_VTABLE;
    this->owner = owner;
    this->position.vx = 0;
    this->position.vy = 0;
    this->position.vz = 0;
    this->position.pad = 0;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocity.pad = 0;
    angle = mersenneTwisterGenerate(&owner->random);
    this->velocity.vx = COS_SIN_TABLE[angle & 0xFFF].sin >> 9;
    this->velocity.vy = COS_SIN_TABLE[angle & 0xFFF].cos >> 9;
    return this;
}

/* The destructor of the entity class of vtable TITLE_STREAK_VTABLE, which TitleStreak_init installs. */
void TitleStreak_destroy(Entity *this, s32 flags) {
    this->vtable = &TITLE_STREAK_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves the entity by its velocity and deletes it once it is more than 0x400 away in x or y.
 * The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
void TitleStreak_tick(TitleStreak *this) {
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;

    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    if (ABS(x) > 0x400 || ABS(y) > 0x400) {
        entityDelete(&this->entity);
    }
}

/* Draws the streak as an additive line from its position back to 8 times its position less its velocity, black
 * to orange, in the owner's camera turned by TITLE_SPIN about z. */
void TitleStreak_draw(TitleStreak *this) {
    SVECTOR from;
    SVECTOR to;
    MATRIX rotation;
    LineG2Shape shape;
    u_long *ot;
    s32 shift;
    u8 *packet;
    s16 x;
    s16 y;

#if VERSION_US
    if (this->owner->unk110C == 0) {
        return;
    }
#endif
    ot = this->owner->camera.ot.slots;
    shift = this->owner->camera.zShift;
    packet = PRIM_BUFFER_FREE;
    shape.flags = 0;
    matrixInitRotationZ(&rotation, TITLE_SPIN);
    setModelMatrix(&rotation);
    from.vz = 0;
    to.vz = 0;
    shape.ends[0] = &from;
    /* The match depends on x and y as s16 locals: they are read with lh, the line's end with lhu. */
    x = this->position.vx;
    y = this->position.vy;
    from.vx = x;
    from.vy = y;
    shape.ends[1] = &to;
    to.vx = (this->position.vx - this->velocity.vx) << 3;
    to.vy = (this->position.vy - this->velocity.vy) << 3;
    shape.colors[0] = 0;
    shape.colors[1] = 0x806020;
    shape.flags = (shape.flags & ~SPRITE_ABR_MASK) | SPRITE_SEMI_TRANS | 1;
    PRIM_BUFFER_FREE = buildLineG2(&shape, packet, ot, shift);
}

/* The constructor of the entity class of vtable TITLE_SPARKLE_VTABLE: TitleStreak_init's, at 1/256 of the
 * sine and cosine, keeping the random number, the speed and a distance and frame of 0.
 * The match depends on the velocity's address taken into a local right after the owner is set (it stays in $s1
 * across the random call) and on the last three fields cleared in the order brightness, distance, frame. */
TitleSparkle *TitleSparkle_init(TitleSparkle *this, TitleLogoTask *owner) {
    VECTOR *velocity;

    entityInit(&this->entity, 0);
    this->entity.vtable = &TITLE_SPARKLE_VTABLE;
    this->owner = owner;
    velocity = &this->velocity;
    this->position.vx = 0;
    this->position.vy = 0;
    this->position.vz = 0;
    this->position.pad = 0;
    this->velocity.vx = 0;
    this->velocity.vy = 0;
    this->velocity.vz = 0;
    this->velocity.pad = 0;
    this->randomAngle = mersenneTwisterGenerate(&owner->random);
    this->velocity.vx = COS_SIN_TABLE[this->randomAngle & 0xFFF].sin >> 8;
    this->velocity.vy = COS_SIN_TABLE[this->randomAngle & 0xFFF].cos >> 8;
    this->speed = vecGetLength(velocity);
    this->brightness = 0;
    this->distance = 0;
    this->frame = 0;
    return this;
}

/* The destructor of the entity class of vtable TITLE_SPARKLE_VTABLE, which TitleSparkle_init installs. */
void TitleSparkle_destroy(Entity *this, s32 flags) {
    this->vtable = &TITLE_SPARKLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves the entity on by its speed: brightens it by 1 a frame up to 0x80 for its first 0x200, then a quarter
 * of the way to 0x80 a frame; deletes it past 0x400, else counts its frames. */
void TitleSparkle_tick(TitleSparkle *this) {
    this->distance += this->speed;
    if (this->distance < 0x200) {
        this->brightness++;
        if (this->brightness > 0x80) {
            this->brightness = 0x80;
        }
    } else {
        this->brightness += (0x80 - this->brightness) >> 2;
    }
    if (this->distance > 0x400) {
        entityDelete(&this->entity);
    } else {
        this->frame++;
    }
}

INCLUDE_ASM("title/nonmatchings/title_logo", TitleSparkle_draw);

/* Update: keeps arg and runs the task's tick. */
void TitleLogoTask_update(TickTask *this, s32 arg) {
    this->updateArg = arg;
    this->vtable->tick.func((u8 *)this + this->vtable->tick.delta, arg);
}

#if VERSION_US
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_UP);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_TARGET);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_CAMERA_EYE);
INCLUDE_RODATA("title/nonmatchings/title_logo", PATH_TITLE_TIM);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_LOGO_STEPS);
#endif
INCLUDE_RODATA("title/nonmatchings/title_logo", SPARKLE_CORNERS);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_SPARKLE_VTABLE);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_STREAK_VTABLE);
#if VERSION_JP
INCLUDE_RODATA("title/nonmatchings/title_logo", D_800645E4);
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_LOGO_PIECE_VTABLE);
#endif
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_LOGO_TASK_VTABLE);
#if VERSION_JP
INCLUDE_RODATA("title/nonmatchings/title_logo", D_80064664);
#endif
