#include "common.h"
#include <abs.h>
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "psyq.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"
#include "engine/gfx/scene_graph.h"

/* The stage's class, which func_800D3B74 constructs on func_8006ED20's. */
typedef struct Unk800D3B74 {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ u8 unkD38[0xD3C - 0xD38];
    /* 0xD3C */ s32 unkD3C; /* counted up by func_800D4A70 */
    /* 0xD40 */ u8 unkD40[0xD4C - 0xD40];
    /* 0xD4C */ u32 unkD4C; /* a sound's handle (stopSound) */
    /* 0xD50 */ ListNode unkD50; /* a list node the destructor unlinks */
    /* 0xD58 */ u8 unkD58[0xDA8 - 0xD58];
    /* 0xDA8 */ RECT unkDA8;     /* 16 by 1 pixels at (0x300, 0xFE): a CLUT */
    /* 0xDB0 */ u8 unkDB0[0xE90 - 0xDB0];
    /* 0xE90 */ u16 unkE90[16]; /* that CLUT, read back from VRAM by func_800D4074 */
    /* 0xEB0 */ u8 unkEB0[0xEC4 - 0xEB0];
    /* 0xEC4 */ MeshScene *unkEC4; /* the stage's meshes */
    /* 0xEC8 */ u32 unkEC8; /* an angle (0..0xFFF), stepped by func_800D704C */
    /* 0xECC */ u8 unkECC[0xEF4 - 0xECC];
    /* 0xEF4 */ s16 unkEF4;
    /* 0xEF6 */ u8 unkEF6[0x1020 - 0xEF6];
    /* 0x1020 */ u32 unk1020[4]; /* func_800D5A34 counts the non-zero ones */
    /* 0x1030 */ s32 unk1030;
    /* 0x1034 */ s32 unk1034[4]; /* func_800D59CC fills them with (r + i) & 3 */
    /* 0x1044 */ s32 unk1044;
    /* 0x1048 */ s16 unk1048;
    /* 0x104A */ u8 unk104A[0x1050 - 0x104A];
    /* 0x1050 */ s32 unk1050; /* while set, func_800D8930's quads turn red */
    /* 0x1054 */ u8 unk1054[0x1060 - 0x1054];
    /* 0x1060 */ Group unk1060; /* drawn by func_800D7A24's entity */
    /* 0x10C0 */ s32 unk10C0;   /* while set, that group is not drawn */
} Unk800D3B74;

void func_800D9B9C(void *this);
UnkSt00Entity *func_800D7978(UnkSt00Entity *this, Unk8006ED20 *owner, MeshScene *meshScene, s32 arg3);
void func_800D4BB0(Unk800D3B74 *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable D_800D3924;
extern EntityVtable D_800D394C;
extern EntityVtable D_800D3974;
extern EntityVtable D_800D399C;
extern EntityVtable D_800D39C4;
extern EntityVtable D_800D39EC;
extern EntityVtable D_800D3A14;
extern EntityVtable D_800D3A3C;
extern EntityVtable D_800D3AB4;
extern VtableEntry D_800D3ADC[];
extern VtableEntry D_800D3AFC[];

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3B74);

/* The stage's destructor: virtual table D_800D3AFC, the group at 0x1060, unlinks the node at 0xD50, then
 * func_8006ED20's class's destructor. */
void func_800D3C34(Unk800D3B74 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_800D3AFC;
    groupDestroy(&this->unk1060, DESTROY_BASES);
    /* listRemove(&this->unkD50), in the access order the game's compiler used */
    node = &this->unkD50;
    this->unkD50.next->prev = this->unkD50.prev;
    node->prev->next = this->unkD50.next;
    this->unkD50.next = node;
    this->unkD50.prev = node;
    func_8006F254(&this->base, flags);
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3CA8);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3D94);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3E80);

