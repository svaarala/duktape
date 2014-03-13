/*
 *  Determine platform features, select feature selection defines
 *  (e.g. _XOPEN_SOURCE), include system headers, and define DUK_USE_XXX
 *  defines which are (only) checked in Duktape internal code for
 *  activated features.  Duktape feature selection is based on automatic
 *  feature detection, user supplied DUK_OPT_xxx defines, and optionally
 *  a "duk_custom.h" user header (if DUK_OPT_HAVE_CUSTOM_H is defined).
 *
 *  This file is included by duk_internal.h before anything else is
 *  included.  Feature selection defines (e.g. _XOPEN_SOURCE) are defined
 *  here before any system headers are included (which is a requirement for
 *  system headers to work correctly).  This file is responsible for including
 *  all system headers and contains all platform dependent cruft in general.
 *
 *  The public duktape.h has minimal feature detection required by the public
 *  API (for instance use of variadic macros is detected there).  Duktape.h
 *  exposes its detection results as DUK_API_xxx.  The public header and the
 *  implementation must agree on e.g. names and argument lists of exposed
 *  calls; these are checked by duk_features_sanity.h (duktape.h is not yet
 *  included when this file is included to avoid fouling up feature selection
 *  defines).
 *
 *  The general order of handling:
 *    - Compiler feature detection (require no includes)
 *    - Intermediate platform detection (-> easier platform defines)
 *    - Platform detection, system includes, byte order detection, etc
 *    - ANSI C wrappers (e.g. DUK_MEMCMP), wrappers for constants, etc
 *    - DUK_USE_xxx defines are resolved based on input defines
 *    - Duktape Date provider settings
 *    - Final sanity checks
 *
 *  DUK_F_XXX are internal feature detection macros which should not
 *  be used outside this header.
 *
 *  Useful resources:
 *
 *    http://sourceforge.net/p/predef/wiki/Home/
 *    http://sourceforge.net/p/predef/wiki/Architectures/
 *    http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
 *    http://en.wikipedia.org/wiki/C_data_types#Fixed-width_integer_types
 *
 *  Preprocessor defines available in a particular GCC:
 *
 *    gcc -dM -E - </dev/null   # http://www.brain-dump.org/blog/entry/107
 */

#ifndef DUK_FEATURES_H_INCLUDED
#define DUK_FEATURES_H_INCLUDED

/*
 *  Compiler features
 */

#undef DUK_F_C99
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define DUK_F_C99
#endif

#undef DUK_F_CPP
#if defined(__cplusplus)
#define DUK_F_CPP
#endif

#undef DUK_F_CPP11
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#define DUK_F_CPP11
#endif

/*
 *  Provides the duk_rdtsc() inline function (if available)
 *
 *  See: http://www.mcs.anl.gov/~kazutomo/rdtsc.html
 */

/* XXX: more accurate detection of what gcc versions work; more inline
 * asm versions for other compilers.
 */
#if defined(__GNUC__) && defined(__i386__) && \
    !defined(__cplusplus) /* unsigned long long not standard */
static __inline__ unsigned long long duk_rdtsc(void) {
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
}
#define DUK_RDTSC_AVAILABLE 1
#elif defined(__GNUC__) && defined(__x86_64__) && \
    !defined(__cplusplus) /* unsigned long long not standard */
static __inline__ unsigned long long duk_rdtsc(void) {
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((unsigned long long) lo) | (((unsigned long long) hi) << 32);
}
#define DUK_RDTSC_AVAILABLE 1
#else
/* not available */
#undef DUK_RDTSC_AVAILABLE
#endif

/*
 *  Intermediate platform, architecture, and compiler detection.  These are
 *  hopelessly intertwined - e.g. architecture defines depend on compiler etc.
 *
 *  Provide easier defines for platforms and compilers which are often tricky
 *  or verbose to detect.  The intent is not to provide intermediate defines for
 *  all features; only if existing feature defines are inconvenient.
 */

/* Intel x86 (32-bit) */
#if defined(i386) || defined(__i386) || defined(__i386__) || \
    defined(__i486__) || defined(__i586__) || defined(__i686__) || \
    defined(__IA32__) || defined(_M_IX86) || defined(__X86__) || \
    defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__)
#define DUK_F_X86
#endif

/* AMD64 (64-bit) */
#if defined(__amd64__) || defined(__amd64) || \
    defined(__x86_64__) || defined(__x86_64) || \
    defined(_M_X64) || defined(_M_AMD64)
#define DUK_F_X64
#endif

/* FIXME: X32: pointers are 32-bit so packed format can be used, but X32
 * support is not yet mature.
 */

/* ARM */
#if defined(__arm__) || defined(__thumb__) || defined(_ARM) || defined(_M_ARM)
#define DUK_F_ARM
#endif

/* MIPS */
#if defined(__mips__) || defined(mips) || defined(_MIPS_ISA) || \
    defined(_R3000) || defined(_R4000) || defined(_R5900) || \
    defined(_MIPS_ISA_MIPS1) || defined(_MIPS_ISA_MIPS2) || \
    defined(_MIPS_ISA_MIPS3) || defined(_MIPS_ISA_MIPS4) || \
    defined(__mips) || defined(__MIPS__)
#define DUK_F_MIPS
#endif

/* Motorola 68K.  Not defined by VBCC, so user must define one of these
 * manually when using VBCC.
 */
#if defined(__m68k__) || defined(M68000) || defined(__MC68K__)
#define DUK_F_M68K
#endif

/* Linux */
#if defined(__linux) || defined(__linux__) || defined(linux)
#define DUK_F_LINUX
#endif

/* FreeBSD */
#if defined(__FreeBSD__) || defined(__FreeBSD)
#define DUK_F_FREEBSD
#endif

/* NetBSD */
#if defined(__NetBSD__) || defined(__NetBSD)
#define DUK_F_NETBSD
#endif

/* OpenBSD */
#if defined(__OpenBSD__) || defined(__OpenBSD)
#define DUK_F_OPENBSD
#endif

/* BSD variant */
#if defined(DUK_F_FREEBSD) || defined(DUK_F_NETBSD) || defined(DUK_F_OPENBSD) || \
    defined(__bsdi__) || defined(__DragonFly__)
#define DUK_F_BSD
#endif

/* Generic Unix */
#if defined(__unix) || defined(__unix__) || defined(unix) || \
    defined(DUK_F_LINUX) || defined(DUK_F_BSD)
#define DUK_F_UNIX
#endif

/* Windows (32-bit or above) */
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) || \
    defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#define DUK_F_WINDOWS
#endif

/* Atari ST TOS. __TOS__ defined by PureC (which doesn't work as a target now
 * because int is 16-bit, to be fixed).  No platform define in VBCC apparently,
 * so to use with VBCC, user must define '__TOS__' manually.
  */
#if defined(__TOS__)
#define DUK_F_TOS
#endif

/* AmigaOS.  Neither AMIGA nor __amigaos__ is defined on VBCC, so user must
 * define 'AMIGA' manually.
 */
#if defined(AMIGA) || defined(__amigaos__)
#define DUK_F_AMIGAOS
#endif

/* Flash player (e.g. Crossbridge) */
#if defined(__FLASHPLAYER__)
#define DUK_F_FLASHPLAYER
#endif

/* Emscripten (provided explicitly by user), improve if possible */
#if defined(EMSCRIPTEN)
#define DUK_F_EMSCRIPTEN
#endif

/* QNX */
#if defined(__QNX__)
#define DUK_F_QNX
#endif

/* GCC and GCC version convenience define. */
#if defined(__GNUC__)
#define DUK_F_GCC
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && defined(__GNUC_PATCHLEVEL__)
/* Convenience, e.g. gcc 4.5.1 == 40501; http://stackoverflow.com/questions/6031819/emulating-gccs-builtin-unreachable */
#define DUK_F_GCC_VERSION  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#error cannot figure out gcc version
#endif
#endif

/* Clang */
#if defined(__clang__)
#define DUK_F_CLANG
#endif

/* MSVC */
#if defined(_MSC_VER)
#define DUK_F_MSVC
#endif

/* MinGW */
#if defined(__MINGW32__) || defined(__MINGW64__)
#define DUK_F_MINGW
#endif

