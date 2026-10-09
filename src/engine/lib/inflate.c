#include "common.h"
#include "engine/lib/inflate.h"
#include "engine/system/main.h"
#include "memory.h"

/* adler32's sum of 16 bytes, unrolled */
#define ADLER_DO1(buf, i) \
    s1 += (buf)[i];       \
    s2 += s1;
#define ADLER_DO2(buf, i) ADLER_DO1(buf, i) ADLER_DO1(buf, i + 1)
#define ADLER_DO4(buf, i) ADLER_DO2(buf, i) ADLER_DO2(buf, i + 2)
#define ADLER_DO8(buf, i) ADLER_DO4(buf, i) ADLER_DO4(buf, i + 4)
#define ADLER_DO16(buf) ADLER_DO8(buf, 0) ADLER_DO8(buf, 8)

/*
 * The bodies of inflate_blocks_reset, inflateReset and inflateEnd. The
 * original build inlined them into their callers in the same file, which
 * come before the functions themselves, so the bodies are shared from here.
 */

/* inflate_blocks_reset: starts over at a block boundary, handing out the
 * check of the output so far in *c. */
static inline void inflate_blocks_reset_inline(InflateBlocksState *s, ZStream *z, u32 *c) {
    if (c != NULL) {
        *c = s->check;
    }
    if (s->mode == BLOCKS_BTREE || s->mode == BLOCKS_DTREE) {
        ZFREE(z, s->sub.trees.blens);
    }
    if (s->mode == BLOCKS_CODES) {
        inflate_codes_free(s->sub.decode.codes, z);
    }
    s->mode = BLOCKS_TYPE;
    s->bitk = 0;
    s->bitb = 0;
    s->read = s->write = s->window;
    if (s->checkfn != NULL) {
        z->adler = s->check = s->checkfn(0, NULL, 0);
    }
}

/* inflateReset: starts the stream over, keeping its window and allocations. */
static inline s32 inflateResetInline(ZStream *z) {
    if (z == NULL || z->state == NULL) {
        return Z_STREAM_ERROR;
    }
    z->totalIn = z->totalOut = 0;
    z->msg = NULL;
    z->state->mode = z->state->nowrap ? INFLATE_BLOCKS : INFLATE_METHOD;
    inflate_blocks_reset(z->state->blocks, z, NULL);
    return Z_OK;
}

/* inflateEnd: frees all the memory of the stream. */
static inline s32 inflateEndInline(ZStream *z) {
    if (z == NULL || z->state == NULL || z->zfree == NULL) {
        return Z_STREAM_ERROR;
    }
    if (z->state->blocks != NULL) {
        inflate_blocks_free(z->state->blocks, z);
    }
    ZFREE(z, z->state);
    z->state = NULL;
    return Z_OK;
}

/* inflateInit2_: sets up a stream to decompress with a window of 2^w bytes,
 * or of 2^-w bytes without the zlib header and check when w is negative. */
s32 inflateInit2_(ZStream *z, s32 w, const u8 *version, s32 streamSize) {
    if (version == NULL || version[0] != ZLIB_VERSION[0] || streamSize != sizeof(ZStream)) {
        return Z_VERSION_ERROR;
    }
    if (z == NULL) {
        return Z_STREAM_ERROR;
    }
    z->msg = NULL;
    if (z->zalloc == NULL) {
        z->zalloc = zcalloc;
        z->opaque = NULL;
    }
    if (z->zfree == NULL) {
        z->zfree = zcfree;
    }
    if ((z->state = ZALLOC(z, 1, sizeof(InflateState))) == NULL) {
        return Z_MEM_ERROR;
    }
    z->state->blocks = NULL;
    z->state->nowrap = 0;
    if (w < 0) {
        w = -w;
        z->state->nowrap = 1;
    }
    if (w < MIN_WBITS || w > MAX_WBITS) {
        inflateEndInline(z);
        return Z_STREAM_ERROR;
    }
    z->state->wbits = w;
    if ((z->state->blocks = inflate_blocks_new(z, z->state->nowrap ? NULL : adler32, 1 << w))
        == NULL) {
        inflateEndInline(z);
        return Z_MEM_ERROR;
    }
    inflateResetInline(z);
    return Z_OK;
}

/* the flag byte's bit for a preset dictionary */
#define PRESET_DICT 0x20
/* inflateSync's marker count for a bad header: above the 4 marker bytes, so
 * that inflateSync gives up at once */
#define SYNC_IMPOSSIBLE 5

/* inflate()'s byte input, which counts what it reads in the stream */
#define NEEDBYTE                \
    {                           \
        if (z->availIn == 0) {  \
            return r;           \
        }                       \
        r = f;                  \
    }
#define NEXTBYTE (z->availIn--, z->totalIn++, *z->nextIn++)

/* inflate: decompresses as much as the input and output allow; f is
 * Z_FINISH when all the input is there and the output has room for it all. */
