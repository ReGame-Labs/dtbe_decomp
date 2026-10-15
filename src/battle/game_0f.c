/* A fighter's states (FighterState and its subclasses): each one runs its slot of the fighter every frame, reads
 * the input, and turns itself into the next state in place. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"

/* functions of FighterState that constructors of its kin call */
void func_8007FCFC(FighterState *this);
void func_8007FED8(FighterState *this);
void func_8007FF1C(FighterState *this);

s32 func_8007F98C(FighterState *this);
FighterState *FighterStateJump_init(FighterState *this, s32 jumpsBefore);
void FighterAttackIdle_attack(FighterState *this, s32 attack);
/* the steps FighterAttackIdle_attack runs with the fighter's input (func_80081E18, 80081EEC and 80081FC4 ignore it) */
void func_80081B84(FighterState *this, InputHistory *input);
void func_80081E18(FighterState *this, InputHistory *input);
void func_80081EEC(FighterState *this, InputHistory *input);
void func_80081FC4(FighterState *this, InputHistory *input);
FighterState *FighterState_init(FighterState *this, s32 state);

/* the virtual tables the functions below install, and the data they read */
extern VtableEntry FIGHTER_ATTACK_IDLE_VTABLE[];
extern VtableEntry FIGHTER_STATE_GET_UP_VTABLE[];
extern VtableEntry D_80067E44[];
extern VtableEntry FIGHTER_STATE_VTABLE[];
extern VtableEntry FIGHTER_STATE_JUMP_VTABLE[];
extern VtableEntry FIGHTER_STATE_GROUND_VTABLE[];
extern VtableEntry D_800680E4[];
extern VtableEntry D_8006811C[];

/* A g++ 2.95 pointer to member function: index -1 for a non-virtual function, which is then func; else virtual,
 * entry index - 1 of the virtual table at vtableOffset in the object. The game keeps such constants in .rodata
 * (D_80067C54 and the ones around it, each {0xFFFF0000, function}). */
typedef struct MemberFunction {
    /* 0x0 */ s16 delta; /* how far to move `this` */
    /* 0x2 */ s16 index;
    union {
        /* 0x4 */ void (*func)();
        /* 0x4 */ s16 vtableOffset;
    } u;
} MemberFunction;

typedef struct FighterStateJump {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 jumpCount; /* the jumps made: a second one while it is below 2 */
    /* 0x14 */ s32 frames; /* frames since the jump */
} FighterStateJump;

/* The state of virtual table D_80067E44 (func_80081874): a FighterState with a word at 0x10. */
typedef struct Unk80081874 {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 unk10;
} Unk80081874;

FighterState *func_80080778(FighterState *this, s32 state);
void FighterState_playClip(FighterState *this, s32 clip, s32 blendFrames);
void func_8007FC50(FighterState *this, s32 clip, s32 blendFrames);
void FighterStateFlinch_init(FighterState *this, s32 hitBits);
/* Constructs, in place, the attack `state` (virtual table FIGHTER_STATE_ATTACK_VTABLE, on func_80081A20's) with
 * its step function, which it then calls. */
FighterState *FighterStateAttack_init(FighterState *this, s32 state, MemberFunction step);
/* the step functions of FighterStateAttack_init's states (pointers to member functions) */
extern MemberFunction D_80067C44;
extern MemberFunction D_80067C4C;
extern MemberFunction D_80067C54;
extern MemberFunction D_80067C5C;
extern MemberFunction D_80067C64;
extern MemberFunction D_80067C6C;
extern MemberFunction D_80067C74;
extern MemberFunction D_80067CCC;
extern MemberFunction D_80067CD4;
extern MemberFunction D_80067CDC;
extern MemberFunction D_80067CE4;
extern MemberFunction D_80067CEC;
extern MemberFunction D_80067CF4;
/* Constructs, in place, the attack chain's `state` (virtual table D_80067D94, on FighterState_init's) with its
 * step function at 0x14. */
FighterState *func_800823EC(FighterState *this, s32 state, MemberFunction step);
extern MemberFunction D_80067D04;
extern MemberFunction D_80067D0C;
extern VtableEntry D_80067D14[];
extern VtableEntry D_80067D54[];
FighterState *func_8008230C(FighterState *this);
FighterState *func_80080FDC(FighterState *this, s32 state);
FighterState *func_80081200(FighterState *this, s32 state);
FighterState *func_80082690(FighterState *this);
FighterState *func_800826E8(FighterState *this);

/* Constructs a state with its FIGHTER_STATE_* id, from frame 0 (virtual table FIGHTER_STATE_VTABLE). */
FighterState *FighterState_init(FighterState *this, s32 state) {
    this->vtable = FIGHTER_STATE_VTABLE;
    this->state = state;
    this->frame = 0;
    return this;
}

/* Returns the fighter's body state. */
FighterState *FighterState_getBodyState(FighterState *this) {
    return this->fighter->bodyState;
}

/* Returns the fighter's attack state. */
FighterState *FighterState_getAttackState(FighterState *this) {
    return this->fighter->attackState;
}

/* Whether the fighter attacks with nothing (its attack state is FIGHTER_STATE_IDLE). */
s32 FighterState_isAttackIdle(FighterState *this) {
    return FighterState_getAttackState(this)->state == FIGHTER_STATE_IDLE;
}

/* Whether the fighter's flag of func_80079AF8 is clear. */
s32 func_8007F98C(FighterState *this) {
    /* The match depends on the ^ 1 after the mask: ! xors before masking. */
    return ((this->fighter->flags >> FIGHTER_FLAG_80079AF8_BIT) & 1) ^ 1;
}

/* Whether the fighter's last input (InputHistory_getHeld) holds the guard button (0x4). */
s32 FighterState_isGuardHeld(FighterState *this) {
    return (InputHistory_getHeld(Fighter_getInput(this->fighter)) & 0x4) != 0;
}

FighterState *FighterStateGround_initCrouch(FighterState *this);
FighterState *FighterStateGround_initGuard(FighterState *this);
FighterState *FighterStateGround_initCrouchGuard(FighterState *this);

/* Guards or stops guarding by the guard button (FighterState_isGuardHeld), in place, unless attacking: never in
 * the air, flinching, down or dizzy; held, a crouch becomes a crouching guard and any other state but the guards a
 * guard; not held, a crouching guard becomes a crouch and a guard becomes idle. Returns whether the state
 * changed. */
s32 FighterState_updateGuard(FighterState *this) {
    s32 state;

    if (!FighterState_isAttackIdle(this)) {
        return 0;
    }
    state = this->state;
    /* The match depends on the switch of case ranges (the game's slti tree). */
    switch (state) {
    case FIGHTER_STATE_JUMP ... FIGHTER_STATE_FALL:
    case FIGHTER_STATE_FLINCH ... FIGHTER_STATE_DIZZY:
        return 0;
    }
    if (FighterState_isGuardHeld(this)) {
        if (state == FIGHTER_STATE_GUARD || state == FIGHTER_STATE_CROUCH_GUARD) {
            return 0;
        }
        if (state == FIGHTER_STATE_CROUCH) {
            FighterStateGround_initCrouchGuard(this);
            return 1;
        }
        FighterStateGround_initGuard(this);
        return 1;
    }
    if (state == FIGHTER_STATE_CROUCH_GUARD) {
        FighterStateGround_initCrouch(this);
        return 1;
    }
    if (state == FIGHTER_STATE_GUARD) {
        FighterStateGround_init(this, FIGHTER_STATE_IDLE);
        return 1;
    }
    return 0;
}

