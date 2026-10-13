#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "psyq.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/look_at.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/task/entity.h"
#include "engine/system/memory.h"

/* the virtual tables the functions below install */
extern EntityVtable D_800D3B1C;
extern EntityVtable D_800D3B44;
extern EntityVtable D_800D3B6C;
extern EntityVtable D_800D3B94;
extern EntityVtable D_800D3BBC;
extern EntityVtable D_800D3BE4;
extern EntityVtable D_800D3C0C;
extern EntityVtable D_800D3C34;
extern EntityVtable D_800D3C5C;
extern EntityVtable D_800D3C84;
extern EntityVtable D_800D3CAC;
extern EntityVtable D_800D3CD4;
extern VtableEntry D_800D3D24[];
extern VtableEntry D_800D3D44[];

/* The stage's class, derived from func_8006ED20's; func_800D3DBC constructs it. */
typedef struct Unk800D3DBC {
    /* 0x000 */ Unk8006ED20 base;
    /* 0xD38 */ MeshScene *scene;
    /* 0xD3C */ u32 unkD3C; /* the frames run by func_800D46D0 */
    /* 0xD40 */ u8 unkD40[0xD44 - 0xD40];
    /* 0xD44 */ u32 unkD44; /* sound handles, stopped by the destructor */
    /* 0xD48 */ u32 unkD48;
    /* 0xD4C */ Camera unkD4C; /* its table linked after the battle camera's (func_800D48E8) */
    /* 0xD88 */ Group unkD88;
    /* 0xDE8 */ u8 unkDE8[0xE12 - 0xDE8];
    /* 0xE12 */ s16 unkE12; /* set to 1 while func_800D5650's entities fall */
    /* 0xE14 */ u8 unkE14[0xE24 - 0xE14];
    /* 0xE24 */ s16 unkE24;
    /* 0xE26 */ s16 unkE26;
    /* 0xE28 */ u8 unkE28[0xF18 - 0xE28];
    /* 0xF18 */ RECT unkF18;     /* 16 by 1 pixels at (0x310, 0xFE): a CLUT */
    /* 0xF20 */ u16 unkF20[16]; /* that CLUT read back from VRAM (func_800D4D88) */
    /* 0xF40 */ u16 unkF40[16]; /* and a second copy */
    /* 0xF60 */ u8 unkF60[0xF7C - 0xF60];
    /* 0xF7C */ s16 unkF7C; /* the Unk800D5BA0 entities alive */
    /* 0xF7E */ s16 unkF7E; /* func_800D5650's state */
    /* 0xF80 */ s32 unkF80;
    /* 0xF84 */ s16 unkF84;
    /* 0xF86 */ s16 unkF86;
    /* 0xF88 */ s16 unkF88;
    /* 0xF8A */ s16 unkF8A;
    /* 0xF8C */ u8 unkF8C[0xF94 - 0xF8C];
    /* 0xF94 */ Vec3 unkF94; /* the characters' positions saved by func_800D4674 (sides 0 and 1) */
    /* 0xFA0 */ u8 unkFA0[0xFA4 - 0xFA0];
    /* 0xFA4 */ Vec3 unkFA4;
    /* 0xFB0 */ u8 unkFB0[0xFB4 - 0xFB0];
    /* 0xFB4 */ Vec3 unkFB4; /* and by func_800D4E80 */
    /* 0xFC0 */ u8 unkFC0[0xFC4 - 0xFC0];
    /* 0xFC4 */ Vec3 unkFC4;
    /* 0xFD0 */ u8 unkFD0[0xFD4 - 0xFD0];
    /* 0xFD4 */ s16 unkFD4; /* bit 8 of the flags at 0x2C8, copied each frame by func_800D46D0 */
} Unk800D3DBC;

/* this stage's functions that run each frame (func_800D46D0) */
void func_800D47B0(Unk800D3DBC *this);
void func_800D4950(Unk800D3DBC *this);
void func_800D58BC(Unk800D3DBC *this);
void func_800D4ECC(Unk800D3DBC *this);
void func_800D50D8(Unk800D3DBC *this);
void func_800D5274(Unk800D3DBC *this);
void func_800D5650(Unk800D3DBC *this);
void func_800D5A50(Unk800D3DBC *this);
void func_800D5534(Unk800D3DBC *this);
void func_800D4DEC(Unk800D3DBC *this);
void func_800D4C54(Unk800D3DBC *this);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D3DBC);

