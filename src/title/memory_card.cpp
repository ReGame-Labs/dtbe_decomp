/* The memory card screens (memoryCardTaskInit): checking, formatting, loading and saving, with their
 * messages. The file is C++, as g++ built it: locals of sibling blocks share a stack slot (MessageWindow_draw),
 * locals with destructors run them at the end of their block (ResetLock, TextPrinter), and the task's
 * constructor and destructor store its virtual table and call the base's in g++'s order. */

#include "common.h"
#include "kernel.h"
/* kernel.h names its file delete erase with a macro, which would rename C++'s delete */
#undef delete
#include "psyq.h"
#include "libmcrd.h"
#include "memory.h"
#include "strings.h"
#include "engine/gfx/box.h"
#include "engine/debug/system_menu.h"
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
#include "title/memory_card.h"

EXTERN_C_BEGIN
/* in .rodata: the yes/no window's choices, messages and a file name pattern */
extern u8 STR_YES[];
extern u8 STR_NO[];
extern u8 STR_LOAD_PROMPT[];
extern u8 STR_CARD_REMOVED[];
extern char STR_ANY_FILE[];
extern u8 STR_CARD_FORMAT_PROMPT[];
extern u8 STR_CARD_FORMATTING[];
extern u8 STR_CARD_FORMAT_FAILED[];
extern u8 STR_CARD_NOT_INSERTED_2[];
extern u8 STR_CARD_NOT_FORMATTED[];
extern u8 STR_CARD_ALREADY_FORMATTED[];
extern u8 STR_CARD_FORMATTED[];
extern u8 STR_LOADING[];
extern u8 STR_LOAD_FAILED[];
extern u8 STR_LOADED[];
extern u8 STR_SAVE_DATA_CORRUPTED[];
extern u8 STR_SAVING[];
extern u8 STR_SAVE_FAILED[];
extern u8 STR_SAVED[];
/* in .data: the save file's name (STR_SAVE_FILE_NAME) and its title */
extern char *SAVE_FILE_NAME;
extern char *SAVE_FILE_TITLE;
/* in .rodata: the save file's icon, three 16x16 frames */
extern u32 SAVE_ICON_TIM[];
EXTERN_C_END

/* The save file, as buildSaveFile makes it: a 0x80-byte header, the icon's three 0x80-byte frames, then the
 * save data; it fits in one card block. */
#define SAVE_FILE_DATA_OFFSET 0x200
#define SAVE_FILE_BLOCK_SIZE 0x2000

/* The buttons that confirm a choice: start or the menus' confirm button. */
#define MEMORY_CARD_CONFIRM (PADstart | PAD_CONFIRM)
/* The buttons that go on past a message: start or any face button (the same in every version). */
#define MEMORY_CARD_ANY_BUTTON (PADstart | PADRup | PADRright | PADRdown | PADRleft)

/* The card file's name ("BISLPS-03357" in Japan), its title and a yes/no pair that no code names. */
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_FILE_NAME);

/* Constructs a closed message window over the rectangle x, y, width, height. */
MessageWindow *MessageWindow_init(MessageWindow *window, s32 x, s32 y, s32 width, s32 height) {
    entityInit(&window->entity, 0);
    window->x = x;
    window->y = y;
    window->width = width;
    window->entity.vtable = &MESSAGE_WINDOW_VTABLE;
    window->scale = 0;
    window->open = 0;
    window->text = NULL;
    window->height = height;
    return window;
}

/* Prints one line of text centred in the window's width, offsetY below its top (8 pixels a character). */
void MessageWindow_printLine(MessageWindow *window, Printer *printer, u8 *text, s32 offsetY) {
    s32 length = strlen((char *)text); /* the SDK's string functions take char */
    s32 left = (window->width - length * 8) / 2;
    s32 top = window->y + offsetY; /* The match depends on top computed first and stored last. */

    printer->x = window->x + left;
    printer->y = top;
    printerPrint(printer, text);
}

/* Opens the window on text. */
void MessageWindow_show(MessageWindow *window, u8 *text) {
    window->open = 1;
    window->text = text;
}

/* Opens the window on the text it has. */
void MessageWindow_open(MessageWindow *window) {
    window->open = 1;
}

/* Closes the window. */
void MessageWindow_close(MessageWindow *window) {
    window->open = 0;
}

/* Whether the window is all the way open. */
s32 MessageWindow_isOpen(MessageWindow *window) {
    return window->scale == 0x1000;
}