/* Whether the fighter's last input (InputHistory_getHeld) holds PAD_DOWN. */
s32 FighterState_isDownHeld(FighterState *this) {
    return (InputHistory_getHeld(Fighter_getInput(this->fighter)) & PAD_DOWN) != 0;
}

/* Crouches or stands up by PAD_DOWN (FighterState_isDownHeld), in place: held, a guard becomes a crouching guard
 * and any other state but the crouches becomes a crouch; not held, a crouching guard becomes a guard and a crouch
 * becomes idle. Returns whether the state changed. */
s32 FighterState_updateCrouch(FighterState *this) {
    s32 state = this->state;

    if (FighterState_isDownHeld(this)) {
        if (state == FIGHTER_STATE_CROUCH || state == FIGHTER_STATE_CROUCH_GUARD) {
            return 0;
        }
        if (state == FIGHTER_STATE_GUARD) {
            FighterStateGround_initCrouchGuard(this);
            return 1;
        }
        FighterStateGround_initCrouch(this);
        return 1;
    }
    if (state == FIGHTER_STATE_CROUCH_GUARD) {
        FighterStateGround_initGuard(this);
        return 1;
    }
    if (state == FIGHTER_STATE_CROUCH) {
        FighterStateGround_init(this, FIGHTER_STATE_IDLE);
        return 1;
    }
    return 0;
}

/* Returns the fighter's clip. */
s32 FighterState_getClip(FighterState *this) {
    return this->fighter->clip;
}

/* Returns the fighter's velocity. */
Vec3 *func_8007FBB0(FighterState *this) {
    return &this->fighter->velocity;
}

/* Turns the fighter left (Fighter_faceLeft) when it is not attacking. */
void func_8007FBBC(FighterState *this) {
    if (FighterState_isAttackIdle(this)) {
        Fighter_faceLeft(this->fighter, 0);
    }
}

/* Turns the fighter right (Fighter_faceRight) when it is not attacking. */
void func_8007FBF4(FighterState *this) {
    if (FighterState_isAttackIdle(this)) {
        Fighter_faceRight(this->fighter, 0);
    }
}

/* Starts the fighter's clip from frame 0 (Fighter_setClip), blended in over blendFrames. */
void FighterState_playClip(FighterState *this, s32 clip, s32 blendFrames) {
    Fighter_setClip(this->fighter, clip, blendFrames, 0);
}

/* Starts the fighter's action clip from frame 0 (Fighter_playActionClip), blended in over blendFrames, plays -1. */
void func_8007FC50(FighterState *this, s32 clip, s32 blendFrames) {
    Fighter_playActionClip(this->fighter, clip, blendFrames, 0, -1);
}

/* Whether the fighter is done with its action clip (Fighter_isActionClipDone) when action is set, else with its
 * clip (Fighter_isClipDone). */
s32 FighterState_isClipDone(FighterState *this, s32 action) {
    s32 done;

    /* The match depends on one result variable: a return in each arm inverts the branch. */
    if (action) {
        done = Fighter_isActionClipDone(this->fighter);
    } else {
        done = Fighter_isClipDone(this->fighter);
    }
    return done;
}

/* Runs the fighter's intro slot (FighterVtable 0x38) with the state's frame, then counts the frame up. */
void func_8007FCB8(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->intro.func((u8 *)fighter + vtable->intro.delta, this->frame++);
}

/* Runs the fighter's getUp slot (FighterVtable 0x40) with the state's frame, then counts the frame up. */
void func_8007FCFC(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->getUp.func((u8 *)fighter + vtable->getUp.delta, this->frame++);
}

/* Runs the fighter's walkLeft slot (FighterVtable 0x48) with the state's frame, then counts the frame up. */
void func_8007FD40(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->walkLeft.func((u8 *)fighter + vtable->walkLeft.delta, this->frame++);
}

/* Runs the fighter's walkRight slot (FighterVtable 0x50) with the state's frame, then counts the frame up. */
void func_8007FD84(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->walkRight.func((u8 *)fighter + vtable->walkRight.delta, this->frame++);
}

/* Runs the fighter's dashLeft slot (FighterVtable 0x58) with the state's frame, then counts the frame up. */
void func_8007FDC8(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->dashLeft.func((u8 *)fighter + vtable->dashLeft.delta, this->frame++);
}

/* Runs the fighter's dashRight slot (FighterVtable 0x60) with the state's frame, then counts the frame up. */
void func_8007FE0C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->dashRight.func((u8 *)fighter + vtable->dashRight.delta, this->frame++);
}

/* Runs the fighter's jump slot (FighterVtable 0x68) with the state's frame, then counts the frame up. */
void func_8007FE50(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->jump.func((u8 *)fighter + vtable->jump.delta, this->frame++);
}

/* Runs the fighter's crouch slot (FighterVtable 0x70) with the state's frame, then counts the frame up. */
void func_8007FE94(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->crouch.func((u8 *)fighter + vtable->crouch.delta, this->frame++);
}

/* Runs the fighter's guard slot (FighterVtable 0x78) with the state's frame, then counts the frame up. */
void func_8007FED8(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->guard.func((u8 *)fighter + vtable->guard.delta, this->frame++);
}

/* Runs the fighter's crouchGuard slot (FighterVtable 0x80) with the state's frame, then counts the frame up. */
void func_8007FF1C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->crouchGuard.func((u8 *)fighter + vtable->crouchGuard.delta, this->frame++);
}

/* Runs the fighter's dizzy slot (FighterVtable 0x88) with the state's frame, then counts the frame up. */
void func_8007FF60(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->dizzy.func((u8 *)fighter + vtable->dizzy.delta, this->frame++);
}

/* Runs the fighter's flinch slot (FighterVtable 0x90) with the state's frame, then counts the frame up. */
void func_8007FFA4(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->flinch.func((u8 *)fighter + vtable->flinch.delta, this->frame++);
}

/* Runs the fighter's airFlinch slot (FighterVtable 0x98) with the state's frame, then counts the frame up. */
void func_8007FFE8(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airFlinch.func((u8 *)fighter + vtable->airFlinch.delta, this->frame++);
}

/* Runs the fighter's attack1 slot (FighterVtable 0xA0) with the state's frame, then counts the frame up. */
void func_8008002C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->attack1.func((u8 *)fighter + vtable->attack1.delta, this->frame++);
}

/* Runs the fighter's attack2 slot (FighterVtable 0xA8) with the state's frame, then counts the frame up. */
void func_80080070(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->attack2.func((u8 *)fighter + vtable->attack2.delta, this->frame++);
}

/* Runs the fighter's attack3 slot (FighterVtable 0xB0) with the state's frame, then counts the frame up. */
void func_800800B4(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->attack3.func((u8 *)fighter + vtable->attack3.delta, this->frame++);
}

/* Runs the fighter's upAttack slot (FighterVtable 0xB8) with the state's frame, then counts the frame up. */
void func_800800F8(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->upAttack.func((u8 *)fighter + vtable->upAttack.delta, this->frame++);
}

/* Runs the fighter's dashAttack slot (FighterVtable 0xC0) with the state's frame, then counts the frame up. */
void func_8008013C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->dashAttack.func((u8 *)fighter + vtable->dashAttack.delta, this->frame++);
}

/* Runs the fighter's crouchAttack slot (FighterVtable 0xC8) with the state's frame, then counts the frame up. */
void func_80080180(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->crouchAttack.func((u8 *)fighter + vtable->crouchAttack.delta, this->frame++);
}

/* Runs the fighter's throw slot (FighterVtable 0xD0) with the state's frame, then counts the frame up. */
void func_800801C4(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->throw.func((u8 *)fighter + vtable->throw.delta, this->frame++);
}

