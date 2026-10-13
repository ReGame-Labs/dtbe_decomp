/* The boot logo (the task "Boot Logo", title/bootlogo.tim). */

#include "common.h"
#include <libetc.h>
#include "engine/gfx/ordering_table.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/pad/pad.h"
#include "title/title_classes.h"

/* A key of the boot logo's fade in: the frames it takes to reach it from the previous key, and the two
 * values it brings the logo to (the first is the brightness func_80067018 draws with). */
typedef struct BootLogoKey {
    /* 0x0 */ s16 frames;
    /* 0x2 */ u8 level;
    /* 0x3 */ u8 unk3;
} BootLogoKey;

/* The boot logo's task (virtual table D_80064404). */
typedef struct BootLogoTask {
    /* 0x00 */ Task task;
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 state; /* 0: fading through the keys, 1: fading out, then waiting to end */
    /* 0x28 */ s32 key;   /* the key being faded from */
    /* 0x2C */ s32 frame; /* in the state, or the key */
    /* 0x30 */ union {
        struct {
            s16 level;
            s16 unk32;
        } values; /* where the fade out starts from, then goes down to 0 */
        s32 both; /* both values at once: 0 once both reached 0 */
    } fade;
} BootLogoTask;

/* the buttons that skip the boot logo: Start, circle and cross in Japan, Start and any of the four in the
 * US */
#if VERSION_JP
#define BOOT_LOGO_SKIP (PADstart | PADRright | PADRdown)
#else
#define BOOT_LOGO_SKIP (PADstart | PADRup | PADRright | PADRdown | PADRleft)
#endif

/* the keys of the fade in, the last one at index 3 */
extern BootLogoKey D_80064444[];

void func_80067018(u_long *ot, s32 arg1, u8 level);

INCLUDE_RODATA("title/nonmatchings/boot_logo", D_800643E0);

INCLUDE_ASM("title/nonmatchings/boot_logo", bootLogoTaskInit);

/* The boot logo's frame: fades the logo through the keys (a skip button starts the fade out from where it
 * is), fades it out by 8 a frame, then ends the task 4 frames later. Draws the logo at its level until then.
 * The match depends on state 2 being a case of its own (three cases make GCC test 1 first, then < 2) and on
 * one variable for each value of the fade out. */
void func_80066E1C(BootLogoTask *this) {
    PadManager *pads = PAD_MANAGER_INSTANCE;
    u_long *ot;
    u8 level;
    u8 unk3;

    switch (this->state) {
    case 0: {
        BootLogoKey *from = &D_80064444[this->key];
        BootLogoKey *to = &D_80064444[this->key + 1];

        level = from->level + (to->level - from->level) * this->frame / to->frames;
        unk3 = from->unk3 + (to->unk3 - from->unk3) * this->frame / to->frames;
        if (++this->frame >= to->frames) {
            this->key++;
            this->frame = 0;
            if (this->key >= 3) {
                this->state = 2;
                break;
            }
        }
        if (padManagerGetPressed(pads, -1) & BOOT_LOGO_SKIP) {
            this->fade.values.level = level;
            this->fade.values.unk32 = unk3;
            this->state++;
        }
        break;
    }
    case 1: {
        s32 levelLeft;
        s32 unk32Left;

        level = this->fade.values.level;
        levelLeft = this->fade.values.level - 8;
        if (levelLeft < 0) {
            levelLeft = 0;
        }
        this->fade.values.level = levelLeft;
        unk32Left = this->fade.values.unk32 - 8;
        if (unk32Left < 0) {
            unk32Left = 0;
        }
        this->fade.values.unk32 = unk32Left;
        if (this->fade.both == 0) {
            this->frame = 0;
            this->state = 2;
        }
        break;
    }
    case 2:
    default:
        if (++this->frame >= 4) {
            taskKill(&this->task);
        }
        return;
    }
    ot = FRAME_OT.firstDrawn;
    func_80067018(ot, 0, level);
    AddPrim(ot, allocDrTpage(0x2A));
}

INCLUDE_ASM("title/nonmatchings/boot_logo", func_80067018);

/* The destructor of bootLogoTaskInit's task (virtual table D_80064404): Task's. */
void func_8006711C(Task *this, s32 flags) {
    taskDestroy(this, flags);
}

/* Update: keeps arg at 0x20 and runs the task's own method at 0x20. */
void func_8006713C(Unk8006713C *this, s32 arg) {
    this->unk20 = arg;
    this->vtable->unk20.func((u8 *)this + this->vtable->unk20.delta, arg);
}

INCLUDE_RODATA("title/nonmatchings/boot_logo", D_80064404);
INCLUDE_RODATA("title/nonmatchings/boot_logo", D_80064434);
INCLUDE_RODATA("title/nonmatchings/boot_logo", D_80064444);
