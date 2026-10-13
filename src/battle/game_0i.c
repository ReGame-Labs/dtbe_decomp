/* An entity on a list the stage supplies, and four classes on one base (virtual tables D_8006D980 to
 * D_8006DBA0). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/system/memory.h"

struct Unk800C82CC *func_800C82CC(struct Unk800C82CC *this, Unk8006ED20 *owner, void *list, s32 arg3);
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3);
Entity *func_800C8778(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6);
struct Unk800CA430 *func_800CA430(struct Unk800CA430 *this, Unk80076F34 *chara, s32 arg);
struct Unk800CA768 *func_800CA768(struct Unk800CA768 *this, Unk80076F34 *chara, s32 arg);
struct Unk800CAACC *func_800CAACC(struct Unk800CAACC *this, Unk80076F34 *chara, s32 arg2, s32 arg3, s32 arg4,
                                  s32 arg5);
extern EntityVtable D_8006D9F0;
extern EntityVtable D_8006DA18;
extern EntityVtable D_8006DA40;
extern EntityVtable D_8006DA68;
extern EntityVtable D_8006DAC0;
extern EntityVtable D_8006DAF0;
extern EntityVtable D_8006DA90;
extern EntityVtable D_8006DB20;
extern VtableEntry D_8006DB80[];
extern EntityVtable D_8006DBA0;

/* The object at 4 of Unk800C8778: its virtual table, at 0x14 as an Entity's,
 * has a method at 0x20 that func_800C8DFC calls. */
typedef struct Unk800C8DFCVtable {
    /* 0x00 */ VtableEntry unk0[4];
    /* 0x20 */ VtableEntry unk20;
} Unk800C8DFCVtable;

typedef struct Unk800C8DFC {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Unk800C8DFCVtable *vtable;
} Unk800C8DFC;

/* The entity class func_800C8778 constructs (virtual table D_8006DB50; the
 * classes of D_8006DAC0, D_8006DAF0 and D_8006DB20 derive from it). */
/* The class func_800C8778 constructs (virtual table D_8006DB80), derived from Unk80076308. */
typedef struct Unk800C8778 {
    /* 0x0 */ Unk80076308 base;
    /* 0x4 */ Unk800C8DFC *unk4;
} Unk800C8778;

typedef struct Unk800C90FC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80075EE4 part;
    /* 0x60 */ Unk800C8778 unk60;
    /* 0x68 */ s32 unk68; /* func_800C8778's fifth argument */
    /* 0x6C */ Vec3 unk6C; /* func_800C8778's position */
    /* 0x78 */ u8 unk78[0x7C - 0x78];
    /* 0x7C */ s32 unk7C;
    /* 0x80 */ s32 unk80;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 unk8C; /* its slot in func_800C82CC's table (func_800C86E4, D_800D3798) */
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ void *unkA4; /* func_800C8778's first argument, the table's owner (func_800C8658) */
    /* 0xA8 */ u8 unkA8[0xAC - 0xA8];
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ u8 unkBC[0xC4 - 0xBC];
} Unk800C90FC;

void func_800C9140(Unk800C90FC *this, s32 arg);
void func_800C9288(Unk800C90FC *this, s32 arg);

/* the destructor of Unk800C90FC, the entity class others derive from */
void func_800C8920(Unk800C90FC *this, s32 flags);
void func_800C8DFC(Unk800C8DFC *object, s32 arg);
void func_800C89B4(void *this);

/* A slot of the table func_800C82CC's entities share. */
typedef struct Unk800C82CCSlot {
    /* 0x0 */ u8 flags; /* cleared by func_800C82CC and func_800C86E4, bits cleared by func_800C90C0 */
    /* 0x1 */ u8 unk1;
    /* 0x2 */ s16 x;    /* the position of the slot's object (func_800C9034) */
    /* 0x4 */ s16 y;
    /* 0x6 */ s16 z;
} Unk800C82CCSlot;

