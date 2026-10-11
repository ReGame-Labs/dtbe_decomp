#ifndef OVERLAY_H
#define OVERLAY_H

/* What the code overlays define: their tasks' constructors, and where they are loaded. */

#include "common.h"

struct ListNode;
struct SystemContext;
struct Task;

EXTERN_C_BEGIN

/*
 * The constructors of the tasks the overlays define: each builds its task in
 * memory from the main heap (operatorNew) and returns it.
 */
struct Task *movieTaskInit(void *memory);
struct Task *func_80065AE4(void *memory, struct SystemContext *context);
struct Task *func_80066084(void *memory, struct SystemContext *context);
struct Task *bootLogoTaskInit(void *memory, struct SystemContext *context);
struct Task *func_80066F74(void *memory, struct SystemContext *context);
struct Task *titleLogoTaskInit(void *memory, struct SystemContext *context);
/* The title's task size: the USA release's task is 0x30 bytes bigger. */
#if VERSION_JP
#define TITLE_LOGO_TASK_SIZE 0x110C
#else
#define TITLE_LOGO_TASK_SIZE 0x113C
#endif
struct Task *func_800673A4(void *memory, struct SystemContext *context);
struct Task *func_800681A4(void *memory, struct SystemContext *context);
struct Task *mainMenuTaskInit(void *memory, s32 arg);
struct Task *func_8006A43C(void *memory, struct SystemContext *context);
struct Task *characterSelectTaskInit(void *memory, struct SystemContext *context, s32 arg);
struct Task *gameMainTaskInit(void *memory, struct SystemContext *context);
struct Task *memoryCardTaskInit(void *memory, s32 arg);
struct Task *guideLogoTaskInit(void *memory, struct SystemContext *context, u16 arg);
struct Task *rankingTaskInit(void *memory, struct SystemContext *context, s32 arg);
struct Task *creditsTaskInit(void *memory, struct SystemContext *context);
struct Task *optionTaskInit(void *memory);
/* called on the list node of a MeshSceneLink that is in a list */
void func_80076A5C(struct ListNode *node);

EXTERN_C_END

/* where the overlays are loaded: the start of the game's .bss, the part crt0
 * does not clear (it clears from 0x800DA320 on) */
extern u8 OVERLAY_MEMORY[];

#endif /* OVERLAY_H */
