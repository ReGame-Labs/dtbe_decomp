#ifndef DTBE_SOUND_SOUND_EFFECT_H
#define DTBE_SOUND_SOUND_EFFECT_H

/* The sound effects: playing them on the sound system's voices, and their handles. */

#include "common.h"
#include "engine/sound/sound.h"
#include "engine/system/handle_table.h"

EXTERN_C_BEGIN

/* how many handles SOUND_EFFECT_HANDLES holds */
#define SND_HANDLE_CAPACITY 0x30

/* the handles of the playing sound effects */
extern HandleTable SOUND_EFFECT_HANDLES;

s32 playSoundEffect(u16 effect);
SndPlayingEffect *allocPlayingEffect(s32 effect, u8 volumeLeft, u8 volumeRight);
u32 playSoundEffectAtVolume(u16 effect, u8 volumeLeft, u8 volumeRight);
void sndPlayingEffectFree(SndPlayingEffect *sndPlayingEffect);
void sndPlayingEffectKeyOff(SndPlayingEffect *sndPlayingEffect);
void sndPlayingEffectStop(SndPlayingEffect *sndPlayingEffect);
s32 canPlaySoundEffect(u16 effect);
void keyOffSoundEffectGroup(s32 group);
s32 getSoundEffectVoice(u32 handle);
void stopSoundEffect(u32 handle);
s32 transferVabBody(u16 vab, u8 *data);
void initOrDestroySoundEffectHandles(s32 initialize, s32 priority);
u32 allocSoundEffectHandle(SndPlayingEffect *effect);
SndPlayingEffect *findPlayingEffect(u32 handle);
void freeSoundEffectHandle(u32 handle);
void initSoundEffectHandles(void);
void destroySoundEffectHandles(void);

EXTERN_C_END

#endif /* DTBE_SOUND_SOUND_EFFECT_H */
