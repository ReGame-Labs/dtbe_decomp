#include "common.h"
#include "battle/game_classes.h"
#include "battle/game.h"
#include "battle/stage_objects.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/math/random.h"
#include "engine/math/vector.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"
#include "stdio.h"

/* The entity class of virtual table D_800D4004. */
typedef struct Unk800D4FC8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18;
} Unk800D4FC8;

extern EntityVtable D_800D4004;

/* The text terminal on the stage's mesh 15 (St02Terminal_init, virtual table ST02_TERMINAL_VTABLE, 0x1F8 bytes).
 * Strings go into a ring of 0x100 characters (St02Terminal_print), which St02Terminal_putNextChar moves into a
 * grid of 12 rows of 16 characters at the cursor, scrolling the rows; St02Terminal_tick blinks the cursor. */
typedef struct St02Terminal {
    /* 0x000 */ Entity entity;
    /* 0x018 */ Mesh *mesh;
    /* 0x01C */ s32 closed; /* while set, no text is added (st02_closeTerminal sets it) */
    /* 0x020 */ s32 frames; /* counts the ticks; bit 2 blinks the cursor */
    /* 0x024 */ s32 readIndex;
    /* 0x028 */ s32 writeIndex;
    /* 0x02C */ u8 queue[0x100];
    /* 0x12C */ s32 column; /* the cursor, 0 to 15 */
    /* 0x130 */ s32 row;    /* the cursor, 0 to 11, counted from topRow */
    /* 0x134 */ s32 topRow; /* the grid row shown first, which scrolling moves */
    /* 0x138 */ u8 grid[12][16];
} St02Terminal;

/* Arena 2's stage, "Turn" in STAGE_NAMES (StageTurn_init, virtual table STAGE_TURN_VTABLE): a Fight whose arena
 * turns about z with the fighters on it; its blocks go and come back, printing to a text terminal. */
typedef struct StageTurn {
    /* 0x000 */ Fight base;
    /* 0xD38 */ s32 unkD38;
    /* 0xD3C */ s32 angle; /* the arena's turn about z (st02_placeFighters, StageTurn_tick) */
    /* 0xD40 */ s32 unkD40;
    /* 0xD44 */ s32 unkD44;
    /* 0xD48 */ s32 unkD48;
    /* 0xD4C */ s32 unkD4C;
    /* 0xD50 */ u32 terminal; /* the St02Terminal's handle (StageTurn_startRound) */
    /* 0xD54 */ VECTOR fighterPoints[2]; /* a point per fighter, turned with the arena (st02_placeFighters) */
    /* 0xD74 */ s32 shakeFrames; /* frames of camera shake left (StageTurn_updateCamera) */
    /* 0xD78 */ s32 cameraWeight; /* the camera's easing weight, 0x1000 down to 0 (st02_startCameraSwing) */
} StageTurn;

extern VtableEntry ST02_HIT_VTABLE[];
extern VtableEntry STAGE_TURN_VTABLE[];

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable ST02_MESH_DRAWER_VTABLE;
extern EntityVtable ST02_MESH_MOVER_VTABLE;

/* A row of the table that StageTurn_startRound hands to createItemSpawner (ST02_POINTS, ended by unk0 = -1); x and y
 * receive the same row of D_800D40A4 transformed (st02_turnPoints). */
typedef struct Unk800D4114 {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s16 x;
    /* 0x6 */ s16 y;
    /* 0x8 */ s16 unk8;
} Unk800D4114;

extern VECTOR D_800D40A4[7];
extern Unk800D4114 ST02_POINTS[];

/* Transforms the seven points of D_800D40A4 by matrix and stores their x and y in ST02_POINTS's rows. */
void st02_turnPoints(MATRIX *matrix) {
    Unk800D4114 *row = ST02_POINTS;
    VECTOR *point = D_800D40A4;
    VECTOR out;
    s32 i;

    for (i = 6; i >= 0; i--) {
        matrixTransformVec(matrix, &out, point);
        point++;
        row->x = out.vx;
        row->y = out.vy;
        row++;
    }
}

/* Constructs the stage: Fight, with virtual table STAGE_TURN_VTABLE. */
StageTurn *StageTurn_init(StageTurn *this) {
    Fight_init(&this->base);
    this->base.vtable = STAGE_TURN_VTABLE;
    this->unkD44 = 0;
    return this;
}

