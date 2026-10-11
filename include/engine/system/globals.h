#ifndef DTBE_SYSTEM_GLOBALS_H
#define DTBE_SYSTEM_GLOBALS_H

/* The global objects: the scheduler, the random numbers, the frame's ordering table, the sound and fade tasks, the game state. */

#include "common.h"

EXTERN_C_BEGIN

void initOrDestroyGlobals(s32 initialize, s32 priority);
void initGlobals(void);
void destroyGlobals(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_GLOBALS_H */