/*
 *  Platform detection, system includes, Date provider selection.
 *
 *  Feature selection (e.g. _XOPEN_SOURCE) must happen before any system
 *  headers are included.
 *
 *  Date provider selection seems a bit out-of-place here, but since
 *  the date headers and provider functions are heavily platform
 *  specific, there's little point in duplicating the platform if-else
 *  ladder.  All platform specific Date provider functions are in
 *  duk_bi_date.c; here we provide appropriate #defines to enable them,
 *  and include all the necessary system headers so that duk_bi_date.c
 *  compiles.  Date "providers" are:
 *
 *    NOW = getting current time (required)
 *    TZO = getting local time offset (required)
 *    PRS = parse datetime (optional)
 *    FMT = format datetime (optional)
 *
 *  There's a lot of duplication here, unfortunately, because many
 *  platforms have similar (but not identical) headers, Date providers,
 *  etc.  The duplication could be removed by more complicated nested
 *  #ifdefs, but it would then be more difficult to make fixes which
 *  affect only a specific platform.
 *
 *  FIXME: add a way to provide custom functions to provide the critical
 *  primitives; this would be convenient when porting to unknown platforms
 *  (rather than muck with Duktape internals).
 */

#if defined(DUK_F_LINUX)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE  200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE      /* e.g. getdate_r */
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE    /* e.g. strptime */
#endif
#endif

#if defined(DUK_F_QNX)
/* See: /opt/qnx650/target/qnx6/usr/include/sys/platform.h */
#define _XOPEN_SOURCE    600
#define _POSIX_C_SOURCE  200112
#endif

#if defined(__APPLE__)
/* Mac OSX, iPhone, Darwin */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <architecture/byte_order.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(DUK_F_OPENBSD)
/* http://www.monkey.org/openbsd/archive/ports/0401/msg00089.html */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <sys/endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(DUK_F_BSD)
/* other BSD */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <sys/endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(DUK_F_TOS)
/* Atari ST TOS */
#define DUK_USE_DATE_NOW_TIME
#define DUK_USE_DATE_TZO_GMTIME
/* no parsing (not an error) */
#define DUK_USE_DATE_FMT_STRFTIME
#include <limits.h>
#include <time.h>
#elif defined(DUK_F_AMIGAOS)
#if defined(DUK_F_M68K)
/* AmigaOS on M68k */
#define DUK_USE_DATE_NOW_TIME
#define DUK_USE_DATE_TZO_GMTIME
/* no parsing (not an error) */
#define DUK_USE_DATE_FMT_STRFTIME
#include <limits.h>
#include <time.h>
#else
#error AmigaOS but not M68K, not supported now
#endif
#elif defined(DUK_F_WINDOWS)
/* Windows 32-bit and 64-bit are currently the same. */
/* MSVC does not have sys/param.h */
#define DUK_USE_DATE_NOW_WINDOWS
#define DUK_USE_DATE_TZO_WINDOWS
/* Note: PRS and FMT are intentionally left undefined for now.  This means
 * there is no platform specific date parsing/formatting but there is still
 * the ISO 8601 standard format.
 */
#include <windows.h>
#include <limits.h>
#elif defined(DUK_F_FLASHPLAYER)
/* Crossbridge */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(DUK_F_QNX)
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(DUK_F_LINUX)
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#elif defined(__posix)
/* POSIX */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#else
/* Other UNIX, hopefully others */
#define DUK_USE_DATE_NOW_GETTIMEOFDAY
#define DUK_USE_DATE_TZO_GMTIME_R
#define DUK_USE_DATE_PRS_STRPTIME
#define DUK_USE_DATE_FMT_STRFTIME
#include <sys/types.h>
#include <endian.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/time.h>
#include <time.h>
#endif

/* Shared includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  /* varargs */
#include <setjmp.h>
#include <stddef.h>  /* e.g. ptrdiff_t */
#ifdef DUK_F_TOS
/*FIXME*/
#else
/* XXX: technically C99 (C++11) but found in many systems */
#include <stdint.h>
#endif
#include <math.h>

/*
 *  Wrapper typedefs and constants for integer types, also sanity check types.
 *
 *  C99 typedefs are quite good but not always available, and we want to avoid
 *  forcibly redefining the C99 typedefs.  So, there are Duktape wrappers for
 *  all C99 typedefs and Duktape code should only use these typedefs.  The
 *  Duktape public API is problematic from type detection perspective and must
 *  be taken into account here.
 *
 *  Type detection when C99 is not supported is quite simplistic now and will
 *  only work on 32-bit platforms (64-bit platforms are OK with C99 types).
 *
 *  Pointer sizes are a portability problem: pointers to different types may
 *  have a different size and function pointers are very difficult to manage
 *  portably.
 *
 *  http://en.wikipedia.org/wiki/C_data_types#Fixed-width_integer_types
 *
 *  Note: avoid typecasts and computations in macro integer constants as they
 *  can then no longer be used in macro relational expressions (such as
 *  #if DUK_SIZE_MAX < 0xffffffffUL).  There is internal code which relies on
 *  being able to compare DUK_SIZE_MAX against a limit.
 */

/* FIXME: How to do reasonable automatic detection on older compilers,
 * and how to allow user override?
 */

/* FIXME: this assumption must be in place until no 'int' variables are
 * used anywhere, including the public Duktape API.  Also all printf()
 * format characters need to be changed.
 */
#if defined(INT_MAX)
#if INT_MAX < 2147483647
#error INT_MAX too small, expected int to be 32 bits at least
#endif
#else
#error INT_MAX not defined
#endif

/* Check that architecture is two's complement, standard C allows e.g.
 * INT_MIN to be -2**31+1 (instead of -2**31).
 */
#if defined(INT_MAX) && defined(INT_MIN)
#if INT_MAX != -(INT_MIN + 1)
#error platform does not seem complement of two
#endif
#else
#error cannot check complement of two
#endif

/* Intermediate define for 'have inttypes.h' */
#undef DUK_F_HAVE_INTTYPES
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && \
    !(defined(DUK_F_AMIGAOS) && defined(__VBCC__))
/* vbcc + AmigaOS has C99 but no inttypes.h */
#define DUK_F_HAVE_INTTYPES
#elif defined(__cplusplus) && (__cplusplus >= 201103L)
/* C++11 apparently ratified stdint.h */
#define DUK_F_HAVE_INTTYPES
#endif

/* Basic integer typedefs and limits, preferably from inttypes.h, otherwise
 * through automatic detection.
 */
#if defined(DUK_F_HAVE_INTTYPES)
/* C99 */
#define DUK_F_HAVE_64BIT
#include <inttypes.h>

typedef uint8_t duk_uint8_t;
typedef int8_t duk_int8_t;
typedef uint16_t duk_uint16_t;
typedef int16_t duk_int16_t;
typedef uint32_t duk_uint32_t;
typedef int32_t duk_int32_t;
typedef uint64_t duk_uint64_t;
typedef int64_t duk_int64_t;
typedef uint_least8_t duk_uint_least8_t;
typedef int_least8_t duk_int_least8_t;
typedef uint_least16_t duk_uint_least16_t;
typedef int_least16_t duk_int_least16_t;
typedef uint_least32_t duk_uint_least32_t;
typedef int_least32_t duk_int_least32_t;
typedef uint_least64_t duk_uint_least64_t;
typedef int_least64_t duk_int_least64_t;
typedef uint_fast8_t duk_uint_fast8_t;
typedef int_fast8_t duk_int_fast8_t;
typedef uint_fast16_t duk_uint_fast16_t;
typedef int_fast16_t duk_int_fast16_t;
typedef uint_fast32_t duk_uint_fast32_t;
typedef int_fast32_t duk_int_fast32_t;
typedef uint_fast64_t duk_uint_fast64_t;
typedef int_fast64_t duk_int_fast64_t;
typedef uintptr_t duk_uintptr_t;
typedef intptr_t duk_intptr_t;
typedef uintmax_t duk_uintmax_t;
typedef intmax_t duk_intmax_t;

