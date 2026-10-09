#include "common.h"
#include "engine/sound/sound.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"
#include "engine/lib/node_pool_inline.h"
#include "engine/sound/voice_pause.h"
#include "engine/system/handle_table.h"
#include "libetc.h"
#include "libsnd.h"
#include "memory.h"
#include "psyq.h"
#include "vtable.h"

/* Sets the reverb type; any type but 0 turns reverb on and sets its depth
 * REVERB_DEPTH_DELAY frames later. 0 on success, else negative. */
s32 setReverb(s32 type, s16 depthLeft, s16 depthRight) {
    if (SsUtSetReverbType(type) == -1) {
        return -1;
    }
    if (type == SS_REV_TYPE_OFF) {
        SOUND_SYSTEM.reverbDelay = 0;
        SsUtReverbOff();
        if (func_80047CF0(-1) == 1) {
            func_800479E0(SPU_OFF);
        }
        return 0;
    }
    if (func_80047CF0(-2) == 1) {
        func_800479E0(SPU_ON);
        SsUtReverbOn();
        SOUND_SYSTEM.reverbDelay = REVERB_DEPTH_DELAY;
        SOUND_SYSTEM.reverbDepthLeft = depthLeft;
        SOUND_SYSTEM.reverbDepthRight = depthRight;
        SOUND_SYSTEM.reverbStart = VSync(-1);
        return 0;
    }
    return -2;
}

/* the VAB a song plays */
u16 getSongVab(u16 song) {
    return SOUND_SYSTEM.seps[SOUND_SYSTEM.songs[song].sep].vab;
}

/* the SEP of a song */
u16 getSongSep(u16 song) {
    return SOUND_SYSTEM.songs[song].sep;
}

/* the VAB of a sound effect */
u16 getSoundEffectVab(u16 effect) {
    return SOUND_SYSTEM.effects[effect].vab;
}

/* the name of a VAB */
char *getVabName(u16 vab) {
    return (char *)SOUND_SYSTEM.def + SOUND_SYSTEM.vabs[vab].name;
}

/* the name of a SEP */
char *getSepName(u16 sep) {
    return (char *)SOUND_SYSTEM.def + SOUND_SYSTEM.seps[sep].name;
}

/* the number of sound effects of the definition */
u16 getSoundEffectCount(void) {
    return SOUND_SYSTEM.effectCount;
}

/* the number of songs of the definition */
u16 getSongCount(void) {
    return SOUND_SYSTEM.songCount;
}

/* Starts the sound system. memory holds the nodes of sepMax * seqMax songs
 * and libsnd's table of as many sequences (getSongMemorySize bytes). */
void startSoundSystem(void *memory, s16 sepMax, s16 seqMax) {
    SpuCommonAttr attr;
    s32 songMax = sepMax * seqMax;

    bzero((u8 *)&SOUND_SYSTEM, sizeof(SoundSystem)); /* bzero takes bytes */
    SsSetMVol(0, 0);
    SsInit();
    SOUND_SYSTEM.songMemory = memory;
    nodePoolInit(&SOUND_SYSTEM.songPool, memory, sizeof(SndPlayingSong), songMax);
    SsSetTableSize((char *)SOUND_SYSTEM.songMemory + songMax * sizeof(SndPlayingSong), sepMax, seqMax);
    nodePoolInit(&SOUND_SYSTEM.effectPool, SOUND_SYSTEM.effectNodes, sizeof(SndPlayingEffect), SND_VOICE_MAX);
    func_80038D44();
    func_80052AB0(SND_VOICE_MAX);
    SsUtSetReverbType(SS_REV_TYPE_OFF);
    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    attr.mask = SPU_COMMON_MVOLL | SPU_COMMON_MVOLR;
    attr.mvol.left = SPU_VOLUME_MAX;
    attr.mvol.right = SPU_VOLUME_MAX;
    SpuSetCommonAttr(&attr);
}

/* the memory startSoundSystem needs for sepMax * seqMax songs */
s32 getSongMemorySize(s16 sepMax, s16 seqMax) {
    /* a song node and its entry in libsnd's SEQ table */
    s32 songSize = sizeof(SndPlayingSong) + SS_SEQ_TABSIZ;

    return sepMax * songSize * seqMax;
}

/* Sets up the pool at 0x358: SND_VOICE_MAX nodes, numbered from 0. Nothing
 * in the executable takes a node from it. */
