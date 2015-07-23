/*
 *  Debugging macro calls.
 */

#include "duk_internal.h"

#ifdef DUK_USE_DEBUG

/*
 *  Debugging enabled
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DUK__DEBUG_BUFSIZE  DUK_USE_DEBUG_BUFSIZE
DUK_LOCAL char duk__debug_buf[DUK__DEBUG_BUFSIZE];

DUK_LOCAL const char *duk__get_level_string(duk_small_int_t level) {
	switch ((int) level) {
	case DUK_LEVEL_DEBUG:
		return "D";
	case DUK_LEVEL_DDEBUG:
		return "DD";
	case DUK_LEVEL_DDDEBUG:
		return "DDD";
	}
	return "???";
}

#ifdef DUK_USE_DPRINT_COLORS

/* http://en.wikipedia.org/wiki/ANSI_escape_code */
#define DUK__TERM_REVERSE  "\x1b[7m"
#define DUK__TERM_BRIGHT   "\x1b[1m"
#define DUK__TERM_RESET    "\x1b[0m"
#define DUK__TERM_BLUE     "\x1b[34m"
#define DUK__TERM_RED      "\x1b[31m"

DUK_LOCAL const char *duk__get_term_1(duk_small_int_t level) {
	DUK_UNREF(level);
	return (const char *) DUK__TERM_RED;
}

DUK_LOCAL const char *duk__get_term_2(duk_small_int_t level) {
	switch ((int) level) {
	case DUK_LEVEL_DEBUG:
		return (const char *) (DUK__TERM_RESET DUK__TERM_BRIGHT);
	case DUK_LEVEL_DDEBUG:
		return (const char *) (DUK__TERM_RESET);
	case DUK_LEVEL_DDDEBUG:
		return (const char *) (DUK__TERM_RESET DUK__TERM_BLUE);
	}
	return (const char *) DUK__TERM_RESET;
}

DUK_LOCAL const char *duk__get_term_3(duk_small_int_t level) {
	DUK_UNREF(level);
	return (const char *) DUK__TERM_RESET;
}

#else

DUK_LOCAL const char *duk__get_term_1(duk_small_int_t level) {
	DUK_UNREF(level);
	return (const char *) "";
}

DUK_LOCAL const char *duk__get_term_2(duk_small_int_t level) {
	DUK_UNREF(level);
	return (const char *) "";
}

DUK_LOCAL const char *duk__get_term_3(duk_small_int_t level) {
	DUK_UNREF(level);
	return (const char *) "";
}

#endif  /* DUK_USE_DPRINT_COLORS */

#ifdef DUK_USE_VARIADIC_MACROS

DUK_INTERNAL void duk_debug_log(duk_small_int_t level, const char *file, duk_int_t line, const char *func, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

	DUK_MEMZERO((void *) duk__debug_buf, (size_t) DUK__DEBUG_BUFSIZE);
	duk_debug_vsnprintf(duk__debug_buf, DUK__DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	DUK_FPRINTF(DUK_STDERR, "%s[%s] <%llu> %s:%ld (%s):%s %s%s\n",
	            (const char *) duk__get_term_1(level),
	            (const char *) duk__get_level_string(level),
	            (unsigned long long) DUK_USE_RDTSC(),  /* match the inline asm in duk_features.h */
	            (const char *) file,
	            (long) line,
	            (const char *) func,
	            (const char *) duk__get_term_2(level),
	            (const char *) duk__debug_buf,
	            (const char *) duk__get_term_3(level));
#else
	DUK_FPRINTF(DUK_STDERR, "%s[%s] %s:%ld (%s):%s %s%s\n",
	            (const char *) duk__get_term_1(level),
	            (const char *) duk__get_level_string(level),
	            (const char *) file,
	            (long) line,
	            (const char *) func,
	            (const char *) duk__get_term_2(level),
	            (const char *) duk__debug_buf,
	            (const char *) duk__get_term_3(level));
#endif
	DUK_FFLUSH(DUK_STDERR);

	va_end(ap);
}

#else  /* DUK_USE_VARIADIC_MACROS */

DUK_INTERNAL char duk_debug_file_stash[DUK_DEBUG_STASH_SIZE];
DUK_INTERNAL char duk_debug_line_stash[DUK_DEBUG_STASH_SIZE];
DUK_INTERNAL char duk_debug_func_stash[DUK_DEBUG_STASH_SIZE];
DUK_INTERNAL duk_small_int_t duk_debug_level_stash;

DUK_INTERNAL void duk_debug_log(const char *fmt, ...) {
	va_list ap;
	duk_small_int_t level = duk_debug_level_stash;

	va_start(ap, fmt);

	DUK_MEMZERO((void *) duk__debug_buf, (size_t) DUK__DEBUG_BUFSIZE);
	duk_debug_vsnprintf(duk__debug_buf, DUK__DEBUG_BUFSIZE - 1, fmt, ap);

#ifdef DUK_USE_DPRINT_RDTSC
	DUK_FPRINTF(DUK_STDERR, "%s[%s] <%llu> %s:%s (%s):%s %s%s\n",
	            (const char *) duk__get_term_1(level),
	            (const char *) duk__get_level_string(duk_debug_level_stash),
	            (unsigned long long) DUK_USE_RDTSC(),  /* match duk_features.h */
	            (const char *) duk_debug_file_stash,
	            (const char *) duk_debug_line_stash,
	            (const char *) duk_debug_func_stash,
	            (const char *) duk__get_term_2(level),
	            (const char *) duk__debug_buf,
	            (const char *) duk__get_term_3(level));
#else
	DUK_FPRINTF(DUK_STDERR, "%s[%s] %s:%s (%s):%s %s%s\n",
	            (const char *) duk__get_term_1(level),
	            (const char *) duk__get_level_string(duk_debug_level_stash),
	            (const char *) duk_debug_file_stash,
	            (const char *) duk_debug_line_stash,
	            (const char *) duk_debug_func_stash,
	            (const char *) duk__get_term_2(level),
	            (const char *) duk__debug_buf,
	            (const char *) duk__get_term_3(level));
#endif
	DUK_FFLUSH(DUK_STDERR);

	va_end(ap);
}

#endif  /* DUK_USE_VARIADIC_MACROS */

#else  /* DUK_USE_DEBUG */

/*
 *  Debugging disabled
 */

#endif  /* DUK_USE_DEBUG */
