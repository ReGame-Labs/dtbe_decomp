#ifndef DTBE_LIB_INFLATE_H
#define DTBE_LIB_INFLATE_H

/* The inflate of zlib 1.1.3, unpacking the game's compressed files: streams and tables. */

#include "common.h"
#include "engine/cd/cdfs.h"

EXTERN_C_BEGIN

/*
 * zlib 1.1.3's decompressor (inflate.c, adler32.c, infblock.c, infcodes.c,
 * inftrees.c, infutil.c, inffast.c and zutil.c), which unpacks the game's
 * compressed files. The functions keep their addresses as names; each one's
 * comment gives its zlib name. It was built with inlining on: the small
 * functions are copied into their callers in the same file.
 */

/* the return codes */
#define Z_OK 0
#define Z_STREAM_END 1
#define Z_NEED_DICT 2
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR (-3)
#define Z_MEM_ERROR (-4)
#define Z_BUF_ERROR (-5)
#define Z_VERSION_ERROR (-6)

/* the flush value of inflate() that asks for all the output at once */
#define Z_FINISH 4

/* the compression method of the zlib header: deflate */
#define Z_DEFLATED 8
/* the smallest and largest windows, as powers of 2, and the default one */
#define MIN_WBITS 8
#define MAX_WBITS 15
#define DEF_WBITS MAX_WBITS

/* the largest Huffman tables a stream can need, in entries */
#define MANY 1440

/* the largest number of bytes adler32 can sum before s2 overflows 32 bits */
#define ADLER_NMAX 5552
/* the largest prime below 65536: adler32 sums modulo it */
#define ADLER_BASE 65521

typedef u32 (*CheckFunc)(u32 check, const u8 *buf, u32 len);

/* An entry of a Huffman decoding table (inflate_huft). */
typedef struct InflateHuft {
    /* 0x0 */ u8 exop; /* number of extra bits, or the operation */
    /* 0x1 */ u8 bits; /* number of bits of this code or subcode */
    /* 0x4 */ u32 base; /* literal, length base, distance base or table offset */
} InflateHuft;

/*
 * The bits of InflateHuft's exop. Without HUFT_LEAF, exop is the bits of the
 * next table, which base links to.
 */
#define HUFT_EXTRA 16 /* a length or distance base, with exop & 15 extra bits */
#define HUFT_EXTRA_BITS 15
#define HUFT_END 32 /* the end of the block */
#define HUFT_LEAF 64 /* no next table: a literal, base, end of block or bad code */
#define HUFT_INVALID 128 /* a code no value has */

/* what inflate_codes is doing */
typedef enum {
    CODES_START,   /* set up for LEN */
    CODES_LEN,     /* get length/literal/eob next */
    CODES_LENEXT,  /* getting length extra (have base) */
    CODES_DIST,    /* get distance next */
    CODES_DISTEXT, /* getting distance extra */
    CODES_COPY,    /* copying bytes from the window, waiting for space */
    CODES_LIT,     /* got a literal, waiting for output space */
    CODES_WASH,    /* got eob, possibly still output waiting */
    CODES_END,     /* got eob and all data flushed */
    CODES_BADCODE  /* got an error: stay here */
} InflateCodesMode;

/* The state of the decoding of a block's codes (inflate_codes_state). */
typedef struct InflateCodesState {
    /* 0x0 */ InflateCodesMode mode;
    /* 0x4 */ u32 len;
    /* 0x8 */ union {
        struct {
            /* 0x8 */ InflateHuft *tree; /* pointer into tree */
            /* 0xC */ u32 need; /* bits needed */
        } code;
        /* 0x8 */ u32 lit; /* if LIT, the literal */
        struct {
            /* 0x8 */ u32 get; /* bits to get for extra */
            /* 0xC */ u32 dist; /* distance back to copy from */
        } copy;
    } sub;
    /* 0x10 */ u8 lbits; /* ltree bits decoded per branch */
    /* 0x11 */ u8 dbits; /* dtree bits decoded per branch */
    /* 0x14 */ InflateHuft *ltree; /* literal/length/eob tree */
    /* 0x18 */ InflateHuft *dtree; /* distance tree */
} InflateCodesState;

/* what inflate_blocks is doing */
typedef enum {
    BLOCKS_TYPE,   /* get type bits (3, including the end bit) */
    BLOCKS_LENS,   /* get lengths for stored */
    BLOCKS_STORED, /* processing a stored block */
    BLOCKS_TABLE,  /* get table lengths */
    BLOCKS_BTREE,  /* get the bit lengths tree of a dynamic block */
    BLOCKS_DTREE,  /* get the length and distance trees of a dynamic block */
    BLOCKS_CODES,  /* processing a fixed or dynamic block */
    BLOCKS_DRY,    /* output the remaining window bytes */
    BLOCKS_DONE,   /* finished the last block, done */
    BLOCKS_BAD     /* got a data error: stuck here */
} InflateBlocksMode;

