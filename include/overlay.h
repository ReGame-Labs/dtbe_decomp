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
struct Task *func_80064494(void *memory);
struct Task *func_80065AE4(void *memory, struct SystemContext *context);
struct Task *func_80066084(void *memory, struct SystemContext *context);
struct Task *func_80066D58(void *memory, struct SystemContext *context);
struct Task *func_80066F74(void *memory, struct SystemContext *context);
struct Task *func_80067170(void *memory, struct SystemContext *context);
struct Task *func_800673A4(void *memory, struct SystemContext *context);
struct Task *func_800681A4(void *memory, struct SystemContext *context);
struct Task *func_8006A360(void *memory, s32 arg);
struct Task *func_8006A43C(void *memory, struct SystemContext *context);
struct Task *func_8006AAF8(void *memory, struct SystemContext *context, s32 arg);
struct Task *func_8006E1CC(void *memory, struct SystemContext *context);
struct Task *func_800709B0(void *memory, s32 arg);
struct Task *func_80071F28(void *memory, struct SystemContext *context, s32 arg);
struct Task *func_8007360C(void *memory, struct SystemContext *context, s32 arg);
struct Task *func_80076504(void *memory, struct SystemContext *context);
struct Task *func_800794E0(void *memory);
/* called on the list node of a MeshSceneLink that is in a list */
void func_80076A5C(struct ListNode *node);

EXTERN_C_END

/* where the overlays are loaded, over the end of the .bss */
extern u8 D_800643E0[];

#endif /* OVERLAY_H */
