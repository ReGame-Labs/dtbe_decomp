#include "common.h"
#include "engine/math/sin_cos.h"
#include <libgpu.h>
#include <libetc.h>
#include "engine/cd/file_load.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_dr_move.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_poly_ft4.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/gfx/scene_graph.h"
#include "engine/lib/list.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"
#include "engine/text/font.h"
#include "title/title_classes.h"

/* functions that func_80070DE8 and func_80070E6C call */
void func_80070420(void *arg);
void func_80071020(Unk80070DE8 *this);
void func_80070EAC(Unk80070DE8 *this, s32 arg);
void func_800718F8(Unk80070DE8 *this, s32 arg1, s32 arg2);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_80064594;
extern EntityVtable D_800645BC;
extern EntityVtable D_800645E4;
extern EntityVtable D_8006460C;
extern TaskVtable D_8006471C;
extern char D_80065FB0[];
extern char D_800663D0[];
extern char D_800663DC[];
extern TaskVtable D_80066974;
extern RECT *D_8007B2DC[][2];
extern RECT *D_8007B2FC[][2];
#include "libgpu.h"

/* the object of func_80074248, which resets it */
typedef struct Unk80074248 {
    /* 0x0 */ s32 unk0;
} Unk80074248;

/* the object of func_800764BC, which sorts the nodes at 8 */
typedef struct Unk800764BC {
    /* 0x0 */ u8 unk0[8];
    /* 0x8 */ LinkNode *unk8;
    /* 0xC */ LinkNode *unkC;
} Unk800764BC;

/* a node of the list func_800764BC sorts (func_800764F4 orders them) */
typedef struct Unk800764F4 {
    /* 0x0 */ struct Unk800764F4 *next;
    /* 0x4 */ s32 unk4;
} Unk800764F4;

/* A Cache with a heap block at 0x18 (func_8007A46C constructs it, virtual
 * table D_80066B68). */
typedef struct Unk8007A46C {
    /* 0x00 */ Cache cache;
    /* 0x18 */ void *unk18; /* freed with mainHeapFree */
    /* 0x1C */ u8 unk1C[0x2C - 0x1C];
    /* 0x2C */ RECT unk2C;
    /* 0x34 */ RECT unk34;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ RECT unk40; /* 0, 0, 24, 24 at first (func_8007A46C) */
    /* 0x48 */ u8 unk48[4]; /* 0x80 each at first; func_8007B008 sets three and 0x80 */
} Unk8007A46C;

extern CacheVtable D_80066B68;

/* func_8007096C points 0 at a word and sets the word to 1; func_80070C2C
 * makes one on its stack for D_8005FCB0. */
typedef struct Unk8007096C {
    /* 0x0 */ s32 *unk0;
} Unk8007096C;

typedef struct Unk80070428 {
    /* 0x00 */ u8 unk0[0x18];
    /* 0x18 */ s32 unk18;
} Unk80070428;

/* a member (at 0x38 and 0xC of the objects func_80073784 and func_80079E2C
 * destroy) whose destructor has nothing to do; its contents are not known */
typedef struct Unk80074220 Unk80074220;

void func_80070B8C(void *object);
void func_80074248(Unk80074248 *this);
LinkNode *func_800764BC(Unk800764BC *this);

/* the object of func_800674F0 (virtual table D_800644A0) */
typedef struct Unk800674F0 {
    /* 0x0000 */ u8 unk0[0x24];
    /* 0x0024 */ EntityGroup group;
    /* 0x0034 */ u8 unk34[0x44 - 0x34];
    /* 0x0044 */ s32 unk44;
    /* 0x0048 */ u8 unk48[0x4C - 0x48];
    /* 0x004C */ s32 unk4C;
    /* 0x0050 */ u8 unk50[0x54 - 0x50];
    /* 0x0054 */ s32 unk54;
    /* 0x0058 */ s32 unk58;
    /* 0x005C */ s32 unk5C;
    /* 0x0060 */ s32 unk60;
    /* 0x0064 */ u8 unk64[0x10FC - 0x64];
    /* 0x10FC */ s32 unk10FC; /* the step */
} Unk800674F0;

/* The task of func_80070EAC and func_80071020, which work the memory card: its
 * thread at 0x24 waits on the card; func_80071854 waits for 0x50 to be set and
 * returns 0x54. */
typedef struct Unk8007175C {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ Thread thread;
    /* 0x30 */ s32 unk30; /* what the card calls get (a port) */
    /* 0x34 */ u8 unk34[0x50 - 0x34];
    /* Set by the memory card callback func_80071550 and polled by the thread, so volatile:
     * func_80071854 stores 0x50 and loads it right back, and func_80071550's last store stays out
     * of the delay slot of its `jr ra`. */
    /* 0x50 */ volatile s32 unk50; /* nonzero once the callback ran */
    /* 0x54 */ volatile s32 unk54; /* the callback's second argument */
} Unk8007175C;

/* two PsyQ memory card calls (libcard), each 0 while the card is busy */
s32 func_800588AC(s32 port);
s32 func_80058C30(s32 port);
s32 func_80071854(Unk8007175C *this);

/* the object of func_8006CC64: 0x88 is the step it is at */
typedef struct Unk8006CC64 {
    /* 0x000 */ u8 unk0[0x88];
    /* 0x088 */ s32 unk88;
    /* 0x08C */ u8 unk8C[0x94 - 0x8C];
    /* 0x094 */ s32 unk94;
    /* 0x098 */ u8 unk98[0x18C - 0x98];
    /* 0x18C */ Vec3 position; /* of the object at 0x220 (func_80077420) */
    /* 0x198 */ u8 unk198[4];
    /* 0x19C */ Vec3 rotation; /* of the object at 0x220: angles about x, y, z */
    /* 0x1A8 */ u8 unk1A8[0x220 - 0x1A8];
    /* 0x220 */ void *unk220;
} Unk8006CC64;

