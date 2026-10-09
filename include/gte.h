#ifndef GTE_H
#define GTE_H

/*
 * The game's own GTE macros: inline asm, as PsyQ's inline_c.h writes them,
 * for the sequences the game has that no PsyQ macro gives. Each is written
 * from the instructions of the functions that use it.
 */

/* out = the number of leading zeros of in, from the GTE's LZCS and LZCR. The
 * two nops wait for the count; the last one, for the load, is there even when
 * the next instruction doesn't read out (buttonMapSet). */
#define gte_leadingZeros(in, out) \
    __asm__ volatile("mtc2 %1, $30; nop; nop; mfc2 %0, $31; nop" : "=r"(out) : "r"(in))

/* Loads the rotation and the translation of the MATRIX m, eight words, into
 * the GTE's control registers 0-7, four loads then four ctc2 through $2-$5
 * (setModelMatrixAndLights, setModelMatrix, setGteViewMatrix). No nop: each register is
 * moved two or more instructions after its load. PsyQ's gte_SetRotMatrix and
 * gte_SetTransMatrix use other registers and another order; gte_setMatrix
 * below gives other registers in these functions. */
#define gte_loadRotTrans(m) __asm__ volatile(                                \
    "lw $5, 0(%0); lw $4, 4(%0); lw $3, 8(%0); lw $2, 12(%0);"              \
    "ctc2 $5, $0; ctc2 $4, $1; ctc2 $3, $2; ctc2 $2, $3;"                   \
    "lw $5, 16(%0); lw $4, 20(%0); lw $3, 24(%0); lw $2, 28(%0);"           \
    "ctc2 $5, $4; ctc2 $4, $5; ctc2 $3, $6; ctc2 $2, $7"                    \
    : : "r"(m) : "$2", "$3", "$4", "$5")

/* Loads x, y and z into the GTE's IR1-IR3 (vecNormalize). */
#define gte_loadIR(x, y, z) \
    __asm__ volatile("mtc2 %0, $9; mtc2 %1, $10; mtc2 %2, $11" : : "r"(x), "r"(y), "r"(z))

/* SQR with sf = 1: MAC1-MAC3 = the squares of IR1-IR3, >> 12. The two nops
 * let the last mtc2 reach the GTE (vecNormalize). PsyQ's gte_sqr12 emits a
 * placeholder word for DMPSX instead of the instruction. */
#define gte_square12() __asm__ volatile("nop; nop; cop2 0xA80428")

/* out = MAC1 + MAC2 + MAC3, read through $24 and $25 and added in the
 * instructions' load delays, as vecNormalize does. */
#define gte_sumMac(out) __asm__ volatile(                                    \
    "mfc2 %0, $25; mfc2 $24, $26; mfc2 $25, $27;"                           \
    "addu %0, %0, $24; addu %0, %0, $25"                                    \
    : "=r"(out) : : "$24", "$25")

/* Loads the rotation and the translation of the MATRIX m into the GTE's
 * control registers 0-7, four words at a time through four temporaries, as
 * two asms (setGteMatrix). The temporaries are shared by both halves, so none
 * can take m's register while m is still to be read. The match depends on
 * their declaration order: declared r3 first, GCC gives r0-r3 $a2, $a1, $v1,
 * $v0, the game's registers; declared r0 first, it gives them other ones. */
