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

/* Refers to file, which is ours to free unless it is NULL; returns this. */
FileRef *fileRefInit(FileRef *this, void *file) {
    this->owned = file != NULL;
    this->file = file;
    return this;
}

/* Destroys the reference, freeing the file if it is ours, and frees the
 * reference itself if flags has DESTROY_FREE. */
void fileRefDestroy(FileRef *this, s32 flags) {
    if (this->owned) {
        mainHeapFree(this->file);
    }
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Refers to the file of other, which stays other's to free; returns this. */
FileRef *fileRefSetBorrowed(FileRef *this, FileRef *other) {
    this->owned = 0;
    this->file = other->file;
    return this;
}

/* Refers to file, which is ours to free; returns this. */
FileRef *fileRefSetOwned(FileRef *this, void *file) {
    this->owned = 1;
    this->file = file;
    return this;
}

/* the loaded characters */
ListNode LOADED_CHARAS = { NULL, NULL };

/* Builds a character with no file loaded, untinted, and adds it to the
 * loaded characters. */
Chara *charaInit(Chara *this, s32 id) {
    ListNode *head;

    this->link.next = &this->link;
    this->link.prev = &this->link;
    fileRefInit(&this->tmd, NULL);
    fileRefInit(&this->mop, NULL);
    fileRefInit(&this->loop, NULL);
    this->id = id;
    this->pack = NULL;
    this->lowerHalf = 0;
    this->tint.hue = 0;
    this->tint.sat = TINT_SCALE_ONE;
    this->tint.val = TINT_SCALE_ONE;
    /* add it at the end of the loaded characters */
    head = &LOADED_CHARAS;
    this->link.next = head;
    this->link.prev = LOADED_CHARAS.prev;
    head->prev->next = &this->link;
    LOADED_CHARAS.prev = &this->link;
    return this;
}

/* Destroys the character, freeing its files, and frees it if flags has
 * DESTROY_FREE. */
void charaDestroy(Chara *this, s32 flags) {
    mainHeapFree(this->pack);
    fileRefDestroy(&this->loop, DESTROY_DELETE & ~DESTROY_FREE);
    fileRefDestroy(&this->mop, DESTROY_DELETE & ~DESTROY_FREE);
    fileRefDestroy(&this->tmd, DESTROY_DELETE & ~DESTROY_FREE);
    this->link.next->prev = this->link.prev;
    this->link.prev->next = this->link.next;
    this->link.next = &this->link;
    this->link.prev = &this->link;
    if (flags & DESTROY_FREE) {
        operatorDelete(this);
    }
}

/* Another loaded character with the same id, or NULL. */
Chara *charaFindSameId(Chara *this) {
    ListNode *head = &LOADED_CHARAS;
    ListNode *node;

    for (node = head->next; node != head; node = node->next) {
        if ((Chara *)node != this && ((Chara *)node)->id == this->id) {
            return (Chara *)node;
        }
    }
    return NULL;
}

/* Sets where the textures go (player 1 puts them in the lower half of VRAM)
 * and the tint they get. */
void charaSetPlayerTint(Chara *this, s32 player, HsvColor *tint) {
    this->lowerHalf = player == 1;
    this->tint = *tint;
}

/* Loads the character's files: puts its textures in VRAM, shares the
 * animations of a character with the same id already loaded, if any, and
 * loads its model. */
void charaLoad(Chara *this) {
    FileRef tims;
    Chara *other;

    fileRefInit(&tims, charaLoadFile(this, ".tim"));
    charaLoadTims(this, tims.file);
    fileRefDestroy(&tims, DESTROY_DELETE & ~DESTROY_FREE);
    other = charaFindSameId(this);
    if (other != NULL && other->mop.file != NULL) {
        fileRefSetBorrowed(&this->mop, &other->mop);
        fileRefSetBorrowed(&this->loop, &other->loop);
    } else {
        fileRefSetOwned(&this->mop, charaLoadRawFile(this, ".mop"));
        fileRefSetOwned(&this->loop, charaLoadFile(this, ".loop"));
    }
    fileRefSetOwned(&this->tmd, charaLoadFile(this, ".tmd"));
    tmdHeaderRelocate(this->tmd.file);
    if (this->lowerHalf) {
        tmdHeaderMoveTexturesDown(this->tmd.file);
    }
}

/* Loads the character's file with extension ext. */
void *charaLoadFile(Chara *this, char *ext) {
    char path[32];

    sprintf(path, CHARA_PATH_FORMAT, this->id, ext);
    return loadFile(path);
}

/* Loads the character's file with extension ext, with loadRawFile. */
void *charaLoadRawFile(Chara *this, char *ext) {
    char path[32];

    sprintf(path, CHARA_PATH_FORMAT, this->id, ext);
    return loadRawFile(path);
}

/* Puts the run of TIMs tims in VRAM, moved and tinted as the character
 * wants. */
void charaLoadTims(Chara *this, u32 *tims) {
    TIM_IMAGE tim;

    func_800585B0(tims);
    while (ReadTIM(&tim) != NULL) {
        if (this->lowerHalf) {
            tim.crect->y += 256;
            tim.prect->y += 256;
        }
        if (this->tint.val != 0) {
            tintTim(&tim, &this->tint);
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
void charaLoadPack(Chara *this, char *path, s32 unused, RECT *bounds) {
    CharaPack *pack = loadRawFile(path);
    TmdHeader *tmd;

    this->pack = pack;
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
    charaLoadTims(this, pack->tims);
    /* the TIMs come last: drop them */
    mainHeapShrink(pack, (u8 *)pack->tims - (u8 *)pack);
    tmd = pack->tmd;
    tmdHeaderRelocate(tmd);
    if (this->lowerHalf) {
        tmdHeaderMoveTexturesDown(tmd);
    }
}

/* The character's model. */
TmdHeader *charaGetTmd(Chara *this) {
    if (this->pack != NULL) {
        return this->pack->tmd;
    }
    return this->tmd.file;
}

/* The character's animations. */
AnimData *charaGetAnimData(Chara *this) {
    if (this->pack != NULL) {
        return this->pack->mop;
    }
    return this->mop.file;
}

/* The first value of the .loop for clip. */
u16 func_80031258(Chara *this, s32 clip) {
    if (this->pack != NULL) {
        return this->pack->loops[clip].unk0;
    }
    /* a FileRef holds a file of any kind */
    return ((CharaLoop *)this->loop.file)[clip].unk0;
}

/* The second value of the .loop for clip. */
u16 func_80031298(Chara *this, s32 clip) {
    if (this->pack != NULL) {
        return this->pack->loops[clip].unk2;
    }
    /* a FileRef holds a file of any kind */
    return ((CharaLoop *)this->loop.file)[clip].unk2;
}

/* What tmdHeaderGetObjectCount gives for the character's model. */
s32 func_800312D8(Chara *this) {
    return tmdHeaderGetObjectCount(charaGetTmd(this));
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

/*
 * g++'s constructor and destructor of the list of loaded characters
 * (__static_initialization_and_destruction_0). The 16-byte frame comes from
 * the inlined deleting destructor of the list's owner, whose dead call to
 * operator delete C has no faithful equivalent for.
 */
INCLUDE_ASM("asm/jp/main/nonmatchings/game/character", initOrDestroyCharaList);

void initCharaList(void) {
    initOrDestroyCharaList(1, 0xFFFF);
}

void destroyCharaList(void) {
    initOrDestroyCharaList(0, 0xFFFF);
}
