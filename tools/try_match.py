#!/usr/bin/env python3
"""Compile a C file with the project toolchain and compare every function in it
byte-wise against the original executable or overlay of the version being
worked on (VERSION, as for make; jp by default), with relocated fields
masked.

usage: tools/try_match.py draft.c [func ...] [--full] [--context N] [--max-rows N]

The functions are looked up in every unit of asm/<version>/; UNIT=<name> (or
any part of the path) picks one when several have the same name.

A function that differs gets one line with its diff count, the count of the
previous run (so an edit that changed nothing shows), and the diff class with
the compiler pass that decides it (the playbook's triage table). Then come
only the differing instructions, side by side (ours | original) and marked
with **, with N instructions of context around them; --full prints the
whole function.
"""
import argparse,difflib,glob,sys,subprocess,struct,re,os,tempfile
import rabbitizer
from elftools.elf.elffile import ELFFile
D=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,f'{D}/tools')
import version
A=str(version.ASM_DIR); BIN=str(version.BIN_DIR)
ap=argparse.ArgumentParser(usage=__doc__.split('\n\n')[1].split(': ',1)[1])
ap.add_argument('src'); ap.add_argument('funcs',nargs='*')
ap.add_argument('--full',action='store_true',help='print every instruction of a differing function')
ap.add_argument('--context',type=int,default=2,help='instructions shown around each difference')
ap.add_argument('--max-rows',type=int,default=60,help='rows printed per function before cutting')
args=ap.parse_args()
src=args.src; want=set(args.funcs)
w=os.path.join(tempfile.mkdtemp(prefix='try_match_'),'draft')
# each function's diff count of the previous run, to show what an edit changed
LAST=os.path.join(tempfile.gettempdir(),f'try_match_last_{os.getuid()}')
os.makedirs(LAST,exist_ok=True)
cc1=os.environ.get('CC1',f'{BIN}/gcc-2.95.2-psx/cc1')
# the executable was built with -G8 but its G0_UNITS with -G0
# (a draft named after its unit, as task.c for src/engine/task/task.c)
g0_units=re.search(r'^G0_UNITS :=(.*)',open(f'{D}/Makefile').read(),re.M).group(1).split()
stem=lambda path:os.path.splitext(os.path.basename(path))[0]
# the overlays were all built with -G0: a draft of their functions too
unit_filter=lambda paths:[p for p in paths if os.environ.get('UNIT','') in p.split(os.sep)]
units={os.path.relpath(p,A).split(os.sep)[0] for f in want for p in unit_filter(glob.glob(f'{A}/*/**/{f}.s',recursive=True))}
g='-G0' if stem(src) in map(stem,g0_units) or (units and 'main' not in units) else '-G8'
cflags=os.environ.get('CFLAGS',f'-O2 {g} -fsigned-char')
mflags=os.environ.get('MASPSXFLAGS',f'--aspsx-version=2.86 {g}')
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

def decode(word):
    """An instruction word as (opcode, operands with the registers kept, operands without them)"""
    i=rabbitizer.Instruction(word,0,rabbitizer.InstrCategory.R3000GTE)
    ops=i.disassemble().split(None,1)[1] if ' ' in i.disassemble() else ''
    return i.getOpcodeName(),ops,re.sub(r'\$\w+','$',ops)

def diff_class(ours,theirs):
    """The triage table's row for two lists of masked instruction words"""
    if len(ours)!=len(theirs):
        return f'{len(ours)} vs {len(theirs)} instructions: a computation added or dropped (combine -dc, CSE -ds, loop -dL, reload -dg)'
    a=[decode(x) for x in ours]; b=[decode(x) for x in theirs]
    if [x[0] for x in a]==[x[0] for x in b]:
        if [x[2] for x in a]==[x[2] for x in b]:
            return 'same instructions, other registers: register allocation (-dl .lreg, -dg .greg)'
        return 'same opcodes, other operands: constants, offsets or the frame layout (CSE -ds, combine -dc)'
    if sorted(x[:2] for x in a)==sorted(x[:2] for x in b):
        return 'same instructions, other order: scheduling (-dS sched1, -dR sched2) or the delay slots (-dd)'
    if sorted(x[0] for x in a)==sorted(x[0] for x in b):
        return 'same opcodes in another order and with other operands: scheduling and allocation together'
    return 'other instructions (combine -dc, jump -dJ, CSE -ds)'

def shown(bad):
    """The rows to print: the differing ones and their context, or every row with --full"""
    if args.full: return list(range(len(bad)))
    keep=set()
    for k,b in enumerate(bad):
        if b: keep.update(range(max(0,k-args.context),min(len(bad),k+args.context+1)))
    return sorted(keep)

for i,(off,name) in enumerate(syms):
    if want and name not in want: continue
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
    # align ours and the original by opcode, as asm-differ does, so that an
    # instruction added or dropped shows as one row, not as a shifted tail
    ours=[(o,struct.unpack('<I',text[o:o+4])[0]) for o in range(off,end,4)]
    theirs=[(k,struct.unpack('<I',binary[rom+4*k:][:4])[0]) for k in range(size//4)]
    sm=difflib.SequenceMatcher(None,[decode(x)[0] for _,x in ours],[decode(x)[0] for _,x in theirs],autojunk=False)
    pairs=[]
    for tag,i1,i2,j1,j2 in sm.get_opcodes():
        n=max(i2-i1,j2-j1)
        pairs+=[(ours[i1+k] if i1+k<i2 else None,theirs[j1+k] if j1+k<j2 else None) for k in range(n)]
    bad=[]; rows=[]; mo=[]; mt=[]
    for p,q in pairs:
        a=p[1] if p else None; b=q[1] if q else None
        if p and q and p[0] in rel:
            mk=0xfc000000 if (a>>26) in (2,3) else 0xffff0000; a&=mk; b&=mk
        if a is not None: mo.append(a)
        if b is not None: mt.append(b)
        bad.append(a!=b)
        rows.append(f"{'**' if a!=b else '  '} {mine.get(p[0],'') if p else '':38s}| {tl[q[0]] if q and q[0]<len(tl) else ''}")
    nd=sum(bad)
    last_path=os.path.join(LAST,name)
    last=open(last_path).read().strip() if os.path.exists(last_path) else None
    open(last_path,'w').write(str(nd))
    was=f', was {last}' if last is not None and last!=str(nd) else ' (unchanged)' if last==str(nd) and nd else ''
    if nd==0:
        print(f'{name}: MATCH (size {end-off:#x})'); continue
    print(f'{name}: {nd} diffs{was} (size {end-off:#x} vs {size:#x})')
    print(f'   class: {diff_class(mo,mt)}')
    keep=shown(bad); prev=-1; printed=0
    for k in keep:
        if printed>=args.max_rows:
            print(f'   ... {sum(bad[k:])} more differing rows (--full or --max-rows)'); break
        if k>prev+1: print(f'   ... {k-prev-1} same')
        print(rows[k]); prev=k; printed+=1
    else:
        if prev<len(rows)-1: print(f'   ... {len(rows)-1-prev} same')
