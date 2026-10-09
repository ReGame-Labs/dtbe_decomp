#ifndef COMMON_H
#define COMMON_H

/* The game's integer types, NULL, offsetof, and the macros that give C linkage in the C++ files. */

#include "include_asm.h"

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

/* the offset of a member in its struct, in bytes */
#ifndef offsetof
#define offsetof(type, member) ((s32) & ((type *)0)->member)
#endif

/* Gives the C functions declared between them C linkage in a C++ file. */
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

#endif /* COMMON_H */