#define gte_setMatrix(m) {                                                   \
    long r3, r2, r1, r0;                                                     \
    __asm__ volatile(                                                        \
        "lw %0, 0(%4)\n\tlw %1, 4(%4)\n\tlw %2, 8(%4)\n\tlw %3, 12(%4)\n\t"  \
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3"           \
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3) : "r"(m));                  \
    __asm__ volatile(                                                        \
        "lw %0, 16(%4)\n\tlw %1, 20(%4)\n\tlw %2, 24(%4)\n\tlw %3, 28(%4)\n\t" \
        "ctc2 %0, $4\n\tctc2 %1, $5\n\tctc2 %2, $6\n\tctc2 %3, $7"           \
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3) : "r"(m));                  \
}

/* The same in one asm (matrixTransformSvec), with a bug of the game's: the
 * temporaries are not early-clobber, so when m is not used after it GCC gives
 * the first one m's register, and the first load overwrites m: the other
 * fourteen loads read through m->m[0][0..1] instead of m. maspsx puts the nop
 * after that first load, which the next load reads. */
#define gte_setMatrixUnsafe(m) {                                             \
    long r3, r2, r1, r0;                                                     \
    __asm__ volatile(                                                        \
        "lw %0, 0(%4)\n\tlw %1, 4(%4)\n\tlw %2, 8(%4)\n\tlw %3, 12(%4)\n\t"  \
        "ctc2 %0, $0\n\tctc2 %1, $1\n\tctc2 %2, $2\n\tctc2 %3, $3\n\t"       \
        "lw %0, 16(%4)\n\tlw %1, 20(%4)\n\tlw %2, 24(%4)\n\tlw %3, 28(%4)\n\t" \
        "ctc2 %0, $4\n\tctc2 %1, $5\n\tctc2 %2, $6\n\tctc2 %3, $7"           \
        : "=r"(r0), "=r"(r1), "=r"(r2), "=r"(r3) : "r"(m));                  \
}

/* MVMVA sf 1, rotation, V0, translation (RTV0TR): MAC1-MAC3 = (R V0 >> 12)
 * + TR. The two nops let the lwc2 of V0 reach the GTE (matrixTransformSvec). PsyQ's
 * gte_rtv0tr emits a placeholder word for DMPSX instead of the instruction. */
#define gte_rotTransV0() __asm__ volatile("nop; nop; cop2 0x480012")

/* m0 = the 3x3 product m0 m1, the translation of m0 kept, as PsyQ's
 * MulMatrix computes it: m0 into the rotation, then each column of m1, packed
 * into V0 by lhu/lw and the mask 0xFFFF0000 in $2, through RTV0 (MVMVA sf 1,
 * rotation, V0, no translation), the results packed back two by two. Fixed
 * registers $2, $3, $8-$15 (matrixRotateAxis, matrixRotateX, matrixRotateY,
 * matrixRotateZ, matrixScale). The two nops before each RTV0 let the mtc2
 * of V0 reach the GTE. */
#define gte_mulMatrix(m0, m1) __asm__ volatile(                              \
    "lui $2, 0xFFFF\n\t"                                                     \
    "lw $8, 0(%0)\n\tlw $9, 4(%0)\n\tlw $10, 8(%0)\n\tlw $11, 12(%0)\n\t"    \
    "lw $12, 16(%0)\n\t"                                                     \
    "ctc2 $8, $0\n\tctc2 $9, $1\n\tctc2 $10, $2\n\tctc2 $11, $3\n\t"         \
    "ctc2 $12, $4\n\t"                                                       \
    "lhu $14, 0(%1)\n\tlw $3, 4(%1)\n\tlw $15, 12(%1)\n\t"                   \
    "and $3, $3, $2\n\tor $14, $14, $3\n\t"                                  \
    "mtc2 $14, $0\n\tmtc2 $15, $1\n\t"                                       \
    "nop\n\tnop\n\tcop2 0x486012\n\t"                                        \
    "lhu $14, 2(%1)\n\tlw $3, 8(%1)\n\tlhu $15, 14(%1)\n\t"                  \
    "sll $3, $3, 16\n\tor $14, $14, $3\n\t"                                  \
    "mfc2 $8, $9\n\tmfc2 $9, $10\n\tmfc2 $10, $11\n\t"                       \
    "mtc2 $14, $0\n\tmtc2 $15, $1\n\t"                                       \
    "nop\n\tnop\n\tcop2 0x486012\n\t"                                        \
    "andi $8, $8, 0xFFFF\n\tsll $9, $9, 16\n\tandi $10, $10, 0xFFFF\n\t"     \
    "lhu $14, 4(%1)\n\tlw $3, 8(%1)\n\tlw $15, 16(%1)\n\t"                   \
    "and $3, $3, $2\n\tor $14, $14, $3\n\t"                                  \
    "mfc2 $11, $9\n\tmfc2 $12, $10\n\tmfc2 $13, $11\n\t"                     \
    "mtc2 $14, $0\n\tmtc2 $15, $1\n\t"                                       \
    "nop\n\tnop\n\tcop2 0x486012\n\t"                                        \
    "sll $11, $11, 16\n\tandi $12, $12, 0xFFFF\n\tsll $13, $13, 16\n\t"      \
    "or $11, $11, $8\n\tor $13, $13, $10\n\t"                                \
    "sw $11, 0(%0)\n\tsw $13, 12(%0)\n\t"                                    \
    "mfc2 $13, $9\n\tmfc2 $14, $10\n\tmfc2 $15, $11\n\t"                     \
    "andi $13, $13, 0xFFFF\n\tsll $14, $14, 16\n\t"                          \
    "or $13, $13, $9\n\tor $14, $14, $12\n\t"                                \
    "sw $13, 4(%0)\n\tsw $14, 8(%0)\n\tsw $15, 16(%0)"                       \
    : : "r"(m0), "r"(m1)                                                     \
    : "$2", "$3", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "memory")

