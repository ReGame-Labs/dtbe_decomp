#include "common.h"
#include "engine/lib/md5.h"

/* Continues an MD5 operation with len more bytes of the message; Final
 * has it inline. */
static inline void MD5UpdateInline(Md5 *this, u8 *input, u32 len) {
    u32 i;
    u32 index;
    u32 partLen;

    index = (this->count[0] >> 3) & 0x3F;
    if ((this->count[0] += len << 3) < (len << 3)) {
        this->count[1]++;
    }
    this->count[1] += len >> 29;
    partLen = 64 - index;
    if (len >= partLen) {
        MD5_memcpy(&this->buffer[index], input, partLen);
        MD5Transform(this->state, this->buffer);
        for (i = partLen; i + 63 < len; i += 64) {
            MD5Transform(this->state, &input[i]);
        }
        index = 0;
    } else {
        i = 0;
    }
    MD5_memcpy(&this->buffer[index], &input[i], len - i);
}

/* Ends an MD5 operation: pads the message, appends its length and writes
 * the 16-byte digest, then clears the context. */
void MD5Final(Md5 *this, u8 *digest) {
    u8 bits[8];
    u32 index;
    u32 padLen;

    Encode(bits, this->count, 8);
    index = (this->count[0] >> 3) & 0x3F;
    padLen = (index < 56) ? (56 - index) : (120 - index);
    MD5UpdateInline(this, PADDING, padLen);
    MD5UpdateInline(this, bits, 8);
    Encode(digest, this->state, 16);
    MD5_memset((u8 *)this, 0, sizeof(Md5)); /* clears the whole context */
}

/* RFC 1321's auxiliary functions, rotation and round steps */
#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))
#define MD5_ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define MD5_FF(a, b, c, d, x, s, ac) {                                          \
    (a) += MD5_F((b), (c), (d)) + (x) + (u32)(ac);                              \
    (a) = MD5_ROTATE_LEFT((a), (s));                                            \
    (a) += (b);                                                                 \
}
#define MD5_GG(a, b, c, d, x, s, ac) {                                          \
    (a) += MD5_G((b), (c), (d)) + (x) + (u32)(ac);                              \
    (a) = MD5_ROTATE_LEFT((a), (s));                                            \
    (a) += (b);                                                                 \
}
#define MD5_HH(a, b, c, d, x, s, ac) {                                          \
    (a) += MD5_H((b), (c), (d)) + (x) + (u32)(ac);                              \
    (a) = MD5_ROTATE_LEFT((a), (s));                                            \
    (a) += (b);                                                                 \
}
#define MD5_II(a, b, c, d, x, s, ac) {                                          \
    (a) += MD5_I((b), (c), (d)) + (x) + (u32)(ac);                              \
    (a) = MD5_ROTATE_LEFT((a), (s));                                            \
    (a) += (b);                                                                 \
}