/* The table func_800C82CC's entities share: a count and 16 slots. */
typedef struct Unk800C82CCTable {
    /* 0x00 */ s32 count;
    /* 0x04 */ Unk800C82CCSlot slots[16];
} Unk800C82CCTable;

extern Unk800C82CCTable D_800D3798;

/* Returns the table func_800C82CC's entities share. */
Unk800C82CCTable *func_800C77BC(void) {
    return &D_800D3798;
}

/* Creates func_800C82CC's entity (0x3C bytes) with the two arguments and adds it to the group at
 * 0x3C (the stages call it with a table of their .data). */
void func_800C77C8(Unk8006ED20 *this, void *arg1, s32 arg2) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        func_800C82CC(entityOperatorNewAt(0x3C, child), this, arg1, arg2);
    }
    func_8006F084(this, child);
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D788);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D798);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7A4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7A8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7AC);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7C0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7C8);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C7844);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C7924);

/* Creates an entity of func_800CA430's class (0x24 bytes) with g++'s new, and hands it to
 * func_8007AD4C. */
void func_800C7B30(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x24);

    if (child != NULL) {
        func_800CA430(entityOperatorNewAt(0x24, child), this, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates an entity of func_800CA768's class (0x30 bytes) with g++'s new, and hands it to
 * func_8007AD4C. */
void func_800C7B9C(Unk80076F34 *this, s32 arg) {
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        func_800CA768(entityOperatorNewAt(0x30, child), this, arg);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800CAACC's entity (0x48 bytes) for the character with the four arguments, and hands
 * it to func_8007AD4C. */
void func_800C7C08(Unk80076F34 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_800CAACC(entityOperatorNewAt(0x48, child), this, arg1, arg2, arg3, arg4);
    }
    func_8007AD4C(this, child);
}

/* Creates func_800CAEF0's entity (0x54 bytes) at the character's func_8007A650 point with 0x100 and
 * the colour (0x2020FF, 0xFF2020), and hands it to func_8007AD4C. arg1 is not read. */
void func_800C7C9C(Unk80076F34 *this, s32 arg1, s32 color) {
    Vec3 position;
    Entity *child;

    func_8007A650(this, &position);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800CAEF0(entityOperatorNewAt(0x54, child), &position, 0x100, color);
    }
    func_8007AD4C(this, child);
}

/* Throws count func_800C3654 entities (entry 9 of D_8006D5C4) from the character's position (func_800775D8),
 * each 256 either side of it in x, at a random velocity (x -8 to 7, y -50 to -19) from the character's
 * generator (func_80077580), with color. */
void func_800C7D14(Unk80076F34 *this, s32 arg1, u32 count, u32 color) {
    Unk8006ED20 *random;
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    random = func_80077580(this);
    while (count--) {
        func_800775D8(this, &position);
        position.vx += (func_8006F004(random) & 0x1FF) - 0x100;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = func_8006F004(random) & 0xF;
        vy = func_8006F004(random) & 0x1F;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x32;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            func_800C3654(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color, 9);
        }
        func_8007AD4C(this, child);
    }
}

/* Throws count func_800C391C entities (entry 11 of D_8006D5C4) from the character's point (func_8007A650), each
 * 16 either side of it in x, at a random velocity (x -8 to 7, y -20 to -13), with color. */
void func_800C7E1C(Unk80076F34 *this, s32 arg1, u32 count, u32 color) {
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    while (count--) {
        func_8007A650(this, &position);
        position.vx += (mersenneTwisterGenerate(&RANDOM) & 0x1F) - 0x10;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = mersenneTwisterGenerate(&RANDOM) & 0xF;
        vy = mersenneTwisterGenerate(&RANDOM) & 7;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x14;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            func_800C391C(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color);
        }
        func_8007AD4C(this, child);
    }
}

/* The direction (ratan2, in 0 to 0xFFF) from from to to, on x and y. The match depends on to's x and y read first. */
s32 func_800C7F14(Vec3 *from, Vec3 *to) {
    s32 x = to->vx;
    s32 y = to->vy;

    return ratan2(y - from->vy, x - from->vx) & 0xFFF;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C7F48);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8008);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C811C);

