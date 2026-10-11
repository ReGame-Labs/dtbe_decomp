#include "common.h"
#include "engine/sound/sound_effect.h"
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
#if VERSION_JP
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
#else
INCLUDE_ASM("main/nonmatchings/sound/sound_effect", allocPlayingEffect);
#endif

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
#if VERSION_JP
void sndPlayingEffectStop(SndPlayingEffect *this) {
    resumeVoices(this->voiceBits);
    if (this->voice & SND_VOICE_KEYED) {
        sndPlayingEffectKeyOffInline(this);
    }
    sndPlayingEffectFreeInline(this);
}
#else
INCLUDE_ASM("main/nonmatchings/sound/sound_effect", sndPlayingEffectStop);
#endif

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
    if (priority == INIT_PRIORITY_DEFAULT) {
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
    initOrDestroySoundEffectHandles(1, INIT_PRIORITY_DEFAULT);
}

/* Destroys the handle table of the playing sound effects: the file's global
 * destructor. */
void destroySoundEffectHandles(void) {
    initOrDestroySoundEffectHandles(0, INIT_PRIORITY_DEFAULT);
}
