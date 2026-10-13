/* The fighter factory, func_80076CB4, which calls one constructor per fighter, and the base class of the
 * 24 fighters (virtual table D_8006530C). */

#include "common.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/game/character.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/system/memory.h"

extern VtableEntry D_800652CC[];
s32 func_8007A044(Unk80076F34 *this, s32 arg);
s32 *func_800792AC(Unk80076F34 *this);

INCLUDE_ASM("game/nonmatchings/fighter", func_80076CB4);

INCLUDE_ASM("game/nonmatchings/fighter", func_80076F34);

INCLUDE_ASM("game/nonmatchings/fighter", func_800770AC);

INCLUDE_ASM("game/nonmatchings/fighter", func_8007720C);

/* func_8006EF90 for the character's index at 0x21C, adding arg. */
void func_800773F4(Unk80076F34 *this, s32 arg) {
    func_8006EF90(this->unk228, this->unk21C, arg);
}

/* func_8007F0F4 on the object at 0x238, with arg. */
void func_80077424(Unk80076F34 *this, s32 arg) {
    func_8007F0F4(this->unk238, arg);
}

/* func_8007F1FC on the object at 0x238, with arg. */
void func_80077448(Unk80076F34 *this, s32 arg) {
    func_8007F1FC(this->unk238, arg);
}

/* Calls func_80079350 when func_8007F5A8 on the object at 0x238 says so. */
void func_8007746C(Unk80076F34 *this) {
    if (func_8007F5A8(this->unk238)) {
        func_80079350(this);
    }
}

/* Clears the timed thing at 0x464. */
void func_800774A8(Unk80076F34 *this) {
    this->unk464.unk0 = 0;
    this->unk464.unk4 = 0;
    this->unk464.unk8 = 0;
    this->unk464.unkC = 0;
    this->unk464.unk10 = 0;
}

/* Starts the timed thing at 0x464. */
void func_800774C0(Unk80076F34 *this, s32 unk0, s32 unk4, s32 unk8) {
    this->unk464.unk0 = unk0;
    this->unk464.unk4 = unk4;
    this->unk464.unk8 = unk8;
    this->unk464.unkC = 0;
    this->unk464.unk10 = 0;
}

/* Runs the timed thing at 0x464 while it is set. */
void func_800774D8(Unk80076F34 *this) {
    if (this->unk464.unk0 != 0) {
        func_800C9790(this);
    }
}

