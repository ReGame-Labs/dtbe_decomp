#include "common.h"
#include "engine/cd/read.h"
#include "engine/cd/cdfs.h"
#include "engine/lib/list.h"
#include "engine/lib/node_pool.h"
#include "engine/lib/string.h"
#include "engine/system/log.h"
#include "memory.h"
#include "psyq.h"
#include "stdio.h"
#include "strings.h"

/* The first node of a list. Read through this, a list of CDFS is
 * addressed as its own symbol, as the game does. */
static inline LinkNode *listHead(LinkList *list) {
    return list->head;
}

/* cdfsRequestFind's body, which the game also copied into cdfsRequestLocate.
 * cdfsRequestFind has its own copy: written as a call to this it keeps the
 * result in another register. */
static inline s32 cdfsFind(CdfsRequest *request) {
    CdfsFile file;

    if (request->flags & CDFS_REQ_FOUND) {
        return 0;
    }
    if (CDFS.find(&file, request->name) != 0) {
        LOG_PRINT(STR_CDFS_FILE_NOT_FOUND, request->name);
        return -1;
    }
    if (request->offset >= file.size) {
        LOG_PRINT(STR_CDFS_OFFSET_SIZE_OVER, request->name);
        return -1;
    }
    request->fileSize = file.size;
    request->fileOffset = file.offset;
    request->loc = file.pos.loc;
    if (file.pos.word == 0) {
        request->flags |= CDFS_REQ_NO_SEEK;
    }
    request->flags |= CDFS_REQ_FOUND;
    return 0;
}

/* Looks name up in the disc's ISO directory; returns 0 if it is not there. */
s32 findIsoFile(CdfsFile *file, char *name) {
    char path[CDFS_ISO_PATH_MAX];
    s32 result;

    path[0] = '\\';
    copyUpperCase(path + 1, name);
    copyBackslashPath(path, path);
    strcat(path, ";1");
    CDFS.hook();
    disableCdfs();
    do {
        result = func_80045920(file, path);
        if (result == 0) {
            break; /* not there */
        }
        if (result != -1) {
            break; /* found */
        }
        /* a drive error: reset it and look again */
        func_80043618();
    } while (1);
    enableCdfs();
    return result;
}

/* Cdfs.find: looks name up in the mounted VFS archives, then on the disc;
 * returns 0 if found. */
s32 findFile(CdfsFile *file, char *name) {
    if (findMountedFile(file, name) == 0) {
        return 0;
    }
    if (findIsoFile(file, name) == 0) {
        return -1;
    }
    /* an ISO file starts at its sector */
    file->offset = 0;
    return 0;
}

/* Looks name up in the mounted VFS archives: the first one whose mount point
 * starts the path moves to the head of the list, so that it is tried first
 * next time. Returns -1 if no archive has the path. */
s32 findMountedFile(CdfsFile *file, char *name) {
    char path[CDFS_ISO_PATH_MAX];
    s32 result = -1;
    Vfs *vfs;
    s32 len;

    path[0] = '/';
    copyLowerCase(path + 1, name);
    copySlashPath(path, path);
    vfs = (Vfs *)listHead(&CDFS.vfs.used);
    while (vfs != NULL) {
        len = strlen(vfs->mountPoint);
        if (strncmp(vfs->mountPoint, path, len) != 0) {
            vfs = (Vfs *)vfs->link.next;
        } else {
            if (listHead(&CDFS.vfs.used) != &vfs->link) {
                linkListRemove(&CDFS.vfs.used, &vfs->link);
                linkListInsertBefore(&CDFS.vfs.used, listHead(&CDFS.vfs.used), &vfs->link);
            }
            result = findVfsFile(file, vfs, path + len);
            break;
        }
    }
    return result;
}

/* Finds a path in a VFS archive's directory tree. The C is right, but GCC lays
 * out the -2/-3 returns in a different block order (60 diffs at best). */
INCLUDE_ASM("asm/jp/main/nonmatchings/cd/read", findVfsFile);

/* Finds the entry of the directory named by the length characters at name
 * (its entries are sorted); returns NULL if there is none. */
