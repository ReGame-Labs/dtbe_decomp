#ifndef DTBE_CD_XA_PLAYER_H
#define DTBE_CD_XA_PLAYER_H

/* CDFSXA, the XA player: plays the disc's XA audio tracks, taking the drive from CDFS. */

#include "common.h"
#include "engine/cd/read.h"

EXTERN_C_BEGIN

/*
 * CDFSXA: plays XA audio tracks from the disc. While it is on it owns the
 * drive; CDFS turns it off (through Cdfs.hook) before reading.
 */

/* A track: its sectors, its channel and the read mode it plays in. */
typedef struct {
    /* 0x0 */ s32 start;
    /* 0x4 */ s32 end;
    /* 0x8 */ CdlFILTER filter;
    /* 0xC */ u8 mode;
} XaTrack;

/* the head of an XAP file, the list of the tracks of an XA file */
typedef struct {
    /* 0x0 */ s32 magic; /* XAP_MAGIC */
    /* 0x4 */ u16 interleave; /* XAP_INTERLEAVE_DOUBLE for double speed tracks */
    /* 0x6 */ u16 count;
} XapHeader;

#define XAP_MAGIC 0x31504158 /* "XAP1" */
#define XAP_INTERLEAVE_DOUBLE 8
/* the tracks follow the head in the next sector */
#define XAP_TRACKS_OFFSET CD_SECTOR_SIZE

#define XA_MODE (CdlModeRT | CdlModeSize1 | CdlModeSF)

/* the start of a sector as CdGetSector reads it with XA_MODE */
typedef struct {
    /* 0x0 */ CdlLOC pos;
    /* 0x4 */ u8 file;
    /* 0x5 */ u8 channel;
    /* 0x6 */ u8 submode; /* XA_SUBMODE_* */
    /* 0x7 */ u8 coding;
} XaSectorHeader;

/* a data sector of the channel ends its track */
#define XA_SUBMODE_DATA 0x08

/* The track playing: where it started and when, to tell where it is. */
typedef struct XaStream {
    /* 0x00 */ CdlLOC pos;   /* where the read starts or resumes */
    /* 0x04 */ CdlLOC start; /* the track's first sector */
    /* 0x08 */ XaTrack *track;
    /* 0x0C */ s32 startTime;   /* the VSync count when the read started */
    /* 0x10 */ s32 startSector; /* where the read started */
    /* 0x14 */ s32 frameRate;   /* VSyncs per second */
} XaStream;

/* XA sectors per second at single speed */
#define XA_SECTORS_PER_SECOND 75

/* XaPlayer.flags */
#define XA_ON 0x1
#define XA_SEEKED 0x2   /* the drive is at the track */
#define XA_STARTING 0x4 /* the read was sent */
#define XA_PLAYING 0x8
#define XA_PAUSED 0x10
#define XA_START_PENDING 0x1000 /* start once the seek ends */
#define XA_LOOP 0x4000          /* the track ended and plays again */
#define XA_END 0x8000           /* the track ended for good */

/* the events of XaPlayer.callback */
#define XA_EVENT_SEEKED 0
#define XA_EVENT_STARTED 1
#define XA_EVENT_PAUSED 2
#define XA_EVENT_RESUMED 3
#define XA_EVENT_STOPPED 4
#define XA_EVENT_LOOPED 5

/* XaPlayer.mixMode */
#define XA_STEREO 0
#define XA_MONO 1

#define XA_VOLUME_MAX 0x7F
#define XA_VOLUME_ON 0x80

typedef void (*XaCallback)(s32 event);

typedef struct XaPlayer {
    /* 0x00 */ volatile u32 flags; /* XA_*, also set by the CD-ROM callbacks */
    /* 0x04 */ void (*cdfsHook)(void); /* the hook CDFS had before */
    /* 0x08 */ u32 prevFlags; /* the flags at the last update */
    /* 0x0C */ u16 loopCount; /* how many times a track plays again */
    /* 0x0E */ u16 loopsLeft;
    /* 0x10 */ u16 volume;
    /* 0x12 */ u16 trackCount;
    /* 0x14 */ XaTrack *tracks;
    /* 0x18 */ u8 unk18[4];
    /* 0x1C */ s32 mixMode; /* XA_STEREO or XA_MONO */
    /* 0x20 */ XaCallback callback;
} XaPlayer;

/* a log message, a rodata file of its own: two functions use it */
extern char STR_CDFSXA_SYSTEM_OFF[]; /* "CDFSXA: System Off.\n" */

extern struct XaStream XA_STREAM; /* the XA track playing */
extern struct XaPlayer XA_PLAYER; /* the XA player */

/* the XA audio tracks, listed by /a.xap */
extern XaTrack XA_TRACKS[];

s32 setCdfsUser(s32 user);
void seekXaTrack(u32 track);
void receiveXaSector(u8 intr);
void updateXa(void);
void initXa(void);
void pauseXa(void);
void playXa(void);
void stopXa(void);
void turnXaOff(void);
void receiveXaSeek(u8 intr);
void startXaRead(void);
void receiveXaReadComplete(u8 intr);
void receiveXaRestart(u8 intr);
s32 stopXaDrive(void);
void estimateXaPosition(void);
s32 isXaSeeking(void);
s32 isXaPlaying(void);
XaCallback setXaCallback(XaCallback callback);
void ignoreXaEvent(s32 event);
s32 setXaMixMode(s32 mixMode);
s32 setXaMono(void);
s32 setXaStereo(void);
void setXaVolume(u32 volume);
u16 setXaLoops(u16 loops);
void waitXaSeek(void);
u16 loadXaTracks(XaTrack *tracks, char *name);
void setXaTracks(XaTrack *tracks, u16 count);

EXTERN_C_END

#endif /* DTBE_CD_XA_PLAYER_H */