/* Runs the fighter's special1 slot (FighterVtable 0xD8) with the state's frame, then counts the frame up. */
void func_80080208(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->special1.func((u8 *)fighter + vtable->special1.delta, this->frame++);
}

/* Runs the fighter's special2 slot (FighterVtable 0xE0) with the state's frame, then counts the frame up. */
void func_8008024C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->special2.func((u8 *)fighter + vtable->special2.delta, this->frame++);
}

/* Runs the fighter's special3 slot (FighterVtable 0xE8) with the state's frame, then counts the frame up. */
void func_80080290(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->special3.func((u8 *)fighter + vtable->special3.delta, this->frame++);
}

/* Runs the fighter's airSpecial3 slot (FighterVtable 0xF0) with the state's frame, then counts the frame up. */
void func_800802D4(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airSpecial3.func((u8 *)fighter + vtable->airSpecial3.delta, this->frame++);
}

/* Runs the fighter's airAttack slot (FighterVtable 0xF8) with the state's frame, then counts the frame up. */
void func_80080318(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airAttack.func((u8 *)fighter + vtable->airAttack.delta, this->frame++);
}

/* Runs the fighter's airUpAttack slot (FighterVtable 0x100) with the state's frame, then counts the frame up. */
void func_8008035C(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airUpAttack.func((u8 *)fighter + vtable->airUpAttack.delta, this->frame++);
}

/* Runs the fighter's airSideAttack slot (FighterVtable 0x108) with the state's frame, then counts the frame up. */
void func_800803A0(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airSideAttack.func((u8 *)fighter + vtable->airSideAttack.delta, this->frame++);
}

/* Runs the fighter's airSpecial1 slot (FighterVtable 0x110) with the state's frame, then counts the frame up. */
void func_800803E4(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airSpecial1.func((u8 *)fighter + vtable->airSpecial1.delta, this->frame++);
}

/* Runs the fighter's airSpecial2 slot (FighterVtable 0x118) with the state's frame, then counts the frame up. */
void func_80080428(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->airSpecial2.func((u8 *)fighter + vtable->airSpecial2.delta, this->frame++);
}

/* Reads a side press in the last input (InputHistory_getHeld): 1 for PAD_LEFT, 2 for PAD_RIGHT, or 3 / 4 when the
 * button was pressed twice in the last 10 frames (a dash: InputHistory_startSearch, InputHistory_findPresses); 0
 * without either. */
s32 readSideTap(InputHistory *input) {
    u32 buttons = InputHistory_getHeld(input);

    /* The match depends on the full if / else if / else chain: a plain `return 0` after them is moved to the
     * end with the branch inverted. */
    if (buttons & PAD_LEFT) {
        InputHistory_startSearch(input, 10);
        if (InputHistory_findPresses(input, PAD_LEFT, 2, 0)) {
            return 3;
        }
        return 1;
    } else if (buttons & PAD_RIGHT) {
        InputHistory_startSearch(input, 10);
        if (InputHistory_findPresses(input, PAD_RIGHT, 2, 0)) {
            return 4;
        }
        return 2;
    } else {
        return 0;
    }
}

/* Flinches (FighterStateFlinch_init with the hit's bits), unless the body state guards against it: a guard
 * without bit 28 of the hit, or a crouching guard without bit 27. */
void FighterState_hitUnlessGuarded(FighterState *this, u32 hitBits) {
    /* The match depends on case 9 breaking on its bit: written as case 10, the two tests are merged. */
    switch (FighterState_getBodyState(this)->state) {
    case FIGHTER_STATE_GUARD:
        if (hitBits & 0x10000000) {
            break;
        }
        return;
    case FIGHTER_STATE_CROUCH_GUARD:
        if (!(hitBits & 0x8000000)) {
            return;
        }
        break;
    }
    FighterStateFlinch_init(this, hitBits);
}

/* The attack idle state's hit: ends the fighter's move (its endMove slot), then starts over as attack idle. */
void FighterAttackIdle_hit(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->endMove.func((u8 *)fighter + vtable->endMove.delta);
    FighterAttackIdle_init(this);
}

/* The intro state (FighterStateIntro_init, virtual table FIGHTER_STATE_INTRO_VTABLE): a FighterState with a flag
 * at 0x10 and a frame count at 0x14. */
typedef struct FighterStateIntro {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 frames;
} FighterStateIntro;

extern VtableEntry FIGHTER_STATE_INTRO_VTABLE[];
s32 FighterState_isClipDone(FighterState *this, s32 action);

INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_PARAMS);

/* Starts the intro (FIGHTER_STATE_INTRO). 0x10 is set unless the low two bits of the character's params flags are
 * 1; when it is clear, the intro clip is run in (func_800797B8). The match depends on taking the table into a
 * local before the call: the game keeps its address in a saved register across it. */
FighterStateIntro *FighterStateIntro_init(FighterStateIntro *this) {
    FighterParams *table;

    FighterState_init(&this->base, FIGHTER_STATE_INTRO);
    this->base.vtable = FIGHTER_STATE_INTRO_VTABLE;
    table = FIGHTER_PARAMS;
    this->unk10 = (table[Fighter_getCharacter(this->base.fighter)].flags & 3) != 1;
    if (!this->unk10) {
        func_800797B8(this->base.fighter, FIGHTER_CLIP_INTRO);
    }
    this->frames = 0;
    func_8007FCB8(&this->base);
    return this;
}

/* The intro's tick: at frame 10 starts the intro clip (directly when 0x10 is set, else blended in over 0x32
 * frames); from frame 90, or from frame 11 once the clip is done, the fighter stands idle (FighterStateGround_init),
 * else its intro slot runs (func_8007FCB8). */
void FighterStateIntro_tick(FighterStateIntro *this) {
    if (++this->frames == 10) {
        if (this->unk10) {
            FighterState_playClip(&this->base, FIGHTER_CLIP_INTRO, 0);
        } else {
            Fighter_setClip(this->base.fighter, FIGHTER_CLIP_INTRO, 0, 0);
            Fighter_setClip(this->base.fighter, FIGHTER_CLIP_INTRO, 0x32, 1);
        }
    }
    if (this->frames >= 90 || (this->frames >= 11 && FighterState_isClipDone(&this->base, 0))) {
        FighterStateGround_init(&this->base, FIGHTER_STATE_IDLE);
    } else {
        func_8007FCB8(&this->base);
    }
}

/* Starts the end pose (FIGHTER_STATE_END_POSE, virtual table D_8006811C) and its clip, blended in over 10 frames. */
FighterState *func_80080730(FighterState *this) {
    FighterState_init(this, FIGHTER_STATE_END_POSE);
    this->vtable = D_8006811C;
    FighterState_playClip(this, FIGHTER_CLIP_END_POSE, 10);
    return this;
}

/* Constructs a pose state (virtual table D_8006811C) with its FIGHTER_STATE_* id. */
FighterState *func_80080778(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = D_8006811C;
    return this;
}

/* Starts the win pose (FIGHTER_STATE_WIN_POSE, func_80080778) and its clip, blended in over 10 frames. */
FighterState *func_800807AC(FighterState *this) {
    func_80080778(this, FIGHTER_STATE_WIN_POSE);
    this->vtable = D_8006811C;
    FighterState_playClip(this, FIGHTER_CLIP_WIN_POSE, 10);
    return this;
}

/* The held state (FighterStateHeld_init, virtual table FIGHTER_STATE_HELD_VTABLE): a FighterState with a frame
 * count at 0x10. */