/* sets the matrix of object's 0x20 from a position and the angles about x, y and z */
void func_80077420(void *object, Vec3 *position, Vec3 *rotation);

/* Stepper's virtual table, which C reaches by its g++ name */
extern VtableEntry STEPPER_VTABLE[] __asm__("_vt.7Stepper");

/* The title's Stepper class of virtual table D_800646BC (func_8006A11C), as C
 * sees it: a Stepper's list node, and its virtual table pointer at 0x1C. */
typedef struct Unk8006A11C {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ VtableEntry *vtable;
    /* 0x20 */ s32 unk20; /* func_8006A11C's first argument */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* 0x2000 from func_8006A11C */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* func_8006A11C's second argument */
} Unk8006A11C;

/* Stepper's constructor, as C sees it */
void *stepperInit(void *this, s32 value);
extern VtableEntry D_800646BC[];

/* the object of func_800773FC, which nothing calls */
typedef struct Unk800773FC {
    /* 0x00 */ u8 unk0[0x18];
    /* 0x18 */ Camera *camera; /* func_8007731C draws the model in it */
    /* 0x1C */ u8 unk1C[0x20 - 0x1C];
    /* 0x20 */ Model *model;
} Unk800773FC;

/* Calls the invalidate method of a scene object (its world moved). */
#define SCENE_OBJECT_INVALIDATE(object) \
    (object)->vtable->invalidate.func((u8 *)(object) + (object)->vtable->invalidate.delta)

/* Tasks whose update stores its argument at 0x20 and calls their own first
 * virtual method (at 0x20 of the virtual table) to do the work: D_80064404,
 * D_80064634, D_80064C10, D_80065EC4, D_80065FE8 and D_800664CC here, and
 * D_80064434 in game.bin (Unk8006ECEC there). The same
 * update in every one of them is probably an inline virtual of a shared
 * header, written out once per source file. */
typedef struct Unk8006713CVtable {
    /* 0x00 */ TaskVtable task;
    /* 0x20 */ VtableEntry unk20;
} Unk8006713CVtable;

typedef struct Unk8006713C {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ Unk8006713CVtable *vtable;
    /* 0x20 */ s32 unk20; /* the argument of the last update */
} Unk8006713C;

void func_80070404(void *unk48, s32 arg);

/* StepperGroup's methods that the title's menus call, as C sees them
 * (engine/menu/stepper.h declares them for C++ only) */
s32 stepperGroupInput(void *group, s32 arg, s32 held, u32 buttons);
void stepperGroupRefresh(void *group, s32 arg);
s32 stepperStepUpClamp(void *stepper);

/* Stepper's members, as C sees them (engine/menu/stepper.h declares the class for C++ only) */
typedef struct StepperMembers {
    /* 0x00 */ ListNode link;
    /* 0x08 */ void *group;
    /* 0x0C */ s32 index; /* among the children of its group */
    /* 0x10 */ s32 value;
    /* 0x14 */ s32 min;
    /* 0x18 */ s32 max;
    /* 0x1C */ VtableEntry *vtable;
} StepperMembers;

/* The title's menu item of virtual table D_80066914 (constructor func_80077AC8): an image drawn at its
 * row, blinking while it is current. Stepper is its virtual base, reached through the pointer at 0x00. */
typedef struct Unk80077AC8 {
    /* 0x00 */ StepperMembers *stepper;
    /* 0x04 */ Unk80073390Image *image;
    /* 0x08 */ s32 blink; /* the blink's angle; 0 while the item is not active */
} Unk80077AC8;

/* The item of virtual table D_800668B4 (constructor func_80077D20): Unk80077AC8 that also draws the image
 * of its value, from a table. */
typedef struct Unk80077D20 {
    /* 0x00 */ Unk80077AC8 base;
    /* 0x0C */ Unk80073390Image **images; /* one per value */
} Unk80077D20;

void func_80077C78(Unk80077AC8 *this, u_long *ot, s32 isCurrent, s32 active);

void func_80067CF0(void *this);
void func_8006803C(void *this);
void func_80072CF0(void *this);
void func_80073208(void);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", bootLogoTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80066E1C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067018);

/* The destructor of bootLogoTaskInit's task (virtual table D_80064404): Task's. */
void func_8006711C(Task *this, s32 flags) {
    taskDestroy(this, flags);
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8006713C(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", titleLogoTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800673F8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800674F0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067808);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067864);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006795C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067A3C);

/* Starts fading the songs out (step 0x44) and counts up 0x10FC. */
void func_80067AE0(Unk800674F0 *this) {
    fadeSongs(0x44);
    this->unk10FC++;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067B14);

void func_80067B60(void) {
}

void func_80067B68(void) {
}

void func_80067B70(void) {
}

void func_80067B78(void) {
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067B80);

/* Draws the object's two parts: both callees write primitives at PRIM_BUFFER_FREE. */
void func_80067CC4(void *this) {
    func_8006803C(this);
    func_80067CF0(this);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80067CF0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006803C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80068320);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800683D0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800683DC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800685A8);

/* On Start or circle on either pad: destroys the group's entities and moves to step 2 (0x54 = 0xFF,
 * 0x58 = 0x1000, 0x60 = 0x50). */
void func_800686D8(Unk800674F0 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on both presses or-ed in one expression: the first result then takes the pad
     * manager's register (a variable for it costs a third one). */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & (PADstart | PADRright)) {
        entityGroupDestroyAll(&this->group);
        this->unk54 = 0xFF;
        this->unk58 = 0x1000;
        this->unk60 = 0x50;
        this->unk10FC = 2;
    }
}

