/* The battle scene (virtual table D_80064EC8): the sides' characters and input, the evolution, effect
 * and map files, and the continue or quit choice. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/cd/read.h"
#include "engine/gfx/box.h"
#include "engine/gfx/fade.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/text/text_printer.h"
#include "engine/system/memory.h"

void func_80071110(Unk8006ED20 *object);

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064CC8);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064CD8);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064CE8);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064CF8);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D08);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D18);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D28);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006ED20);

/* Returns the camera at 0x1B0. */
Camera *func_8006EF5C(Unk8006ED20 *this) {
    return &this->camera;
}

/* Returns the look-at at 0x12C. */
LookAt *func_8006EF64(Unk8006ED20 *this) {
    return &this->lookAt;
}

/* Returns unk10 of GAME_STATE's fighter of side. */
s32 func_8006EF6C(Unk8006ED20 *this, s32 side) {
    return GAME_STATE.fighters[side].unk10;
}

/* Adds add to unk10 of GAME_STATE's fighter of side. */
void func_8006EF90(Unk8006ED20 *this, s32 side, s32 add) {
    GAME_STATE.fighters[side].unk10 += add;
}

/* Sets bit of GAME_STATE.save.unk18. */
void func_8006EFC0(s16 bit) {
    GAME_STATE.save.unk18 |= 1 << bit;
}

/* Returns the word index of the array at 0x240. */
s32 func_8006EFE8(Unk8006ED20 *this, s32 index) {
    return this->unk240[index];
}

/* Returns the vector at 0x2A4. */
VECTOR *func_8006EFFC(Unk8006ED20 *this) {
    return &this->unk2A4;
}

/* Returns a random number from the object's own generator. */
u32 func_8006F004(Unk8006ED20 *this) {
    return mersenneTwisterGenerate(&this->random);
}

/* Adds node at the end of the list at 0x228. */
void func_8006F024(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk228, node);
}

/* Adds node at the end of the list at 0x230. */
void func_8006F044(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk230, node);
}

/* Adds node at the end of the list at 0x238. */
void func_8006F064(Unk8006ED20 *this, ListNode *node) {
    func_80075C64(&this->unk238, node);
}

/* Adds entity, if there is one, to the group at 0x3C; returns it. */
Entity *func_8006F084(Unk8006ED20 *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->unk3C, entity);
    }
    return entity;
}

/* Adds entity, if there is one, to the group at 0x4C; returns it. */
Entity *func_8006F0B4(Unk8006ED20 *this, Entity *entity) {
    if (entity != NULL) {
        entityGroupAppend(&this->unk4C, entity);
    }
    return entity;
}

/* Adds an entity to the group at 0x5C; returns the group. */
EntityGroup *func_8006F0E4(Unk8006ED20 *this, Entity *entity) {
    return entityGroupAppend(&this->unk5C, entity);
}

/* Whether the group at 0x5C holds any entity. */
s32 func_8006F104(Unk8006ED20 *this) {
    return entityGroupGetCount(&this->unk5C) != 0;
}

/* Sets bit 2 of the flags at 0x2C8 to flag. */
void func_8006F124(Unk8006ED20 *this, s32 flag) {
    this->unk2C8 = (this->unk2C8 & ~4) | ((flag & 1) << 2);
}

/* Whether bit 2 of the flags at 0x2C8 is set or the group at 0x5C holds any entity (func_8006F104). */
s32 func_8006F144(Unk8006ED20 *this) {
    return ((this->unk2C8 >> 2) & 1) || func_8006F104(this);
}

/* Returns bit 4 of the flags at 0x2C8. */
s32 func_8006F190(Unk8006ED20 *this) {
    return (this->unk2C8 >> 4) & 1;
}

/* Returns bit 8 of the flags at 0x2C8. */
s32 func_8006F1A4(Unk8006ED20 *this) {
    return (this->unk2C8 >> 8) & 1;
}

/* Returns the mesh scene. */
MeshScene *func_8006F1B8(Unk8006ED20 *this) {
    return this->meshScene;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F1C4);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F254);

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D38);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F414);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F57C);

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064D9C);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F690);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006F780);

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064DDC);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064DE4);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006FAF4);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8006FEB0);

