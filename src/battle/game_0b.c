/* Helpers with no rodata or data: a side's input history, the boxes fighters hit with and the lists of game_0c's
 * collision shapes. The boundary is unproven: nothing in .rodata or .data marks it. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"

void InputHistory_clear(InputHistory *this);
/* buttons with PAD_LEFT and PAD_RIGHT swapped, when mirrored and only one of them is in it */
u32 InputHistory_mirror(InputHistory *this, u32 buttons);

/* Returns the word at p (no caller: neither called nor in a virtual table). */
s32 func_80074FF4(s32 *p) {
    return *p;
}

/* Constructs an empty, unmirrored input history; returns it. */
void *InputHistory_init(void *this) {
    InputHistory_clear(this);
    InputHistory_setMirrored(this, 0);
    return this;
}

/* With swap (the fighter faces left), InputHistory_mirror swaps PAD_LEFT and PAD_RIGHT in the buttons it reads;
 * without, it leaves them. */
void InputHistory_setMirrored(InputHistory *this, s32 swap) {
    this->mirrorButtons = swap ? PAD_LEFT | PAD_RIGHT : 0;
}

/* Records the buttons held this frame: in history, and which of them were pressed and released since the last
 * frame. The match depends on count read once into a local and incremented in the index (count + 1 written in
 * the store schedules the increment after the address). */
void InputHistory_record(InputHistory *this, u32 buttons) {
    s32 count = this->count;
    u32 changed;

    this->history[count++] = buttons;
    changed = buttons ^ this->held;
    this->count = count & 0xFF;
    this->held = buttons;
    this->pressed = buttons & changed;
    this->released = ~buttons & changed;
}

/* Clears the history and the buttons. The match depends on clearing history a word at a time, walking up to held
 * in a do/while (an indexed loop is strength-reduced into a countdown). */
void InputHistory_clear(InputHistory *this) {
    u32 *word = (u32 *)this->history;

    do {
        *word++ = 0;
    } while (word != (u32 *)&this->held);
    this->held = 0;
    this->pressed = 0;
    this->released = 0;
    this->count = 0;
    this->searchIndex = 0;
    this->searchFrames = 0;
}

/* The last buttons of the history, through InputHistory_mirror. The (u8) index is what reads count as a byte (lbu), as
 * the game does. */
u32 InputHistory_getHeld(InputHistory *this) {
    return InputHistory_mirror(this, this->history[(u8)(this->count - 1)]);
}

/* The buttons pressed this frame, through InputHistory_mirror. */
u32 InputHistory_getPressed(InputHistory *this) {
    return InputHistory_mirror(this, this->pressed);
}

/* The buttons released this frame, through InputHistory_mirror. */
u32 InputHistory_getReleased(InputHistory *this) {
    return InputHistory_mirror(this, this->released);
}

/* The entry of history n - 1 frames before count. */
u32 InputHistory_getAgo(InputHistory *this, s32 n) {
    return this->history[(u8)(this->count - (n - 1))];
}

/* Starts a search of the history (InputHistory_findHeld) at the last entry, over n + 1 frames (1 to 256). */
void InputHistory_startSearch(InputHistory *this, s32 n) {
    s32 limited;
    s32 frames;

    this->searchIndex = (u8)(this->count - 1);
    n += 1;
    if (n < 0x100) {
        limited = n;
    } else {
        limited = 0x100;
    }
    /* The match depends on the second variable: one variable for both clamps drops the game's move. */
    frames = limited;
    if (frames <= 0) {
        frames = 1;
    }
    this->searchFrames = frames;
}

u32 getButtonGroup(u32 buttons);
s32 matchButtons(u32 held, u32 buttons, s32 any);

/* Looks back through the history, from the search's entry, for frames that hold buttons (all of them, or any
 * when any is set); stops at the first frame after them that does not, and then keeps that place for the next
 * search. Returns whether it found them. */