/* Clears the timed thing at 0x478. */
void func_80077508(Unk80076F34 *this) {
    this->unk478.unk0 = 0;
    this->unk478.unk4 = 0;
    this->unk478.unk8 = 0;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Starts the timed thing at 0x478. */
void func_80077520(Unk80076F34 *this, s32 unk0, s32 unk4, s32 unk8) {
    this->unk478.unk0 = unk0;
    this->unk478.unk4 = unk4;
    this->unk478.unk8 = unk8;
    this->unk478.unkC = 0;
    this->unk478.unk10 = 0;
}

/* Runs the timed thing at 0x478 while it is set. */
void func_80077538(Unk80076F34 *this) {
    if (this->unk478.unk0 != 0) {
        func_800C99B0(this);
    }
}

/* Returns unk218. */
s32 func_80077568(Unk80076F34 *this) {
    return this->unk218;
}

/* Returns unk21C. */
s32 func_80077574(Unk80076F34 *this) {
    return this->unk21C;
}

/* Returns unk228. */
Unk8006ED20 *func_80077580(Unk80076F34 *this) {
    return this->unk228;
}

/* Returns unk238. */
Unk8007F184 *func_8007758C(Unk80076F34 *this) {
    return this->unk238;
}

/* Returns bit 0 of the flags at 0x37C. */
s32 func_80077598(Unk80076F34 *this) {
    return this->flags & 1;
}

/* Returns the character's model (0x3FC). */
Model *func_800775A4(Unk80076F34 *this) {
    return this->model;
}

/* Sets the character's position from whole units (0x384 keeps it in 1/256ths; func_800775D8 reads it back).
 * The match depends on reading the three words before storing any, as vecCopy does. */
void func_800775B0(Unk80076F34 *chara, Vec3 *in) {
    s32 x = in->vx << 8;
    s32 y = in->vy << 8;
    s32 z = in->vz << 8;

    chara->position.vx = x;
    chara->position.vy = y;
    chara->position.vz = z;
}

/* The character's position in whole units (0x384 keeps it in 1/256ths); returns out. */
Vec3 *func_800775D8(Unk80076F34 *chara, Vec3 *out) {
    out->vx = chara->position.vx >> 8;
    out->vy = chara->position.vy >> 8;
    out->vz = chara->position.vz >> 8;
    return out;
}

/* The vector at 0x394 in whole units (it is kept in 1/256ths); returns out. */
Vec3 *func_8007760C(Unk80076F34 *chara, Vec3 *out) {
    out->vx = chara->unk394.vx >> 8;
    out->vy = chara->unk394.vy >> 8;
    out->vz = chara->unk394.vz >> 8;
    return out;
}

/* The character's position plus the vector at 0x3A4, in whole units; returns out. */
Vec3 *func_80077640(Unk80076F34 *chara, Vec3 *out) {
    out->vx = (chara->position.vx + chara->unk3A4.vx) >> 8;
    out->vy = (chara->position.vy + chara->unk3A4.vy) >> 8;
    out->vz = (chara->position.vz + chara->unk3A4.vz) >> 8;
    return out;
}

/* Returns unk440. */
s32 func_8007768C(Unk80076F34 *this) {
    return this->unk440;
}

/* Returns the character's table at 0x220. */
Unk80077698 *func_80077698(Unk80076F34 *this) {
    return this->unk220;
}

extern s8 D_80067AD4[][10];

/* The row unk218 of the table D_80067AD4 (10 bytes each; func_8007A044 reads it). */
s8 *func_800776A4(Unk80076F34 *this) {
    /* The match depends on the table taken into a local first (the game computes its address before the load). */
    s8 (*rows)[10] = D_80067AD4;

    return rows[this->unk218];
}

/* Returns the character's input (0x234). */
Unk80075000 *func_800776C4(Unk80076F34 *this) {
    return this->unk234;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_800776D0);

/* Whether bit 0 of the first word of the character's table at 0x220 (func_80077698) is clear. */
s32 func_80077794(Unk80076F34 *this) {
    return !(func_80077698(this)->unk0 & 1);
}

/* Returns bit 1 of the flags at 0x37C. */
s32 func_800777BC(Unk80076F34 *this) {
    return (this->flags >> 1) & 1;
}

/* Adds the character's model to group. */
void func_800777D0(Unk80076F34 *this, Group *group) {
    /* The match depends on reading the model into a local first (the game loads it before moving group). */
    Model *model = this->model;

    groupAddChild(group, &model->group.object);
}

/* Takes the character's model out of the group it is a child of. */
void func_800777F8(Unk80076F34 *this) {
    listRemove(&this->model->group.object.link);
}

/* Returns the flag bit func_80079AE4 sets, and clears it. */
s32 func_8007782C(Unk80076F34 *this) {
    u32 flags = this->flags;
    u32 shifted = flags >> UNK80076F34_FLAG_AE4_BIT;

    /* The match depends on flags masked in place (set twice): a set-once value lets sched1 move the shift. */
    flags &= ~(1 << UNK80076F34_FLAG_AE4_BIT);
    this->flags = flags;
    return shifted & 1;
}

/* Picks a node of D_8006437C (func_80076474) for the segment from 0x26 to 0x1E26 above the character's
 * position; returns what func_80076474 returns. The USA release first checks bit 3 of the flags at 0x37C. */
#if VERSION_JP
s32 func_80077848(Unk80076F34 *this) {
    Vec3 from;
    Vec3 to;
    Unk80076444 picker;

    func_800775D8(this, &from);
    from.vy += 0x26;
    func_800775D8(this, &to);
    to.vy += 0x1E26;
    func_80076444(&picker);
    return func_80076474(&picker, &from, &to);
}
#else
s32 func_80077848(Unk80076F34 *this) {
    Vec3 from;
    Vec3 to;
    Unk80076444 picker;

    if (!((this->flags >> 3) & 1)) {
        return 0;
    }
    func_800775D8(this, &from);
    from.vy += 0x26;
    func_800775D8(this, &to);
    to.vy += 0x1E26;
    func_80076444(&picker);
    return func_80076474(&picker, &from, &to);
}
#endif

/* Returns bit 2 of the flags at 0x37C. */
s32 func_800778BC(Unk80076F34 *this) {
    return (this->flags >> 2) & 1;
}

/* Sets bit n of 0x380 and returns 1, or returns 0 when it is set already. */
s32 func_800778D0(Unk80076F34 *this, s32 n) {
    s32 bit = 1 << n;

    if (this->unk380 & bit) {
        return 0;
    }
    this->unk380 |= bit;
    return 1;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_800778FC);

INCLUDE_ASM("game/nonmatchings/fighter", func_80077B20);

/* When func_8007AED0 holds: func_80079B2C with whether the first word of the object at 0x22C is 0xB to 0xE or
 * 0x10. */
void func_80077C70(Unk80076F34 *this) {
    if (func_8007AED0(this)) {
        s32 value = func_800792A0(this)->unk0;

        func_80079B2C(this, (value >= 0xB && value <= 0xE) || value == 0x10);
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80077CD4);

INCLUDE_ASM("game/nonmatchings/fighter", func_80077E2C);

INCLUDE_ASM("game/nonmatchings/fighter", func_80077F80);

/* Whether bit 2 of the flags at 0x37C is set (func_800778BC) and the node of D_8006437C picked last holds
 * matrix (func_80076598). */
s32 func_8007814C(Unk80076F34 *this, MATRIX *matrix) {
    if (!func_800778BC(this)) {
        return 0;
    }
    return matrix == func_80076598(&this->unk260);
}

/* How far below the vector at 0x394 func_8007819C's second segment starts: 0x80, 0x40 in the USA release. */
#if VERSION_JP
#define FUNC_8007819C_DROP 0x80
#else
#define FUNC_8007819C_DROP 0x40
#endif

/* Whether a node of D_8006437C is picked (func_80076474 on the member at 0x260) for the segment from 0x80
 * below to 8 above the character's position, or else for the one from below the vector at 0x394 to the
 * position. */
s32 func_8007819C(Unk80076F34 *this) {
    Unk80076444 *picker = &this->unk260;
    Vec3 from;
    Vec3 to;

    func_800775D8(this, &from);
    func_800775D8(this, &to);
    from.vy -= 0x80;
    to.vy += 8;
    if (func_80076474(picker, &from, &to)) {
        return 1;
    }
    func_8007760C(this, &from);
    from.vy -= FUNC_8007819C_DROP;
    func_800775D8(this, &to);
    return func_80076474(picker, &from, &to) != 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078254);

INCLUDE_ASM("game/nonmatchings/fighter", func_8007835C);

extern s32 D_800D3790;

/* out = (-0x3A4.vx scaled by 0x3C4, then by D_800D3790, both 16.16), 0, 0; returns out. */
Vec3 *func_80078418(Unk80076F34 *this, Vec3 *out) {
    s32 scaled = ((s64)-this->unk3A4.vx * this->unk3C4) >> 16;

    out->vy = 0;
    out->vz = 0;
    out->vx = ((s64)scaled * D_800D3790) >> 16;
    return out;
}

/* func_80072F80 on the vertical segment from 0x100 above to 0x4C below the character's position, moved along x by
 * distance times its facing (func_80079E10); returns what it returns. */
s32 func_80078478(Unk80076F34 *this, s32 distance) {
    Vec3 from;
    Vec3 to;
    s32 delta;

    func_800775D8(this, &from);
    delta = distance * func_80079E10(this);
    to.vz = from.vz;
    to.vy = from.vy - 0x4C;
    from.vy += 0x100;
    from.vx += delta;
    to.vx = from.vx;
    return func_80072F80(&from, &to);
}

/* Plays sound in the character's group (0x21C, 0x218) + 0x40 and keeps its handle at 0x25C. */
s32 func_800784F4(Unk80076F34 *this, s32 sound) {
    s32 handle = soundControlPlaySound(&SOUND_CONTROL, sound, (this->unk21C << 7) + this->unk218 + 0x40);

    this->unk25C = handle;
    return handle;
}

/* Plays sound in the character's group (0x21C, 0x218). */
s32 func_80078540(Unk80076F34 *this, s32 sound) {
    /* The match depends on the u8 local: the game reads both fields as bytes (lbu). */
    u8 group = this->unk218 + (this->unk21C << 7);

    return soundControlPlaySound(&SOUND_CONTROL, sound, group);
}

/* Steps an angle by diff: all of it when it is within 3, else a quarter of it.
 * The match depends on reading the angle first and on a new result in each arm. */
static inline void stepAngle(s32 *angle, s32 diff) {
    s32 value = *angle;
    s32 result;

    if (diff < -3 || diff > 3) {
        result = value + diff / 4;
    } else {
        result = value + diff;
    }
    *angle = result;
}

/* Turns the three angles at 0x3D8 toward the ones at 0x3E8, the short way round (12-bit angles). */
void func_80078578(Unk80076F34 *this) {
    s32 dx = ((this->unk3E8 - this->unk3D8 - 0x800) & 0xFFF) - 0x800;
    s32 dy = ((this->unk3EC - this->unk3DC - 0x800) & 0xFFF) - 0x800;
    s32 dz = ((this->unk3F0 - this->unk3E0 - 0x800) & 0xFFF) - 0x800;

    stepAngle(&this->unk3D8, dx);
    stepAngle(&this->unk3DC, dy);
    stepAngle(&this->unk3E0, dz);
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078660);

/* Sets 0x3EC to -0x401 (facing -1), and 0x3DC too when arg is set. */
void func_80078804(Unk80076F34 *this, s32 arg) {
    this->unk3EC = -0x401;
    if (arg) {
        this->unk3DC = -0x401;
    }
}

/* Sets 0x3EC to 0x401 (facing 1), and 0x3DC too when arg is set. */
void func_8007881C(Unk80076F34 *this, s32 arg) {
    this->unk3EC = 0x401;
    if (arg) {
        this->unk3DC = 0x401;
    }
}

/* Sets 0x3EC to 0x800. */
void func_80078834(Unk80076F34 *this) {
    this->unk3EC = 0x800;
}

/* Clears 0x3EC. */
void func_80078840(Unk80076F34 *this) {
    this->unk3EC = 0;
}

/* func_80075034 on arg with whether 0x3EC is -0x401 (facing -1). */
void func_80078848(Unk80076F34 *this, void *arg) {
    func_80075034(arg, this->unk3EC == -0x401);
}

/* Returns the entity at 0x14 of sum when func_8002BD68 gives 0x10000 for it, else NULL. */
Entity *func_80078878(Unk80075EE4 *sum) {
    Entity *entity = sum->unk14;

    if (func_8002BD68(entity) != 0x10000) {
        return NULL;
    }
    return entity;
}

struct Unk8007BF74; /* the counter pair defined at func_8007BF74 */
void func_8007BF84(struct Unk8007BF74 *this, u16 unk0, u16 unk2);

/* Sets the counter pair at index of 0x348 (func_8007BF84) to value and the table's (func_80077698) word at
 * index of 0x1C. */
void func_800788B4(Unk80076F34 *this, s32 index, s32 value) {
    func_8007BF84((struct Unk8007BF74 *)this->unk348[index], value, func_80077698(this)->unk1C[index]);
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078910);

/* Keeps arg at 0x234 and passes it to the virtual method at 0x30 (func_800778FC
 * calls it with what func_80073260 returns). */
void func_80078C28(Unk80076F34 *this, struct Unk80075000 *arg) {
    Unk80076F34Vtable *vtable;

    this->unk234 = arg;
    vtable = UNK80076F34_VTABLE(this);
    vtable->unk30.func((u8 *)this + vtable->unk30.delta, arg);
}

/* Starts a vibration of the character's pad (port 0x21C), unless bit 0 of the flags is set, a demo plays or
 * the save data's word at 0xC (likely the vibration option) is 0. */
void func_80078C5C(Unk80076F34 *this, s32 mode, u8 power0, u8 power1, s32 duration, s32 priority) {
    PadManager *padManager;

    if (this->flags & 1) {
        return;
    }
    if (GAME_STATE.demo != 0) {
        return;
    }
    if (GAME_STATE.save.unkC == 0) {
        return;
    }
    /* The match depends on reading the pad manager in its own statement: the game loads it before the
     * stack arguments. */
    padManager = PAD_MANAGER_INSTANCE;
    padManagerStartVibration(padManager, this->unk21C, mode, power0, power1, duration, priority);
}

/* Writes 0x3A4 scaled by 0x3C4 (16.16) to out and returns it. */
Vec3 *func_80078CE0(Unk80076F34 *this, Vec3 *out) {
    s32 scale = this->unk3C4;

    out->vx = ((s64)this->unk3A4.vx * scale) >> 16;
    out->vy = ((s64)this->unk3A4.vy * scale) >> 16;
    out->vz = ((s64)this->unk3A4.vz * scale) >> 16;
    return out;
}

/* Whether the first word of the object at 0x22C is 0xD to 0xF, the word 0x230 points to is 0x18, or 0x498 is
 * above 0. */
s32 func_80078D64(Unk80076F34 *this) {
    s32 state = func_800792A0(this)->unk0;
    s32 mode = *func_800792AC(this);
    s32 result = 0;

    if (state == 0xF || state == 0xD || state == 0xE || mode == 0x18 || this->unk498 > 0) {
        result = 1;
    }
    return result;
}

/* Whether the first word of the object at 0x22C is 9 or 10. */
s32 func_80078DE0(Unk80076F34 *this) {
    s32 value = func_800792A0(this)->unk0;

    return value >= 9 && value <= 10;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078E08);

/* While the timed thing at 0x464 is of kind 1 or 2, scales the count in the low 18 bits of value by its unk8
 * (in 1/256ths), keeping the bits above. */
u32 func_80078E6C(Unk80076F34 *this, u32 value) {
    if (this->unk464.unk0 == 1 || this->unk464.unk0 == 2) {
        /* The match depends on one variable for the count's steps (separate expressions take other
         * registers). */
        s32 count = value & 0x3FFFF;

        count = (count * this->unk464.unk8) >> 8;
        value = count | (value & 0xFFFC0000);
    }
    return value;
}

/* percentages (-5, 0 or 5) by two indexes of 0 to 3 */
extern s8 D_8006520C[4][4];

INCLUDE_RODATA("game/nonmatchings/fighter", D_8006520C);

/* value with the percentage D_8006520C[row][column] added. */
s32 func_80078EB0(s32 row, s32 column, s32 value) {
    return value * (D_8006520C[row][column] + 100) / 100;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80078EF8);

/* Sets 0x258 to 0x12. */
void func_8007925C(Unk80076F34 *this) {
    this->unk258 = 0x12;
}

/* Sets 0x258 to 0x11. */
void func_80079268(Unk80076F34 *this) {
    this->unk258 = 0x11;
}

/* func_800807F4 on the object at 0x22C, with arg. */
void *func_80079274(Unk80076F34 *this, s32 arg) {
    return func_800807F4(this->unk22C, arg);
}

void func_80079298(void) {
}

/* Returns the object at 0x22C. */
Unk8007F920 *func_800792A0(Unk80076F34 *this) {
    return this->unk22C;
}

/* Returns the pointer at 0x230. */
s32 *func_800792AC(Unk80076F34 *this) {
    return (s32 *)this->unk230;
}

/* The first word of the object at 0x22C (func_800792A0). */
s32 func_800792B8(Unk80076F34 *this) {
    return func_800792A0(this)->unk0;
}

/* The word 0x230 points to (func_800792AC). */
s32 func_800792DC(Unk80076F34 *this) {
    return *func_800792AC(this);
}

/* Clears 0x18 to 0x2C of the Unk80075EE4 at 0x2DC and returns it (func_800708F4 hands it to func_80075CEC). */
Unk80075EE4 *func_80079300(Unk80076F34 *this) {
    this->unk2DC.unk18 = 0;
    this->unk2DC.unk1C = 0;
    this->unk2DC.unk20.vx = 0;
    this->unk2DC.unk20.vy = 0;
    this->unk2DC.unk20.vz = 0;
    this->unk2DC.unk2C = 0;
    return &this->unk2DC;
}

/* Stops the model's clip (func_80079830 with -1). */
void func_80079320(Unk80076F34 *this) {
    func_80079830(this, -1, 0, 0, -1);
}

/* While the object at 0x238 holds some (0x20 above 0): func_800774A8, func_80077508, then a new handle
 * for the character (func_8007F6FC; Unk8007F6FC is that object's class seen from its 0x30). */
void func_80079350(Unk80076F34 *this) {
    if (this->unk238->unk20 > 0) {
        func_800774A8(this);
        func_80077508(this);
        func_8007F6FC((Unk8007F6FC *)this->unk238);
    }
}

void func_80079420(Unk80076F34 *this);

/* Moves the character to the 0x228 object's group at 0x5C (func_8006F0E4), makes its entity (func_80079420)
 * and sets the 0x228 object's 0xD30 to the side + 1; returns whether that entity exists. */
s32 func_800793A4(Unk80076F34 *this) {
    listRemove(&this->entity.link);
    func_8006F0E4(this->unk228, &this->entity);
    func_80079420(this);
    this->unk228->unkD30 = this->unk21C + 1;
    return getEntityByHandle(this->unk248) != NULL;
}

/* Constructs, in place, the entity of class D_8006D3E4 for side (game_0g). */
Entity *func_800C4204(void *place, s32 side);

/* For the characters of index 0, 2, 3, 11, 12 and 17 to 20 at 0x218: makes the func_800C4204 entity of its
 * side, keeps its handle at 0x248 and plays the side's fighter sound. */
void func_80079420(Unk80076F34 *this) {
    Entity *entity;

    switch (this->unk218) {
    case 0:
    case 2:
    case 3:
    case 11:
    case 12:
    case 17:
    case 18:
    case 19:
    case 20:
        entity = func_800C4204(entityOperatorNew(0x40), this->unk21C);
        this->unk248 = entityGetHandle(entity);
        entityAddSibling(&this->entity, entity);
        soundControlPlaySound(&SOUND_CONTROL, gameStateGetFighterSound(&GAME_STATE, this->unk21C), 0);
        break;
    }
}

/* Destroys the entity of 0x248 and moves the character to the 0x228 object's group at 0x4C (func_8006F0B4),
 * clearing that object's 0xD30. */
void func_800794CC(Unk80076F34 *this) {
    destroyEntityByHandle(this->unk248);
    this->unk248 = 0;
    listRemove(&this->entity.link);
    func_8006F0B4(this->unk228, &this->entity);
    this->unk228->unkD30 = 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079534);

/* Unk80076F34's Chara (0x3F8) and the blend frames of its clip at 0x250 (0x24C) sit in gaps of
 * game_classes.h, which this file cannot change: these name them until the header does (RESULT.md). */
#define CHARA_OF(chara) (*(Chara **)((u8 *)(chara) + 0x3F8))
#define CLIP_BLEND_FRAMES_OF(chara) (*(s32 *)((u8 *)(chara) + 0x24C))

/* Starts clip with the play count of the character's .loop file, keeping clip and blendFrames at 0x250/0x24C,
 * when func_80079A0C allows it; a negative clip stops the model. */
void func_800796F0(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->unk250 = clip;
    CLIP_BLEND_FRAMES_OF(this) = blendFrames;
    if (func_80079A0C(this)) {
        modelStartClip(model, clip, blendFrames, charaGetClipPlays(CHARA_OF(this), clip), frame);
        modelSetLoopFrame(model, charaGetLoopFrame(CHARA_OF(this), clip));
    }
}

/* Starts clip and runs the model's animation twice, then stops the model. */
void func_800797B8(Unk80076F34 *this, s32 clip) {
    Model *model = this->model;
    s32 i;

    /* The match depends on counting down to 0 (an upward count is reversed into a bgez loop). */
    for (i = 2; i != 0; i--) {
        modelStartClip(model, clip, 0, 0, 0);
        modelUpdateAnimation(model);
    }
    modelStartClip(model, -1, 0, 0, 0);
}

/* Starts clip on the model (a negative one stops it), keeping it at 0x254; plays < 0 takes the count from the
 * character's .loop file. The match depends on the ternary: assigning plays loads it at the entry. */
void func_80079830(Unk80076F34 *this, s32 clip, s32 blendFrames, s32 frame, s32 plays) {
    Model *model = this->model;

    if (clip < 0) {
        modelStartClip(model, -1, 0, 0, 0);
        return;
    }
    this->unk254 = clip;
    modelStartClip(model, clip, blendFrames, plays < 0 ? charaGetClipPlays(CHARA_OF(this), clip) : plays, frame);
    modelSetLoopFrame(model, charaGetLoopFrame(CHARA_OF(this), clip));
}

/* Whether the model has no clip of that number, or the character's .loop file plays it 0x1000 times or
 * more. */
s32 func_800798F4(Unk80076F34 *this, s32 clip) {
    s32 plays;

    if (animatorIsClipEmpty(&this->model->animator, clip)) {
        return 1;
    }
    /* The match depends on the s32 local: comparing the u16 itself gives an unsigned sltiu. */
    plays = charaGetClipPlays(CHARA_OF(this), clip);
    return plays >= 0x1000;
}

/* Whether the model is stopped; once it is, a clip kept at 0x254 is forgotten (-1). */
s32 func_80079950(Unk80076F34 *this) {
    s32 stopped = modelIsStopped(this->model);

    if (stopped && this->unk254 >= 0) {
        this->unk254 = -1;
    }
    return stopped;
}

/* Whether the model is done with the clip at 0x250: stopped, no clip there, or playing another. */
s32 func_800799A0(Unk80076F34 *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->unk250 < 0) {
        return 1;
    }
    return modelGetClip(model) != this->unk250;
}

/* Whether the model is done with the clip at 0x254: stopped, no clip there, or playing another. */
s32 func_80079A0C(Unk80076F34 *this) {
    Model *model = this->model;

    if (modelIsStopped(model)) {
        return 1;
    }
    if (this->unk254 < 0) {
        return 1;
    }
    return modelGetClip(model) != this->unk254;
}

/* Sets a flag, func_80077424 with 1, clears 0x3A4 to 0x3C0, and calls func_800808EC with 1 and func_800819E0 on
 * the objects at 0x22C and 0x230. */
void func_80079A78(Unk80076F34 *this) {
    this->flags |= 1 << UNK80076F34_FLAG_A78_BIT;
    func_80077424(this, 1);
    this->unk3B4.vx = 0;
    this->unk3B4.vy = 0;
    this->unk3B4.vz = 0;
    this->unk3C0 = 0;
    this->unk3A4.vx = 0;
    this->unk3A4.vy = 0;
    this->unk3A4.vz = 0;
    this->unk3B0 = 0;
    func_800808EC(this->unk22C, 1);
    /* 0x230 is kept as a word in game_classes.h; it holds an Unk8007F920 as 0x22C does */
    func_800819E0((Unk8007F920 *)this->unk230);
}

/* Sets a flag at 0x37C (bit 6, bit 7 in the USA release). */
void func_80079AE4(Unk80076F34 *this) {
    this->flags |= 1 << UNK80076F34_FLAG_AE4_BIT;
}

/* Sets a flag at 0x37C (bit 5, bit 6 in the USA release). */
void func_80079AF8(Unk80076F34 *this) {
    this->flags |= 1 << UNK80076F34_FLAG_AF8_BIT;
}

/* Sets 0x334 to arg. */
void func_80079B0C(Unk80076F34 *this, s32 arg) {
    this->unk334 = arg;
}

/* Sets 0x338 to arg. */
void func_80079B14(Unk80076F34 *this, s32 arg) {
    this->unk338 = arg;
}

/* Sets 0x33C to arg. */
void func_80079B1C(Unk80076F34 *this, s32 arg) {
    this->unk33C = arg;
}

/* Gives the character its table at 0x224. */
void func_80079B24(Unk80076F34 *this, Unk80079B24Entry *table) {
    this->unk224 = table;
}

/* Sets the character's 0x444 to arg, unless it holds arg's complement. */
void func_80079B2C(Unk80076F34 *this, s32 arg) {
    /* game_classes.h keeps 0x444 as padding bytes */
    s32 *unk444 = (s32 *)this->unk444;

    if (~*unk444 != arg) {
        *unk444 = arg;
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079B4C);

/* Moves 0x3A4 by 0x3B4 (divided by 0x3C4, func_80079C1C), then clears 0x3B4 and 0x3C0. */
void func_80079BD8(Unk80076F34 *this) {
    Vec3 mid;

    func_80079C1C(&mid, &this->unk3A4, &this->unk3B4, this->unk3C4);
    this->unk3B4.vx = 0;
    this->unk3B4.vy = 0;
    this->unk3B4.vz = 0;
    this->unk3C0 = 0;
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079C1C);
extern s16 D_80065270[]; /* distances added by func_80079CF4, by its index */

INCLUDE_RODATA("game/nonmatchings/fighter", D_80065270);

/* NULL unless the other character (func_80073218) passes func_80079D94; else func_80079E50 with AXIS_Z, the
 * square of both characters' func_80079DD8 plus D_80065270[index], and 0x155. */
Unk80076F34 *func_80079CF4(Unk80076F34 *this, s32 index) {
    Unk80076F34 *other = func_80073218(this->unk228, this);
    s32 distance;

    if (!func_80079D94(other)) {
        return 0;
    }
    distance = func_80079DD8(this) + func_80079DD8(other) + D_80065270[index];
    return func_80079E50(this, &AXIS_Z, distance * distance, 0x155);
}

/* Returns bit 3 of the flags at 0x37C, or 0 while func_80078D64 holds. */
s32 func_80079D94(Unk80076F34 *this) {
    if (func_80078D64(this) != 0) {
        return 0;
    }
    return (this->flags >> 3) & 1;
}

/* Three quarters of the half word at 0x2 of the character's table (func_80077698). */
s32 func_80079DD8(Unk80076F34 *this) {
    /* game_classes.h keeps 0x0 of the table as one word (func_80077794 reads its low half) */
    s32 size = func_80077698(this)->unk2;

    return size - size / 4;
}

/* The way the character faces: 1 when 0x3EC is 0x401, else -1. */
s32 func_80079E10(Unk80076F34 *this) {
    s32 side = -1;

    if (this->unk3EC == 0x401) {
        side = 1;
    }
    return side;
}

/* func_8007FB9C on the object at 0x22C. */
s32 func_80079E2C(Unk80076F34 *this) {
    return func_8007FB9C(this->unk22C);
}

INCLUDE_ASM("game/nonmatchings/fighter", func_80079E50);

INCLUDE_ASM("game/nonmatchings/fighter", func_80079FEC);

INCLUDE_ASM("game/nonmatchings/fighter", func_8007A044);

/* Adds the bits func_8007A044 returns for arg to 0x448. */
void func_8007A0B4(Unk80076F34 *this, s32 arg) {
    this->unk448 |= func_8007A044(this, arg);
}

/* Stores what func_8007A044 returns for arg at 0x448. */
void func_8007A0E4(Unk80076F34 *this, s32 arg) {
    this->unk448 = func_8007A044(this, arg);
}

/* Returns 0x340 (func_8007A118 counts it up). */
s32 func_8007A10C(Unk80076F34 *this) {
    return this->unk340;
}

/* Counts up 0x340. */
void func_8007A118(Unk80076F34 *this) {
    this->unk340++;
}

/* With on, sets 0x494 to 5 unless it is above 0 already; else clears it. */
void func_8007A12C(Unk80076F34 *this, s32 on) {
    if (on) {
        if (this->unk494 <= 0) {
            this->unk494 = 5;
        }
    } else {
        this->unk494 = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_8007A158);

/* Tests func_80072F80 toward point (raised by dy) from both sides, a third more than width away: -1 for the left
 * one only, 1 for the right one only, else 0. */
s32 func_8007A3EC(Vec3 *point, s32 dy, s32 width) {
    Vec3 left;
    Vec3 center;
    Vec3 right;
    s32 reach = width + width / 3;
    s32 side = 0;

    center.vx = point->vx;
    center.vz = point->vz;
    center.vy = point->vy + dy;
    left.vx = center.vx - reach;
    left.vy = center.vy;
    left.vz = center.vz;
    right.vx = center.vx + reach;
    right.vy = center.vy;
    right.vz = center.vz;
    if (func_80072F80(&left, &center)) {
        side = -1;
    }
    if (func_80072F80(&right, &center)) {
        side++;
    }
    return side;
}

#if VERSION_US
/* Sets 0x48C to 8 (the JP build stores it in func_80080B4C itself). */
void func_8007A718(Unk80076F34 *this) {
    this->unk48C = 8;
}
#endif

INCLUDE_ASM("game/nonmatchings/fighter", func_8007A4A4);

/* Writes the point halfway between the model's nodes node0 and node1 (each moved by its offset,
 * func_8007A4A4) to out and returns it. */
VECTOR *func_8007A598(Unk80076F34 *this, VECTOR *out, s32 node0, VECTOR *offset0, s32 node1, VECTOR *offset1) {
    VECTOR first;

    func_8007A4A4(this, &first, node0, offset0);
    func_8007A4A4(this, out, node1, offset1);
    out->vx = (out->vx + first.vx) / 2;
    out->vy = (out->vy + first.vy) / 2;
    out->vz = (out->vz + first.vz) / 2;
    return out;
}

/* Writes the character's position (func_800775D8) to out, raised by the half word at 0x2 of its table, and
 * returns it. */
Vec3 *func_8007A650(Unk80076F34 *this, Vec3 *out) {
    func_800775D8(this, out);
    /* game_classes.h keeps 0x0 of the table as one word (func_80077794 reads its low half) */
    out->vy -= func_80077698(this)->unk2;
    return out;
}

/* Writes the position of this one's character, in whole units, to out and returns it. */
Vec3 *func_8007A69C(Unk80076F34 *this, Vec3 *out) {
    return func_800775D8(func_8007AEEC(this), out);
}

/* As func_8007A69C, through func_8007A650. */
Vec3 *func_8007A6CC(Unk80076F34 *this, Vec3 *out) {
    return func_8007A650(func_8007AEEC(this), out);
}

/* With a count above 0 (at most 8), spawns func_800C5DF4's entity (kind 6, color 0xFFFFFF) at the character's
 * position, moving 14 * dir the way it faces and 6 up, and hands it to func_8007AD4C. */
void func_8007A6FC(Unk80076F34 *this, s32 dir, s32 arg2, s32 count) {
    if (count > 0) {
        Vec3 position;
        Vec3 velocity;
        s32 speed;
        Entity *child;

        if (count > 8) {
            count = 8;
        }
        /* The match depends on speed: scaling dir itself reorders the parameter copies. */
        speed = dir * func_80079E10(this);
        func_800775D8(this, &position);
        velocity.vx = speed * 14;
        velocity.vy = -6;
        velocity.vz = 0;
        child = allocEntityMemory(0x48);
        if (child != NULL) {
            func_800C5DF4(entityOperatorNewAt(0x48, child), 6, &position, &velocity, arg2, count, 0xFFFFFF);
        }
        func_8007AD4C(this, child);
    }
}

Vec3 *func_8007A920(Unk80076F34 *this, Vec3 *out);

/* Spawns func_800C32E0's entity (0x48 bytes, color 0x808080) with arg at the point func_8007A920 gives, and
 * hands it to func_8007AD4C. */
void func_8007A7DC(Unk80076F34 *this, s32 arg) {
    Vec3 position;
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_800C32E0(entityOperatorNewAt(0x48, child), func_8007A920(this, &position), &D_80010814, arg, 0x808080);
    }
    func_8007AD4C(this, child);
}

/* Spawns the entity of vtable D_8006D4D4 (func_800C2FBC with frame 8, no velocity, the size given, color
 * 0xA0A0A0) at the point func_8007A920 gives, and hands it to func_8007AD4C. */
void func_8007A874(Unk80076F34 *this, s32 size) {
    Vec3 position;
    Entity *entity;
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        entity = entityOperatorNewAt(0x58, child);
        func_800C2FBC((Unk800C2F14 *)entity, 8, (VECTOR *)func_8007A920(this, &position), &D_80010814,
                      size, 0xA0A0A0, 0);
        entity->vtable = &D_8006D4D4;
    }
    func_8007AD4C(this, child);
}

/* Writes the point a quarter of the way from the other character's position (func_8007A6CC) toward this one's
 * (func_8007A650), in x and y, to out and returns it. */
Vec3 *func_8007A920(Unk80076F34 *this, Vec3 *out) {
    Vec3 own;

    func_8007A650(this, &own);
    func_8007A6CC(this, out);
    out->vx += (own.vx - out->vx) / 4;
    out->vy += (own.vy - out->vy) / 4;
    return out;
}

/* func_8007A874 with 0x60 when the other character passes func_80078DE0, else func_8007A7DC with 0x100. */
void func_8007A9B0(Unk80076F34 *this) {
    if (func_80078DE0(func_8007AEEC(this))) {
        func_8007A874(this, 0x60);
    } else {
        func_8007A7DC(this, 0x100);
    }
}

/* An item of the lists the tables of func_80079B24 point to (Unk80079B24Entry.unk0, count of them): a
 * rectangle of the frame buffer and where func_8007A9FC moves it. */
typedef struct Unk8007A9FCItem {
    /* 0x0 */ DVECTOR *position;
    /* 0x4 */ RECT *rect;
} Unk8007A9FCItem;

DR_MOVE *allocDrMove(RECT *rect, s32 x, s32 y);

/* Draws the list of the table entry at index 0x444 (when there is one, then marks it done by complementing
 * the index): a DR_MOVE for each item, 0x100 lower in both rectangle and position for a side other than 0. */
void func_8007A9FC(Unk80076F34 *this, Camera *camera) {
    /* game_classes.h keeps 0x444 as padding bytes */
    s32 *unk444 = (s32 *)this->unk444;
    Unk80079B24Entry *table = this->unk224;
    Unk80079B24Entry *entry;
    Unk8007A9FCItem *item;
    Unk8007A9FCItem *end;
    s32 index;
    s16 dy; /* The match depends on its type: a frame buffer coordinate, as RECT's (s32 swaps the add) */
    u_long *ot;

    if (table == NULL) {
        return;
    }
    index = *unk444;
    if (index < 0) {
        return;
    }
    entry = &table[index];
    *unk444 = ~index;
    item = entry->unk0;
    end = item + entry->count;
    dy = (this->unk21C != 0) << 8;
    ot = camera->ot.firstDrawn;
    while (item != end) {
        RECT rect = *item->rect;
        DVECTOR *position;

        rect.y += dy;
        position = item->position;
        item++;
        AddPrim(ot, allocDrMove(&rect, position->vx, position->vy + dy));
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_8007AAE4);

/* Returns 0x324. */
s32 func_8007ACC4(Unk80076F34 *this) {
    return this->unk324;
}

/* Returns 0x328. */
s32 func_8007ACD0(Unk80076F34 *this) {
    return this->unk328;
}

/* Returns the word at 0x32C. */
s32 func_8007ACDC(Unk80076F34 *this) {
    return this->unk32C;
}

/* Returns the word at 0x330. */
s32 func_8007ACE8(Unk80076F34 *this) {
    return this->unk330;
}

/* Adds node to one of the two lists of the object at 0x228, picked by 0x21C (0: the list at 0x230,
 * 1: the one at 0x228). node is passed on untouched in $a1. */
void func_8007ACF4(Unk80076F34 *this, ListNode *node) {
    switch (this->unk21C) {
    case 0:
        func_8006F044(this->unk228, node);
        break;
    case 1:
        func_8006F024(this->unk228, node);
        break;
    }
}

/* Adds entity, if there is one, to group 0x4C of func_8006ED20's object at 0x228.
 * It returns nothing: func_800B94AC puts a constant in $v0 right after calling it,
 * which a call with a value would keep it from. */
void func_8007AD4C(Unk80076F34 *this, Entity *entity) {
    func_8006F0B4(this->unk228, entity);
}

void func_8007ADCC(Unk80076F34 *this, MeshPart *part, u32 handle);

/* func_8007ADCC with the part of chara's model at index and chara's handle. */
void func_8007AD70(Unk80076F34 *this, Unk80076F34 *chara, s32 index) {
    MeshPart *part = chara->model->parts[index];

    func_8007ADCC(this, part, entityGetHandle(&chara->entity));
}

/* Unless the model is already attached, attaches its transform to part's (to none when part is NULL) and
 * keeps handle and its entity at 0x244 and 0x240. */
void func_8007ADCC(Unk80076F34 *this, MeshPart *part, u32 handle) {
    if (this->unk244 == 0) {
        transformAttach(&this->model->group.object.transform, part != NULL ? &part->mesh.object.transform : NULL);
        this->unk244 = handle;
        this->unk240 = getEntityByHandle(handle);
    }
}

void func_80077F80(Unk80076F34 *this);

/* Undoes func_8007ADCC: detaches the model and takes its local translation (x, y, in 1/256ths, z 0) as the
 * position, then calls func_80077F80. */
void func_8007AE34(Unk80076F34 *this) {
    Model *model;
    s32 y;

    if (this->unk244 != 0) {
        this->unk244 = 0;
        this->unk240 = NULL;
        transformDetach(&this->model->group.object.transform);
        model = this->model;
        this->position.vx = model->group.object.transform.local.m.t[0] << 8;
        /* The match depends on y: the game reads it before it clears vz. */
        y = model->group.object.transform.local.m.t[1];
        this->position.vz = 0;
        this->position.vy = y << 8;
        func_80077F80(this);
    }
}

/* Whether the first word of the object at 0x22C is 11 or 12. */
s32 func_8007AEA8(Unk80076F34 *this) {
    s32 value = func_800792A0(this)->unk0;

    return value >= 11 && value <= 12;
}

/* Whether the word the pointer at 0x230 points to is 1 (as func_8007F964; the header types it s32). */
s32 func_8007AED0(Unk80076F34 *this) {
    return *(s32 *)this->unk230 == 1;
}

/* The Unk80076F34 that func_80073218 gives for the object at 0x228. */
Unk80076F34 *func_8007AEEC(Unk80076F34 *this) {
    return func_80073218(this->unk228, this);
}

/* When arg is 0, starts the model's clip 0x12 (func_800796F0). */
void func_8007AF14(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0x12, 2, 0);
    }
}

