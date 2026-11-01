#ifndef DTBE_MATH_BLOCK_COPY_H
#define DTBE_MATH_BLOCK_COPY_H

/*
 * The game's 32-byte copy (a MATRIX, or anything word-aligned of that size):
 * four loads then four stores, twice, through the two addresses as they are,
 * in registers.
 *
 * GCC 2.95.2 never emits it from C. mips.c expand_block_move sends every
 * aligned copy of 32 bytes or less to move_by_pieces, which interleaves the
 * loads and stores and folds the addresses into the offsets; only unaligned
 * copies take movstrsi_internal, and those use lwl/lwr. So a struct
 * assignment, memcpy, casts or four C locals all give another sequence, and
 * so do the other GCC 2.95.x builds and the SDK's own.
 *
 * One asm statement with the four words as outputs gives every detail of
 * the game's copy: the addresses are never folded (they are "r" operands),
 * the words come out in descending registers (local-alloc gives the last
 * output of an asm the lowest free register), and the copy is one insn, so
 * no store goes into the jr slot.
 *
 * It matches when dst and src are already in registers (parameters, or dst
 * also returned in v0). Where an address is worked out in the same block,
 * the game has it in a register above the four words, and this macro gives
 * it one below them: those functions stay in asm for now.
 */
#define copy32Bytes(dst, src) \
    ({ \
        long w0_, w1_, w2_, w3_; \
        __asm__ volatile( \
            "lw %0, 0(%5); lw %1, 4(%5); lw %2, 8(%5); lw %3, 12(%5); " \
            "sw %0, 0(%4); sw %1, 4(%4); sw %2, 8(%4); sw %3, 12(%4); " \
            "lw %0, 16(%5); lw %1, 20(%5); lw %2, 24(%5); lw %3, 28(%5); " \
            "sw %0, 16(%4); sw %1, 20(%4); sw %2, 24(%4); sw %3, 28(%4)" \
            : "=&r"(w0_), "=&r"(w1_), "=&r"(w2_), "=&r"(w3_) \
            : "r"(dst), "r"(src) \
            : "memory"); \
    })

#endif /* DTBE_MATH_BLOCK_COPY_H */