/* The method at 0x50 of Unk8006ED20's virtual table, which the stages override: (this, index, meshScene) gives
 * the entity of node index of the mesh scene, or NULL. */
typedef Entity *(*Unk8006ED20NodeEntity)();

/* Creates the entities of the nodes of the mesh scene (the method at 0x50) and adds them to the group at 0x3C. */
void func_8006FFD4(Unk8006ED20 *this) {
    s32 count = meshSceneGetNodeCount(this->meshScene);
    s32 i;
    Entity *entity;

    for (i = 0; i < count; i++) {
        entity = ((Unk8006ED20NodeEntity)this->vtable[10].func)((u8 *)this + this->vtable[10].delta, i,
                                                                 this->meshScene);
        if (entity != NULL) {
            entityGroupAppend(&this->unk3C, entity);
        }
    }
}

/* Creates the entity of node index of the mesh scene (the method at 0x50); NULL for an index out of range. */
Entity *func_8007005C(Unk8006ED20 *this, s32 index) {
    if (index < meshSceneGetNodeCount(this->meshScene) && index >= 0) {
        return ((Unk8006ED20NodeEntity)this->vtable[10].func)((u8 *)this + this->vtable[10].delta, index,
                                                               this->meshScene);
    }
    return NULL;
}

/* Returns 0. */
s32 func_800700C0(void) {
    return 0;
}

/* An object Unk8006ED20 keeps at 0x2C by side (typed void * in game.h): its character at 0x8. */
typedef struct Unk800700C8 {
    /* 0x0 */ u8 unk0[8];
    /* 0x8 */ Unk80076F34 *chara;
} Unk800700C8;

/* Returns the character of side. */
Unk80076F34 *func_800700C8(Unk8006ED20 *this, s32 side) {
    return ((Unk800700C8 *)this->unk2C[side])->chara;
}

/* Sets the words at 0x33C to 0x354 (the stages pass their own), and 0x35C to half the smaller of unk34C - unk33C
 * and 4/3 of unk350 - unk340. */
void func_800700E4(Unk8006ED20 *this, s32 unk33C, s32 unk34C, s32 unk340, s32 unk350, s32 unk354) {
    s32 size = ((unk350 - unk340) << 2) / 3;

    this->unk33C = unk33C;
    this->unk340 = unk340;
    this->unk34C = unk34C;
    this->unk350 = unk350;
    this->unk354 = unk354;
    if (unk34C - unk33C < size) {
        size = unk34C - unk33C;
    }
    this->unk35C = size >> 1;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80070140);

/* Creates the object of a virtual table slot that the stages override with their own (0x6C bytes here). */
void *func_800702E0(void *this, s32 unk, s32 arg) {
    return func_8007C17C(operatorNew(0x6C), arg);
}
void func_80070C28(Unk8006ED20 *this);
void func_800703BC(Unk8006ED20 *this);

/* A step of the object's thread: switches to it, then gathers the group at 0x5C (with no input for both characters)
 * while it holds any entity, else runs func_800703BC and gathers the group at 0x3C. */
void func_80070314(Unk8006ED20 *this) {
    threadSwitchTo(&this->thread, 0);
    if (func_8006F104(this)) {
        entityGroupGather(&this->unk5C, (s32)this);
        func_8007504C(this->unk24[0], 0);
        func_8007504C(this->unk24[1], 0);
        func_80070C28(this);
    } else {
        func_800703BC(this);
        entityGroupGather(&this->unk3C, (s32)this);
        func_80070C28(this);
    }
}

/* The entry of the thread of func_8006ED20's object. */
void func_8007039C(s32 unused, Unk8006ED20 *object) {
    func_80071110(object);
}

void func_80070474(Unk8006ED20 *this);
void func_800708C4(Unk8006ED20 *this);
void func_800708F4(Unk8006ED20 *this);
void func_800706A4(Unk8006ED20 *this);

/* Runs func_80070474, gathers the group at 0x4C, and with bits 4 and 5 of the flags at 0x2C8 clear, updates both
 * sides' objects (0x2C) around it; runs func_800706A4 unless bit 8 is set. */
