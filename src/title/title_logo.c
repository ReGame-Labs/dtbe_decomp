/* The title screen (the task "Title Logo", title/title.tim). */

#include "common.h"
#include <abs.h>
#include <libetc.h>
#include "engine/debug/system_menu.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/sprite.h"
#include "engine/math/random.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/sound/sound.h"
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
    /* 0x003C */ u8 unk3C[0x44 - 0x3C];
#else
    /* 0x0024 */ RECT clutRect;  /* 16 by 1 pixels at (0x200, 0xFD): a CLUT */
    /* 0x002C */ u16 clut[16];   /* that CLUT, read back from VRAM by TitleLogoTask_readClut */
    /* 0x004C */ u16 clutPhase; /* the phase of the sine func_8007236C pulses the CLUT with */
    /* 0x004E */ u8 unk4E[0x50 - 0x4E];
    /* 0x0050 */ EntityGroup group;
    /* 0x0060 */ Sprite *sprites[3];
#endif
    /* 0x0044 */ s32 startPressed; /* Start was pressed on the title */
    /* 0x0048 */ u8 unk48[0x4C - 0x48];
    /* 0x004C */ s32 stepFrame; /* frames since the step began */
    /* 0x0050 */ s32 spawnRate;      /* added to spawnCounter a frame: 2, up to 8 from the logo's exit */
    /* 0x0054 */ s32 logoBrightness; /* 0x80 up to 0xFF */
    /* 0x0058 */ s32 logoScale;      /* 0x1000 the logo's own size */
    /* 0x005C */ s32 promptPhase;    /* the angle the press Start prompt blinks with */
    /* 0x0060 */ s32 shownCopyrightLevel; /* the copyright's level while the logo shows (80 full); Japan copies it into
                                         * copyrightLevel each frame */
#if VERSION_JP
    /* 0x0064 */ MATRIX view;
    /* 0x0084 */ SVECTOR ringPoints[6][33]; /* the tunnel's rings (TitleLogoTask_initTunnel) */
    /* 0x06B4 */ s16 unk6B4; /* which entities TitleLogoTask_tick makes */
    /* 0x06B6 */ u8 unk6B6[0x6B8 - 0x6B6];
    /* 0x06B8 */ s32 ringScales[16];
#else
    /* 0x0064 */ u8 unk64[0x720 - 0x8C];
#endif
    /* 0x06F8 */ s16 copyrightLevel; /* the copyright's brightness, 80 for full */
    /* 0x06FA */ s16 unk6FA; /* the US logo's phase (TitleLogoTask_stepShowLogo) */
#if VERSION_US
    /* 0x06FC */ s16 unk6FC; /* the US logo's shake */
    /* 0x06FE */ s16 unk6FE;
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

INCLUDE_ASM("title/nonmatchings/title_logo", titleLogoTaskInit);

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

#if VERSION_JP
/* A g++ 2.95 pointer to member function: index -1 for a non-virtual function, which is then func; else
 * virtual, entry index - 1 of the virtual table at vtableOffset in the object. */
typedef struct MemberFunction {
    /* 0x0 */ s16 delta; /* how far to move `this` */
    /* 0x2 */ s16 index;
    union {
        /* 0x4 */ void (*func)();
        /* 0x4 */ s16 vtableOffset;
    } u;
} MemberFunction;

/* A virtual table entry that fits a register pair (g++ keeps the one a pointer to member function reaches in
 * one). */
typedef union VirtualFunction {
    VtableEntry entry;
    u64 pair;
} VirtualFunction;

/* the title logo task's step functions, by step */
extern MemberFunction TITLE_LOGO_STEPS[];
/* an angle that turns by 4 a frame, and the sum of its sines (at 1/256) */
extern s32 TITLE_SPIN_PHASE;
extern s32 TITLE_SPIN;

void func_80067CC4(TitleLogoTask *this);
void TitleLogoTask_drawLogo(TitleLogoTask *this, u_long *ot);
void TitleLogoTask_drawPressStart(TitleLogoTask *this, u_long *ot);
TitleStreak *TitleStreak_init(TitleStreak *this, TitleLogoTask *owner);
TitleSparkle *TitleSparkle_init(TitleSparkle *this, TitleLogoTask *owner);

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
    if (this->unk6B4 != 0) {
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

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_stepShowLogo);
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

INCLUDE_ASM("title/nonmatchings/title_logo", func_8007240C);

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

        this->unk6FC = COS_SIN_TABLE[(t << 10) & 0xFFF].sin / divisor;
        this->unk6FE = COS_SIN_TABLE[(t << 8) & 0xFFF].sin / divisor;
        break;
    }
    }
    frame = this->stepFrame;
    this->shownCopyrightLevel = frame * 80 / 60;
    this->stepFrame = frame + 1;
    if (frame >= 0x78) {
        this->stepFrame = 0;
        this->unk6FC = 0;
        this->unk6FE = 0;
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

/* Waits for Start (TitleLogoTask_checkStart): starts sound 0x16 once, dims the logo by 4 a frame down to 0x80,
 * blinks the press Start prompt and raises the copyright by 2 a frame up to 80; after
 * TITLE_WAIT_START_FRAMES moves to the next step. */
void TitleLogoTask_stepWaitStart(TitleLogoTask *this) {
    s32 *level;

    if (!this->soundStarted) {
        this->sound = soundControlPlaySound(&SOUND_CONTROL, 0x16, 0);
        this->soundStarted = 1;
    }
    this->logoBrightness -= 4;
    if (this->logoBrightness < 0x80) {
        this->logoBrightness = 0x80;
    }
    this->promptPhase += 0x30;
    /* MIN evaluates the increment twice: the copyright rises by 2 a frame below 80. The match depends on
     * that, and on the level written through a pointer taken here (the store after the join goes through it). */
    level = &this->shownCopyrightLevel;
    *level = MIN(++*level, 80);
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

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_drawPanels);

#if VERSION_JP
INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_spawnLogoPieces);

INCLUDE_ASM("title/nonmatchings/title_logo", func_800683D0);

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_drawLogo);
#else
INCLUDE_ASM("title/nonmatchings/title_logo", func_80073438);
#endif

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoTask_drawPressStart);

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
        this->unk6FC = 0;
        this->unk6FE = 0;
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
INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoPiece_init);

/* The destructor of the entity class of vtable TITLE_LOGO_PIECE_VTABLE, which TitleLogoPiece_init installs. */
void TitleLogoPiece_destroy(Entity *this, s32 flags) {
    this->vtable = &TITLE_LOGO_PIECE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoPiece_tick);

INCLUDE_ASM("title/nonmatchings/title_logo", TitleLogoPiece_draw);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80068E0C);

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

INCLUDE_ASM("title/nonmatchings/title_logo", func_80069064);
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

INCLUDE_ASM("title/nonmatchings/title_logo", TitleStreak_draw);

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

#if VERSION_JP
INCLUDE_RODATA("title/nonmatchings/title_logo", TITLE_LOGO_STEPS);
INCLUDE_RODATA("title/nonmatchings/title_logo", LOGO_PIECE_CORNERS);
#else
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
