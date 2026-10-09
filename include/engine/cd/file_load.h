#ifndef DTBE_CD_FILE_LOAD_H
#define DTBE_CD_FILE_LOAD_H

/* Loading files into memory, inflating the compressed ones, and putting TIMs in VRAM. */

#include "common.h"

EXTERN_C_BEGIN

/* The first word of a TIM, its version byte masked out */
#define TIM_ID_MASK 0xFFFF00FF
#define TIM_ID 0x10
/* The flags of a TIM, its second word */
#define TIM_HAS_CLUT 8
/* The header of a compressed file; the zlib stream follows it. */
typedef struct {
    char magic[4]; /* COMPRESSED_MAGIC */
    s32 size;      /* the inflated size */
} CompressedHeader;

/* a symbol the linker puts at address 1, so its address is never NULL */
extern u8 D_1[];

extern char LAST_LOADED_PATH[]; /* the path of the file loadCompressedFileInto loaded last */

void loadCompressedFileInto(void *dest, char *path);
void uploadTim(u32 *tim);
void uploadTims(u32 *tims);
u32 *findTim(u32 *tim, s32 index);
void *loadFile(char *path);
void *loadFileLargest(char *path);
void *loadRawFile(char *path);
void *loadRawFileLargest(char *path);
void *loadFileWith(char *path, void *(*allocFile)(s32), void *(*allocData)(s32));
void *loadRawFileWith(char *path, void *(*alloc)(s32));
s32 loadFileAsync(char *path, void **result);
/* a CdfsDoneFunc: user is the result pointer of loadFileAsync */
void finishLoadFileAsync(void *buffer, u32 size, char *name, s32 user);
void *decompressFileIfCompressed(CompressedHeader *file, s32 size);
void *decompressFile(CompressedHeader *file);
void *decompressStreamTo(void *dest, u8 *source);

s32 isFileCompressed(CompressedHeader *file);

EXTERN_C_END

#endif /* DTBE_CD_FILE_LOAD_H */
