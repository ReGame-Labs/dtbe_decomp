/* The projectile (Projectile_init, virtual tables PROJECTILE_HIT_VTABLE and PROJECTILE_VTABLE) that nine fighters'
 * projectiles and the item projectile derive from. */

#include "common.h"
#include "battle/game_classes.h"

INCLUDE_ASM("game/nonmatchings/game_0h", Projectile_init);

INCLUDE_ASM("game/nonmatchings/game_0h", Projectile_tick);

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

INCLUDE_ASM("game/nonmatchings/game_0h", Projectile_spawnHitSpark);

INCLUDE_ASM("game/nonmatchings/game_0h", createHitSpark);

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