/* Grows the window open, or shrinks it closed, by 0x111 a frame. */
void MessageWindow_tick(MessageWindow *window) {
    if (window->open) {
        if (window->scale != 0x1000) {
            window->scale += 0x111;
            if (window->scale > 0x1000) {
                window->scale = 0x1000;
            }
        }
    } else {
        window->scale -= 0x111;
        if (window->scale < 0) {
            window->scale = 0;
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

/* The message window's draw: once open, its lines centred one under the other (16 pixels apart), on the
 * framed dark rectangle of the window scaled to how far open it is. */
void MessageWindow_draw(MessageWindow *window, s32 arg) {
    u_long *ot;

    if (window->scale <= 0) {
        return;
    }
    ot = FRAME_OT.lastDrawn - 1;
    if (MessageWindow_isOpen(window) && window->text != NULL) {
        TextPrinter printer(ot);
        u8 line[40];
        u8 *text;
        s32 offsetY;

        printer.setShadow(1);
        TEXT_SET_COLOR(printer, 0x909080);
        printer.setShadowColor(0x700000);
        text = window->text;
        offsetY = (window->height - countLines(text) * 16) / 2;
        while (text != NULL) {
            text = copyLine(line, text);
            MessageWindow_printLine(window, &printer, line, offsetY);
            offsetY += 16;
        }
    }
    {
        RECT rect;

        rect.x = window->x;
        rect.y = window->y;
        rect.w = window->width;
        rect.h = window->height;
        scaleRect(&rect, window->scale);
        drawFramedDarkRect(ot, &rect);
    }
}

/* Opens the yes/no window on yes, or on no when defaultYes is 0. */
void YesNoWindow_open(YesNoWindow *question, s32 defaultYes) {
    MessageWindow_open(&question->window);
    if (defaultYes) {
        question->choice = 0;
    } else {
        question->choice = 1;
    }
    question->decided = 0;
}

/* Whether the player confirmed a choice. */
s32 YesNoWindow_isDecided(YesNoWindow *question) {
    return question->decided;
}

/* Whether the choice is yes (the first one). */
s32 YesNoWindow_isYes(YesNoWindow *question) {
    return question->choice == 0;
}

/* The yes/no window's update: once open, up and down move the choice and the confirm button closes it
 * on that choice. */
void YesNoWindow_tick(YesNoWindow *question) {
    u32 pressed;

    MessageWindow_tick(&question->window);
    if (!MessageWindow_isOpen(&question->window)) {
        return;
    }
    if (YesNoWindow_isDecided(question)) {
        return;
    }
    pressed = padManagerGetPressed(PAD_MANAGER_INSTANCE, -1);
    if (pressed & MEMORY_CARD_CONFIRM) {
        soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CONFIRM, 0);
        question->decided = 1;
        MessageWindow_close(&question->window);
        return;
    }
    if (question->choice <= 0) {
        if (pressed & PAD_DOWN) {
            question->choice++;
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
        }
    } else {
        if (pressed & PAD_UP) {
            question->choice--;
            soundControlPlaySound(&SOUND_CONTROL, SOUND_MENU_CURSOR, 0);
        }
    }
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_YES);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_NO);

/* The yes/no window's draw: once open, its two choices (the one picked bright) under the text. */
void YesNoWindow_draw(YesNoWindow *question, s32 arg) {
    u32 firstColor;
    u32 secondColor;

    if (!MessageWindow_isOpen(&question->window)) {
        return;
    }
    firstColor = 0x808080;
    secondColor = 0x404040;
    if (question->choice != 0) {
        firstColor = 0x404040;
        secondColor = 0x808080;
    }
    TextPrinter printer(FRAME_OT.lastDrawn - 1);

    printer.setShadow(1);
    printer.setShadowColor(0x700000);
    TEXT_SET_COLOR(printer, firstColor);
    MessageWindow_printLine(&question->window, &printer, STR_YES, 0);
    TEXT_SET_COLOR(printer, secondColor);
    MessageWindow_printLine(&question->window, &printer, STR_NO, 0x10);
    MessageWindow_draw(&question->window, arg);
}

/* Turns the reset off: keeps flag and sets it to 1. */
ResetLock::ResetLock(s32 *flag) {
    this->flag = flag;
    *flag = 1;
}

/* Turns the reset back on (the flag back to 0). */
ResetLock::~ResetLock() {
    *flag = 0;
}

/* Constructs the memory card task for mode: starts the card and installs its callback, checks the
 * ranking's records (the table's constructor resets broken ones), and makes its thread and two windows. */
MemoryCardTask::MemoryCardTask(s32 mode) : Task(0, "no name") {
    RankingTable records;
    YesNoWindow *question;

    threadInit(&this->thread, memoryCardThreadEntry, 0x1000, this, 0);
    entityGroupInit(&this->windows);
    this->mode = mode;
    MemCardStart();
    MemCardCallback(memoryCardCallback);
    this->channel = 0;
    RankingTable_init(&records, (RankingRecord *)GAME_STATE.save.records, 8);
    /* the windows are entities, made in entityOperatorNew's memory */
    this->message = MessageWindow_init((MessageWindow *)entityOperatorNew(sizeof(MessageWindow)), 0x20, 0x30, 0x100,
                                       0x50);
    /* the yes/no window: YesNoWindow_init, inlined */
    question = (YesNoWindow *)entityOperatorNew(sizeof(YesNoWindow));
    MessageWindow_init(&question->window, 0x78, 0x90, 0x50, 0x20);
    question->window.entity.vtable = &YES_NO_WINDOW_VTABLE;
    question->choice = 0;
    this->question = question;
    entityGroupAppend(&this->windows, &this->message->entity);
    entityGroupAppend(&this->windows, &this->question->window.entity);
    MEMORY_CARD_TASK = this;
    this->done = 0;
    RankingTable_destroy(&records, DESTROY_BASES);
}

/* The memory card task's destructor: stops the card, then destroys the windows and the thread. */
MemoryCardTask::~MemoryCardTask() {
    MemCardCallback(NULL);
    MemCardStop();
    MEMORY_CARD_TASK = NULL;
    entityGroupDestroy(&this->windows, DESTROY_BASES);
    threadDestroy(&this->thread, DESTROY_BASES);
}

/* Whether the task's work is done. */
s32 MemoryCardTask::isDone() {
    return this->done;
}

/* The entry of the thread of memoryCardTaskInit's object. */
void memoryCardThreadEntry(s32 unused, void *object) {
    ((MemoryCardTask *)object)->run();
}

/* The task's thread: does the work of its mode, then hands back 1 every frame. */
void MemoryCardTask::run() {
    switch (this->mode) {
    case MEMORY_CARD_OPTION_LOAD:
        runOptionLoad();
        break;
    case MEMORY_CARD_OPTION_SAVE:
        runOptionSave();
        break;
    case MEMORY_CARD_LOAD:
        runLoad();
        break;
    case MEMORY_CARD_SAVE:
        runSave();
        break;
    }
    for (;;) {
        threadSwitchBack(&this->thread, 1);
    }
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_FORMAT_PROMPT);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_FORMATTING);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_FORMAT_FAILED);