/* On Start on either pad: moves to step 3 (0x44 = 1, 0x5C and 0x4C cleared). */
void func_8006875C(Unk800674F0 *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;

    /* The match depends on one expression, as in func_800686D8. */
    if ((padManagerGetPressed(pads, 0) | padManagerGetPressed(pads, 1)) & PADstart) {
        this->unk44 = 1;
        this->unk5C = 0;
        this->unk10FC = 3;
        this->unk4C = 0;
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800687CC);

/* The destructor of the entity class of vtable D_8006460C, which func_800687CC installs. */
void func_800688E8(Entity *this, s32 flags) {
    this->vtable = &D_8006460C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006893C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80068BB4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80068E0C);

/* The destructor of the entity class of vtable D_800645E4, which func_80068E0C installs. */
void func_80068F6C(Entity *this, s32 flags) {
    this->vtable = &D_800645E4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80068FC0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069064);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069218);

/* The destructor of the entity class of vtable D_800645BC, which func_80069218 installs. */
void func_800692C0(Entity *this, s32 flags) {
    this->vtable = &D_800645BC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069314);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800693B0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800694B8);

/* The destructor of the entity class of vtable D_80064594, which func_800694B8 installs. */
void func_80069580(Entity *this, s32 flags) {
    this->vtable = &D_80064594;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800695D4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800696A8);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_80069910(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069944);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069980);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069A58);

/* An image in VRAM (D_80064768): its texture page, its corner in the page, its CLUT and its size. */
typedef struct Unk80069CFC {
    /* 0x0 */ u16 tpage;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u8 u;
    /* 0x5 */ u8 v;
    /* 0x6 */ u16 clut;
    /* 0x8 */ s16 w;
    /* 0xA */ s16 h;
} Unk80069CFC;

/* Draws the image at (x, y) as a POLY_FT4, mirrored left to right when notMirrored is 0. */
void func_80069CFC(u_long *ot, s32 x, s32 y, Unk80069CFC *image, s32 notMirrored) {
    s32 u = image->u;
    s32 v = image->v;
    s32 w = image->w;
    s16 h = image->h - 1;
    POLY_FT4 *poly = allocPolyFT4();

    if (notMirrored == 0) {
        poly->u0 = u + (w - 1);
        poly->v0 = v;
        poly->u1 = u;
        poly->v1 = v;
        poly->u2 = u + (w - 1);
        poly->v2 = v + h;
        poly->u3 = u;
        poly->v3 = v + h;
    } else {
        poly->u0 = u;
        poly->v0 = v;
        poly->u1 = u + w;
        poly->v1 = v;
        poly->u2 = u;
        poly->v2 = v + h;
        poly->u3 = u + w;
        poly->v3 = v + h;
    }
    poly->x0 = x;
    poly->y0 = y;
    poly->x1 = w + x;
    poly->y1 = y;
    poly->x2 = x;
    poly->y2 = h + y;
    poly->x3 = w + x;
    poly->y3 = h + y;
    poly->clut = image->clut;
    SetSemiTrans(poly, 0);
    poly->tpage = image->tpage;
    poly->r0 = 0x80;
    poly->g0 = 0x80;
    poly->b0 = 0x80;
    AddPrim(ot, poly);
}

/* Draws the image's shadow at (x, y): func_80069CFC's quad, half bright and semi-transparent, with the CLUT
 * 0x40 further and texture page 0x58. */
void func_80069E4C(u_long *ot, s32 x, s32 y, Unk80069CFC *image, s32 notMirrored) {
    s32 u = image->u;
    s32 v = image->v;
    s32 w = image->w;
    s16 h = image->h - 1;
    POLY_FT4 *poly = allocPolyFT4();

    if (notMirrored == 0) {
        poly->u0 = u + (w - 1);
        poly->v0 = v;
        poly->u1 = u;
        poly->v1 = v;
        poly->u2 = u + (w - 1);
        poly->v2 = v + h;
        poly->u3 = u;
        poly->v3 = v + h;
    } else {
        poly->u0 = u;
        poly->v0 = v;
        poly->u1 = u + w;
        poly->v1 = v;
        poly->u2 = u;
        poly->v2 = v + h;
        poly->u3 = u + w;
        poly->v3 = v + h;
    }
    poly->x0 = x;
    poly->y0 = y;
    poly->x1 = w + x;
    poly->y1 = y;
    poly->x2 = x;
    poly->y2 = h + y;
    poly->x3 = w + x;
    poly->y3 = h + y;
    poly->clut = image->clut + 0x40;
    SetSemiTrans(poly, 1);
    poly->tpage = 0x58;
    poly->r0 = 0x40;
    poly->g0 = 0x40;
    poly->b0 = 0x40;
    AddPrim(ot, poly);
}


INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80069FA0);

/* Entry 0x1C of D_800646BC: plays sound 0xF. */
void func_8006A0B4(void) {
    soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
}

/* The cancel method (entry 0x10) of D_800646BC: refused (-1) when own is set, else plays sound 0x11
 * and accepts. */
s32 func_8006A0E0(Unk8006A11C *this, s32 arg, s32 held, s32 own) {
    if (own != 0) {
        return -1;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x11, 0);
    return 0;
}

/* Constructs the title's Stepper of virtual table D_800646BC with its two values (0x20 and 0x30). */
Unk8006A11C *func_8006A11C(Unk8006A11C *this, s32 arg1, s32 arg2) {
    stepperInit(this, 0);
    this->vtable = D_800646BC;
    /* The match depends on storing arg2 first (its shorter life gives it s1). */
    this->unk30 = arg2;
    this->unk20 = arg1;
    this->unk24 = 0;
    this->unk28 = 0x2000;
    this->unk2C = 0;
    return this;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A184);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A1E4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A224);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A348);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", mainMenuTaskInit);

