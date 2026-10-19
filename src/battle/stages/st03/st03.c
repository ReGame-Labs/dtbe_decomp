#include "common.h"
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/gfx/camera.h"
#include "engine/gfx/lights.h"
#include "engine/gfx/mesh_scene.h"
#include "engine/gfx/prim/alloc_dr_load.h"
#include "engine/gfx/prim/build_line_g2.h"
#include "engine/gfx/prim_buffer.h"
#include "engine/gfx/scene_graph.h"
#include "engine/gfx/sprite.h"
#include "engine/math/matrix.h"
#include "engine/math/vector.h"
#include "engine/math/sin_cos.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "engine/task/entity.h"

/* Arena 3's stage, "Shrine" in STAGE_NAMES (StageShrine_init, virtual table STAGE_SHRINE_VTABLE): a Fight whose
 * scenery has lifts, falling blocks, bolt sources and an animated palette. */
typedef struct StageShrine {
    /* 0x000 */ Fight base;
    /* 0xD38 */ MeshSceneLink collisionLink; /* lines of the arena itself, in COLLISION_LINKS (StageShrine_init) */
    /* 0xD50 */ s32 liftRaised; /* while set, the lifts move out (St03Lift_moveOffset); func_800D878C reads it */
    /* 0xD54 */ u8 unkD54[0xD58 - 0xD54];
    /* 0xD58 */ s32 brightness; /* 0x80, stepped down by 4 to 0x20 and back (StageShrine_tickScenery); the scenery
                                 * entities shade by it */
    /* 0xD5C */ u8 unkD5C[0xD60 - 0xD5C];
    /* 0xD60 */ Sprite sprite; /* the 0xA0 by 0xAE TIM image at (0x240, 0) of VRAM, dithered */
    /* 0xD78 */ Group group; /* drawn by St03GroupDrawer (st03_drawGroup) */
} StageShrine;

/* The entity that draws the stage's group (St03GroupDrawer_draw). */
typedef struct St03GroupDrawer {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageShrine *stage;
} St03GroupDrawer;

/* The stage's entity func_800D59C8 constructs: it shows and hides one node of
 * a mesh scene (meshSceneShowNode/meshSceneHideNode). */
typedef struct Unk800D59C8 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *meshScene;
    /* 0x1C */ s32 node;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ VECTOR position;    /* only x, y and z set */
    /* 0x34 */ VECTOR target; /* x, y and z of ST03_BOLT_POINTS[point] */
    /* 0x44 */ s32 point;     /* an index into ST03_BOLT_POINTS */
    /* 0x48 */ s32 unk48;
    /* 0x4C */ s32 unk4C;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 unk58;
} Unk800D59C8;

extern EntityVtable ST03_GROUP_DRAWER_VTABLE;

void func_800D4640(void *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable ST03_MESH_MOVER_VTABLE;

extern VtableEntry STAGE_SHRINE_VTABLE[];

/* An arena collision line as the stage defines it: its ends. */
typedef struct St03LineEnds {
    /* 0x0 */ DVECTOR from;
    /* 0x4 */ DVECTOR to;
} St03LineEnds;

extern St03LineEnds ST03_TERRAIN_LINE_ENDS[2];
extern TerrainLine ST03_TERRAIN_LINES[2];
extern RECT ST03_SPRITE_CLUT_RECT; /* the sprite's CLUT: 256 by 1 at (0, 0x1E0) */
extern RECT ST03_SPRITE_IMAGE_RECT; /* its pixels: 0xA0 by 0xAE at (0x240, 0) */

/* Constructs the stage: Fight, with virtual table STAGE_SHRINE_VTABLE, its sprite, its group, its two collision
 * lines (the walls at x -0x504 and 0x3FC), built from ST03_TERRAIN_LINE_ENDS and linked into COLLISION_LINKS, and the sprite's
 * image already in VRAM, dithered. */
StageShrine *StageShrine_init(StageShrine *this) {
    TIM_IMAGE tim;
    s32 i;

    Fight_init(&this->base);
    this->base.vtable = STAGE_SHRINE_VTABLE;
    this->collisionLink.link.next = &this->collisionLink.link;
    this->collisionLink.link.prev = &this->collisionLink.link;
    spriteInit(&this->sprite);
    groupInit(&this->group);
    for (i = 0; i < 2; i++) {
        initTerrainLine(&ST03_TERRAIN_LINES[i], &ST03_TERRAIN_LINE_ENDS[i].from, &ST03_TERRAIN_LINE_ENDS[i].to);
    }
    /* MeshSceneLink.lines is typed by the engine's own lines; the stage's are TerrainLines */
    addCollisionLink(&this->collisionLink, (s32 *)ST03_TERRAIN_LINES, 2);
    tim.mode = 9; /* 8-bit, with a CLUT */
    tim.crect = &ST03_SPRITE_CLUT_RECT;
    tim.prect = &ST03_SPRITE_IMAGE_RECT;
    spriteSetTim(&this->sprite, &tim, 0);
    this->sprite.flags |= SPRITE_DITHER;
    return this;
}

INCLUDE_ASM("st03/nonmatchings/st03", StageShrine_startRound);

INCLUDE_ASM("st03/nonmatchings/st03", StageShrine_createNodeEntity);

INCLUDE_ASM("st03/nonmatchings/st03", StageShrine_tickScenery);

/* The base's Fight_drawWorld, then the stage's own part (func_800D4640). */
void StageShrine_drawWorld(void *this) {
    Fight_drawWorld(this);
    func_800D4640(this);
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D4640);

/* Draws the stage's group as camera sees it, then marks it to be worked out again. */
void st03_drawGroup(StageShrine *this, Camera *camera) {
    sceneObjectDrawInView(&this->group.object, camera);
    groupInvalidate(&this->group);
}

/* The stage's animated palette (St03PaletteLoad_init, virtual table ST03_PALETTE_LOAD_VTABLE, made by
 * StageShrine_startRound): twenty colour words and one more, all cleared. */
typedef struct St03PaletteLoad {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18[20];
    /* 0x68 */ s32 unk68;
} St03PaletteLoad;

extern EntityVtable ST03_PALETTE_LOAD_VTABLE;

/* Constructs an St03PaletteLoad with everything cleared, the twenty words last first. */
St03PaletteLoad *St03PaletteLoad_init(St03PaletteLoad *this) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST03_PALETTE_LOAD_VTABLE;
    for (i = 19; i >= 0; i--) {
        this->unk18[i] = 0;
    }
    this->unk68 = 0;
    return this;
}

INCLUDE_ASM("st03/nonmatchings/st03", St03PaletteLoad_tick);

/* One VRAM upload of St03PaletteLoad_draw: the rectangle, the first halfword of the entity's colours (0x18) and
 * how many words to copy from there. */
typedef struct Unk800D3964 {
    /* 0x0 */ RECT *rect;
    /* 0x4 */ s32 start;
    /* 0x8 */ s32 words;
} Unk800D3964;

extern Unk800D3964 ST03_PALETTE_LOADS[6];

/* St03PaletteLoad's draw: one DR_LOAD per rectangle of ST03_PALETTE_LOADS, filled with its run of the colours at 0x18
 * and added to the camera's first-drawn slot. */
void St03PaletteLoad_draw(St03PaletteLoad *this, Camera *camera) {
    u_long *ot = camera->ot.firstDrawn;
    u32 colors = (u32)this->unk18;
    Unk800D3964 *load = ST03_PALETTE_LOADS;

    do {
        DR_LOAD *prim = allocDrLoad(load->rect);
        u_long *dst = prim->p;
        /* The match depends on adding the halfword offset to the colours' address as integers, in this order
         * (pointer arithmetic adds them the other way round). */
        u_long *src = (u_long *)(load->start * 2 + colors);
        u_long *end = src + load->words;

        do {
            *dst++ = *src++;
        } while (src != end);
        AddPrim(ot, prim);
        load++;
    } while (load != ST03_PALETTE_LOADS + 6);
}