/* Formats the card, with the reset off: when askFirst, asks first (the cursor on no) and formats only a card
 * that MemoryCardTask_acceptCard finds unformatted, showing how it went; otherwise only says the card is not
 * formatted. Then waits for a button. Returns 0 once the card is formatted (or was already), else -1.
 * The match depends on the nested card tests (a switch or `||` tests McErrCardNotExist and McErrCardInvalid as
 * one range) and on each text set before the tests that keep it. */
s32 MemoryCardTask::formatCard(s32 askFirst) {
    ResetLock lock(&SYSTEM_CONTEXT.resetDisabled);
    s32 result = -1;
    u8 *text = STR_CARD_NOT_FORMATTED;
    s32 card;

    if (askFirst) {
        showMessage(STR_CARD_FORMAT_PROMPT);
        if (!ask(0)) {
            return -1;
        }
        card = acceptCard();
        text = STR_CARD_NOT_INSERTED_2;
        if (card != McErrCardNotExist) {
            if (card != McErrCardInvalid) {
                text = STR_CARD_ALREADY_FORMATTED;
                if (card != McErrNotFormat) {
                    result = 0;
                } else {
                    showMessage(STR_CARD_FORMATTING);
                    wait(4, 0);
                    if (checkResult(MemCardFormat(this->channel), STR_CARD_FORMAT_FAILED) != 0) {
                        goto waitButton;
                    }
                    result = 0;
                    text = STR_CARD_FORMATTED;
                }
            }
        }
    }
    showMessage(text);
waitButton:
    waitButton();
    return result;
}

/* Opens the message window on text. */
void MemoryCardTask::showMessage(u8 *text) {
    MessageWindow_show(this->message, text);
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_LOAD_PROMPT);

/* MEMORY_CARD_OPTION_LOAD: asks "Load data?" (the cursor on no) and on yes loads, waiting for a button at the end;
 * then closes the message window and waits 0x14 frames. */
void MemoryCardTask::runOptionLoad() {
    showMessage(STR_LOAD_PROMPT);
    if (ask(0)) {
        load(0);
    }
    MessageWindow_close(this->message);
    wait(0x14, 0);
}