VfsEntry *vfsEntryFind(VfsEntry *dir, char *names, char *name, s32 length) {
    s32 low = 0;
    s32 high = dir->size - 1;
    s32 middle;
    s32 order;

    /* from here on the directory's entries */
    dir = dir->at.entries;
    middle = (low + high) / 2;
    while ((order = compareVfsName(names + dir[middle].nameOffset, name, length)) != 0) {
        if (low == high) {
            return NULL;
        }
        if (order > 0) {
            high = middle - 1;
        } else {
            low = middle + 1;
        }
        if (high < low) {
            return NULL;
        }
        middle = (low + high) / 2;
    }
    return &dir[middle];
}

/* Compares the name with the length characters at key: 0 if they are the
 * name, else where the name goes from them (as strncmp, the name having
 * to end there too). */
s32 compareVfsName(u8 *name, u8 *key, s32 length) {
    s32 diff;
    s32 left;

    for (left = length - 1; left != -1; left--) {
        diff = *name++ - *key++;
        if (diff != 0) {
            return diff;
        }
    }
    return *name;
}

/* Starts the request, or restarts it after an error; returns 0, or a
 * negative number when its file cannot be read (it is then dropped). */
s32 cdfsRequestStart(CdfsRequest *request) {
    s32 result;

    CDFS.flags |= CDFS_STARTING;
    if (!(request->flags & CDFS_REQ_STARTED)) {
        result = cdfsRequestLocate(request);
        if (result < 0) {
            linkListRemove(&CDFS.requests.used, &request->link);
            nodePoolFree(&CDFS.requests, &request->link);
            CDFS.result = 0;
            CDFS.flags &= ~CDFS_STARTING;
            return result;
        }
        CDFS_READ.savedSectors = CDFS_READ.sectors;
        CDFS_READ.savedDest = CDFS_READ.dest;
        request->flags |= CDFS_REQ_STARTED;
    } else {
        CDFS_READ.sectors = CDFS_READ.savedSectors;
        CDFS_READ.dest = CDFS_READ.savedDest;
    }
    CDFS.hook();
    CDFS.current = request;
    if (request->flags & CDFS_REQ_NO_SEEK) {
        cdfsRequestSkipRead(request);
    } else {
        cdfsRequestStartRead(request);
    }
    CDFS.idle(CDFS.idleArg);
    return 0;
}

/* Looks the request's file up, clips the size to it and finds where the
 * read starts; returns -1 if the file cannot be read. */
s32 cdfsRequestLocate(CdfsRequest *request) {
    u32 offset = request->offset;

    if (!(request->flags & CDFS_REQ_SECTOR)) {
        if (cdfsFind(request) != 0) {
            return -1;
        }
        if (request->fileSize - offset < request->size || request->size == 0) {
            request->size = request->fileSize - offset;
        }
        if (request->flags & CDFS_REQ_NO_SEEK) {
            return 0;
        }
        offset += request->fileOffset;
    }
    if (offset != 0) {
        func_80046B60(func_80046C70(&request->loc) + (offset >> CD_SECTOR_SHIFT), &request->loc);
        offset &= CD_SECTOR_SIZE - 1;
    }
    cdfsRequestPlanRead(request, offset);
    return 0;
}

/* Picks how to read the request from where it starts in its first sector:
 * whole sectors go straight to an aligned buffer. */
void cdfsRequestPlanRead(CdfsRequest *request, u32 offset) {
    /* CdGetSector stores words: only a word aligned buffer takes them */
    if (offset != 0 || ((u32)request->buffer & 3)) {
        if ((request->size + offset + CD_SECTOR_SIZE - 1) >> CD_SECTOR_SHIFT == 1) {
            CDFS_READ.head = CDFS_HEAD_BUFFER + offset;
            CDFS_READ.headSize = request->size;
            CDFS_READ.ready = receiveSmallReadSector;
            CDFS_READ.finish = finishSmallRead;
            CDFS_READ.mode = CDFS_READ_SMALL;
            return;
        }
        cdfsRequestPlanHeadRead(request, offset);
        return;
    }
    CDFS_READ.dest = request->buffer;
    CDFS_READ.sectors = (request->size + CD_SECTOR_SIZE - 1) >> CD_SECTOR_SHIFT;
    CDFS_READ.tailSize = request->size & (CD_SECTOR_SIZE - 1);
    if (CDFS_READ.tailSize == 0) {
        CDFS_READ.ready = receiveWholeReadSector;
        CDFS_READ.finish = finishCdfsRequest;
        CDFS_READ.mode = CDFS_READ_WHOLE;
        return;
    }
    CDFS_READ.ready = receiveTailReadSector;
    CDFS_READ.finish = finishTailRead;
    CDFS_READ.mode = CDFS_READ_TAIL;
}