/* A virtual function of Unk80076F34 (entry 0x48 of D_8006530C): with arg 0, func_800796F0,
 * func_80078804 and func_8007A6FC; then func_8007835C with minus the table's unk8. */
void func_8007AF40(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 1, 4, 0);
        func_80078804(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 2 : 3);
    }
    func_8007835C(this, -func_80077698(this)->unk8);
}

/* A virtual function of Unk80076F34 (entry 0x50 of D_8006530C): with arg 0, func_800796F0,
 * func_8007881C and func_8007A6FC; then func_8007835C with the table's unk8. */
void func_8007AFBC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 1, 4, 0);
        func_8007881C(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 2 : 3);
    }
    func_8007835C(this, func_80077698(this)->unk8);
}

/* The flag func_8007B034 and func_8007B0E0 set: 0x80, 0x100 in the USA release. */
#if VERSION_JP
#define FUNC_8007B034_FLAG 0x80
#else
#define FUNC_8007B034_FLAG 0x100
#endif

/* Which side of the character func_8007A3EC finds blocked (-1 left, 1 right, 0 none); the JP release
 * does not test it, so there it is always 0. */
static inline s32 func_8007B034_blockedSide(Unk80076F34 *this) {
#if VERSION_JP
    return 0;
#else
    Vec3 position;

    func_800775D8(this, &position);
    return func_8007A3EC(&position, -0x33, func_80077698(this)->unk2);
#endif
}

