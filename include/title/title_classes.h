#ifndef DTBE_TITLE_TITLE_CLASSES_H
#define DTBE_TITLE_TITLE_CLASSES_H

/* Classes of title.bin, known only by their layout and named after a function that uses them. */

#include "common.h"
#include "engine/game/character.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/scene_graph.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"
#include "engine/text/text_printer.h"

EXTERN_C_BEGIN

/* A picture of a title screen's texture: its texture page, its corner in the page, its CLUT and its size.
 * drawImage and its copies (guideDrawImage, optionDrawImage, mainMenuDrawImage) copy the uv, the clut and
 * the size into a SPRT as words; the main menu's quads read them one by one. The tables of them (the
 * title's pictures GUIDE_IMAGES, RANKING_IMAGES) have more after. */
typedef struct TitleImage {
    /* 0x0 */ u16 tpage;
    /* 0x2 */ u8 unk2[2];
    /* 0x4 */ union {
        u32 word; /* as a SPRT has them */
        struct {
            u8 u;
            u8 v;
            u16 clut;
        } part;
    } uvClut;
    /* 0x8 */ union {
        u32 word;
        struct {
            s16 w;
            s16 h;
        } part;
    } wh;
} TitleImage;

/* The constructor of StepperGroup (engine/menu/stepper.h), a C++ class, as the C of title.bin calls it (inCharge
 * 1: it builds the virtual base too). */
void *stepperGroupInit(void *group, s32 inCharge, const void *buttons);

/* The destructor of StepperGroup (engine/menu/stepper.h), a C++ class, as the C of title.bin calls it. */
void stepperGroupDestroy(void *group, s32 flags) __asm__("_._12StepperGroup");

/* A character model of the title (TitleChara_init, TITLE_CHARA_VTABLE): character 0x18's model seen by
 * camera, which the character select and the guide show. It blinks, moves its mouth while its voice plays
 * and plays the action clips the screens ask for; TitleChara_setEyes and TitleChara_setMouth copy their
 * pictures through the camera's ordering table. */
typedef struct TitleChara {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Camera *camera;
    /* 0x1C */ Chara *chara;
    /* 0x20 */ Model *model;
    /* 0x24 */ s16 frame;      /* counted every tick: a new mouth every 4 while talking */
    /* 0x26 */ s16 nextAction; /* TITLE_CHARA_*: the action TitleChara_setAction asks for */
    /* 0x28 */ s16 action;     /* the action playing (TitleChara_getAction) */
    /* 0x2A */ u8 unk2A[0x2C - 0x2A];
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ u16 eyes;       /* the eye picture (TITLE_CHARA_EYES) */
    /* 0x32 */ u16 blinkState; /* 0 while blinking, 1 while waiting for the next blink */
    /* 0x34 */ u16 blinkWait;  /* frames until the next blink, random */
    /* 0x36 */ u16 blinkTimer;
    /* 0x38 */ s32 interruptible; /* set: a new action cuts the one playing and its voice */
    /* 0x3C */ s32 voice;      /* the handle of the voice line playing (TitleChara_setVoice) */
} TitleChara; /* size 0x40 */

/* the actions the screens ask for (TitleChara_setAction); 1, 9 and 10 are other clips */
#define TITLE_CHARA_IDLE 0
#define TITLE_CHARA_TALK 11 /* talks: the mouth moves until the voice ends */

TitleChara *TitleChara_init(TitleChara *chara, Camera *camera, Vec3 *position, Vec3 *rotation);
void TitleChara_setAction(TitleChara *chara, s32 action);
void TitleChara_setVoice(TitleChara *chara, s32 voice);
s32 TitleChara_getAction(TitleChara *chara);

/* sets the matrix of a title character's model from a position and the angles about x, y and z */
void TitleChara_setTransform(TitleChara *chara, Vec3 *position, Vec3 *rotation);

/* Calls the invalidate method of a scene object (its world moved). */
#define SCENE_OBJECT_INVALIDATE(object) \
    (object)->vtable->invalidate.func((u8 *)(object) + (object)->vtable->invalidate.delta)

/* The name entry's object (constructor NameEntry_init, destructor NameEntry_destroy): a member of the
 * ranking's object (at 0x38, RankingTask_destroy destroys it) and of an options item (at 0xC). A grid of
 * characters scrolls in a window; the name is a string of two-byte characters. */
typedef struct NameEntry {
    /* 0x00 */ s32 active;    /* updated and drawn only while set (NameEntry_open sets it) */
    /* 0x04 */ s32 finished;  /* the name was confirmed: the entry closes 11 frames later */
    /* 0x08 */ s32 mode;      /* NAME_ENTRY_MODE_*: the grid (NAME_GRID or CODE_GRID), title and background */
    /* 0x0C */ s32 maxLength; /* in characters */
    /* 0x10 */ s32 position;  /* the character being entered */
    /* 0x14 */ u8 name[0x24];
    /* 0x38 */ s32 closeDelay; /* countdown from the confirmation to finished */
    /* 0x3C */ s32 padDelay;  /* frames before the pad is read */
    /* 0x40 */ s32 frame;     /* frames since the entry opened (bit 3 blinks the name's cursor) */
    /* 0x44 */ s32 closingFrames; /* frames since finished: the entry closes at 11 */
    /* 0x48 */ s32 message;   /* which of input.tim's 192 x 48 messages NameEntry_drawMessage shows */
    /* 0x4C */ s32 messageTime; /* a countdown: NameEntry_drawMessage draws while it runs */
    /* 0x50 */ u8 **grid;     /* its rows of two-byte cells (NameEntry_setGrid) */
    /* 0x54 */ s32 columns;   /* of the grid, 20 pixels each */
    /* 0x58 */ s32 rows;
    /* 0x5C */ s32 port;      /* the pad read */
    /* 0x60 */ s32 score;     /* the player's, given to RankingTable_insertScore */
    /* 0x64 */ s32 character; /* the player's, stored in the record */
    /* 0x68 */ s32 rank;      /* RankingTable_insertScore's: 1-based, 0 when the score does not rank */
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
    /* 0x84 */ s32 cellCode;  /* the two-byte character under the cursor, 0 for a blank */
    /* 0x88 */ s32 lastCell;  /* NameEntry_readCell's last result: a change plays the cursor sound */
    /* 0x8C */ s32 cellButton; /* the button under the cursor: -1 A (end), -2 B (erase), -3 C (exit) */
    /* 0x90 */ s32 speed;     /* the cursor moves speed >> 8 pixels a frame */
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
} NameEntry;