#define DUK_UINT8_MIN         0
#define DUK_UINT8_MAX         UINT8_MAX
#define DUK_INT8_MIN          INT8_MIN
#define DUK_INT8_MAX          INT8_MAX
#define DUK_UINT_LEAST8_MIN   0
#define DUK_UINT_LEAST8_MAX   UINT_LEAST8_MAX
#define DUK_INT_LEAST8_MIN    INT_LEAST8_MIN
#define DUK_INT_LEAST8_MAX    INT_LEAST8_MAX
#define DUK_UINT_FAST8_MIN    0
#define DUK_UINT_FAST8_MAX    UINT_FAST8_MAX
#define DUK_INT_FAST8_MIN     INT_FAST8_MIN
#define DUK_INT_FAST8_MAX     INT_FAST8_MAX
#define DUK_UINT16_MIN        0
#define DUK_UINT16_MAX        UINT16_MAX
#define DUK_INT16_MIN         INT16_MIN
#define DUK_INT16_MAX         INT16_MAX
#define DUK_UINT_LEAST16_MIN  0
#define DUK_UINT_LEAST16_MAX  UINT_LEAST16_MAX
#define DUK_INT_LEAST16_MIN   INT_LEAST16_MIN
#define DUK_INT_LEAST16_MAX   INT_LEAST16_MAX
#define DUK_UINT_FAST16_MIN   0
#define DUK_UINT_FAST16_MAX   UINT_FAST16_MAX
#define DUK_INT_FAST16_MIN    INT_FAST16_MIN
#define DUK_INT_FAST16_MAX    INT_FAST16_MAX
#define DUK_UINT32_MIN        0
#define DUK_UINT32_MAX        UINT32_MAX
#define DUK_INT32_MIN         INT32_MIN
#define DUK_INT32_MAX         INT32_MAX
#define DUK_UINT_LEAST32_MIN  0
#define DUK_UINT_LEAST32_MAX  UINT_LEAST32_MAX
#define DUK_INT_LEAST32_MIN   INT_LEAST32_MIN
#define DUK_INT_LEAST32_MAX   INT_LEAST32_MAX
#define DUK_UINT_FAST32_MIN   0
#define DUK_UINT_FAST32_MAX   UINT_FAST32_MAX
#define DUK_INT_FAST32_MIN    INT_FAST32_MIN
#define DUK_INT_FAST32_MAX    INT_FAST32_MAX
#define DUK_UINT64_MIN        0
#define DUK_UINT64_MAX        UINT64_MAX
#define DUK_INT64_MIN         INT64_MIN
#define DUK_INT64_MAX         INT64_MAX
#define DUK_UINT_LEAST64_MIN  0
#define DUK_UINT_LEAST64_MAX  UINT_LEAST64_MAX
#define DUK_INT_LEAST64_MIN   INT_LEAST64_MIN
#define DUK_INT_LEAST64_MAX   INT_LEAST64_MAX
#define DUK_UINT_FAST64_MIN   0
#define DUK_UINT_FAST64_MAX   UINT_FAST64_MAX
#define DUK_INT_FAST64_MIN    INT_FAST64_MIN
#define DUK_INT_FAST64_MAX    INT_FAST64_MAX
#define DUK_UINTPTR_MIN       0
#define DUK_UINTPTR_MAX       UINTPTR_MAX
#define DUK_INTPTR_MIN        INTPTR_MIN
#define DUK_INTPTR_MAX        INTPTR_MAX
#define DUK_UINTMAX_MIN       0
#define DUK_UINTMAX_MAX       UINTMAX_MAX
#define DUK_INTMAX_MIN        INTMAX_MIN
#define DUK_INTMAX_MAX        INTMAX_MAX
#define DUK_SIZE_MIN          0
#define DUK_SIZE_MAX          SIZE_MAX

#else  /* C99 types */

/* When C99 types are not available, we use simplistic detection to get
 * the basic 8, 16, and 32 bit types.  The fast/least types are then
 * assumed to be exactly the same for now: these could be improved per
 * platform but C99 types are very often now available.  64-bit types
 * are not defined at all now (duk_uint64_t etc).
 *
 * This detection code is necessarily a bit hacky and can provide typedefs
 * and defines that won't work correctly on some exotic platform.  It only
 * really works on 32-bit platforms.
 */

#undef DUK_F_HAVE_64BIT

#if (defined(CHAR_BIT) && (CHAR_BIT == 8)) || \
    (defined(UCHAR_MAX) && (UCHAR_MAX == 255))
typedef unsigned char duk_uint8_t;
typedef signed char duk_int8_t;
#else
#error cannot detect 8-bit type
#endif

#if defined(USHRT_MAX) && (USHRT_MAX == 65535)
typedef unsigned short duk_uint16_t;
typedef signed short duk_int16_t;
#else
#error cannot detect 16-bit type
#endif

#if defined(UINT_MAX) && (UINT_MAX == 4294967295)
typedef unsigned int duk_uint32_t;
typedef signed int duk_int32_t;
#elif defined(ULONG_MAX) && (ULONG_MAX == 4294967295)
/* On some platforms int is 16-bit but long is 32-bit (e.g. PureC) */
typedef unsigned long duk_uint32_t;
typedef signed long duk_int32_t;
#else
#error cannot detect 32-bit type
#endif

typedef duk_uint8_t duk_uint_least8_t;
typedef duk_int8_t duk_int_least8_t;
typedef duk_uint16_t duk_uint_least16_t;
typedef duk_int16_t duk_int_least16_t;
typedef duk_uint32_t duk_uint_least32_t;
typedef duk_int32_t duk_int_least32_t;
typedef duk_uint8_t duk_uint_fast8_t;
typedef duk_int8_t duk_int_fast8_t;
typedef duk_uint16_t duk_uint_fast16_t;
typedef duk_int16_t duk_int_fast16_t;
typedef duk_uint32_t duk_uint_fast32_t;
typedef duk_int32_t duk_int_fast32_t;
typedef duk_int32_t duk_intmax_t;
typedef duk_uint32_t duk_uintmax_t;

/* This detection is not very reliable, and only supports 32-bit platforms
 * now (64-bit platforms work if C99 types are available).
 */
#if (defined(__WORDSIZE) && (__WORDSIZE == 32)) || \
    (defined(DUK_F_MINGW) && defined(_X86_)) || \
    (defined(DUK_F_MSVC) && defined(_M_IX86))
typedef duk_int32_t duk_intptr_t;
typedef duk_uint32_t duk_uintptr_t;
#elif (defined(DUK_F_MINGW) && defined(_WIN64)) || \
      (defined(DUK_F_MSVC) && defined(_WIN64))
/* Both MinGW and MSVC have a 64-bit type. */
typedef long long duk_intptr_t;
typedef unsigned long long duk_uintptr_t;
#else
#error cannot determine intptr type
#endif

/* Pretend that maximum int is 32 bits. */
typedef duk_uint32_t duk_uintmax_t;
typedef duk_int32_t duk_intmax_t;

#define DUK_UINT8_MIN         0UL
#define DUK_UINT8_MAX         0xffUL
#define DUK_INT8_MIN          (-0x80L)
#define DUK_INT8_MAX          0x7fL
#define DUK_UINT_LEAST8_MIN   0UL
#define DUK_UINT_LEAST8_MAX   0xffUL
#define DUK_INT_LEAST8_MIN    (-0x80L)
#define DUK_INT_LEAST8_MAX    0x7fL
#define DUK_UINT_FAST8_MIN    0UL
#define DUK_UINT_FAST8_MAX    0xffUL
#define DUK_INT_FAST8_MIN     (-0x80L)
#define DUK_INT_FAST8_MAX     0x7fL
#define DUK_UINT16_MIN        0UL
#define DUK_UINT16_MAX        0xffffUL
#define DUK_INT16_MIN         (-0x8000L)
#define DUK_INT16_MAX         0x7fffL
#define DUK_UINT_LEAST16_MIN  0UL
#define DUK_UINT_LEAST16_MAX  0xffffUL
#define DUK_INT_LEAST16_MIN   (-0x8000L)
#define DUK_INT_LEAST16_MAX   0x7fffL
#define DUK_UINT_FAST16_MIN   0UL
#define DUK_UINT_FAST16_MAX   0xffffUL
#define DUK_INT_FAST16_MIN    (-0x8000L)
#define DUK_INT_FAST16_MAX    0x7fffL
#define DUK_UINT32_MIN        0UL
#define DUK_UINT32_MAX        0xffffffffUL
#define DUK_INT32_MIN         (-0x80000000L)
#define DUK_INT32_MAX         0x7fffffffL
#define DUK_UINT_LEAST32_MIN  0UL
#define DUK_UINT_LEAST32_MAX  0xffffffffUL
#define DUK_INT_LEAST32_MIN   (-0x80000000L)
#define DUK_INT_LEAST32_MAX   0x7fffffffL
#define DUK_UINT_FAST32_MIN   0UL
#define DUK_UINT_FAST32_MAX   0xffffffffUL
#define DUK_INT_FAST32_MIN    (-0x80000000L)
#define DUK_INT_FAST32_MAX    0x7fffffffL
#define DUK_UINT64_MIN        0ULL
#define DUK_UINT64_MAX        0xffffffffffffffffULL
#define DUK_INT64_MIN         (-0x8000000000000000LL)
#define DUK_INT64_MAX         0x7fffffffffffffffULL
#define DUK_UINT_LEAST64_MIN  0ULL
#define DUK_UINT_LEAST64_MAX  0xffffffffffffffffULL
#define DUK_INT_LEAST64_MIN   (-0x8000000000000000LL)
#define DUK_INT_LEAST64_MAX   0x7fffffffffffffffULL
#define DUK_UINT_FAST64_MIN   0ULL
#define DUK_UINT_FAST64_MAX   0xffffffffffffffffULL
#define DUK_INT_FAST64_MIN    (-0x8000000000000000LL)
#define DUK_INT_FAST64_MAX    0x7fffffffffffffffULL
#define DUK_UINTPTR_MIN       0UL
#define DUK_UINTPTR_MAX       0xffffffffUL
#define DUK_INTPTR_MIN        (-0x80000000L)
#define DUK_INTPTR_MAX        0x7fffffffL
#define DUK_UINTMAX_MIN       0UL
#define DUK_UINTMAX_MAX       0xffffffffUL
#define DUK_INTMAX_MIN        (-0x80000000L)
#define DUK_INTMAX_MAX        0x7fffffffL