typedef struct FighterStateHeld {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 frames; /* counted down */
} FighterStateHeld;

extern VtableEntry FIGHTER_STATE_HELD_VTABLE[];

/* Holds the fighter (FIGHTER_STATE_HELD) for `frames`, with no clip (-1); a side stunned to its limit
 * (func_8007F1C0) has its stun count cleared (func_8007F1B8). */
void *FighterStateHeld_init(FighterState *this, s32 frames) {
    BattleSide *side;

    FighterState_init(this, FIGHTER_STATE_HELD);
    this->vtable = FIGHTER_STATE_HELD_VTABLE;
    ((FighterStateHeld *)this)->frames = frames;
    FighterState_playClip(this, -1, 0);
    side = func_8007758C(this->fighter);
    if (func_8007F1C0(side)) {
        func_8007F1B8(side);
    }
    return this;
}

/* The down state (FighterStateDown_init, virtual table FIGHTER_STATE_DOWN_VTABLE): a FighterState with a frame
 * count at 0x10. */
typedef struct FighterStateDown {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 frames; /* counted down */
} FighterStateDown;

FighterState *FighterStateDown_init(FighterState *this, s32 state);

/* The held state's tick: once the side's life is out the fighter is knocked out (FighterStateDown_init); else when
 * the frames are out it stands idle (FighterStateGround_init); else counts them down. */
void FighterStateHeld_tick(FighterStateHeld *this) {
    if (func_8007758C(this->base.fighter)->life <= 0) {
        FighterStateDown_init(&this->base, FIGHTER_STATE_KNOCKED_OUT);
    } else if (this->frames == 0) {
        FighterStateGround_init(&this->base, FIGHTER_STATE_IDLE);
    } else if (this->frames > 0) {
        this->frames--;
    }
}

/* The fall state (FighterStateFall_init, virtual table FIGHTER_STATE_FALL_VTABLE): a FighterStateJump with a word
 * at 0x1C. */
typedef struct FighterStateFall {
    /* 0x00 */ FighterStateJump base;
    /* 0x18 */ u8 unk18[0x1C - 0x18];
    /* 0x1C */ s32 unk1C;
} FighterStateFall;

FighterState *FighterStateFall_init(FighterState *this, s32 jumpCount, s32 fromJump);

/* Starts a ground state (`state`, virtual table FIGHTER_STATE_GROUND_VTABLE); off the ground it falls instead
 * (FighterStateFall_init). Idle, it starts the stand clip, blended in over 4 frames. */
FighterState *FighterStateGround_init(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = FIGHTER_STATE_GROUND_VTABLE;
    if (!Fighter_isOnGround(this->fighter)) {
        FighterStateFall_init(this, 0, 0);
    } else if (this->state == FIGHTER_STATE_IDLE) {
        FighterState_playClip(this, FIGHTER_CLIP_STAND, 4);
    }
    return this;
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067AD4);

/* A ground state's tick: off the ground the fighter falls (FighterStateFall_init); else unless it guarded
 * (FighterState_updateGuard) or crouched (FighterState_updateCrouch, when not attacking): idle, jumping, falling or
 * walking and not attacking, a side press (readSideTap) makes it walk (func_80080FDC) or dash (func_80081200);
 * otherwise, once the side's life is out, it is knocked out (FighterStateDown_init). */
void FighterStateGround_tick(FighterState *this) {
    if (!Fighter_isOnGround(this->fighter)) {
        FighterStateFall_init(this, 0, 0);
        return;
    }
    if (FighterState_updateGuard(this)) {
        return;
    }
    if (FighterState_isAttackIdle(this) && FighterState_updateCrouch(this)) {
        return;
    }
    /* The match depends on the loop its first break leaves (as in func_80081B84): the game rolls that test to the
     * loop's end and enters it with a jump; a plain if gives the test first. The jump table starts at case 0. */
    do {
        if (!FighterState_isAttackIdle(this) ||
            !(this->state == FIGHTER_STATE_IDLE || this->state == FIGHTER_STATE_FALL ||
              this->state == FIGHTER_STATE_JUMP || this->state == FIGHTER_STATE_WALK_LEFT ||
              this->state == FIGHTER_STATE_WALK_RIGHT)) {
            break;
        }
        switch (readSideTap(Fighter_getInput(this->fighter))) {
        case 0:
            break;
        case 1:
            func_80080FDC(this, FIGHTER_STATE_WALK_LEFT);
            return;
        case 2:
            func_80080FDC(this, FIGHTER_STATE_WALK_RIGHT);
            return;
        case 3:
            func_80081200(this, FIGHTER_STATE_DASH_LEFT);
            return;
        case 4:
            func_80081200(this, FIGHTER_STATE_DASH_RIGHT);
            return;
        }
    } while (0);
    if (func_8007758C(this->fighter)->life <= 0) {
        FighterStateDown_init(this, FIGHTER_STATE_KNOCKED_OUT);
    }
}

/* A ground state's jump (slot 0x10): for a positive arg, when not attacking, crouching or guarding, the fighter
 * jumps (FighterStateJump_init). */
void FighterStateGround_jump(FighterState *this, s32 arg) {
    if (arg > 0 && FighterState_isAttackIdle(this)) {
        switch (FighterState_getBodyState(this)->state) {
        case FIGHTER_STATE_CROUCH:
        case FIGHTER_STATE_GUARD:
        case FIGHTER_STATE_CROUCH_GUARD:
            break;
        default:
            FighterStateJump_init(this, 0);
            break;
        }
    }
}

/* A ground state's drop (slot 0x18): over a floor below (Fighter_probeFloorBelow) and when not attacking, the
 * fighter falls through (FighterStateFall_init (1, 0)) and its 0x48C is set to 8 (the US build calls func_8007A718
 * for that). */
void FighterStateGround_drop(FighterState *this) {
    if (Fighter_probeFloorBelow(this->fighter) && FighterState_isAttackIdle(this)) {
        FighterStateFall_init(this, 1, 0);
#if VERSION_JP
        this->fighter->unk48C = 8;
#else
        func_8007A718(this->fighter);
#endif
    }
}

/* Crouches (FIGHTER_STATE_CROUCH, a ground state), then runs func_8007FE94 and FighterState_updateGuard. */
FighterState *FighterStateGround_initCrouch(FighterState *this) {
    FighterStateGround_init(this, FIGHTER_STATE_CROUCH);
    this->vtable = FIGHTER_STATE_GROUND_VTABLE;
    func_8007FE94(this);
    FighterState_updateGuard(this);
    return this;
}

/* Jumps (FIGHTER_STATE_JUMP): plays the jump sound (8), counts this jump after jumpsBefore, runs func_8007FE50 and
 * clears the frames. */
FighterState *FighterStateJump_init(FighterState *this, s32 jumpsBefore) {
    FighterState_init(this, FIGHTER_STATE_JUMP);
    this->vtable = FIGHTER_STATE_JUMP_VTABLE;
    soundControlPlaySound(&SOUND_CONTROL, 8, 0);
    ((FighterStateJump *)this)->jumpCount = jumpsBefore + 1;
    func_8007FE50(this);
    ((FighterStateJump *)this)->frames = 0;
    return this;
}

/* Constructs an airborne state (virtual table FIGHTER_STATE_JUMP_VTABLE) with its FIGHTER_STATE_* id and the jumps
 * made (FighterStateFall_init's base). */
FighterStateJump *func_80080C5C(FighterStateJump *this, s32 state, s32 jumpCount) {
    FighterState_init(&this->base, state);
    this->base.vtable = FIGHTER_STATE_JUMP_VTABLE;
    this->jumpCount = jumpCount;
    return this;
}