void func_800703BC(Unk8006ED20 *this) {
    s32 skipSides;

    func_80070474(this);
    skipSides = func_8006F190(this) || ((this->unk2C8 >> 5) & 1);
    if (!skipSides) {
        func_8007F22C(this->unk2C[0]);
        func_8007F22C(this->unk2C[1]);
    }
    entityGroupGather(&this->unk4C, (s32)this);
    if (!skipSides) {
        func_800708C4(this);
        func_800708F4(this);
    }
    if (!func_8006F1A4(this)) {
        func_800706A4(this);
    }
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80070474);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_800706A4);

/* Method 0x58 of the virtual table: whether position is more than 0x1000 below unk350 (y grows downwards). */
s32 func_80070824(Unk8006ED20 *this, Vec3 *position) {
    return this->unk350 + 0x1000 < position->vy;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80070838);

/* Runs func_8007A158 on the characters of both sides. */
void func_800708C4(Unk8006ED20 *this) {
    func_8007A158(((Unk800700C8 *)this->unk2C[0])->chara, ((Unk800700C8 *)this->unk2C[1])->chara);
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_800708F4);

void func_80072F2C(Unk8006ED20 *this);

/* Counts the step, runs the method at 0x60 of the virtual table, then func_80072F2C. */
void func_80070C28(Unk8006ED20 *this) {
    this->steps++;
    this->vtable[12].func((u8 *)this + this->vtable[12].delta);
    func_80072F2C(this);
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80070C74);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80070DBC);

/* A value of side by kind, on a 0-10000 scale: 0 is 10000 when 0x20 of its object at 0x2C is full, 1 that
 * fill as a share, 2 and 4 the count at 0xD28 / 30 * 100, 3 a flat 5000. */
s32 func_80070F40(Unk8006ED20 *this, s32 side, s32 kind) {
    Unk8007F184 *object = this->unk2C[side];
    s32 value = object->unk20;
    s32 most = object->unk1C;

    switch (kind) {
    case 0:
        return (value == most) ? 10000 : 0;
    case 1:
        return value * 10000 / most;
    case 2:
    case 4:
        return this->unkD28 / 30 * 100;
    case 3:
        return 5000;
    }
    return 0;
}

/* The sum of func_80070F40's values 0 to 2 for side, or of 3 and 4 with bit 0 of the flags at 0xD2C set. */
s32 func_80071014(Unk8006ED20 *this, s32 side) {
    s32 sum;

    if (!(this->unkD2C & 1)) {
        sum = func_80070F40(this, side, 0);
        sum += func_80070F40(this, side, 1);
        sum += func_80070F40(this, side, 2);
    } else {
        sum = func_80070F40(this, side, 3);
        sum += func_80070F40(this, side, 4);
    }
    return sum;
}

/* Switches back from the object's thread frames times (once at least). */
void func_800710B8(Unk8006ED20 *this, s32 frames) {
    do {
        threadSwitchBack(&this->thread, 0);
    } while (--frames > 0);
}

/* Sets bit 3 of the flags at 0x2C8. */
void func_800710FC(Unk8006ED20 *this) {
    this->unk2C8 |= 8;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071110);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071460);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_800715EC);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071738);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071894);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071A8C);
s32 func_80071D38(Unk8006ED20 *this, Entity *entity, s32 port);

/* Plays sounds 0x8009 and 0x42, then adds an entity of func_800CB7B0's class (10) and waits for it
 * (func_80071D38). */
void func_80071B74(Unk8006ED20 *this) {
    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    func_80071D38(this, func_8006F0B4(this, func_800CB7B0(entityOperatorNew(0x59C), 10, 0)), -1);
}

/* func_80071B74 with class 9, after func_80079268 on both sides' characters. */
void func_80071BF8(Unk8006ED20 *this) {
    Unk80076F34 *chara0;
    Unk80076F34 *chara1;

    soundControlPlaySound(&SOUND_CONTROL, 0x8009, 0);
    soundControlPlaySound(&SOUND_CONTROL, 0x42, 0);
    /* The match depends on reading both characters before the calls. */
    chara0 = ((Unk800700C8 *)this->unk2C[0])->chara;
    chara1 = ((Unk800700C8 *)this->unk2C[1])->chara;
    func_80079268(chara0);
    func_80079268(chara1);
    func_80071D38(this, func_8006F0B4(this, func_800CB7B0(entityOperatorNew(0x59C), 9, 0)), -1);
}

