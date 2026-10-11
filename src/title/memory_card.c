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
#include "engine/gfx/ordering_table.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/system/thread.h"
#include "engine/task/entity.h"
#include "engine/text/font.h"
#include "engine/text/text_printer.h"
#include "title/title_classes.h"


/* A message window (constructor func_800702F8, virtual table D_800656B8): a rectangle that grows open
 * and shrinks closed (func_8007043C) with lines of text centred in it (func_800705A8). The four values
 * of the constructor are the rectangle, as func_800705A8 copies them into the RECT it draws. */
typedef struct Unk800702F8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 scale; /* how far open, 0 (closed) to 0x1000 */
    /* 0x1C */ s32 open;  /* nonzero while opening or open, 0 while closing */
    /* 0x20 */ u8 *text;  /* the lines it shows, NULL for none */
    /* 0x24 */ s32 x;
    /* 0x28 */ s32 y;
    /* 0x2C */ s32 width;
    /* 0x30 */ s32 height;
} Unk800702F8;

/* The yes/no window (constructor func_80071C78, virtual table D_80065690): a message window with two
 * choices under the text, D_80065304 and D_8006530C, picked with up and down (func_80070768). */
typedef struct Unk80071C78 {
    /* 0x00 */ Unk800702F8 window;
    /* 0x34 */ s32 choice;  /* 0 for the first choice, 1 for the second */
    /* 0x38 */ s32 decided; /* set once the player confirmed a choice */
} Unk80071C78;

extern EntityVtable D_800656B8;
extern EntityVtable D_80065690;

/* The memory card task (memoryCardTaskInit, virtual table D_800656E0). Its thread (func_80070B8C) does
 * the work of the mode it was made with and shows what happens in its two windows; the card callback
 * func_80071550 hands it the results the thread waits for (func_80071854). */
typedef struct MemoryCardTask {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 mode;    /* the work the thread does (func_80070B8C) */
    /* 0x24 */ Thread thread;
    /* 0x30 */ s32 channel; /* what the card calls get */
    /* 0x34 */ EntityGroup windows;
    /* 0x44 */ s32 done;    /* set once the thread returned */
    /* 0x48 */ Unk800702F8 *message;
    /* 0x4C */ Unk80071C78 *question;
    /* Set by the memory card callback func_80071550 and polled by the thread, so volatile:
     * func_80071854 stores 0x50 and loads it right back, and func_80071550's last store stays out
     * of the delay slot of its `jr ra`. */
    /* 0x50 */ volatile s32 cmds;   /* the command the callback reported (MemCB's cmds), 0 until it ran */
    /* 0x54 */ volatile s32 result; /* its result (MemCB's rslt) */
} MemoryCardTask;

/* func_8007096C points 0 at a word and sets the word to 1; func_80070C2C
 * makes one on its stack for D_8005FCB0. */
typedef struct Unk8007096C {
    /* 0x0 */ s32 *unk0;
} Unk8007096C;

/* in .rodata: the yes/no window's choices, the task's name, messages and a file name pattern */
extern u8 D_80065304[];
extern u8 D_8006530C[];
extern u8 D_80065314[];
extern u8 D_800653B8[];
extern u8 D_8006564C[];
extern u8 D_80065688[];
extern TaskVtable D_800656E0;
/* the task memoryCardTaskInit builds, while it lives */
extern MemoryCardTask *D_8007B238;

/* The buttons that confirm a choice: start or circle in Japan, start or cross in the US. */
#if VERSION_JP
#define MEMORY_CARD_CONFIRM 0x820
#else
#define MEMORY_CARD_CONFIRM 0x840
#endif
/* start, triangle, circle, cross or square */
#define MEMORY_CARD_ANY_BUTTON 0x8F0

/* A TextPrinter's setColor, inlined as the game has it (its virtual table read from the stack). */
#define TEXT_SET_COLOR(printer, color) \
    (printer).printer.vtable->setColor.func((u8 *)&(printer) + (printer).printer.vtable->setColor.delta, color)

void func_800705A8(Unk800702F8 *this, s32 arg);
void func_80070B6C(s32 unused, void *object);
void func_80070B8C(MemoryCardTask *this);
void func_80070D84(MemoryCardTask *this);
void func_80070DE8(MemoryCardTask *this);
void func_80070E28(MemoryCardTask *this);
void func_80070E6C(MemoryCardTask *this);
void func_80070EAC(MemoryCardTask *this, s32 arg);
void func_80071020(MemoryCardTask *this);
void func_80071550(long cmds, long result);
s32 func_80071854(MemoryCardTask *this);
void func_800718F8(MemoryCardTask *this, s32 frames, s32 skippable);
s32 func_80071980(MemoryCardTask *this, s32 onFirst);

