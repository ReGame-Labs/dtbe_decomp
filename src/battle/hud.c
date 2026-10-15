/* The battle's messages (READY? FIGHT!, KO, YOU WIN!, TIME UP, GAME OVER and the rest), the text and font they draw
 * with, the continue screen and the score tally after a won round. */

#include "common.h"
#include "strings.h"
#include "battle/game_classes.h"
#include "engine/game/game_state.h"
#include "engine/gfx/box.h"
#include "engine/gfx/prim/alloc_sprt.h"
#include "engine/math/sin_cos.h"
#include "engine/pad/pad.h"
#include "engine/sound/sound_control.h"
#include "engine/text/console.h"

extern EntityVtable SCORE_TALLY_VTABLE;
/* adds the tally's three bonuses into its score and clears them */
struct ScoreTally;
void ScoreTally_addAll(struct ScoreTally *this);

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_init);

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_tick);

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_draw);

/* A battle message (BattleMessage_init, 0x59C bytes): its kind's layout and text from BATTLE_MESSAGE_LAYOUTS, and
 * the kind's tick and draw members from BATTLE_MESSAGE_TICKS and BATTLE_MESSAGE_DRAWS, which the entity's own tick
 * and draw run. */
typedef struct BattleMessage {
    /* 0x000 */ Entity entity;
    /* 0x018 */ s16 charStep;  /* from BATTLE_MESSAGE_LAYOUTS: the step of a character (home's w and h) */
    /* 0x01A */ s16 textWidth; /* from BATTLE_MESSAGE_LAYOUTS: the width of the text */
    /* 0x01C */ u8 *text;      /* hudTextSetText copies it into hudText */
    /* 0x020 */ RECT home;     /* x centred, y 0x50; drawGlyphAtCursor's metrics: x of a new line, w (twice
                                * a space), h (a line); TIME UP slides the cursor to it */
    /* 0x028 */ RECT cursor;   /* where BattleMessage_drawTyped starts drawing the text */
    /* 0x030 */ s32 shownChars; /* BattleMessage_drawTyped: how many characters it draws */
    /* 0x034 */ s32 palette;   /* clut row 0x1E9 + palette; EVOLUTION_MESSAGE_PALETTES[character] for EVOLUTION */
    /* 0x038 */ s32 state;
    /* 0x03C */ s32 frame;     /* counted every tick, restarted by the states */
    /* 0x040 */ HudText hudText;
    /* 0x58C */ VtableEntry tick; /* the kind's entry of BATTLE_MESSAGE_TICKS */
    /* 0x594 */ VtableEntry draw; /* the kind's entry of BATTLE_MESSAGE_DRAWS */
} BattleMessage;

/* The NEW CHALLENGER and DOUBLE KO tick (BATTLE_MESSAGE_TICKS): types the text out a character a frame, and deletes
 * the message after 91 frames. */
void BattleMessage_tickTypeOut(BattleMessage *this) {
    this->shownChars++;
    this->frame++;
    if (this->frame >= 91) {
        entityDelete(&this->entity);
    }
}

void BattleMessage_drawTyped(BattleMessage *this, u_long *ot);
void func_800CF2A4(u_long *ot, s32 brightness);

void drawGlyphAtCursor(u_long *ot, RECT *cursor, RECT *metrics, u8 c, s32 palette);
void func_800D008C(u_long *ot);

/* A draw of BATTLE_MESSAGE_DRAWS: draws the first shownChars characters of the text (drawGlyphAtCursor) from the
 * cursor, then the font's tpage (func_800D008C). */
void BattleMessage_drawTyped(BattleMessage *this, u_long *ot) {
    s32 count = this->shownChars;
    u8 *c = this->text;
    RECT cursor = this->cursor;

    while (count > 0 && *c != 0) {
        drawGlyphAtCursor(ot, &cursor, &this->home, *c, this->palette);
        c++;
        count--;
    }
    func_800D008C(ot);
}

void hudTextDraw(HudText *this, u_long *ot, s32 x, s32 y);

/* A draw of BATTLE_MESSAGE_DRAWS: draws the hud text (hudTextDraw) centred on the screen's width, at y 0x50. */
void BattleMessage_drawCentered(BattleMessage *this, u_long *ot) {
    s32 width = (this->hudText.length - 1) * this->hudText.step;

    hudTextDraw(&this->hudText, ot, (320 - width) / 2, 0x50);
}

