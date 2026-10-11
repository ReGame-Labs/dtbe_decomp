#!/usr/bin/env python3
"""Find the functions whose code looks like a given one, or group them all
into families, across the executable and every overlay.

usage: tools/similar.py FUNC [-n 8] [--min 0.8]
       tools/similar.py --families [--min 0.9] [-o FILE]

A function is read from splat's per-function asm of the version being worked
on (VERSION, as for make; jp by default): asm/<version>/<binary>/matchings/
for the ones already in C, nonmatchings/ for the ones still in asm. Two
functions are compared by the edit distance between their instruction
sequences, with the symbols and labels they name masked (sotn-decomp's
tools/dups does the same), so a copy that only uses other addresses scores
1.0 and is marked "same". The masking hides the functions called, so a
relative that calls others is marked "other calls"; --opcodes compares the opcodes alone, which also
finds the copies built with other registers or constants.

FUNC prints its closest functions, so a function in asm can start from the C
of one already matched. --families groups every function with the ones that
score at least --min with it, and lists first the families that have a
member in C and some still in asm, the most asm bytes first.
"""
import argparse,glob,os,re,sys
from rapidfuzz import process
from rapidfuzz.distance import Levenshtein
D=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0,f'{D}/tools')
import version
A=str(version.ASM_DIR)
INSN=re.compile(r'^\s+/\* [0-9A-F]+ [0-9A-F]{8} [0-9A-F]{8} \*/\s+(\S+)\s*(.*)$',re.M)
# operands that name a symbol or a label: the addresses a copy may differ in
SYMBOL=re.compile(r'%\w+\([^)]*\)|(?<![\w$.])[A-Za-z_.][\w.]*')

class Function:
    def __init__(self,path,in_c):
        self.name=os.path.basename(path)[:-2]
        rel=os.path.relpath(path,A).split(os.sep)
        self.binary=rel[0]; self.in_c=in_c
        # (a jump table splat put in the function's file is data, not code)
        insns=[i for i in INSN.findall(open(path).read()) if not i[0].startswith('.')]
        self.size=4*len(insns)
        self.shape=tuple((op,SYMBOL.sub('S',args)) for op,args in insns)
        # the functions it calls, which the masking hides
        self.calls=tuple(args for op,args in insns if op=='jal')
        # one character per instruction, so that the edit distance runs on
        # strings: the instruction without its addresses, or only its opcode
        key=(lambda i:i[0]) if ARGS.opcodes else (lambda i:i)
        self.opcodes=''.join(KEYS.setdefault(key(i),chr(0x100+len(KEYS))) for i in self.shape)

KEYS={}
def load():
    funcs=[]
    for kind,in_c in (('matchings',True),('nonmatchings',False)):
        for path in glob.glob(f'{A}/*/{kind}/**/*.s',recursive=True):
            f=Function(path,in_c)
            if f.size: funcs.append(f)
    return funcs

def describe(f,score,ref=None):
    same=' same' if ref is not None and f.shape==ref.shape else ''
    calls=' other calls' if ref is not None and f.calls!=ref.calls else ''
    return f"{score:.2f} {'C  ' if f.in_c else 'asm'} {f.name:28s} {f.binary:6s} {f.size:#6x}{same}{calls}"

def nearest(funcs,name,n,cutoff):
    ref=next((f for f in funcs if f.name==name),None)
    if ref is None: sys.exit(f'{name}: no such function in {A}')
    print(describe(ref,1.0)+'  <- this one')
    hits=process.extract(ref.opcodes,[f.opcodes for f in funcs],scorer=Levenshtein.normalized_similarity,
                         score_cutoff=cutoff,limit=n+1)
    for _,score,k in hits:
        if funcs[k] is not ref: print(describe(funcs[k],score,ref))

def families(funcs,cutoff,out):
    # functions shorter than 4 instructions (empty bodies, getters) match too
    # many others to say anything: they stay out of the families
    funcs=[f for f in funcs if f.size>=16]
    # a pair can only reach cutoff when the shorter is at least cutoff times
    # the longer: compare each function with the next ones by length only
    funcs.sort(key=lambda f:len(f.opcodes))
    parent=list(range(len(funcs)))
    def root(k):
        while parent[k]!=k: parent[k]=parent[parent[k]]; k=parent[k]
        return k
    for i,f in enumerate(funcs):
        j=i+1
        while j<len(funcs) and cutoff*len(funcs[j].opcodes)<=len(f.opcodes): j+=1
        for _,_,k in process.extract(f.opcodes,[g.opcodes for g in funcs[i+1:j]],limit=None,
                                     scorer=Levenshtein.normalized_similarity,score_cutoff=cutoff):
            parent[root(i)]=root(i+1+k)
    groups={}
    for k,f in enumerate(funcs): groups.setdefault(root(k),[]).append(f)
    fams=[g for g in groups.values() if len(g)>1]
    asm_bytes=lambda g:sum(f.size for f in g if not f.in_c)
    ready=lambda g:any(f.in_c for f in g) and asm_bytes(g)>0
    fams.sort(key=lambda g:(not ready(g),-asm_bytes(g)))
    lines=[]
    for g in fams:
        c=[f.name for f in g if f.in_c]; a=[f for f in g if not f.in_c]
        if not a: continue
        head=f"{'ready' if c else 'asm  '} {len(a)} in asm, {asm_bytes(g):#x} bytes"
        lines.append(head+(f"  C: {' '.join(c)}" if c else ''))
        lines.append('    '+' '.join(f'{f.name}({f.binary},{f.size:#x})' for f in sorted(a,key=lambda f:-f.size)))
    open(out,'w').write('\n'.join(lines)+'\n') if out else print('\n'.join(lines))
    n_ready=sum(1 for g in fams if ready(g))
    total=sum(asm_bytes(g) for g in fams)
    print(f'{len(fams)} families at {cutoff}: {n_ready} with a member in C; {total:#x} bytes of asm in them',
          file=sys.stderr)

ap=argparse.ArgumentParser(usage=__doc__.split('\n\n')[1].split(': ',1)[1])
ap.add_argument('func',nargs='?')
ap.add_argument('-n',type=int,default=8,help='how many functions to print')
ap.add_argument('--min',type=float,help='the lowest score kept (0.8 for FUNC, 0.9 for --families)')
ap.add_argument('--families',action='store_true')
ap.add_argument('--opcodes',action='store_true',help='compare opcodes only, so other registers and constants still score 1.0')
ap.add_argument('-o',dest='out',help='write the families to this file')
ARGS=ap.parse_args()
funcs=load()
if ARGS.families: families(funcs,ARGS.min or 0.9,ARGS.out)
elif ARGS.func: nearest(funcs,ARGS.func,ARGS.n,ARGS.min or 0.8)
else: ap.print_usage()