s32 inflate(ZStream *z, s32 f) {
    s32 r;
    u32 b;

    if (z == NULL || z->state == NULL || z->nextIn == NULL) {
        return Z_STREAM_ERROR;
    }
    f = f == Z_FINISH ? Z_BUF_ERROR : Z_OK;
    r = Z_BUF_ERROR;
    while (1) {
        switch (z->state->mode) {
        case INFLATE_METHOD:
            NEEDBYTE
            if (((z->state->sub.method = NEXTBYTE) & 0xF) != Z_DEFLATED) {
                z->state->mode = INFLATE_BAD;
                z->msg = "unknown compression method";
                z->state->sub.marker = SYNC_IMPOSSIBLE;
                break;
            }
            if ((z->state->sub.method >> 4) + 8 > z->state->wbits) {
                z->state->mode = INFLATE_BAD;
                z->msg = "invalid window size";
                z->state->sub.marker = SYNC_IMPOSSIBLE;
                break;
            }
            z->state->mode = INFLATE_FLAG;
        case INFLATE_FLAG:
            NEEDBYTE
            b = NEXTBYTE;
            /* the method and flag bytes, as a 16-bit number, are a multiple of 31 */
            if (((z->state->sub.method << 8) + b) % 31) {
                z->state->mode = INFLATE_BAD;
                z->msg = "incorrect header check";
                z->state->sub.marker = SYNC_IMPOSSIBLE;
                break;
            }
            if (!(b & PRESET_DICT)) {
                z->state->mode = INFLATE_BLOCKS;
                break;
            }
            z->state->mode = INFLATE_DICT4;
        case INFLATE_DICT4:
            NEEDBYTE
            z->state->sub.check.need = (u32)NEXTBYTE << 24;
            z->state->mode = INFLATE_DICT3;
        case INFLATE_DICT3:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE << 16;
            z->state->mode = INFLATE_DICT2;
        case INFLATE_DICT2:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE << 8;
            z->state->mode = INFLATE_DICT1;
        case INFLATE_DICT1:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE;
            z->adler = z->state->sub.check.need;
            z->state->mode = INFLATE_DICT0;
            return Z_NEED_DICT;
        case INFLATE_DICT0:
            z->state->mode = INFLATE_BAD;
            z->msg = "need dictionary";
            z->state->sub.marker = 0; /* inflateSync can try */
            return Z_STREAM_ERROR;
        case INFLATE_BLOCKS:
            r = inflate_blocks(z->state->blocks, z, r);
            if (r == Z_DATA_ERROR) {
                z->state->mode = INFLATE_BAD;
                z->state->sub.marker = 0; /* inflateSync can try */
                break;
            }
            if (r == Z_OK) {
                r = f;
            }
            if (r != Z_STREAM_END) {
                return r;
            }
            r = f;
            inflate_blocks_reset(z->state->blocks, z, &z->state->sub.check.was);
            if (z->state->nowrap) {
                z->state->mode = INFLATE_DONE;
                break;
            }
            z->state->mode = INFLATE_CHECK4;
        case INFLATE_CHECK4:
            NEEDBYTE
            z->state->sub.check.need = (u32)NEXTBYTE << 24;
            z->state->mode = INFLATE_CHECK3;
        case INFLATE_CHECK3:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE << 16;
            z->state->mode = INFLATE_CHECK2;
        case INFLATE_CHECK2:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE << 8;
            z->state->mode = INFLATE_CHECK1;
        case INFLATE_CHECK1:
            NEEDBYTE
            z->state->sub.check.need += (u32)NEXTBYTE;
            if (z->state->sub.check.was != z->state->sub.check.need) {
                z->state->mode = INFLATE_BAD;
                z->msg = "incorrect data check";
                z->state->sub.marker = SYNC_IMPOSSIBLE;
                break;
            }
            z->state->mode = INFLATE_DONE;
        case INFLATE_DONE:
            return Z_STREAM_END;
        case INFLATE_BAD:
            return Z_DATA_ERROR;
        default:
            return Z_STREAM_ERROR;
        }
    }
}

#undef NEEDBYTE
#undef NEXTBYTE

/* the bytes a full flush ends with (an empty stored block), which
 * inflateSync looks for */
static const u8 mark[4] = { 0, 0, 0xFF, 0xFF };

/* inflateSync: skips the input up to the marker of a full flush and starts
 * decompressing over after it. */
s32 inflateSync(ZStream *z) {
    u32 n;
    u8 *p;
    u32 m;
    u32 r;
    u32 w;

    if (z == NULL || z->state == NULL) {
        return Z_STREAM_ERROR;
    }
    if (z->state->mode != INFLATE_BAD) {
        z->state->mode = INFLATE_BAD;
        z->state->sub.marker = 0;
    }
    if ((n = z->availIn) == 0) {
        return Z_BUF_ERROR;
    }
    p = z->nextIn;
    m = z->state->sub.marker;

    while (n != 0 && m < 4) {
        if (*p == mark[m]) {
            m++;
        } else if (*p != 0) {
            m = 0;
        } else {
            m = 4 - m;
        }
        p++;
        n--;
    }

    z->totalIn += p - z->nextIn;
    z->nextIn = p;
    z->availIn = n;
    z->state->sub.marker = m;
    if (m != 4) {
        return Z_DATA_ERROR;
    }
    r = z->totalIn;
    w = z->totalOut;
    inflateResetInline(z);
    z->totalIn = r;
    z->totalOut = w;
    z->state->mode = INFLATE_BLOCKS;
    return Z_OK;
}

/* inflateEnd: frees all the memory of a stream. */
s32 inflateEnd(ZStream *z) {
    return inflateEndInline(z);
}

/* inflateSetDictionary: gives the preset dictionary the stream asked for. */
s32 inflateSetDictionary(ZStream *z, const u8 *dictionary, u32 dictLength) {
    u32 length = dictLength;

    if (z == NULL || z->state == NULL || z->state->mode != INFLATE_DICT0) {
        return Z_STREAM_ERROR;
    }
    if (adler32(1, dictionary, dictLength) != z->adler) {
        return Z_DATA_ERROR;
    }
    z->adler = 1;
    if (length >= (1U << z->state->wbits)) {
        length = (1 << z->state->wbits) - 1;
        dictionary += dictLength - length;
    }
    inflate_set_dictionary(z->state->blocks, dictionary, length);
    z->state->mode = INFLATE_BLOCKS;
    return Z_OK;
}

/* inflateReset: starts the stream over, keeping its window and allocations. */
s32 inflateReset(ZStream *z) {
    return inflateResetInline(z);
}

/* inflateInit_: sets up a stream to decompress with the default window. */
s32 inflateInit_(ZStream *z, const u8 *version, s32 streamSize) {
    return inflateInit2_(z, DEF_WBITS, version, streamSize);
}