/* Constructs a St03GroupDrawer for the stage. */
St03GroupDrawer *St03GroupDrawer_init(St03GroupDrawer *this, StageShrine *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST03_GROUP_DRAWER_VTABLE;
    this->stage = stage;
    return this;
}

/* St03GroupDrawer's draw: draws the stage's group (st03_drawGroup) unless bit 8 of its flags is set. */
void St03GroupDrawer_draw(St03GroupDrawer *this, Camera *camera) {
    if (!Fight_isContinueShown(&this->stage->base)) {
        st03_drawGroup(this->stage, camera);
    }
}

/* A hit of the stage (St03Hit_init): game.bin's Hit has the same offsets, but not the same class: its 0x4 is a
 * character, where St03DropBlock_init stores 0x80001000, and its constructor takes the vector after the words. */
typedef struct St03Hit {
    /* 0x00 */ CollisionOwner base;
    /* 0x04 */ s32 damage; /* HIT_DAMAGE_MASK, with flags above it */
    /* 0x08 */ s32 kind;
    /* 0x0C */ s32 withEffect; /* St03Hit_apply gives the hit fighter a func_800D80DC entity */
    /* 0x10 */ Vec3 knockback;
} St03Hit;

extern VtableEntry ST03_HIT_VTABLE[];

struct Unk800D80DC *func_800D80DC(struct Unk800D80DC *this, Fighter *fighter);

/* St03Hit's slot 0x08: when getHitFighter finds a fighter in sum, adds its damage (saturating at HIT_DAMAGE_MASK),
 * its kind and its knockback into sum, whose flags get its own; withEffect gives the fighter a new Unk800D80DC
 * (Fighter_addEntity). */
void St03Hit_apply(St03Hit *this, CollisionSphere *sum) {
    Entity *fighter = getHitFighter(sum);
    u32 flags;
    s32 count;
    s32 x, y, z;

    if (fighter == NULL) {
        return;
    }
    /* The match depends on the flags taken before the count, and the vector's three words read before any
     * is written (Hit_apply's shape). */
    flags = (this->damage | sum->hitDamage) & ~HIT_DAMAGE_MASK;
    count = (this->damage & HIT_DAMAGE_MASK) + (sum->hitDamage & HIT_DAMAGE_MASK);
    if (count > HIT_DAMAGE_MASK) {
        count = HIT_DAMAGE_MASK;
    }
    sum->hitDamage = flags | count;
    sum->hitKind += this->kind;
    x = sum->hitKnockback.vx;
    y = sum->hitKnockback.vy;
    z = sum->hitKnockback.vz;
    x += this->knockback.vx;
    y += this->knockback.vy;
    z += this->knockback.vz;
    sum->hitKnockback.vx = x;
    sum->hitKnockback.vy = y;
    sum->hitKnockback.vz = z;
    if (this->withEffect != 0) {
        Fighter_addEntity((Fighter *)fighter, NEW_ENTITY(0x3C, func_800D80DC, (Fighter *)fighter));
    }
}

/* The virtual table of St03Lift (ST03_LIFT_VTABLE): Entity's four entries, then the class's own method, which
 * moves the offset (St03Lift_moveOffset; St03LiftPart's St03LiftPart_moveOffset). */
typedef struct St03LiftVtable {
    /* 0x00 */ EntityVtable entity;
    /* 0x20 */ VtableEntry unk20; /* (this, stage) */
} St03LiftVtable;

/* The sound of a lift or a block that stops against something. */
#define ST03_SOUND_LAND 0x62

/* A lift (St03Lift_init, made by StageShrine_createNodeEntity and derived by St03LiftPart): one node of a mesh
 * scene, put at position moved by offset every tick. */
typedef struct St03Lift {
    /* 0x00 */ Entity entity;
    /* 0x18 */ MeshScene *meshScene;
    /* 0x1C */ VECTOR position;    /* only x, y and z set */
    /* 0x2C */ VECTOR offset; /* the translation added to position */
    /* 0x3C */ s32 node;
} St03Lift; /* size 0x40 */

extern St03LiftVtable ST03_LIFT_VTABLE;

/* The class St03LiftPart_init constructs on St03Lift_init's entity (virtual table ST03_LIFT_PART_VTABLE, and D_800D3C74
 * for its St03Hit at 0x40), with a part at 0x60. */
typedef struct St03LiftPart {
    /* 0x00 */ St03Lift base;
    /* 0x40 */ St03Hit unk40;
    /* 0x5C */ u8 unk5C[0x60 - 0x5C];
    /* 0x60 */ CollisionSphere part;
} St03LiftPart;

extern VtableEntry D_800D3C74[];
extern St03LiftVtable ST03_LIFT_PART_VTABLE;