void func_80038D44(void) {
    u32 i;

    nodePoolInit(&SOUND_SYSTEM.unk358, SOUND_SYSTEM.unk370, sizeof(SndUnk370), SND_VOICE_MAX);
    for (i = 0; i < SND_VOICE_MAX; i++) {
        SOUND_SYSTEM.unk370[i].index = i;
    }
}

/* Takes the tables of a sound definition file: 0, or -1 when it is not one.
 * A voice count of 0 or above SND_VOICE_MAX means all the voices. */
s32 setSndDef(SndDef *def) {
    u16 voiceCount;

    if (def->magic[0] == SND_DEF_MAGIC0 && def->magic[1] == SND_DEF_MAGIC1) {
        if (def->version != SND_DEF_VERSION) {
            return -1;
        }
        SOUND_SYSTEM.def = def;
        SOUND_SYSTEM.vabs = (SndVab *)((u8 *)def + def->vabsOffset);
        SOUND_SYSTEM.seps = (SndSep *)((u8 *)def + def->sepsOffset);
        SOUND_SYSTEM.effects = (SndEffect *)((u8 *)def + def->effectsOffset);
        SOUND_SYSTEM.songs = (SndSong *)((u8 *)def + def->songsOffset);
        SOUND_SYSTEM.vabCount = def->vabCount;
        SOUND_SYSTEM.sepCount = def->sepCount;
        SOUND_SYSTEM.effectCount = def->effectCount;
        SOUND_SYSTEM.songCount = def->songCount;
        voiceCount = def->voiceCount;
        SOUND_SYSTEM.voiceCount = voiceCount;
        if (voiceCount == 0 || voiceCount > SND_VOICE_MAX) {
            SOUND_SYSTEM.voiceCount = SND_VOICE_MAX;
        }
        return 0;
    }
    return -1;
}

/* Frees the sound effects that ended: a looping one when its timer runs out
 * or its voice went quiet, any other once its voice is keyed off. Then keys
 * on the new ones, and frees those that got no voice and do not loop. */
void updateSoundEffects(void) {
    u8 keys[SND_VOICE_MAX];
    /* libspu takes it as a short, but compares it unsigned */
    u16 envelope;
    SndPlayingEffect *effect;
    SndPlayingEffect *next;
    s32 voice;
    LinkList *used;

    func_80047FC4(keys);
    used = &SOUND_SYSTEM.effectPool.used;
    for (effect = (SndPlayingEffect *)used->head; effect != NULL; effect = next) {
        next = (SndPlayingEffect *)effect->link.next;
        voice = effect->voice & SND_VOICE_NUMBER;
        if (effect->voice != 0) {
            SpuGetVoiceEnvelope(voice, (s16 *)&envelope);
            if (effect->priority & SND_PRIORITY_LOOP) {
                if (effect->timer-- == 0) {
                    sndPlayingEffectStop(effect);
                } else if (keys[voice] != SPU_ON && envelope == 0) {
                    sndPlayingEffectFree(effect);
                }
            } else if (keys[voice] != SPU_ON) {
                sndPlayingEffectFree(effect);
            }
        }
    }
    used = &SOUND_SYSTEM.effectPool.used;
    for (effect = (SndPlayingEffect *)used->head; effect != NULL; effect = next) {
        next = (SndPlayingEffect *)effect->link.next;
        if (effect->voice == 0) {
            sndPlayingEffectKeyOn(effect);
            if (!(effect->priority & SND_PRIORITY_LOOP) && (effect->voice & SND_VOICE_NONE)) {
                sndPlayingEffectFree(effect);
            }
        }
    }
}

/* Moves the fading songs a frame on; a song that fades out stops. */
void updateSongFades(void) {
    SndPlayingSong *song;
    SndPlayingSong *next;
    SndSong *def;
    SndSep *sep;
    NodePool *pool = &SOUND_SYSTEM.songPool;
    s32 volumeLeft;
    s32 volumeRight;

    for (song = (SndPlayingSong *)pool->used.head; song != NULL; song = next) {
        next = (SndPlayingSong *)song->link.next;
        if (song->fadeStep != 0) {
            song->fade -= song->fadeStep;
            if (song->fade <= 0) {
                song->fade = 0;
                song->fadeStep = 0;
            } else if (song->fade >= SND_FADE_FULL) {
                song->fade = SND_FADE_FULL;
                song->fadeStep = 0;
            }
            volumeLeft = (song->volumeLeft * song->fade) >> SND_FADE_SHIFT;
            volumeRight = (song->volumeRight * song->fade) >> SND_FADE_SHIFT;
            def = &SOUND_SYSTEM.songs[song->song];
            sep = &SOUND_SYSTEM.seps[def->sep];
            if ((volumeLeft | volumeRight) == 0) {
                SsSepStop(sep->id, def->seq);
                nodePoolReleaseInline(&SOUND_SYSTEM.songPool, &song->link);
            } else {
                SsSepSetVol(sep->id, def->seq, volumeLeft, volumeRight);
            }
        }
    }
}

