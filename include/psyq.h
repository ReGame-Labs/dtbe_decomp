#ifndef PSYQ_H
#define PSYQ_H

/* The PsyQ headers the game uses, and the PsyQ functions they lack. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libcd.h"
#include "libds.h"
#include "libspu.h"
#include "libsnd.h"
#include "libetc.h"

EXTERN_C_BEGIN

s32 func_800479E0(s32);
/* the unnamed libsnd and libspu functions the sound system calls; the names
 * in the comments are guesses from what they do */
s32 func_80047CF0(s32);
/* SsIsEos */
s16 func_80049460(s16 sep, s16 seq);
/* SsSetReservedVoice */
void func_80052AB0(s32 voices);
/* opens the head of a VAB (like SsVabOpenHead) */
s16 func_80052F10(void *header, s16 vabId);
/* memcmp or bcmp: libc's two have the same code */
s32 func_8003DF40(void *a, void *b, s32 size);
/* SsSetStereo */
void func_80052A90(void);
/* SsSetMono */
void func_80052AA0(void);

/*
 * The CD-ROM functions the game calls are libds's rather than libcd's (the
 * twins of the two libraries have the same code): the commands go through a
 * queue of 8 with retry counts, which libds has and libcd lacks, and the
 * game's message after DsSystemStatus names libds's DslReady.
 */

/* DsGetSector: copies words of the sector the drive read to dst */
s32 func_80046930(void *dst, s32 words);

EXTERN_C_END

#endif /* PSYQ_H */