/* The state of the decoding of the blocks and the sliding window
 * (inflate_blocks_state). */
typedef struct InflateBlocksState {
    /* 0x0 */ InflateBlocksMode mode;
    /* 0x4 */ union {
        /* 0x4 */ u32 left; /* if STORED, bytes left to copy */
        struct {
            /* 0x4 */ u32 table; /* table lengths (14 bits) */
            /* 0x8 */ u32 index; /* index into blens (or border) */
            /* 0xC */ u32 *blens; /* bit lengths of codes */
            /* 0x10 */ u32 bb; /* bit length tree depth */
            /* 0x14 */ InflateHuft *tb; /* bit length decoding tree */
        } trees; /* if DTREE, decoding info for the trees */
        struct {
            /* 0x4 */ InflateCodesState *codes;
        } decode; /* if CODES, the current state */
    } sub;
    /* 0x18 */ u32 last; /* true if this block is the last block */
    /* 0x1C */ u32 bitk; /* bits in the bit buffer */
    /* 0x20 */ u32 bitb; /* bit buffer */
    /* 0x24 */ InflateHuft *hufts; /* single allocation for the tree space */
    /* 0x28 */ u8 *window; /* sliding window */
    /* 0x2C */ u8 *end; /* one byte after the sliding window */
    /* 0x30 */ u8 *read; /* window read pointer */
    /* 0x34 */ u8 *write; /* window write pointer */
    /* 0x38 */ CheckFunc checkfn; /* check function */
    /* 0x3C */ u32 check; /* check on output */
} InflateBlocksState;

/* what inflate is doing */
typedef enum {
    INFLATE_METHOD, /* waiting for the method byte */
    INFLATE_FLAG,   /* waiting for the flag byte */
    INFLATE_DICT4,  /* four dictionary check bytes to go */
    INFLATE_DICT3,  /* three dictionary check bytes to go */
    INFLATE_DICT2,  /* two dictionary check bytes to go */
    INFLATE_DICT1,  /* one dictionary check byte to go */
    INFLATE_DICT0,  /* waiting for inflateSetDictionary */
    INFLATE_BLOCKS, /* decompressing blocks */
    INFLATE_CHECK4, /* four check bytes to go */
    INFLATE_CHECK3, /* three check bytes to go */
    INFLATE_CHECK2, /* two check bytes to go */
    INFLATE_CHECK1, /* one check byte to go */
    INFLATE_DONE,   /* finished the check, done */
    INFLATE_BAD     /* got an error: stay here */
} InflateMode;

/* The state of a stream's decompression (zlib's internal_state). */
typedef struct InflateState {
    /* 0x0 */ InflateMode mode;
    /* 0x4 */ union {
        /* 0x4 */ u32 method; /* if FLAGS, the method byte */
        struct {
            /* 0x4 */ u32 was; /* computed check value */
            /* 0x8 */ u32 need; /* stream check value */
        } check; /* if CHECK, the check values to compare */
        /* 0x4 */ u32 marker; /* if BAD, inflateSync's marker bytes count */
    } sub;
    /* 0xC */ s32 nowrap; /* no zlib header and check */
    /* 0x10 */ u32 wbits; /* log2(window size) (8..15) */
    /* 0x14 */ InflateBlocksState *blocks;
} InflateState;

/* A stream: the caller's buffers, counters and allocator (z_stream). */
typedef struct ZStream {
    /* 0x0 */ u8 *nextIn; /* next input byte */
    /* 0x4 */ u32 availIn; /* number of bytes available at nextIn */
    /* 0x8 */ u32 totalIn; /* total number of input bytes read so far */
    /* 0xC */ u8 *nextOut; /* where the next output byte goes */
    /* 0x10 */ u32 availOut; /* remaining free space at nextOut */
    /* 0x14 */ u32 totalOut; /* total number of bytes output so far */
    /* 0x18 */ const char *msg; /* last error message, NULL if no error */
    /* 0x1C */ InflateState *state;
    /* 0x20 */ AllocFunc zalloc; /* allocates the internal state */
    /* 0x24 */ FreeFunc zfree; /* frees the internal state */
    /* 0x28 */ void *opaque; /* private data passed to zalloc and zfree */
    /* 0x2C */ s32 dataType; /* best guess about the data type: ascii or binary */
    /* 0x30 */ u32 adler; /* adler32 value of the uncompressed data */
    /* 0x34 */ u32 reserved;
} ZStream;

