#ifndef DTBE_CD_CDFS_H
#define DTBE_CD_CDFS_H

/* The VFS archives: their tables on the disc, mounting them on a path, inflating them. */

#include "common.h"
#include "engine/lib/inflate.h"
#include "engine/lib/list.h"

EXTERN_C_BEGIN

#define CDFS_VFS_PROBE 0x20      /* reading a VFS archive's head */

/* An entry of a VFS archive's table: a file, or a directory whose sorted
 * entries are elsewhere in the table. */
typedef struct VfsEntry {
    /* 0x0 */ union {
        u32 sector;               /* a file's first sector */
        struct VfsEntry *entries; /* a directory's entries, once mounted */
    } at;
    /* 0x4 */ s32 size; /* a file's size, a directory's entry count */
    /* 0x8 */ u16 nameOffset; /* where its name is in the names */
    /* 0xA */ u16 flags;      /* VFS_DIRECTORY | the file's offset in its sector */
} VfsEntry;

/* VfsEntry.flags: the entry is a directory, and the bits of the file's offset
 * in its sector */
#define VFS_DIRECTORY 0x800
#define VFS_OFFSET_MASK 0x7FF

/* the head of a VFS archive on the disc */
typedef struct {
    /* 0x0 */ s32 magic; /* VFS_MAGIC */
    /* 0x4 */ s32 count; /* entries in its table */
    /* 0x8 */ s32 dataSector; /* where its files start, from its own sector */
    /* 0xC */ s32 unkC;
} VfsHeader;

#define VFS_MAGIC 0x32534656 /* "VFS2" */
/* the buffer size a table entry needs: the entry and its name */
#define VFS_BYTES_PER_ENTRY 32

/* where a VFS archive's table and files are */
typedef struct {
    /* 0x0 */ u16 count;
    /* 0x4 */ s32 tableSector; /* the compressed table */
    /* 0x8 */ s32 dataSector;
} VfsInfo;

/* A mounted VFS archive (a node of Cdfs.vfs). */
typedef struct {
    /* 0x00 */ LinkNode link;
    /* 0x08 */ VfsEntry *entries; /* the first is the root directory */
    /* 0x0C */ char *names;
    /* 0x10 */ char mountPoint[0x70];
} Vfs;

/* how many reads the queue holds */
#define CDFS_REQUEST_MAX 16

/* what mountVfs returns when it mounts nothing */
#define VFS_FULL ((Vfs *)-1)  /* all the mount slots are in use */
#define VFS_TAKEN ((Vfs *)-2) /* the path has an archive already */

/* a log message, a rodata file of its own: it sits before an earlier
 * function's rodata */
extern char STR_CDFS_INVALID_VFS_FILE[]; /* "CDFS: Invalid VFS file.\n" */

s32 initCdfs(s32 noInit);
void skipCdfsHook(void);
Vfs *mountVfs(char *path, VfsEntry *table, VfsInfo *info);
s32 loadVfsInfo(VfsInfo *info, char *path);
void setVfsAllocator(AllocFunc zalloc, FreeFunc zfree, void *opaque);
s32 decompressStream(VfsEntry *dst, u8 *src);
void vfsLoadTable(Vfs *vfs, VfsEntry *table, VfsInfo *info);

EXTERN_C_END

#endif /* DTBE_CD_CDFS_H */