/* Sets up a read that starts inside its first sector: the body is read to
 * the word aligned place at or before it, then moved up. */
void cdfsRequestPlanHeadRead(CdfsRequest *request, u32 offset) {
    u32 headSize = CD_SECTOR_SIZE - offset;
    u8 *body = request->buffer + headSize;
    /* the body's place rounded down to a word, CdGetSector stores words
     * (an integer, to mask it) */
    u32 aligned = (u32)body & ~3;
    u32 shift = (u32)body - aligned;
    u8 *dest = (u8 *)aligned;
    u32 sectors = (request->size + offset + CD_SECTOR_SIZE - 1) >> CD_SECTOR_SHIFT;
    u8 *head = CDFS_HEAD_BUFFER + offset;
    u32 rest = request->size - headSize;
    u32 bodySize = rest & ~(CD_SECTOR_SIZE - 1);
    u8 *tail = body + bodySize;
    u32 tailSize = rest - bodySize;

    if (bodySize != 0 && headSize < shift) {
        /* the aligned place is before the buffer: read a word later */
        dest += 4;
        shift -= 4;
        CDFS_READ.ready = receiveShiftedReadSector;
        CDFS_READ.finish = finishShiftedRead;
        CDFS_READ.mode = CDFS_READ_SHIFTED;
    } else {
        CDFS_READ.ready = receiveHeadReadSector;
        CDFS_READ.finish = finishHeadRead;
        CDFS_READ.mode = CDFS_READ_HEAD;
    }
    CDFS_READ.sectors = sectors;
    CDFS_READ.headSize = headSize;
    CDFS_READ.bodySize = bodySize;
    CDFS_READ.tailSize = tailSize;
    CDFS_READ.shift = shift;
    CDFS_READ.dest = dest;
    CDFS_READ.head = head;
    CDFS_READ.body = body;
    CDFS_READ.tail = tail;
}

/* The data ready callback of a shifted read: the first sector goes to the
 * head's buffer, the first word of the second there too. */
void receiveShiftedReadSector(u8 intr) {
    if (intr != CdlDataReady) {
        reportCdfsError(651);
        return;
    }
    if (--CDFS_READ.sectors != 0) {
        switch (CDFS_READ.sectorIndex++) {
            case 0:
                func_80046930(CDFS_HEAD_BUFFER, CD_SECTOR_WORDS);
                break;
            case 1:
                /* the word before the shifted body goes to the unused
                 * start of the head's buffer (finishShiftedRead moves it) */
                func_80046930(CDFS_HEAD_BUFFER, 1);
                func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS - 1);
                CDFS_READ.dest += CD_SECTOR_SIZE - 4;
                break;
            default:
                func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
                CDFS_READ.dest += CD_SECTOR_SIZE;
                break;
        }
        return;
    }
    func_80046E00(receiveCdfsLastSector);
    if (CDFS_READ.tailSize == 0) {
        func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
    } else {
        func_80046930(CDFS_TAIL_BUFFER, (CDFS_READ.tailSize + 3) / 4);
    }
    func_80046304();
}

/* The end of a shifted read: puts its pieces in place. */
void finishShiftedRead(void) {
    u8 *body = CDFS_READ.body;

    /* the word receiveShiftedReadSector kept apart, then the body in its place */
    memcpy(body, CDFS_HEAD_BUFFER, 4);
    body += 4;
    memmove(body, body - CDFS_READ.shift, CDFS_READ.bodySize - 4);
    memcpy(CDFS.current->buffer, CDFS_READ.head, CDFS_READ.headSize);
    if (CDFS_READ.tailSize != 0) {
        memcpy(CDFS_READ.tail, CDFS_TAIL_BUFFER, CDFS_READ.tailSize);
    }
    finishCdfsRequest();
}