/* An entry of func_800C82CC's list: the list ends at an entry whose first halfword is -1. */
typedef struct Unk800C82CCEntry {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2[4];
} Unk800C82CCEntry;

/* The entity func_800C82CC constructs for the game (virtual table D_8006DBA0): a list of entries counted at 0x28
 * and the shared table D_800D3798. */
typedef struct Unk800C82CC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *owner;
    /* 0x1C */ Unk800C82CCEntry *list;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ Unk800C82CCTable *table;
    /* 0x28 */ s32 count; /* the entries of the list (func_800C84C8) */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* 90 from func_800C82CC and func_800C86E4 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800C82CC; /* size 0x3C */

void func_800C84C8(Unk800C82CC *this);

/* Constructs an Unk800C82CC for owner with list: empties the shared table, then counts the list. The match depends
 * on the table read into a local after the count is cleared and on the slots cleared from the last. */
Unk800C82CC *func_800C82CC(Unk800C82CC *this, Unk8006ED20 *owner, void *list, s32 arg3) {
    Unk800C82CCTable *table;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DBA0;
    this->table = &D_800D3798;
    this->owner = owner;
    this->list = list;
    this->unk20 = arg3;
    this->unk2C = 0;
    this->unk30 = 90;
    this->unk34 = 300;
    this->unk38 = 0;
    D_800D3798.count = 0;
    table = this->table;
    for (i = 15; i >= 0; i--) {
        table->slots[i].flags = 0;
    }
    func_800C84C8(this);
    return this;
}

/* The destructor of the entity class of vtable D_8006DBA0, which func_800C82CC installs. */
void func_800C8380(Entity *this, s32 flags) {
    this->vtable = &D_8006DBA0;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C83D4);

/* Counts the entries of the list, up to the one whose first halfword is -1. */
void func_800C84C8(Unk800C82CC *this) {
    Unk800C82CCEntry *list = this->list;

    this->count = 0;
    if (list != NULL) {
        while (list[this->count].unk0 != -1) {
            this->count++;
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8520);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C85C4);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8658);

/* Frees object's slot of the shared table: one fewer in it, the slot's flags cleared, and 0x30 back to 90. */
void func_800C86E4(Unk800C82CC *this, Unk800C90FC *object) {
    this->unk30 = 90;
    this->table->count--;
    this->table->slots[object->unk8C].flags = 0;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8718);

/* The first method of Unk800C8778: func_800C8DFC on the object at 4, with arg. */
void func_800C8754(Unk800C8778 *this, s32 arg) {
    func_800C8DFC(this->unk4, arg);
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8778);

extern EntityVtable D_8006DB50;

/* The destructor of the entity class of vtable D_8006DB50, which func_800C8778 installs. */
void func_800C8920(Unk800C90FC *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &D_8006DB50;
    /* the members' destructors, inlined in reverse order: the one at 0x60 first */
    this->unk60.base.vtable = D_80065164;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C89B4);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8B0C);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8C40);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8CD0);

/* Calls the object's virtual method at 0x20 with arg. */
void func_800C8DFC(Unk800C8DFC *object, s32 arg) {
    object->vtable->unk20.func((u8 *)object + object->vtable->unk20.delta, arg);
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8E30);

/* func_8002BD68 of the entity at 0x14 of arg, or 0 when there is none. */
s32 func_800C8F04(void *this, Unk80075EE4 *arg) {
    s32 result = 0;

    if (arg->unk14) {
        result = func_8002BD68(arg->unk14);
    }
    return result;
}

/* Stops the part's motion: 0x7C..0x88 cleared, then 0x7C set to +-0x133 by the sign of its x speed, which is cleared
 * with the rest. */
