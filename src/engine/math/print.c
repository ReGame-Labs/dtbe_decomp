#include "common.h"
#include "engine/math/print.h"
#include "stdio.h"

/* Returns its argument. */
s32 func_80023468(s32 arg0) {
    return arg0;
}

/* Prints a matrix, its elements as 16-bit hex (hence the casts). */
void matrixPrint(MATRIX *m) {
    printf("\n%04x %04x %04x\n%04x %04x %04x\n%04x %04x %04x\n%08x %08x %08x\n",
           (u16)m->m[0][0], (u16)m->m[0][1], (u16)m->m[0][2],
           (u16)m->m[1][0], (u16)m->m[1][1], (u16)m->m[1][2],
           (u16)m->m[2][0], (u16)m->m[2][1], (u16)m->m[2][2],
           m->t[0], m->t[1], m->t[2]);
}

/* Prints a short vector, its components as 16-bit hex (hence the casts). */
void svecPrint(SVECTOR *v) {
    printf("\n%04x %04x %04x\n", (u16)v->vx, (u16)v->vy, (u16)v->vz);
}

/* Prints a vector. */
void vecPrint(VECTOR *v) {
    printf("\n%08x %08x %08x\n", v->vx, v->vy, v->vz);
}