/* The stage's destructor: stops its two sounds, destroys its group and camera, then the battle
 * object's (func_8006F254). */
void func_800D3FF0(Unk800D3DBC *this, s32 flags) {
    this->base.vtable = D_800D3D44;
    stopSound(this->unkD44);
    stopSound(this->unkD48);
    groupDestroy(&this->unkD88, DESTROY_BASES);
    cameraDestroy(&this->unkD4C, DESTROY_BASES);
    func_8006F254(&this->base, flags);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4060);

/* Runs func_80070314 and func_800D47B0, then saves the positions of the characters of sides 0 and 1 at 0xF94
 * and 0xFA4. */
void func_800D4674(Unk800D3DBC *this) {
    func_80070314(&this->base);
    func_800D47B0(this);
    func_800775D8(func_800700C8(&this->base, 0), &this->unkF94);
    func_800775D8(func_800700C8(&this->base, 1), &this->unkFA4);
}

#if VERSION_US
/* Method 0x58 of the virtual table (func_80070824 in the base class): whether position is more than 0x200
 * below unk350 (y grows downwards). */
s32 func_800D49B0(Unk800D3DBC *this, Vec3 *position) {
    return this->base.unk350 + 0x200 < position->vy;
}

/* the camera's update of func_8006ED20's class, and its LookAt (at 0x12C) */
void func_80072088(Unk8006ED20 *object);
LookAt *func_8006EF64(Unk8006ED20 *object);

/* The camera's eye of func_8006ED20's class, which func_80072088 computes (as in st05). */
#define BASE_EYE(stage) (*(Vec3 *)&(stage)->base.unk2CC[0x31C - 0x2CC])

/* The stage's camera step: func_80072088, then keeps the eye at y 0 or above it (lower y), copying it into
 * the LookAt when it moves it. The match depends on the eye read into locals before the copy, as in st05's
 * func_800D4A10. */
void func_800D49C4(Unk800D3DBC *this) {
    LookAt *lookAt;
    s32 x;
    s32 y;
    s32 z;

    func_80072088(&this->base);
    if (BASE_EYE(this).vy > 0) {
        BASE_EYE(this).vy = 0;
        lookAt = func_8006EF64(&this->base);
        lookAt->dirty = 1;
        x = BASE_EYE(this).vx;
        y = BASE_EYE(this).vy;
        z = BASE_EYE(this).vz;
        lookAt->eye.vx = x;
        lookAt->eye.vy = y;
        lookAt->eye.vz = z;
    }
}
#endif

/* One frame of the stage: func_80071F6C, then 0xFD4 from bit 8 of the flags at 0x2C8; while that bit is clear
 * and (func_8006F144 is false or bit 4 is set), runs the stage's parts, plays sound 0x5A every 256 frames
 * and counts the frame at 0xD3C. */
void func_800D46D0(Unk800D3DBC *this) {
    func_80071F6C();
    if (func_8006F1A4(&this->base)) {
        this->unkFD4 = 1;
        return;
    }
    this->unkFD4 = 0;
    if (func_8006F144(&this->base) && !func_8006F190(&this->base)) {
        return;
    }
    func_800D4950(this);
    func_800D58BC(this);
    func_800D4ECC(this);
    func_800D50D8(this);
    func_800D5274(this);
    func_800D5650(this);
    func_800D5A50(this);
    func_800D5534(this);
    func_800D4DEC(this);
    func_800D4C54(this);
    if ((u8)this->unkD3C == 0) {
        soundControlPlaySound(&SOUND_CONTROL, 0x5A, 0);
    }
    this->unkD3C++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D47B0);

void func_800D54C8(Unk800D3DBC *this);

/* The stage's override of func_80072748 (in D_800D3D24): func_800D54C8 first, then links the stage's
 * own ordering table after the camera's first-drawn slot. */
void func_800D48E8(Unk800D3DBC *this) {
    Camera *camera;

    func_800D54C8(this);
    camera = func_8006EF5C(&this->base);
    func_80072748(this);
    orderingTableLinkAfter(&this->unkD4C.ot, camera->ot.firstDrawn);
}