void func_800C8F34(Unk800C90FC *this) {
    s32 speed = this->part.unk20.vx;

    this->unk7C = 0;
    this->unk80 = 0;
    this->unk84 = 0;
    this->unk88 = 0;
    if (speed > 0) {
        this->unk7C = 0x133;
    }
    if (this->part.unk20.vx < 0) {
        this->unk7C = -0x133;
    }
    this->part.unk20.vx = 0;
    this->part.unk20.vy = 0;
    this->part.unk20.vz = 0;
    this->part.unk2C = 0;
}

/* Whether func_8007AED0 is false for the target (a chara). */
s32 func_800C8F7C(Unk800C90FC *this, Entity *target) {
    return func_8007AED0((Unk80076F34 *)target) ^ 1;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8F9C);

/* Copies the position at 0x6C into the object's slot of D_800D3798, when it has an owner. */
void func_800C9034(Unk800C90FC *this) {
    if (this->unkA4) {
        Unk800C82CCSlot *slot = &D_800D3798.slots[this->unk8C];

        slot->x = this->unk6C.vx;
        slot->y = this->unk6C.vy;
        slot->z = this->unk6C.vz;
    }
}

/* Sets bits in the flags of the object's slot of D_800D3798, when it has an owner. */
void func_800C9080(Unk800C90FC *this, u8 bits) {
    if (this->unkA4) {
        D_800D3798.slots[this->unk8C].flags |= bits;
    }
}

/* Clears bits in the flags of the object's slot of D_800D3798, when it has an owner. */
void func_800C90C0(Unk800C90FC *this, u8 bits) {
    if (this->unkA4) {
        D_800D3798.slots[this->unk8C].flags &= ~bits;
    }
}

/* The update of the four classes derived from Unk800C90FC: func_800C9288 while 0x68 is under 11, func_800C9140 after. */
void func_800C90FC(Unk800C90FC *this, s32 arg) {
    if (this->unk68 < 11) {
        func_800C9288(this, arg);
    } else {
        func_800C9140(this, arg);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D874);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9140);

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D894);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D8A4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D8C4);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9288);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C94C4);

/* Runs the character's timed effect at 0x464: counts it down and clears it at 0; otherwise, unless
 * func_8006F190 says so for its 0x228 object, emits the effect of its kind (1 to 6) every 16, 4 or 2 frames. */
