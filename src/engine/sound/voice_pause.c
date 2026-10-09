#include "common.h"
#include "engine/sound/voice_pause.h"
#include "engine/sound/sound.h"
#include "libsnd.h"
#include "memory.h"
#include "psyq.h"

/* Pauses or resumes the sound effects. Pausing keeps the pitch and volume of
 * the voices that play, or -1 for the pitch of the others, and silences them. */
void setSoundEffectsPaused(s32 paused) {
    SpuVoiceAttr attr;
    u32 voices;
    s32 i;
    SpuVolume *volume;
    s16 *pitch;
    s32 voicePitch;
    s16 left;
    s16 right;

    if (SOUND_SYSTEM.paused != paused) {
        SOUND_SYSTEM.paused = paused;
        if (!paused) {
            resumeVoices(-1);
            return;
        }
        pitch = SOUND_SYSTEM.pausedPitch;
        volume = SOUND_SYSTEM.pausedVolume;
        voices = getSoundEffectVoices();
        attr.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_VOLMODEL | SPU_VOICE_VOLMODER | SPU_VOICE_PITCH;
        for (i = 0; i < SND_MAX_VOICES; i++) {
            attr.voice = 1 << i;
            if (voices & attr.voice) {
                func_80048BF0(&attr);
                voicePitch = attr.pitch;
                left = attr.volume.left;
                right = attr.volume.right;
                attr.pitch = 0;
                attr.volume.left = 0;
                attr.volume.right = 0;
                attr.volmode.left = 0;
                attr.volmode.right = 0;
                SpuSetVoiceAttr(&attr);
            } else {
                voicePitch = -1;
                left = 0;
                right = 0;
            }
            pitch[i] = voicePitch;
            volume[i].left = left;
            volume[i].right = right;
        }
    }
}

/* Gives the paused voices among voices their pitch back. Only resuming all of
 * them (voices -1) gives back their volume too; the others stay silent. */
void resumeVoices(s32 voices) {
    SpuVoiceAttr attr;
    s32 keepVolume = voices >> 31;
    s32 i;
    s16 *pitch;
    SpuVolume *volume;
    s16 voicePitch;
    s32 left;
    s32 right;
    u32 bit;

    attr.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR | SPU_VOICE_VOLMODEL | SPU_VOICE_VOLMODER | SPU_VOICE_PITCH;
    attr.volmode.left = 0;
    attr.volmode.right = 0;
    pitch = SOUND_SYSTEM.pausedPitch;
    volume = SOUND_SYSTEM.pausedVolume;
    for (i = 0; i < SND_MAX_VOICES; i++) {
        bit = 1 << i;
        if (voices & bit) {
            voicePitch = pitch[i];
            left = volume[i].left & keepVolume;
            right = volume[i].right & keepVolume;
            pitch[i] = -1;
            if (voicePitch > 0) {
                attr.volume.right = right;
                attr.voice = bit;
                attr.pitch = voicePitch;
                attr.volume.left = left;
                SpuSetVoiceAttr(&attr);
            }
        }
    }
}

/* the voices of every playing sound effect, as bits */
u32 getSoundEffectVoices(void) {
    SndPlayingEffect *effect;
    u32 voiceBits = 0;

    for (effect = (SndPlayingEffect *)SOUND_SYSTEM.effectPool.used.head; effect != NULL;
         effect = (SndPlayingEffect *)effect->link.next) {
        voiceBits |= effect->voiceBits;
    }
    return voiceBits;
}
