#ifndef DTBE_GAME_SAVE_DATA_H
#define DTBE_GAME_SAVE_DATA_H

/* The save data: the options, the button slots and the progress, checked by its MD5. */

#include "common.h"
#include "engine/pad/button_map.h"

EXTERN_C_BEGIN

/* one of the buttons the options let the player assign */
typedef struct ButtonSlot {
    /* 0x0 */ u16 button;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u16 defaultIndex; /* the slot whose unk2 it gets by default */
} ButtonSlot;

#define BUTTON_SLOT_COUNT 6

/* The options and progress kept on the memory card. Its MD5 tells whether it
 * changed since it was saved. */
typedef struct {
    /* 0x000 */ s32 unk0; /* picks the sound output */
    /* 0x004 */ s32 unk4;
    /* 0x008 */ s32 unk8;
    /* 0x00C */ s32 unkC;
    /* 0x010 */ s32 unk10;
    /* 0x014 */ s16 screenX;
    /* 0x016 */ s16 screenY;
    /* 0x018 */ s32 unk18;
    /* 0x01C */ u32 unlocked; /* a bit for each character that can be picked */
    /* 0x020 */ u32 cleared;  /* a bit for each character a run was won with */
    /* 0x024 */ u16 buttons[2][BUTTON_SLOT_COUNT]; /* a slot index for each slot, by player */
    /* 0x03C */ u8 unk3C[0x23C - 0x3C];
    /* 0x23C */ u8 md5[16];
} SaveData;

extern struct ButtonSlot BUTTON_SLOTS[]; /* the slots of the button options */

void saveDataApplyOptions(SaveData *saveData);
void saveDataResetButtons(SaveData *saveData);
void saveDataApplyButtons(SaveData *saveData, ButtonMap *map0, ButtonMap *map1);
u16 saveDataGetButtonSlot(SaveData *saveData, s32 player, s32 slot);
void saveDataSetButtonSlot(SaveData *saveData, s32 player, s32 slot, s32 index);

s32 saveDataIsValid(SaveData *saveData);
void saveDataStoreMd5(SaveData *saveData);
void saveDataGetMd5(SaveData *saveData, u8 *digest);

EXTERN_C_END

#endif /* DTBE_GAME_SAVE_DATA_H */
