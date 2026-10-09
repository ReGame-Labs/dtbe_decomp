#!/usr/bin/env python3
"""Check a C file's .sdata against the executable.
usage: tools/sdata_check.py <unit> <first vram> <end vram>
(unit as in src/engine, e.g. gfx/display)
Builds build/<version>/src/engine/<unit>.c.o and compares its .sdata with the
bytes the game has at [first, end); the object's last bytes may be the
padding the linker adds."""
import sys, os, subprocess
from elftools.elf.elffile import ELFFile
D = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, f'{D}/tools')
import version
unit, first, end = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16)
obj = f'build/{version.VERSION}/src/engine/{unit}.c.o'
r = subprocess.run(['make', '-s', obj], cwd=D, capture_output=True, text=True)
if r.returncode:
    sys.exit(r.stdout + r.stderr)
sec = ELFFile(open(f'{D}/{obj}', 'rb')).get_section_by_name('.sdata')
have = sec.data() if sec else b''
exe = open(f'{D}/disks/{version.VERSION}/SLPS_033.57', 'rb').read()
want = exe[first - 0x80010000 + 0x800:end - 0x80010000 + 0x800]
pad = -len(have) % 4
if have + bytes(pad) == want:
    print(f'{unit}: .sdata OK ({len(have):#x} bytes)')
    sys.exit(0)
print(f'{unit}: .sdata differs ({len(have):#x} bytes, want {len(want):#x})')
for i in range(0, max(len(have), len(want)), 4):
    a, b = have[i:i + 4], want[i:i + 4]
    print(f'{first + i:08X} {a.hex():8} {b.hex():8} {"" if a.ljust(4, bytes(1)) == b else "**"}')
sys.exit(1)
