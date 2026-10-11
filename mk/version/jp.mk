# The Japanese release (SLPS-03357)
#
# Plain values: tools/version.py reads them from here too.

# the executable, as it is on the disc
EXE_NAME := SLPS_033.57

# where movie.bin, title.bin and game.bin load: the start of the game's .bss
# (OVERLAY_MEMORY)
OVERLAY_VRAM := 0x800643E0
# where game.bin loads the battle's stage, after itself
STAGE_VRAM := 0x800D3820
# the end of the overlays' memory: crt0 clears the .bss from here on
OVERLAY_END := 0x800DA320