/* A virtual function of Unk80076F34 (entry 0x58 of D_8006530C): with arg 0, func_800796F0, takes
 * the table's unkC (scaled by unk3C4, in 16.16) from unk3B4.vx unless the left side is blocked, then
 * func_80078804 and func_8007A6FC; always sets FUNC_8007B034_FLAG. */
void func_8007B034(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        s32 delta;

        func_800796F0(this, 2, 1, 0);
        if (func_8007B034_blockedSide(this) >= 0) {
            delta = ((s64)func_80077698(this)->unkC * this->unk3C4) >> 16;
            this->unk3B4.vx -= delta;
        }
        func_80078804(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 3 : 4);
    }
    this->flags |= FUNC_8007B034_FLAG;
}

/* A virtual function of Unk80076F34 (entry 0x60 of D_8006530C): with arg 0, func_800796F0, adds
 * the table's unkC (scaled by unk3C4, in 16.16) to unk3B4.vx unless the right side is blocked, then
 * func_8007881C and func_8007A6FC; always sets FUNC_8007B034_FLAG. */
void func_8007B0E0(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        s32 delta;

        func_800796F0(this, 2, 1, 0);
        if (func_8007B034_blockedSide(this) <= 0) {
            /* The match depends on delta: as one expression, the sum takes unk3B4's register. */
            delta = ((s64)func_80077698(this)->unkC * this->unk3C4) >> 16;
            this->unk3B4.vx += delta;
        }
        func_8007881C(this, 0);
        func_8007A6FC(this, -1, 0x60, func_80077794(this) ? 3 : 4);
    }
    this->flags |= FUNC_8007B034_FLAG;
}

