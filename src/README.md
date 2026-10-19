# The source

`engine/` is the executable, `SLPS_033.57` (splat's segment `main`: its config
is `config/<version>/main.yaml`, whose `src_path` is `src/engine`, and its asm
is `asm/<version>/main/`). It has one folder per module and one file per
object of the original build, as far as the executable shows where each
ends. A file is named after the class or the job of its code.

The code overlays, `A.VFS`'s `/bin/<name>.bin`, are their own binaries
(config `config/<version>/<name>.yaml`, asm `asm/<version>/<name>/`), in the
folder of the part of the game they run:

| Folder | Overlay | What it is |
|---|---|---|
| `movie/` | `movie.bin` | the movie player |
| `title/` | `title.bin` | the boot logo, title screen and menus |
| `battle/` | `game.bin` | the battle |
| `battle/stages/st<NN>/` | `st<NN>.bin` | the battle's stages |

Each overlay is one file until it is split into its modules. `title/` and
`battle/` are split, one file per object of the original build in the order
of `config/<version>/title.yaml` and `game.yaml`; each file's `.rodata` and
`.data` are its own pieces of the overlay:

| File | What it is |
|---|---|
| `boot_logo.c` | the boot logo |
| `title_logo.c` | the title screen |
| `main_menu.c` | the main menu |
| `chara_select.cpp` | the character and stage selection |
| `memory_card.cpp` | the memory card screens |
| `guide.c` | the guide: the minigames' explanations |
| `ranking.c` | the ranking |
| `name_entry.cpp` | the name entry, which the ranking and the options open |
| `credits.c` | the credits |
| `title_chara.c` | the character model the character select and the guide show (boundary unproven) |
| `option.cpp` | the options |
| `digifont.c` | a font of 24 by 24 glyphs cached in VRAM, unused in the title (boundary unproven) |
| `zascii_printer.c` | the `Printer` that draws /zascii.tim's characters for the ranking and the name entry (boundary unproven) |

`include/title/title_classes.h` has what several of them share.

| File | What it is |
|---|---|
| `game_main.c` | the battle's main task: loads the stage and runs the battle |
| `battle_scene.c` | the battle scene: the sides' characters and input, the continue or quit choice |
| `side_hud.c` | a side's HUD: two gauges and a counter |
| `game_0b.c` | a pad input history, a box class and lists of spheres (boundary unproven) |
| `game_0c.c` | collision spheres and a node picker |
| `fighter.c` | the fighter factory and the fighters' base class |
| `cpu_input.c` | the computer's input for a side without a pad |
| `game_0e.c`, `game_0f.c` | classes not identified yet |
| `fighters/<character>.c` | the 24 fighters, one file each, named after their `CHARACTER_*` id |
| `game_0g.c`, `game_0h.c`, `game_0i.c` | entity classes the fighters derive from or create |
| `hud.c` | the battle's messages: READY?, FIGHT!, YOU WIN! |
| `battle_loading.c` | the battle's loading screen |
| `game_0k.c` | a class derived from the battle scene that prints figures |

`include/battle/game_classes.h` has what several of them share.

## Modules

| Folder | What it is |
|---|---|
| `system/` | the global objects, `main`, the RAM heap, the stub `calloc` and `free` were linked to, threads, running another executable, the main heap, the log, handle tables |
| `game/` | the sequencer that runs the scenes, the game state, the save data, the loading screen, characters |
| `task/` | the tasks the scheduler runs every frame, and the entities |
| `gfx/` | the display, ordering tables and primitive buffers, the screen fade, the scene graph and its animations, mesh scenes, the cameras and look-at views, TMD models and their lights, sprites, the VRAM image cache, colors, grey boxes and darkened rectangles |
| `gfx/prim/` | the primitives: taking them from the frame's buffer (`alloc_*`) and projecting shapes (a flags word, vertex pointers, colors, texture words) into packets (`build_*`) |
| `math/` | fixed-point division, vectors, matrices and quaternions, trigonometry, interpolation, random numbers and shuffles |
| `pad/` | the pads and the map of their buttons |
| `sound/` | the SPU sound system, the sound files a scene loads, pausing the voices |
| `cd/` | reading the CD-ROM, the file system of `A.VFS`, loading compressed files, the XA player |
| `text/` | the debug console, printing on the screen, the font cache, Shift JIS text |
| `menu/` | countdowns, and the steppers: values the buttons step between a minimum and a maximum, alone or in groups |
| `debug/` | the debug menu |
| `lib/` | code the game linked as is: strings, `vsprintf`, MD5, lists, node pools, the heap, zlib's inflate |

## Files

Where one file ends and the next starts comes from the build, not from what
the code does: each file's `.rodata` and `.sdata` are one piece in the
executable, the data a file reads through `$gp` is its own, and g++ puts a
file's synthesized functions, vtables and static constructors at its end.
Between those, a cut follows what the code does. The comment after the
include guard of a file's header says what the file is about;
`config/<version>/main.yaml` lists the files in the executable's order.

A `.cpp` file is one that only g++ builds as the game has it; the rest are
C:

| File | Why only g++ builds it |
|---|---|
| `menu/stepper.cpp`, `debug/system_menu.cpp`, `title/option.cpp` | constructors and destructors of classes with a virtual base, which copy the vtable to the stack and fix it up |
| `gfx/vram_cache.cpp` | `cacheInit`'s variable-length array |
| `game/character.cpp`, `gfx/mesh_scene.cpp` | a global `List` each, set up and taken down by g++'s static constructor and destructor |
| `game/game_state.cpp` | `JoinState`'s flags are `bool` bit fields, which g++ tests one at a time in `joinStateUpdate` |

Their headers declare the classes for C++ only (under `__cplusplus`); a C
file sees what it uses of them as C.

The files that hold a single kind of primitive keep the name of the first
one (`gfx/prim/alloc_sprt.c` also takes the tiles), since each is one of a
row of small objects of the same shape.

`include/engine/` has the same folders: a file's declarations are in the
header of the same name (`include/engine/gfx/display.h` for
`gfx/display.c`), with the types its functions work on and the data it owns
(its `.rodata` and `.sdata`, or the global objects of its types). Next to
them, `include/` has `common.h`, `psyq.h` (the PsyQ functions the SDK's
headers lack), `gte.h` (GTE macros), `vtable.h` (g++ 2.95's virtual tables
and static constructors as the C files see them) and `overlay.h` (what the code overlays define).
