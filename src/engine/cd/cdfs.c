#include "common.h"
#include "engine/cd/cdfs.h"
#include "engine/cd/read.h"
#include "engine/lib/inflate.h"
#include "engine/lib/node_pool.h"
#include "engine/lib/string.h"
#include "engine/system/log.h"
#include "memory.h"
#include "psyq.h"
#include "strings.h"

/* the largest size: zlib reads and writes until the stream ends */
#define ZSTREAM_SIZE_UNKNOWN 0x7FFFFFFF

/* Sets the reader up, running; also initializes the drive unless noInit. */
s32 initCdfs(s32 noInit) {
    bzero(&CDFS, sizeof(Cdfs));
    initVfsPool();
    nodePoolInit(&CDFS.requests, CDFS.requestNodes, sizeof(CdfsRequest),
                  CDFS_REQUEST_MAX);
    CDFS.hook = skipCdfsHook;
    CDFS.find = findFile;
    /* the no-op hook serves as the idle function too, its argument unused */
    CDFS.idle = (void (*)(s32))skipCdfsHook;
    CDFS.flags |= CDFS_READY;
    if (noInit == 0) {
        return func_800434D4();
    }
    return 0;
}

/* The default hook and idle function: does nothing. */
void skipCdfsHook(void) {
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/cd/cdfs", STR_CDFS_INVALID_VFS_FILE);

/* Mounts the VFS archive whose table is read into table on path. Returns
 * it, VFS_FULL when all are in use or VFS_TAKEN when path has one. */
Vfs *mountVfs(char *path, VfsEntry *table, VfsInfo *info) {
    /* the pool's nodes are Vfs, which start with their LinkNode */
    Vfs *vfs = (Vfs *)nodePoolAlloc(&CDFS.vfs);
    Vfs *other;

    if (vfs == NULL) {
        return VFS_FULL;
    }
    getFullPath(vfs->mountPoint, path);
    strcat(vfs->mountPoint, "/");
    copySlashPath(vfs->mountPoint, vfs->mountPoint);
    copyLowerCase(vfs->mountPoint, vfs->mountPoint);
    /* the mounted list links Vfs nodes, LinkNode first */
    for (other = (Vfs *)CDFS.vfs.used.head; other != NULL; other = (Vfs *)other->link.next) {
        if (strcmp(other->mountPoint, vfs->mountPoint) == 0) {
            nodePoolFree(&CDFS.vfs, &vfs->link);
            return VFS_TAKEN;
        }
    }
    vfsLoadTable(vfs, table, info);
    nodePoolPushBack(&CDFS.vfs, &vfs->link);
    LOG_PRINT("CDFS: VFS has been mounted on %s\n", vfs->mountPoint);
    return vfs;
}

/* Reads the head of the VFS archive at path into info; returns the size
 * of the buffer its table needs, or 0 if it is not a VFS archive. */
s32 loadVfsInfo(VfsInfo *info, char *path) {
    VfsHeader header;
    s32 sector;

    CDFS.flags |= CDFS_VFS_PROBE;
    if (loadFileSync(&header, path, 0, sizeof(header)) != 0) {
        if (header.magic == VFS_MAGIC) {
            sector = func_80046C70(&CDFS.loc);
            info->tableSector = sector + 1;
            info->count = header.count;
            info->dataSector = sector + header.dataSector;
            return header.count * VFS_BYTES_PER_ENTRY;
        }
        LOG_PRINT(STR_CDFS_INVALID_VFS_FILE);
    }
    return 0;
}

/* Sets the allocator of the VFS tables. */
void setVfsAllocator(AllocFunc zalloc, FreeFunc zfree, void *opaque) {
    CDFS.zalloc = zalloc;
    CDFS.zfree = zfree;
    CDFS.opaque = opaque;
}

/* Inflates the zlib stream at source into dest; returns 0, or 1 on an
 * error. */
s32 decompressStream(VfsEntry *dest, u8 *source) {
    ZStream stream;
    s32 result;
    s32 failed;

    /* the table inflates as bytes */
    stream.nextOut = (u8 *)dest;
    stream.nextIn = source;
    stream.availIn = ZSTREAM_SIZE_UNKNOWN;
    stream.availOut = ZSTREAM_SIZE_UNKNOWN;
    stream.zalloc = CDFS.zalloc;
    stream.zfree = CDFS.zfree;
    stream.opaque = CDFS.opaque;
    result = inflateInit_(&stream, ZLIB_VERSION, sizeof(ZStream));
    failed = result != Z_OK;
    if (!failed) {
        while (result != Z_STREAM_END) {
            result = inflate(&stream, 0);
            failed = result != Z_OK;
            if (result == Z_STREAM_END) {
                break;
            }
            if (failed) {
                inflateEnd(&stream);
                return failed;
            }
        }
        return inflateEnd(&stream) != Z_OK;
    }
    inflateEnd(&stream);
    return failed;
}

/* Reads and inflates the VFS archive's table into table and makes its
 * entries point: a directory at its entries, a file at its sector. */
void vfsLoadTable(Vfs *vfs, VfsEntry *table, VfsInfo *info) {
    CdlLOC loc;
    VfsEntry *entry = table;
    u32 size;
    u8 *packed;
    s32 i;

    vfs->entries = entry;
    /* the names follow the entries */
    vfs->names = (char *)(entry + info->count);
    size = (info->dataSector - info->tableSector) << CD_SECTOR_SHIFT;
    packed = CDFS.zalloc(CDFS.opaque, 1, size);
    func_80046B60(info->tableSector, &loc);
    syncCdfs(0);
    loadSectors(packed, &loc, 0, size);
    syncCdfs(0);
    decompressStream(entry, packed);
    CDFS.zfree(CDFS.opaque, packed);
    i = info->count;
    while (--i != -1) {
        if (entry->flags & VFS_DIRECTORY) {
            /* the offset in bytes of its entries from the table becomes
             * their address */
            entry->at.sector += (u32)table;
        } else {
            entry->at.sector += info->dataSector;
        }
        entry++;
    }
}
