/* The battle's items: the stage's item spawner, the pickup items and what they give a fighter, their HUD popup
 * and icon, and the item shot (virtual tables D_8006D980 to ITEM_SPAWNER_VTABLE). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"

struct ItemSpawner *ItemSpawner_init(struct ItemSpawner *this, Fight *scene, void *spawnPoints, s32 arg3);
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3);
Entity *Item_init(Entity *this, struct ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                      s32 arg6);
struct ItemPopup *ItemPopup_init(struct ItemPopup *this, Fighter *fighter, s32 itemKind);
struct ItemEffectIcon *ItemEffectIcon_init(struct ItemEffectIcon *this, Fighter *fighter, s32 itemKind);
struct Unk800CAACC *func_800CAACC(struct Unk800CAACC *this, Fighter *chara, s32 arg2, s32 arg3, s32 arg4,
                                  s32 arg5);
extern EntityVtable D_8006D9F0;
extern EntityVtable D_8006DA18;
extern EntityVtable ITEM_EFFECT_ICON_VTABLE;
extern EntityVtable ITEM_POPUP_VTABLE;
extern EntityVtable D_8006DAC0;
extern EntityVtable D_8006DAF0;
extern EntityVtable D_8006DA90;
extern EntityVtable D_8006DB20;
extern VtableEntry ITEM_HIT_LISTENER_VTABLE[];
extern EntityVtable ITEM_SPAWNER_VTABLE;

/* The item an ItemHitListener forwards to, seen as Item_callTouch sees it: its entity's virtual table (at 0x14)
 * with the touch slot at 0x20 (Item_touch). */
typedef struct Unk800C8DFCVtable {
    /* 0x00 */ VtableEntry unk0[4];
    /* 0x20 */ VtableEntry touch;
} Unk800C8DFCVtable;

typedef struct Unk800C8DFC {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Unk800C8DFCVtable *vtable;
} Unk800C8DFC;

/* An item's hit listener (ItemHitListener_init, virtual table ITEM_HIT_LISTENER_VTABLE; Item_init builds it inline
 * at Item 0x60), a CollisionOwner: the owner of the item's hit part, it forwards a touch to the item's slot 0x20. */
typedef struct ItemHitListener {
    /* 0x0 */ CollisionOwner base;
    /* 0x4 */ Unk800C8DFC *item;
} ItemHitListener;

/* A pickup item (Item_init, virtual table ITEM_VTABLE, entity kind 0x20000; the classes of D_8006DAC0,
 * D_8006DAF0 and D_8006DB20 derive from it): a fighter that touches it gets its kind's effect (Item_giveTo). */
typedef struct Item {
    /* 0x00 */ Entity entity;
    /* 0x18 */ CollisionSphere part; /* the hit sphere, radius 0xA0 */
    /* 0x60 */ ItemHitListener listener;
    /* 0x68 */ s32 kind;     /* ITEM_KIND_POOLS resolved (resolveItemKind): its effect (Item_giveTo) */
    /* 0x6C */ Vec3 position;
    /* 0x78 */ u8 unk78[0x7C - 0x78];
    /* 0x7C */ s32 unk7C;
    /* 0x80 */ s32 unk80;
    /* 0x84 */ s32 unk84;
    /* 0x88 */ s32 unk88;
    /* 0x8C */ s32 slot;     /* its slot of ITEM_SLOTS (ItemSpawner_removeItem) */
    /* 0x90 */ u8 unk90[0xA4 - 0x90];
    /* 0xA4 */ void *spawner; /* the ItemSpawner it came from, or NULL (func_800C8658) */
    /* 0xA8 */ u8 unkA8[0xAC - 0xA8];
    /* 0xAC */ s32 life;     /* frames; the item box's opening adds 150 */
    /* 0xB0 */ s32 unkB0;
    /* 0xB4 */ s32 unkB4;
    /* 0xB8 */ s32 unkB8;
    /* 0xBC */ u8 unkBC[0xC4 - 0xBC];
} Item;

void func_800C9140(Item *this, s32 arg);
void func_800C9288(Item *this, s32 arg);

/* the destructor of Item, the entity class others derive from */
void Item_destroy(Item *this, s32 flags);
void Item_callTouch(Unk800C8DFC *object, s32 arg);
void Item_tick(void *this);

/* A slot of ITEM_SLOTS: where an item of the spawner is. */
typedef struct ItemSlot {
    /* 0x0 */ u8 flags; /* cleared by ItemSpawner_init and ItemSpawner_removeItem; Item_clearSlotFlags clears bits */
    /* 0x1 */ u8 unk1;
    /* 0x2 */ s16 x;    /* the position of the slot's object (Item_syncSlot) */
    /* 0x4 */ s16 y;
    /* 0x6 */ s16 z;
} ItemSlot;

/* ITEM_SLOTS: the items the spawner has out, and a slot for each of its spawn points. */
typedef struct ItemSlots {
    /* 0x00 */ s32 count;
    /* 0x04 */ ItemSlot slots[16];
} ItemSlots;

extern ItemSlots ITEM_SLOTS;

/* A spawn point of the stage's list (ItemSpawner_init): the list ends at one whose first halfword is -1; then
 * the kind (negative for a pool of ITEM_KIND_POOLS) and the position, x, y and z. */
typedef struct ItemSpawnPoint {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2[4];
} ItemSpawnPoint;

/* The stage's item spawner (ItemSpawner_init, virtual table ITEM_SPAWNER_VTABLE): its spawn points, one slot of
 * ITEM_SLOTS each, an item every 90 frames, and a cooldown on the healing items (kinds 11 to 13). */
