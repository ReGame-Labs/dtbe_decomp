/* The memory card screens (memoryCardTaskInit): checking, formatting, loading and saving, with their
 * messages. */

#include "common.h"
#include "kernel.h"
#include "psyq.h"
#include "libmcrd.h"
#include "memory.h"
#include "strings.h"
#include "engine/gfx/box.h"
#include "engine/game/game_state.h"
#include "engine/game/sequencer.h"
#include "engine/gfx/ordering_table.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"

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

/* The memory card task (memoryCardTaskInit, MEMORY_CARD_TASK_VTABLE). Its thread (MemoryCardTask_run) does
 * the work of the mode it was made with and shows what happens in its two windows; the card callback
 * memoryCardCallback hands it the results the thread waits for (MemoryCardTask_waitResult). */
typedef struct MemoryCardTask {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 mode;    /* the work the thread does (MemoryCardTask_run): MEMORY_CARD_* */
    /* 0x24 */ Thread thread;
    /* 0x30 */ s32 channel; /* what the card calls get */
    /* 0x34 */ EntityGroup windows;
    /* 0x44 */ s32 done;    /* set once the thread returned */
    /* 0x48 */ MessageWindow *message;
    /* 0x4C */ YesNoWindow *question;
    /* Set by the memory card callback memoryCardCallback and polled by the thread, so volatile:
     * MemoryCardTask_waitResult stores 0x50 and loads it right back, and memoryCardCallback's last store
     * stays out of the delay slot of its `jr ra`. */
    /* 0x50 */ volatile s32 cmds;   /* the command the callback reported (MemCB's cmds), 0 until it ran */
    /* 0x54 */ volatile s32 result; /* its result (MemCB's rslt) */
} MemoryCardTask;

/* Keeps the reset buttons off while it lives: the card's format, read and write make one on their stack
 * over SYSTEM_CONTEXT.resetDisabled (D_8005FCB0), so the game is not reset in the middle of a card write. */
typedef struct ResetLock {
    /* 0x0 */ s32 *flag; /* set to 1 by ResetLock_init, back to 0 by ResetLock_destroy */
} ResetLock;

/* in .rodata: the yes/no window's choices, messages and a file name pattern */
extern u8 STR_YES[];
extern u8 STR_NO[];
extern u8 STR_LOAD_PROMPT[];
extern u8 STR_CARD_REMOVED[];
extern u8 STR_ANY_FILE[];
extern TaskVtable MEMORY_CARD_TASK_VTABLE;
/* the task memoryCardTaskInit builds, while it lives */
extern MemoryCardTask *MEMORY_CARD_TASK;

/* The buttons that confirm a choice: start or the menus' confirm button. */
#define MEMORY_CARD_CONFIRM (PADstart | PAD_CONFIRM)
/* start, triangle, circle, cross or square */
#define MEMORY_CARD_ANY_BUTTON 0x8F0

/* A TextPrinter's setColor, inlined as the game has it (its virtual table read from the stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)

void MessageWindow_draw(MessageWindow *this, s32 arg);
void memoryCardThreadEntry(s32 unused, void *object);
void MemoryCardTask_run(MemoryCardTask *this);
void MemoryCardTask_runOptionLoad(MemoryCardTask *this);
void MemoryCardTask_runOptionSave(MemoryCardTask *this);
void MemoryCardTask_runLoad(MemoryCardTask *this);
void MemoryCardTask_runSave(MemoryCardTask *this);
void MemoryCardTask_load(MemoryCardTask *this, s32 timed);
void MemoryCardTask_save(MemoryCardTask *this);
void memoryCardCallback(long cmds, long result);
s32 MemoryCardTask_waitResult(MemoryCardTask *this);
void MemoryCardTask_wait(MemoryCardTask *this, s32 frames, s32 skippable);
s32 MemoryCardTask_ask(MemoryCardTask *this, s32 defaultYes);

/* The card file's name ("BISLPS-03357" in Japan), its title and a yes/no pair that no code names. */
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_FILE_NAME);