/* inflateSyncPoint: whether the stream stands at the end of a block that a
 * full flush ended, where decompression can start over. */
s32 inflateSyncPoint(ZStream *z) {
    if (z == NULL || z->state == NULL || z->state->blocks == NULL) {
        return Z_STREAM_ERROR;
    }
    return inflate_blocks_sync_point(z->state->blocks);
}

/* adler32: updates an Adler-32 checksum with len bytes. */
u32 adler32(u32 adler, const u8 *buf, u32 len) {
    u32 s1 = adler & 0xFFFF;
    u32 s2 = (adler >> 16) & 0xFFFF;
    s32 k;

    if (buf == NULL) {
        return 1;
    }
    while (len > 0) {
        k = len < ADLER_NMAX ? len : ADLER_NMAX;
        len -= k;
        while (k >= 16) {
            ADLER_DO16(buf);
            buf += 16;
            k -= 16;
        }
        if (k != 0) {
            do {
                s1 += *buf++;
                s2 += s1;
            } while (--k);
        }
        s1 %= ADLER_BASE;
        s2 %= ADLER_BASE;
    }
    return (s2 << 16) | s1;
}

/* inflate_blocks_new: allocates the state of the decoding of the blocks, with
 * a window of w bytes and the check function c (NULL for none). */
