#include "common.h"
#include "engine/pad/pad.h"
#include "engine/system/memory.h"
#include "libpad.h"
#include "memory.h"
#include "vtable.h"

/* Constructs the pad manager: sets up both ports, not paused. */
PadManager *padManagerInit(PadManager *this) {
    s32 port;

    PADS_PAUSED = FALSE;
    PAD_MANAGER_INSTANCE = this;
    for (port = 0; port < 2; port++) {
        padInit(&PADS[port], port, 0);
    }
    return this;
}

/* Destroys the pad manager. */
void padManagerDestroy(PadManager *this, s32 flags) {
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Starts the pad library on both ports' receive buffers. */
void padManagerStart(PadManager *this) {
    PadInitDirect(PADS[0].buffer, PADS[1].buffer);
    PadStartCom();
}

/* Sets after how many frames, then every how many, a held button repeats on both pads. */
void padManagerSetRepeat(PadManager *this, s32 delay, s32 rate) {
    Pad *pad = PADS;
    s32 i;

    for (i = 1; i >= 0; i--) {
        padSetRepeat(pad, delay, rate);
        pad++;
    }
}

/* Resets both pads. */
void resetPads(void) {
    Pad *pad = PADS;
    s32 i;

    for (i = 1; i >= 0; i--) {
        padReset(pad);
        pad++;
    }
}

/* Runs both pads' vibration and reads them, unless paused. */
void padManagerUpdate(PadManager *this) {
    Pad *pad;
    s32 i;

    if (!PADS_PAUSED) {
        pad = PADS;
        for (i = 1; i >= 0; i--) {
            padUpdateVibration(pad);
            padRead(pad);
            pad++;
        }
    }
}

/* Starts a rumble on a pad. */
void padManagerStartVibration(PadManager *this, s32 port, s32 mode, u8 power0, u8 power1, s32 duration,
                   s32 priority) {
    vibrationStart(&PADS[port].vibration, mode, power0, power1, duration, priority);
}

/* Returns the button field at offset of a pad, or of both pads together when port is negative. */
u32 padManagerGetButtons(PadManager *this, s32 port, s32 offset) {
    u32 buttons;
    u8 *field;
    s32 i;

    if (port < 0) {
        buttons = 0;
        field = (u8 *)PADS + offset;
        for (i = 1; i >= 0; i--) {
            buttons |= *(u32 *)field;
            field += sizeof(Pad);
        }
        return cancelOppositeDirections(buttons);
    }
    return *(u32 *)((u8 *)&PADS[port] + offset);
}

/* Returns the buttons held on a pad (port -1: on either). */
u32 padManagerGetHeld(PadManager *this, s32 port) {
    return padManagerGetButtons(this, port, offsetof(Pad, held));
}

/* Returns the buttons pressed this frame on a pad (port -1: on either). */
u32 padManagerGetPressed(PadManager *this, s32 port) {
    return padManagerGetButtons(this, port, offsetof(Pad, pressed));
}

/* Returns the buttons released this frame on a pad (port -1: on either). */
u32 padManagerGetReleased(PadManager *this, s32 port) {
    return padManagerGetButtons(this, port, offsetof(Pad, released));
}

/* Returns the buttons pressed or repeating this frame on a pad (port -1: on either). */
u32 padManagerGetRepeated(PadManager *this, s32 port) {
    return padManagerGetButtons(this, port, offsetof(Pad, repeated));
}

/* Copies a pad's analog sticks. */
void padManagerGetSticks(PadManager *this, s32 port, Sticks *sticks) {
    *sticks = PADS[port].sticks;
}

/* Sets the analog sticks' dead zone of both pads. */
void padManagerSetDeadZone(PadManager *this, s32 deadZone) {
    s32 i;

    for (i = 1; i >= 0; i--) {
        PADS[i].deadZone = deadZone;
    }
}

/* Pauses or resumes reading the pads, resetting them when that changes. */
void padManagerSetPaused(PadManager *this, s32 paused) {
    if (PADS_PAUSED != paused) {
        resetPads();
    }
    PADS_PAUSED = paused;
}

/* Sets up a pad on a port and multitap slot. */
void padInit(Pad *this, s32 port, s32 slot) {
    this->port = (port << 4) | slot;
    this->stable = FALSE;
    this->state = PadStateExecCmd;
    this->deadZone = PAD_DEAD_ZONE;
    vibrationInit(&this->vibration);
    padSetRepeat(this, PAD_REPEAT_DELAY, PAD_REPEAT_RATE);
}

/* Reads a pad, then forgets its vibration and button events. */
void padReset(Pad *this) {
    padRead(this);
    vibrationStop(&this->vibration);
    this->pressed = 0;
    this->released = 0;
    this->repeated = 0;
    this->repeatTimer = 0;
    padSetRepeat(this, PAD_REPEAT_DELAY, PAD_REPEAT_RATE);
}

/* Runs a pad's vibration for one frame. */
void padUpdateVibration(Pad *this) {
    vibrationUpdate(&this->vibration);
}

/* Drops both directions of an opposite pair held together. */
u32 cancelOppositeDirections(u32 buttons) {
    if ((buttons & (PAD_UP | PAD_DOWN)) == (PAD_UP | PAD_DOWN)) {
        buttons &= ~(PAD_UP | PAD_DOWN);
    }
    if ((buttons & (PAD_RIGHT | PAD_LEFT)) == (PAD_RIGHT | PAD_LEFT)) {
        buttons &= ~(PAD_RIGHT | PAD_LEFT);
    }
    return buttons;
}

/*
 * Turns an analog stick axis (0 to 0xFF, centered on 0x80) into -0x1000 to
 * 0x1000, as 0 within deadZone of the center. distance is how far the stick
 * is past the dead zone, then how far it can go past it: the original keeps
 * both in one variable (GCC then gives it the divisor's v0).
 */
static inline s32 padAxis(u8 value, u8 deadZone) {
    s32 distance = value - 0x80;
    s32 scaled;
    s32 axis = 0;

    if (distance > 0) {
        distance -= deadZone;
        if (distance > 0) {
            scaled = distance << 12;
            distance = 0x7F - deadZone;
            axis = scaled / distance;
        }
    } else {
        distance = value - 0x7F;
        distance += deadZone;
        if (distance < 0) {
            scaled = distance << 12;
            distance = 0x7F - deadZone;
            axis = scaled / distance;
        }
    }
    return axis;
}

/*
 * Reads a pad: its analog sticks, and its buttons with the left stick as the
 * d-pad, then what was pressed and released. Its jump table covers the pad
 * types 0 to 8, so the switch has labels for the types it doesn't read,
 * sending them where a pad reads as nothing; the low half of an axis adds the
 * dead zone as a statement of its own, and the result starts at 0. The held
 * buttons go back into buttons (the original copies the call's v0 to buttons'
 * a1).
 */
void padRead(Pad *pad) {
    s32 type;
    u32 buttons;
    u32 previous;
    u32 changed;
    s32 deadZone;

    if (padUpdateState(pad) && padUpdateSetup(pad) && pad->buffer[0] == 0) {
        type = pad->buffer[1] >> 4;
        if (pad->type != type) {
            pad->type = type;
            if (pad->vibration.hasActuators) {
                pad->vibration.alignStep = ALIGN_SEND;
            }
        } else {
            switch (pad->type) {
            case PAD_ID_DIGITAL:
                pad->sticks.rightX = 0;
                pad->sticks.rightY = 0;
                pad->sticks.leftX = 0;
                pad->sticks.leftY = 0;
                break;
            case PAD_ID_ANALOG:
                if (pad->vibration.hasActuators && pad->vibration.alignStep == ALIGN_DONE) {
                    PadSetAct(pad->port, pad->vibration.motor, 2);
                }
                deadZone = pad->deadZone;
                pad->sticks.rightX = padAxis(pad->buffer[4], deadZone);
                pad->sticks.rightY = padAxis(pad->buffer[5], deadZone);
                pad->sticks.leftX = padAxis(pad->buffer[6], deadZone);
                pad->sticks.leftY = padAxis(pad->buffer[7], deadZone);
                break;
            case PAD_ID_NONE:
            case PAD_ID_MOUSE:
            case PAD_ID_NEGCON:
            case PAD_ID_KONAMI_GUN:
            case PAD_ID_ANALOG_STICK:
            case PAD_ID_GUNCON:
            case PAD_ID_MULTITAP:
            default:
                goto none;
            }
            /* the pad sends its buttons active-low, big-endian */
            buttons = ((pad->buffer[2] << 8) | pad->buffer[3]) ^ 0xFFFF;
            if (pad->sticks.leftX < -0x800) {
                buttons |= PAD_LEFT;
            }
            if (pad->sticks.leftX > 0x800) {
                buttons |= PAD_RIGHT;
            }
            if (pad->sticks.leftY < -0x800) {
                buttons |= PAD_UP;
            }
            if (pad->sticks.leftY > 0x800) {
                buttons |= PAD_DOWN;
            }
            pad->raw = buttons;
            previous = pad->held;
            buttons = cancelOppositeDirections(buttons);
            pad->held = buttons;
            changed = buttons ^ previous;
            pad->pressed = buttons & changed;
            pad->released = ~buttons & changed;
            padUpdateRepeat(pad);
            return;
        }
    }
none:
    pad->raw = 0;
    pad->held = 0;
    pad->pressed = 0;
    pad->released = 0;
    pad->repeated = 0;
    pad->repeatTimer = 0;
    pad->sticks.rightX = 0;
    pad->sticks.rightY = 0;
    pad->sticks.leftX = 0;
    pad->sticks.leftY = 0;
}

/* Works out the buttons repeating this frame from the ones pressed and held. */
void padUpdateRepeat(Pad *this) {
    u32 held;

    this->repeated = 0;
    if (this->pressed != 0) {
        this->repeated = this->pressed;
        this->repeatTimer = 0;
        return;
    }
    held = this->raw;
    if (held != 0) {
        this->repeatTimer++;
        if (this->repeatDelay < this->repeatTimer) {
            this->repeated = held;
            this->repeatTimer = this->repeatRestart;
        }
    }
}

/* Sets after how many frames, then every how many, a held button repeats. */
void padSetRepeat(Pad *this, s32 delay, s32 rate) {
    this->repeatDelay = delay;
    this->repeatRestart = delay - rate;
}

/* Polls the state of a pad's port; returns whether a pad is ready there. */
s32 padUpdateState(Pad *this) {
    s32 state = PadGetState(this->port);
    s32 stable = FALSE;

    if (state == PadStateFindCTP1 || state == PadStateStable) {
        stable = TRUE;
    }
    if (state == PadStateDiscon) {
        this->vibration.setupStep = SETUP_START;
    }
    this->state = state;
    this->stable = stable;
    return stable;
}

/*
 * Takes one step of a pad's setup: switches a digital-mode DualShock to analog,
 * finds its actuators and sends their alignment. Returns whether the setup is over.
 */
s32 padUpdateSetup(Pad *this) {
    s32 i;
    s32 id;
    s32 exId;

    if (this->vibration.hasActuators && this->vibration.setupStep != SETUP_START) {
        switch (this->vibration.alignStep) {
        case ALIGN_NONE:
            break;
        case ALIGN_SEND:
            this->vibration.align[0] = 0;
            this->vibration.align[1] = 1;
            for (i = 2; i < 6; i++) {
                this->vibration.align[i] = 0xFF;
            }
            if (PadSetActAlign(this->port, this->vibration.align) == 1) {
                this->vibration.alignStep++;
            }
            return FALSE;
        case ALIGN_WAIT:
            if (this->state != PadStateStable) {
                return FALSE;
            }
            this->vibration.alignStep = ALIGN_DONE;
            return FALSE;
        case ALIGN_DONE:
            break;
        }
    }
    switch (this->vibration.setupStep) {
    case SETUP_START:
        this->type = 0;
        this->vibration.hasActuators = FALSE;
        this->vibration.alignStep = ALIGN_NONE;
        id = PadInfoMode(this->port, InfoModeCurID, 0);
        if (id == 0) {
            break;
        }
        exId = PadInfoMode(this->port, InfoModeCurExID, 0);
        if (exId > 0) {
            id = exId;
        }
        switch (id) {
        case PAD_ID_DIGITAL:
            this->vibration.setupStep = SETUP_FIND_ANALOG;
            break;
        case PAD_ID_ANALOG:
            this->vibration.setupStep = SETUP_ACTUATORS;
            break;
        default:
            this->vibration.setupStep = SETUP_UNSUPPORTED;
            break;
        }
        break;
    case SETUP_FIND_ANALOG:
        if (PadInfoMode(this->port, InfoModeIdTable, -1) == 0) {
            this->vibration.setupStep = SETUP_FAILED;
            break;
        }
        this->vibration.setupStep++;
        /* fall through */
    case SETUP_SET_ANALOG:
        if (PadSetMainMode(this->port, PadInfoMode(this->port, InfoModeCurExOffs, 0) + 1, 1) != 1) {
            break;
        }
        this->vibration.setupStep++;
        break;
    case SETUP_WAIT_ANALOG:
        if (this->state != PadStateStable) {
            break;
        }
        this->vibration.setupStep = SETUP_ACTUATORS;
        break;
    case SETUP_ACTUATORS:
        if (!this->stable) {
            break;
        }
        if (PadInfoAct(this->port, -1, 0) == 0) {
            this->vibration.setupStep = SETUP_FAILED;
            break;
        }
        this->vibration.hasActuators = TRUE;
        this->vibration.setupStep++;
        break;
    default:
        return TRUE;
    }
    return FALSE;
}

/* Constructs a pad's vibration: no rumble. */
void vibrationInit(Vibration *this) {
    bzero((u8 *)this, sizeof(Vibration)); /* bzero takes bytes */
    this->mode = VIBRATION_NONE;
}

/* Runs a rumble for one frame: counts its frames and works out the motors' power. */
void vibrationUpdate(Vibration *this) {
    s32 elapsed;

    this->timer--;
    if (this->timer < 0) {
        this->timer = 0;
        this->mode = VIBRATION_NONE;
    }
    switch (this->mode) {
    case VIBRATION_STEADY:
        break;
    case VIBRATION_FADE_OUT:
        this->motor[0] = this->power[0] * this->timer / this->duration;
        this->motor[1] = this->power[1] * this->timer / this->duration;
        break;
    case VIBRATION_FADE_IN:
        elapsed = this->duration - this->timer;
        this->motor[0] = this->power[0] * elapsed / this->duration;
        this->motor[1] = this->power[1] * elapsed / this->duration;
        break;
    default:
        this->motor[0] = 0;
        this->motor[1] = 0;
        break;
    }
}

/* Starts a rumble unless one of higher priority is running. */
void vibrationStart(Vibration *this, s32 mode, u8 power0, u8 power1, s32 duration,
                   s32 priority) {
    if (this->timer != 0 && priority < this->priority) {
        return;
    }
    this->priority = priority;
    this->mode = mode;
    /* unsigned: VIBRATION_NONE is not below it either */
    if ((u32)mode < VIBRATION_FADE_IN) {
        this->motor[0] = power0;
        this->motor[1] = power1;
    } else {
        this->motor[0] = 0;
        this->motor[1] = 0;
    }
    this->power[0] = power0;
    this->power[1] = power1;
    this->timer = duration;
    this->duration = duration;
}

/* Stops a pad's rumble. */
void vibrationStop(Vibration *this) {
    this->mode = VIBRATION_NONE;
    this->motor[0] = 0;
    this->motor[1] = 0;
}

/* Returns the pad manager. */
PadManager *getPadManager(void) {
    return PAD_MANAGER_INSTANCE;
}
