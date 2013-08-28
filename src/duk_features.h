/*
 *  Determines the build-time feature defines to be used, based on
 *  DUK_PROFILE, other user-supplied defines, and feature detection.
 *  Defines individual DUK_USE_XXX defines which are (only) checked
 *  in internal code.
 *
 *  This is included before anything else.
 *
 *  Useful resources:
 *
 *    http://sourceforge.net/p/predef/wiki/Architectures/
 *
 *  FIXME: at the moment there is no direct way of configuring
 *  or overriding individual settings.
 */

#ifndef DUK_FEATURES_H_INCLUDED
#define DUK_FEATURES_H_INCLUDED

#include "duk_rdtsc.h"  /* DUK_RDTSC_AVAILABLE */

/*
 *  Feature detection (produce feature defines, but don't use them yet)
 */

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define  _DUK_C99
#else
#undef   _DUK_C99
#endif

/*
 *  Byte order and double memory layout detection
 *
 *  This needs to be done before choosing a default profile, as it affects
 *  profile selection.
 */

/* FIXME: Not very good detection right now, expect to find __BYTE_ORDER
 * and __FLOAT_WORD_ORDER or resort to GCC/ARM specifics.  Improve the
 * detection code and perhaps allow some compiler define to override the
 * detection for unhandled cases.
 */

#ifdef __APPLE__
#include <architecture/byte_order.h>
#else
#include <endian.h>
#endif

#include <limits.h>
#include <sys/param.h>

/* determine endianness variant: little-endian (LE), big-endian (BE), or "middle-endian" (ME) i.e. ARM */
#if (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN)) || \
    (defined(__LITTLE_ENDIAN__))
#if defined(__FLOAT_WORD_ORDER) && defined(__LITTLE_ENDIAN) && (__FLOAT_WORD_ORDER == __LITTLE_ENDIAN) || \
    (defined(__GNUC__) && !defined(__arm__))
#define DUK_USE_DOUBLE_LE
#elif (defined(__FLOAT_WORD_ORDER) && defined(__BIG_ENDIAN) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN)) || \
      (defined(__GNUC__) && defined(__arm__))
#define DUK_USE_DOUBLE_ME
#else
#error unsupported: byte order is little endian but cannot determine IEEE double word order
#endif
#elif (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN)) || \
      (defined(__BIG_ENDIAN__))
#if (defined(__FLOAT_WORD_ORDER) && defined(__BIG_ENDIAN) && (__FLOAT_WORD_ORDER == __BIG_ENDIAN)) || \
    (defined(__GNUC__) && !defined(__arm__))
#define DUK_USE_DOUBLE_BE
#else
#error unsupported: byte order is big endian but cannot determine IEEE double word order
#endif
#else
#error unsupported: cannot determine byte order
#endif

#if !defined(DUK_USE_DOUBLE_LE) && !defined(DUK_USE_DOUBLE_ME) && !defined(DUK_USE_DOUBLE_BE)
#error unsupported: cannot determine IEEE double byte order variant
#endif

/*
 *  Check whether or not a packed duk_tval representation is possible
 */

/* best effort viability checks, not particularly accurate */
#if (defined(__WORDSIZE) && (__WORDSIZE == 32)) && \
    (defined(UINT_MAX) && (UINT_MAX == 4294967295))
#define DUK_USE_PACKED_TVAL_POSSIBLE
#else
#undef  DUK_USE_PACKED_TVAL_POSSIBLE
#endif

/*
 *  Support for unaligned accesses
 */

/* FIXME: currently just a hack for ARM, what would be a good way to detect? */
#if defined(__arm__) || defined(__thumb__) || defined(_ARM) || defined(_M_ARM)
#undef   DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#else
#define  DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#endif

/* 
 *  Profile processing
 *
 *  DUK_PROFILE values:
 *    0      custom
 *    100    FULL
 *    101    FULL_DEBUG
 *    200    MINIMAL
 *    201    MINIMAL_DEBUG
 *    300    TINY
 *    301    TINY_DEBUG
 *    400    PORTABLE        [tagged types]
 *    401    PORTABLE_DEBUG  [tagged types]
 *    500    TORTURE         [tagged types + torture]
 *    501    TORTURE_DEBUG   [tagged types + torture]
 */

#if !defined(DUK_PROFILE)
#if defined(DUK_USE_PACKED_TVAL_POSSIBLE)
#define  DUK_PROFILE  100
#else
#define  DUK_PROFILE  400
#endif
#endif

#if (DUK_PROFILE > 0)

/* start with the settings for the FULL profile */