INCLUDE_ASM("st02/nonmatchings/st02", StageTurn_startRound);

INCLUDE_ASM("st02/nonmatchings/st02", StageTurn_createNodeEntity);

INCLUDE_ASM("st02/nonmatchings/st02", StageTurn_tickScenery);

INCLUDE_ASM("st02/nonmatchings/st02", StageTurn_tick);

/* The stage's override of Fight_isFallen: whether a
 * character's position (Fight_catchFalls passes it) has a y greater than 0x1200. */
s32 StageTurn_isFallen(StageTurn *this, Vec3 *position) {
    return position->vy > 0x1200;
}

INCLUDE_ASM("st02/nonmatchings/st02", StageTurn_updateCamera);

/* Starts the camera's state 3 (StageTurn_updateCamera) with its weight at 0x1000, unless it is in it already. */
void st02_startCameraSwing(StageTurn *this) {
    if (this->base.cameraFocus != 3) {
        this->base.cameraFocus = 3;
        this->cameraWeight = 0x1000;
    }
}

void St02Terminal_print(St02Terminal *this, u8 *text);
void St02Terminal_printAt(St02Terminal *this, u8 *text, s32 column, s32 row);

/* Adds text to the St02Terminal entity's queue (St02Terminal_print), or forgets its handle once it is gone. */
void st02_printText(StageTurn *this, u8 *text) {
    St02Terminal *terminal = (St02Terminal *)getEntityByHandle(this->terminal);

    if (terminal != NULL) {
        St02Terminal_print(terminal, text);
    } else {
        this->terminal = 0;
    }
}

/* Writes text into the St02Terminal entity's grid at column, row (St02Terminal_printAt), or forgets its handle once
 * it is gone. */
void st02_printTextAt(StageTurn *this, u8 *text, s32 column, s32 row) {
    St02Terminal *terminal = (St02Terminal *)getEntityByHandle(this->terminal);

    if (terminal != NULL) {
        St02Terminal_printAt(terminal, text, column, row);
    } else {
        this->terminal = 0;
    }
}

/* Sets the St02Terminal entity closed (no more text), or forgets its handle once it is gone. */
void st02_closeTerminal(StageTurn *this) {
    St02Terminal *terminal = (St02Terminal *)getEntityByHandle(this->terminal);

    if (terminal != NULL) {
        terminal->closed = 1;
    } else {
        this->terminal = 0;
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D4DE4);

/* Puts each fighter at its point turned by the arena's angle, raised by the halfword at 0x2 of
 * its Fighter_getParams table, and clears its 0x3A4 and 0x3B0. */
void st02_placeFighters(StageTurn *this) {
    MATRIX rotation;
    VECTOR position;
    s32 side;

    matrixInitRotationZ(&rotation, this->angle);
    for (side = 0; side < 2; side++) {
        Fighter *fighter = Fight_getFighter(&this->base, side);

        matrixTransformVec(&rotation, &position, &this->fighterPoints[side]);
        position.vy += Fighter_getParams(fighter)->height;
        Fighter_setPosition(fighter, (Vec3 *)&position);
        fighter->velocity.vx = 0;
        fighter->velocity.vy = 0;
        fighter->velocity.vz = 0;
        fighter->velocityPad = 0;
    }
}

/* Constructs an entity of the class of virtual table D_800D4004. */
Unk800D4FC8 *func_800D4FC8(Unk800D4FC8 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D4004;
    this->unk18 = 0;
    return this;
}

/* Unk800D4FC8's tick: adds 0x40 to 0x18. */
void func_800D5004(Unk800D4FC8 *this) {
    this->unk18 += 0x40;
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D5018);

/* A hit of the stage (St02Hit_init): game.bin's Hit without its owner, derived from CollisionOwner. */
typedef struct St02Hit {
    /* 0x00 */ CollisionOwner base;
    /* 0x04 */ s32 damage; /* HIT_DAMAGE_MASK, with flags above it */
    /* 0x08 */ s32 kind;
    /* 0x0C */ Vec3 knockback;
} St02Hit;

/* St02Hit's slot 0x08 (game's Hit_apply one field earlier): adds its damage (saturating at HIT_DAMAGE_MASK), its
 * kind and its knockback into sum, whose flags it merges with its own. */
void St02Hit_apply(St02Hit *this, CollisionSphere *sum) {
    u32 flags = (this->damage | sum->hitDamage) & ~HIT_DAMAGE_MASK;
    s32 count = (this->damage & HIT_DAMAGE_MASK) + (sum->hitDamage & HIT_DAMAGE_MASK);
    s32 x, y, z;

    if (count > HIT_DAMAGE_MASK) {
        count = HIT_DAMAGE_MASK;
    }
    sum->hitDamage = flags | count;
    sum->hitKind += this->kind;
    /* The match depends on vecAddInPlace's shape: the three words read before any is written */
    x = sum->hitKnockback.vx;
    y = sum->hitKnockback.vy;
    z = sum->hitKnockback.vz;
    x += this->knockback.vx;
    y += this->knockback.vy;
    z += this->knockback.vz;
    sum->hitKnockback.vx = x;
    sum->hitKnockback.vy = y;
    sum->hitKnockback.vz = z;
}

/* The entity class func_800D554C constructs for a stage object (mesh scene, position): a St02Hit part
 * at 0x18, a mesh of node 2 of the scene at 0x80 and the mesh's world position at 0x84 (func_800D5C24). */
typedef struct Unk800D554C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ St02Hit unk18;
    /* 0x30 */ u8 unk30[0x34 - 0x30];
    /* 0x34 */ CollisionSphere part; /* of radius 0x80 at the position, for unk18; disabled at construction */
    /* 0x7C */ MeshScene *scene; /* meshSceneRelocateNodeTmd, meshSceneGetMesh */
    /* 0x80 */ Mesh *mesh;
    /* 0x84 */ Vec3 unk84;  /* its y falls by func_800D5964's steps */
    /* 0x90 */ u8 unk90[0x94 - 0x90];
    /* 0x94 */ Vec3 position;
    /* 0xA0 */ u8 unkA0[0xA4 - 0xA0];
    /* 0xA4 */ s32 fallSpeed; /* func_80079C1C's velocity.vy (func_800D5964) */
    /* 0xA8 */ s32 unkA8;
    /* 0xAC */ s32 unkAC;
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;
} Unk800D554C;

