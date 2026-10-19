/* The projectile (Projectile_init, virtual tables PROJECTILE_HIT_VTABLE and PROJECTILE_VTABLE) that nine fighters'
 * projectiles and the item projectile derive from. */

#include "common.h"
#include "battle/game_classes.h"

/* Constructs a projectile of fighter's at position moving by velocity: its hit (damage, kind, knocked the way it
 * flies by 64 times its speed and up by its speed) and its hit part, growing to maxRadius, over range; the part goes
 * among the shapes of the fighter's side. The match depends on each vector copy reading its three words first. */
Projectile *Projectile_init(Projectile *this, Fighter *fighter, Vec3 *position, Vec3 *velocity, s32 maxRadius,
                            u32 damage, s32 range, s32 kind) {
    VECTOR knockback;
    s32 speed;
    s32 dir;
    Fight *fight;
    s32 x, y, z;
    s32 vx, vy, vz;

    entityInit(&this->parts.entity, ENTITY_KIND_FIGHTER_OBJECT);
    Hit_init(&this->parts.base, fighter, damage, (Vec3 *)&ZERO_VECTOR, kind);
    this->parts.base.vtable = PROJECTILE_HIT_VTABLE;
    this->parts.entity.vtable = &PROJECTILE_VTABLE;
    CollisionSphere_init(&this->parts.part, position, 0, &this->parts.base);
    speed = vecGetLength((VECTOR *)velocity);
    x = position->vx;
    y = position->vy;
    z = position->vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    vx = velocity->vx;
    vy = velocity->vy;
    vz = velocity->vz;
    this->velocity.vx = vx;
    this->velocity.vy = vy;
    this->velocity.vz = vz;
    this->hitLevel = (damage >> 24) & 3;
    this->radius = 0;
    this->maxRadius = maxRadius;
    this->speed = speed;
    this->distance = 0;
    this->range = range;
    this->age = 0;
    this->unkBC = Fighter_getVramY(fighter);
    dir = velocity->vx < 0 ? -1 : 1;
    knockback.vx = dir * (speed << 6);
    knockback.vy = -speed;
    knockback.vz = 0;
    Hit_setKnockback(&this->parts.base, &knockback);
    fight = Fighter_getScene(fighter);
    if (Fighter_getSide(fighter) == 0) {
        Fight_addSide0Shape(fight, &this->parts.part.link);
    } else {
        Fight_addSide1Shape(fight, &this->parts.part.link);
    }
    this->parts.part.entity = &this->parts.entity;
    return this;
}

/* The projectile's frame: moves it by its velocity and carries its hit part along; the hit part grows to
 * maxRadius until the projectile has gone its range, then it slows to half speed and shrinks by an eighth of
 * maxRadius a frame, and the projectile deletes itself once the radius is gone. */
void Projectile_tick(Projectile *this, s32 arg) {
    s32 x, y, z;

    /* The match depends on the vector sum reading everything before writing (as vecAddInPlace). */
    x = this->position.vx;
    y = this->position.vy;
    z = this->position.vz;
    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    this->position.vx = x;
    this->position.vy = y;
    this->position.vz = z;
    this->parts.part.position = this->position;
    this->parts.part.radius = this->radius;
    this->distance += this->speed;
    if (this->distance >= this->range) {
        this->velocity.vx >>= 1;
        this->velocity.vy >>= 1;
        this->velocity.vz >>= 1;
        this->radius -= this->maxRadius >> 3;
        if (this->radius <= 0) {
            entityDelete(&this->parts.entity);
            return;
        }
    } else {
        this->radius += (this->maxRadius - this->radius) >> 2;
    }
    this->age++;
}

/* The projectile's hit (its Hit's slot 0x08): applies the hit (Hit_apply), spawns a burst by its hit level and a
 * hit spark toward the fighter it hit, then deletes itself. */
void Projectile_hit(Projectile *this, CollisionSphere *sum) {
    Hit_apply(&this->parts.base, sum);
    Projectile_spawnBurst(this, Projectile_getBurstKind(this));
    Projectile_spawnHitSpark(this, getHitFighter(sum));
    entityDelete(&this->parts.entity);
}

/* The particle kind of the projectile's burst by its hit level: 3 for 0 or 1 (and any other value), 4 for 2, 5
 * for 3. */
s32 Projectile_getBurstKind(Projectile *this) {
    switch (this->hitLevel) {
    case 0:
    case 1:
        return 3;
    case 2:
        return 4;
    case 3:
        return 5;
    }
    return 3;
}

/* Spawns a burst of 6 particles of kind (ParticleBurst_init, one a frame, size 0x50, grey) at position, spread by
 * a quarter of the projectile's speed, as a sibling. */
