/* The collision shapes the fight tests against each other: spheres (a position, a radius and an owner) and
 * their overlap test; then the terrain probe, which finds where a segment crosses the collision lines of the
 * stage (COLLISION_LINKS), and the line helpers. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/math/divide.h"
#include "engine/math/sin_cos.h"
#include "engine/system/memory.h"

s32 isInTerrainLineSpanXY(TerrainLine *line, s32 x, s32 y);

extern VtableEntry COLLISION_SPHERE_VTABLE[];

/* Constructs a sphere of owner at position with radius: CollisionShape_init's fields, inlined, with unk10 = 0,
 * then its own virtual table (as CollisionSphere_initDisabled).
 * The match depends on the virtual table stored after the base fields (vtable first gives other registers to
 * the flags), and on reading the three words of position before storing any, as vecCopy does. */
CollisionSphere *CollisionSphere_init(CollisionSphere *this, Vec3 *position, s32 radius, void *owner) {
    s32 x;
    s32 y;
    s32 z;

    this->link.next = &this->link;
    this->link.prev = &this->link;
    this->owner = owner;
    this->unk10 = 0;
    this->entity = NULL;
    this->hitDamage = 0;
    this->hitKind = 0;
    this->hitKnockback.vx = 0;
    this->hitKnockback.vy = 0;
    this->hitKnockback.vz = 0;
    this->hitKnockbackPad = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    this->vtable = COLLISION_SPHERE_VTABLE;
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->radius = radius;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    return this;
}

/* Whether the two spheres overlap: their centres are nearer than the sum of their radii (never when other's
 * unk10 is set). */
s32 CollisionSphere_overlaps(CollisionSphere *this, CollisionSphere *other) {
    VECTOR delta;
    s32 x;
    s32 y;
    s32 z;

    if (other->unk10 != 0) {
        return 0;
    }
    /* The match depends on x, y and z each set twice (read, then subtract): set-once locals let sched1 sink
     * every load to its subtraction. */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x -= other->position.vx;
    y -= other->position.vy;
    z -= other->position.vz;
    delta.vx = x;
    delta.vy = y;
    delta.vz = z;
    return vecGetLength(&delta) < this->radius + other->radius;
}

INCLUDE_RODATA("game/nonmatchings/game_0c", COLLISION_SPHERE_OUTLINE);

INCLUDE_ASM("game/nonmatchings/game_0c", CollisionSphere_draw);

/* Constructs a CollisionShape at this: unlinked, with owner and unk10, no entity, no hits added, neither
 * disabled nor touched. Written on CollisionSphere, which spells out the hit fields CollisionShape leaves as a
 * gap. */