/* The sound system's work for a frame. */
void updateSoundSystem(void) {
    updateSoundEffects();
    updateSongFades();
    if (SOUND_SYSTEM.reverbDelay != 0 && (u16)(VSync(-1) - SOUND_SYSTEM.reverbStart) >= SOUND_SYSTEM.reverbDelay) {
        SOUND_SYSTEM.reverbDelay = 0;
        SsUtSetReverbDepth(SOUND_SYSTEM.reverbDepthLeft, SOUND_SYSTEM.reverbDepthRight);
    }
}

/* Keys on the voice of a sound effect. */
void sndPlayingEffectKeyOn(SndPlayingEffect *this) {
    s32 keyed;
    u8 voice;
    u32 voiceBits;
    s8 tone = this->tone;

    voice = SND_VOICE_NONE;
    if (tone >= 0) {
        keyed = SsUtKeyOn(this->vabId, this->prog, tone, this->note, this->fine, this->volumeLeft,
                          this->volumeRight);
        voice = keyed;
        voiceBits = 1 << keyed;
    } else {
        voiceBits = func_8004D4F0((this->vabId << 8) | this->prog, (this->note << 8) | this->fine,
                                  this->volumeLeft, this->volumeRight);
    }
    this->voice = voice | SND_VOICE_KEYED;
    this->voiceBits = voiceBits;
}

/* Stops the songs of a SEP and closes it. */
void closeSep(u16 sepIndex) {
    SndSep *sep = &SOUND_SYSTEM.seps[sepIndex];
    SndPlayingSong *song;
    SndPlayingSong *next;
    SndSong *def;

    if (SOUND_SYSTEM.seps != NULL && sep->id >= 0) {
        for (song = (SndPlayingSong *)SOUND_SYSTEM.songPool.used.head; song != NULL; song = next) {
            def = &SOUND_SYSTEM.songs[song->song];
            next = (SndPlayingSong *)song->link.next;
            if (def->sep == sepIndex) {
                SsSepStop(sep->id, def->seq);
                nodePoolReleaseInline(&SOUND_SYSTEM.songPool, &song->link);
            }
        }
        SsSepClose(sep->id);
        sep->id = -1;
    }
}

/* Starts every playing song fading by fadeStep a frame. */
void fadeSongs(s16 fadeStep) {
    SndPlayingSong *song;

    for (song = (SndPlayingSong *)SOUND_SYSTEM.songPool.used.head; song != NULL;
         song = (SndPlayingSong *)song->link.next) {
        song->fadeStep = fadeStep;
    }
}

/* Opens a SEP from data, once its VAB is open: its access number, or -1. */
s16 openSep(u16 sepIndex, void *data) {
    SndSep *sep = &SOUND_SYSTEM.seps[sepIndex];
    SndVab *vab = &SOUND_SYSTEM.vabs[sep->vab];
    s16 id;

    if (SOUND_SYSTEM.seps == NULL) {
        return -1;
    }
    if (sep->id >= 0) {
        return sep->id;
    }
    if (vab->id < 0) {
        return -1;
    }
    id = SsSepOpen(data, vab->id, sep->seqCount);
    if (id >= 0) {
        sep->id = id;
    }
    return id;
}

/* Plays a song at its own volume. */
void playSong(u16 song) {
    SndSong *def;

    if (song < SOUND_SYSTEM.songCount && SOUND_SYSTEM.songs != NULL) {
        def = &SOUND_SYSTEM.songs[song];
        playSongAtVolume(song, def->volumeLeft, def->volumeRight);
    }
}

/* Plays a song from its start at the volume given, starting it over when it
 * plays already. */
