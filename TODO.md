# TODO

The executable builds byte for byte from splat's split (`make compare`),
with every function still in assembly.

## Splitting

- [x] Find where the game ends and the PsyQ libraries start: the SDK's
  signatures match the code after crt0, and the libraries' rodata starts
  at `0x80019F58`, their `.data` at `0x80060F98` and their `.bss` at
  `0x801168E8`. They stay assembly and out of the progress.
- [ ] Check the split of the game's code into objects. The 60 files of
  `src/main/` are cut where no local rodata (strings, jump tables), no
  shared data and no call to a nearby function used only there crosses,
  in pieces of at least 0x200 bytes; most are named after their first
  function. Merge, move or rename them as the code shows what each object
  is. A few references go to rodata far from the rest of their file's (the
  `const` tables at `0x80010BAC`-`0x80018940`, `die`'s `0x80010028`), most
  likely another object's data.
- [ ] Some functions address small data through `$gp` (17 of the files
  have some): find which objects were built with `-G8` and give their C
  files that flag.
- [ ] Split the game's `.data` and `.bss` into the files of `src/main/`, so
  that they count as data in the progress, and find where `.sbss`
  and `.bss` start (crt0 only clears `0x800DA320`-`0x8011C248`).
- [ ] Unpack `A.VFS` (header `VFS2`): it holds the code overlays
  (`/bin/<name>.bin`) and the game's data, some of it compressed (the
  executable has zlib's inflate).

## Code

- [ ] The heap library (`include/heap.h`): `heapAllocNext`, `heapAllocPrev`
  and `heapShrink` are still asm. The first two test their first block
  before the loop, as a `while` loop that GCC rotates would.
- [ ] Name the game's heaps (`D_80114468`, `D_80114490`) and the wrappers
  around them from their callers.

## Toolchain

- [ ] Confirm the compiler on more functions: GCC 2.95.2 at `-O2` matches the
  scheduling of the first ones tried, 2.7.2 and 2.8.x don't.

## Names

- [x] Name the PsyQ functions from the SDK's signatures (the ones that only
  one SDK function matches).
- [ ] Name the game's functions from the strings, calls and data they use.