/* The destructor of Unk8006A62C (task vtable D_8006471C): fades the screen in, then destroys the sprite,
 * the steppers, the thread and the task. */
void func_8006A62C(Unk8006A62C *this, s32 flags) {
    this->task.vtable = &D_8006471C;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->sprite, DESTROY_BASES);
    stepperGroupDestroy(this->steppers, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A6A4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006A7EC);

/* Unk8006A11C's destructor: Stepper's, inlined (it takes the stepper out of its group's list). */
void func_8006AAA0(Unk8006A11C *this, s32 flags) {
    this->vtable = STEPPER_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", characterSelectTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B104);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B1DC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B568);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B700);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B770);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006B924);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BAE8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BB74);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BD18);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BE7C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BEA8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006BF94);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C144);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C244);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C350);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C408);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C544);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C6E4);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006C940);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CA54);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CAFC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CC2C);

/* Starts fading the songs out (step 0x44) and sets 0x88 to 9. */
void func_8006CC64(Unk8006CC64 *this) {
    fadeSongs(0x44);
    this->unk88 = 9;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CC94);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CCD4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006CD6C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006D98C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006DA20);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006DBA4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006DCCC);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006DFA4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006E1A0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006E368);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006EAF0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006EED4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006EF94);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006F3C4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006FAD8);

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

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8006FEB8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800700A4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070298);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_800702C4(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

/* The entity class func_800702F8 constructs (virtual table D_800656B8) with four values; func_80071C78's
 * (virtual table D_80065690) derives from it and adds 0x34. */
typedef struct Unk800702F8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
} Unk800702F8;

extern EntityVtable D_800656B8;
extern EntityVtable D_80065690;

Unk800702F8 *func_800702F8(Unk800702F8 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800702F8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007037C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070404);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070414);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070420);

/* Whether 0x18 is 0x1000. */
s32 func_80070428(Unk80070428 *this) {
    return this->unk18 == 0x1000;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007043C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070494);

/* The number of lines in text: its line breaks plus one. */
s32 func_80070518(const u8 *text) {
    s32 lines = 1;
    u8 c;

    while ((c = *text++) != '\0') {
        if (c == '\n') {
            lines++;
        }
    }
    return lines;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070554);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800705A8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070708);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070750);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007075C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070768);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007084C);

/* Points this at word and sets the word to 1; returns this. */
Unk8007096C *func_8007096C(Unk8007096C *this, s32 *word) {
    this->unk0 = word;
    *word = 1;
    return this;
}

/* Unk8007096C's destructor: sets the word it points at back to 0. */
void func_80070980(Unk8007096C *this, s32 flags) {
    *this->unk0 = 0;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", memoryCardTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070AEC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070B60);

/* The entry of the thread of memoryCardTaskInit's object. */
void func_80070B6C(s32 unused, void *object) {
    func_80070B8C(object);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070B8C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070C2C);

/* func_80070404 on the object at 0x48, with arg. */
void func_80070D60(Unk80070DE8 *this, s32 arg) {
    func_80070404(this->unk48, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070D84);

/* func_80071020, resets the object at 0x48 (func_80070420), then func_800718F8 with 0x14. */
void func_80070DE8(Unk80070DE8 *this) {
    func_80071020(this);
    func_80070420(this->unk48);
    func_800718F8(this, 0x14, 0);
}

/* func_80070EAC with 1, resets the object at 0x48 (func_80070420), then func_800718F8 with 0x14. */
void func_80070E28(Unk80070DE8 *this) {
    func_80070EAC(this, 1);
    func_80070420(this->unk48);
    func_800718F8(this, 0x14, 0);
}

/* The same as func_80070DE8. */
void func_80070E6C(Unk80070DE8 *this) {
    func_80071020(this);
    func_80070420(this->unk48);
    func_800718F8(this, 0x14, 0);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80070EAC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80071020);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80071204);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007138C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800714DC);

/* the task memoryCardTaskInit builds, while it lives */
extern Unk8007175C *D_8007B238;

/* The memory card callback that memoryCardTaskInit installs (func_80059C74, after MemCardStart): keeps
 * its two arguments in the object for the thread waiting in func_80071854. */
void func_80071550(s32 unk50, s32 unk54) {
    D_8007B238->unk50 = unk50;
    D_8007B238->unk54 = unk54;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007156C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800715AC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007163C);

/* Runs func_800588AC on the card (once it is free) and waits for its result, up to
 * three times: 1 for a result of 0 or 3, 0 when all three are others. */
s32 func_8007175C(Unk8007175C *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!func_800588AC(this->unk30)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = func_80071854(this);
        if (result == 0 || result == 3) {
            return 1;
        }
    } while (--tries != 0);
    return 0;
}

/* Runs func_80058C30 on the card (once it is free) and waits for its result, up to
 * three times while the result is neither 0 nor 3; returns the last one. */
s32 func_800717DC(Unk8007175C *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!func_80058C30(this->unk30)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = func_80071854(this);
        if (result == 0 || result == 3) {
            break;
        }
    } while (--tries != 0);
    return result;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80071854);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800718A4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800718F8);

/* The object func_80071980 runs in its thread (probably Unk80070DE8, whose layout title_classes.h only
 * knows to 0x48): its thread at 0x24 and the object it waits on at 0x4C. */
typedef struct Unk80071980 {
    /* 0x00 */ u8 unk0[0x24];
    /* 0x24 */ Thread thread;
    /* 0x30 */ u8 unk30[0x4C - 0x30];
    /* 0x4C */ void *unk4C;
} Unk80071980;

void func_80070708(void *object, s32 arg);
s32 func_80070750(void *object);
s32 func_8007075C(void *object);