void hudTextSetText(HudText *this, u8 *text);
void hudTextSetOffsets(HudText *this, s32 dx, s32 dy);
void hudTextSetOffsetRange(HudText *this, s32 dx, s32 dy, s32 start, s32 count);
void hudTextArrangeOnCircle(HudText *this, s32 radius, s32 angle, s32 step);
void hudTextSetPalettes(HudText *this, s32 palette);
void hudTextSetPaletteRange(HudText *this, s32 palette, s32 start, s32 count);
void hudTextSetSize(HudText *this, s32 size);
void hudTextSetSizeRange(HudText *this, s32 w, s32 h, s32 start, s32 count);
void hudTextSetScale(HudText *this, s32 scale);
void hudTextSetGray(HudText *this, s32 gray);
void hudTextSetOpacity(HudText *this, s32 opacity);
void hudTextSetOpacityRange(HudText *this, s32 opacity, s32 start, s32 count);

/* The READY? FIGHT! tick (BATTLE_MESSAGE_TICKS), the round's call: "READY?" turns in on a closing circle
 * (hudTextArrangeOnCircle, its characters a sixth of a turn apart) while its size shrinks, waits 9 frames, turns
 * out on an opening circle while it grows; then "FIGHT!" (US: "RUMBLE!") shrinks from 0x80 while it fades in, its
 * gray falls from 0x100 and it fades out, and the message is deleted. */
void BattleMessage_tickReadyFight(BattleMessage *this) {
    HudText *text = &this->hudText;

    switch (this->state) {
    case 0:
        hudTextSetText(text, "READY?");
        hudTextSetSize(text, 0x80);
        hudTextSetOffsets(text, 0, -0x60);
        this->state++;
        break;
    case 1:
        hudTextSetSize(text, 0x80 - this->frame * 6);
        hudTextArrangeOnCircle(text, 0xA0 - this->frame * 10, -this->frame << 8, 0x2AA);
        if (this->frame >= 16) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 2:
        if (this->frame >= 9) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 3:
        hudTextSetSize(text, (this->frame << 3) + 0x20);
        hudTextArrangeOnCircle(text, this->frame * 320 / 12, -this->frame << 8, 0x2AA);
        if (this->frame >= 12) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 4:
#if VERSION_JP
        hudTextSetText(text, "FIGHT!");
#else
        hudTextSetText(text, "RUMBLE!");
#endif
        hudTextSetPalettes(text, 1);
        hudTextSetSize(text, 0x80);
        hudTextSetOpacity(text, 0);
        this->state++;
        break;
    case 5:
        hudTextSetSize(text, 0x80 - this->frame * 12);
        hudTextSetOpacity(text, this->frame << 4);
        if (this->frame >= 8) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 6:
        hudTextSetGray(text, 0x100 - (this->frame << 3));
        if (this->frame >= 16) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 7:
        hudTextSetOpacity(text, 0x80 - (this->frame << 3));
        if (this->frame >= 16) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frame++;
}

/* The NEW CHALLENGER draw (BATTLE_MESSAGE_DRAWS): the typed text, then the shade bands (func_800CF2A4) in the next
 * slot of ot, darker as the frames go by. */
void BattleMessage_drawNewChallenger(BattleMessage *this, u_long *ot) {
    BattleMessage_drawTyped(this, ot);
    func_800CF2A4(ot + 1, 90 - this->frame);
}

#if VERSION_JP
INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_tickEvolution);
#else
/* The EVOLUTION tick (BATTLE_MESSAGE_TICKS): in state 0 types the text out on even frames up to its last
 * character, then in state 1 blinks that last character (the cursor '_'); deletes the message after 46 frames. The
 * length is the text's width over the step of a character (JP: 10, and that body is still asm: its registers
 * differ). */
void BattleMessage_tickEvolution(BattleMessage *this) {
    s32 length = this->textWidth / this->charStep;

    switch (this->state) {
    case 0:
        length--;
        /* The match depends on the store-flag test (xori + andi); !(frame & 1) gives andi + bnez. */
        if ((this->frame ^ 1) & 1) {
            if (++this->shownChars >= length) {
                this->state = 1;
            }
        }
        break;
    case 1:
        if (this->frame & 2) {
            length--;
        }
        this->shownChars = length;
        break;
    }
    if (++this->frame >= 46) {
        entityDelete(&this->entity);
    }
}
#endif

/* The EVOLUTION draw (BATTLE_MESSAGE_DRAWS): the typed text, then the shade bands (func_800CF2A4) in the next slot
 * of ot, their brightness rising and falling with half a turn of the sine over the message's 45 frames. */
void BattleMessage_drawEvolution(BattleMessage *this, u_long *ot) {
    BattleMessage_drawTyped(this, ot);
    func_800CF2A4(ot + 1, (COS_SIN_TABLE[((this->frame << 11) / 45) & 0xFFF].sin * 255) >> 12);
}