/* Constructs an St03Lift for a node of a mesh scene and where it is, with no offset. */
St03Lift *St03Lift_init(St03Lift *this, MeshScene *meshScene, s32 node, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &ST03_LIFT_VTABLE.entity;
    this->meshScene = meshScene;
    this->node = node;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->offset.vx = 0;
    this->offset.vy = 0;
    this->offset.vz = 0;
    this->offset.pad = 0;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* Sets the mesh's matrix to no rotation at pos, out of date. The match depends on reading pos before writing
 * the matrix. */
static inline void meshSetPosition(Mesh *mesh, VECTOR *pos) {
    s32 x, y, z;

    mesh->object.transform.valid = 0;
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x;
    mesh->object.transform.local.m.t[1] = y;
    mesh->object.transform.local.m.t[2] = z;
}

/* St03Lift's tick: runs the class's own method (0x20) with the stage, then puts the node's mesh at position
 * (no rotation) and translates it by offset. */
void St03Lift_tick(St03Lift *this, StageShrine *stage) {
    St03LiftVtable *vtable = (St03LiftVtable *)this->entity.vtable;
    Mesh *mesh;

    vtable->unk20.func((u8 *)this + vtable->unk20.delta, stage);
    mesh = meshSceneGetMesh(this->meshScene, this->node);
    meshSetPosition(mesh, &this->position);
    matrixTranslate(&mesh->object.transform.local.m, this->offset.vx, this->offset.vy, this->offset.vz);
}

/* St03Lift's own method: while the stage's liftRaised is clear, moves offset y back up to 0 by 8 a call;
 * while it is set, down by 8 to -0xF0. */
void St03Lift_moveOffset(St03Lift *this, StageShrine *stage) {
    if (stage->liftRaised == 0) {
        this->offset.vy += 8;
        if (this->offset.vy >= 0) {
            this->offset.vy = 0;
        }
    } else {
        this->offset.vy -= 8;
        if (this->offset.vy <= -0xF0) {
            this->offset.vy = -0xF0;
        }
    }
}

/* Constructs an St03LiftPart: an St03Lift whose St03Hit (damage 0x80004000, kind 10, knockback 0x38E0 up) is the
 * owner of a sphere 0x62 under the lift's position. The match depends on the knockback and the centre being local
 * Vec3s copied by struct assignment (the game stores them on the stack too). */
St03LiftPart *St03LiftPart_init(St03LiftPart *this, MeshScene *meshScene, s32 node, Vec3 *pos) {
    s32 x, y, z;
    Vec3 knockback;
    Vec3 center;

    St03Lift_init(&this->base, meshScene, node, pos);
    /* St03Hit_init(&this->unk40, 0x80004000, &ZERO_VECTOR, 10, 0), inline (the vector read before any store) */
    this->unk40.base.vtable = ST03_HIT_VTABLE;
    this->unk40.damage = 0x80004000;
    this->unk40.kind = 10;
    this->unk40.withEffect = 0;
    x = ZERO_VECTOR.vx;
    y = ZERO_VECTOR.vy;
    z = ZERO_VECTOR.vz;
    this->unk40.knockback.vx = x;
    this->unk40.knockback.vy = y;
    this->unk40.knockback.vz = z;
    this->unk40.base.vtable = D_800D3C74;
    this->base.entity.vtable = &ST03_LIFT_PART_VTABLE.entity;
    CollisionSphere_init(&this->part, pos, 0, &this->unk40);
    knockback.vx = 0;
    knockback.vy = -0x38E0;
    knockback.vz = 0;
    this->unk40.knockback = knockback;
    this->part.entity = &this->base.entity;
    center.vx = this->base.position.vx;
    center.vy = this->base.position.vy - 0x62;
    center.vz = this->base.position.vz;
    this->part.position = center;
    return this;
}

void St03LiftPart_relink(St03LiftPart *this, StageShrine *stage);

/* listIsAlone(&this->part.node), reading the fields through this (the match depends on it) */
static inline s32 partIsAlone(St03LiftPart *this) {
    ListNode *node = &this->part.link;

    return this->part.link.next == node && node == this->part.link.prev;
}

/* listRemove(&this->part.node), in the access order the game's compiler used (the match depends on it) */
static inline void partRemove(St03LiftPart *this) {
    ListNode *node = &this->part.link;

    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
}

/* St03LiftPart's own method (0x20): while the stage's liftRaised is clear, moves offset y down to 0 by 8 a call;
 * while it is set, up by 8 to 0xF0. Between 9 and 0x7F the part follows it (and is put back in the stage's
 * lists by St03LiftPart_relink while liftRaised is clear); outside, the part leaves its list with
 * ST03_SOUND_LAND. The node shows unless the offset is 0. */
void St03LiftPart_moveOffset(St03LiftPart *this, StageShrine *stage) {
    s32 flag = stage->liftRaised;
    s32 y;

    if (flag == 0) {
        this->base.offset.vy -= 8;
        if (this->base.offset.vy <= 0) {
            this->base.offset.vy = 0;
        }
    } else {
        this->base.offset.vy += 8;
        if (this->base.offset.vy >= 0xF0) {
            this->base.offset.vy = 0xF0;
        }
    }
    y = this->base.offset.vy;
    if (y >= 9 && y < 0x80) {
        this->part.radius = y;
        if (flag == 0) {
            St03LiftPart_relink(this, stage);
        }
    } else if (!partIsAlone(this)) {
        partRemove(this);
        soundControlPlaySound(&SOUND_CONTROL, ST03_SOUND_LAND, 0);
    }
    if (this->base.offset.vy == 0) {
        meshSceneHideNode(this->base.meshScene, this->base.node);
    } else {
        meshSceneShowNode(this->base.meshScene, this->base.node);
    }
}

/* Puts the part back in the stage's lists by which characters stand on the node's matrix: the list at 0x228
 * when only the first does, 0x230 when only the second does, 0x238 when both do; none when neither does. */
void St03LiftPart_relink(St03LiftPart *this, StageShrine *stage) {
    MATRIX *matrix = meshSceneGetWorldMatrix(this->base.meshScene, this->base.node);
    s32 first = Fighter_isStandingOn(Fight_getFighter(&stage->base, 0), matrix);
    s32 second = Fighter_isStandingOn(Fight_getFighter(&stage->base, 1), matrix);

    partRemove(this);
    if (first != 0 && second == 0) {
        Fight_addSide1Shape(&stage->base, &this->part.link);
    } else if (first == 0 && second != 0) {
        Fight_addSide0Shape(&stage->base, &this->part.link);
    } else if (first != 0 && second != 0) {
        Fight_addStageShape(&stage->base, &this->part.link);
    }
}

/* Constructs a St03MeshMover for a mesh of the stage and where it is (StageShrine_createNodeEntity). */
StageMeshMover *St03MeshMover_init(StageMeshMover *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST03_MESH_MOVER_VTABLE;
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

/* getSin (engine/math/sin_cos.c), inline. */
static inline s16 getSinInline(s32 angle) {
    return COS_SIN_TABLE[angle & 0xFFF].sin;
}

/* St03MeshMover's tick: puts the mesh at position raised by motion (no rotation), then moves motion along a sine (128
 * frames a period). */
void St03MeshMover_tick(StageMeshMover *this) {
    /* stage_objects.h has position as a Vec3 and 0x28 apart, but the game copies the four words as a VECTOR */
    VECTOR pos = *(VECTOR *)&this->position;

    pos.vy += this->motion;
    meshSetPosition(this->mesh, &pos);
    /* The match depends on the sine read through getSin's inline (a sine read in the expression is narrowed to
     * an s16 shift, loaded with lhu). */
    this->motion = getSinInline(this->frame << 5) >> 7;
    this->frame++;
}

/* The entity func_800D5464 makes for a mesh of the stage (StageShrine_createNodeEntity), at position: 0x2C0, 0x40
 * and 0x80 at 0x1C-0x1F. */
typedef struct Unk800D5464 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E;
    /* 0x1F */ u8 unk1F;
    /* 0x20 */ VECTOR position;  /* its tick copies the four words, pad included */
    /* 0x30 */ s32 bob;     /* added to position.vy: the sine of 0x34, by 64 */
    /* 0x34 */ s32 counter;
} Unk800D5464; /* size 0x38 */

extern EntityVtable D_800D3C24;

/* Constructs an entity of D_800D3C24 for a mesh of the stage and where it is (StageShrine_createNodeEntity). */
Unk800D5464 *func_800D5464(Unk800D5464 *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3C24;
    this->mesh = mesh;
    this->unk1C = 0x2C0;
    this->unk1E = 0x40;
    this->unk1F = 0x80;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->bob = 0;
    this->counter = 0;
    return this;
}

/* The tick of func_800D5464's entity: puts the mesh at position raised by bob (no rotation), then moves bob along a
 * sine (128 steps) and 0x1C by 8. */
void func_800D54F0(Unk800D5464 *this) {
    VECTOR pos = this->position;

    pos.vy += this->bob;
    meshSetPosition(this->mesh, &pos);
    this->unk1C += 8;
    /* The match depends on getSin's inline, as in St03MeshMover_tick. */
    this->bob = getSinInline(this->counter << 5) >> 6;
    this->counter++;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5594);

/* The entity func_800D5804 makes for a mesh of the stage (StageShrine_createNodeEntity), at position (StageMeshMover's
 * layout): the mesh turned about x and spun about y, scaled up while the stage's liftRaised is 2 and down at 4
 * (func_800D5880). */
typedef struct Unk800D5804 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Mesh *mesh;
    /* 0x1C */ VECTOR position;
    /* 0x2C */ s32 scale; /* of the mesh: eases to 0x1000 at liftRaised 2, to 0 at 4 */
    /* 0x30 */ s32 angle; /* about y */
    /* 0x34 */ s32 spin;  /* angle's step: 0x10, up to 0x60 once scale passes 0xC00, down to 0 at liftRaised 4 */
} Unk800D5804; /* size 0x38 */

extern EntityVtable D_800D3BFC;

/* Constructs an entity of D_800D3BFC for a mesh of the stage and where it is (StageShrine_createNodeEntity). */
Unk800D5804 *func_800D5804(Unk800D5804 *this, Mesh *mesh, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BFC;
    this->mesh = mesh;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->scale = 0;
    this->angle = 0;
    this->spin = 0x10;
    return this;
}