/* Reads the 16-colour CLUT at (0x300, 0xFE) back from VRAM into 0xE90. */
void func_800D4074(Unk800D3B74 *this) {
    this->unkDA8.x = 0x300;
    this->unkDA8.y = 0xFE;
    this->unkEF4 = 0;
    this->unkDA8.w = 16;
    this->unkDA8.h = 1;
    StoreImage(&this->unkDA8, (u_long *)this->unkE90);
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D40C0);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D4198);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D468C);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D47C8);

/* One frame of the stage: func_80071F6C, then flag 0x10C0 from bit 8 of the flags at 0x2C8; while that bit
 * is clear and (func_8006F144 is false or bit 4 is set), runs func_800D4BB0 and counts the frame at 0xD3C. */
void func_800D4A70(Unk800D3B74 *this) {
    func_80071F6C();
    if (func_8006F1A4(&this->base)) {
        this->unk10C0 = 1;
        return;
    }
    this->unk10C0 = 0;
    if (func_8006F144(&this->base) && !func_8006F190(&this->base)) {
        return;
    }
    func_800D4BB0(this);
    this->unkD3C++;
}

/* Stops the sound at 0xD4C. */
void func_800D4AE8(Unk800D3B74 *this) {
    stopSound(this->unkD4C);
}

/* Moves the mesh of node 28 out of the stage's meshes into the group at 0x1060, and creates the entity of
 * func_800D7978's class that draws that group, put into the entity group of this. */
void func_800D4B0C(Unk800D3B74 *this) {
    Mesh *mesh = this->unkEC4->meshes[28];
    Entity *entity;

    listRemove(&mesh->object.link);
    groupAddChild(&this->unk1060, &mesh->object);
    entity = NEW_ENTITY(sizeof(UnkSt00Entity), func_800D7978, &this->base, this->unkEC4, 0x1C);
    if (entity != NULL) {
        func_8006F0B4(&this->base, entity);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D4BB0);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D4DC4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D508C);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D5154);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D55B4);

/* Fills 0x1034 with four consecutive values (mod 4) from a random start, clears 0x1044 and 0x1030, and
 * sets 0x1048 to a random bit. */
void func_800D59CC(Unk800D3B74 *this) {
    u16 value = func_8006F004(&this->base) & 3;
    s32 i;

    for (i = 0; i < 4; i++) {
        this->unk1034[i] = value++ & 3;
    }
    this->unk1044 = 0;
    this->unk1030 = 0;
    this->unk1048 = func_8006F004(&this->base) & 1;
}

/* Returns how many of the four words at 0x1020 are set. */
s32 func_800D5A34(Unk800D3B74 *this) {
    s32 count = 0;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (this->unk1020[i] != 0) {
            count++;
        }
    }
    return count;
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D5A68);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D5E30);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D6534);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D6D74);

/* Turns the angle at 0xEC8 by 0x40. */
void func_800D704C(Unk800D3B74 *this) {
    this->unkEC8 = (this->unkEC8 + 0x40) & 0xFFF;
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7064);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D70E4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D76FC);

/* Constructs an entity of D_800D3AB4 for the Unk8006ED20 whose entity group will hold it, and the
 * MeshScene that object keeps at 0xEC4 (func_800D4B0C, which also passes 0x1C, unused). */
UnkSt00Entity *func_800D7978(UnkSt00Entity *this, Unk8006ED20 *owner, MeshScene *meshScene, s32 arg3) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3AB4;
    this->owner = owner;
    this->unk1C = meshScene;
    return this;
}

