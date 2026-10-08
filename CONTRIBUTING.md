# Contributing

The conventions follow the other ReGame Labs decomps, such as
[Digimon World 3's](https://github.com/ReGame-Labs/dw3_decomp), so that the
Digimon decomps read the same way. The README explains how to set up and
build the project; this file is about the work itself.

## Decompiling a function

Every function that isn't C yet is an `INCLUDE_ASM` line in `src/main/`,
which includes splat's disassembly of it:
```c
INCLUDE_ASM("asm/jp/main/nonmatchings/game", heapShrink);
```

1. **Get a first draft.** m2c turns the `.s` into C to start from:
   ```
   python3 external/m2c/m2c.py asm/jp/main/nonmatchings/game/heapShrink.s
   ```
   The draft is a starting point: give it the real types, the struct fields
   and the calls of the code around it (`include/heap.h` has the heap's).
2. **Make it match on its own.** Put the draft in a file of its own that
   includes the headers it needs, and compare it with the original:
   ```
   tools/try_match.py draft.c heapShrink
   ```
   It compiles the draft with the project's compiler and prints `MATCH`, or
   both versions side by side with the differing instructions marked `**`.
   Relocated fields are masked, so a different symbol name doesn't count as
   a difference.
3. **Search for a near miss.** When only register allocation or instruction
   order is left, try other source shapes first: types, the order of
   statements, a temporary, a loop written another way. Then the permuter
   can search for you:
   ```
   tools/permuter_import.py draft.c heapShrink
   python3 external/decomp-permuter/permuter.py permuter/heapShrink -j8
   ```
   The permuter's finds are hints, not answers: keep only what reads as C
   someone would write.
4. **Put it in place.** Replace the `INCLUDE_ASM` line with the C, add the
   prototypes and types it needs to the headers, and name what you
   understood in `config/jp/symbols.txt`. Then rebuild and check:
   ```
   make regenerate && make -j$(nproc) && make compare
   ```
   splat writes the asm under the new names, so a renamed function that is
   still asm needs its `INCLUDE_ASM` line renamed too.
5. **Check the progress.** `make report` writes the progress that
   decomp.dev shows; `make objdiff` and the objdiff GUI show each function
   against the original.

Rodata is migrated into the functions that use it: a function's strings and
jump tables live in its own `.s` file, so its C version emits them itself.

## Matching

- A change only counts if `make compare` still says `OK`: the executable has
  to stay byte for byte identical.
- Only byte-identical matches go in. No `NON_MATCHING` code, no `#if 0`
  blocks and no inline assembly in place of C. A function that doesn't
  match yet stays behind its `INCLUDE_ASM`. `tools/hacks.py` fails the CI
  on these.
- A fake match is the last resort: only for a function that natural C has
  failed to match after a real search. Mark the spot with a comment that
  starts with `/* fake match:` and says what is forced and why.
- Only the game is decompiled. The PsyQ libraries stay splat's disassembly
  and out of the progress; their names come from matching the SDK's
  functions (`config/jp/symbols.txt`).