/* the version of zlib */
#define ZLIB_VERSION "1.1.3"

#define ZALLOC(strm, items, size) (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr) (*((strm)->zfree))((strm)->opaque, (void *)(addr))

/* the order of the bit lengths of the code lengths (border) */
extern const u32 border[];
/* "invalid distance code", "invalid literal/length code" (inflate_fast's) */
extern const char STR_INVALID_DISTANCE_CODE[];
extern const char STR_INVALID_LITERAL_LENGTH_CODE[];
/* inftrees.c's copyright notice */
extern const char inflate_copyright[];
/* the bases and extra bits of the length codes 257..285 (cplens, cplext) */
extern const u32 cplens[];
extern const u32 cplext[];
/* the bases and extra bits of the distance codes 0..29 (cpdist, cpdext) */
extern const u32 cpdist[];
extern const u32 cpdext[];
/* "oversubscribed dynamic bit lengths tree" */
extern const char STR_OVERSUBSCRIBED_BIT_LENGTHS_TREE[];
/* "incomplete dynamic bit lengths tree" */
extern const char STR_INCOMPLETE_BIT_LENGTHS_TREE[];
/* zError's messages, from Z_VERSION_ERROR up */
extern const char STR_INCOMPATIBLE_VERSION[];
extern const char STR_BUFFER_ERROR[];
extern const char STR_INSUFFICIENT_MEMORY[];
extern const char STR_DATA_ERROR[];
extern const char STR_STREAM_ERROR[];
extern const char STR_FILE_ERROR[];
extern const char STR_STREAM_END[];
extern const char STR_NEED_DICTIONARY[];
/* masks of the low 0..16 bits (inflate_mask) */
extern u32 inflate_mask[];
/* the messages of the error codes, from Z_NEED_DICT down (zlib's z_errmsg) */
extern const char *z_errmsg[];
extern InflateHuft fixed_tl[]; /* the fixed literal/length table */
extern InflateHuft fixed_td[]; /* the fixed distance table */

s32 inflateInit2_(ZStream *z, s32 w, const u8 *version, s32 streamSize);
s32 inflate(ZStream *z, s32 f);
s32 inflateSync(ZStream *z);
s32 inflateEnd(ZStream *z);
s32 inflateSetDictionary(ZStream *z, const u8 *dictionary, u32 dictLength);
s32 inflateReset(ZStream *z);
s32 inflateInit_(ZStream *z, const u8 *version, s32 streamSize);
s32 inflateSyncPoint(ZStream *z);
u32 adler32(u32 adler, const u8 *buf, u32 len);
InflateBlocksState *inflate_blocks_new(ZStream *z, CheckFunc c, u32 w);
s32 inflate_blocks(InflateBlocksState *s, ZStream *z, s32 r);
void inflate_blocks_reset(InflateBlocksState *s, ZStream *z, u32 *c);
s32 inflate_blocks_free(InflateBlocksState *s, ZStream *z);
void inflate_set_dictionary(InflateBlocksState *s, const u8 *d, u32 n);
s32 inflate_blocks_sync_point(InflateBlocksState *s);
s32 inflate_codes(InflateBlocksState *s, ZStream *z, s32 r);
InflateCodesState *inflate_codes_new(u32 bl, u32 bd, InflateHuft *tl, InflateHuft *td, ZStream *z);
void inflate_codes_free(InflateCodesState *c, ZStream *z);
s32 inflate_fast(u32 bl, u32 bd, InflateHuft *tl, InflateHuft *td, InflateBlocksState *s,
                  ZStream *z);
s32 huft_build(u32 *b, u32 n, u32 s, const u32 *d, const u32 *e, InflateHuft **t, u32 *m,
                  InflateHuft *hp, u32 *hn, u32 *v);
s32 inflate_trees_dynamic(u32 nl, u32 nd, u32 *c, u32 *bl, u32 *bd, InflateHuft **tl, InflateHuft **td,
                  InflateHuft *hp, ZStream *z);
s32 inflate_trees_bits(u32 *c, u32 *bb, InflateHuft **tb, InflateHuft *hp, ZStream *z);
s32 inflate_trees_fixed(u32 *bl, u32 *bd, InflateHuft **tl, InflateHuft **td, ZStream *z);
s32 inflate_flush(InflateBlocksState *s, ZStream *z, s32 r);
const char *zlibVersion(void);
const char *zError(s32 err);
void *zcalloc(void *opaque, u32 items, u32 size);
void zcfree(void *opaque, void *ptr);

EXTERN_C_END

#endif /* DTBE_LIB_INFLATE_H */
