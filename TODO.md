# TODO

The executable builds byte for byte (`make compare`); 1034 of the game's 1118
functions are C. What keeps the other 84 in assembly is listed below.

## Splitting

- [x] Find where the game ends and the PsyQ libraries start: the SDK's
  signatures match the code after crt0, and the libraries' rodata starts
  at `0x80019F58`, their `.data` at `0x80060F98` and their `.bss` at
  `0x801168E8`. They stay assembly and out of the progress.
- [ ] Check the split of the game's code into objects. The 85 files of
  `src/engine/` ([by module](src/README.md)) are cut where each one's
  `.rodata` and `.sdata` stay one piece, the data read through `$gp` stays
  with its readers and g++'s end-of-file code closes a C++ file; between
  those, a cut follows what the code does. Merge, move or rename them as
  the code shows more of what each object is. A few references go to
  rodata far from the rest of their file's (the `const` tables at
  `0x80010BAC`-`0x80018940`, `die`'s `0x80010028`), most likely another
  object's data.
- [ ] `D_8006434C` (8 bytes between `text/screen_print`'s and
  `game/character`'s `.sdata`, read absolutely by `gfx/prim/build_line_g2`)
  and `APP_SEQUENCER_HANDLE` (12 bytes at gp, from the AppSequencer's
  handle on) stay in the asm piece of the gp region:
  find their owners.
- [ ] `task/task` and `task/entity` are built with `-G0` (`G0_UNITS`): they
  read `TASK_HANDLES`, a 4-byte global in `.data`, as code that does not take it
  for small data. Find out whether they are one object built that way.
- [ ] Split the game's `.data` and `.bss` into the files of `src/engine/`, so
  that they count as data in the progress, and find where `.sbss`
  and `.bss` start (crt0 only clears `0x800DA320`-`0x8011C248`).
- [ ] Unpack `A.VFS` (header `VFS2`): it holds the code overlays
  (`/bin/<name>.bin`) and the game's data, some of it compressed (the
  executable has zlib's inflate).

## Code

- [ ] The game is C++ (classes with vtables, constructors returning `this`,
  destructors freeing on `flags & 1`). Most of it matches as C. The
  constructors and destructors of classes with a virtual base (they copy the
  vtable to the stack and fix it up) only come out of `cc1plus`, so their
  files have to be C++: `src/engine/menu/stepper.cpp` (Countdown, Stepper,
  StepperGroup), `src/engine/debug/system_menu.cpp` (the debug menu) and
  `src/engine/gfx/vram_cache.cpp` (cacheInit's variable-length array) and
  `src/engine/game/character.cpp` and `src/engine/gfx/mesh_scene.cpp` (a
  global `List` each, set up and taken down by g++'s static constructor and
  destructor) are. Their headers
  declare the classes for C++ and the same objects as structs for C; the
  game's headers give their functions C linkage (`EXTERN_C_BEGIN`). g++
  writes the synthesized destructors and the inline constructors a class
  with a vtable here needs at the end of the file, the vtables after them in
  reverse order of definition, and file-scope `static const` objects last,
  which is how `system_menu`'s order was found. The inline functions of a
  header under `#pragma interface` come out after the static constructor,
  in the file that has `#pragma implementation` (`gfx/mesh_scene`'s
  accessors). Still to do: the other files with C++-only code
  (`game/sequencer`, `math/matrix`).
- [ ] `menu/stepper.cpp` and `menu/stepper_group.c` are likely one file: Stepper's vtable
  sits in the first one's rodata while the functions g++ would have emitted
  it with (Stepper's first virtual functions, its out-of-line destructor
  `stepperDestroy`) are in the second.
- [ ] Block moves (about 29 functions, listed in the comments that point
  here): the game copies 12- to 32-byte structs (MATRIX, Vec3) as GCC's
  `movstrsi_internal` block move: the two addresses copied to registers, then
  four loads and four stores at a time. GCC 2.95.2 (C and C++; also PsyQ 4.6's
  CC1PSX, the same compiler) copies a word-aligned struct of 32 bytes or less
  a word at a time instead (`expand_block_move` calls `move_by_pieces` when
  `align == UNITS_PER_WORD`). GCC 2.7.2, 2.8.x and egcs 2.91.66 (PsyQ 4.0
  to 4.5, C and C++) use the block move, but fold the addresses into the
  offsets, give its four scratch registers in ascending order where the
  game's take the four lowest free ones in descending order (`a3, a2, v1,
  v0`), and do not match the rest of the game. The compiler that built these is a 2.95.2 that
  we do not have. Two more things tell it apart: with that `move_by_pieces`
  branch skipped (a scratch copy of cc1, only to look), 2.95.2 gives the
  block move, but its first CSE pass then folds the addresses into the
  offsets (`find_best_addr`), which the original's did not; and a copy
  through an inline function (`*dst = *src`) keeps both addresses in
  registers in stock 2.95.2, word by word. `-fforce-addr`,
  `-fno-expensive-optimizations`, `-mmemcpy`, `-Os`, 8-byte alignment,
  `memcpy` and g++ change none of this.
- [ ] The heap library (`include/engine/lib/heap.h`): `heapAllocNext` and
  `heapAllocPrev` reload `heap->rover` every pass, which C only gets with a
  fake form.
- [ ] GTE code. The game's C reaches the GTE through inline-asm macros
  (`include/gte.h`, and PsyQ's `inline_c.h`). Handwritten functions (no
  frame, an unfilled `jr $ra` slot, fixed registers, loops that test their
  count as no compiler does) stay assembly. The compiled ones still in
  assembly, each with a comment on what is left:
  - the primitive builders (`buildPolyF3`, `buildLineF2`,
    `buildLineG2`, `buildPolyFT4`, `buildPolyFT4Windowed`, `buildPolyGT4`,
    `buildPolyGT3`): every instruction right, the registers of the color
    block and of `this`/`ot` not; most likely one shared source form;
  - the face functions of `gfx/prim/face_*` (one template):
    the primitive's length is loaded early, as only a register set more than
    once gives;
  - `divide12`, `divide16`, `getTan`: a `div` without the
    divide-by-zero break, issued before the test of the denominator;
  - `matrixInitLookAt` (look at), `cameraSetScreen`, `blendVec3s`: register
    allocation or one load out of order.
- [ ] `die` reads its return address with `move s1, ra`; only inline asm
  gives that, so it stays assembly.
- [x] Move the PsyQ prototypes out of the game's headers into
  `include/psyq.h`. `func_80045920` (CdSearchFile) stays in
  `engine/cd/read.h`: it takes the game's `CdfsFile`, most likely PsyQ's
  `CdlFILE`.
- [ ] Type `overlay.h`'s parameters.
- [x] Name the game's heaps (`DEBUG_HEAP`, `MAIN_HEAP`, `ENTITY_HEAP`) and
  the wrappers around them (`debugHeap*`, `mainHeap*`) from their callers.

## Toolchain

- [ ] Confirm the compiler on more functions: GCC 2.95.2 at `-O2` matches the
  scheduling of the first ones tried, 2.7.2 and 2.8.x don't.

## Names

- [x] Name the PsyQ functions from the SDK's signatures (the ones that only
  one SDK function matches).
- [x] Name the game's functions from the strings, calls and data they use
  (`tools/renames/`).
- [ ] Name the 70 game functions left with splat's names: copies of other
  functions with no caller to tell them apart, accessors of fields not
  understood yet, the g++ static constructors and destructors (their names
  come from a symbol of their file's), and the scenes and `GameState`
  functions that depend on unknown fields.