/* SIZE_MAX may be missing so use an approximate value for it. */
#undef DUK_SIZE_MAX_COMPUTED
#if !defined(SIZE_MAX)
#define DUK_SIZE_MAX_COMPUTED
#define SIZE_MAX              ((size_t) (-1))
#endif
#define DUK_SIZE_MIN          0
#define DUK_SIZE_MAX          SIZE_MAX

#endif  /* C99 types */

/* The best type for an "all around int" in Duktape internals is "at least
 * 32 bit signed integer" which is fastest.  Same for unsigned type.
 */
typedef duk_int_fast32_t duk_int_t;
typedef duk_uint_fast32_t duk_uint_t;
#define DUK_INT_MIN           DUK_INT_FAST32_MIN
#define DUK_INT_MAX           DUK_INT_FAST32_MAX
#define DUK_UINT_MIN          DUK_UINT_FAST32_MIN
#define DUK_UINT_MAX          DUK_UINT_FAST32_MAX

/* Small integers (16 bits or more) can fall back to the 'int' type, but
 * have a typedef so they are marked "small" explicitly.
 */
typedef int duk_small_int_t;
typedef unsigned int duk_small_uint_t;
#define DUK_SMALL_INT_MIN     INT_MIN
#define DUK_SMALL_INT_MAX     INT_MAX
#define DUK_SMALL_UINT_MIN    0
#define DUK_SMALL_UINT_MAX    UINT_MAX

/* Codepoint type.  Must be 32 bits or more because it is used also for
 * internal codepoints.  The type is signed because negative codepoints
 * are used as internal markers (e.g. to mark EOF or missing argument).
 * (X)UTF-8/CESU-8 encode/decode take and return an unsigned variant to
 * ensure duk_uint32_t casts back and forth nicely.  Almost everything
 * else uses the signed one.
 */
typedef duk_int32_t duk_codepoint_t;
typedef duk_uint32_t duk_ucodepoint_t;

/* IEEE double typedef. */
typedef double duk_double_t;

/* We're generally assuming that we're working on a platform with a 32-bit
 * address space.  If DUK_SIZE_MAX is a typecast value (which is necessary
 * if SIZE_MAX is missing), the check must be avoided because the
 * preprocessor can't do a comparison.
 */
#if !defined(DUK_SIZE_MAX)
#error DUK_SIZE_MAX is undefined, probably missing SIZE_MAX
#elif !defined(DUK_SIZE_MAX_COMPUTED)
#if DUK_SIZE_MAX < 0xffffffffUL
/* XXX: compare against a lower value; can SIZE_MAX realistically be
 * e.g. 0x7fffffffUL on a 32-bit system?
 */
#error size_t is too small
#endif
#endif

/* Convenience define: 32-bit pointers.  This is an important optimization
 * target (e.g. for struct sizing).
 */
#undef DUK_USE_32BIT_PTRS
#if DUK_UINTPTR_MAX <= 0xffffffffUL
#define DUK_USE_32BIT_PTRS
#endif

/*
 *  Check whether we should use 64-bit integers
 */

/* Quite incomplete now: require C99, avoid 64-bit types on VBCC because
 * they seem to misbehave.  Should use 64-bit operations at least on 64-bit
 * platforms even when C99 not available (perhaps integrate to bit type
 * detection?).
 */
#if defined(DUK_F_HAVE_64BIT) && !defined(__VBCC__)
#define DUK_USE_64BIT_OPS
#else
#undef DUK_USE_64BIT_OPS
#endif

/*
 *  Alignment requirement and support for unaligned accesses
 *
 *  Assume unaligned accesses are not supported unless specifically allowed
 *  in the target platform.  Some platforms may support unaligned accesses
 *  but alignment to 4 or 8 may still be desirable.
 */

#undef DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#undef DUK_USE_ALIGN_4
#undef DUK_USE_ALIGN_8

#if defined(DUK_F_EMSCRIPTEN)
/* Required on at least some targets, so use whenever Emscripten used,
 * regardless of compilation target.
 */
#define DUK_USE_ALIGN_8
#elif defined(DUK_F_ARM)
#define DUK_USE_ALIGN_4
#elif defined(DUK_F_MIPS)
#define DUK_USE_ALIGN_4
#elif defined(DUK_F_X86) || defined(DUK_F_X64)
#define DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#else
/* unknown, use safe default */
#define DUK_USE_ALIGN_8
#endif

/* User forced alignment to 4 or 8. */
#if defined(DUK_OPT_FORCE_ALIGN)
#undef DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#undef DUK_USE_ALIGN_4
#undef DUK_USE_ALIGN_8
#if (DUK_OPT_FORCE_ALIGN == 4)
#define DUK_USE_ALIGN_4
#elif (DUK_OPT_FORCE_ALIGN == 8)
#define DUK_USE_ALIGN_8
#else
#error invalid DUK_OPT_FORCE_ALIGN value
#endif
#endif

/* Compiler specific hackery needed to force struct size to match aligment,
 * see e.g. duk_hbuffer.h.
 *
 * http://stackoverflow.com/questions/11130109/c-struct-size-alignment
 * http://stackoverflow.com/questions/10951039/specifying-64-bit-alignment
 */
#if defined(DUK_F_MSVC)
#define DUK_USE_PACK_MSVC_PRAGMA
#elif defined(DUK_F_GCC)
#define DUK_USE_PACK_GCC_ATTR
#elif defined(DUK_F_CLANG)
#define DUK_USE_PACK_CLANG_ATTR
#else
#define DUK_USE_PACK_DUMMY_MEMBER
#endif

#ifdef DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#define DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS
#else
#undef DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS
#endif

/* Object property allocation layout has implications for memory and code
 * footprint and generated code size/speed.  The best layout also depends
 * on whether the platform has alignment requirements or benefits from
 * having mostly aligned accesses.
 */
#undef DUK_USE_HOBJECT_LAYOUT_1
#undef DUK_USE_HOBJECT_LAYOUT_2
#undef DUK_USE_HOBJECT_LAYOUT_3
#if defined(DUK_USE_UNALIGNED_ACCESSES_POSSIBLE) && \
    !defined(DUK_USE_ALIGN_4) && !defined(DUK_USE_ALIGN_8)
/* On platforms without any alignment issues, layout 1 is preferable
 * because it compiles to slightly less code and provides direct access
 * to property keys.
 */
#define DUK_USE_HOBJECT_LAYOUT_1
#else
/* On other platforms use layout 2, which requires some padding but
 * is a bit more natural than layout 3 in ordering the entries.  Layout
 * 3 is currently not used.
 */
#define DUK_USE_HOBJECT_LAYOUT_2
#endif

/*
 *  Byte order and double memory layout detection
 *
 *  Endianness detection is a major portability hassle because the macros
 *  and headers are not standardized.  There's even variance across UNIX
 *  platforms.  Even with "standard" headers, details like underscore count
 *  varies between platforms, e.g. both __BYTE_ORDER and _BYTE_ORDER are used
 *  (Crossbridge has a single underscore, for instance).
 *
 *  The checks below are structured with this in mind: several approaches are
 *  used, and at the end we check if any of them worked.  This allows generic
 *  approaches to be tried first, and platform/compiler specific hacks tried
 *  last.  As a last resort, the user can force a specific endianness, as it's
 *  not likely that automatic detection will work on the most exotic platforms.
 *
 *  Duktape supports little and big endian machines.  There's also support
 *  for a hybrid used by some ARM machines where integers are little endian
 *  but IEEE double values use a mixed order (12345678 -> 43218765).  This
 *  byte order for doubles is referred to as "mixed endian".
 */

#undef DUK_F_BYTEORDER
#undef DUK_USE_BYTEORDER_FORCED

/* DUK_F_BYTEORDER is set as an intermediate value when detection
 * succeeds, to one of:
 *   1 = little endian
 *   2 = mixed (arm hybrid) endian
 *   3 = big endian
 */

/* For custom platforms allow user to define byteorder explicitly.
 * Since endianness headers are not standardized, this is a useful
 * workaround for custom platforms for which endianness detection
 * is not directly supported.  Perhaps custom hardware is used and
 * user cannot submit upstream patches.
 */