/* sum = (x*x + y*y + z*z + w*w) >> 12, by two SQR (sf 1) of IR1-IR2, the first
 * pair's squares read through $25 while the second pair goes in, and added in
 * the mfc2s' load delays (quaternionNormalize). The two nops before each SQR let
 * the mtc2s reach the GTE. */
#define gte_sumSquares4(x, y, z, w, sum) __asm__ volatile(                   \
    "mtc2 %1, $9\n\tmtc2 %2, $10\n\tnop\n\tnop\n\tcop2 0xA80428\n\t"         \
    "mfc2 %0, $25\n\tmfc2 $25, $26\n\t"                                      \
    "mtc2 %3, $9\n\tmtc2 %4, $10\n\tnop\n\tnop\n\tcop2 0xA80428\n\t"         \
    "addu %0, %0, $25\n\tmfc2 $24, $25\n\tmfc2 $25, $26\n\t"                 \
    "addu %0, %0, $24\n\taddu %0, %0, $25"                                   \
    : "=r"(sum) : "r"(x), "r"(y), "r"(z), "r"(w) : "$24", "$25")

/* Loads t into IR0 and the SVECTOR v into IR1-IR3, through x, y and z
 * (svecScale12, svecScale). x is early-clobber: it is loaded before v's
 * z is. */
#define gte_ldIr0SVector(t, v, x, y, z) __asm__ volatile(                    \
    "mtc2 %3, $8\n\tlw %0, 0(%4)\n\tlh %2, 4(%4)\n\tsra %1, %0, 16\n\t"      \
    "mtc2 %0, $9\n\tmtc2 %1, $10\n\tmtc2 %2, $11"                            \
    : "=&r"(x), "=r"(y), "=r"(z) : "r"(t), "r"(v))

/* GPF: MAC1-MAC3 = IR0 * IR1-IR3, >> 12 for gte_scaleByIr0_12 (sf 1), as is
 * for gte_scaleByIr0 (sf 0). The two nops let the last mtc2 reach the GTE
 * (svecScale12, svecScale). PsyQ's gte_gpf12 and gte_gpf0 emit
 * placeholder words for DMPSX instead of the instructions. */
#define gte_scaleByIr0_12() __asm__ volatile("nop\n\tnop\n\tcop2 0x198003D")
#define gte_scaleByIr0() __asm__ volatile("nop\n\tnop\n\tcop2 0x190003D")

/* Meant to store IR1-IR3 to the SVECTOR v through x, y and z, but the mfc2s
 * have their operands swapped, a bug of the game's (svecScale12,
 * svecScale): they read the GTE registers numbered like x, y and z into
 * $9-$11, and v gets x, y and z as they were. */
