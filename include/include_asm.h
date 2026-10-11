#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

/* INCLUDE_ASM and INCLUDE_RODATA: the functions and rodata still in splat's assembly. */

#if !defined(M2CTX) && !defined(PERMUTER) && !defined(SKIP_ASM)

/*
 * FOLDER is relative to the version's splat output, ASM_DIR (asm/<version>),
 * which the Makefile defines: INCLUDE_ASM("main/nonmatchings/pad/pad", NAME)
 * includes asm/us/main/nonmatchings/pad/pad/NAME.s when building VERSION=us.
 */
#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME) \
    __asm__( \
        ".section .text\n" \
        "    .set noat\n" \
        "    .set noreorder\n" \
        "    .include \"" ASM_DIR "/" FOLDER "/" #NAME ".s\"\n" \
        "    .set reorder\n" \
        "    .set at\n" \
    )
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME) \
    __asm__( \
        ".section .rodata\n" \
        "    .include \"" ASM_DIR "/" FOLDER "/" #NAME ".s\"\n" \
        ".section .text" \
    )
#endif

#if defined(INCLUDE_ASM_USE_MACRO_INC) && INCLUDE_ASM_USE_MACRO_INC
__asm__(".include \"include/macro.inc\"\n");
#else
__asm__(".include \"include/labels.inc\"\n");
#endif

#else

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#endif /* !defined(M2CTX) && !defined(PERMUTER) */

#endif /* INCLUDE_ASM_H */