/* Constructs a closed message window over the rectangle x, y, width, height. */
MessageWindow *MessageWindow_init(MessageWindow *this, s32 x, s32 y, s32 width, s32 height) {
    entityInit(&this->entity, 0);
    this->x = x;
    this->y = y;
    this->width = width;
    this->entity.vtable = &MESSAGE_WINDOW_VTABLE;
    this->scale = 0;
    this->open = 0;
    this->text = NULL;
    this->height = height;
    return this;
}

/* Prints one line of text centred in the window's width, offsetY below its top (8 pixels a character). */
void MessageWindow_printLine(MessageWindow *this, Printer *printer, u8 *text, s32 offsetY) {
    s32 length = strlen(text);
    s32 left = (this->width - length * 8) / 2;
    s32 top = this->y + offsetY; /* The match depends on top computed first and stored last. */

    printer->x = this->x + left;
    printer->y = top;
    printerPrint(printer, text);
}

/* Opens the window on text. */
void MessageWindow_show(MessageWindow *this, u8 *text) {
    this->open = 1;
    this->text = text;
}

/* Opens the window on the text it has. */
void MessageWindow_open(MessageWindow *this) {
    this->open = 1;
}

/* Closes the window. */
void MessageWindow_close(MessageWindow *this) {
    this->open = 0;
}

/* Whether the window is all the way open. */
s32 MessageWindow_isOpen(MessageWindow *this) {
    return this->scale == 0x1000;
}

/* Grows the window open, or shrinks it closed, by 0x111 a frame. */
void MessageWindow_tick(MessageWindow *this) {
    if (this->open) {
        if (this->scale != 0x1000) {
            this->scale += 0x111;
            if (this->scale > 0x1000) {
                this->scale = 0x1000;
            }
        }
    } else {
        this->scale -= 0x111;
        if (this->scale < 0) {
            this->scale = 0;
        }
    }
}

/* Shrinks rect about its centre to scale (0x1000 is the whole), to no less than 1 by 1. */
void scaleRect(RECT *rect, s32 scale) {
    s32 w = rect->w;
    s32 h = rect->h;
    s32 scaledW = (w * scale) >> 12;
    s32 scaledH = (h * scale) >> 12;

    if (scaledW == 0) {
        scaledW = 1;
    }
    if (scaledH == 0) {
        scaledH = 1;
    }
    rect->w = scaledW;
    rect->x += (w - scaledW) / 2;
    rect->h = scaledH;
    rect->y += (h - scaledH) / 2;
}

/* The number of lines in text: its line breaks plus one. */
s32 countLines(const u8 *text) {
    s32 lines = 1;
    u8 c;

    while ((c = *text++) != '\0') {
        if (c == '\n') {
            lines++;
        }
    }
    return lines;
}

/* Copies the first line of text into line; returns the text after its line break, NULL after the
 * last line. */
u8 *copyLine(u8 *line, u8 *text) {
    u8 c = *text++;

    while (c != '\0' && c != '\n') {
        *line++ = c;
        c = *text++;
    }
    *line = '\0';
    return c != '\0' ? text : NULL;
}

INCLUDE_ASM("title/nonmatchings/memory_card", MessageWindow_draw);

/* Opens the yes/no window on yes, or on no when defaultYes is 0. */
void YesNoWindow_open(YesNoWindow *this, s32 defaultYes) {
    MessageWindow_open(&this->window);
    if (defaultYes) {
        this->choice = 0;
    } else {
        this->choice = 1;
    }
    this->decided = 0;
}

/* Whether the player confirmed a choice. */
s32 YesNoWindow_isDecided(YesNoWindow *this) {
    return this->decided;
}

/* Whether the choice is yes (the first one). */
s32 YesNoWindow_isYes(YesNoWindow *this) {
    return this->choice == 0;
}

/* The yes/no window's update: once open, up and down move the choice and the confirm button closes it
 * on that choice. */
