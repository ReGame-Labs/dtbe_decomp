#include "common.h"
#include "engine/sound/sound_control.h"
#include "engine/cd/file_load.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/lib/list.h"
#include "engine/math/lerp.h"
#include "engine/math/shuffle.h"
#include "engine/sound/sound.h"
#include "engine/sound/voice_pause.h"
#include "engine/system/memory.h"
#include "engine/task/task.h"
#include "vtable.h"
#include "libsnd.h"
#include "memory.h"
#include "psyq.h"
#include "stdio.h"
#include "strings.h"
#include "overlay.h"

/* Builds a SoundFile with nothing loaded. */
SoundFile *soundFileInit(SoundFile *this, s32 id) {
    this->link.next = &this->link;
    this->link.prev = &this->link;
    this->vtable = &SOUND_FILE_VTABLE;
    this->id = id;
    this->data = NULL;
    this->resident = 0;
    return this;
}

/* Destroys a SoundFile: frees its data and takes it out of its list. */
void soundFileDestroy(SoundFile *this, s32 flags) {
    this->vtable = &SOUND_FILE_VTABLE;
    mainHeapFree(this->data);
    this->link.next->prev = this->link.prev;
    this->link.prev->next = this->link.next;
    this->link.next = &this->link;
    this->link.prev = &this->link;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Keeps the file loaded for good. */
void soundFileSetResident(SoundFile *this) {
    this->resident = 1;
}

/* Loads /sound/<name><ext> into file, through the file's setData. */
void soundFileLoad(SoundFile *file, char *name, char *ext) {
    char path[128];
    s32 size;
    void *buffer;

    sprintf(path, "%s/%s%s", "/sound", name, ext);
    size = getFileSize(path);
    buffer = mainHeapAllocLargest(size);
    turnXaOff();
    setCdfsDone(finishSoundFileRead);
    /* the read hands the file to finishSoundFileRead as its user word */
    setCdfsUser((s32)file);
    queueFileLoad(buffer, path, 0, size);
    setCdfsDone(NULL);
    setCdfsUser(0);
}

/* The end of a sound file's read: inflates the data when it is compressed,
 * or else copies it out of the read buffer, frees the buffer and hands the
 * data to the file's setData. */
void finishSoundFileRead(void *data, u32 size, char *name, s32 user) {
    /* soundFileLoad passed the file as the read's user word */
    SoundFile *file = (SoundFile *)user;
    void *inflated;
    void *copy;

    if (isFileCompressed(data)) {
        inflated = decompressFile(data);
        mainHeapFree(data);
        file->vtable->setData.func((u8 *)file + file->vtable->setData.delta, inflated);
        return;
    }
    copy = mainHeapAllocBest(size);
    memcpy(copy, data, size);
    mainHeapFree(data);
    file->vtable->setData.func((u8 *)file + file->vtable->setData.delta, copy);
}

/* Builds the SoundFile of sequence id and loads it. */
SoundFile *soundFileInitSep(SoundFile *this, s32 id) {
    soundFileInit(this, id);
    this->vtable = &SEP_FILE_VTABLE;
    soundFileLoad(this, getSepName(id & 0xFFFF), ".sep");
    return this;
}

/* Destroys the SoundFile of a sequence: closes the sequence first. */
void soundFileDestroySep(SoundFile *this, s32 flags) {
    this->vtable = &SEP_FILE_VTABLE;
    closeSep(this->id);
    soundFileDestroy(this, flags);
}

/* Opens the sequence just loaded. */
void soundFileSetSepData(SoundFile *this, void *data) {
    /* the sound ids are 16 bits */
    openSep((u16)this->id, data);
    this->data = data;
}

/* Builds the SoundFile of VAB id and loads its header and samples. */
SoundFile *soundFileInitVab(SoundFile *this, s32 id) {
    char *name;

    soundFileInit(this, id);
    this->vtable = &VAB_FILE_VTABLE;
    name = getVabName(id & 0xFFFF);
    soundFileLoad(this, name, ".vh");
    soundFileLoad(this, name, ".vb");
    return this;
}

/* Destroys the SoundFile of a VAB: closes the VAB first. */
void soundFileDestroyVab(SoundFile *this, s32 flags) {
    this->vtable = &VAB_FILE_VTABLE;
    closeVab(this->id);
    soundFileDestroy(this, flags);
}

/* Opens the VAB header just loaded, or sends the samples loaded after it to
 * the sound memory. */
void soundFileSetVabData(SoundFile *this, void *data) {
    /* the sound ids are 16 bits */
    if (this->data == NULL) {
        openVabHead((u16)this->id, data);
        this->data = data;
        return;
    }
    transferVabBody((u16)this->id, data);
    mainHeapFree(data);
}

/* Makes this an empty list. */
ListNode *listInit(ListNode *this) {
    this->next = this;
    this->prev = this;
    return this;
}

/* Deletes the files of list that are not resident. */
void listUnloadFiles(ListNode *list) {
    ListNode *node = list->next;
    ListNode *next;
    SoundFile *file;

    if (list != node) {
        do {
            /* the link is the first member of the file */
            file = (SoundFile *)node;
            next = node->next;
            if (!file->resident && file != NULL) {
                file->vtable->destroy.func((u8 *)file + file->vtable->destroy.delta, DESTROY_DELETE);
            }
            node = next;
        } while (list != node);
    }
}

/* Adds node at the end of list. */
void listPushBack(ListNode *list, ListNode *node) {
    node->next = list;
    node->prev = list->prev;
    list->prev->next = node;
    list->prev = node;
}

/* Counts the nodes of list. */
s32 listCount(ListNode *list) {
    ListNode *node = list->next;
    s32 count = 0;

    if (list != node) {
        do {
            node = node->next;
            count++;
        } while (list != node);
    }
    return count;
}

/* Finds the file of list with the id, or NULL. */
SoundFile *listFindFile(ListNode *list, s32 id) {
    ListNode *node = list->next;

    if (list != node) {
        do {
            /* the link is the first member of the file */
            if (((SoundFile *)node)->id == id) {
                return (SoundFile *)node;
            }
            node = node->next;
        } while (list != node);
    }
    return NULL;
}

/* Deletes the file of list with the id unless it is resident. */
void listUnloadFile(ListNode *list, s32 id) {
    SoundFile *file = listFindFile(list, id);

    if (file != NULL && !file->resident) {
        file->vtable->destroy.func((u8 *)file + file->vtable->destroy.delta, DESTROY_DELETE);
    }
}

/* Builds the sound task. */
SoundControl *soundControlInit(SoundControl *this) {
    /* SoundControl starts with a Task (see engine/sound/sound_control.h) */
    taskInit((Task *)this, 0x1000, "Sound Control");
    this->vtable = &SOUND_CONTROL_VTABLE;
    listInit(&this->vabs);
    listInit(&this->seps);
    lerpInit(&this->volume, 80);
    this->unk54 = 0;
    this->pauseCount = 0;
    return this;
}

/* Starts the sound library, the sound definitions and the XA audio, and
 * loads the sound effects every scene uses for good. */
void soundControlStart(SoundControl *this) {
    u32 size = getSongMemorySize(8, 8);
    void *sdf;

    startSoundSystem(operatorVecNew((size >> 2) * 4), 8, 8);
    SsSetTickMode(SS_TICKVSYNC);
    func_8004BEA0();
    setReverb(2, 42, 42);
    func_800479E0(1);
    sdf = loadFile("/sound/digimon.sdf");
    this->sdf = sdf;
    setSndDef(sdf);
    this->unk24 = 0;
    initXa();
    this->trackCount = loadXaTracks(XA_TRACKS, "/a.xap");
    setXaTracks(XA_TRACKS, this->trackCount);
    soundFileSetResident(soundControlLoadVab(this, getSoundEffectVab(15)));
}

/* Loads VAB id, making room if it has to; a negative id unloads VAB -id. */
SoundFile *soundControlLoadVab(SoundControl *this, s32 id) {
    SoundFile *file = NULL;

    if (id < 0) {
        this->pauseCount = 0;
        setSoundEffectsPaused(0);
        listUnloadFile(&this->vabs, -id);
    } else {
        file = listFindFile(&this->vabs, id);
        if (file != NULL) {
            return file;
        }
        if (listCount(&this->vabs) >= SOUND_MAX_VABS) {
            soundControlUnloadVabs(this);
        }
        file = soundFileInitVab(operatorNew(sizeof(SoundFile)), id);
        listPushBack(&this->vabs, &file->link);
    }
    return file;
}

/* Loads sequence id, making room if it has to; a negative id unloads
 * sequence -id. */
SoundFile *soundControlLoadSep(SoundControl *this, s32 id) {
    SoundFile *file = NULL;

    if (id < 0) {
        listUnloadFile(&this->seps, -id);
    } else {
        file = listFindFile(&this->seps, id);
        if (file != NULL) {
            return file;
        }
        if (listCount(&this->seps) >= SOUND_MAX_SEPS) {
            soundControlUnloadSeps(this);
        }
        file = soundFileInitSep(operatorNew(sizeof(SoundFile)), id);
        listPushBack(&this->seps, &file->link);
    }
    return file;
}

/* Unloads the VABs that are not resident. */
void soundControlUnloadVabs(SoundControl *this) {
    this->pauseCount = 0;
    setSoundEffectsPaused(0);
    listUnloadFiles(&this->vabs);
}

/* Unloads the sequences that are not resident. */
void soundControlUnloadSeps(SoundControl *this) {
    listUnloadFiles(&this->seps);
}

/* The sound task's update: applies the volume and every 15 frames halves
 * unk54 down to 16. */
void soundControlUpdate(SoundControl *this) {
    s16 volume;
    s32 level;
    s32 frame;

    setSoundEffectsPaused(this->pauseCount != 0);
    updateSoundSystem();
    volume = lerpUpdate(&this->volume);
    SsSetSerialVol(SS_SERIAL_A, volume, volume);
    level = this->unk54;
    if (level > 16) {
        frame = this->frame++;
        if (frame == frame / 15 * 15) {
            playSoundEffectAtVolume(this->unk50 & 0xFFFF, level & 0xFF, level & 0xFF);
            this->unk54 -= level >> 1;
        }
    }
}

/* Loads the song or the sound effects of id. */
void soundControlLoadSound(SoundControl *this, s32 id, s32 wait) {
    if (id & SOUND_ID_SONG) {
        soundControlLoadSong(this, id & SOUND_ID_NUMBER, wait);
        return;
    }
    soundControlLoadSoundEffect(this, id, wait);
}

/* Whether the sound files asked for are all loaded. */
s32 isSoundLoaded(void) {
    return syncCdfs(-1) == 0;
}

/* Unloads the song or the sound effects of id. */
void soundControlUnloadSound(SoundControl *this, s32 id) {
    if (id & SOUND_ID_SONG) {
        soundControlLoadSong(this, -(id & SOUND_ID_NUMBER), 1);
        return;
    }
    soundControlLoadSoundEffect(this, -id, 1);
}

/* Plays song id, or the sound effects id in the group. */
s32 soundControlPlaySound(SoundControl *this, s32 id, u8 group) {
    if (id & SOUND_ID_SONG) {
        playSong(id & SOUND_ID_NUMBER);
        return 0;
    }
    setNextSoundEffectGroup(group);
    return playSoundEffect(id);
}

/* Sets unk50 and unk54 and restarts the frame count of the update. */
void func_8001BEC8(SoundControl *this, s32 arg1) {
    this->unk50 = arg1;
    this->unk54 = 127;
    this->frame = 0;
}

/* Stops the sound effect of handle. */
void stopSound(u32 handle) {
    stopSoundEffect(handle);
}

/* Whether the sound effect of handle is still playing. */
u32 isSoundPlaying(u32 handle) {
    return (u32)~getSoundEffectVoice(handle) >> 31;
}

/* Stops every song. */
void stopSongs(void) {
    stopAllSongs();
}

/* Loads song id, its VAB and its sequence; a negative id unloads song -id. */
void soundControlLoadSong(SoundControl *this, s32 id, s32 wait) {
    s32 number = id < 0 ? -id : id;
    s32 vab;
    s32 sep;

    if (number < getSongCount()) {
        vab = getSongVab(number & 0xFFFF);
        sep = getSongSep(number & 0xFFFF);
        if (id < 0) {
            soundControlLoadSep(this, -sep);
            soundControlLoadVab(this, -vab);
            return;
        }
        soundControlLoadVab(this, vab);
        soundControlLoadSep(this, sep);
        if (wait) {
            syncCdfs(0);
        }
    }
}

/* Loads the VAB of sound effects id; a negative id unloads it. */
void soundControlLoadSoundEffect(SoundControl *this, s32 id, s32 wait) {
    s32 number = id < 0 ? -id : id;
    s32 vab;

    if (number < getSoundEffectCount()) {
        vab = getSoundEffectVab(number & 0xFFFF);
        if (id < 0) {
            soundControlLoadVab(this, -vab);
            return;
        }
        soundControlLoadVab(this, vab);
        if (wait) {
            syncCdfs(0);
        }
    }
}

/* Loads song id and plays it. */
void soundControlLoadAndPlaySong(SoundControl *this, s32 id) {
    soundControlLoadSong(this, id, 1);
    playSong(id & 0xFFFF);
}

/* Loads the sound effects id and plays them: the handle of the effect played,
 * or -1. */
s32 soundControlLoadAndPlaySoundEffect(SoundControl *this, s32 id) {
    soundControlLoadSoundEffect(this, id, 1);
    return playSoundEffect(id & 0xFFFF);
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/sound/sound_control", VAB_FILE_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/sound/sound_control", SEP_FILE_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/sound/sound_control", SOUND_FILE_VTABLE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/sound/sound_control", SOUND_CONTROL_VTABLE);

/* Seeks to an XA track, to loop it loops times once it plays (the player is
 * turned on; the music starts with soundControlResumeMusic). */
void seekMusicTrack(u32 track, s32 loops) {
    seekXaTrack(track);
    setXaLoops(loops);
}

/* Waits for the XA seek to end. */
void waitMusicSeek(void) {
    waitXaSeek();
}

/* Resumes the music at the normal volume. */
void soundControlResumeMusic(SoundControl *this) {
    soundControlFadeIn(this, 0);
    playXa();
}

/* Resumes the music at the full volume. */
void soundControlResumeMusicFull(SoundControl *this) {
    lerpStart(&this->volume, SOUND_VOLUME_FULL, 0);
    playXa();
}

void pauseMusic(void) {
    pauseXa();
}

void stopMusic(void) {
    stopXa();
}

/* Fades the volume to the normal volume over duration frames. */
void soundControlFadeIn(SoundControl *this, s32 duration) {
    lerpStart(&this->volume, SOUND_VOLUME_NORMAL, duration);
}

/* Fades the music out over duration frames. */
void soundControlFadeOut(SoundControl *this, s32 duration) {
    lerpStart(&this->volume, 0, duration);
}

/* Fades the volume to volume, at most the normal volume, over duration frames. */
void soundControlFadeTo(SoundControl *this, s32 volume, s32 duration) {
    if (volume > SOUND_VOLUME_NORMAL) {
        volume = SOUND_VOLUME_NORMAL;
    }
    lerpStart(&this->volume, volume, duration);
}

/* Pauses the game's sound, or resumes it. */
void soundControlPause(SoundControl *this, s32 pause) {
    soundControlCountPause(this, pause);
    if (pause) {
        pauseMusic();
        soundControlPlaySound(this, 0x14, 0);
        return;
    }
    soundControlResumeMusic(this);
}

/* Counts the pauses: one more, or one fewer. */
void soundControlCountPause(SoundControl *this, s32 pause) {
    if (pause) {
        this->pauseCount++;
        return;
    }
    if (this->pauseCount > 0) {
        this->pauseCount--;
    }
}

/* Destroys the sound task, after emptying its lists. */
void soundControlDestroy(SoundControl *this, s32 flags) {
    ListNode *seps;
    ListNode *vabs;

    /* listRemove(&this->seps) and listRemove(&this->vabs), in the access
     * order the game's compiler used */
    seps = &this->seps;
    this->seps.next->prev = this->seps.prev;
    seps->prev->next = this->seps.next;
    this->seps.next = seps;
    this->seps.prev = seps;
    vabs = &this->vabs;
    this->vabs.next->prev = this->vabs.prev;
    vabs->prev->next = this->vabs.next;
    this->vabs.next = vabs;
    this->vabs.prev = vabs;
    /* SoundControl starts with a Task (see engine/sound/sound_control.h) */
    taskDestroy((Task *)this, flags);
}

/* The number of XA tracks. */
s32 soundControlGetTrackCount(SoundControl *this) {
    return this->trackCount;
}
