#ifndef VTABLE_H
#define VTABLE_H

/* The virtual tables of g++ 2.95, as the C files see them: an entry, destructor flags. */

#include "common.h"

EXTERN_C_BEGIN

/* An entry of a g++ 2.95 virtual table: how far to move `this` to reach the
 * object the function belongs to, and the function. */
typedef struct {
    /* 0x0 */ s16 delta;
    /* 0x2 */ s16 index;
    /* 0x4 */ void (*func)();
} VtableEntry;

/* the flags a deleting destructor is called with */
#define DESTROY_FREE 1   /* free the object's memory afterwards */
#define DESTROY_BASES 2  /* destroy the virtual bases too: a whole object */
#define DESTROY_DELETE 3 /* what `delete` passes */

EXTERN_C_END

#endif /* VTABLE_H */
