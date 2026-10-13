/* The title screen (the task "Title Logo", title/title.tim). */

#include "common.h"
#include <abs.h>
#include <libetc.h>
#include "engine/debug/system_menu.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/display.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/sprite.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/sound/sound.h"
#include "engine/task/entity.h"
#include "title/title_classes.h"

/* the virtual tables of the entity classes below */
extern EntityVtable D_80064594;
extern EntityVtable D_800645BC;
extern EntityVtable D_800645E4;
extern EntityVtable D_8006460C;

/* The title logo task (titleLogoTaskInit, virtual table D_80064634), which func_800674F0 runs in
 * Japan. The US title screen was rewritten: its object has another sprite and other fields, so the layout
 * is per version. Offsets and unk names are the Japanese ones; the US object has 0x28 more bytes before
 * startPressed and 0x2C more before camera. */
typedef struct Unk800674F0 {
    /* 0x0000 */ Task task;
    /* 0x0020 */ s32 unk20;
#if VERSION_JP
    /* 0x0024 */ EntityGroup group;
    /* 0x0034 */ Sprite *sprites[2];
    /* 0x003C */ u8 unk3C[0x44 - 0x3C];
#else
    /* 0x0024 */ RECT unk24;     /* 16 by 1 pixels at (0x200, 0xFD): a CLUT */
    /* 0x002C */ u16 unk2C[16];  /* that CLUT, read back from VRAM by func_80072320 */
    /* 0x004C */ s16 unk4C;
    /* 0x004E */ u8 unk4E[0x50 - 0x4E];
    /* 0x0050 */ EntityGroup group;
    /* 0x0060 */ Sprite *sprites[3];
#endif
    /* 0x0044 */ s32 startPressed; /* Start was pressed on the title */
    /* 0x0048 */ u8 unk48[0x4C - 0x48];
    /* 0x004C */ s32 stepFrame; /* frames since the step began */
    /* 0x0050 */ u8 unk50[0x54 - 0x50];
    /* 0x0054 */ s32 unk54;
    /* 0x0058 */ s32 unk58;
    /* 0x005C */ s32 unk5C;
    /* 0x0060 */ s32 unk60;
#if VERSION_JP
    /* 0x0064 */ u8 unk64[0x6F8 - 0x64];
#else
    /* 0x0064 */ u8 unk64[0x720 - 0x8C];
#endif
    /* 0x06F8 */ s16 unk6F8;
    /* 0x06FA */ s16 unk6FA; /* the US logo's phase (func_80072860) */
    /* 0x06FC */ s16 unk6FC;
    /* 0x06FE */ s16 unk6FE;
#if VERSION_JP
    /* 0x0700 */ u8 unk700[0x10C0 - 0x700];
#else
    /* 0x0700 */ u8 unk700[0x10EC - 0x728];
#endif
    /* 0x10C0 */ Camera camera;
    /* 0x10FC */ s32 step;
    /* 0x1100 */ u8 unk1100[0x1108 - 0x1100];
    /* 0x1108 */ u32 sound; /* a sound's handle (stopSound) */
#if VERSION_US
    /* 0x110C */ s16 unk110C; /* 0x1138 in the US object, which alone has it */
#endif
} Unk800674F0;

/* the title logo task's virtual table */
extern Unk8006713CVtable D_80064634;

/* The entity class of vtable D_800645BC (func_80069218), which moves until it is out of the title. */
typedef struct Unk80069218 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800674F0 *owner;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 velocity; /* from a random angle */
    /* 0x38 */ s32 unk38;
} Unk80069218;

/* The entity class of vtable D_80064594 (func_800694B8): Unk80069218's fields, then its speed (the
 * velocity's length) and the distance it went. */
typedef struct Unk800694B8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800674F0 *owner;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 velocity;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s16 frame;
    /* 0x3E */ s16 brightness; /* rises to 0x80 */
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 speed;
    /* 0x48 */ s32 distance;
} Unk800694B8;

/* Destroys a sprite and frees it, as `delete` does: nothing for NULL. */
static inline void spriteDelete(Sprite *sprite) {
    if (sprite != NULL) {
        spriteDestroy(sprite, DESTROY_DELETE);
    }
}

void func_80067CF0(void *this);
void func_8006803C(void *this);
void func_800686D8(Unk800674F0 *this);
#if VERSION_US
void func_8007236C(Unk800674F0 *this);
#endif