/* Ends the current request: starts the next one and calls its done. */
void finishCdfsRequest(void) {
    CdfsRequest *request = CDFS.current;

    CDFS.current = NULL;
    CDFS.flags &= ~CDFS_STARTING;
    linkListRemove(&CDFS.requests.used, &request->link);
    if (!(request->flags & CDFS_REQ_NO_SEEK)) {
        LOG_PRINT("done\n");
    }
    if (CDFS.requests.used.count != 0 && !(CDFS.flags & CDFS_STARTING) &&
        (CDFS.flags & CDFS_READY) == CDFS_READY) {
        /* the queue links requests, LinkNode first */
        cdfsRequestStart((CdfsRequest *)CDFS.requests.used.head);
    }
    CDFS.result = request->size;
    CDFS.loc = request->loc;
    if (!(request->flags & CDFS_REQ_SECTOR) && request->done != NULL) {
        request->done(request->buffer, request->size, request->name, request->user);
    }
    nodePoolFree(&CDFS.requests, &request->link);
}

/* Reads size bytes of the file at offset into buffer, ahead of the queue,
 * and waits for it; returns the size read. */
s32 loadFileSync(void *buffer, char *name, u32 offset, u32 size) {
    u32 running = CDFS.flags & CDFS_READY;
    CdfsRequest *request;

    CDFS.hook();
    CDFS.flags &= ~CDFS_ENABLED;
    syncCdfs(0);
    /* the pool's nodes are requests, which start with their LinkNode */
    request = (CdfsRequest *)nodePoolAlloc(&CDFS.requests);
    request->buffer = buffer;
    request->offset = offset;
    request->size = size;
    request->done = CDFS.done;
    request->user = CDFS.user;
    request->flags = 0;
    getFullPath(request->name, name);
    nodePoolPushFront(&CDFS.requests, &request->link);
    cdfsRequestStart(request);
    CDFS.flags &= ~CDFS_ENABLED;
    syncCdfs(0);
    CDFS.flags |= CDFS_ENABLED;
    startNextCdfsRequest();
    CDFS.flags |= running;
    return CDFS.result;
}

/* Queues a read request. The C written from the assembly differs only in
 * how CDFS is addressed: the original reaches done and user at
 * negative offsets from &CDFS.requests, the pool it just passed, and
 * makes CDFS's own address only for the last read; GCC makes it at
 * once. What in the source decides that is not known. */
INCLUDE_ASM("asm/jp/main/nonmatchings/cd/read", queueFileLoad);

/* Services the request queue, and unless noWait, until it is empty or the
 * drive stops. Same base register sharing as queueFileLoad (26 diffs). */
INCLUDE_ASM("asm/jp/main/nonmatchings/cd/read", syncCdfs);

/* Runs the queue. */
void resumeCdfs(void) {
    CDFS.flags |= CDFS_RUNNING;
    startNextCdfsRequest();
}

/* Stops the queue once the read under way ends. */
void pauseCdfs(void) {
    CDFS.flags &= ~CDFS_RUNNING;
    syncCdfs(0);
}

/* Handles what the CD-ROM callbacks flagged: an error, the last sector. */
void updateCdfs(void) {
    if (CDFS.flags & CDFS_ERROR) {
        retryCdfsRequest();
    }
    if (CDFS.flags & CDFS_SECTOR_DONE) {
        CDFS.flags &= ~CDFS_SECTOR_DONE;
        CDFS_READ.finish();
    }
}

/* Sets the function called while waiting (none: the default). */
void setCdfsIdle(void (*idle)(s32 arg), s32 arg) {
    if (idle == NULL) {
        idle = skipCdfsIdle;
    }
    CDFS.idle = idle;
    CDFS.idleArg = arg;
}

/* Makes the full path of path: '/' separated, relative ones from cwd. */
void getFullPath(char *dest, char *path) {
    u8 clean[CDFS_ISO_PATH_MAX];

    copySlashPath(clean, path);
    if (clean[0] == '/') {
        strcpy(dest, clean);
        return;
    }
    strcpy(dest, CDFS.cwd);
    strcat(dest, clean);
}

/* Looks the request's file up once: sets its position, size and offset in
 * its sector. Returns -1 when there is no such file or the read would start
 * past its end. */
