/* Collision spheres (a position, a radius and an owner) and their overlap test, then the node picker of
 * func_80076444 that the fighters use. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/system/memory.h"

typedef struct Unk800763F4 {
    /* 0x00 */ u8 unk0[0x34];
    /* 0x34 */ Vec3 unk34;
} Unk800763F4;

/* The box func_8007691C tests points against: two corners. func_800766B4 walks an array of them (0x10 each)
 * and reads 0x8-0xC as the line equation normalX * x + normalY * y + offset (func_80076888, func_800768B8). */
typedef struct Unk8007691C {
    /* 0x0 */ s16 x0;
    /* 0x2 */ s16 y0;
    /* 0x4 */ s16 x1;
    /* 0x6 */ s16 y1;
    /* 0x8 */ s16 normalX;
    /* 0xA */ s16 normalY;
    /* 0xC */ s32 offset;
} Unk8007691C;
s32 func_8007691C(Unk8007691C *box, s32 x, s32 y);
ListNode *func_80076A00(void);

extern VtableEntry D_8006513C[];

/* Constructs an Unk80075EE4 (virtual table D_8006513C) of owner at position, with unk44: func_800761AC's
 * fields, inlined, with unk10 = 0, then its own virtual table (as func_80076394).
 * The match depends on the virtual table stored after the base fields (vtable first gives other registers to
 * the flags), and on reading the three words of position before storing any, as vecCopy does. */
Unk80075EE4 *func_80075EE4(Unk80075EE4 *this, Vec3 *position, s32 unk44, void *owner) {
    s32 x;
    s32 y;
    s32 z;

    this->node.next = &this->node;
    this->node.prev = &this->node;
    this->owner = owner;
    this->unk10 = 0;
    this->unk14 = NULL;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20.vx = 0;
    this->unk20.vy = 0;
    this->unk20.vz = 0;
    this->unk2C = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    this->vtable = D_8006513C;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->unk44 = unk44;
    this->unk34.vx = x;
    this->unk34.vy = y;
    this->unk34.vz = z;
    return this;
}

/* Whether other (unless its unk10 is set) is nearer to this than the sum of their unk44: the distance of their
 * vectors at 0x34. */
s32 func_80075F58(Unk80075EE4 *this, Unk80075EE4 *other) {
    VECTOR delta;
    s32 x;
    s32 y;
    s32 z;

    if (other->unk10 != 0) {
        return 0;
    }
    /* The match depends on x, y and z each set twice (read, then subtract): set-once locals let sched1 sink
     * every load to its subtraction. */
    x = this->unk34.vx;
    y = this->unk34.vy;
    z = this->unk34.vz;
    x -= other->unk34.vx;
    y -= other->unk34.vy;
    z -= other->unk34.vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    return vecGetLength(&delta) < this->unk44 + other->unk44;
}

INCLUDE_RODATA("game/nonmatchings/game_0c", D_800650F4);

INCLUDE_ASM("game/nonmatchings/game_0c", func_80075FE4);

/* Constructs the class of virtual table D_80065184 (Unk800761AC) at this: unlinked, with owner and unk10, its
 * words 0x14-0x2C cleared and flag bits 0 and 1 clear. Written on Unk80075EE4, which spells out the fields
 * game_classes.h's Unk800761AC leaves as gaps. */
