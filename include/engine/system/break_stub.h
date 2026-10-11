#ifndef DTBE_SYSTEM_BREAK_STUB_H
#define DTBE_SYSTEM_BREAK_STUB_H

/* The stub calloc and free were linked to: a break, forever. */

#include "common.h"

EXTERN_C_BEGIN

void breakForever(void);

EXTERN_C_END

#endif /* DTBE_SYSTEM_BREAK_STUB_H */