#if defined(DUK_OPT_FORCE_BYTEORDER)
#if (DUK_OPT_FORCE_BYTEORDER == 1)
#define DUK_F_BYTEORDER 1
#elif (DUK_OPT_FORCE_BYTEORDER == 2)
#define DUK_F_BYTEORDER 2
#elif (DUK_OPT_FORCE_BYTEORDER == 3)
#define DUK_F_BYTEORDER 3
#else
#error invalid DUK_OPT_FORCE_BYTEORDER value
#endif
#define DUK_USE_BYTEORDER_FORCED
#endif  /* DUK_OPT_FORCE_BYTEORDER */

/* More or less standard endianness predefines provided by header files.
 * The ARM hybrid case is detected by assuming that __FLOAT_WORD_ORDER
 * will be big endian, see: http://lists.mysql.com/internals/443.
 */
#if !defined(DUK_F_BYTEORDER)
#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN) || \
    defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && (_BYTE_ORDER == _LITTLE_ENDIAN) || \
    defined(__LITTLE_ENDIAN__)
/* Integer little endian */
#if defined(__FLOAT_WORD_ORDER) && defined(__LITTLE_ENDIAN) && (__FLOAT_WORD_ORDER == __LITTLE_ENDIAN) || \
    defined(_FLOAT_WORD_ORDER) && defined(_LITTLE_ENDIAN) && (_FLOAT_WORD_ORDER == _LITTLE_ENDIAN)
#define DUK_F_BYTEORDER 1
#elif defined(__FLOAT_WORD_ORDER) && defined(__BIG_ENDIAN) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN) || \
      defined(_FLOAT_WORD_ORDER) && defined(_BIG_ENDIAN) && (_FLOAT_WORD_ORDER == _BIG_ENDIAN)
#define DUK_F_BYTEORDER 2
#elif !defined(__FLOAT_WORD_ORDER) && !defined(_FLOAT_WORD_ORDER)
/* Float word order not known, assume not a hybrid. */
#define DUK_F_BYTEORDER 1
#else
/* byte order is little endian but cannot determine IEEE double word order */
#endif  /* float word order */
#elif defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN) || \
      defined(_BYTE_ORDER) && defined(_BIG_ENDIAN) && (_BYTE_ORDER == _BIG_ENDIAN) || \
      defined(__BIG_ENDIAN__)
/* Integer big endian */
#if defined(__FLOAT_WORD_ORDER) && defined(__BIG_ENDIAN) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN) || \
    defined(_FLOAT_WORD_ORDER) && defined(_BIG_ENDIAN) && (_FLOAT_WORD_ORDER == _BIG_ENDIAN) ||
#define DUK_F_BYTEORDER 3
#elif !defined(__FLOAT_WORD_ORDER) && !defined(_FLOAT_WORD_ORDER)
/* Float word order not known, assume not a hybrid. */
#define DUK_F_BYTEORDER 3
#else
/* byte order is big endian but cannot determine IEEE double word order */
#endif  /* float word order */
#else
/* cannot determine byte order */
#endif  /* integer byte order */
#endif  /* !defined(DUK_F_BYTEORDER) */

/* GCC and Clang provide endianness defines as built-in predefines, with
 * leading and trailing double underscores (e.g. __BYTE_ORDER__).  See
 * output of "make gccpredefs" and "make clangpredefs".  Clang doesn't
 * seem to provide __FLOAT_WORD_ORDER__.
 * http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
 */
#if !defined(DUK_F_BYTEORDER) && defined(__BYTE_ORDER__)
#if defined(__ORDER_LITTLE_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
/* Integer little endian */
#if defined(__FLOAT_WORD_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    (__FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define DUK_F_BYTEORDER 1
#elif defined(__FLOAT_WORD_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
      (__FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__)
#define DUK_F_BYTEORDER 2
#elif !defined(__FLOAT_WORD_ORDER__)
/* Float word order not known, assume not a hybrid. */
#define DUK_F_BYTEORDER 1
#else
/* byte order is little endian but cannot determine IEEE double word order */
#endif  /* float word order */
#elif defined(__ORDER_BIG_ENDIAN__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
/* Integer big endian */
#if defined(__FLOAT_WORD_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    (__FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__)
#define DUK_F_BYTEORDER 3
#elif !defined(__FLOAT_WORD_ORDER__)
/* Float word order not known, assume not a hybrid. */
#define DUK_F_BYTEORDER 3
#else
/* byte order is big endian but cannot determine IEEE double word order */
#endif  /* float word order */
#else
/* cannot determine byte order; __ORDER_PDP_ENDIAN__ is related to 32-bit
 * integer ordering and is not relevant
 */
#endif  /* integer byte order */
#endif  /* !defined(DUK_F_BYTEORDER) && defined(__BYTE_ORDER__) */

/* Atari ST TOS */
#if !defined(DUK_F_BYTEORDER) && defined(DUK_F_TOS)
#define DUK_F_BYTEORDER 3
#endif

/* AmigaOS on M68k */
#if !defined(DUK_F_BYTEORDER) && defined(DUK_F_AMIGAOS)
#if defined(DUK_F_M68K)
#define DUK_F_BYTEORDER 3
#endif
#endif

/* On Windows, assume we're little endian.  Even Itanium which has a
 * configurable endianness runs little endian in Windows.
 */
#if !defined(DUK_F_BYTEORDER) && defined(DUK_F_WINDOWS)
/* XXX: verify that Windows on ARM is little endian for floating point
 * values too.
 */
#define DUK_F_BYTEORDER 1
#endif  /* Windows */

/* Crossbridge should work with the standard byteorder #ifdefs.  It doesn't
 * provide _FLOAT_WORD_ORDER but the standard approach now covers that case
 * too.  This has been left here just in case.
 */
#if !defined(DUK_F_BYTEORDER) && defined(DUK_F_FLASHPLAYER)
#define DUK_F_BYTEORDER 1
#endif

/* QNX gcc cross compiler seems to define e.g. __LITTLEENDIAN__ or __BIGENDIAN__:
 *  $ /opt/qnx650/host/linux/x86/usr/bin/i486-pc-nto-qnx6.5.0-gcc -dM -E - </dev/null | grep -ni endian
 *  67:#define __LITTLEENDIAN__ 1
 *  $ /opt/qnx650/host/linux/x86/usr/bin/mips-unknown-nto-qnx6.5.0-gcc -dM -E - </dev/null | grep -ni endian
 *  81:#define __BIGENDIAN__ 1
 *  $ /opt/qnx650/host/linux/x86/usr/bin/arm-unknown-nto-qnx6.5.0-gcc -dM -E - </dev/null | grep -ni endian
 *  70:#define __LITTLEENDIAN__ 1
 */
#if !defined(DUK_F_BYTEORDER) && defined(DUK_F_QNX)
/* XXX: ARM hybrid? */
#if defined(__LITTLEENDIAN__)
#define DUK_F_BYTEORDER 1
#elif defined(__BIGENDIAN__)
#define DUK_F_BYTEORDER 3
#endif
#endif

/* Check whether or not byte order detection worked based on the intermediate
 * define, and define final values.  If detection failed, #error out.
 */
#if defined(DUK_F_BYTEORDER)
#if (DUK_F_BYTEORDER == 1)
#define DUK_USE_INTEGER_LE
#define DUK_USE_DOUBLE_LE
#elif (DUK_F_BYTEORDER == 2)
#define DUK_USE_INTEGER_LE  /* integer endianness is little on purpose */
#define DUK_USE_DOUBLE_ME
#elif (DUK_F_BYTEORDER == 3)
#define DUK_USE_INTEGER_BE
#define DUK_USE_DOUBLE_BE
#else
#error unsupported: byte order detection failed (internal error, should not happen)
#endif  /* byte order */
#else
#error unsupported: byte order detection failed
#endif  /* defined(DUK_F_BYTEORDER) */

/*
 *  Check whether or not a packed duk_tval representation is possible.
 *  What's basically required is that pointers are 32-bit values
 *  (sizeof(void *) == 4).  Best effort check, not always accurate.
 */

#undef DUK_USE_PACKED_TVAL_POSSIBLE
#if defined(UINTPTR_MAX) && (UINTPTR_MAX <= 0xffffffffUL)
/* strict C99 check */
#define DUK_USE_PACKED_TVAL_POSSIBLE
#endif

#if !defined(DUK_USE_PACKED_TVAL_POSSIBLE) && defined(DUK_SIZE_MAX) && !defined(DUK_SIZE_MAX_COMPUTED)
#if DUK_SIZE_MAX <= 0xffffffffUL
#define DUK_USE_PACKED_TVAL_POSSIBLE
#endif
#endif

#if !defined(DUK_USE_PACKED_TVAL_POSSIBLE) && defined(DUK_F_M68K)
#define DUK_USE_PACKED_TVAL_POSSIBLE
#endif

