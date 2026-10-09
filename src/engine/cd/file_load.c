#include "common.h"
#include "engine/cd/file_load.h"
#include "engine/cd/cdfs.h"
#include "engine/cd/read.h"
#include "engine/cd/xa_player.h"
#include "engine/system/memory.h"
#include "libapi.h"
#include "libgpu.h"
#include "strings.h"
#include "psyq.h"

/*
 * Loads the compressed file at path, inflated, into dst, unless it is the
 * one loaded last.
 */
void loadCompressedFileInto(void *dst, char *path) {
    CompressedHeader *file;

    /* D_1 is a symbol the linker put at 1: the test always passes */
    if (D_1 != NULL && strcmp(LAST_LOADED_PATH, path) != 0) {
        strcpy(LAST_LOADED_PATH, path);
        file = mainHeapAllocLargest(getFileSize(path));
        loadFileSync(file, path, 0, 0);
        decompressStreamTo(dst, (u8 *)(file + 1));
        mainHeapFree(file);
        FlushCache();
    }
}

/* Loads the image and the CLUT of a TIM into VRAM. */
void uploadTim(u32 *tim) {
    TIM_IMAGE image;

    func_800585B0(tim);
    if (ReadTIM(&image) != NULL) {
        if (image.caddr != NULL) {
            func_80055B28(image.crect, image.caddr);
        }
        if (image.paddr != NULL) {
            func_80055B28(image.prect, image.paddr);
        }
        DrawSync(0);
    }
}

/* Loads the images and the CLUTs of a run of TIMs into VRAM. */
void uploadTims(u32 *tims) {
    TIM_IMAGE image;

    func_800585B0(tims);
    while (ReadTIM(&image) != NULL) {
        if (image.caddr != NULL) {
            func_80055B28(image.crect, image.caddr);
        }
        if (image.paddr != NULL) {
            func_80055B28(image.prect, image.paddr);
        }
        DrawSync(0);
    }
}

/*
 * Returns the TIM number index of a run of TIMs, or NULL if the run ends
 * before it. Each block of a TIM starts with its length in bytes.
 */
u32 *findTim(u32 *tim, s32 index) {
    u32 *block;

    for (;;) {
        if ((tim[0] & TIM_ID_MASK) != TIM_ID) {
            return NULL;
        }
        if (--index == -1) {
            break;
        }
        /* skip the header, the CLUT block if there is one, then the image block */
        block = tim + 2;
        if (block[-1] & TIM_HAS_CLUT) {
            block += tim[2] / sizeof(u32);
        }
        tim = block + block[0] / sizeof(u32);
    }
    return tim;
}

/* Loads a file, inflating it if it is compressed, into memory from mainHeapAllocBest. */
void *loadFile(char *path) {
    return loadFileWith(path, mainHeapAllocLargest, mainHeapAllocBest);
}

/* Loads a file, inflating it if it is compressed, into memory from mainHeapAllocLargest. */
void *loadFileLargest(char *path) {
    return loadFileWith(path, mainHeapAllocBest, mainHeapAllocLargest);
}

/* Loads a file as it is into memory from mainHeapAllocBest. */
void *loadRawFile(char *path) {
    return loadRawFileWith(path, mainHeapAllocBest);
}

/* Loads a file as it is into memory from mainHeapAllocLargest. */
void *loadRawFileLargest(char *path) {
    return loadRawFileWith(path, mainHeapAllocLargest);
}

/* the magic a compressed file starts with (4 bytes, no terminator) */
static char COMPRESSED_MAGIC[4] = "ZP00";

/*
 * Loads a file into memory from allocFile and, inflated if it is compressed,
 * into memory from allocData; frees the first. Returns the data.
 */
