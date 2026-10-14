#include "common.h"
#include <abs.h>
#include "battle/game.h"
#include "battle/game_classes.h"
#include "battle/stage_objects.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"
#include "psyq.h"
#include "engine/math/sin_cos.h"
#include "engine/math/vector.h"
#include "engine/task/entity.h"
#include "engine/gfx/scene_graph.h"

/* Arena 0's stage, "Machine" in STAGE_NAMES (StageMachine_init, virtual table STAGE_MACHINE_VTABLE): a Fight
 * whose scenery animates the map's nodes, its CLUTs and its textures (StageMachine_tickScenery). */
typedef struct StageMachine {
    /* 0x000 */ Fight base;
    /* 0xD38 */ u8 unkD38[0xD3C - 0xD38];
    /* 0xD3C */ s32 frames; /* the scenery's frames (StageMachine_tickScenery) */
    /* 0xD40 */ u8 unkD40[0xD4C - 0xD40];
    /* 0xD4C */ u32 sound; /* a sound's handle, stopped before the next round */
    /* 0xD50 */ ListNode unkD50; /* a list node the destructor unlinks */
    /* 0xD58 */ u8 unkD58[0xDA8 - 0xD58];
    /* 0xDA8 */ RECT clutRect; /* 16 by 1 pixels at (0x300, 0xFE): a CLUT */
    /* 0xDB0 */ u8 unkDB0[0xE90 - 0xDB0];
    /* 0xE90 */ u16 clut[16]; /* that CLUT, read back from VRAM by st00_readClut */
    /* 0xEB0 */ u8 unkEB0[0xEC4 - 0xEB0];
    /* 0xEC4 */ MeshScene *meshScene; /* the stage's meshes */
    /* 0xEC8 */ u32 angle; /* 0..0xFFF, stepped by st00_turnAngle */
    /* 0xECC */ s32 turnFrames; /* the frames st00_turnAngle has run since func_800D5A68 cleared it */
    /* 0xED0 */ u8 unkED0[0xEF4 - 0xED0];
    /* 0xEF4 */ s16 unkEF4;
    /* 0xEF6 */ u8 unkEF6[0xF08 - 0xEF6];
    /* The four falling objects (st00_resetFallingObject resets one, func_800D70E4 drops it): */
    /* 0xF08 */ s32 unkF08[4];
    /* 0xF18 */ s32 unkF18[4];
    /* 0xF28 */ s16 unkF28[4];
    /* 0xF30 */ VECTOR position[4];
    /* 0xF70 */ VECTOR force[4];    /* func_80079C1C's force: gravity 0x71C, cleared after each step */
    /* 0xFB0 */ VECTOR velocity[4]; /* func_80079C1C's velocity, cleared on landing */
    /* 0xFF0 */ s32 unkFF0[4];
    /* 0x1000 */ s32 groundFrames[4]; /* the frames on the ground, up to 10 */
    /* 0x1010 */ s32 inAir[4];        /* set while func_80072FE0 finds no ground under it */
    /* 0x1020 */ u32 unk1020[4]; /* func_800D5A34 counts the non-zero ones */
    /* 0x1030 */ s32 unk1030;
    /* 0x1034 */ s32 unk1034[4]; /* func_800D59CC fills them with (r + i) & 3 */
    /* 0x1044 */ s32 unk1044;
    /* 0x1048 */ s16 unk1048;
    /* 0x104A */ u8 unk104A[0x1050 - 0x104A];
    /* 0x1050 */ s32 unk1050; /* while set, St00SignalQuads turn red */
    /* 0x1054 */ u8 unk1054[0x1060 - 0x1054];
    /* 0x1060 */ Group group;      /* node 28's mesh, drawn by St00GroupDrawer */
    /* 0x10C0 */ s32 continueShown; /* Fight_isContinueShown: the group is not drawn, the scenery stops */
} StageMachine;

void func_800D9B9C(void *this);
St00GroupDrawer *St00GroupDrawer_init(St00GroupDrawer *this, Fight *owner, MeshScene *meshScene, s32 arg3);
void func_800D4BB0(StageMachine *this);

/* the virtual tables the functions below install, and the data they read */
extern EntityVtable ST00_SLIDING_SPRITES_VTABLE;
extern EntityVtable ST00_CROSSING_SPRITES_VTABLE;
extern EntityVtable ST00_COLOR_QUAD_VTABLE;
extern EntityVtable ST00_SIGNAL_QUADS_VTABLE;
extern EntityVtable D_800D39C4;
extern EntityVtable ST00_GLOW_QUAD_VTABLE;
extern EntityVtable D_800D3A14;
extern EntityVtable D_800D3A3C;
extern EntityVtable ST00_GROUP_DRAWER_VTABLE;
extern VtableEntry STAGE_HIT_VTABLE[];
extern VtableEntry STAGE_MACHINE_VTABLE[];

