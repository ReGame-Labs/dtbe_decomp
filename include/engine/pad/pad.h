#ifndef DTBE_PAD_PAD_H
#define DTBE_PAD_PAD_H

/* The pads: both ports' buttons and analog sticks, held buttons repeating, rumble. */

#include "common.h"

EXTERN_C_BEGIN

#ifndef offsetof
#define offsetof(type, member) ((s32) & ((type *)0)->member)
#endif

/* Button bits of Pad's button fields (the pad's active-high reading). */
#define PAD_UP 0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000

/* PadInfoMode's terminal ids */
#define PAD_ID_DIGITAL 4
#define PAD_ID_ANALOG 7

/* Vibration's setupStep */
#define SETUP_UNSUPPORTED -1
#define SETUP_START 0
#define SETUP_FIND_ANALOG 40 /* a DualShock in digital mode: switch it to analog */
#define SETUP_SET_ANALOG 41
#define SETUP_WAIT_ANALOG 42
#define SETUP_ACTUATORS 70
#define SETUP_DONE 71
#define SETUP_FAILED 99

/* Vibration's alignStep */
#define ALIGN_NONE 0
#define ALIGN_SEND 1 /* send the actuator alignment */
#define ALIGN_WAIT 2 /* wait for the pad to take it */
#define ALIGN_DONE 3

/* The vibration of one pad: its actuator setup and the current rumble. */
typedef struct {
    s32 setupStep;    /* where the pad's mode and actuator setup is */
    s32 hasActuators;
    s32 alignStep;    /* where sending the actuator alignment is */
    s32 mode;         /* how the rumble fades; -1 when there is none */
    s32 priority;     /* a new rumble replaces the current one only at no lower priority */
    s32 timer;        /* frames left */
    s32 duration;     /* frames in all */
    s32 power[2];     /* the motors' power at full strength */
    u8 motor[2];      /* what is sent to the motors this frame */
    u8 unk26[4];
    u8 align[6];      /* the actuator alignment sent to the pad */
} Vibration;

/* The analog sticks of a pad, each axis from -0x1000 to 0x1000. */
typedef struct {
    s32 leftX;
    s32 leftY;
    s32 rightX;
    s32 rightY;
} Sticks;

/* One controller port and the state read from it. */
typedef struct {
    u8 buf[0x22];     /* what PadInitDirect receives into */
    u8 unk22[2];
    s32 port;         /* the port, as PadGetState and the like take it */
    s32 state;        /* PadGetState's last answer */
    s32 stable;       /* whether the pad is connected and set up */
    s32 type;         /* the terminal type of the pad */
    s32 deadZone;     /* how far from the centre an analog stick must go to count */
    u32 raw;          /* the buttons, with the left stick as the d-pad */
    u32 held;         /* the buttons, without opposite directions */
    u32 pressed;      /* the buttons pressed this frame */
    u32 released;     /* the buttons released this frame */
    u32 repeated;     /* the buttons pressed this frame or repeating */
    s32 repeatTimer;
    s32 repeatDelay;  /* frames until a held button repeats */
    s32 repeatRestart; /* where the timer restarts after a repeat */
    Sticks sticks;
    Vibration vibration;
} Pad;

/* The game's controllers, as one object; its state is static. */
typedef struct PadManager PadManager;

extern struct PadManager PAD_MANAGER; /* the pad manager */

/* the game's compiler addressed these word-sized globals as small data */
extern PadManager *PAD_MANAGER_INSTANCE;
extern Pad PADS[2];
/* whether reading the pads is paused */
extern s32 PADS_PAUSED;

void padManagerStartVibration(PadManager *padManager, s32 port, s32 mode, u8 power0, u8 power1, s32 duration,
                   s32 priority);
void vibrationStart(Vibration *vibration, s32 mode, u8 power0, u8 power1, s32 duration,
                   s32 priority);

PadManager *padManagerInit(PadManager *padManager);
void padManagerDestroy(PadManager *padManager, s32 flags);
void padManagerStart(PadManager *padManager);
void padManagerSetRepeat(PadManager *padManager, s32 delay, s32 rate);
void resetPads(void);
void padManagerUpdate(PadManager *padManager);
u32 padManagerGetButtons(PadManager *padManager, s32 port, s32 offset);
u32 padManagerGetHeld(PadManager *padManager, s32 port);
u32 padManagerGetPressed(PadManager *padManager, s32 port);
u32 padManagerGetReleased(PadManager *padManager, s32 port);
u32 padManagerGetRepeated(PadManager *padManager, s32 port);
void padManagerGetSticks(PadManager *padManager, s32 port, Sticks *sticks);
void padManagerSetDeadZone(PadManager *padManager, s32 deadZone);
void padManagerSetPaused(PadManager *padManager, s32 paused);
void padInit(Pad *pad, s32 port, s32 slot);
void padReset(Pad *pad);
void padUpdateVibration(Pad *pad);
u32 cancelOppositeDirections(u32 buttons);
void padRead(Pad *pad);
void padUpdateRepeat(Pad *pad);
void padSetRepeat(Pad *pad, s32 delay, s32 rate);
s32 padUpdateState(Pad *pad);
s32 padUpdateSetup(Pad *pad);
void vibrationInit(Vibration *vibration);
void vibrationUpdate(Vibration *vibration);
void vibrationStop(Vibration *vibration);

struct PadManager *getPadManager(void);

EXTERN_C_END

#endif /* DTBE_PAD_PAD_H */