/* Starts the object at 0x4C (func_80070708 with arg) and gives up the thread each frame until
 * func_80070750 says it is done; returns func_8007075C. */
s32 func_80071980(Unk80071980 *this, s32 arg) {
    void *object = this->unk4C;

    func_80070708(object, arg);
    do {
        threadSwitchBack(&this->thread, 0);
    } while (!func_80070750(object));
    return func_8007075C(object);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800719D8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80071A44);

/* The destructor of the entity class of vtable D_800656B8, which func_800702F8 installs. */
void func_80071BE8(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_80065690, which memoryCardTaskInit and func_80071C78 install. */
void func_80071C30(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs func_800702F8's entity with virtual table D_80065690, 0x34 cleared. */
Unk800702F8 *func_80071C78(Unk800702F8 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    func_800702F8(this, arg1, arg2, arg3, arg4);
    this->entity.vtable = &D_80065690;
    this->unk34 = 0;
    return this;
}

/* The same as func_80073390. */
SprtColorWords *func_80071CB8(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* func_8007342C with texture (0..15, 0x40..0x80), page 0x16 and CLUT 0x7F18: two columns of six scrolling
 * strips. The match depends on the same forms as func_8007342C's. */
void func_80071D54(u_long *ot, s32 frame) {
    s32 i;

    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 + (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 0x12E;
        poly->y0 = y - 64;
        poly->x1 = 0x13E;
        poly->y1 = y - 64;
        poly->x2 = 0x12E;
        poly->y2 = y;
        poly->x3 = 0x13E;
        poly->y3 = y;
        setUV4(poly, 0, 0x40, 0x10, 0x40, 0, 0x80, 0x10, 0x80);
        poly->tpage = 0x16;
        poly->clut = 0x7F18;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 - (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 2;
        poly->y0 = y;
        poly->x1 = 0x12;
        poly->y1 = y;
        poly->x2 = 2;
        poly->y2 = y + 64;
        poly->x3 = 0x12;
        poly->y3 = y + 64;
        setUV4(poly, 0, 0x40, 0x10, 0x40, 0, 0x80, 0x10, 0x80);
        poly->tpage = 0x16;
        poly->clut = 0x7F18;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", guideLogoTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80072100);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800721C0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800722CC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800723FC);

/* Calls the empty func_80073208, then func_80072CF0, which resets fields 0xA76-0xA8A. */
void func_800724D8(void *this) {
    func_80073208();
    func_80072CF0(this);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80072504);

void func_80072560(void) {
}

/* The guide logo task, which guideLogoTaskInit sets up. */
typedef struct GuideLogoTask {
    /* 0x000 */ Task task;
    /* 0x020 */ u8 unk20[0x24 - 0x20];
    /* 0x024 */ EntityGroup entities;
    /* 0x034 */ u8 unk34[0x3C - 0x34];
    /* 0x03C */ Camera camera;
    /* 0x078 */ u8 unk78[0xA6C - 0x78]; /* 0xA8: a Mersenne twister */
    /* 0xA6C */ u16 unkA6C; /* a frame count (func_800721C0) */
    /* 0xA6E */ s16 unkA6E;
    /* 0xA70 */ s16 unkA70;
    /* 0xA72 */ s16 unkA72;
    /* 0xA74 */ u16 unkA74; /* guideLogoTaskInit's argument */
} GuideLogoTask;

extern Unk80073390Image D_80066024[];
SprtColorWords *func_80073390(u_long *ot, s32 x, s32 y, Unk80073390Image *image);
void func_8007342C(u_long *ot, s32 frame);

extern Unk80073390Image D_80065EF4[];

/* Draws the guide logo screen: func_80071D54's strips at the frame count, picture 4 + 0xA74 and picture 14
 * of D_80065EF4, then the 0x140 x 0xF0 picture at (0, 0) of texture pages 6 and 7 (func_80073F94's shape). */
void func_80072568(GuideLogoTask *this) {
    u_long *ot = this->camera.ot.head;
    Unk80073390Image *pictures;
    SPRT *sprt;
    DR_TPAGE *tpage;

    func_80071D54(ot, this->unkA6C);
    /* The match depends on the table in a local, set after the first call (it then keeps s1). */
    pictures = D_80065EF4;
    sprt = (SPRT *)func_80071CB8(ot, 0x2F, 0xD, &pictures[(u16)(this->unkA74 + 4)]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)func_80071CB8(ot, 0x46, 0xD1, &pictures[14]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    tpage = allocDrTpage(6);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0, 0);
    setWH(sprt, 0x100, 0xF0);
    sprt->clut = getClut(0x180, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    tpage = allocDrTpage(7);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0x100, 0);
    setWH(sprt, 0x40, 0xF0);
    sprt->clut = getClut(0x180, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

void func_800726E4(void) {
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800726EC);

void func_800728D0(void) {
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800728D8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80072CF0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80072D2C);

void func_80073200(void) {
}

void func_80073208(void) {
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073210);

/* Clears 0xA6E, 0xA70 and 0xA72. */
void func_800732D8(GuideLogoTask *this) {
    this->unkA6E = 0;
    this->unkA70 = 0;
    this->unkA72 = 0;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800732E8);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8007335C(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

/* Draws image at (x, y): a SPRT, then the DR_TPAGE of its texture page, added to ot. Returns the SPRT. */
SprtColorWords *func_80073390(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* Draws two columns of six 16 by 64 strips (texture (0..15, 0x48..0x88)) that scroll with frame: down at x 0x12E,
 * up at x 2. */
/* The match depends on frame & 0x3F written in each loop (GCSE shares the two) and on y declared per loop
 * (a function-wide y keeps the second loop's counter from being reversed). */
void func_8007342C(u_long *ot, s32 frame) {
    s32 i;

    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 + (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 0x12E;
        poly->y0 = y - 64;
        poly->x1 = 0x13E;
        poly->y1 = y - 64;
        poly->x2 = 0x12E;
        poly->y2 = y;
        poly->x3 = 0x13E;
        poly->y3 = y;
        setUV4(poly, 0, 0x48, 0x10, 0x48, 0, 0x88, 0x10, 0x88);
        poly->tpage = 0x17;
        poly->clut = 0x7F5C;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
    for (i = 0; i < 6; i++) {
        POLY_FT4 *poly = allocPolyFT4();
        s32 y = i * 64 - (frame & 0x3F);

        setRGB0(poly, 0x80, 0x80, 0x80);
        poly->x0 = 2;
        poly->y0 = y;
        poly->x1 = 0x12;
        poly->y1 = y;
        poly->x2 = 2;
        poly->y2 = y + 64;
        poly->x3 = 0x12;
        poly->y3 = y + 64;
        setUV4(poly, 0, 0x48, 0x10, 0x48, 0, 0x88, 0x10, 0x88);
        poly->tpage = 0x17;
        poly->clut = 0x7F5C;
        SetSemiTrans(poly, 1);
        AddPrim(ot, poly);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", rankingTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073784);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073818);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073918);

/* Loads "title/ranking.tim" (D_80065FB0), uploads its TIMs to VRAM and frees it. */
void func_80073960(void) {
    u32 *file = loadFile(D_80065FB0);

    uploadTims(file);
    mainHeapFree(file);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007399C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073A04);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073B34);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073D40);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80073E34);

void func_80073F8C(void) {
}

/* The object func_80073818 runs: 0x10C counts its frames (it fades out at 0x1CC). */
typedef struct Unk80073818 {
    /* 0x000 */ u8 unk0[0x10C];
    /* 0x10C */ s32 frame;
} Unk80073818;

/* Draws func_80073818's screen: func_8007342C's part, two of D_80066024's sprites, then the 0x140 x 0xF0
 * picture at (0, 0) of texture pages 7 and 8. */
void func_80073F94(Unk80073818 *this) {
    u_long *ot = FRAME_OT.firstDrawn + 5;
    SPRT *sprt;
    DR_TPAGE *tpage;

    func_8007342C(ot, this->frame);
    sprt = (SPRT *)func_80073390(ot, 0x2F, 0xD, &D_80066024[0]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    sprt = (SPRT *)func_80073390(ot, 0x5E, 0xD0, &D_80066024[2]);
    setRGB0(sprt, 0x80, 0x80, 0x80);
    tpage = allocDrTpage(7);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0, 0);
    setWH(sprt, 0x100, 0xF0);
    sprt->clut = getClut(0x1C0, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
    tpage = allocDrTpage(8);
    sprt = allocSprt();
    setRGB0(sprt, 0x80, 0x80, 0x80);
    setUV0(sprt, 0, 0);
    setXY0(sprt, 0x100, 0);
    setWH(sprt, 0x40, 0xF0);
    sprt->clut = getClut(0x1C0, 0xFF);
    AddPrim(ot, sprt);
    AddPrim(ot, tpage);
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_800740FC(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074130);

/* Unk80074220's destructor: frees it when asked, nothing else. */
void func_80074220(Unk80074220 *this, s32 flags) {
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074248);

/* Resets the object (func_80074248) and sets its first field to 1. */
void func_80074318(Unk80074248 *this) {
    func_80074248(this);
    this->unk0 = 1;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074344);

/* Loads "/zascii.tim" (D_800663D0), uploads its TIMs to VRAM and frees it. */
void func_8007434C(void) {
    u32 *file = loadFile(D_800663D0);

    uploadTims(file);
    mainHeapFree(file);
}

/* Loads "/title/input.tim" (D_800663DC), uploads its TIMs to VRAM and frees it. */
void func_80074388(void) {
    u32 *file = loadFile(D_800663DC);

    uploadTims(file);
    mainHeapFree(file);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800743C4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800744BC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007460C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074740);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800747F8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007484C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800748A0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800748A8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800748F0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074938);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074988);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074A9C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074AA8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074C7C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074D44);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074D90);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074E28);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80074F68);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075238);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075304);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007560C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800756F0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007588C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075A54);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075B14);

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x35) to (0xCF, 0x44) of page 12, clut 0x3C68, w and h to each
 * side of (x, y), all its corners of the gray shade (func_80075DF8 passes its object as this, unused). */
void func_80075C18(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
    POLY_FT4 *poly = allocPolyFT4();
    s32 left, top;

    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    /* The match depends on doubling w and h in place: left + w * 2 folds to x + w. */
    left = x - w;
    top = y - h;
    w *= 2;
    h *= 2;
    poly->x0 = left;
    poly->y0 = top;
    poly->x1 = left + w;
    poly->y1 = top;
    poly->x2 = left;
    poly->y2 = top + h;
    poly->x3 = left + w;
    poly->y3 = top + h;
    poly->u0 = 0xC0;
    poly->v0 = 0x35;
    poly->u1 = 0xCF;
    poly->v1 = 0x35;
    poly->u2 = 0xC0;
    poly->v2 = 0x44;
    poly->u3 = 0xCF;
    poly->v3 = 0x44;
    /* The match depends on setting tpage before clut. */
    poly->tpage = 0xC;
    poly->clut = 0x3C68;
    AddPrim(ot, poly);
}

/* Draws in ot a POLY_FT4 of the texels (0xC0, 0x20) to (0xFF, 0x34) of page 12, clut 0x3CA8, w and h to each
 * side of (x, y), all its corners of the gray shade (func_80075DF8 passes its object as this, unused). */
void func_80075D08(void *this, u_long *ot, s32 x, s32 y, s32 w, s32 h, s32 shade) {
    POLY_FT4 *poly = allocPolyFT4();
    s32 left, top;

    poly->r0 = shade;
    poly->g0 = shade;
    poly->b0 = shade;
    /* The match depends on doubling w and h in place: left + w * 2 folds to x + w. */
    left = x - w;
    top = y - h;
    w *= 2;
    h *= 2;
    poly->x0 = left;
    poly->y0 = top;
    poly->x1 = left + w;
    poly->y1 = top;
    poly->x2 = left;
    poly->y2 = top + h;
    poly->x3 = left + w;
    poly->y3 = top + h;
    poly->u0 = 0xC0;
    poly->v0 = 0x20;
    poly->u1 = 0xFF;
    poly->v1 = 0x20;
    poly->u2 = 0xC0;
    poly->v2 = 0x34;
    poly->u3 = 0xFF;
    poly->v3 = 0x34;
    /* The match depends on setting tpage before clut. */
    poly->tpage = 0xC;
    poly->clut = 0x3CA8;
    AddPrim(ot, poly);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075DF8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075EC8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075F74);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075F80);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075F8C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075F98);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075FA4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80075FAC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076040);

