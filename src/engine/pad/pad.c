#include "common.h"
#include "engine/pad/pad.h"
#include "engine/system/memory.h"
#include "libgpu.h"
#include "libpad.h"
#include "memory.h"

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
    if (flags & 1) {
        operatorDelete(this);
    }
}

/* Starts the pad library on both ports' receive buffers. */
void padManagerStart(PadManager *this) {
    PadInitDirect(PADS[0].buf, PADS[1].buf);
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
void padInit(Pad *pad, s32 port, s32 slot) {
    pad->port = (port << 4) | slot;
    pad->stable = FALSE;
    pad->state = PadStateExecCmd;
    pad->deadZone = 0x25;
    vibrationInit(&pad->vibration);
    padSetRepeat(pad, 16, 4);
}

/* Reads a pad, then forgets its vibration and button events. */
void padReset(Pad *pad) {
    padRead(pad);
    vibrationStop(&pad->vibration);
    pad->pressed = 0;
    pad->released = 0;
    pad->repeated = 0;
    pad->repeatTimer = 0;
    padSetRepeat(pad, 16, 4);
}

/* Runs a pad's vibration for one frame. */
void padUpdateVibration(Pad *pad) {
    vibrationUpdate(&pad->vibration);
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
 * Reads a pad: its analog sticks, and its buttons with the left stick as the
 * d-pad, then what was pressed and released. What the C needs is known: its
 * jump table covers the pad types 0 to 8 (so the switch has labels for more
 * types than the two it reads, 4 and 7, sending the rest where a pad reads as
 * nothing); the low half of an axis adds the dead zone as a statement of its
 * own, and the result starts at 0. Not known: why the original keeps an axis'
 * byte in a1 and its distance in v0, where GCC swaps them.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/pad/pad", padRead);

/* Works out the buttons repeating this frame from the ones pressed and held. */
void padUpdateRepeat(Pad *pad) {
    u32 held;

    pad->repeated = 0;
    if (pad->pressed != 0) {
        pad->repeated = pad->pressed;
        pad->repeatTimer = 0;
        return;
    }
    held = pad->raw;
    if (held != 0) {
        pad->repeatTimer++;
        if (pad->repeatDelay < pad->repeatTimer) {
            pad->repeated = held;
            pad->repeatTimer = pad->repeatRestart;
        }
    }
}

/* Sets after how many frames, then every how many, a held button repeats. */
void padSetRepeat(Pad *pad, s32 delay, s32 rate) {
    pad->repeatDelay = delay;
    pad->repeatRestart = delay - rate;
}

/* Polls the state of a pad's port; returns whether a pad is ready there. */
s32 padUpdateState(Pad *pad) {
    s32 state = PadGetState(pad->port);
    s32 stable = FALSE;

    if (state == PadStateFindCTP1 || state == PadStateStable) {
        stable = TRUE;
    }
    if (state == PadStateDiscon) {
        pad->vibration.setupStep = 0;
    }
    pad->state = state;
    pad->stable = stable;
    return stable;
}

/*
 * Takes one step of a pad's setup: switches a digital-mode DualShock to analog,
 * finds its actuators and sends their alignment. Returns whether the setup is over.
 */
s32 padUpdateSetup(Pad *pad) {
    s32 i;
    s32 id;
    s32 exId;

    if (pad->vibration.hasActuators && pad->vibration.setupStep != SETUP_START) {
        switch (pad->vibration.alignStep) {
        case ALIGN_NONE:
            break;
        case ALIGN_SEND:
            pad->vibration.align[0] = 0;
            pad->vibration.align[1] = 1;
            for (i = 2; i < 6; i++) {
                pad->vibration.align[i] = 0xFF;
            }
            if (PadSetActAlign(pad->port, pad->vibration.align) == 1) {
                pad->vibration.alignStep++;
            }
            return FALSE;
        case ALIGN_WAIT:
            if (pad->state != PadStateStable) {
                return FALSE;
            }
            pad->vibration.alignStep = ALIGN_DONE;
            return FALSE;
        case ALIGN_DONE:
            break;
        }
    }
    switch (pad->vibration.setupStep) {
    case SETUP_START:
        pad->type = 0;
        pad->vibration.hasActuators = FALSE;
        pad->vibration.alignStep = ALIGN_NONE;
        id = PadInfoMode(pad->port, InfoModeCurID, 0);
        if (id == 0) {
            break;
        }
        exId = PadInfoMode(pad->port, InfoModeCurExID, 0);
        if (exId > 0) {
            id = exId;
        }
        switch (id) {
        case PAD_ID_DIGITAL:
            pad->vibration.setupStep = SETUP_FIND_ANALOG;
            break;
        case PAD_ID_ANALOG:
            pad->vibration.setupStep = SETUP_ACTUATORS;
            break;
        default:
            pad->vibration.setupStep = SETUP_UNSUPPORTED;
            break;
        }
        break;
    case SETUP_FIND_ANALOG:
        if (PadInfoMode(pad->port, InfoModeIdTable, -1) == 0) {
            pad->vibration.setupStep = SETUP_FAILED;
            break;
        }
        pad->vibration.setupStep++;
        /* fall through */
    case SETUP_SET_ANALOG:
        if (PadSetMainMode(pad->port, PadInfoMode(pad->port, InfoModeCurExOffs, 0) + 1, 1) != 1) {
            break;
        }
        pad->vibration.setupStep++;
        break;
    case SETUP_WAIT_ANALOG:
        if (pad->state != PadStateStable) {
            break;
        }
        pad->vibration.setupStep = SETUP_ACTUATORS;
        break;
    case SETUP_ACTUATORS:
        if (!pad->stable) {
            break;
        }
        if (PadInfoAct(pad->port, -1, 0) == 0) {
            pad->vibration.setupStep = SETUP_FAILED;
            break;
        }
        pad->vibration.hasActuators = TRUE;
        pad->vibration.setupStep++;
        break;
    default:
        return TRUE;
    }
    return FALSE;
}

/* Constructs a pad's vibration: no rumble. */
void vibrationInit(Vibration *vibration) {
    bzero((u8 *)vibration, sizeof(Vibration));
    vibration->mode = -1;
}

/* Runs a rumble for one frame: counts its frames and works out the motors' power. */
void vibrationUpdate(Vibration *vibration) {
    s32 elapsed;

    vibration->timer--;
    if (vibration->timer < 0) {
        vibration->timer = 0;
        vibration->mode = -1;
    }
    switch (vibration->mode) {
    case 0:
        break;
    case 1:
        /* fading out */
        vibration->motor[0] = vibration->power[0] * vibration->timer / vibration->duration;
        vibration->motor[1] = vibration->power[1] * vibration->timer / vibration->duration;
        break;
    case 2:
        /* fading in */
        elapsed = vibration->duration - vibration->timer;
        vibration->motor[0] = vibration->power[0] * elapsed / vibration->duration;
        vibration->motor[1] = vibration->power[1] * elapsed / vibration->duration;
        break;
    default:
        vibration->motor[0] = 0;
        vibration->motor[1] = 0;
        break;
    }
}

/* Starts a rumble unless one of higher priority is running. */
void vibrationStart(Vibration *vibration, s32 mode, u8 power0, u8 power1, s32 duration,
                   s32 priority) {
    if (vibration->timer != 0 && priority < vibration->priority) {
        return;
    }
    vibration->priority = priority;
    vibration->mode = mode;
    if ((u32)mode < 2) {
        vibration->motor[0] = power0;
        vibration->motor[1] = power1;
    } else {
        vibration->motor[0] = 0;
        vibration->motor[1] = 0;
    }
    vibration->power[0] = power0;
    vibration->power[1] = power1;
    vibration->timer = duration;
    vibration->duration = duration;
}

/* Stops a pad's rumble. */
void vibrationStop(Vibration *vibration) {
    vibration->mode = -1;
    vibration->motor[0] = 0;
    vibration->motor[1] = 0;
}

/* Returns the pad manager. */
PadManager *getPadManager(void) {
    return PAD_MANAGER_INSTANCE;
}