/* Starts clip 3 with unk3B4.vy set to minus the table's unk10 (scaled by unk3C4, in 16.16) and unk3A4.vy
 * cleared; when func_800778BC, func_8007A6FC on both sides; then keeps unk3A4.vx within 1.5 times the
 * table's unk8 either way. */
void func_8007B18C(Unk80076F34 *this) {
    s32 value = func_80077698(this)->unk10;
    s32 scale = this->unk3C4;

    this->unk3B4.vy = ((s64)-value * scale) >> 16;
    this->unk3A4.vy = 0;
    func_800796F0(this, 3, 1, 0);
    if (func_800778BC(this)) {
        s32 count = func_80077794(this) ? 2 : 3;

        func_8007A6FC(this, 1, 0x60, count);
        func_8007A6FC(this, -1, 0x60, count);
    }
    value = func_80077698(this)->unk8;
    value += func_80077698(this)->unk8 / 2;
    if (value < this->unk3A4.vx) {
        this->unk3A4.vx = value;
    } else if (this->unk3A4.vx < -value) {
        this->unk3A4.vx = -value;
    }
}

/* Starts the model's clip 0x13 (func_800796F0). */
void func_8007B298(Unk80076F34 *this) {
    func_800796F0(this, 0x13, 2, 0);
}