/* Sorts the nodes at 8 (func_800764BC) and keeps the first at 0xC. */
void func_80076138(Unk800764BC *this) {
    this->unkC = func_800764BC(this);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076160);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007618C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076220);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076298);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076310);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076380);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800764BC);

/* Orders the nodes by their word at 4, largest first. */
s32 func_800764F4(Unk800764F4 *a, Unk800764F4 *b) {
    return b->unk4 - a->unk4;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", creditsTaskInit);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800766A4);

/* The number of line breaks in text: creditsTaskInit's count of the credits' lines. */
s32 func_80076728(const u8 *text) {
    s32 lines = 0;
    u8 c;

    while ((c = *text++) != '\0') {
        if (c == '\n') {
            lines++;
        }
    }
    return lines;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076764);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800767AC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076858);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076B0C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076C14);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076CC4);

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_80076DF0(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80076E24);

extern EntityVtable D_8006659C;

/* Destroys a scene object and frees it, as `delete` does: nothing for NULL (scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* The destructor of the title's character model entity (vtable D_8006659C): deletes the model and the
 * character. */
void func_80076F6C(Unk80076E24 *this, s32 flags) {
    this->entity.vtable = &D_8006659C;
    sceneObjectDelete(&this->model->group.object);
    if (this->chara != NULL) {
        charaDestroy(this->chara, DESTROY_DELETE);
    }
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077000);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077208);

