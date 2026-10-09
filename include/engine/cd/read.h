#ifndef DTBE_CD_READ_H
#define DTBE_CD_READ_H

/* CDFS, the file reader: a queue of reads, found in a VFS archive or on the disc. */

#include "common.h"
#include <libcd.h>
#include "engine/cd/cdfs.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"

EXTERN_C_BEGIN

/*
 * CDFS: the game's file reader. A read is a request in a queue; the head
 * request is looked up (in a mounted VFS archive, else in the disc's ISO
 * directory), then read sector by sector from the CD-ROM interrupt
 * callbacks into the caller's buffer.
 */

/* the bytes of data in a CD-ROM sector, and their log2 */
#define CD_SECTOR_SIZE 0x800
#define CD_SECTOR_SHIFT 11
/* CdGetSector counts in words */
#define CD_SECTOR_WORDS (CD_SECTOR_SIZE / 4)

/* the read mode of the data reads */
#define CDFS_MODE (CdlModeSpeed | CdlModeSize1)

/* Cdfs.flags */
#define CDFS_SECTOR_DONE 0x1 /* the last sector of the read arrived */
#define CDFS_ENABLED 0x2     /* the queue may start its head request */
#define CDFS_RUNNING 0x4     /* the queue runs (not paused) */
#define CDFS_READY (CDFS_ENABLED | CDFS_RUNNING)

#define CDFS_STARTING 0x20000000 /* a request is being started */
#define CDFS_ERROR 0x40000000    /* the drive reported an error */
#define CDFS_READING 0x80000000  /* a data read is under way */

/* CdfsRequest.flags */
#define CDFS_REQ_STARTED 0x1 /* issued once: a retry restores the read state */
#define CDFS_REQ_SECTOR 0x2  /* a read of sectors, not of a file */
#define CDFS_REQ_FOUND 0x4   /* the file was looked up */
#define CDFS_REQ_NO_SEEK 0x8 /* the file is at position 0: no data read */

/* the size of a request's file name */
#define CDFS_NAME_MAX 0x59

/* called when a file read ends */
typedef void (*CdfsDoneFunc)(void *buffer, u32 size, char *name, s32 user);

/* A queued read (a node of Cdfs.requests). */
typedef struct CdfsRequest {
    /* 0x00 */ LinkNode link;
    /* 0x08 */ u8 *buffer;
    /* 0x0C */ u32 offset; /* where in the file the read starts */
    /* 0x10 */ u32 size;   /* bytes to read; 0 reads to the end of the file */
    /* 0x14 */ CdfsDoneFunc done;
    /* 0x18 */ s32 user; /* given back to done */
    /* 0x1C */ CdlLOC loc; /* the file's first sector, then the read's */
    /* 0x20 */ u32 fileSize;
    /* 0x24 */ u16 fileOffset; /* where the file starts in its first sector */
    /* 0x26 */ u8 flags;       /* CDFS_REQ_* */
    /* 0x27 */ char name[CDFS_NAME_MAX];
} CdfsRequest;

/* What a file lookup finds: the position, size and name of a file. */
typedef struct {
    /* 0x00 */ union {
        /* 0x00 */ CdlLOC loc;
        /* 0x00 */ u32 word; /* 0 for no position: nothing to read */
    } position;
    /* 0x04 */ u32 size;
    /* 0x08 */ char name[22];
    /* 0x1E */ u16 offset; /* where the file starts in its first sector */
} CdfsFile;

/* how many VFS archives can be mounted, and the size of the current
 * directory */
#define CDFS_VFS_MAX 4
#define CDFS_PATH_MAX 0x40
/* the size of the path buffers the lookups build */
#define CDFS_ISO_PATH_MAX 0x60

/* The file reader: its queue of requests, the mounted VFS archives and the
 * current directory. */
typedef struct Cdfs {
    /* 0x000 */ volatile u32 flags; /* CDFS_*, also set by the CD-ROM callbacks */
    /* 0x004 */ CdfsRequest *current;
    /* 0x008 */ CdfsDoneFunc done; /* given to the next requests */
    /* 0x00C */ s32 user;          /* given to the next requests */
    /* 0x010 */ void (*idle)(s32 arg); /* called while waiting for the queue */
    /* 0x014 */ s32 idleArg;
    /* 0x018 */ s32 result; /* the size the last request read */
    /* 0x01C */ CdlLOC loc; /* where the last request read */
    /* 0x020 */ void (*hook)(void); /* called before the drive is used */
    /* 0x024 */ void (*unk24)(void);
    /* 0x028 */ s32 (*find)(CdfsFile *file, char *name);
    /* 0x02C */ NodePool requests;
    /* 0x040 */ CdfsRequest requestNodes[CDFS_REQUEST_MAX];
    /* 0x840 */ NodePool vfs;
    /* 0x854 */ Vfs vfsNodes[CDFS_VFS_MAX];
    /* 0xA54 */ char cwd[CDFS_PATH_MAX]; /* prefixed to the relative paths */
    /* 0xA94 */ AllocFunc zalloc; /* for the VFS tables */
    /* 0xA98 */ FreeFunc zfree;
    /* 0xA9C */ void *opaque;
} Cdfs;

