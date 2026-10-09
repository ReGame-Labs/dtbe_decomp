#ifndef DTBE_SOUND_VOICE_PAUSE_H
#define DTBE_SOUND_VOICE_PAUSE_H

/* Pausing the sound effects: silencing their SPU voices, and giving them back. */

#include "common.h"

EXTERN_C_BEGIN

void setSoundEffectsPaused(s32 paused);
void resumeVoices(s32 voices);
u32 getSoundEffectVoices(void);

EXTERN_C_END

#endif /* DTBE_SOUND_VOICE_PAUSE_H */