void func_800C9790(Unk80076F34 *this) {
    s32 timer = this->unk464.unk4;

    if (timer != 0) {
        timer--;
        this->unk464.unk4 = timer;
        if (timer == 0) {
            func_800774A8(this);
        } else if (func_8006F190(func_80077580(this)) == 0) {
            switch (this->unk464.unk0) {
            case 1:
                if ((this->unk464.unk4 & 0xF) == 0) {
                    func_800C7C08(this, 0, 0x80, 0x80, 0x80);
                }
                break;
            case 2:
                if ((this->unk464.unk4 & 0xF) == 0) {
                    func_800C7C08(this, 1, 0x20, 0x20, 0x80);
                }
                break;
            case 3:
                if ((this->unk464.unk4 & 0x3) == 0) {
                    func_800C7E1C(this, 0, 1, 0x808080);
                }
                break;
            case 4:
                if ((this->unk464.unk4 & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0x2020FF);
                }
                break;
            case 5:
                if ((this->unk464.unk4 & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0xFF2020);
                }
                break;
            case 6:
                if ((this->unk464.unk4 & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0x20FF20);
                }
                break;
            }
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9938);

void func_800C811C(Unk80076F34 *this);

/* Runs the chara's timed thing at 0x478: counts it down and clears it at 0, or as soon as func_80079E2C is not
 * 13, 14, 16 or 17; else on even frames, while its kind is 1 to 3, calls func_800C811C. */
void func_800C99B0(Unk80076F34 *this) {
    s32 timer = this->unk478.unk4;

    if (timer != 0) {
        timer--;
        this->unk478.unk4 = timer;
        if (timer == 0) {
            func_80077508(this);
            return;
        }
        if (func_8006F190(func_80077580(this))) {
            return;
        }
        switch (func_80079E2C(this)) {
        default:
            func_80077508(this);
            break;
        case 13:
        case 14:
        case 16:
        case 17:
            /* The match depends on the nested tests: one && condition is folded into an unsigned range test. */
            if (this->unk478.unk0 < 4) {
                if (this->unk478.unk0 > 0 && !(this->unk478.unk4 & 1)) {
                    func_800C811C(this);
                }
            }
            break;
        }
    }
}

/* Constructs an entity of D_8006DB20: func_800C8778 with the same arguments. */
Entity *func_800C9A88(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DB20;
    return this;
}

/* The first method of the class of virtual table D_8006DB20: func_800C89B4. */
void func_800C9AD4(void *this) {
    func_800C89B4(this);
}

/* The entity class of virtual table D_8006DAF0 (func_800C9AF4), derived from func_800C8778's. */
typedef struct Unk800C9AF4 {
    /* 0x00 */ Unk800C90FC base;
    /* 0xC4 */ s32 unkC4;
} Unk800C9AF4;

/* Constructs an entity of D_8006DAF0: func_800C8778 with the same arguments, 0xB8 moved by 0x800, then
 * func_800C9080 with bit 1. */
Unk800C9AF4 *func_800C9AF4(Unk800C9AF4 *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(&this->base.entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->base.entity.vtable = &D_8006DAF0;
    this->unkC4 = 0;
    this->base.unkB8 += 0x800;
    func_800C9080(&this->base, 2);
    return this;
}

/* The first method of the class of virtual table D_8006DAF0: turns 0xB8 by 0x200 (of 0x1000) while 0xC4 is set and
 * it is not 0, then func_800C89B4. */
void func_800C9B5C(Unk800C9AF4 *this) {
    if (this->unkC4 && this->base.unkB8) {
        this->base.unkB8 = (this->base.unkB8 + 0x200) & 0xFFF;
    }
    func_800C89B4(this);
}

void func_800C8E30(Unk800C90FC *this, Unk80075EE4 *arg);
void func_800C9C58(Unk800C9AF4 *this);

/* The second method of the class of virtual table D_8006DAF0: before 0xC4 or 0xB4 is set, func_800C9C58 unless
 * func_800C8F04 gives 0x20000, or 0x10000 with func_800C8F7C false; after, func_800C8E30 once 0xB8 is 0. */
void func_800C9BA4(Unk800C9AF4 *this, Unk80075EE4 *arg) {
    if (this->unkC4 == 0 && this->base.unkB4 == 0) {
        /* The match depends on a switch over the entity kinds with three cases: the game's compare tree has
         * 0x20000 at its root (tests ==, then > to the default arm, then 0x10000), which gcc builds only from a
         * third case above 0x20000 that shares the default arm. Its value is not in the code: 0x30000 (an
         * entity kind of entityInit) is a guess. func_800C8E30 has the same tree. */
        switch (func_800C8F04(this, arg)) {
        case 0x20000:
            break;
        case 0x10000:
            if (!func_800C8F7C(&this->base, func_80078878(arg))) {
                break;
            }
            /* fallthrough */
        case 0x30000:
        default:
            func_800C9C58(this);
            break;
        }
    } else if (this->base.unkB8 == 0) {
        func_800C8E30(&this->base, arg);
    }
}

/* Sets 0xC4 and the part's flag bit 0, 0xB0 to 30, adds 150 to 0xAC, then clears bit 1 of its slot (func_800C90C0). */
void func_800C9C58(Unk800C9AF4 *this) {
    this->unkC4 = 1;
    this->base.unkB0 = 30;
    this->base.part.flags |= 1;
    this->base.unkAC += 150;
    func_800C90C0(&this->base, 2);
}

/* Constructs an entity of D_8006DAC0: func_800C8778 with the same arguments. */
Entity *func_800C9CA0(Entity *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    func_800C8778(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DAC0;
    return this;
}

/* The first method of the class of virtual table D_8006DAC0: func_800C89B4. */
void func_800C9CEC(void *this) {
    func_800C89B4(this);
}

/* The entity class of virtual table D_8006DA90 (func_800C9D0C), derived from func_800C8778's: four 0x24-byte
 * records from 0xC8. */
typedef struct Unk800C9D0CItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 unk2[0x24 - 0x2];
} Unk800C9D0CItem;

typedef struct Unk800C9D0C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0xC4 - 0x18];
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ Unk800C9D0CItem unkC8[4];
} Unk800C9D0C;

/* Constructs an entity of D_8006DA90: func_800C8778 with the same arguments, then clears the first halfword of
 * its four records, the last first. */
Unk800C9D0C *func_800C9D0C(Unk800C9D0C *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, Vec3 *arg5, s32 arg6) {
    s32 i;

    func_800C8778(&this->entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->entity.vtable = &D_8006DA90;
    this->unkC4 = 0;
    /* The match depends on the loop running from the last record (the game clears 0x134 first). */
    for (i = 3; i >= 0; i--) {
        this->unkC8[i].unk0 = 0;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9D74);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA028);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA1B4);

/* The entity class of virtual table D_8006DA68 (func_800CA430): a point at (0x30 or 0x110, 0x130) by the
 * chara's side that func_800CA4FC moves up the screen. */
typedef struct Unk800CA430 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 x;
    /* 0x1E */ s16 y;
    /* 0x20 */ s32 frame;
} Unk800CA430;

