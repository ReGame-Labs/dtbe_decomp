# The source

`engine/` is the executable, `SLPS_033.57` (splat's segment `main`: its config
is `config/<version>/main.yaml`, whose `src_path` is `src/engine`, and its asm
is `asm/<version>/main/`). It has one folder per module and one file per
object of the original build, as far as the executable shows where each
ends. A file is named after the class or the job of its code. The code
overlays in `A.VFS` are not decompiled yet.

## Modules

| Folder | What it is |
|---|---|
| `system/` | `main` and the global objects, threads, running another executable, the main heap, the log, handle tables |
| `game/` | the sequencer that runs the scenes, the game state, the save data, the loading screen, characters |
| `task/` | the tasks the scheduler runs every frame, and the entities |
| `gfx/` | the display, the screen fade, the scene graph, the cameras, TMD models and their lights, sprites, colors, grey boxes and darkened rectangles |
| `gfx/prim/` | the primitives: taking them from the frame's buffer (`alloc_*`), projecting shapes into packets (`build_*`) and the faces of a model (`face_*`) |
| `math/` | fixed-point vectors, matrices and quaternions, trigonometry, interpolation, random numbers |
| `pad/` | the pads and the map of their buttons |
| `sound/` | the SPU sound system, the sound files a scene loads, pausing the voices |
| `cd/` | reading the CD-ROM, the file system of `A.VFS`, loading compressed files, the XA player |
| `text/` | the debug console, printing on the screen, the font cache, Shift JIS text |
| `menu/` | countdowns, and the steppers: values the buttons step between a minimum and a maximum, alone or in groups |
| `debug/` | the debug menu (C++) |
| `lib/` | code the game linked as is: strings, `vsprintf`, MD5, lists, node pools, the heap, zlib's inflate |

## Files

Where one file ends and the next starts comes from the build, not from what
the code does: each file's `.rodata` and `.sdata` are one piece in the
executable, the data a file reads through `$gp` is its own, and g++ puts a
file's synthesized functions, vtables and static constructors at its end
(the TODO has the rest). The comment at the top of a file's first function
says what it is about; `config/<version>/main.yaml` lists the files in the
executable's order.

The files that hold a single kind of primitive keep the name of the first
one (`gfx/prim/alloc_sprt.c` also takes the tiles), since each is one of a
row of small objects of the same shape.

`include/engine/` has the same folders: a file's declarations are in the
header of the same name (`include/engine/gfx/display.h` for
`gfx/display.c`), with the types its functions work on and the data it owns
(its `.rodata` and `.sdata`, or the global objects of its types). Next to
them, `include/` has `common.h`, `psyq.h` (the PsyQ functions the SDK's
headers lack), `gte.h` (GTE macros), `vtable.h` (g++ 2.95's virtual tables
as the C files see them) and `overlay.h` (what the code overlays define).
