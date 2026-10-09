#ifndef DTBE_PAD_BUTTON_MAP_H
#define DTBE_PAD_BUTTON_MAP_H

/* The button maps: what each button of a pad stands for. */

#include "common.h"

EXTERN_C_BEGIN

/* What the buttons of a pad stand for: bit i of the pad gives map[i]. */
typedef struct {
    /* 0x00 */ u16 map[16];
} ButtonMap;

ButtonMap *buttonMapInit(ButtonMap *buttonMap);
void buttonMapReset(ButtonMap *buttonMap);
void buttonMapSet(ButtonMap *buttonMap, u16 button, u16 value);
u16 buttonMapApply(ButtonMap *buttonMap, u16 buttons);

EXTERN_C_END

#endif /* DTBE_PAD_BUTTON_MAP_H */