static inline void spawnBurstAt(Projectile *this, s32 kind, Vec3 *position) {
    Entity *burst = allocEntityMemory(0x3C);

    if (burst != NULL) {
        ParticleBurst_init(entityOperatorNewAt(0x3C, burst), kind, position, this->speed >> 2, 0x50, 6, 0x808080);
    }
    entityAddSibling(&this->parts.entity, burst);
}

/* Spawns the projectile's burst of kind a step ahead of it (position + velocity). The match depends on the spawn
 * as an inline taking &ahead (the address stays in a saved register across allocEntityMemory) and on the
 * position read whole, then the velocity added in place. */
void Projectile_spawnBurst(Projectile *this, s32 kind) {
    Vec3 ahead;
    s32 x = this->position.vx;
    s32 y = this->position.vy;
    s32 z = this->position.vz;

    x += this->velocity.vx;
    y += this->velocity.vy;
    z += this->velocity.vz;
    ahead.vx = x;
    ahead.vy = y;
    ahead.vz = z;
    spawnBurstAt(this, kind, &ahead);
}

/* Spawns the spark of the projectile's hit on fighter at position, as a sibling: a guard flash (3/4 of maxRadius,
 * color 0xA0A0A0) when fighter guards, else a hit spark turned the projectile's way (4/3 of maxRadius, color
 * 0x808080), as createHitSpark builds them. */
static inline void spawnHitSparkAt(Projectile *this, Fighter *fighter, Vec3 *position) {
    Entity *child;

    if (Fighter_isGuarding(fighter)) {
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_8006D4D4_initSized(entityOperatorNewAt(0x58, child), (VECTOR *)position, this->maxRadius * 3 / 4,
                                    0xA0A0A0);
        }
        entityAddSibling(&this->parts.entity, child);
    } else {
        child = allocEntityMemory(0x48);
        if (child != NULL) {
            HitSpark_init(entityOperatorNewAt(0x48, child), position, (VECTOR *)&this->velocity,
                          this->maxRadius * 4 / 3, 0x808080);
        }
        entityAddSibling(&this->parts.entity, child);
    }
}

/* Spawns the spark of the projectile's hit on target halfway between the projectile and the target's point
 * (func_8007A650). The match depends on the spawn as an inline taking &position (the address stays in a saved
 * register across Fighter_isGuarding). */
void Projectile_spawnHitSpark(Projectile *this, Entity *target) {
    Fighter *fighter = (Fighter *)target;
    Vec3 position;

    if (target == NULL) {
        return;
    }
    func_8007A650(fighter, &position);
    position.vx = (position.vx + this->position.vx) >> 1;
    position.vy = (position.vy + this->position.vy) >> 1;
    position.vz = (position.vz + this->position.vz) >> 1;
    spawnHitSparkAt(this, fighter, &position);
}

/* Spawns the spark of a hit on target at its point (func_8007A650) and returns it: a guard flash (vtable
 * D_8006D4D4 on Particle_initById with frame 8, no velocity, 3/4 of size, color 0xA0A0A0) when it guards, else a
 * hit spark (HitSpark_init) turned the way of direction, 4/3 of size, color 0x808080. The match depends on the
 * guard flash built by the inlined constructor func_8006D4D4_initSized (its size is computed before ZERO_VECTOR). */
Entity *createHitSpark(Entity *target, Vec3 *direction, s32 size) {
    Fighter *fighter = (Fighter *)target;
    Vec3 position;
    Entity *child;

    func_8007A650(fighter, &position);
    if (Fighter_isGuarding(fighter)) {
        child = allocEntityMemory(0x58);
        if (child != NULL) {
            func_8006D4D4_initSized(entityOperatorNewAt(0x58, child), (VECTOR *)&position, size * 3 / 4, 0xA0A0A0);
        }
    } else {
        child = allocEntityMemory(0x48);
        if (child != NULL) {
            HitSpark_init(entityOperatorNewAt(0x48, child), &position, (VECTOR *)direction, size * 4 / 3, 0x808080);
        }
    }
    return child;
}

/* The projectile's destructor (Projectile_init's virtual tables): game has 11 identical copies, one per derived
 * class. */
void Projectile_destroy(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = PROJECTILE_HIT_VTABLE;
    this->entity.vtable = &PROJECTILE_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    this->base.vtable = COLLISION_OWNER_VTABLE;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The projectile's draw: nothing (the derived classes draw). */
void Projectile_draw(void) {
}

INCLUDE_RODATA("game/nonmatchings/game_0h", PROJECTILE_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0h", PROJECTILE_VTABLE);
