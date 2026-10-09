#include "common.h"
#include "engine/pad/button_map.h"
#include "gte.h"

/* Builds a map that leaves the buttons as they are. */
ButtonMap *buttonMapInit(ButtonMap *this) {
    buttonMapReset(this);
    return this;
}

/* Makes every button stand for itself. */
void buttonMapReset(ButtonMap *this) {
    s32 i;

    for (i = 0; i < 16; i++) {
        this->map[i] = 1 << i;
    }
}

/* Adds the buttons of value to the map entry of a button (a single bit), or
 * clears the entry when value is 0. */
void buttonMapSet(ButtonMap *this, u16 button, u16 value) {
    s32 zeros;
    s32 bit;

    gte_leadingZeros(button, zeros);
    bit = 31 - zeros;
    if (value == 0) {
        this->map[bit] = 0;
    } else {
        this->map[bit] |= value;
    }
}

/* The buttons the pressed buttons stand for. */
u16 buttonMapApply(ButtonMap *this, u16 buttons) {
    u16 *map = this->map;
    s32 mapped = 0;

    while (buttons != 0) {
        if (buttons & 1) {
            mapped |= *map;
        }
        buttons >>= 1;
        map++;
    }
    return mapped;
}