Unk80075EE4 *func_800761AC(Unk80075EE4 *this, s32 unk10, void *owner) {
    this->vtable = D_80065184;
    this->node.next = &this->node;
    this->node.prev = &this->node;
    this->owner = owner;
    this->unk10 = unk10;
    this->unk14 = NULL;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20.vx = 0;
    this->unk20.vy = 0;
    this->unk20.vz = 0;
    this->unk2C = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

/* Returns unk10. */
s32 func_80076204(Unk800761AC *this) {
    return this->unk10;
}

/* Returns the owning entity (0x14). */
void *func_80076210(Unk800761AC *this) {
    return this->unk14;
}

/* Unk800761AC's destructor: takes it out of its list. */
void func_8007621C(Unk800761AC *this, s32 flags) {
    this->vtable = D_80065184;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets unk8 (the owner's second base). */
void func_80076274(Unk800761AC *this, void *unk8) {
    this->unk8 = unk8;
}

/* Sets the owning entity (0x14). */
void func_8007627C(Unk800761AC *this, void *unk14) {
    this->unk14 = unk14;
}

/* Sets bit 0 of the flags at 0xC to bit 0 of value; no caller found. */
void func_80076284(Unk800761AC *this, s32 value) {
    this->unkC = (this->unkC & ~1) | (value & 1);
}

/* Returns bit 0 of the flags at 0xC (func_80076284 sets it). */
s32 func_800762A0(Unk800761AC *this) {
    return this->unkC & 1;
}

/* Returns bit 1 of the flags at 0xC; no caller found. */
s32 func_800762AC(Unk800761AC *this) {
    return (this->unkC >> 1) & 1;
}

/* Clears bit 1 of the flags at 0xC. */
void func_800762C0(Unk800761AC *this) {
    this->unkC &= ~2;
}

/* Adds node at the end of the list head (func_80075C64); no caller found. */
void func_800762D4(ListNode *head, ListNode *node) {
    func_80075C64(head, node);
}

/* Constructs an Unk80076308: only its virtual table, D_80065164. */
Unk80076308 *func_800762F4(Unk80076308 *this) {
    this->vtable = D_80065164;
    return this;
}

/* Unk80076308's destructor. */
void func_80076308(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The destructor of func_80075EE4's class (virtual table D_8006513C): Unk800761AC's, inlined. */
void func_8007633C(Unk800761AC *this, s32 flags) {
    this->vtable = D_80065184;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an Unk80075EE4 (virtual table D_8006513C) of owner: func_800761AC's fields, inlined, with
 * unk10 = 0, then flag bit 0 set. */
Unk80075EE4 *func_80076394(Unk80075EE4 *this, void *owner) {
    this->node.next = &this->node;
    this->node.prev = &this->node;
    this->owner = owner;
    this->unk10 = 0;
    this->unk14 = NULL;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20.vx = 0;
    this->unk20.vy = 0;
    this->unk20.vz = 0;
    this->unk2C = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    this->vtable = D_8006513C;
    this->flags &= ~1;
    this->flags |= 1;
    return this;
}

/* Copies the vector at 0x34 to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800763F4(Unk800763F4 *this, Vec3 *out) {
    s32 x = this->unk34.vx;
    s32 y = this->unk34.vy;
    s32 z = this->unk34.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* Sets the vector at 0x34 from in (func_800763F4's counterpart).
 * The match depends on reading the three words before storing any, as vecCopy does. */
void func_80076414(Unk800763F4 *this, Vec3 *in) {
    s32 x = in->vx;
    s32 y = in->vy;
    s32 z = in->vz;

    this->unk34.vx = x;
    this->unk34.vy = y;
    this->unk34.vz = z;
}

/* Sets unk44 (func_80075EE4's third argument). */
void func_80076430(Unk80075EE4 *this, s32 unk44) {
    this->unk44 = unk44;
}

/* Returns unk44. */
s32 func_80076438(Unk80075EE4 *this) {
    return this->unk44;
}

/* Constructs the member at 0x260 of Unk80076F34: 0x78 is the first node of D_8006437C. */
Unk80076444 *func_80076444(Unk80076444 *this) {
    this->unk78 = (Unk80076A0C *)func_80076A00()->next;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0c", func_80076474);

/* Returns the node of D_8006437C picked last. */
Unk80076A0C *func_80076584(Unk80076444 *this) {
    return this->unk78;
}

/* Returns this; nothing calls it. */
Unk80076444 *func_80076590(Unk80076444 *this) {
    return this;
}

/* Returns the matrix of the node of D_8006437C picked last. */
MATRIX *func_80076598(Unk80076444 *this) {
    return this->unk78->unkC;
}

/* Returns the vector at 0x68. */
VECTOR *func_800765AC(Unk80076444 *this) {
    return &this->unk68;
}

Vec3 *func_800765F0(Vec3 *out, VECTOR *ends, s32 unk10, s32 unk14);

/* func_800765F0 of the two vectors at 0x48 and 0x58 with unk10 and unk14 into out; returns out. */
Vec3 *func_800765B4(Unk80076444 *this, Vec3 *out) {
    func_800765F0(out, &this->unk48, this->unk10, this->unk14);
    return out;
}

INCLUDE_ASM("game/nonmatchings/game_0c", func_800765F0);

INCLUDE_ASM("game/nonmatchings/game_0c", func_800766B4);

/* The dot product of the line's normal with point (x, y). */
s32 func_80076888(Unk8007691C *this, s32 *point) {
    return this->normalX * point[0] + this->normalY * point[1];
}

/* The line equation at point (x, y): its signed distance from the line, scaled by the normal. */
s32 func_800768B8(Unk8007691C *this, s32 *point) {
    return this->normalX * point[0] + this->normalY * point[1] + this->offset;
}

/* Whether point (x, y) is inside the box (func_8007691C). */
s32 func_800768F0(Unk8007691C *this, s32 *point) {
    return func_8007691C(this, point[0], point[1]);
}

/* Whether (x, y) is inside the circle whose diameter joins the two corners:
 * (x - x0)(x - x1) + (y - y0)(y - y1) < 1. */
s32 func_8007691C(Unk8007691C *box, s32 x, s32 y) {
    return (x - box->x0) * (x - box->x1) + (y - box->y0) * (y - box->y1) < 1;
}

/* Swaps the four words at a with the four at b by xor (swapFourWords' body, a == b left alone) and
 * returns b. */
u32 *func_80076958(u32 *a, u32 *b) {
    u32 a0, b0, a1, b1;

    /* The match depends on b == a (the game compares b first). */
    if (b == a) {
        return b;
    }
    a0 = a[0];
    b0 = b[0];
    a1 = a[1];
    b1 = b[1];
    b0 ^= a0;
    b1 ^= a1;
    a0 ^= b0;
    a1 ^= b1;
    b0 ^= a0;
    b1 ^= a1;
    a[0] = a0;
    b[0] = b0;
    a[1] = a1;
    b[1] = b1;

    a0 = a[2];
    b0 = b[2];
    a1 = a[3];
    b1 = b[3];
    b0 ^= a0;
    b1 ^= a1;
    a0 ^= b0;
    a1 ^= b1;
    b0 ^= a0;
    b1 ^= a1;
    a[2] = a0;
    b[2] = b0;
    a[3] = a1;
    b[3] = b1;
    return b;
}

extern Vec3 D_800D3780;

/* Copies v to the static vector D_800D3780 and returns it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *func_800769DC(Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    D_800D3780.vx = x;
    D_800D3780.vy = y;
    D_800D3780.vz = z;
    return &D_800D3780;
}

/* Returns the head of the list D_8006437C. */
ListNode *func_80076A00(void) {
    return &D_8006437C;
}

/* Moves node to the front of the list D_8006437C. */
void func_80076A0C(Unk80076A0C *node) {
    listRemove(&node->link);
    listInsertAfter(&D_8006437C, &node->link);
}

/* Moves node to the back of the list D_8006437C. */
void func_80076A5C(Unk80076A0C *node) {
    listRemove(&node->link);
    node->link.next = &D_8006437C;
    node->link.prev = D_8006437C.prev;
    D_8006437C.prev->next = &node->link;
    D_8006437C.prev = &node->link;
}

extern MATRIX D_800108B4;

/* Sets up node at the front of the list D_8006437C, with the matrix D_800108B4, unk8, unk10 and unk14 = 1. */
void func_80076AAC(Unk80076A0C *node, s32 unk8, s32 unk10) {
    listInsertAfter(&D_8006437C, &node->link);
    node->unkC = &D_800108B4;
    node->unk8 = unk8;
    node->unk10 = unk10;
    node->unk14 = 1;
}

INCLUDE_ASM("game/nonmatchings/game_0c", func_80076AF4);

INCLUDE_ASM("game/nonmatchings/game_0c", func_80076BB4);

/* Turns the angle from toward target by at most step (12-bit angles, the short way round). */
s32 func_80076C74(s32 target, s32 from, s32 step) {
    s16 delta = ((target - from - 0x800) & 0xFFF) - 0x800;

    if (delta < -step) {
        delta = -step;
    } else if (step < delta) {
        delta = step;
    }
    return from + delta;
}

INCLUDE_RODATA("game/nonmatchings/game_0c", D_8006513C);
INCLUDE_RODATA("game/nonmatchings/game_0c", D_80065164);
INCLUDE_RODATA("game/nonmatchings/game_0c", D_80065184);