/* When arg is 0, starts the model's clip 0xC (func_800796F0). */
void func_8007B2C0(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0xC, 3, 0);
    }
}

/* When arg is 0, starts the model's clip 0x15 (func_800796F0). */
void func_8007B2EC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0x15, 3, 0);
    }
}

INCLUDE_ASM("game/nonmatchings/fighter", func_8007B318);

/* When arg is 0, starts the model's clip 0xD (func_800796F0). */
void func_8007B41C(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_800796F0(this, 0xD, 1, 0);
    }
}

/* Starts the model's clip 0xE (func_800796F0). */
void func_8007B448(Unk80076F34 *this) {
    func_800796F0(this, 0xE, 1, 0);
}

INCLUDE_RODATA("game/nonmatchings/fighter", D_80065278);

INCLUDE_ASM("game/nonmatchings/fighter", func_8007B470);

/* A virtual function of Unk80076F34 (entry 0xA0 of D_8006530C): func_80079AE4, func_80079830
 * with 5 when arg is 0, then func_8007B470 with the table's entries[0]. */
void func_8007B6A0(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 5, 0, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[0], 1);
}

/* A virtual function of Unk80076F34 (entry 0xA8 of D_8006530C): func_80079AE4, func_80079830
 * with 6 and func_8007835C when arg is 0, then func_8007B470 with the table's entries[1]. */
