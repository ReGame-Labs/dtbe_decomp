#ifndef PSYQ_H
#define PSYQ_H

/* The PsyQ functions the SDK's headers lack: libsnd, libspu, libcd, the BIOS and more. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libcd.h"
#include "libspu.h"

EXTERN_C_BEGIN

s32 func_800479E0(s32);
s32 func_8004D4F0(s32, s32, u8, u8);
/* the unnamed libsnd and libspu functions the sound system calls; the names
 * in the comments are guesses from what they do */
/* SpuGetAllKeysStatus */
void func_80047FC4(u8 *status);
/* SpuGetVoiceAttr */
void func_80048BF0(SpuVoiceAttr *attr);
s32 func_80047CF0(s32);
/* SsIsEos */
s16 func_80049460(s16 sep, s16 seq);
/* SsUtSetVVol */
void func_8004F330(s16 voice, s16 volumeLeft, s16 volumeRight);
/* SsSetReservedVoice */
void func_80052AB0(s32 voices);
/* opens the head of a VAB (like SsVabOpenHead) */
s16 func_80052F10(void *header, s16 vabId);
/* OpenTIM */
s32 func_800585B0(u32 *addr);
/* ResetCallback: it calls the fourth function of libetc's interrupt table */
s32 func_8003F780(void);
/* memcmp */
s32 func_8003DF40(void *a, void *b, s32 size);
/* MemCardInit: InitCARD(shared), StartCARD and _bu_init */
void func_80058740(s32 shared);
/* ApplyMatrix */
VECTOR *func_80053DF0(MATRIX *m, SVECTOR *v, VECTOR *r);
/* StoreImage */
s32 func_80058084(RECT *rect, u_long *dst);
/* StopCallback */
void func_8003F874(void);
/* SquareRoot12: the square root of a 4.12 fixed-point number */
s32 func_80053D50(s32 a);
/* SsSetStereo */
void func_80052A90(void);
/* SsSetMono */
void func_80052AA0(void);

/*
 * The CD-ROM functions the game calls. They are libds's rather than libcd's
 * (the twins of the two libraries have the same code): the commands go
 * through a queue of 8 with retry counts, which libds has and libcd lacks,
 * and the game's message after func_800468E0 names libds's DslReady. The
 * names in the comments are the libds functions of the same arguments.
 */

/* DsFlush */
void func_800468A0(void);

/* DsIntToPos (CdIntToPos's code is the same, at 0x80042C40) */
CdlLOC *func_80046B60(s32 sector, CdlLOC *loc);

/* DsPosToInt (CdPosToInt's code is the same, at 0x800408F0) */
s32 func_80046C70(CdlLOC *loc);

/* DsCommand: queues a command and waits for it, trying up to retries times */
s32 func_80043724(u8 command, void *param, u8 *result, s32 retries);

/* DsPacket: seeks to loc in mode, then sends command, callback on its
 * interrupts */
s32 func_80043998(u8 mode, CdlLOC *loc, u8 command, void (*callback)(u8 intr), s32 retries);

/* DsInit */
s32 func_800434D4(void);

/* DsReset */
void func_80043618(void);

/* DsSystemStatus: 1 (DslReady) once the drive is ready, else 2 (DslBusy) */
s32 func_800468E0(void);

/* DsStartReadySystem: hands the data ready interrupts to ready */
s32 func_80046280(void (*ready)(u8 intr), s32 retries);

/* DsEndReadySystem: pauses the drive and gives the callbacks back */
void func_80046304(void);

/* DsGetSector: copies words of the sector the drive read to dst */
s32 func_80046930(void *dst, s32 words);

/* DsReadyCallback: sets the data ready callback, returns the old one */
void (*func_80046DC0(void (*ready)(u8 intr)))(u8 intr);

/* DsDataCallback: the CD-ROM DMA's callback, DMACallback(3, done) */
void (*func_80046E00(void (*done)(void)))(void);

/* PsyQ LoadImage, StoreImage or MoveImage (identical bodies) */
s32 func_80055B28(RECT *rect, u_long *p);

/* LoadImage2 */
s32 func_80057F98(RECT *rect, u_long *p);

/* SetDrawLoad */
void func_80058540(DR_LOAD *p, RECT *rect);

EXTERN_C_END

#endif /* PSYQ_H */