Vec3 *func_8007FBB0(FighterState *this);
void func_80080D38(FighterStateJump *this);

/* The jump's tick: once the fighter stops rising (velocity vy not negative) it falls (FighterStateFall_init, from
 * a jump); else, from the second frame on, pushes its force.vy by its mass times 0x71C, then runs func_80080D38.
 * The match depends on the step being a local of its own: added in place, the old vy takes the step's register. */
void FighterStateJump_tick(FighterStateJump *this) {
    s32 vy = func_8007FBB0(&this->base)->vy;
    Fighter *fighter = this->base.fighter;

    if (vy >= 0) {
        FighterStateFall_init(&this->base, this->jumpCount, 1);
    } else {
        if (this->frames) {
            s32 delta = ((s64)fighter->mass * 0x71C) >> 16;

            fighter->force.vy += delta;
        }
        this->frames = 1;
        func_80080D38(this);
    }
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_80080D38);

/* The jump (slot 0x10) of the jump and the fall: clears the frames, then for a positive arg, when not attacking
 * and with fewer than 2 jumps made, jumps again (FighterStateJump_init). */
void FighterStateJump_jump(FighterStateJump *this, s32 arg) {
    this->frames = 0;
    if (arg > 0 && FighterState_isAttackIdle(&this->base) && this->jumpCount < 2) {
        FighterStateJump_init(&this->base, this->jumpCount);
    }
}

extern VtableEntry FIGHTER_STATE_FALL_VTABLE[];

/* Falls (FIGHTER_STATE_FALL, after jumpCount jumps; fromJump is 1 only from FighterStateJump_tick) and starts the
 * fall clip, blended in over 6 frames. */
FighterState *FighterStateFall_init(FighterState *this, s32 jumpCount, s32 fromJump) {
    func_80080C5C((FighterStateJump *)this, FIGHTER_STATE_FALL, jumpCount);
    this->vtable = FIGHTER_STATE_FALL_VTABLE;
    ((FighterStateFall *)this)->unk1C = fromJump;
    FighterState_playClip(this, FIGHTER_CLIP_FALL, 6);
    return this;
}

/* The fall's tick: on the ground with its 0x48C down to 0, the fighter lands: dust on both sides
 * (Fighter_spawnDust: 2 or 3 by Fighter_canEvolve), the landing sound (9), and it stands idle
 * (FighterStateGround_init); else runs func_80080D38. */
void FighterStateFall_tick(FighterStateFall *this) {
    Fighter *fighter = this->base.base.fighter;
    s32 arg;

    if (Fighter_isOnGround(fighter) && fighter->unk48C <= 0) {
        arg = Fighter_canEvolve(fighter) ? 2 : 3;
        Fighter_spawnDust(fighter, 1, 0x60, arg);
        Fighter_spawnDust(fighter, -1, 0x60, arg);
        soundControlPlaySound(&SOUND_CONTROL, 9, 0);
        FighterStateGround_init(&this->base.base, FIGHTER_STATE_IDLE);
    } else {
        func_80080D38(&this->base);
    }
}

extern VtableEntry D_80067F94[];

/* Walks (`state`, virtual table D_80067F94): plays sound 0xB, then takes a step left (func_8007FD40) or right
 * (func_8007FD84) by the state. */
FighterState *func_80080FDC(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = D_80067F94;
    soundControlPlaySound(&SOUND_CONTROL, 0xB, 0);
    if (this->state == FIGHTER_STATE_WALK_LEFT) {
        func_8007FD40(this);
    }
    if (this->state == FIGHTER_STATE_WALK_RIGHT) {
        func_8007FD84(this);
    }
    return this;
}

/* The walk's tick: off the ground the fighter falls (FighterStateFall_init); else unless it guarded
 * (FighterState_updateGuard), by the side press (readSideTap): none, or attacking, it stands idle; left or right
 * walks that way (func_80080FDC unless already) and steps; a double press dashes (func_80081200). */
void func_80081054(FighterState *this) {
    s32 press;
    s32 state;

    if (!Fighter_isOnGround(this->fighter)) {
        FighterStateFall_init(this, 0, 0);
        return;
    }
    if (FighterState_updateGuard(this)) {
        return;
    }
    press = readSideTap(Fighter_getInput(this->fighter));
    if (press == 0 || !FighterState_isAttackIdle(this)) {
        FighterStateGround_init(this, FIGHTER_STATE_IDLE);
        return;
    }
    state = this->state;
    switch (press) {
    /* press is not 0 here, but the game's jump table starts at case 0 (it indexes it by press, not press - 1) */
    case 0:
        if (state == FIGHTER_STATE_WALK_LEFT) {
            func_8007FD40(this);
        }
        if (state == FIGHTER_STATE_WALK_RIGHT) {
            func_8007FD84(this);
        }
        break;
    case 1:
        if (state != FIGHTER_STATE_WALK_LEFT) {
            func_80080FDC(this, FIGHTER_STATE_WALK_LEFT);
        }
        func_8007FD40(this);
        break;
    case 2:
        if (state != FIGHTER_STATE_WALK_RIGHT) {
            func_80080FDC(this, FIGHTER_STATE_WALK_RIGHT);
        }
        func_8007FD84(this);
        break;
    case 3:
        func_80081200(this, FIGHTER_STATE_DASH_LEFT);
        break;
    case 4:
        func_80081200(this, FIGHTER_STATE_DASH_RIGHT);
        break;
    }
}

/* The jump (slot 0x10) of the walk and the dash: jumps (FighterStateJump_init) when not attacking and arg is
 * positive. */
void func_800811B8(FighterState *this, s32 arg) {
    if (FighterState_isAttackIdle(this) && arg > 0) {
        FighterStateJump_init(this, 0);
    }
}

extern VtableEntry D_80067F5C[];

/* Dashes (`state`, virtual table D_80067F5C, on the walk's constructor), then pushes left (func_8007FDC8) or
 * right (func_8007FE0C) by the state. */
FighterState *func_80081200(FighterState *this, s32 state) {
    s32 current;

    func_80080FDC(this, state);
    this->vtable = D_80067F5C;
    current = this->state;
    if (current == FIGHTER_STATE_DASH_LEFT) {
        func_8007FDC8(this);
    }
    if (current == FIGHTER_STATE_DASH_RIGHT) {
        func_8007FE0C(this);
    }
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_80081264);

/* Guards (FIGHTER_STATE_GUARD, a ground state), then runs func_8007FED8. */
FighterState *FighterStateGround_initGuard(FighterState *this) {
    FighterStateGround_init(this, FIGHTER_STATE_GUARD);
    this->vtable = FIGHTER_STATE_GROUND_VTABLE;
    func_8007FED8(this);
    return this;
}

/* Constructs a ground state with its FIGHTER_STATE_* id (FighterStateGround_initCrouchGuard's base). */
FighterState *func_800813C4(FighterState *this, s32 state) {
    FighterStateGround_init(this, state);
    this->vtable = FIGHTER_STATE_GROUND_VTABLE;
    return this;
}