/* MEMORY_CARD_OPTION_SAVE: saves (MemoryCardTask_save asks "Save data?" itself), then closes the message
 * window and waits 0x14 frames. */
void MemoryCardTask::runOptionSave() {
    save();
    MessageWindow_close(this->message);
    wait(0x14, 0);
}

/* MEMORY_CARD_LOAD: loads without asking, closing "Loaded." after 0x2D frames (or a button); then closes the
 * message window and waits 0x14 frames. */
void MemoryCardTask::runLoad() {
    load(1);
    MessageWindow_close(this->message);
    wait(0x14, 0);
}

/* MEMORY_CARD_SAVE: the same as MemoryCardTask_runOptionSave (it asks "Save data?" too). */
void MemoryCardTask::runSave() {
    save();
    MessageWindow_close(this->message);
    wait(0x14, 0);
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_CHECKING_DO_NOT_TOUCH);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_CHECKING);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_load);

INCLUDE_ASM("title/nonmatchings/memory_card", MemoryCardTask_save);

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_LOADING);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_LOAD_FAILED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_LOADED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_DATA_CORRUPTED);

/* Reads the save file (SAVE_FILE_NAME, its first block) and, when its save data is valid, copies it to
 * GAME_STATE.save and applies its options; shows the result in the message window. Returns the card result
 * (0) when it loaded, -1 when it did not. The match depends on the do/while (0) block its failures break out
 * of: the game's code is laid out as a loop (the read at its end, its constant addresses loaded before it),
 * which an if/else chain does not give. */
s32 MemoryCardTask::readSave() {
    ResetLock lock(&SYSTEM_CONTEXT.resetDisabled);
    s32 result;
    u8 *file;

    showMessage(STR_LOADING);
    file = new u8[SAVE_FILE_BLOCK_SIZE];
    do {
        threadSwitchBack(&this->thread, 0);
        MemCardReadFile(this->channel, SAVE_FILE_NAME, (u_long *)file, 0, SAVE_FILE_BLOCK_SIZE);
        result = waitResult();
        if (checkResult(result, STR_LOAD_FAILED) != 0) {
            result = -1;
            break;
        }
        if (saveDataIsValid((SaveData *)(file + SAVE_FILE_DATA_OFFSET))) {
            GAME_STATE.save = *(SaveData *)(file + SAVE_FILE_DATA_OFFSET);
            saveDataApplyOptions(&GAME_STATE.save);
            showMessage(STR_LOADED);
        } else {
            showMessage(STR_SAVE_DATA_CORRUPTED);
            result = -1;
        }
    } while (0);
    delete[] file;
    return result;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVING);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_FAILED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVED);

/* Writes the save file: stores the MD5 of GAME_STATE.save, builds the file around it (buildSaveFile), opens
 * or creates SAVE_FILE_NAME (one block, up to 10 tries each) and writes the file's whole blocks; shows the
 * result in the message window. Returns 0 when it saved, -1 when it did not. The match depends on the
 * do/while (0) block its failures break out of (as in MemoryCardTask_readSave) and on result and save being
 * set before the lock is made (the game sets both before ResetLock_init). */
s32 MemoryCardTask::writeSave() {
    s32 result = 0;
    SaveData *save = &GAME_STATE.save;
    ResetLock lock(&SYSTEM_CONTEXT.resetDisabled);
    u32 size;
    u8 *file;

    showMessage(STR_SAVING);
    saveDataStoreMd5(save);
    file = buildSaveFile(&size, SAVE_FILE_TITLE, SAVE_ICON_TIM, (u8 *)save, sizeof(SaveData));
    do {
        if (checkResult(openOrCreateFile(SAVE_FILE_NAME, 1, 10), STR_SAVE_FAILED) != 0) {
            result = -1;
            break;
        }
        MemCardWriteData((u_long *)file, 0, (size + SAVE_FILE_BLOCK_SIZE - 1) & ~(SAVE_FILE_BLOCK_SIZE - 1));
        if (checkResult(waitResult(), STR_SAVE_FAILED) != 0) {
            result = -1;
            break;
        }
        showMessage(STR_SAVED);
    } while (0);
    MemCardClose();
    delete[] file;
    return result;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_REMOVED);

/* Shows a card result (McErr*) in the message window: McErrNone shows nothing and returns 0; no card or a
 * new card shows STR_CARD_REMOVED (the card was pulled out), any other result shows text; returns -1 then. */