extern VtableEntry D_800D3F9C[];
extern EntityVtable D_800D3FBC;

void func_800D5B30(Unk800D554C *this);
void func_800D5BC4(Unk800D554C *this);
void func_800D5C24(Unk800D554C *this);

/* Constructs an Unk800D554C at position for a mesh scene: its St02Hit (damage 0x40004000, kind 4, no knockback;
 * the derived class's table D_800D3F9C), a disabled CollisionSphere for it, the mesh of node 2 added to the scene's
 * group, a random unkB8, then func_800D5C24, func_800D5B30 and func_800D5BC4. */
Unk800D554C *func_800D554C(Unk800D554C *this, MeshScene *scene, Vec3 *position) {
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    /* St02Hit_init(&this->unk18, 0x40004000, &ZERO_VECTOR, 4), inline (the vector read before any store). The match
     * depends on its own vx, vy, vz in this block, apart from the position's x, y, z below. */
    {
        s32 vx, vy, vz;

        this->unk18.base.vtable = ST02_HIT_VTABLE;
        this->unk18.damage = 0x40004000;
        this->unk18.kind = 4;
        vx = ZERO_VECTOR.vx;
        vy = ZERO_VECTOR.vy;
        vz = ZERO_VECTOR.vz;
        this->unk18.knockback.vx = vx;
        this->unk18.knockback.vy = vy;
        this->unk18.knockback.vz = vz;
    }
    this->unk18.base.vtable = D_800D3F9C;
    this->entity.vtable = &D_800D3FBC;
    CollisionSphere_init(&this->part, position, 0x80, &this->unk18);
    this->part.flags.bits.disabled = 1;
    this->part.entity = &this->entity;
    this->scene = scene;
    this->mesh = meshInit(operatorNew(sizeof(Mesh)), meshSceneRelocateNodeTmd(scene, 2), 0);
    groupAddChild(&scene->group, &this->mesh->object);
    this->unkB4 = 0;
    this->unkB8 = mersenneTwisterGenerate(&RANDOM) & 0xFF;
    this->unkBC = 0;
    /* The match depends on the position read before the four zero stores. */
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->fallSpeed = 0;
    this->unkA8 = 0;
    this->unkAC = 0;
    this->unkB0 = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    func_800D5C24(this);
    func_800D5B30(this);
    func_800D5BC4(this);
    return this;
}

