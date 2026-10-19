#ifndef DTBE_TITLE_MEMORY_CARD_H
#define DTBE_TITLE_MEMORY_CARD_H

/* The memory card task, its message and yes/no windows, and the reset lock (title/memory_card.cpp). */

#ifdef __cplusplus
/* MemoryCardTask's virtual table stays in the overlay's assembly (.rodata), so memory_card.cpp writes it
 * out nowhere. */
#pragma interface
#endif

#include "common.h"
#include <libgpu.h>
#include "engine/text/text_printer.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/task/task.h"

EXTERN_C_BEGIN

/* A message window (MessageWindow_init, MESSAGE_WINDOW_VTABLE): a rectangle that grows open and shrinks
 * closed (MessageWindow_tick) with lines of text centred in it (MessageWindow_draw). The four values of the
 * constructor are the rectangle, as MessageWindow_draw copies them into the RECT it draws. */
typedef struct MessageWindow {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 scale; /* how far open, 0 (closed) to 0x1000 */
    /* 0x1C */ s32 open;  /* nonzero while opening or open, 0 while closing */
    /* 0x20 */ u8 *text;  /* the lines it shows, NULL for none */
    /* 0x24 */ s32 x;
    /* 0x28 */ s32 y;
    /* 0x2C */ s32 width;
    /* 0x30 */ s32 height;
} MessageWindow;

/* The yes/no window (YesNoWindow_init, YES_NO_WINDOW_VTABLE): a message window with two choices under the
 * text, STR_YES and STR_NO, picked with up and down (YesNoWindow_tick). */
typedef struct YesNoWindow {
    /* 0x00 */ MessageWindow window;
    /* 0x34 */ s32 choice;  /* 0 for yes (the first choice), 1 for no */
    /* 0x38 */ s32 decided; /* set once the player confirmed a choice */
} YesNoWindow;

extern EntityVtable MESSAGE_WINDOW_VTABLE;
extern EntityVtable YES_NO_WINDOW_VTABLE;

#ifdef __cplusplus
class MemoryCardTask;
#else
typedef struct MemoryCardTask MemoryCardTask;
#endif
/* the task memoryCardTaskInit builds, while it lives */
extern MemoryCardTask *MEMORY_CARD_TASK;

MessageWindow *MessageWindow_init(MessageWindow *window, s32 x, s32 y, s32 width, s32 height);
void MessageWindow_printLine(MessageWindow *window, Printer *printer, u8 *text, s32 offsetY);
void MessageWindow_show(MessageWindow *window, u8 *text);
void MessageWindow_open(MessageWindow *window);
void MessageWindow_close(MessageWindow *window);
s32 MessageWindow_isOpen(MessageWindow *window);
void MessageWindow_tick(MessageWindow *window);
void scaleRect(RECT *rect, s32 scale);
s32 countLines(const u8 *text);
u8 *copyLine(u8 *line, u8 *text);
void MessageWindow_draw(MessageWindow *window, s32 arg);
void YesNoWindow_open(YesNoWindow *question, s32 defaultYes);
s32 YesNoWindow_isDecided(YesNoWindow *question);
s32 YesNoWindow_isYes(YesNoWindow *question);
void YesNoWindow_tick(YesNoWindow *question);
void YesNoWindow_draw(YesNoWindow *question, s32 arg);
void memoryCardThreadEntry(s32 unused, void *object);
void memoryCardCallback(long cmds, long result);
u8 *buildSaveFile(u32 *fileSize, char *title, u32 *icon, u8 *data, s32 size);
void MessageWindow_destroy(Entity *window, s32 flags);
void YesNoWindow_destroy(Entity *window, s32 flags);
YesNoWindow *YesNoWindow_init(YesNoWindow *question, s32 x, s32 y, s32 width, s32 height);

EXTERN_C_END

#ifdef __cplusplus

/* Keeps the reset buttons off while it lives: the card's format, read and write make one on their stack
 * over SYSTEM_CONTEXT.resetDisabled (D_8005FCB0), so the game is not reset in the middle of a card write. */
class ResetLock {
public:
    /* 0x0 */ s32 *flag; /* set to 1 by the constructor, back to 0 by the destructor */

    ResetLock(s32 *flag) __asm__("ResetLock_init");
    ~ResetLock();
};

/* The memory card task (memoryCardTaskInit). Its thread (run) does the work of the mode it was made with and
 * shows what happens in its two windows; the card callback memoryCardCallback hands it the results the thread
 * waits for (waitResult). */
class MemoryCardTask : public Task {
public:
    /* 0x20 */ s32 mode;    /* the work the thread does (run): MEMORY_CARD_* */
    /* 0x24 */ Thread thread;
    /* 0x30 */ s32 channel; /* what the card calls get */
    /* 0x34 */ EntityGroup windows;
    /* 0x44 */ s32 done;    /* set once the thread returned */
    /* 0x48 */ MessageWindow *message;
    /* 0x4C */ YesNoWindow *question;
    /* Set by the memory card callback memoryCardCallback and polled by the thread, so volatile: waitResult
     * stores 0x50 and loads it right back, and memoryCardCallback's last store stays out of the delay slot
     * of its `jr ra`. */
    /* 0x50 */ volatile s32 cmds;   /* the command the callback reported (MemCB's cmds), 0 until it ran */
    /* 0x54 */ volatile s32 result; /* its result (MemCB's rslt) */

    MemoryCardTask(s32 mode) __asm__("memoryCardTaskInit");
    ~MemoryCardTask();
    s32 isDone() __asm__("MemoryCardTask_isDone");
    void run() __asm__("MemoryCardTask_run");
    s32 formatCard(s32 askFirst) __asm__("MemoryCardTask_formatCard");
    void showMessage(u8 *text) __asm__("MemoryCardTask_showMessage");
    void runOptionLoad() __asm__("MemoryCardTask_runOptionLoad");
    void runOptionSave() __asm__("MemoryCardTask_runOptionSave");
    void runLoad() __asm__("MemoryCardTask_runLoad");
    void runSave() __asm__("MemoryCardTask_runSave");
    void load(s32 timed) __asm__("MemoryCardTask_load");
    void save() __asm__("MemoryCardTask_save");
    s32 readSave() __asm__("MemoryCardTask_readSave");
    s32 writeSave() __asm__("MemoryCardTask_writeSave");
    s32 checkResult(s32 result, u8 *text) __asm__("MemoryCardTask_checkResult");
    s32 countFiles(char *name) __asm__("MemoryCardTask_countFiles");
    s32 getFreeBlocks() __asm__("MemoryCardTask_getFreeBlocks");
    s32 openOrCreateFile(char *name, s32 blocks, s32 tries) __asm__("MemoryCardTask_openOrCreateFile");
    s32 isCardPresent() __asm__("MemoryCardTask_isCardPresent");
    s32 acceptCard() __asm__("MemoryCardTask_acceptCard");
    s32 waitResult() __asm__("MemoryCardTask_waitResult");
    void waitButton() __asm__("MemoryCardTask_waitButton");
    void wait(s32 frames, s32 skippable) __asm__("MemoryCardTask_wait");
    s32 ask(s32 defaultYes) __asm__("MemoryCardTask_ask");
    void update(s32 arg) __asm__("MemoryCardTask_update");
};

#endif /* __cplusplus */

#endif /* DTBE_TITLE_MEMORY_CARD_H */