/* The destructor of the entity class of vtable D_800D3AB4, which func_800D7978 installs. */
void func_800D79D0(Entity *this, s32 flags) {
    this->vtable = &D_800D3AB4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Its update: unless the stage's 0x10C0 is set, draws the stage's group in view of camera and marks its
 * matrices out of date. */
void func_800D7A24(UnkSt00Entity *this, Camera *camera) {
    if (((Unk800D3B74 *)this->owner)->unk10C0 == 0) {
        sceneObjectDrawInView(&((Unk800D3B74 *)this->owner)->unk1060.object, camera);
        groupInvalidate(&((Unk800D3B74 *)this->owner)->unk1060);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7A70);

/* The destructor of the entity class of vtable D_800D3A3C, which func_800D7A70 installs. */
void func_800D7B78(Entity *this, s32 flags) {
    this->vtable = &D_800D3A3C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800D7BCC(void) {
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7BD4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7E90);

/* The destructor of the entity class of vtable D_800D3A14, which func_800D7E90 installs. */
void func_800D7F34(Entity *this, s32 flags) {
    this->vtable = &D_800D3A14;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7F88);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D7FDC);

/* An entity of the stage (func_800D819C, vtable D_800D39EC; 0x60 bytes): a 1024-wide quad of the owner, from
 * height -0xA8E to -0x68E. */
typedef struct Unk800D819C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *owner;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D[0x40 - 0x1D];
    /* 0x40 */ SVECTOR corners[4];
} Unk800D819C; /* size 0x60 */

/* Constructs the stage's quad entity of owner: corners from (-0x200, -0xA8E) to (0x200, -0x68E). */
Unk800D819C *func_800D819C(Unk800D819C *this, Unk8006ED20 *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D39EC;
    this->owner = owner;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0x200;
    this->corners[1].vx = this->corners[3].vx = 0x200;
    this->corners[0].vy = this->corners[1].vy = -0xA8E;
    this->corners[2].vy = this->corners[3].vy = -0x68E;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->unk1C = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D39EC, which func_800D819C installs. */
void func_800D8228(Entity *this, s32 flags) {
    this->vtable = &D_800D39EC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D827C);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D82D4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8470);

/* The destructor of the entity class of vtable D_800D39C4, which func_800D8470 installs. */
void func_800D85F4(Entity *this, s32 flags) {
    this->vtable = &D_800D39C4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8648);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D875C);

/* An entity of the stage (func_800D8930, vtable D_800D399C; func_800D4198 constructs it): two quads side by
 * side at z -0x400, teal while the stage's 0x1050 is clear and red while it is set. */
typedef struct Unk800D8930 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D3B74 *stage;
    /* 0x1C */ s32 red; /* the color below is the red one */
    /* 0x20 */ u32 color;
    /* 0x24 */ SVECTOR corners[8]; /* two quads of four */
} Unk800D8930;

/* Constructs the stage's two-quad entity: x from -0x540 to -0x240 and from 0x240 to 0x540, y from 0x60 to
 * 0x360, z -0x400. */
Unk800D8930 *func_800D8930(Unk800D8930 *this, Unk800D3B74 *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D399C;
    this->stage = stage;
    this->red = 0;
    this->corners[0].vx = this->corners[2].vx = -0x540;
    this->corners[1].vx = this->corners[3].vx = -0x240;
    this->corners[0].vy = this->corners[1].vy = 0x60;
    this->corners[2].vy = this->corners[3].vy = 0x360;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x400;
    this->corners[4].vx = this->corners[6].vx = 0x240;
    this->corners[5].vx = this->corners[7].vx = 0x540;
    this->corners[4].vy = this->corners[5].vy = 0x60;
    this->corners[6].vy = this->corners[7].vy = 0x360;
    this->corners[4].vz = this->corners[5].vz = this->corners[6].vz = this->corners[7].vz = -0x400;
    this->color = 0x808010;
    return this;
}