/* Deletes a scene object through its virtual destructor (engine scene_graph.c's inline). */
static inline void sceneObjectDelete(SceneObject *object) {
    if (object != NULL) {
        object->vtable->destroy.func((u8 *)object + object->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Unk800D554C's destructor: takes its mesh out of the scene and deletes it, then its member's and its base's
 * destructors, inlined, and entityDestroy. */
void func_800D56AC(Unk800D554C *this, s32 flags) {
    ListNode *link;

    this->unk18.base.vtable = D_800D3F9C;
    this->entity.vtable = &D_800D3FBC;
    listRemove(&this->mesh->object.link);
    sceneObjectDelete(&this->mesh->object);
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.link) written out: the match depends on this access order, as in st04's
     * func_800D8618 */
    link = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    link->prev->next = this->part.link.next;
    this->part.link.next = link;
    this->part.link.prev = link;
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D57A0);

/* One step of its fall: gravity 0x71C on a mass of 0x18000 (func_80079C1C) moves unk84's y by the step in 1/256ths
 * and keeps the speed in fallSpeed; returns castMapRay's result for the ray from the old point 0x80 below unk84 to the
 * new one (the vector of the step then holds the ray's end). */
s32 func_800D5964(Unk800D554C *this) {
    Vec3 from;
    Vec3 move;
    Vec3 velocity;
    Vec3 force;

    from.vx = this->unk84.vx;
    from.vy = this->unk84.vy + 0x80;
    from.vz = 0;
    velocity.vx = 0;
    velocity.vy = this->fallSpeed;
    velocity.vz = 0;
    force.vx = 0;
    force.vy = 0x71C;
    force.vz = 0;
    func_80079C1C(&move, &velocity, &force, 0x18000);
    this->fallSpeed = velocity.vy;
    this->unk84.vy += move.vy >> 8;
    move.vx = this->unk84.vx;
    move.vy = this->unk84.vy + 0x80;
    move.vz = 0;
    return castMapRay(&move, &from);
}

void func_800D5A64(Unk800D554C *this);

/* The override of its St02Hit part's first method: adds that part into sum, and the first time
 * also spawns func_800D5A64's particles. */
void func_800D5A1C(Unk800D554C *this, CollisionSphere *sum) {
    St02Hit_apply(&this->unk18, sum);
    if (this->unkAC == 0) {
        func_800D5A64(this);
    }
    this->unkAC = 1;
}

/* Spawns the entity of ParticleBurst_init at 0x84 (radius 0x30, colour 0x808080), as a sibling, and plays
 * sound 0x4D. */
void func_800D5A64(Unk800D554C *this) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, child), 3, &this->unk84, 0x30, 0x80, -0x18, 0x808080);
    }
    entityAddSibling(&this->entity, child);
    soundControlPlaySound(&SOUND_CONTROL, 0x4D, 0);
}

