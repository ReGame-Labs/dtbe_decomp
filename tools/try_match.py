#!/usr/bin/env python3
"""Compile a C file with the project toolchain and compare every function in it
byte-wise against the original executable or overlay of the version being
worked on (VERSION, as for make; jp by default), with relocated fields
masked.

usage: tools/try_match.py draft.c [func ...]

The functions are looked up in every unit of asm/<version>/; UNIT=<name> (or
any part of the path) picks one when several have the same name.

Functions that differ are printed side by side (ours | original) with the
differing instructions marked with **.
"""
import sys,subprocess,struct,re,os,tempfile
from elftools.elf.elffile import ELFFile
D=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,f'{D}/tools')
import version
A=str(version.ASM_DIR); BIN=str(version.BIN_DIR)
src=sys.argv[1]; want=set(sys.argv[2:])
w=os.path.join(tempfile.mkdtemp(prefix='try_match_'),'draft')
cc1=os.environ.get('CC1',f'{BIN}/gcc-2.95.2-psx/cc1')
cflags=os.environ.get('CFLAGS','-O2 -G0 -fsigned-char')
mflags=os.environ.get('MASPSXFLAGS','--aspsx-version=2.86 -G0')
cmd=f"mipsel-linux-gnu-cpp -P -undef -D__GNUC__=2 -D__GNUC_MINOR__=95 -Dmips -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -I{D}/include -I{D}/external/psyq_headers/psyq_lib47/include -DSKIP_ASM -DVERSION_{version.VERSION.upper()} {src} > {w}.i && {cc1} -quiet -mips1 -mcpu=3000 -mgas -msoft-float -fgnu-linker -Wall -Wno-unused {cflags} -o {w}.s {w}.i && python3 {D}/external/maspsx/maspsx.py {mflags} < {w}.s > {w}.ms.s && mipsel-linux-gnu-as -EL -march=r3000 -no-pad-sections -O1 -G0 -o {w}.o {w}.ms.s"
r=subprocess.run(cmd,shell=True,capture_output=True,text=True)
if r.returncode: print(r.stderr); sys.exit(1)
if r.stderr.strip(): print(r.stderr.strip())
e=ELFFile(open(w+'.o','rb'))
text=e.get_section_by_name('.text').data()
rel={}
rs=e.get_section_by_name('.rel.text')
if rs: rel={x['r_offset']:x for x in rs.iter_relocations()}
syms=sorted([(s['st_value'],s.name) for s in e.get_section_by_name('.symtab').iter_symbols() if s['st_info']['type']=='STT_FUNC'])
dis=subprocess.run(['mipsel-linux-gnu-objdump','-d','-z','--no-show-raw-insn',w+'.o'],capture_output=True,text=True).stdout
mine={}
for l in dis.splitlines():
    m=re.match(r'\s+([0-9a-f]+):\s+(.*)',l)
    if m: mine[int(m.group(1),16)]=re.sub(r'\s+',' ',m.group(2)).strip()
for i,(off,name) in enumerate(syms):
    if want and name not in want: continue
    m=re.match(r'func_([0-9A-F]{8})',name)
    import glob
    unit_filter=lambda paths:[p for p in paths if os.environ.get('UNIT','') in p.split(os.sep)]
    asm=next(iter(unit_filter(glob.glob(f'{A}/*/nonmatchings/**/{name}.s',recursive=True))),None)
    if asm is not None:
        t=open(asm).read()
    else:
        # already in C: take it from splat's full disassembly
        t=None
        for full in unit_filter(glob.glob(f'{A}/*/**/*.s',recursive=True)):
            if '/nonmatchings/' in full: continue
            ft=open(full).read()
            k=ft.find(f'nonmatching {name}, ')
            if k>=0:
                e=ft.find(f'endlabel {name}',k)
                t=ft[k:e]; asm=full; break
        if t is None: print(name,'?'); continue
    # the unit is asm/<version>/<unit>/...: its binary is
    # config/<version>/<unit>.yaml's target_path
    parts=os.path.relpath(asm,A).split(os.sep)
    unit=parts[0]
    yaml=f'{version.CONFIG_DIR}/{unit}.yaml'
    target=re.search(r'target_path:\s*(\S+)',open(yaml).read()).group(1)
    binary=open(f'{D}/{target}','rb').read()
    size=int(re.search(r'nonmatching \w+, 0x([0-9A-F]+)',t).group(1),16)
    rom=int(re.search(r'glabel '+name+r'\n\s+/\* ([0-9A-F]+) [0-9A-F]{8} ',t).group(1),16)
    end=syms[i+1][0] if i+1<len(syms) else len(text)
    body=t[t.index('glabel '+name):] if 'glabel '+name in t else t
    tl=[re.sub(r'\s+',' ',re.sub(r'.*\*/\s+','',l)).strip() for l in body.splitlines() if re.match(r'\s+/\*',l)]
    nd=0; rows=[]
    for k in range(max(size,end-off)//4):
        o=off+4*k
        a=struct.unpack('<I',text[o:o+4])[0] if o<end else None
        b=struct.unpack('<I',binary[rom+4*k:][:4])[0] if 4*k<size else None
        if a is not None and b is not None and o in rel:
            mk=0xfc000000 if (a>>26) in (2,3) else 0xffff0000; a&=mk; b&=mk
        bad=a!=b; nd+=bad
        rows.append(f"{'**' if bad else '  '} {mine.get(o,'') if o<end else '':38s}| {tl[k] if k<len(tl) else ''}")
    print(f'{name}: {"MATCH" if nd==0 else str(nd)+" diffs"} (size {end-off:#x} vs {size:#x})')
    if nd: print('\n'.join(rows))
