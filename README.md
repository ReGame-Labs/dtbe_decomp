# Digimon Rumble Arena decomp

| Version | Code | Data | Functions |
|---|---|---|---|
| 🇯🇵 Japan (`SLPS_033.57`) | [![Code](https://decomp.dev/ReGame-Labs/dtbe_decomp.svg?mode=shield&measure=code&label=Code)](https://decomp.dev/ReGame-Labs/dtbe_decomp) | [![Data](https://decomp.dev/ReGame-Labs/dtbe_decomp.svg?mode=shield&measure=data&label=Data)](https://decomp.dev/ReGame-Labs/dtbe_decomp) | [![Functions](https://decomp.dev/ReGame-Labs/dtbe_decomp.svg?mode=shield&measure=functions&label=Functions)](https://decomp.dev/ReGame-Labs/dtbe_decomp) |

[![Build](https://github.com/ReGame-Labs/dtbe_decomp/actions/workflows/build.yaml/badge.svg)](https://github.com/ReGame-Labs/dtbe_decomp/actions/workflows/build.yaml)
[![Platform](https://img.shields.io/badge/platform-PlayStation-003791)](#the-executable)
[![Compiler](https://img.shields.io/badge/compiler-GCC%202.95.2-orange)](#toolchain)
[![License](https://img.shields.io/github/license/ReGame-Labs/dtbe_decomp)](LICENSE)

A work-in-progress matching decompilation of **Digimon Rumble Arena** for
the PlayStation, from its Japanese release, *Digimon Tamers: Battle
Evolution* (デジモンテイマーズ バトルエボリューション): C source that compiles
back into a byte-identical copy of the game's executable.

This repository does not contain any game data. You need your own copy of the
game to build it.

## Status

The build already gives back the original executable byte for byte: every
function is still splat's assembly behind `INCLUDE_ASM`, and decompiling
them one by one, keeping the match, is the work. Progress is measured by
[objdiff](https://github.com/encounter/objdiff) and tracked on
[decomp.dev](https://decomp.dev/ReGame-Labs/dtbe_decomp).

## The executable

`SLPS_033.57` loads at `0x80010000` and starts at crt0 (`0x8003DD74`):

| Address | What |
|---|---|
| `0x80010000` | `.rodata` (psylink places it ahead of `.text`) |
| `0x8001AE18` | the game's code, `main` at `0x8001AF68` (`src/main/game.c`) |
| `0x8003DD74` | crt0: the entry point, `__main` and its constructor loop |
| `0x8003DEE8` | the code after crt0, a first guess at the PsyQ libraries (`src/main/psyq.c`) |
| `0x8005EE40` | `.data`, with `$gp` at `0x8006414C` |
| `0x800643E0` | `.bss`, stored in the file as zeros up to `0x8011C248` |

The rest of the game, its code overlays (`/bin/<name>.bin`) included, is in
the disc's `A.VFS` archive; the movies are in `A.STR` and the sound in
`A.XAP`.

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

Then split the executable, build it and check it against the original:

```sh
make generate
make -j$(nproc)
make compare
```

`make report` writes objdiff's progress report (`build/jp/report.json`), and
`make objdiff` the `objdiff.json` that objdiff's GUI opens.

## Toolchain

- **Compiler:** GCC 2.95.2 for the PlayStation (`cc1`, from
  [decompals/old-gcc](https://github.com/decompals/old-gcc)) at `-O2 -G0`.
  Its instruction scheduling is the one the game's code has; 2.7.2 and
  2.8.x schedule the loads differently.
- **Assembler:** [maspsx](https://github.com/mkst/maspsx) turns GCC's output
  into what Sony's `aspsx` 2.86 would have made, then GNU `as` assembles it.
- **Splitting:** [splat](https://github.com/ethteck/splat)
  (`config/jp/main.yaml`).
- **Decompiling:** [m2c](https://github.com/matt-kempster/m2c) and
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) are in
  `external/`.

## Contributing

Pick a function behind `INCLUDE_ASM` in `src/main/`, write it as C, and check
that `make compare` still passes. Only byte-identical matches go in. The
[TODO](TODO.md) lists what comes next.
