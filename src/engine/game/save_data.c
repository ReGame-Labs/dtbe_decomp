#include "common.h"
#include "engine/game/save_data.h"
#include "engine/cd/xa_player.h"
#include "engine/gfx/display.h"
#include "engine/lib/md5.h"
#include "engine/pad/button_map.h"
#include "psyq.h"

/* Whether the save data is intact: its MD5 is the one stored in it. */
s32 saveDataIsValid(SaveData *this) {
    u8 digest[MD5_DIGEST_SIZE];

    saveDataGetMd5(this, digest);
    return func_8003DF40(digest, this->md5, MD5_DIGEST_SIZE) == 0;
}

/* Stores the MD5 of the save data in it. */
void saveDataStoreMd5(SaveData *this) {
    saveDataGetMd5(this, this->md5);
}

/* The MD5 of the save data, but for the MD5 itself. */
void saveDataGetMd5(SaveData *this, u8 *digest) {
    Md5 md5;

    md5Init(&md5);
    MD5Update(&md5, (u8 *)this, offsetof(SaveData, md5));
    MD5Final(&md5, digest);
}

/* Applies the saved options: where the screen shows, and the sound output.
 * It reads CURRENT_DISPLAY at its absolute address, as the code of this file does:
 * the display code (gfx/display.c), which defines it, reaches it through $gp. */
void saveDataApplyOptions(SaveData *this) {
    displaySetTvPosition(CURRENT_DISPLAY, this->screenX, this->screenY);
    if (this->mono) {
        setXaMono();
        func_80052A90();
    } else {
        setXaStereo();
        func_80052AA0();
    }
}

/* Gives every player the default buttons. */
void saveDataResetButtons(SaveData *this) {
    s32 i;

    for (i = 0; i < BUTTON_SLOT_COUNT; i++) {
        u16 index = BUTTON_SLOTS[i].defaultIndex;

        this->buttons[0][i] = index;
        this->buttons[1][i] = index;
    }
    this->unkC = 0;
}

/* Sets the maps of both players from the slot indexes they gave the slots:
 * the pad button of the index given to a slot stands for the slot's button. */
void saveDataApplyButtons(SaveData *this, ButtonMap *map0, ButtonMap *map1) {
    s32 i;
    s32 slot;

    for (i = 0; i < BUTTON_SLOT_COUNT; i++) {
        u16 button = BUTTON_SLOTS[i].button;

        buttonMapSet(map0, button, 0);
        buttonMapSet(map1, button, 0);
    }
    for (slot = 0; slot < BUTTON_SLOT_COUNT; slot++) {
        u16 given0 = this->buttons[0][slot];
        u16 given1 = this->buttons[1][slot];
        u16 button = BUTTON_SLOTS[slot].button;

        buttonMapSet(map0, BUTTON_SLOTS[given0].padButton, button);
        buttonMapSet(map1, BUTTON_SLOTS[given1].padButton, button);
    }
}

/* The slot index a player gave a slot. */
u16 saveDataGetButtonSlot(SaveData *this, s32 player, s32 slot) {
    return this->buttons[player][slot];
}

/* Gives a player's slot the slot index; the slot that had that index gets
 * the slot's old one, so that no two slots share an index. */
void saveDataSetButtonSlot(SaveData *this, s32 player, s32 slot, s32 index) {
    u16 old = this->buttons[player][slot];
    s32 i;

    this->buttons[player][slot] = index;
    for (i = 0; i < BUTTON_SLOT_COUNT; i++) {
        if (i == slot) {
            continue;
        }
        if (this->buttons[player][i] == index) {
            this->buttons[player][i] = old;
            break;
        }
    }
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/save_data", BUTTON_SLOTS);