#define  DUK_USE_SELF_TEST_TVAL
#define  DUK_USE_PACKED_TVAL
#undef   DUK_USE_FULL_TVAL
#define  DUK_USE_REFERENCE_COUNTING
#define  DUK_USE_DOUBLE_LINKED_HEAP
#define  DUK_USE_MARK_AND_SWEEP
#define  DUK_USE_AUGMENT_ERRORS
#define  DUK_USE_TRACEBACKS
#undef   DUK_USE_GC_TORTURE
#undef   DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#undef   DUK_USE_DPRINT_RDTSC                       /* feature determination below */
#define  DUK_USE_VERBOSE_ERRORS
#undef   DUK_USE_ASSERTIONS
#undef   DUK_USE_VARIADIC_MACROS                    /* feature determination below */
#define  DUK_USE_PROVIDE_DEFAULT_ALLOC_FUNCTIONS
#undef   DUK_USE_EXPLICIT_NULL_INIT
#define  DUK_USE_REGEXP_SUPPORT
#define  DUK_USE_STRICT_UTF8_SOURCE
#define  DUK_USE_OCTAL_SUPPORT
#define  DUK_USE_SOURCE_NONBMP
#define  DUK_USE_DPRINT_COLORS
#define  DUK_USE_BROWSER_LIKE
#define  DUK_USE_SECTION_B

/* unaligned accesses */
#ifdef DUK_USE_UNALIGNED_ACCESSES_POSSIBLE
#define  DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS
#define  DUK_USE_HOBJECT_UNALIGNED_LAYOUT
#else
#undef   DUK_USE_HASHBYTES_UNALIGNED_U32_ACCESS
#undef   DUK_USE_HOBJECT_UNALIGNED_LAYOUT
#endif

/* profile specific modifications */

#if (DUK_PROFILE == 100)
/* FULL */
#elif (DUK_PROFILE == 101)
/* FULL_DEBUG */
#define  DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#define  DUK_USE_ASSERTIONS
#elif (DUK_PROFILE == 200)
/* MINIMAL */
#undef   DUK_USE_VERBOSE_ERRORS
#elif (DUK_PROFILE == 201)
/* MINIMAL_DEBUG */
#define  DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#define  DUK_USE_ASSERTIONS
#elif (DUK_PROFILE == 300)
/* TINY */
#undef   DUK_USE_SELF_TEST_TVAL
#undef   DUK_USE_REFERENCE_COUNTING
#undef   DUK_USE_DOUBLE_LINKED_HEAP
#define  DUK_USE_MARK_AND_SWEEP
#undef   DUK_USE_AUGMENT_ERRORS
#undef   DUK_USE_TRACEBACKS
#undef   DUK_USE_VERBOSE_ERRORS
#elif (DUK_PROFILE == 301)
/* TINY_DEBUG */
#undef   DUK_USE_SELF_TEST_TVAL
#undef   DUK_USE_REFERENCE_COUNTING
#undef   DUK_USE_DOUBLE_LINKED_HEAP
#define  DUK_USE_MARK_AND_SWEEP
#undef   DUK_USE_AUGMENT_ERRORS
#undef   DUK_USE_TRACEBACKS
#define  DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#define  DUK_USE_ASSERTIONS
#undef   DUK_USE_VERBOSE_ERRORS
#elif (DUK_PROFILE == 400)
#undef   DUK_USE_PACKED_TVAL
#undef   DUK_USE_FULL_TVAL
#define  DUK_USE_EXPLICIT_NULL_INIT
#elif (DUK_PROFILE == 401)
#undef   DUK_USE_PACKED_TVAL
#undef   DUK_USE_FULL_TVAL
#undef   DUK_USE_GC_TORTURE
#define  DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#define  DUK_USE_ASSERTIONS
#define  DUK_USE_EXPLICIT_NULL_INIT
#elif (DUK_PROFILE == 500)
#undef   DUK_USE_PACKED_TVAL
#undef   DUK_USE_FULL_TVAL
#define  DUK_USE_GC_TORTURE
#elif (DUK_PROFILE == 501)
#undef   DUK_USE_PACKED_TVAL
#undef   DUK_USE_FULL_TVAL
#define  DUK_USE_GC_TORTURE
#define  DUK_USE_DEBUG
#undef   DUK_USE_DDEBUG
#undef   DUK_USE_DDDEBUG
#undef   DUK_USE_ASSERTIONS
#else
#error unknown DUK_PROFILE
#endif

/* FIXME: how to handle constants like these? */
#if defined(DUK_USE_TRACEBACKS) && !defined(DUK_OPT_TRACEBACK_DEPTH)
#define  DUK_OPT_TRACEBACK_DEPTH  10
#endif