/* The destructor of the entity class of vtable D_800D399C, which func_800D8930 installs. */
void func_800D8A04(Entity *this, s32 flags) {
    this->vtable = &D_800D399C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Follows the stage's 0x1050: red (0x101080) while it is set, teal (0x808010) otherwise. */
void func_800D8A58(Unk800D8930 *this) {
    if (this->stage->unk1050) {
        if (this->red != 1) {
            this->color = 0x101080;
            this->red = 1;
        }
    } else if (this->red != 0) {
        this->color = 0x808010;
        this->red = 0;
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8AB4);

/* An entity of the stage (func_800D8C0C, vtable D_800D3974; nothing calls its constructor): a 512 by 512 quad
 * at depth -0x200, color 0x208080. */
typedef struct Unk800D8C0C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ void *unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E[0x20 - 0x1E];
    /* 0x20 */ u32 color;
    /* 0x24 */ SVECTOR corners[4];
} Unk800D8C0C;

/* Constructs the stage's colored quad entity: corners from (-0x100, -0x100) to (0x100, 0x100) at z -0x200. */
Unk800D8C0C *func_800D8C0C(Unk800D8C0C *this, void *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3974;
    this->unk18 = arg;
    this->unk1C = 0;
    /* The match depends on these chained assignments, as in st07's func_800D5080. */
    this->corners[0].vx = this->corners[2].vx = -0x100;
    this->corners[1].vx = this->corners[3].vx = 0x100;
    this->corners[0].vy = this->corners[1].vy = -0x100;
    this->corners[2].vy = this->corners[3].vy = 0x100;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x200;
    this->color = 0x208080;
    return this;
}

/* The destructor of the entity class of vtable D_800D3974, which func_800D8C0C installs. */
void func_800D8CA0(Entity *this, s32 flags) {
    this->vtable = &D_800D3974;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8CF4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8D48);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D8ED4);

/* The destructor of the entity class of vtable D_800D394C, which func_800D8ED4 installs. */
void func_800D8FB4(Entity *this, s32 flags) {
    this->vtable = &D_800D394C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9008);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9128);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9234);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D932C);

/* The entity func_800D950C constructs (virtual table D_800D3924): up to 16 items that slide out from
 * x = 0x880 (even) or -0x880 (odd), brightest at x = +-0xA80. */
typedef struct Unk800D950C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18; /* func_800D950C's second argument */
    /* 0x1C */ u16 frame;
    /* 0x1E */ u16 count; /* items moving: 2 more every 16 frames, up to 16 */
    /* 0x20 */ u16 speed[16];
    /* 0x40 */ u8 brightness[16];
    /* 0x50 */ u16 unk50[16]; /* the item stays still when set */
    /* 0x70 */ VECTOR position[16];
} Unk800D950C;

/* PsyQ libc (stdlib.h); not in psyq.h yet. */

/* Whether item i is an even one (they move right).
 * The match depends on this inline: its store-flag gives the game's xori/andi test. */
static inline s32 isEven(s32 i) {
    return (i & 1) == 0;
}

/* Constructs the stage's sliding items: even ones at x 0x880, odd ones at -0x880, all at y -0x900, still. */
Unk800D950C *func_800D950C(Unk800D950C *this, s32 arg) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3924;
    this->unk18 = arg;
    for (i = 0; i < 16; i++) {
        /* The match depends on a whole setVector in each arm (the stores before the join). */
        if (isEven(i)) {
            setVector(&this->position[i], 0x880, -0x900, 0);
        } else {
            setVector(&this->position[i], -0x880, -0x900, 0);
        }
        this->brightness[i] = 0;
        this->speed[i] = 0;
        this->unk50[i] = 0;
    }
    this->frame = 0;
    this->count = 0;
    return this;
}

/* The destructor of the entity class of vtable D_800D3924, which func_800D950C installs. */
void func_800D95C0(Entity *this, s32 flags) {
    this->vtable = &D_800D3924;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}


/* Unk800D950C's gather: every 16 frames two more items move; each speeds up, outwards, its brightness
 * falling with the distance to +-0xA80, and starts again from +-0x880 past it. */