void func_80071EC0(Unk8006ED20 *this);

/* Stops the music and plays sound 0x8007, then func_80071BF8 with class 1, and ends with func_80071EC0. */
void func_80071C9C(Unk8006ED20 *this) {
    Unk80076F34 *chara0;
    Unk80076F34 *chara1;

    stopMusic();
    soundControlPlaySound(&SOUND_CONTROL, 0x8007, 0);
    chara0 = ((Unk800700C8 *)this->unk2C[0])->chara;
    chara1 = ((Unk800700C8 *)this->unk2C[1])->chara;
    func_80079268(chara0);
    func_80079268(chara1);
    func_80071D38(this, func_8006F0B4(this, func_800CB7B0(entityOperatorNew(0x59C), 1, 0)), -1);
    func_80071EC0(this);
}

s32 func_80071D7C(Unk8006ED20 *this, u32 handle, s32 port);

/* func_80071D7C for the entity's handle: waits until it is gone or the port presses 0x800. */
s32 func_80071D38(Unk8006ED20 *this, Entity *entity, s32 port) {
    return func_80071D7C(this, entityGetHandle(entity), port);
}

/* Waits, a frame at a time, until the entity of handle is gone: returns 0 then. With a port (>= 0), a press of
 * 0x800 on it destroys the entity and returns 1. */
s32 func_80071D7C(Unk8006ED20 *this, u32 handle, s32 port) {
    while (getEntityByHandle(handle) != NULL) {
        func_800710B8(this, 0);
        if (port >= 0 && (padManagerGetPressed(PAD_MANAGER_INSTANCE, port) & 0x800)) {
            destroyEntityByHandle(handle);
            return 1;
        }
    }
    return 0;
}

/* Counts unk2B8 down from 0x20 by 2 a frame, until it is under -4. */
void func_80071E10(Unk8006ED20 *this) {
    this->unk2B8 = 0x20;
    do {
        func_800710B8(this, 0);
        this->unk2B8 -= 2;
    } while (this->unk2B8 >= -4);
}

/* Counts unk2B8 up from 0 by 2 a frame, until it is over 0x24, then fades the screen to level 0 at once. */
void func_80071E60(Unk8006ED20 *this) {
    this->unk2B8 = 0;
    do {
        func_800710B8(this, 0);
        this->unk2B8 += 2;
    } while (this->unk2B8 < 0x25);
    fadeControlFadeTo(&FADE_CONTROL, 0, 0);
}

/* func_80071E60, then sets bit 0 of the flags at 0x2C8 and only switches frames: the thread never returns. */
void func_80071EC0(Unk8006ED20 *this) {
    func_80071E60(this);
    this->unk2C8 |= 1;
    for (;;) {
        func_800710B8(this, 0);
    }
}

void func_8006FEB0(Unk8006ED20 *this);
void func_8006FAF4(Unk8006ED20 *this);

/* Runs func_8006FEB0, func_8006FAF4 and the method at 0x8 of the virtual table, waits for the CD, then plays the
 * arena's music from where it was. */
void func_80071EF8(Unk8006ED20 *this) {
    func_8006FEB0(this);
    func_8006FAF4(this);
    this->vtable[1].func((u8 *)this + this->vtable[1].delta);
    syncCdfs(0);
    seekMusicTrack(gameStateGetArena(&GAME_STATE), 0);
    soundControlResumeMusic(&SOUND_CONTROL);
}

void func_80071F6C(void) {
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80071F74);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072088);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072680);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072748);

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064E28);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8007286C);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_8007293C);

/* Draws rect less a pixel of width and height, subtracting color from the screen (semi-transparent, in a texture
 * page of abr 2). */
