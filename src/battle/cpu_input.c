/* The computer's input: pad words made from random numbers for a side without a pad. */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/pad/pad.h"

/* The computer player of a side (0x6C bytes), the controller Fight_createController makes, the virtual table slot
 * the bonus stages override. */
typedef struct ComputerController {
    /* 0x00 */ Controller base;
    /* 0x08 */ s16 unk8;
    /* 0x0A */ s16 unkA;
    /* 0x0C */ u8 unkC[0x1C - 0xC];
    /* 0x1C */ Vec3 shapePosition; /* the position of the shape ComputerController_findNearestShape found */
    /* 0x28 */ u8 unk28[0x2C - 0x28];
    /* 0x2C */ Vec3 unk2C; /* that shape's unk20 */
    /* 0x38 */ u8 unk38[0x3C - 0x38];
    /* 0x3C */ s32 shapeRadius; /* that shape's radius, in 1/256ths */
    /* 0x40 */ s32 hasShape; /* whether ComputerController_findNearestShape found one */
    /* 0x44 */ u8 unk44[0x4C - 0x44];
    /* 0x4C */ s16 unk4C;
    /* 0x4E */ s16 unk4E;
    /* 0x50 */ s16 unk50;
    /* 0x52 */ s16 unk52;
    /* 0x54 */ s16 unk54;
    /* 0x56 */ s16 unk56;
    /* 0x58 */ s16 unk58;
    /* 0x5A */ s16 unk5A; /* -1 after the constructor */
    /* 0x5C */ s16 unk5C;
    /* 0x5E */ s16 unk5E;
    /* 0x60 */ u8 unk60[0x62 - 0x60];
    /* 0x62 */ s16 unk62;
    /* 0x64 */ u8 unk64[0x6C - 0x64];
} ComputerController;

extern VtableEntry COMPUTER_CONTROLLER_VTABLE[];

/* Constructs the computer player of side (its side object): clears its fields, 0x5A to -1. */
ComputerController *ComputerController_init(ComputerController *this, BattleSide *side) {
    Controller_init(&this->base, side);
    this->base.vtable = COMPUTER_CONTROLLER_VTABLE;
    this->unk8 = 0;
    this->unkA = 0;
    this->hasShape = 0;
    this->unk4E = 0;
    this->unk50 = 0;
    this->unk52 = 0;
    this->unk54 = 0;
    this->unk62 = 0;
    this->unk56 = 0;
    this->unk58 = 0;
    this->unk4C = 0;
    this->unk5A = -1;
    this->unk5C = 0;
    this->unk5E = 0;
    return this;
}

INCLUDE_RODATA("game/nonmatchings/cpu_input", D_80065434);

INCLUDE_ASM("game/nonmatchings/cpu_input", ComputerController_getButtons);

/* Looks for the opponent's collision shape nearest to fighter's func_8007A650 point: keeps its position, its
 * unk20 and its radius (in 1/256ths), and returns whether there was one. */
s32 ComputerController_findNearestShape(ComputerController *this, Fighter *fighter) {
    Fight *fight = CURRENT_FIGHT;
    Vec3 point;
    CollisionSphere *shape;

    func_8007A650(fighter, &point);
    this->hasShape = 0;
    /* The match depends on testing == 0: the game puts side 0's list in the branch's delay slot. */
    shape = shapeListFindNearest(Fighter_getSide(fighter) == 0 ? &fight->side1Shapes : &fight->side0Shapes, &point,
                                 &this->shapePosition);
    if (shape != NULL) {
        s32 x, y, z;

        this->hasShape = 1;
        /* The match depends on reading the three words before storing any, as vecCopy does. */
        x = shape->hitKnockback.vx;
        y = shape->hitKnockback.vy;
        z = shape->hitKnockback.vz;
        this->unk2C.vx = x;
        this->unk2C.vy = y;
        this->unk2C.vz = z;
        this->shapeRadius = shape->radius << 8;
    }
    return this->hasShape;
}

/* Returns flags with PAD_RIGHT set when direction is not negative, else with PAD_LEFT. */
s32 addTowardButton(s32 flags, s32 direction) {
    /* The match depends on the early return for a negative direction (an if/else gives one exit). */
    if (direction < 0) {
        return flags | PAD_LEFT;
    }
    return flags | PAD_RIGHT;
}

/* The computer player's destructor: Controller's. */
void ComputerController_destroy(Controller *this, s32 flags) {
    Controller_destroy(this, flags);
}

INCLUDE_RODATA("game/nonmatchings/cpu_input", COMPUTER_CONTROLLER_VTABLE);