/* The KO tick (BATTLE_MESSAGE_TICKS): in state 0 copies the text into the hud text and moves its two letters apart;
 * in 1 brings them in by quarters while the gray goes up to 0xFF, in 2 dims it from 0xFF down to 0x80, waits 17
 * frames in 3, and in 4 darkens it while the two letters drift apart again, deleting the message after 17
 * frames. The match depends on this being a copy of the parameter (it then takes s1 and the text s2) and
 * on state 1's loop having a counter of its own (seed: Ashar2shahid's US func_800CC2C4). */
void BattleMessage_tickKo(BattleMessage *arg) {
    BattleMessage *this = arg;
    HudText *text = &this->hudText;
    s32 i;
    s32 way;

    switch (this->state) {
    case 0:
        hudTextSetText(text, this->text);
        hudTextSetOffsetRange(text, 0x58, -0xB0, 0, 1);
        hudTextSetOffsetRange(text, -0x58, 0xB0, 1, 1);
        hudTextSetSize(text, 0x30);
        this->state++;
        break;
    case 1: {
        s32 n;

        for (n = 0; n < 2; n++) {
            text->chars[n].dx >>= 2;
            text->chars[n].dy >>= 2;
        }
        hudTextSetGray(text, (this->frame << 5) + 0x7F);
        if (this->frame >= 4) {
            this->frame = 0;
            this->state++;
        }
        break;
    }
    case 2:
        hudTextSetGray(text, 0xFF - (this->frame << 4));
        if (this->frame >= 8) {
            hudTextSetGray(text, 0x80);
            this->state++;
        }
        break;
    case 3:
        if (this->frame >= 17) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 4:
        hudTextSetGray(text, 0x80 - (this->frame << 3));
        for (i = 0; i < 2; i++) {
            way = (i == 0) ? 1 : -1;
            text->chars[i].dx -= way * this->frame;
            text->chars[i].dy += way * (this->frame << 1);
        }
        if (this->frame >= 17) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frame++;
}

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_drawKo);

/* The YOU WIN! tick (BATTLE_MESSAGE_TICKS): over 12 frames "YOU" closes in from the left (its 2nd and 3rd letters
 * 3 and 5 times faster) while it shrinks from wide to 0x20 square and fades in; over 8 frames the 4 characters
 * from the 5th (palette 1) shrink from tall to square while they fade in; then the gray falls from 0xFF over 32
 * frames and the text fades out over 16, and the message is deleted. */