/* With Emscripten, force unpacked duk_tval just to be safe. */
#if defined(DUK_F_EMSCRIPTEN) && defined(DUK_USE_PACKED_TVAL_POSSIBLE)
#undef DUK_USE_PACKED_TVAL_POSSIBLE
#endif

/* GCC/clang inaccurate math would break compliance and probably duk_tval,
 * so refuse to compile.  Relax this if -ffast-math is tested to work.
 */
#if defined(__FAST_MATH__)
#error __FAST_MATH__ defined, refusing to compile
#endif

/*
 *  Detection of double constants and math related functions.  Availability
 *  of constants and math functions is a significant porting concern.
 *
 *  INFINITY/HUGE_VAL is problematic on GCC-3.3: it causes an overflow warning
 *  and there is no pragma in GCC-3.3 to disable it.  Using __builtin_inf()
 *  avoids this problem for some reason.
 */

#define DUK_DOUBLE_2TO32     4294967296.0
#define DUK_DOUBLE_2TO31     2147483648.0

#undef DUK_USE_COMPUTED_INFINITY
#if defined(DUK_F_GCC_VERSION) && (DUK_F_GCC_VERSION < 40600)
/* GCC older than 4.6: avoid overflow warnings related to using INFINITY */
#define DUK_DOUBLE_INFINITY  (__builtin_inf())
#elif defined(INFINITY)
#define DUK_DOUBLE_INFINITY  ((double) INFINITY)
#elif !defined(__VBCC__) && !defined(_MSC_VER)
#define DUK_DOUBLE_INFINITY  (1.0 / 0.0)
#else
/* In VBCC (1.0 / 0.0) results in a warning and 0.0 instead of infinity.
 * Use a computed infinity (initialized when a heap is created at the
 * latest).
 */
extern double duk_computed_infinity;
#define DUK_USE_COMPUTED_INFINITY
#define DUK_DOUBLE_INFINITY  duk_computed_infinity
#endif

#undef DUK_USE_COMPUTED_NAN
#if defined(NAN)
#define DUK_DOUBLE_NAN       NAN
#elif !defined(__VBCC__) && !defined(_MSC_VER)
#define DUK_DOUBLE_NAN       (0.0 / 0.0)
#else
/* In VBCC (0.0 / 0.0) results in a warning and 0.0 instead of NaN.
 * In MSVC (VS2010 Express) (0.0 / 0.0) results in a compile error.
 * Use a computed NaN (initialized when a heap is created at the
 * latest).
 */
extern double duk_computed_nan;
#define DUK_USE_COMPUTED_NAN
#define DUK_DOUBLE_NAN       duk_computed_nan
#endif

/* Many platforms are missing fpclassify() and friends, so use replacements
 * if necessary.  The replacement constants (FP_NAN etc) can be anything but
 * match Linux constants now.
 */
#undef DUK_USE_REPL_FPCLASSIFY
#undef DUK_USE_REPL_SIGNBIT
#undef DUK_USE_REPL_ISFINITE
#undef DUK_USE_REPL_ISNAN
#undef DUK_USE_REPL_ISINF

/* complex condition broken into separate parts */
#undef DUK_F_USE_REPL_ALL
#if !(defined(FP_NAN) && defined(FP_INFINITE) && defined(FP_ZERO) && \
      defined(FP_SUBNORMAL) && defined(FP_NORMAL))
/* missing some obvious constants */
#define DUK_F_USE_REPL_ALL
#elif defined(DUK_F_AMIGAOS) && defined(__VBCC__)
/* VBCC is missing the built-ins even in C99 mode (perhaps a header issue) */
#define DUK_F_USE_REPL_ALL
#elif defined(DUK_F_FREEBSD) && defined(DUK_F_CLANG)
/* Placeholder fix for (detection is wider than necessary):
 * http://llvm.org/bugs/show_bug.cgi?id=17788
 */
#define DUK_F_USE_REPL_ALL
#endif

#if defined(DUK_F_USE_REPL_ALL)
#define DUK_USE_REPL_FPCLASSIFY
#define DUK_USE_REPL_SIGNBIT
#define DUK_USE_REPL_ISFINITE
#define DUK_USE_REPL_ISNAN
#define DUK_USE_REPL_ISINF
#define DUK_FPCLASSIFY       duk_repl_fpclassify
#define DUK_SIGNBIT          duk_repl_signbit
#define DUK_ISFINITE         duk_repl_isfinite
#define DUK_ISNAN            duk_repl_isnan
#define DUK_ISINF            duk_repl_isinf
#define DUK_FP_NAN           0
#define DUK_FP_INFINITE      1
#define DUK_FP_ZERO          2
#define DUK_FP_SUBNORMAL     3
#define DUK_FP_NORMAL        4
#else
#define DUK_FPCLASSIFY       fpclassify
#define DUK_SIGNBIT          signbit
#define DUK_ISFINITE         isfinite
#define DUK_ISNAN            isnan
#define DUK_ISINF            isinf
#define DUK_FP_NAN           FP_NAN
#define DUK_FP_INFINITE      FP_INFINITE
#define DUK_FP_ZERO          FP_ZERO
#define DUK_FP_SUBNORMAL     FP_SUBNORMAL
#define DUK_FP_NORMAL        FP_NORMAL
#endif

#if defined(DUK_F_USE_REPL_ALL)
#undef DUK_F_USE_REPL_ALL
#endif

/* Some math functions are C99 only.  This is also an issue with some
 * embedded environments using uclibc where uclibc has been configured
 * not to provide some functions.  For now, use replacements whenever
 * using uclibc.
 */
#if defined(DUK_F_C99) && \
    !defined(__UCLIBC__) /* uclibc may be missing these */ && \
    !(defined(DUK_F_AMIGAOS) && defined(__VBCC__)) /* vbcc + AmigaOS may be missing these */
#define DUK_USE_MATH_FMIN
#define DUK_USE_MATH_FMAX
#define DUK_USE_MATH_ROUND
#else
#undef DUK_USE_MATH_FMIN
#undef DUK_USE_MATH_FMAX
#undef DUK_USE_MATH_ROUND
#endif

/* NetBSD 6.0 x86 (at least) has a few problems with pow() semantics,
 * see test-bug-netbsd-math-pow.js.  Use NetBSD specific workaround.
 * (This might be a wider problem; if so, generalize the define name.)
 */
#undef DUK_USE_POW_NETBSD_WORKAROUND
#if defined(DUK_F_NETBSD)
#define DUK_USE_POW_NETBSD_WORKAROUND
#endif

/*
 *  ANSI C string/memory function wrapper defines to allow easier workarounds.
 *  Also convenience macros like DUK_MEMZERO which may be mapped to existing
 *  platform function to zero memory (like the deprecated bzero).
 *
 *  For instance, some platforms don't support zero-size memcpy correctly,
 *  some arcane uclibc versions have a buggy memcpy (but working memmove)
 *  and so on.  Such broken platforms can be dealt with here.
 */

typedef FILE duk_file;
#define DUK_STDIN       stdin
#define DUK_STDOUT      stdout
#define DUK_STDERR      stderr

/* Old uclibcs have a broken memcpy so use memmove instead (this is overly
 * wide now on purpose):
 * http://lists.uclibc.org/pipermail/uclibc-cvs/2008-October/025511.html
 */
#if defined(__UCLIBC__)
#define DUK_MEMCPY       memmove
#else
#define DUK_MEMCPY       memcpy
#endif

#define DUK_MEMMOVE      memmove
#define DUK_MEMCMP       memcmp
#define DUK_MEMSET       memset
#define DUK_STRLEN       strlen
#define DUK_STRCMP       strcmp
#define DUK_STRNCMP      strncmp
#define DUK_PRINTF       printf
#define DUK_FPRINTF      fprintf
#define DUK_SPRINTF      sprintf
#if defined(DUK_F_MSVC)
/* _snprintf() does NOT NUL terminate on truncation, but Duktape code never
 * assumes that.
 * http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
 */
#define DUK_SNPRINTF     _snprintf
#else
#define DUK_SNPRINTF     snprintf
#endif
#define DUK_VSPRINTF     vsprintf
#define DUK_VSNPRINTF    vsnprintf
#define DUK_SSCANF       sscanf
#define DUK_VSSCANF      vsscanf
#define DUK_FOPEN        fopen
#define DUK_FCLOSE       fclose
#define DUK_FREAD        fread
#define DUK_FWRITE       fwrite
#define DUK_FSEEK        fseek
#define DUK_FTELL        ftell
#define DUK_FFLUSH       fflush

#define DUK_MEMZERO(p,n)  DUK_MEMSET((p), 0, (n))