#define gte_stIrSVectorSwapped(v, x, y, z) __asm__ volatile(                 \
    "mfc2 $9, %0\n\tmfc2 $10, %1\n\tmfc2 $11, %2\n\t"                        \
    "sh %0, 0(%3)\n\tsh %1, 2(%3)\n\tsh %2, 4(%3)"                           \
    : : "r"(x), "r"(y), "r"(z), "r"(v) : "$9", "$10", "$11", "memory")

/* Loads the color MATRIX m of Lights into the GTE: its 3x3 part as the light
 * color matrix (control registers 16-20), and its translation, << 4, as both
 * the back color (13-15) and the far color (21-23), shifted once for both,
 * through $8-$11 (lightsSetGteColors). No nop: each register is moved two or more
 * instructions after its load. */
#define gte_loadLightColors(m) __asm__ volatile(                             \
    "lw $8, 0(%0); lw $9, 4(%0); lw $10, 8(%0); lw $11, 12(%0);"            \
    "ctc2 $8, $16; ctc2 $9, $17; ctc2 $10, $18; ctc2 $11, $19;"             \
    "lw $8, 16(%0); lw $9, 20(%0); lw $10, 24(%0); lw $11, 28(%0);"         \
    "ctc2 $8, $20; sll $9, $9, 4; sll $10, $10, 4; sll $11, $11, 4;"        \
    "ctc2 $9, $13; ctc2 $10, $14; ctc2 $11, $15;"                           \
    "ctc2 $9, $21; ctc2 $10, $22; ctc2 $11, $23"                            \
    : : "r"(m) : "$8", "$9", "$10", "$11")

/* out = the outer (cross) product a x b of the VECTORs a and b, by OP: the
 * diagonal of the rotation saved in r0, r2 and r4, set to a, the product
 * stored and the diagonal put back (vecCross, vecCross12). a is read
 * inside the asm, so it is still live while r0-r4 are written and they don't
 * take its register; all outputs are early-clobber, each written before an
 * input is last read. The two nops let the lwc2 of b reach the GTE. */
#define gte_outerProductOp(cmd, a, b, out) {                                 \
    long r0, r2, r4, x, y, z;                                                \
    __asm__ volatile(                                                        \
        "lw %3, 0(%6)\n\tlw %4, 4(%6)\n\tlw %5, 8(%6)\n\t"                   \
        "cfc2 %0, $0\n\tcfc2 %1, $2\n\tcfc2 %2, $4\n\t"                      \
        "ctc2 %3, $0\n\tctc2 %4, $2\n\tctc2 %5, $4\n\t"                      \
        "lwc2 $9, 0(%7)\n\tlwc2 $10, 4(%7)\n\tlwc2 $11, 8(%7)\n\t"           \
        "nop\n\tnop\n\tcop2 " cmd "\n\t"                                     \
        "swc2 $25, 0(%8)\n\tswc2 $26, 4(%8)\n\tswc2 $27, 8(%8)\n\t"          \
        "ctc2 %0, $0\n\tctc2 %1, $2\n\tctc2 %2, $4"                          \
        : "=&r"(r0), "=&r"(r2), "=&r"(r4), "=&r"(x), "=&r"(y), "=&r"(z)      \
        : "r"(a), "r"(b), "r"(out) : "memory");                              \
}

/* The outer product as integers (OP sf 0) and in 4.12 (OP sf 1). */
#define gte_outerProduct0(a, b, out) gte_outerProductOp("0x170000C", a, b, out)
#define gte_outerProduct12(a, b, out) gte_outerProductOp("0x178000C", a, b, out)

/* out = H, the distance to the projection plane, from the GTE's control
 * register 26 (lookAtInit). The nop is the cfc2's load delay, there before
 * out is first read. PsyQ's gte_ReadGeomScreen stores H to memory instead. */
#define gte_getGeomScreen(out) __asm__ volatile("cfc2 %0, $26; nop" : "=r"(out))

#endif /* GTE_H */