/* Adds 0xBC to 0xB8; 0xB4 says whether it went past 0x100 (which it then takes off). */
void func_800D5AF8(Unk800D554C *this) {
    this->unkB8 += this->unkBC;
    if (this->unkB8 >= 0x100) {
        this->unkB8 -= 0x100;
        this->unkB4 = 1;
    } else {
        this->unkB4 = 0;
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D5B30);

/* Attaches its mesh's transform to the scene's mesh 15 (the one StageTurn_startRound gives the St02Terminal). */
void func_800D5BC4(Unk800D554C *this) {
    Mesh *parent = meshSceneGetMesh(this->scene, 0xF);

    transformAttach(&this->mesh->object.transform, &parent->object.transform);
}

/* Detaches its mesh's transform (transformDetach). */
void func_800D5C00(Unk800D554C *this) {
    transformDetach(&this->mesh->object.transform);
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D5C24);

INCLUDE_ASM("st02/nonmatchings/st02", func_800D5D04);

void func_800D5E58(Unk800D554C *this);

/* Once 0xA8 has run out: calls func_800D5E58 while 0xB4 is clear, and hides its mesh while 0xB4 is clear. */
void func_800D5DF0(Unk800D554C *this) {
    if (this->unkA8 > 0) {
        return;
    }
    if (this->unkB4 == 0) {
        func_800D5E58(this);
    }
    this->mesh->object.hidden = (this->unkB4 == 0);
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D5E58);

/* A block of the scenery (St02Block_init, virtual table ST02_BLOCK_VTABLE, 0x78 bytes), made for a node of the
 * stage's mesh scene (StageTurn_createNodeEntity): St02Block_tick hides the node and shows it again, telling the
 * St02Terminal. */
typedef struct St02Block {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *scene;
    /* 0x1C */ Mesh *mesh; /* the node's */
    /* 0x20 */ s32 node;
    /* 0x24 */ s32 unk24; /* bit 0 of an argument */
    /* 0x28 */ s32 state;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30;
    /* 0x34 */ s32 unk34; /* a count of frames */
    /* 0x38 */ char hideText[0x20]; /* "block -c%02d" with the node */
    /* 0x58 */ char showText[0x20]; /* "block -r%02d" with the node */
} St02Block;

extern EntityVtable ST02_BLOCK_VTABLE;
/* "block -c%02d" and "block -r%02d" */
extern char D_800D3C6C[];
extern char D_800D3C7C[];

/* Constructs an St02Block for node of scene, with bit 0 of unk24, and writes its two texts. */
St02Block *St02Block_init(St02Block *this, MeshScene *scene, s32 node, s32 unk24) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST02_BLOCK_VTABLE;
    this->scene = scene;
    this->mesh = meshSceneGetMesh(scene, node);
    this->node = node;
    this->unk24 = unk24 & 1;
    this->state = 0;
    this->unk2C = 0x80;
    this->unk30 = 0x1000;
    this->unk34 = 0;
    sprintf(this->hideText, D_800D3C6C, node);
    sprintf(this->showText, D_800D3C7C, node);
    return this;
}

/* The sounds of a block. */
enum {
    ST02_SOUND_BLOCK_SHOW = 0x5F, /* it comes back */
    ST02_SOUND_BLOCK_HIDE = 0x60, /* it goes */
};

s32 func_800D6298(St02Block *this, StageTurn *stage);

/* St02Block's tick: once func_800D6298 has held for 61 frames it hides the node (ST02_SOUND_BLOCK_HIDE), then
 * eases unk30 to 0x2000 and unk2C to 0 until the count runs out (ST02_SOUND_BLOCK_SHOW), then raises unk2C back to 0x80
 * and shows the node; each step sends its text to the St02Terminal entity. */
