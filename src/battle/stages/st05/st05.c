#include "common.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "battle/game_classes.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* The stage's class, which func_800D3CEC constructs on func_8006ED20's. */
typedef struct Unk800D3CEC {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ u8 unkD38[0xD80 - 0xD38];
    /* 0xD80 */ u32 unkD80; /* a sound's handle (stopSound) */
} Unk800D3CEC;

void func_800D4BB4(void *this);

/* the virtual tables the functions below install */
extern VtableEntry D_800D3C0C[];

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D3CEC);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D3FC0);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D40CC);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4210);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D44CC);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4570);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D45E0);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4768);

/* The stage's override of func_80070824 (at 0x58 of D_800D3C2C): whether a
 * character's position (func_800706A4 passes it) has a y greater than 0x400. */
s32 func_800D47D4(Unk800D3CEC *this, Vec3 *position) {
    return position->vy > 0x400;
}

/* The base's func_80072748, then the stage's own part (func_800D4BB4). */
void func_800D47E8(void *this) {
    func_80072748(this);
    func_800D4BB4(this);
}

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4814);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4A10);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4A74);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4B10);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4BB4);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4D3C);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D4E04);

void func_800D4FFC(void) {
}

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5004);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5144);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5254);

/* Starts sound 0x67 into 0xD80, unless one is playing there. */
void func_800D5364(Unk800D3CEC *this) {
    if (this->unkD80 == 0) {
        this->unkD80 = soundControlPlaySound(&SOUND_CONTROL, 0x67, 0);
    }
}

/* Stops the sound at 0xD80, if there is one. */
void func_800D53A8(Unk800D3CEC *this) {
    if (this->unkD80 != 0) {
        stopSound(this->unkD80);
        this->unkD80 = 0;
    }
}

/* The entity class func_800D53E4 constructs (virtual table D_800D3BE4): tables of halfwords from 0x47E,
 * then the handle of sound 0x66. */
typedef struct Unk800D53E4 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u8 unk18[0x8D8 - 0x18];
    /* 0x8D8 */ u32 sound; /* soundControlPlaySound's handle */
    /* 0x8DC */ s32 unk8DC;
} Unk800D53E4;

extern EntityVtable D_800D3BE4;

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D53E4);

/* The destructor of func_800D53E4's class: stops its sound. */
void func_800D54B8(Unk800D53E4 *this, s32 flags) {
    this->entity.vtable = &D_800D3BE4;
    stopSound(this->sound);
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5518);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5690);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5968);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5A90);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5BD4);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5D54);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D5E3C);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D621C);

/* The entity class func_800D627C constructs (virtual table D_800D3B6C). */
typedef struct Unk800D627C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 unk38;
} Unk800D627C;

extern EntityVtable D_800D3B6C;

/* Constructs an Unk800D627C at position, with unk30 and unk34; 0x38 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D627C *func_800D627C(Unk800D627C *this, Vec3 *position, s32 unk30, s32 unk34) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B6C;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = 0;
    this->unk30 = unk30;
    this->unk34 = unk34;
    this->unk38 = 0xFF;
    return this;
}

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6308);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6388);

/* The entity class func_800D64FC constructs (virtual table D_800D3B44). */
typedef struct Unk800D64FC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34;
} Unk800D64FC;

extern EntityVtable D_800D3B44;

/* Constructs an Unk800D64FC at position, with unk30 and unk2C; 0x34 is 0xFF.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D64FC *func_800D64FC(Unk800D64FC *this, Vec3 *position, s32 unk30, s32 unk2C) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B44;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = unk2C;
    this->unk30 = unk30;
    this->unk34 = 0xFF;
    return this;
}

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6584);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D65F8);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6740);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6814);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D68A4);

INCLUDE_ASM("asm/jp/st05/nonmatchings/st05", func_800D6A08);

/* The destructor of the class of virtual table D_800D3C0C: Unk80076308's, inlined. */
void func_800D6A14(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an object of the class of vtable D_800D3C0C, which derives from the class of
 * D_80065164, with a word. */
UnkStageObject *func_800D6A48(UnkStageObject *this, s32 arg) {
    this->vtable = D_800D3C0C;
    this->unk4 = arg;
    return this;
}

/* Sets the object's vector, reading all three components before writing any (as vecCopy
 * does). */
void func_800D6A60(UnkStageObject *this, Vec3 *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk8.vx = x;
    this->unk8.vy = y;
    this->unk8.vz = z;
}

/* The destructor of the entity class of vtable D_800D3BBC, which func_800D5968 installs. */
void func_800D6A7C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class func_800D5D54 constructs (virtual table D_800D3B94): an
 * Unk80076308 base at 0x18 (virtual table D_800D3C0C) and a member of
 * func_80075EE4's class at 0x30. */
typedef struct Unk800D5D54 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk80076308 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ Vec3 unk20; /* func_800D5D54's third argument */
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ Unk800761AC unk30;
} Unk800D5D54;

/* Unk800D5D54's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D6AC4(Unk800D5D54 *this, s32 flags) {
    ListNode *link;

    this->unk30.vtable = D_80065184;
    /* listRemove(&this->unk30.link) written out: the match depends on this
     * access order (soundControlDestroy's), as in st04's func_800D8618 */
    link = &this->unk30.link;
    this->unk30.link.next->prev = this->unk30.link.prev;
    link->prev->next = this->unk30.link.next;
    this->unk30.link.next = link;
    this->unk30.link.prev = link;
    this->unk18.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B6C, which func_800D627C installs. */
void func_800D6B4C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B44, which func_800D64FC installs. */
void func_800D6B94(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B1C, which func_800D6740 installs. */
void func_800D6BDC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