/*
 *  Macro hackery to convert e.g. __LINE__ to a string without formatting,
 *  see: http://stackoverflow.com/questions/240353/convert-a-preprocessor-token-to-a-string
 */

#define DUK_F_STRINGIFY_HELPER(x)  #x
#define DUK_MACRO_STRINGIFY(x)  DUK_F_STRINGIFY_HELPER(x)

/*
 *  Cause segfault macro.
 *
 *  This is optionally used by panic handling to cause the program to segfault
 *  (instead of e.g. abort()) on panic.  Valgrind will then indicate the C
 *  call stack leading to the panic.
 */

#define DUK_CAUSE_SEGFAULT()  do { \
		*((uint32_t *) NULL) = (uint32_t) 0xdeadbeefUL; \
	} while (0)

/*
 *  Macro for suppressing warnings for potentially unreferenced variables.
 *  The variables can be actually unreferenced or unreferenced in some
 *  specific cases only; for instance, if a variable is only debug printed,
 *  it is unreferenced when debug printing is disabled.
 *
 *  (Introduced here because it's potentially compiler specific.)
 */

#define DUK_UNREF(x)  do { \
		(void) (x); \
	} while (0)

/*
 *  Macro for declaring a 'noreturn' function, detection in duktape.h.
 */

#define DUK_NORETURN(decl) DUK_API_NORETURN(decl)

/*
 *  Macro for stating that a certain line cannot be reached.
 *
 *  http://gcc.gnu.org/onlinedocs/gcc-4.5.0/gcc/Other-Builtins.html#Other-Builtins
 *  http://clang.llvm.org/docs/LanguageExtensions.html
 */

#if defined(DUK_F_GCC_VERSION) && (DUK_F_GCC_VERSION >= 40500)
/* since gcc-4.5 */
#define DUK_UNREACHABLE()  do { __builtin_unreachable(); } while(0)
#elif defined(__clang__) && defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
/* same as gcc */
#define DUK_UNREACHABLE()  do { __builtin_unreachable(); } while(0)
#endif
#else
/* unknown */
#endif

#if !defined(DUK_UNREACHABLE)
/* Don't know how to declare unreachable point, so don't do it; this
 * may cause some spurious compilation warnings (e.g. "variable used
 * uninitialized").
 */
#define DUK_UNREACHABLE()  /* unreachable */
#endif

/*
 *  Likely and unlikely branches.  Using these is not at all a clear cut case,
 *  so the selection is a two-step process: (1) DUK_USE_BRANCH_HINTS is set
 *  if the architecture, compiler etc make it useful to use the hints, and (2)
 *  a separate check determines how to do them.
 *
 *  These macros expect the argument to be a relational expression with an
 *  integer value.  If used with pointers, you should use an explicit check
 *  like:
 *
 *    if (DUK_LIKELY(ptr != NULL)) { ... }
 *
 *  instead of:
 *
 *    if (DUK_LIKELY(ptr)) { ... }
 *
 *  http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html  (__builtin_expect)
 */

/* pretty much a placeholder now */
#if defined(DUK_F_GCC)
#define DUK_USE_BRANCH_HINTS
#elif defined(DUK_F_CLANG)
#define DUK_USE_BRANCH_HINTS
#else
#undef DUK_USE_BRANCH_HINTS
#endif

#if defined(DUK_USE_BRANCH_HINTS)
#if defined(DUK_F_GCC_VERSION) && (DUK_F_GCC_VERISON >= 40500)
/* GCC: test not very accurate; enable only in relatively recent builds
 * because of bugs in gcc-4.4 (http://lists.debian.org/debian-gcc/2010/04/msg00000.html)
 */
#define DUK_LIKELY(x)    __builtin_expect((x), 1)
#define DUK_UNLIKELY(x)  __builtin_expect((x), 0)
#elif defined(DUK_F_CLANG)
#define DUK_LIKELY(x)    __builtin_expect((x), 1)
#define DUK_UNLIKELY(x)  __builtin_expect((x), 0)
#endif
#endif  /* DUK_USE_BRANCH_HINTS */

#if !defined(DUK_LIKELY)
#define DUK_LIKELY(x)    (x)
#endif
#if !defined(DUK_UNLIKELY)
#define DUK_UNLIKELY(x)  (x)
#endif

/*
 *  __FILE__, __LINE__, __func__ are wrapped.  Especially __func__ is a
 *  problem because it is not available even in some compilers which try
 *  to be C99 compatible (e.g. VBCC with -c99 option).
 */

#define DUK_FILE_MACRO  __FILE__

#define DUK_LINE_MACRO  __LINE__

#if !defined(__VBCC__) && !defined(_MSC_VER)
#define DUK_FUNC_MACRO  __func__
#else
#define DUK_FUNC_MACRO  "unknown"
#endif

/*
 *  Architecture string, human readable value exposed in Duktape.env
 */

#if defined(DUK_F_X86)
#define DUK_USE_ARCH_STRING "x86"
#elif defined(DUK_F_X64)
#define DUK_USE_ARCH_STRING "x64"
#elif defined(DUK_F_ARM)
#define DUK_USE_ARCH_STRING "arm"
#elif defined(DUK_F_MIPS)
#define DUK_USE_ARCH_STRING "mips"
#elif defined(DUK_F_M68K)
#define DUK_USE_ARCH_STRING "m68k"
#elif defined(DUK_F_FLASHPLAYER)
#define DUK_USE_ARCH_STRING "flashplayer"
#elif defined(DUK_F_EMSCRIPTEN)
#define DUK_USE_ARCH_STRING "emscripten"
#else
#define DUK_USE_ARCH_STRING "unknown"
#endif

/* 
 *  Tagged type representation (duk_tval)
 */

#undef DUK_USE_PACKED_TVAL
#undef DUK_USE_FULL_TVAL

#if defined(DUK_USE_PACKED_TVAL_POSSIBLE) && !defined(DUK_OPT_NO_PACKED_TVAL)
#define DUK_USE_PACKED_TVAL
#undef DUK_USE_FULL_TVAL
#endif

/*
 *  Memory management options
 */

#define DUK_USE_REFERENCE_COUNTING
#define DUK_USE_DOUBLE_LINKED_HEAP
#define DUK_USE_MARK_AND_SWEEP
#define DUK_USE_MS_STRINGTABLE_RESIZE
#undef DUK_USE_GC_TORTURE

#if defined(DUK_OPT_NO_REFERENCE_COUNTING)
#undef DUK_USE_REFERENCE_COUNTING
#undef DUK_USE_DOUBLE_LINKED_HEAP
/* XXX: undef DUK_USE_MS_STRINGTABLE_RESIZE as it is more expensive
 * with more frequent mark-and-sweeps?
 */
#endif

#if defined(DUK_OPT_NO_MARK_AND_SWEEP)
#undef DUK_USE_MARK_AND_SWEEP
#endif

#if defined(DUK_USE_MARK_AND_SWEEP)
#define DUK_USE_VOLUNTARY_GC
#if defined(DUK_OPT_NO_VOLUNTARY_GC)
#undef DUK_USE_VOLUNTARY_GC
#endif
#endif

#if !defined(DUK_USE_MARK_AND_SWEEP) && !defined(DUK_USE_REFERENCE_COUNTING)
#error must have either mark-and-sweep or reference counting enabled
#endif

#if defined(DUK_OPT_NO_MS_STRINGTABLE_RESIZE)
#undef DUK_USE_MS_STRINGTABLE_RESIZE
#endif

#if defined(DUK_OPT_GC_TORTURE)
#define DUK_USE_GC_TORTURE
#endif

/*
 *  Error handling options
 */

#define DUK_USE_AUGMENT_ERRORS
#define DUK_USE_TRACEBACKS
#define DUK_USE_VERBOSE_ERRORS

#if defined(DUK_OPT_NO_AUGMENT_ERRORS)
#undef DUK_USE_AUGMENT_ERRORS
#undef DUK_USE_TRACEBACKS
#elif defined(DUK_OPT_NO_TRACEBACKS)
#undef DUK_USE_TRACEBACKS
#endif

#if defined(DUK_OPT_NO_VERBOSE_ERRORS)
#undef DUK_USE_VERBOSE_ERRORS
#endif

#if defined(DUK_USE_TRACEBACKS)
#if defined(DUK_OPT_TRACEBACK_DEPTH)
#define DUK_USE_TRACEBACK_DEPTH  DUK_OPT_TRACEBACK_DEPTH
#else
#define DUK_USE_TRACEBACK_DEPTH  10
#endif
#endif

/* Include messages in executor internal errors. */
#define DUK_USE_VERBOSE_EXECUTOR_ERRORS

/*
 *  Execution and debugger options
 */

