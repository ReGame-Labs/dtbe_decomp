# TODO

The executable builds byte for byte from splat's split (`make compare`),
with every function still in assembly.

## Splitting

- [x] Find where the game ends and the PsyQ libraries start: the SDK's
  signatures match the code after crt0, and the libraries' rodata starts
  at `0x80019F58`, their `.data` at `0x80060F98` and their `.bss` at
  `0x801168E8`. They stay assembly and out of the progress.
- [ ] Split `game.c` into one file per original object, where the
  alignment padding and the rodata (splat suggests splits at `0x9FB4` and
  `0xA788` from the jump tables) show the boundaries.
- [ ] Split the game's `.data` and `.bss` by module, and find where `.sbss`
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
