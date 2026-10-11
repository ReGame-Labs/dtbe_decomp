#ifndef DTBE_TITLE_TITLE_CLASSES_H
#define DTBE_TITLE_TITLE_CLASSES_H

/* Classes of title.bin, known only by their layout and named after a function that uses them. */

#include "common.h"
#include "engine/game/character.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/sprite.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"

EXTERN_C_BEGIN

/* An object that func_80070B8C handles through func_80070DE8 and func_80070E6C; the object it
 * keeps at 0x48 is one func_80070420 resets (func_80070D60 runs func_80070404 on it). */
typedef struct Unk80070DE8 {
    /* 0x00 */ u8 unk0[0x48];
    /* 0x48 */ void *unk48;
} Unk80070DE8;

/* An image that func_80073390 (and its copies func_80071CB8 and func_800776DC) draws: its texture page,
 * then the uv, clut, width and height of its SPRT as words, copied as they are. The tables of them
 * (the title's pictures D_80065EF4, D_80066024) have more after. */
typedef struct Unk80073390Image {
    /* 0x0 */ u16 tpage;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u32 uvClut; /* u0, v0, then the clut */
    /* 0x8 */ u32 wh;     /* w, then h */
} Unk80073390Image;

/* The entity class of vtable D_8006659C (func_80076E24): a character model of the title, seen by camera.
 * func_800774D4 and func_800775AC draw through the camera's ordering table. */
typedef struct Unk80076E24 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Camera *camera;
    /* 0x1C */ Chara *chara;
    /* 0x20 */ Model *model;
    /* 0x24 */ u8 unk24[0x40 - 0x24];
} Unk80076E24; /* size 0x40 */

/* The destructor of StepperGroup (engine/menu/stepper.h), a C++ class, as the C of title.bin calls it. */
void stepperGroupDestroy(void *group, s32 flags) __asm__("_._12StepperGroup");

/* Two tasks of title.bin with a thread, a StepperGroup and a sprite, named after their destructors. */
typedef struct Unk8006A62C {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ u8 steppers[0x44]; /* a StepperGroup */
    /* 0x70 */ Sprite sprite;
} Unk8006A62C;

typedef struct Unk800797D8 {
    /* 0x00 */ Task task;
    /* 0x20 */ Thread thread;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ u8 steppers[0x44]; /* a StepperGroup */
    /* 0x74 */ Sprite sprite;
} Unk800797D8;

EXTERN_C_END

#endif /* DTBE_TITLE_TITLE_CLASSES_H */