void BattleMessage_tickYouWin(BattleMessage *this) {
    HudText *text = &this->hudText;
    s32 shown;
    s32 left;

    switch (this->state) {
    case 0:
        hudTextSetText(text, this->text);
        hudTextSetOffsetRange(text, 8, 0, 0, 3);
        hudTextSetOffsetRange(text, -8, 0, 4, 4);
        hudTextSetPaletteRange(text, 1, 4, 4);
        hudTextSetSizeRange(text, 0xA0, 0x20, 0, 3);
        hudTextSetSizeRange(text, 0x20, 0xA0, 4, 4);
        hudTextSetOpacity(text, 0);
        this->state++;
        break;
    case 1:
        shown = (this->frame << 7) / 12;
        left = 0x80 - shown;
        hudTextSetOffsetRange(text, 8 - left, 0, 0, 1);
        hudTextSetOffsetRange(text, 8 - left * 3, 0, 1, 1);
        hudTextSetOffsetRange(text, 8 - left * 5, 0, 2, 1);
        hudTextSetSizeRange(text, left + 0x20, left / 8 + 0x20, 0, 3);
        hudTextSetOpacityRange(text, shown, 0, 3);
        if (this->frame >= 12) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 2:
        shown = this->frame << 4;
        left = 0x80 - shown;
        hudTextSetSizeRange(text, left / 8 + 0x20, left + 0x20, 4, 4);
        hudTextSetOpacityRange(text, shown, 4, 4);
        if (this->frame >= 8) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 3:
        hudTextSetGray(text, 0xFF - (this->frame << 2));
        if (this->frame >= 32) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 4:
        hudTextSetOpacity(text, 0x80 - (this->frame << 3));
        if (this->frame >= 16) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frame++;
}

/* The YOU LOSE tick (BATTLE_MESSAGE_TICKS): YOU WIN!'s, but in state 1, on its first frame, palette 2 for the 4
 * characters from the 5th, and in state 3 the gray held at 0x80. */
void BattleMessage_tickYouLose(BattleMessage *this) {
    HudText *text;

    BattleMessage_tickYouWin(this);
    /* The match depends on the text's address taken once, before the switch. */
    text = &this->hudText;
    switch (this->state) {
    case 1:
        if (this->frame == 1) {
            hudTextSetPaletteRange(text, 2, 4, 4);
        }
        break;
    case 3:
        hudTextSetGray(text, 0x80);
        break;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_tickPlayerWin);

/* The TIME UP tick (BATTLE_MESSAGE_TICKS): slides the cursor in from x 320 to home by quarters, opens its height by
 * 4 a frame, waits 61 frames, then closes it by 4 a frame and deletes the message. */
void BattleMessage_tickTimeUp(BattleMessage *this) {
    switch (this->state) {
    case 0:
        this->cursor.x = 320;
        this->cursor.h = 0;
        this->state++;
        break;
    case 1:
        this->cursor.x += (this->home.x - this->cursor.x) >> 2;
        if (this->cursor.x <= this->home.x) {
            this->shownChars = 7;
            this->state++;
        }
        break;
    case 2:
        this->cursor.h += 4;
        if (this->cursor.h >= this->home.h) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 3:
        if (this->frame >= 61) {
            this->state++;
        }
        break;
    case 4:
        this->cursor.h -= 4;
        if (this->cursor.h <= 0) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frame++;
}

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_drawTimeUp);

INCLUDE_ASM("game/nonmatchings/hud", BattleMessage_tickDrawGame);

s32 spawnGlyphShards(EntityGroup *group, s32 x, s32 y, u8 c);

/* The GAME OVER tick (BATTLE_MESSAGE_TICKS): copies the text into the hud text at a scale of 0x8200, brings the
 * scale down to 0x200 and back to 0x1000 along a cosine over 96 frames, waits 61 frames, breaks the 9 characters
 * into glyph shards in the message's group (spawnGlyphShards, 16 or 32 apart from x 0x18, at y 0x40) with sound
 * 0x4D, and deletes the message 61 frames later. */
void BattleMessage_tickGameOver(BattleMessage *this) {
    HudText *text = &this->hudText;
    s32 angle;
    s32 scale;
    EntityGroup *group;
    u8 *c;
    u8 *end;
    s32 x;

    switch (this->state) {
    case 0:
        hudTextSetText(text, this->text);
        hudTextSetOffsetRange(text, 8, 0, 0, 4);
        hudTextSetOffsetRange(text, -8, 0, 5, 4);
        hudTextSetPalettes(text, 2);
        hudTextSetScale(text, 0x8200);
        this->state++;
        break;
    case 1:
        angle = this->frame << 5;
        if (angle < 0x800) {
            scale = (COS_SIN_TABLE[angle & 0xFFF].cos << 2) + 0x3200;
        } else {
            scale = (COS_SIN_TABLE[angle & 0xFFF].cos * 7) >> 3;
        }
        hudTextSetScale(text, scale + 0x1000);
        if (this->frame >= 0x60) {
            this->frame = 0;
            this->state++;
        }
        break;
    case 2:
        if (this->frame >= 61) {
            this->state++;
        }
        break;
    case 3:
        group = func_8002BDF4(&this->entity);
        /* The match depends on walking c to end with the test at the bottom: an index loop keeps the add of
         * c + i (gcc does not reduce it), a pointer for loop adds a test before the first character. */
        c = this->text;
        end = c + 9;
        x = 0x18;
        do {
            if (spawnGlyphShards(group, x, 0x40, *c) == 0) {
                x += 32;
            } else {
                x += 16;
            }
            c++;
        } while (c != end);
        soundControlPlaySound(&SOUND_CONTROL, 0x4D, 0);
        this->frame = 0;
        this->state++;
        break;
    case 4:
        if (this->frame >= 61) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->frame++;
}

/* The GAME OVER draw (BATTLE_MESSAGE_DRAWS): the text centred at its scale, until 2 frames after it has burst into
 * shards (state 4). */
void BattleMessage_drawGameOver(BattleMessage *this, u_long *ot) {
    if (this->state < 4 || this->frame < 2) {
        s32 width = (this->hudText.length - 1) * this->hudText.step;

        /* The match depends on width scaled in place (one expression ties the product to the other register). */
        width = (width * this->hudText.scale) >> 12;
        hudTextDraw(&this->hudText, ot, (320 - width) / 2, 0x50);
    }
}

/* How many characters BattleMessage_tickDemonstration shows in the visible half of its blink. */
#if VERSION_JP
#define DEMONSTRATION_SHOWN_CHARS 16
#else
#define DEMONSTRATION_SHOWN_CHARS 4
#endif

/* The DEMONSTRATION tick (BATTLE_MESSAGE_TICKS): on its first frame goes to state 1 and moves the text (JP: y 0x78;
 * US: x and y 0x20), then shows the text or nothing, by turns of 16 frames. */
void BattleMessage_tickDemonstration(BattleMessage *this) {
    if (this->state == 0) {
        this->state = 1;
#if VERSION_JP
        this->cursor.y = 0x78;
        this->home.y = 0x78;
        this->shownChars = 13;
#else
        this->cursor.x = 0x20;
        this->home.x = 0x20;
        this->cursor.y = 0x20;
        this->home.y = 0x20;
#endif
    }
    this->shownChars = (this->frame & 0x10) ? 0 : DEMONSTRATION_SHOWN_CHARS;
    this->frame++;
}

/* Sets *length to the text's length, at most HUD_TEXT_MAX. */
static inline void setClampedLength(s32 *length, u8 *text) {
    *length = strlen(text) < HUD_TEXT_MAX ? strlen(text) : HUD_TEXT_MAX;
}

/* Copies text (at most HUD_TEXT_MAX characters) and resets every character: no offset, size 0x20, grey color,
 * palette 0; the step is 0x20 and the scale 1.
 * The match depends on the length being set by an inline that takes its address (setClampedLength): the game
 * computes &this->length into a saved register before the first strlen and reloads the field for strncpy. A plain
 * field store puts it in strncpy's delay slot and passes the value instead. */
void hudTextSetText(HudText *this, u8 *text) {
    s32 i;

    setClampedLength(&this->length, text);
    strncpy(this->text, text, this->length);
    for (i = 0; i < this->length; i++) {
        this->chars[i].dx = 0;
        this->chars[i].dy = 0;
        this->chars[i].w = 0x20;
        this->chars[i].h = 0x20;
        *(u32 *)&this->chars[i].color = 0x80808080; /* the color and cd, a word at once */
        this->chars[i].palette = 0;
        this->chars[i].unk10 = 0;
    }
    this->step = 0x20;
    this->scale = 0x1000;
}

/* Sets every character's offset. */
void hudTextSetOffsets(HudText *this, s32 dx, s32 dy) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].dx = dx;
        this->chars[i].dy = dy;
    }
}

