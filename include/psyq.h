#ifndef PSYQ_H
#define PSYQ_H

/* The PsyQ headers the game uses, and the PsyQ function they declare without a prototype. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libcd.h"
/*
 * The CD-ROM functions the game calls are libds's rather than libcd's (the
 * twins of the two libraries have the same code): the commands go through a
 * queue of 8 with retry counts, which libds has and libcd lacks, and the
 * game's message after DsSystemStatus names libds's DslReady.
 */
#include "libds.h"
#include "libspu.h"
#include "libsnd.h"
#include "libetc.h"

EXTERN_C_BEGIN

/* libc's: memory.h declares it without its parameters (the prototype is the
 * one GCC has for its builtin) */
s32 memcmp(const void *a, const void *b, u32 size);

EXTERN_C_END

#endif /* PSYQ_H */
