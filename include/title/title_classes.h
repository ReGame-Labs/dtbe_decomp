#ifndef DTBE_TITLE_TITLE_CLASSES_H
#define DTBE_TITLE_TITLE_CLASSES_H

/* Classes of title.bin, known only by their layout and named after a function that uses them. */

#include "common.h"
#include "engine/gfx/scene_graph.h"
#include "engine/task/task.h"
#include "engine/text/text_printer.h"

EXTERN_C_BEGIN

/* An image that func_80073390 (and its copies func_80071CB8 and func_800776DC) draws: its texture page,
 * then the uv, clut, width and height of its SPRT as words, copied as they are. The tables of them
 * (the title's pictures D_80065EF4, D_80066024) have more after. */
typedef struct Unk80073390Image {
    /* 0x0 */ u16 tpage;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ u32 uvClut; /* u0, v0, then the clut */
    /* 0x8 */ u32 wh;     /* w, then h */
} Unk80073390Image;

/* The destructor of StepperGroup (engine/menu/stepper.h), a C++ class, as the C of title.bin calls it. */
void stepperGroupDestroy(void *group, s32 flags) __asm__("_._12StepperGroup");

/* sets the matrix of the model of a title character model entity (title_0a.c) from a position and the angles
 * about x, y and z */
struct Unk80076E24;
void func_80077420(struct Unk80076E24 *object, Vec3 *position, Vec3 *rotation);

/* Calls the invalidate method of a scene object (its world moved). */
#define SCENE_OBJECT_INVALIDATE(object) \
    (object)->vtable->invalidate.func((u8 *)(object) + (object)->vtable->invalidate.delta)

/* The name entry's object (constructor func_80074130, destructor func_80074220): a member of the
 * ranking's object (at 0x38, func_80073784 destroys it) and of an options item (at 0xC). A grid of
 * characters scrolls in a window; the name is a string of two-byte characters. */
typedef struct Unk80074220 {
    /* 0x00 */ s32 active;    /* updated and drawn only while set (func_80074318 sets it) */
    /* 0x04 */ s32 finished;  /* the name was confirmed: the entry closes 11 frames later */
    /* 0x08 */ s32 unk8;      /* picks the character grid (D_8007B250 when 0, else D_8007B280) */
    /* 0x0C */ s32 maxLength; /* in characters */
    /* 0x10 */ s32 position;  /* the character being entered */
    /* 0x14 */ u8 name[0x24];
    /* 0x38 */ s32 unk38;     /* countdown from the confirmation to finished */
    /* 0x3C */ s32 unk3C;     /* frames before the pad is read */
    /* 0x40 */ s32 frame;     /* frames since the entry opened (bit 3 blinks the name's cursor) */
    /* 0x44 */ s32 unk44;     /* frames since finished */
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;     /* a countdown: func_80075B14 draws while it runs */
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 columns;   /* of the grid, 20 pixels each */
    /* 0x58 */ s32 rows;
    /* 0x5C */ s32 port;      /* the pad read */
    /* 0x60 */ s32 unk60;
    /* 0x64 */ s32 unk64;
    /* 0x68 */ s32 unk68;
    /* 0x6C */ u32 held;      /* this frame's pad buttons */
    /* 0x70 */ u32 repeated;
    /* 0x74 */ s16 windowX;   /* where the grid is shown */
    /* 0x76 */ s16 windowY;
    /* 0x78 */ s16 windowW;
    /* 0x7A */ s16 windowH;
    /* 0x7C */ s16 boundsX;   /* where the cursor stays; the grid scrolls beyond it */
    /* 0x7E */ s16 boundsY;
    /* 0x80 */ s16 boundsW;
    /* 0x82 */ s16 boundsH;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 unk8C;
    /* 0x90 */ s32 unk90;
    /* 0x94 */ s32 column;    /* the cell of the grid under the cursor */
    /* 0x98 */ s32 row;
    /* 0x9C */ s32 cursorX;   /* on screen */
    /* 0xA0 */ s32 cursorY;
    /* 0xA4 */ s32 scrollX;
    /* 0xA8 */ s32 scrollY;
    /* 0xAC */ s32 maxScrollX;
    /* 0xB0 */ s32 maxScrollY;
    /* 0xB4 */ s32 moveTime;  /* 0x1000 down to 0 while the cursor moves from moveFrom to moveTo */
    /* 0xB8 */ s32 moveToX;
    /* 0xBC */ s32 moveToY;
    /* 0xC0 */ s32 moveFromX;
    /* 0xC4 */ s32 moveFromY;
    /* 0xC8 */ s32 moveLastX; /* the point of the move reached last frame */
    /* 0xCC */ s32 moveLastY;
} Unk80074220;

void func_80074220(Unk80074220 *entry, s32 flags);

/* The table of the ranking's records (constructor func_80075FAC, destructor func_80076040), sorted by
 * func_800764BC: the ranking (at 0x24) and memoryCardTaskInit build one over GAME_STATE.save.records. */
typedef struct Unk800764BC {
    /* 0x00 */ s32 count;
    /* 0x04 */ struct Unk800764F4 *entries;
    /* 0x08 */ struct Unk800764F4 *head;    /* the first of the sorted list */
    /* 0x0C */ struct Unk800764F4 *cursor;  /* func_80076138 starts it at head, func_80076160 steps it */
    /* 0x10 */ struct Unk800764F4 *pending; /* the record func_80076220 fills next */
} Unk800764BC;

/* a record of the table, a node of the list func_800764BC sorts (func_800764F4 orders them) */
typedef struct Unk800764F4 {
    /* 0x00 */ struct Unk800764F4 *next;
    /* 0x04 */ s32 unk4; /* the sort key, largest first */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ u8 name[0x24];
    /* 0x30 */ u8 digest[0x10]; /* the MD5 of 0x4 to 0x30 (func_80076310) */
} Unk800764F4;

Unk800764BC *func_80075FAC(Unk800764BC *table, Unk800764F4 *entries, u32 count);
void func_80076040(Unk800764BC *table, s32 flags);

/* The Printer of virtual table D_80066B88 (title_0c.c), which func_8007B0A0 builds: it draws its characters
 * as 16 by 16 sprites into ot. The name entry builds one on the stack. */
typedef struct Unk8007B0A0 {
    /* 0x00 */ Printer printer;
    /* 0x0C */ u_long *ot;
    /* 0x10 */ u32 color; /* 0xBBGGRR, with the semi-transparency bit of the code (0x02) above */
} Unk8007B0A0;

Unk8007B0A0 *func_8007B0A0(Unk8007B0A0 *printer, u_long *ot);

#ifndef __cplusplus
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
#endif

EXTERN_C_END

#endif /* DTBE_TITLE_TITLE_CLASSES_H */