void *loadFileWith(char *path, void *(*allocFile)(s32), void *(*allocData)(s32)) {
    s32 size = getFileSize(path);
    CompressedHeader *file = loadRawFileWith(path, allocFile);
    void *data;

    if (func_8003DF40(file, COMPRESSED_MAGIC, sizeof(COMPRESSED_MAGIC)) == 0) {
        data = allocData(file->size);
        decompressStreamTo(data, (u8 *)(file + 1));
    } else {
        data = allocData(size);
        memmove(data, (u8 *)file, size);
    }
    mainHeapFree(file);
    return data;
}

/* Loads a file as it is into memory from alloc. */
void *loadRawFileWith(char *path, void *(*alloc)(s32)) {
    s32 size = getFileSize(path);
    void *data = alloc(size);

    loadFileSync(data, path, 0, size);
    return data;
}

/*
 * Starts reading the file at path in the background. *result is NULL until
 * the read ends; finishLoadFileAsync then sets it to the data, inflated if the file
 * is compressed. Returns 0, or -1 if the read could not start.
 */
s32 loadFileAsync(char *path, void **result) {
    u32 size = getFileSize(path);
    void *buffer = mainHeapAllocLargest(size);
    s32 status;

    setCdfsDone(finishLoadFileAsync);
    /* the read hands result to finishLoadFileAsync as its user word */
    setCdfsUser((s32)result);
    status = queueFileLoad(buffer, path, 0, size);
    setCdfsDone(NULL);
    *result = NULL;
    return status < 0 ? -1 : 0;
}

/*
 * Ends a read of loadFileAsync: puts the data, inflated if the file is
 * compressed, in memory from mainHeapAllocBest, frees the buffer and stores the
 * data in *user.
 */
void finishLoadFileAsync(void *buffer, u32 size, char *name, s32 user) {
    CompressedHeader *file = buffer;
    void *data;

    if (func_8003DF40(file, COMPRESSED_MAGIC, sizeof(COMPRESSED_MAGIC)) == 0) {
        data = mainHeapAllocBest(file->size);
        decompressStreamTo(data, (u8 *)(file + 1));
    } else {
        data = mainHeapAllocBest(size);
        memmove(data, buffer, size);
    }
    mainHeapFree(file);
    *(void **)user = data;
}

/*
 * Inflates a loaded file of size bytes, if it is compressed, into memory from
 * mainHeapAllocBest, freeing it. Returns the data: file itself if it is not
 * compressed.
 */
void *decompressFileIfCompressed(CompressedHeader *file, s32 size) {
    s32 streamSize;
    s32 inflatedSize;
    u8 *stream;
    void *data;

    if (func_8003DF40(file, COMPRESSED_MAGIC, sizeof(COMPRESSED_MAGIC)) != 0) {
        return file;
    }
    streamSize = size - sizeof(CompressedHeader);
    inflatedSize = file->size;
    /* move the stream out of the way of the inflated data */
    stream = mainHeapAllocLargest(streamSize);
    memmove(stream, (u8 *)(file + 1), streamSize);
    mainHeapFree(file);
    data = mainHeapAllocBest(inflatedSize);
    decompressStreamTo(data, stream);
    mainHeapFree(stream);
    return data;
}

/* Inflates a compressed file into memory from mainHeapAllocBest; returns the data.
 * The magic is compared but not checked. */
void *decompressFile(CompressedHeader *file) {
    void *data;

    func_8003DF40(file, COMPRESSED_MAGIC, sizeof(COMPRESSED_MAGIC));
    data = mainHeapAllocBest(file->size);
    decompressStreamTo(data, (u8 *)(file + 1));
    return data;
}

/* Inflates the zlib stream at src into dst; returns dst. */
void *decompressStreamTo(void *dst, u8 *src) {
    decompressStream(dst, src);
    return dst;
}

/* Returns whether a loaded file is compressed. */
s32 isFileCompressed(CompressedHeader *file) {
    return func_8003DF40(file, COMPRESSED_MAGIC, sizeof(COMPRESSED_MAGIC)) == 0;
}