s32 cdfsRequestFind(CdfsRequest *request) {
    CdfsFile file;

    if (request->flags & CDFS_REQ_FOUND) {
        return 0;
    }
    if (CDFS.find(&file, request->name) != 0) {
        LOG_PRINT(STR_CDFS_FILE_NOT_FOUND, request->name);
        return -1;
    }
    if (request->offset >= file.size) {
        LOG_PRINT(STR_CDFS_OFFSET_SIZE_OVER, request->name);
        return -1;
    }
    request->fileSize = file.size;
    request->fileOffset = file.offset;
    request->loc = file.pos.loc;
    if (file.pos.word == 0) {
        request->flags |= CDFS_REQ_NO_SEEK;
    }
    request->flags |= CDFS_REQ_FOUND;
    return 0;
}

/* Reads size bytes from offset of the sectors at loc into buffer. */
void loadSectors(void *buffer, CdlLOC *loc, u32 offset, u32 size) {
    /* the pool's nodes are requests, which start with their LinkNode */
    CdfsRequest *request = (CdfsRequest *)nodePoolAlloc(&CDFS.requests);

    request->buffer = buffer;
    request->offset = offset;
    request->size = size;
    request->flags = CDFS_REQ_SECTOR | CDFS_REQ_FOUND;
    request->loc = *loc;
    sprintf(request->name, " Sector No.%08x ", func_80046C70(loc));
    nodePoolPushBack(&CDFS.requests, &request->link);
    cdfsRequestStart(request);
}

/* Lets the queue start reads. */
void enableCdfs(void) {
    CDFS.flags |= CDFS_ENABLED;
    startNextCdfsRequest();
}

/* Keeps the queue from starting reads, once the one under way ends. */
void disableCdfs(void) {
    CDFS.flags &= ~CDFS_ENABLED;
    syncCdfs(0);
}

/* Starts the head request if the queue is ready and idle. */
void startNextCdfsRequest(void) {
    LinkList *queue = &CDFS.requests.used;

    if (queue->count != 0 && !(CDFS.flags & CDFS_STARTING) &&
        (CDFS.flags & CDFS_READY) == CDFS_READY) {
        /* the queue links requests, LinkNode first */
        cdfsRequestStart((CdfsRequest *)queue->head);
    }
}

/* Starts the data read of the request. */
void cdfsRequestStartRead(CdfsRequest *request) {
    LOG_PRINT("CDFS: Read(%d) %p %08x \"%s\" ..", CDFS_READ.mode, request->buffer, request->size,
               request->name);
    func_80043998(CDFS_MODE, &request->loc, CdlReadN, receiveCdfsReadComplete, -1);
    CDFS_READ.sectorIndex = 0;
    CDFS.flags |= CDFS_READING;
}

/* Ends a request that has nothing to read. */
void cdfsRequestSkipRead(CdfsRequest *request) {
    CDFS.unk24();
    CDFS.flags |= CDFS_SECTOR_DONE;
    CDFS_READ.finish = finishCdfsRequest;
}

/* The read command's callback: the sectors come to the ready callback. */
void receiveCdfsReadComplete(u8 intr) {
    if (intr != CdlComplete) {
        reportCdfsError(529);
        return;
    }
    func_80046E00(NULL);
    func_80046280(CDFS_READ.ready, -1);
}

/* The data ready callback of a read of whole sectors. */
void receiveWholeReadSector(u8 intr) {
    if (intr != CdlDataReady) {
        reportCdfsError(548);
        return;
    }
    if (--CDFS_READ.sectors == 0) {
        func_80046E00(receiveCdfsLastSector);
    }
    func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
    if (CDFS_READ.sectors == 0) {
        func_80046304();
        return;
    }
    CDFS_READ.dest += CD_SECTOR_SIZE;
}

/* The data ready callback of a read ending in a partial sector. */
void receiveTailReadSector(u8 intr) {
    if (intr != CdlDataReady) {
        reportCdfsError(573);
        return;
    }
    if (--CDFS_READ.sectors != 0) {
        func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
        CDFS_READ.dest += CD_SECTOR_SIZE;
        return;
    }
    func_80046E00(receiveCdfsLastSector);
    func_80046930(CDFS_TAIL_BUFFER, (CDFS_READ.tailSize + 3) / 4);
    func_80046304();
}

/* The end of a read ending in a partial sector: copies that in. */
void finishTailRead(void) {
    memcpy(CDFS_READ.dest, CDFS_TAIL_BUFFER, CDFS_READ.tailSize);
    finishCdfsRequest();
}