/* Sets the offset of count characters from start on (as far as the text goes). */
void hudTextSetOffsetRange(HudText *this, s32 dx, s32 dy, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].dx = dx;
        this->chars[i].dy = dy;
    }
}

/* Puts the characters on a circle of radius: the first at angle, each next one step further (dx the sine, dy the
 * cosine). */
void hudTextArrangeOnCircle(HudText *this, s32 radius, s32 angle, s32 step) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].dx = (radius * COS_SIN_TABLE[angle & 0xFFF].sin) >> 12;
        this->chars[i].dy = (radius * COS_SIN_TABLE[angle & 0xFFF].cos) >> 12;
        angle += step;
    }
}

/* Sets every character's palette. */
void hudTextSetPalettes(HudText *this, s32 palette) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].palette = palette;
    }
}

/* Sets the palette of count characters from start on (as far as the text goes). */
void hudTextSetPaletteRange(HudText *this, s32 palette, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].palette = palette;
    }
}

/* Sets every character's glyph to a square of size. */
void hudTextSetSize(HudText *this, s32 size) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].w = size;
        this->chars[i].h = size;
    }
}

/* Sets every character's glyph size. */
void hudTextSetSizes(HudText *this, s32 w, s32 h) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].w = w;
        this->chars[i].h = h;
    }
}

/* Sets the glyph size of count characters from start on (as far as the text goes). */
void hudTextSetSizeRange(HudText *this, s32 w, s32 h, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].w = w;
        this->chars[i].h = h;
    }
}

/* Sets the text's scale (4.12 fixed point). */
void hudTextSetScale(HudText *this, s32 scale) {
    this->scale = scale;
}

/* Sets every character's color to a gray. */
void hudTextSetGray(HudText *this, s32 gray) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].color.r = this->chars[i].color.g = this->chars[i].color.b = gray;
    }
}

/* Sets every character's opacity (the color's cd byte, 0 to 0x80). */
void hudTextSetOpacity(HudText *this, s32 opacity) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].color.cd = opacity;
    }
}

/* Sets the opacity of count characters from start on (as far as the text goes). */
void hudTextSetOpacityRange(HudText *this, s32 opacity, s32 start, s32 count) {
    s32 end = this->length;
    s32 i;

    /* The match depends on end starting at the length (the other way round, the registers differ). */
    if (start + count < end) {
        end = start + count;
    }
    for (i = start; i < end; i++) {
        this->chars[i].color.cd = opacity;
    }
}