INCLUDE_ASM("st00/nonmatchings/st00", StageMachine_init);

/* The stage's destructor: its virtual table, the group, unlinks the node at 0xD50, then Fight's destructor. */
void StageMachine_destroy(StageMachine *this, s32 flags) {
    ListNode *node;

    this->base.vtable = STAGE_MACHINE_VTABLE;
    groupDestroy(&this->group, DESTROY_BASES);
    /* listRemove(&this->unkD50), in the access order the game's compiler used */
    node = &this->unkD50;
    this->unkD50.next->prev = this->unkD50.prev;
    node->prev->next = this->unkD50.next;
    this->unkD50.next = node;
    this->unkD50.prev = node;
    Fight_destroy(&this->base, flags);
}

INCLUDE_ASM("st00/nonmatchings/st00", st00_resetRound);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3D94);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D3E80);

/* Reads the 16-colour CLUT at (0x300, 0xFE) back from VRAM into clut. */
void st00_readClut(StageMachine *this) {
    this->clutRect.x = 0x300;
    this->clutRect.y = 0xFE;
    this->unkEF4 = 0;
    this->clutRect.w = 16;
    this->clutRect.h = 1;
    StoreImage(&this->clutRect, (u_long *)this->clut);
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D40C0);

INCLUDE_ASM("st00/nonmatchings/st00", StageMachine_startRound);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D468C);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D47C8);

/* One frame of the scenery: Fight_tickScenery; while the continue prompt is not shown and (func_8006F144 is false
 * or the round is over), runs the animations (func_800D4BB0) and counts the frame. */
void StageMachine_tickScenery(StageMachine *this) {
    Fight_tickScenery();
    if (Fight_isContinueShown(&this->base)) {
        this->continueShown = 1;
        return;
    }
    this->continueShown = 0;
    if (func_8006F144(&this->base) && !Fight_isRoundOver(&this->base)) {
        return;
    }
    func_800D4BB0(this);
    this->frames++;
}

/* Before the next round: stops the stage's sound. */
void StageMachine_prepareNextRound(StageMachine *this) {
    stopSound(this->sound);
}

/* Moves the mesh of node 28 out of the stage's meshes into its group, and creates the St00GroupDrawer that draws
 * that group, put into the entity group of this. */
void st00_createGroupDrawer(StageMachine *this) {
    Mesh *mesh = this->meshScene->meshes[28];
    Entity *entity;

    listRemove(&mesh->object.link);
    groupAddChild(&this->group, &mesh->object);
    entity = NEW_ENTITY(sizeof(St00GroupDrawer), St00GroupDrawer_init, &this->base, this->meshScene, 0x1C);
    if (entity != NULL) {
        Fight_addEntity(&this->base, entity);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D4BB0);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D4DC4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D508C);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D5154);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D55B4);

/* Fills 0x1034 with four consecutive values (mod 4) from a random start, clears 0x1044 and 0x1030, and
 * sets 0x1048 to a random bit. */
void func_800D59CC(StageMachine *this) {
    u16 value = Fight_random(&this->base) & 3;
    s32 i;

    for (i = 0; i < 4; i++) {
        this->unk1034[i] = value++ & 3;
    }
    this->unk1044 = 0;
    this->unk1030 = 0;
    this->unk1048 = Fight_random(&this->base) & 1;
}

/* Returns how many of the four words at 0x1020 are set. */
s32 func_800D5A34(StageMachine *this) {
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

/* Turns the stage's angle by 0x40. */
void st00_turnAngle(StageMachine *this) {
    this->angle = (this->angle + 0x40) & 0xFFF;
}

/* vector_ops.c's vecClear, inlined: the match depends on clearing through a pointer. */
static inline VECTOR *clearVector(VECTOR *v) {
    v->vx = 0;
    v->vy = 0;
    v->vz = 0;
    v->pad = 0;
    return v;
}

/* Puts falling object `index` back at its start, high over the arena's left side, at rest. */
void st00_resetFallingObject(StageMachine *this, s32 index) {
    this->unkF08[index] = 0;
    clearVector(&this->force[index]);
    clearVector(&this->velocity[index]);
    this->unkF28[index] = 0;
    this->groundFrames[index] = 0;
    this->inAir[index] = 0;
    this->position[index].vx = -0xC80;
    this->position[index].vy = -0xA40;
    this->position[index].vz = 0;
    this->position[index].pad = 0;
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D70E4);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D76FC);

