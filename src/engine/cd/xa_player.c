#include "common.h"
#include "engine/cd/xa_player.h"
#include "engine/cd/read.h"
#include "engine/system/log.h"
#include "libetc.h"
#include "libsnd.h"
#include "memory.h"
#include "psyq.h"

/*
 * Some functions are also copied into their callers (the game was built
 * with inlining): their bodies are the inline functions below, which the
 * functions of the same work call too.
 */

/* Sets the attenuation of the CD audio to the volume, in the mix mode. */
static inline void setXaVolumeInline(u32 volume) {
    CdlATV atv;

    XA_PLAYER.volume = volume;
    switch (XA_PLAYER.mixMode) {
    case XA_STEREO:
        atv.val0 = volume;
        atv.val1 = 0;
        atv.val2 = volume;
        atv.val3 = 0;
        break;
    case XA_MONO:
        /* both channels to both sides, at half the volume */
        volume >>= 1;
        atv.val0 = volume;
        atv.val1 = volume;
        atv.val2 = volume;
        atv.val3 = volume;
        break;
    }
    CdMix(&atv);
}

/* Sets the mix mode, keeping the volume; returns the old mode. */
static inline s32 setXaMixModeInline(s32 mixMode) {
    s32 old = XA_PLAYER.mixMode;

    XA_PLAYER.mixMode = mixMode;
    setXaVolumeInline(XA_PLAYER.volume);
    return old;
}

/* Reads the track from the stream's position. */
static inline void startXaReadInline(void) {
    XA_PLAYER.flags |= XA_STARTING;
    XA_STREAM.startSector = func_80046C70(&XA_STREAM.position);
    func_80043998(XA_STREAM.track->mode, &XA_STREAM.position, CdlReadS, receiveXaReadComplete, -1);
}

/* Stops the track and tells the callback. */
static inline void stopXaInline(void) {
    if (stopXaDrive() != 0) {
        XA_PLAYER.flags &= ~XA_STARTING;
        XA_PLAYER.callback(XA_EVENT_STOPPED);
    }
}

/* Turns the player off and gives the drive back to CDFS. */
static inline void turnXaOffInline(void) {
    XA_PLAYER.cdfsHook();
    if (XA_PLAYER.flags & XA_ON) {
        setXaVolume(0);
        LOG_PRINT(STR_CDFSXA_SYSTEM_OFF);
        func_80046DC0(NULL);
        func_800468A0();
        func_80043724(CdlPause, NULL, NULL, -1);
        enableCdfs();
        XA_PLAYER.flags = XA_SEEKED;
    }
}