void St02Block_tick(St02Block *this, StageTurn *stage) {
    switch (this->state) {
    case 0:
        if (func_800D6298(this, stage)) {
            this->unk34++;
            if (this->unk34 >= 0x3D) {
                meshSceneHideNode(this->scene, this->node);
                this->unk2C = 0x400;
                st02_printText(stage, this->hideText);
                soundControlPlaySound(&SOUND_CONTROL, ST02_SOUND_BLOCK_HIDE, 0);
                this->state++;
            }
        } else {
            this->unk34--;
            if (this->unk34 < 0) {
                this->unk34 = 0;
            }
        }
        break;
    case 1:
        this->unk30 += (0x2000 - this->unk30) >> 2;
        this->unk2C += -this->unk2C >> 2;
        this->unk34--;
        if (this->unk34 <= 0) {
            this->unk30 = 0x1000;
            this->unk2C = 0;
            st02_printText(stage, this->showText);
            soundControlPlaySound(&SOUND_CONTROL, ST02_SOUND_BLOCK_SHOW, 0);
            this->state++;
        }
        break;
    case 2:
        this->unk2C += 4;
        if (this->unk2C > 0x80) {
            meshSceneShowNode(this->scene, this->node);
            this->unk2C = 0x80;
            this->state = 0;
        }
        break;
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D6298);

INCLUDE_ASM("st02/nonmatchings/st02", St02Block_draw);

/* Constructs a St02MeshMover for a mesh of the stage and where it is (StageTurn_createNodeEntity). */
StageMeshMover *St02MeshMover_init(StageMeshMover *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST02_MESH_MOVER_VTABLE;
    this->mesh = mesh;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->motion = 0;
    this->frame = 0;
    return this;
}

/* St02MeshMover's destructor. */
void St02MeshMover_destroy(Entity *this, s32 flags) {
    this->vtable = &ST02_MESH_MOVER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St02MeshMover's tick: turns its mesh about z by motion, which grows by 0xC0, and counts its frames. */
void St02MeshMover_tick(StageMeshMover *this) {
    Mesh *mesh = this->mesh;

    mesh->object.transform.valid = 0;
    matrixSetRotationZ(&mesh->object.transform.local.m, this->motion);
    this->motion += 0xC0;
    this->frame++;
}

/* The entity class func_800D6924 constructs (virtual table D_800D3F24). */
typedef struct Unk800D6924 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[4];
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 unk30; /* bit 0 of an argument */
} Unk800D6924;

extern EntityVtable D_800D3F24;

/* Constructs an Unk800D6924 at position, with unk2C and bit 0 of unk30.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Unk800D6924 *func_800D6924(Unk800D6924 *this, Vec3 *position, s32 unk2C, s32 unk30) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3F24;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk28 = 0;
    this->unk2C = unk2C;
    this->unk30 = unk30 & 1;
    return this;
}

/* Unk800D6924's tick: steps unk28 through 0 to 3 and adds 0x10 to unk2C. */
void func_800D69A8(Unk800D6924 *this) {
    this->unk28 = (this->unk28 + 1) & 3;
    this->unk2C += 0x10;
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D69C8);

extern EntityVtable ST02_TERMINAL_VTABLE;

void St02Terminal_clearQueue(St02Terminal *this);
void St02Terminal_clearGrid(St02Terminal *this);

/* Constructs the terminal for mesh, then clears its queue and its grid. */
St02Terminal *St02Terminal_init(St02Terminal *this, Mesh *mesh) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST02_TERMINAL_VTABLE;
    this->mesh = mesh;
    this->closed = 0;
    this->frames = 0;
    St02Terminal_clearQueue(this);
    St02Terminal_clearGrid(this);
    return this;
}

void St02Terminal_putNextChar(St02Terminal *this);

/* Moves two characters of the queue into the grid, then draws the cursor, '_' or ' ' by turns. */
void St02Terminal_tick(St02Terminal *this) {
    s32 i;
    s32 row;

    for (i = 1; i >= 0; i--) {
        St02Terminal_putNextChar(this);
    }
    row = (this->topRow + this->row) % 12;
    this->grid[row][this->column] = (this->frames & 4) ? ' ' : '_';
    this->frames++;
}

void St02Terminal_clearRow(St02Terminal *this, s32 row);

/* Ends the cursor's line: a 0 at the cursor, then column 0 of the next row. */
static inline void endLine(St02Terminal *this, s32 gridRow) {
    this->grid[gridRow][this->column] = 0;
    this->column = 0;
    this->row++;
}

/* Moves the next character of the queue into the grid at the cursor: '\n' ends the line, as a 0 does after some
 * text unless the terminal is closed; past column 15 the cursor wraps, and past row 11 the grid scrolls up a row
 * (the top one cleared). The match depends on the switch with its default case first (the game lays the bodies out
 * default, '\n', '\0', the two endLine copies cross-jumped into one tail). */
void St02Terminal_putNextChar(St02Terminal *this) {
    s32 gridRow;
    u8 c;

    if (this->readIndex == this->writeIndex) {
        return;
    }
    c = this->queue[this->readIndex++];
    gridRow = (this->topRow + this->row) % 12;
    if (this->readIndex >= 0x100) {
        this->readIndex = 0;
    }
    switch (c) {
    default:
        this->grid[gridRow][this->column++] = c;
        if (this->column >= 16) {
            this->column = 0;
            this->row++;
        }
        break;
    case '\n':
        endLine(this, gridRow);
        break;
    case '\0':
        if (this->column != 0 && !this->closed) {
            endLine(this, gridRow);
        }
        break;
    }
    if (this->row >= 12) {
        this->row = 11;
        St02Terminal_clearRow(this, this->topRow);
        this->topRow++;
        if (this->topRow >= 12) {
            this->topRow = 0;
        }
    }
}

/* Empties the queue. */
void St02Terminal_clearQueue(St02Terminal *this) {
    s32 i = 0xFF;
    u8 *p = &this->queue[0xFF];

    /* The match depends on setting the count before the pointer. */
    for (; i >= 0; i--) {
        *p-- = 0;
    }
    this->readIndex = 0;
    this->writeIndex = 0;
}

