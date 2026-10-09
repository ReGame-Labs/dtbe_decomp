#include "common.h"
#include "engine/game/character.h"
#include "engine/cd/file_load.h"
#include "engine/gfx/animator.h"
#include "engine/gfx/color.h"
#include "engine/gfx/tmd.h"
#include "engine/lib/list.h"
#include "engine/system/memory.h"
#include "vtable.h"
#include "psyq.h"
#include "stdio.h"

/* Refers to file, which is ours to free unless it is NULL; returns fileRef. */
FileRef *fileRefInit(FileRef *fileRef, void *file) {
    fileRef->owned = file != NULL;
    fileRef->file = file;
    return fileRef;
}

/* Destroys the reference, freeing the file if it is ours, and frees the
 * reference itself if flags has DESTROY_FREE. */
void fileRefDestroy(FileRef *fileRef, s32 flags) {
    if (fileRef->owned) {
        mainHeapFree(fileRef->file);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(fileRef);
    }
}

/* Refers to the file of other, which stays other's to free; returns fileRef. */
FileRef *fileRefSetBorrowed(FileRef *fileRef, FileRef *other) {
    fileRef->owned = 0;
    fileRef->file = other->file;
    return fileRef;
}

/* Refers to file, which is ours to free; returns fileRef. */
FileRef *fileRefSetOwned(FileRef *fileRef, void *file) {
    fileRef->owned = 1;
    fileRef->file = file;
    return fileRef;
}

/* the loaded characters */
List LOADED_CHARAS;

/* Builds a character with no file loaded, untinted, and adds it to the
 * loaded characters. */
Chara *charaInit(Chara *chara, s32 id) {
    ListNode *head;

    chara->link.next = &chara->link;
    chara->link.prev = &chara->link;
    fileRefInit(&chara->tmd, NULL);
    fileRefInit(&chara->mop, NULL);
    fileRefInit(&chara->loop, NULL);
    chara->id = id;
    chara->pack = NULL;
    chara->lowerHalf = 0;
    chara->tint.hue = 0;
    chara->tint.sat = TINT_SCALE_ONE;
    chara->tint.val = TINT_SCALE_ONE;
    /* add it at the end of the loaded characters */
    head = &LOADED_CHARAS;
    chara->link.next = head;
    chara->link.prev = LOADED_CHARAS.prev;
    head->prev->next = &chara->link;
    LOADED_CHARAS.prev = &chara->link;
    return chara;
}

/* Destroys the character, freeing its files, and frees it if flags has
 * DESTROY_FREE. */
void charaDestroy(Chara *chara, s32 flags) {
    mainHeapFree(chara->pack);
    fileRefDestroy(&chara->loop, DESTROY_DELETE & ~DESTROY_FREE);
    fileRefDestroy(&chara->mop, DESTROY_DELETE & ~DESTROY_FREE);
    fileRefDestroy(&chara->tmd, DESTROY_DELETE & ~DESTROY_FREE);
    chara->link.next->prev = chara->link.prev;
    chara->link.prev->next = chara->link.next;
    chara->link.next = &chara->link;
    chara->link.prev = &chara->link;
    if (flags & DESTROY_FREE) {
        operatorDelete(chara);
    }
}

/* Another loaded character with the same id, or NULL. */
Chara *charaFindSameId(Chara *chara) {
    ListNode *head = &LOADED_CHARAS;
    ListNode *node;

    for (node = head->next; node != head; node = node->next) {
        if ((Chara *)node != chara && ((Chara *)node)->id == chara->id) {
            return (Chara *)node;
        }
    }
    return NULL;
}

/* Sets where the textures go (player 1 puts them in the lower half of VRAM)
 * and the tint they get. */
void charaSetPlayerTint(Chara *chara, s32 player, HsvColor *tint) {
    chara->lowerHalf = player == 1;
    chara->tint = *tint;
}

/* Loads the character's files: puts its textures in VRAM, shares the
 * animations of a character with the same id already loaded, if any, and
 * loads its model. */
void charaLoad(Chara *chara) {
    FileRef tims;
    Chara *other;

    fileRefInit(&tims, charaLoadFile(chara, ".tim"));
    charaLoadTims(chara, (u32 *)tims.file);
    fileRefDestroy(&tims, DESTROY_DELETE & ~DESTROY_FREE);
    other = charaFindSameId(chara);
    if (other != NULL && other->mop.file != NULL) {
        fileRefSetBorrowed(&chara->mop, &other->mop);
        fileRefSetBorrowed(&chara->loop, &other->loop);
    } else {
        fileRefSetOwned(&chara->mop, charaLoadRawFile(chara, ".mop"));
        fileRefSetOwned(&chara->loop, charaLoadFile(chara, ".loop"));
    }
    fileRefSetOwned(&chara->tmd, charaLoadFile(chara, ".tmd"));
    tmdHeaderRelocate((TmdHeader *)chara->tmd.file);
    if (chara->lowerHalf) {
        tmdHeaderMoveTexturesDown((TmdHeader *)chara->tmd.file);
    }
}

/* Loads the character's file with extension ext. */
void *charaLoadFile(Chara *chara, char *ext) {
    char path[32];

    sprintf(path, CHARA_PATH_FORMAT, chara->id, ext);
    return loadFile(path);
}