typedef struct ItemSpawner {
    /* 0x00 */ Entity entity;
    /* 0x18 */ Fight *scene;
    /* 0x1C */ ItemSpawnPoint *spawnPoints;
    /* 0x20 */ s32 unk20;           /* ItemSpawner_init's last argument (st04 passes 2) */
    /* 0x24 */ ItemSlots *slots;
    /* 0x28 */ s32 spawnPointCount; /* ItemSpawner_countSpawnPoints */
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 spawnTimer;      /* 90 from ItemSpawner_init and ItemSpawner_removeItem */
    /* 0x34 */ s32 healCooldown;    /* 300 at first, 600 after a healing item */
    /* 0x38 */ s32 unk38;
} ItemSpawner; /* size 0x3C */

/* Returns ITEM_SLOTS. */
ItemSlots *getItemSlots(void) {
    return &ITEM_SLOTS;
}

/* Creates the stage's item spawner (0x3C bytes) with its spawn points and adds it to the fight's stage entities
 * (the stages call it with a list of their .data). */
void createItemSpawner(Fight *this, void *spawnPoints, s32 arg2) {
    Entity *child = allocEntityMemory(0x3C);

    if (child != NULL) {
        ItemSpawner_init(entityOperatorNewAt(0x3C, child), this, spawnPoints, arg2);
    }
    Fight_addStageEntity(this, child);
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D788);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D798);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7A4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7A8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7AC);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7B8);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D7C0);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_KIND_POOLS);

/* A pool of item kinds to draw one from at random (ITEM_KIND_POOLS, by kind -10 to -2). */
typedef struct ItemKindPool {
    /* 0x0 */ s8 *kinds;
    /* 0x4 */ s32 count;
} ItemKindPool;

extern ItemKindPool ITEM_KIND_POOLS[];

/* Resolves a kind: -10 to -2 draw one at random from the pools of ITEM_KIND_POOLS, any other is returned as it is. */
s32 resolveItemKind(s32 kind) {
    s32 pool;
    s8 *kinds;
    s32 count;

    switch (kind) {
    case -10:
        pool = 0;
        break;
    case -9:
        pool = 1;
        break;
    case -8:
        pool = 2;
        break;
    case -7:
        pool = 3;
        break;
    case -6:
        pool = 4;
        break;
    case -5:
        pool = 5;
        break;
    case -4:
        pool = 6;
        break;
    case -3:
        pool = 7;
        break;
    case -2:
        pool = 8;
        break;
    default:
        return kind;
    }
    kinds = ITEM_KIND_POOLS[pool].kinds;
    count = ITEM_KIND_POOLS[pool].count;
    /* the game scales the draw with a signed shift (sra): the u32 draw is taken as an s32 */
    return kinds[((s32)(mersenneTwisterGenerate(&RANDOM) & 0xFF) * count) >> 8];
}

Entity *func_800C9A88(Entity *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                      s32 arg6);
struct Unk800C9AF4 *func_800C9AF4(struct Unk800C9AF4 *this, ItemSpawner *arg1, s32 arg2, Fight *arg3,
                                  s32 arg4, Vec3 *arg5, s32 arg6);
Entity *func_800C9CA0(Entity *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                      s32 arg6);
struct Unk800C9D0C *func_800C9D0C(struct Unk800C9D0C *this, ItemSpawner *arg1, s32 arg2, Fight *arg3,
                                  s32 arg4, Vec3 *arg5, s32 arg6);

/* Creates the entity of a kind (resolved by resolveItemKind) from this's list: kinds 0 to 10 one of two classes,
 * 11 to 13 func_800C9CA0's (with no list when this is NULL; with one, only while 0x34 is 0, which then holds it
 * for 600 frames, else as kind 14), 14 func_800C9D0C's. Returns NULL for any other kind. */
Entity *createItem(ItemSpawner *this, s32 arg1, Fight *arg2, s32 kind, Vec3 *arg4, s32 arg5) {
    Entity *child;

    kind = resolveItemKind(kind);
    switch (kind) {
    case 0:
    case 1:
    case 3:
    case 8:
    case 9:
    case 10:
        child = allocEntityMemory(0xC4);
        if (child != NULL) {
            func_800C9A88(entityOperatorNewAt(0xC4, child), this, arg1, arg2, kind, arg4, arg5);
        }
        return child;
    case 2:
    case 4:
    case 5:
    case 6:
    case 7:
        child = allocEntityMemory(0xC8);
        if (child != NULL) {
            func_800C9AF4(entityOperatorNewAt(0xC8, child), this, arg1, arg2, kind, arg4, arg5);
        }
        return child;
    case 11:
    case 12:
    case 13:
        if (this == NULL) {
            child = allocEntityMemory(0xC4);
            if (child != NULL) {
                func_800C9CA0(entityOperatorNewAt(0xC4, child), NULL, arg1, arg2, kind, arg4, arg5);
            }
            return child;
        }
        if (this->healCooldown == 0) {
            this->healCooldown = 600;
            child = allocEntityMemory(0xC4);
            if (child != NULL) {
                func_800C9CA0(entityOperatorNewAt(0xC4, child), this, arg1, arg2, kind, arg4, arg5);
            }
            return child;
        }
        /* fallthrough */
    case 14:
        child = allocEntityMemory(0x158);
        if (child != NULL) {
            func_800C9D0C(entityOperatorNewAt(0x158, child), this, arg1, arg2, 14, arg4, arg5);
        }
        return child;
    }
    return NULL;
}

/* Creates an entity of ItemPopup_init's class (0x24 bytes) with g++'s new, and hands it to
 * Fighter_addEntity. */
void showItemPopup(Fighter *this, s32 arg) {
    Entity *child = allocEntityMemory(0x24);

    if (child != NULL) {
        ItemPopup_init(entityOperatorNewAt(0x24, child), this, arg);
    }
    Fighter_addEntity(this, child);
}

/* Creates an entity of ItemEffectIcon_init's class (0x30 bytes) with g++'s new, and hands it to
 * Fighter_addEntity. */