INCLUDE_ASM("st02/nonmatchings/st02", St02Terminal_clearGrid);

/* Clears a row of the grid. */
void St02Terminal_clearRow(St02Terminal *this, s32 row) {
    s32 i;

    for (i = 15; i >= 0; i--) {
        this->grid[row][i] = 0;
    }
}

/* Clamps a field to 0..count - 1 through a pointer to it: the match depends on the stores going through that
 * pointer (its address is computed before the clamp). */
#define CLAMP_INDEX(field, count)                                                       \
    {                                                                                   \
        s32 *value = &(field);                                                          \
        *value = (*value < 0) ? 0 : ((*value >= (count)) ? (count) - 1 : *value);       \
    }

/* Erases the cursor and moves it to column, row (clamped to the grid). */
void St02Terminal_moveCursor(St02Terminal *this, s32 column, s32 row) {
    s32 gridRow = (this->topRow + this->row) % 12;

    this->grid[gridRow][this->column] = 0;
    this->column = column;
    this->row = row;
    CLAMP_INDEX(this->column, 16);
    CLAMP_INDEX(this->row, 12);
}

/* Adds text, with its terminating 0, to the queue (unless the terminal is closed). */
void St02Terminal_print(St02Terminal *this, u8 *text) {
    if (this->closed == 0) {
        s32 i = this->writeIndex;

        do {
            this->queue[i] = *text;
            i++;
            if (i >= 0x100) {
                i = 0;
            }
        } while (*text++ != 0);
        this->writeIndex = i;
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", St02Terminal_printAt);

INCLUDE_ASM("st02/nonmatchings/st02", St02Terminal_draw);

/* The entity class of virtual table D_800D3ED4 (0x34 bytes), made for the stage mesh 15 (StageTurn_tickScenery). */
typedef struct Unk800D73E8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ s32 unk1C; /* bits 0 and 1 of an argument */
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 state;
    /* 0x30 */ s32 unk30; /* frames left in state 1 */
} Unk800D73E8;

extern EntityVtable D_800D3ED4;

/* Constructs an Unk800D73E8 for mesh; bit 1 of flags makes it wait 0x7C frames instead of 0x5C. */
Unk800D73E8 *func_800D73E8(Unk800D73E8 *this, Mesh *mesh, s32 flags) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3ED4;
    this->mesh = mesh;
    this->unk1C = flags & 3;
    this->unk20 = 0x990;
    this->unk24 = 0x990;
    this->unk28 = 0;
    this->state = 0;
    this->unk30 = (flags & 2) ? 0x7C : 0x5C;
    return this;
}

/* Unk800D73E8's tick: eases 0x20 and 0x24 to 0x264, raises 0x28 by 8 up to 0x80, waits,
 * then grows 0x20 and 0x24 and lowers 0x28 by 8 until it deletes itself. */