/* Guards crouching (FIGHTER_STATE_CROUCH_GUARD, a ground state), then runs func_8007FF1C. */
FighterState *FighterStateGround_initCrouchGuard(FighterState *this) {
    func_800813C4(this, FIGHTER_STATE_CROUCH_GUARD);
    this->vtable = FIGHTER_STATE_GROUND_VTABLE;
    func_8007FF1C(this);
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0f", FighterStateFlinch_init);

/* The flinch (FighterStateFlinch_init, virtual table FIGHTER_STATE_FLINCH_VTABLE): a FighterState with a frame
 * count at 0x10. */
typedef struct FighterStateFlinch {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 frames; /* counted down */
} FighterStateFlinch;

FighterState *FighterStateAirFlinch_init(FighterState *this);

/* The flinch's tick: counts the frames down; off the ground the fighter flinches in the air
 * (FighterStateAirFlinch_init), once the side's life is out it is knocked out (FighterStateDown_init), else when the
 * frames are out it stands idle. */
void FighterStateFlinch_tick(FighterStateFlinch *this) {
    Fighter *fighter = this->base.fighter;
    BattleSide *side = func_8007758C(fighter);

    this->frames--;
    if (!Fighter_isOnGround(fighter)) {
        FighterStateAirFlinch_init(&this->base);
    } else if (side->life <= 0) {
        FighterStateDown_init(&this->base, FIGHTER_STATE_KNOCKED_OUT);
    } else if (this->frames < 0) {
        FighterStateGround_init(&this->base, FIGHTER_STATE_IDLE);
    }
}

/* The class of virtual table FIGHTER_STATE_AIR_FLINCH_VTABLE (FighterStateAirFlinch_init): FighterState_init's with a
 * frame count at 0x10. */
typedef struct FighterStateAirFlinch {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 frames; /* counted down */
} FighterStateAirFlinch;

extern VtableEntry FIGHTER_STATE_AIR_FLINCH_VTABLE[];

/* Flinches in the air (FIGHTER_STATE_AIR_FLINCH): clears the fighter's 0x494 (func_8007A12C with 0), counts
 * 4 frames and runs func_8007FFE8. */
FighterState *FighterStateAirFlinch_init(FighterState *this) {
    FighterState_init(this, FIGHTER_STATE_AIR_FLINCH);
    this->vtable = FIGHTER_STATE_AIR_FLINCH_VTABLE;
    func_8007A12C(this->fighter, 0);
    ((FighterStateAirFlinch *)this)->frames = 4;
    func_8007FFE8(this);
    return this;
}

/* The air flinch's tick: once its frames are out and the fighter is on the ground, it lands with dust on both
 * sides (Fighter_spawnDust: 2 or 3 by Fighter_canEvolve) and is knocked out (FighterStateDown_init). */
void FighterStateAirFlinch_tick(FighterStateAirFlinch *this) {
    s32 arg;

    if (--this->frames < 0 && Fighter_isOnGround(this->base.fighter)) {
        arg = Fighter_canEvolve(this->base.fighter) ? 2 : 3;
        Fighter_spawnDust(this->base.fighter, 1, 0x60, arg);
        Fighter_spawnDust(this->base.fighter, -1, 0x60, arg);
        FighterStateDown_init(&this->base, FIGHTER_STATE_KNOCKED_OUT);
    }
}

extern VtableEntry FIGHTER_STATE_DOWN_VTABLE[];

/* Goes down (`state`): knocked out starts the down clip (blended in over 2 frames) with no frames to wait; any
 * other (knocked down) starts the knocked down clip (over 1 frame) and waits 0x16 frames. The match depends on
 * the switch: the game tests 0xD, then 0xE, and falls into the 0xD arm. */
FighterState *FighterStateDown_init(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = FIGHTER_STATE_DOWN_VTABLE;
    switch (state) {
    case FIGHTER_STATE_KNOCKED_DOWN:
    default:
        FighterState_playClip(this, FIGHTER_CLIP_KNOCKED_DOWN, 1);
        ((FighterStateDown *)this)->frames = 0x16;
        break;
    case FIGHTER_STATE_KNOCKED_OUT:
        FighterState_playClip(this, FIGHTER_CLIP_DOWN, 2);
        ((FighterStateDown *)this)->frames = 0;
        break;
    }
    return this;
}

FighterState *FighterStateGetUp_init(FighterState *this);

/* The down state's tick: while the side has life, counts the frames down; once they are out, the fighter is on the
 * ground and the clip is done, it gets up (FighterStateGetUp_init). */
void FighterStateDown_tick(FighterStateDown *this) {
    if (func_8007758C(this->base.fighter)->life > 0 && --this->frames <= 0 && Fighter_isOnGround(this->base.fighter)
        && FighterState_isClipDone(&this->base, 0)) {
        FighterStateGetUp_init(&this->base);
    }
}

/* Gets up (FIGHTER_STATE_GET_UP), running the fighter's getUp slot (func_8007FCFC). */
FighterState *FighterStateGetUp_init(FighterState *this) {
    FighterState_init(this, FIGHTER_STATE_GET_UP);
    this->vtable = FIGHTER_STATE_GET_UP_VTABLE;
    func_8007FCFC(this);
    return this;
}

Unk80081874 *func_80081874(Unk80081874 *this);

/* The get-up's tick: once the clip is done, a side stunned to its limit (func_8007F1C0) turns dizzy
 * (func_80081874); else the fighter gets 8 invulnerable frames and stands idle. Until then its getUp slot runs. */
void FighterStateGetUp_tick(FighterState *this) {
    if (FighterState_isClipDone(this, 0)) {
        if (func_8007F1C0(func_8007758C(this->fighter))) {
            func_80081874((Unk80081874 *)this);
        } else {
            this->fighter->invulnerableFrames = 8;
            FighterStateGround_init(this, FIGHTER_STATE_IDLE);
        }
    } else {
        func_8007FCFC(this);
    }
}

/* Turns dizzy (FIGHTER_STATE_DIZZY, virtual table D_80067E44): runs the fighter's dizzy slot (func_8007FF60) and
 * clears the frame count at 0x10. */
Unk80081874 *func_80081874(Unk80081874 *this) {
    FighterState_init(&this->base, FIGHTER_STATE_DIZZY);
    this->base.vtable = D_80067E44;
    func_8007FF60(&this->base);
    this->unk10 = 0;
    return this;
}

/* The dizzy state's tick: once the side's life is out the fighter is knocked out; else its dizzy slot runs, sound
 * 0xC plays every 22 frames (count at 0x10), and once the clip is done the side's stun count is cleared
 * (func_8007F1B8) and the fighter stands idle. */
void func_800818B8(Unk80081874 *this) {
    BattleSide *side = func_8007758C(this->base.fighter);

    if (side->life <= 0) {
        FighterStateDown_init(&this->base, FIGHTER_STATE_KNOCKED_OUT);
    } else {
        func_8007FF60(&this->base);
        if (this->unk10++ % 22 == 0) {
            soundControlPlaySound(&SOUND_CONTROL, 0xC, 0);
        }
        if (FighterState_isClipDone(&this->base, 0)) {
            func_8007F1B8(side);
            FighterStateGround_init(&this->base, FIGHTER_STATE_IDLE);
        }
    }
}

/* The dizzy state's hit (slot 0x28): clears the side's stun count (func_8007F1B8), then flinches
 * (FighterStateFlinch_init). */
void func_80081998(FighterState *this, s32 hitBits) {
    func_8007F1B8(func_8007758C(this->fighter));
    FighterStateFlinch_init(this, hitBits);
}

/* Makes the attack state idle (FIGHTER_STATE_IDLE, virtual table FIGHTER_ATTACK_IDLE_VTABLE): no parts hit. */
FighterState *FighterAttackIdle_init(FighterState *this) {
    FighterState_init(this, FIGHTER_STATE_IDLE);
    this->vtable = FIGHTER_ATTACK_IDLE_VTABLE;
    this->fighter->attackParts = 0;
    return this;
}

/* Constructs an attack state with its FIGHTER_STATE_* id (virtual table FIGHTER_ATTACK_IDLE_VTABLE; the attacks'
 * base). */
FighterState *func_80081A20(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = FIGHTER_ATTACK_IDLE_VTABLE;
    return this;
}

/* The attack idle state's attack: clears the fighter's attack parts and hit, then, unless its body state is in the
 * intro, guarding, hit, down, getting up, dizzy or held (or crouching, for a special), starts the attack the input
 * asks for: 0 an attack (func_80081B84), 1 to 3 the specials 1 to 3. */
void FighterAttackIdle_attack(FighterState *this, s32 attack) {
    Fighter *fighter;
    InputHistory *input;

    this->fighter->attackParts = 0;
    this->fighter->hitDamage = 0;
    /* The match depends on these four through a local: the game reloads 8 after each store above, not here. */
    fighter = this->fighter;
    fighter->hitKnockback.vx = 0;
    fighter->hitKnockback.vy = 0;
    fighter->hitKnockback.vz = 0;
    fighter->hitKnockbackPad = 0;
    input = Fighter_getInput(this->fighter);
    switch (FighterState_getBodyState(this)->state) {
    case FIGHTER_STATE_INTRO:
    case FIGHTER_STATE_GUARD:
    case FIGHTER_STATE_CROUCH_GUARD:
    case FIGHTER_STATE_FLINCH:
    case FIGHTER_STATE_AIR_FLINCH:
    case FIGHTER_STATE_KNOCKED_DOWN:
    case FIGHTER_STATE_KNOCKED_OUT:
    case FIGHTER_STATE_GET_UP:
    case FIGHTER_STATE_DIZZY:
    case FIGHTER_STATE_HELD:
        break;
    case FIGHTER_STATE_CROUCH:
        if (attack != 0) {
            break;
        }
        /* fallthrough */
    default:
        switch (attack) {
        case 0:
            func_80081B84(this, input);
            break;
        case 1:
            func_80081E18(this, input);
            break;
        case 2:
            func_80081EEC(this, input);
            break;
        case 3:
            func_80081FC4(this, input);
            break;
        }
        break;
    }
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C44);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C4C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C54);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C5C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C64);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C6C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067C74);