/* Sets the user value of the next file reads; returns the old one. */
s32 setCdfsUser(s32 user) {
    s32 old = CDFS.user;

    CDFS.user = user;
    return old;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/cd/xa_player", STR_CDFSXA_SYSTEM_OFF);

/* Turns the player on (taking the drive from CDFS) and seeks to a track. */
void seekXaTrack(u32 track) {
    if (track < XA_PLAYER.trackCount) {
        disableCdfs();
        if (!(XA_PLAYER.flags & XA_ON)) {
            LOG_PRINT("CDFSXA: System On.\n");
        }
        func_80046DC0(NULL);
        func_800468A0();
        XA_PLAYER.flags |= XA_ON;
        XA_PLAYER.flags &= ~(XA_SEEKED | XA_PLAYING | XA_PAUSED | XA_START_PENDING);
        setXaVolume(0);
        XA_STREAM.track = &XA_PLAYER.tracks[track];
        func_80046B60(XA_STREAM.track->start, &XA_STREAM.start);
        XA_STREAM.position = XA_STREAM.start;
        func_80043724(CdlSetmode, &XA_STREAM.track->mode, NULL, -1);
        func_80043724(CdlSetfilter, &XA_STREAM.track->filter, NULL, -1);
        func_80043998(XA_STREAM.track->mode, &XA_STREAM.start, CdlSeekP, receiveXaSeek, -1);
    }
}

/* The data ready callback while playing: ends the track at its first data
 * sector, or restarts the read after an error. */
void receiveXaSector(u8 intr) {
    XaSectorHeader header;

    if (intr != CdlDataReady) {
        func_80046DC0(NULL);
        estimateXaPosition();
        XA_STREAM.startSector = func_80046C70(&XA_STREAM.position);
        func_80043998(XA_STREAM.track->mode, &XA_STREAM.position, CdlReadS, receiveXaRestart, -1);
        return;
    }
    func_80046930(&header, sizeof(header) / 4);
    if ((header.submode & XA_SUBMODE_DATA) && XA_STREAM.track->filter.chan == header.channel) {
        func_80046DC0(NULL);
        if (XA_PLAYER.loopsLeft != 0) {
            if (!(XA_PLAYER.flags & (XA_LOOP | XA_STARTING)) && !(XA_PLAYER.prevFlags & XA_LOOP)) {
                XA_PLAYER.loopsLeft--;
                XA_PLAYER.flags |= XA_LOOP;
            }
        } else {
            XA_PLAYER.flags |= XA_END;
        }
        setXaVolume(0);
    }
}

/* Runs once a frame: tells the callback what changed, loops or stops the
 * ended track, and turns the player off when CDFS has reads waiting. */
void updateXa(void) {
    u32 flags;
    u32 changed;
    u32 rose;
    u32 fell;

    if (XA_PLAYER.flags & XA_ON) {
        flags = XA_PLAYER.flags;
        changed = XA_PLAYER.prevFlags;
        XA_PLAYER.prevFlags = flags;
        changed ^= flags;
        rose = flags & changed;
        fell = ~flags & changed;
        if (rose & XA_SEEKED) {
            XA_PLAYER.callback(XA_EVENT_SEEKED);
        }
        if (rose & XA_PLAYING) {
            if (fell & XA_PAUSED) {
                XA_PLAYER.callback(XA_EVENT_RESUMED);
            } else {
                XA_PLAYER.callback(XA_EVENT_STARTED);
            }
        }
        if (rose & XA_LOOP) {
            XA_PLAYER.flags &= ~XA_LOOP;
            XA_STREAM.position = XA_STREAM.start;
            startXaReadInline();
            XA_PLAYER.callback(XA_EVENT_LOOPED);
        }
        if (rose & XA_END) {
            XA_PLAYER.flags &= ~XA_LOOP;
            stopXaInline();
        } else if (!(flags & XA_PLAYING)) {
            if (syncCdfs(1) != 0) {
                turnXaOffInline();
            } else if ((flags & (XA_START_PENDING | XA_SEEKED)) == (XA_START_PENDING | XA_SEEKED)) {
                startXaReadInline();
                XA_PLAYER.flags &= ~XA_START_PENDING;
            }
        }
    }
}

/* Sets the player up, off, and makes CDFS turn it off before reading. */
void initXa(void) {
    bzero((u8 *)&XA_PLAYER, sizeof(XaPlayer)); /* bzero takes bytes */
    XA_PLAYER.cdfsHook = CDFS.hook;
    CDFS.hook = turnXaOff;
    setXaCallback(NULL);
    setXaVolume(0);
    SsSetSerialVol(SS_SERIAL_A, XA_VOLUME_MAX, XA_VOLUME_MAX);
    XA_PLAYER.flags |= XA_SEEKED;
    XA_STREAM.frameRate = GetVideoMode() == MODE_NTSC ? 60 : 50;
}

/* Pauses the track. */
void pauseXa(void) {
    if (stopXaDrive() != 0) {
        XA_PLAYER.flags |= XA_PAUSED;
        XA_PLAYER.callback(XA_EVENT_PAUSED);
    }
}

/* Plays the track, or resumes it: now if the seek ended, else after it. */
void playXa(void) {
    if ((XA_PLAYER.flags & XA_ON) && !(XA_PLAYER.flags & XA_PLAYING)) {
        if (!(XA_PLAYER.flags & XA_PAUSED)) {
            XA_PLAYER.loopsLeft = XA_PLAYER.loopCount;
        }
        XA_PLAYER.flags &= ~(XA_LOOP | XA_END);
        if (XA_PLAYER.flags & XA_SEEKED) {
            startXaRead();
            return;
        }
        XA_PLAYER.flags |= XA_START_PENDING;
    }
}

/* Stops the track. */
void stopXa(void) {
    stopXaInline();
}

/* Cdfs.hook: turns the player off before CDFS reads. */
void turnXaOff(void) {
    turnXaOffInline();
}

/* The seek's callback: the drive is at the track. */
void receiveXaSeek(u8 intr) {
    s32 discard;

    if (intr == CdlComplete) {
        func_80046930(&discard, 1);
        XA_PLAYER.flags |= XA_SEEKED;
    }
}

/* Reads the track from the stream's position. */
void startXaRead(void) {
    startXaReadInline();
}

/* The read command's callback: the track plays from now. */
void receiveXaReadComplete(u8 intr) {
    XA_STREAM.startTime = VSync(-1);
    if (intr == CdlComplete) {
        XA_PLAYER.flags &= ~(XA_STARTING | XA_PAUSED);
        XA_PLAYER.flags |= XA_PLAYING;
        func_80046DC0(receiveXaSector);
        setXaVolume(XA_VOLUME_ON);
    }
}

/* The callback of a read restarted after an error. */
void receiveXaRestart(u8 intr) {
    if (intr == CdlComplete) {
        func_80046DC0(receiveXaSector);
        XA_STREAM.startTime = VSync(-1);
    }
}

/* Stops the drive if the track plays, keeping where it was; returns
 * whether it played. */
s32 stopXaDrive(void) {
    if ((XA_PLAYER.flags & XA_ON) && (XA_PLAYER.flags & XA_PLAYING) && !(XA_PLAYER.flags & XA_PAUSED)) {
        func_80046DC0(NULL);
        XA_PLAYER.flags &= ~XA_PLAYING;
        estimateXaPosition();
        setXaVolume(0);
        func_800468A0();
        func_80043724(CdlPause, NULL, NULL, -1);
        return 1;
    }
    return 0;
}

/* Sets the stream's position to where the track should be by now, from the
 * time it has played. */
void estimateXaPosition(void) {
    s32 elapsed = VSync(-1) - XA_STREAM.startTime;

    if (elapsed < 0) {
        elapsed = 0;
    }
    /* double speed reads two sectors per single speed one */
    if (XA_STREAM.track->mode & CdlModeSpeed) {
        elapsed *= 2;
    }
    /* VSyncs to sectors, times frameRate */
    elapsed *= XA_SECTORS_PER_SECOND;
    func_80046B60(XA_STREAM.startSector + elapsed / XA_STREAM.frameRate, &XA_STREAM.position);
}

/* Returns whether a seek is under way. */
s32 isXaSeeking(void) {
    return !(XA_PLAYER.flags & XA_SEEKED);
}

/* Returns whether a track plays or is about to. */
s32 isXaPlaying(void) {
    if (XA_PLAYER.flags & XA_PAUSED) {
        return 0;
    }
    return (XA_PLAYER.flags & (XA_STARTING | XA_PLAYING | XA_START_PENDING)) != 0;
}

/* Sets the callback of the events (none: the default); returns the old one. */
XaCallback setXaCallback(XaCallback callback) {
    XaCallback old = XA_PLAYER.callback;

    if (callback == NULL) {
        callback = ignoreXaEvent;
    }
    XA_PLAYER.callback = callback;
    return old;
}

/* The default callback: ignores the events. */
void ignoreXaEvent(s32 event) {
}

/* Sets the mix mode; returns the old one. */
s32 setXaMixMode(s32 mixMode) {
    return setXaMixModeInline(mixMode);
}

/* Plays in mono; returns the old mix mode. */
s32 setXaMono(void) {
    return setXaMixModeInline(XA_MONO);
}

/* Plays in stereo; returns the old mix mode. */
s32 setXaStereo(void) {
    return setXaMixModeInline(XA_STEREO);
}

/* Sets the volume. */
void setXaVolume(u32 volume) {
    setXaVolumeInline(volume);
}

/* Sets how many times the tracks play; returns the old count of replays. */
u16 setXaLoops(u16 loops) {
    u16 old = XA_PLAYER.loopCount;

    XA_PLAYER.loopCount = loops - 1;
    return old;
}

/* Waits for the seek to end, if the player is on. */
void waitXaSeek(void) {
    if (XA_PLAYER.flags & XA_ON) {
        while (!(XA_PLAYER.flags & XA_SEEKED)) {
        }
    }
}

/* Reads the XAP file of the tracks of an XA file into tracks, their sectors
 * made absolute; returns how many there are, or 0 on an error. */
u16 loadXaTracks(XaTrack *tracks, char *name) {
    XapHeader header;
    CdfsDoneFunc done = setCdfsDone(NULL);
    s32 count;
    s32 base;
    s32 i;
    u8 mode;

    if (loadFileSync(&header, name, 0, sizeof(header)) != 0 && header.magic == XAP_MAGIC) {
        count = header.count;
        base = func_80046C70(&CDFS.loc);
        if (loadFileSync(tracks, name, XAP_TRACKS_OFFSET, count * sizeof(XaTrack)) != 0) {
            setCdfsDone(done);
            mode = XA_MODE;
            if (header.interleave == XAP_INTERLEAVE_DOUBLE) {
                mode = XA_MODE | CdlModeSpeed;
            }
            i = count;
            while (i-- != 0) {
                tracks->mode = mode;
                tracks->start += base;
                tracks->end += base;
                tracks++;
            }
            return count;
        }
    }
    setCdfsDone(done);
    LOG_PRINT("CDFSXA: XAP read error!\n");
    return 0;
}

/* Sets the tracks the player plays. */
void setXaTracks(XaTrack *tracks, u16 count) {
    XA_PLAYER.tracks = tracks;
    XA_PLAYER.trackCount = count;
}
