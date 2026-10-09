#ifndef DTBE_SOUND_SOUND_H
#define DTBE_SOUND_SOUND_H

/* The sound system: the sound definition's VABs, SEPs, effects and songs, on the SPU. */

#include "common.h"
#include <libspu.h>
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"
#include "engine/system/handle_table.h"

EXTERN_C_BEGIN

/* the head of a sound definition file (SND-DEF): where its tables start,
 * as offsets from the head, and how many entries each has */
typedef struct {
    /* 0x00 */ s32 magic[2];
    /* 0x08 */ s32 version;
    /* 0x0C */ s32 vabsOffset;
    /* 0x10 */ s32 sepsOffset;
    /* 0x14 */ s32 effectsOffset;
    /* 0x18 */ s32 songsOffset;
    /* 0x1C */ u16 vabCount;
    /* 0x1E */ u16 sepCount;
    /* 0x20 */ u16 effectCount;
    /* 0x22 */ u16 songCount;
    /* 0x24 */ u16 voiceCount;
} SndDef;

#define SND_DEF_MAGIC0 0x2D444E53  /* "SND-" */
#define SND_DEF_MAGIC1 0x464544    /* "DEF" */
#define SND_DEF_VERSION 0x302E3432 /* "24.0" */

/* a VAB of the definition */
typedef struct {
    /* 0x0 */ u16 name; /* the offset of its name from the definition */
    /* 0x2 */ u8 unk2[4];
    /* 0x6 */ s16 id;   /* its VAB id while open, else negative */
} SndVab;

/* a SEP (a set of songs) of the definition */
typedef struct {
    /* 0x0 */ u16 name; /* the offset of its name from the definition */
    /* 0x2 */ u16 vab;  /* the VAB its songs play */
    /* 0x4 */ s16 seqCount;
    /* 0x6 */ s16 id; /* its SEP access number while open, else negative */
} SndSep;

/* a sound effect of the definition: a tone of a VAB */
typedef struct {
    /* 0x0 */ u16 vab;
    /* 0x2 */ u8 flags; /* SND_EFFECT_* */
    /* 0x3 */ u8 priority;
    /* 0x4 */ u8 prog;
    /* 0x5 */ s8 tone;
    /* 0x6 */ u8 note;
    /* 0x7 */ u8 fine;
    /* 0x8 */ u8 volumeLeft;
    /* 0x9 */ u8 volumeRight;
    /* 0xA */ u8 group; /* a new effect stops the playing ones of its group */
    /* 0xB */ u8 unkB[5];
} SndEffect;

/* SndEffect.flags: the effect loops */
#define SND_EFFECT_LOOP 1

/* a song of the definition: a sequence of a SEP */
typedef struct {
    /* 0x0 */ u16 sep;
    /* 0x2 */ s16 seq;
    /* 0x4 */ u16 loops;
    /* 0x6 */ u8 volumeLeft;
    /* 0x7 */ u8 volumeRight;
} SndSong;

/* a song that plays */
typedef struct {
    /* 0x00 */ LinkNode link;
    /* 0x08 */ s32 flags;
    /* 0x0C */ u16 song;
    /* 0x0E */ u8 volumeLeft;
    /* 0x0F */ u8 volumeRight;
    /* 0x10 */ s16 fade;     /* its volume, 0 to SND_FADE_FULL */
    /* 0x12 */ s16 fadeStep; /* taken off fade every frame */
} SndPlayingSong;

/* fade is in 4.12 fixed point */
#define SND_FADE_SHIFT 12
#define SND_FADE_FULL (1 << SND_FADE_SHIFT)

/* a sound effect that plays */
typedef struct {
    /* 0x00 */ LinkNode link;
    /* 0x08 */ u32 handle;
    /* 0x0C */ s32 timer;
    /* 0x10 */ u16 effect;
    /* 0x12 */ u8 vabId;
    /* 0x13 */ u8 voice; /* SND_VOICE_*, 0 until keyed on */
    /* 0x14 */ u32 voiceBits;
    /* 0x18 */ u8 priority; /* with SND_PRIORITY_LOOP for a looping effect */
    /* 0x19 */ u8 prog;
    /* 0x1A */ s8 tone;
    /* 0x1B */ u8 note;
    /* 0x1C */ u8 fine;
    /* 0x1D */ u8 volumeLeft;
    /* 0x1E */ u8 volumeRight;
    /* 0x1F */ u8 group;
} SndPlayingEffect;

