#ifndef DTBE_PAD_PAD_H
#define DTBE_PAD_PAD_H

/* The pads: both ports' buttons and analog sticks, held buttons repeating, rumble. */

#include "common.h"

EXTERN_C_BEGIN

/* Button bits of Pad's button fields (the pad's active-high reading). */
#define PAD_UP 0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000

/* The terminal ids of the controllers (PadInfoMode's, and the high nibble of
 * a pad's second received byte), as the hardware numbers them */
#define PAD_ID_NONE 0
#define PAD_ID_MOUSE 1
#define PAD_ID_NEGCON 2
#define PAD_ID_KONAMI_GUN 3
#define PAD_ID_DIGITAL 4
#define PAD_ID_ANALOG_STICK 5
#define PAD_ID_GUNCON 6
#define PAD_ID_ANALOG 7
#define PAD_ID_MULTITAP 8

/* Vibration's setupStep */
#define SETUP_UNSUPPORTED -1
#define SETUP_START 0
#define SETUP_FIND_ANALOG 40 /* a DualShock in digital mode: switch it to analog */
#define SETUP_SET_ANALOG 41
#define SETUP_WAIT_ANALOG 42
#define SETUP_ACTUATORS 70
#define SETUP_DONE 71
#define SETUP_FAILED 99

/* Vibration's mode: how the rumble's power goes */
#define VIBRATION_NONE -1
#define VIBRATION_STEADY 0
#define VIBRATION_FADE_OUT 1
#define VIBRATION_FADE_IN 2

/* Vibration's alignStep */
#define ALIGN_NONE 0
#define ALIGN_SEND 1 /* send the actuator alignment */
#define ALIGN_WAIT 2 /* wait for the pad to take it */
#define ALIGN_DONE 3

/* The vibration of one pad: its actuator setup and the current rumble. */
typedef struct {
    /* 0x00 */ s32 setupStep; /* where the pad's mode and actuator setup is */
    /* 0x04 */ s32 hasActuators;
    /* 0x08 */ s32 alignStep; /* where sending the actuator alignment is */
    /* 0x0C */ s32 mode;      /* VIBRATION_* */
    /* 0x10 */ s32 priority;  /* a new rumble replaces the current one only at no lower priority */
    /* 0x14 */ s32 timer;     /* frames left */
    /* 0x18 */ s32 duration;  /* frames in all */
    /* 0x1C */ s32 power[2];  /* the motors' power at full strength */
    /* 0x24 */ u8 motor[2];   /* what is sent to the motors this frame */
    /* 0x26 */ u8 unk26[4];
    /* 0x2A */ u8 align[6];   /* the actuator alignment sent to the pad */
} Vibration;

/* The analog sticks of a pad, each axis from -0x1000 to 0x1000. */
typedef struct {
    /* 0x0 */ s32 leftX;
    /* 0x4 */ s32 leftY;
    /* 0x8 */ s32 rightX;
    /* 0xC */ s32 rightY;
} Sticks;

/* One controller port and the state read from it. */
typedef struct {
    /* 0x00 */ u8 buffer[0x22];    /* what PadInitDirect receives into */
    /* 0x22 */ u8 unk22[2];
    /* 0x24 */ s32 port;           /* the port, as PadGetState and the like take it */
    /* 0x28 */ s32 state;          /* PadGetState's last answer */
    /* 0x2C */ s32 stable;         /* whether the pad is connected and set up */
    /* 0x30 */ s32 type;           /* the terminal type of the pad */
    /* 0x34 */ s32 deadZone;       /* how far from the center an analog stick must go to count */
    /* 0x38 */ u32 raw;            /* the buttons, with the left stick as the d-pad */
    /* 0x3C */ u32 held;           /* the buttons, without opposite directions */
    /* 0x40 */ u32 pressed;        /* the buttons pressed this frame */
    /* 0x44 */ u32 released;       /* the buttons released this frame */
    /* 0x48 */ u32 repeated;       /* the buttons pressed this frame or repeating */
    /* 0x4C */ s32 repeatTimer;
    /* 0x50 */ s32 repeatDelay;    /* frames until a held button repeats */
    /* 0x54 */ s32 repeatRestart;  /* where the timer restarts after a repeat */
    /* 0x58 */ Sticks sticks;
    /* 0x68 */ Vibration vibration;
} Pad;

/* a pad's settings until the game changes them: the dead zone, and after how
 * many frames, then every how many, a held button repeats */
#define PAD_DEAD_ZONE 0x25
#define PAD_REPEAT_DELAY 16
#define PAD_REPEAT_RATE 4

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