void func_8007B70C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 6, 0, 0, -1);
        func_8007835C(this, 0);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[1], 1);
}

/* A virtual function of Unk80076F34 (entry 0xB0 of D_8006530C): func_80079AE4, func_80079830 with 7
 * and halving the x at 0x3A4 when arg is 0, then func_8007B470 with the table's entries[2]. */
void func_8007B784(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 7, 0, 0, -1);
        this->unk3A4.vx -= this->unk3A4.vx / 2;
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[2], 2);
}

/* A virtual function of Unk80076F34 (entry 0xB8 of D_8006530C): func_80079AE4, func_80079830
 * with 31 and func_8007835C when arg is 0, then func_8007B470 with the table's entries[4]. */
void func_8007B80C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 31, 0, 0, -1);
        func_8007835C(this, 0);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[4], 1);
}

/* A virtual function of Unk80076F34 (entry 0xC0 of D_8006530C): func_80079AE4; when arg is 0,
 * func_800796F0 with -1, func_80079830 with 8 and clearing the x at 0x3A4 and 0x3B4; then
 * func_8007B470 with the table's entries[3]. */
void func_8007B884(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_800796F0(this, -1, 0, 0);
        func_80079830(this, 8, 2, 0, -1);
        this->unk3A4.vx = 0;
        this->unk3B4.vx = 0;
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[3], 2);
}

/* A virtual function of Unk80076F34 (entry 0xC8 of D_8006530C): func_80079AE4, func_80079830
 * with 20 when arg is 0, then func_8007B470 with the table's entries[5]. */
void func_8007B90C(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 20, 0, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[5], 1);
}

/* func_80079AE4, then, when arg is 0, starts the model's clip 0xB (func_80079830). */
void func_8007B978(Unk80076F34 *this, s32 arg) {
    func_80079AE4(this);
    if (arg == 0) {
        func_80079830(this, 0xB, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x16 (func_80079830). */
void func_8007B9C8(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x16, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x18 (func_80079830). */
void func_8007B9FC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x18, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x1A (func_80079830). */
void func_8007BA30(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x1A, 0, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x1B (func_80079830). */
void func_8007BA64(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x1B, 2, 0, -1);
    }
}

/* A virtual function of Unk80076F34 (entry 0xF8 of D_8006530C): func_80079830 with 9 when arg is 0,
 * then func_8007B470 with the table's entries[8]. */
void func_8007BA98(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 9, 2, 0, -1);
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[8], 1);
}

/* A virtual function of Unk80076F34 (entry 0x100 of D_8006530C): func_80079830 with 10 and clearing the
 * y at 0x3A8 when arg is 0, then func_8007B470 with the table's entries[6]. */
void func_8007BAFC(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 10, 2, 0, -1);
        this->unk3A4.vy = 0;
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[6], 1);
}

/* A virtual function of Unk80076F34 (entry 0x108 of D_8006530C): when arg is 0, func_80079830 with the
 * table's clip at 0x18 and clearing unk3B4.vx, unk3A4 and unk3B0; then func_8007B470 with the table's
 * entries[7]. */
void func_8007BB64(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, func_80077698(this)->unk18, 2, 0, -1);
        this->unk3B4.vx = 0;
        this->unk3A4.vx = 0;
        this->unk3A4.vy = 0;
        this->unk3A4.vz = 0;
        this->unk3B0 = 0;
    }
    func_8007B470(this, arg, &func_80077698(this)->entries[7], 1);
}

/* When arg is 0, starts the model's clip 0x17 (func_80079830). */
void func_8007BBE8(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x17, 2, 0, -1);
    }
}

/* When arg is 0, starts the model's clip 0x19 (func_80079830). */
void func_8007BC1C(Unk80076F34 *this, s32 arg) {
    if (arg == 0) {
        func_80079830(this, 0x19, 2, 0, -1);
    }
}

/* The 0x20 bytes that func_800708F4 copies from the object at 0x20 of func_800756E4's result and passes to
 * Unk80076F34's method at 0x28 (func_8007BC50): a position at 0x14. */
typedef struct Unk800708F4 {
    /* 0x00 */ u8 unk0[0x14];
    /* 0x14 */ Vec3 position;
} Unk800708F4;
s32 func_80078E08(Unk80076F34 *this, u32 unk);

/* g++'s new of the entity class of vtable D_8006D4D4 (0x58 bytes: func_800C2FBC with frame 8, no velocity,
 * size 0x60, color 0xA0A0A0) at hit's position: NULL when the allocation fails. */
static inline Entity *func_8007BC50_newFlash(Unk800708F4 *hit) {
    Entity *child = allocEntityMemory(0x58);

    if (child != NULL) {
        Unk800C2F14 *flash = entityOperatorNewAt(0x58, child);

        func_800C2FBC(flash, 8, (VECTOR *)&hit->position, &D_80010814, 0x60, 0xA0A0A0, 0);
        flash->entity.vtable = &D_8006D4D4;
    }
    return child;
}

/* g++'s new of func_800C32E0's entity (0x48 bytes) at hit's position with velocity: NULL when the allocation
 * fails. */
static inline Entity *func_8007BC50_newC32E0(Unk800708F4 *hit, Vec3 *velocity) {
    Entity *child = allocEntityMemory(0x48);

    if (child != NULL) {
        func_800C32E0(entityOperatorNewAt(0x48, child), &hit->position, (VECTOR *)velocity, 0xC0, 0x808080);
    }
    return child;
}