/* Clears the four halfwords at 0xF84 and the two at 0xE24; called by the stage's init. */
void func_800D4934(Unk800D3DBC *this) {
    this->unkF84 = 0;
    this->unkF86 = 0;
    this->unkF88 = 0;
    this->unkF8A = 0;
    this->unkE24 = 0;
    this->unkE26 = 0;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4950);

Entity *func_800D9744(Entity *this, Unk8006ED20 *stage, Vec3 *vec);

/* Adds 16 entities of func_800D9744's class to the stage: eight with a random positive x and y -0x7D0, eight
 * with a random negative x and y -0x800. The match depends on each side's vy being a local set before the random
 * call: its constant then lives across the call and loop hoists it into a saved register, as in the game (a
 * plain `vec.vy = -0x7D0` after vx sets it next to its store and keeps it in the loop). */
void func_800D4B14(Unk800D3DBC *this) {
    Vec3 vec;
    Entity *entity;
    s32 i;

    for (i = 0; i < 16; i++) {
        if (i < 8) {
            s32 vy = -0x7D0;

            vec.vx = ((func_8006F004(&this->base) & 0x1F) << 4) + 0x800;
            vec.vy = vy;
        } else {
            s32 vy = -0x800;

            vec.vx = -(((func_8006F004(&this->base) & 0x1F) << 4) + 0x800);
            vec.vy = vy;
        }
        vec.vz = 0;
        entity = NEW_ENTITY(0x3C, func_800D9744, &this->base, &vec);
        if (entity != NULL) {
            func_8006F084(&this->base, entity);
        }
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4BFC);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4C54);

/* Reads the 16-colour CLUT at (0x310, 0xFE) back from VRAM twice, into 0xF20 and 0xF40. */
void func_800D4D88(Unk800D3DBC *this) {
    this->unkF18.x = 0x310;
    this->unkF18.y = 0xFE;
    this->unkF18.w = 16;
    this->unkF18.h = 1;
    StoreImage(&this->unkF18, (u_long *)this->unkF20);
    StoreImage(&this->unkF18, (u_long *)this->unkF40);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4DEC);

/* Saves the positions of the characters of sides 0 and 1 at 0xFB4 and 0xFC4. */
void func_800D4E80(Unk800D3DBC *this) {
    func_800775D8(func_800700C8(&this->base, 0), &this->unkFB4);
    func_800775D8(func_800700C8(&this->base, 1), &this->unkFC4);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D4ECC);

/* The node of the stage's scene whose mesh func_800D500C moves into the stage's group: the only code
 * difference of this function between the releases (the scene data behind it was not checked). */
#if VERSION_JP
#define ST01_GROUP_NODE 17
#else
#define ST01_GROUP_NODE 16
#endif

/* Copies the 8 by 64 pixels at (0x338, 0x80) in VRAM to (0x2E0, 0) and to (0x2E0, 0x40), then moves the mesh of
 * node ST01_GROUP_NODE of the stage's scene into the stage's group at 0xD88. */
void func_800D500C(Unk800D3DBC *this) {
    RECT rect;
    Mesh *mesh;

    rect.x = 0x338;
    rect.y = 0x80;
    rect.w = 8;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0);
    rect.x = 0x338;
    rect.y = 0x80;
    rect.w = 8;
    rect.h = 64;
    MoveImage(&rect, 0x2E0, 0x40);
    mesh = this->scene->meshes[ST01_GROUP_NODE];
    listRemove(&mesh->object.link);
    groupAddChild(&this->unkD88, &mesh->object);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D50D8);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D512C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5274);

/* Draws the stage's group at 0xD88 through its own camera at 0xD4C, set to the battle's view. */
void func_800D54C8(Unk800D3DBC *this) {
    Camera *camera = &this->unkD4C;
    MATRIX view;

    cameraClearOt(camera);
    lookAtGetView(func_8006EF64(&this->base), &view);
    cameraSetView(camera, &view);
    sceneObjectDrawInView(&this->unkD88.object, camera);
    groupInvalidate(&this->unkD88);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5534);