void YesNoWindow_tick(YesNoWindow *this) {
    u32 pressed;

    MessageWindow_tick(&this->window);
    if (!MessageWindow_isOpen(&this->window)) {
        return;
    }
    if (YesNoWindow_isDecided(this)) {
        return;
    }
    pressed = padManagerGetPressed(PAD_MANAGER_INSTANCE, -1);
    if (pressed & MEMORY_CARD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        this->decided = 1;
        MessageWindow_close(&this->window);
        return;
    }
    if (this->choice <= 0) {
        if (pressed & PAD_DOWN) {
            this->choice++;
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
        }
    } else {
        if (pressed & PAD_UP) {
            this->choice--;
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
        }
    }
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_YES);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_NO);

/* The yes/no window's draw: once open, its two choices (the one picked bright) under the text. */
void YesNoWindow_draw(YesNoWindow *this, s32 arg) {
    TextPrinter printer;
    u32 firstColor;
    u32 secondColor;

    if (!MessageWindow_isOpen(&this->window)) {
        return;
    }
    firstColor = 0x808080;
    secondColor = 0x404040;
    if (this->choice != 0) {
        firstColor = 0x404040;
        secondColor = 0x808080;
    }
    textPrinterInit(&printer, FRAME_OT.lastDrawn - 1);
    textPrinterSetShadow(&printer, 1);
    textPrinterSetShadowColor(&printer, 0x700000);
    TEXT_SET_COLOR(printer, firstColor);
    MessageWindow_printLine(&this->window, &printer.printer, STR_YES, 0);
    TEXT_SET_COLOR(printer, secondColor);
    MessageWindow_printLine(&this->window, &printer.printer, STR_NO, 0x10);
    MessageWindow_draw(&this->window, arg);
    printerDestroy(&printer.printer, DESTROY_BASES);
}

/* Turns the reset off: points this at flag and sets the flag to 1; returns this. */
ResetLock *ResetLock_init(ResetLock *this, s32 *flag) {
    this->flag = flag;
    *flag = 1;
    return this;
}

/* ResetLock's destructor: turns the reset back on (its flag back to 0). */
void ResetLock_destroy(ResetLock *this, s32 flags) {
    *this->flag = 0;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs the memory card task for mode: starts the card and installs its callback, checks the
 * ranking's records (the table's constructor resets broken ones), and makes its thread and two windows. */
MemoryCardTask *memoryCardTaskInit(MemoryCardTask *this, s32 mode) {
    RankingTable records;
    YesNoWindow *question;

    taskInit(&this->task, 0, "no name");
    this->task.vtable = &MEMORY_CARD_TASK_VTABLE;
    threadInit(&this->thread, memoryCardThreadEntry, 0x1000, this, 0);
    entityGroupInit(&this->windows);
    this->mode = mode;
    MemCardStart();
    MemCardCallback(memoryCardCallback);
    this->channel = 0;
    RankingTable_init(&records, (RankingRecord *)GAME_STATE.save.records, 8);
    this->message = MessageWindow_init(entityOperatorNew(sizeof(MessageWindow)), 0x20, 0x30, 0x100, 0x50);
    /* the yes/no window: YesNoWindow_init, inlined */
    question = entityOperatorNew(sizeof(YesNoWindow));
    MessageWindow_init(&question->window, 0x78, 0x90, 0x50, 0x20);
    question->window.entity.vtable = &YES_NO_WINDOW_VTABLE;
    question->choice = 0;
    this->question = question;
    entityGroupAppend(&this->windows, &this->message->entity);
    entityGroupAppend(&this->windows, &this->question->window.entity);
    MEMORY_CARD_TASK = this;
    this->done = 0;
    RankingTable_destroy(&records, DESTROY_BASES);
    return this;
}