CollisionSphere *CollisionShape_init(CollisionSphere *this, s32 unk10, void *owner) {
    this->vtable = COLLISION_SHAPE_VTABLE;
    this->link.next = &this->link;
    this->link.prev = &this->link;
    this->owner = owner;
    this->unk10 = unk10;
    this->entity = NULL;
    this->hitDamage = 0;
    this->hitKind = 0;
    this->hitKnockback.vx = 0;
    this->hitKnockback.vy = 0;
    this->hitKnockback.vz = 0;
    this->hitKnockbackPad = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

/* Returns unk10 (the overlap test needs the other's to be 0). */
s32 func_80076204(CollisionShape *this) {
    return this->unk10;
}

/* The entity the shape belongs to. */
void *CollisionShape_getEntity(CollisionShape *this) {
    return this->entity;
}

/* CollisionShape's destructor: takes it out of its list. */
void CollisionShape_destroy(CollisionShape *this, s32 flags) {
    this->vtable = COLLISION_SHAPE_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Sets the shape's owner (the CollisionOwner told of each touch). */
void CollisionShape_setOwner(CollisionShape *this, void *owner) {
    this->owner = owner;
}

/* Sets the entity the shape belongs to. */
void CollisionShape_setEntity(CollisionShape *this, void *entity) {
    this->entity = entity;
}

/* Disables the shape (collideShapes skips it) or enables it again; no caller found. */
void CollisionShape_setDisabled(CollisionShape *this, s32 disabled) {
    this->flags = (this->flags & ~COLLISION_SHAPE_DISABLED) | (disabled & COLLISION_SHAPE_DISABLED);
}

/* Whether the shape is disabled. */
s32 CollisionShape_isDisabled(CollisionShape *this) {
    return this->flags & COLLISION_SHAPE_DISABLED;
}

/* Whether the shape touched another; no caller found. */
s32 CollisionShape_isTouched(CollisionShape *this) {
    return (this->flags >> 1) & 1;
}

/* Forgets that the shape touched another. */
void CollisionShape_clearTouched(CollisionShape *this) {
    this->flags &= ~COLLISION_SHAPE_TOUCHED;
}

/* Adds node at the end of the list head (listAppend); no caller found. */
void func_800762D4(ListNode *head, ListNode *node) {
    listAppend(head, node);
}

/* Constructs a CollisionOwner: only its virtual table, whose slot 0x08 is told of each touch. */
CollisionOwner *CollisionOwner_init(CollisionOwner *this) {
    this->vtable = COLLISION_OWNER_VTABLE;
    return this;
}

/* CollisionOwner's destructor. */
void CollisionOwner_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* CollisionSphere's destructor: CollisionShape's, inlined. */
void CollisionSphere_destroy(CollisionShape *this, s32 flags) {
    this->vtable = COLLISION_SHAPE_VTABLE;
    listRemove(&this->link);
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs a disabled sphere of owner: CollisionShape_init's fields, inlined, with unk10 = 0, then
 * COLLISION_SHAPE_DISABLED set. */
CollisionSphere *CollisionSphere_initDisabled(CollisionSphere *this, void *owner) {
    this->link.next = &this->link;
    this->link.prev = &this->link;
    this->owner = owner;
    this->unk10 = 0;
    this->entity = NULL;
    this->hitDamage = 0;
    this->hitKind = 0;
    this->hitKnockback.vx = 0;
    this->hitKnockback.vy = 0;
    this->hitKnockback.vz = 0;
    this->hitKnockbackPad = 0;
    this->flags &= ~1;
    this->flags &= ~2;
    this->vtable = COLLISION_SPHERE_VTABLE;
    this->flags &= ~1;
    this->flags |= 1;
    return this;
}

/* Copies the sphere's centre to out and returns out; nothing calls it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *CollisionSphere_getPosition(CollisionSphere *this, Vec3 *out) {
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;

    out->vx = x;
    out->vy = y;
    out->vz = z;
    return out;
}

/* Moves the sphere's centre to in.
 * The match depends on reading the three words before storing any, as vecCopy does. */
void CollisionSphere_setPosition(CollisionSphere *this, Vec3 *in) {
    s32 x = in->vx;
    s32 y = in->vy;
    s32 z = in->vz;

    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
}

/* Sets the sphere's radius. */
void CollisionSphere_setRadius(CollisionSphere *this, s32 radius) {
    this->radius = radius;
}

/* The sphere's radius. */
s32 CollisionSphere_getRadius(CollisionSphere *this) {
    return this->radius;
}

/* Constructs a terrain probe, starting at the first link of COLLISION_LINKS. */
TerrainProbe *TerrainProbe_init(TerrainProbe *this) {
    this->link = (MeshSceneLink *)getCollisionLinks()->next;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0c", TerrainProbe_cast);

/* The link of COLLISION_LINKS hit last. */
MeshSceneLink *TerrainProbe_getLink(TerrainProbe *this) {
    return this->link;
}

/* Returns this; nothing calls it. */
TerrainProbe *func_80076590(TerrainProbe *this) {
    return this;
}

/* The world matrix of the link hit last: the stage part the segment crossed. */
MATRIX *TerrainProbe_getMatrix(TerrainProbe *this) {
    return this->link->world;
}

/* The normal of the line hit, in world space. */
VECTOR *TerrainProbe_getNormal(TerrainProbe *this) {
    return &this->normal;
}

Vec3 *getCrossingPoint(Vec3 *out, VECTOR *ends, s32 startDistance, s32 endDistance);

/* Where the segment crossed the line hit (getCrossingPoint of its ends and their distances); returns out. */
Vec3 *TerrainProbe_getHitPoint(TerrainProbe *this, Vec3 *out) {
    getCrossingPoint(out, &this->start, this->startDistance, this->endDistance);
    return out;
}

INCLUDE_ASM("game/nonmatchings/game_0c", getCrossingPoint);

INCLUDE_ASM("game/nonmatchings/game_0c", castTerrainLink);

/* The dot product of the line's normal with point (x, y). */
s32 getTerrainLineDot(TerrainLine *this, s32 *point) {
    return this->normalX * point[0] + this->normalY * point[1];
}

/* The line equation at point (x, y): its signed distance from the line, scaled by the normal. */
s32 getTerrainLineDistance(TerrainLine *this, s32 *point) {
    return this->normalX * point[0] + this->normalY * point[1] + this->offset;
}

/* Whether point (x, y) lies between the line's endpoints (isInTerrainLineSpanXY). */
s32 isInTerrainLineSpan(TerrainLine *this, s32 *point) {
    return isInTerrainLineSpanXY(this, point[0], point[1]);
}

/* Whether (x, y) lies between the line's endpoints: inside the circle whose diameter joins them,
 * (x - x0)(x - x1) + (y - y0)(y - y1) < 1. */
s32 isInTerrainLineSpanXY(TerrainLine *line, s32 x, s32 y) {
    return (x - line->x0) * (x - line->x1) + (y - line->y0) * (y - line->y1) < 1;
}

/* Swaps two lines (four words each) by xor (swapFourWords' body, a == b left alone) and returns b: its one
 * caller moves the line hit to the front of the link's lines. */
u32 *swapTerrainLines(u32 *a, u32 *b) {
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

extern Vec3 TERRAIN_UP;

/* Sets the up vector of the terrain (the fight passes (0, -0x1000, 0)) and returns it.
 * The match depends on reading the three words before storing any, as vecCopy does. */
Vec3 *setTerrainUp(Vec3 *v) {
    s32 x = v->vx;
    s32 y = v->vy;
    s32 z = v->vz;

    TERRAIN_UP.vx = x;
    TERRAIN_UP.vy = y;
    TERRAIN_UP.vz = z;
    return &TERRAIN_UP;
}

/* Returns the list of the mesh scene links that have collision lines (mesh_scene.h). */
ListNode *getCollisionLinks(void) {
    return &COLLISION_LINKS;
}

/* Moves a link to the front of COLLISION_LINKS: TerrainProbe_cast does it with the link it hit. */
void moveCollisionLinkToFront(MeshSceneLink *link) {
    listRemove(&link->link);
    listInsertAfter(&COLLISION_LINKS, &link->link);
}

/* Moves a link to the back of COLLISION_LINKS, where meshSceneHideNode puts the hidden ones. */
void moveCollisionLinkToBack(MeshSceneLink *link) {
    listRemove(&link->link);
    link->link.next = &COLLISION_LINKS;
    link->link.prev = COLLISION_LINKS.prev;
    COLLISION_LINKS.prev->next = &link->link;
    COLLISION_LINKS.prev = &link->link;
}

extern MATRIX D_800108B4;

/* Adds a shown link at the front of COLLISION_LINKS, with the matrix D_800108B4 and the collision lines given
 * (the stages build them with initTerrainLine). */
void addCollisionLink(MeshSceneLink *link, s32 *lines, s32 lineCount) {
    listInsertAfter(&COLLISION_LINKS, &link->link);
    link->world = &D_800108B4;
    link->lines = lines;
    link->lineCount = lineCount;
    link->shown = 1;
}

/* Builds the line from `from` to `to`: its endpoints, then its normal (the cosine and sine of the angle of
 * ratan2(x0 - x1, y1 - y0)) and the offset that puts both endpoints at distance 0. The match depends on the
 * normal read into locals (the stored fields reload as lhu) and on the angle masked in the index (masked into
 * the local, the table's address loads after the mask). */
void initTerrainLine(TerrainLine *this, DVECTOR *from, DVECTOR *to) {
    s32 angle;
    s32 normalX;
    s32 normalY;

    this->x0 = from->vx;
    this->y0 = from->vy;
    this->x1 = to->vx;
    this->y1 = to->vy;
    angle = ratan2(this->x0 - this->x1, this->y1 - this->y0);
    normalX = COS_SIN_TABLE[angle & 0xFFF].cos;
    this->normalX = normalX;
    normalY = COS_SIN_TABLE[angle & 0xFFF].sin;
    this->normalY = normalY;
    this->offset = -(normalX * this->x0 + normalY * this->y0);
}

/* Fills out with the hit line's normal scaled by run * endDistance / ((endDistance - startDistance) / 2), run
 * being the x extent of the segment in the link's space (no caller in the game). The match depends on x and y
 * each computed in place, product, quotient, then scaled: written as one expression per component, cse shares
 * the product and the quotient between them. */
void func_80076BB4(TerrainProbe *this, VECTOR *out) {
    s32 run = this->localEnd.vx - this->localStart.vx;
    s32 half = (this->endDistance - this->startDistance) / 2;
    s32 x;
    s32 y;

    x = run * this->endDistance;
    x /= half;
    y = run * this->endDistance;
    y /= half;
    x = mul12(x, this->line.normalX);
    y = mul12(y, this->line.normalY);
    out->vx = x;
    out->vy = y;
    out->vz = 0;
}

/* Turns the angle from toward target by at most step (12-bit angles, the short way round). */
s32 turnAngleToward(s32 target, s32 from, s32 step) {
    s16 delta = ((target - from - 0x800) & 0xFFF) - 0x800;

    if (delta < -step) {
        delta = -step;
    } else if (step < delta) {
        delta = step;
    }
    return from + delta;
}

INCLUDE_RODATA("game/nonmatchings/game_0c", COLLISION_SPHERE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0c", COLLISION_OWNER_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0c", COLLISION_SHAPE_VTABLE);