s32 InputHistory_findHeld(InputHistory *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->searchIndex;
    s32 left = this->searchFrames;
    u32 wanted = InputHistory_mirror(this, buttons);
    u32 mask = getButtonGroup(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on found = 1 coming last in the loop: GCC 2.95's loop rotation then puts it before
         * the test, as in the game. */
        if (!matchButtons(held, wanted, any)) {
            if (found) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->searchIndex = (i + 1) & 0xFF;
        this->searchFrames = left + 1;
    }
    return found;
}

/* InputHistory_findHeld that gives up at the first frame, before the buttons, that holds any of the other buttons of
 * the group (an empty frame does not stop it). */
s32 InputHistory_findHeldStrict(InputHistory *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->searchIndex;
    s32 left = this->searchFrames;
    u32 wanted = InputHistory_mirror(this, buttons);
    u32 mask = getButtonGroup(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on masking held again here, and on found = 1 coming last (InputHistory_findHeld). */
        held &= mask;
        if (!matchButtons(held, wanted, any)) {
            if (found || held != 0) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->searchIndex = (i + 1) & 0xFF;
        this->searchFrames = left + 1;
    }
    return found;
}

/* InputHistory_findHeld for a run of at least frames frames holding buttons: a frame that does not hold them
 * restarts the count, or ends the search once the run was found. Keeps the place after the run's end (the next
 * entry, unlike InputHistory_findHeld) for the next search. Returns whether it found the run.
 * The match depends on run declared before found (the game sets run first). */
s32 InputHistory_findHeldFor(InputHistory *this, u32 buttons, s32 frames, s32 any) {
    s32 run = frames;
    s32 found = 0;
    s32 i = this->searchIndex;
    s32 left = this->searchFrames;
    u32 wanted = InputHistory_mirror(this, buttons);
    u32 mask = getButtonGroup(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        if (!matchButtons(held, wanted, any)) {
            if (found) {
                break;
            }
            run = frames;
        } else if (--run <= 0) {
            found = 1;
        }
    }
    if (found) {
        this->searchIndex = i;
        this->searchFrames = left;
    }
    return found;
}

INCLUDE_ASM("game/nonmatchings/game_0b", InputHistory_findPresses);

/* buttons with PAD_LEFT and PAD_RIGHT swapped, when mirrored and only one of them is in it. */
u32 InputHistory_mirror(InputHistory *this, u32 buttons) {
    u32 swap = this->mirrorButtons;
    u32 both = buttons & swap;

    if (both != 0 && both != swap) {
        buttons ^= swap;
    }
    return buttons;
}

/* The group of buttons: 0xF000 (the pad directions) when buttons hold one of them, else 0xFF. */
u32 getButtonGroup(u32 buttons) {
    if ((buttons & 0xF000) != 0) {
        return 0xF000;
    }
    return 0xFF;
}

/* Whether held holds any of buttons (any set) or exactly buttons. */
s32 matchButtons(u32 held, u32 buttons, s32 any) {
    if (any != 0) {
        return (held & buttons) != 0;
    }
    return (held ^ buttons) == 0;
}

/* A box a fighter hits or is hit with (Fighter_updateBoxes keeps them): the least and greatest corner of the eight it
 * was fitted to, and the matrix they were placed with. */
typedef struct Box {
    /* 0x00 */ VECTOR min;
    /* 0x10 */ VECTOR max;
    /* 0x20 */ MATRIX *matrix; /* what matrixMul returned (Fighter_updateBoxes) */
} Box;

/* Returns other when the two boxes overlap (touching counts), else NULL. */
Box *boxOverlaps(Box *this, Box *other) {
    if (this->min.vx > other->max.vx || this->max.vx < other->min.vx || this->min.vy > other->max.vy ||
        this->max.vy < other->min.vy || this->min.vz > other->max.vz || this->max.vz < other->min.vz) {
        return NULL;
    }
    return other;
}

/* The first of the count boxes that overlaps this one (boxOverlaps), or NULL. */
Box *boxFindOverlap(Box *this, Box *boxes, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        if (boxOverlaps(this, boxes)) {
            return boxes;
        }
        boxes++;
    }
    return NULL;
}

/* The first box of others that overlaps one of the count boxes (boxFindOverlap), or NULL. */
Box *boxesFindOverlap(Box *boxes, s32 count, Box *others, s32 othersCount) {
    Box *end = boxes + count;
    Box *found = NULL;

    for (; boxes != end; boxes++) {
        found = boxFindOverlap(boxes, others, othersCount);
        if (found != NULL) {
            return found;
        }
    }
    return found;
}

void boxFitCorners(Box *this, VECTOR *points);

/* Keeps matrix, then fits the box to the 8 corners at points. */
void boxSetCorners(Box *this, VECTOR *points, void *matrix) {
    this->matrix = matrix;
    boxFitCorners(this, points);
}

/* Fits the box to the 8 corners at points: the least and the greatest of each axis. */
void boxFitCorners(Box *this, VECTOR *points) {
    s32 maxX = points[0].vx;
    s32 minX = maxX;
    s32 maxY = points[0].vy;
    s32 minY = maxY;
    s32 maxZ = points[0].vz;
    s32 minZ = maxZ;
    s32 value;
    s32 i;

    for (i = 1; i < 8; i++) {
        value = points[i].vx;
        if (maxX < value) {
            maxX = value;
        } else if (value < minX) {
            minX = value;
        }
        value = points[i].vy;
        if (maxY < value) {
            maxY = value;
        } else if (value < minY) {
            minY = value;
        }
        value = points[i].vz;
        if (maxZ < value) {
            maxZ = value;
        } else if (value < minZ) {
            minZ = value;
        }
    }
    this->min.vx = minX;
    this->min.vy = minY;
    this->min.vz = minZ;
    this->max.vx = maxX;
    this->max.vy = maxY;
    this->max.vz = maxZ;
}

INCLUDE_ASM("game/nonmatchings/game_0b", func_80075848);

INCLUDE_ASM("game/nonmatchings/game_0b", func_80075968);

/* Copies the matrix of box to out; returns out. */
MATRIX *boxGetMatrix(MATRIX *out, Box *box) {
    *out = *box->matrix;
    return out;
}

void collideShapes(CollisionSphere *this, CollisionSphere *other);

INCLUDE_ASM("game/nonmatchings/game_0b", collideShapes);

/* Adds node at the end of the list head; returns head. */
ListNode *listAppend(ListNode *head, ListNode *node) {
    listInsertAfter(head->prev, node);
    return head;
}

void shapeListCollide(ListNode *list, CollisionSphere *other);

/* shapeListCollide of list with every CollisionSphere of others. */
void shapeListsCollide(ListNode *list, ListNode *others) {
    ListNode *node = others->next;

    /* The match depends on the one iterator, stepped before the call (shapeListCollide keeps a next). */
    while (node != others) {
        CollisionSphere *part = (CollisionSphere *)node;

        node = node->next;
        shapeListCollide(list, part);
    }
}

/* collideShapes of every CollisionSphere of list with other. */
void shapeListCollide(ListNode *list, CollisionSphere *other) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        next = node->next;
        collideShapes((CollisionSphere *)node, other);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0b", shapeListFindNearest);

/* Calls the virtual method at 0x18 of every CollisionSphere of list with arg. */
void shapeListDraw(ListNode *list, s32 arg) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        CollisionSphere *part = (CollisionSphere *)node;

        next = node->next;
        part->vtable[3].func((u8 *)part + part->vtable[3].delta, arg);
    }
}