s32 MemoryCardTask::checkResult(s32 result, u8 *text) {
    switch (result) {
    case McErrNone:
        return 0;
    case McErrCardNotExist:
    case McErrNewCard:
        showMessage(STR_CARD_REMOVED);
        break;
    case McErrCardInvalid:
    case McErrNotFormat:
    case McErrFileNotExist:
    default:
        showMessage(text);
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
s32 MemoryCardTask::countFiles(char *name) {
    struct DIRENTRY entry;
    long files;

    if (MemCardGetDirentry(this->channel, name, &entry, &files, 0, 1) != 0) {
        return -1;
    }
    return files;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", STR_ANY_FILE);

/* The free blocks of the card (of its 15), -1 when the card can't say. */
s32 MemoryCardTask::getFreeBlocks() {
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
s32 MemoryCardTask::openOrCreateFile(char *name, s32 blocks, s32 tries) {
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
s32 MemoryCardTask::isCardPresent() {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardExist(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = waitResult();
        if (result == McErrNone || result == McErrNewCard) {
            return 1;
        }
    } while (--tries != 0);
    return 0;
}

/* Runs MemCardAccept on the card (once it is free) and waits for its result, up to three times while the
 * result is neither McErrNone nor McErrNewCard; returns the last one. */
s32 MemoryCardTask::acceptCard() {
    s32 tries = 3;
    s32 result;

    do {
        while (!MemCardAccept(this->channel)) {
            threadSwitchBack(&this->thread, 0);
        }
        result = waitResult();
        if (result == McErrNone || result == McErrNewCard) {
            break;
        }
    } while (--tries != 0);
    return result;
}

/* Gives up the thread each frame until the card callback ran; returns what it got. */
s32 MemoryCardTask::waitResult() {
    this->cmds = 0;
    while (this->cmds == 0) {
        threadSwitchBack(&this->thread, 0);
    }
    return this->result;
}

/* Gives up the thread each frame until a button is pressed. */
void MemoryCardTask::waitButton() {
    PadManager *pad = PAD_MANAGER_INSTANCE;

    while (!(padManagerGetPressed(pad, -1) & MEMORY_CARD_ANY_BUTTON)) {
        threadSwitchBack(&this->thread, 0);
    }
}

/* Gives up the thread for frames frames, or until a button is pressed when skippable. */
void MemoryCardTask::wait(s32 frames, s32 skippable) {
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
s32 MemoryCardTask::ask(s32 defaultYes) {
    YesNoWindow *question = this->question;

    YesNoWindow_open(question, defaultYes);
    do {
        threadSwitchBack(&this->thread, 0);
    } while (!YesNoWindow_isDecided(question));
    return YesNoWindow_isYes(question);
}

/* The task's update: runs its thread for a frame and kills the task once the thread is done, then
 * updates the windows. */
void MemoryCardTask::update(s32 arg) {
    s32 result = threadSwitchTo(&this->thread, 0);

    this->done = result > 0;
    if (this->done) {
        taskKill(this);
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
    file = new u8[((total + 3) >> 2) << 2];
    bzero(file, 0x80);
    file[0] = 'S';
    file[1] = 'C';
    file[3] = (*fileSize + 0x1FFF) >> 13;
    file[2] = frames + 0x10;
    strncpy((char *)file + 4, title, 0x40);
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
void MessageWindow_destroy(Entity *window, s32 flags) {
    entityDestroy(window, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(window);
    }
}

/* The yes/no window's destructor (YES_NO_WINDOW_VTABLE, installed by YesNoWindow_init and its inlined copy in
 * memoryCardTaskInit). */
void YesNoWindow_destroy(Entity *window, s32 flags) {
    entityDestroy(window, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(window);
    }
}

/* Constructs a yes/no window on the first choice. */
YesNoWindow *YesNoWindow_init(YesNoWindow *question, s32 x, s32 y, s32 width, s32 height) {
    MessageWindow_init(&question->window, x, y, width, height);
    question->window.entity.vtable = &YES_NO_WINDOW_VTABLE;
    question->choice = 0;
    return question;
}

INCLUDE_RODATA("title/nonmatchings/memory_card", YES_NO_WINDOW_VTABLE);
INCLUDE_RODATA("title/nonmatchings/memory_card", MESSAGE_WINDOW_VTABLE);
INCLUDE_RODATA("title/nonmatchings/memory_card", _vt.14MemoryCardTask);
INCLUDE_RODATA("title/nonmatchings/memory_card", SAVE_ICON_TIM);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_NOT_INSERTED_2);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_SAVE_DATA_NOT_FOUND);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_NOT_FORMATTED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_ALREADY_FORMATTED);
INCLUDE_RODATA("title/nonmatchings/memory_card", STR_CARD_FORMATTED);
