#include "common.h"
#include "engine/math/print.h"
#include "stdio.h"

/* Returns its argument. */
s32 func_80023468(s32 value) {
    return value;
}

/* Prints a matrix, its elements as 16-bit hex (hence the u16 casts). %x
 * takes an unsigned int: the translation, a long of the same 32 bits, is
 * cast to u32 for it. */
void matrixPrint(MATRIX *m) {
    printf("\n%04x %04x %04x\n%04x %04x %04x\n%04x %04x %04x\n%08x %08x %08x\n",
           (u16)m->m[0][0], (u16)m->m[0][1], (u16)m->m[0][2],
           (u16)m->m[1][0], (u16)m->m[1][1], (u16)m->m[1][2],
           (u16)m->m[2][0], (u16)m->m[2][1], (u16)m->m[2][2],
           (u32)m->t[0], (u32)m->t[1], (u32)m->t[2]);
}

/* Prints a short vector, its components as 16-bit hex (hence the casts). */
void svecPrint(SVECTOR *v) {
    printf("\n%04x %04x %04x\n", (u16)v->vx, (u16)v->vy, (u16)v->vz);
}

/* Prints a vector, its long components cast to the u32 %x takes. */
void vecPrint(VECTOR *v) {
    printf("\n%08x %08x %08x\n", (u32)v->vx, (u32)v->vy, (u32)v->vz);
}
