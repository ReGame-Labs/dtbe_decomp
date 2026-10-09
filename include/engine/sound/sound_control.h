#ifndef DTBE_SOUND_SOUND_CONTROL_H
#define DTBE_SOUND_SOUND_CONTROL_H

/* The sound task: the VABs and sequences loaded from /sound, songs, XA music, fades. */

#include "common.h"
#include "vtable.h"
#include "engine/lib/list.h"
#include "engine/math/lerp.h"

EXTERN_C_BEGIN

typedef struct SoundFileVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry setData; /* (SoundFile *, void *data) */
    /* 0x10 */ VtableEntry destroy; /* (SoundFile *, s32 flags) */
} SoundFileVtable;

/* A sound file loaded from /sound, kept in one of the lists of SoundControl:
 * a VAB (the .vh header and the .vb samples) or a .sep sequence. */
typedef struct SoundFile {
    /* 0x00 */ ListNode link;
    /* 0x08 */ s32 id;
    /* 0x0C */ void *data;
    /* 0x10 */ s32 resident; /* never unloaded to make room */
    /* 0x14 */ SoundFileVtable *vtable;
} SoundFile;

/* the most files of each kind SoundControl keeps loaded */
#define SOUND_MAX_VABS 16
#define SOUND_MAX_SEPS 8

/* the bit of a sound id that says it is a song; the rest is its number */
#define SOUND_ID_SONG 0x8000
#define SOUND_ID_NUMBER 0x7FFF

/* the TaskVtable (engine/task/task.h) of SoundControl */
typedef struct SoundControlVtable {
    /* 0x00 */ VtableEntry unused;
    /* 0x08 */ VtableEntry destroy;  /* soundControlDestroy */
    /* 0x10 */ VtableEntry update;   /* soundControlUpdate */
    /* 0x18 */ VtableEntry survives; /* taskSurvives */
} SoundControlVtable;

/* The sound task: the loaded VABs and sequences and the master volume. It is
 * a Task (engine/task/task.h) whose virtual table is a SoundControlVtable, so
 * the Task's fields before the virtual table pointer are only bytes here. */
typedef struct SoundControl {
    /* 0x00 */ u8 task[0x1C]; /* the Task fields before its vtable */
    /* 0x1C */ SoundControlVtable *vtable;
    /* 0x20 */ void *sdf; /* /sound/digimon.sdf */
    /* 0x24 */ s32 unk24;
    /* 0x28 */ s32 trackCount; /* of the XA audio */
    /* 0x2C */ ListNode vabs;
    /* 0x34 */ ListNode seps;
    /* 0x3C */ Lerp volume;
    /* 0x50 */ s32 unk50;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 frame;
    /* 0x5C */ s32 pauseCount;
} SoundControl;

/* SoundControl.volume: the full volume, and the most a fade goes to */
#define SOUND_VOLUME_FULL 0x7F
#define SOUND_VOLUME_NORMAL 0x50

/* the virtual tables of the VAB, the .sep and the base SoundFile */
extern struct SoundFileVtable VAB_FILE_VTABLE;
extern struct SoundFileVtable SEP_FILE_VTABLE;
extern struct SoundFileVtable SOUND_FILE_VTABLE;
extern struct SoundControlVtable SOUND_CONTROL_VTABLE;

extern struct SoundControl SOUND_CONTROL;

SoundFile *soundFileInit(SoundFile *soundFile, s32 id);
void soundFileDestroy(SoundFile *soundFile, s32 flags);
void soundFileSetResident(SoundFile *soundFile);
void soundFileLoad(SoundFile *file, char *name, char *ext);
void finishSoundFileRead(void *data, u32 size, char *name, s32 user); /* a CdfsDoneFunc */
SoundFile *soundFileInitSep(SoundFile *soundFile, s32 id);
void soundFileDestroySep(SoundFile *soundFile, s32 flags);
void soundFileSetSepData(SoundFile *soundFile, void *data);
SoundFile *soundFileInitVab(SoundFile *soundFile, s32 id);
void soundFileDestroyVab(SoundFile *soundFile, s32 flags);
void soundFileSetVabData(SoundFile *soundFile, void *data);
ListNode *listInit(ListNode *listNode);
void listUnloadFiles(ListNode *list);
void listPushBack(ListNode *list, ListNode *node);
s32 listCount(ListNode *list);
SoundFile *listFindFile(ListNode *list, s32 id);
void listUnloadFile(ListNode *list, s32 id);
SoundControl *soundControlInit(SoundControl *soundControl);
void soundControlStart(SoundControl *soundControl);
SoundFile *soundControlLoadVab(SoundControl *soundControl, s32 id);
SoundFile *soundControlLoadSep(SoundControl *soundControl, s32 id);
void soundControlUnloadVabs(SoundControl *soundControl);
void soundControlUnloadSeps(SoundControl *soundControl);
void soundControlUpdate(SoundControl *soundControl);
void soundControlLoadSound(SoundControl *soundControl, s32 id, s32 wait);
s32 isSoundLoaded(void);
void soundControlUnloadSound(SoundControl *soundControl, s32 id);
s32 soundControlPlaySound(SoundControl *soundControl, s32 id, u8 group);
void func_8001BEC8(SoundControl *soundControl, s32 arg1);
void stopSound(u32 handle);
u32 isSoundPlaying(u32 handle);
void stopSongs(void);
void soundControlLoadSong(SoundControl *soundControl, s32 id, s32 wait);
void soundControlLoadSoundEffect(SoundControl *soundControl, s32 id, s32 wait);
void soundControlLoadAndPlaySong(SoundControl *soundControl, s32 id);
s32 soundControlLoadAndPlaySoundEffect(SoundControl *soundControl, s32 id);

void seekMusicTrack(u32 track, s32 loops);
void waitMusicSeek(void);
void soundControlResumeMusic(SoundControl *soundControl);
void soundControlResumeMusicFull(SoundControl *soundControl);
void pauseMusic(void);
void stopMusic(void);
void soundControlFadeIn(SoundControl *soundControl, s32 duration);
void soundControlFadeOut(SoundControl *soundControl, s32 duration);
void soundControlFadeTo(SoundControl *soundControl, s32 volume, s32 duration);
void soundControlPause(SoundControl *soundControl, s32 pause);
void soundControlCountPause(SoundControl *soundControl, s32 pause);
void soundControlDestroy(SoundControl *soundControl, s32 flags);
s32 soundControlGetTrackCount(SoundControl *soundControl);

EXTERN_C_END

#endif /* DTBE_SOUND_SOUND_CONTROL_H */
