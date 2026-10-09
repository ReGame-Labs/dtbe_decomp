#ifndef DTBE_MATH_PRINT_H
#define DTBE_MATH_PRINT_H

/* Printing matrices and vectors in hexadecimal. */

#include "common.h"
#include <libgte.h>

EXTERN_C_BEGIN

s32 func_80023468(s32 arg0);
void matrixPrint(MATRIX *m);
void svecPrint(SVECTOR *v);
void vecPrint(VECTOR *v);

EXTERN_C_END

#endif /* DTBE_MATH_PRINT_H */
