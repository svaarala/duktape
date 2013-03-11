/*
 *  Bit types such as duk_u32.
 */

#ifndef DUK_BITTYPES_H_INCLUDED
#define DUK_BITTYPES_H_INCLUDED

/* FIXME: Is there a reason not to rely on C99 types only, and only fall
 * back to guessing if C99 types are not available?
 */

/* FIXME: How to do reasonable automatic detection on older compilers,
 * and how to allow user override?
 */

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
/* C99 */
#include <inttypes.h>
typedef uint8_t duk_u8;
typedef int8_t duk_i8;
typedef uint16_t duk_u16;
typedef int16_t duk_i16;
typedef uint32_t duk_u32;
typedef int32_t duk_i32;
#else
/* FIXME: need actual detection here */
typedef unsigned char duk_u8;
typedef signed char duk_i8;
typedef unsigned short duk_u16;
typedef signed short duk_i16;
typedef unsigned int duk_u32;
typedef signed int duk_i32;
#endif

#endif  /* DUK_BITTYPES_H_INCLUDED */