void playSongAtVolume(u16 song, u8 volumeLeft, u8 volumeRight) {
    SndSong *def;
    SndSep *sep;
    SndPlayingSong *playing;
    u16 loops;

    freeEndedSongs();
    if (song < SOUND_SYSTEM.songCount && SOUND_SYSTEM.songs != NULL) {
        def = &SOUND_SYSTEM.songs[song];
        sep = &SOUND_SYSTEM.seps[def->sep];
        if (sep->id >= 0) {
            for (playing = (SndPlayingSong *)SOUND_SYSTEM.songPool.used.head; playing != NULL;
                 playing = (SndPlayingSong *)playing->link.next) {
                if (playing->song == song) {
                    /* it plays already: start it over */
                    SsSepStop(sep->id, def->seq);
                    break;
                }
            }
            if (playing == NULL) {
                playing = (SndPlayingSong *)nodePoolAllocInline(&SOUND_SYSTEM.songPool);
                if (playing == NULL) {
                    return;
                }
                nodePoolPushFrontInline(&SOUND_SYSTEM.songPool, &playing->link);
                playing->song = song;
            }
            playing->flags = 0;
            playing->volumeLeft = volumeLeft;
            playing->volumeRight = volumeRight;
            playing->fade = SND_FADE_FULL;
            playing->fadeStep = 0;
            SsSepSetVol(sep->id, def->seq, volumeLeft, volumeRight);
            if (def->loops == 0) {
                loops = SSPLAY_INFINITY;
            } else {
                loops = def->loops;
            }
            SsSepPlay(sep->id, def->seq, SSPLAY_PLAY, loops);
        }
    }
}

/* Frees the songs that came to their end. */
void freeEndedSongs(void) {
    NodePool *pool = &SOUND_SYSTEM.songPool;
    SndPlayingSong *song;
    SndPlayingSong *next;
    SndSong *def;
    SndSep *sep;

    for (song = (SndPlayingSong *)pool->used.head; song != NULL; song = next) {
        next = (SndPlayingSong *)song->link.next;
        if (!(song->flags & 1)) {
            def = &SOUND_SYSTEM.songs[song->song];
            sep = &SOUND_SYSTEM.seps[def->sep];
            if (func_80049460(sep->id, def->seq) == 0) {
                nodePoolReleaseInline(&SOUND_SYSTEM.songPool, &song->link);
            }
        }
    }
}

/* Stops every playing song. */
void stopAllSongs(void) {
    LinkNode *node;
    LinkNode *next;

    for (node = SOUND_SYSTEM.songPool.used.head; node != NULL; node = next) {
        next = node->next;
        sndPlayingSongStop((SndPlayingSong *)node);
    }
}

/* Stops a song and frees it. */
void sndPlayingSongStop(SndPlayingSong *this) {
    SndSong *def = &SOUND_SYSTEM.songs[this->song];
    SndSep *sep = &SOUND_SYSTEM.seps[def->sep];

    SsSepStop(sep->id, def->seq);
    nodePoolReleaseInline(&SOUND_SYSTEM.songPool, &this->link);
}

/* Closes the SEPs that use a VAB, stops its sound effects and closes it. */
void closeVab(u16 vabIndex) {
    SndVab *vab = &SOUND_SYSTEM.vabs[vabIndex];
    SndPlayingEffect *effect;
    SndPlayingEffect *next;
    LinkList *used;
    s32 i;

    if (SOUND_SYSTEM.vabs != NULL && vab->id >= 0) {
        for (i = 0; i < SOUND_SYSTEM.sepCount; i++) {
            if (SOUND_SYSTEM.seps[i].vab == vabIndex) {
                closeSep(i);
            }
        }
        used = &SOUND_SYSTEM.effectPool.used;
        for (effect = (SndPlayingEffect *)used->head; effect != NULL; effect = next) {
            next = (SndPlayingEffect *)effect->link.next;
            if (effect->vabId == vab->id) {
                sndPlayingEffectKeyOff(effect);
                SsUtKeyOffV(effect->voice & SND_VOICE_NUMBER);
                func_8004F330(effect->voice & SND_VOICE_NUMBER, 0, 0);
                sndPlayingEffectFree(effect);
            }
        }
        SsVabClose(vab->id);
        vab->id = -1;
    }
}

/* Sets the group of the next sound effect. */
void setNextSoundEffectGroup(u8 group) {
    SOUND_SYSTEM.nextGroup = group;
}

/* Opens the head of a VAB: its VAB id, or negative. */
s16 openVabHead(u16 vabIndex, void *header) {
    SndVab *vab = &SOUND_SYSTEM.vabs[vabIndex];
    s16 id;

    if (SOUND_SYSTEM.vabs == NULL) {
        return -1;
    }
    if (vab->id >= 0) {
        return vab->id;
    }
    id = func_80052F10(header, -1);
    if (id >= 0) {
        vab->id = id;
    }
    return id;
}