/* The memory card task's destructor: stops the card, then destroys the windows and the thread. */
void MemoryCardTask_destroy(MemoryCardTask *this, s32 flags) {
    this->task.vtable = &MEMORY_CARD_TASK_VTABLE;
    MemCardCallback(NULL);
    MemCardStop();
    MEMORY_CARD_TASK = NULL;
    entityGroupDestroy(&this->windows, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

/* Whether the task's work is done. */
s32 MemoryCardTask_isDone(MemoryCardTask *this) {
    return this->done;
}

/* The entry of the thread of memoryCardTaskInit's object. */
void memoryCardThreadEntry(s32 unused, void *object) {
    MemoryCardTask_run(object);
}

/* The task's thread: does the work of its mode, then hands back 1 every frame. */
void MemoryCardTask_run(MemoryCardTask *this) {
    switch (this->mode) {
    case MEMORY_CARD_OPTION_LOAD:
        MemoryCardTask_runOptionLoad(this);
        break;
    case MEMORY_CARD_OPTION_SAVE:
        MemoryCardTask_runOptionSave(this);
        break;
    case MEMORY_CARD_LOAD:
        MemoryCardTask_runLoad(this);
        break;
    case MEMORY_CARD_SAVE:
        MemoryCardTask_runSave(this);
        break;
    }
    for (;;) {
        threadSwitchBack(&this->thread, 1);
    }
}

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_formatCard);

/* Opens the message window on text. */
void MemoryCardTask_showMessage(MemoryCardTask *this, u8 *text) {
    MessageWindow_show(this->message, text);
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_LOAD_PROMPT);

/* MEMORY_CARD_OPTION_LOAD: asks "Load data?" (the cursor on no) and on yes loads, waiting for a button at the end;
 * then closes the message window and waits 0x14 frames. */
void MemoryCardTask_runOptionLoad(MemoryCardTask *this) {
    MemoryCardTask_showMessage(this, STR_LOAD_PROMPT);
    if (MemoryCardTask_ask(this, 0)) {
        MemoryCardTask_load(this, 0);
    }
    MessageWindow_close(this->message);
    MemoryCardTask_wait(this, 0x14, 0);
}

/* MEMORY_CARD_OPTION_SAVE: saves (MemoryCardTask_save asks "Save data?" itself), then closes the message
 * window and waits 0x14 frames. */
void MemoryCardTask_runOptionSave(MemoryCardTask *this) {
    MemoryCardTask_save(this);
    MessageWindow_close(this->message);
    MemoryCardTask_wait(this, 0x14, 0);
}

/* MEMORY_CARD_LOAD: loads without asking, closing "Loaded." after 0x2D frames (or a button); then closes the
 * message window and waits 0x14 frames. */
void MemoryCardTask_runLoad(MemoryCardTask *this) {
    MemoryCardTask_load(this, 1);
    MessageWindow_close(this->message);
    MemoryCardTask_wait(this, 0x14, 0);
}

/* MEMORY_CARD_SAVE: the same as MemoryCardTask_runOptionSave (it asks "Save data?" too). */
void MemoryCardTask_runSave(MemoryCardTask *this) {
    MemoryCardTask_save(this);
    MessageWindow_close(this->message);
    MemoryCardTask_wait(this, 0x14, 0);
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_CHECKING_DO_NOT_TOUCH);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_CHECKING);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_load);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_save);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_readSave);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_writeSave);

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_REMOVED);

/* Shows a card result (McErr*) in the message window: McErrNone shows nothing and returns 0; no card or a
 * new card shows STR_CARD_REMOVED (the card was pulled out), any other result shows text; returns -1 then. */
s32 MemoryCardTask_checkResult(MemoryCardTask *this, s32 result, u8 *text) {
    switch (result) {
    case McErrNone:
        return 0;
    case McErrCardNotExist:
    case McErrNewCard:
        MemoryCardTask_showMessage(this, STR_CARD_REMOVED);
        break;
    case McErrCardInvalid:
    case McErrNotFormat:
    case McErrFileNotExist:
    default:
        MemoryCardTask_showMessage(this, text);
        break;
    }
    return -1;
}