void func_800D746C(Unk800D73E8 *this) {
    this->unk20 += (0x264 - this->unk20) >> 2;
    this->unk24 += (0x264 - this->unk24) >> 2;
    switch (this->state) {
    case 0:
        this->unk28 += 8;
        if (this->unk28 >= 0x80) {
            this->state = 1;
        }
        break;
    case 1:
        this->unk30--;
        if (this->unk30 < 0) {
            this->state = 2;
        }
        break;
    case 2:
        this->unk20 += 0x40;
        this->unk24 += 0x20;
        this->unk28 -= 8;
        if (this->unk28 <= 0) {
            entityDelete(&this->entity);
        }
        break;
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D7574);

/* The screen of node 15: its quads scroll at four angles (St02Screen_init, virtual table ST02_SCREEN_VTABLE,
 * 0xA0 bytes), made for a mesh of the stage (StageTurn_createNodeEntity). */
typedef struct St02Screen {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ s32 unk1C; /* 0x1000 down to -0x1200 by 0x10, again and again */
    /* 0x20 */ s32 unk20[32];
} St02Screen;

extern EntityVtable ST02_SCREEN_VTABLE;

/* Constructs an St02Screen for mesh: unk20[i] = i up to 8, the rest of unk20 -1. */
St02Screen *St02Screen_init(St02Screen *this, Mesh *mesh) {
    s32 *table;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST02_SCREEN_VTABLE;
    this->mesh = mesh;
    this->unk1C = 0x1000;
    /* The match depends on taking the table's address before the two loops. */
    table = this->unk20;
    for (i = 8; i >= 0; i--) {
        table[i] = i;
    }
    for (i = 9; i < 32; i++) {
        table[i] = -1;
    }
    return this;
}

/* St02Screen's tick: lowers unk1C by 0x10, back to 0x1000 once below -0x1200. */
void St02Screen_tick(St02Screen *this) {
    this->unk1C -= 0x10;
    if (this->unk1C < -0x1200) {
        this->unk1C = 0x1000;
    }
}

void func_800D7918(St02Screen *this, s32 arg, s32 angle);
void func_800D7BAC(St02Screen *this, s32 arg, s32 angle);

/* St02Screen's draw: func_800D7BAC then func_800D7918 at the angles 0, 0x400, 0x800 and 0xC00. */
void St02Screen_draw(St02Screen *this, s32 arg) {
    s32 angle;

    for (angle = 0; angle < 0x1000; angle += 0x400) {
        func_800D7BAC(this, arg, angle);
        func_800D7918(this, arg, angle);
    }
}

INCLUDE_ASM("st02/nonmatchings/st02", func_800D7918);

INCLUDE_ASM("st02/nonmatchings/st02", func_800D7BAC);

/* Clips a quad (corners 0 and 2 on the left, 1 and 3 on the right) to x 0..0x1000, moving the texture's left
 * or right u to match; -1 when it is wholly outside. */
s32 func_800D7F38(SVECTOR *corners, u8 *left, u8 *right) {
    s32 x1 = corners[1].vx;
    s32 x0;

    if (x1 <= 0) {
        return -1;
    }
    x0 = corners[0].vx;
    if (x0 >= 0x1000) {
        return -1;
    }
    if (x0 < 0) {
        corners[2].vx = 0;
        corners[0].vx = 0;
        *left = *right - (x1 >> 4);
    } else if (x1 > 0x1000) {
        corners[3].vx = 0x1000;
        corners[1].vx = 0x1000;
        *right = *left + ((0x1000 - x0) >> 4);
    }
    return 0;
}

/* Constructs a St02MeshDrawer for a mesh and the Fight whose entity group will hold it (StageTurn_createNodeEntity). */
St02MeshDrawer *St02MeshDrawer_init(St02MeshDrawer *this, Mesh *mesh, Fight *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST02_MESH_DRAWER_VTABLE;
    this->mesh = mesh;
    this->owner = owner;
    return this;
}

INCLUDE_ASM("st02/nonmatchings/st02", St02MeshDrawer_draw);

/* The stage's destructor: Fight's. */
void StageTurn_destroy(StageTurn *this, s32 flags) {
    Fight_destroy(&this->base, flags);
}

/* The destructor of the entity class of vtable D_800D4004, which func_800D4FC8 installs. */
void func_800D8230(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St02Hit's destructor: CollisionOwner's, inlined. */
void St02Hit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs a St02Hit with its damage, knockback and kind (game's Hit_init). */
St02Hit *St02Hit_init(St02Hit *this, s32 damage, Vec3 *knockback, s32 kind) {
    s32 x, y, z;

    this->base.vtable = ST02_HIT_VTABLE;
    this->damage = damage;
    this->kind = kind;
    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
    return this;
}

/* Sets the hit's knockback. The match depends on reading the three words before storing any, as vecCopy does. */
void St02Hit_setKnockback(St02Hit *this, Vec3 *knockback) {
    s32 x = knockback->vx;
    s32 y = knockback->vy;
    s32 z = knockback->vz;

    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}

/* St02Block's destructor. */
void St02Block_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3F24, which func_800D6924 installs. */
void func_800D8344(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St02Terminal's destructor. */
void St02Terminal_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Closes the terminal, as st02_closeTerminal does (no caller or table names it; the type is
 * assumed from that). */
void func_800D83D4(St02Terminal *this) {
    this->closed = 1;
}

/* The destructor of the entity class of vtable D_800D3ED4, which func_800D73E8 installs. */
void func_800D83E0(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St02Screen's destructor. */
void St02Screen_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St02MeshDrawer's destructor. */
void St02MeshDrawer_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
