# Contributing

The conventions follow the other ReGame Labs decomps, such as
[Digimon World 3's](https://github.com/ReGame-Labs/dw3_decomp), so that the
Digimon decomps read the same way. The README explains how to set up and
build the project; this file is about the work itself.

## Decompiling a function

Every function that isn't C yet is an `INCLUDE_ASM` line in `src/engine/`,
which includes splat's disassembly of it:
```c
INCLUDE_ASM("asm/jp/main/nonmatchings/lib/format", formatterPadNumber);
```

1. **Get a first draft.** m2c turns the `.s` into C to start from:
   ```
   python3 external/m2c/m2c.py asm/jp/main/nonmatchings/lib/format/formatterPadNumber.s
   ```
   The draft is a starting point: give it the real types, the struct fields
   and the calls of the code around it (`include/engine/lib/format.h` has the
   formatter's).
2. **Make it match on its own.** Put the draft in a file of its own that
   includes the headers it needs, and compare it with the original:
   ```
   tools/try_match.py draft.c formatterPadNumber
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
   tools/permuter_import.py draft.c formatterPadNumber
   python3 external/decomp-permuter/permuter.py permuter/formatterPadNumber -j8
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

## Small data and `$gp`

The game was built with `-G8`: every object of up to 8 bytes, string
literals included, went to `.sdata`. Like Sony's `aspsx`, maspsx reaches a
variable through `$gp` only from the file that defines it; everywhere else
it is an extern and gets an absolute address. So a function that reads
`%gp_rel(D_X)` belongs to the file that defines `D_X`, and that file's C
defines its whole piece of `.sdata`, in address order, with the piece listed
as `.sdata` for the file in `config/jp/main.yaml`.
`tools/sdata_check.py <unit> <first> <end>` compares a file's `.sdata` with
the game's bytes.

## Layout

- `src/engine/` is the executable, a folder per module and a file per object
  of the original build; [src/README.md](src/README.md) lists the modules
  and how the files are cut. A file is named after the class or the job of
  its code, in snake_case (`gfx/scene_graph.c`, `cd/xa_player.c`), and
  `config/jp/main.yaml` lists the files in the executable's order.
- `include/engine/` has the same folders and a header per file:
  `include/engine/gfx/display.h` declares what `src/engine/gfx/display.c`
  defines, the types its functions work on and the data it owns. Its guard
  is `DTBE_<MODULE>_<FILE>_H`, and it is included by its path from
  `include/` (`#include "engine/gfx/display.h"`).
- A source file includes `common.h`, its own header, then the headers of
  the other files it uses, in alphabetical order, then the SDK's and
  `include/`'s own (`psyq.h`, `gte.h`, `vtable.h`, `overlay.h`).
- The PsyQ libraries are not in `src/`: they stay splat's assembly, and
  `include/psyq.h` declares the SDK functions the SDK's headers lack.

## Code

Look at the C around you and write the same way:

- Four spaces, no tabs; the opening brace on the same line; braces around
  every `if`, `for` and `while` body, even a one-line one.
- Lines of code up to 120 columns; comments wrapped at 80.
- A header says what it is about in a line after its include guard
  (`/* The pads: both ports' buttons and analog sticks, held buttons
  repeating, rumble. */`), which `make docs` shows as the file's.
- Comments are `/* */`. A function, a type, a global and a macro get a
  comment right above them that says what they are or do in the game
  (`/* Runs its tasks every frame and destroys the killed ones the frame
  after. */`); a member, an enumerator or a field one after it on its line.
  `make docs` takes these comments as they are, so they say what the
  evidence shows and no more.
- A file includes the headers whose names it uses, itself, and no others
  (the SDK's include what they need).
- The game's code uses the types of `include/common.h` (`s8`, `u8`, `s16`,
  `u16`, `s32`, `u32`) and the SDK's where it talks to the SDK (`RECT`,
  `SVECTOR`, `CdlLOC`).
- Struct fields keep their offset in a comment, and the fields that aren't
  understood yet are named by it (`/* 0x1C */ s32 unk1C;`).
- A C++ class (`src/engine/menu/stepper.cpp`, `src/engine/debug/system_menu.cpp`)
  is declared in its header for C++ only (under `__cplusplus`); what C
  files use of a C++ file gets a C declaration there too (a global `List`
  as a `ListNode`). The game's functions have C linkage
  (`EXTERN_C_BEGIN`/`EXTERN_C_END`).

## Names

A name has to come from evidence: the strings a function uses, the SDK
calls it makes, its callers, the data it reads and the comment that
describes it. What isn't understood yet keeps splat's name or its offset
(`func_8002A1B4`, `D_80064200`, `unk1C`). The names follow the other
ReGame Labs decomps' ([Digimon World](https://github.com/ReGame-Labs/dw_decomp)'s,
[Digimon World 3](https://github.com/ReGame-Labs/dw3_decomp)'s):

| What | Style | Examples |
|---|---|---|
| Functions of an object | camelCase: the type, then a verb | `heapAllocBest(Heap *)`, `schedulerRunTasks(Scheduler *)`, `cameraSetFov(Camera *)` |
| Other functions | camelCase, a verb that says what the function does | `loadFile`, `playSoundEffect`, `drawRaisedBox` |
| Globals and file-scope data | UPPER_SNAKE | `MAIN_HEAP`, `SCHEDULER`, `MENU_BUTTONS` |
| Constants and macros | UPPER_SNAKE, the module's or the type's word first | `SYSTEM_FLAG_FONTDISP`, `HEAP_BLOCK_SIZE` |
| Types and classes | PascalCase, the typedef named as its tag | `Scheduler`, `SceneObject`, `StepperGroup` |
| Fields, methods, parameters and locals | camelCase, the same word for the same thing everywhere | `/* 0x18 */ ListNode *cursor;`, `task`, `ot`, `count` |
| Files and folders | snake_case, after the class or the job | `scene_graph.c`, `gfx/prim/` |

A function whose first parameter is the object it works on is named after
that object's type (`stepperGroup...` for a `StepperGroup *`), and one that
works on a single global object after that object (`mainHeapAllocBest` for
`MAIN_HEAP`, `mainConsolePrint`). The verbs say the same thing everywhere:

- `init` fills an object the caller has, `create` allocates one and fills
  it, `destroy` undoes `create` and `free` gives memory back;
- `update` runs an object's frame, `draw` puts it in the ordering table,
  `build` writes primitives;
- `load` reads from the disc, `play`/`stop` sound, `get`/`set` a value,
  `is`/`has` answer a yes or no, `find` searches, `alloc` takes memory.

The SDK's functions keep the SDK's names, and the code the game linked as
is its own: zlib's (`inflate_blocks`, `huft_build`), RSA's MD5 (`MD5Update`)
and the C library's (`vsprintf`). g++'s own symbols (`_._4Task`, `_vt.7Stepper`) are its mangled
names.

Every name is in `config/jp/symbols.txt`, so that splat's disassembly uses
it too and objdiff keeps pairing the functions: the game's names are in a
section per module, in address order. A rename is made with
`tools/rename.py`, from a list in `tools/renames/<module>.txt`:

```
python3 tools/rename.py -f tools/renames/gfx.txt
make regenerate && make -j$(nproc) && make compare
```

It changes the name in the symbol file, `src/`, `include/` and the docs,
files splat's names in the symbol file's section of the module, and can be
run again: a list that is already applied changes nothing, so a branch that
still has the old names catches up the same way.

## Clean code

Every change follows these rules, after the match: nothing goes in that
breaks a function or a byte of data.

- **Names tell the truth.** Name only what the evidence supports, and keep
  `unkXX`, `func_` and `D_` rather than guess. One word for one concept
  everywhere.
- **No magic numbers.** Use the named constants and `sizeof`. A number whose
  meaning is known gets a constant in its owner's header; one that isn't
  stays as it is.
- **Types instead of casts.** Real struct members, unions for two views of
  the same memory, the right return types. A cast that stays says why.
- **One declaration per symbol**, in the header of the file that defines
  it, which its users include. No copies of externs or prototypes in `.c`
  files or in other headers.
- **One job per file and per function.** A file holds one object of the
  original build and is named after it; a helper (a `static inline`
  function or a macro) goes in only when the original's code shows it and
  it matches.
- **Comments say why.** A comment on how the match depends on a form stays,
  short and precise. Stale comments, commented-out code and TODOs go (the
  TODO file lists what is left).
- **No dead code or fake matches**: no junk temporaries, unread locals,
  `volatile`, an empty `do`/`while (0)` or one without a `break` used as a
  barrier, or branches with identical arms.
- **Renames by script**, and a commit has one theme.