/* Constructs a closed message window over the rectangle x, y, width, height. */
Unk800702F8 *func_800702F8(Unk800702F8 *this, s32 x, s32 y, s32 width, s32 height) {
    entityInit(&this->entity, 0);
    this->x = x;
    this->y = y;
    this->width = width;
    this->entity.vtable = &D_800656B8;
    this->scale = 0;
    this->open = 0;
    this->text = NULL;
    this->height = height;
    return this;
}

/* Prints one line of text centred in the window's width, offsetY below its top (8 pixels a character). */
void func_8007037C(Unk800702F8 *this, Printer *printer, u8 *text, s32 offsetY) {
    s32 length = strlen(text);
    s32 left = (this->width - length * 8) / 2;
    s32 top = this->y + offsetY; /* The match depends on top computed first and stored last. */

    printer->x = this->x + left;
    printer->y = top;
    printerPrint(printer, text);
}

/* Opens the window on text. */
void func_80070404(Unk800702F8 *this, u8 *text) {
    this->open = 1;
    this->text = text;
}

/* Opens the window on the text it has. */
void func_80070414(Unk800702F8 *this) {
    this->open = 1;
}

/* Closes the window. */
void func_80070420(Unk800702F8 *this) {
    this->open = 0;
}

/* Whether the window is all the way open. */
s32 func_80070428(Unk800702F8 *this) {
    return this->scale == 0x1000;
}