InflateBlocksState *inflate_blocks_new(ZStream *z, CheckFunc c, u32 w) {
    InflateBlocksState *s = ZALLOC(z, 1, sizeof(InflateBlocksState));

    if (s == NULL) {
        return s;
    }
    if ((s->hufts = ZALLOC(z, sizeof(InflateHuft), MANY)) == NULL) {
        ZFREE(z, s);
        return NULL;
    }
    if ((s->window = ZALLOC(z, 1, w)) == NULL) {
        ZFREE(z, s->hufts);
        ZFREE(z, s);
        return NULL;
    }
    s->end = s->window + w;
    s->checkfn = c;
    s->mode = BLOCKS_TYPE;
    inflate_blocks_reset_inline(s, z, NULL);
    return s;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", border);

/*
 * The decoders keep the state of the input, the bit buffer and the window
 * in locals while they run (zlib's infutil.h): p and n are the input and its
 * count, b and k the bit buffer and its bit count, q the window's write
 * pointer and m the room after it.
 */
#define UPDBITS          \
    {                    \
        s->bitb = b;     \
        s->bitk = k;     \
    }
#define UPDIN                         \
    {                                 \
        z->availIn = n;               \
        z->totalIn += p - z->nextIn;  \
        z->nextIn = p;                \
    }
#define UPDOUT          \
    {                   \
        s->write = q;   \
    }
#define UPDATE  \
    {           \
        UPDBITS \
        UPDIN   \
        UPDOUT  \
    }
#define LEAVE                                   \
    {                                           \
        UPDATE return inflate_flush(s, z, r);   \
    }
#define LOADIN              \
    {                       \
        p = z->nextIn;      \
        n = z->availIn;     \
        b = s->bitb;        \
        k = s->bitk;        \
    }
#define NEEDBYTE        \
    {                   \
        if (n) {        \
            r = Z_OK;   \
        } else {        \
            LEAVE       \
        }               \
    }
#define NEXTBYTE (n--, *p++)
#define NEEDBITS(j)                         \
    {                                       \
        while (k < (j)) {                   \
            NEEDBYTE;                       \
            b |= ((u32)NEXTBYTE) << k;      \
            k += 8;                         \
        }                                   \
    }
#define DUMPBITS(j)     \
    {                   \
        b >>= (j);      \
        k -= (j);       \
    }
/* the room in the window after q: up to the read pointer or the end */
#define WAVAIL (u32)(q < s->read ? s->read - q - 1 : s->end - q)
#define LOADOUT             \
    {                       \
        q = s->write;       \
        m = WAVAIL;         \
    }
#define WRAP                                        \
    {                                               \
        if (q == s->end && s->read != s->window) {  \
            q = s->window;                          \
            m = WAVAIL;                             \
        }                                           \
    }
#define FLUSH                               \
    {                                       \
        UPDOUT r = inflate_flush(s, z, r);  \
        LOADOUT                             \
    }
#define NEEDOUT                 \
    {                           \
        if (m == 0) {           \
            WRAP                \
            if (m == 0) {       \
                FLUSH           \
                WRAP            \
                if (m == 0) {   \
                    LEAVE       \
                }               \
            }                   \
        }                       \
        r = Z_OK;               \
    }
#define OUTBYTE(a)          \
    {                       \
        *q++ = (u8)(a);     \
        m--;                \
    }
#define LOAD    \
    {           \
        LOADIN  \
        LOADOUT \
    }

/* inflate_blocks: decodes blocks, their headers and their trees, as far as
 * the input and the room in the window allow. */
s32 inflate_blocks(InflateBlocksState *s, ZStream *z, s32 r) {
    u32 t;
    u32 b;
    u32 k;
    u8 *p;
    u32 n;
    u8 *q;
    u32 m;

    LOAD
    while (1) {
        switch (s->mode) {
        case BLOCKS_TYPE:
            NEEDBITS(3)
            t = (u32)b & 7;
            s->last = t & 1;
            switch (t >> 1) {
            case 0: /* stored */
                DUMPBITS(3)
                t = k & 7; /* go to a byte boundary */
                DUMPBITS(t)
                s->mode = BLOCKS_LENS;
                break;
            case 1: { /* fixed */
                u32 bl;
                u32 bd;
                InflateHuft *tl;
                InflateHuft *td;

                inflate_trees_fixed(&bl, &bd, &tl, &td, z);
                s->sub.decode.codes = inflate_codes_new(bl, bd, tl, td, z);
                if (s->sub.decode.codes == NULL) {
                    r = Z_MEM_ERROR;
                    LEAVE
                }
                DUMPBITS(3)
                s->mode = BLOCKS_CODES;
                break;
            }
            case 2: /* dynamic */
                DUMPBITS(3)
                s->mode = BLOCKS_TABLE;
                break;
            case 3: /* illegal */
                DUMPBITS(3)
                s->mode = BLOCKS_BAD;
                z->msg = "invalid block type";
                r = Z_DATA_ERROR;
                LEAVE
            }
            break;
        case BLOCKS_LENS:
            NEEDBITS(32)
            /* the length, then its complement */
            if ((((~b) >> 16) & 0xFFFF) != (b & 0xFFFF)) {
                s->mode = BLOCKS_BAD;
                z->msg = "invalid stored block lengths";
                r = Z_DATA_ERROR;
                LEAVE
            }
            s->sub.left = (u32)b & 0xFFFF;
            b = k = 0; /* dump the bits */
            s->mode = s->sub.left ? BLOCKS_STORED : (s->last ? BLOCKS_DRY : BLOCKS_TYPE);
            break;
        case BLOCKS_STORED:
            if (n == 0) {
                LEAVE
            }
            NEEDOUT
            t = s->sub.left;
            if (t > n) {
                t = n;
            }
            if (t > m) {
                t = m;
            }
            memcpy(q, p, t);
            p += t;
            n -= t;
            q += t;
            m -= t;
            if ((s->sub.left -= t) != 0) {
                break;
            }
            s->mode = s->last ? BLOCKS_DRY : BLOCKS_TYPE;
            break;
        case BLOCKS_TABLE:
            /* HLIT - 257 (5 bits), HDIST - 1 (5 bits) and HCLEN - 4 (4 bits) */
            NEEDBITS(14)
            s->sub.trees.table = t = (u32)b & 0x3FFF;
            if ((t & 0x1F) > 29 || ((t >> 5) & 0x1F) > 29) {
                s->mode = BLOCKS_BAD;
                z->msg = "too many length or distance symbols";
                r = Z_DATA_ERROR;
                LEAVE
            }
            t = 258 + (t & 0x1F) + ((t >> 5) & 0x1F);
            if ((s->sub.trees.blens = ZALLOC(z, t, sizeof(u32))) == NULL) {
                r = Z_MEM_ERROR;
                LEAVE
            }
            DUMPBITS(14)
            s->sub.trees.index = 0;
            s->mode = BLOCKS_BTREE;
        case BLOCKS_BTREE:
            /* the bit lengths of the code lengths, in border's order */
            while (s->sub.trees.index < 4 + (s->sub.trees.table >> 10)) {
                NEEDBITS(3)
                s->sub.trees.blens[border[s->sub.trees.index++]] = (u32)b & 7;
                DUMPBITS(3)
            }
            while (s->sub.trees.index < 19) {
                s->sub.trees.blens[border[s->sub.trees.index++]] = 0;
            }
            s->sub.trees.bb = 7;
            t = inflate_trees_bits(s->sub.trees.blens, &s->sub.trees.bb, &s->sub.trees.tb, s->hufts, z);
            if (t != Z_OK) {
                ZFREE(z, s->sub.trees.blens);
                r = t;
                if (r == Z_DATA_ERROR) {
                    s->mode = BLOCKS_BAD;
                }
                LEAVE
            }
            s->sub.trees.index = 0;
            s->mode = BLOCKS_DTREE;
        case BLOCKS_DTREE:
            while (t = s->sub.trees.table,
                   s->sub.trees.index < 258 + (t & 0x1F) + ((t >> 5) & 0x1F)) {
                InflateHuft *h;
                u32 i;
                u32 j;
                u32 c;

                t = s->sub.trees.bb;
                NEEDBITS(t)
                h = s->sub.trees.tb + ((u32)b & inflate_mask[t]);
                t = h->bits;
                c = h->base;
                if (c < 16) {
                    DUMPBITS(t)
                    s->sub.trees.blens[s->sub.trees.index++] = c;
                } else { /* 16..18: repeat the last length or a zero */
                    i = c == 18 ? 7 : c - 14;
                    j = c == 18 ? 11 : 3;
                    NEEDBITS(t + i)
                    DUMPBITS(t)
                    j += (u32)b & inflate_mask[i];
                    DUMPBITS(i)
                    i = s->sub.trees.index;
                    t = s->sub.trees.table;
                    if (i + j > 258 + (t & 0x1F) + ((t >> 5) & 0x1F) || (c == 16 && i < 1)) {
                        ZFREE(z, s->sub.trees.blens);
                        s->mode = BLOCKS_BAD;
                        z->msg = "invalid bit length repeat";
                        r = Z_DATA_ERROR;
                        LEAVE
                    }
                    c = c == 16 ? s->sub.trees.blens[i - 1] : 0;
                    do {
                        s->sub.trees.blens[i++] = c;
                    } while (--j);
                    s->sub.trees.index = i;
                }
            }
            s->sub.trees.tb = NULL;
            {
                u32 bl;
                u32 bd;
                InflateHuft *tl;
                InflateHuft *td;
                InflateCodesState *c;

                /* at most 9 bits, which inflate_fast's lookahead needs */
                bl = 9;
                bd = 6;
                t = s->sub.trees.table;
                t = inflate_trees_dynamic(257 + (t & 0x1F), 1 + ((t >> 5) & 0x1F), s->sub.trees.blens,
                                  &bl, &bd, &tl, &td, s->hufts, z);
                ZFREE(z, s->sub.trees.blens);
                if (t != Z_OK) {
                    if (t == (u32)Z_DATA_ERROR) {
                        s->mode = BLOCKS_BAD;
                    }
                    r = t;
                    LEAVE
                }
                if ((c = inflate_codes_new(bl, bd, tl, td, z)) == NULL) {
                    r = Z_MEM_ERROR;
                    LEAVE
                }
                s->sub.decode.codes = c;
            }
            s->mode = BLOCKS_CODES;
        case BLOCKS_CODES:
            UPDATE
            if ((r = inflate_codes(s, z, r)) != Z_STREAM_END) {
                return inflate_flush(s, z, r);
            }
            r = Z_OK;
            inflate_codes_free(s->sub.decode.codes, z);
            LOAD
            if (!s->last) {
                s->mode = BLOCKS_TYPE;
                break;
            }
            s->mode = BLOCKS_DRY;
        case BLOCKS_DRY:
            FLUSH
            if (s->read != s->write) {
                LEAVE
            }
            s->mode = BLOCKS_DONE;
        case BLOCKS_DONE:
            r = Z_STREAM_END;
            LEAVE
        case BLOCKS_BAD:
            r = Z_DATA_ERROR;
            LEAVE
        default:
            r = Z_STREAM_ERROR;
            LEAVE
        }
    }
}

/* inflate_blocks_reset: starts over at a block boundary, handing out the check
 * of the output so far in *c. */
void inflate_blocks_reset(InflateBlocksState *s, ZStream *z, u32 *c) {
    inflate_blocks_reset_inline(s, z, c);
}

/* inflate_blocks_free: frees the state of the decoding of the blocks. */
s32 inflate_blocks_free(InflateBlocksState *s, ZStream *z) {
    inflate_blocks_reset_inline(s, z, NULL);
    ZFREE(z, s->window);
    ZFREE(z, s->hufts);
    ZFREE(z, s);
    return Z_OK;
}

/* inflate_set_dictionary: puts a preset dictionary in the window. */
void inflate_set_dictionary(InflateBlocksState *s, const u8 *d, u32 n) {
    memcpy(s->window, d, n);
    s->read = s->write = s->window + n;
}

/* inflate_blocks_sync_point: whether the blocks stand where a full flush ended
 * them, waiting for the lengths of a stored block. */
s32 inflate_blocks_sync_point(InflateBlocksState *s) {
    return s->mode == BLOCKS_LENS;
}

/* inflate_codes: decodes the literals, lengths and distances of a block, as
 * far as the input and the room in the window allow. */
s32 inflate_codes(InflateBlocksState *s, ZStream *z, s32 r) {
    u32 j;
    InflateHuft *t;
    u32 e;
    u32 b;
    u32 k;
    u8 *p;
    u32 n;
    u8 *q;
    u32 m;
    u8 *f;
    InflateCodesState *c = s->sub.decode.codes;

    LOAD
    while (1) {
        switch (c->mode) {
        case CODES_START:
            /* the fast decoder needs room for a longest match and enough input */
            if (m >= 258 && n >= 10) {
                UPDATE
                r = inflate_fast(c->lbits, c->dbits, c->ltree, c->dtree, s, z);
                LOAD
                if (r != Z_OK) {
                    c->mode = r == Z_STREAM_END ? CODES_WASH : CODES_BADCODE;
                    break;
                }
            }
            c->sub.code.need = c->lbits;
            c->sub.code.tree = c->ltree;
            c->mode = CODES_LEN;
        case CODES_LEN:
            j = c->sub.code.need;
            NEEDBITS(j)
            t = c->sub.code.tree + ((u32)b & inflate_mask[j]);
            DUMPBITS(t->bits)
            e = (u32)(t->exop);
            if (e == 0) { /* literal */
                c->sub.lit = t->base;
                c->mode = CODES_LIT;
                break;
            }
            if (e & HUFT_EXTRA) { /* length */
                c->sub.copy.get = e & HUFT_EXTRA_BITS;
                c->len = t->base;
                c->mode = CODES_LENEXT;
                break;
            }
            if ((e & HUFT_LEAF) == 0) { /* next table */
                c->sub.code.need = e;
                c->sub.code.tree = t + t->base;
                break;
            }
            if (e & HUFT_END) { /* end of block */
                c->mode = CODES_WASH;
                break;
            }
            c->mode = CODES_BADCODE;
            z->msg = "invalid literal/length code";
            r = Z_DATA_ERROR;
            LEAVE
        case CODES_LENEXT:
            j = c->sub.copy.get;
            NEEDBITS(j)
            c->len += (u32)b & inflate_mask[j];
            DUMPBITS(j)
            c->sub.code.need = c->dbits;
            c->sub.code.tree = c->dtree;
            c->mode = CODES_DIST;
        case CODES_DIST:
            j = c->sub.code.need;
            NEEDBITS(j)
            t = c->sub.code.tree + ((u32)b & inflate_mask[j]);
            DUMPBITS(t->bits)
            e = (u32)(t->exop);
            if (e & HUFT_EXTRA) { /* distance */
                c->sub.copy.get = e & HUFT_EXTRA_BITS;
                c->sub.copy.dist = t->base;
                c->mode = CODES_DISTEXT;
                break;
            }
            if ((e & HUFT_LEAF) == 0) { /* next table */
                c->sub.code.need = e;
                c->sub.code.tree = t + t->base;
                break;
            }
            c->mode = CODES_BADCODE;
            z->msg = "invalid distance code";
            r = Z_DATA_ERROR;
            LEAVE
        case CODES_DISTEXT:
            j = c->sub.copy.get;
            NEEDBITS(j)
            c->sub.copy.dist += (u32)b & inflate_mask[j];
            DUMPBITS(j)
            c->mode = CODES_COPY;
        case CODES_COPY:
            /* the window wraps around */
            f = (u32)(q - s->window) < c->sub.copy.dist
                    ? s->end - (c->sub.copy.dist - (q - s->window))
                    : q - c->sub.copy.dist;
            while (c->len) {
                NEEDOUT
                OUTBYTE(*f++)
                if (f == s->end) {
                    f = s->window;
                }
                c->len--;
            }
            c->mode = CODES_START;
            break;
        case CODES_LIT:
            NEEDOUT
            OUTBYTE(c->sub.lit)
            c->mode = CODES_START;
            break;
        case CODES_WASH:
            if (k > 7) { /* give back the unused byte, if any */
                k -= 8;
                n++;
                p--;
            }
            FLUSH
            if (s->read != s->write) {
                LEAVE
            }
            c->mode = CODES_END;
        case CODES_END:
            r = Z_STREAM_END;
            LEAVE
        case CODES_BADCODE:
            r = Z_DATA_ERROR;
            LEAVE
        default:
            r = Z_STREAM_ERROR;
            LEAVE
        }
    }
}

/* inflate_codes_new: allocates the state of the decoding of a block's codes. */
InflateCodesState *inflate_codes_new(u32 bl, u32 bd, InflateHuft *tl, InflateHuft *td, ZStream *z) {
    InflateCodesState *c = ZALLOC(z, 1, sizeof(InflateCodesState));

    if (c != NULL) {
        c->mode = CODES_START;
        c->lbits = bl;
        c->dbits = bd;
        c->ltree = tl;
        c->dtree = td;
    }
    return c;
}

/* inflate_codes_free: frees the state of the decoding of a block's codes. */
void inflate_codes_free(InflateCodesState *c, ZStream *z) {
    ZFREE(z, c);
}

/*
 * inflate_fast's messages, the same as inflate_codes' but its own copies: the
 * two were separate files, and in one file the compiler would share them.
 */
const char STR_INVALID_DISTANCE_CODE[] = "invalid distance code";
const char STR_INVALID_LITERAL_LENGTH_CODE[] = "invalid literal/length code";

/* inflate_fast's bit input, which needs no check, and its giving back of the
 * whole bytes left in the bit buffer */
#define GRABBITS(j)                         \
    {                                       \
        while (k < (j)) {                   \
            b |= ((u32)NEXTBYTE) << k;      \
            k += 8;                         \
        }                                   \
    }
#define UNGRAB                              \
    {                                       \
        c = z->availIn - n;                 \
        c = (k >> 3) < c ? k >> 3 : c;      \
        n += c;                             \
        p -= c;                             \
        k -= c << 3;                        \
    }

/* inflate_fast: decodes literals and matches without checking the input and
 * the room in the window, while there are enough of both for a longest code. */
s32 inflate_fast(u32 bl, u32 bd, InflateHuft *tl, InflateHuft *td, InflateBlocksState *s,
                  ZStream *z) {
    InflateHuft *t;
    u32 e; /* extra bits or operation */
    u32 b;
    u32 k;
    u8 *p;
    u32 n;
    u8 *q;
    u32 m;
    u32 ml; /* mask for the literal/length tree */
    u32 md; /* mask for the distance tree */
    u32 c; /* bytes to copy */
    u32 d; /* distance back to copy from */
    u8 *r; /* copy source */

    LOAD
    ml = inflate_mask[bl];
    md = inflate_mask[bd];
    do {
        GRABBITS(20) /* the longest literal/length code */
        if ((e = (t = tl + ((u32)b & ml))->exop) == 0) {
            DUMPBITS(t->bits)
            *q++ = (u8)t->base;
            m--;
            continue;
        }
        do {
            DUMPBITS(t->bits)
            if (e & HUFT_EXTRA) {
                e &= HUFT_EXTRA_BITS;
                c = t->base + ((u32)b & inflate_mask[e]);
                DUMPBITS(e)
                GRABBITS(15) /* the longest distance code */
                e = (t = td + ((u32)b & md))->exop;
                do {
                    DUMPBITS(t->bits)
                    if (e & HUFT_EXTRA) {
                        e &= HUFT_EXTRA_BITS;
                        GRABBITS(e)
                        d = t->base + ((u32)b & inflate_mask[e]);
                        DUMPBITS(e)
                        m -= c;
                        if ((u32)(q - s->window) >= d) {
                            /* a match is at least 3 bytes long */
                            r = q - d;
                            *q++ = *r++;
                            c--;
                            *q++ = *r++;
                            c--;
                        } else { /* the source wraps around the window */
                            e = d - (u32)(q - s->window);
                            r = s->end - e;
                            if (c > e) {
                                c -= e;
                                do {
                                    *q++ = *r++;
                                } while (--e);
                                r = s->window;
                            }
                        }
                        do {
                            *q++ = *r++;
                        } while (--c);
                        break;
                    } else if ((e & HUFT_LEAF) == 0) {
                        t += t->base;
                        e = (t += ((u32)b & inflate_mask[e]))->exop;
                    } else {
                        z->msg = STR_INVALID_DISTANCE_CODE;
                        UNGRAB
                        UPDATE
                        return Z_DATA_ERROR;
                    }
                } while (1);
                break;
            }
            if ((e & HUFT_LEAF) == 0) {
                t += t->base;
                if ((e = (t += ((u32)b & inflate_mask[e]))->exop) == 0) {
                    DUMPBITS(t->bits)
                    *q++ = (u8)t->base;
                    m--;
                    break;
                }
            } else if (e & HUFT_END) {
                UNGRAB
                UPDATE
                return Z_STREAM_END;
            } else {
                z->msg = STR_INVALID_LITERAL_LENGTH_CODE;
                UNGRAB
                UPDATE
                return Z_DATA_ERROR;
            }
        } while (1);
    } while (m >= 258 && n >= 10);
    UNGRAB
    UPDATE
    return Z_OK;
}

/* inftrees.c's copyright notice, which zlib keeps in the binary */
const char inflate_copyright[] = " inflate 1.1.3 Copyright 1995-1998 Mark Adler ";

/* the longest code, in bits */
#define BMAX 15
/* huft_build's clearing of its count table, unrolled (BMAX + 1 is 16) */
#define CLEAR1 *p++ = 0;
#define CLEAR4 CLEAR1 CLEAR1 CLEAR1 CLEAR1
#define CLEAR16 CLEAR4 CLEAR4 CLEAR4 CLEAR4

/* huft_build: builds the Huffman decoding tables of the n code lengths in b,
 * in hp. The codes below s stand for themselves (256 ends a block); the others
 * take their bases from d and their extra bits from e. *m is the bits of the
 * first table, which it hands out with the table in *t. */
s32 huft_build(u32 *b, u32 n, u32 s, const u32 *d, const u32 *e, InflateHuft **t, u32 *m,
                  InflateHuft *hp, u32 *hn, u32 *v) {
    u32 a; /* codes of length k left */
    u32 c[BMAX + 1]; /* codes of each bit length */
    u32 f; /* i repeats in the table every f entries */
    s32 g; /* longest code */
    s32 h; /* table level */
    u32 i; /* current code */
    u32 j;
    s32 k; /* bits of the current code */
    s32 l; /* bits per table (handed out in m) */
    u32 mask; /* (1 << w) - 1 */
    u32 *p; /* pointer into c, b or v */
    InflateHuft *q; /* current table */
    InflateHuft r; /* the table entry being made */
    InflateHuft *u[BMAX]; /* table stack */
    s32 w; /* bits before this table == (l * h) */
    u32 x[BMAX + 1]; /* bit offsets, then code stack */
    u32 *xp; /* pointer into x */
    s32 y; /* dummy codes added */
    u32 z; /* entries in the current table */

    /* count the codes of each bit length */
    p = c;
    CLEAR16
    p = b;
    i = n;
    do {
        c[*p++]++;
    } while (--i);
    if (c[0] == n) { /* no codes at all */
        *t = NULL;
        *m = 0;
        return Z_OK;
    }

    /* bound *m by the shortest and longest codes */
    l = *m;
    for (j = 1; j <= BMAX; j++) {
        if (c[j]) {
            break;
        }
    }
    k = j;
    if ((u32)l < j) {
        l = j;
    }
    for (i = BMAX; i; i--) {
        if (c[i]) {
            break;
        }
    }
    g = i;
    if ((u32)l > i) {
        l = i;
    }
    *m = l;

    /* fill the last length's count out to complete codes, if needed */
    for (y = 1 << j; j < i; j++, y <<= 1) {
        if ((y -= c[j]) < 0) {
            return Z_DATA_ERROR;
        }
    }
    if ((y -= c[i]) < 0) {
        return Z_DATA_ERROR;
    }
    c[i] += y;

    /* the offsets of each length in the value table */
    x[1] = j = 0;
    p = c + 1;
    xp = x + 2;
    while (--i) {
        *xp++ = (j += *p++);
    }

    /* the values in order of bit length */
    p = b;
    i = 0;
    do {
        if ((j = *p++) != 0) {
            v[x[j]++] = i;
        }
    } while (++i < n);
    n = x[g];

    /* make the codes and their table entries */
    x[0] = i = 0;
    p = v;
    h = -1;
    w = -l;
    u[0] = NULL;
    q = NULL;
    z = 0;
    for (; k <= g; k++) {
        a = c[k];
        while (a--) {
            /* make the tables up to the level i needs */
            while (k > w + l) {
                h++;
                w += l;

                /* the smallest table up to l bits that fits */
                z = g - w;
                z = z > (u32)l ? l : z;
                if ((f = 1 << (j = k - w)) > a + 1) {
                    f -= a + 1;
                    xp = c + k;
                    if (j < z) {
                        while (++j < z && (f <<= 1) > *++xp) {
                            f -= *xp;
                        }
                    }
                }
                z = 1 << j;

                if (*hn + z > MANY) {
                    return Z_MEM_ERROR;
                }
                u[h] = q = hp + *hn;
                *hn += z;

                /* link it from the table before, if any */
                if (h) {
                    x[h] = i;
                    r.bits = (u8)l;
                    r.exop = (u8)j;
                    j = i >> (w - l);
                    r.base = (u32)(q - u[h - 1] - j);
                    u[h - 1][j] = r;
                } else {
                    *t = q;
                }
            }

            r.bits = (u8)(k - w);
            if (p >= v + n) {
                r.exop = HUFT_INVALID | HUFT_LEAF;
            } else if (*p < s) {
                r.exop = (u8)(*p < 256 ? 0 : HUFT_END | HUFT_LEAF);
                r.base = *p++;
            } else {
                r.exop = (u8)(e[*p - s] + HUFT_EXTRA + HUFT_LEAF);
                r.base = d[*p++ - s];
            }

            /* fill every entry the code stands for */
            f = 1 << (k - w);
            for (j = i >> w; j < z; j += f) {
                q[j] = r;
            }

            /* the next code, incrementing i bit-reversed */
            for (j = 1 << (k - 1); i & j; j >>= 1) {
                i ^= j;
            }
            i ^= j;

            /* back up over the finished tables */
            mask = (1 << w) - 1;
            while ((i & mask) != x[h]) {
                h--;
                w -= l;
                mask = (1 << w) - 1;
            }
        }
    }

    /* an incomplete set of codes is only fine for a single code */
    return y != 0 && g != 1 ? Z_BUF_ERROR : Z_OK;
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", cplens);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", cplext);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", cpdist);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", cpdext);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", STR_OVERSUBSCRIBED_BIT_LENGTHS_TREE);

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/inflate", STR_INCOMPLETE_BIT_LENGTHS_TREE);

