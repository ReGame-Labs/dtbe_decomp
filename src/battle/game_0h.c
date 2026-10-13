/* The class of the constructor func_800C7024 (virtual tables D_8006D740 and D_8006D760), which nine
 * fighters derive from. */

#include "common.h"
#include "battle/game_classes.h"

INCLUDE_ASM("game/nonmatchings/game_0h", func_800C7024);

INCLUDE_ASM("game/nonmatchings/game_0h", func_800C71DC);

/* The second base's first method of Unk800C7024: func_8007BDF0 with sum, func_800C73A8 with what func_800C7358
 * picks, func_800C7474 with the entity func_80078878 finds in sum, then deletes itself. */
void func_800C72D8(Unk800C7024 *this, Unk80075EE4 *sum) {
    func_8007BDF0(&this->parts.base, sum);
    func_800C73A8(this, func_800C7358(this));
    func_800C7474(this, func_80078878(sum));
    entityDelete(&this->parts.entity);
}

/* Picks 3 for 0xA0 of 0 or 1 (and any other value), 4 for 2, 5 for 3. */
s32 func_800C7358(Unk800C7024 *this) {
    switch (this->unkA0) {
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

INCLUDE_ASM("game/nonmatchings/game_0h", func_800C73A8);

INCLUDE_ASM("game/nonmatchings/game_0h", func_800C7474);

INCLUDE_ASM("game/nonmatchings/game_0h", func_800C75FC);

/* A copy of the destructor of the UnkPartsEntity38 class of vtables D_8006D760 and D_8006D740,
 * which func_800C7024 installs: game has 11 identical ones. */
void func_800C7714(UnkPartsEntity38 *this, s32 flags) {
    ListNode *node;

    this->base.vtable = D_8006D740;
    this->entity.vtable = &D_8006D760;
    this->part.vtable = D_80065184;
    /* listRemove(&this->part.node), in the access order the game's compiler used */
    node = &this->part.node;
    this->part.node.next->prev = this->part.node.prev;
    node->prev->next = this->part.node.next;
    this->part.node.next = node;
    this->part.node.prev = node;
    this->base.vtable = D_80065164;
    entityDestroy(&this->entity, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

void func_800C77B4(void) {
}

INCLUDE_RODATA("game/nonmatchings/game_0h", D_8006D740);
INCLUDE_RODATA("game/nonmatchings/game_0h", D_8006D760);