/* func_800D5804's tick: the mesh at position, scaled by scale, turned 0x40 about x and angle about y; angle steps by
 * spin. At the stage's liftRaised 2 the scale eases an eighth of the way to 0x1000 and, past 0xC00, spin grows by 4 up
 * to 0x60; at 4 spin falls by 2 down to 0 and the scale shrinks by a sixteenth. */
void func_800D5880(Unk800D5804 *this, StageShrine *stage) {
    Mesh *mesh = this->mesh;
    MATRIX *matrix;
    VECTOR scale;

    /* The match depends on the scale going through a VECTOR: the game stores it on the stack and reads it back. */
    scale.vx = this->scale;
    scale.vy = this->scale;
    scale.vz = this->scale;
    meshSetPosition(mesh, &this->position);
    matrix = &mesh->object.transform.local.m;
    matrixScale(matrix, scale.vx, scale.vy, scale.vz);
    matrixRotateX(matrix, 0x40);
    matrixRotateY(matrix, this->angle);
    this->angle += this->spin;
    if (stage->liftRaised == 2) {
        this->scale += (ONE - this->scale) >> 3;
        if (this->scale > 0xC00) {
            this->spin += 4;
            if (this->spin >= 0x60) {
                this->spin = 0x60;
            }
        }
    } else if (stage->liftRaised == 4) {
        this->spin -= 2;
        this->scale += -this->scale >> 4;
        if (this->spin <= 0) {
            this->spin = 0;
        }
    }
}

extern EntityVtable D_800D3BD4;

/* the eight points of func_800D59C8's entities */
extern VECTOR ST03_BOLT_POINTS[8];

/* Constructs an entity of D_800D3BD4 for a node of a mesh scene (hidden) and where it is: nodes 5 and 6 start
 * at points 2 and 4 of ST03_BOLT_POINTS (0x58 1 and 2), the others at point 0. */
Unk800D59C8 *func_800D59C8(Unk800D59C8 *this, MeshScene *meshScene, s32 node, s32 unk20, Vec3 *pos) {
    s32 x, y, z;
    s32 tx, ty, tz;
    VECTOR *point;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3BD4;
    this->meshScene = meshScene;
    this->node = node;
    this->unk20 = unk20;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    switch (node) {
    case 0:
    default:
        this->point = 0;
        this->unk58 = 0;
        break;
    case 5:
        this->point = 2;
        this->unk58 = 1;
        break;
    case 6:
        this->point = 4;
        this->unk58 = 2;
        break;
    }
    point = &ST03_BOLT_POINTS[this->point];
    tx = point->vx;
    ty = point->vy;
    tz = point->vz;
    this->unk48 = 0;
    this->unk4C = 0;
    this->unk50 = 0;
    this->unk54 = 0;
    this->target.vx = tx;
    this->target.vy = ty;
    this->target.vz = tz;
    meshSceneHideNode(meshScene, this->node);
    return this;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D5ADC);

/* vecGetLength through an inline. The match depends on it: the inline's parameter is the one pseudo for &offset
 * that the loop keeps in a saved register (hypothesis: an inline length method in the original). */
static inline s32 vecLengthInline(VECTOR *v) {
    return vecGetLength(v);
}

/* Shows the node once no character is within reach of position (0xC0 plus the high half of the first word of the
 * character's record); does nothing while one is. */
void func_800D5E48(Unk800D59C8 *this) {
    Fight *stage = CURRENT_FIGHT;
    VECTOR offset;
    s32 x, y, z;
    s32 side;

    for (side = 0; side < 2; side++) {
        Fighter *fighter = Fight_getFighter(stage, side);
        s32 reach = Fighter_getParams(fighter)->height + 0xC0;

        func_8007A650(fighter, (Vec3 *)&offset);
        /* The match depends on reading the whole offset before subtracting (as vecSubInPlace). */
        x = offset.vx;
        y = offset.vy;
        z = offset.vz;
        x -= this->position.vx;
        y -= this->position.vy;
        z -= this->position.vz;
        offset.vx = x;
        offset.vy = y;
        offset.vz = z;
        if (vecLengthInline(&offset) < reach) {
            return;
        }
    }
    meshSceneShowNode(this->meshScene, this->node);
}

/* Hides the entity's node of its mesh scene. */
void func_800D5F2C(Unk800D59C8 *this) {
    meshSceneHideNode(this->meshScene, this->node);
}

/* A falling block (St03DropBlock_init, made by StageShrine_createNodeEntity for a node of a mesh scene): a St03Hit
 * at 0x18 and a CollisionSphere part at 0x38 for it, positions in 256ths, and a state (0xCC) its tick steps. */
typedef struct St03DropBlock {
    /* 0x00 */ Entity entity;
    /* 0x18 */ St03Hit unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ CollisionSphere part;
    /* 0x80 */ Mesh *mesh;
    /* 0x84 */ MeshScene *meshScene;
    /* 0x88 */ s32 node;
    /* 0x8C */ VECTOR position;  /* in 256ths */
    /* 0x9C */ VECTOR home; /* the position at construction, in 256ths */
    /* 0xAC */ Fighter *riders[2]; /* their weights add to the entity's */
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ s32 unkBC;  /* the speed St03DropBlock_push keeps */
    /* 0xC0 */ s32 unkC0;
    /* 0xC4 */ s32 unkC4;   /* a countdown of state 2 */
    /* 0xC8 */ s32 unkC8;   /* counts the ticks from a random start */
    /* 0xCC */ s32 state;
} St03DropBlock; /* size 0xD0 */

extern EntityVtable ST03_DROP_BLOCK_VTABLE;

/* Constructs an St03DropBlock for a node of a mesh scene, its mesh and where it is (the node's world translation),
 * in state 0. */
St03DropBlock *St03DropBlock_init(St03DropBlock *this, MeshScene *meshScene, s32 node, Mesh *mesh, Vec3 *pos) {
    s32 vx, vy, vz;
    s32 x, y;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &ST03_DROP_BLOCK_VTABLE;
    /* St03Hit_init(&this->unk18, 0x80001000, &ZERO_VECTOR, 6, 0), inline (the vector read before any store) */
    this->unk18.base.vtable = ST03_HIT_VTABLE;
    this->unk18.damage = 0x80001000;
    this->unk18.kind = 6;
    this->unk18.withEffect = 0;
    vx = ZERO_VECTOR.vx;
    vy = ZERO_VECTOR.vy;
    vz = ZERO_VECTOR.vz;
    this->unk18.knockback.vx = vx;
    this->unk18.knockback.vy = vy;
    this->unk18.knockback.vz = vz;
    CollisionSphere_init(&this->part, pos, 0x70, &this->unk18);
    this->part.entity = &this->entity;
    this->mesh = mesh;
    this->meshScene = meshScene;
    this->node = node;
    x = pos->vx;
    y = pos->vy;
    this->position.vz = 0;
    this->home.vz = 0;
    this->riders[0] = NULL;
    this->riders[1] = NULL;
    this->unkB4 = 0;
    this->unkB8 = 0;
    this->unkBC = 0;
    this->unkC0 = 0;
    this->unkC4 = 0;
    this->position.vx = x << 8;
    this->position.vy = y << 8;
    this->home.vx = x << 8;
    this->home.vy = y << 8;
    this->unkC8 = mersenneTwisterGenerate(&RANDOM);
    this->state = 0;
    return this;
}

/* listRemove(&this->part.node), in the access order the game's compiler used (as partRemove) */
static inline void unk800D5F58PartRemove(St03DropBlock *this) {
    ListNode *node = &this->part.link;

    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
}

/* Sets the mesh's matrix to no rotation at x and y of pos, given in 256ths (z 0), out of date: meshSetPosition's
 * order (pos read after the flag store). */