/* inflate_trees_dynamic: builds the literal/length and distance decoding
 * tables of a dynamic block from the nl + nd code lengths in c. */
s32 inflate_trees_dynamic(u32 nl, u32 nd, u32 *c, u32 *bl, u32 *bd, InflateHuft **tl, InflateHuft **td,
                  InflateHuft *hp, ZStream *z) {
    s32 r;
    u32 hn = 0; /* hufts used in hp */
    u32 *v; /* work area for huft_build */

    if ((v = ZALLOC(z, 288, sizeof(u32))) == NULL) {
        return Z_MEM_ERROR;
    }
    r = huft_build(c, nl, 257, cplens, cplext, tl, bl, hp, &hn, v);
    if (r != Z_OK || *bl == 0) {
        if (r == Z_DATA_ERROR) {
            z->msg = "oversubscribed literal/length tree";
        } else if (r != Z_MEM_ERROR) {
            z->msg = "incomplete literal/length tree";
            r = Z_DATA_ERROR;
        }
        ZFREE(z, v);
        return r;
    }
    r = huft_build(c + nl, nd, 0, cpdist, cpdext, td, bd, hp, &hn, v);
    if (r != Z_OK || (*bd == 0 && nl > 257)) {
        if (r == Z_DATA_ERROR) {
            z->msg = "oversubscribed distance tree";
        } else if (r == Z_BUF_ERROR) {
            z->msg = "incomplete distance tree";
            r = Z_DATA_ERROR;
        } else if (r != Z_MEM_ERROR) {
            z->msg = "empty distance tree with lengths";
            r = Z_DATA_ERROR;
        }
        ZFREE(z, v);
        return r;
    }
    ZFREE(z, v);
    return Z_OK;
}