void func_800D9614(Unk800D950C *this, s32 arg) {
    s32 i;

    if ((this->frame & 0xF) == 0) {
        this->count += 2;
        if (this->count > 16) {
            this->count = 16;
        }
    }
    for (i = 0; i < this->count; i++) {
        if (this->unk50[i] != 0) {
            continue;
        }
        if (isEven(i)) {
            /* The match depends on step and x being local to each arm (step before the position, abs()):
             * x then stays in one block and shares the position load's register. */
            s32 step = (s16)++this->speed[i] >> 2;
            s32 x = this->position[i].vx + step;

            this->position[i].vx = x;
            this->brightness[i] = 0x80 - (abs(x - 0xA80) >> 2);
            if (this->position[i].vx > 0xA80) {
                this->position[i].vx = 0x880;
                this->brightness[i] = 0;
                this->speed[i] = 0;
            }
        } else {
            s32 step = (s16)++this->speed[i] >> 2;
            s32 x = this->position[i].vx - step;

            this->position[i].vx = x;
            this->brightness[i] = 0x80 - (abs(x + 0xA80) >> 2);
            if (this->position[i].vx < -0xA80) {
                this->position[i].vx = -0x880;
                this->brightness[i] = 0;
                this->speed[i] = 0;
            }
        }
    }
    this->frame++;
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D977C);

/* Constructs an object of D_800D3ADC (an UnkStageObject: a part with a virtual table, a word and a
 * vector) for arg, with the vector (0x40, 0, 0). */
UnkStageObject *func_800D9970(UnkStageObject *this, s32 arg) {
    Vec3 vec;

    /* The match depends on vec, a whole vector copied in (its stores stay on the stack), and on filling it
     * after the vtable store. */
    this->vtable = D_800D3ADC;
    vec.vx = 0x40;
    vec.vy = 0;
    vec.vz = 0;
    this->unk4 = arg;
    this->unk8 = vec;
    return this;
}

/* Entry 0xC of D_800D3ADC: adds the word of the object to the part's unk18, and its vector to the part's
 * unk20 (func_80075B48 calls it for the part linked to the one the object owns). */
void func_800D99AC(UnkStageObject *this, Unk80075EE4 *part) {
    s32 x, y, z;

    part->unk18 += this->unk4;
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = part->unk20.vx;
    y = part->unk20.vy;
    z = part->unk20.vz;
    x += this->unk8.vx;
    y += this->unk8.vy;
    z += this->unk8.vz;
    part->unk20.vx = x;
    part->unk20.vy = y;
    part->unk20.vz = z;
}

/* The entity class func_800D99F0 constructs (virtual table D_800D3A8C): an
 * UnkStageObject part at 0x44 and an Unk80075EE4 member at 0x5C. */
typedef struct Unk800D99F0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *list;
    /* 0x1C */ u32 unk1C;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ Vec3 velocity;
    /* 0x3C */ u8 unk3C[0x40 - 0x3C];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ UnkStageObject unk44;
    /* 0x58 */ u8 unk58[0x5C - 0x58];
    /* 0x5C */ Unk80075EE4 unk5C;
} Unk800D99F0;

extern EntityVtable D_800D3A8C;


/* Constructs Unk800D99F0 at position and adds its member to list; its part's vector is the velocity
 * scaled and lifted as func_8008A9BC does it. */
Unk800D99F0 *func_800D99F0(Unk800D99F0 *this, Unk8006ED20 *list, Vec3 *position, u16 arg) {
    VECTOR velocity;
    VECTOR v;
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3A8C;
    func_800D9970(&this->unk44, 0x80002800);
    func_80075EE4(&this->unk5C, position, 0x80, &this->unk44);
    this->list = list;
    this->unk1C = arg;
    func_8006F064(list, &this->unk5C.node);
    this->unk40 = 0;
    this->unk5C.flags |= 1;
    /* The match depends on velocity and v being whole VECTORs on the stack (v = velocity copies its pad
     * word) and on copying the position before the velocity */
    velocity.vx = 0x20;
    velocity.vy = 0;
    velocity.vz = 0;
    v = velocity;
    v.vx <<= 5;
    v.vy -= 0x3000;
    v.vz <<= 5;
    this->unk44.unk8.vx = v.vx;
    this->unk44.unk8.vy = v.vy;
    this->unk44.unk8.vz = v.vz;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->velocity.vx = velocity.vx;
    this->velocity.vy = velocity.vy;
    this->velocity.vz = velocity.vz;
    return this;
}