/* The MD5 basic transformation: updates state with a 64-byte block. */
void MD5Transform(u32 *state, u8 *block) {
    u32 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    Decode(x, block, 64);

    /* Round 1 */
    MD5_FF(a, b, c, d, x[0], 7, 0xD76AA478);
    MD5_FF(d, a, b, c, x[1], 12, 0xE8C7B756);
    MD5_FF(c, d, a, b, x[2], 17, 0x242070DB);
    MD5_FF(b, c, d, a, x[3], 22, 0xC1BDCEEE);
    MD5_FF(a, b, c, d, x[4], 7, 0xF57C0FAF);
    MD5_FF(d, a, b, c, x[5], 12, 0x4787C62A);
    MD5_FF(c, d, a, b, x[6], 17, 0xA8304613);
    MD5_FF(b, c, d, a, x[7], 22, 0xFD469501);
    MD5_FF(a, b, c, d, x[8], 7, 0x698098D8);
    MD5_FF(d, a, b, c, x[9], 12, 0x8B44F7AF);
    MD5_FF(c, d, a, b, x[10], 17, 0xFFFF5BB1);
    MD5_FF(b, c, d, a, x[11], 22, 0x895CD7BE);
    MD5_FF(a, b, c, d, x[12], 7, 0x6B901122);
    MD5_FF(d, a, b, c, x[13], 12, 0xFD987193);
    MD5_FF(c, d, a, b, x[14], 17, 0xA679438E);
    MD5_FF(b, c, d, a, x[15], 22, 0x49B40821);

    /* Round 2 */
    MD5_GG(a, b, c, d, x[1], 5, 0xF61E2562);
    MD5_GG(d, a, b, c, x[6], 9, 0xC040B340);
    MD5_GG(c, d, a, b, x[11], 14, 0x265E5A51);
    MD5_GG(b, c, d, a, x[0], 20, 0xE9B6C7AA);
    MD5_GG(a, b, c, d, x[5], 5, 0xD62F105D);
    MD5_GG(d, a, b, c, x[10], 9, 0x02441453);
    MD5_GG(c, d, a, b, x[15], 14, 0xD8A1E681);
    MD5_GG(b, c, d, a, x[4], 20, 0xE7D3FBC8);
    MD5_GG(a, b, c, d, x[9], 5, 0x21E1CDE6);
    MD5_GG(d, a, b, c, x[14], 9, 0xC33707D6);
    MD5_GG(c, d, a, b, x[3], 14, 0xF4D50D87);
    MD5_GG(b, c, d, a, x[8], 20, 0x455A14ED);
    MD5_GG(a, b, c, d, x[13], 5, 0xA9E3E905);
    MD5_GG(d, a, b, c, x[2], 9, 0xFCEFA3F8);
    MD5_GG(c, d, a, b, x[7], 14, 0x676F02D9);
    MD5_GG(b, c, d, a, x[12], 20, 0x8D2A4C8A);

    /* Round 3 */
    MD5_HH(a, b, c, d, x[5], 4, 0xFFFA3942);
    MD5_HH(d, a, b, c, x[8], 11, 0x8771F681);
    MD5_HH(c, d, a, b, x[11], 16, 0x6D9D6122);
    MD5_HH(b, c, d, a, x[14], 23, 0xFDE5380C);
    MD5_HH(a, b, c, d, x[1], 4, 0xA4BEEA44);
    MD5_HH(d, a, b, c, x[4], 11, 0x4BDECFA9);
    MD5_HH(c, d, a, b, x[7], 16, 0xF6BB4B60);
    MD5_HH(b, c, d, a, x[10], 23, 0xBEBFBC70);
    MD5_HH(a, b, c, d, x[13], 4, 0x289B7EC6);
    MD5_HH(d, a, b, c, x[0], 11, 0xEAA127FA);
    MD5_HH(c, d, a, b, x[3], 16, 0xD4EF3085);
    MD5_HH(b, c, d, a, x[6], 23, 0x04881D05);
    MD5_HH(a, b, c, d, x[9], 4, 0xD9D4D039);
    MD5_HH(d, a, b, c, x[12], 11, 0xE6DB99E5);
    MD5_HH(c, d, a, b, x[15], 16, 0x1FA27CF8);
    MD5_HH(b, c, d, a, x[2], 23, 0xC4AC5665);

    /* Round 4 */
    MD5_II(a, b, c, d, x[0], 6, 0xF4292244);
    MD5_II(d, a, b, c, x[7], 10, 0x432AFF97);
    MD5_II(c, d, a, b, x[14], 15, 0xAB9423A7);
    MD5_II(b, c, d, a, x[5], 21, 0xFC93A039);
    MD5_II(a, b, c, d, x[12], 6, 0x655B59C3);
    MD5_II(d, a, b, c, x[3], 10, 0x8F0CCC92);
    MD5_II(c, d, a, b, x[10], 15, 0xFFEFF47D);
    MD5_II(b, c, d, a, x[1], 21, 0x85845DD1);
    MD5_II(a, b, c, d, x[8], 6, 0x6FA87E4F);
    MD5_II(d, a, b, c, x[15], 10, 0xFE2CE6E0);
    MD5_II(c, d, a, b, x[6], 15, 0xA3014314);
    MD5_II(b, c, d, a, x[13], 21, 0x4E0811A1);
    MD5_II(a, b, c, d, x[4], 6, 0xF7537E82);
    MD5_II(d, a, b, c, x[11], 10, 0xBD3AF235);
    MD5_II(c, d, a, b, x[2], 15, 0x2AD7D2BB);
    MD5_II(b, c, d, a, x[9], 21, 0xEB86D391);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zeroize sensitive information. */
    MD5_memset((u8 *)x, 0, sizeof(x));
}

/* The game's constructor of an MD5 context, around RSA's MD5Init: readies it
 * for a new message and returns it. */
Md5 *md5Init(Md5 *this) {
    MD5Init(this);
    return this;
}

/* Begins an MD5 operation. */
void MD5Init(Md5 *this) {
    this->count[0] = this->count[1] = 0;
    this->state[0] = 0x67452301;
    this->state[1] = 0xEFCDAB89;
    this->state[2] = 0x98BADCFE;
    this->state[3] = 0x10325476;
}

/* Continues an MD5 operation with len more bytes of the message. */
void MD5Update(Md5 *this, u8 *input, u32 len) {
    MD5UpdateInline(this, input, len);
}

/* Writes words as little-endian bytes; len counts the bytes. */
void Encode(u8 *output, u32 *input, u32 len) {
    u32 i;
    u32 j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = input[i] & 0xFF;
        output[j + 1] = (input[i] >> 8) & 0xFF;
        output[j + 2] = (input[i] >> 16) & 0xFF;
        output[j + 3] = (input[i] >> 24) & 0xFF;
    }
}

/* Reads little-endian bytes into words; len counts the bytes. */
void Decode(u32 *output, u8 *input, u32 len) {
    u32 i;
    u32 j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[i] = input[j] | (input[j + 1] << 8) | (input[j + 2] << 16) | (input[j + 3] << 24);
    }
}

/* Copies len bytes. */
void MD5_memcpy(u8 *output, u8 *input, u32 len) {
    u32 i;

    for (i = 0; i < len; i++) {
        output[i] = input[i];
    }
}

/* Fills len bytes with value. */
void MD5_memset(u8 *output, s32 value, u32 len) {
    u32 i;

    for (i = 0; i < len; i++) {
        output[i] = value;
    }
}

INCLUDE_RODATA("asm/jp/main/nonmatchings/lib/md5", PADDING);