/* inflate_trees_bits: builds the decoding table of the code lengths of a
 * dynamic block from their 19 3-bit lengths. */
s32 inflate_trees_bits(u32 *c, u32 *bb, InflateHuft **tb, InflateHuft *hp, ZStream *z) {
    s32 r;
    u32 hn = 0; /* hufts used in hp */
    u32 *v = ZALLOC(z, 19, sizeof(u32)); /* work area for huft_build */

    if (v == NULL) {
        return Z_MEM_ERROR;
    }
    r = huft_build(c, 19, 19, NULL, NULL, tb, bb, hp, &hn, v);
    if (r == Z_DATA_ERROR) {
        z->msg = STR_OVERSUBSCRIBED_BIT_LENGTHS_TREE;
    } else if (r == Z_BUF_ERROR || *bb == 0) {
        z->msg = STR_INCOMPLETE_BIT_LENGTHS_TREE;
        r = Z_DATA_ERROR;
    }
    ZFREE(z, v);
    return r;
}

/* the bits of the first lookup of the fixed literal and distance tables */
static u32 fixed_bl = 9;
static u32 fixed_bd = 5;

/* inflate_trees_fixed: hands out the fixed Huffman tables of deflate. */
s32 inflate_trees_fixed(u32 *bl, u32 *bd, InflateHuft **tl, InflateHuft **td, ZStream *z) {
    *bl = fixed_bl;
    *bd = fixed_bd;
    *tl = fixed_tl;
    *td = fixed_td;
    return Z_OK;
}

