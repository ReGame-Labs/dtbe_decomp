# Digimon Rumble Arena decomp

| 🇯🇵 Japan (`SLPS_033.57`) | Code | Data | Functions |
|---|---|---|---|
| the whole game | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=code&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=data&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=functions&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57) |
| the executable | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=code&category=executable&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=executable) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=data&category=executable&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=executable) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=functions&category=executable&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=executable) |
| the code overlays (`A.VFS`'s `/bin/`) | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=code&category=overlays&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=overlays) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=data&category=overlays&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=overlays) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57.svg?mode=shield&measure=functions&category=overlays&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLPS_033.57?category=overlays) |

| 🇺🇸 USA (`SLUS_014.04`) | Code | Data | Functions |
|---|---|---|---|
| the whole game | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=code&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=data&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=functions&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04) |
| the executable | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=code&category=executable&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=executable) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=data&category=executable&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=executable) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=functions&category=executable&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=executable) |
| the code overlays (`A.VFS`'s `/bin/`) | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=code&category=overlays&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=overlays) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=data&category=overlays&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=overlays) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04.svg?mode=shield&measure=functions&category=overlays&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp/SLUS_014.04?category=overlays) |

[![Build](https://github.com/ReGame-Labs/dtbe_decomp/actions/workflows/build.yaml/badge.svg)](https://github.com/ReGame-Labs/dtbe_decomp/actions/workflows/build.yaml)
[![Platform](https://img.shields.io/badge/platform-PlayStation-003791)](#the-executable)
[![Compiler](https://img.shields.io/badge/compiler-GCC%202.95.2-orange)](#toolchain)
[![License](https://img.shields.io/github/license/ReGame-Labs/dtbe_decomp)](LICENSE)

A work-in-progress matching decompilation of **Digimon Rumble Arena** for
the PlayStation, from its Japanese release, *Digimon Tamers: Battle
Evolution* (デジモンテイマーズ バトルエボリューション): C source that compiles
back into a byte-identical copy of the game's executable and code overlays.

This repository does not contain any game data. You need your own copy of the
game to build it.

## Status

The build gives back the original executable and its 13 code overlays byte
for byte, for both the Japanese release (`SLPS_033.57`, the default) and the
USA one (`SLUS_014.04`, `VERSION=us`), from the same source. Most of the
executable's functions are C already; about a third of the overlays' are,
and the rest is still splat's assembly behind `INCLUDE_ASM`. Decompiling
them one by one, keeping the match in both versions, is the work. The
progress counts the whole game of each version, executable and overlays,
with each part in its own category. Progress is measured by
[objdiff](https://github.com/encounter/objdiff) and tracked on
[decomp.dev](https://decomp.dev/ReGame-Labs/dtbe_decomp).

<a name="the-executable"></a>
## The executable

`SLPS_033.57` loads at `0x80010000` and starts at crt0 (`0x8003DD74`):

| Address | What |
|---|---|
| `0x80010000` | `.rodata` (psylink places it ahead of `.text`) |
| `0x8001AE18` | the game's code, `main` at `0x8001AF68` (`src/engine/`, [by module](src/README.md)) |
| `0x8003DD74` | crt0: the entry point, `__main` and its constructor loop |
| `0x8003DEE8` | SN's runtime wrappers and the PsyQ libraries |
| `0x8005EE40` | `.data`, with `$gp` at `0x8006414C` |
| `0x800643E0` | `.bss`, stored in the file as zeros up to `0x8011C248` |

Only the game is decompiled. The PsyQ libraries stay splat's disassembly and
out of the progress: their code from `0x8003DF10`, their rodata from
`0x80019F58`, their `.data` from `0x80060F98` and their `.bss` from
`0x801168E8`.

The rest of the game, its code overlays (`/bin/<name>.bin`) included, is in
the disc's `A.VFS` archive; the movies are in `A.STR` and the sound in
`A.XAP`.

## The overlays

The code overlays are `A.VFS`'s `/bin/<name>.bin`, zlib-compressed in the
archive; the build splits and checks them inflated. Each one starts with its
`.rodata`, and has its own splat config (`config/<version>/<name>.yaml`) and
symbols (`config/<version>/symbols_<name>.txt`); it links against the executable's symbols,
and a stage against `game.bin`'s too.

| File | Loads at | Size | What | Source |
|---|---|---|---|---|
| `movie.bin` | `0x800643E0` (`OVERLAY_MEMORY`; USA `0x800643C0`) | 10 KB | the movie player (MDEC, `/a.str`) | `src/movie/` |
| `title.bin` | `0x800643E0` (USA `0x800643C0`) | 92 KB | the boot logo, title screen and menus: main menu, character and map select, ranking, credits, options | `src/title/` |
| `game.bin` | `0x800643E0` (USA `0x800643C0`) | 445 KB | the battle ("game main process") | `src/battle/` |
| `st00.bin`-`st09.bin` | `0x800D3820`, after `game.bin` (USA `0x800D3B00`) | 4-27 KB | the battle's stage, which `game.bin` loads (`/bin/st%02d.bin`) | `src/battle/stages/` |

## Building

On Linux (Ubuntu 24.04 or similar):

```sh
sudo apt install binutils-mipsel-linux-gnu gcc-mipsel-linux-gnu python3-venv
git clone --recursive https://github.com/ReGame-Labs/dtbe_decomp.git
cd dtbe_decomp
python3 -m venv .venv && . .venv/bin/activate
pip install -r requirements.txt
tools/dl_deps.sh
```

Extract the game's files from your disc image into `disks/jp`:

```sh
python3 tools/extract_disc.py "DigimonTamers - Battle Evolution (Japan).bin" disks/jp
```

The build unpacks `A.VFS` into `assets/jp` itself (`tools/unpack_vfs.py`):
the overlays are split from there, and the game's data is there to look at.

Then split the executable and the overlays, build them and check them
against the originals:

```sh
make generate
make -j$(nproc)
make compare
```

The USA release (`SLUS_014.04`) builds the same way, with `VERSION=us` on
every command: its files go in `disks/us`, and `make VERSION=us compare`
checks it. `jp` is the default. Both builds share the source; the few
functions whose code differs between the releases keep the Japanese code
behind `#if VERSION_JP`.

`make report` writes objdiff's progress report (`build/<version>/report.json`), and
`make objdiff` the `objdiff.json` that objdiff's GUI opens.

`make docs` writes the source's documentation, `build/docs/html/index.html`,
with [doxygen](https://www.doxygen.nl) and the
[doxygen-awesome](https://github.com/jothepro/doxygen-awesome-css) theme
(`sudo apt install doxygen graphviz`). It takes the code's comments as they
are: the comment above a function, a type or a global is its description.

<a name="toolchain"></a>
## Toolchain

- **Compiler:** GCC 2.95.2 for the PlayStation (`cc1`, from
  [decompals/old-gcc](https://github.com/decompals/old-gcc)) at `-O2`, `-G8` for the executable and `-G0` for the overlays.
  Its instruction scheduling is the one the game's code has; 2.7.2 and
  2.8.x schedule the loads differently.
- **Assembler:** [maspsx](https://github.com/mkst/maspsx) turns GCC's output
  into what Sony's `aspsx` 2.86 would have made, then GNU `as` assembles it.
- **Splitting:** [splat](https://github.com/ethteck/splat)
  (`config/<version>/main.yaml`).
- **Decompiling:** [m2c](https://github.com/matt-kempster/m2c) and
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) are in
  `external/`.

## Contributing

Pick a function behind `INCLUDE_ASM`, most of them in the overlays
(`src/battle/`, `src/title/`), write it as C, and check that `make compare`
and `make VERSION=us compare` still pass. Only byte-identical matches go in.
[CONTRIBUTING](CONTRIBUTING.md) describes the tools and the rules.