/* Starts an attack by the fighter's clip and input: in the air an air up attack (PAD_UP), an air side attack
 * (PAD_LEFT or PAD_RIGHT) or an air attack, each using an air move; dashing a dash attack; crouching a crouch
 * attack; crouch guarding none; else a throw (toward an opponent in reach), an up attack (PAD_UP) or the attack
 * chain (func_8008230C). Each sets the move's reward (Fighter_setMoveReward). */
void func_80081B84(FighterState *this, InputHistory *input) {
    s32 clip = FighterState_getClip(this);
    Fighter *fighter = this->fighter;

    switch (clip) {
    case FIGHTER_CLIP_JUMP:
    case FIGHTER_CLIP_FALL:
        InputHistory_startSearch(input, 4);
        if (InputHistory_findHeld(input, PAD_UP, 0)) {
            if (Fighter_tryUseAirMove(this->fighter, 0)) {
                Fighter_setMoveReward(fighter, MOVE_AIR_UP, 400);
                FighterStateAttack_init(this, FIGHTER_STATE_AIR_UP_ATTACK, D_80067C44);
            }
        } else if (InputHistory_getHeld(input) & (PAD_LEFT | PAD_RIGHT)) {
            if (Fighter_tryUseAirMove(this->fighter, 1)) {
                Fighter_setMoveReward(fighter, MOVE_AIR_SIDE, 400);
                FighterStateAttack_init(this, FIGHTER_STATE_AIR_ATTACK, D_80067C4C);
            }
        } else {
            Fighter_setMoveReward(fighter, MOVE_AIR, 400);
            FighterStateAttack_init(this, FIGHTER_STATE_AIR_ATTACK, D_80067C54);
        }
        break;
    case FIGHTER_CLIP_DASH:
        Fighter_setMoveReward(fighter, MOVE_DASH, 300);
        FighterStateAttack_init(this, FIGHTER_STATE_DASH_ATTACK, D_80067C5C);
        break;
    case FIGHTER_CLIP_CROUCH:
        Fighter_setMoveReward(fighter, MOVE_CROUCH, 100);
        FighterStateAttack_init(this, FIGHTER_STATE_CROUCH_ATTACK, D_80067C64);
        break;
    case FIGHTER_CLIP_CROUCH_GUARD:
        break;
    default:
        /* The match depends on a loop around the 0x2000 test that its break leaves: the game rolls that test to the
         * loop's end (entered with a jump), hoists &D_80067C6C out of it and moves the 0x17 block before it, which
         * a plain if/else never gives. A do/while (0) and a for (;;) ending in return both match. */
        do {
            if (Fighter_getOpponentInReach(this->fighter, 0)) {
                Fighter_mirrorInput(this->fighter, input);
                InputHistory_startSearch(input, 4);
                if (InputHistory_findHeld(input, PAD_RIGHT, 0)) {
                    Fighter_setMoveReward(fighter, MOVE_THROW, 400);
                    FighterStateAttack_init(this, FIGHTER_STATE_THROW, D_80067C6C);
                    break;
                }
            }
            InputHistory_startSearch(input, 4);
            if (InputHistory_findHeld(input, PAD_UP, 0)) {
                Fighter_setMoveReward(fighter, MOVE_UP, 200);
                FighterStateAttack_init(this, FIGHTER_STATE_UP_ATTACK, D_80067C74);
            } else {
                func_8008230C(this);
            }
        } while (0);
        break;
    }
}

/* Starts special 1: in the air (jump or fall clip) its air form, using an air move; else its ground form; only
 * when the model has the clip (Fighter_lacksClip). Either way the move pays its reward. */
void func_80081E18(FighterState *this, InputHistory *input) {
    s32 value = FighterState_getClip(this);
    Fighter *fighter = this->fighter;

    switch (value) {
    case FIGHTER_CLIP_JUMP:
    case FIGHTER_CLIP_FALL:
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_AIR_SPECIAL1) && Fighter_tryUseAirMove(fighter, 2)) {
            FighterStateAttack_init(this, FIGHTER_STATE_AIR_SPECIAL1, D_80067CCC);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL1, 100);
        }
        break;
    default:
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_SPECIAL1)) {
            FighterStateAttack_init(this, FIGHTER_STATE_SPECIAL1, D_80067CD4);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL1, 100);
        }
        break;
    }
}

/* Starts special 2, as func_80081E18 does special 1. */
void func_80081EEC(FighterState *this, InputHistory *input) {
    s32 value = FighterState_getClip(this);
    Fighter *fighter = this->fighter;

    switch (value) {
    case FIGHTER_CLIP_JUMP:
    case FIGHTER_CLIP_FALL:
        /* the game reads 0x8 again here, where its siblings use fighter */
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_AIR_SPECIAL2) && Fighter_tryUseAirMove(this->fighter, 3)) {
            FighterStateAttack_init(this, FIGHTER_STATE_AIR_SPECIAL2, D_80067CDC);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL2, 1000);
        }
        break;
    default:
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_SPECIAL2)) {
            FighterStateAttack_init(this, FIGHTER_STATE_SPECIAL2, D_80067CE4);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL2, 1000);
        }
        break;
    }
}