/* inflate_flush: copies as much as possible from the window to the output. */
s32 inflate_flush(InflateBlocksState *s, ZStream *z, s32 r) {
    u32 n;
    u8 *p;
    u8 *q;

    p = z->nextOut;
    q = s->read;

    /* as far as the end of the window */
    n = (q <= s->write ? s->write : s->end) - q;
    if (n > z->availOut) {
        n = z->availOut;
    }
    if (n != 0 && r == Z_BUF_ERROR) {
        r = Z_OK;
    }
    z->availOut -= n;
    z->totalOut += n;
    if (s->checkfn != NULL) {
        z->adler = s->check = s->checkfn(s->check, q, n);
    }
    memcpy(p, q, n);
    p += n;
    q += n;

    /* then from the start of the window, when it wrapped */
    if (q == s->end) {
        q = s->window;
        if (s->write == s->end) {
            s->write = s->window;
        }
        n = s->write - q;
        if (n > z->availOut) {
            n = z->availOut;
        }
        if (n != 0 && r == Z_BUF_ERROR) {
            r = Z_OK;
        }
        z->availOut -= n;
        z->totalOut += n;
        if (s->checkfn != NULL) {
            z->adler = s->check = s->checkfn(s->check, q, n);
        }
        memcpy(p, q, n);
        p += n;
        q += n;
    }
    z->nextOut = p;
    s->read = q;
    return r;
}

