#include "common.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"

/* The entity class of virtual table D_800D3A4C (0x28 bytes). */
typedef struct Unk800D5164 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5164;

extern EntityVtable D_800D3A4C;

/* The entity class func_800D5040 constructs (virtual table D_800D3A74). */
typedef struct Unk800D5040 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
} Unk800D5040;

extern EntityVtable D_800D3A74;

/* The entity class func_800D557C constructs (virtual table D_800D3A24). */
typedef struct Unk800D557C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ Vec3 unk28;
} Unk800D557C;

extern EntityVtable D_800D3A24;

/* The entity class of virtual table D_800D3B5C. */
typedef struct Unk800D58D8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
} Unk800D58D8;

extern EntityVtable D_800D3B5C;

/* The entity class of virtual table D_800D3B2C (func_800D4468 constructs it). */
typedef struct Unk800D4468 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ s32 unk20; /* func_800D4468's second argument */
    /* 0x24 */ s32 unk24;
} Unk800D4468;

void func_800D45BC(Unk800D4468 *this, s32 arg);
void func_800D46C4(Unk800D4468 *this, s32 arg);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D3C10);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D3CD0);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D3D40);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D3ED8);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4008);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D41BC);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D433C);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D43A4);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4434);

extern EntityVtable D_800D3B2C;

/* Constructs the entity of virtual table D_800D3B2C with arg at 0x20, the rest cleared. */
Unk800D4468 *func_800D4468(Unk800D4468 *this, s32 arg) {
    entityInit(&this->entity, 0);
    /* The match depends on clearing 0x18 before installing the virtual table. */
    this->unk18 = 0;
    this->entity.vtable = &D_800D3B2C;
    this->unk1C = 0;
    this->unk1E = 0;
    this->unk1F = 0;
    this->unk20 = arg;
    this->unk24 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D44C4);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4520);

/* The update of Unk800D4468: func_800D45BC when 0x20 is 0, func_800D46C4 otherwise. */
void func_800D457C(Unk800D4468 *this, s32 arg) {
    if (this->unk20 == 0) {
        func_800D45BC(this, arg);
    } else {
        func_800D46C4(this, arg);
    }
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D45BC);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D46C4);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4958);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4A00);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4B78);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4D8C);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4E64);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4EC0);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D4F9C);

/* Constructs an entity of the class of virtual table D_800D3A74, 0x24 set to 0x100. */
Unk800D5040 *func_800D5040(Unk800D5040 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A74;
    this->unk18 = 0;
    this->unk1C = 0;
    this->unk20 = 0;
    this->unk24 = 0x100;
    return this;
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D508C);

void func_800D515C(void) {
}

/* Constructs an entity of the class of virtual table D_800D3A4C; func_800D508C
 * makes them, passing 0 and 1 in turn as arg. */
Unk800D5164 *func_800D5164(Unk800D5164 *this, s32 arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A4C;
    this->unk18 = 0;
    this->unk1C = 0x10;
    this->unk20 = arg;
    this->unk24 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D51BC);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D5254);

/* Constructs an entity of the class of virtual table D_800D3A24: 0x1C -0x200, 0x20 0x4000, the vector at
 * 0x28 zero. */
Unk800D557C *func_800D557C(Unk800D557C *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3A24;
    this->unk18 = 0;
    this->unk1C = -0x200;
    this->unk20 = 0x4000;
    this->unk28.vx = 0;
    this->unk28.vy = 0;
    this->unk28.vz = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D55D4);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D5690);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D57C4);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D5824);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D586C);

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D5874);

/* The destructor of the entity class of vtable D_800D3B5C, which func_800D58D8 installs. */
void func_800D5880(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D58C8(void) {
}

void func_800D58D0(void) {
}

/* Constructs an entity of the class of virtual table D_800D3B5C. */
Unk800D58D8 *func_800D58D8(Unk800D58D8 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B5C;
    this->unk18 = 0;
    return this;
}

INCLUDE_ASM("asm/jp/st06/nonmatchings/st06", func_800D5914);

/* The destructor of the entity class of vtable D_800D3B2C, which func_800D4468 installs. */
void func_800D591C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AFC, which func_800D4958 installs. */
void func_800D5964(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3ACC, which func_800D4D8C installs. */
void func_800D59AC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AA4, which func_800D4EC0 installs. */
void func_800D59F4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A74, which func_800D5040 installs. */
void func_800D5A3C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A4C, which func_800D5164 installs. */
void func_800D5A84(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3A24, which func_800D557C installs. */
void func_800D5ACC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D39FC, which func_800D57C4 installs. */
void func_800D5B14(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
