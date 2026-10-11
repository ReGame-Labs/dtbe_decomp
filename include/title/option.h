#ifndef DTBE_TITLE_OPTION_H
#define DTBE_TITLE_OPTION_H

/* The options' menu items and task (title/option.cpp). */

#ifdef __cplusplus
/* Their virtual tables stay in the overlay's assembly (.rodata), so option.cpp
 * writes none of them out. */
#pragma interface
#endif

#include "common.h"
#include "engine/menu/stepper.h"
#include "engine/system/thread.h"
#include "engine/gfx/sprite.h"
#include "engine/task/task.h"
#include "title/title_classes.h"

#ifdef __cplusplus

/*
 * The member functions keep the names of the symbol files through asm labels
 * (g++ would mangle them); the destructors, which take no asm label, have
 * g++'s names there.
 */

/* An item of the options' menu (virtual table D_80066914): an image drawn at its row, blinking while it
 * is current. The other items derive from it; Stepper is its virtual base, reached through the pointer
 * at 0x00 and placed after the members of the most derived class. */
class Unk80077AC8 : public virtual Stepper {
public:
    /* 0x04 */ Unk80073390Image *image;
    /* 0x08 */ s32 blink; /* the blink's angle; 0 while the item is not active */

    void stepUp(s32 arg, s32 held, u32 buttons) __asm__("func_80077820");
    void stepDown(s32 arg, s32 held, u32 buttons) __asm__("func_8007786C");
    void refresh(s32 arg) __asm__("func_800778F8");
    void func_8007799C() __asm__("func_8007799C");
    void focus(s32 arg) __asm__("func_80077A88");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_80077AB4");
    Unk80077AC8(Unk80073390Image *image) __asm__("func_80077AC8");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_80077BF8");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_80077C2C");
    s32 func_80077C44(s32 isCurrent) __asm__("func_80077C44");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_80077C78");
    ~Unk80077AC8();
};

/* The item of virtual table D_800668B4: Unk80077AC8 that also draws the image of its value, from a
 * table. */
class Unk80077D20 : public Unk80077AC8 {
public:
    /* 0x0C */ Unk80073390Image **images; /* one per value */

    Unk80077D20(Unk80073390Image *image, Unk80073390Image **images, s32 count) __asm__("func_80077D20");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_80077E78");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_8007A3B0");
    ~Unk80077D20();
};

/* The item of virtual table D_80066854: confirm starts a memory card task (memoryCardTaskInit) in
 * mode, and the item keeps that task while it runs. */
class Unk80077F28 : public Unk80077AC8 {
public:
    /* 0x0C */ s32 mode; /* memoryCardTaskInit's */
    /* 0x10 */ Task *task;
    /* 0x14 */ u32 taskHandle; /* the task's handle, in TASK_HANDLES */

    Unk80077F28(Unk80073390Image *image, s32 unkC) __asm__("func_80077F28");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_80078078");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_800780F4");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_80078160");
    ~Unk80077F28();
};

/* The item of virtual table D_800667F4: moves the screen (the save data's screenX and screenY). */
class Unk800782D8 : public Unk80077AC8 {
public:
    /* 0x0C */ s32 screenX; /* the screen position as shown (SaveData.screenX, screenY) */
    /* 0x10 */ s32 screenY;
    /* 0x14 */ s32 keptX;   /* and as it was when the item was confirmed; cancel goes back to it */
    /* 0x18 */ s32 keptY;
    /* 0x1C */ s32 moving;  /* the screen is being moved (confirm without own) */

    Unk800782D8(Unk80073390Image *image) __asm__("func_800782D8");
    void refresh(s32 arg) __asm__("func_8007841C");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_80078448");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_800784A0");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_8007850C");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_80078628");
    ~Unk800782D8();
};

/* The item of virtual table D_80066794, with a name entry. */
class Unk800786E4 : public Unk80077AC8 {
public:
    /* 0x0C */ Unk80074220 entry;

    Unk800786E4(Unk80073390Image *image) __asm__("func_800786E4");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_80078848");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_800788A0");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_800788F4");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_8007A3B8");
    ~Unk800786E4();
};

/* The item of virtual table D_80066734. */
class Unk80078914 : public Unk80077AC8 {
public:
    Unk80078914(Unk80073390Image *image) __asm__("func_80078914");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_8007A3C0");
    ~Unk80078914();
};

/* The item of virtual table D_800666D4: the button slot of its row (its index) for both players, in the
 * save data. */
class Unk80078A4C : public Unk80077AC8 {
public:
    /* 0x0C */ s32 editing;  /* confirm without own started a change */
    /* 0x10 */ s32 slots[2]; /* by player, while editing */

    Unk80078A4C(Unk80073390Image *image) __asm__("func_80078A4C");
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_80078B84");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_80078C64");
    void refresh(s32 arg) __asm__("func_80078CB8");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_80078D20");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_80078E3C");
    ~Unk80078A4C();
};

/* The options' menu (virtual table D_80066674): a StepperGroup and an Unk80077AC8 (at 0x24) on one
 * Stepper. It overrides what both of them override, mostly by calling StepperGroup's. */
class Unk80079254 : public StepperGroup, public Unk80077AC8 {
public:
    Unk80079254(Unk80073390Image *image) __asm__("func_80078F5C");
    ~Unk80079254();
    s32 confirm(s32 arg, s32 held, s32 own) __asm__("func_800793D0");
    s32 input(s32 arg, s32 held, u32 buttons) __asm__("func_8007943C");
    void draw(s32 arg, s32 isCurrent, s32 active) __asm__("func_8007945C");
    s32 cancel(s32 arg, s32 held, s32 own) __asm__("func_8007A41C");
    void refresh(s32 arg) __asm__("func_8007A44C");
};

/* The options' task (virtual table D_80066974), like main_menu.c's Unk8006A62C with a word at 0x2C, named
 * after its destructor. */
class Unk800797D8 : public Task {
public:
    /* 0x20 */ Thread thread;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ u8 steppers[0x44]; /* a StepperGroup */
    /* 0x74 */ Sprite sprite;

    Unk800797D8() __asm__("optionTaskInit");
    ~Unk800797D8();
    void update(s32 arg) __asm__("func_80079978");
};

#endif /* __cplusplus */

EXTERN_C_BEGIN

/* the first row of the options' menu on the screen */
extern s32 D_8007B32C;

s32 func_80077690(s32 index);
s32 func_800776BC(s32 index);
SprtColorWords *func_800776DC(u_long *ot, s32 x, s32 y, Unk80073390Image *image);
SprtColorWords *func_80077778(u_long *ot, s32 x, s32 y, Unk80073390Image *image, s32 index);

EXTERN_C_END

#endif /* DTBE_TITLE_OPTION_H */