/* Starts special 3, as func_80081E18 does special 1. */
void func_80081FC4(FighterState *this, InputHistory *input) {
    Fighter *fighter = this->fighter;
    s32 value = FighterState_getClip(this);

    switch (value) {
    case FIGHTER_CLIP_JUMP:
    case FIGHTER_CLIP_FALL:
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_AIR_SPECIAL3) && Fighter_tryUseAirMove(fighter, 4)) {
            FighterStateAttack_init(this, FIGHTER_STATE_AIR_SPECIAL3, D_80067CEC);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL3, 3000);
        }
        break;
    default:
        if (!Fighter_lacksClip(fighter, FIGHTER_CLIP_SPECIAL3)) {
            FighterStateAttack_init(this, FIGHTER_STATE_SPECIAL3, D_80067CF4);
            Fighter_setMoveReward(fighter, MOVE_SPECIAL3, 3000);
        }
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/game_0f", FighterStateAttack_init);

INCLUDE_ASM("game/nonmatchings/game_0f", FighterStateAttack_tick);

/* An attack's hit (slot 0x28): ends the fighter's move (its endMove slot), makes the attack state idle and stops
 * no action clip (func_8007FC50 with -1). */
void FighterStateAttack_hit(FighterState *this) {
    Fighter *fighter = this->fighter;
    FighterVtable *vtable = GET_FIGHTER_VTABLE(fighter);

    vtable->endMove.func((u8 *)fighter + vtable->endMove.delta);
    FighterAttackIdle_init(this);
    func_8007FC50(this, -1, 0);
}

/* An attack's attack: starts the next one (FighterAttackIdle_attack) unless func_8007F98C holds. */
void FighterStateAttack_attack(FighterState *this, s32 attack) {
    if (!func_8007F98C(this)) {
        FighterAttackIdle_attack(this, attack);
    }
}

/* The attack chain's state (func_800823EC, virtual table D_80067D94): a FighterState with a flag at 0x10 and the
 * step function at 0x14. */
typedef struct Unk800823EC {
    /* 0x00 */ FighterState base;
    /* 0x10 */ s32 unk10;
    /* 0x14 */ MemberFunction step;
} Unk800823EC;

/* A virtual table entry that fits a register pair (g++ keeps the one a pointer to member function reaches in
 * one). */
typedef union VirtualFunction {
    VtableEntry entry;
    u64 pair;
} VirtualFunction;

extern VtableEntry D_80067D94[];
/* the chain's first step: func_8008002C */
extern const MemberFunction D_80067CFC;

/* Turns the state, in place, into the attack chain (state 0x14, virtual table D_80067D94) on its first step
 * (D_80067CFC), worth 100 as an attack (MOVE_ATTACK1); runs the step and plays sound 0.
 * The match depends on D_80067CFC being const (.rodata: both words load before the stores) and on the step's delta
 * read once before the two sums are chosen. */
FighterState *func_8008230C(FighterState *this) {
    Unk800823EC *chain = (Unk800823EC *)this;
    VirtualFunction method;
    void (*func)();
    s32 index;
    s32 delta;
    s32 offset;

    FighterState_init(&chain->base, 0x14);
    chain->base.vtable = D_80067D94;
    Fighter_setMoveReward(chain->base.fighter, MOVE_ATTACK1, 100);
    chain->step = D_80067CFC;
    chain->unk10 = 0;
    /* (chain->*step)(), as g++ 2.95 expands it */
    index = chain->step.index;
    if (index >= 0) {
        method = (*(VirtualFunction **)((u8 *)chain + chain->step.u.vtableOffset))[index - 1];
        func = method.entry.func;
    } else {
        func = chain->step.u.func;
    }
    delta = chain->step.delta;
    offset = index >= 0 ? method.entry.delta + delta : delta;
    func((u8 *)chain + offset);
    soundControlPlaySound(&SOUND_CONTROL, 0, 0);
    return this;
}

INCLUDE_ASM("game/nonmatchings/game_0f", func_800823EC);

INCLUDE_ASM("game/nonmatchings/game_0f", func_800824B8);

/* The attack chain's attack (slot 0x20 of D_80067D54 and D_80067D94): for an attack (not a special), unless
 * func_8007F98C holds, sets 0x10, sets no action clip (func_8007FC50 with -1) and runs its slot 0x30, the next
 * link of the chain (func_8008261C in D_80067D94). */
void func_800825AC(Unk800823EC *this, s32 attack) {
    if (!func_8007F98C(&this->base) && attack == 0) {
        this->unk10 = 1;
        func_8007FC50(&this->base, -1, 0);
        this->base.vtable[6].func((u8 *)this + this->base.vtable[6].delta);
    }
}

/* The chain's next link after the first attack: the second (func_80082690). */
FighterState *func_8008261C(FighterState *this) {
    return func_80082690(this);
}

/* Makes the attack state idle (FighterAttackIdle_init) and sets no action clip (func_8007FC50 with -1). */
void func_8008263C(void *this) {
    FighterAttackIdle_init(this);
    func_8007FC50(this, -1, 0);
}

/* The chain's next link after the second attack: the third (func_800826E8). */
FighterState *func_80082670(FighterState *this) {
    return func_800826E8(this);
}

/* Starts the chain's second attack (FIGHTER_STATE_ATTACK2, virtual table D_80067D54, step D_80067D04), with its
 * reward. */
FighterState *func_80082690(FighterState *this) {
    func_800823EC(this, FIGHTER_STATE_ATTACK2, D_80067D04);
    this->vtable = D_80067D54;
    Fighter_setMoveReward(this->fighter, MOVE_ATTACK2, 100);
    return this;
}

/* Starts the chain's third attack (FIGHTER_STATE_ATTACK3, virtual table D_80067D14, step D_80067D0C), with its
 * reward. */
FighterState *func_800826E8(FighterState *this) {
    func_800823EC(this, FIGHTER_STATE_ATTACK3, D_80067D0C);
    this->vtable = D_80067D14;
    Fighter_setMoveReward(this->fighter, MOVE_ATTACK3, 200);
    return this;
}

/* The base state's tick: does nothing. */
void FighterState_tick(void) {
}

/* The base state's jump: does nothing. */
void FighterState_jump(void) {
}

/* The base state's drop: does nothing. */
void FighterState_drop(void) {
}

/* The base state's attack: does nothing. */
void FighterState_attack(void) {
}

/* The base state's hit: does nothing. */
void FighterState_hit(void) {
}

/* Returns its second argument (no caller is known: a virtual method, as its neighbours, is likely). */
s32 func_80082768(FighterState *this, s32 arg) {
    return arg;
}

/* Returns the state's FIGHTER_STATE_* id (no caller is known). */
s32 func_80082770(FighterState *this) {
    return this->state;
}

/* Constructs a pose state (virtual table D_8006811C) with its FIGHTER_STATE_* id, as func_80080778 does. */
FighterState *func_8008277C(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = D_8006811C;
    return this;
}

/* Constructs a state of virtual table D_800680E4 with its FIGHTER_STATE_* id. */
FighterState *func_800827B0(FighterState *this, s32 state) {
    FighterState_init(this, state);
    this->vtable = D_800680E4;
    return this;
}

/* Does nothing (no caller is known). */
void func_800827E4(void) {
}

INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CCC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CD4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CDC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CE4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CEC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CF4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067CFC);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D04);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D0C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D14);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D54);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067D94);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_ATTACK_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_ATTACK_IDLE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067E44);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_GET_UP_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_DOWN_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_AIR_FLINCH_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_FLINCH_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067F5C);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_80067F94);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_FALL_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_JUMP_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_HELD_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_GROUND_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_INTRO_VTABLE);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_800680E4);
INCLUDE_RODATA("game/nonmatchings/game_0f", D_8006811C);
INCLUDE_RODATA("game/nonmatchings/game_0f", FIGHTER_STATE_VTABLE);