/* Clears 0xF7E and 0xF80 and stops the sound at 0xD48, if any. */
void func_800D5614(Unk800D3DBC *this) {
    u32 sound = this->unkD48;

    this->unkF7E = 0;
    this->unkF80 = 0;
    if (sound != 0) {
        stopSound(sound);
        this->unkD48 = 0;
    }
}

struct Unk800D5BA0 *func_800D5BA0(struct Unk800D5BA0 *this, Unk800D3DBC *stage, MeshScene *scene, Vec3 *vec);

/* A new Unk800D5BA0 entity for the stage, in its scene (func_800D5650 and func_800D58BC). The match depends on vec
 * going through this helper's parameter: its address is taken into a register before the allocation. */
static inline Entity *newUnk800D5BA0(Unk800D3DBC *stage, Vec3 *vec) {
    return NEW_ENTITY(0xF4, func_800D5BA0, stage, stage->scene, vec);
}

struct Unk800D7EC4 *func_800D7EC4(struct Unk800D7EC4 *this, Unk8006ED20 *stage);

/* A sequence by states (0xF7E), with 0xF80 as its timer: after 0x400 frames, 20 falling Unk800D7EC4 entities and
 * sound 0x58; once no Unk800D5BA0 entity is alive, one more falling entity every 8 frames and an Unk800D5BA0 every
 * 0x40 frames from frame 0x5B, while 0x32C sways; it ends at frame 0x11B or when func_8006F190 says so. */
void func_800D5650(Unk800D3DBC *this) {
    s32 i;
    s32 frame;

    switch (this->unkF7E) {
    case 0:
        if (++this->unkF80 < 0x400) {
            break;
        }
        for (i = 0; i < 20; i++) {
            Entity *entity = NEW_ENTITY(0x4C, func_800D7EC4, &this->base);
            if (entity != NULL) {
                func_8006F084(&this->base, entity);
            }
        }
        this->unkF80 = 0;
        this->unkF7E++;
        if (this->unkE12 == 0) {
            this->unkE12 = 1;
        }
        if (this->unkD48 == 0) {
            this->unkD48 = soundControlPlaySound(&SOUND_CONTROL, 0x58, 0);
        }
        /* fall through */
    case 1:
        if (this->unkF7C != 0) {
            break;
        }
        this->unkF7E++;
        /* fall through */
    case 2:
        if ((this->unkF80 & 7) == 0) {
            Entity *entity = NEW_ENTITY(0x4C, func_800D7EC4, &this->base);
            if (entity != NULL) {
                func_8006F084(&this->base, entity);
            }
        }
        if ((this->unkF80 & 0x3F) == 0 && this->unkF80 >= 0x5B) {
            Vec3 vec;
            Entity *entity;

            vec.vx = 0;
            vec.vy = 0;
            vec.vz = 0x2000;
            entity = newUnk800D5BA0(this, &vec);
            if (entity != NULL) {
                func_8006F084(&this->base, entity);
            }
        }
        frame = ++this->unkF80;
        this->base.unk32C += COS_SIN_TABLE[(frame << 10) & 0xFFF].sin >> 9;
        if (frame >= 0x11B || func_8006F190(&this->base) != 0) {
            u32 sound;

            if (this->unkE12 != 0) {
                this->unkE12 = 0;
            }
            sound = this->unkD48;

            this->unkF80 = 0;
            this->unkF7E = 0;
            if (sound != 0) {
                stopSound(sound);
                this->unkD48 = 0;
            }
        }
        break;
    }
}

/* Every 0x200 frames, when no Unk800D5BA0 entity is alive and 0xF7E is clear, adds a new one to the stage.
 * The two halfword tests are one word load: gcc merges tests of adjacent fields. */
void func_800D58BC(Unk800D3DBC *this) {
    Entity *entity;

    if (((this->unkD3C + 0x80) & 0x1FF) == 0 && this->unkF7C == 0 && this->unkF7E == 0
        && func_8006F190(&this->base) == 0) {
        Vec3 vec;

        vec.vx = 0;
        vec.vy = 0;
        vec.vz = 0x2000;
        entity = newUnk800D5BA0(this, &vec);
        if (entity != NULL) {
            func_8006F084(&this->base, entity);
        }
    }
}