/* Grows the window open, or shrinks it closed, by 0x111 a frame. */
void func_8007043C(Unk800702F8 *this) {
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
void func_80070494(RECT *rect, s32 scale) {
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

/* Copies the first line of text into line; returns the text after its line break, NULL after the
 * last line. */
u8 *func_80070554(u8 *line, u8 *text) {
    u8 c = *text++;

    while (c != '\0' && c != '\n') {
        *line++ = c;
        c = *text++;
    }
    *line = '\0';
    return c != '\0' ? text : NULL;
}

INCLUDE_ASM("title/nonmatchings/memory_card", func_800705A8);

/* Opens the yes/no window on the first choice, or on the second when onFirst is 0. */
void func_80070708(Unk80071C78 *this, s32 onFirst) {
    func_80070414(&this->window);
    if (onFirst) {
        this->choice = 0;
    } else {
        this->choice = 1;
    }
    this->decided = 0;
}

/* Whether the player confirmed a choice. */
s32 func_80070750(Unk80071C78 *this) {
    return this->decided;
}

/* Whether the choice is the first one. */
s32 func_8007075C(Unk80071C78 *this) {
    return this->choice == 0;
}

/* The yes/no window's update: once open, up and down move the choice and the confirm button closes it
 * on that choice. */
void func_80070768(Unk80071C78 *this) {
    u32 pressed;

    func_8007043C(&this->window);
    if (!func_80070428(&this->window)) {
        return;
    }
    if (func_80070750(this)) {
        return;
    }
    pressed = padManagerGetPressed(PAD_MANAGER_INSTANCE, -1);
    if (pressed & MEMORY_CARD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, 0x10, 0);
        this->decided = 1;
        func_80070420(&this->window);
        return;
    }
    if (this->choice <= 0) {
        if (pressed & PAD_DOWN) {
            this->choice++;
            soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
        }
    } else {
        if (pressed & PAD_UP) {
            this->choice--;
            soundControlPlaySound(&SOUND_CONTROL, 0xF, 0);
        }
    }
}

/* The yes/no window's draw: once open, its two choices (the one picked bright) under the text. */
void func_8007084C(Unk80071C78 *this, s32 arg) {
    TextPrinter printer;
    u32 firstColor;
    u32 secondColor;

    if (!func_80070428(&this->window)) {
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
    func_8007037C(&this->window, &printer.printer, D_80065304, 0);
    TEXT_SET_COLOR(printer, secondColor);
    func_8007037C(&this->window, &printer.printer, D_8006530C, 0x10);
    func_800705A8(&this->window, arg);
    printerDestroy(&printer.printer, DESTROY_BASES);
}

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

/* Constructs the memory card task for mode: starts the card and installs its callback, checks the
 * ranking's records (the table's constructor resets broken ones), and makes its thread and two windows. */
MemoryCardTask *memoryCardTaskInit(MemoryCardTask *this, s32 mode) {
    Unk800764BC records;
    Unk80071C78 *question;

    taskInit(&this->task, 0, D_80065314);
    this->task.vtable = &D_800656E0;
    threadInit(&this->thread, func_80070B6C, 0x1000, this, 0);
    entityGroupInit(&this->windows);
    this->mode = mode;
    MemCardStart();
    MemCardCallback(func_80071550);
    this->channel = 0;
    func_80075FAC(&records, (Unk800764F4 *)GAME_STATE.save.records, 8);
    this->message = func_800702F8(entityOperatorNew(sizeof(Unk800702F8)), 0x20, 0x30, 0x100, 0x50);
    /* the yes/no window: func_80071C78, inlined */
    question = entityOperatorNew(sizeof(Unk80071C78));
    func_800702F8(&question->window, 0x78, 0x90, 0x50, 0x20);
    question->window.entity.vtable = &D_80065690;
    question->choice = 0;
    this->question = question;
    entityGroupAppend(&this->windows, &this->message->entity);
    entityGroupAppend(&this->windows, &this->question->window.entity);
    D_8007B238 = this;
    this->done = 0;
    func_80076040(&records, DESTROY_BASES);
    return this;
}

/* The memory card task's destructor: stops the card, then destroys the windows and the thread. */
void func_80070AEC(MemoryCardTask *this, s32 flags) {
    this->task.vtable = &D_800656E0;
    MemCardCallback(NULL);
    MemCardStop();
    D_8007B238 = NULL;
    entityGroupDestroy(&this->windows, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
    taskDestroy(&this->task, flags);
}

/* Whether the task's work is done. */
s32 func_80070B60(MemoryCardTask *this) {
    return this->done;
}

/* The entry of the thread of memoryCardTaskInit's object. */
void func_80070B6C(s32 unused, void *object) {
    func_80070B8C(object);
}

/* The task's thread: does the work of its mode, then hands back 1 every frame. */
void func_80070B8C(MemoryCardTask *this) {
    switch (this->mode) {
    case 0:
        func_80070D84(this);
        break;
    case 1:
        func_80070DE8(this);
        break;
    case 2:
        func_80070E28(this);
        break;
    case 3:
        func_80070E6C(this);
        break;
    }
    for (;;) {
        threadSwitchBack(&this->thread, 1);
    }
}

INCLUDE_ASM("title/nonmatchings/memory_card", func_80070C2C);

/* Opens the message window on text. */
void func_80070D60(MemoryCardTask *this, u8 *text) {
    func_80070404(this->message, text);
}

/* Mode 0: shows D_800653B8 and asks; on the first choice runs func_80070EAC with 0. */
void func_80070D84(MemoryCardTask *this) {
    func_80070D60(this, D_800653B8);
    if (func_80071980(this, 0)) {
        func_80070EAC(this, 0);
    }
    func_80070420(this->message);
    func_800718F8(this, 0x14, 0);
}

/* func_80071020, resets the object at 0x48 (func_80070420), then func_800718F8 with 0x14. */
void func_80070DE8(MemoryCardTask *this) {
    func_80071020(this);
    func_80070420(this->message);
    func_800718F8(this, 0x14, 0);
}

/* func_80070EAC with 1, resets the object at 0x48 (func_80070420), then func_800718F8 with 0x14. */
void func_80070E28(MemoryCardTask *this) {
    func_80070EAC(this, 1);
    func_80070420(this->message);
    func_800718F8(this, 0x14, 0);
}

/* The same as func_80070DE8. */
void func_80070E6C(MemoryCardTask *this) {
    func_80071020(this);
    func_80070420(this->message);
    func_800718F8(this, 0x14, 0);
}

INCLUDE_ASM("title/nonmatchings/memory_card", func_80070EAC);

INCLUDE_ASM("title/nonmatchings/memory_card", func_80071020);

INCLUDE_ASM("title/nonmatchings/memory_card", func_80071204);

INCLUDE_ASM("title/nonmatchings/memory_card", func_8007138C);

INCLUDE_ASM("title/nonmatchings/memory_card", func_800714DC);

/* The memory card callback (a MemCB) that memoryCardTaskInit installs: keeps the command and its
 * result in the task for the thread waiting in func_80071854. */
void func_80071550(long cmds, long result) {
    D_8007B238->cmds = cmds;
    D_8007B238->result = result;
}

/* The number of files on the card whose name matches name, -1 when the card can't say. */
s32 func_8007156C(MemoryCardTask *this, char *name) {
    struct DIRENTRY entry;
    long files;

    if (MemCardGetDirentry(this->channel, name, &entry, &files, 0, 1) != 0) {
        return -1;
    }
    return files;
}

/* The free blocks of the card (of its 15), -1 when the card can't say. */
s32 func_800715AC(MemoryCardTask *this) {
    struct DIRENTRY entries[15];
    long files;
    s32 freeBlocks = 15;
    s32 i;

    if (MemCardGetDirentry(this->channel, D_80065688, entries, &files, 0, freeBlocks) != 0) {
        return -1;
    }
    for (i = 0; i < files; i++) {
        freeBlocks -= (entries[i].size + 0x1FFF) / 0x2000;
    }
    return freeBlocks;
}

/* Makes sure the file name exists: opens it, else creates it with blocks blocks and opens it again,
 * each step tried up to tries frames; returns the last card result (0 when it worked). */
s32 func_8007163C(MemoryCardTask *this, char *name, s32 blocks, s32 tries) {
    s32 result = 0;
    s32 n;

    n = tries;
    while (n != 0) {
        result = MemCardOpen(this->channel, name, 2);
        if (result == 0) {
            return 0;
        }
        if (result == 5) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    if (result != 5) {
        return result;
    }
    n = tries;
    while (n != 0) {
        result = MemCardCreateFile(this->channel, name, blocks);
        if (result == 0) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    if (result != 0) {
        return result;
    }
    n = tries;
    while (n != 0) {
        result = MemCardOpen(this->channel, name, 2);
        if (result == 0) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
        n--;
    }
    return result;
}

/* Runs MemCardExist on the card (once it is free) and waits for its result, up to
 * three times: 1 for a result of 0 or 3, 0 when all three are others. */
s32 func_8007175C(MemoryCardTask *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardExist(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = func_80071854(this);
        if (result == 0 || result == 3) {
            return 1;
        }
    } while (--tries != 0);
    return 0;
}

/* Runs MemCardAccept on the card (once it is free) and waits for its result, up to
 * three times while the result is neither 0 nor 3; returns the last one. */
s32 func_800717DC(MemoryCardTask *this) {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardAccept(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = func_80071854(this);
        if (result == 0 || result == 3) {
            break;
        }
    } while (--tries != 0);
    return result;
}

/* Gives up the thread each frame until the card callback ran; returns what it got. */
s32 func_80071854(MemoryCardTask *this) {
    this->cmds = 0;
    while (this->cmds == 0) {
        threadSwitchBack(&this->thread, 0);
    }
    return this->result;
}

/* Gives up the thread each frame until a button is pressed. */
void func_800718A4(MemoryCardTask *this) {
    PadManager *pad = PAD_MANAGER_INSTANCE;

    while (!(padManagerGetPressed(pad, -1) & MEMORY_CARD_ANY_BUTTON)) {
        threadSwitchBack(&this->thread, 0);
    }
}

/* Gives up the thread for frames frames, or until a button is pressed when skippable. */
void func_800718F8(MemoryCardTask *this, s32 frames, s32 skippable) {
    /* The match depends on the test against -1 (kept in a saved register) after the decrement. */
    while (--frames != -1) {
        if (skippable && (padManagerGetPressed(PAD_MANAGER_INSTANCE, -1) & MEMORY_CARD_ANY_BUTTON)) {
            break;
        }
        threadSwitchBack(&this->thread, 0);
    }
}

/* Asks the yes/no window's question (opened on the first choice unless onFirst is 0) and gives up the
 * thread each frame until the player answers; returns whether the answer is the first choice. */
s32 func_80071980(MemoryCardTask *this, s32 onFirst) {
    Unk80071C78 *object = this->question;

    func_80070708(object, onFirst);
    do {
        threadSwitchBack(&this->thread, 0);
    } while (!func_80070750(object));
    return func_8007075C(object);
}

/* The task's update: runs its thread for a frame and kills the task once the thread is done, then
 * updates the windows. */
void func_800719D8(MemoryCardTask *this) {
    s32 result = threadSwitchTo(&this->thread, 0);

    this->done = result > 0;
    if (this->done) {
        taskKill(&this->task);
    }
    if (result >= 0) {
        entityGroupGather(&this->windows, (s32)this);
        entityGroupUpdate(&this->windows, (s32)this);
    }
}

/* Builds a save file in a new buffer and stores its size at fileSize: the 0x80-byte header (the magic
 * "SC", the icon's frame count, the size in 0x2000-byte blocks, title, the icon's clut), the icon's
 * 16x16 frames from the TIM at icon, then size bytes of data.
 * The match depends on: the header plus the data summed apart (end), both TIM pointers read before the
 * clut copy (an inline unaligned block move), the bytes 3 then 2, and a running offset for the last two
 * copies. */
u8 *func_80071A44(u32 *fileSize, char *title, u32 *icon, u8 *data, s32 size) {
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

/* Constructs a yes/no window on the first choice. */
Unk80071C78 *func_80071C78(Unk80071C78 *this, s32 x, s32 y, s32 width, s32 height) {
    func_800702F8(&this->window, x, y, width, height);
    this->window.entity.vtable = &D_80065690;
    this->choice = 0;
    return this;
}