/* Constructs a St00GroupDrawer for the Fight whose entity group will hold it, and the stage's MeshScene
 * (st00_createGroupDrawer, which also passes 0x1C, unused). */
St00GroupDrawer *St00GroupDrawer_init(St00GroupDrawer *this, Fight *owner, MeshScene *meshScene, s32 arg3) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST00_GROUP_DRAWER_VTABLE;
    this->owner = owner;
    this->meshScene = meshScene;
    return this;
}

/* St00GroupDrawer's destructor. */
void St00GroupDrawer_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_GROUP_DRAWER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St00GroupDrawer's draw: unless the continue prompt is shown, draws the stage's group in view of camera and marks
 * its matrices out of date. */
void St00GroupDrawer_draw(St00GroupDrawer *this, Camera *camera) {
    if (((StageMachine *)this->owner)->continueShown == 0) {
        sceneObjectDrawInView(&((StageMachine *)this->owner)->group.object, camera);
        groupInvalidate(&((StageMachine *)this->owner)->group);
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

/* A screen-wide translucent tile and a 1024-wide quad of the owner, from height -0xA8E to -0x68E, that glow
 * while the stage's angle turns (St00GlowQuad_tick). */
typedef struct St00GlowQuad {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *owner;
    /* 0x1C */ u8 brightness; /* the tile's red, and the quad's (half of it in green and blue) */
    /* 0x1D */ u8 unk1D[0x40 - 0x1D];
    /* 0x40 */ SVECTOR corners[4];
} St00GlowQuad; /* size 0x60 */

/* Constructs the stage's quad entity of owner: corners from (-0x200, -0xA8E) to (0x200, -0x68E). */
St00GlowQuad *St00GlowQuad_init(St00GlowQuad *this, Fight *owner) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST00_GLOW_QUAD_VTABLE;
    this->owner = owner;
    /* The match depends on these chained assignments, as in st07's BonusThrower_init. */
    this->corners[0].vx = this->corners[2].vx = -0x200;
    this->corners[1].vx = this->corners[3].vx = 0x200;
    this->corners[0].vy = this->corners[1].vy = -0xA8E;
    this->corners[2].vy = this->corners[3].vy = -0x68E;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = 0;
    this->brightness = 0;
    return this;
}

/* St00GlowQuad's destructor. */
void St00GlowQuad_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_GLOW_QUAD_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St00GlowQuad's tick: while the stage's angle turns, the brightness follows the sine of that angle (0 to 64);
 * otherwise it is dark. */
void St00GlowQuad_tick(St00GlowQuad *this) {
    StageMachine *stage = (StageMachine *)this->owner;

    if (stage->turnFrames != 0) {
        this->brightness = ((COS_SIN_TABLE[stage->angle & 0xFFF].sin >> 5) + 0x80) >> 2;
    } else {
        this->brightness = 0;
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", St00GlowQuad_draw);

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

/* Two quads side by side at z -0x400 (StageMachine_startRound makes it), teal while the stage's 0x1050 is clear
 * and red while it is set. */
typedef struct St00SignalQuads {
    /* 0x00 */ Entity entity;
    /* 0x18 */ StageMachine *stage;
    /* 0x1C */ s32 red; /* the color below is the red one */
    /* 0x20 */ u32 color;
    /* 0x24 */ SVECTOR corners[8]; /* two quads of four */
} St00SignalQuads;

/* Constructs the St00SignalQuads: x from -0x540 to -0x240 and from 0x240 to 0x540, y from 0x60 to
 * 0x360, z -0x400. */
St00SignalQuads *St00SignalQuads_init(St00SignalQuads *this, StageMachine *stage) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST00_SIGNAL_QUADS_VTABLE;
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

/* St00SignalQuads's destructor. */
void St00SignalQuads_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_SIGNAL_QUADS_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* St00SignalQuads's tick: follows the stage's 0x1050: red (0x101080) while it is set, teal (0x808010) otherwise. */
void St00SignalQuads_tick(St00SignalQuads *this) {
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

INCLUDE_ASM("st00/nonmatchings/st00", St00SignalQuads_draw);

/* A 512 by 512 quad at depth -0x200, color 0x208080 (nothing calls St00ColorQuad_init). */
typedef struct St00ColorQuad {
    /* 0x00 */ Entity entity;
    /* 0x18 */ void *unk18;
    /* 0x1C */ s16 unk1C;
    /* 0x1E */ u8 unk1E[0x20 - 0x1E];
    /* 0x20 */ u32 color;
    /* 0x24 */ SVECTOR corners[4];
} St00ColorQuad;

/* Constructs the St00ColorQuad: corners from (-0x100, -0x100) to (0x100, 0x100) at z -0x200. */
St00ColorQuad *St00ColorQuad_init(St00ColorQuad *this, void *arg) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &ST00_COLOR_QUAD_VTABLE;
    this->unk18 = arg;
    this->unk1C = 0;
    /* The match depends on these chained assignments, as in st07's BonusThrower_init. */
    this->corners[0].vx = this->corners[2].vx = -0x100;
    this->corners[1].vx = this->corners[3].vx = 0x100;
    this->corners[0].vy = this->corners[1].vy = -0x100;
    this->corners[2].vy = this->corners[3].vy = 0x100;
    this->corners[0].vz = this->corners[1].vz = this->corners[2].vz = this->corners[3].vz = -0x200;
    this->color = 0x208080;
    return this;
}

/* St00ColorQuad's destructor. */
void St00ColorQuad_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_COLOR_QUAD_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", St00ColorQuad_tick);

INCLUDE_ASM("st00/nonmatchings/st00", St00ColorQuad_draw);

INCLUDE_ASM("st00/nonmatchings/st00", St00CrossingSprites_init);

/* St00CrossingSprites's destructor. */
void St00CrossingSprites_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_CROSSING_SPRITES_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", St00CrossingSprites_tick);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9128);

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9234);

INCLUDE_ASM("st00/nonmatchings/st00", St00CrossingSprites_draw);

/* Up to 16 sprites that slide out from x = 0x880 (even) or -0x880 (odd), brightest at x = +-0xA80. */
typedef struct St00SlidingSprites {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 unk18; /* St00SlidingSprites_init's second argument */
    /* 0x1C */ u16 frame;
    /* 0x1E */ u16 count; /* sprites moving: 2 more every 16 frames, up to 16 */
    /* 0x20 */ u16 speed[16];
    /* 0x40 */ u8 brightness[16];
    /* 0x50 */ u16 unk50[16]; /* the sprite stays still when set */
    /* 0x70 */ VECTOR position[16];
} St00SlidingSprites;

/* Whether sprite i is an even one (they move right).
 * The match depends on this inline: its store-flag gives the game's xori/andi test. */
static inline s32 isEven(s32 i) {
    return (i & 1) == 0;
}

/* Constructs the St00SlidingSprites: even ones at x 0x880, odd ones at -0x880, all at y -0x900, still. */
St00SlidingSprites *St00SlidingSprites_init(St00SlidingSprites *this, s32 arg) {
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ST00_SLIDING_SPRITES_VTABLE;
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

/* St00SlidingSprites's destructor. */
void St00SlidingSprites_destroy(Entity *this, s32 flags) {
    this->vtable = &ST00_SLIDING_SPRITES_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}


/* St00SlidingSprites's tick: every 16 frames two more sprites move; each speeds up, outwards, its brightness
 * falling with the distance to +-0xA80, and starts again from +-0x880 past it. */
void St00SlidingSprites_tick(St00SlidingSprites *this, s32 arg) {
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

INCLUDE_ASM("st00/nonmatchings/st00", St00SlidingSprites_draw);

/* Constructs a StageHit with damage and the knockback (0x40, 0, 0). */
StageHit *StageHit_init(StageHit *this, s32 damage) {
    Vec3 vec;

    /* The match depends on vec, a whole vector copied in (its stores stay on the stack), and on filling it
     * after the vtable store. */
    this->vtable = STAGE_HIT_VTABLE;
    vec.vx = 0x40;
    vec.vy = 0;
    vec.vz = 0;
    this->damage = damage;
    this->knockback = vec;
    return this;
}

/* StageHit's slot 0x08 (collideShapes calls it with the sphere its own touched): adds its damage and knockback
 * into the sphere's. */
void StageHit_apply(StageHit *this, CollisionSphere *part) {
    s32 x, y, z;

    part->hitDamage += this->damage;
    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = part->hitKnockback.vx;
    y = part->hitKnockback.vy;
    z = part->hitKnockback.vz;
    x += this->knockback.vx;
    y += this->knockback.vy;
    z += this->knockback.vz;
    part->hitKnockback.vx = x;
    part->hitKnockback.vy = y;
    part->hitKnockback.vz = z;
}

/* The entity class func_800D99F0 constructs (virtual table D_800D3A8C): an
 * StageHit part at 0x44 and a CollisionSphere member at 0x5C. */
typedef struct Unk800D99F0 {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *list;
    /* 0x1C */ u32 unk1C;
    /* 0x20 */ Vec3 position;
    /* 0x2C */ u8 unk2C[0x30 - 0x2C];
    /* 0x30 */ Vec3 velocity;
    /* 0x3C */ u8 unk3C[0x40 - 0x3C];
    /* 0x40 */ s32 unk40;
    /* 0x44 */ StageHit unk44;
    /* 0x58 */ u8 unk58[0x5C - 0x58];
    /* 0x5C */ CollisionSphere unk5C;
} Unk800D99F0;

extern EntityVtable D_800D3A8C;


/* Constructs Unk800D99F0 at position and adds its member to list; its part's vector is the velocity
 * scaled and lifted as HolydramonSpecial1Hitbox_init does it. */
Unk800D99F0 *func_800D99F0(Unk800D99F0 *this, Fight *list, Vec3 *position, u16 arg) {
    VECTOR velocity;
    VECTOR v;
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3A8C;
    StageHit_init(&this->unk44, 0x80002800);
    CollisionSphere_init(&this->unk5C, position, 0x80, &this->unk44);
    this->list = list;
    this->unk1C = arg;
    Fight_addStageShape(list, &this->unk5C.link);
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
    this->unk44.knockback.vx = v.vx;
    this->unk44.knockback.vy = v.vy;
    this->unk44.knockback.vz = v.vz;
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
    this->unk5C.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk5C.node) written out: the match depends on this
     * access order (soundControlDestroy's) */
    link = &this->unk5C.link;
    this->unk5C.link.next->prev = this->unk5C.link.prev;
    link->prev->next = this->unk5C.link.next;
    this->unk5C.link.next = link;
    this->unk5C.link.prev = link;
    this->unk44.vtable = COLLISION_OWNER_VTABLE;
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
 * StageHit part at 0x3C and a CollisionSphere member at 0x54. */
typedef struct Unk800D9DDC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *list;
    /* 0x1C */ Vec3 position;
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ Vec3 velocity;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ StageHit unk3C;
    /* 0x50 */ u8 unk50[0x54 - 0x50];
    /* 0x54 */ CollisionSphere unk54;
} Unk800D9DDC;

extern EntityVtable D_800D3A64;

/* Constructs Unk800D9DDC at position and adds its member to list: func_800D99F0's
 * twin, with its part's vector at four times the velocity. */
Unk800D9DDC *func_800D9DDC(Unk800D9DDC *this, Fight *list, Vec3 *position) {
    VECTOR velocity;
    VECTOR v;
    s32 x, y, z;

    entityInit(&this->entity, 0x40000);
    this->entity.vtable = &D_800D3A64;
    StageHit_init(&this->unk3C, 0x80002800);
    CollisionSphere_init(&this->unk54, position, 0x80, &this->unk3C);
    this->list = list;
    Fight_addStageShape(list, &this->unk54.link);
    this->unk54.flags |= 1;
    /* The match depends on the same velocity, v copy and order as in func_800D99F0 */
    velocity.vx = 0x20;
    velocity.vy = 0;
    velocity.vz = 0;
    v = velocity;
    v.vx <<= 2;
    v.vy <<= 2;
    v.vz <<= 2;
    this->unk3C.knockback.vx = v.vx;
    this->unk3C.knockback.vy = v.vy;
    this->unk3C.knockback.vz = v.vz;
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
    this->unk54.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->unk54.node) written out: the match depends on this
     * access order (soundControlDestroy's) */
    link = &this->unk54.link;
    this->unk54.link.next->prev = this->unk54.link.prev;
    link->prev->next = this->unk54.link.next;
    this->unk54.link.next = link;
    this->unk54.link.prev = link;
    this->unk3C.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("st00/nonmatchings/st00", func_800D9F70);

/* Returns whether the stage's 0x1050 is set. */
s32 func_800DA140(StageMachine *this) {
    return this->unk1050 != 0;
}

/* StageHit's destructor: CollisionOwner's, inlined. */
void StageHit_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the hit's knockback, reading all three components before writing any (as vecCopy does). */
void StageHit_setKnockback(StageHit *this, Vec3 *knockback) {
    s32 x, y, z;

    x = knockback->vx;
    y = knockback->vy;
    z = knockback->vz;
    this->knockback.vx = x;
    this->knockback.vy = y;
    this->knockback.vz = z;
}

/* Sets the hit's damage. */
void StageHit_setDamage(StageHit *this, s32 damage) {
    this->damage = damage;
}
