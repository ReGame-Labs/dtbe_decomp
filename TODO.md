# TODO

The executable builds byte for byte (`make compare`); 1052 of the game's 1119
functions are C and the other 67 are assembly, each with a comment on what
keeps it there; the causes several share are listed below.

## Splitting

- [x] Find where the game ends and the PsyQ libraries start: the SDK's
  signatures match the code after crt0, and the libraries' rodata starts
  at `0x80019F58`, their `.data` at `0x80060F98` and their `.bss` at
  `0x801168E8`. They stay assembly and out of the progress.
- [ ] Check the split of the game's code into objects. The 82 files of
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
- [x] Unpack `A.VFS` (header `VFS2`): it holds the code overlays
  (`/bin/<name>.bin`) and the game's data, 127 of its 1142 files
  zlib-compressed (`ZP00`). `tools/unpack_vfs.py` unpacks it; its
  docstring describes the format.
- [ ] Split the overlays too (`/bin/title.bin`, `game.bin`, `movie.bin` at
  `OVERLAY_MEMORY`, and `st00`-`st09`): the executable calls their tasks'
  constructors by address (`overlay.h`).

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
  destructor) and `src/engine/game/game_state.cpp` (`JoinState`'s flags are
  `bool` bit fields, which g++ tests one at a time in `joinStateUpdate`)
  are. Their headers
  declare the classes for C++ only (under `__cplusplus`); a C file sees
  what it uses of them as C: a global `List` as a `ListNode`
  (`LOADED_CHARAS`), `gfx/mesh_scene`'s inline functions as prototypes. The
  game's headers give their functions C linkage (`EXTERN_C_BEGIN`). g++
  writes the synthesized destructors and the inline constructors a class
  with a vtable here needs at the end of the file, the vtables after them in
  reverse order of definition, and file-scope `static const` objects last,
  which is how `system_menu`'s order was found. The inline functions of a
  header under `#pragma interface` come out after the static constructor,
  in the file that has `#pragma implementation` (`gfx/mesh_scene`'s
  accessors, `menu/stepper`'s virtual functions and accessors). Still to do: the other files with C++-only code
  (`game/sequencer`, `math/matrix`).
- [x] `menu/stepper.cpp` and `menu/stepper_group.c` were one file: Stepper's
  virtual functions, its destructor and the accessors are the inline
  functions of `stepper.h` under `#pragma interface`, written at the end of
  `stepper.cpp` after StepperGroup's members, and g++ writes the vtables.
- [ ] Block moves (22 functions, listed in the comments that point
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
- [ ] GTE code. The game's C reaches the GTE through inline-asm macros
  (`include/gte.h`, and PsyQ's `inline_c.h`). Handwritten functions (no
  frame, an unfilled `jr $ra` slot, fixed registers, loops that test their
  count as no compiler does) stay assembly. The compiled ones still in
  assembly, each with a comment on what is left:
  - the primitive builders (`buildPolyF3`, `buildLineF2`,
    `buildLineG2`, `buildPolyFT4`, `buildPolyFT4Windowed`, `buildPolyGT4`,
    `buildPolyGT3`): every instruction right, the registers of the color
    block and of `shape`/`ot` not; most likely one shared source form. The
    packet tail matches with a `{ POLY_F3 poly; DR_TPAGE tpage; }` struct
    (`buildPolyF3`, 25 lines left). What is left: the parameters set once get
    a `REG_EQUIV` to their stack slot, which doubles their live length
    (`update_equiv_regs`), so `ot` (set twice) outranks `shape` in global-alloc
    where the game's `shape` gets `s1`; `const` parameters and `shape` as
    the `this` of a C++ member change nothing;
  - the other primitive builders (`buildLineF3`, `buildLineF4`,
    `buildLineG3`, `buildLineG4`, `buildPolyF4`, `buildPolyG3`, `buildPolyG4`,
    `buildPolyFT3`, `buildTile`; one template):
    the primitive's length is loaded first in its block and into `a1`, which
    sched1 does only for a pseudo set more than once (`birthing_insn_p`),
    and the first extra word (uv or color) is loaded before `move a0`, which
    needs its pseudo set twice with one set leaving no instruction. cc1 and
    cc1plus give the same code. No natural source found (`buildPolyFT3`: 5
    instructions left, `buildPolyG4`: 17); with the `REG_EQUIV` of the group
    above, most likely the game's GCC variant counts sets differently;
  - `divide12`, `divide16`, `getTan`: a `div` without the
    divide-by-zero break, issued before the test of the denominator
    (`-mno-check-zero-division` gives that, 31 lines left). The game keeps
    the quotient in LO until the end, where regclass ties LO and the general
    registers and global-alloc takes `v0`; and it divides a copy of the
    numerator made after the GTE asm, which GCSE propagates away;
  - `matrixInitLookAt` (look at), `cameraSetScreen`: register
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

- [x] Confirm the compiler on more functions: GCC 2.95.2 at `-O2 -G8`
  builds every C function; 2.7.2 and 2.8.x schedule the loads
  differently.

## Names

- [x] Name the PsyQ functions from the SDK's signatures (the ones that only
  one SDK function matches).
- [x] Name the game's functions from the strings, calls and data they use
  (`tools/renames/`).
- [ ] Name the 40 game functions left with splat's names: copies of other
  functions with no caller to tell them apart (the vector and matrix ops
  that return their argument, the functions that do nothing), accessors of
  fields not understood yet (`gfx/mesh_scene`'s, `GameState`'s) and
  functions with no caller.
