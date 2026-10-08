# TODO

The executable builds byte for byte from splat's split (`make compare`),
with every function still in assembly.

## Splitting

- [ ] Find where the game's code ends and the PsyQ libraries start. The
  split now puts everything before crt0 in `game.c` and everything after it
  in `psyq.c`; the libraries' `$Id:` strings (`intr.c`, `bios.c`, `sys.c`)
  and the SDK's function signatures place each library object.
- [ ] Split `game.c` into one file per original object, where the
  alignment padding and the rodata (splat suggests splits at `0x9FB4` and
  `0xA788` from the jump tables) show the boundaries.
- [ ] Split `.data` and `.bss` by module, and find where `.sbss` and `.bss`
  start (crt0 only clears `0x800DA320`-`0x8011C248`).
- [ ] Unpack `A.VFS` (header `VFS2`): it holds the code overlays
  (`/bin/<name>.bin`) and the game's data, some of it compressed (the
  executable has zlib's inflate).

## Toolchain

- [ ] Confirm the compiler on more functions: GCC 2.95.2 at `-O2` matches the
  scheduling of the first ones tried, 2.7.2 and 2.8.x don't. The PsyQ
  libraries may need their own compiler and flags.

## Names

- [ ] Name the PsyQ functions from the SDK's signatures, and the game's from
  the strings, calls and data they use.