/* Plays a sound effect at its own volume: its handle, or -1. */
s32 playSoundEffect(u16 effect) {
    SndEffect *def;

    if (effect >= SOUND_SYSTEM.effectCount) {
        return -1;
    }
    if (SOUND_SYSTEM.effects == NULL) {
        return -1;
    }
    def = &SOUND_SYSTEM.effects[effect];
    return playSoundEffectAtVolume(effect, def->volumeLeft, def->volumeRight);
}

/* A node for a new sound effect: a free one, or else the playing effect of
 * the lowest priority when that is not above its own. NULL when there is
 * none or its VAB is not open. */
SndPlayingEffect *allocPlayingEffect(s32 effect, u8 volumeLeft, u8 volumeRight) {
    SndEffect *def = &SOUND_SYSTEM.effects[effect];
    s16 vabId = SOUND_SYSTEM.vabs[def->vab].id;
    SndPlayingEffect *node;
    u8 priority;
    LinkList *used;

    if (vabId < 0) {
        return NULL;
    }
    priority = def->priority & ~SND_PRIORITY_LOOP;
    if (def->flags & SND_EFFECT_LOOP) {
        priority |= SND_PRIORITY_LOOP;
    }
    used = &SOUND_SYSTEM.effectPool.used;
    if (SOUND_SYSTEM.effectPool.used.count == SOUND_SYSTEM.voiceCount) {
        node = (SndPlayingEffect *)SOUND_SYSTEM.effectPool.used.tail;
        if (priority < node->priority) {
            return NULL;
        }
        linkListRemove(used, &node->link);
        freeSoundEffectHandle(node->handle);
    } else {
        node = (SndPlayingEffect *)nodePoolAllocInline(&SOUND_SYSTEM.effectPool);
    }
    node->handle = allocSoundEffectHandle(node);
    node->timer = -1;
    node->effect = effect;
    node->vabId = vabId;
    node->voice = 0;
    node->priority = priority;
    node->prog = def->prog;
    node->tone = def->tone;
    node->note = def->note;
    node->fine = def->fine;
    node->volumeLeft = volumeLeft;
    node->volumeRight = volumeRight;
    if (SOUND_SYSTEM.nextGroup != 0) {
        node->group = SOUND_SYSTEM.nextGroup;
        SOUND_SYSTEM.nextGroup = 0;
    } else {
        node->group = def->group;
    }
    return node;
}

/* Plays a sound effect at the volume given: its handle, or 0 when it does
 * not play. The playing effects are kept from the highest priority down. */
u32 playSoundEffectAtVolume(u16 effect, u8 volumeLeft, u8 volumeRight) {
    SndPlayingEffect *node;
    SndPlayingEffect *at;

    if (effect >= SOUND_SYSTEM.effectCount || SOUND_SYSTEM.effects == NULL || canPlaySoundEffect(effect) == 0) {
        return 0;
    }
    node = allocPlayingEffect(effect, volumeLeft, volumeRight);
    if (node == NULL) {
        return 0;
    }
    at = (SndPlayingEffect *)SOUND_SYSTEM.effectPool.used.head;
    while (at != NULL && at->priority > node->priority) {
        at = (SndPlayingEffect *)at->link.next;
    }
    if (at == NULL) {
        nodePoolPushBackInline(&SOUND_SYSTEM.effectPool, &node->link);
    } else {
        linkListInsertBefore(&SOUND_SYSTEM.effectPool.used, &at->link, &node->link);
    }
    return node->handle;
}

/* Frees a sound effect and its handle. sndPlayingEffectFree is its out-of-line
 * copy; the code after it inlines it, as the game did. */
static inline void sndPlayingEffectFreeInline(SndPlayingEffect *this) {
    freeSoundEffectHandle(this->handle);
    nodePoolReleaseInline(&SOUND_SYSTEM.effectPool, &this->link);
}

/* Frees a sound effect and its handle. */
void sndPlayingEffectFree(SndPlayingEffect *this) {
    sndPlayingEffectFreeInline(this);
}

/* Keys off the voice of a sound effect and silences it. sndPlayingEffectKeyOff is its
 * out-of-line copy; sndPlayingEffectStop inlines it, as the game did. */
static inline void sndPlayingEffectKeyOffInline(SndPlayingEffect *this) {
    SpuVoiceAttr attr;

    SpuSetKey(SPU_OFF, this->voiceBits);
    attr.mask = SPU_VOICE_VOLL | SPU_VOICE_VOLR;
    attr.volume.left = 0;
    attr.volume.right = 0;
    attr.voice = this->voiceBits;
    SpuSetVoiceAttr(&attr);
}

