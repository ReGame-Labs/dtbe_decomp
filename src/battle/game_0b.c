/* Helpers with no rodata or data: a pad input history, a box class and lists of game_0c's spheres. The
 * boundary is unproven: nothing in .rodata or .data marks it. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"

void func_80075090(Unk80075000 *this);
/* buttons with the two of 0x214 swapped when only one of them is in it */
u32 func_80075574(Unk80075000 *this, u32 buttons);

/* Returns the word at p (no caller: neither called nor in a virtual table). */
s32 func_80074FF4(s32 *p) {
    return *p;
}

/* Clears an object and starts it (func_80075034 with 0); returns it. */
void *func_80075000(void *this) {
    func_80075090(this);
    func_80075034(this, 0);
    return this;
}

/* With swap, func_80075574 swaps PAD_LEFT and PAD_RIGHT in the buttons it reads; without, it leaves them. */
void func_80075034(Unk80075000 *this, s32 swap) {
    this->unk214 = swap ? PAD_LEFT | PAD_RIGHT : 0;
}

/* Records the buttons held this frame: in history, and which of them were pressed and released since the last
 * frame. The match depends on count read once into a local and incremented in the index (count + 1 written in
 * the store schedules the increment after the address). */
void func_8007504C(Unk80075000 *this, u32 buttons) {
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
void func_80075090(Unk80075000 *this) {
    u32 *word = (u32 *)this->history;

    do {
        *word++ = 0;
    } while (word != (u32 *)&this->held);
    this->held = 0;
    this->pressed = 0;
    this->released = 0;
    this->count = 0;
    this->unk20C = 0;
    this->unk210 = 0;
}

/* The last buttons of the history, through func_80075574. The (u8) index is what reads count as a byte (lbu), as
 * the game does. */
u32 func_800750C4(Unk80075000 *this) {
    return func_80075574(this, this->history[(u8)(this->count - 1)]);
}

/* The buttons pressed (0x202), through func_80075574. */
u32 func_80075100(Unk80075000 *this) {
    return func_80075574(this, this->pressed);
}

/* The buttons released (0x204), through func_80075574. */
u32 func_80075124(Unk80075000 *this) {
    return func_80075574(this, this->released);
}

/* The entry of history n - 1 frames before count. */
u32 func_80075148(Unk80075000 *this, s32 n) {
    return this->history[(u8)(this->count - (n - 1))];
}

/* Starts a search of the history (func_800751A8) at the last entry, over n + 1 frames (1 to 256). */
void func_8007516C(Unk80075000 *this, s32 n) {
    s32 limited;
    s32 frames;

    this->unk20C = (u8)(this->count - 1);
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
    this->unk210 = frames;
}

u32 func_8007559C(u32 buttons);
s32 func_800755B4(u32 held, u32 buttons, s32 any);

/* Looks back through the history, from the entry at 0x20C, for frames that hold buttons (all of them, or any
 * when any is set); stops at the first frame after them that does not, and then keeps that place at
 * 0x20C/0x210. Returns whether it found them. */
s32 func_800751A8(Unk80075000 *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->unk20C;
    s32 left = this->unk210;
    u32 wanted = func_80075574(this, buttons);
    u32 mask = func_8007559C(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on found = 1 coming last in the loop: GCC 2.95's loop rotation then puts it before
         * the test, as in the game. */
        if (!func_800755B4(held, wanted, any)) {
            if (found) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->unk20C = (i + 1) & 0xFF;
        this->unk210 = left + 1;
    }
    return found;
}

/* func_800751A8 that gives up at the first frame, before the buttons, that holds any of the other buttons of
 * the group (an empty frame does not stop it). */
s32 func_8007527C(Unk80075000 *this, u32 buttons, s32 any) {
    s32 found = 0;
    s32 i = this->unk20C;
    s32 left = this->unk210;
    u32 wanted = func_80075574(this, buttons);
    u32 mask = func_8007559C(wanted);
    u16 held;

    for (;;) {
        held = this->history[i] & mask;
        i = (i - 1) & 0xFF;
        if (--left < 0) {
            break;
        }
        /* The match depends on masking held again here, and on found = 1 coming last (func_800751A8). */
        held &= mask;
        if (!func_800755B4(held, wanted, any)) {
            if (found || held != 0) {
                break;
            }
        } else {
            found = 1;
        }
    }
    if (found) {
        this->unk20C = (i + 1) & 0xFF;
        this->unk210 = left + 1;
    }
    return found;
}

INCLUDE_ASM("game/nonmatchings/game_0b", func_8007536C);

INCLUDE_ASM("game/nonmatchings/game_0b", func_80075454);

/* buttons with PAD_LEFT and PAD_RIGHT swapped (0x214) when only one of them is in it. */
u32 func_80075574(Unk80075000 *this, u32 buttons) {
    u32 swap = this->unk214;
    u32 both = buttons & swap;

    if (both != 0 && both != swap) {
        buttons ^= swap;
    }
    return buttons;
}

/* The group of buttons: 0xF000 (the pad directions) when buttons hold one of them, else 0xFF. */
u32 func_8007559C(u32 buttons) {
    if ((buttons & 0xF000) != 0) {
        return 0xF000;
    }
    return 0xFF;
}

/* Whether held holds any of buttons (any set) or exactly buttons. */
s32 func_800755B4(u32 held, u32 buttons, s32 any) {
    if (any != 0) {
        return (held & buttons) != 0;
    }
    return (held ^ buttons) == 0;
}

/* One of func_8007AAE4's 0x24-byte objects: a box, which func_80075784 fits to points, and 0x20 keeps a pointer. */
typedef struct Unk80075764 {
    /* 0x00 */ VECTOR min;
    /* 0x10 */ VECTOR max;
    /* 0x20 */ MATRIX *unk20; /* func_8007AAE4 keeps here what matrixMul returned */
} Unk80075764;

/* Returns other when the two boxes overlap (touching counts), else NULL. */
Unk80075764 *func_800755D4(Unk80075764 *this, Unk80075764 *other) {
    if (this->min.vx > other->max.vx || this->max.vx < other->min.vx || this->min.vy > other->max.vy ||
        this->max.vy < other->min.vy || this->min.vz > other->max.vz || this->max.vz < other->min.vz) {
        return NULL;
    }
    return other;
}

/* The first of the count boxes that overlaps this one (func_800755D4), or NULL. */
Unk80075764 *func_80075674(Unk80075764 *this, Unk80075764 *boxes, s32 count) {
    s32 i;

    for (i = 0; i < count; i++) {
        if (func_800755D4(this, boxes)) {
            return boxes;
        }
        boxes++;
    }
    return NULL;
}

/* The first box of others that overlaps one of the count boxes (func_80075674), or NULL. */
Unk80075764 *func_800756E4(Unk80075764 *boxes, s32 count, Unk80075764 *others, s32 othersCount) {
    Unk80075764 *end = boxes + count;
    Unk80075764 *found = NULL;

    for (; boxes != end; boxes++) {
        found = func_80075674(boxes, others, othersCount);
        if (found != NULL) {
            return found;
        }
    }
    return found;
}

void func_80075784(Unk80075764 *this, VECTOR *points);

/* Keeps arg2 at 0x20, then func_80075784 with points. */
void func_80075764(Unk80075764 *this, VECTOR *points, void *arg2) {
    this->unk20 = arg2;
    func_80075784(this, points);
}

/* Fits the box to the 8 corners at points: the least and the greatest of each axis. */
void func_80075784(Unk80075764 *this, VECTOR *points) {
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

/* Copies the matrix of box (0x20) to out; returns out. */
MATRIX *func_80075AE0(MATRIX *out, Unk80075764 *box) {
    *out = *box->unk20;
    return out;
}

void func_80075B48(Unk80075EE4 *this, Unk80075EE4 *other);

INCLUDE_ASM("game/nonmatchings/game_0b", func_80075B48);

/* Adds node at the end of the list head; returns head. */
ListNode *func_80075C64(ListNode *head, ListNode *node) {
    listInsertAfter(head->prev, node);
    return head;
}

void func_80075CEC(ListNode *list, Unk80075EE4 *other);

/* func_80075CEC of list with every Unk80075EE4 of others. */
void func_80075C94(ListNode *list, ListNode *others) {
    ListNode *node = others->next;

    /* The match depends on the one iterator, stepped before the call (func_80075CEC keeps a next). */
    while (node != others) {
        Unk80075EE4 *part = (Unk80075EE4 *)node;

        node = node->next;
        func_80075CEC(list, part);
    }
}

/* func_80075B48 of every Unk80075EE4 of list with other. */
void func_80075CEC(ListNode *list, Unk80075EE4 *other) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        next = node->next;
        func_80075B48((Unk80075EE4 *)node, other);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0b", func_80075D44);

/* Calls the virtual method at 0x18 of every Unk80075EE4 of list with arg. */
void func_80075E7C(ListNode *list, s32 arg) {
    ListNode *node;
    ListNode *next;

    for (node = list->next; node != list; node = next) {
        Unk80075EE4 *part = (Unk80075EE4 *)node;

        next = node->next;
        part->vtable[3].func((u8 *)part + part->vtable[3].delta, arg);
    }
}