static inline void meshSetPosition256(Mesh *mesh, VECTOR *pos) {
    s32 x, y;

    mesh->object.transform.valid = 0;
    x = pos->vx;
    y = pos->vy;
    mesh->object.transform.local.words[0] = ONE;
    mesh->object.transform.local.words[1] = 0;
    mesh->object.transform.local.words[2] = ONE;
    mesh->object.transform.local.words[3] = 0;
    mesh->object.transform.local.words[4] = ONE;
    mesh->object.transform.local.m.t[0] = x >> 8;
    mesh->object.transform.local.m.t[1] = y >> 8;
    mesh->object.transform.local.m.t[2] = 0;
}

/* The destructor of St03DropBlock: puts the mesh back at home (no rotation), then destroys the part, the
 * St03Hit and the entity. */
void St03DropBlock_destroy(St03DropBlock *this, s32 flags) {
    this->entity.vtable = &ST03_DROP_BLOCK_VTABLE;
    meshSetPosition256(this->mesh, &this->home);
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    unk800D5F58PartRemove(this);
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

s32 func_800D62B4(St03DropBlock *this, StageShrine *stage);
s32 func_800D6440(St03DropBlock *this, s32 speed);
void func_800D6550(St03DropBlock *this, StageShrine *stage);
void func_800D66B4(St03DropBlock *this);
void func_800D6738(St03DropBlock *this, StageShrine *stage);

/* St03DropBlock's tick, by state: 0 moves by an eighth of the way from 0x100 above position to home until 0xC0
 * passes 8; 1 moves (func_800D6738) and at 0x71C (func_800D6440) leaves the part's list, waiting 0x1E; 2
 * counts that down and shows the node again (state 0) once func_800D62B4 said 0. */
void St03DropBlock_tick(St03DropBlock *this, StageShrine *stage) {
    s32 busy = func_800D62B4(this, stage);
    s32 top;

    switch (this->state) {
    case 0:
        this->unkBC = 0;
        top = this->position.vy + 0x100;
        func_800D6440(this, (this->home.vy - top) >> 3);
        if (this->unkC0 >= 9) {
            this->unkBC = 0;
            this->state++;
        }
        break;
    case 1:
        func_800D6738(this, stage);
        if (func_800D6440(this, 0x71C) != 0) {
            unk800D5F58PartRemove(this);
            this->unkB8 = 0;
            this->unkB4 = 0;
            this->unkC4 = 0x1E;
            this->state++;
        }
        break;
    case 2:
        func_800D6550(this, stage);
        this->unkC8 = 0;
        if (--this->unkC4 <= 0 && busy == 0) {
            meshSceneShowNode(this->meshScene, this->node);
            this->state = 0;
        }
        break;
    }
    func_800D66B4(this);
    this->unkC8++;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D62B4);

void St03DropBlock_push(St03DropBlock *this, s32 speed);
void func_800D6874(St03DropBlock *this);

/* Moves the entity by speed (St03DropBlock_push) and stops it: at home (returns whether it was going down), at
 * 0x40000 (hides the node; returns whether it was going up), or where its point (0x40 right, 0x10 down)
 * hits something on the way (func_800D6874, ST03_SOUND_LAND; returns whether it was going up). */
s32 func_800D6440(St03DropBlock *this, s32 speed) {
    /* VECTORs: the frame keeps them 0x10 apart, and castMapRay reads the first three words of each */
    VECTOR from;
    VECTOR to;

    from.vz = 0;
    from.vx = (this->position.vx >> 8) + 0x40;
    from.vy = (this->position.vy >> 8) - 0x10;
    St03DropBlock_push(this, speed);
    to.vz = 0;
    to.vx = (this->position.vx >> 8) + 0x40;
    to.vy = (this->position.vy >> 8) - 0x10;
    if (this->position.vy < this->home.vy) {
        this->position.vy = this->home.vy;
        this->unkBC = 0;
        return speed < 0;
    }
    if (this->position.vy > 0x40000) {
        this->position.vy = 0x40000;
        this->unkBC = 0;
        if (speed < 0) {
            return 0;
        }
        meshSceneHideNode(this->meshScene, this->node);
        return 1;
    }
    if (castMapRay((Vec3 *)&to, (Vec3 *)&from) != 0) {
        this->unkBC = 0;
        this->position.vy = to.vy << 8;
        func_800D6874(this);
        soundControlPlaySound(&SOUND_CONTROL, ST03_SOUND_LAND, 0);
        return speed >= 0;
    }
    return 0;
}

/* State 2's move: drops the entity by 0xF0 (in whole units) when nothing is in the way of its point, else
 * pushes it with 0x71C (St03DropBlock_push). */
void func_800D6550(St03DropBlock *this, StageShrine *stage) {
    /* VECTORs, as in func_800D6440 */
    VECTOR from;
    VECTOR to;
    s32 x = (this->position.vx >> 8) + 0x40;
    s32 y = this->position.vy >> 8;

    from.vx = x;
    from.vy = y - 0x10;
    from.vz = 0;
    to.vx = x;
    to.vy = y + 0xF0;
    to.vz = 0;
    if (castMapRay((Vec3 *)&to, (Vec3 *)&from) != 0) {
        this->unkBC = 0;
        this->position.vy = to.vy << 8;
    } else {
        St03DropBlock_push(this, 0x71C);
    }
}

/* Pushes the entity up or down by force: its mass is 0x20000 plus the riders' weights (0x4 of their record),
 * its speed kept at 0xBC. */
void St03DropBlock_push(St03DropBlock *this, s32 force) {
    /* VECTORs: the frame keeps them 0x10 apart, and func_80079C1C reads the first three words of each */
    VECTOR move;
    VECTOR velocity;
    VECTOR push;
    s32 mass = 0x20000;
    s32 i;

    for (i = 0; i < 2; i++) {
        if (this->riders[i] != NULL) {
            mass += Fighter_getParams(this->riders[i])->mass;
        }
    }
    velocity.vx = 0;
    velocity.vy = this->unkBC;
    velocity.vz = 0;
    push.vx = 0;
    push.vy = force;
    push.vz = 0;
    func_80079C1C((Vec3 *)&move, (Vec3 *)&velocity, (Vec3 *)&push, mass);
    this->unkBC = velocity.vy;
    this->position.vy += move.vy;
}

/* Puts the mesh at position (no rotation), then tilts it about x along a sine of the tick count (64 steps of
 * 0x40, a 32nd of the sine). */
void func_800D66B4(St03DropBlock *this) {
    Mesh *mesh = this->mesh;

    meshSetPosition256(mesh, &this->position);
    matrixRotateX(&mesh->object.transform.local.m, COS_SIN_TABLE[(this->unkC8 << 6) & 0xFFF].sin >> 5);
}

/* State 1's move: sets the part's vector at 0x34 (0x70 above the entity) and the St03Hit's (a random x
 * below 0x1000, y -0x38E0), counts down 0xB4 and 0xB8 and puts the part back in the stage's lists by which of
 * them were running: 0x238 when neither was, 0x230 when only 0xB4 was, 0x228 when only 0xB8 was; none when
 * both were. */
void func_800D6738(St03DropBlock *this, StageShrine *stage) {
    ListNode *node = &this->part.link;
    VECTOR v;
    s32 wait0, wait1;

    v.vz = 0;
    v.vx = this->position.vx >> 8;
    v.vy = (this->position.vy >> 8) + 0x70;
    this->part.position.vx = v.vx;
    this->part.position.vy = v.vy;
    this->part.position.vz = v.vz;
    v.vx = (s32)mersenneTwisterGenerate(&RANDOM) % 0x1000;
    v.vy = -0x38E0;
    v.vz = 0;
    this->unk18.knockback.vx = v.vx;
    this->unk18.knockback.vy = v.vy;
    this->unk18.knockback.vz = v.vz;
    wait0 = this->unkB4 > 0;
    wait1 = this->unkB8 > 0;
    if (this->unkB4 > 0) {
        this->unkB4--;
    }
    if (this->unkB8 > 0) {
        this->unkB8--;
    }
    unk800D5F58PartRemove(this);
    if (!wait0 && !wait1) {
        Fight_addStageShape(&stage->base, node);
    } else if (wait0 && !wait1) {
        Fight_addSide0Shape(&stage->base, node);
    } else if (!wait0 && wait1) {
        Fight_addSide1Shape(&stage->base, node);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6874);

/* The entity class func_800D69C4 constructs at a position (virtual table D_800D3B84); func_800D8B4C's
 * derives from it. 0x2C is 4 plus a 256th of how far the position is in front of z 0x1000. */
typedef struct Unk800D69C4 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Vec3 position;
    /* 0x24 */ u8 unk24[0x2C - 0x24];
    /* 0x2C */ s32 unk2C;     /* how far it moves left each tick */
    /* 0x30 */ u8 brightness; /* the stage's, each tick: the quad's grey */
} Unk800D69C4;

extern EntityVtable D_800D3B84;

extern EntityVtable D_800D3B5C;

/* Constructs an entity of D_800D3B84 at pos: it moves 4 plus a 256th of how far pos is in front of z 0x1000. */
Unk800D69C4 *func_800D69C4(Unk800D69C4 *this, Vec3 *pos) {
    s32 x, y, z;

    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3B84;
    /* The match depends on reading the whole position before writing it (as vecCopy). */
    x = pos->vx;
    y = pos->vy;
    z = pos->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->unk2C = ((0x1000 - pos->vz) >> 8) + 4;
    return this;
}

/* Unk800D69C4's tick: copies the stage's 0xD58 and moves x left by 0x2C, wrapping from -0x300 to 0x300. */
void func_800D6A38(Unk800D69C4 *this, StageShrine *stage) {
    s32 x = this->position.vx - this->unk2C;

    this->brightness = stage->brightness;
    this->position.vx = x;
    if (x <= -0x300) {
        this->position.vx = x + 0x600;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6A6C);

/* The tick of func_800D8B4C's entity: copies the stage's 0xD58 and moves x left by 0x2C; past -0x1800 it
 * wraps to the right at a new random depth (0x200 to 0x9FF), which sets the speed again. */
void func_800D6C94(Unk800D69C4 *this, StageShrine *stage) {
    s32 x = this->position.vx - this->unk2C;
    /* The match depends on the generator's address taken here, before the test. */
    MersenneTwister *random = &RANDOM;

    this->brightness = stage->brightness;
    this->position.vx = x;
    if (x <= -0x1800) {
        s32 z;

        this->position.vx = x + 0x3000;
        z = (mersenneTwisterGenerate(random) & 0x7FF) + 0x200;
        this->position.vz = z;
        this->unk2C = ((0x1000 - z) >> 8) + 4;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6D0C);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D6E38);

/* A jittering line through up to 8 points (three of them sit at 0x18 of func_800D7AC4's object, 0x14C apart):
 * St03JitterLine_layOut lays the targets out, St03JitterLine_move moves each point toward its target plus a random
 * offset, St03JitterLine_draw draws the segments between the points. */
typedef struct St03JitterLine {
    /* 0x000 */ VECTOR target[8];
    /* 0x080 */ VECTOR point[8];
    /* 0x100 */ s32 unk100[8];
    /* 0x120 */ u32 color[8]; /* r, g, b bytes; word-aligned (lw), so not a CVECTOR */
    /* 0x140 */ s32 count;
    /* 0x144 */ s32 jitter; /* the range of the random offset */
    /* 0x148 */ s32 unk148; /* scales the segments' width (func_800D70BC) */
} St03JitterLine; /* size 0x14C */

void func_800D70BC(St03JitterLine *this, Camera *camera, VECTOR *point, u32 *color, s32 unk100);
void St03JitterLine_layOut(St03JitterLine *this, VECTOR *from, VECTOR *to);
void St03JitterLine_fadeColor(St03JitterLine *this, u32 from, u32 to);

/* Moves each point a sixteenth of the way toward its target plus a random offset in x and y. */
void St03JitterLine_move(St03JitterLine *this) {
    MersenneTwister *random = &RANDOM;
    /* The match depends on taking point before target (point comes from $a0, target keeps this's copy). */
    VECTOR *point = this->point;
    VECTOR *target = this->target;
    s32 jitter = this->jitter;
    s32 count = this->count;
    s32 i;
    s32 dx;
    s32 dy;

    if (jitter <= 0) {
        jitter = 1;
    }
    for (i = 0; i < count; i++) {
        dx = (s32)mersenneTwisterGenerate(random) % jitter;
        dy = (s32)mersenneTwisterGenerate(random) % jitter;
        point->vx += (target->vx + dx - point->vx) >> 4;
        point->vy += (target->vy + dy - point->vy) >> 4;
        target++;
        point++;
    }
}

/* Draws the segments between consecutive points. */
void St03JitterLine_draw(St03JitterLine *this, Camera *camera) {
    VECTOR *point = this->point;
    s32 *unk100 = this->unk100;
    u32 *color = this->color;
    s32 segments = this->count - 1;
    s32 i = 0;

    /* The match depends on counting the segment before drawing it: the counter steps at the top of the loop. */
    while (i < segments) {
        i++;
        func_800D70BC(this, camera, point, color, *unk100);
        point++;
        unk100++;
        color++;
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D70BC);

/* Lays the targets out evenly from from to to, one per 128 units of length (3 to 8 of them); when their count
 * changes, the points jump straight to their targets. */
void St03JitterLine_layOut(St03JitterLine *this, VECTOR *from, VECTOR *to) {
    VECTOR delta;
    s32 x, y, z;
    s32 count;
    s32 i;

    x = to->vx;
    y = to->vy;
    z = to->vz;
    x -= from->vx;
    y -= from->vy;
    z -= from->vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    count = vecGetLength(&delta) / 128 + 1;
    if (count > 8) {
        count = 8;
    }
    if (count < 3) {
        count = 3;
    }
    for (i = 0; i < count; i++) {
        vecLerp(&this->target[i], from, to, (i << 12) / (count - 1));
    }
    if (this->count != count) {
        for (i = 0; i < count; i++) {
            /* The match depends on a three-word copy (Vec3): the loop then walks the points, not the targets. */
            *(Vec3 *)&this->point[i] = *(Vec3 *)&this->target[i];
        }
    }
    this->count = count;
}

/* Gives the inner points color and blacks out both ends. */
void St03JitterLine_setColor(St03JitterLine *this, u32 color) {
    s32 last = this->count - 1;
    s32 i;

    for (i = 1; i < last; i++) {
        this->color[i] = color;
    }
    this->color[last] = 0;
    this->color[0] = 0;
}

/* Fades the points' colors from from (first point) to to (last point); a color is r, g, b in its low three bytes. */
void St03JitterLine_fadeColor(St03JitterLine *this, u32 from, u32 to) {
    VECTOR fromRgb;
    VECTOR toRgb;
    VECTOR rgb;
    u8 *fromBytes = (u8 *)&from;
    u8 *toBytes = (u8 *)&to;
    s32 last;
    s32 i;

    fromRgb.vx = fromBytes[0];
    fromRgb.vy = fromBytes[1];
    fromRgb.vz = fromBytes[2];
    toRgb.vx = toBytes[0];
    toRgb.vy = toBytes[1];
    toRgb.vz = toBytes[2];
    last = this->count - 1;
    for (i = 0; i <= last; i++) {
        vecLerp(&rgb, &fromRgb, &toRgb, (i << 12) / last);
        this->color[i] = rgb.vx | (rgb.vy << 8) | (rgb.vz << 16);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7534);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7764);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7AC4);

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7B58);

/* The entity class func_800D7B58 constructs (virtual table D_800D3B0C): an St03Hit base at 0x18 and a sphere at
 * 0x38 (CollisionSphere_init's class) that follows a point drifting toward its target, as high above it as height and
 * as wide as life. */
typedef struct Unk800D7B58 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ St03Hit unk18;
    /* 0x34 */ u8 unk34[0x38 - 0x34];
    /* 0x38 */ CollisionSphere unk38;
    /* 0x80 */ s32 ticks;
    /* 0x84 */ VECTOR position;
    /* 0x94 */ VECTOR target;
    /* 0xA4 */ s32 unkA4; /* 0..3 at random each tick */
    /* 0xA8 */ s32 life;  /* the sphere's radius, 2 less per tick */
    /* 0xAC */ s32 height; /* closes on 0x300 by a 16th per tick; above 0x400 the sphere is left out */
    /* 0xB0 */ s32 timer;
} Unk800D7B58;

