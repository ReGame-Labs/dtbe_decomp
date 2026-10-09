#ifndef DTBE_GAME_CHARACTER_H
#define DTBE_GAME_CHARACTER_H

/* The characters: each one's model, animations and tinted textures, from /chara. */

#include "common.h"
#include <libgpu.h>
#include "engine/gfx/animator.h"
#include "engine/gfx/color.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"

EXTERN_C_BEGIN

/* A loaded file, and whether it is ours to free. */
typedef struct {
    /* 0x0 */ s32 owned;
    /* 0x4 */ void *file;
} FileRef;

/* What a .loop file holds for each clip. */
typedef struct {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
} CharaLoop;

/* The files of a character packed in one. Its offsets are relocated to
 * pointers on loading; the TIMs are dropped once they are in VRAM. */
typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ AnimData *mop;
    /* 0x0C */ void *unkC;
    /* 0x10 */ void *unk10; /* may be absent (0) */
    /* 0x14 */ TmdHeader *tmd;
    /* 0x18 */ u32 *tims;
    /* 0x1C */ CharaLoop *loops;
} CharaPack;

/* The model, animations and textures of a character, /chara/<id>. */
typedef struct {
    /* 0x00 */ ListNode link;  /* in LOADED_CHARAS, the loaded characters */
    /* 0x08 */ s32 id;
    /* 0x0C */ FileRef tmd;    /* .tmd */
    /* 0x14 */ FileRef mop;    /* .mop, an AnimData */
    /* 0x1C */ FileRef loop;   /* .loop, CharaLoops */
    /* 0x24 */ CharaPack *pack; /* in place of the three, if not NULL */
    /* 0x28 */ s32 lowerHalf;  /* the textures go 256 lines down in VRAM */
    /* 0x2C */ HsvColor tint;  /* applied to the textures */
} Chara;

/* the sat or val of an HsvColor tint that leaves them as they are (n / 128) */
#define TINT_SCALE_ONE 0x80

extern char CHARA_PATH_FORMAT[]; /* "/chara/%02d%s" */

/* the loaded characters */
#ifdef __cplusplus
extern List LOADED_CHARAS;
#else
extern ListNode LOADED_CHARAS;
#endif

FileRef *fileRefInit(FileRef *fileRef, void *file);
void fileRefDestroy(FileRef *fileRef, s32 flags);
FileRef *fileRefSetBorrowed(FileRef *fileRef, FileRef *other);
FileRef *fileRefSetOwned(FileRef *fileRef, void *file);
Chara *charaInit(Chara *chara, s32 id);
void charaDestroy(Chara *chara, s32 flags);
Chara *charaFindSameId(Chara *chara);
void charaSetPlayerTint(Chara *chara, s32 player, HsvColor *tint);
void charaLoad(Chara *chara);
void *charaLoadFile(Chara *chara, char *ext);
void *charaLoadRawFile(Chara *chara, char *ext);
void charaLoadTims(Chara *chara, u32 *tims);
void charaLoadPack(Chara *chara, char *path, s32 unused, RECT *bounds);
TmdHeader *charaGetTmd(Chara *chara);
AnimData *charaGetAnimData(Chara *chara);
u16 func_80031258(Chara *chara, s32 clip);
u16 func_80031298(Chara *chara, s32 clip);
s32 func_800312D8(Chara *chara);
s32 func_80031300(void);
s32 func_80031308(void);
void tintTim(TIM_IMAGE *tim, HsvColor *tint);


EXTERN_C_END

#endif /* DTBE_GAME_CHARACTER_H */