/* The memory card callback (a MemCB) that memoryCardTaskInit installs: keeps the command and its
 * result in the task for the thread waiting in MemoryCardTask_waitResult. */
void memoryCardCallback(long cmds, long result) {
    MEMORY_CARD_TASK->cmds = cmds;
    MEMORY_CARD_TASK->result = result;
}

/* The number of files on the card whose name matches name, -1 when the card can't say. */
s32 MemoryCardTask_countFiles(MemoryCardTask *this, char *name) {
    struct DIRENTRY entry;
    long files;

    if (MemCardGetDirentry(this->channel, name, &entry, &files, 0, 1) != 0) {
        return -1;
    }
    return files;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_ANY_FILE);

/* The free blocks of the card (of its 15), -1 when the card can't say. */
s32 MemoryCardTask_getFreeBlocks(MemoryCardTask *this) {
    struct DIRENTRY entries[15];
    long files;
    s32 freeBlocks = 15;
    s32 i;

    if (MemCardGetDirentry(this->channel, STR_ANY_FILE, entries, &files, 0, freeBlocks) != 0) {
        return -1;
    }
    for (i = 0; i < files; i++) {
        freeBlocks -= (entries[i].size + 0x1FFF) / 0x2000;
    }
    return freeBlocks;
}

/* Makes sure the file name exists: opens it, else creates it with blocks blocks and opens it again,
 * each step tried up to tries frames; returns the last card result (0 when it worked). */