void func_80072ADC(u_long *ot, RECT *rect, u32 color) {
    /* to set the color and code, and copy the position, a word at a time */
    TileWords *tile = (TileWords *)allocTile();

    tile->rgbCode = ((((TILE *)tile)->code | PRIM_SEMI_TRANS) << 24) | color;
    tile->xy = ((RectWords *)rect)->xy;
    ((TILE *)tile)->w = rect->w - 1;
    ((TILE *)tile)->h = rect->h - 1;
    AddPrim(ot, tile);
    AddPrim(ot, allocDrTpage(getTPage(0, 2, 960, 256)));
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072B74);

/* Prints text of length characters (8 pixels each) centered on the 320-pixel screen, at y. */
void func_80072C6C(Printer *printer, u8 *text, s32 length, s32 y) {
    printer->x = (0x140 - (length << 3)) >> 1;
    printer->y = y;
    printerPrint(printer, text);
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072CA0);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80072E20);

/* Sets the camera's view from the look-at and marks the world matrices of the groups and the mesh scene out of
 * date. */
void func_80072F2C(Unk8006ED20 *this) {
    MATRIX view;

    lookAtGetView(&this->lookAt, &view);
    cameraSetView(&this->camera, &view);
    groupInvalidate(&this->unkCC);
    groupInvalidate(&this->unk6C);
    groupInvalidate(&this->meshScene->group);
}

/* Picks a node of D_8006437C for b and a (func_80076474) with a temporary Unk80076444; when it picks one, writes
 * a through it (func_800765B4) and returns 1. */
s32 func_80072F80(Vec3 *a, Vec3 *b) {
    Unk80076444 picker;

    func_80076444(&picker);
    if (func_80076474(&picker, b, a)) {
        func_800765B4(&picker, a);
        return 1;
    }
    return 0;
}

/* Picks a node of D_8006437C for the two vectors (func_80076474) with a temporary Unk80076444. */
void func_80072FE0(Vec3 *a, Vec3 *b) {
    Unk80076444 picker;

    func_80076444(&picker);
    func_80076474(&picker, a, b);
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80073024);

INCLUDE_ASM("game/nonmatchings/battle_scene", func_80073130);

/* Returns the character of the other side than chara's (func_80077574). */
Unk80076F34 *func_80073218(Unk8006ED20 *this, Unk80076F34 *chara) {
    if (func_80077574(chara) == 0) {
        return ((Unk800700C8 *)this->unk2C[1])->chara;
    }
    return ((Unk800700C8 *)this->unk2C[0])->chara;
}

/* Returns the func_80075000 object of the character index. */
struct Unk80075000 *func_80073260(Unk8006ED20 *this, s32 index) {
    return this->unk24[index];
}

/* Returns the object at 0x2C of side; no caller found. */
void *func_80073274(Unk8006ED20 *this, s32 side) {
    return this->unk2C[side];
}

/* Sets bit 5 of the flags at 0x2C8. */
void func_80073288(Unk8006ED20 *this) {
    this->unk2C8 |= 0x20;
}

/* Clears bit 5 of the flags at 0x2C8. */
void func_8007329C(Unk8006ED20 *this) {
    this->unk2C8 &= ~0x20;
}

/* Sets bit 6 of the flags at 0x2C8. */
void func_800732B0(Unk8006ED20 *this) {
    this->unk2C8 |= 0x40;
}

/* Clears bit 6 of the flags at 0x2C8. */
void func_800732C4(Unk8006ED20 *this) {
    this->unk2C8 &= ~0x40;
}

INCLUDE_ASM("game/nonmatchings/battle_scene", func_800732D8);

void func_8007338C(void) {
}

void func_80073394(void) {
}

/* Returns the word at 0x314 (no caller). */
s32 func_8007339C(Unk8006ED20 *this) {
    return this->unk314;
}

/* Returns the word at 0x258 (no caller). */
s32 func_800733A8(Unk8006ED20 *this) {
    return this->unk258;
}

/* Returns the object func_8006ED20 constructed. */
Unk8006ED20 *func_800733B4(void) {
    return D_800D377C;
}

INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064EC8);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064F40);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064F44);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064FA4);
INCLUDE_RODATA("game/nonmatchings/battle_scene", D_80064FF4);