/* Takes the mesh of node 9 out of the stage's scene's group and hides that node. */
void func_800D5974(Unk800D3DBC *this) {
    listRemove(&this->scene->meshes[9]->object.link);
    meshSceneHideNode(this->scene, 9);
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D59D0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5A50);

/* Constructs an object of the class of vtable D_800D3D24, which derives from the class of
 * D_80065164, with a word. */
UnkStageObject *func_800D5B80(UnkStageObject *this, s32 arg) {
    this->vtable = D_800D3D24;
    this->unk4 = arg;
    return this;
}

void func_800D5B98(void) {
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5BA0);

/* The entity func_800D5BA0 constructs (virtual table D_800D3CFC): a mesh of the stage's scene with an object
 * of D_800D3D24 and a member of func_80075EE4's class. */
typedef struct Unk800D5BA0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk800D3DBC *stage; /* counts them at 0xF7C */
    /* 0x1C */ MeshScene *scene;
    /* 0x20 */ Mesh *mesh; /* node 9 of the scene */
    /* 0x24 */ UnkStageObject unk24;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ Unk80075EE4 unk3C;
    /* 0x84 */ s32 unk84;
} Unk800D5BA0;

extern EntityVtable D_800D3CFC;

/* Deletes a scene object through its virtual destructor (engine scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Unk800D5BA0's destructor: one entity fewer for the stage, the mesh out of the scene and deleted, then its
 * member's and its base's destructors, inlined, and entityDestroy. */
void func_800D5CB0(Unk800D5BA0 *this, s32 flags) {
    ListNode *link;

    this->entity.vtable = &D_800D3CFC;
    this->stage->unkF7C--;
    listRemove(&this->mesh->object.link);
    sceneObjectDelete(&this->mesh->object);
    this->unk3C.vtable = D_80065184;
    /* listRemove(&this->unk3C.node) written out: the match depends on this access order, as in st04's
     * func_800D8618 */
    link = &this->unk3C.node;
    this->unk3C.node.next->prev = this->unk3C.node.prev;
    link->prev->next = this->unk3C.node.next;
    this->unk3C.node.next = link;
    this->unk3C.node.prev = link;
    this->unk24.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5DB0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D5E84);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D607C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D63F8);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D64D0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D652C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D67F8);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6994);

/* Returns the local matrix of the mesh (transformGetLocal, inlined). */
MATRIX *func_800D6C94(Unk800D5BA0 *this) {
    return &this->mesh->object.transform.local.m;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6CA0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6D08);

/* The destructor of the entity class of vtable D_800D3CD4, which func_800D6D08 installs. */
void func_800D6DF4(Entity *this, s32 flags) {
    this->vtable = &D_800D3CD4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6E48);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D6EA4);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7058);

/* The destructor of the entity class of vtable D_800D3CAC, which func_800D7058 installs. */
void func_800D7124(Entity *this, s32 flags) {
    this->vtable = &D_800D3CAC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity of vtable D_800D3CAC (func_800D7058): a 3 by 9 grid of points around a position, with a pulsing
 * level. */
typedef struct Unk800D7058 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u16 phase;
    /* 0x1A */ u16 level; /* 0x80 at first */
    /* 0x1C */ SVECTOR unk1C[3][9];
    /* 0xF4 */ Vec3 unkF4;
} Unk800D7058;

/* The update of D_800D3CAC's entity: the level follows a sine of the phase (64 to 96), and the phase steps. */
void func_800D7178(Unk800D7058 *this) {
    this->level = ((COS_SIN_TABLE[(this->phase << 4) & 0xFFF].sin >> 7) + 160) >> 1;
    this->phase++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D71B8);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D737C);

/* The destructor of the entity class of vtable D_800D3C84, which func_800D737C installs. */
void func_800D7408(Entity *this, s32 flags) {
    this->vtable = &D_800D3C84;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D745C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D74AC);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7670);

/* The destructor of the entity class of vtable D_800D3C5C, which func_800D7670 installs. */
void func_800D7750(Entity *this, s32 flags) {
    this->vtable = &D_800D3C5C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D77A4);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7880);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7A94);

/* The destructor of the entity class of vtable D_800D3C34, which func_800D7A94 installs. */
void func_800D7B8C(Entity *this, s32 flags) {
    this->vtable = &D_800D3C34;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7BE0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7CE8);