/* Constructs an entity of D_8006DA68 at x 0x110 when func_80077574 is set for chara, else 0x30, and y 0x130. */
Unk800CA430 *func_800CA430(Unk800CA430 *this, Unk80076F34 *chara, s32 arg) {
    s16 x;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DA68;
    this->unk18 = arg;
    if (func_80077574(chara) == 0) {
        x = 0x30;
    } else {
        x = 0x110;
    }
    this->x = x;
    this->y = 0x130;
    this->frame = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006DA68, which func_800CA430 installs. */
void func_800CA4A8(Entity *this, s32 flags) {
    this->vtable = &D_8006DA68;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of Unk800CA430: moves it up by 0x30 while it is below 0x79, and every frame after the 12th; deletes
 * it once it is above -0x30. */
void func_800CA4FC(Unk800CA430 *this) {
    if (this->y > 0x78) {
        this->y -= 0x30;
    }
    if (this->frame > 12) {
        this->y -= 0x30;
    }
    if (this->y < -0x30) {
        entityDelete(&this->entity);
    } else {
        this->frame++;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA5AC);

/* The entity class of virtual table D_8006DA40 (func_800CA768): a point at (0x2C or 0x114, 0xAC) by the side of
 * the chara it keeps the handle of, with the kind of the chara's timed thing at 0x464. */
typedef struct Unk800CA768 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 charaHandle;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s16 x;
    /* 0x22 */ s16 y;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28; /* the chara's unk464.unk0 */
    /* 0x2C */ s32 unk2C;
} Unk800CA768;

/* Constructs an entity of D_8006DA40 for chara at x 0x114 when func_80077574 is set for it, else 0x2C, and y 0xAC. */
Unk800CA768 *func_800CA768(Unk800CA768 *this, Unk80076F34 *chara, s32 arg) {
    s16 x;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DA40;
    this->charaHandle = entityGetHandle(&chara->entity);
    this->unk1C = arg;
    if (func_80077574(chara) == 0) {
        x = 0x2C;
    } else {
        x = 0x114;
    }
    this->x = x;
    this->y = 0xAC;
    this->unk24 = 0;
    this->unk28 = chara->unk464.unk0;
    this->unk2C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_8006DA40, which func_800CA768 installs. */
void func_800CA7F8(Entity *this, s32 flags) {
    this->vtable = &D_8006DA40;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA84C);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA920);

/* The entity class of virtual table D_8006DA18 (func_800CAACC): follows the point (func_8007A650) of the chara it
 * keeps the handle of for 8 frames, while 0x3C and 0x40 move in opposite directions (which by unk1C). */
typedef struct Unk800CAACC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 charaHandle;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 frame;
} Unk800CAACC;

/* Constructs an entity of D_8006DA18 for chara at its point, 0x3C and 0x40 starting at 0 and
 * 0x80 when arg2 is 0, else at 0x140 and 0. */
Unk800CAACC *func_800CAACC(Unk800CAACC *this, Unk80076F34 *chara, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DA18;
    this->charaHandle = entityGetHandle(&chara->entity);
    this->unk1C = arg2;
    this->unk20 = arg3;
    this->unk24 = arg4;
    this->unk28 = arg5;
    func_8007A650(chara, &this->position);
    this->frame = 0;
    if (this->unk1C == 0) {
        this->unk3C = 0;
        this->unk40 = 0x80;
    } else {
        this->unk3C = 0x140;
        this->unk40 = 0;
    }
    return this;
}

/* The destructor of the entity class of vtable D_8006DA18, which func_800CAACC installs. */
void func_800CAB94(Entity *this, s32 flags) {
    this->vtable = &D_8006DA18;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of Unk800CAACC: deletes it once its chara is gone or after 8 frames, else moves 0x3C by 0x28 and 0x40
 * by 0x10 (in opposite directions, by unk1C) and takes the chara's point again. */
void func_800CABE8(Unk800CAACC *this) {
    Unk80076F34 *chara = (Unk80076F34 *)getEntityByHandle(this->charaHandle);

    if (chara == NULL || this->frame >= 8) {
        entityDelete(&this->entity);
    } else {
        if (this->unk1C == 0) {
            this->unk3C += 0x28;
            this->unk40 -= 0x10;
        } else {
            this->unk3C -= 0x28;
            this->unk40 += 0x10;
        }
        func_8007A650(chara, &this->position);
        this->frame++;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CACB4);

/* Constructs an entity of the class of virtual table D_8006D9F0, derived from func_800C3CFC's at pos. */
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3) {
    func_800C3CFC(this, pos, arg2, arg3);
    this->entity.vtable = &D_8006D9F0;
    this->unk50 = 8;
    return this;
}

/* The class of virtual tables D_8006D9C8 (and D_8006D9A8 for the base at 0x18), derived from Unk800C7024: its
 * index picks the word passed to func_800C7024 (D_8006DBC8) and the frame of the pieces func_800CB25C throws. */
typedef struct Unk800CAF2C {
    /* 0x00 */ Unk800C7024 base;
    /* 0xC0 */ s32 index;
    /* 0xC4 */ s32 angle;
    /* 0xC8 */ s32 unkC8; /* counts 0 to 3 */
} Unk800CAF2C;

extern u32 D_8006DBC8[];
extern VtableEntry D_8006D9A8[];
extern EntityVtable D_8006D9C8;

/* Constructs func_800C7024's entity with 0xC0, entry index of D_8006DBC8, 0xA00 and 2, then the virtual tables
 * D_8006D9C8 (and D_8006D9A8 for the base at 0x18). */
Unk800CAF2C *func_800CAF2C(Unk800CAF2C *this, Unk80076F34 *chara, Vec3 *position, Vec3 *velocity, s32 index,
                           s32 angle) {
    func_800C7024(&this->base, chara, position, velocity, 0xC0, D_8006DBC8[index], 0xA00, 2);
    this->base.parts.base.vtable = D_8006D9A8;
    this->base.parts.entity.vtable = &D_8006D9C8;
    this->index = index;
    this->angle = angle & 0xFFF;
    this->unkC8 = 0;
    return this;
}
void func_800CB25C(Unk800CAF2C *this);

/* The update of Unk800CAF2C: throws a piece (func_800CB25C), steps 0xC8 through 0 to 3, then the base's update. */
void func_800CAFC4(Unk800CAF2C *this, s32 arg) {
    func_800CB25C(this);
    this->unkC8 = (this->unkC8 + 1) & 3;
    func_800C71DC(&this->base, arg);
}

/* func_800C72D8 for a derived class of Unk800C7024: only when func_80078878 finds an entity in sum, which also
 * gets func_800773F4 with 50 on its character (func_8007AEEC). */
void func_800CB010(Unk800C7024 *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->parts.base, sum);
    func_800C73A8(this, func_800C7358(this));
    target = func_80078878(sum);
    if (target != NULL) {
        func_800773F4(func_8007AEEC((Unk80076F34 *)target), 50);
        func_800C7474(this, target);
    }
    entityDelete(&this->parts.entity);
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CB0B0);

struct Unk800CB338 *func_800CB338(struct Unk800CB338 *this, VECTOR *position, VECTOR *velocity, s32 index,
                                  s32 angle);

/* Throws a piece (func_800CB338, 0x60 bytes) from the position, a quarter of the velocity backwards in x and y with
 * up to 8 either way at random, as a sibling. */
void func_800CB25C(Unk800CAF2C *this) {
    VECTOR velocity;
    Entity *child;
    s32 vx;

    /* The match depends on vel.vx read into a local before the other words are copied (the game loads it first). */
    vx = this->base.vel.vx;
    velocity.vz = this->base.vel.vz;
    velocity.pad = this->base.unk9C;
    velocity.vx = -vx >> 2;
    velocity.vy = -this->base.vel.vy >> 2;
    velocity.vx += (mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    velocity.vy += (mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    child = allocEntityMemory(0x60);
    if (child != NULL) {
        func_800CB338(entityOperatorNewAt(0x60, child), (VECTOR *)&this->base.pos, &velocity, this->index,
                      this->angle);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* The entity class of virtual table D_8006D980 (func_800CB338): a sprite of Unk800C2F14 with a frame of
 * D_8006D940 and an angle. */
typedef struct Unk800CB338 {
    /* 0x00 */ Unk800C2F14 base;
    /* 0x58 */ s32 index;
    /* 0x5C */ s32 angle;
} Unk800CB338;

extern u16 D_8006D940[][10];
extern EntityVtable D_8006D980;

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D940);

/* Constructs an entity of D_8006D980: func_800C2F14 with frame index of D_8006D940, size 0x80 and color
 * 0x404040. */
Unk800CB338 *func_800CB338(Unk800CB338 *this, VECTOR *position, VECTOR *velocity, s32 index, s32 angle) {
    /* The match depends on the table's address taken into a local first (the game computes it before index * 20). */
    u16 (*frames)[10] = D_8006D940;

    func_800C2F14(&this->base, frames[index], position, velocity, 0x80, 0x404040, 0);
    this->base.entity.vtable = &D_8006D980;
    this->index = index;
    this->angle = angle;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CB3CC);

/* The destructor of the class of virtual table D_8006DB80: Unk80076308's, inlined. */
void func_800CB508(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an object of the class of vtable D_8006DB80, which derives from the class of
 * D_80065164, with a word. */
Unk800CB53C *func_800CB53C(Unk800CB53C *this, s32 arg) {
    this->vtable = D_8006DB80;
    this->unk4 = arg;
    return this;
}

/* The destructor of the entity class of vtable D_8006DB20, which func_800C9A88 installs. */
void func_800CB554(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAF0, which func_800C9AF4 installs. */
void func_800CB59C(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAC0, which func_800C9CA0 installs. */
void func_800CB5E4(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DA90, which func_800C9D0C installs. */
void func_800CB62C(Unk800C90FC *this, s32 flags) {
    func_800C8920(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D9F0, which func_800CAEF0 installs. */
void func_800CB674(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800CB6BC(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the entity class of vtable D_8006D59C, which func_800C2F14,
 * func_800C2FBC and func_800C6820 install: game has 15 identical ones. */
void func_800CB75C(Entity *this, s32 flags) {
    this->vtable = &D_8006D59C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D980);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D9A8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D9C8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D9F0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA18);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA40);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA68);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA90);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DAC0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DAF0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DB20);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DB50);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DB80);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DBA0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DBC8);
