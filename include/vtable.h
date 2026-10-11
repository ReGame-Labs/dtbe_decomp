#ifndef VTABLE_H
#define VTABLE_H

/* The virtual tables of g++ 2.95, as the C files see them: their entries, destructor flags, static init priority. */

#include "common.h"

EXTERN_C_BEGIN

/* An entry of a g++ 2.95 virtual table: how far to move `this` to reach the
 * object the function belongs to, and the function. */
typedef struct {
    /* 0x0 */ s16 delta;
    /* 0x2 */ s16 index;
#ifdef __cplusplus
    /* 0x4 */ void (*func)(...); /* C++ has no unprototyped functions */
#else
    /* 0x4 */ void (*func)();
#endif
} VtableEntry;

/* An entry of a g++ 2.95 virtual table whose function returns a value. */
typedef struct {
    /* 0x0 */ s16 delta;
    /* 0x2 */ s16 index;
#ifdef __cplusplus
    /* 0x4 */ s32 (*func)(...);
#else
    /* 0x4 */ s32 (*func)();
#endif
} VtableEntryS32;

/* the flags a deleting destructor is called with */
#define DESTROY_FREE 1   /* free the object's memory afterwards */
#define DESTROY_BASES 2  /* destroy the virtual bases too: a whole object */
#define DESTROY_DELETE 3 /* what `delete` passes */

/* the priority of the global objects without init_priority (g++'s
 * DEFAULT_INIT_PRIORITY): a file's _GLOBAL_.I and _GLOBAL_.D pass it to its
 * __static_initialization_and_destruction_0, which builds or destroys them
 * only for it */
#define INIT_PRIORITY_DEFAULT 0xFFFF

EXTERN_C_END

#endif /* VTABLE_H */