/* listRemove(&this->unk38.link), in the access order the game's compiler used (the match depends on it) */
static inline void unk800D7B58SphereRemove(Unk800D7B58 *this) {
    ListNode *link = &this->unk38.link;

    this->unk38.link.next->prev = this->unk38.link.prev;
    link->prev->next = this->unk38.link.next;
    this->unk38.link.next = link;
    this->unk38.link.prev = link;
}

/* Unk800D7B58's tick: the point drifts a 16th of the way to its target in x and z and height a 16th of the way to
 * 0x300; while height is above 0x400 the sphere is out of the lists, else it moves height above the point with radius
 * life and every eighth tick joins the stage's shapes (out of the lists the other ticks). The entity deletes itself
 * when its life or its timer runs out. */
void func_800D7D88(Unk800D7B58 *this, StageShrine *stage) {
    VECTOR center;
    ListNode *link;

    this->position.vx += (this->target.vx - this->position.vx) >> 4;
    this->position.vz += (this->target.vz - this->position.vz) >> 4;
    this->height += (0x300 - this->height) >> 4;
    this->unkA4 = mersenneTwisterGenerate(&RANDOM) & 3;
    if (this->height > 0x400) {
        unk800D7B58SphereRemove(this);
    } else {
        center = this->position;
        center.vy -= this->height;
        this->unk38.position.vx = center.vx;
        this->unk38.position.vy = center.vy;
        this->unk38.position.vz = center.vz;
        this->unk38.radius = this->life;
        /* The match depends on link taken here: the second remove shares it and is not merged with the first. */
        link = &this->unk38.link;
        this->ticks++;
        if ((this->ticks & 7) == 0) {
            Fight_addStageShape(&stage->base, link);
        } else {
            unk800D7B58SphereRemove(this);
        }
    }
    this->life -= 2;
    if (this->life <= 0 || --this->timer <= 0) {
        entityDelete(&this->entity);
    }
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D7F20);