void showItemEffectIcon(Fighter *this, s32 arg) {
    Entity *child = allocEntityMemory(0x30);

    if (child != NULL) {
        ItemEffectIcon_init(entityOperatorNewAt(0x30, child), this, arg);
    }
    Fighter_addEntity(this, child);
}

/* Creates func_800CAACC's entity (0x48 bytes) for the character with the four arguments, and hands
 * it to Fighter_addEntity. */
void func_800C7C08(Fighter *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_800CAACC(entityOperatorNewAt(0x48, child), this, arg1, arg2, arg3, arg4);
    }
    Fighter_addEntity(this, child);
}

/* Creates func_800CAEF0's entity (0x54 bytes) at the character's func_8007A650 point with 0x100 and
 * the colour (0x2020FF, 0xFF2020), and hands it to Fighter_addEntity. arg1 is not read. */
void func_800C7C9C(Fighter *this, s32 arg1, s32 color) {
    Vec3 position;
    Entity *child;

    func_8007A650(this, &position);
    child = allocEntityMemory(0x54);
    if (child != NULL) {
        func_800CAEF0(entityOperatorNewAt(0x54, child), &position, 0x100, color);
    }
    Fighter_addEntity(this, child);
}

/* Throws count Sparkle_init entities (entry 9 of PARTICLE_ANIMS) from the character's position (Fighter_getPosition),
 * each 256 either side of it in x, at a random velocity (x -8 to 7, y -50 to -19) from the character's
 * generator (Fighter_getScene), with color. */
void throwSparkles(Fighter *this, s32 arg1, u32 count, u32 color) {
    Fight *random;
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    random = Fighter_getScene(this);
    while (count--) {
        Fighter_getPosition(this, &position);
        position.vx += (Fight_random(random) & 0x1FF) - 0x100;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = Fight_random(random) & 0xF;
        vy = Fight_random(random) & 0x1F;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x32;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            Sparkle_init(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color, 9);
        }
        Fighter_addEntity(this, child);
    }
}

/* Throws count func_800C391C entities (entry 11 of PARTICLE_ANIMS) from the character's point (func_8007A650), each
 * 16 either side of it in x, at a random velocity (x -8 to 7, y -20 to -13), with color. */
void func_800C7E1C(Fighter *this, s32 arg1, u32 count, u32 color) {
    Vec3 position;
    Vec3 velocity;
    s32 vx, vy;
    Entity *child;

    while (count--) {
        func_8007A650(this, &position);
        position.vx += (mersenneTwisterGenerate(&RANDOM) & 0x1F) - 0x10;
        /* The match depends on drawing both random words first and centring them in the stores. */
        vx = mersenneTwisterGenerate(&RANDOM) & 0xF;
        vy = mersenneTwisterGenerate(&RANDOM) & 7;
        velocity.vx = vx - 8;
        velocity.vy = vy - 0x14;
        velocity.vz = 0;
        child = allocEntityMemory(0x5C);
        if (child != NULL) {
            func_800C391C(entityOperatorNewAt(0x5C, child), &position, &velocity, 0x1E, color);
        }
        Fighter_addEntity(this, child);
    }
}

/* The direction (ratan2, in 0 to 0xFFF) from from to to, on x and y. The match depends on to's x and y read first. */
s32 getAngleTo(Vec3 *from, Vec3 *to) {
    s32 x = to->vx;
    s32 y = to->vy;

    return ratan2(y - from->vy, x - from->vx) & 0xFFF;
}

INCLUDE_ASM("game/nonmatchings/game_0i", fireItemShot);

INCLUDE_ASM("game/nonmatchings/game_0i", fireItemShots);

/* Puffs a grey particle of SPIN_PARTICLE_VTABLE (0x80 big) from the character's point (func_8007A650), moved by -32 to
 * 31 in x and y, at a random velocity (x and y -32 to 31): its frame is 0, 1 or 2 for the kind 1, 2 or 3 of 0x478, none
 * for any other kind. */
void func_800C811C(Fighter *this) {
    VECTOR position;
    VECTOR velocity;
    Entity *entity;

    func_8007A650(this, (Vec3 *)&position);
    position.vx += (mersenneTwisterGenerate(&RANDOM) & 0x3F) - 0x20;
    position.vy += (mersenneTwisterGenerate(&RANDOM) & 0x3F) - 0x20;
    velocity.vx = (mersenneTwisterGenerate(&RANDOM) & 0x3F) - 0x20;
    velocity.vy = (mersenneTwisterGenerate(&RANDOM) & 0x3F) - 0x20;
    velocity.vz = 0;
    switch (this->unk478.kind) {
    case 1:
        entity = NEW_ENTITY(0x58, func_8006D574_init, &position, &velocity, 0x80, 0);
        break;
    case 2:
        entity = NEW_ENTITY(0x58, func_8006D574_init, &position, &velocity, 0x80, 1);
        break;
    case 3:
        entity = NEW_ENTITY(0x58, func_8006D574_init, &position, &velocity, 0x80, 2);
        break;
    default:
        return;
    }
    Fighter_addEntity(this, entity);
}

void ItemSpawner_countSpawnPoints(ItemSpawner *this);

/* Constructs the item spawner of scene with its spawn points: empties ITEM_SLOTS, then counts the points. The
 * match depends on the table read into a local after the count is cleared and on the slots cleared from the
 * last. */
ItemSpawner *ItemSpawner_init(ItemSpawner *this, Fight *scene, void *spawnPoints, s32 arg3) {
    ItemSlots *table;
    s32 i;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ITEM_SPAWNER_VTABLE;
    this->slots = &ITEM_SLOTS;
    this->scene = scene;
    this->spawnPoints = spawnPoints;
    this->unk20 = arg3;
    this->unk2C = 0;
    this->spawnTimer = 90;
    this->healCooldown = 300;
    this->unk38 = 0;
    ITEM_SLOTS.count = 0;
    table = this->slots;
    for (i = 15; i >= 0; i--) {
        table->slots[i].flags = 0;
    }
    ItemSpawner_countSpawnPoints(this);
    return this;
}