/* Sets every character's unk10, adding step for each next one. */
void func_800CD534(HudText *this, s32 unk10, s32 step) {
    s32 i;

    for (i = 0; i < this->length; i++) {
        this->chars[i].unk10 = unk10;
        unk10 += step;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", hudTextDraw);

/* The continue screen (ContinuePrompt_init, 0x2C bytes), which Fight_showContinue makes for the player's side:
 * "CONTINUE?" and a countdown; START continues, at 0 it shows GAME OVER. */
typedef struct ContinuePrompt {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 side;           /* gameStateGetPlayerSide; also the pad port */
    /* 0x1C */ s32 gameOverHandle; /* the GAME OVER message's handle, made when the countdown ends */
    /* 0x20 */ s32 unk20;
    /* 0x24 */ s32 framesLeft;     /* counts down a frame at a time; drawn in seconds (/ 30) */
    /* 0x28 */ s32 unk28;
} ContinuePrompt;

extern EntityVtable CONTINUE_PROMPT_VTABLE;

/* Constructs the continue screen for side, with 0x149 frames left, clears GAME_STATE.continued and starts music
 * track 0x11 (once). */
ContinuePrompt *ContinuePrompt_init(ContinuePrompt *this, s32 side) {
    entityInit(&this->entity, 0);
    this->entity.vtable = &CONTINUE_PROMPT_VTABLE;
    this->framesLeft = 0x149;
    this->side = side;
    this->unk20 = 0;
    this->unk28 = 0;
    this->gameOverHandle = 0;
    GAME_STATE.continued = 0;
    stopMusic();
    seekMusicTrack(0x11, 1);
    soundControlResumeMusic(&SOUND_CONTROL);
    return this;
}

INCLUDE_ASM("game/nonmatchings/hud", ContinuePrompt_tick);

INCLUDE_ASM("game/nonmatchings/hud", ContinuePrompt_draw);

INCLUDE_ASM("game/nonmatchings/hud", ScoreTally_init);

/* Destroys the score tally: adds what is left of the bonuses into the score first (ScoreTally_addAll). */
void ScoreTally_destroy(Entity *this, s32 flags) {
    this->vtable = &SCORE_TALLY_VTABLE;
    ScoreTally_addAll((struct ScoreTally *)this);
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* The score tally after a won round (ScoreTally_init), made for a human winner: the round's bonuses (Fight_getBonus)
 * counted 100 a frame into the side's score. */
typedef struct ScoreTally {
    /* 0x00 */ Entity entity;
    /* 0x18 */ s32 player; /* the pad port padManagerGetPressed reads */
    /* 0x1C */ s32 unk1C;  /* the player cannot skip while set */
    /* 0x20 */ s32 unk20;  /* opens the panel while set */
    /* 0x24 */ s32 bonusIndex; /* the bonus being counted */
    /* 0x28 */ s32 bonuses[3]; /* Fight_getBonus with 0 to 2 (or 3, 4) */
    /* 0x34 */ s32 score;      /* Fight_getScore */
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 unk44;
    /* 0x48 */ s32 unk48;
    /* 0x4C */ RECT panel;
    /* 0x54 */ s32 wait;       /* frames left before the next state */
    /* 0x58 */ s32 brightness; /* 0 to 0x80: the fade in and out */
    /* 0x5C */ s32 state;
    /* 0x60 */ s32 frame; /* ScoreTally_tick counts it every frame */
} ScoreTally;

void ScoreTally_skipOnPress(ScoreTally *this);

/* Runs the score tally: fades in while the fields from 0x38 ease to their final values, opens the panel (when unk20)
 * and shrinks it into place, then takes up to 100 a frame from each bonus into the score, and fades out and
 * deletes the tally. Waits `wait` frames between states, and in states 1 to 6 the player can skip
 * (ScoreTally_skipOnPress) unless unk1C. */
void ScoreTally_tick(ScoreTally *this) {
    s32 step;
    s32 *bonus;
    s32 taken;

    if (this->state >= 1 && this->state <= 6 && this->unk1C == 0) {
        ScoreTally_skipOnPress(this);
    }
    switch (this->state) {
    case 1:
        this->brightness += 8;
        if (this->brightness > 0x80) {
            this->brightness = 0x80;
        }
        step = (0xEA - this->unk38) >> 1;
        if (step <= 0) {
            step = 1;
        }
        this->unk38 += step;
        this->unk3C -= this->unk3C >> 1;
        this->unk40 -= this->unk40 >> 1;
        this->unk44 += (0xE8 - this->unk44) >> 1;
        this->unk48 += (0xC5 - this->unk48) >> 1;
        if (this->unk38 >= 0xF2) {
            if (this->unk20) {
                this->panel.x = 0xA0;
                this->panel.y = 0x60;
                this->panel.w = 0x200;
                this->panel.h = 0x80;
                this->state++;
            } else {
                this->state = 6;
            }
        }
        break;
    case 2:
        this->panel.w += (0xE0 - this->panel.w) >> 1;
        this->panel.h += (0x38 - this->panel.h) >> 1;
        if (this->panel.w <= 0xE0) {
            this->wait = 30;
            this->state++;
        }
        break;
    case 4:
        this->panel.x += (0x40 - this->panel.x) >> 1;
        this->panel.y += (0x95 - this->panel.y) >> 1;
        this->panel.w += (0x60 - this->panel.w) >> 1;
        this->panel.h += (0x18 - this->panel.h) >> 1;
        if (this->panel.w <= 0x60) {
            this->wait = 20;
            this->state++;
        }
        break;
    case 6:
        if (this->wait <= 0) {
            bonus = &this->bonuses[this->bonusIndex];
            if (*bonus > 0) {
                taken = *bonus;
                if (taken > 100) {
                    taken = 100;
                }
                *bonus -= taken;
                this->score += taken;
            } else {
                this->bonusIndex++;
                if (this->bonusIndex >= 3) {
                    this->wait = 30;
                    this->state++;
                }
            }
        }
        break;
    /* The match depends on the waiting states' case being here, after case 6 (the game's block order). */
    case 0:
    case 3:
    case 5:
    case 7:
        if (this->wait <= 0) {
            this->state++;
        }
        break;
    case 8:
        this->brightness -= 8;
        if (this->brightness <= 0) {
            entityDelete(&this->entity);
            return;
        }
        break;
    }
    this->wait--;
    this->frame++;
}


/* The buttons that make ScoreTally_skipOnPress skip: the four face buttons, and START too in US. */
#if VERSION_JP
#define SCORE_TALLY_SKIP_BUTTONS 0xF0
#else
#define SCORE_TALLY_SKIP_BUTTONS 0x8F0
#endif

/* When the player presses a skip button: adds the bonuses into the score at once (ScoreTally_addAll) and sets the
 * fields from 0x38 to their final values. */
void ScoreTally_skipOnPress(ScoreTally *this) {
    if (padManagerGetPressed(PAD_MANAGER_INSTANCE, this->player) & SCORE_TALLY_SKIP_BUTTONS) {
        ScoreTally_addAll(this);
        this->unk38 = 0xF2;
        this->unk3C = 0;
        this->unk40 = 0;
        this->unk44 = 0xE8;
        this->unk48 = 0xC5;
        if (this->unk20) {
            this->panel.x = 0x40;
            this->panel.y = 0x94;
            this->panel.w = 0x60;
            this->panel.h = 0x18;
        }
        this->wait = 10;
        this->brightness = 0x80;
        this->state = 7;
    }
}

/* Adds the three bonuses into the score and clears them. */
void ScoreTally_addAll(ScoreTally *this) {
    s32 i;

    for (i = 0; i < 3; i++) {
        this->score += this->bonuses[i];
        this->bonuses[i] = 0;
    }
}

INCLUDE_ASM("game/nonmatchings/hud", ScoreTally_draw);

INCLUDE_ASM("game/nonmatchings/hud", func_800CE240);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DC90);

INCLUDE_ASM("game/nonmatchings/hud", func_800CE388);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCB8);

extern RECT D_8006DCB8; /* the panel's texture rectangle (u, v, w, h) */

/* Draws a semi-transparent textured quad over rect (tpage 0x1E) from the texture rectangle uv with clut. */
void drawSemiTransQuad(u_long *ot, RECT *rect, RECT *uv, u16 clut);

/* The rectangle of the tally's panel, whose x and y are its centre. */
static inline RECT getPanelRect(ScoreTally *this) {
    RECT rect;

    memset(&rect, 0, sizeof(rect));
    rect.x = this->panel.x - (this->panel.w >> 1);
    rect.y = this->panel.y - (this->panel.h >> 1);
    rect.w = this->panel.w;
    rect.h = this->panel.h;
    return rect;
}

/* Draws the tally's panel. The match depends on the rectangle copied twice before the call (getPanelRect's
 * result into rect, then into box, declared first): the game builds it at sp+0x20 and passes sp+0x10. */
void ScoreTally_drawPanel(ScoreTally *this, u_long *ot) {
    RECT box;
    RECT rect = getPanelRect(this);

    box = rect;
    drawSemiTransQuad(ot, &box, &D_8006DCB8, 0x7BBB);
}

INCLUDE_RODATA("game/nonmatchings/hud", SCORE_TALLY_DECIMAL_PLACES);

INCLUDE_ASM("game/nonmatchings/hud", func_800CEB0C);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCDC);

