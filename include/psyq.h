#ifndef PSYQ_H
#define PSYQ_H

/* The PsyQ functions the SDK's headers lack: libsnd, libspu, libcd, the BIOS and more. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libcd.h"
#include "libspu.h"

EXTERN_C_BEGIN

s32 func_800402B0();
s32 func_80046280(s32, s32);
s32 func_80046304();
s32 func_80046930(s32 *, s32);
s32 func_80046DC0(s32 *);
s32 func_80046E00(s32 *);
s32 func_800479E0(s32);
s32 func_8004BEA0();
s32 func_8004D4F0(s32, s32, u8, u8);
/* the unnamed libsnd and libspu functions the sound system calls; the names
 * in the comments are guesses from what they do */
/* SpuGetAllKeysStatus */
void func_80047FC4(u8 *status);
/* SpuGetVoiceAttr */
void func_80048BF0(SpuVoiceAttr *attr);
s32 func_80047CF0(s32);
/* SsSepClose */
void func_80049180(s16 sep);
/* SsIsEos */
s16 func_80049460(s16 sep, s16 seq);
/* SsSepStop */
void func_8004D038(s16 sep, s16 seq);
/* SsSepSetVol */
void func_8004D638(s16 sep, s16 seq, s16 volLeft, s16 volRight);
/* SsUtReverbOff */
void func_8004F120(void);
/* SsUtReverbOn */
void func_8004F140(void);
/* SsUtSetVVol */
void func_8004F330(s16 voice, s16 volLeft, s16 volRight);
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
/* ChangeTh */
s32 func_80040320(s32 id);
/* Krom2RawAdd: the address of the glyph of a Shift JIS character in the BIOS
 * ROM; it takes the code unmasked */
u8 *func_80040390(u32 code);
/* MemCardInit: InitCARD(shared), StartCARD and _bu_init */
void func_80058740(s32 shared);
/* ApplyMatrix */
VECTOR *func_80053DF0(MATRIX *m, SVECTOR *v, VECTOR *r);
/* CloseTh */
s32 func_80040310(s32 id);
/* EnterCriticalSection */
void func_80040360(void);
/* ExitCriticalSection */
void func_80040370(void);
/* StoreImage */
s32 func_80058084(RECT *rect, u_long *dest);
/* StopCallback */
void func_8003F874(void);
/* _96_remove: takes the CD-ROM driver out of the BIOS, before LoadExec */
void func_800402D0(void);
/* LoadExec: runs another executable with the stack at sp */
void func_800402C0(char *file, u32 sp, u32 size);
/* SquareRoot12: the square root of a 4.12 fixed-point number */
s32 func_80053D50(s32 a);
/* SsSetStereo */
void func_80052A90(void);
/* SsSetMono */
void func_80052AA0(void);

/* libcd */
/* CdFlush */
void func_800468A0(void);

/* CdIntToPos */
CdlLOC *func_80046B60(s32 sector, CdlLOC *loc);

/* CdPosToInt */
s32 func_80046C70(CdlLOC *loc);

/* sends a command and waits for it, trying up to retries times */
s32 func_80043724(u8 command, void *param, u8 *result, s32 retries);

/* seeks to loc in mode, then sends command, callback on its interrupts */
s32 func_80043998(u8 mode, CdlLOC *loc, u8 command, void (*callback)(u8 intr), s32 retries);

/* CdInit */
s32 func_800434D4(void);

/* CdReset */
void func_80043618(void);

/* CdDiskReady */
s32 func_800468E0(void);

/* GetVideoMode: MODE_NTSC or MODE_PAL */
s32 func_80040294(void);

/* PsyQ LoadImage, StoreImage or MoveImage (identical bodies) */
s32 func_80055B28(RECT *rect, u_long *p);

/* LoadImage2 */
s32 func_80057F98(RECT *rect, u_long *p);

/* SetDrawLoad */
void func_80058540(DR_LOAD *p, RECT *rect);

EXTERN_C_END

#endif /* PSYQ_H */
