/* One side of the fight (BattleSide): its two forms, the one fighting, and the side's life, evolution gauge and
 * stun, which both forms share. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"

INCLUDE_ASM("game/nonmatchings/game_0e", BattleSide_init);

/* Takes the side's fighter out of its entity list, and its evolution when it has one. */
void BattleSide_unlinkCharacters(BattleSide *this) {
    listRemove(&this->fighter->entity.link);
    if (this->fighter != this->evolution) {
        listRemove(&this->evolution->entity.link);
    }
}

extern VtableEntry BATTLE_SIDE_VTABLE[];

/* BattleSide's destructor: deletes the side's fighter, and its evolution when it has one. */
void BattleSide_destroy(BattleSide *this, s32 flags) {
    this->vtable = BATTLE_SIDE_VTABLE;
    entityDelete(&this->fighter->entity);
    /* The match depends on this operand order (0 loads first). */
    if (this->fighter != this->evolution) {
        entityDelete(&this->evolution->entity);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Unless frozen: takes amount off the life (down to 0) and, when the fighter can evolve (Fighter_canEvolve or
 * func_800777BC), adds half of it to the gauge. */
void BattleSide_takeDamage(BattleSide *this, s32 amount) {
    if (this->frozen == 0) {
        if (Fighter_canEvolve(this->current) || func_800777BC(this->current)) {
            BattleSide_addGauge(this, amount / 2);
        }
        /* The match depends on amount taking the result (a new variable swaps the saved registers). */
        amount = this->life - amount;
        if (amount < 0) {
            amount = 0;
        }
        this->life = amount;
    }
}

/* Adds amount to the life, kept between 0 and lifeMax. */
void BattleSide_addLife(BattleSide *this, s32 amount) {
    amount += this->life;
    if (amount > this->lifeMax) {
        amount = this->lifeMax;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->life = amount;
}

/* Unless frozen, adds amount << 12 to the stun count; when that wraps around, it takes the params' stunLimit
 * instead (the value func_8007F1C0 compares it with). */
void func_8007F124(BattleSide *this, s32 amount) {
    s32 value;

    if (this->frozen == 0) {
        value = this->stun + (amount << 12);
        if (value < this->stun) {
            value = Fighter_getParams(this->current)->stunLimit;
        }
        this->stun = value;
    }
}

/* Fills the stun count up to the params' stunLimit. */
void func_8007F184(BattleSide *this) {
    this->stun = Fighter_getParams(this->current)->stunLimit;
}

/* Clears the stun count. */
void func_8007F1B8(BattleSide *this) {
    this->stun = 0;
}

/* Whether the stun count has reached the params' stunLimit. */
s32 func_8007F1C0(BattleSide *this) {
    return this->stun >= Fighter_getParams(this->current)->stunLimit;
}

/* Adds amount to the evolution gauge, kept between 0 and 0x10000 (full). */
void BattleSide_addGauge(BattleSide *this, s32 amount) {
    amount += this->gauge;
    if (amount > 0x10000) {
        amount = 0x10000;
    }
    if (amount < 0) {
        amount = 0;
    }
    this->gauge = amount;
}

void func_8007F7F4(BattleSide *this);

/* Each frame, unless frozen: the stun count wears off by 0x44 while it is under the params' stunLimit; when the
 * fighter's params have FIGHTER_PARAM_EVOLVED and func_800777BC does not hold, the gauge drains by 0x49, and once
 * it is empty and the fighter is not attacking, calls func_8007F7F4. */
void BattleSide_tickGauges(BattleSide *this) {
    Fighter *fighter;
    u16 bits;

    if (this->frozen == 0) {
        fighter = this->current;
        bits = Fighter_getParams(fighter)->flags;
        if (!func_8007F1C0(this)) {
            this->stun -= 0x44;
            if (this->stun < 0) {
                this->stun = 0;
            }
        }
        if ((bits & FIGHTER_PARAM_EVOLVED) && !func_800777BC(fighter)) {
            this->gauge -= 0x49;
            if (this->gauge < 0) {
                this->gauge = 0;
            }
            if (this->gauge <= 0 && Fighter_isAttackIdle(fighter)) {
                func_8007F7F4(this);
            }
        }
    }
}

INCLUDE_ASM("game/nonmatchings/game_0e", BattleSide_tick);

/* Freezes the side: its life, gauge and stun count stop changing. */
void BattleSide_freeze(BattleSide *this) {
    this->frozen = 1;
}

/* Whether the side may evolve now: the gauge is full, the character's params lack FIGHTER_PARAM_EVOLVED, the
 * fighter is not attacking, and it is not flinching, knocked down or out, getting up, dizzy or held. */
s32 BattleSide_canEvolve(BattleSide *this) {
    FighterParams *tables = FIGHTER_PARAMS;
    s32 state;

    if (tables[Fighter_getCharacter(this->current)].flags & FIGHTER_PARAM_EVOLVED) {
        return 0;
    }
    if (!Fighter_isAttackIdle(this->current)) {
        return 0;
    }
    state = Fighter_getBodyStateId(this->current);
    /* The match depends on one if per state: an || chain is folded into a range test (11 to 16). */
    if (state == FIGHTER_STATE_FLINCH) {
        return 0;
    }
    if (state == FIGHTER_STATE_AIR_FLINCH) {
        return 0;
    }
    if (state == FIGHTER_STATE_KNOCKED_DOWN) {
        return 0;
    }
    if (state == FIGHTER_STATE_KNOCKED_OUT) {
        return 0;
    }
    if (state == FIGHTER_STATE_GET_UP) {
        return 0;
    }
    if (state == FIGHTER_STATE_DIZZY) {
        return 0;
    }
    if (state == FIGHTER_STATE_HELD) {
        return 0;
    }
    return this->gauge > 0xFFFF;
}

/* When func_800777BC holds for the fighter, whether the gauge is full; else, when the character's params have
 * FIGHTER_PARAM_EVOLVED, whether the gauge is not empty. */
s32 func_8007F674(BattleSide *this) {
    FighterParams *tables = FIGHTER_PARAMS;
    u16 bits = tables[Fighter_getCharacter(this->current)].flags;
    s32 result;

    if (func_800777BC(this->current)) {
        return this->gauge > 0xFFFF;
    }
    /* The match depends on result starting at 0 (an if/else returns straight from $v0). */
    result = 0;
    if (bits & FIGHTER_PARAM_EVOLVED) {
        result = this->gauge != 0;
    }
    return result;
}

/* Asks for the evolution: gives the fighter a new handle (entityRenewHandle) and sets 0x30. */
void func_8007F6FC(BattleSide *this) {
    entityRenewHandle(&this->current->entity);
    this->unk30 = 1;
}

void func_8007F7A0(BattleSide *this);

/* When func_800777BC holds for the fighter and bit 1 of its params' flags is clear: func_8007F7A0, then 30 at 0x10
 * and 3 at 0x18. */
void func_8007F730(BattleSide *this) {
    Fighter *fighter = this->current;

    if (func_800777BC(fighter) && !(Fighter_getParams(fighter)->flags & 2)) {
        func_8007F7A0(this);
        this->unk10 = 30;
        this->unk18 = 3;
    }
}

/* Plays sound 0x800E and adds TransformFlash_init's entity (0x48 bytes) for this side as a sibling of the fighter. */
void func_8007F7A0(BattleSide *this) {
    soundControlPlaySound(&SOUND_CONTROL, 0x800E, 0);
    entityAddSibling(&this->current->entity, &TransformFlash_init(entityOperatorNew(0x48), this)->entity);
}

/* Unless swapDelay is still running: sets it to 16, plays sound 0x800F, adds func_800C4F74's entity (0x4A8 bytes)
 * for this side as a sibling of the fighter, renews the fighter's handle, calls func_80079B0C, func_80079B14 and
 * func_80079B1C with 30 on it, and clears the stun count (func_8007F1B8). */
void func_8007F7F4(BattleSide *this) {
    Fighter *fighter;

    if (this->swapDelay <= 0) {
        fighter = this->current;
        this->swapDelay = 16;
        soundControlPlaySound(&SOUND_CONTROL, 0x800F, 0);
        entityAddSibling(&fighter->entity, &func_800C4F74(entityOperatorNew(0x4A8), this)->entity);
        entityRenewHandle(&fighter->entity);
        func_80079B0C(fighter, 30);
        func_80079B14(fighter, 30);
        func_80079B1C(fighter, 30);
        func_8007F1B8(this);
    }
}

/* Swaps the fighting form for the other one (back to the first when it was the evolution); the new one takes over
 * from the old one (Fighter_takeOverFrom), and the old one gets a new handle. */
void BattleSide_swapCharacter(BattleSide *this) {
    Fighter *previous = this->current;
    Fighter *next = this->evolution;

    if (previous == next) {
        next = this->fighter;
    }
    this->current = next;
    Fighter_takeOverFrom(next, previous);
    entityRenewHandle(&previous->entity);
}

/* Returns the fighting form; no caller found. */
Fighter *BattleSide_getCharacter(BattleSide *this) {
    return this->current;
}

/* Returns the life; no caller found. */
s32 BattleSide_getLife(BattleSide *this) {
    return this->life;
}

/* Returns the most life the side can have; no caller found. */
s32 BattleSide_getLifeMax(BattleSide *this) {
    return this->lifeMax;
}

/* Returns the evolution gauge; no caller found. */
s32 BattleSide_getGauge(BattleSide *this) {
    return this->gauge;
}

/* Returns the stun count; no caller found. */
s32 func_8007F914(BattleSide *this) {
    return this->stun;
}

INCLUDE_RODATA("game/nonmatchings/game_0e", BATTLE_SIDE_VTABLE);