/* what the name entry is for: the grid, title and background it shows */
#define NAME_ENTRY_MODE_NAME 0 /* a name for the ranking */
#define NAME_ENTRY_MODE_CODE 1 /* a code, from the options */

/* NameEntry.cellButton: the grid's buttons */
#define NAME_ENTRY_BUTTON_END (-1)   /* A: confirms the name (a code is tried) */
#define NAME_ENTRY_BUTTON_ERASE (-2) /* B: erases a character */
#define NAME_ENTRY_BUTTON_EXIT (-3)  /* C: leaves the code entry */

void NameEntry_destroy(NameEntry *entry, s32 flags);

/* The table of the ranking's records (constructor RankingTable_init, destructor RankingTable_destroy), sorted by
 * RankingTable_sort: the ranking (at 0x24) and memoryCardTaskInit build one over GAME_STATE.save.records. */
typedef struct RankingTable {
    /* 0x00 */ s32 count;
    /* 0x04 */ struct RankingRecord *entries;
    /* 0x08 */ struct RankingRecord *head;    /* the first of the sorted list */
    /* 0x0C */ struct RankingRecord *cursor;  /* RankingTable_first starts it at head, RankingTable_next steps */
    /* 0x10 */ struct RankingRecord *pending; /* the record RankingTable_fillPending fills next */
} RankingTable;

/* a record of the table, a node of the list RankingTable_sort sorts (compareRankingRecords orders them) */
typedef struct RankingRecord {
    /* 0x00 */ struct RankingRecord *next;
    /* 0x04 */ s32 score;     /* the sort key, largest first */
    /* 0x08 */ s32 character; /* whose face the ranking draws */
    /* 0x0C */ u8 name[0x24];
    /* 0x30 */ u8 digest[0x10]; /* the MD5 of 0x4 to 0x30 (computeRecordDigest) */
} RankingRecord;

RankingTable *RankingTable_init(RankingTable *table, RankingRecord *entries, u32 count);
void RankingTable_destroy(RankingTable *table, s32 flags);
s32 RankingTable_insertScore(RankingTable *table, s32 score);
RankingRecord *RankingTable_first(RankingTable *table);
RankingRecord *RankingTable_next(RankingTable *table);

EXTERN_C_END

#ifdef __cplusplus

/* The zascii printer (ZasciiPrinter_init, ZASCII_PRINTER_VTABLE, zascii_printer.c): a Printer that draws its
 * characters as 16 by 16 sprites of /zascii.tim into ot. The name entry builds one on the stack. The C files
 * see it as the struct in the #else part. */
class ZasciiPrinter : public Printer {
public:
    /* 0x0C */ u_long *ot;
    /* 0x10 */ u32 color; /* 0xBBGGRR, with the semi-transparency bit of the code (0x02) above */

    ZasciiPrinter(u_long *ot) __asm__("ZasciiPrinter_init");
};

#else

/* The zascii printer (ZasciiPrinter_init, ZASCII_PRINTER_VTABLE, zascii_printer.c): a Printer that draws its
 * characters as 16 by 16 sprites of /zascii.tim into ot. The name entry builds one on the stack. */
typedef struct ZasciiPrinter {
    /* 0x00 */ Printer printer;
    /* 0x0C */ u_long *ot;
    /* 0x10 */ u32 color; /* 0xBBGGRR, with the semi-transparency bit of the code (0x02) above */
} ZasciiPrinter;

#endif /* __cplusplus */

EXTERN_C_BEGIN

#ifndef __cplusplus
ZasciiPrinter *ZasciiPrinter_init(ZasciiPrinter *printer, u_long *ot);
#endif
void ZasciiPrinter_drawChar(ZasciiPrinter *printer, s32 x, s32 y, u32 code);

#ifndef __cplusplus
/* Tasks whose update keeps its argument and calls their own per-frame method, tick (at 0x20 of the virtual
 * table): BOOT_LOGO_TASK_VTABLE, TITLE_LOGO_TASK_VTABLE, _vt.19CharacterSelectTask,
 * GUIDE_LOGO_TASK_VTABLE, RANKING_TASK_VTABLE and CREDITS_TASK_VTABLE here, and BOOT_LOGO_SPRITES in game.bin
 * (Unk8006ECEC there). The same update in every one of them is probably an inline virtual of a shared header,
 * written out once per source file. */
typedef struct TickTaskVtable {
    /* 0x00 */ TaskVtable task;
    /* 0x20 */ VtableEntry tick;
} TickTaskVtable;

typedef struct TickTask {
    /* 0x00 */ ListNode link;
    /* 0x08 */ u8 unk8[0x1C - 0x8];
    /* 0x1C */ TickTaskVtable *vtable;
    /* 0x20 */ s32 updateArg; /* the argument of the last update */
} TickTask;
#endif

EXTERN_C_END

#endif /* DTBE_TITLE_TITLE_CLASSES_H */