/* SndPlayingEffect.priority: set for a looping effect */
#define SND_PRIORITY_LOOP 0x80
#define SND_VOICE_NUMBER 0x1F /* the voice it plays on */
#define SND_VOICE_NONE 0x20   /* no voice was free */
#define SND_VOICE_KEYED 0x80

/* the SPU's voices: how many effects can play at once */
#define SND_VOICE_MAX 24

/* the SPU's full volume */
#define SPU_VOLUME_MAX 0x3FFF

/* frames from turning reverb on to setting its depth: 3 seconds */
#define REVERB_DEPTH_DELAY 180

/* a node of the pool at SoundSystem 0x358 */
typedef struct {
    /* 0x0 */ LinkNode link;
    /* 0x8 */ u8 index; /* its place in the pool */
    /* 0x9 */ u8 unk9[3];
} SndUnk370;

/* the sound system */
typedef struct SoundSystem {
    /* 0x000 */ s32 paused;
    /* 0x004 */ SndDef *def;
    /* 0x008 */ SndVab *vabs;
    /* 0x00C */ SndSep *seps;
    /* 0x010 */ SndEffect *effects;
    /* 0x014 */ SndSong *songs;
    /* 0x018 */ u16 vabCount;
    /* 0x01A */ u16 sepCount;
    /* 0x01C */ u16 effectCount;
    /* 0x01E */ u16 songCount;
    /* 0x020 */ u16 voiceCount;
    /* 0x024 */ NodePool songPool;
    /* 0x038 */ void *songMemory; /* the song nodes, then libsnd's SEQ table */
    /* 0x03C */ NodePool effectPool;
    /* 0x050 */ SndPlayingEffect effectNodes[SND_VOICE_MAX];
    /* 0x350 */ s16 reverbDepthLeft;
    /* 0x352 */ s16 reverbDepthRight;
    /* 0x354 */ s16 reverbDelay; /* frames until the depth is set */
    /* 0x356 */ u16 reverbStart; /* VSync count when the delay began */
    /* 0x358 */ NodePool unk358;
    /* 0x36C */ s32 unk36C;
    /* 0x370 */ SndUnk370 unk370[SND_VOICE_MAX];
    /* 0x490 */ s16 pausedPitch[SND_VOICE_MAX];
    /* 0x4C0 */ SpuVolume pausedVolume[SND_VOICE_MAX];
    /* 0x520 */ u8 nextGroup; /* the group of the next effect, when not 0 */
} SoundSystem;

/* how many handles SOUND_EFFECT_HANDLES holds */
#define SND_HANDLE_CAPACITY 0x30

/* the handles of the playing sound effects */
extern HandleTable SOUND_EFFECT_HANDLES;

extern struct SoundSystem SOUND_SYSTEM; /* the sound system */

s32 setReverb(s32 type, s16 depthLeft, s16 depthRight);
u16 getSongVab(u16 song);
u16 getSongSep(u16 song);
u16 getSoundEffectVab(u16 effect);
char *getVabName(u16 vab);
char *getSepName(u16 sep);
u16 getSoundEffectCount(void);
u16 getSongCount(void);
void startSoundSystem(void *memory, s16 sepMax, s16 seqMax);
s32 getSongMemorySize(s16 sepMax, s16 seqMax);
void func_80038D44(void);
s32 setSndDef(SndDef *def);
void updateSoundEffects(void);
void updateSongFades(void);
void updateSoundSystem(void);
void sndPlayingEffectKeyOn(SndPlayingEffect *sndPlayingEffect);
void closeSep(u16 sepIndex);
void fadeSongs(s16 fadeStep);
s16 openSep(u16 sepIndex, void *data);
void playSong(u16 song);
void playSongAtVolume(u16 song, u8 volumeLeft, u8 volumeRight);
void freeEndedSongs(void);
void stopAllSongs(void);
void sndPlayingSongStop(SndPlayingSong *sndPlayingSong);
void closeVab(u16 vabIndex);
void setNextSoundEffectGroup(u8 group);
s16 openVabHead(u16 vabIndex, void *header);
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

#endif /* DTBE_SOUND_SOUND_H */
