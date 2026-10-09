#ifndef DTBE_TEXT_SCREEN_PRINT_H
#define DTBE_TEXT_SCREEN_PRINT_H

/* Printing straight into VRAM in bold full-width characters, as die shows its message. */

#include "common.h"
#include "engine/lib/format.h"

EXTERN_C_BEGIN

void screenPrint(const char *fmt, ...);
void screenVprint(const char *fmt, VaList args);
void screenPutChar(s16 x, s16 y, s32 code);

EXTERN_C_END

#endif /* DTBE_TEXT_SCREEN_PRINT_H */