#define DUK_USE_INTERRUPT_COUNTER
#if defined(DUK_OPT_NO_INTERRUPT_COUNTER)
#undef DUK_USE_INTERRUPT_COUNTER
#endif

/*
 *  Debug printing and assertion options
 */

#undef DUK_USE_DEBUG
#undef DUK_USE_DDEBUG
#undef DUK_USE_DDDEBUG
#undef DUK_USE_DPRINT_RDTSC
#undef DUK_USE_ASSERTIONS

#if defined(DUK_OPT_DEBUG)
#define DUK_USE_DEBUG
#endif
#if defined(DUK_OPT_DDEBUG)
#define DUK_USE_DDEBUG
#endif
#if defined(DUK_OPT_DDDEBUG)
#define DUK_USE_DDDEBUG
#endif

#undef DUK_USE_DPRINT_COLORS
#if defined(DUK_OPT_DPRINT_COLORS)
#define DUK_USE_DPRINT_COLORS
#endif

#if defined(DUK_RDTSC_AVAILABLE) && defined(DUK_OPT_DPRINT_RDTSC)
#define DUK_USE_DPRINT_RDTSC
#else
#undef DUK_USE_DPRINT_RDTSC
#endif

#if defined(DUK_OPT_ASSERTIONS)
#define DUK_USE_ASSERTIONS
#endif

/* The static buffer for debug printing is quite large by default, so there
 * is an option to shrink it manually for constrained builds.
 */
#if defined(DUK_OPT_DEBUG_BUFSIZE)
#define DUK_USE_DEBUG_BUFSIZE  DUK_OPT_DEBUG_BUFSIZE
#else
#define DUK_USE_DEBUG_BUFSIZE  65536
#endif

/*
 *  Ecmascript features / compliance options
 */

#define DUK_USE_REGEXP_SUPPORT
#if defined(DUK_OPT_NO_REGEXP_SUPPORT)
#undef DUK_USE_REGEXP_SUPPORT
#endif

#undef DUK_USE_STRICT_UTF8_SOURCE
#if defined(DUK_OPT_STRICT_UTF8_SOURCE)
#define DUK_USE_STRICT_UTF8_SOURCE
#endif

#define DUK_USE_OCTAL_SUPPORT
#if defined(DUK_OPT_NO_OCTAL_SUPPORT)
#undef DUK_USE_OCTAL_SUPPORT
#endif

#define DUK_USE_SOURCE_NONBMP
#if defined(DUK_OPT_NO_SOURCE_NONBMP)
#undef DUK_USE_SOURCE_NONBMP
#endif

#define DUK_USE_BROWSER_LIKE
#if defined(DUK_OPT_NO_BROWSER_LIKE)
#undef DUK_USE_BROWSER_LIKE
#endif

#define DUK_USE_SECTION_B
#if defined(DUK_OPT_NO_SECTION_B)
#undef DUK_USE_SECTION_B
#endif

/* Treat function statements (function declarations outside top level of
 * Program or FunctionBody) same as normal function declarations.  This is
 * also V8 behavior.  See test-dev-func-decl-outside-top.js.
 */ 
#define DUK_USE_FUNC_STMT
#if defined(DUK_OPT_NO_FUNC_STMT)
#undef DUK_USE_FUNC_STMT
#endif

/* Array.prototype.splice() non-standard but real world compatible behavior
 * when deleteCount is omitted.
 */
#define DUK_USE_ARRAY_SPLICE_NONSTD_DELCOUNT
#if defined(DUK_OPT_NO_ARRAY_SPLICE_NONSTD_DELCOUNT)
#undef DUK_USE_ARRAY_SPLICE_NONSTD_DELCOUNT
#endif

/* Non-standard 'caller' property for function instances, see
 * test-bi-function-nonstd-caller-prop.js.
 */
#undef DUK_USE_FUNC_NONSTD_CALLER_PROPERTY
#if defined(DUK_OPT_FUNC_NONSTD_CALLER_PROPERTY)
#define DUK_USE_FUNC_NONSTD_CALLER_PROPERTY
#endif

/*
 *  Function instance features.
 */

#define DUK_USE_PC2LINE
#if defined(DUK_OPT_NO_PC2LINE)
#undef DUK_USE_PC2LINE
#endif

/*
 *  Deep vs. shallow stack.
 *
 *  Some embedded platforms have very shallow stack (e.g. 64kB); default to
 *  a shallow stack on unknown platforms or known embedded platforms.
 */

#if defined(DUK_F_LINUX) || defined(DUK_F_BSD) || defined(DUK_F_WINDOWS)
#define DUK_USE_DEEP_C_STACK
#else
#undef DUK_USE_DEEP_C_STACK
#endif

/*
 *  User panic handler, panic exit behavior for default panic handler
 */

#undef DUK_USE_PANIC_HANDLER
#if defined(DUK_OPT_PANIC_HANDLER)
#define DUK_USE_PANIC_HANDLER(code,msg) DUK_OPT_PANIC_HANDLER((code),(msg))
#endif

#undef DUK_USE_PANIC_ABORT
#undef DUK_USE_PANIC_EXIT
#undef DUK_USE_PANIC_SEGFAULT

#if defined(DUK_OPT_SEGFAULT_ON_PANIC)
#define DUK_USE_PANIC_SEGFAULT
#else
#define DUK_USE_PANIC_ABORT
#endif

/*
 *  File I/O support.  This is now used in a few API calls to e.g. push
 *  a string from file contents or eval a file.  For portability it must
 *  be possible to disable I/O altogether.
 */

#undef DUK_USE_FILE_IO
#if !defined(DUK_OPT_NO_FILE_IO)
#define DUK_USE_FILE_IO
#endif

/*
 *  Optional run-time self tests executed when a heap is created.  Some
 *  platform/compiler issues cannot be determined at compile time.  One
 *  particular example is the bug described in misc/clang_aliasing.c.
 */

#undef DUK_USE_SELF_TESTS
#if defined(DUK_OPT_SELF_TESTS)
#define DUK_USE_SELF_TESTS
#endif

/*
 *  Codecs
 */

#define DUK_USE_JSONX
#if defined(DUK_OPT_NO_JSONX)
#undef DUK_USE_JSONX
#endif

#define DUK_USE_JSONC
#if defined(DUK_OPT_NO_JSONC)
#undef DUK_USE_JSONC
#endif

/*
 *  InitJS code
 */

#define DUK_USE_INITJS

/*
 *  Miscellaneous
 */

#define DUK_USE_PROVIDE_DEFAULT_ALLOC_FUNCTIONS
#undef DUK_USE_EXPLICIT_NULL_INIT

#if !defined(DUK_USE_PACKED_TVAL)
#define DUK_USE_EXPLICIT_NULL_INIT
#endif

#if defined(DUK_F_C99) || (defined(DUK_F_CPP11) && defined(__GNUC__))
#define DUK_USE_VARIADIC_MACROS
#else
#undef DUK_USE_VARIADIC_MACROS
#endif

/*
 *  Variable size array initialization.
 *
 *  Variable size array at the end of a structure is nonportable. 
 *  There are three alternatives:
 *
 *    1) C99 (flexible array member): char buf[]
 *    2) Compiler specific (e.g. GCC): char buf[0]
 *    3) Portable but wastes memory / complicates allocation: char buf[1]
 */

/* XXX: Currently unused, only hbuffer.h needed this at some point. */
#undef DUK_USE_FLEX_C99
#undef DUK_USE_FLEX_ZEROSIZE
#undef DUK_USE_FLEX_ONESIZE
#if defined(DUK_F_C99)
#define DUK_USE_FLEX_C99
#elif defined(__GNUC__)
#define DUK_USE_FLEX_ZEROSIZE
#else
#define DUK_USE_FLEX_ONESIZE
#endif

/*
 *  GCC pragmas
 */

/* XXX: GCC pragma inside a function fails in some earlier GCC versions (e.g. gcc 4.5).
 * This is very approximate but allows clean builds for development right now.
 */
/* http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html */
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)
#define DUK_USE_GCC_PRAGMAS
#else
#undef DUK_USE_GCC_PRAGMAS
#endif

/*
 *  User declarations
 */

#if defined(DUK_OPT_DECLARE)
#define DUK_USE_USER_DECLARE() DUK_OPT_DECLARE
#else
#define DUK_USE_USER_DECLARE() /* no user declarations */
#endif

/*
 *  Alternative customization header
 *
 *  If you want to modify the final DUK_USE_xxx flags directly (without
 *  using the available DUK_OPT_Xxx flags), define DUK_OPT_HAVE_CUSTOM_H
 *  and tweak the final flags there.
 */

#if defined(DUK_OPT_HAVE_CUSTOM_H)
#include "duk_custom.h"
#endif

#endif  /* DUK_FEATURES_H_INCLUDED */