/* The data ready callback of a read starting in a partial sector. */
void receiveHeadReadSector(u8 intr) {
    if (intr != CdlDataReady) {
        reportCdfsError(604);
        return;
    }
    if (--CDFS_READ.sectors != 0) {
        if (++CDFS_READ.sectorIndex == 1) {
            func_80046930(CDFS_HEAD_BUFFER, CD_SECTOR_WORDS);
            return;
        }
        func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
        CDFS_READ.dest += CD_SECTOR_SIZE;
        return;
    }
    func_80046E00(receiveCdfsLastSector);
    if (CDFS_READ.tailSize == 0) {
        func_80046930(CDFS_READ.dest, CD_SECTOR_WORDS);
    } else {
        func_80046930(CDFS_TAIL_BUFFER, (CDFS_READ.tailSize + 3) / 4);
    }
    func_80046304();
}

/* The end of a read starting in a partial sector: puts its pieces in
 * place. */
void finishHeadRead(void) {
    if (CDFS_READ.shift != 0) {
        memmove(CDFS_READ.body, CDFS_READ.body - CDFS_READ.shift, CDFS_READ.bodySize);
    }
    memcpy(CDFS.current->buffer, CDFS_READ.head, CDFS_READ.headSize);
    if (CDFS_READ.tailSize != 0) {
        memcpy(CDFS_READ.tail, CDFS_TAIL_BUFFER, CDFS_READ.tailSize);
    }
    finishCdfsRequest();
}

/* The data ready callback of a read inside one sector. */
void receiveSmallReadSector(u8 intr) {
    if (intr != CdlDataReady) {
        reportCdfsError(711);
        return;
    }
    func_80046E00(receiveCdfsLastSector);
    func_80046930(CDFS_HEAD_BUFFER, CD_SECTOR_WORDS);
    func_80046304();
}

/* The end of a read inside one sector: copies it out. */
void finishSmallRead(void) {
    memcpy(CDFS.current->buffer, CDFS_READ.head, CDFS_READ.headSize);
    finishCdfsRequest();
}

/* The sync callback of the last sector: tells updateCdfs. */
void receiveCdfsLastSector(void) {
    func_80046E00(NULL);
    CDFS.flags |= CDFS_SECTOR_DONE;
    CDFS.flags &= ~CDFS_READING;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/cd/read", STR_CDFS_FILE_NOT_FOUND);

INCLUDE_RODATA("asm/jp/main/nonmatchings/cd/read", STR_CDFS_OFFSET_SIZE_OVER);

/* Logs a drive error, at line in the game's cd/read.c, and stops. */
void reportCdfsError(s32 line) {
    LOG_PRINT("CDFS: Error detected! cd/read.c :%d:\n", line);
    func_80046304();
    CDFS.flags |= CDFS_ERROR;
}

/* After an error: restarts the current request once the drive is ready. */
void retryCdfsRequest(void) {
    if (func_800468E0() == 1) {
        LOG_PRINT("\nCDFS: DslReady detected\n");
        CDFS.flags &= ~CDFS_ERROR;
        cdfsRequestStart(CDFS.current);
    }
}

/* The default idle function: does nothing. */
void skipCdfsIdle(s32 arg) {
}

/* Unmounts a VFS archive; returns the number of archives left, or -1 if it
 * was not mounted. */
s32 vfsUnmount(Vfs *vfs) {
    LinkNode *node;

    for (node = listHead(&CDFS.vfs.used); node != NULL; node = node->next) {
        if (node == &vfs->link) {
            NodePool *pool = &CDFS.vfs;

            nodePoolRelease(pool, node);
            return pool->used.count;
        }
    }
    return -1;
}

/* Sets the pool of the VFS archives up. */
void initVfsPool(void) {
    nodePoolInit(&CDFS.vfs, CDFS.vfsNodes, sizeof(Vfs), CDFS_VFS_MAX);
}

/* Sets the done function of the next reads; returns the old one. */
CdfsDoneFunc setCdfsDone(CdfsDoneFunc done) {
    CdfsDoneFunc old = CDFS.done;

    CDFS.done = done;
    return old;
}

/* Returns the size of the file, or 0 if there is no such file. */
u32 getFileSize(char *name) {
    char path[CDFS_ISO_PATH_MAX];
    CdfsFile file;

    getFullPath(path, name);
    if (CDFS.find(&file, path) == 0) {
        return file.size;
    }
    return 0;
}