INCLUDE_ASM("game/nonmatchings/hud", func_800CED10);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE0);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE4);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCE8);

INCLUDE_ASM("game/nonmatchings/hud", func_800CEEDC);

/* Draws one semi-transparent sprite: the place and size from rect, the texture coordinates and clut from uvClut. */
void drawSemiTransSprite(u_long *ot, RectWords *rect, u32 *uvClut) {
    SpriteWords *sprite = (SpriteWords *)allocSprt();

    sprite->rgbCode = (((SPRT *)sprite)->code << 24) | 0x808080;
    sprite->xy = rect->xy;
    sprite->uvClut = *uvClut;
    sprite->wh = rect->wh;
    /* The match depends on setting the semi-transparency after the size. */
    setSemiTrans((SPRT *)sprite, 1);
    AddPrim(ot, sprite);
}

INCLUDE_ASM("game/nonmatchings/hud", drawSemiTransQuad);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DCEC);

INCLUDE_ASM("game/nonmatchings/hud", func_800CF2A4);

/* Gives where the font has a character, as (v << 8) | u: the letters, then '0' to ':' (counted from '.'), then '!',
 * '?' and '_'; -1 for any other character. */
s32 getFontGlyphUv(u8 c) {
    s32 index = c;
    u32 v;
    u32 u;

    if ((u8)(index - 'A') < 26) {
        index = (u8)(index - 'A');
        v = (index << 2) & 0xE0;
        u = (index << 5) & 0xFF;
    } else if ((u32)(index - '0') < 11) {
        index = (u8)(index - '.');
        v = (((index & 0xF8) << 2) + 0x60) & 0xFF;
        u = (index << 5) & 0xFF;
    } else {
        switch (index) {
        case '!':
            u = 0xA0;
            v = 0x80;
            break;
        case '?':
            u = 0xC0;
            v = 0x80;
            break;
        case '_':
            u = 0xE0;
            v = 0x80;
            break;
        default:
            return -1;
        }
    }
    return (v << 8) | u;
}

