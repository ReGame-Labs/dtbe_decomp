#ifndef DTBE_LIB_MD5_H
#define DTBE_LIB_MD5_H

/* RSA's MD5 message digest (RFC 1321): its context and padding. */

#include "common.h"

EXTERN_C_BEGIN

/* MD5 message digest context (RFC 1321) */
typedef struct {
    /* 0x00 */ u32 state[4];
    /* 0x10 */ u32 count[2]; /* number of bits, modulo 2^64, low word first */
    /* 0x18 */ u8 buffer[64];
} Md5;

/* the size of an MD5 digest */
#define MD5_DIGEST_SIZE 16

extern u8 PADDING[64]; /* the MD5 padding: 0x80, then zeros */

void MD5Final(Md5 *md5, u8 *digest);
void MD5Transform(u32 *state, u8 *block);
Md5 *md5Init(Md5 *md5);
void MD5Init(Md5 *md5);
void MD5Update(Md5 *md5, u8 *input, u32 len);
void Encode(u8 *output, u32 *input, u32 len);
void Decode(u32 *output, u8 *input, u32 len);
void MD5_memcpy(u8 *output, u8 *input, u32 len);
void MD5_memset(u8 *output, s32 value, u32 len);

EXTERN_C_END

#endif /* DTBE_LIB_MD5_H */