/* the message of the error codes without one (z_errmsg) */
const char STR_EMPTY[] = "";

/* zlibVersion: the version of zlib. */
const char *zlibVersion(void) {
    return ZLIB_VERSION;
}

/* zError: the message of an error code. */
const char *zError(s32 err) {
    return z_errmsg[Z_NEED_DICT - err];
}

/* zcalloc: allocates memory for the stream with calloc(). */
void *zcalloc(void *opaque, s32 items, s32 size) {
    /* calloc and free were linked to the same break stub, which takes nothing */
    return ((void *(*)(s32, s32))breakForever)(items, size);
}

/* zcfree: frees memory for the stream with free(). */
void zcfree(void *opaque, void *ptr) {
    /* calloc and free were linked to the same break stub, which takes nothing */
    ((void (*)(void *))breakForever)(ptr);
}

/*
 * The messages of zError (zutil.c's z_errmsg), from Z_VERSION_ERROR up. Their
 * table, z_errmsg, is still in the .data assembly, which only knows their
 * addresses.
 */
const char STR_INCOMPATIBLE_VERSION[] = "incompatible version";
const char STR_BUFFER_ERROR[] = "buffer error";
const char STR_INSUFFICIENT_MEMORY[] = "insufficient memory";
const char STR_DATA_ERROR[] = "data error";
const char STR_STREAM_ERROR[] = "stream error";
const char STR_FILE_ERROR[] = "file error";
const char STR_STREAM_END[] = "stream end";
const char STR_NEED_DICTIONARY[] = "need dictionary";