INCLUDE_ASM("game/nonmatchings/hud", drawGlyphAtCursor);

INCLUDE_ASM("game/nonmatchings/hud", drawGlyph);

INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD04);
INCLUDE_RODATA("game/nonmatchings/hud", D_8006DD14);

INCLUDE_ASM("game/nonmatchings/hud", spawnGlyphShards);

INCLUDE_ASM("game/nonmatchings/hud", GlyphShard_init);

INCLUDE_ASM("game/nonmatchings/hud", GlyphShard_tick);

INCLUDE_ASM("game/nonmatchings/hud", GlyphShard_draw);

/* Destroys a battle message. */
void BattleMessage_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Destroys the continue screen. */
void ContinuePrompt_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Destroys a glyph shard. */
void GlyphShard_destroy(Entity *this, s32 flags) {
    entityDestroy(this, 0);
    if (flags & DESTROY_FREE) {
        entityOperatorDelete(this);
    }
}

/* Adds the font's DR_TPAGE (tpage 30) to the ordering table slot, after its glyphs. */
void func_800D008C(u_long *ot) {
    AddPrim(ot, allocDrTpageMode(0, 0, 30));
}

INCLUDE_RODATA("game/nonmatchings/hud", GLYPH_SHARD_VTABLE);
INCLUDE_RODATA("game/nonmatchings/hud", SCORE_TALLY_VTABLE);
INCLUDE_RODATA("game/nonmatchings/hud", CONTINUE_PROMPT_VTABLE);
INCLUDE_RODATA("game/nonmatchings/hud", BATTLE_MESSAGE_VTABLE);
INCLUDE_RODATA("game/nonmatchings/hud", EVOLUTION_MESSAGE_PALETTES);
INCLUDE_RODATA("game/nonmatchings/hud", STR_READY_FIGHT);
INCLUDE_RODATA("game/nonmatchings/hud", STR_NEW_CHALLENGER);
INCLUDE_RODATA("game/nonmatchings/hud", STR_EVOLUTION);
INCLUDE_RODATA("game/nonmatchings/hud", STR_KO);
INCLUDE_RODATA("game/nonmatchings/hud", STR_YOU_WIN);
INCLUDE_RODATA("game/nonmatchings/hud", STR_YOU_LOSE);
INCLUDE_RODATA("game/nonmatchings/hud", STR_1P_WIN);
INCLUDE_RODATA("game/nonmatchings/hud", STR_2P_WIN);
INCLUDE_RODATA("game/nonmatchings/hud", STR_TIME_UP);
INCLUDE_RODATA("game/nonmatchings/hud", STR_DRAW_GAME);
INCLUDE_RODATA("game/nonmatchings/hud", STR_DOUBLE_KO);
INCLUDE_RODATA("game/nonmatchings/hud", STR_GAME_OVER);
INCLUDE_RODATA("game/nonmatchings/hud", STR_DEMONSTRATION);
INCLUDE_RODATA("game/nonmatchings/hud", BATTLE_MESSAGE_LAYOUTS);