/* Loads the character's file with extension ext, with loadRawFile. */
void *charaLoadRawFile(Chara *chara, char *ext) {
    char path[32];

    sprintf(path, CHARA_PATH_FORMAT, chara->id, ext);
    return loadRawFile(path);
}

/* Puts the run of TIMs tims in VRAM, moved and tinted as the character
 * wants. */
void charaLoadTims(Chara *chara, u32 *tims) {
    TIM_IMAGE tim;

    func_800585B0(tims);
    while (ReadTIM(&tim) != NULL) {
        if (chara->lowerHalf) {
            tim.crect->y += 256;
            tim.prect->y += 256;
        }
        if (chara->tint.val != 0) {
            tintTim(&tim, &chara->tint);
        }
        if (tim.caddr != NULL) {
            func_80057F98(tim.crect, tim.caddr);
        }
        if (tim.paddr != NULL) {
            func_80057F98(tim.prect, tim.paddr);
        }
    }
}

/* Pointers in a CharaPack are offsets from its start until relocated. */
#define RELOCATE(pack, offset) ((s32)(offset) + (s32)(pack))

/* Loads the character from the CharaPack at path: puts its textures in VRAM,
 * frees them, and prepares its model. If bounds is not NULL, it gets the
 * rectangle around the CLUTs of the textures. */
void charaLoadPack(Chara *chara, char *path, s32 unused, RECT *bounds) {
    CharaPack *pack = (CharaPack *)loadRawFile(path);
    TmdHeader *tmd;

    chara->pack = pack;
    pack->mop = (AnimData *)RELOCATE(pack, pack->mop);
    pack->unkC = (void *)RELOCATE(pack, pack->unkC);
    if (pack->unk10 != NULL) {
        pack->unk10 = (void *)RELOCATE(pack, pack->unk10);
    }
    pack->tmd = (TmdHeader *)RELOCATE(pack, pack->tmd);
    pack->tims = (u32 *)RELOCATE(pack, pack->tims);
    pack->loops = (CharaLoop *)RELOCATE(pack, pack->loops);
    if (bounds != NULL) {
        s32 left = 0x7FFF;
        s32 top = 0x7FFF;
        s32 right = -0x8000;
        s32 bottom = -0x8000;
        TIM_IMAGE tim;

        func_800585B0(pack->tims);
        while (ReadTIM(&tim) != NULL) {
            if (tim.caddr != NULL) {
                RECT *clut = tim.crect;

                if (clut->x < left) {
                    left = clut->x;
                }
                if (clut->y < top) {
                    top = clut->y;
                }
                if (clut->x + clut->w > right) {
                    right = clut->x + clut->w;
                }
                if (clut->y + clut->h > bottom) {
                    bottom = clut->y + clut->h;
                }
            }
        }
        bounds->x = left;
        bounds->y = top;
        bounds->w = right - left;
        bounds->h = bottom - top;
    }
    charaLoadTims(chara, pack->tims);
    /* the TIMs come last: drop them */
    mainHeapShrink(pack, (u8 *)pack->tims - (u8 *)pack);
    tmd = pack->tmd;
    tmdHeaderRelocate(tmd);
    if (chara->lowerHalf) {
        tmdHeaderMoveTexturesDown(tmd);
    }
}

/* The character's model. */
TmdHeader *charaGetTmd(Chara *chara) {
    if (chara->pack != NULL) {
        return chara->pack->tmd;
    }
    return (TmdHeader *)chara->tmd.file;
}

/* The character's animations. */
AnimData *charaGetAnimData(Chara *chara) {
    if (chara->pack != NULL) {
        return chara->pack->mop;
    }
    return (AnimData *)chara->mop.file;
}

/* The first value of the .loop for clip. */
u16 func_80031258(Chara *chara, s32 clip) {
    if (chara->pack != NULL) {
        return chara->pack->loops[clip].unk0;
    }
    /* a FileRef holds a file of any kind */
    return ((CharaLoop *)chara->loop.file)[clip].unk0;
}

/* The second value of the .loop for clip. */
u16 func_80031298(Chara *chara, s32 clip) {
    if (chara->pack != NULL) {
        return chara->pack->loops[clip].unk2;
    }
    /* a FileRef holds a file of any kind */
    return ((CharaLoop *)chara->loop.file)[clip].unk2;
}

/* What tmdHeaderGetObjectCount gives for the character's model. */
s32 func_800312D8(Chara *chara) {
    return tmdHeaderGetObjectCount(charaGetTmd(chara));
}

/* Returns 0. */
s32 func_80031300(void) {
    return 0;
}

/* Returns 0. */
s32 func_80031308(void) {
    return 0;
}

/*
 * Shifts the colors of a TIM's clut, and of its pixels when they are 16-bit
 * (mode 2), by tint (shiftColors). It runs on a stack in the scratchpad: it
 * saves $fp and $sp at 0x1F8003F8 and points both there, which C cannot
 * express (the original used inline asm for it).
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/character", tintTim);

INCLUDE_RODATA("asm/jp/main/nonmatchings/game/character", CHARA_PATH_FORMAT);