/*
 *  Dynamically detected features
 */

#if defined(DUK_RDTSC_AVAILABLE) && defined(DUK_OPT_DPRINT_RDTSC)
#define  DUK_USE_DPRINT_RDTSC
#else
#undef  DUK_USE_DPRINT_RDTSC
#endif

#ifdef _DUK_C99
#define  DUK_USE_VARIADIC_MACROS
#else
#undef  DUK_USE_VARIADIC_MACROS
#endif

/* zero-size array at end of struct (char buf[0]) instead of C99 version (char buf[]) */
#ifdef _DUK_C99
#undef  DUK_USE_STRUCT_HACK
#else
#define  DUK_USE_STRUCT_HACK  /* non-portable */
#endif

/* FIXME: GCC pragma inside a function fails in some earlier GCC versions.
 * This is very approximate but allows clean builds for development right now.
 */
/* http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html */
#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)
#define  DUK_USE_GCC_PRAGMAS
#else
#undef  DUK_USE_GCC_PRAGMAS
#endif

/*
 *  Platform specific defines
 *
 *  http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
 */

/* NOW = getting current time (required)
 * TZO = getting local time offset (required)
 * PRS = parse datetime (optional)
 * FMT = format datetime (optional)
 */

#if defined(_WIN64)
/* Windows 64-bit */
#error WIN64 not supported
#elif defined(_WIN32) || defined(WIN32)
/* Windows 32-bit */
#error WIN32 not supported
#elif defined(__APPLE__)
/* Mac OSX, iPhone, Darwin */
#define  DUK_USE_DATE_NOW_GETTIMEOFDAY
#define  DUK_USE_DATE_TZO_GMTIME
#define  DUK_USE_DATE_PRS_STRPTIME
#define  DUK_USE_DATE_FMT_STRFTIME
#elif defined(__linux)
/* Linux (__unix also defined) */
#define  DUK_USE_DATE_NOW_GETTIMEOFDAY
#define  DUK_USE_DATE_TZO_GMTIME
#define  DUK_USE_DATE_PRS_STRPTIME
#define  DUK_USE_DATE_FMT_STRFTIME
#elif defined(__unix)
/* Other Unix */
#define  DUK_USE_DATE_NOW_GETTIMEOFDAY
#define  DUK_USE_DATE_TZO_GMTIME
#define  DUK_USE_DATE_PRS_STRPTIME
#define  DUK_USE_DATE_FMT_STRFTIME
#elif defined(__posix)
/* POSIX */
#define  DUK_USE_DATE_NOW_GETTIMEOFDAY
#define  DUK_USE_DATE_TZO_GMTIME
#define  DUK_USE_DATE_PRS_STRPTIME
#define  DUK_USE_DATE_FMT_STRFTIME
#else
#error platform not supported
#endif

#else  /* DUK_PROFILE > 0 */

/*
 *  All DUK_USE_ defines must be defined manually, no compiler
 *  or platform feature detection.
 */

#endif  /* DUK_PROFILE > 0 */

/* FIXME: An alternative approach to customization would be to include
 * some user define file at this point.  The user file could then modify
 * the base settings.  Something like:
#ifdef DUK_CUSTOM_HEADER
#include "duk_custom.h"
#endif
 */

/*
 *  Sanity checks on defines
 */

#if defined(DUK_DDEBUG) && !defined(DUK_DEBUG)
#error DUK_DEBUG and DUK_DDEBUG should not be defined (obsolete)
#endif

#if defined(DUK_USE_DDEBUG) && !defined(DUK_USE_DEBUG)
#error DUK_USE_DDEBUG defined without DUK_USE_DEBUG
#endif

#if defined(DUK_USE_DDDEBUG) && !defined(DUK_USE_DEBUG)
#error DUK_USE_DDDEBUG defined without DUK_USE_DEBUG
#endif

#if defined(DUK_USE_DDDEBUG) && !defined(DUK_USE_DDEBUG)
#error DUK_USE_DDDEBUG defined without DUK_USE_DDEBUG
#endif

#if defined(DUK_USE_REFERENCE_COUNTING) && !defined(DUK_USE_DOUBLE_LINKED_HEAP)
#error DUK_USE_REFERENCE_COUNTING defined without DUK_USE_DOUBLE_LINKED_HEAP
#endif

#if defined(DUK_USE_GC_TORTURE) && !defined(DUK_USE_MARK_AND_SWEEP)
#error DUK_USE_GC_TORTURE defined without DUK_USE_MARK_AND_SWEEP
#endif

#endif  /* DUK_FEATURES_H_INCLUDED */