/* A virtual function of Unk80076F34 (entry 0x28 of D_8006530C): when a throw is set at 0x44C, throws other
 * with it (func_80078EF8, the velocity at 0x454 and the word at 0x450), spawns at hit's position the entity
 * of vtable D_8006D4D4 when func_80078E08 holds, else func_800C32E0's, hands it to func_8007AD4C and clears
 * 0x44C to 0x460. */
void func_8007BC50(Unk80076F34 *this, Unk80076F34 *other, Unk800708F4 *hit) {
    if (this->unk44C != 0) {
        Vec3 *velocity = (Vec3 *)&this->unk454;

        func_80078EF8(other, this->unk44C, velocity, this->unk450);
        /* The match depends on the two news as inlines (a ternary of their results). */
        func_8007AD4C(this, func_80078E08(other, this->unk44C) ? func_8007BC50_newFlash(hit)
                                                              : func_8007BC50_newC32E0(hit, velocity));
        this->unk454 = 0;
        this->unk458 = 0;
        this->unk45C = 0;
        this->unk460 = 0;
        this->unk44C = 0;
        this->unk450 = 0;
    }
}

/* Constructs the class of virtual table D_800652EC: the three words, then the vector at 0x10.
 * The match depends on reading the vector's three words before storing any, as vecCopy does. */
Unk8007BD8C *func_8007BD8C(Unk8007BD8C *this, struct Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC) {
    s32 x, y, z;

    this->vtable = D_800652EC;
    this->chara = chara;
    this->unk8 = unk8;
    this->unkC = unkC;
    x = unk10->vx;
    y = unk10->vy;
    z = unk10->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
    return this;
}

/* Sets the words at 0x8 and 0xC. */
void func_8007BDC8(Unk8007BD8C *this, s32 unk8, s32 unkC) {
    this->unk8 = unk8;
    this->unkC = unkC;
}

/* Sets the vector at 0x10, reading all three components before writing any. */
void func_8007BDD4(Unk8007BD8C *this, VECTOR *vec) {
    s32 x, y, z;

    x = vec->vx;
    y = vec->vy;
    z = vec->vz;
    this->unk10.vx = x;
    this->unk10.vy = y;
    this->unk10.vz = z;
}

/* Unk8007BD8C's first method: adds its count (saturating at 0x3FFFF), its 0xC and its vector into sum,
 * whose flags it replaces with its own. */
void func_8007BDF0(Unk8007BD8C *this, Unk80075EE4 *sum) {
    u32 flags = this->unk8 & ~0x3FFFF;
    /* The match depends on starting count with this object's part, then adding
     * sum's: count then lives in $a3 */
    s32 count = this->unk8 & 0x3FFFF;
    s32 x, y, z;

    count += sum->unk18 & 0x3FFFF;
    if (count > 0x3FFFF) {
        count = 0x3FFFF;
    }
    sum->unk18 = flags | count;
    sum->unk1C += this->unkC;
    /* The match depends on vecAddInPlace's shape: the three words read before any is written */
    x = sum->unk20.vx;
    y = sum->unk20.vy;
    z = sum->unk20.vz;
    x += this->unk10.vx;
    y += this->unk10.vy;
    z += this->unk10.vz;
    sum->unk20.vx = x;
    sum->unk20.vy = y;
    sum->unk20.vz = z;
}

/* Unk8007BF20's first method: func_8007BDF0 with sum, then sets bit 0 of the flags when sum's entity is of
 * kind 0x10000 and bit 1 when it is of kind 0x30000 (func_8002BD68). */
void func_8007BE78(Unk8007BF20 *this, Unk80075EE4 *sum) {
    Entity *target;

    func_8007BDF0(&this->base, sum);
    target = sum->unk14;
    if (target != NULL) {
        if (func_8002BD68(target) == 0x10000) {
            this->flags |= 1;
        }
        if (func_8002BD68(target) == 0x30000) {
            this->flags |= 2;
        }
    }
}

/* Returns bit 0 of the flags at 0x20. */
s32 func_8007BF00(Unk8007BF20 *this) {
    return this->flags & 1;
}

/* Returns bit 1 of the flags at 0x20; no caller found. */
s32 func_8007BF0C(Unk8007BF20 *this) {
    return (this->flags >> 1) & 1;
}

/* Constructs an Unk8007BF20 on func_8007BD8C's (the same arguments) and clears bits 0 and 1 of its
 * flags. */
Unk8007BF20 *func_8007BF20(Unk8007BF20 *this, Unk80076F34 *chara, s32 unk8, Vec3 *unk10, s32 unkC) {
    func_8007BD8C(&this->base, chara, unk8, unk10, unkC);
    this->base.vtable = D_800652CC;
    this->flags &= ~1;
    this->flags &= ~2;
    return this;
}

/* Two counters (func_8007BF74 clears them, func_8007BF84 sets them, func_8007BF90 scales them). */
typedef struct Unk8007BF74 {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
} Unk8007BF74;

/* Constructs an Unk8007BF74 with both counters at 0. */
Unk8007BF74 *func_8007BF74(Unk8007BF74 *this) {
    this->unk0 = 0;
    this->unk2 = 0;
    return this;
}

/* Sets both counters. */
void func_8007BF84(Unk8007BF74 *this, u16 unk0, u16 unk2) {
    this->unk0 = unk0;
    this->unk2 = unk2;
}

/* Multiplies the first counter by scale and grows the second by (scale - 1) percent of itself. */
Unk8007BF74 *func_8007BF90(Unk8007BF74 *this, s32 scale) {
    this->unk0 *= scale;
    this->unk2 += this->unk2 * (scale - 1) / 100;
    return this;
}

void func_8007C028(Unk8007BF74 *this, Unk80076F34 *chara);

/* Passes the first counter to chara (func_800773F4), the second with func_8007C028, and clears the first. */
void func_8007BFE0(Unk8007BF74 *this, Unk80076F34 *chara) {
    func_800773F4(chara, this->unk0);
    func_8007C028(this, chara);
    this->unk0 = 0;
}

/* Passes the second counter to chara with func_80077448 when func_80077794 or func_800777BC holds for it, and
 * clears the counter. */
void func_8007C028(Unk8007BF74 *this, Unk80076F34 *chara) {
    if (func_80077794(chara) || func_800777BC(chara)) {
        func_80077448(chara, this->unk2);
    }
    this->unk2 = 0;
}

INCLUDE_RODATA("game/nonmatchings/fighter", D_800652AC);

/* Constructs an Unk8007C084 with its word (virtual table D_800652AC). */
Unk8007C084 *func_8007C084(Unk8007C084 *object, s32 unk0) {
    object->vtable = D_800652AC;
    object->unk0 = unk0;
    return object;
}

/* Unk8007C084's destructor. */
void func_8007C09C(Unk8007C084 *this, s32 flags) {
    this->vtable = D_800652AC;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Calls the object's virtual method at 8 (pure in Unk8007C084) with the word at 8 of what unk0 points to (game.h
 * types unk0 s32), and returns its result. */
s32 func_8007C0D0(Unk8007C084 *this) {
    VtableEntryS32 *entry = (VtableEntryS32 *)&this->vtable[1];

    return entry->func((u8 *)this + entry->delta, ((s32 *)this->unk0)[2]);
}

void func_8007C104(void) {
}

void func_8007C10C(void) {
}

/* The destructor of the class of virtual table D_800652EC: Unk80076308's, inlined. */
void func_8007C114(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* The destructor of the class of virtual table D_800652CC: Unk80076308's, inlined. */
void func_8007C148(Unk80076308 *this, s32 flags) {
    this->vtable = D_80065164;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

INCLUDE_RODATA("game/nonmatchings/fighter", D_800652CC);
INCLUDE_RODATA("game/nonmatchings/fighter", D_800652EC);
INCLUDE_RODATA("game/nonmatchings/fighter", D_8006530C);