/* The state of the data read in progress. A read copies a first partial
 * sector (head) through CDFS_HEAD_BUFFER, reads the whole sectors straight into
 * the buffer, and copies a last partial sector (tail) through CDFS_TAIL_BUFFER. */
typedef struct CdfsRead {
    /* 0x00 */ u8 *dst;
    /* 0x04 */ u8 *savedDst; /* dst and sectors at the start, for retries */
    /* 0x08 */ u16 sectors;   /* sectors left to read */
    /* 0x0A */ u16 savedSectors;
    /* 0x0C */ u16 sectorIndex;
    /* 0x0E */ u16 headSize;
    /* 0x10 */ s32 bodySize;
    /* 0x14 */ u16 tailSize;
    /* 0x16 */ u16 shift; /* how far the body was read off its place */
    /* 0x18 */ u8 *head;  /* the file's first byte in CDFS_HEAD_BUFFER */
    /* 0x1C */ u8 *body;
    /* 0x20 */ u8 *tail;
    /* 0x24 */ void (*ready)(u8 intr); /* the data ready callback */
    /* 0x28 */ void (*finish)(void);   /* run once the last sector arrived */
    /* 0x2C */ s32 mode; /* CDFS_READ_*, for the log */
} CdfsRead;

/* the read modes: how the file lies in its sectors */
#define CDFS_READ_WHOLE 0   /* whole sectors */
#define CDFS_READ_TAIL 1    /* whole sectors, then a partial one */
#define CDFS_READ_HEAD 2    /* a partial first sector, then the rest */
#define CDFS_READ_SHIFTED 3 /* as HEAD, the body read 4-byte aligned */
#define CDFS_READ_SMALL 4   /* inside one sector */

/* log messages, rodata files of their own: two functions use each */
extern char STR_CDFS_FILE_NOT_FOUND[]; /* "CDFS: File not found \"%s\"\n" */
extern char STR_CDFS_OFFSET_SIZE_OVER[]; /* "CDFS: Offset size over \"%s\"\n" */

extern struct Cdfs CDFS; /* the file reader */
/* the sector buffers of a read's first and last partial sectors */
extern u8 CDFS_HEAD_BUFFER[0x800];
extern u8 CDFS_TAIL_BUFFER[0x800];
extern struct CdfsRead CDFS_READ; /* the data read in progress */

/* DsSearchFile (see psyq.h's CD-ROM functions): looks a file up in the disc's
 * ISO directory; 0 when it is not there, -1 on a drive error. Declared here as
 * it fills a CdfsFile, which starts as the SDK's CdlFILE. */
s32 func_80045920(CdfsFile *file, char *name);

s32 findIsoFile(CdfsFile *file, char *name);
s32 findFile(CdfsFile *file, char *name);
s32 findMountedFile(CdfsFile *file, char *name);
s32 findVfsFile(CdfsFile *file, Vfs *vfs, char *path);
VfsEntry *vfsEntryFind(VfsEntry *directory, char *names, char *name, s32 length);
s32 compareVfsName(u8 *name, u8 *key, s32 length);
s32 cdfsRequestStart(CdfsRequest *cdfsRequest);
s32 cdfsRequestLocate(CdfsRequest *cdfsRequest);
void cdfsRequestPlanRead(CdfsRequest *cdfsRequest, u32 offset);
void cdfsRequestPlanHeadRead(CdfsRequest *cdfsRequest, u32 offset);
void receiveShiftedReadSector(u8 intr);
void finishShiftedRead(void);
void finishCdfsRequest(void);
s32 loadFileSync(void *buffer, char *name, u32 offset, u32 size);
s32 queueFileLoad(void *buffer, char *name, u32 offset, u32 size);
s32 syncCdfs(s32 noWait);
void resumeCdfs(void);
void pauseCdfs(void);
void updateCdfs(void);
void setCdfsIdle(void (*idle)(s32 arg), s32 arg);
void getFullPath(char *dst, char *path);
s32 cdfsRequestFind(CdfsRequest *cdfsRequest);
void loadSectors(void *buffer, CdlLOC *loc, u32 offset, u32 size);
void enableCdfs(void);
void disableCdfs(void);
void startNextCdfsRequest(void);
void cdfsRequestStartRead(CdfsRequest *cdfsRequest);
void cdfsRequestSkipRead(CdfsRequest *cdfsRequest);
void receiveCdfsReadComplete(u8 intr);
void receiveWholeReadSector(u8 intr);
void receiveTailReadSector(u8 intr);
void finishTailRead(void);
void receiveHeadReadSector(u8 intr);
void finishHeadRead(void);
void receiveSmallReadSector(u8 intr);
void finishSmallRead(void);
void receiveCdfsLastSector(void);
void reportCdfsError(s32 line);
void retryCdfsRequest(void);
void skipCdfsIdle(s32 arg);
s32 vfsUnmount(Vfs *vfs);
void initVfsPool(void);
CdfsDoneFunc setCdfsDone(CdfsDoneFunc done);
u32 getFileSize(char *name);

EXTERN_C_END

#endif /* DTBE_CD_READ_H */