/* Keys off the voice of a sound effect and silences it. */
void sndPlayingEffectKeyOff(SndPlayingEffect *this) {
    sndPlayingEffectKeyOffInline(this);
}

/* Stops a sound effect and frees it. */
void sndPlayingEffectStop(SndPlayingEffect *this) {
    resumeVoices(this->voiceBits);
    if (this->voice & SND_VOICE_KEYED) {
        sndPlayingEffectKeyOffInline(this);
    }
    sndPlayingEffectFreeInline(this);
}

/* Stops the effects of the group of a new sound effect, and tells whether it
 * may play: one that does not loop may not while the same effect still
 * waits for a voice. */
s32 canPlaySoundEffect(u16 effect) {
    SndEffect *def = &SOUND_SYSTEM.effects[effect];
    SndPlayingEffect *playing;
    SndPlayingEffect *next;
    NodePool *pool;

    keyOffSoundEffectGroup(SOUND_SYSTEM.nextGroup != 0 ? SOUND_SYSTEM.nextGroup : def->group);
    if (!(def->flags & SND_EFFECT_LOOP)) {
        pool = &SOUND_SYSTEM.effectPool;
        for (playing = (SndPlayingEffect *)pool->used.head; playing != NULL; playing = next) {
            next = (SndPlayingEffect *)playing->link.next;
            if (playing->effect == effect && playing->voice == 0) {
                return 0;
            }
        }
    }
    return 1;
}

/* Keys off the playing sound effects of group; group 0 is no group. */
void keyOffSoundEffectGroup(s32 group) {
    SndPlayingEffect *effect;
    SndPlayingEffect *next;

    if (group != 0) {
        for (effect = (SndPlayingEffect *)SOUND_SYSTEM.effectPool.used.head; effect != NULL; effect = next) {
            next = (SndPlayingEffect *)effect->link.next;
            if (effect->group == group) {
                sndPlayingEffectKeyOff(effect);
            }
        }
    }
}

/* the voice of the sound effect of handle, or -1 once it stopped */
s32 getSoundEffectVoice(u32 handle) {
    SndPlayingEffect *effect = findPlayingEffect(handle);

    if (effect == NULL) {
        return -1;
    }
    return effect->voice & SND_VOICE_NUMBER;
}

/* Stops the sound effect of handle, if it still plays. */
void stopSoundEffect(u32 handle) {
    SndPlayingEffect *effect = findPlayingEffect(handle);

    if (effect != NULL) {
        sndPlayingEffectStop(effect);
    }
}

/* Transfers the waveforms of a VAB to the SPU and waits for them. */
s32 transferVabBody(u16 vab, u8 *data) {
    s32 result = SsVabTransBody(data, SOUND_SYSTEM.vabs[vab].id);

    SsVabTransCompleted(SS_WAIT_COMPLETED);
    return result;
}

/* Builds (initialize) or destroys the handle table of the playing sound
 * effects: g++'s __static_initialization_and_destruction_0 of this file. */
void initOrDestroySoundEffectHandles(s32 initialize, s32 priority) {
    if (priority == 0xFFFF) {
        if (initialize) {
            handleTableInit(&SOUND_EFFECT_HANDLES, SND_HANDLE_CAPACITY);
            return;
        }
        handleTableDestroy(&SOUND_EFFECT_HANDLES, DESTROY_BASES);
    }
}

/* a new handle for a playing sound effect */
u32 allocSoundEffectHandle(SndPlayingEffect *effect) {
    return handleTableAdd(&SOUND_EFFECT_HANDLES, effect);
}

/* the playing sound effect of handle, or NULL once it stopped */
SndPlayingEffect *findPlayingEffect(u32 handle) {
    return handleTableGet(&SOUND_EFFECT_HANDLES, handle);
}

/* Gives back the handle of a playing sound effect. */
void freeSoundEffectHandle(u32 handle) {
    handleTableRemove(&SOUND_EFFECT_HANDLES, handle);
}

/* Builds the handle table of the playing sound effects: the file's global
 * constructor. */
void initSoundEffectHandles(void) {
    initOrDestroySoundEffectHandles(1, 0xFFFF);
}

/* Destroys the handle table of the playing sound effects: the file's global
 * destructor. */
void destroySoundEffectHandles(void) {
    initOrDestroySoundEffectHandles(0, 0xFFFF);
}