INCLUDE_ASM("title/nonmatchings/title_logo", titleLogoTaskInit);

/* The destructor of the title logo task: stops its sound, gives the display back its 320 x 240 size, tells
 * the system whether Start was left unpressed (SYSTEM_CONTEXT.unk18), deletes the sprites, unloads the
 * sounds, fades the screen in, then destroys the camera, the entities and the task. The US object has a
 * third sprite and deletes them in the other order. */
void func_800673F8(Unk800674F0 *this, s32 flags) {
    Display *display;

    this->task.vtable = &D_80064634.task;
    stopSound(this->sound);
    display = CURRENT_DISPLAY;
    displayEnable(display, 0);
    displaySetSize(display, 0x140, 0xF0);
    SYSTEM_CONTEXT.unk18 = this->startPressed == 0;
#if VERSION_JP
    spriteDelete(this->sprites[1]);
    spriteDelete(this->sprites[0]);
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
INCLUDE_ASM("title/nonmatchings/title_logo", func_800674F0);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80067808);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80067864);
#else
/* Reads the 16-colour CLUT at (0x200, 0xFD) back from VRAM into 0x2C. */
void func_80072320(Unk800674F0 *this) {
    this->unk24.x = 0x200;
    this->unk24.y = 0xFD;
    this->unk4C = 0;
    this->unk24.w = 16;
    this->unk24.h = 1;
    StoreImage(&this->unk24, (u_long *)this->unk2C);
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_8007236C);

INCLUDE_ASM("title/nonmatchings/title_logo", func_8007240C);

/* Waits 0x100 frames, then moves to the next step; Start or circle skips ahead (func_800686D8). */
void func_80072814(Unk800674F0 *this) {
    if (++this->stepFrame > 0x100) {
        this->stepFrame = 0;
        this->step++;
    }
    func_800686D8(this);
}

/* The US logo's entrance over 0x78 frames: brightens 0x54 by 3 a frame; 0x58 grows to 0x1800 (phase 0),
 * shrinks back to 0x1000 (phase 1), then 0x6FC and 0x6FE shake with sines over a growing divisor
 * (phase 2); 0x60 follows the frame (x 80 / 60). Then clears the shake and moves to the next step.
 * Start or circle skips ahead (func_800686D8). */