/* The destructor of the entity class of vtable ITEM_SPAWNER_VTABLE, which ItemSpawner_init installs. */
void ItemSpawner_destroy(Entity *this, s32 flags) {
    this->vtable = &ITEM_SPAWNER_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", ItemSpawner_tick);

/* Counts the spawn points, up to the one whose first halfword is -1. */
void ItemSpawner_countSpawnPoints(ItemSpawner *this) {
    ItemSpawnPoint *list = this->spawnPoints;

    this->spawnPointCount = 0;
    if (list != NULL) {
        while (list[this->spawnPointCount].unk0 != -1) {
            this->spawnPointCount++;
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8520);

INCLUDE_ASM("game/nonmatchings/game_0i", ItemSpawner_spawnItem);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8658);

/* Takes an item off the spawner: one fewer in ITEM_SLOTS, its slot's flags cleared, and the spawn timer back
 * to 90. */
void ItemSpawner_removeItem(ItemSpawner *this, Item *object) {
    this->spawnTimer = 90;
    this->slots->count--;
    this->slots->slots[object->slot].flags = 0;
}

INCLUDE_ASM("game/nonmatchings/game_0i", rotNclip3);

/* An item's listener's hit (slot 0x08): the item is touched (Item_callTouch). */
void ItemHitListener_onHit(ItemHitListener *this, s32 arg) {
    Item_callTouch(this->item, arg);
}

INCLUDE_ASM("game/nonmatchings/game_0i", Item_init);

extern EntityVtable ITEM_VTABLE;

/* The destructor of the entity class of vtable ITEM_VTABLE, which Item_init installs. */
void Item_destroy(Item *this, s32 flags) {
    ListNode *node;

    this->entity.vtable = &ITEM_VTABLE;
    /* the members' destructors, inlined in reverse order: the one at 0x60 first */
    this->listener.base.vtable = COLLISION_OWNER_VTABLE;
    this->part.vtable = COLLISION_SHAPE_VTABLE;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.link;
    this->part.link.next->prev = this->part.link.prev;
    node->prev->next = this->part.link.next;
    this->part.link.next = node;
    this->part.link.prev = node;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", Item_tick);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8B0C);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8C40);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8CD0);

/* Runs the item's touch slot (0x20, Item_touch) with arg. */
void Item_callTouch(Unk800C8DFC *object, s32 arg) {
    object->vtable->touch.func((u8 *)object + object->vtable->touch.delta, arg);
}

INCLUDE_ASM("game/nonmatchings/game_0i", Item_touch);

/* entityGetKind of the entity at 0x14 of arg, or 0 when there is none. */
s32 getToucherKind(void *this, CollisionSphere *arg) {
    s32 result = 0;

    if (arg->entity) {
        result = entityGetKind(arg->entity);
    }
    return result;
}

/* Stops the part's motion: 0x7C..0x88 cleared, then 0x7C set to +-0x133 by the sign of its x speed, which is cleared
 * with the rest. */
void func_800C8F34(Item *this) {
    s32 speed = this->part.hitKnockback.vx;

    this->unk7C = 0;
    this->unk80 = 0;
    this->unk84 = 0;
    this->unk88 = 0;
    if (speed > 0) {
        this->unk7C = 0x133;
    }
    if (this->part.hitKnockback.vx < 0) {
        this->unk7C = -0x133;
    }
    this->part.hitKnockback.vx = 0;
    this->part.hitKnockback.vy = 0;
    this->part.hitKnockback.vz = 0;
    this->part.hitKnockbackPad = 0;
}

/* Whether Fighter_isAttackIdle is false for the target (a chara). */
s32 func_800C8F7C(Item *this, Entity *target) {
    return Fighter_isAttackIdle((Fighter *)target) ^ 1;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C8F9C);

/* Copies the item's position into its slot of ITEM_SLOTS, when it came from a spawner. */
void Item_syncSlot(Item *this) {
    if (this->spawner) {
        ItemSlot *slot = &ITEM_SLOTS.slots[this->slot];

        slot->x = this->position.vx;
        slot->y = this->position.vy;
        slot->z = this->position.vz;
    }
}

/* Sets bits in the flags of the item's slot of ITEM_SLOTS, when it came from a spawner. */
void Item_setSlotFlags(Item *this, u8 bits) {
    if (this->spawner) {
        ITEM_SLOTS.slots[this->slot].flags |= bits;
    }
}

/* Clears bits in the flags of the item's slot of ITEM_SLOTS, when it came from a spawner. */
void Item_clearSlotFlags(Item *this, u8 bits) {
    if (this->spawner) {
        ITEM_SLOTS.slots[this->slot].flags &= ~bits;
    }
}

/* The items' draw: a capsule (func_800C9288) for kinds under 11, a model (func_800C9140) after. */
void Item_draw(Item *this, s32 arg) {
    if (this->kind < 11) {
        func_800C9288(this, arg);
    } else {
        func_800C9140(this, arg);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D874);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9140);

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D894);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D8A4);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D8C4);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9288);

/* Gives the fighter what the item's kind holds, unless the round is over: 500 score, then by kind 0 to 13 a
 * change to its side's gauge or life, or an item effect (Fighter.itemEffect, with its HUD icon), with a sound;
 * kinds 0 to 10 then show their popup. */