/* The update (entry 0x10) of vtable D_8006659C: animates the model and draws it in the camera at 0x18. */
void func_8007731C(Unk800773FC *this) {
    Model *model = this->model;

    modelUpdateAnimation(model);
    SCENE_OBJECT_INVALIDATE(&model->group.object);
    sceneObjectDrawInView(&model->group.object, this->camera);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077378);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800773C8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800773E8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800773F0);

/* Returns whether the model's animation has stopped; nothing calls it. */
s32 func_800773FC(Unk800773FC *this) {
    return modelIsStopped(this->model);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077420);

/* Copies the two VRAM rects of entry index of D_8007B2DC to (360, 0) and (320, 254) with DR_MOVEs in the camera's
 * ordering table. */
void func_800774D4(Unk80076E24 *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *D_8007B2DC[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x168, 0));
    rect = *D_8007B2DC[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFE));
}

/* Copies the two VRAM rects of entry index of D_8007B2FC to (368, 0) and (320, 253) with DR_MOVEs in the camera's
 * ordering table. */
void func_800775AC(Unk80076E24 *this, s32 index) {
    u_long *ot = this->camera->ot.slots;
    RECT rect;

    rect = *D_8007B2FC[index][0];
    AddPrim(ot, allocDrMove(&rect, 0x170, 0));
    rect = *D_8007B2FC[index][1];
    AddPrim(ot, allocDrMove(&rect, 0x140, 0xFD));
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077684);

extern s32 D_8007B32C;

/* The x of the menu row index: 45, or 35 on every other row. */
s32 func_80077690(s32 index) {
    return 0x2D - ((index + D_8007B32C) & 1) * 10;
}

/* The y of the menu row index: 17 pixels a row, from 51. */
s32 func_800776BC(s32 index) {
    return (index + D_8007B32C) * 17 + 0x33;
}

/* The same as func_80073390. */
SprtColorWords *func_800776DC(u_long *ot, s32 x, s32 y, Unk80073390Image *image) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut;
    sprt->wh = image->wh;
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* func_800776DC's 16 by 16 version: draws cell index of image (u moved by 16 per cell) at (x, y). */
SprtColorWords *func_80077778(u_long *ot, s32 x, s32 y, Unk80073390Image *image, s32 index) {
    SprtColorWords *sprt = (SprtColorWords *)allocSprt16();

    sprt->xy = (y << 16) | (x & 0xFFFF);
    sprt->uvClut = image->uvClut + (index << 4);
    AddPrim(ot, sprt);
    AddPrim(ot, allocDrTpage(image->tpage));
    return sprt;
}

/* A title stepper class with Stepper as a virtual base, reached through the pointer at 0x00 (as in
 * StepperGroup). */
typedef struct Unk80077820 {
    /* 0x00 */ void *stepper; /* the virtual base Stepper */
} Unk80077820;

void func_8007799C(Unk80077820 *this);

/* The stepUp override in D_80066674 and three more tables: steps the value up, playing sound 0x13 when it
 * moved, then func_8007799C. */
void func_80077820(Unk80077820 *this, s32 arg, s32 held, u32 buttons) {
    if (stepperStepUpClamp(this->stepper) == 0) {
        soundControlPlaySound(&SOUND_CONTROL, 0x13, 0);
    }
    func_8007799C(this);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007786C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800778F8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007799C);