extern EntityVtable D_800D3AE4;

/* The stage's entity of func_800D80DC (virtual table D_800D3AE4), which keeps a fighter's handle. */
typedef struct Unk800D80DC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 fighterHandle; /* the fighter's (entityGetHandle) */
    /* 0x1C */ Vec3 position; /* the character's, at construction */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ s32 scale;      /* the character's height (Fighter_getParams), the square's scale */
    /* 0x30 */ s32 frame;      /* 0, 1, 2: the picture's row */
    /* 0x34 */ s32 angle;      /* random each tick, the square's turn about z */
    /* 0x38 */ s32 brightness; /* 0xFF at construction, fades by 4 a tick */
} Unk800D80DC;

/* Constructs the stage's entity for a character: its handle, its position and a value of its record. */
Unk800D80DC *func_800D80DC(Unk800D80DC *this, Fighter *fighter) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3AE4;
    this->fighterHandle = entityGetHandle(&fighter->entity);
    func_8007A650(fighter, &this->position);
    this->scale = Fighter_getParams(fighter)->height;
    this->frame = 0;
    this->angle = 0;
    this->brightness = 0xFF;
    return this;
}

/* Follows the character: deletes itself once the character is gone or brightness has faded out (4 a frame);
 * meanwhile takes the character's position, a new random angle and steps frame through 0, 1, 2. */
void func_800D8158(Unk800D80DC *this) {
    Fighter *fighter = (Fighter *)getEntityByHandle(this->fighterHandle);

    if (fighter == NULL) {
        entityDelete(&this->entity);
        return;
    }
    func_8007A650(fighter, &this->position);
    this->angle = mersenneTwisterGenerate(&RANDOM);
    this->frame++;
    if (this->frame >= 3) {
        this->frame = 0;
    }
    this->brightness -= 4;
    if (this->brightness <= 0) {
        entityDelete(&this->entity);
    }
}

extern SVECTOR D_800D3A88[4];

/* Unk800D80DC's draw: the square of D_800D3A88 turned about z by angle and scaled by scale at the character's
 * position, with frame's 32-pixel row of the 32 by 32 picture at (0xA0, 0) of the texture page at (512, 256) and
 * the CLUT at (528, 504), semi-transparent in brightness grey. The match depends on the depth read first and the
 * ordering table after the matrix's translation, on the matrix passed through one pointer, and on the shape's fields
 * in this order with the uv bytes stored as chains. */
void func_800D8224(Unk800D80DC *this, s32 arg) {
    Camera *camera = (Camera *)arg;
    s32 depth = camera->zShift + 1;
    u_long *ot;
    MatrixWords m;
    MATRIX *matrix;
    PolyFT4Shape shape;
    Vec3 *position;
    s32 x, y, z;
    u8 v;

    matrix = &m.m;
    m.words[0] = ONE;
    m.words[1] = 0;
    m.words[2] = ONE;
    m.words[3] = 0;
    m.words[4] = ONE;
    position = &this->position;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    m.m.t[0] = x;
    m.m.t[1] = y;
    m.m.t[2] = z;
    ot = camera->ot.slots;
    matrixSetRotationZ(matrix, this->angle);
    matrixScale(matrix, this->scale, this->scale, this->scale);
    setModelMatrix(matrix);
    shape.corners[0] = &D_800D3A88[0];
    shape.corners[1] = &D_800D3A88[1];
    shape.corners[2] = &D_800D3A88[2];
    shape.corners[3] = &D_800D3A88[3];
    UV_BYTES(shape.uv0)[0] = UV_BYTES(shape.uv2)[0] = 0xA0;
    UV_BYTES(shape.uv1)[0] = UV_BYTES(shape.uv3)[0] = 0xBF;
    shape.clut = getClut(528, 504);
    shape.tpage = getTPage(0, 1, 512, 256);
    shape.flags = SPRITE_SEMI_TRANS | 1;
    shape.color = this->brightness | (this->brightness << 8) | (this->brightness << 16);
    v = this->frame << 5;
    UV_BYTES(shape.uv0)[1] = UV_BYTES(shape.uv1)[1] = v;
    UV_BYTES(shape.uv2)[1] = UV_BYTES(shape.uv3)[1] = v + 0x1F;
    PRIM_BUFFER_FREE = buildPolyFT4(&shape, PRIM_BUFFER_FREE, ot, depth);
}

/* The entity class func_800D8374 constructs (virtual table D_800D3ABC). */
typedef struct Unk800D8374 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ u32 colors[9][7]; /* greys, by row and column: 0x80 at column 0 down to the stage's brightness */
    /* 0x114 */ s32 unk114;
    /* 0x118 */ Vec3 unk118;
} Unk800D8374;

extern EntityVtable D_800D3ABC;