/* Unk800D99F0's destructor: its member's and its part's inlined, then entityDestroy. */
void func_800D9B08(Unk800D99F0 *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &D_800D3A8C;
    this->unk5C.vtable = D_80065184;
    /* listRemove(&this->unk5C.node) written out: the match depends on this
     * access order (soundControlDestroy's) */
    link = &this->unk5C.node;
    this->unk5C.node.next->prev = this->unk5C.node.prev;
    link->prev->next = this->unk5C.node.next;
    this->unk5C.node.next = link;
    this->unk5C.node.prev = link;
    this->unk44.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9B9C);

/* The first method of the entity class of virtual table D_800D3A8C: func_800D9B9C. */
void func_800D9DBC(void *this) {
    func_800D9B9C(this);
}

/* The entity class func_800D9DDC constructs (virtual table D_800D3A64): an
 * UnkStageObject part at 0x3C and an Unk80075EE4 member at 0x54. */
typedef struct Unk800D9DDC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *list;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ Vec3 velocity;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ UnkStageObject unk3C;
    /* 0x50 */ u8 unk50[0x54 - 0x50];
    /* 0x54 */ Unk80075EE4 unk54;
} Unk800D9DDC;

extern EntityVtable D_800D3A64;

/* Constructs Unk800D9DDC at position and adds its member to list: func_800D99F0's
 * twin, with its part's vector at four times the velocity. */
Unk800D9DDC *func_800D9DDC(Unk800D9DDC *this, Unk8006ED20 *list, Vec3 *position) {
    VECTOR velocity;
    VECTOR v;
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3A64;
    func_800D9970(&this->unk3C, 0x80002800);
    func_80075EE4(&this->unk54, position, 0x80, &this->unk3C);
    this->list = list;
    func_8006F064(list, &this->unk54.node);
    this->unk54.flags |= 1;
    /* The match depends on the same velocity, v copy and order as in func_800D99F0 */
    velocity.vx = 0x20;
    velocity.vy = 0;
    velocity.vz = 0;
    v = velocity;
    v.vx <<= 2;
    v.vy <<= 2;
    v.vz <<= 2;
    this->unk3C.unk8.vx = v.vx;
    this->unk3C.unk8.vy = v.vy;
    this->unk3C.unk8.vz = v.vz;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->velocity.vx = velocity.vx;
    this->velocity.vy = velocity.vy;
    this->velocity.vz = velocity.vz;
    return this;
}

/* Unk800D9DDC's destructor: its member's and its part's inlined, then entityDestroy. */
void func_800D9EDC(Unk800D9DDC *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &D_800D3A64;
    this->unk54.vtable = D_80065184;
    /* listRemove(&this->unk54.node) written out: the match depends on this
     * access order (soundControlDestroy's) */
    link = &this->unk54.node;
    this->unk54.node.next->prev = this->unk54.node.prev;
    link->prev->next = this->unk54.node.next;
    this->unk54.node.next = link;
    this->unk54.node.prev = link;
    this->unk3C.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9F70);

/* Returns whether the stage's 0x1050 is set. */
s32 func_800DA140(Unk800D3B74 *this) {
    return this->unk1050 != 0;
}

/* The destructor of the class of virtual table D_800D3ADC: Unk80076308's, inlined. */
void func_800DA14C(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the object's vector, reading all three components before writing any (as vecCopy
 * does). */
void func_800DA180(UnkStageObject *this, Vec3 *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk8.vx = x;
    this->unk8.vy = y;
    this->unk8.vz = z;
}

/* Sets the object's word at 4. */
void func_800DA19C(UnkStageObject *this, s32 value) {
    this->unk4 = value;
}