/* Entry 0x1C of D_80066674 and seven more tables: plays sound 0xF. */
void func_80077A88(void) {
    soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077AB4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077AC8);

/* The confirm method (entry 0x8) of D_80066914: plays sound 0x10 unless own is set; always accepts. */
s32 func_80077BF8(void *this, s32 arg, s32 held, s32 own) {
    /* The match depends on the early return, func_8006A0E0's shape: the return label it leaves keeps
     * the final v0 = 0 out of the epilogue's load delay slot. */
    if (own != 0) {
        return 0;
    }
    soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
    return 0;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077C2C);

/* engine getCos, inlined.
 * The match depends on this s16 inline: a table read in the expression is narrowed to lhu + sll + sra. */
static inline s16 getCosInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].cos;
}

/* The item's brightness: 128 while it is current, 64 otherwise, swung by 1/128 of the blink's cosine. */
s32 func_80077C44(Unk80077AC8 *this, s32 isCurrent) {
    s32 swing = getCosInline(this->blink) >> 7;

    /* The match depends on testing !isCurrent: it keeps the 128 return on the fall-through path. */
    if (!isCurrent) {
        return swing + 0x40;
    }
    return swing + 0x80;
}

/* Draw: the item's image at its row, its brightness from func_80077C44; an inactive item stops blinking. */
void func_80077C78(Unk80077AC8 *this, u_long *ot, s32 isCurrent, s32 active) {
    s32 index = this->stepper->index;
    SPRT *sprt = (SPRT *)func_800776DC(ot, func_80077690(index), func_800776BC(index), this->image);
    s32 brightness;

    if (!active) {
        this->blink = 0;
    }
    brightness = func_80077C44(this, isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077D20);

/* Draw: func_80077C78's, then the image of the value 128 pixels right of it, as bright. */
void func_80077E78(Unk80077D20 *this, u_long *ot, s32 isCurrent, s32 active) {
    StepperMembers *stepper;
    Unk80073390Image **image;
    s32 index;
    SPRT *sprt;
    s32 brightness;

    func_80077C78(&this->base, ot, isCurrent, active);
    stepper = this->base.stepper;
    image = &this->images[stepper->value];
    index = stepper->index;
    sprt = (SPRT *)func_800776DC(ot, func_80077690(index) + 0x80, func_800776BC(index), *image);
    brightness = func_80077C44(&this->base, isCurrent);
    setRGB0(sprt, brightness, brightness, brightness);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80077F28);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078078);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800780F4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078160);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078168);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800782D8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007841C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078448);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800784A0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007850C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078628);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800786E4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078848);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800788A0);

/* The draw method of the menu of virtual table D_80066794: func_80077C78's. */
void func_800788F4(Unk80077AC8 *this, u_long *ot, s32 isCurrent, s32 active) {
    func_80077C78(this, ot, isCurrent, active);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078914);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078A4C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078B84);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078C64);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078CB8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078D20);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078E3C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80078F5C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079254);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_800793D0);

/* The input method of the menu of virtual table D_80066674: StepperGroup's. */
s32 func_8007943C(void *this, s32 arg, s32 held, u32 buttons) {
    return stepperGroupInput(this, arg, held, buttons);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007945C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", optionTaskInit);

/* The destructor of Unk800797D8 (task vtable D_80066974): the same steps as func_8006A62C. */
void func_800797D8(Unk800797D8 *this, s32 flags) {
    this->task.vtable = &D_80066974;
    fadeControlFadeIn(&FADE_CONTROL, 0);
    spriteDestroy(&this->sprite, DESTROY_BASES);
    stepperGroupDestroy(this->steppers, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079850);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079978);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079B64);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079CC8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079E2C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_80079F9C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A100);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A264);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A3B0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A3B8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A3C0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A41C);

/* The refresh method of the menu of virtual table D_80066674: StepperGroup's. */
void func_8007A44C(void *this, s32 arg) {
    stepperGroupRefresh(this, arg);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A46C);

/* Unk8007A46C's destructor: frees the block at 0x18, then destroys the Cache. */
void func_8007A534(Unk8007A46C *this, s32 flags) {
    this->cache.vtable = &D_80066B68;
    mainHeapFree(this->unk18);
    cacheDestroy(&this->cache, flags);
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A580);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A6A0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A6EC);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A82C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A8C4);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007A9C8);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007ABB0);

/* Copies the rectangle at 0x34 to out. */
void func_8007AF88(Unk8007A46C *this, RECT *out) {
    *out = this->unk34;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007AFB0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007AFBC);

/* Sets the rectangle at 0x40 (24 by 24 at first) from rect. */
void func_8007AFE0(Unk8007A46C *this, RECT *rect) {
    this->unk40 = *rect;
}

/* Sets the color at 0x48 to r, g, b with 0x80 in the fourth byte. */
void func_8007B008(Unk8007A46C *this, u8 r, u8 g, u8 b) {
    this->unk48[0] = r;
    this->unk48[1] = g;
    this->unk48[2] = b;
    this->unk48[3] = 0x80;
}

/* Sets the four bytes of the color at 0x48. */
void func_8007B020(Unk8007A46C *this, u8 r, u8 g, u8 b, u8 a) {
    this->unk48[0] = r;
    this->unk48[1] = g;
    this->unk48[2] = b;
    this->unk48[3] = a;
}

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B038);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B040);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B048);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B0A0);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B100);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B11C);

INCLUDE_ASM("asm/jp/title/nonmatchings/title", func_8007B184);

/* The destructor of the Printer class of virtual table D_80066B88: Printer's. */
void func_8007B210(Printer *this, s32 flags) {
    printerDestroy(this, flags);
}