s32 MemoryCardTask_openOrCreateFile(MemoryCardTask *this, char *name, s32 blocks, s32 tries) {
    s32 result = 0;
    s32 n;

    n = tries;
    while (n != 0) {
        result = MemCardOpen(this->channel, name, 2);
        if (result == McErrNone) {
            return 0;
        }
        if (result == McErrFileNotExist) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    if (result != McErrFileNotExist) {
        return result;
    }
    n = tries;
    while (n != 0) {
        result = MemCardCreateFile(this->channel, name, blocks);
        if (result == McErrNone) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    if (result != McErrNone) {
        return result;
    }
    n = tries;
    while (n != 0) {
        result = MemCardOpen(this->channel, name, 2);
        if (result == McErrNone) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    return result;
}

/* Runs MemCardExist on the card (once it is free) and waits for its result, up to three times: 1 for a
 * card there (McErrNone or McErrNewCard), 0 when all three are others. */
s32 MemoryCardTask_isCardPresent(MemoryCardTask *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardExist(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = MemoryCardTask_waitResult(this);
        if (result == McErrNone || result == McErrNewCard) {
            return 1;
        }
    } while (--tries != 0);
    return 0;
}

/* Runs MemCardAccept on the card (once it is free) and waits for its result, up to three times while the
 * result is neither McErrNone nor McErrNewCard; returns the last one. */
s32 MemoryCardTask_acceptCard(MemoryCardTask *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardAccept(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = MemoryCardTask_waitResult(this);
        if (result == McErrNone || result == McErrNewCard) {
            break;
        }
    } while (--tries != 0);
    return result;
}

/* Gives up the thread each frame until the card callback ran; returns what it got. */
s32 MemoryCardTask_waitResult(MemoryCardTask *this) {
    this->cmds = 0;
    while (this->cmds == 0) {
        threadSwitchBack(&this->thread, 0);
    }
    return this->result;
}

/* Gives up the thread each frame until a button is pressed. */
void MemoryCardTask_waitButton(MemoryCardTask *this) {
    PadManager *pad = PAD_MANAGER_INSTANCE;

    while (!(padManagerGetPressed(pad, -1) & MEMORY_CARD_ANY_BUTTON)) {
        threadSwitchBack(&this->thread, 0);
    }
}

/* Gives up the thread for frames frames, or until a button is pressed when skippable. */
void MemoryCardTask_wait(MemoryCardTask *this, s32 frames, s32 skippable) {
    /* The match depends on the test against -1 (kept in a saved register) after the decrement. */
    while (--frames != -1) {
        if (skippable && (padManagerGetPressed(PAD_MANAGER_INSTANCE, -1) & MEMORY_CARD_ANY_BUTTON)) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
    }
}

/* Asks the yes/no window's question (opened on yes unless defaultYes is 0) and gives up the thread each
 * frame until the player answers; returns whether the answer is yes. */
s32 MemoryCardTask_ask(MemoryCardTask *this, s32 defaultYes) {
    YesNoWindow *question = this->question;

    YesNoWindow_open(question, defaultYes);
    do {
        threadSwitchBack(&this->thread, 0);
    } while (!YesNoWindow_isDecided(question));
    return YesNoWindow_isYes(question);
}

/* The task's update: runs its thread for a frame and kills the task once the thread is done, then
 * updates the windows. */
void MemoryCardTask_update(MemoryCardTask *this) {
    s32 result = threadSwitchTo(&this->thread, 0);

    this->done = result > 0;
    if (this->done) {
        taskKill(&this->task);
    }
    if (result >= 0) {
        entityGroupTick(&this->windows, (s32)this);
        entityGroupDraw(&this->windows, (s32)this);
    }
}

/* Builds a save file in a new buffer and stores its size at fileSize: the 0x80-byte header (the magic
 * "SC", the icon's frame count, the size in 0x2000-byte blocks, title, the icon's clut), the icon's
 * 16x16 frames from the TIM at icon, then size bytes of data.
 * The match depends on: the header plus the data summed apart (end), both TIM pointers read before the
 * clut copy (an inline unaligned block move), the bytes 3 then 2, and a running offset for the last two
 * copies. */
u8 *buildSaveFile(u32 *fileSize, char *title, u32 *icon, u8 *data, s32 size) {
    TIM_IMAGE tim;
    s32 frames;
    s32 iconSize;
    s32 end;
    u32 total;
    u8 *file;
    u_long *clut;
    u_long *pixels;
    s32 offset;

    OpenTIM((u_long *)icon);
    ReadTIM(&tim);
    frames = tim.prect->h / 16;
    iconSize = frames * 0x80;
    end = size + 0x80;
    total = iconSize + end;
    *fileSize = total;
    file = operatorVecNew(((total + 3) >> 2) << 2);
    bzero(file, 0x80);
    file[0] = 'S';
    file[1] = 'C';
    file[3] = (*fileSize + 0x1FFF) >> 13;
    file[2] = frames + 0x10;
    strncpy(file + 4, title, 0x40);
    clut = tim.caddr;
    pixels = tim.paddr;
    memcpy(file + 0x60, clut, 0x20);
    offset = 0x80;
    memcpy(file + offset, pixels, iconSize);
    offset += iconSize;
    memcpy(file + offset, data, size);
    return file;
}

/* The message window's destructor (MESSAGE_WINDOW_VTABLE, installed by MessageWindow_init). */
void MessageWindow_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The yes/no window's destructor (YES_NO_WINDOW_VTABLE, installed by YesNoWindow_init and its inlined copy in
 * memoryCardTaskInit). */
void YesNoWindow_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs a yes/no window on the first choice. */
YesNoWindow *YesNoWindow_init(YesNoWindow *this, s32 x, s32 y, s32 width, s32 height) {
    MessageWindow_init(&this->window, x, y, width, height);
    this->window.entity.vtable = &YES_NO_WINDOW_VTABLE;
    this->choice = 0;
    return this;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", YES_NO_WINDOW_VTABLE);
INCLUDE_RODATA("title/nonmatchings/memory_card", MESSAGE_WINDOW_VTABLE);
INCLUDE_RODATA("title/nonmatchings/memory_card", MEMORY_CARD_TASK_VTABLE);
INCLUDE_RODATA("title/nonmatchings/memory_card", SAVE_ICON_TIM);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_NOT_INSERTED_2);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_DATA_NOT_FOUND);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_NOT_FORMATTED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_ALREADY_FORMATTED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_FORMATTED);