/* The entity of vtable D_800D3C0C, which func_800D5650 and func_800D5534 make: a gray particle that starts at a
 * random place and falls (func_800D7EC4) or starts around a position (func_800D7F98). */
typedef struct Unk800D7EC4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Unk8006ED20 *stage;
    /* 0x1C */ s16 unk1C; /* 4 */
    /* 0x1E */ s16 unk1E; /* a random bit */
    /* 0x20 */ u32 color;
    /* 0x24 */ s16 unk24;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 angle; /* random */
    /* 0x2A */ u8 unk2A[0x2C - 0x2A];
    /* 0x2C */ Vec3 pos;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ Vec3 start;
    /* 0x48 */ s32 unk48; /* random, 6 to 13 */
} Unk800D7EC4;

/* Constructs a particle of the stage at a random x (-0x800 to 0x7FF) and z (0 to 0x3FF), at y -0xD00. */
Unk800D7EC4 *func_800D7EC4(Unk800D7EC4 *this, Unk8006ED20 *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3C0C;
    this->stage = stage;
    this->start.vx = this->pos.vx = (func_8006F004(stage) & 0xFFF) - 0x800;
    this->start.vy = this->pos.vy = -0xD00;
    this->start.vz = this->pos.vz = func_8006F004(stage) & 0x3FF;
    this->unk48 = (func_8006F004(stage) & 7) + 6;
    this->unk1E = ((s32)func_8006F004(stage) >> 3) & 1;
    this->unk24 = this->unk26 = 0;
    this->angle = func_8006F004(stage) & 0xFFF;
    this->color = 0x808080;
    this->unk1C = 4;
    return this;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D7F98);

/* The destructor of the entity class of vtable D_800D3C0C, which func_800D7EC4 and func_800D7F98 install. */
void func_800D8088(Entity *this, s32 flags) {
    this->vtable = &D_800D3C0C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D80DC);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8308);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D84B8);

/* The destructor of the entity class of vtable D_800D3BE4, which func_800D84B8 installs. */
void func_800D85FC(Entity *this, s32 flags) {
    this->vtable = &D_800D3BE4;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8650);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8808);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8994);

/* The destructor of the entity class of vtable D_800D3BBC, which func_800D8994 installs. */
void func_800D8AA0(Entity *this, s32 flags) {
    this->vtable = &D_800D3BBC;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8AF4);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8C70);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8E1C);

/* The destructor of the entity class of vtable D_800D3B94, which func_800D8E1C installs. */
void func_800D8EB4(Entity *this, s32 flags) {
    this->vtable = &D_800D3B94;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8F08);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D8FD4);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9158);

/* The destructor of the entity class of vtable D_800D3B6C, which func_800D9158 installs. */
void func_800D927C(Entity *this, s32 flags) {
    this->vtable = &D_800D3B6C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D92D0);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9458);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9744);

/* The destructor of the entity class of vtable D_800D3B44, which func_800D9744 installs. */
void func_800D97F8(Entity *this, s32 flags) {
    this->vtable = &D_800D3B44;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D984C);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9A98);

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9BF4);

/* The destructor of the entity class of vtable D_800D3B1C, which func_800D9BF4 installs. */
void func_800D9C90(Entity *this, s32 flags) {
    this->vtable = &D_800D3B1C;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The entity class func_800D9BF4 constructs (virtual table D_800D3B1C); its
 * gather counts at 0x1C, which the constructor clears. */
typedef struct Unk800D9BF4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
    /* 0x1C */ s16 unk1C;
} Unk800D9BF4;

/* Adds 1 to 0x1C each time its group gathers it (the gather of D_800D3B1C). */
void func_800D9CE4(Unk800D9BF4 *this) {
    this->unk1C++;
}

INCLUDE_ASM("st01/nonmatchings/st01", func_800D9CF8);

/* The destructor of the class of virtual table D_800D3D24: Unk80076308's, inlined. */
void func_800D9F74(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the object's vector, reading all three components before writing any (as vecCopy
 * does). */
void func_800D9FA8(UnkStageObject *this, Vec3 *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk8.vx = x;
    this->unk8.vy = y;
    this->unk8.vz = z;
}