void func_80072860(Unk800674F0 *this) {
    s32 frame;

    if (this->stepFrame == 0) {
        entityGroupDestroyAll(&this->group);
    }
    this->unk54 += 3;
    if (this->unk54 > 0xFF) {
        this->unk54 = 0xFF;
    }
    switch (this->unk6FA) {
    case 0:
        this->unk58 = ((this->stepFrame << 15) / 60) >> 1;
        if (this->unk58 < 0x1800) {
            break;
        }
        this->unk6FA++;
        /* fallthrough */
    case 1:
        /* The match depends on the doubling written apart from the * 100: as * 200, gcc builds
         * another shift-and-add chain. */
        this->unk58 = 0x1800 - ((this->stepFrame - 23) << 1) * 100;
        if (this->unk58 > 0x1000) {
            break;
        }
        this->unk58 = 0x1000;
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
    this->unk60 = frame * 80 / 60;
    this->stepFrame = frame + 1;
    if (frame >= 0x78) {
        this->stepFrame = 0;
        this->unk6FC = 0;
        this->unk6FE = 0;
        this->step++;
    }
    func_800686D8(this);
}
#endif

INCLUDE_ASM("title/nonmatchings/title_logo", func_8006795C);

/* Grows 0x58 by 5 times the step's frame (up to 0x8000), takes 0x6F8 down by 8 to 0 and brightens 0x54
 * by 0x12 (up to 0xFF); after 0x3C frames moves to the next step. */
void func_80067A3C(Unk800674F0 *this) {
    this->unk58 += this->stepFrame * 5;
    this->unk6F8 -= 8;
    if (this->unk6F8 < 0) {
        this->unk6F8 = 0;
    }
    if (this->unk58 > 0x8000) {
        this->unk58 = 0x8000;
    }
    this->unk54 += 0x12;
    if (this->unk54 > 0xFF) {
        this->unk54 = 0xFF;
    }
    if (++this->stepFrame > 0x3C) {
        this->step++;
    }
}

/* Starts fading the songs out (step 0x44) and moves to the next step. */
void func_80067AE0(Unk800674F0 *this) {
    fadeSongs(0x44);
    this->step++;
}

/* Fades the screen out, over 0x40 frames once Start was pressed (else 0x80), and ends the steps (-1). */
void func_80067B14(Unk800674F0 *this) {
    fadeControlFadeOut(&FADE_CONTROL, this->startPressed != 0 ? 0x40 : 0x80);
    this->step = -1;
}

void func_80067B60(void) {
}

void func_80067B68(void) {
}

void func_80067B70(void) {
}

void func_80067B78(void) {
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_80067B80);

/* Draws the object's two parts: both callees write primitives at PRIM_BUFFER_FREE. The US build calls
 * func_8007236C first. */
void func_80067CC4(void *this) {
#if VERSION_US
    func_8007236C(this);
#endif
    func_8006803C(this);
    func_80067CF0(this);
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_80067CF0);

INCLUDE_ASM("title/nonmatchings/title_logo", func_8006803C);

#if VERSION_JP
INCLUDE_ASM("title/nonmatchings/title_logo", func_80068320);

INCLUDE_ASM("title/nonmatchings/title_logo", func_800683D0);

INCLUDE_ASM("title/nonmatchings/title_logo", func_800683DC);
#else
INCLUDE_ASM("title/nonmatchings/title_logo", func_80073438);
#endif

INCLUDE_ASM("title/nonmatchings/title_logo", func_800685A8);

/* On Start or circle on either pad: destroys the group's entities and moves to step 2 (0x54 = 0xFF,
 * 0x58 = 0x1000, 0x60 = 0x50); the US build also clears 0x110C and the shake at 0x6FC and 0x6FE. */
void func_800686D8(Unk800674F0 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on both presses or-ed in one expression: the first result then takes the pad
     * manager's register (a variable for it costs a third one). */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & (PADstart | PADRright)) {
        entityGroupDestroyAll(&this->group);
        this->unk54 = 0xFF;
        this->unk58 = 0x1000;
        this->unk60 = 0x50;
        this->step = 2;
#if VERSION_US
        this->unk110C = 0;
        this->unk6FC = 0;
        this->unk6FE = 0;
#endif
    }
}

/* On Start on either pad: moves to step 3 (startPressed set, 0x5C and stepFrame cleared). */
void func_8006875C(Unk800674F0 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on one expression, as in func_800686D8. */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & PADstart) {
        this->startPressed = 1;
        this->unk5C = 0;
        this->step = 3;
        this->stepFrame = 0;
    }
}

#if VERSION_JP
INCLUDE_ASM("title/nonmatchings/title_logo", func_800687CC);

/* The destructor of the entity class of vtable D_8006460C, which func_800687CC installs. */
void func_800688E8(Entity *this, s32 flags) {
    this->vtable = &D_8006460C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_8006893C);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80068BB4);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80068E0C);

/* The destructor of the entity class of vtable D_800645E4, which func_80068E0C installs. */
void func_80068F6C(Entity *this, s32 flags) {
    this->vtable = &D_800645E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("title/nonmatchings/title_logo", func_80068FC0);

INCLUDE_ASM("title/nonmatchings/title_logo", func_80069064);
#endif

INCLUDE_ASM("title/nonmatchings/title_logo", func_80069218);

/* The destructor of the entity class of vtable D_800645BC, which func_80069218 installs. */
void func_800692C0(Entity *this, s32 flags) {
    this->vtable = &D_800645BC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves the entity by its velocity and deletes it once it is more than 0x400 away in x or y.
 * The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
void func_80069314(Unk80069218 *this) {
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

INCLUDE_ASM("title/nonmatchings/title_logo", func_800693B0);

INCLUDE_ASM("title/nonmatchings/title_logo", func_800694B8);

/* The destructor of the entity class of vtable D_80064594, which func_800694B8 installs. */
void func_80069580(Entity *this, s32 flags) {
    this->vtable = &D_80064594;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Moves the entity on by its speed: brightens it by 1 a frame up to 0x80 for its first 0x200, then a quarter
 * of the way to 0x80 a frame; deletes it past 0x400, else counts its frames. */
void func_800695D4(Unk800694B8 *this) {
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

INCLUDE_ASM("title/nonmatchings/title_logo", func_800696A8);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_80069910(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}