/* Constructs an entity of the class of virtual table D_800D3ABC: 0x114 0x200, the vector at 0x118 zero. */
Unk800D8374 *func_800D8374(Unk800D8374 *this) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_800D3ABC;
    this->unk114 = 0x200;
    this->unk118.vx = 0;
    this->unk118.vy = 0;
    this->unk118.vz = 0;
    return this;
}

/* Unk800D8374's tick: each row of colors fades from grey 0x80 to the stage's brightness across its columns; unk114
 * steps and unk118 adds its sine and cosine to vx and vy, and 1 to vz. */
void func_800D83C0(Unk800D8374 *this, StageShrine *stage) {
    s32 row;
    s32 column;
    s32 level;
    s32 fade = 0x80 - stage->brightness;

    for (row = 0; row < 9; row++) {
        for (column = 0; column < 7; column++) {
            level = 0x80 - column * fade / 6;
            this->colors[row][column] = level | (level << 8) | (level << 16);
        }
    }
    this->unk114++;
    this->unk118.vx += COS_SIN_TABLE[this->unk114 & 0xFFF].sin;
    this->unk118.vy += COS_SIN_TABLE[this->unk114 & 0xFFF].cos;
    this->unk118.vz++;
}

INCLUDE_ASM("st03/nonmatchings/st03", func_800D849C);

/* The stage's destructor: the group, the sprite, unlinks its collision link, then Fight's destructor. */
void StageShrine_destroy(StageShrine *this, s32 flags) {
    ListNode *node;

    groupDestroy(&this->group, DESTROY_BASES);
    spriteDestroy(&this->sprite, DESTROY_BASES);
    /* listRemove(&this->collisionLink.link), in the access order the game's compiler used */
    node = &this->collisionLink.link;
    this->collisionLink.link.next->prev = this->collisionLink.link.prev;
    node->prev->next = this->collisionLink.link.next;
    this->collisionLink.link.next = node;
    this->collisionLink.link.prev = node;
    Fight_destroy(&this->base, flags);
}

/* Returns the stage's liftRaised. */
s32 func_800D878C(StageShrine *this) {
    return this->liftRaised;
}

/* St03PaletteLoad's destructor. */
void St03PaletteLoad_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St03GroupDrawer's destructor. */
void St03GroupDrawer_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St03Hit's destructor: CollisionOwner's, inlined. */
void St03Hit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs a St03Hit with its damage, knockback, kind and withEffect.
 * The match depends on reading the vector's three words before storing any, as vecCopy does. */
St03Hit *St03Hit_init(St03Hit *this, s32 damage, Vec3 *knockback, s32 kind, s32 withEffect) {
    s32 x, y, z;

    this->base.vtable = ST03_HIT_VTABLE;
    this->damage = damage;
    this->kind = kind;
    this->withEffect = withEffect;
    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
    return this;
}

/* Sets the hit's knockback, reading all three components before writing any (game.bin's Hit_setKnockback is the
 * same function). */
void St03Hit_setKnockback(St03Hit *this, VECTOR *knockback) {
    s32 x, y, z;

    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}

/* St03Lift's destructor. */
void St03Lift_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of St03LiftPart (virtual tables ST03_LIFT_PART_VTABLE and D_800D3C74). */
void St03LiftPart_destroy(St03LiftPart *this, s32 flags) {
    this->unk40.base.vtable = D_800D3C74;
    this->base.entity.vtable = &ST03_LIFT_PART_VTABLE.entity;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    partRemove(this);
    this->unk40.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->base.entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St03MeshMover's destructor. */
void St03MeshMover_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3C24, which func_800D5464 installs. */
void func_800D89E4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3BFC, which func_800D5804 installs. */
void func_800D8A2C(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3BD4, which func_800D59C8 installs. */
void func_800D8A74(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B84, which func_800D69C4 installs. */
void func_800D8ABC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3B5C, which StageShrine_startRound and func_800D8B4C install. */
void func_800D8B04(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Constructs func_800D69C4's entity at pos with virtual table D_800D3B5C, 0x2C doubled. */
Unk800D69C4 *func_800D8B4C(Unk800D69C4 *this, Vec3 *pos) {
    func_800D69C4(this, pos);
    this->entity.vtable = &D_800D3B5C;
    this->unk2C *= 2;
    return this;
}

/* St03JitterLine_layOut from point to the last point. */
void func_800D8B8C(St03JitterLine *this, VECTOR *point) {
    St03JitterLine_layOut(this, point, &this->point[this->count - 1]);
}

/* St03JitterLine_layOut from the last point to point. */
void func_800D8BBC(St03JitterLine *this, VECTOR *point) {
    St03JitterLine_layOut(this, &this->point[this->count - 1], point);
}

/* St03JitterLine_fadeColor from color to the last point's color. */
void func_800D8BEC(St03JitterLine *this, u32 color) {
    St03JitterLine_fadeColor(this, color, this->color[this->count - 1]);
}

/* St03JitterLine_fadeColor from the last point's color to color. */
void func_800D8C24(St03JitterLine *this, u32 color) {
    St03JitterLine_fadeColor(this, this->color[this->count - 1], color);
}

/* Sets the range of the points' random offset. */
void func_800D8C5C(St03JitterLine *this, s32 jitter) {
    this->jitter = jitter;
}

/* Sets unk148. */
void func_800D8C64(St03JitterLine *this, s32 unk148) {
    this->unk148 = unk148;
}

/* Returns this: an empty constructor of St03JitterLine (hypothesis: it follows its methods; nothing calls it). */
St03JitterLine *func_800D8C6C(St03JitterLine *this) {
    return this;
}

/* The entity func_800D7534 constructs (virtual table D_800D3B34: func_800D7764, func_800D7AC4 and
 * func_800D8C74): four jittering lines (func_800D7AC4 draws all four), a St03Hit (virtual table
 * ST03_HIT_VTABLE) and two parts in its list. */
typedef struct Unk800D7534 {
    /* 0x000 */ Entity entity;
    /* 0x018 */ St03JitterLine lines[4];
    /* 0x548 */ St03Hit unk548;
    /* 0x564 */ u8 unk564[4];
    /* 0x568 */ CollisionSphere unk568;
    /* 0x5B0 */ CollisionSphere unk5B0;
} Unk800D7534; /* size 0x5F8 */

/* The destructor of Unk800D7534: its parts' (last first) and its St03Hit's, inlined. */
void func_800D8C74(Unk800D7534 *this, s32 flags) {
    /* Each listRemove written out in the access order the game's compiler used (as partRemove), with a node
     * of its own (one node assigned twice gives other registers). */
    this->unk5B0.vtable = COLLISION_SHAPE_VTABLE;
    {
        ListNode *node = &this->unk5B0.link;

        this->unk5B0.link.next->prev = this->unk5B0.link.prev;
        node->prev->next = this->unk5B0.link.next;
        this->unk5B0.link.next = node;
        this->unk5B0.link.prev = node;
    }
    this->unk568.vtable = COLLISION_SHAPE_VTABLE;
    {
        ListNode *node = &this->unk568.link;

        this->unk568.link.next->prev = this->unk568.link.prev;
        node->prev->next = this->unk568.link.next;
        this->unk568.link.next = node;
        this->unk568.link.prev = node;
    }
    this->unk548.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Unk800D7B58's destructor: its member's and its base's inlined, then entityDestroy. */
void func_800D8D24(Unk800D7B58 *this, s32 flags) {
    this->unk38.vtable = COLLISION_SHAPE_VTABLE;
    unk800D7B58SphereRemove(this);
    this->unk18.base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3AE4, which func_800D80DC installs. */
void func_800D8DAC(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_800D3ABC, which func_800D8374 installs. */
void func_800D8DF4(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}