void Item_giveTo(Item *this, Fighter *chara) {
    if (Fight_isRoundOver(Fighter_getScene(chara)) != 0) {
        return;
    }
    Fighter_addScore(chara, 500);
    switch (this->kind) {
    case 0:
        Fighter_addGauge(chara, 0x2000);
        throwSparkles(chara, 0, 10, 0xC8C8C8);
        soundControlPlaySound(&SOUND_CONTROL, 0x49, 0);
        break;
    case 1:
        Fighter_addGauge(chara, -0x2000);
        throwSparkles(chara, 0, 15, 0xC82020);
        soundControlPlaySound(&SOUND_CONTROL, 0x4A, 0);
        break;
    case 2:
        Fighter_addGauge(chara, 0x10000);
        throwSparkles(chara, 0, 15, 0xFFFFFF);
        soundControlPlaySound(&SOUND_CONTROL, 0x49, 0);
        break;
    case 3:
        soundControlPlaySound(&SOUND_CONTROL, 0x49, 0);
        Fighter_evolveIfReady(chara);
        break;
    case 4:
        Fighter_startItemEffect(chara, 1, 300, 0x200);
        showItemEffectIcon(chara, 4);
        soundControlPlaySound(&SOUND_CONTROL, 0x49, 0);
        break;
    case 5:
        Fighter_startItemEffect(chara, 2, 300, 0x80);
        showItemEffectIcon(chara, 5);
        soundControlPlaySound(&SOUND_CONTROL, 0x49, 0);
        break;
    case 6:
        Fighter_startItemEffect(chara, 3, 150, 0);
        showItemEffectIcon(chara, 6);
        soundControlPlaySound(&SOUND_CONTROL, 0x4B, 0);
        break;
    case 7:
        Fighter_startItemEffect(chara, 3, 240, 0);
        showItemEffectIcon(chara, 7);
        soundControlPlaySound(&SOUND_CONTROL, 0x4B, 0);
        break;
    case 8:
        Fighter_startItemEffect(chara, 4, 300, 0);
        showItemEffectIcon(chara, 8);
        soundControlPlaySound(&SOUND_CONTROL, 0x4E, 0);
        break;
    case 9:
        Fighter_startItemEffect(chara, 5, 300, 0);
        showItemEffectIcon(chara, 9);
        soundControlPlaySound(&SOUND_CONTROL, 0x4E, 0);
        break;
    case 10:
        Fighter_startItemEffect(chara, 6, 300, 0);
        showItemEffectIcon(chara, 10);
        soundControlPlaySound(&SOUND_CONTROL, 0x4E, 0);
        break;
    case 11:
    case 12:
    case 13:
        Fighter_addHp(chara, 0x2000);
        throwSparkles(chara, 0, 10, 0x20C820);
        soundControlPlaySound(&SOUND_CONTROL, 0x4C, 0);
        return;
    case 14:
    default:
        return;
    }
    showItemPopup(chara, this->kind);
}

/* Runs the fighter's item effect (Fighter.itemEffect): counts it down and clears it at 0; otherwise, unless the
 * round is over, emits the effect of its kind (1 to 6) every 16, 4 or 2 frames. */
void updateItemEffect(Fighter *this) {
    s32 timer = this->itemEffect.timer;

    if (timer != 0) {
        timer--;
        this->itemEffect.timer = timer;
        if (timer == 0) {
            Fighter_clearItemEffect(this);
        } else if (Fight_isRoundOver(Fighter_getScene(this)) == 0) {
            switch (this->itemEffect.kind) {
            case 1:
                if ((this->itemEffect.timer & 0xF) == 0) {
                    func_800C7C08(this, 0, 0x80, 0x80, 0x80);
                }
                break;
            case 2:
                if ((this->itemEffect.timer & 0xF) == 0) {
                    func_800C7C08(this, 1, 0x20, 0x20, 0x80);
                }
                break;
            case 3:
                if ((this->itemEffect.timer & 0x3) == 0) {
                    func_800C7E1C(this, 0, 1, 0x808080);
                }
                break;
            case 4:
                if ((this->itemEffect.timer & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0x2020FF);
                }
                break;
            case 5:
                if ((this->itemEffect.timer & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0xFF2020);
                }
                break;
            case 6:
                if ((this->itemEffect.timer & 0x1) == 0) {
                    func_800C7C9C(this, 0, 0x20FF20);
                }
                break;
            }
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9938);

void func_800C811C(Fighter *this);

/* Runs the chara's timed thing at 0x478: counts it down and clears it at 0, or as soon as func_80079E2C is not
 * 13, 14, 16 or 17; else on even frames, while its kind is 1 to 3, calls func_800C811C. */
void func_800C99B0(Fighter *this) {
    s32 timer = this->unk478.timer;

    if (timer != 0) {
        timer--;
        this->unk478.timer = timer;
        if (timer == 0) {
            func_80077508(this);
            return;
        }
        if (Fight_isRoundOver(Fighter_getScene(this))) {
            return;
        }
        switch (func_80079E2C(this)) {
        default:
            func_80077508(this);
            break;
        case 13:
        case 14:
        case 16:
        case 17:
            /* The match depends on the nested tests: one && condition is folded into an unsigned range test. */
            if (this->unk478.kind < 4) {
                if (this->unk478.kind > 0 && !(this->unk478.timer & 1)) {
                    func_800C811C(this);
                }
            }
            break;
        }
    }
}

/* Constructs an entity of D_8006DB20: Item_init with the same arguments. */
Entity *func_800C9A88(Entity *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                      s32 arg6) {
    Item_init(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DB20;
    return this;
}

/* The first method of the class of virtual table D_8006DB20: Item_tick. */
void func_800C9AD4(void *this) {
    Item_tick(this);
}

/* The entity class of virtual table D_8006DAF0 (func_800C9AF4), derived from Item_init's. */
typedef struct Unk800C9AF4 {
    /* 0x00 */ Item base;
    /* 0xC4 */ s32 unkC4;
} Unk800C9AF4;

/* Constructs an entity of D_8006DAF0: Item_init with the same arguments, 0xB8 moved by 0x800, then
 * Item_setSlotFlags with bit 1. */
Unk800C9AF4 *func_800C9AF4(Unk800C9AF4 *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                           s32 arg6) {
    Item_init(&this->base.entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->base.entity.vtable = &D_8006DAF0;
    this->unkC4 = 0;
    this->base.unkB8 += 0x800;
    Item_setSlotFlags(&this->base, 2);
    return this;
}

/* The first method of the class of virtual table D_8006DAF0: turns 0xB8 by 0x200 (of 0x1000) while 0xC4 is set and
 * it is not 0, then Item_tick. */
void func_800C9B5C(Unk800C9AF4 *this) {
    if (this->unkC4 && this->base.unkB8) {
        this->base.unkB8 = (this->base.unkB8 + 0x200) & 0xFFF;
    }
    Item_tick(this);
}

void Item_touch(Item *this, CollisionSphere *arg);
void func_800C9C58(Unk800C9AF4 *this);

/* The item box's touch (slot 0x20 of D_8006DAF0): while closed (0xC4 and 0xB4 clear), a touch that is not an
 * item's, nor a fighter's that func_800C8F7C refuses, opens it (func_800C9C58); once open, Item_touch when 0xB8
 * is 0. */
void func_800C9BA4(Unk800C9AF4 *this, CollisionSphere *arg) {
    if (this->unkC4 == 0 && this->base.unkB4 == 0) {
        /* The match depends on a switch over the entity kinds with three cases: the game's compare tree has
         * 0x20000 at its root (tests ==, then > to the default arm, then 0x10000), which gcc builds only from a
         * third case above 0x20000 that shares the default arm. Its value is not in the code:
         * ENTITY_KIND_FIGHTER_OBJECT is a guess. Item_touch has the same tree. */
        switch (getToucherKind(this, arg)) {
        case ENTITY_KIND_ITEM:
            break;
        case ENTITY_KIND_FIGHTER:
            if (!func_800C8F7C(&this->base, getHitFighter(arg))) {
                break;
            }
            /* fallthrough */
        case ENTITY_KIND_FIGHTER_OBJECT:
        default:
            func_800C9C58(this);
            break;
        }
    } else if (this->base.unkB8 == 0) {
        Item_touch(&this->base, arg);
    }
}

/* Opens the item box: sets 0xC4, disables its part, 0xB0 to 30, 150 more frames of life, and clears its
 * slot's closed bit (Item_clearSlotFlags with 2). */
void func_800C9C58(Unk800C9AF4 *this) {
    this->unkC4 = 1;
    this->base.unkB0 = 30;
    this->base.part.flags |= COLLISION_SHAPE_DISABLED;
    this->base.life += 150;
    Item_clearSlotFlags(&this->base, 2);
}

/* Constructs an entity of D_8006DAC0: Item_init with the same arguments. */
Entity *func_800C9CA0(Entity *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                      s32 arg6) {
    Item_init(this, arg1, arg2, arg3, arg4, arg5, arg6);
    this->vtable = &D_8006DAC0;
    return this;
}

/* The first method of the class of virtual table D_8006DAC0: Item_tick. */
void func_800C9CEC(void *this) {
    Item_tick(this);
}

/* The entity class of virtual table D_8006DA90 (func_800C9D0C), derived from Item_init's: four 0x24-byte
 * records from 0xC8. */
typedef struct Unk800C9D0CItem {
    /* 0x00 */ s16 unk0;
    /* 0x02 */ u8 unk2[0x24 - 0x2];
} Unk800C9D0CItem;

typedef struct Unk800C9D0C {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u8 unk18[0xC4 - 0x18];
    /* 0xC4 */ s32 unkC4;
    /* 0xC8 */ Unk800C9D0CItem unkC8[4];
} Unk800C9D0C;

/* Constructs an entity of D_8006DA90: Item_init with the same arguments, then clears the first halfword of
 * its four records, the last first. */
Unk800C9D0C *func_800C9D0C(Unk800C9D0C *this, ItemSpawner *arg1, s32 arg2, Fight *arg3, s32 arg4, Vec3 *arg5,
                           s32 arg6) {
    s32 i;

    Item_init(&this->entity, arg1, arg2, arg3, arg4, arg5, arg6);
    this->entity.vtable = &D_8006DA90;
    this->unkC4 = 0;
    /* The match depends on the loop running from the last record (the game clears 0x134 first). */
    for (i = 3; i >= 0; i--) {
        this->unkC8[i].unk0 = 0;
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800C9D74);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA028);

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CA1B4);

/* The HUD popup of an item taken (ItemPopup_init, virtual table ITEM_POPUP_VTABLE): it rises by the fighter's side
 * from (0x30 or 0x110, 0x130) to 0x78, waits 12 frames, and flies off. */
typedef struct ItemPopup {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 itemKind;
    /* 0x1C */ s16 x;
    /* 0x1E */ s16 y;
    /* 0x20 */ s32 frame;
} ItemPopup;

/* Constructs the popup of itemKind for fighter, by its side: x 0x30 for side 0, else 0x110, and y 0x130. */
ItemPopup *ItemPopup_init(ItemPopup *this, Fighter *fighter, s32 itemKind) {
    s16 x;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ITEM_POPUP_VTABLE;
    this->itemKind = itemKind;
    if (Fighter_getSide(fighter) == 0) {
        x = 0x30;
    } else {
        x = 0x110;
    }
    this->x = x;
    this->y = 0x130;
    this->frame = 0;
    return this;
}

/* The destructor of the entity class of vtable ITEM_POPUP_VTABLE, which ItemPopup_init installs. */
void ItemPopup_destroy(Entity *this, s32 flags) {
    this->vtable = &ITEM_POPUP_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The popup's tick: rises by 0x30 while it is below 0x79, and every frame after the 12th; deleted once it is
 * above -0x30. */
void ItemPopup_tick(ItemPopup *this) {
    if (this->y > 0x78) {
        this->y -= 0x30;
    }
    if (this->frame > 12) {
        this->y -= 0x30;
    }
    if (this->y < -0x30) {
        entityDelete(&this->entity);
    } else {
        this->frame++;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", ItemPopup_draw);

/* The HUD icon of a fighter's item effect (ItemEffectIcon_init, virtual table ITEM_EFFECT_ICON_VTABLE): at
 * (0x2C or 0x114, 0xAC) by the side of the fighter it keeps the handle of, while that fighter lives. */
typedef struct ItemEffectIcon {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 charaHandle;
    /* 0x1C */ s32 itemKind;
    /* 0x20 */ s16 x;
    /* 0x22 */ s16 y;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 effectKind; /* the fighter's itemEffect.kind */
    /* 0x2C */ s32 unk2C;
} ItemEffectIcon;

/* Constructs the effect icon of itemKind for fighter, by its side: x 0x2C for side 0, else 0x114, and y 0xAC. */
ItemEffectIcon *ItemEffectIcon_init(ItemEffectIcon *this, Fighter *fighter, s32 itemKind) {
    s16 x;

    entityInit(&this->entity, 0);
    this->entity.vtable = &ITEM_EFFECT_ICON_VTABLE;
    this->charaHandle = entityGetHandle(&fighter->entity);
    this->itemKind = itemKind;
    if (Fighter_getSide(fighter) == 0) {
        x = 0x2C;
    } else {
        x = 0x114;
    }
    this->x = x;
    this->y = 0xAC;
    this->unk24 = 0;
    this->effectKind = fighter->itemEffect.kind;
    this->unk2C = 0;
    return this;
}

/* The destructor of the entity class of vtable ITEM_EFFECT_ICON_VTABLE, which ItemEffectIcon_init installs. */
void ItemEffectIcon_destroy(Entity *this, s32 flags) {
    this->vtable = &ITEM_EFFECT_ICON_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", ItemEffectIcon_tick);

INCLUDE_ASM("game/nonmatchings/game_0i", ItemEffectIcon_draw);

/* The entity class of virtual table D_8006DA18 (func_800CAACC): follows the point (func_8007A650) of the chara it
 * keeps the handle of for 8 frames, while 0x3C and 0x40 move in opposite directions (which by unk1C). */
typedef struct Unk800CAACC {
    /* 0x00 */ Entity entity;
    /* 0x18 */ u32 charaHandle;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ Vec3 position;
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 frame;
} Unk800CAACC;

/* Constructs an entity of D_8006DA18 for chara at its point, 0x3C and 0x40 starting at 0 and
 * 0x80 when arg2 is 0, else at 0x140 and 0. */
Unk800CAACC *func_800CAACC(Unk800CAACC *this, Fighter *chara, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &D_8006DA18;
    this->charaHandle = entityGetHandle(&chara->entity);
    this->unk1C = arg2;
    this->unk20 = arg3;
    this->unk24 = arg4;
    this->unk28 = arg5;
    func_8007A650(chara, &this->position);
    this->frame = 0;
    if (this->unk1C == 0) {
        this->unk3C = 0;
        this->unk40 = 0x80;
    } else {
        this->unk3C = 0x140;
        this->unk40 = 0;
    }
    return this;
}

/* The destructor of the entity class of vtable D_8006DA18, which func_800CAACC installs. */
void func_800CAB94(Entity *this, s32 flags) {
    this->vtable = &D_8006DA18;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The update of Unk800CAACC: deletes it once its chara is gone or after 8 frames, else moves 0x3C by 0x28 and 0x40
 * by 0x10 (in opposite directions, by unk1C) and takes the chara's point again. */
void func_800CABE8(Unk800CAACC *this) {
    Fighter *chara = (Fighter *)getEntityByHandle(this->charaHandle);

    if (chara == NULL || this->frame >= 8) {
        entityDelete(&this->entity);
    } else {
        if (this->unk1C == 0) {
            this->unk3C += 0x28;
            this->unk40 -= 0x10;
        } else {
            this->unk3C -= 0x28;
            this->unk40 += 0x10;
        }
        func_8007A650(chara, &this->position);
        this->frame++;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CACB4);

/* Constructs an entity of the class of virtual table D_8006D9F0, derived from func_800C3CFC's at pos. */
Unk800C3CFC *func_800CAEF0(Unk800C3CFC *this, Vec3 *pos, s32 arg2, s32 arg3) {
    func_800C3CFC(this, pos, arg2, arg3);
    this->entity.vtable = &D_8006D9F0;
    this->unk50 = 8;
    return this;
}

/* The class of virtual tables ITEM_SHOT_VTABLE (and ITEM_SHOT_HIT_VTABLE for the base at 0x18), derived from
 * Projectile: its index picks the word passed to Projectile_init (ITEM_SHOT_ATTACKS) and the frame of the pieces
 * ItemShot_throwPiece throws. */
typedef struct ItemShot {
    /* 0x00 */ Projectile base;
    /* 0xC0 */ s32 index;
    /* 0xC4 */ s32 angle;
    /* 0xC8 */ s32 pieceFrame; /* the frame of the pieces it throws, 0 to 3 */
} ItemShot;

extern u32 ITEM_SHOT_ATTACKS[];
extern VtableEntry ITEM_SHOT_HIT_VTABLE[];
extern EntityVtable ITEM_SHOT_VTABLE;

/* Constructs Projectile_init's entity with 0xC0, entry index of ITEM_SHOT_ATTACKS, 0xA00 and 2, then the virtual tables
 * ITEM_SHOT_VTABLE (and ITEM_SHOT_HIT_VTABLE for the base at 0x18). */
ItemShot *ItemShot_init(ItemShot *this, Fighter *chara, Vec3 *position, Vec3 *velocity, s32 index,
                           s32 angle) {
    Projectile_init(&this->base, chara, position, velocity, 0xC0, ITEM_SHOT_ATTACKS[index], 0xA00, 2);
    this->base.parts.base.vtable = ITEM_SHOT_HIT_VTABLE;
    this->base.parts.entity.vtable = &ITEM_SHOT_VTABLE;
    this->index = index;
    this->angle = angle & 0xFFF;
    this->pieceFrame = 0;
    return this;
}
void ItemShot_throwPiece(ItemShot *this);

/* The update of ItemShot: throws a piece (ItemShot_throwPiece), steps 0xC8 through 0 to 3, then the base's update. */
void ItemShot_tick(ItemShot *this, s32 arg) {
    ItemShot_throwPiece(this);
    this->pieceFrame = (this->pieceFrame + 1) & 3;
    Projectile_tick(&this->base, arg);
}

/* Projectile_hit for a derived class of Projectile: only when getHitFighter finds an entity in sum, which also
 * gets Fighter_addScore with 50 on its character (Fighter_getOpponent). */
void ItemShot_hit(Projectile *this, CollisionSphere *sum) {
    Entity *target;

    Hit_apply(&this->parts.base, sum);
    Projectile_spawnBurst(this, Projectile_getBurstKind(this));
    target = getHitFighter(sum);
    if (target != NULL) {
        Fighter_addScore(Fighter_getOpponent((Fighter *)target), 50);
        Projectile_spawnHitSpark(this, target);
    }
    entityDelete(&this->parts.entity);
}

INCLUDE_ASM("game/nonmatchings/game_0i", ItemShot_draw);

struct Unk800CB338 *func_800CB338(struct Unk800CB338 *this, VECTOR *position, VECTOR *velocity, s32 index,
                                  s32 angle);

/* Throws a piece (func_800CB338, 0x60 bytes) from the position, a quarter of the velocity backwards in x and y with
 * up to 8 either way at random, as a sibling. */
void ItemShot_throwPiece(ItemShot *this) {
    VECTOR velocity;
    Entity *child;
    s32 vx;

    /* The match depends on vel.vx read into a local before the other words are copied (the game loads it first). */
    vx = this->base.vel.vx;
    velocity.vz = this->base.vel.vz;
    velocity.pad = this->base.velPad;
    velocity.vx = -vx >> 2;
    velocity.vy = -this->base.vel.vy >> 2;
    velocity.vx += (mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    velocity.vy += (mersenneTwisterGenerate(&RANDOM) & 0xF) - 8;
    child = allocEntityMemory(0x60);
    if (child != NULL) {
        func_800CB338(entityOperatorNewAt(0x60, child), (VECTOR *)&this->base.pos, &velocity, this->index,
                      this->angle);
    }
    entityAddSibling(&this->base.parts.entity, child);
}

/* The entity class of virtual table D_8006D980 (func_800CB338): a sprite of Particle with a frame of
 * ITEM_SHOT_PIECE_ANIMS and an angle. */
typedef struct Unk800CB338 {
    /* 0x00 */ Particle base;
    /* 0x58 */ s32 index;
    /* 0x5C */ s32 angle;
} Unk800CB338;

extern u16 ITEM_SHOT_PIECE_ANIMS[][10];
extern EntityVtable D_8006D980;

INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_SHOT_PIECE_ANIMS);

/* Constructs an entity of D_8006D980: Particle_init with frame index of ITEM_SHOT_PIECE_ANIMS, size 0x80 and color
 * 0x404040. */
Unk800CB338 *func_800CB338(Unk800CB338 *this, VECTOR *position, VECTOR *velocity, s32 index, s32 angle) {
    /* The match depends on the table's address taken into a local first (the game computes it before index * 20). */
    u16 (*frames)[10] = ITEM_SHOT_PIECE_ANIMS;

    Particle_init(&this->base, frames[index], position, velocity, 0x80, 0x404040, 0);
    this->base.entity.vtable = &D_8006D980;
    this->index = index;
    this->angle = angle;
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0i", func_800CB3CC);

/* The destructor of the class of virtual table ITEM_HIT_LISTENER_VTABLE: CollisionOwner's, inlined. */
void ItemHitListener_destroy(CollisionOwner *this, s32 flags) {
    this->vtable = COLLISION_OWNER_VTABLE;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Constructs an item's hit listener for item (Item_init builds the same inline). */
ItemHitListener *ItemHitListener_init(ItemHitListener *this, Unk800C8DFC *item) {
    this->base.vtable = ITEM_HIT_LISTENER_VTABLE;
    this->item = item;
    return this;
}

/* The destructor of the entity class of vtable D_8006DB20, which func_800C9A88 installs. */
void func_800CB554(Item *this, s32 flags) {
    Item_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAF0, which func_800C9AF4 installs. */
void func_800CB59C(Item *this, s32 flags) {
    Item_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DAC0, which func_800C9CA0 installs. */
void func_800CB5E4(Item *this, s32 flags) {
    Item_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006DA90, which func_800C9D0C installs. */
void func_800CB62C(Item *this, s32 flags) {
    Item_destroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The destructor of the entity class of vtable D_8006D9F0, which func_800CAEF0 installs. */
void func_800CB674(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* A copy of the destructor of the UnkPartsEntity38 class of vtables PROJECTILE_VTABLE and PROJECTILE_HIT_VTABLE,
 * which Projectile_init installs: game has 11 identical ones. */
void ItemShot_destroy(UnkPartsEntity38 *this, s32 flags) {
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

/* A copy of the destructor of the entity class of vtable PARTICLE_VTABLE, which Particle_init,
 * Particle_initById and Particle_initStill install: game has 15 identical ones. */
void func_800CB75C(Entity *this, s32 flags) {
    this->vtable = &PARTICLE_VTABLE;
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D980);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_SHOT_HIT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_SHOT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006D9F0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA18);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_EFFECT_ICON_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_POPUP_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DA90);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DAC0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DAF0);
INCLUDE_RODATA("game/nonmatchings/game_0i", D_8006DB20);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_HIT_LISTENER_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_SPAWNER_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0i", ITEM_SHOT_ATTACKS);
